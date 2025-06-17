/*
 *
 * Copyright 2018-2020,2022,2023 NXP.
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

#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary_Internal.h"
#include "phNxpLogApis_UwbApi.h"
#include "uwa_api.h"
#include "phOsalUwb.h"
#include "uci_ext_defs.h"
#include "UwbAdaptation.h"
#include "AppConfigParams.h"
#include "PrintUtility.h"
#include "uwa_api.h"
#include "uwa_dm_int.h"
#include "UwbApi_Utility.h"
#include "UwbApi.h"
#include "uwbiot_ver.h"
#include "phNxpUwbConfig.h"

#if (UWBFTR_SE_SE051W)
#include "StateMachine.h"
#endif

#define WRITE_OTP_PAYLOAD_MAX_SIZE \
    17 /* 1(Calib param) + 16(Max size of Calib param i.e TX_POWER_PARAMS and TX_TEMP_COMP) */
/** Use to store Delay calibration value in OTP */
#define DELAY_CALIB_OTP (0x0B)

/**
 * \brief Returns UCI, FW and MW version
 *
 * \param[out] pdevInfo   Pointer to \ref phUwbDevInfo
 *
 * \retval UWBAPI_STATUS_OK               if successful
 * \retval UWBAPI_STATUS_NOT_INITIALIZED  if UCI stack is not initialized
 * \retval UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval UWBAPI_STATUS_FAILED           otherwise
 * \retval UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetDeviceInfo(phUwbDevInfo_t *pdevInfo)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter; ", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pdevInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: pdevInfo is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = getDeviceInfo();

    if (status == UWBAPI_STATUS_OK) {
        if (parseDeviceInfo(pdevInfo) == FALSE) {
            NXPLOG_UWBAPI_E("%s: Parsing Device Information Failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Parsing Device Information Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Parsing Device Information failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Do VCO PLL calibration parameters.
 *
 * \param[in] paramId       Channel
 * \param[out] paramcalibResp   Pointer to \ref phCalibRespStatus
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_FAILED             otherwise \return
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_DoVcoPllCalibration(uint8_t channel, phCalibRespStatus_t *calibResp)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (calibResp == NULL) {
        NXPLOG_UWBAPI_E("%s: calibResp is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    sep_SetWaitEvent(UWA_DM_VENDOR_DO_VCO_PLL_CALIBRATION_RESP_EVT);
    cmdLen = serializedoVcoPllCalibPayload(channel, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_DO_VCO_PLL_CALIBRATION, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(VENDOR_UCI_MSG_DO_VCO_PLL_CALIBRATION, UWBD_DO_CALIB_NTF_TIMEOUT);

        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = uwbContext.rsp_data;
            UWB_STREAM_TO_UINT8(calibResp->status, p);
            if (calibResp->status == UWBAPI_STATUS_OK) {
                calibResp->length = (uint8_t)(uwbContext.rsp_len - sizeof(calibResp->status));
                UWB_STREAM_TO_ARRAY(calibResp->calibValueOut, p, calibResp->length);
                NXPLOG_UWBAPI_D("%s: Do Calibration notification successful", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Do Calibration notification failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Do Calibration notification time out", __FUNCTION__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Do Calibration Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Do Calibration failed", __FUNCTION__);
    }

    if (status != UWBAPI_STATUS_OK) {
        calibResp->length = 0;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Set calibration parameters.
 *
 * \param[in] channel               channel
 * \param[in] paramId               Calibration parameter ID
 * \param[in] calibrationValue      Calibration value
 * \param[in] length                Calibration value array length
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetCalibration(
    uint8_t channel, eCalibParam paramId, uint8_t *calibrationValue, uint16_t length)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    switch (paramId) {
    case VCO_PLL:
    case RF_CLK_ACCURACY_CALIB:
    case RX_ANT_DELAY_CALIB:
    case PDOA_OFFSET_CALIB:
    case TX_POWER_PER_ANTENNA:
    case MANUAL_TX_POW_CTRL:
    case AOA_ANTENNAS_PDOA_CALIB:
    case PDOA_MANUFACT_ZERO_OFFSET_CALIB:
    case AOA_THRESHOLD_PDOA:
    case TX_TEMPERATURE_COMP_PER_ANTENNA:
    case SNR_CALIB_CONSTANT_PER_ANTENNA:
    case RSSI_CALIB_CONSTANT_HIGH_PWR:
    case RSSI_CALIB_CONSTANT_LOW_PWR:
#if UWBIOT_UWBD_SR1XXT
    case PA_PPA_CALIB_CTRL:
    case AOA_ANTENNAS_MULTIPOINT_CALIB:
#endif // UWBIOT_UWBD_SR1XXT
#if UWBIOT_UWBD_SR150
    case AOA_ANTENNAS_PDOA_CALIB_EXTENDED_SUPPORT:
#endif // UWBIOT_UWBD_SR150
#if UWBIOT_UWBD_SR2XXT
    case AOA_PHASEFLIP_ANTSPACING:
    case PLATFORM_ID:
    case CONFIG_VERSION:
    case TX_ANT_DELAY_CALIB:
    case TRA2_LOFT_CALIB:
    case TRA1_LOFT_CALIB:
#endif // UWBIOT_UWBD_SR2XXT
        break;
    default:
        NXPLOG_UWBAPI_E("%s: Invalid calibration parameter %0X ", __FUNCTION__, paramId);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (calibrationValue == NULL || length == 0) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (length > (MAX_CMD_BUFFER_DATA_TRANSFER - 3)) { // channel +  T(ag)calibParam + L(ength) + V(alue)calib data
        NXPLOG_UWBAPI_E("%s: calibration data is more that MAX_CMD_BUFFER_TRANSFER", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_VENDOR_SET_DEVICE_CALIBRATION_RESP_EVT);

    cmdLen = serializeSetCalibPayload(channel, paramId, calibrationValue, length, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Set Calibration successful", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Set Calibration Command Timed Out", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_INVALID_RANGE) {
        NXPLOG_UWBAPI_E("%s: Set Calibration Command failed, Invalid value range", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Set Calibration failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Get calibration parameters.
 *
 * \param[in]       channel         Channel
 * \param[in]       paramId         Calibration param Id
 * \param[inout]    calibResp       Pointer to \ref phCalibRespStatus
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with sessionHandle
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 *
 * Note: For #AOA_ANTENNAS_PDOA_CALIB, calibResp acts as an in/out param
 *       To get the caliberation values of the specified antennaId, the antennaId needs to be passed from application,
 *       by accessing the member of phCalibRespStatus_t i.e. inRxAntennaPair.
 *
 * Example to get the caliberation values for #AOA_ANTENNAS_PDOA_CALIB with the specific antennaID:
 * @code
 *  phCalibRespStatus_t calibResp = {0x00};
 *  calibResp.inRxAntennaPair = 0x01;
 *  status                    = UwbApi_GetCalibration(channel, AOA_ANTENNAS_PDOA_CALIB, &calibResp);
 *  if (status != UWBAPI_STATUS_OK) {
 *      NXPLOG_APP_E("Set Calib param AOA_ANTENNAS_PDOA_CALIB Failed");
 *      goto exit;
 *  }
 * @endcode
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetCalibration(uint8_t channel, eCalibParam paramId, phCalibRespStatus_t *calibResp)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    switch (paramId) {
    case VCO_PLL:
    case RF_CLK_ACCURACY_CALIB:
    case RX_ANT_DELAY_CALIB:
    case PDOA_OFFSET_CALIB:
    case TX_POWER_PER_ANTENNA:
    case MANUAL_TX_POW_CTRL:
    case AOA_ANTENNAS_PDOA_CALIB:
    case PDOA_MANUFACT_ZERO_OFFSET_CALIB:
    case AOA_THRESHOLD_PDOA:
    case TX_TEMPERATURE_COMP_PER_ANTENNA:
    case SNR_CALIB_CONSTANT_PER_ANTENNA:
    case RSSI_CALIB_CONSTANT_HIGH_PWR:
    case RSSI_CALIB_CONSTANT_LOW_PWR:
#if UWBIOT_UWBD_SR1XXT
    case PA_PPA_CALIB_CTRL:
    case AOA_ANTENNAS_MULTIPOINT_CALIB:
#endif // UWBIOT_UWBD_SR1XXT
#if UWBIOT_UWBD_SR150
    case AOA_ANTENNAS_PDOA_CALIB_EXTENDED_SUPPORT:
#endif // UWBIOT_UWBD_SR150
#if UWBIOT_UWBD_SR2XXT
    case AOA_PHASEFLIP_ANTSPACING:
    case PLATFORM_ID:
    case CONFIG_VERSION:
    case TX_ANT_DELAY_CALIB:
    case TRA2_LOFT_CALIB:
    case TRA1_LOFT_CALIB:
#endif // UWBIOT_UWBD_SR2XXT
        break;
    default:
        NXPLOG_UWBAPI_E("%s: Invalid calibration parameter ", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (calibResp == NULL) {
        NXPLOG_UWBAPI_E("%s: calibResp is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_VENDOR_GET_DEVICE_CALIBRATION_RESP_EVT);

    if (((paramId >> 8) & 0xFF) != EXTENTED_CALIB_PARAM_ID){
        cmdLen = serializeGetCalibPayload(channel, paramId, calibResp->inRxAntennaPair, &uwbContext.snd_data[0]);
    }
#if UWBIOT_UWBD_SR150
    else{
        cmdLen = serializeGetExtCalibPayload(channel, paramId, calibResp->inRxAntennaPair, &uwbContext.snd_data[0]);
    }
#endif // UWBIOT_UWBD_SR150
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_GET_DEVICE_CALIBRATION, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        if (((paramId >> 8) & 0xFF) != EXTENTED_CALIB_PARAM_ID){
            uint8_t *calibrationPtr = &uwbContext.rsp_data[0];

            calibResp->length =
                (uint16_t)(uwbContext.rsp_len - (sizeof(calibResp->status) + (1 /* sizeof(calibResp->calibState) */) +
                                                    1 /* Tag */ + 1 /* Length */));

            if (calibResp->length > sizeof(calibResp->calibValueOut)) {
                NXPLOG_UWBAPI_E("%s: Response data size is more than response buffer", __FUNCTION__);
                status = UWBAPI_STATUS_BUFFER_OVERFLOW;
            }
            else {
                UWB_STREAM_TO_UINT8(calibResp->status, calibrationPtr);
                calibResp->calibState = (eCalibState)*calibrationPtr++;
                if ((*calibrationPtr++) != (uint8_t)paramId) {
                    NXPLOG_UWBAPI_E("%s: Calibration tag mismatch", __FUNCTION__);
                    status = UWBAPI_STATUS_FAILED;
                }
                else {
                    if (calibResp->length != (*calibrationPtr++)) {
                        NXPLOG_UWBAPI_E("%s: Calibration length mismatch", __FUNCTION__);
                        status = UWBAPI_STATUS_FAILED;
                    }
                    else {
                        UWB_STREAM_TO_ARRAY(calibResp->calibValueOut, calibrationPtr, calibResp->length);
                        NXPLOG_UWBAPI_D("%s: Get Calibration successful", __FUNCTION__);
                    }
                }
            }
        }
#if UWBIOT_UWBD_SR150
        else {
            parseExtGetCalibResponse(&uwbContext.rsp_data[0], calibResp);
        }
#endif //UWBIOT_UWBD_SR150
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get Calibration Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get Calibration value failed", __FUNCTION__);
    }
    if (status != UWBAPI_STATUS_OK)
        calibResp->length = 0;
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Set Uwb Debug Configuration Parameters.
 * This API Can be used to set any number of debug paramters at once.
 *
 * To easily set the DebugParams list, following macros have been defined.
 *
 * UWB_SET_DEBUG_PARAM_VALUE_u8(Parameter, Value): This macro sets the value of the
 * corresponding parameter with the given Value.This shall be used to set value of 8 bit wide.
 *
 * UWB_SET_DEBUG_PARAM_VALUE_u16(Parameter, Value): This macro sets the value of the
 * corresponding parameter with the given Value.This shall be used to set value of 16 bit wide.
 *
 * UWB_SET_DEBUG_PARAM_VALUE_u32(Parameter, Value): This macro sets the value of the
 * corresponding parameter with the given Value.This shall be used to set value of 32 bit wide.
 *
 * UWB_SET_DEBUG_PARAM_ARRAY(PARAM, ARRAY, LENGTH): This macro sets the value of the
 * corresponding parameter with given Value & length. This shall be used to set value of arrays.
 * Example: To set DATA_LOGGER_NTF to zero, macro shall be invoked as given below for SR1XX.
 * @code
 * UWB_DebugParams_List_t SetDebugParamsList[] = {UWB_SET_DEBUG_PARAM_VALUE_u8(DATA_LOGGER_NTF, 0)};
 * @endcode
 * Example: To set DATA_LOGGER_NTF to zero, macro shall be invoked as given below for SR2XX.
 * DATA_LOGGER_NTF is 6 byte for SR2XX.
 * @code
 * UWB_DebugParams_List_t SetDebugParamsList[] = {UWB_SET_DEBUG_PARAM_ARRAY(DATA_LOGGER_NTF, uint8_t*, 6)};
 * @endcode
 *
 * \param[in] sessionHandle          Initialized Session Handle
 * \param[in] noOfparams         Number of App Config Parameters
 * \param[in] DebugParams_List   Debug parameters values in tlv format
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetDebugParams(
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_DebugParams_List_t *DebugParams_List)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen                  = 0;
    uint8_t paramLen                 = 0;
    uint8_t flag_for_data_logger_ntf = false;
    UWB_SR1XX_DBG_CFG_t paramId;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    UWB_Debug_Params_value_t output_param_value;

    /* Check if the device is initialized or not */
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    /* Check if the passed list is having parameters or not */
    if ((DebugParams_List == NULL) || ((noOfparams == 0))) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Assign the buffer for storing all the configs */
    output_param_value.param_value = uwbContext.snd_data;

    for (uint32_t LoopCnt = 0; LoopCnt < noOfparams; ++LoopCnt) {
        paramId = DebugParams_List[LoopCnt].param_id;

        if (paramId >= END_OF_SUPPORTED_EXT_DEBUG_CONFIGS) {
            NXPLOG_UWBAPI_E("%s: Invalid Parameter value", __FUNCTION__);
            return UWBAPI_STATUS_INVALID_PARAM;
        }
        if (paramId == kUWB_SR1XX_DBG_CFG_DATA_LOGGER_NTF) {
            flag_for_data_logger_ntf = true;
        }
        /* parse and get input length and pointer */
        if (DebugConfig_TlvParser(&DebugParams_List[LoopCnt], &output_param_value) != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: DebugConfig_TlvParser() failed", __FUNCTION__);
            return UWBAPI_STATUS_FAILED;
        }

        // we have only extended debug configs
        paramLen = (uint16_t)(paramLen + getVendorDebugConfigTLVBuffer(paramId,
                                             (void *)(output_param_value.param_value),
                                             output_param_value.param_len,
                                             &uwbContext.rsp_data[payloadOffset + paramLen]));
    }

    if (paramLen == 0) {
        NXPLOG_UWBAPI_E("%s: getVendorDebugConfigTLVBuffer() failed", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    sep_SetWaitEvent(UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, paramLen, uwbContext.rsp_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.rsp_data);

    if (status == UWBAPI_STATUS_OK && (flag_for_data_logger_ntf)) {
        tHAL_UWB_IOCTL ioCtl;
        InputOutputData_t ioData;
        ioData.enableFwDump  = FALSE;
        ioData.enableCirDump = FALSE;

        if (flag_for_data_logger_ntf) {
            ioData.enableFwDump = true;
        }
        const tHAL_UWB_ENTRY *halFuncEntries = NULL;
        halFuncEntries                       = GetHalEntryFuncs();
        if (halFuncEntries) {
            ioCtl.pIoData = &ioData;
            halFuncEntries->ioctl(HAL_UWB_IOCTL_DUMP_FW_LOG, &ioCtl);
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Get Uwb Debug Configuration Parameters.
 * This API Can be used to get any number of debug paramters at once.
 *
 * To easily get the DebugParams list, following macro has been defined.
 *
 * UWB_SET_GETAPP_PARAM_u8(Parameter): This macro gets the value of the
 * corresponding parameter.This shall be used to get values of 8 bit wide.
 *
 *  * UWB_SET_GETAPP_PARAM_u16(Parameter): This macro gets the value of the
 * corresponding parameter.This shall be used to get values of 16 bit wide.
 *
 *  * UWB_SET_GETAPP_PARAM_u32(Parameter): This macro gets the value of the
 * corresponding parameter.This shall be used to get values of 32 bit wide.
 *
 * Example: To get DATA_LOGGER_NTF macro shall be invoked as given below.
 *
 * @code
 * UWB_DebugParams_List_t GetDebugParamsList[] = {UWB_SET_GETDEBUG_PARAM_u8(DATA_LOGGER_NTF),};
 * @endcode
 *
 * \param[in] sessionHandle          Initialized Session Handle
 * \param[in] noOfparams         Number of App Config Parameters
 * \param[in] DebugParams_List   Debug parameters values in tlv format
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetDebugParams(
    uint32_t sessionHandle, uint8_t noOfparams, UWB_DebugParams_List_t *DebugParams_List)
{
    tUWBAPI_STATUS status;
    uint8_t i = 0;
    UWB_SR1XX_DBG_CFG_t paramId;
    uint8_t *pConfigCommand = NULL;
    uint8_t payloadOffset   = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    uint16_t cmdLen         = 0;
    uint8_t offset          = 0;
    uint8_t *rspPtr         = NULL;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((DebugParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pConfigCommand = &uwbContext.snd_data[payloadOffset];
    for (i = 0; i < noOfparams; i++) {
        paramId = DebugParams_List[i].param_id;

        if (paramId >= END_OF_SUPPORTED_EXT_DEBUG_CONFIGS) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }

        // we have only extended debug configs
        pConfigCommand[cmdLen++] = (uint8_t)(paramId & 0xFF);
    }

    sep_SetWaitEvent(UWA_DM_SESSION_GET_VENDOR_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, cmdLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        rspPtr = &uwbContext.rsp_data[offset];
        parseDebugParams(rspPtr, noOfparams, DebugParams_List);
    }
    return status;
}

#if UWBFTR_SE_SN110
/**
 * \brief DoBind Perform Factory Binding using this API
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_PerformBinding(phSeDoBindStatus_t *doBindStatus)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (doBindStatus == NULL) {
        NXPLOG_UWBAPI_E("%s: doBindStatus is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    sep_SetWaitEvent(UWA_DM_PROP_DO_BIND_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_DO_BIND, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        sep_SetWaitEvent(VENDOR_UCI_MSG_SE_DO_BIND);
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWBD_SE_TIMEOUT) == UWBSTATUS_SUCCESS) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(doBindStatus->status, p);
            if (doBindStatus->status != UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_FAILED;
                NXPLOG_UWBAPI_W(
                    "%s: Get doBindStatus is not success, status is %0x", __FUNCTION__, doBindStatus->status);
            }
            UWB_STREAM_TO_UINT8(doBindStatus->count_remaining, p);
            UWB_STREAM_TO_UINT8(doBindStatus->binding_state, p);
            UWB_STREAM_TO_UINT16(doBindStatus->se_instruction_code, p);
            UWB_STREAM_TO_UINT16(doBindStatus->se_error_status, p);
        }
        else {
            NXPLOG_UWBAPI_E("%s: DoBind status is failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Do Binding Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Binding is failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //(UWBFTR_SE_SN110)

#if (UWBFTR_SE_SE051W)
/**
 * \brief Performs Factory Binding only if the current state is not bound and
 * not locked.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 *
 * Note: This API is recommended to be used in Low Power Mode Disabled Condition to avoid the low power mode related transitions.
 */
EXTERNC tUWBAPI_STATUS UwbApi_PerformBinding(void)
{
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    return Binding_Process();
}

/**
  * \brief Performs Lock Binding only if the current state is bound and
  * Unlocked. This is only supported with Helios Mainline Firmware.
  *
  * \retval #UWBAPI_STATUS_OK               on success
  * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
  * \retval #UWBAPI_STATUS_FAILED           otherwise
  *
  * Note: This API is recommended to be used in Low Power Mode Disabled Condition to avoid the low power mode related transitions.
  */
EXTERNC tUWBAPI_STATUS UwbApi_PerformLocking(void)
{
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    return locking_Process();
}

/**
 * \brief Host shall use this API to set Wrapped RDS application confifuration parameter.
 *
 * \param[in] sessionHandle        Initialized Session Handle
 * \param[in] pWrappedRds      Wrapped RDS
 * \param[in] WrappedRdsLen    Wrapped RDS Length
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfigWrappedRDS(uint32_t sessionHandle, uint8_t *pWrappedRds, size_t WrappedRdsLen)
{
    tUWBAPI_STATUS uwb_status = UWBAPI_STATUS_FAILED;
    uint16_t cmdLen           = 0;
    uint8_t payloadOffset     = 0;

    uint32_t WrappedRdsLenU32 = (uint32_t)WrappedRdsLen;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        uwb_status = UWBAPI_STATUS_NOT_INITIALIZED;
        goto exit;
    }

    if (pWrappedRds == NULL) {
        NXPLOG_UWBAPI_E("%s: pWrappedRds is NULL", __FUNCTION__);
        uwb_status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    cmdLen                        = serializeAppConfigPayload(sessionHandle, 1, payloadOffset, uwbContext.snd_data);
    uwbContext.snd_data[cmdLen++] = UCI_VENDOR_PARAM_ID_WRAPPED_RDS;
    uwbContext.snd_data[cmdLen++] = (uint8_t)WrappedRdsLen;
    phOsalUwb_MemCopy(&uwbContext.snd_data[cmdLen], (void *)pWrappedRds, WrappedRdsLenU32);
    cmdLen += (uint16_t)WrappedRdsLen;

    sep_SetWaitEvent(UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT);
    uwb_status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);
exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, uwb_status);
    return uwb_status;
}
#endif

/**
 * \brief Write the calibration parameters to OTP.
 *
 * \param[in] channel        Channel ID
 * \param[in] bitMask        bit mask for the configurations which are set by user and passed part of phCalibPayload_t.
 *                     bitmask description:
 *                        set bit 0 -> VCO_PLL
 *                        set bit 1 -> TX_POWER_ID
 *                        set bit 2 -> XTAL_CAP_VALUES
 *                        set bit 3 -> RSSI_CONSTANT1
 *                        set bit 4 -> RSSI_CONSTANT2
 *                            bit 5 -> RFU
 *                        set bit 6 -> TX_POWER_PARAMS
 *                            bit 7 -> RFU
 *                        set bit 8 -> PA_PPA_CALIB_CTRL
 *                        set bit 9 -> TX_TEMP_COMP
 *                           bit 10 -> RFU
 *                       set bit 11 -> DELAY_CALIB
 *                       bit 12 to 15 -> RFU
 * \param[in] pCalibPayload   param values to be set
 *
 * \retval #UWBAPI_STATUS_OK  on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM  if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT  if command is timeout
 * \retval #UWBAPI_STATUS_FAILED  otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_WriteOtpCalibDataCmd(uint8_t channel, uint16_t bitMask, phCalibPayload_t *pCalibPayload)
{
    tUWBAPI_STATUS status                         = UWBAPI_STATUS_FAILED;
    uint16_t bitPos                               = 0;
    uint8_t writeDataLen                          = 0x00;
    uint8_t writeData[WRITE_OTP_PAYLOAD_MAX_SIZE] = {0x00};
    uint16_t cmdLen                               = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pCalibPayload == NULL) {
        NXPLOG_UWBAPI_E("%s: Calib params is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    for (bitPos = 1; bitPos <= 16; bitPos++) {
        if ((bitMask >> (bitPos - 1)) & 1) {
            switch (bitPos - 1) {
            case OTP_CALIB_VCO_PLL:
                writeDataLen = sizeof(uint8_t) + sizeof(pCalibPayload->VCO_PLL);
                writeData[0] = (uint8_t)OTP_CALIB_VCO_PLL;
                writeData[1] = (uint8_t)((pCalibPayload->VCO_PLL) & 0x00FF);
                writeData[2] = (uint8_t)(((pCalibPayload->VCO_PLL) >> 8) & 0x00FF);
                break;
            case OTP_CALIB_TX_POWER:
                writeDataLen = sizeof(pCalibPayload->TX_POWER_ID);
                writeData[0] = (uint8_t)OTP_CALIB_TX_POWER;
                phOsalUwb_MemCopy(&writeData[1], pCalibPayload->TX_POWER_ID, writeDataLen);
                writeDataLen = (uint8_t)(writeDataLen + sizeof(uint8_t));
                break;
            case OTP_CALIB_RF_XTAL_CAP:
                writeDataLen = sizeof(pCalibPayload->XTAL_CAP_VALUES);
                writeData[0] = (uint8_t)OTP_CALIB_RF_XTAL_CAP;
                phOsalUwb_MemCopy(&writeData[1], pCalibPayload->XTAL_CAP_VALUES, writeDataLen);
                writeDataLen = (uint8_t)(writeDataLen + sizeof(uint8_t));
                break;
            case OTP_CALIB_RSSI_CALIB_CONST1:
                writeDataLen = sizeof(pCalibPayload->RSSI_CONSTANT1);
                writeData[0] = (uint8_t)OTP_CALIB_RSSI_CALIB_CONST1;
                phOsalUwb_MemCopy(&writeData[1], pCalibPayload->RSSI_CONSTANT1, writeDataLen);
                writeDataLen = (uint8_t)(writeDataLen + sizeof(uint8_t));
                break;
            case OTP_CALIB_RSSI_CALIB_CONST2:
                writeDataLen = sizeof(pCalibPayload->RSSI_CONSTANT2);
                writeData[0] = (uint8_t)OTP_CALIB_RSSI_CALIB_CONST2;
                phOsalUwb_MemCopy(&writeData[1], pCalibPayload->RSSI_CONSTANT2, writeDataLen);
                writeDataLen = (uint8_t)(writeDataLen + sizeof(uint8_t));
                break;
            case OTP_CALIB_MANUAL_TX_POW_CTRL:
                writeDataLen = sizeof(pCalibPayload->TX_POWER_PARAMS);
                writeData[0] = (uint8_t)OTP_CALIB_MANUAL_TX_POW_CTRL;
                phOsalUwb_MemCopy(&writeData[1], pCalibPayload->TX_POWER_PARAMS, writeDataLen);
                writeDataLen = (uint8_t)(writeDataLen + sizeof(uint8_t));
                break;
            case OTP_CALIB_PAPPPA_CALIB_CTRL:
                writeDataLen = sizeof(uint8_t) + sizeof(pCalibPayload->PA_PPA_CALIB_CTRL);
                writeData[0] = (uint8_t)OTP_CALIB_PAPPPA_CALIB_CTRL;
                writeData[1] = (uint8_t)(pCalibPayload->PA_PPA_CALIB_CTRL);
                writeData[2] = (uint8_t)((pCalibPayload->PA_PPA_CALIB_CTRL) >> 8);
                break;
            case OTP_CALIB_TX_TEMPARATURE_COMP:
                writeDataLen = sizeof(pCalibPayload->TX_TEMP_COMP);
                writeData[0] = (uint8_t)OTP_CALIB_TX_TEMPARATURE_COMP;
                phOsalUwb_MemCopy(&writeData[1], pCalibPayload->TX_TEMP_COMP, writeDataLen);
                writeDataLen = (uint8_t)(writeDataLen + sizeof(uint8_t));
                break;
            case OTP_CALIB_DELAY_CALIB:
                writeDataLen = sizeof(uint8_t) + sizeof(pCalibPayload->DELAY_CALIB_VALUE);
                writeData[0] = (uint8_t)OTP_CALIB_DELAY_CALIB;
                writeData[1] = (uint8_t)(pCalibPayload->DELAY_CALIB_VALUE);
                writeData[2] = (uint8_t)((pCalibPayload->DELAY_CALIB_VALUE) >> 8);
                break;
#if UWBIOT_UWBD_SR1XXT
            case OTP_PDOA_MFG_ZERO_OFFSET_CALIB:
                writeDataLen = sizeof(uint8_t) + sizeof(pCalibPayload->PDOA_MFG_0_OFFSET_CALIB);
                writeData[0] = (uint8_t)OTP_PDOA_MFG_ZERO_OFFSET_CALIB;
                phOsalUwb_MemCopy(&writeData[1],
                    pCalibPayload->PDOA_MFG_0_OFFSET_CALIB,
                    sizeof(pCalibPayload->PDOA_MFG_0_OFFSET_CALIB));
                break;
            case OTP_AOA_ANT_MULTIPOINT_CALIB:
                writeDataLen = sizeof(uint8_t) + sizeof(pCalibPayload->AOA_ANT_MULTIPOINT_CALIB);
                writeData[0] = OTP_AOA_ANT_MULTIPOINT_CALIB;
                phOsalUwb_MemCopy(&writeData[1],
                    pCalibPayload->AOA_ANT_MULTIPOINT_CALIB,
                    sizeof(pCalibPayload->AOA_ANT_MULTIPOINT_CALIB));
                break;
#endif //#if UWBIOT_UWBD_SR1XXT
            default:
                NXPLOG_UWBAPI_E("%s: Invalid calibration parameter ", __FUNCTION__);
                return UWBAPI_STATUS_INVALID_PARAM;
            }

            if (writeDataLen > (MAX_UCI_PACKET_SIZE - 3)) { // channle + calibParam + len
                NXPLOG_UWBAPI_E("%s: not enough buffer for calibration data OTP", __FUNCTION__);
                return UWBAPI_STATUS_INVALID_PARAM;
            }

            sep_SetWaitEvent(UWA_DM_PROP_WRITE_OTP_CALIB_DATA_RSP_EVT);
            cmdLen = serializeWriteOtpCalibDataPayload(
                channel, (uint8_t)CALIB_PARAM, writeDataLen, writeData, &uwbContext.snd_data[0]);
            status = sendUciCommandAndWait(UWA_DM_API_PROP_WRITE_OTP_CALIB_DATA, cmdLen, uwbContext.snd_data);

            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: Write Calibration Data command successful", __FUNCTION__);
                status = waitforNotification(EXT_UCI_MSG_WRITE_CALIB_DATA_CMD, UWBD_CALIB_NTF_TIMEOUT);
                if (status == UWBAPI_STATUS_OK) {
                    status = uwbContext.wstatus;
                    if (status == UWBAPI_STATUS_OK) {
                        NXPLOG_UWBAPI_D("%s: Write Calibration Data Notification successfull", __FUNCTION__);
                    }
                    else {
                        NXPLOG_UWBAPI_E("%s: Write Calibration Data Notification failed", __FUNCTION__);
                        break;
                    }
                }
                else {
                    NXPLOG_UWBAPI_E("%s: Write Calibration Data Notification Timed out", __FUNCTION__);
                    break;
                }
            }
            else if (status == UWBAPI_STATUS_TIMEOUT) {
                NXPLOG_UWBAPI_E("%s: Write Calibration Data Command Timed Out", __FUNCTION__);
                break;
            }
            else {
                NXPLOG_UWBAPI_E("%s: Write Calibration Data Command failed", __FUNCTION__);
                break;
            }
        }
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Read the calibration parameters from OTP.
 *
 * \param[in] channel        Channel ID
 * \param[in] bitMask        bit mask for the params to be retrived from OTP
 *                     bitmask description:
 *                        set bit 0 -> VCO_PLL
 *                        set bit 1 -> TX_POWER_ID
 *                        set bit 2 -> XTAL_CAP_VALUES
 *                        set bit 3 -> RSSI_CONSTANT1
 *                        set bit 4 -> RSSI_CONSTANT2
 *                            bit 5 -> RFU
 *                        set bit 6 -> TX_POWER_PARAMS
 *                            bit 7 -> RFU
 *                        set bit 8 -> PA_PPA_CALIB_CTRL
 *                        set bit 9 -> TX_TEMP_COMP
 *                           bit 10 -> RFU
 *                       set bit 11 -> DELAY_CALIB
 *                       bit 12 to 15 -> RFU
 * \param[out] pCalibPayload   Calib payload structure
 *
 * \retval #UWBAPI_STATUS_OK  on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM  if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT  if command is timeout
 * \retval #UWBAPI_STATUS_FAILED  otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_ReadOtpCalibDataCmd(uint8_t channel, uint16_t bitMask, phCalibPayload_t *pCalibPayload)
{
    tUWBAPI_STATUS status      = UWBAPI_STATUS_FAILED;
    uint8_t *pReadCalibDataRsp = NULL;
    uint16_t cmdLen            = 0;
    eOtpCalibParam calibParam;
    uint16_t bitPos = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pCalibPayload == NULL) {
        NXPLOG_UWBAPI_E("%s: Calib params is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    for (bitPos = 1; bitPos <= 16; bitPos++) {
        if ((bitMask >> (bitPos - 1)) & 1) {
            switch (bitPos - 1) {
            case OTP_CALIB_VCO_PLL:
                calibParam = OTP_CALIB_VCO_PLL;
                break;
            case OTP_CALIB_TX_POWER:
                calibParam = OTP_CALIB_TX_POWER;
                break;
            case OTP_CALIB_RF_XTAL_CAP:
                calibParam = OTP_CALIB_RF_XTAL_CAP;
                break;
            case OTP_CALIB_RSSI_CALIB_CONST1:
                calibParam = OTP_CALIB_RSSI_CALIB_CONST1;
                break;
            case OTP_CALIB_RSSI_CALIB_CONST2:
                calibParam = OTP_CALIB_RSSI_CALIB_CONST2;
                break;
            case OTP_CALIB_MANUAL_TX_POW_CTRL:
                calibParam = OTP_CALIB_MANUAL_TX_POW_CTRL;
                break;
            case OTP_CALIB_PAPPPA_CALIB_CTRL:
                calibParam = OTP_CALIB_PAPPPA_CALIB_CTRL;
                break;
            case OTP_CALIB_TX_TEMPARATURE_COMP:
                calibParam = OTP_CALIB_TX_TEMPARATURE_COMP;
                break;
            case OTP_CALIB_DELAY_CALIB:
                calibParam = OTP_CALIB_DELAY_CALIB;
                break;
#if UWBIOT_UWBD_SR1XXT
            case OTP_PDOA_MFG_ZERO_OFFSET_CALIB:
                calibParam = OTP_PDOA_MFG_ZERO_OFFSET_CALIB;
                break;
            case OTP_AOA_ANT_MULTIPOINT_CALIB:
                calibParam = OTP_AOA_ANT_MULTIPOINT_CALIB;
                break;
#endif //UWBIOT_UWBD_SR1XXT
            default:
                NXPLOG_UWBAPI_E("%s: Invalid calibration parameter ", __FUNCTION__);
                return UWBAPI_STATUS_INVALID_PARAM;
            }

            sep_SetWaitEvent(UWA_DM_PROP_READ_OTP_CALIB_DATA_RSP_EVT);
            cmdLen =
                serializeReadOtpCalibDataPayload(channel, (uint8_t)CALIB_PARAM, calibParam, &uwbContext.snd_data[0]);
            status = sendUciCommandAndWait(UWA_DM_API_PROP_READ_OTP_CALIB_DATA, cmdLen, uwbContext.snd_data);

            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: Read Calibration Data command successful", __FUNCTION__);
                status = waitforNotification(UWA_DM_PROP_SE_READ_CALIB_DATA_NTF_EVT, UWBD_CALIB_NTF_TIMEOUT);
                if (status == UWBAPI_STATUS_OK) {
                    status = uwbContext.wstatus;
                    if (status == UWBAPI_STATUS_OK) {
                        pReadCalibDataRsp = uwbContext.rsp_data;
                        NXPLOG_UWBAPI_D("%s: Read Calibration Data Notification successfull", __FUNCTION__);
                        switch (calibParam) {
                        case OTP_CALIB_VCO_PLL:
                            UWB_STREAM_TO_UINT16(pCalibPayload->VCO_PLL, pReadCalibDataRsp);
                            break;
                        case OTP_CALIB_TX_POWER:
                            phOsalUwb_MemCopy(pCalibPayload->TX_POWER_ID, uwbContext.rsp_data, uwbContext.rsp_len);
                            break;
                        case OTP_CALIB_RF_XTAL_CAP:
                            phOsalUwb_MemCopy(pCalibPayload->XTAL_CAP_VALUES, uwbContext.rsp_data, uwbContext.rsp_len);
                            break;
                        case OTP_CALIB_RSSI_CALIB_CONST1:
                            phOsalUwb_MemCopy(pCalibPayload->RSSI_CONSTANT1, uwbContext.rsp_data, uwbContext.rsp_len);
                            break;
                        case OTP_CALIB_RSSI_CALIB_CONST2:
                            phOsalUwb_MemCopy(pCalibPayload->RSSI_CONSTANT2, uwbContext.rsp_data, uwbContext.rsp_len);
                            break;
                        case OTP_CALIB_MANUAL_TX_POW_CTRL:
                            phOsalUwb_MemCopy(pCalibPayload->TX_POWER_PARAMS, uwbContext.rsp_data, uwbContext.rsp_len);
                            break;
                        case OTP_CALIB_PAPPPA_CALIB_CTRL:
                            UWB_STREAM_TO_UINT16(pCalibPayload->PA_PPA_CALIB_CTRL, pReadCalibDataRsp);
                            break;
                        case OTP_CALIB_TX_TEMPARATURE_COMP:
                            phOsalUwb_MemCopy(pCalibPayload->TX_TEMP_COMP, uwbContext.rsp_data, uwbContext.rsp_len);
                            break;
                        case OTP_CALIB_DELAY_CALIB:
                            UWB_STREAM_TO_UINT16(pCalibPayload->DELAY_CALIB_VALUE, pReadCalibDataRsp);
                            break;
#if UWBIOT_UWBD_SR1XXT
                        case OTP_PDOA_MFG_ZERO_OFFSET_CALIB:
                            phOsalUwb_MemCopy(pCalibPayload->PDOA_MFG_0_OFFSET_CALIB,
                                uwbContext.rsp_data,
                                sizeof(pCalibPayload->PDOA_MFG_0_OFFSET_CALIB));
                            break;
                        case OTP_AOA_ANT_MULTIPOINT_CALIB:
                            phOsalUwb_MemCopy(pCalibPayload->AOA_ANT_MULTIPOINT_CALIB,
                                uwbContext.rsp_data,
                                sizeof(pCalibPayload->AOA_ANT_MULTIPOINT_CALIB));
                            break;
#endif //UWBIOT_UWBD_SR1XXT
                        default:
                            break;
                        }
                    }
                    else {
                        NXPLOG_UWBAPI_E("%s: Read Calibration Data Notification failed", __FUNCTION__);
                        break;
                    }
                }
                else {
                    NXPLOG_UWBAPI_E("%s: Read Calibration Data Notification Timed out", __FUNCTION__);
                    break;
                }
            }
            else if (status == UWBAPI_STATUS_TIMEOUT) {
                NXPLOG_UWBAPI_E("%s: Read Calibration Data Command Timed Out", __FUNCTION__);
                break;
            }
            else {
                NXPLOG_UWBAPI_E("%s: Read Calibration Data Command failed", __FUNCTION__);
                break;
            }
        }
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
/**
 * Read the Module Maker Info from OTP.
 * This api can be used with both Factory and Mainline Firmware
 * \param[in]      paramType   parameter to write into otp See :cpp:type:`eOtpParam_Type_t`
 * \param[in,out]  infoLength  Size of pInfo buffer, number of bytes expected
 * \param[out]     pInfo       Module Maker Info, the size of this buffer shall be equal to infoLength
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if the operation timed out
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_ReadOtpCmd(eOtpParam_Type_t paramType, uint8_t *pInfo, uint8_t *infoLength)
{
    tUWBAPI_STATUS status;
    uint8_t *pDataCmd   = NULL;
    uint8_t receivedLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: Output buffer is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    pDataCmd = uwbContext.snd_data;
    switch (paramType) {
    case kUWB_OTP_ModuleMakerInfo:
        UCI_MSG_BLD_HDR0(pDataCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
        UCI_MSG_BLD_HDR1(pDataCmd, EXT_UCI_MSG_READ_MODULE_MAKER_ID);
        break;
    default:
        NXPLOG_UWBAPI_E("%s: paramType is wrong", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    UWB_UINT8_TO_STREAM(pDataCmd, 0x00);
    UWB_UINT8_TO_STREAM(pDataCmd, 0x00);
    status = sendRawUci(uwbContext.snd_data, (uint16_t)UCI_MSG_HDR_SIZE);
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Read Otp command successful", __FUNCTION__);
        receivedLen = uwbContext.rsp_data[UCI_RESPONSE_LEN_OFFSET] - 1; // exclude 1 byte status
        if (receivedLen < MODULE_MAKER_ID_MAX_SIZE) {
            NXPLOG_UWBAPI_E("%s: Received wrong data ", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
        else if (*infoLength < MODULE_MAKER_ID_MAX_SIZE) {
            NXPLOG_UWBAPI_E("%s: Not enough buffer to store information", __FUNCTION__);
            status = UWBAPI_STATUS_BUFFER_OVERFLOW;
        }
        else {
            /* Work around in MW to support Module Maker ID size of 2 bytes
               Currently FW sends 8 bytes, change this when FW changes */
            *infoLength = MODULE_MAKER_ID_MAX_SIZE;
            phOsalUwb_MemCopy(pInfo, &uwbContext.rsp_data[UCI_RESPONSE_PAYLOAD_OFFSET], MODULE_MAKER_ID_MAX_SIZE);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Read Otp Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Read Otp Command failed", __FUNCTION__);
    }
    return status;
}

#if 0
/**
 * \brief Write Bitmask value to OTP.
 *
 * \param[in] bitMask   bit mask for calib params
 *
 * \retval #UWBAPI_STATUS_OK  on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT  if command is timeout
 * \retval #UWBAPI_STATUS_FAILED  otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_WriteOtpBitMask(uint16_t bitMask)
{
    tUWBAPI_STATUS status;
    uint8_t payloadLen = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(bitMask);
    uint8_t* pSetBitMaskCmd = NULL;
    otpRWOption otpWriteOption = BIT_MASK_VALUE;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    pSetBitMaskCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pSetBitMaskCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pSetBitMaskCmd, EXT_UCI_MSG_WRITE_CALIB_DATA_CMD);
    UWB_UINT8_TO_STREAM(pSetBitMaskCmd, 0x00);
    UWB_UINT8_TO_STREAM(pSetBitMaskCmd, payloadLen);
    UWB_UINT8_TO_STREAM(pSetBitMaskCmd, 0x00); /* Channel ID is ignored */
    UWB_UINT8_TO_STREAM(pSetBitMaskCmd, (uint8_t)otpWriteOption);
    UWB_UINT16_TO_STREAM(pSetBitMaskCmd, sizeof(bitMask));
    UWB_UINT16_TO_STREAM(pSetBitMaskCmd, bitMask);

    status = sendRawUci(uwbContext.snd_data, (uint16_t)(payloadLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Write BitMask command successful", __FUNCTION__);
        status = waitforNotification(EXT_UCI_MSG_WRITE_CALIB_DATA_CMD, UWBD_CALIB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            status = uwbContext.wstatus;
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: Write BitMask Notification successfull", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Write BitMask Notification failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Write BitMask Notification Timed out", __FUNCTION__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Write BitMask Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Write BitMask Command failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Read Bitmask value from OTP.
 *
 * \param[out] bitMask   bit mask for calib params
 *
 * \retval #UWBAPI_STATUS_OK  on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT  if command is timeout
 * \retval #UWBAPI_STATUS_FAILED  otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_ReadOtpBitMask(uint16_t bitMask)
{
    tUWBAPI_STATUS status;
    uint8_t payloadLen = sizeof(uint8_t) + sizeof(uint8_t);
    uint8_t* pGetBitMaskCmd = NULL;
    uint8_t* pBitMaskRsp = NULL;
    otpRWOption otpReadOption = BIT_MASK_VALUE;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    pGetBitMaskCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pGetBitMaskCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pGetBitMaskCmd, EXT_UCI_MSG_READ_CALIB_DATA_CMD);
    UWB_UINT8_TO_STREAM(pGetBitMaskCmd, 0x00);
    UWB_UINT8_TO_STREAM(pGetBitMaskCmd, payloadLen);
    UWB_UINT8_TO_STREAM(pGetBitMaskCmd, 0x00); /* Channel ID is ignored */
    UWB_UINT8_TO_STREAM(pGetBitMaskCmd, (uint8_t)otpReadOption);

    status = sendRawUci(uwbContext.snd_data, (uint16_t)(payloadLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Read BitMask command successful", __FUNCTION__);
        status = waitforNotification(EXT_UCI_MSG_READ_CALIB_DATA_CMD, UWBD_CALIB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            status = uwbContext.wstatus;
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: Read BitMask Notification successfull", __FUNCTION__);
                pBitMaskRsp = uwbContext.rsp_data;
                UWB_STREAM_TO_UINT16(bitMask, pBitMaskRsp);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Read BitMask Notification failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Read BitMask Notification Timed out", __FUNCTION__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Read BitMask Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Read BitMask Command failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Write CMAC tag value to OTP.
 *
 * \param[in] pCmacTag   CMAC tag value, must be 16bytes.
 *
 * \retval #UWBAPI_STATUS_OK  on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameter is passed
 * \retval #UWBAPI_STATUS_TIMEOUT  if command is timeout
 * \retval #UWBAPI_STATUS_FAILED  otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_WriteOtpCmacTag(uint8_t* pCmacTag)
{
    tUWBAPI_STATUS status;
    uint8_t payloadLen = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + UWB_TAG_CMAC_LENGTH;
    uint8_t* pSetCmacTagCmd = NULL;
    otpRWOption otpWriteOption = CMAC_TAG;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pCmacTag == NULL) {
        NXPLOG_UWBAPI_E("%s: CMAC Tag is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pSetCmacTagCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pSetCmacTagCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pSetCmacTagCmd, EXT_UCI_MSG_WRITE_CALIB_DATA_CMD);
    UWB_UINT8_TO_STREAM(pSetCmacTagCmd, 0x00);
    UWB_UINT8_TO_STREAM(pSetCmacTagCmd, payloadLen);
    UWB_UINT8_TO_STREAM(pSetCmacTagCmd, 0x00); /* Channel ID is ignored */
    UWB_UINT8_TO_STREAM(pSetCmacTagCmd, (uint8_t)otpWriteOption);
    UWB_UINT16_TO_STREAM(pSetCmacTagCmd, (uint16_t)UWB_TAG_CMAC_LENGTH);
    UWB_ARRAY_TO_STREAM(pSetCmacTagCmd, pCmacTag, UWB_TAG_CMAC_LENGTH);

    status = sendRawUci(uwbContext.snd_data, (uint16_t)(payloadLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Write Cmac Tag command successful", __FUNCTION__);
        status = waitforNotification(EXT_UCI_MSG_WRITE_CALIB_DATA_CMD, UWBD_CALIB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            status = uwbContext.wstatus;
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: Write Cmac Tag Notification successfull", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Write Cmac Tag Notification failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Write Cmac Tag Notification Timed out", __FUNCTION__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Write Cmac Tag Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Write Cmac Tag Command failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160

/**
 * \brief Get the binding count using this API. This API shall be used with
 * Factory mode FW only.
 *
 * \param[out] getBindingCount      getBindingCount data. valid only if API
 * status is success
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetBindingCount(phSeGetBindingCount_t *getBindingCount)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (getBindingCount == NULL) {
        NXPLOG_UWBAPI_E("%s: getBindingCount is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_GET_BINDING_COUNT_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_GET_BINDING_COUNT, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Binding Count is successful", __FUNCTION__);
        /* rsp_data contains complete rsp, we have to skip Header */
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        UWB_STREAM_TO_UINT8(status, rspPtr);
        if (status == UWBAPI_STATUS_OK) {
            UWB_STREAM_TO_UINT8(getBindingCount->bindingStatus, rspPtr);
            UWB_STREAM_TO_UINT8(getBindingCount->uwbdBindingCount, rspPtr);
            UWB_STREAM_TO_UINT8(getBindingCount->seBindingCount, rspPtr);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get Binding Count Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Binding Count is failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#if (UWBFTR_SE_SN110)
/**
 * \brief TestConnectivity Perform SE connectivity test using this API
 *
 * \param[out] ConnectivityStatus    : Structure containing for the result of SUS AID selection and the status shall indicate the success or failures,
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_TestConnectivity(SeConnectivityStatus_t *ConnectivityStatus)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (ConnectivityStatus == NULL) {
        NXPLOG_UWBAPI_E("%s: ConnectivityStatus is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_TEST_CONNECTIVITY_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_TEST_CONNECTIVITY, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(VENDOR_UCI_MSG_SE_DO_TEST_CONNECTIVITY, UWB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(ConnectivityStatus->status, p);
            if (ConnectivityStatus->status != UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_FAILED;
                NXPLOG_UWBAPI_W(
                    "%s: Get ConnectivityStatus is not success %0x", __FUNCTION__, ConnectivityStatus->status);
            }
            UWB_STREAM_TO_UINT16(ConnectivityStatus->se_instruction_code, p);
            UWB_STREAM_TO_UINT16(ConnectivityStatus->se_error_status, p);
            NXPLOG_UWBAPI_D("%s: Get ESE Test cmd passed", __FUNCTION__);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Get  ESE Test cmd failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Connectivity test Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Connectivity test response is failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief TestLoop Perform SE loop test using this API
 *
 * \param[in] loopCnt         No of times test to be run
 * \param[in] timeInterval    time interval in ms
 * \param[out] testLoopData    Test loop notification data
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SeTestLoop(uint16_t loopCnt, uint16_t timeInterval, phTestLoopData_t *testLoopData)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (testLoopData == NULL) {
        NXPLOG_UWBAPI_E("%s: testLoopData is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    sep_SetWaitEvent(UWA_DM_PROP_SE_TEST_LOOP_RESP_EVT);
    cmdLen = serializeSeLoopTestPayload(loopCnt, timeInterval, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_TEST_SE_LOOP, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(
            EXT_UCI_MSG_SE_DO_TEST_LOOP, (uint32_t)((uint32_t)(loopCnt * timeInterval) + UWBD_SE_TIMEOUT));
        /*
     * Increasing the delay as it is not sufficient to get the notification.
     */
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(testLoopData->status, p);
            if (testLoopData->status == UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_OK;
                UWB_STREAM_TO_UINT16(testLoopData->loop_cnt, p);
                UWB_STREAM_TO_UINT16(testLoopData->loop_pass_count, p);
                NXPLOG_UWBAPI_D("%s: Loop test is successful", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Loop test is failed", __FUNCTION__);
                status = UWBAPI_STATUS_FAILED;
            }
        }
        else {
            testLoopData->status = 0xFF;
            NXPLOG_UWBAPI_E("%s: Loop test is failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Loop test Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Loop test is failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief API to get current binding status
 * Use of this API will lock binding status if UWBD is in unlock state
 * \param[in]  getBindingStatus    Binding status notification
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetBindingStatus(phSeGetBindingStatus_t *getBindingStatus)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (getBindingStatus == NULL) {
        NXPLOG_UWBAPI_E("%s: getBindingStatus is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_GET_BINDING_STATUS_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_GET_BINDING_STATUS, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(VENDOR_UCI_MSG_ESE_BINDING_CHECK_CMD, UWBD_SE_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(getBindingStatus->status, p);
            if (getBindingStatus->status != 0x02) {
                status = UWBAPI_STATUS_FAILED;
                NXPLOG_UWBAPI_W(
                    "Get binding status is NOT bound and Unlocked , status is %0x", getBindingStatus->status);
            }
            UWB_STREAM_TO_UINT8(getBindingStatus->se_binding_count, p);
            UWB_STREAM_TO_UINT8(getBindingStatus->uwbd_binding_count, p);
            UWB_STREAM_TO_UINT16(getBindingStatus->se_instruction_code, p);
            UWB_STREAM_TO_UINT16(getBindingStatus->se_error_status, p);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Get binding status cmd failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get binding status Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get binding status cmd failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Delete RDS/URSK entries in SUS applet using this API
 *  Before calling the api user must allocate the memory for
 *  sessionHandleList and phUrskDeletionRequestStatus_t
 * \param[in] noOfSessionHandle         No of sessionHandles to be removed
 * \param[in] sessionHandleList         List of sessionHandles to be removed
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_URSKdeletionRequest(
    uint8_t noOfSessionHandle, uint32_t *sessionHandleList, phUrskDeletionRequestStatus_t *urskDeletionStatus)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (sessionHandleList == NULL) {
        NXPLOG_UWBAPI_E("%s: sessionHandleList is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_URSK_DELETION_REQUEST_RESP_EVT);
    cmdLen = serializeUrskDeletionRequestPayload(noOfSessionHandle, sessionHandleList, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_URSK_DELETION_REQUEST, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(VENDOR_UCI_MSG_URSK_DELETION_REQ, UWB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(urskDeletionStatus->status, p);
            if (urskDeletionStatus->status == UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_OK;
                UWB_STREAM_TO_UINT8(urskDeletionStatus->noOfSessionHandles, p);
                for (int i = 0; i < urskDeletionStatus->noOfSessionHandles; i++) {
                    UWB_STREAM_TO_UINT32(urskDeletionStatus->sessionHandleList[i].sessionHandle, p);
                    UWB_STREAM_TO_UINT8(urskDeletionStatus->sessionHandleList[i].status, p);
                }
            }
            else {
                urskDeletionStatus->status = 0xFF;
                NXPLOG_UWBAPI_E("%s: URSK deletion request ntf is failed", __FUNCTION__);
                status = UWBAPI_STATUS_FAILED;
            }
        }
        else {
            urskDeletionStatus->status = 0xFF;
            NXPLOG_UWBAPI_E("%s: URSK deletion request ntf not received", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: URSK deletion request Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: URSK deletion request is failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //(UWBFTR_SE_SN110)

#if !(UWBIOT_UWBD_SR040)
/**
 * \brief API to get the current temperature
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_QueryTemperature(uint8_t *pTemperatureValue)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pTemperatureValue == NULL) {
        NXPLOG_UWBAPI_E("%s: pTemperatureValue is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_QUERY_TEMPERATURE_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_QUERY_TEMP, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Query temperature cmd successful", __FUNCTION__);
        /* rsp_data contains complete rsp, we have to skip Header */
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        UWB_STREAM_TO_UINT8(status, rspPtr);

        if (status == UWBAPI_STATUS_OK) {
            UWB_STREAM_TO_UINT8(*pTemperatureValue, rspPtr);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Query temperature cmd Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Query temperature cmd failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //!(UWBIOT_UWBD_SR040)

#if (UWBIOT_UWBD_SR100T)
/**
 * \brief Calibration Integrity Protection for all the Calibration Parameters.
 *
 * \param[in] tagOption      Tag Option indicating Device/Model Specific tag
 * \arg 0x00 indicates Device Specific tag option
 * \arg 0x01 indicates Model Specific tag option
 * \param[in] calibBitMask   bit mask for calibration parameters.
 * Following bits to be set for corresponding calibration parameters to enable integrity protection.
 * \arg bit0 - VCO_PLL
 * \arg bit1 - TX_POWER
 * \arg bit2 - 38.4MHz_XTAL_CAP
 * \arg bit3 - RSSI_CALIB_CONSTANT1
 * \arg bit4 - RSSI_CALIB_CONSTANT2
 * \arg bit5 - SNR_CALIB_CONSTANT
 * \arg bit6 - MANUAL_TX_POW_CTRL
 * \arg bit7 - PDOA_OFFSET
 * \arg bit8 - PA_PPA_CALIB_CTRL
 * \arg bit9 - TX_TEMPERATURE_COMP
 * \arg bit10- AOA_FINE_CALIB_PARAM
 * \arg bit11- DELAY_CALIB
 * \arg bit12- AOA_CALIB_CTRL
 * \arg bit13- RFU
 * \arg bit14- RFU
 * \arg bit15- RFU
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS UwbApi_CalibrationIntegrityProtection(eCalibTagOption tagOption, uint16_t calibBitMask)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_PROP_CALIB_INTEGRITY_PROTECTION_RESP_EVT);
    cmdLen = serializecalibIntegrityProtectionPayload(tagOption, calibBitMask, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_CALIB_INTEGRITY_PROTECTION, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Calibration Integrity Protection command successful", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Calibration Integrity Protection Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Calibration Integrity Protection Command failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Verify Calibration data for all the set Calibration Parameters.
 *
 * \param[in] pCmacTagResp         Cmac Tag
 * \param[in] tagOption            Tag Option indicating Device/Model Specific tag
 * \param[in] tagVersion           Tag Version only for Model Specific Tag verification process.
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS UwbApi_VerifyCalibData(uint8_t *pCmacTag, uint8_t tagOption, uint16_t tagVersion)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pCmacTag == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_VERIFY_CALIB_DATA_RESP_EVT);
    cmdLen = serializeVerifyCalibDataPayload(pCmacTag, tagOption, tagVersion, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_VERIFY_CALIB_DATA, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(EXT_UCI_MSG_VERIFY_CALIB_DATA, UWBD_GENERATE_TAG_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            status = uwbContext.wstatus;
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: Verify Calib Data successful", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Verify Calib Data failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Verify Calib Data notification time out", __FUNCTION__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Verify Calib Data Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Verify Calib Data Command failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //UWBIOT_UWBD_SR100T
/**
 * \brief API to get the UWB Timestamp for UWB time synchronization
 *
 * \param[in] len                 Length of i/p buffer. It should be 8 to hold 8 bytes timestamp value
 * \param[out] pTimestampValue    Timestamp data. It should be 8 bytes in size to hold 8 bytes timestamp value.
 * \On successful execution, this buffer will contain 8 bytes timestamp value.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW  if response length is more than expected response size
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_QueryUwbTimestamp(uint8_t len, uint8_t pTimestampValue[])
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((len < 8) || (pTimestampValue == NULL)) {
        NXPLOG_UWBAPI_E("%s: pTimestampValue is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_QUERY_TIMESTAMP_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_CORE_QUERY_UWBS_TIMESTAMP, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Query timestamp cmd successful", __FUNCTION__);
        /* rsp_data contains complete rsp, we have to skip Header */
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        UWB_STREAM_TO_UINT8(status, rspPtr);

        if (status == UWBAPI_STATUS_OK) {
            if (uwbContext.rsp_len > (UCI_MSG_CORE_UWBS_TIMESTAMP_LEN + UCI_RESPONSE_PAYLOAD_OFFSET)) {
                NXPLOG_UWBAPI_E("%s: Response data size is more than response buffer", __FUNCTION__);
                return UWBAPI_STATUS_BUFFER_OVERFLOW;
            }
            UWB_STREAM_TO_ARRAY(pTimestampValue, rspPtr, UCI_MSG_CORE_UWBS_TIMESTAMP_LEN);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Query timestamp cmd Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Query timestamp cmd failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#if UWBFTR_DL_TDoA_Anchor
/**
 * \brief Update the active rounds during the DL-TDoA Session for a initiator or responder device.
 *
 * \param[in]   sessionHandle      : Unique Session Handle
 * \param[in]   nActiveRounds      : Number of active rounds
 * \param[in]   macAddressingMode  : MAC addressing mode- 2/8 bytes
 * \param[in]   roundConfigList    : List/array of size nActiveRounds of round index + role tuple
 * \param[out]  pNotActivatedRound : Structure containing list of not activated index which couldn't be activated, in case return code is #UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED
 *
 * \retval #UWBAPI_STATUS_OK                                               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED                                  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST                                if session is not initialized with sessionHandle
 * \retval #UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED                  if one or more rounds couldn't be activated
 * \retval #UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED   one or more given rounds exceed number of rounds available
 * \retval #UWBAPI_STATUS_TIMEOUT                                          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED                                           otherwise
 */
tUWBAPI_STATUS UwbApi_UpdateActiveRoundsAnchor(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    UWB_MacAddressMode_t macAddressingMode,
    const phActiveRoundsConfig_t roundConfigList[],
    phNotActivatedRounds_t *pNotActivatedRound)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    uint16_t cmdLen = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((!nActiveRounds) || (roundConfigList == NULL)) {
        NXPLOG_UWBAPI_E("%s: nActiveRounds is 0 or roundConfigList is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pNotActivatedRound == NULL) {
        NXPLOG_UWBAPI_E("%s: pNotActivatedRound is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_RSP_EVT);
    cmdLen = serializeUpdateActiveRoundsAnchorPayload(
        sessionHandle, nActiveRounds, macAddressingMode, roundConfigList, &uwbContext.snd_data[0]);
    if (cmdLen != 0) {
        status =
            sendUciCommandAndWait(UWA_DM_API_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_EVT, cmdLen, uwbContext.snd_data);
    }
    else {
        NXPLOG_UWBAPI_E("%s: responderSlots or dstMacAddr is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Update active rounds successful!", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED) {
        NXPLOG_UWBAPI_D("%s: One or more rounds couldn't be activated", __FUNCTION__);
        uint8_t *rspPtr               = &uwbContext.rsp_data[0];
        pNotActivatedRound->noOfIndex = (uint8_t)(uwbContext.rsp_len - sizeof(status));
        for (int i = 0; i < pNotActivatedRound->noOfIndex; i++) {
            pNotActivatedRound->indexList[i] = *rspPtr++;
        }
    }
    else if (status == UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED) {
        NXPLOG_UWBAPI_E("%s: Number of active rounds exceeded", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Update active rounds cmd failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //UWBFTR_DL_TDoA_Anchor

#if UWBFTR_DL_TDoA_Tag
/**
 * \brief Update the active rounds during the DL-TDoA Session for a receiver device.
 *
 * \param[in]  sessionHandle         : Unique Session Handle
 * \param[in]  nActiveRounds         : Number of active rounds
 * \param[in]  RangingroundIndexList : List/array of size nActiveRounds of round index
 * \param[out] pNotActivatedRound    : Structure containing list of not activated index which couldn't be activated, in case return code is #UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED
 *
 * \retval #UWBAPI_STATUS_OK                                               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED                                  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST                                if session is not initialized with sessionHandle
 * \retval #UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED                  if one or more rounds couldn't be activated
 * \retval #UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED   one or more given rounds exceed number of rounds available
 * \retval #UWBAPI_STATUS_TIMEOUT                                          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED                                           otherwise
 */
tUWBAPI_STATUS UwbApi_UpdateActiveRoundsReceiver(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    const uint8_t RangingroundIndexList[],
    phNotActivatedRounds_t *pNotActivatedRound)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    uint16_t cmdLen = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((!nActiveRounds) || (RangingroundIndexList == NULL)) {
        NXPLOG_UWBAPI_E("%s: nActiveRounds is 0 or roundConfigList is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pNotActivatedRound == NULL) {
        NXPLOG_UWBAPI_E("%s: pNotActivatedRound is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_RSP_EVT);
    cmdLen = serializeUpdateActiveRoundsReceiverPayload(
        sessionHandle, nActiveRounds, RangingroundIndexList, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Update active rounds successful!", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED) {
        NXPLOG_UWBAPI_D("%s: One or more rounds couldn't be activated", __FUNCTION__);
        uint8_t *rspPtr               = &uwbContext.rsp_data[0];
        pNotActivatedRound->noOfIndex = (uint8_t)(uwbContext.rsp_len - sizeof(status));
        for (int i = 0; i < pNotActivatedRound->noOfIndex; i++) {
            pNotActivatedRound->indexList[i] = *rspPtr++;
        }
    }
    else if (status == UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED) {
        NXPLOG_UWBAPI_E("%s: Number of active rounds exceeded", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Update active rounds cmd failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //UWBFTR_DL_TDoA_Tag

tUWBAPI_STATUS UwbApi_GetFwCrashLog(phFwCrashLogInfo_t *pLogInfo)
{
    tHAL_UWB_IOCTL ioCtl;
    const tHAL_UWB_ENTRY *halFuncEntries = NULL;

    if (pLogInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: pLogInfo is null ", __FUNCTION__);
        return UWBAPI_STATUS_FAILED;
    }
    if (pLogInfo->pLog == NULL) {
        NXPLOG_UWBAPI_E("%s: buffer is null ", __FUNCTION__);
        return UWBAPI_STATUS_FAILED;
    }

    halFuncEntries   = GetHalEntryFuncs();
    ioCtl.pCrashInfo = pLogInfo;
    if ((halFuncEntries->ioctl(HAL_UWB_IOCTL_DUMP_FW_CRASH_LOG, &ioCtl)) == UWBSTATUS_SUCCESS) {
        return UWBAPI_STATUS_OK;
    }
    else {
        return UWBAPI_STATUS_FAILED;
    }
}

tUWBAPI_STATUS UwbApi_SetDefaultCoreConfigs()
{
    return setDefaultCoreConfigs();
}

#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160

/**
 * \brief Prepare sharable Configuration Data.
 * \param[in]      pShareableData           : sharable data which contain all information.
 * \param[in]      ShareableDataLength      : Size of sharable data
 * \param[in,out]  pProfileInfo             : contains profile information.
 * \param[in]      noOfVendorAppParams      : number of VendorAppParams.
 * \param[in]      VendorAppParams_List     : List of VendorAppParams to be set.
 * \param[in]      noOfDebugParams          : number of DebugParams.
 * \param[in]      DebugParams_List         : List of Debug params to be set.
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
tUWBAPI_STATUS UwbApi_ConfigureData_iOS(uint8_t *pShareableData,
    uint16_t ShareableDataLength,
    phUwbProfileInfo_t *pProfileInfo,
    uint8_t noOfVendorAppParams,
    const UWB_VendorAppParams_List_t *VendorAppParams_List,
    uint8_t noOfDebugParams,
    const UWB_DebugParams_List_t *DebugParams_List)
{
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    tUWBAPI_STATUS status;

    if (pProfileInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: pProfileInfo is invalid", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }
    if (pShareableData == NULL) {
        NXPLOG_UWBAPI_E("%s: pShareableData is invalid", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    if ((ShareableDataLength != TOTAL_PROFILE_BLOB_SIZE_v1_1) &&
        (ShareableDataLength != TOTAL_PROFILE_BLOB_SIZE_v1_0)) {
        NXPLOG_UWBAPI_E("%s: profile blob size should be %d or %d bytes",
            __FUNCTION__,
            TOTAL_PROFILE_BLOB_SIZE_v1_0,
            TOTAL_PROFILE_BLOB_SIZE_v1_1);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    pProfileInfo->profileId = kUWB_Profile_1;

    status = UwbApi_SetProfileParams(pShareableData, ShareableDataLength, pProfileInfo);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_SetProfileParams failed");
        goto exit;
    }

    if ((noOfVendorAppParams != 0) && (VendorAppParams_List != NULL)) {
        status = UwbApi_SetVendorAppConfigs(pProfileInfo->sessionHandle, noOfVendorAppParams, VendorAppParams_List);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SetVendorAppConfigs failed");
            goto exit;
        }
    }
    if ((noOfDebugParams != 0) && (DebugParams_List != NULL)) {
        status = UwbApi_SetDebugParams(pProfileInfo->sessionHandle, noOfDebugParams, DebugParams_List);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SetDebugParams failed");
            goto exit;
        }
    }

    status = UwbApi_StartRangingSession(pProfileInfo->sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }
exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Set phone uwb configuration data.
 * \param[in]      pUwbPhoneConfigData      : UwbPhoneConfigData_t data which contain all information.
 * \param[in]      UwbPhoneConfigDataLen    : Size of phone configuration data
 * \param[in,out]  pProfileInfo             : contains profile information
 * \param[in]      noOfVendorAppParams      : number of VendorAppParams.
 * \param[in]      VendorAppParams_List     : List of VendorAppParams to be set.
 * \param[in]      noOfDebugParams          : number of DebugParams.
 * \param[in]      DebugParams_List         : List of Debug params to be set.
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 *
 * \note There are 3 configurations supported namely CONFIG_ID_1, CONFIG_ID_2 and CONFIG_ID_3. The API sets/configures the respective configuration based on the value of config_id from Android jetpack.
 */
tUWBAPI_STATUS UwbApi_ConfigureData_Android(uint8_t *pUwbPhoneConfigData,
    uint16_t UwbPhoneConfigDataLen,
    phUwbProfileInfo_t *pProfileInfo,
    uint8_t noOfVendorAppParams,
    const UWB_VendorAppParams_List_t *VendorAppParams_List,
    uint8_t noOfDebugParams,
    const UWB_DebugParams_List_t *DebugParams_List)
{
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    tUWBAPI_STATUS status;
    uint32_t sessionHandle = 0;

    const uint8_t stsStatic[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    const uint8_t vendorId[]  = {0x08, 0x07};

    phRangingParams_t inRangingParams = {0};
    UwbPhoneConfigData_t UwbPhoneConfig;
    UWB_AppParams_List_t SetAppParamsList[MAX_CONFIG_ID_NUM_CONFIGS];

    if (pUwbPhoneConfigData == NULL || pProfileInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: Phone Config Data or profile info is invalid", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    if (UwbPhoneConfigDataLen == SHAREABLE_DATA_HEADER_LENGTH_ANDROID) {
        serializeUwbPhoneConfigData(&UwbPhoneConfig, pUwbPhoneConfigData);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Phone Config Data length is invalid : %d", __FUNCTION__, UwbPhoneConfigDataLen);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    status = UwbApi_SessionInit(UwbPhoneConfig.session_id, UWBD_RANGING_SESSION, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    pProfileInfo->sessionHandle = sessionHandle;
    LOG_W("CONFIG_ID_%d", UwbPhoneConfig.config_id);
    switch (UwbPhoneConfig.config_id) {
    case UWB_CONFIG_ID_1: {
        /**
         * +----------------------+------------------------------------+
         * | UWB Params           | CONFIG_ID_1 (0x01)                 |
         * +======================+====================================+
         * | Multinode Mode       | Unicast                            |
         * | STS                  | Static                             |
         * | Ranging Method       | DS-TWR Deferred mode               |
         * | Ranging Interval     | 240ms                              |
         * | AOA_RESULT_REQ       | 0x01 (Enabled)                     |
         * | Slot Duration        | 2400                               |
         * | Slots per RR         | 10                                 |
         * | STS IV               | 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 |
         * | Vendor ID            | 0x08, 0x07                         |
         * | Number of Controlees | 1                                  |
         * +----------------------+------------------------------------+
         */
        inRangingParams.multiNodeMode = kUWB_MultiNodeMode_UniCast;

        SetAppParamsList[0] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 240);
        SetAppParamsList[1] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, ENABLED);
        SetAppParamsList[2] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10);

    } break;

    case UWB_CONFIG_ID_2: {
        /**
         * +----------------------+------------------------------------+
         * | UWB Params           | CONFIG_ID_2 (0x02)                 |
         * +======================+====================================+
         * | Multinode Mode       | One to many                        |
         * | STS                  | Static                             |
         * | Ranging Method       | DS-TWR Deferred mode               |
         * | Ranging Interval     | 200ms                              |
         * | AOA_RESULT_REQ       | 0x01 (Enabled)                     |
         * | Slot Duration        | 2400                               |
         * | Slots per RR         | 10                                 |
         * | STS IV               | 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 |
         * | Vendor ID            | 0x08, 0x07                         |
         * | Number of Controlees | 1                                  |
         * +----------------------+------------------------------------+
         */
        inRangingParams.multiNodeMode = kUWB_MultiNodeMode_OnetoMany;

        SetAppParamsList[0] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 200);
        SetAppParamsList[1] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, ENABLED);
        SetAppParamsList[2] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10);

    } break;

    case UWB_CONFIG_ID_3: {
        /**
         * +----------------------+------------------------------------+
         * | UWB Params           | CONFIG_ID_3(0x03)                  |
         * +======================+====================================+
         * | Multinode Mode       | Unicast                            |
         * | STS                  | Static                             |
         * | Ranging Method       | DS-TWR Deferred mode               |
         * | Ranging Interval     | 240ms                              |
         * | AOA_RESULT_REQ       | 0x00 (Disabled)                    |
         * | Slot Duration        | 2400                               |
         * | Slots per RR         | 10                                 |
         * | STS IV               | 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 |
         * | Vendor ID            | 0x08, 0x07                         |
         * | Number of Controlees | 1                                  |
         * +----------------------+------------------------------------+
         */
        inRangingParams.multiNodeMode = kUWB_MultiNodeMode_UniCast;

        SetAppParamsList[0] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 240);
        SetAppParamsList[1] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, DISABLED);
        SetAppParamsList[2] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10);

    } break;

    default: {
        NXPLOG_UWBAPI_E("Profile ID not supported");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    } break;
    }

    /* Common configs across config IDs 1, 2 and 3*/
    SetAppParamsList[3] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 2400);
    SetAppParamsList[4] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_ARRAY(STATIC_STS_IV, &stsStatic[0], sizeof(stsStatic)); // Android shows [1, 2, 3, 4, 5, 6]
    SetAppParamsList[5] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_ARRAY(VENDOR_ID, &vendorId[0], sizeof(vendorId)); // Android shows [7, 8]
    SetAppParamsList[6] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, UwbPhoneConfig.preamble_id);
    SetAppParamsList[7] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, UwbPhoneConfig.channel_number);
    SetAppParamsList[8] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, 1);
    SetAppParamsList[9] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, UwbPhoneConfig.phone_mac_address, MAC_SHORT_ADD_LEN);


    /* Set the Config_Id Parameters */
    status = UwbApi_SetAppConfigMultipleParams(
        pProfileInfo->sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    switch (UwbPhoneConfig.device_ranging_role) {
    case UWB_DEVICE_CONTROLLER: {
        inRangingParams.deviceRole = kUWB_DeviceRole_Initiator;
        inRangingParams.deviceType = kUWB_DeviceType_Controller;
    } break;

    case UWB_DEVICE_CONTROLEE: {
        inRangingParams.deviceRole = kUWB_DeviceRole_Responder;
        inRangingParams.deviceType = kUWB_DeviceType_Controlee;
    } break;

    default: {
        NXPLOG_UWBAPI_E("Role not supported");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    } break;
    }

    inRangingParams.deviceMacAddr[0] = pProfileInfo->mac_addr[0];
    inRangingParams.deviceMacAddr[1] = pProfileInfo->mac_addr[1];

    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    status = UwbApi_SetRangingParams(pProfileInfo->sessionHandle, &inRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    if ((noOfVendorAppParams != 0) && (VendorAppParams_List != NULL)) {
        status = UwbApi_SetVendorAppConfigs(pProfileInfo->sessionHandle, noOfVendorAppParams, VendorAppParams_List);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SetVendorAppConfigs failed");
            goto exit;
        }
    }

    if ((noOfDebugParams != 0) && (DebugParams_List != NULL)) {
        status = UwbApi_SetDebugParams(pProfileInfo->sessionHandle, noOfDebugParams, DebugParams_List);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SetDebugParams failed");
            goto exit;
        }
    }

    status = UwbApi_StartRangingSession(pProfileInfo->sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }
exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160

#if UWBIOT_UWBD_SR150
tUWBAPI_STATUS UwbApi_SessionSetLocZone(phSessionSetLocZone_t *pSetLocZone)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    uint16_t cmdLen = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        status = UWBAPI_STATUS_NOT_INITIALIZED;
        goto exit;
    }

    if (NULL == pSetLocZone) {
        NXPLOG_UWBAPI_E("%s: pSetLocZone is NULL", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_LOCALIZATION_ZONE_RSP_EVENT);
    cmdLen = serializeSessionSetLocZoneCmd(pSetLocZone, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_LOCALIZATION_ZONE_EVT, cmdLen, uwbContext.snd_data);

    if (UWBAPI_STATUS_OK != status) {
        NXPLOG_UWBAPI_E("%s: Session Set Localization Zone Failed!", __FUNCTION__);
        goto exit;
    }

exit:
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#endif // UWBIOT_UWBD_SR150