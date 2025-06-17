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
#include "phUwbStatus.h"
#include "AppInternal.h"
#include "UwbApi.h"
#include "AppRecovery.h"
#include "UwbApi_Types_Proprietary.h"
#include "uwb_types.h"
#include "phOsalUwb.h"

#define DEMO_INBAND_DATA_TRANSFER_APP_DATA_TRANSFER_SESSION_ID 0x11223344
#define DEMO_INBAND_DATA_TRANSFER_APP_DEV_TYPE_CONTROLLER      1
#define DEMO_INBAND_DATA_TRANSFER_APP_DEV_ROLE_INITIATOR       1
#define DEMO_INBAND_DATA_TRANSFER_APP_MULTI_NODE_MODE_SINGLE   0
#define DEMO_INBAND_DATA_TRANSFER_APP_NO_OF_CONTROLEES_P2P     1
#define DEMO_INBAND_DATA_TRANSFER_APP_DEVICE_MAC_ADD_MODE      0x0
#define DEMO_INBAND_DATA_TRANSFER_APP_RFRAME_CONFIG            1
#define DEMO_INBAND_DATA_TRANSFER_APP_MAC_PAYLOAD_ENCRYPTION   0
#define DEMO_INBAND_DATA_TRANSFER_APP_INBAND_DATA_TX_BLOCK     12
#define DEMO_INBAND_DATA_TRANSFER_APP_INBAND_DATA_RX_BLOCK     0
#define DEMO_INBAND_DATA_TRANSFER_TX_TASK_SIZE                 400
#define DEMO_INBAND_DATA_TRANSFER_TX_TASK_NAME                 "DemoDataTransferTx"
#define DEMO_INBAND_DATA_TRANSFER_TX_TASK_PRIO                 4

#ifndef UWBIOT_APP_BUILD__DEMO_INBAND_DATA_TRANSFER_TX
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_INBAND_DATA_TRANSFER_TX

static const uint8_t gkDeviceMacAddr[8] = {0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t gkDstMacAddr[8]    = {0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t *gData                          = dataToSend;

phUwbDataPkt_t receivedData;

#define TOF_ID_X(X) kUWBAntCfgRxMode_ToA_Mode, 0x01, (X)

/* Helper macro to select TOF configuration mode */
#define ANT_ID_TOF TOF_ID_X(1)

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status;
    phRangingParams_t inRangingParam;
    phUwbDataPkt_t sendData;
    phUwbQueryDataSize_t queryDataSz = {0};
    phUwbDevInfo_t devInfo;
    uint32_t sessionHandle = 0;

    uint8_t antennaeConfigurationRx[] = {ANT_ID_TOF};

    const UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_INBAND_DATA_TRANSFER_APP_NO_OF_CONTROLEES_P2P),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gkDstMacAddr, MAC_SHORT_ADD_LEN),
    };

    PRINT_APP_NAME("Demo Data Transfer Tx");

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

    status = UwbApi_SessionInit(
        DEMO_INBAND_DATA_TRANSFER_APP_DATA_TRANSFER_SESSION_ID, UWBD_RANGING_WITH_INBAND_DATA_TRANSFER, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = demo_set_common_app_config(sessionHandle, kUWB_StsConfig_StaticSts);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("demo_set_common_app_config() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfig(sessionHandle, LINK_LAYER_MODE, Link_Layer_Mode_Bypass);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig() Failed");
        goto exit;
    }

    const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_ARRAY(
            ANTENNAE_CONFIGURATION_RX, &antennaeConfigurationRx[0], sizeof(antennaeConfigurationRx)),
        UWB_SET_VENDOR_APP_PARAM_VALUE(
            SESSION_INBAND_DATA_TX_BLOCKS, DEMO_INBAND_DATA_TRANSFER_APP_INBAND_DATA_TX_BLOCK),
        UWB_SET_VENDOR_APP_PARAM_VALUE(
            SESSION_INBAND_DATA_RX_BLOCKS, DEMO_INBAND_DATA_TRANSFER_APP_INBAND_DATA_RX_BLOCK),
    };

    status = UwbApi_SetVendorAppConfigs(
        sessionHandle, sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]), &SetVendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SetVendorAppConfigs() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfig(sessionHandle, LINK_LAYER_MODE, Link_Layer_Mode_Bypass);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfig(sessionHandle, RFRAME_CONFIG, kUWB_RfFrameConfig_SP1);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    inRangingParam.multiNodeMode     = DEMO_INBAND_DATA_TRANSFER_APP_MULTI_NODE_MODE_SINGLE;
    inRangingParam.macAddrMode       = DEMO_INBAND_DATA_TRANSFER_APP_DEVICE_MAC_ADD_MODE;
    inRangingParam.deviceRole        = DEMO_INBAND_DATA_TRANSFER_APP_DEV_ROLE_INITIATOR;
    inRangingParam.deviceType        = DEMO_INBAND_DATA_TRANSFER_APP_DEV_TYPE_CONTROLLER;
    inRangingParam.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParam.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    for (int i = 0; i < MAC_SHORT_ADD_LEN; i++) {
        inRangingParam.deviceMacAddr[i] = gkDeviceMacAddr[i];
    }

    status = UwbApi_SetRangingParams(sessionHandle, &inRangingParam);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    queryDataSz.sessionHandle = sessionHandle;
    status                    = UwbApi_SessionQueryDataSize(&queryDataSz);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionQueryDataSize() Failed");
        goto exit;
    }
    GENERATE_SEND_DATA(gData, queryDataSz.dataSize)

    status = UwbApi_StartRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    phOsalUwb_Delay(1000); //Wait for ranging data notification

    sendData.sessionHandle = sessionHandle;
    phOsalUwb_MemCopy(&sendData.mac_address, &gkDstMacAddr, MAC_EXT_ADD_LEN);
    sendData.sequence_number = 0;
    sendData.data_size       = queryDataSz.dataSize;
    sendData.data            = &gData[0];
    status                   = UwbApi_SendData(&sendData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SendData() Failed");
        goto exit;
    }

    phOsalUwb_Delay(10000); //Wait for ranging data notification
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
    threadparams.stackdepth   = DEMO_INBAND_DATA_TRANSFER_TX_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_INBAND_DATA_TRANSFER_TX_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_INBAND_DATA_TRANSFER_TX_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_INBAND_DATA_TRANSFER_TX
