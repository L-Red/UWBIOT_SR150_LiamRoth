/* Copyright 2021-2023 NXP
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
#if !defined(UWBIOT_APP_BUILD__DEMO_MCTT_PCTT)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_MCTT_PCTT)

#include "AppInternal.h"
#include "Utilities.h"
#include "mctt_pctt_app.h"
#include "UwbHif.h"

#include "mctt_pctt_handler.h"
#include "phOsalUwb.h"
UWBOSAL_TASK_HANDLE uwb_demo_start(void);

static void UWB_HandleTLV(phLibUwb_Message_t *tlv);

void *mCmdMutex = NULL;
intptr_t mHifCommandQueue;
static UWBOSAL_TASK_HANDLE hifTaskHandle;
#define MCTT_APP_TASK_SIZE 1024
#define MCTT_APP_TASK_PRIO 4
#define MCTT_APP_TASK_NAME "MCTT_APP"

static volatile bool gBlockRangingNotification = TRUE;
#define GOTOASSERT while (1)

#if DEBUG
#pragma message( \
    "MCTT PCTT in debug mode can't handle queued responses and notifications, please select release mode for building mctt pctt.")
#endif

static void UWB_HandleTLV(phLibUwb_Message_t *tlv)
{
    static uint8_t respBuffer[HIF_RESP_BUFF_SIZE];

    uint16_t respSize = 0;
    uint8_t respTagType;
    uint8_t *respPayload = &respBuffer[4];

    switch (tlv->eMsgType) {
    case UWB_MCTT_UCI_READY: {
        mctt_handler((uint8_t *)tlv->pMsgData, tlv->Size, &respSize, &respBuffer[0]);
    } break;
    default:
        respTagType = tlv->eMsgType;
        (void)respTagType;
        respPayload[0] = UWBAPI_STATUS_UNSUPPORTED;
        respSize++;
        break;
    }
    if (respSize != 0) {
        // Send Response Over Host interface
        UWB_Hif_SendRsp(respBuffer, respSize);
    }
}

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    phLibUwb_Message_t evt;
    phOsalUwb_ThreadCreationParams_t threadParams;
    threadParams.stackdepth = HIF_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadParams, HIF_TASK_NAME);
    threadParams.pContext = NULL;
    threadParams.priority = HIF_TASK_PRIORITY;

#if defined(CPU_K32W032S1M2VPJ) || defined(CPU_MIMXRT1176DVMAA)
    BOARD_DeInitUsbDebugConsole();
    BOARD_InitDebugConsole();
#endif

    /* Do the Initilization and Apply the calibration */
    if (mctt_pctt_common_init() != UWBAPI_STATUS_OK) {
        PRINTF("mctt_pctt_common_init() Failed\n");
        goto exit;
    }
    PRINTF("mctt_pctt_common_init Success\n");
    /*common setting for  Mctt and Pctt */
    if (mctt_pctt_common_settings() != UWBAPI_STATUS_OK) {
        PRINTF("mctt_pctt_common_settings() Failed\n");
        goto exit;
    }
    PRINTF("mctt_pctt_common_settings Success\n");
    PRINTF(" Ready for MCTT/ITT/PCTT TESTING\n");
#if defined(QN9090DK6) || defined(NORDIC_MCU)
    /* DeInitialize UART Debug instance */
    /**
     * @brief  Disabling the Debug console here from here same uart port uses for mctt/pctt communications.
     *
     */
    BOARD_DeinitDebugConsole();
#endif
    /*SET MCTT MODE*/
    HifSetMode(kUWB_MODE_MCTT);
    HifInit(kUWB_COMM_Serial);
    if (phOsalUwb_CreateMutex(&mCmdMutex) != UWBSTATUS_SUCCESS) {
        PRINTF("Error: StandaloneTask(), could not create mutex mUsbSyncMutex\n");
        while (TRUE)
            ;
    }

    if (phOsalUwb_Thread_Create((void **)&hifTaskHandle, &UWB_HIFTask, &threadParams) != UWBSTATUS_SUCCESS) {
        PRINTF("Task UWB_HIFTask creation Failed");
        while (TRUE)
            ;
    }

    while (TRUE) {
        if (phOsalUwb_msgrcv(mHifCommandQueue, &evt, MAX_DELAY) == UWBSTATUS_FAILED) {
            continue;
        }
        //        PRINTF("data rececived\n");
        UWB_HandleTLV(&evt);
    }
exit:
    if (mctt_pctt_common_deinit() != UWBAPI_STATUS_OK) {
        PRINTF("mctt_pctt_common_deinit() Failed");
    }
}

/*
 * @brief   Application entry point.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;

#if defined(QN9090DK6) || defined(NORDIC_MCU)
    /* Enable the SWO Debug consle for only in debug session */
    if (IS_DEBUG_SESSION) {
        BOARD_InitSwoDebugConsole();
    }
#endif
    threadparams.stackdepth = MCTT_APP_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, MCTT_APP_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = MCTT_APP_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif //UWBIOT_APP_BUILD__DEMO_MCTT_PCTT
