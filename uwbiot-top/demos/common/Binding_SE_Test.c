/* Copyright 2019,2020 NXP
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

#if UWBIOT_UWBD_SR100T
// Enable for SR150 if needed

#include "AppInternal.h"
#if UWBFTR_SE_SN110
#include "SeApi.h"
#include "wearable_platform_int.h"
#endif // UWBFTR_SE_SN110
#include "Binding_SE_Test.h"
#include "UwbApi_Proprietary_Fm.h"
#define FAIL_STATUS           (uint8_t)(0xF)
#define TEST_LOOP_INTERVAL_MS 20
#define TEST_LOOP_COUNT       100

mainLineFwTestStatus_t doMainlineFwTest()
{
    tUWBAPI_STATUS status;
    mainLineFwTestStatus_t mainLineFwTestStatus;
    phOsalUwb_SetMemory(&mainLineFwTestStatus, FAIL_STATUS, sizeof(mainLineFwTestStatus));
    mainLineFwTestStatus.testStatus = UWBAPI_STATUS_FAILED;

    mainLineFwTestStatus.shutDownStatus = UwbApi_ShutDown();
    if (mainLineFwTestStatus.shutDownStatus != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ShutDown() Failed\n");
        mainLineFwTestStatus.testStatus = mainLineFwTestStatus.shutDownStatus;
        goto exit;
    }

#if UWBFTR_SE_SN110
    mainLineFwTestStatus.seInitStatus = SeApi_InitWithFirmware(&transactionOccurred,
        NULL,
        gphDnldNfc_DlSeqSz,
        gphDnldNfc_DlSequence,
        gphDnldNfc_DlSeqDummyFwSz,
        gphDnldNfc_DlSequenceDummyFw);
    if (mainLineFwTestStatus.seInitStatus != SEAPI_STATUS_OK) {
        NXPLOG_APP_E("SeApi_Init() Failed\n");
        mainLineFwTestStatus.testStatus = mainLineFwTestStatus.seInitStatus;
        goto exit;
    }
#endif // UWBFTR_SE_SN110
    NXPLOG_APP_I("SeApi_InitWithFirmware() pass\n");

    mainLineFwTestStatus.fwDlStatus = UwbApi_Init(&AppCallback_cdc);
    if (mainLineFwTestStatus.fwDlStatus != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Init() Failed for Mainline FW\n");
        mainLineFwTestStatus.testStatus = mainLineFwTestStatus.fwDlStatus;
        goto exit;
    }
    NXPLOG_APP_I("UwbApi_Init() pass\n");

    status = UwbApi_SeTestLoop(TEST_LOOP_COUNT, TEST_LOOP_INTERVAL_MS, &mainLineFwTestStatus.testLoopData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SeTestLoop() Failed for Mainline FW\n");
        mainLineFwTestStatus.testStatus = status;
        goto exit;
    }

    printTestLoopNtfData(&mainLineFwTestStatus.testLoopData);
    NXPLOG_APP_I("UwbApi_SeTestLoop() pass\n");

    status = UwbApi_GetBindingStatus(&mainLineFwTestStatus.getBindingStatus);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetBindingStatus() Failed for Mainline FW\n");
        mainLineFwTestStatus.testStatus = status;
        goto exit;
    }

    NXPLOG_APP_I("UwbApi_GetBindingStatus() pass\n");
#if (UWBFTR_SE_SN110)
    printGetBindingStatus(&mainLineFwTestStatus.getBindingStatus);
#endif //(UWBFTR_SE_SN110)

    mainLineFwTestStatus.testStatus = UWBAPI_STATUS_OK;
    return mainLineFwTestStatus;
exit:
    NXPLOG_APP_E("Mainline Fw Test Fail\n");
    return mainLineFwTestStatus;
}
// Factory mode is enabled
#if UWBFTR_FactoryMode
factoryFwTestStatus_t doFactoryFwTest()
{
    tUWBAPI_STATUS status;
    factoryFwTestStatus_t factoryTestStatus;

    phOsalUwb_SetMemory(&factoryTestStatus, FAIL_STATUS, sizeof(factoryTestStatus));
    factoryTestStatus.testStatus = UWBAPI_STATUS_OK;

    factoryTestStatus.shutDownStatus = UwbApi_ShutDown();
    if (factoryTestStatus.shutDownStatus != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ShutDown() Failed\n");
        factoryTestStatus.testStatus = factoryTestStatus.shutDownStatus;
        goto exit;
    }

#if UWBFTR_SE_SN110
    factoryTestStatus.seInitStatus = SeApi_InitWithFirmware(&transactionOccurred,
        NULL,
        gphDnldNfc_DlSeqSz,
        gphDnldNfc_DlSequence,
        gphDnldNfc_DlSeqDummyFwSz,
        gphDnldNfc_DlSequenceDummyFw);
    if (factoryTestStatus.seInitStatus != SEAPI_STATUS_OK) {
        NXPLOG_APP_E("SeApi_Init() Failed\n");
        factoryTestStatus.testStatus = factoryTestStatus.seInitStatus;
        goto exit;
    }
#endif // UWBFTR_SE_SN110

    factoryTestStatus.fwDlStatus = UwbApi_FactoryInit(&AppCallback_cdc);
    if (factoryTestStatus.fwDlStatus != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_FactoryInit() Failed for Factory FW\n");
        factoryTestStatus.testStatus = factoryTestStatus.fwDlStatus;
        goto exit;
    }
    NXPLOG_APP_I("UwbApi_FactoryInit() pass\n");

    factoryTestStatus.eseTestConnnectivity = UwbApi_TestConnectivity(&factoryTestStatus.eseTestConnnectivity);
    if (factoryTestStatus.connectivityTestStatus != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_TestConnectivity() Failed\n");
        factoryTestStatus.testStatus = factoryTestStatus.connectivityTestStatus;
        goto exit;
    }
    NXPLOG_APP_I("UwbApi_TestConnectivity() pass\n");

    status = UwbApi_GetBindingCount(&factoryTestStatus.getBindingCount);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetBindingCount() Failed\n");
        factoryTestStatus.testStatus = status;
        goto exit;
    }
    NXPLOG_APP_I("UwbApi_GetBindingCount() pass\n");

    status = UwbApi_PerformBinding(&factoryTestStatus.doBindStatus);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_PerformBinding() Failed\n");
        factoryTestStatus.testStatus = status;
        goto exit;
    }
    NXPLOG_APP_I("UwbApi_PerformBinding() pass\n");
#if (UWBFTR_SE_SN110)
    printDoBindStatus(&factoryTestStatus.doBindStatus);
#endif //(UWBFTR_SE_SN110)

    return factoryTestStatus;
exit:
    NXPLOG_APP_E("Factory Fw Test Fail\n");
    return factoryTestStatus;
}
#endif // UWBFTR_FactoryMode
#endif //UWBIOT_UWBD_SR100T
