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

#ifndef UWBIOT_APP_BUILD__DEMO_OTP_STORAGE_MAINLINE
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_OTP_STORAGE_MAINLINE

/*
  * Below list contains the application configs which are only related to default
  * configuration.
  */

/********************************************************************************/
#define DEMO_OTP_STORAGE_ML_TASK_SIZE 512
#define DEMO_OTP_STORAGE_ML_TASK_NAME "DemoOtpMainline"
#define DEMO_OTP_STORAGE_ML_TASK_PRIO 4

AppContext_t appContext;

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    phUwbappContext_t appCtx = {0};
    uint8_t channel          = 0x05;
    uint16_t bitMask;
    phCalibPayload_t readCalibData = {0x00};
    phCalibRespStatus_t calibResp  = {0x00};
    uint16_t getCalibData          = 0;

    uint8_t calibValues[16] = {0x00};
    uint8_t *pSetCalibValue;
    uint8_t *temp_response;
    phUwbDevInfo_t devInfo = {0};

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    eOtpParam_Type_t paramType = kUWB_OTP_ModuleMakerInfo;
    uint8_t readMMId[MODULE_MAKER_ID_MAX_SIZE];
    uint8_t readMMIdLen = MODULE_MAKER_ID_MAX_SIZE;
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160

    PRINT_APP_NAME("Demo Data Otp Storage Mainline");
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

    bitMask = (VCO_PLL_POS | PAPPPA_CALIB_CTRL_POS);
    status  = UwbApi_ReadOtpCalibDataCmd(channel, bitMask, &readCalibData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ReadOtpCalibDataCmd Failed");
        goto exit;
    }

    pSetCalibValue = calibValues;
    UWB_UINT16_TO_STREAM(pSetCalibValue, readCalibData.VCO_PLL);
    status = UwbApi_SetCalibration(channel, VCO_PLL, calibValues, sizeof(readCalibData.VCO_PLL));
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Set Calib param VCO_PLL Failed");
        goto exit;
    }

    pSetCalibValue -= 2;
    UWB_UINT16_TO_STREAM(pSetCalibValue, readCalibData.PA_PPA_CALIB_CTRL);
    status = UwbApi_SetCalibration(channel, PA_PPA_CALIB_CTRL, calibValues, sizeof(readCalibData.PA_PPA_CALIB_CTRL));
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Set Calib param PA_PPA_CALIB_CTRL Failed");
        goto exit;
    }

    status = UwbApi_GetCalibration(channel, VCO_PLL, &calibResp);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Set Calib param VCO_PLL Failed");
        goto exit;
    }
    temp_response = calibResp.calibValueOut;
    UWB_STREAM_TO_UINT16(getCalibData, temp_response);
    if (getCalibData != readCalibData.VCO_PLL) {
        NXPLOG_APP_E("Get Calib VCO_PLL data verification Failed");
        goto exit;
    }

    status = UwbApi_GetCalibration(channel, PA_PPA_CALIB_CTRL, &calibResp);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Set Calib param PA_PPA_CALIB_CTRL Failed");
        goto exit;
    }
    temp_response -= 2;
    UWB_STREAM_TO_UINT16(getCalibData, temp_response);
    if (getCalibData != readCalibData.PA_PPA_CALIB_CTRL) {
        NXPLOG_APP_E("Get Calib PA_PPA_CALIB_CTRL data verification Failed");
    }

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    status = UwbApi_ReadOtpCmd(paramType, readMMId, &readMMIdLen);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ReadOtpCmd Failed");
        goto exit;
    }
    NXPLOG_APP_I("Module Make ID is:");
    LOG_AU8_I(readMMId, readMMIdLen);
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160

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
    threadparams.stackdepth   = DEMO_OTP_STORAGE_ML_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_OTP_STORAGE_ML_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_OTP_STORAGE_ML_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_OTP_STORAGE_MAINLINE
