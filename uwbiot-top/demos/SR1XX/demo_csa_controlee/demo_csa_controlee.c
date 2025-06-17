/* Copyright 2022-2024 NXP
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
#include "UwbApi_Utility.h"

#ifndef UWBIOT_APP_BUILD__DEMO_CSA_CONTROLEE
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_CSA_CONTROLEE

/*
  * Below list contains the application configs which are only related to default
  * configuration.
  */

/********************************************************************************/
/*               Ranging APP configuration setting here */
/********************************************************************************/

#define RANGING_APP_SESSION_ID 0x11223344

#define RANGING_APP_NO_OF_ANCHORS_P2P 1

#define DEMO_CSA_CONTROLEE_TASK_SIZE 500
#define DEMO_CSA_CONTROLEE_TASK_NAME "DemoCsaCtrlee"
#define DEMO_CSA_CONTROLEE_TASK_PRIO 4

#if DEMO_CSA_CONTROLEE_TASK_SIZE < 500
#pragma message("DEMO_CSA_CONTROLEE_TASK_SIZE < 500 :  demo will not work")
#endif // DEMO_CSA_CONTROLEE_TASK_SIZE

uint8_t DEVICE_MAC_ADDR_SHORT[2] = {0x22, 0x22};
uint8_t DST_MAC_ADDR_SHORT[2]    = {0x11, 0x11};

uint8_t EXT_DEVICE_MAC_ADDR[8] = {0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x1, 0x1};
uint8_t EXT_DST_MAC_ADDR[8]    = {0xd, 0x7, 0x6, 0x5, 0x4, 0x2, 0x2, 0xd};

/** 3D AoA Configuration
 *  AoA_Mode | NoOfAntPAirs | Horizontal Ant Pair ID | Vertical Ant Pair Id
 */
#define TOA_ID_X(H, V)    kUWBAntCfgRxMode_AoA_Mode, 0x02, (H), (V)
#define ANT_ID_AOA_3D_AoA TOA_ID_X(1, 2)

#define RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT 0x0

/* Session ID(4 octets) || Random(12 octets) || Plain text Session Key(32 octets) */
#define RANDOM_KEY_LEN      12
#define CCC_SESSION_KEY_LEN 32
#define CCC_WRAPPED_RDS_LEN (SESSION_ID_LEN + RANDOM_KEY_LEN + CCC_SESSION_KEY_LEN)

/********************************************************************************/

AppContext_t appContext;

/* clang-format off */

/* doc:start:form wrapped rds */

/*
 * Example code to form the Wrapped Rds.
 */


tUWBAPI_STATUS set_wrapred_rds(uint32_t sessionHandle){

    tUWBAPI_STATUS status;
    //  To Set Wrapped RDS which includes
    // Session ID(4 octets) || Random(12 octets) || Plain text Session Key(32 octets)
    uint8_t wrappedRds[CCC_WRAPPED_RDS_LEN] = {0};
    // 12 bytes of Random Key with its 1st 4 bytes set as 0xB5 to distinguish from normal WRAPPED_RDS command.
    uint8_t RandomKey[RANDOM_KEY_LEN] = {0xB5, 0xB5, 0xB5, 0xB5, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    // 32 bytes of Plain Text Session Key
    uint8_t SessionKey[CCC_SESSION_KEY_LEN] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,\
                                               0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 \
                                              };

    // Form The Wrapped RDS
    serializeSessionHandlePayload(sessionHandle, &wrappedRds[0]);
    phOsalUwb_MemCopy(&wrappedRds[SESSION_ID_LEN], RandomKey, RANDOM_KEY_LEN);
    phOsalUwb_MemCopy(&wrappedRds[SESSION_ID_LEN + RANDOM_KEY_LEN], SessionKey, CCC_SESSION_KEY_LEN);

    status = UwbApi_SetAppConfigWrappedRDS(sessionHandle, wrappedRds, CCC_WRAPPED_RDS_LEN);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetAppConfigWrappedRDS Failed");
        goto exit;
    }
exit :
    return status;

}

/* doc:end:form wrapped rds */
/* clang-format on */

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status     = UWBAPI_STATUS_FAILED;
    const uint32_t session_id = RANGING_APP_SESSION_ID;

    phUwbappContext_t appCtx = {0};
    phUwbDevInfo_t devInfo;
    phRangingParams_t inRangingParams = {0};
    uint32_t sessionHandle            = 0;
    uint8_t antennaeConfigurationRx[] = {ANT_ID_AOA_3D_AoA};
    uint32_t delay;

    PRINT_APP_NAME("DEMO_CSA_contrlee");

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

    status = UwbApi_SessionInit(session_id, UWBD_CSA_SESSION, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    const UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, kUWB_StsConfig_DynamicSts),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, CH_9),
        UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 2400),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 96),
        UWB_SET_APP_PARAM_VALUE(STS_INDEX, 0),
        UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, 0x01),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, 9),
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 24),
        UWB_SET_APP_PARAM_VALUE(RESPONDER_SLOT_INDEX, 0x00),
        UWB_SET_APP_PARAM_VALUE(MAX_NUMBER_OF_MEASUREMENTS, 0xFFFF),

        UWB_SET_APP_PARAM_VALUE(HOPPING_MODE, 0),
        UWB_SET_APP_PARAM_VALUE(URSK_TTL, 720),
        UWB_SET_APP_PARAM_VALUE(KEY_ROTATION, 0x03),
        UWB_SET_APP_PARAM_VALUE(CCC_CONFIG_QUIRKS, 0x01),
        UWB_SET_APP_PARAM_VALUE(RANGING_PROTOCOL_VER, 0x100),
        UWB_SET_APP_PARAM_VALUE(UWB_CONFIG_ID, 0xFFFF),
        UWB_SET_APP_PARAM_VALUE(PULSESHAPE_COMBO, 0x0),

        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, RANGING_APP_NO_OF_ANCHORS_P2P),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, DST_MAC_ADDR_SHORT, MAC_SHORT_ADD_LEN),

    };

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_VALUE(SWAP_ANTENNA_PAIR_3D_AOA, 0x01),
        UWB_SET_VENDOR_APP_PARAM_VALUE(TEST_KDF_FEATURE, 0x00),
        UWB_SET_VENDOR_APP_PARAM_ARRAY(
            ANTENNAE_CONFIGURATION_RX, &antennaeConfigurationRx[0], sizeof(antennaeConfigurationRx)),
    };

    status = UwbApi_SetVendorAppConfigs(
        sessionHandle, sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]), &SetVendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetVendorAppConfigs() Failed");
        goto exit;
    }

    /* doc:start:call wrapped rds Api */

    status = set_wrapred_rds(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("set_wrapred_rds Failed");
        goto exit;
    }

    /* doc:end:call wrapped rds Api */

    inRangingParams.deviceRole        = kUWB_DeviceRole_Responder;
    inRangingParams.multiNodeMode     = kUWB_MultiNodeMode_UniCast;
    inRangingParams.macAddrMode       = RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    inRangingParams.deviceType        = kUWB_DeviceType_Controlee;
    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    inRangingParams.deviceMacAddr[0] = DEVICE_MAC_ADDR_SHORT[0];
    inRangingParams.deviceMacAddr[1] = DEVICE_MAC_ADDR_SHORT[1];

    /*  Note : Since for CCC all the required mandatory app configs are not available in UwbApi_SetRangingParams() which waits for SESSION_STATE_IDLE ntf so it will get timed out.
        To avoid this all the required mandatory App configs are configured before the UwbApi_SetRangingParams().*/
    status = UwbApi_SetRangingParams(sessionHandle, &inRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    status = UwbApi_StartRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /* Delay 5 Mins for Ranging MILLISECONDS = MINUTES * 60 * 1000 */
    delay = 5 * 60 * 1000; /*Waiting for 5 mins*/

    phOsalUwb_Delay(delay);

    status = UwbApi_StopRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopRangingSession() Failed");
        goto exit;
    }

    status = UwbApi_SessionDeinit(sessionHandle);
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
    threadparams.stackdepth   = DEMO_CSA_CONTROLEE_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_CSA_CONTROLEE_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_CSA_CONTROLEE_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // UWBIOT_APP_BUILD__DEMO_CSA_CONTROLEE
