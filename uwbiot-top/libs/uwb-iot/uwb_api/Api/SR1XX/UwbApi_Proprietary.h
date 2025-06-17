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

#ifndef UWBAPI_PROPRIETARY_SR1XX_H
#define UWBAPI_PROPRIETARY_SR1XX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "phUwbTypes.h"
#include <UwbApi_Types_Proprietary.h>
#include "uwb_hal_api.h"

#if (UWBFTR_SE_SE051W)
#include "SE_Wrapper.h"
#include "UWB_Wrapper.h"
#endif

/** \addtogroup uwb_apis_sr1xx
 *
 * APIs for SR100 and SR150
 *
 * @{ */

/**
 *  \brief APIs exposed to application to access UWB Baord Specific Functionality
 */

/**
 * \brief Get Firmware Crash Log
 *
 * \param[out] pLogInfo   Pointer to \ref phFwCrashLogInfo_t
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetFwCrashLog(phFwCrashLogInfo_t *pLogInfo);

/**
 * \brief Set Default Core configs
 *
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetDefaultCoreConfigs();

/**
 * \brief Do calibration parameters.
 *
 * \param[in] channel    Channel
 * \param[out] calibResp   Pointer to \ref phCalibRespStatus_t
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_DoVcoPllCalibration(uint8_t channel, phCalibRespStatus_t *calibResp);

/* Deprecated APIs
 * TODO: Remove in future releases
 */
#define UwbApi_DoCalibration(channel, paramId, calibResp) \
    UwbApi_DoVcoPllCalibration(channel, calibResp);       \
    (void)(paramId);

/**
 * \brief Set calibration parameters.
 *
 * \param[in] channel       channel
 * \param[in] paramId       Calibration parameter ID
 * \param[in] calibrationValue     Calibration value
 * \param[in] length         Calibration value array length
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
    uint8_t channel, eCalibParam paramId, uint8_t *calibrationValue, uint16_t length);

/**
 * \brief Get calibration parameters.
 *
 * \param[in]       channel         Channel
 * \param[in]       paramId         Calibration param Id
 * \param[inout]    calibResp       Pointer to \ref phCalibRespStatus_t
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
EXTERNC tUWBAPI_STATUS UwbApi_GetCalibration(uint8_t channel, eCalibParam paramId, phCalibRespStatus_t *calibResp);

/**
 * \brief Set Uwb Debug Configuration Parameters.
 * This API Can be used to set any number of debug parameters at once.
 *
 * To easily set the DebugParams list, following macros have been defined.
 *
 * UWB_SET_DEBUG_PARAM_VALUE_u8(Parameter, Value): This macro sets the value of the
 * corresponding parameter with the given Value.This shall be used to set value of 8 bit wide.
 *
 * UWB_SET_DEBUG_PARAM_VALUE_u16(Parameter, Value): This macro sets the value of the
 * corresponding parameter with the given Value.This shall be used to set  value of 16 bit wide.
 *
 * UWB_SET_DEBUG_PARAM_VALUE_u32(Parameter, Value): This macro sets the value of the
 * corresponding parameter with the given Value.This shall be used to set value of 32 bit wide.
 *
 * Example: To set DATA_LOGGER_NTF to zero, macro shall be invoked as given below.
 *
 * @code
 * UWB_DebugParams_List_t SetDebugParamsList[] = {UWB_SET_DEBUG_PARAM_VALUE(DATA_LOGGER_NTF, 0)};
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
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_DebugParams_List_t *DebugParams_List);

/**
 * \brief Get Uwb Debug Configuration Parameters.
 * This API Can be used to get any number of debug parameters at once.
 *
 * To easily get the DebugParams list, following macro has been defined.
 *
 * UWB_SET_GETDEBUG_PARAM_u8(Parameter): This macro gets the value of the
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
 * UWB_DebugParams_List_t GetDebugParamsList[] = {UWB_SET_GETAPP_PARAM(DATA_LOGGER_NTF),};
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
    uint32_t sessionHandle, uint8_t noOfparams, UWB_DebugParams_List_t *DebugParams_List);

/**
 * \brief Get the binding count using this API
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
EXTERNC tUWBAPI_STATUS UwbApi_GetBindingCount(phSeGetBindingCount_t *getBindingCount);

/**
 * \brief API to get the current temperature
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_QueryTemperature(uint8_t *pTemperatureValue);

/**
 * \brief API to get the UWB Timestamp for UWB time synchronization
 *
 * \param[in] len                 Length of i/p buffer. It should be 8 to hold 8 bytes timestamp value
 * \param[out] pTimestampValue    Timestamp data. It should be 8 bytes in size to hold 8 bytes timestamp value.
 *
 * On successful execution, this buffer will contain 8 bytes timestamp value.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW  if response length is more than expected response size
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_QueryUwbTimestamp(uint8_t len, uint8_t pTimestampValue[]);
#if (UWBIOT_UWBD_SR100T)
/**
 * \brief Verify Calibration data for all the set Calibration Parameters.
 *
 * \param[in] pCmacTag             Cmac Tag
 * \param[in] tagOption            Tag Option indicating Device/Model Specific tag
 * \param[in] tagVersion           Tag Version only for Model Specific Tag verification process.
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_VerifyCalibData(uint8_t *pCmacTag, uint8_t tagOption, uint16_t tagVersion);

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
EXTERNC tUWBAPI_STATUS UwbApi_CalibrationIntegrityProtection(eCalibTagOption tagOption, uint16_t calibBitMask);
#endif //UWBIOT_UWBD_SR100T

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

EXTERNC tUWBAPI_STATUS UwbApi_UpdateActiveRoundsAnchor(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    UWB_MacAddressMode_t macAddressingMode,
    const phActiveRoundsConfig_t roundConfigList[],
    phNotActivatedRounds_t *pNotActivatedRound);

#endif //UWBFTR_DL_TDoA_Anchor

#if UWBFTR_DL_TDoA_Tag
/**
 * \brief Update the active rounds during the DL-TDoA Session for a receiver device.
 *
 * \param[in]  sessionHandle             : Unique Session Handle
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
EXTERNC tUWBAPI_STATUS UwbApi_UpdateActiveRoundsReceiver(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    const uint8_t RangingroundIndexList[],
    phNotActivatedRounds_t *pNotActivatedRound);

#endif //UWBFTR_DL_TDoA_Tag

/** @} */

#if UWBFTR_SE_SN110

/** \addtogroup uwb_apis_sr100t APIs for SR100T
 *
 * @{ */

/**
 * \brief DoBind Perform Factory Binding using this API
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_PerformBinding(phSeDoBindStatus_t *doBindStatus);

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
EXTERNC tUWBAPI_STATUS UwbApi_TestConnectivity(SeConnectivityStatus_t *ConnectivityStatus);

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
EXTERNC tUWBAPI_STATUS UwbApi_SeTestLoop(uint16_t loopCnt, uint16_t timeInterval, phTestLoopData_t *testLoopData);

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
EXTERNC tUWBAPI_STATUS UwbApi_GetBindingStatus(phSeGetBindingStatus_t *getBindingStatus);

/**
 * \brief Delete RDS/URSK entries in SUS applet using this API
 *
 * \param[in] noOfsessionHandle         No of sessionHandles to be removed
 * \param[in] sessionHandleList         List of sessionHandles to be removed
 * \param[out] UrskDaletionStatus    URSK Deletion Status
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_URSKdeletionRequest(
    uint8_t noOfsessionHandle, uint32_t *sessionHandleList, phUrskDeletionRequestStatus_t *UrskDaletionStatus);

/** @} */

#endif // UWBFTR_SE_SN110

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
 *                         set bit 9 -> TX_TEMP_COMP
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
EXTERNC tUWBAPI_STATUS UwbApi_WriteOtpCalibDataCmd(uint8_t channel, uint16_t bitMask, phCalibPayload_t *pCalibPayload);

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
EXTERNC tUWBAPI_STATUS UwbApi_ReadOtpCalibDataCmd(uint8_t channel, uint16_t bitMask, phCalibPayload_t *pCalibPayload);

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

/** \addtogroup uwb_apis_sr150
 *
 * @{ */
/**
 * Read the Information from OTP based on param type See :cpp:type:`eOtpParam_Type_t`
 * This api can be used with both Factory and Mainline Firmware
 * \param[in]      paramType   parameter to write into otp See :cpp:type:`eOtpParam_Type_t`
 * \param[in,out]  infoLength  Size of pInfo buffer, number of bytes expected
 * \param[out]     pInfo       Otp Info, the size of this buffer shall be equal to infoLength
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if the operation timed out
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_ReadOtpCmd(eOtpParam_Type_t paramType, uint8_t *pInfo, uint8_t *infoLength);
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
EXTERNC tUWBAPI_STATUS UwbApi_WriteOtpBitMask(uint16_t BitMask);


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
EXTERNC tUWBAPI_STATUS UwbApi_ReadOtpBitMask(uint16_t bitMask);

/**
 * \brief Write CMAC tag value to OTP.
 *
 * \param[in] pCmacTag   CMAC tag value, must be 16bytes.
 *
 * \retval #UWBAPI_STATUS_OK  on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM  if invalid parameter is passed
 * \retval #UWBAPI_STATUS_TIMEOUT  if command is timeout
 * \retval #UWBAPI_STATUS_FAILED  otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_WriteOtpCmacTag(uint8_t* pCmacTag);
#endif //0
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160

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
EXTERNC tUWBAPI_STATUS UwbApi_PerformBinding(void);

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
EXTERNC tUWBAPI_STATUS UwbApi_PerformLocking(void);

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
EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfigWrappedRDS(
    uint32_t sessionHandle, uint8_t *pWrappedRds, size_t WrappedRdsLen);

#endif // (UWBFTR_SE_SE051W)

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
    const UWB_DebugParams_List_t *DebugParams_List);

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
 *
 */
tUWBAPI_STATUS UwbApi_ConfigureData_Android(uint8_t *pUwbPhoneConfigData,
    uint16_t UwbPhoneConfigDataLen,
    phUwbProfileInfo_t *pProfileInfo,
    uint8_t noOfVendorAppParams,
    const UWB_VendorAppParams_List_t *VendorAppParams_List,
    uint8_t noOfDebugParams,
    const UWB_DebugParams_List_t *DebugParams_List);

#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160

#if UWBIOT_UWBD_SR150
/**
 * \brief Command to set the localization Zone information which shall be sent in Final Data 2 message in the following Ranging Blocks.
 *
 * \param[in] psetLocZone                   : Session set localization zone information set from Application.
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 *
 * \note This command is only applicable to controlee.
 */
tUWBAPI_STATUS UwbApi_SessionSetLocZone(phSessionSetLocZone_t *pSetLocZone);
#endif     // UWBIOT_UWBD_SR150

/** @}  */ /* @addtogroup uwb_apis_sr150 */

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif // UWBAPI_PROPRIETARY_SR1XX_H
