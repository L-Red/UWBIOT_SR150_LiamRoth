/*
 *
 * Copyright 2021,2023 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

#include "phUwb_BuildConfig.h"
#include "uwb_types.h"
#include "AppInternal.h"
#include "UwbApi.h"
#include "AppRecovery.h"

#ifndef UWBIOT_APP_BUILD__DEMO_DLTDOA_TAG
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_DLTDOA_TAG

#define DEMO_RANGING_DLTDOA_TASK_SIZE 400
#define DEMO_RANGING_DLTDOA_TASK_NAME "DemoRngDlTdoA"
#define DEMO_RANGING_DLTDOA_TASK_PRIO 4

/**
 * @brief Our own mac address. Need to be set unique on each device
  */
static const uint16_t gkOwnMacAddr = 0xD161;

static const uint8_t gkrangingroundIndexList[] = {0, 1, 2, 3};
#define DEMO_RANGING_APP_SESSION_ID 0x11111111
uint32_t gSessionHandle = 0;

const UWB_AppParams_List_t SetAppParamsList[] = {
    UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 200),
    UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10),
    UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 1200),
    UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, kUWB_RfFrameConfig_SP1),
    UWB_SET_APP_PARAM_VALUE(IN_BAND_TERMINATION_ATTEMPT_COUNT, 0),
    UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
    UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, 1),
};

#define CHECK_SET_APP_SUCCESS(status) \
    if (status != UWBAPI_STATUS_OK)   \
    return status

static tUWBAPI_STATUS ApplySessionConfig()
{
    tUWBAPI_STATUS status;

    /* Set the required configs for DL-TDOA ranging */
    status = UwbApi_SetAppConfigMultipleParams(
        gSessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        return status;
    }
    return status;
}

static tUWBAPI_STATUS ApplyRangingParameter()
{
    phRangingParams_t inRangingParam;
    uint8_t *pDevMacAddr = inRangingParam.deviceMacAddr;
    tUWBAPI_STATUS status;
    UWB_UINT16_TO_STREAM(pDevMacAddr, gkOwnMacAddr);

    inRangingParam.multiNodeMode     = kUWB_MultiNodeMode_OnetoMany;
    inRangingParam.macAddrMode       = kUWB_MacAddressMode_2bytes;
    inRangingParam.deviceRole        = kUWB_DeviceRole_DlTDoA_Tag;
    inRangingParam.deviceType        = kUWB_DeviceType_Controlee;
    inRangingParam.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParam.rangingRoundUsage = kUWB_RangingRoundUsage_DL_TDOA;

    printRangingParams(&inRangingParam);

    status = UwbApi_SetRangingParams(gSessionHandle, &inRangingParam);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        return status;
    }

    phNotActivatedRounds_t notActivatedRounds;

    status = UwbApi_UpdateActiveRoundsReceiver(gSessionHandle,
        sizeof(gkrangingroundIndexList) / sizeof(uint8_t),
        gkrangingroundIndexList,
        &notActivatedRounds);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_UpdateActiveRoundsReceiver() Failed");
        return status;
    }
    return status;
}

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Demo DLTDOA Tag");
    tUWBAPI_STATUS status;
    phUwbDevInfo_t devInfo = {0};
    uint32_t delay;

    phUwbappContext_t appCtx = {0};
#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = MAINLINE_FW;
    appCtx.pCallback         = AppCallback;
    appCtx.pCdcCallback      = NULL;
    appCtx.pMcttCallback     = NULL;
    status                   = UwbApi_Init_New(&appCtx);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Init Failed");
        goto exit;
    }

    status = UwbApi_GetDeviceInfo(&devInfo);
    printDeviceInfo(&devInfo);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetDeviceInfo() Failed");
        goto exit;
    }

    status = UwbApi_SessionInit(DEMO_RANGING_APP_SESSION_ID, UWBD_RANGING_SESSION, &gSessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = ApplySessionConfig();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("ApplySessionConfig() Failed");
        goto exit;
    }

    status = ApplyRangingParameter();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("ApplyRangingParameter() Failed");
        goto exit;
    }

    status = UwbApi_StartRangingSession(gSessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /* Delay 5 Mins for Ranging MILLISECONDS = MINUTES * 60 * 1000 */
    delay = 5 * 60 * 1000; /*Waiting for 5 mins*/

    phOsalUwb_Delay(delay);

    status = UwbApi_StopRangingSession(gSessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopRangingSession() Failed");
        goto exit;
    }

    status = UwbApi_SessionDeinit(gSessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionDeinit() Failed");
        goto exit;
    }

exit:
    if (status == UWBAPI_STATUS_TIMEOUT) {
        Handle_ErrorScenario(TIME_OUT);
    }

    UWBIOT_EXAMPLE_END(status);
}

/*
 * Interface which will be called from Main to create the required task with its own parameters.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;
    threadparams.stackdepth   = DEMO_RANGING_DLTDOA_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_RANGING_DLTDOA_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_RANGING_DLTDOA_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif //UWBIOT_APP_BUILD__DEMO_DLTDOA_TAG
