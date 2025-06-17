/* Copyright 2020,2022,2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"
#if !defined(UWBIOT_APP_BUILD__DEMO_PNP)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_PNP)

#include <stdio.h>
#include "board.h"

#include "UWBT_PowerMode.h"
#include "LED.h"
#include "GPIO.h"
#include "UWB_Spi_Driver_Interface.h"
#include "UWB_Evt_Pnp.h"
#include "Uwb_Read_task.h"
#include "peripherals.h"
#include "UwbPnpInternal.h"
#include "phOsalUwb.h"
#include "phUwbTypes.h"
#include "UWB_DRIVER.h"
#include "phTmlUwb_transport.h"
#include "uwbiot_ver.h"
#include "UwbApi_Utility.h"

#define HIF_TASK_PRIO        3
#define PNP_APP_TASK_PRIO    3
#define UCI_READER_TASK_PRIO 3
#define HIF_WRITER_TASK_PRIO 2

UWBOSAL_TASK_HANDLE mHifTask;
UWBOSAL_TASK_HANDLE mPnpAppTask;
UWBOSAL_TASK_HANDLE mUciReaderTask;
UWBOSAL_TASK_HANDLE mHifWriterTask;
void *mHifWriteMutex = NULL;
void *mHifSyncMutex  = NULL;
intptr_t mHifWriteQueue;
void *mHifIsr_Sem = NULL;
static uint8_t mRxData[UWB_MAX_HELIOS_RSP_SIZE];
static uint8_t mTlvBuf[TLV_RESP_SIZE];

/* Allocate the memory for the heap. */
uint8_t __attribute__((section(".bss.$SRAM1"))) ucHeap[configTOTAL_HEAP_SIZE];

/*USB header is handled here as per designed doc*/
void UWB_Handle_SR1XXT_TLV(tlv_t *tlv)
{
    switch (tlv->type) {
    case UCI_CMD: {
#if (ENABLE_UCI_CMD_LOGGING == ENABLED)
        PRINTF("UCI cmd:");
        for (int i = 0; i < tlv->size; i++) {
            PRINTF(" %02x", tlv->value[i]);
        }
        PRINTF("\n");
#endif

        if (UWB_SpiUciWrite(tlv->value, tlv->size) == kUWBSTATUS_SUCCESS) {
        }
        else {
            PRINTF("ERROR: error processing UCI command\n");
        }

    } break;

    case HBCI_CMD: {
        size_t rspLen = UWB_MAX_HELIOS_RSP_SIZE;
#if (ENABLE_UCI_CMD_LOGGING == ENABLED)
        PRINTF("HBCI cmd:");
        for (uint16_t i = 0; i < tlv->size; i++) {
            PRINTF(" %02x", tlv->value[i]);
        }
        PRINTF("\n");
#endif

        if (kUWBSTATUS_SUCCESS == UWB_SpiHbciXfer(tlv->value, tlv->size, mRxData, &rspLen)) {
            UWB_Hif_SendRsp(mRxData, rspLen);
        }
        else {
            PRINTF("ERROR: error processing UCI command\n");
        }
    } break;

    case HBCI_QUERY_CMD: {
#if (ENABLE_UCI_CMD_LOGGING == ENABLED)
        PRINTF("HBCI Query cmd:");
        for (int i = 1; i < tlv->size; i++) {
            PRINTF(" %02x", tlv->value[i]);
        }
        PRINTF("\n");
#endif
        uint8_t readLen = tlv->value[0];
        if (kUWBSTATUS_SUCCESS == UWB_SpiHbciXferWithLen(&tlv->value[1], (tlv->size - 1), mRxData, readLen)) {
            if (readLen > 0) {
                UWB_Hif_SendRsp(mRxData, readLen);
            }
        }
        else {
            PRINTF("ERROR: error processing UCI command\n");
        }
    } break;

    case HBCI_LAST_CMD: {
        size_t rspLen = UWB_MAX_HELIOS_RSP_SIZE;
#if (ENABLE_UCI_CMD_LOGGING == ENABLED)
        PRINTF("HBCI Last cmd:");
        for (int i = 0; i < tlv->size; i++) {
            PRINTF(" %02x", tlv->value[i]);
        }
        PRINTF("\n");
#endif
        /* For the query command after HBCI image DND, host needs to wait till IRQ is received from Helios */
        if (!UWB_Uwbs_Interupt_Status()) {
            phOsalUwb_Delay(100);
        }
        if (kUWBSTATUS_SUCCESS == UWB_SpiHbciXfer(tlv->value, tlv->size, mRxData, &rspLen)) {
            UWB_Hif_SendRsp(mRxData, rspLen);
        }
        else {
            PRINTF("ERROR: error processing UCI command\n");
        }
        /*After Query command Spurious Interrupt is expected, Add 100msec delay before resuming UCI UCI read task*/
        phOsalUwb_Delay(10);
        /* Set back tml transfer mode to uci before resuming the tml reader task */
        UWB_Tml_Set_Mode(kUWB_UWBS_TML_MODE_UCI);
        /* After HBCI DND resume UCI mode by resuming tasks required for UCI*/
        phOsalUwb_TaskResume(mUciReaderTask);
        phOsalUwb_TaskResume(mHifWriterTask);
        UWB_Uwbs_Enable_Interrupt();
    } break;
    case RESET: {
        PRINTF_WITH_TIME("Reset Received\n");
        /*Acquire mReadTaskSyncMutex to make sure there is no pending
             * operation in the read task and then suspend read task*/
        //(void)phOsalUwb_LockMutex(mReadTaskSyncMutex);
        //PRINTF_WITH_TIME("After mReadTaskSyncMutex\n");
        /*Acquire mUsbSyncMutex to make sure there is no pending
             * operation in the mUsbWriterTask task and then suspend mUsbWriterTask task*/
        (void)phOsalUwb_LockMutex(mHifSyncMutex);
        PRINTF_WITH_TIME("After mUsbSyncMutex\n");
        phOsalUwb_TaskSuspend(mUciReaderTask);
        phOsalUwb_TaskSuspend(mHifWriterTask);

        /*Delete all the elements from the USB Write Queue before going in to Bootrom mode*/
        int itemsInQueue = (WRITER_QUEUE_SIZE - phOsalUwb_queueSpacesAvailable(mHifWriteQueue));
        PRINTF_WITH_TIME("items in Queue : %d\n", itemsInQueue);
        for (int i = 0; i < itemsInQueue; i++) {
            phLibUwb_Message_t tlv;
            if (phOsalUwb_msgrcv(mHifWriteQueue, &tlv, NO_DELAY) == UWBSTATUS_FAILED) {
                PRINTF("Failed to Receive an item\n");
                continue;
            }
            if (tlv.pMsgData != NULL) {
                phOsalUwb_FreeMemory(tlv.pMsgData);
                tlv.pMsgData = NULL;
                PRINTF("FREE\r\n");
            }
        }
        PRINTF_WITH_TIME("Chip Enable Started\n");
        // Assert/deassert pin
        UWB_Tml_Io_Set(kUWBS_IO_O_ENABLE_HELIOS, 0);
        UWB_Tml_Io_Set(kUWBS_IO_O_HELIOS_RTC_SYNC, 0);
        phOsalUwb_Delay(5);
        UWB_Tml_Io_Set(kUWBS_IO_O_ENABLE_HELIOS, 1);
        UWB_Tml_Io_Set(kUWBS_IO_O_HELIOS_RTC_SYNC, 1);
        phOsalUwb_Delay(50);
        PRINTF_WITH_TIME("Chip Enable Completed\n");
        (void)phOsalUwb_UnlockMutex(mHifSyncMutex);
        //(void)phOsalUwb_UnlockMutex(mReadTaskSyncMutex);
        mTlvBuf[0] = 0x01;
        mTlvBuf[1] = 0x02;
        mTlvBuf[2] = 0x03;
        mTlvBuf[3] = 0x04;
        /* set HBCI MODE */
        UWB_Tml_Set_Mode(kUWB_UWBS_TML_MODE_HBCI);
        // Send response over USB to HOST upper layer
        UWB_Hif_SendRsp(mTlvBuf, RESET_RESPONSE_SIZE);
    } break;
    case GET_SOFTWARE_VERSION: {
        mTlvBuf[0] = GET_SOFTWARE_VERSION;
        mTlvBuf[1] = 0x02;
        mTlvBuf[2] = UWBIOTVER_STR_VER_MAJOR;
        mTlvBuf[3] = UWBIOTVER_STR_VER_MINOR;
        UWB_Hif_SendRsp(mTlvBuf, GET_SOFTWARE_VERSION_RESPONSE_SIZE);
    } break;
    case GET_BOARD_ID: {
        uint8_t len = 0;
        mTlvBuf[0]  = GET_BOARD_ID;
        mTlvBuf[1]  = 0x10;
        BOARD_GetMCUUid(&mTlvBuf[2], &len);
        UWB_Hif_SendRsp(mTlvBuf, (uint16_t)(len + 2));
    } break;
    case MCU_RESET: {
        PRINTF("Issuing NVIC_SystemReset, MCU going to reboot!\n");
        BOARD_MCUReset(); // No prints after this line wont appear
    } break;
    case USB_LOOPBACK: { // echo back to usb
        UWB_Hif_SendRsp(tlv->value - 3, (uint16_t)tlv->size + 3);
    } break;
    case GET_BOARD_VERSION: {
        mTlvBuf[0] = UWB_BOARD_VERSION;
        UWB_Hif_SendRsp(mTlvBuf, 1);
    } break;
    case GET_VERISON_INFO: {
        size_t rspSize = UWBPnP_GetVersionInfo(mTlvBuf, sizeof(mTlvBuf), kUBWPnPBoardIdentifier_SR1XX_RV4);
        UWB_Hif_SendRsp(mTlvBuf, rspSize);
    } break;
    default:
        PRINTF("ERROR: invalid TLV type %02x\n", tlv->type);
    }
}

/*
 * @brief   Application entry point.
 */
int main(void)
{
    phOsalUwb_ThreadCreationParams_t threadParams;
    /* Init board hardware. */
    hardware_init();

    LED_Init();
    GPIO_InitTimer();
    /* Override pin configurations with SWO, if in debug session */
    //BOARD_InitSwoPins();
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    /* DeInitialize UART Debug instance */
    BOARD_DeinitDebugConsole();
    /* Enable the SWO Debug consle for only in debug session */
    if (IS_DEBUG_SESSION) {
        BOARD_InitSwoDebugConsole();
    }

    PRINT_APP_NAME("Demo PNP Rhodes V4");

    /* Init Helios subsystem */
    UWB_Tml_Io_Init();

    /* Init Helios subsystem */
    if (kUWBSTATUS_SUCCESS == UWB_HeliosSpiInit()) {
        PRINTF_WITH_TIME("main(): Helios initialized\n");
    }
    else {
        PRINTF_WITH_TIME("CRITICAL: error initializing Helios\n");
        while (1)
            ;
    }

    PRINTF_WITH_TIME("main(): GPIO/IRQ module initialized\n");

    /* This mutex is used to make USB write operations(Bulkin) from Rhodes mutually exclusive . */
    if (phOsalUwb_CreateMutex(&mHifWriteMutex) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: UWB_HeliosInit(), could not create mutex mUsbWriteMutex\n");
        while (1)
            ;
    }
    /* This mutex is used to make Reset operation and USB write operation from UWB_WriterTask mutually exclusive
     * anytime Host can send a Reset command when ranging is ongoing*/
    if (phOsalUwb_CreateMutex(&mHifSyncMutex) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: UWB_HeliosInit(), could not create mutex mUsbSyncMutex\n");
        while (1)
            ;
    }
    /* This semaphore is signaled in the USB CDC ISR context when any command is received from Host*/
    if (phOsalUwb_CreateBinSem(&mHifIsr_Sem) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mSem\n");
        while (1)
            ;
    }
    /* This Queue is used to store the notifications received from helios
     * Currently it can store WRITER_QUEUE_SIZE elements*/
    mHifWriteQueue = phOsalUwb_msgget(WRITER_QUEUE_SIZE);
    if (!mHifWriteQueue) {
        PRINTF_WITH_TIME("Error: main, could not create queue mUsbWriteQueue\n");
        while (1)
            ;
    }
    /* This Queue is used to store the commands received from Host
     * Currently it can store MAX 1 element at a time*/
    mHifCommandQueue = phOsalUwb_msgget(1);
    if (!mHifCommandQueue) {
        PRINTF_WITH_TIME("Error: main, could not create queue mHifCommandQueue\n");
        while (1)
            ;
    }

    threadParams.stackdepth = PNP_APP_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadParams, "UWB_Pnp_App_Task");
    threadParams.pContext = NULL;
    threadParams.priority = PNP_APP_TASK_PRIO;
    /*This is the PNP Rhodes Application task which receives all the command sent by Host*/
    phOsalUwb_Thread_Create((void **)&mPnpAppTask, &UWB_Pnp_App_Task, &threadParams);

    threadParams.stackdepth = HIF_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadParams, "UWB_HIF_Task");
    threadParams.pContext = NULL;
    threadParams.priority = HIF_TASK_PRIO;
    /*This task is waiting on a CDC USB interrupt. Once USB command is received, it is forwarded to UWB_HeliosTask queue for the
     *further processing */
    phOsalUwb_Thread_Create((void **)&mHifTask, &UWB_Hif_Handler_Task, &threadParams);

    threadParams.stackdepth = UCI_READER_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadParams, "UCI_ReaderTask");
    threadParams.pContext = NULL;
    threadParams.priority = UCI_READER_TASK_PRIO;
    /*This task is used for reading UCI resp and notifications from Helios. its blocked on a helios IRQ interrupt*/
    phOsalUwb_Thread_Create((void **)&mUciReaderTask, &UCI_ReaderTask, &threadParams);

    threadParams.stackdepth = HIF_WRITER_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadParams, "UWB_WriterTask");
    threadParams.pContext = NULL;
    threadParams.priority = HIF_WRITER_TASK_PRIO;
    /*This task is used for sending all notifications received from helios to Host via CDC USB interface.*/
    phOsalUwb_Thread_Create((void **)&mHifWriterTask, &UWB_WriterTask, &threadParams);

    phOsalUwb_TaskStartScheduler();
    return 0;
}

#endif //UWBIOT_APP_BUILD__DEMO_PNP
