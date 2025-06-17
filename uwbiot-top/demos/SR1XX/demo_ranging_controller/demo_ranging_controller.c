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
#include "AppRecovery.h"
#include "UwbApi.h"
#include <AppInternal.h>

#ifndef UWBIOT_APP_BUILD__DEMO_RANGING_CONTROLLER
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_RANGING_CONTROLLER

/*
  * Below list contains the application configs which are only related to default
  * configuration.
  */

/********************************************************************************/
/*               Ranging APP configuration setting here */
/********************************************************************************/

#define DEMO_RANGING_APP_SESSION_ID 0x11223344

#define DEMO_RANGING_APP_NO_OF_ANCHORS_P2P 1
#define DEMO_RNG_CONTROLLER_TASK_SIZE      400
#define DEMO_RNG_CONTROLLER_TASK_NAME      "DemoRngController"
#define DEMO_RNG_CONTROLLER_TASK_PRIO      4

static const uint8_t gkDstMacAddress[2]   = {0x22, 0x22};
static const uint8_t gDeviceMacAddress[2] = {0x11, 0x11};

uint8_t gExtDeviceMacAddr[8] = {0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x1, 0x1};
uint8_t gExtDstMacAddr[8]    = {0xd, 0x7, 0x6, 0x5, 0x4, 0x2, 0x2, 0xd};

#define DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT 0x0

/********************************************************************************/

AppContext_t appContext;

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status     = UWBAPI_STATUS_FAILED;
    const uint32_t session_id = DEMO_RANGING_APP_SESSION_ID;
    uint32_t sessionHandle    = 0;
    phUwbDevInfo_t devInfo;
    phRangingParams_t inRangingParams = {0};
    uint32_t delay;

    const UWB_AppParams_List_t SetAppParamsList[] = {

        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_RANGING_APP_NO_OF_ANCHORS_P2P),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gkDstMacAddress, MAC_SHORT_ADD_LEN),
    };

    PRINT_APP_NAME("Demo Ranging Controller");

    phUwbappContext_t appCtx = {0};
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

    // TX_POWER_ID
    uint16_t bitmask = (1<<1); // TX_POWER_ID
    phCalibPayload_t calib;
    /*  calib.TX_POWER_ID[4]
        [0]: TX_RMS_POWER_IND for TX ID1
        [1]: TX_Peak_POWER_IND for TX ID1
        [2]: TX_RMS_POWER_IND for TX ID2
        [3]: TX_Peak_POWER_IND for TX ID2
    */
    uint8_t channels[] = {5, 9};
    uint8_t channel;
    uint8_t TX_POWER[11] = {0x02 /* Number of parameters */,
                           0x01 /* for TX ID 1 */, 0x00, 0x00, 0x00, 0x00,
						   0x02 /* for TX ID 2 */, 0x00, 0x00, 0x00, 0x00};
    /* TX_POWER[11]
       [0]: Number of paramters
       [1]: TX ID 1
       [2][3]: TX_POWER_DELTA_PEAK for TX ID 1
       [4][5]: TX_POWER_ID_RMS for TX ID 1
       [6]: TX ID 2
       [7][8]: TX_POWER_DELTA_PEAK for TX ID 2
       [9][10]: TX_POWER_ID_RMS for TX ID 2
    */
    uint8_t CLK_ACCURACY[7] = { 0x03 /* Number of parameters */,
                                0x00, 0x00 /* CAP1 */,
								0x00, 0x00 /* CAP2 */,
								0x00, 0x00 /* GM CURRNT CONTROL */};

    int i;
    for(i = 0; i < sizeof(channels); i++) {
        channel = channels[i];
        UwbApi_ReadOtpCalibDataCmd(channel, bitmask, &calib);
        TX_POWER[4] = calib.TX_POWER_ID[0] + (2.1-0.6+0.5)*4;
        TX_POWER[2] = calib.TX_POWER_ID[1];
        UwbApi_SetCalibration(channel, TX_POWER_PER_ANTENNA, TX_POWER, sizeof(TX_POWER));
    }

    // XTAL_CAP
    bitmask = (1<<2); // XTAL_CAP
    UwbApi_ReadOtpCalibDataCmd(9, bitmask, &calib);    // channel 9 is dummy. not used for XTAL_CAP
    CLK_ACCURACY[1] = calib.XTAL_CAP_VALUES[0];
    CLK_ACCURACY[3] = calib.XTAL_CAP_VALUES[1];
    CLK_ACCURACY[5] = calib.XTAL_CAP_VALUES[2];
    UwbApi_SetCalibration(9, RF_CLK_ACCURACY_CALIB, CLK_ACCURACY, sizeof(CLK_ACCURACY));

    status = UwbApi_GetDeviceInfo(&devInfo);
    printDeviceInfo(&devInfo);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetDeviceInfo() Failed");
        goto exit;
    }

    status = UwbApi_SessionInit(session_id, UWBD_RANGING_SESSION, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    inRangingParams.deviceRole        = kUWB_DeviceRole_Initiator;
    inRangingParams.multiNodeMode     = kUWB_MultiNodeMode_UniCast;
    inRangingParams.macAddrMode       = DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    inRangingParams.deviceType        = kUWB_DeviceType_Controller;
    inRangingParams.deviceMacAddr[0]  = gDeviceMacAddress[0];
    inRangingParams.deviceMacAddr[1]  = gDeviceMacAddress[1];
    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    status = UwbApi_SetRangingParams(sessionHandle, &inRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    status = demo_set_common_app_config(sessionHandle, kUWB_StsConfig_StaticSts);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("demo_set_common_app_config() Failed");
        goto exit;
    }

    status = UwbApi_StartRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /* Delay 5 Mins for Ranging MILLISECONDS = MINUTES * 60 * 1000 */
    delay = 5 * 60 * 1000; /*Waiting for 5 mins*/

    /* When Ranging is terminated due to inband termination this semaphore will
     * be signaled, otherwise ranging will be performed for the time specified */
    if (UWBSTATUS_SUCCESS == phOsalUwb_ConsumeSemaphore_WithTimeout(inBandterminationSem, delay)) {
        status = UWBAPI_STATUS_OK;
        NXPLOG_APP_I(
            "\n-------------------------------------------\n in band termination "
            "is done  \n-------------------------------------------\n");
        UwbApi_SessionDeinit(sessionHandle);
        goto exit;
    }

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
    threadparams.stackdepth   = DEMO_RNG_CONTROLLER_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_RNG_CONTROLLER_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_RNG_CONTROLLER_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_RANGING_CONTROLLER
