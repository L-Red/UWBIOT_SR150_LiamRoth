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
#include "UwbApi_Proprietary_Fm.h"

#ifndef UWBIOT_APP_BUILD__DEMO_OTP_STORAGE_FACTORY
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_OTP_STORAGE_FACTORY

/*
  * Below list contains the application configs which are only related to default
  * configuration.
  */

/********************************************************************************/
/*               Ranging APP configuration setting here */
/********************************************************************************/

#define DEMO_RANGING_APP_NO_OF_ANCHORS_P2P 1

uint8_t gDeviceMacAddrShort[2] = {0x22, 0x22};
uint8_t gDstMacAddrShort[2]    = {0x11, 0x11};

uint8_t gExtDeviceMacAddr[8] = {0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x1, 0x1};
uint8_t gExtDstMacAddr[8]    = {0xd, 0x7, 0x6, 0x5, 0x4, 0x2, 0x2, 0xd};

/* clang-format off */
uint8_t gPdoaOffsetCalib[] = {0x02, 0x01, 0x62, 0xd4};
uint8_t gMultiptCalib[]    = {0x44, 0x80, 0x1d, 0x1c, 0x50, 0x5c,  /* pair 1*/
        0xc3, 0x11, 0x44, 0x80, 0x98, 0x46, 0x50, 0x5c, 0x42,
        0xb6 /* pair 2*/};
/* clang-format on */

#define DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT 0x0
/********************************************************************************/

#define DEMO_OTP_STORAGE_FACTORY_TASK_SIZE 512
#define DEMO_OTP_STORAGE_FACTORY_TASK_NAME "DemoOtpFactory"
#define DEMO_OTP_STORAGE_FACTORY_TASK_PRIO 4

AppContext_t appContext;

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    phUwbappContext_t appCtx      = {0};
    phCalibRespStatus_t calibResp = {0x00};

    phUwbDevInfo_t devInfo;

    uint8_t channel = 0x05;
    uint16_t bitMask;
    phCalibPayload_t writeCalibData = {0x00};
    phCalibPayload_t readCalibData  = {0x00};
    uint8_t *tempResp;

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    eOtpParam_Type_t paramType             = kUWB_OTP_ModuleMakerInfo;
    uint8_t mmId[MODULE_MAKER_ID_MAX_SIZE] = {0x01, 0x02};
    uint8_t readMMId[MODULE_MAKER_ID_MAX_SIZE];
    uint8_t readMMIdLen = MODULE_MAKER_ID_MAX_SIZE;
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160

    PRINT_APP_NAME("Demo Data Otp Storage Factory");

#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedFactoryFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedFactoryFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = FACTORY_FW;
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

    status = UwbApi_DoVcoPllCalibration(channel, &calibResp);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_DoVcoPllCalibration VCO_PLL Failed");
        goto exit;
    }

    tempResp = calibResp.calibValueOut;
    UWB_STREAM_TO_UINT16(writeCalibData.VCO_PLL, tempResp);

    bitMask = VCO_PLL_POS;

#if UWBIOT_UWBD_SR150
    phOsalUwb_MemCopy(writeCalibData.PDOA_MFG_0_OFFSET_CALIB, gPdoaOffsetCalib, sizeof(gPdoaOffsetCalib));
    phOsalUwb_MemCopy(writeCalibData.AOA_ANT_MULTIPOINT_CALIB, gMultiptCalib, sizeof(gMultiptCalib));
    bitMask |= (PDOA_MFG_OFFSET_POS | PDOA_AOA_ANT_MULTIPOINT_CALIB);
#endif
    status = UwbApi_WriteOtpCalibDataCmd(channel, bitMask, &writeCalibData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_WriteOtpCalibDataCmd Failed");
        goto exit;
    }

    status = UwbApi_ReadOtpCalibDataCmd(channel, bitMask, &readCalibData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ReadOtpCalibDataCmd Failed");
        goto exit;
    }

    if ((writeCalibData.VCO_PLL != readCalibData.VCO_PLL) ||
        (writeCalibData.PA_PPA_CALIB_CTRL != readCalibData.PA_PPA_CALIB_CTRL)) {
        NXPLOG_APP_E("Write and Read OTP Calib data are mismatched");
        status = UWBAPI_STATUS_FAILED;
    }
#if UWBIOT_UWBD_SR150
    if (phOsalUwb_MemCompare(writeCalibData.PDOA_MFG_0_OFFSET_CALIB,
            readCalibData.PDOA_MFG_0_OFFSET_CALIB,
            sizeof(readCalibData.PDOA_MFG_0_OFFSET_CALIB))) {
        NXPLOG_APP_E("Write and Read OTP Calib for pdoa mfg0 offset data are mismatched");
        status = UWBAPI_STATUS_FAILED;
    }

    if (phOsalUwb_MemCompare(writeCalibData.AOA_ANT_MULTIPOINT_CALIB,
            readCalibData.AOA_ANT_MULTIPOINT_CALIB,
            sizeof(readCalibData.AOA_ANT_MULTIPOINT_CALIB))) {
        NXPLOG_APP_E("Write and Read OTP Calib for aoa antenna multipoint calib are mismatched");
        status = UWBAPI_STATUS_FAILED;
    }
#endif // #if UWBIOT_UWBD_SR150

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    status = UwbApi_WriteOtpCmd(paramType, mmId, (uint8_t)MODULE_MAKER_ID_MAX_SIZE);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_WriteOtpCmd Failed");
        goto exit;
    }

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
    threadparams.stackdepth   = DEMO_OTP_STORAGE_FACTORY_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_OTP_STORAGE_FACTORY_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_OTP_STORAGE_FACTORY_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_OTP_STORAGE_FACTORY
