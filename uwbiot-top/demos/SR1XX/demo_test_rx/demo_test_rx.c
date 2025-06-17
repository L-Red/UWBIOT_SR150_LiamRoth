/* Copyright 2021,2023 NXP
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

#ifndef UWBIOT_APP_BUILD__DEMO_TEST_RX
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_TEST_RX

#include "UwbApi.h"
#include <AppInternal.h>
#include "AppRecovery.h"
#include <UwbApi_Internal.h>
#include "UwbApi_RfTest.h"
#include <AppInternal.h>
#include "../demo_test_tx/app_Test_Cfg.h"

#define DEMO_NUM_PACKETS 40
#define DEMO_T_GAP       (4000)
#define DEMO_T_START     (450)
#define DEMO_T_WIN       (750)

/* DEMO_RANDOMIZE_OR_FIXED_PSDU
 * 0 = fixed
 * 1 = random
 */
#define DEMO_RANDOMIZE_OR_FIXED_PSDU 0

/** 1 == receive 100s of fames,
 * 0 == receive 1 by 1 */
#define DEMO_USE_PER_MODE 1

#define DEMO_TEST_RX_TASK_SIZE 400
#define DEMO_TEST_RX_TASK_NAME "DemoTestRx"
#define DEMO_TEST_RX_TASK_PRIO 4

phRfTestParams_t rfTestParams = {
    /* OK */
    .numOfPckts       = DEMO_NUM_PACKETS,
    .tGap             = DEMO_T_GAP,
    .tStart           = DEMO_T_START,
    .tWin             = DEMO_T_WIN,
    .randomizedSize   = DEMO_RANDOMIZE_OR_FIXED_PSDU,
    .phrRangingBit    = 0,
    .rmarkerRxStart   = 0,
    .rmarkerTxStart   = 0,
    .stsIndexAutoIncr = 0,
};

/* Select the channel ID we want to use */
static const uint8_t gkChannelId = DEMO_RF_TEST_CHANNEL_ID;

/*  [9-24]:BPRF, [25-32]:HPRF
 *
 * 10 default in Helios Test modes
 */
static const uint8_t gkPreambleCodeIndex = DEMO_RF_TEST_PREAMBLE_CODE_INDEX;

/*  [0,2]:BPRF, [1,3]:HPRF */
static const uint8_t gkSfdId = DEMO_RF_TEST_SFD_ID;

const UWB_RfFrameConfig_t rframeConfig = DEMO_RF_TEST_RFRAME_CONFIG;

/* Initialize the rx data*/
uint8_t gRxData[PSDU_DATA_SIZE] = {0};
/*
 * Below list contains the application configs which are only related to default configuration.
 */
OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status     = UWBAPI_STATUS_FAILED;
    phRfStartData_t startData = {0};
    phUwbDevInfo_t devInfo;
    uint32_t sessionHandle                  = 0;
    UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, gkChannelId),
        UWB_SET_APP_PARAM_VALUE(SFD_ID, gkSfdId),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, gkPreambleCodeIndex),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, rframeConfig),
        UWB_SET_APP_PARAM_VALUE(PSDU_DATA_RATE, DEMO_RF_TEST_PSDU_DATA_RATE),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_DURATION, DEMO_RF_TEST_PREAMBLE_DURATION),
        UWB_SET_APP_PARAM_VALUE(PRF_MODE, DEMO_RF_TEST_PRF_MODE),
    };

    PRINT_APP_NAME("Demo Test RX");

    /* Initialize the UWB Middleware */
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

    status = UwbApi_SessionInit(SESSION_ID_RFTEST, UWBD_RFTEST, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = UwbApi_SetRfTestParams(sessionHandle, &rfTestParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRfTestParams() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfig(sessionHandle, NUMBER_OF_STS_SEGMENTS, DEMO_RF_TEST_NUMBER_OF_STS_SEGMENTS);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig() for NUMBER_OF_STS_SEGMENTS is Failed");
        goto exit;
    }

    status = UwbApi_SetTestConfig(sessionHandle, TEST_SESSION_STS_KEY_OPTION, 1 /* IEEE Keys == 1 */);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetTestConfig() Failed");
        goto exit;
    }

    startData.startPerRxData.rxDataLength = sizeof(PSDU_DATA_SIZE);
    startData.startPerRxData.rxData       = gRxData;
#if DEMO_USE_PER_MODE
    status = UwbApi_StartRfTest(RF_START_PER_RX, &startData);
#else
    status = UwbApi_StartRfTest(RF_TEST_RX, &startData);

#endif
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartRfTest() Failed");
        goto exit;
    }
    /* Delay 1 Min for Ranging MILLISECONDS = MINUTES * 60 * 1000 */
    phOsalUwb_Delay(0.5 * 60 * 1000);

#if !(DEMO_USE_PER_MODE)
    /* Verify whether sent data and received data are same or not
    If status returns UWBAPI_STATUS_OK, it means data is successfully received at receiver
    Otherwise, some data loss or corruption is there */
    status = validateReceivedData();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("validateReceivedData() Failed");
        goto exit;
    }
#endif

    UwbApi_SessionDeinit(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionDeinit() Failed");
        goto exit;
    }
exit:
    if (status == UWBAPI_STATUS_TIMEOUT) {
        Handle_ErrorScenario(TIME_OUT);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        /* This must after HPD. Device Reset is must to come out of HPD*/
        status = UwbApi_ShutDown();
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_ShutDown() Failed");
        }
        UWBIOT_EXAMPLE_END(status);
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
    threadparams.stackdepth   = DEMO_TEST_RX_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_TEST_RX_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_TEST_RX_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // UWBIOT_APP_BUILD__DEMO_TEST_RX
