/* Copyright 2023 NXP
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
#include "AppRecovery.h"
#include "UwbApi.h"
#include <AppInternal.h>
#include "phUwbStatus.h"
#include "UwbApi_Types_Proprietary.h"
#include "uwb_types.h"
#include "phOsalUwb.h"

#ifndef UWBIOT_APP_BUILD__DEMO_HYBRID_RANGING_CONTROLLER
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_HYBRID_RANGING_CONTROLLER

/*
  * Below list contains the application configs which are only related to default
  * configuration.
  */

/********************************************************************************/
/*               Ranging APP configuration setting here */
/********************************************************************************/

#define DEMO_PRIMARY_SESSION_ID    0x11223344
#define DEMO_SECONDARY_SESSION_ID1 0x22334455
#define DEMO_SECONDARY_SESSION_ID2 0x33445566
#define DEMO_SECONDARY_SESSION_ID3 0x44556677

#define DEMO_RANGING_APP_NO_OF_ANCHORS_P2P 1

#define DEMO_RANGING_CONTROLLER_TASK_SIZE 1600
#define DEMO_RANGING_CONTROLLER_TASK_NAME "DemoHybrdCtrller"
#define DEMO_RANGING_CONTROLLER_TASK_PRIO 4

static const uint8_t gkDestinationMacAddress[2] = {0xBB, 0xBB};
static const uint8_t gkDeviceMacAddress[2]      = {0xAA, 0xAA};
uint8_t gDataTx[6]                              = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

static const uint8_t gkHusUpdateTime[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT 0x0

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status               = UWBAPI_STATUS_FAILED;
    const uint32_t primary_SessionID    = DEMO_PRIMARY_SESSION_ID;    // Hybrid UWB Session
    const uint32_t secondary_SessionID1 = DEMO_SECONDARY_SESSION_ID1; // Used for CFP
    const uint32_t secondary_SessionID2 = DEMO_SECONDARY_SESSION_ID2; // Used for CAP
    const uint32_t secondary_SessionID3 = DEMO_SECONDARY_SESSION_ID3; // Used for ranging with data transfer

    uint32_t primarySessionHandle     = 0;
    uint32_t secondarySessionHandle_1 = 0;
    uint32_t secondarySessionHandle_2 = 0;
    uint32_t secondarySessionHandle_3 = 0;

    uint32_t delay;

    phUwbappContext_t appCtx = {0};
    phUwbDevInfo_t devInfo;
    phRangingParams_t hybrid_RangingParams          = {0};
    phRangingParams_t phase1_RangingParams          = {0};
    phRangingParams_t phase2_RangingParams          = {0};
    phRangingParams_t phase3_RangingParams          = {0};
    phControllerHusSessionConfig_t husSessionConfig = {0};

    PRINT_APP_NAME("Demo Hybrid Ranging Controller");

    /*** App Configs **/
    /** Primary Session */
    const UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 100),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 100),
        UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 1200),
        UWB_SET_APP_PARAM_VALUE(NUMBER_OF_STS_SEGMENTS, 1),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, kUWB_RfFrameConfig_SP1),
        UWB_SET_APP_PARAM_VALUE(PRF_MODE, 0),
        UWB_SET_APP_PARAM_VALUE(IN_BAND_TERMINATION_ATTEMPT_COUNT, 0),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gkDestinationMacAddress, MAC_SHORT_ADD_LEN),
    };

    const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_VALUE(MAC_PAYLOAD_ENCRYPTION, 1),
    };

    /** Secondary Session 1 (Time Scheduled) */
    const UWB_AppParams_List_t Phase1_SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 25),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 100),
        UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 1200),
        UWB_SET_APP_PARAM_VALUE(RANGING_ROUND_USAGE, kUWB_RangingRoundUsage_DS_TWR),
        UWB_SET_APP_PARAM_VALUE(IN_BAND_TERMINATION_ATTEMPT_COUNT, 0),
        UWB_SET_APP_PARAM_VALUE(SFD_ID, 2),
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, 0),
        UWB_SET_APP_PARAM_VALUE(NUMBER_OF_STS_SEGMENTS, 1),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, kUWB_RfFrameConfig_SP1),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
        UWB_SET_APP_PARAM_VALUE(PRF_MODE, 0),
        UWB_SET_APP_PARAM_VALUE(BPRF_PHR_DATA_RATE, 0),
        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_RANGING_APP_NO_OF_ANCHORS_P2P),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gkDestinationMacAddress, MAC_SHORT_ADD_LEN),
    };

    /** Secondary Session 2 (Contention Based) */
    const UWB_AppParams_List_t Phase2_SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 25),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 100),
        UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 1200),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, 1),
        UWB_SET_APP_PARAM_VALUE(RANGING_ROUND_USAGE, kUWB_RangingMethod_SS_TWR_ND),
        UWB_SET_APP_PARAM_VALUE(RANGING_ROUND_CONTROL, 6),
        UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, 1),
        UWB_SET_APP_PARAM_VALUE(PRF_MODE, 0),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
        UWB_SET_APP_PARAM_VALUE(CAP_SIZE_RANGE, 0x050A),
    };

    /** Secondary Session 3 (Ranging and Data Transfer) */
    const UWB_AppParams_List_t Phase3_SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(SFD_ID, 2),
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 25),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 100),
        UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 1200),
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, kUWB_StsConfig_StaticSts),
        UWB_SET_APP_PARAM_VALUE(NUMBER_OF_STS_SEGMENTS, 1),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, 1),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, 9),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_DURATION, 1),
        UWB_SET_APP_PARAM_VALUE(LINK_LAYER_MODE, 0),
        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_RANGING_APP_NO_OF_ANCHORS_P2P),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gkDestinationMacAddress, MAC_SHORT_ADD_LEN),
    };

    const UWB_VendorAppParams_List_t Phase3_SetvendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_VALUE(SESSION_INBAND_DATA_TX_BLOCKS, 6),
        UWB_SET_VENDOR_APP_PARAM_VALUE(SESSION_INBAND_DATA_RX_BLOCKS, 0),
    };

#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = MAINLINE_FW;
    appCtx.pCallback         = AppCallback;
    appCtx.pCdcCallback      = NULL;
    appCtx.pMcttCallback     = NULL;
    appCtx.seHandle          = NULL;

    status = UwbApi_Init_New(&appCtx);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Init_New Failed");
        goto exit;
    }

    status = UwbApi_GetDeviceInfo(&devInfo);
    printDeviceInfo(&devInfo);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetDeviceInfo() Failed");
        goto exit;
    }

    /** Primary Session Initialization */
    status = UwbApi_SessionInit(primary_SessionID, UWBD_RANGING_WITH_INBAND_DATA_TRANSFER, &primarySessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(
        primarySessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    status = UwbApi_SetVendorAppConfigs(primarySessionHandle,
        sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]),
        &SetVendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetVendorAppConfigs() Failed");
        goto exit;
    }

    hybrid_RangingParams.macAddrMode       = DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    hybrid_RangingParams.deviceType        = kUWB_DeviceType_Controller;
    hybrid_RangingParams.deviceRole        = kUWB_DeviceRole_Initiator;
    hybrid_RangingParams.multiNodeMode     = kUWB_MultiNodeMode_OnetoMany;
    hybrid_RangingParams.deviceMacAddr[0]  = gkDeviceMacAddress[0];
    hybrid_RangingParams.deviceMacAddr[1]  = gkDeviceMacAddress[1];
    hybrid_RangingParams.scheduledMode     = kUWB_ScheduledMode_HybridBased;
    hybrid_RangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    /** Primary Session Set Ranging Parameters */
    status = UwbApi_SetRangingParams(primarySessionHandle, &hybrid_RangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    /* Secondary session 1 Initialization */
    status = UwbApi_SessionInit(secondary_SessionID1, UWBD_RANGING_ONLY_PHASE, &secondarySessionHandle_1);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(secondarySessionHandle_1,
        sizeof(Phase1_SetAppParamsList) / sizeof(Phase1_SetAppParamsList[0]),
        &Phase1_SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    phase1_RangingParams.deviceRole        = kUWB_DeviceRole_Initiator;
    phase1_RangingParams.multiNodeMode     = kUWB_MultiNodeMode_OnetoMany;
    phase1_RangingParams.macAddrMode       = DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    phase1_RangingParams.deviceType        = kUWB_DeviceType_Controller;
    phase1_RangingParams.deviceMacAddr[0]  = gkDeviceMacAddress[0];
    phase1_RangingParams.deviceMacAddr[1]  = gkDeviceMacAddress[1];
    phase1_RangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    phase1_RangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    status = UwbApi_SetRangingParams(secondarySessionHandle_1, &phase1_RangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    /* Secondary session 2 Initialization */
    status = UwbApi_SessionInit(secondary_SessionID2, UWBD_RANGING_ONLY_PHASE, &secondarySessionHandle_2);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(secondarySessionHandle_2,
        sizeof(Phase2_SetAppParamsList) / sizeof(Phase2_SetAppParamsList[0]),
        &Phase2_SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    phase2_RangingParams.deviceRole        = kUWB_DeviceRole_Initiator;
    phase2_RangingParams.multiNodeMode     = kUWB_MultiNodeMode_OnetoMany;
    phase2_RangingParams.macAddrMode       = DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    phase2_RangingParams.deviceType        = kUWB_DeviceType_Controller;
    phase2_RangingParams.deviceMacAddr[0]  = gkDeviceMacAddress[0];
    phase2_RangingParams.deviceMacAddr[1]  = gkDeviceMacAddress[1];
    phase2_RangingParams.scheduledMode     = kUWB_ScheduledMode_ContentionBased;
    phase2_RangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_SS_TWR_nd;

    status = UwbApi_SetRangingParams(secondarySessionHandle_2, &phase2_RangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    /* Secondary session 3 Initialization */
    status = UwbApi_SessionInit(secondary_SessionID3, UWBD_RANGING_WITH_DATA_PHASE, &secondarySessionHandle_3);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(secondarySessionHandle_3,
        sizeof(Phase3_SetAppParamsList) / sizeof(Phase3_SetAppParamsList[0]),
        &Phase3_SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    // Applying  Vendor specific parameter for the session
    status = UwbApi_SetVendorAppConfigs(secondarySessionHandle_3,
        sizeof(Phase3_SetvendorAppParamsList) / sizeof(Phase3_SetvendorAppParamsList[0]),
        &Phase3_SetvendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetVendorAppConfigs() Failed");
        goto exit;
    }

    phase3_RangingParams.deviceRole        = kUWB_DeviceRole_Initiator;
    phase3_RangingParams.multiNodeMode     = kUWB_MultiNodeMode_OnetoMany;
    phase3_RangingParams.macAddrMode       = DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    phase3_RangingParams.deviceType        = kUWB_DeviceType_Controller;
    phase3_RangingParams.deviceMacAddr[0]  = gkDeviceMacAddress[0];
    phase3_RangingParams.deviceMacAddr[1]  = gkDeviceMacAddress[1];
    phase3_RangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    phase3_RangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    status = UwbApi_SetRangingParams(secondarySessionHandle_3, &phase3_RangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    /* Set HUS Session */
    husSessionConfig.sessionHandle = primarySessionHandle;
    husSessionConfig.phase_count   = 3;
    phOsalUwb_MemCopy(&husSessionConfig.update_time[0], &gkHusUpdateTime[0], 8);
    /* time-scheduled session configuration */
    husSessionConfig.phase_list[0].phase_sessionHandle = secondarySessionHandle_1;
    husSessionConfig.phase_list[0].start_slot_index    = 1;
    husSessionConfig.phase_list[0].end_slot_index      = 20;
    husSessionConfig.phase_list[0].phase_participation = kUWB_CtrlDeviceRoleParticipation;
    phOsalUwb_MemCopy(husSessionConfig.phase_list[0].mac_addr, gkDeviceMacAddress, MAC_SHORT_ADD_LEN);
    /* Contention based session configuration */
    husSessionConfig.phase_list[1].phase_sessionHandle = secondarySessionHandle_2;
    husSessionConfig.phase_list[1].start_slot_index    = 21;
    husSessionConfig.phase_list[1].end_slot_index      = 40;
    husSessionConfig.phase_list[1].phase_participation = kUWB_CtrlDeviceRoleParticipation;
    phOsalUwb_MemCopy(husSessionConfig.phase_list[1].mac_addr, gkDeviceMacAddress, MAC_SHORT_ADD_LEN);
    /* Data Transfer session configuration */
    husSessionConfig.phase_list[2].phase_sessionHandle = secondarySessionHandle_3;
    husSessionConfig.phase_list[2].start_slot_index    = 41;
    husSessionConfig.phase_list[2].end_slot_index      = 60;
    husSessionConfig.phase_list[2].phase_participation = kUWB_CtrlDeviceRoleParticipation;
    phOsalUwb_MemCopy(husSessionConfig.phase_list[2].mac_addr, gkDeviceMacAddress, MAC_SHORT_ADD_LEN);

    status = UwbApi_SetControllerHusSession(&husSessionConfig);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetControllerHusSession() Failed");
        goto exit;
    }

    /** Secondary Session 1 Start Ranging */
    status = UwbApi_StartRangingSession(secondarySessionHandle_1);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("1st Secondary UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /** Secondary Session 2 Start Ranging */
    status = UwbApi_StartRangingSession(secondarySessionHandle_2);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("2nd Secondary UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /** Secondary Session 3 Start Ranging */
    status = UwbApi_StartRangingSession(secondarySessionHandle_3);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("3rd Secondary UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /** Primary Session Start Ranging */
    status = UwbApi_StartRangingSession(primarySessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Primary UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /* Transmit data */
    phUwbDataPkt_t packet;
    phOsalUwb_SetMemory(packet.mac_address, 0, 8);
    packet.mac_address[0]  = gkDestinationMacAddress[0];
    packet.mac_address[1]  = gkDestinationMacAddress[1];
    packet.sequence_number = 0;
    packet.sessionHandle   = secondarySessionHandle_3;
    packet.data_size       = sizeof(gDataTx);
    packet.data            = &gDataTx[0];
    status                 = UwbApi_SendData(&packet);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SendData() Failed");
        goto exit;
    }

    /* Delay 5 Mins for Ranging MILLISECONDS = MINUTES * 60 * 1000 */
    delay = 5 * 60 * 1000; /*Waiting for 5 mins*/

    phOsalUwb_Delay(delay);

    /* Secondary Session 1 Stop Ranging */
    status = UwbApi_StopRangingSession(secondarySessionHandle_1);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopRangingSession() Failed Secondary Session 1");
        goto exit;
    }

    /* Secondary Session 2 Stop Ranging */
    status = UwbApi_StopRangingSession(secondarySessionHandle_2);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopRangingSession() Failed Secondary Session 2");
        goto exit;
    }

    /* Secondary Session 3 Stop Ranging */
    status = UwbApi_StopRangingSession(secondarySessionHandle_3);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopRangingSession() Failed Secondary Session 3");
        goto exit;
    }

    /** Primary Session Stop Ranging */
    status = UwbApi_StopRangingSession(primarySessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopRangingSession() Failed");
        goto exit;
    }

    /* Secondary Session 1 DeInit */
    status = UwbApi_SessionDeinit(secondarySessionHandle_1);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionDeinit() Failed");
        goto exit;
    }

    /* Secondary Session 2 DeInit */
    status = UwbApi_SessionDeinit(secondarySessionHandle_2);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionDeinit() Failed");
        goto exit;
    }

    /* Secondary Session 3 DeInit */
    status = UwbApi_SessionDeinit(secondarySessionHandle_3);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionDeinit() Failed");
        goto exit;
    }

    /* Primary Session DeInit */
    status = UwbApi_SessionDeinit(primarySessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionDeinit() Failed");
        goto exit;
    }

exit:
    if (UwbApi_ShutDown() != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ShutDown Failed");
    }

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
    threadparams.stackdepth   = DEMO_RANGING_CONTROLLER_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_RANGING_CONTROLLER_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_RANGING_CONTROLLER_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // UWBIOT_APP_BUILD__DEMO_HYBRID_RANGING_CONTROLLER
