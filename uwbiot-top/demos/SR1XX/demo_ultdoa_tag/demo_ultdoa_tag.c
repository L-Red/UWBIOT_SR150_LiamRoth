/* Copyright 2025 Liam Roth
 * Ported over from v04.04.03 to v04.06.05
 *
 *
 *  Copyright 2022 NXP
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

#ifndef UWBIOT_APP_BUILD__DEMO_ULTDOA_TAG
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_ULTDOA_TAG

/*
  * Below list contains the application configs which are only related to default
  * configuration.
  */

/********************************************************************************/
/*               Ranging APP configuration setting here */
/********************************************************************************/

#define DEMO_MAC_ADDR_MODE                 EXTENDED_MAC_ADDRESS_MODE_WITH_HEADER
#define DEMO_RANGING_APP_SESSION_ID        0x22334455
#define DEMO_RANGING_APP_NO_OF_ANCHORS_P2P 2

#if (DEMO_MAC_ADDR_MODE == SHORT_MAC_ADDRESS_MODE)
#define DEMO_MAC_ADDR_LEN MAC_SHORT_ADD_LEN
uint8_t DEVICE_MAC_ADDR[MAC_SHORT_ADD_LEN] = {0X00, 0x01};
#else
#define DEMO_MAC_ADDR_LEN MAC_EXT_ADD_LEN
static const uint8_t gkDeviceMacAddr[MAC_EXT_ADD_LEN] = {0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x1, 0x1};
#endif

uint8_t data[2023]      = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint8_t DST_MAC_ADDR[8] = {0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#define APP_DATA_SIZE (100)

/********************************************************************************/
#define DEMO_TDOA_TAG_TASK_SIZE 400
#define DEMO_TDOA_TAG_TASK_NAME "DemoULTDoATag"
#define DEMO_TDOA_TAG_TASK_PRIO 4

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
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, kUWB_StsConfig_StaticSts),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 200),
        UWB_SET_APP_PARAM_VALUE(SESSION_INFO_NTF, 0),
        UWB_SET_APP_PARAM_VALUE(SFD_ID, 0),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, 10),
        UWB_SET_APP_PARAM_VALUE(MAC_FCS_TYPE, 0),
		UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_RANGING_APP_NO_OF_ANCHORS_P2P),
    };

    phUwbDevInfo_t devInfo = {0};
    phRangingParams_t inRangingParams;

    PRINT_APP_NAME("SR1XXT APP UL-TDOA Ranging - Tag");

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

    status = UwbApi_SessionInit(DEMO_RANGING_APP_SESSION_ID, UWBD_RANGING_SESSION, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

//    status = demo_set_common_app_config(DEMO_RANGING_APP_SESSION_ID, kUWB_StsConfig_StaticSts);
//    if (status != UWBAPI_STATUS_OK) {
//        NXPLOG_APP_E("demo_set_common_app_config() Failed");
//        goto exit;
//    }

    status = UwbApi_SetAppConfigMultipleParams(
    		sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    inRangingParams.deviceRole     = kUWB_DeviceRole_UT_Tag;
    inRangingParams.multiNodeMode  = kUWB_MultiNodeMode_OnetoMany;
    inRangingParams.macAddrMode    = DEMO_MAC_ADDR_MODE;
    inRangingParams.deviceType     = kUWB_DeviceType_Controller;
    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
	inRangingParams.rangingRoundUsage = kUWB_RangingMethod_TDoA;


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
#if 0
    /************************************************************************************************/
    phUwbDataPkt_t sendData                 = {0};
    uint8_t dtTransferBuffer[APP_DATA_SIZE] = {0};

    phOsalUwb_SetMemory(dtTransferBuffer, 0xFE, APP_DATA_SIZE);

    /* Wait for 1 minute to send data by that time ranging data notification is received*/
    phOsalUwb_Delay(2 * 1000);

    sendData.session_id = sessionHandle;
    phOsalUwb_MemCopy(&sendData.mac_address, &DST_MAC_ADDR, MAC_EXT_ADD_LEN);
    sendData.dst_endpoint    = 0;
    sendData.sequence_number = 0;
    sendData.data_size       = APP_DATA_SIZE;
    sendData.data            = dtTransferBuffer;
    status                   = UwbApi_SendData(&sendData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SendData() Failed");
        goto exit;
    }
/************************************************************************************************/
#endif
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
    threadparams.stackdepth   = DEMO_TDOA_TAG_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_TDOA_TAG_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_TDOA_TAG_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_ULTDOA_TAG)
