/*
 *
 * Copyright 2018-2020,2022 NXP.
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

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

// Factory mode is enabled
#if UWBFTR_FactoryMode
#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary_Internal.h"
#include "phNxpLogApis_UwbApi.h"
#include "uci_ext_defs.h"
#include "UwbAdaptation.h"
#include "Factory_Firmware.h"
#include "uwa_api.h"
#include "uwa_dm_int.h"
#include "UwbApi_Utility.h"
#include "UwbApi.h"

#include "uwb_fwdl_provider.h"

/**
 * \brief Initialize the UWB Middleware stack with Factory Firmware
 *
 * \param[in] pCallback   Pointer to \ref tUwbApi_AppCallback
 *                         (Callback function to receive notifications at
 * application layer.)
 *
 * \retval #UWBAPI_STATUS_OK             on success
 * \retval #UWBAPI_STATUS_FAILED         otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT        if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_FactoryInit(tUwbApi_AppCallback *pCallback)
{
    tUWBAPI_STATUS status;
    phUwbFWImageContext_t fwImageCtx;
#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    fwImageCtx.fwImage   = (uint8_t *)heliosEncryptedFactoryFwImage;
    fwImageCtx.fwImgSize = sizeof(heliosEncryptedFactoryFwImage);
#endif
    fwImageCtx.fwMode = FACTORY_FW;
    status            = uwb_fwdl_getFwImage(&fwImageCtx);
    if (status != kUWBSTATUS_SUCCESS) {
        NXPLOG_UWBAPI_E("uwb_fwdl_getFwImage failed");
        return status;
    }

    /** Initialize with Default mode. */
    status = uwbInit(pCallback, kOPERATION_MODE_default);
    return status;
}

/**
 * \brief API to recover from Factory Firmware crash, cmd timeout.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_RecoverFactoryUWBS()
{
    tUWBAPI_STATUS status;
    phUwbFWImageContext_t fwImageCtx;
#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    fwImageCtx.fwImage   = (uint8_t *)heliosEncryptedFactoryFwImage;
    fwImageCtx.fwImgSize = sizeof(heliosEncryptedFactoryFwImage);
#endif
    fwImageCtx.fwMode = FACTORY_FW;
    status            = uwb_fwdl_getFwImage(&fwImageCtx);
    if (status != kUWBSTATUS_SUCCESS) {
        NXPLOG_UWBAPI_E("uwb_fwdl_getFwImage failed");
        return status;
    }

    status = recoverUWBS();
    return status;
}

#if (UWBIOT_UWBD_SR100T)
/**
 * \brief API to configure the auth tag options. Only applicable in Factory Firmware.
 * \param[in]  deviceTag     device Tag
 * \0x0 or 0xFF signifies that none of the calibration parameters are integrity protected by “Device Specific” Tag. Any other value signifies that the calibration parameters are integrity protected by “Device specific tag.
 * \param[in]  modelTag      model Tag
 * \0x0 or 0xFF signifies that none of the calibration parameters are integrity protected by “Model Specific” Tag. Any other value signifies that Calibration parameters are integrity protected by “Model specific tag.
 * \param[in]  labelValue    label Value
 * \This value must be different for each customer and also different for each model sold to same customer. NXP is responsible for choosing the customer specific label and providing to customer to be used in their production lines.
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_ConfigureAuthTagOptions(uint8_t deviceTag, uint8_t modelTag, uint16_t labelValue)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_PROP_CONFIGURE_AUTH_TAG_OPTION_RESP_EVT);
    cmdLen = serializeConfigureAuthTagOptionsPayload(deviceTag, modelTag, labelValue, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_CONFIG_AUTH_TAG_OPTIONS, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Configure Auth Tag Options cmd successful", __FUNCTION__);
        status = waitforNotification(EXT_UCI_MSG_CONFIGURE_AUTH_TAG_OPTIONS_CMD, UWBD_CALIB_NTF_TIMEOUT);

        if (status == UWBAPI_STATUS_OK) {
            status = uwbContext.wstatus;
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: Configure Auth Tag Options notification successful", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Configure Auth Tag Options notification failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Configure Auth Tag Options notification time out", __FUNCTION__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Configure Auth Tag Options cmd Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Configure Auth Tag Options cmd failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief API to configure the auth tag version. Only applicable in Factory Firmware.
 * \param[in]  tagVersion     Tag Version
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_ConfigureAuthTagVersion(uint16_t tagVersion)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_PROP_CONFIGURE_AUTH_TAG_VERSION_RESP_EVT);
    cmdLen = serializeConfigureAuthTagVersionPayload(tagVersion, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_CONFIG_AUTH_TAG_VERSIONS, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Configure Auth Tag Version cmd successful", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Configure Auth Tag Version cmd Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Configure Auth Tag Version cmd failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Generate Tag for the Calibration Parameters.
 *
 * \param[in] tagOption            Tag Option indicating Device/Model Specific tag
 * \param[out] pCmacTagResp         Pointer to \ref phGenerateTagRespStatus_t
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW    if response length is more than expected response size
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS UwbApi_GenerateTag(uint8_t tagOption, phGenerateTagRespStatus_t *pCmacTagResp)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pCmacTagResp == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_GENERATE_TAG_RESP_EVT);
    uwbContext.snd_data[0] = tagOption;
    cmdLen++;
    status = sendUciCommandAndWait(UWA_DM_API_PROP_GENERATE_TAG, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_PROP_GENERATE_TAG_NTF_EVT, UWBD_GENERATE_TAG_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(pCmacTagResp->status, p);
            if (pCmacTagResp->status == UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_OK;
                UWB_STREAM_TO_ARRAY(pCmacTagResp->cmactag, p, UWB_TAG_CMAC_LENGTH);
                NXPLOG_UWBAPI_D("%s: Generate Tag notification successful", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Generate Tag notification failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Generate Tag notification time out", __FUNCTION__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Generate Tag Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Generate Tag Command failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //UWBIOT_UWBD_SR100T

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
/**
 * \brief Write the Module Maker Info to OTP.
 *        This api can only be used with Factory Firmware
 * \param[in]  paramType    parameter to write into otp See :cpp:type:`eOtpParam_Type_t`
 * \param[in]  pInfo        Module Maker Info
 * \param[in]  infoLength   Info Length
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if the operation timed out
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_WriteOtpCmd(eOtpParam_Type_t paramType, uint8_t *pInfo, uint8_t infoLength)
{
    tUWBAPI_STATUS status;
    uint8_t *pDataCmd = NULL;
    uint16_t dataLen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: input params is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    pDataCmd = uwbContext.snd_data;
    switch (paramType) {
    case kUWB_OTP_ModuleMakerInfo: {
        UCI_MSG_BLD_HDR0(pDataCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
        UCI_MSG_BLD_HDR1(pDataCmd, EXT_UCI_MSG_WRITE_MODULE_MAKER_ID);
        UWB_UINT8_TO_STREAM(pDataCmd, 0x00);
        /* Work around in MW to support Module Maker ID size of 2 bytes
           Currently FW needs 8 bytes, change this when FW changes */
        if (infoLength == MODULE_MAKER_ID_MAX_SIZE) {
            UWB_UINT8_TO_STREAM(pDataCmd, MODULE_MAKER_ID_MAX_SIZE_FW);
            UWB_ARRAY_TO_STREAM(pDataCmd, pInfo, infoLength);
            phOsalUwb_SetMemory(pDataCmd, 0x00, MODULE_MAKER_ID_MAX_SIZE_FW - MODULE_MAKER_ID_MAX_SIZE);
            dataLen = (uint16_t)(MODULE_MAKER_ID_MAX_SIZE_FW + UCI_MSG_HDR_SIZE);
        }
        else {
            NXPLOG_UWBAPI_E(
                "%s: Max Module maker ID size supported is %d bytes", __FUNCTION__, MODULE_MAKER_ID_MAX_SIZE);
            return UWBAPI_STATUS_INVALID_PARAM;
        }
    } break;
    default:
        NXPLOG_UWBAPI_E("%s: paramType is wrong", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = sendRawUci(uwbContext.snd_data, dataLen);
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Write Otp Command successful", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Write Otp Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Write Otp Command failed", __FUNCTION__);
    }
    return status;
}
#endif //UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160
#endif // UWBFTR_FactoryMode
