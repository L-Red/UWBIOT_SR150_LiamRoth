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
#ifndef UWBAPI_PROPRIETARY_FM_H
#define UWBAPI_PROPRIETARY_FM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "phUwbTypes.h"
#include <UwbApi_Types_Proprietary.h>

EXTERNC const uint8_t heliosEncryptedFactoryFwImage[];
EXTERNC const uint32_t heliosEncryptedFactoryFwImageLen;

/** \addtogroup uwb_factorytest
 *
 * APIs for factory mode test
 *
 * @{ */

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
EXTERNC tUWBAPI_STATUS UwbApi_FactoryInit(tUwbApi_AppCallback *pCallback);

/**
 * \brief API to recover from Factory Firmware crash, cmd timeout.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_RecoverFactoryUWBS();

#if (UWBIOT_UWBD_SR100T)
/**
 * \brief API to configure the auth tag options. Only applicable in Factory Firmware.
 * \param[in]  deviceTag     device Tag
 *  0x0 or 0xFF signifies that none of the calibration parameters are integrity protected by “Device Specific” Tag. Any other value signifies that the calibration parameters are integrity protected by “Device specific tag.
 * \param[in]  modelTag      model Tag
 *  0x0 or 0xFF signifies that none of the calibration parameters are integrity protected by “Model Specific” Tag. Any other value signifies that Calibration parameters are integrity protected by “Model specific tag.
 * \param[in]  labelValue    label Value
 *  This value must be different for each customer and also different for each model sold to same customer. NXP is responsible for choosing the customer specific label and providing to customer to be used in their production lines.
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_ConfigureAuthTagOptions(uint8_t deviceTag, uint8_t modelTag, uint16_t labelValue);

/**
 * \brief API to configure the auth tag version. Only applicable in Factory Firmware.
 * \param[in]  tagVersion     Tag Version
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_ConfigureAuthTagVersion(uint16_t tagVersion);

/**
 * \brief Generate Tag for the Calibration Parameters. Only applicable in factory firmware.
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
EXTERNC tUWBAPI_STATUS UwbApi_GenerateTag(uint8_t tagOption, phGenerateTagRespStatus_t *pCmacTagResp);
#endif //UWBIOT_UWBD_SR100T

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
/**
 * \brief Write the Information to OTP based on param type See :cpp:type:`eOtpParam_Type_t`
 *        This api can only be used with Factory Firmware
 * \param[in]  paramType    parameter to write into otp See :cpp:type:`eOtpParam_Type_t`
 * \param[in]  pInfo        Info to write
 * \param[in]  infoLength   Info Length
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if the operation timed out
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_WriteOtpCmd(eOtpParam_Type_t paramType, uint8_t *pInfo, uint8_t infoLength);
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160

/** @} */

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif // UWBAPI_PROPRIETARY_FM_H
#endif // UWBFTR_FactoryMode
