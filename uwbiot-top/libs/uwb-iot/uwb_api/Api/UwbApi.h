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

#ifndef UWBAPI_H
#define UWBAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <uwb_board.h>
#include "phUwb_BuildConfig.h"
#include "phUwbTypes.h"
#include "UwbApi_Types.h"
#include "uwb_types.h"
#include <UwbApi_Types_Proprietary.h>
#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary.h"
#include "phOsalUwb.h"

#if UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT
EXTERNC const uint8_t heliosEncryptedMainlineFwImage[];
EXTERNC const uint32_t heliosEncryptedMainlineFwImageLen;
#endif

#ifndef UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT
#error UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT must be defined in uwb_board.h
#endif

#ifndef UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD
#error UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD must be defined in ``uwb_board.h``
#endif

#if UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD
/* FW Download is from an external flash. Only supported for RV4 */
#define UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST 0
#else
/* FW Download is directly from host, and SR1XX FW Image is compiled in as a
 * a part of the host application.
 *
 * This is default implementation for almost every system. */
#define UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST 1
#endif

#define ENSURE_AND_GOTO_EXIT          \
    if (UWBAPI_STATUS_OK != status) { \
        LOG_E("UWBAPI STAUS FAILED"); \
        goto exit;                    \
    }

/*
* UwbApi_GetStackCapabilities() is going to deprecated...
* Instead of UwbApi_GetStackCapabilities(), use UwbApi_GetDeviceInfo()
*
* To maintain backward compatibilty of API used #define here
*/
#define UwbApi_GetStackCapabilities UwbApi_GetDeviceInfo

/**
 * \brief APIs exposed to application to access UWB Functionality.
 */

/** \addtogroup uwb_apis
 *
 * @{ */

/**
 * \brief Initialize the UWB Device stack in the required mode. Operating mode
 *        will be set as per the Callback functions. Operating Modes supported include
 *        Standalone mode [default mode], CDC mode and MCTT mode.
 *        Atleast one of the call backs shall not be NULL.
 *        When all the callbacks are set then "Standalone" mode will take precedence.
 *
 * \param[in] pAppCtx   Pointer to \ref phUwbappContext_t structure
 *
 * \retval #UWBAPI_STATUS_OK             on success
 * \retval #UWBAPI_STATUS_TIMEOUT        if command is timeout
 * \retval #UWBAPI_STATUS_FAILED         otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_Init_New(phUwbappContext_t *pAppCtx);

/**
 * \brief To switch the Operating mode to MCTT
 *
 * \param[in] pAppCtx   Pointer to \ref phUwbappContext_t strucutre
 *
 * \retval #UWBAPI_STATUS_OK             on success
 * \retval #UWBAPI_STATUS_FAILED         otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SwitchToMCTTMode(phUwbappContext_t *pAppCtx);

/**
 * \brief Initialize the UWB Middleware stack in standalone mode
 *
 * \param[in] pCallback   Pointer to \ref tUwbApi_AppCallback
 *                         (Callback function to receive notifications (Ranging
 * data/App Data/Per Tx & Rx) at application layer.)
 *
 * \retval #UWBAPI_STATUS_OK             on success
 * \retval #UWBAPI_STATUS_TIMEOUT        if command is timeout
 * \retval #UWBAPI_STATUS_FAILED         otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_Init(tUwbApi_AppCallback *pCallback);

/**
 * \brief De-initializes the UWB Middleware stack
 *        Sequence of task deinitialization must be maintained
 *         -> Deinit client thread
 *         -> Deinit reader thread
 *         -> Deinit uwb_task thread
 *
 * \retval #UWBAPI_STATUS_OK      on success
 * \retval #UWBAPI_STATUS_FAILED  otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_ShutDown();

#if !(UWBIOT_UWBD_SR040)
/**
 * \brief API to recover from crash, cmd timeout.
 *
 * \retval #UWBAPI_STATUS_OK       on success
 * \retval #UWBAPI_STATUS_FAILED   otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT  if command is timeout
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_RecoverUWBS();
#endif //!(UWBIOT_UWBD_SR040)

/**
 * \brief Resets UWBD device to Ready State
 *
 * \param[in] resetConfig   Supported Value: UWBD_RESET
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_UwbdReset(uint8_t resetConfig);

/**
 * \brief Gets UWB Device State
 *
 * \param[out] pDeviceState   pointer to uint8_t to get Device State. Valid only
 * if API status is success.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if parameter is invalid
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetUwbDevState(uint8_t *pDeviceState);

/**
 * \brief Initializes session for a Type(Ranging/Data/Per)
 *
 * \param[in]   sessionId           Session ID.
 * \param[in]   sessionType         Type of Session(Ranging/Data/Per).
 * \param[out]  sessionHandle       Session Handle.
 *
 *
 * \retval #UWBAPI_STATUS_OK                     on success
 *
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED        if UWB stack is not initialized
 *
 * \retval #UWBAPI_STATUS_MAX_SESSIONS_EXCEEDED  if more than 5 sessions are
 *                                                exceeded
 *
 * \retval #UWBAPI_STATUS_TIMEOUT                if command is timeout
 *
 * \retval #UWBAPI_STATUS_FAILED                 otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionInit(uint32_t sessionId, eSessionType sessionType, uint32_t *sessionHandle);

/**
 * \brief De-initialize based on Session Handle
 *
 * \param[in] sessionHandle   Initialized Session Handle
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionDeinit(uint32_t sessionHandle);

/**
 * \brief Returns UCI, FW and MW version
 *
 * \param[out] pdevInfo Pointer to \ref phUwbDevInfo_t
 *
 * \retval UWBAPI_STATUS_OK              if successful
 * \retval UWBAPI_STATUS_NOT_INITIALIZED if UCI stack is not initialized
 * \retval UWBAPI_STATUS_INVALID_PARAM   if invalid parameters are passed
 * \retval UWBAPI_STATUS_FAILED          otherwise
 * \retval UWBAPI_STATUS_TIMEOUT         if command is timeout
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetDeviceInfo(phUwbDevInfo_t *pdevInfo);

/**
 * \brief Set session specific ranging parameters.
 *
 * For contention based ranging DST_MAC_ADDRESS and NO_OF_CONTROLEES parameter is not required both should be set to zero.
 *
 * Example: For time based and contention based configuration given below.
 * @code
 * // Time based Ranging :
 *
 * phRangingParams_t inRangingParams = {0};
 * inRangingParams.noOfControlees = 1;
 * inRangingParams.dstMacAddr[] = {0x11,0x22};
 *
 * // Contention based Ranging :
 *
 * phRangingParams_t inRangingParams = {0};
 * inRangingParams.noOfControlees = 0;
 * inRangingParams.dstMacAddr[] = {0x00,0x00};
 *
 * @endcode
 *
 * \param[in] sessionHandle       Initialized Session Handle
 * \param[in] pRangingParam   Pointer to \ref phRangingParams_t
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetRangingParams(uint32_t sessionHandle, const phRangingParams_t *pRangingParam);

/**
 * \brief Get session specific ranging parameters

 * \param[in] sessionHandle         Initialized Session Handle
 * \param[out] pRangingParams   Pointer to \ref phRangingParams_t .Valid only if
 *                              API status is success
 *
 * \retval #UWBAPI_STATUS_OK on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetRangingParams(uint32_t sessionHandle, phRangingParams_t *pRangingParams);

/**
 * \brief Set session specific app config parameters.
 *
 * \param[in] sessionHandle       Initialized Session Handle
 * \param[in] param_id   App Config Parameter Id
 * \param[in] param_value   Param value for App config param id
 *
 * \warning For setting STATIC_STS_IV and UWB_INITIATION_TIME, use UwbApi_SetAppConfigMultipleParams API.
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 * Note : FOR SR1XXT and SR2XXT this API can only be used to set FIRA-specific AppCfgs.
 *
 */

EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfig(uint32_t sessionHandle, eAppConfig param_id, uint32_t param_value);

/**
 * \brief Host shall use this API to set multiple application configuration parameters.
 * Number of Parameters also needs to be indicated.
 *
 * To easily set the AppParams list, following macros have been defined.
 *
 * UWB_SET_APP_PARAM_VALUE(Parameter, Value): This macro sets the value of the
 * corresponding parameter with the given Value.This shall be used to set
 * all types of values of 8 or 16 or 32 bit wide.For more than 32-bit values,
 * following macro shall be used.
 *
 * UWB_SET_APP_PARAM_ARRAY(Parameter, ArrayValue, Length): This macro sets the value
 * of the corresponding parameter as an array of 8bit. Length parameter contains the total
 * length of the array.
 *
 * Example: To set SFD Id to zero and static sts iv, macro shall be invoked as given below.
 *
 * @code
 * UWB_AppParams_List_t SetAppParamsList[] = {UWB_SET_APP_PARAM_VALUE(SFD_ID, 0)};
 * uint8_t static_sts_iv[] = {1,2,3,4,5,6};
 * UWB_AppParams_List_t SetAppParamsList[] = {UWB_SET_APP_PARAM_ARRAY(STATIC_STS_IV, static_sts_iv, sizeof(static_sts_iv))};
 * @endcode
 *
 * \param[in] sessionHandle        Initialized Session Handle
 * \param[in] noOfparams       Number of App Config Parameters
 * \param[in] AppParams_List   Application parameters values in tlv format
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 * Note : FOR SR1XXT and SR2XXT this API can only be used to set FIRA-specific AppCfgs.
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfigMultipleParams(
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_AppParams_List_t *AppParams_List);

/** Helper macro to limit the parameters and avoid error case */
#define UWBAPI_SETAPPCONFIGMULTIPLEPARAMS(SESSION_HANDLE, PARMS_LIST) \
    UwbApi_SetAppConfigMultipleParams((SESSION_HANDLE), sizeof(PARMS_LIST) / sizeof(PARMS_LIST[0]), &PARMS_LIST[0])

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief Host shall use this API to set multiple Vendor specific application configuration parameters.
 * Number of Parameters also needs to be indicated.
 *
 * To easily set the AppParams list, following macros have been defined.
 *
 * UWB_SET_VENDOR_APP_PARAM_VALUE(Parameter, Value): This macro sets the value of the
 * corresponding parameter with the given Value.This shall be used to set
 * all types of values of 8 or 16 or 32 bit wide.For more than 32-bit values,
 * following macro shall be used.
 *
 * UWB_SET_VENDOR_APP_PARAM_ARRAY(Parameter, ArrayValue, Length): This macro sets the value
 * of the corresponding parameter as an array of 8bit. Length parameter contains the total
 * length of the array.
 *
 * Example: To set MAC Palyoad encription Id to zero and antenna config tx, macro shall be invoked as given below.
 *
 * @code
 * UWB_VendorAppParams_List_t SetAppParamsList[] = {UWB_SET_VENDOR_APP_PARAM_VALUE(MAC_PAYLOAD_ENCRYPTION, 0)};
 * uint8_t antennas_configuration_tx[] = {1,2,3,4,5,6};
 * UWB_VendorAppParams_List_t SetAppParamsList[] = {UWB_SET_VENDOR_APP_PARAM_ARRAY(ANTENNAS_CONFIGURATION_TX, antennas_configuration_tx, sizeof(antennas_configuration_tx))};
 * @endcode
 *
 * \param[in] sessionHandle             Initialized Session Handle
 * \param[in] noOfparams            Number of App Config Parameters
 * \param[in] vendorAppParams_List  vendor specific Application parameters values in tlv format
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetVendorAppConfigs(
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_VendorAppParams_List_t *vendorAppParams_List);

#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief Get session specific app config parameters.
 *
 * \param[in] sessionHandle       Initialized Session Handle
 * \param[in] param_id   App Config Parameter Id
 * \param[out] param_value   Param value for App config param id
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 * Note : FOR SR1XXT and SR2XXT this API can only be used to get FIRA-specific AppCfgs.
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetAppConfig(uint32_t sessionHandle, eAppConfig param_id, uint32_t *param_value);

/**
 * \brief Host shall use this API to get multiple application configuration parameters.
 * Number of Parameters also needs to be indicated.
 *
 * Following macros can be used, to easily set the AppParams list
 *
 * UWB_SET_GETAPP_PARAM(Parameter): This macro sets parameter, in UWB_AppParams_List_t structure.
 * This shall be used to get all types of values of 8 or 16 or 32 bit wide.
 * For more than 32-bit values, following macro shall be used.
 *
 * UWB_SET_APP_PARAM_ARRAY(Parameter, ArrayValue, Length): This macro sets parameter and array of 8bit
 * to store the configuration, in UWB_AppParams_List_t structure Length parameter contains the total
 * length of the array.
 *
 * Example: To get SFD Id and static sts iv, macro shall be invoked as given below.
 *
 * @code
 * uint8_t static_sts_iv[6];
 * UWB_AppParams_List_t SetAppParamsList[] = {UWB_SET_GETAPP_PARAM_VALUE(SFD_ID),
 *                                       UWB_SET_APP_PARAM_ARRAY(STATIC_STS_IV, static_sts_iv, sizeof(static_sts_iv)),};
 * @endcode
 *
 * \param[in] sessionHandle        Initialized Session Handle
 * \param[in] noOfparams       Number of App Config Parameters
 * \param[in] AppParams_List   Application parameters
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 * Note : FOR SR1XXT and SR2XXT this API can only be used to get FIRA-specific AppCfgs.
 *
 */

EXTERNC tUWBAPI_STATUS UwbApi_GetAppConfigMultipleParams(
    uint32_t sessionHandle, uint8_t noOfparams, UWB_AppParams_List_t *AppParams_List);

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief Host shall use this API to get multiple vendor application configuration parameters.
 * Number of Parameters also needs to be indicated.
 *
 * Following macros can be used, to easily set the AppParams list
 *
 * UWB_SET_GETVENDOR_APP_PARAM_VALUE(Parameter): This macro sets parameter, in UWB_VendorAppParams_List_t structure.
 * This shall be used to get all types of values of 8 or 16 or 32 bit wide.
 * For more than 32-bit values, following macro shall be used.
 *
 * UWB_VENDOR_SET_APP_PARAM_ARRAY(Parameter, ArrayValue, Length): This macro sets parameter and array of 8bit
 * to store the configuration, in UWB_VendorAppParams_List_t structure Length parameter contains the total
 * length of the array.
 *
 * Example: To get MAC Palyoad encription Id and antenna config tx, macro shall be invoked as given below.
 *
 * @code
 * uint8_t antennas_configuration_tx[6];
 * UWB_VendorAppParams_List_t SetAppParamsList[] = {UWB_SET_GETVENDOR_APP_PARAM_VALUE(MAC_PAYLOAD_ENCRYPTION),
 *                                       UWB_VENDOR_SET_APP_PARAM_ARRAY(ANTENNAS_CONFIGURATION_TX, antennas_configuration_tx, sizeof(antennas_configuration_tx)),};
 * @endcode
 *
 * \param[in] sessionHandle        Initialized Session Handle
 * \param[in] noOfparams       Number of App Config Parameters
 * \param[in] vendorAppParams_List   Vendor Application parameters
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                            sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */

EXTERNC tUWBAPI_STATUS UwbApi_GetVendorAppConfigs(
    uint32_t sessionHandle, uint8_t noOfparams, UWB_VendorAppParams_List_t *vendorAppParams_List);

#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief Sets session specific app config parameters Vendor ID and Static STS
 * IV.
 *
 * \param[in] sessionHandle       Initialized Session Handle
 * \param[in] vendorId        App Config Parameter Vendor Id
 * \param[in] staticStsIv     Param value for App config param static Sts Iv
 *                            It is the responsibility of the caller that
 *                            STS IV is exactly UCI_PARAM_LEN_STATIC_STS_IV
 *                            bytes long.
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetStaticSts(uint32_t sessionHandle, uint16_t vendorId, uint8_t const *const staticStsIv);

/**
 * \brief Start Ranging for a session. Before Invoking Start ranging its
 * mandatory to set all the ranging configurations.
 *
 * \param[in] sessionHandle   Initialized Session Handle
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_StartRangingSession(uint32_t sessionHandle);

/**
 * \brief Sets device configuration
 *
 * \param[in] param_id        device configuration param id
 * \param[in] param_len       Parameter length
 * \param[in] param_value     Param value
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetDeviceConfig(
    eDeviceConfig param_id, uint8_t param_len, phDeviceConfigData_t *param_value);

/**
 * \brief Get device config parameters.
 *
 * \param[in] param_id      Device Config Parameter Id
 * \param[in,out] param_value   Param value structure for device config param id
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetDeviceConfig(eDeviceConfig param_id, phDeviceConfigData_t *param_value);

/**
 * \brief Stop Ranging for a session
 *
 * \param[in] sessionHandle   Initialized Session Handle
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_StopRangingSession(uint32_t sessionHandle);

/**
 * \brief Enable Ranging Data Notifications different options.

 * \param[in] sessionHandle              Initialized Session Handle
 * \param[in] enableRangingDataNtf   Enable Ranging data notification  0/1/2.
 option 2 is not allowed when ranging is ongoing.
 * \param[in] proximityNear          Proximity Near value valid if
 enableRangingDataNtf sets to 2
 * \param[in] proximityFar           Proximity far value valid if
 enableRangingDataNtf sets to 2
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_EnableRangingDataNtf(
    uint32_t sessionHandle, uint8_t enableRangingDataNtf, uint16_t proximityNear, uint16_t proximityFar);

/**
 * \brief Get Session State
 *
 * \param[in] sessionHandle      Initialized Session Handle
 * \param[out] sessionState   Session Status
 *
 * \retval #UWBAPI_STATUS_OK                on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            otherwise
 *
 * if API returns #UWBAPI_STATUS_OK, Session State would be one of the below
 * values
 * \n #UWBAPI_SESSION_INIT_SUCCESS     - Session is Initialized
 * \n #UWBAPI_SESSION_DEINIT_SUCCESS   - Session is De-initialized
 * \n #UWBAPI_SESSION_ACTIVATED        - Session is Busy
 * \n #UWBAPI_SESSION_IDLE             - Session is Idle
 * \n #UWBAPI_SESSION_ERROR            - Session Not Found
 *
 * if API returns not #UWBAPI_STATUS_OK, Session State is set to
 * #UWBAPI_SESSION_ERROR
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetSessionState(uint32_t sessionHandle, uint8_t *sessionState);

/**
 * \brief Send UCI RAW command.
 *
 * \param[in] data       UCI Command to be sent
 * \param[in] data_len   Length of the UCI Command
 * \param[out] pResp      Response Received
 * \param[out] pRespLen   Response length
 *
 * \retval #UWBAPI_STATUS_OK               if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if wrong parameter is passed
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW  if response buffer is not sufficient
 *                                          to hold the response
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SendRawCommand(uint8_t data[], uint16_t data_len, uint8_t *pResp, uint16_t *pRespLen);

/**
 * \brief Update Controller Multicast List.
 *
 * \param[in] pControleeContext   Controlee Context
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_UpdateControllerMulticastList(phMulticastControleeListContext_t *pControleeContext);

#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
/**
 * \brief Get TRNG api.
 *
 * \param[in] trng_size   Size of ptrng buffer and number of bytes expected
 * \param[out] ptrng   : the size of this buffer shall be equal to trng size.
 *
 * \warning On SR040, maximum 4 bytes can be drawn during an active ranging session
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetTrng(uint8_t trng_size, uint8_t *ptrng);

/**
 * \brief Setting up Profile blob.
 *
 * \param[in]      pProfileBlob : Profile Blob which contain all information.
 * \param[in]      blobSize     : Size of Blob
 * \param[in,out]  pProfileInfo : contains profile information.
 *
 * \retval #UWBAPI_STATUS_OK                on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetProfileParams(
    uint8_t *pProfileBlob, uint16_t blobSize, phUwbProfileInfo_t *pProfileInfo);

/**
 * \brief Fill Accessory UWB related configuration data.
 * \param device_role                -[in] device role
 * \param uwb_data_content           -[Out] Pointer to structure of AccessoryUwbConfigDataContent_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 *
 */
tUWBAPI_STATUS UwbApi_GetUwbConfigData_iOS(
    UWB_DeviceRole_t device_role, AccessoryUwbConfigDataContent_t *uwb_data_content);

/**
 * \brief Fill Accessory UWB related configuration data.
 * \param[out] uwb_data_content : Pointer to structure of UwbDeviceConfigData_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 *
 */
tUWBAPI_STATUS UwbApi_GetUwbConfigData_Android(UwbDeviceConfigData_t *uwb_data_content);

#endif // (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)

/**
 * \brief Frames the device capabilities in TLV format.
 *
 * \param pDevCap    - [Out] Pointer to structure of device capability data
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetDeviceCapability(phUwbCapInfo_t *pDevCap);

#if UWBFTR_DataTransfer
/**
 * \brief Host shall use this API to send data over UWB interface.
 * If SESSION_DATA_TRANSFER_STATUS_NTF is disabled, then the UWBS shall not send
 * SESSION_DATA_TRANSFER_STATUS_NTF for every Application Data transmission except for last.
 *
 * \warning : There is a possibility of UwbApi_SendData API returning Timeout (UWBAPI_STATUS_FAILED status) in the following sceanrio. Although API status is failed but the outcome of testing scenario to be treated as SUCCESS only.
 *
 * EX: SESSION_DATA_TRANSFER_STATUS_NTF=0(Disable) and DATA_REPETITION_COUNT=5 and RANGING_ROUND_USAGE=200 ms
 *
 * UWA_DM_DATA_TRANSMIT_STATUS_EVT will receive after 1,000 ms (DATA_REPETITION_COUNT * RANGING_ROUND_USAGE )
 *
 * Limitation:
 *
 * Notification Read Timeout : Reading UWA_DM_DATA_TRANSMIT_STATUS_EVT Notfication from the UWB will result in a
 * read timeout if UWB Notification time exceeds the define limit.
 *
 * EX: if DATA_REPETITION_COUNT=60 and SESSION_DATA_TRANSFER_STATUS_NTF=0 and RANGING_ROUND_USAGE=200 ms
 * then UWA_DM_DATA_TRANSMIT_STATUS_EVT will receive after ~12 secs (DATA_REPETITION_COUNT * RANGING_ROUND_USAGE)
 * by that time UwbApi_SendData API time out will happen .
 *
 * \param[in] phUwbDataPkt_t   Send Data Content
 *
 * \retval #UWBAPI_STATUS_OK                   on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED      if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM        if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT              if command is timeout
 * \retval #UWBAPI_STATUS_REJECTED             if session is not established when data packet sent
 * \retval #UWBAPI_STATUS_NO_CREDIT_AVAILABLE  if buffer is not available to accept data
 * \retval #UWBAPI_STATUS_DATA_TRANSFER_ERROR  if data is not sent due to an unrecoverable error
 * \retval #UWBAPI_STATUS_FAILED               otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SendData(phUwbDataPkt_t *pSendData);
#endif //UWBFTR_DataTransfer
#if !(UWBIOT_UWBD_SR040)
/**
 * \brief API to get max data size that can be transferred during a single ranging round.
 *
 * \param[inout] pQueryDataSize               Pointer to structure of Query Data Size
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_UNKNOWN          Unknown error
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 *
 */
tUWBAPI_STATUS UwbApi_SessionQueryDataSize(phUwbQueryDataSize_t *pQueryDataSize);
#endif // !(UWBIOT_UWBD_SR040)

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief Frames the HUS session config in TLV format for Controller.
 *
 * \param[in]   pHusSessionCfg              : Pointer to structure of device HUS Controller session config data.
 *
 * \retval #UWBAPI_STATUS_OK                                        on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED                           if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST                         if Session is not existing or not created.
 * \retval #UWBAPI_STATUS_INVALID_PHASE_PARTICIPATION               if Invalid Phase Participation values in HUS CONFIG CMD.
 * \retval #UWBAPI_STATUS_SESSION_NOT_CONFIGURED                    if Session is not configured with required app configurations.
 * \retval #UWBAPI_STATUS_TIMEOUT                                   if command is timeout
 * \retval #UWBAPI_STATUS_FAILED                                    otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetControllerHusSession(phControllerHusSessionConfig_t *pHusSessionCfg);

/**
 * \brief Frames the HUS session config in TLV format for Controlee.
 *
 * \param[in]   pHusSessionCfg              : Pointer to structure of device HUS Controlee session config data.
 *
 * \retval #UWBAPI_STATUS_OK                                        on success
 * \retval #UWBAPI_STATUS_REJECTED                                  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST                         if Session is not existing or not created.
 * \retval #UWBAPI_STATUS_INVALID_PHASE_PARTICIPATION               if Invalid Phase Participation values in HUS CONFIG CMD.
 * \retval #UWBAPI_STATUS_SESSION_NOT_CONFIGURED                    if command is timeout
 * \retval #UWBAPI_STATUS_FAILED                                    otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetControleeHusSession(phControleeHusSessionConfig_t *pHusSessionCfg);

/**
 * \brief Frames the Data Transfer Phase Control Message in TLV format.
 *
 * \param phDataTxPhaseCfg    - Pointer to structure of Data Transfer Phase Configuration.
 *
 * \retval #UWBAPI_STATUS_OK_DTPCM_CONFIG_SUCCESS           - if DTPCM is configured for given MAC Address.
 * \retval #UWBAPI_STATUS_ERROR_INVALID_SLOT_BITMAP         - if configured slot bit map size exceeds RANGING_DURATION.
 * \retval #UWBAPI_STATUS_ERROR_DUPLICATE_SLOT_ASSIGMENT    - if configured slot assignments is inconsistent.
 * \retval #UWBAPI_STATUS_ERROR_INVALID_MAC_ADDRESS         - if DTPCM is configured for given MAC Address.
 * \retval #UWBAPI_STATUS_INVALID_PARAM                     - if given MAC address is not found.
 * \retval #UWBAPI_STATUS_FAILED                            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionDataTxPhaseConfig(phDataTxPhaseConfig_t *phDataTxPhaseCfg);
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

/** @}  */ /* @addtogroup Uwb_Apis */
#endif     // UWBAPI_H
