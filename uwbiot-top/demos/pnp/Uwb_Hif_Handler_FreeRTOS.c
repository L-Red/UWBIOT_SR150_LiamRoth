/*
 * Copyright 2019,2020,2022,2023 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "phUwb_BuildConfig.h"

#if UWBIOT_OS_FREERTOS

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_PNP)

#include "app_config.h"
#include "uwb_board.h"
#include "UWB_Spi_Driver_Interface.h"
#include "UwbPnpInternal.h"
#include "UwbApi_Utility.h"

#include "Uwb_Vcom_Pnp.h"
#include "UWB_Evt_Pnp.h"

#include "phUwb_BuildConfig.h"
#include "phOsalUwb.h"

#include <FreeRTOS.h>
#include <timers.h>

#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
#include "UWB_Hbci.h"
#endif //  (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)

#if !defined ENABLED
#error ENABLED must be defined
#endif

extern bool mError;
volatile uint16_t mRcvTlvSize    = 0;
volatile uint16_t mRcvTlvSizeExp = 0;
static uint8_t mRcvTlvBuf[HIF_MAX_PKT_SIZE];
intptr_t mHifCommandQueue;
#define MAX_HIF_TASK_TLV_WAIT_TIMEOUT (1000)

//
// Timer for receiving packets from HIF.
//
// In case there was any corruption at the host level and
// the Host "Missed" any packet, PnP FW was getting stuck in an endless loop.
// Never ever recovering.
//
// Hence using a FreeRTOS Timer here to get status of operation
//
// Using FeeRTOS here directly because this code is for FreeRTOS based operation,
// and Osal Timer APIs are bulkier to do the same thing from an ISR Context here.
//
static TimerHandle_t gTimerHIF;

bool Uwb_Is_Hif_Active()
{
    return mError;
}

void Uwb_Reset_Hif_State(bool state)
{
    mError = state;
}

uint32_t UWB_Hif_UciSendNtfn(uint8_t *pData, uint16_t size)
{
    return UWB_Vcom_UciSendNtfn(pData, size);
}

uint32_t UWB_Hif_SendUCIRsp(uint8_t *pData, uint16_t size)
{
    return UWB_Vcom_SendUCIRsp(pData, size);
}

uint32_t UWB_Hif_SendRsp(uint8_t *pData, uint16_t size)
{
    return UWB_Vcom_SendRsp(pData, size);
}

void uwb_pnp_reload_timer()
{
    // we are receiving HIF Call back for the first time for this packet.
    // a.k.a. we are getting first part of the command from the host.
    // so restart the timer.
    if (phPlatform_Is_Irq_Context()) {
        xTimerStartFromISR(gTimerHIF, 0);
    }
    else {
        xTimerStart(gTimerHIF, 0);
    }
}

void uwb_pnp_stop_timer()
{
    if (phPlatform_Is_Irq_Context()) {
        xTimerStopFromISR(gTimerHIF, 0);
    }
    else {
        xTimerStop(gTimerHIF, 0);
    }
}

//
// One shot call back in case the RX timer expires.
//
static void fnTimerHIF_vTimerCallback(TimerHandle_t xTimer)
{
    // we were waiting for HIF Call Back... but we never got it.
    //
    // hence, reset our counters and start receiving frame from start.
    mRcvTlvSize = mRcvTlvSizeExp = 0;
    uwb_pnp_board_protocol_error_handler();
}

void Uwb_Hif_ReadDataCb(uint8_t *pData, uint32_t *pLen)
{
    // xTimerStart() starts a timer that was previously created using the xTimerCreate() API function.
    // If the timer had already been started and was already in the active state, then xTimerStart()
    // has equivalent functionality to the xTimerReset() API function.

    if (0 == mRcvTlvSizeExp) {
        if (*pLen > 0 && *pData == 0) {
            /* Error case, just forget about this frame.
             *
             * This is the First frame that we are receiving,
             * but why is the first byte  == Message Type 0 ?
             *
             */
            *pLen       = 0;
            mRcvTlvSize = mRcvTlvSizeExp = 0;
            mError                       = FALSE;
        }
        else {
            /* code */
            uwb_pnp_reload_timer();
        }
    }

    if (*pLen > sizeof(mRcvTlvBuf)) {
        uwb_pnp_stop_timer();
        fnTimerHIF_vTimerCallback(gTimerHIF);
        goto exit;
    }

    while (*pLen > 0) {
        int32_t cpSize;
        if (mRcvTlvSize < 3) {
            cpSize = 1;
        }
        else if (mRcvTlvSize + *pLen <= mRcvTlvSizeExp) {
            cpSize = *pLen;
        }
        else {
            cpSize = (mRcvTlvSizeExp - mRcvTlvSize);
        }
        if (cpSize <= 0) {
            /* Error case */
            break;
        }
        if ((mRcvTlvSize + (uint32_t)cpSize) > sizeof(mRcvTlvBuf)) {
            /* Error case */
            break;
        }

        phOsalUwb_MemCopy(&mRcvTlvBuf[mRcvTlvSize], pData, cpSize);
        *pLen -= cpSize;
        pData += cpSize;
        mRcvTlvSize += (uint16_t)cpSize;

        if (mRcvTlvSize == 3) {
            /* TLV size received */
            mRcvTlvSizeExp = (uint16_t)(3 + (mRcvTlvBuf[1] << 8 | mRcvTlvBuf[2]));
        }
        if (mRcvTlvSizeExp > sizeof(mRcvTlvBuf)) {
            uwb_pnp_stop_timer();
            fnTimerHIF_vTimerCallback(gTimerHIF);
        }
    }
exit:
    if (mRcvTlvSize && mRcvTlvSize == mRcvTlvSizeExp) {
        mRcvTlvSize = mRcvTlvSizeExp = 0;
        mError                       = FALSE;
        (void)phOsalUwb_ProduceSemaphore(mHifIsr_Sem);
        uwb_pnp_stop_timer();
    }
}

void UWB_Hif_Init()
{
    gTimerHIF = xTimerCreate(/* Just a text name, not used by the RTOS
                     kernel. */
        "HIF",
        pdMS_TO_TICKS(800),
        /* FALSE ==> One Shot */
        pdFALSE,
        /* The ID is used to store a count of the number of times the timer has expired, which is initialised to 0. */
        (void *)0,
        /* Each timer calls the same callback when
                     it expires. */
        fnTimerHIF_vTimerCallback);
    Uwb_Vcom_Init(&Uwb_Hif_ReadDataCb);
}

OSAL_TASK_RETURN_TYPE UWB_Hif_Handler_Task(void *args)
{
    phLibUwb_Message_t evt;
    tlv_t tlv;

    evt.eMsgType = USB_TLV_EVT;
    evt.pMsgData = &tlv;
    UWB_Hif_Init();

    while (1) {
        /* Wait to receive TLV over USB */
        if (!mError &&
            phOsalUwb_ConsumeSemaphore_WithTimeout(mHifIsr_Sem, MAX_HIF_TASK_TLV_WAIT_TIMEOUT) != UWBSTATUS_SUCCESS) {
            continue;
        }
        if (!mError) {
            tlv.type  = mRcvTlvBuf[0];
            tlv.size  = (uint16_t)(mRcvTlvBuf[1] << 8 | mRcvTlvBuf[2]);
            tlv.value = &mRcvTlvBuf[3];
            if (tlv.size > 3) {
                DEBUGOUT("USB 0x%X 0x%X 0x%X 0x%X\n", mRcvTlvBuf[0], mRcvTlvBuf[1], mRcvTlvBuf[2], mRcvTlvBuf[3]);
            }
            else {
                DEBUGOUT("USB 0x%X 0x%X 0x%X\n", mRcvTlvBuf[0], mRcvTlvBuf[1], mRcvTlvBuf[2]);
            }
            DEBUGOUT("[TLV %02x]\n", tlv.type);
            (void)phOsalUwb_msgsnd(mHifCommandQueue, &evt, MAX_DELAY);
        }
    }
}

OSAL_TASK_RETURN_TYPE UWB_WriterTask(void *args)
{
    phLibUwb_Message_t tlv = {0};
    while (1) {
        (void)phOsalUwb_LockMutex(mHifSyncMutex);
        if (phOsalUwb_msgrcv(mHifWriteQueue, &tlv, 5) == UWBSTATUS_FAILED) {
            (void)phOsalUwb_UnlockMutex(mHifSyncMutex);
            phOsalUwb_Thread_Context_Switch();
            continue;
        }
        (void)phOsalUwb_LockMutex(mHifWriteMutex);
        //uint8_t *Buffer = (uint8_t *)tlv.pMsgData;
        //DEBUGOUT("Sending ntf: 0x%X 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3], Buffer[4]);
        UWB_Hif_UciSendNtfn(tlv.pMsgData, tlv.Size);
        DEBUGOUT("After Sending ntf\n");
        if ((uint8_t *)tlv.pMsgData != NULL) {
            phOsalUwb_FreeMemory((uint8_t *)tlv.pMsgData);
            tlv.pMsgData = NULL;
            /* DEBUGOUT("FREE\r\n"); */
        }
        (void)phOsalUwb_UnlockMutex(mHifWriteMutex);
        (void)phOsalUwb_UnlockMutex(mHifSyncMutex);
        //phOsalUwb_Delay(2); /* Give some time for Reader Task to Read the Data.*/
    }
}

void UWB_HandleEvt(UWB_EvtType_t ev, void *args)
{
    switch (ev) {
    case USB_TLV_EVT:
#if UWBIOT_UWBD_SR1XXT
        UWB_Handle_SR1XXT_TLV((tlv_t *)args);
#elif UWBIOT_UWBD_SR2XXT
        UWB_Handle_SR2XXT_TLV((tlv_t *)args);
#elif UWBIOT_UWBD_SR040
        UWB_Handle_SR040_TLV((tlv_t *)args);
#endif
        break;
    default:
        PRINTF_WITH_TIME("ERROR: Unknown event type %02x\n", ev);
        break;
    }
}

OSAL_TASK_RETURN_TYPE UWB_Pnp_App_Task(void *args)
{
    /* This may go somehwere else.. */
    phLibUwb_Message_t evt = {0};

    PRINTF_WITH_TIME("UWB_HeliosTask(): suspending communication interfaces\n");
    /*Suspend USB task which gets USB messages from the Host. During HBCI internal download Rhodes can not accept
     * any commands from Host*/
    phOsalUwb_TaskSuspend(mHifTask);
    /*Suspend ALL tasks which are used for UCI operations. During HBCI mode UCI tasks need to be disabled.*/
    phOsalUwb_TaskSuspend(mUciReaderTask);
    phOsalUwb_TaskSuspend(mHifWriterTask);

#if UWBIOT_UWBD_SR040
    UpdateFirmware();
#elif UWBIOT_UWBD_SR1XXT
#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
    UWB_Tml_Io_Set(kUWBS_IO_O_ENABLE_HELIOS, 1);
    UWB_Tml_Io_Set(kUWBS_IO_O_HELIOS_RTC_SYNC, 1);
    phOsalUwb_Delay(100);
    if (UWB_HbciEncryptedFwDownload()) {
        PRINTF_WITH_TIME("UWB_HeliosTask(): HELIOS FW download completed\n");
        phOsalUwb_Delay(100);
        phOsalUwb_TaskResume(mUciReaderTask);
    }
    else {
        PRINTF_WITH_TIME("UWB_HeliosTask(): CRITICAL ERROR downloading Helios image\n");
    }
#endif // INTERNAL_FIRMWARE_DOWNLOAD
#endif // UWBIOT_UWBD_SR040

    PRINTF_WITH_TIME("UWB_HeliosTask(): resuming communication interfaces\n");
    /* After HBCI DND resume USB task. Now Host can send command over USB CDC interface.*/
    phOsalUwb_TaskResume(mHifTask);
    /*Resume ALL tasks which are used for UCI operations. After HBCI mode UCI tasks need to be enabled.*/
#if UWBIOT_UWBD_SR040
    phOsalUwb_TaskResume(mUciReaderTask);
#endif
    phOsalUwb_TaskResume(mHifWriterTask);
    while (1) {
        if (phOsalUwb_msgrcv(mHifCommandQueue, &evt, MAX_DELAY) == UWBSTATUS_FAILED) {
            continue;
        }
        if (evt.pMsgData != NULL) {
            UWB_HandleEvt((UWB_EvtType_t)evt.eMsgType, evt.pMsgData);
        }
        else {
            PRINTF_WITH_TIME("UWB_Pnp_App_Task(): Queue is empty\n");
        }
    }
}

#endif /* defined(UWBIOT_APP_BUILD__DEMO_PNP) */

#endif // UWBIOT_OS_FREERTOS