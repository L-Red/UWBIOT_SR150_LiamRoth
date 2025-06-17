/*
 *
 * Copyright 2021-2023 NXP.
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

#ifndef UWBIOT_APP_BUILD__DEMO_DLTDOA_INITIATOR
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_DLTDOA_INITIATOR

#define DEMO_RANGING_DLTDOA_TASK_SIZE 400
#define DEMO_RANGING_DLTDOA_TASK_NAME "DemoRngDlTdoA"
#define DEMO_RANGING_DLTDOA_TASK_PRIO 4

#define DLTDOA_ANCHOR1_TIMEBASE_64BIT 0x02

/**
 * @brief Active Round Configuration
 * Configuration table to set active ranging round configuration.
 * A device can participate in multiple rounds with different roles,
 * whereas the device can be max. one initiator and multiple responder.
 * Note: if deviceRole is set to DEVICE_ROLE_INITIATOR, then gkControleeMacAddrs must be set as well.
 *
 * Example for only one active round (single cell) initiator:
 * static phActiveRoundsConfig_t activeRoundsConfig[] =
 * {
 *  {
 *      .roundIndex = 0,
 *      .deviceRole = DEVICE_ROLE_INITIATOR
 *  }
 * };
 *
 * static phActiveRoundsConfig_t activeRoundsConfig[] =
 * {
 *  {
 *      .roundIndex = 0,
 *      .deviceRole = DEVICE_ROLE_RESPONDER
 *  }
 * };
 */

uint8_t addList1[] = {0x22, 0x22};

const phActiveRoundsConfig_t activeRoundsConfig[] = {{.roundIndex                 = 0,
                                                         .rangingRole             = kUWB_DeviceRole_Initiator,
                                                         .noofResponders          = 1,
                                                         .responderMacAddressList = addList1,
                                                         .responderSlotScheduling = 0,
                                                         .responderSlots          = NULL},
    {.roundIndex = 1, .rangingRole = kUWB_DeviceRole_Responder},
    /** Subsequent fields are only present if Ranging Role is set to 0x01.
    Otherwise, these fields are not present.  */
    {.roundIndex                 = 2,
        .rangingRole             = kUWB_DeviceRole_Initiator,
        .noofResponders          = 1,
        .responderMacAddressList = addList1,
        .responderSlotScheduling = 0,
        .responderSlots          = NULL},
    {.roundIndex = 3, .rangingRole = kUWB_DeviceRole_Responder}};

/**
 * @brief Our own mac address. Need to be set unique on each device
  */
static const uint16_t gKnownMacAddr = 0x1111;

/**
 * @brief List of controlee MAC addresses
 * This is only used if device role is initiator specified in activeRoundsConfig
 */
static const uint16_t gkControleeMacAddrs[] = {0x2222};
static const uint8_t gkAnchorLoc[]          = {0x00};
#define DEMO_RANGING_APP_SESSION_ID 0x11111111
uint32_t gSessionHandle = 0;

#define CHECK_SET_APP_SUCCESS(status) \
    if (status != UWBAPI_STATUS_OK)   \
    return status

static tUWBAPI_STATUS ApplySessionConfig()
{
    tUWBAPI_STATUS status;
    uint8_t numControlees = (uint8_t)(sizeof(gkControleeMacAddrs) / sizeof(uint16_t));
    /* our device (also) acts as initiator. Configure device MAC address list (5 devices * short_addr_len) */
    uint8_t dstMacAddrList[MAX_DST_ADDR_LIST_SIZE * MAC_SHORT_ADD_LEN];
    for (int i = 0; i < numControlees; i++) {
        UWB_UINT16_TO_FIELD(dstMacAddrList, gkControleeMacAddrs[i]);
    }

    const UWB_AppParams_List_t SetAppParamsList[] = {UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 200),
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10),
        UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 1200),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, kUWB_RfFrameConfig_SP1),
        UWB_SET_APP_PARAM_VALUE(IN_BAND_TERMINATION_ATTEMPT_COUNT, 0),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
        UWB_SET_APP_PARAM_VALUE(DLTDOA_TIME_REF_ANCHOR, 1),
        UWB_SET_APP_PARAM_VALUE(DLTDOA_ANCHOR_CFO, 1),
        UWB_SET_APP_PARAM_ARRAY(DLTDOA_ANCHOR_LOCATION, gkAnchorLoc, 1),
        UWB_SET_APP_PARAM_VALUE(DLTDOA_TX_ACTIVE_RANGING_ROUNDS, 1),
        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, 1),
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, kUWB_StsConfig_StaticSts),
        UWB_SET_APP_PARAM_VALUE(MULTI_NODE_MODE, kUWB_MultiNodeMode_OnetoMany),
        UWB_SET_APP_PARAM_VALUE(DL_TDOA_HOP_COUNT, 1),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, dstMacAddrList, MAC_SHORT_ADD_LEN * numControlees),
        UWB_SET_APP_PARAM_VALUE(DLTDOA_TX_TIMESTAMP_CONF, DLTDOA_ANCHOR1_TIMEBASE_64BIT)};

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
    /** TODO : for numOfActiveRounds new logic needs to added*/
    uint8_t numOfActiveRounds = 4;

    UWB_UINT16_TO_STREAM(pDevMacAddr, gKnownMacAddr);

    inRangingParam.multiNodeMode     = kUWB_MultiNodeMode_OnetoMany;
    inRangingParam.macAddrMode       = kUWB_MacAddressMode_2bytes;
    inRangingParam.deviceRole        = kUWB_DeviceRole_DlTDoA_Anchor;
    inRangingParam.deviceType        = kUWB_DeviceType_Controller;
    inRangingParam.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParam.rangingRoundUsage = kUWB_RangingRoundUsage_DL_TDOA;

    status = UwbApi_SetRangingParams(gSessionHandle, &inRangingParam);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        return status;
    }

    phNotActivatedRounds_t notActivatedRounds;
    status = UwbApi_UpdateActiveRoundsAnchor(
        gSessionHandle, numOfActiveRounds, kUWB_MacAddressMode_2bytes, activeRoundsConfig, &notActivatedRounds);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_UpdateActiveRounds() Failed");
        return status;
    }

    return status;
}

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Demo DLTDOA Anchor1");
    tUWBAPI_STATUS status;
    phUwbDevInfo_t devInfo;
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

    phDeviceConfigData_t devConfig;
    devConfig.lowPowerMode = DISABLED;
    status                 = UwbApi_SetDeviceConfig(LOW_POWER_MODE, sizeof(devConfig.lowPowerMode), &devConfig);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetDeviceConfig() Failed");
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

#endif //UWBIOT_APP_BUILD__DEMO_DLTDOA_INITIATOR
