/* Copyright 2019,2022,2023 NXP
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
#include "PrintUtility.h"

#ifndef UWBIOT_APP_BUILD__DEMO_OWR_AOA_ADVERTISER
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_OWR_AOA_ADVERTISER

/*
  * Below list contains the application configs which are only related to default
  * configuration.
  */

/********************************************************************************/
/*               Ranging APP configuration setting here */
/********************************************************************************/

#define DEMO_MAC_ADDR_MODE                 EXTENDED_MAC_ADDRESS_MODE_WITH_HEADER
#define DEMO_RANGING_APP_SESSION_ID        0x22334455
#define DEMO_RANGING_APP_NO_OF_ANCHORS_P2P 1

#define DEMO_DATA_SIZE 2

#if (DEMO_MAC_ADDR_MODE == SHORT_MAC_ADDRESS_MODE)
#define DEMO_MAC_ADDR_LEN MAC_SHORT_ADD_LEN
static const uint8_t gkDeviceMacAddr[MAC_SHORT_ADD_LEN] = {0X00, 0x01};
#else
#define DEMO_MAC_ADDR_LEN MAC_EXT_ADD_LEN
static const uint8_t gkDeviceMacAddr[MAC_EXT_ADD_LEN] = {0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x1, 0x1};
#endif

#if UWBFTR_DataTransfer
static const uint8_t gkDestAddr[MAC_EXT_ADD_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#endif // UWBFTR_DataTransfer

uint8_t gDataToTransfer[DEMO_DATA_SIZE];

/********************************************************************************/
#define DEMO_OWR_AOA_ADVERTISER_TASK_SIZE 400
#define DEMO_OWR_AOA_ADVERTISER_TASK_NAME "DemoOwrAoaAdvertiser"
#define DEMO_OWR_AOA_ADVERTISER_TASK_PRIO 4

AppContext_t appContext;

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status;
    uint8_t offset                                = 0;
    uint32_t sessionHandle                        = 0;
    uint32_t delay;
    const UWB_AppParams_List_t SetAppParamsList[] = {
        /* mandatory as per config Digest Usage*/
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, kUWB_RfFrameConfig_Sfd_Sts),
        UWB_SET_APP_PARAM_VALUE(MIN_FRAMES_PER_RR, 10),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 200),
        UWB_SET_APP_PARAM_VALUE(MTU_SIZE, 87),
        UWB_SET_APP_PARAM_VALUE(INTER_FRAME_INTERVAL, 2), // Inter Frame Interval(IFI) is 2 Milli Seconds
#if UWBFTR_DataTransfer
        UWB_SET_APP_PARAM_VALUE(DATA_REPETITION_COUNT, 0),

#endif // UWBFTR_DataTransfer
        UWB_SET_APP_PARAM_VALUE(SESSION_INFO_NTF, kUWB_EnableSession_Info_Ntf),
        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_RANGING_APP_NO_OF_ANCHORS_P2P),

    };

#if UWBFTR_DataTransfer
    const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_VALUE(SESSION_INBAND_DATA_TX_BLOCKS, 12),
        UWB_SET_VENDOR_APP_PARAM_VALUE(SESSION_INBAND_DATA_RX_BLOCKS, 0),
    };
#endif // UWBFTR_DataTransfer

    phUwbDevInfo_t devInfo;

#if UWBFTR_DataTransfer
    phUwbDataPkt_t sendData = {0};
#endif // UWBFTR_DataTransfer

    PRINT_APP_NAME("Demo Owr Aoa Advertiser");

    status = UwbApi_Init(AppCallback);

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

    status = UwbApi_SessionInit(DEMO_RANGING_APP_SESSION_ID, UWBD_RANGING_WITH_INBAND_DATA_TRANSFER, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = demo_set_common_app_config(sessionHandle, kUWB_StsConfig_StaticSts);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("demo_set_common_app_config() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

#if UWBFTR_DataTransfer
    status = UwbApi_SetVendorAppConfigs(
        sessionHandle, sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]), &SetVendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SetVendorAppConfigs() Failed");
        goto exit;
    }
#endif //UWBFTR_DataTransfer
    phRangingParams_t inRangingParams;
    inRangingParams.deviceRole        = kUWB_DeviceRole_Advertiser;
    inRangingParams.multiNodeMode     = kUWB_MultiNodeMode_OnetoMany;
    inRangingParams.macAddrMode       = DEMO_MAC_ADDR_MODE;
    inRangingParams.deviceType        = kUWB_DeviceType_Controller;
    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_OWR_AOA;

    for (offset = 0; offset < DEMO_MAC_ADDR_LEN; offset++) {
        inRangingParams.deviceMacAddr[offset] = gkDeviceMacAddr[offset];
    }

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

#if !(UWBFTR_DataTransfer)
    NXPLOG_APP_W("Enable Data Transfer feature for this demo!!!!!!!");
#else
    GENERATE_SEND_DATA(gDataToTransfer, sizeof(gDataToTransfer))
    sendData.sessionHandle = sessionHandle;
    for (offset = 0; offset < DEMO_MAC_ADDR_LEN; offset++) {
        sendData.mac_address[offset] = gkDestAddr[offset];
    }
    sendData.data_size = sizeof(gDataToTransfer);
    sendData.data      = &gDataToTransfer[0];
    status             = UwbApi_SendData(&sendData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SendData() Failed");
        goto exit;
    }
#endif // UWBFTR_DataTransfer

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
    threadparams.stackdepth   = DEMO_OWR_AOA_ADVERTISER_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_OWR_AOA_ADVERTISER_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_OWR_AOA_ADVERTISER_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_OWR_AOA_ADVERTISER
