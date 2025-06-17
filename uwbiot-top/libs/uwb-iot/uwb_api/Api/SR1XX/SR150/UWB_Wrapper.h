/*
 * Copyright 2021-2022 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 */

#ifndef UWB_WRAPPER_H
#define UWB_WRAPPER_H

#include "UwbApi_Types.h"

/* HELIOS_LC(1 byte) | RAND_HE(8 Bytes) | HELOS_ID(16 Bytes) */
#define INIT_BIND_DATA_LEN 25u
#define HELIOS_LC_LEN      1u
#define RAND_HE_LEN        8u
#define HELOS_ID_LEN       16u
#define RAND_SE_LEN        8u
#define SE_ID_LEN          16u
#define SE_ID_OFFSET       8u
#define BDI_LEN            16u

#define CARD_CHALLENGE_LEN       8u
#define CRYPTOGRAM_CHALLENGE_LEN 8u
#define CARD_CRYPTOGRAM_OFFSET   13u
#define HELIOS_LC_OFFSET         0u
#define RAND_HE_OFFSET           1u
#define HELOS_ID_OFFSET          9u

/* RAND_SE(8 Bytes) | SE_ID(16 Bytes) */
#define SE_SECRET_LEN 24

/**
 * \brief  Enumeration to define the APDU type for binding.
 */
typedef enum APDU_Type
{
    FINALIZE  = 0x30,
    LOCK_BIND = 0x31
} eAPDU_Type_t;

/**
 * \brief  Structure to store the initialize data required for binding.
 */
typedef struct InitBindingData
{
    /** Binding Root Key ID */
    uint8_t BRK_ID;
    /** Bind Init Data */
    uint8_t initData[INIT_BIND_DATA_LEN];
} phInitBindingData_t;

/**
 * \brief  Structure to store response APDU payload and its length.
 */
typedef struct phApduResponse
{
    /** APDU payload length */
    uint8_t APDUPayloadLength;
    /** APDU payload data */
    uint8_t APDUPayloadData[UCI_MAX_PAYLOAD_SIZE];
} phApduResponse_t;

/**
 * \brief  Structure to store BDI related params.
 */
typedef struct bdi_ctx
{
    uint8_t rand_HE[RAND_HE_LEN];
    uint8_t helios_id[HELOS_ID_LEN];
    uint8_t rand_SE[RAND_SE_LEN];
    uint8_t se_id[SE_ID_LEN];
    uint8_t bdi[BDI_LEN];
} bdi_ctx_t;

/**
 * \brief Retrieve SR150 initialize data required for binding.
 *
 * \param[out] pInitBindingData        Initiate Binding Data
 *                                (BRK_ID, HELIOS_LC | RAND_HE | HELOS_ID)
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timedout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS Get_Init_Binding_Data(phInitBindingData_t *pInitBindingData);

/**
 * \brief Send SE secrets to SR150, required to compute the SCP03 keys.
 *
 * \param[in] SE_SecretLen      Length of SE Secret, shall be 24 bytes.
 * \param[in] pSE_Secret        SE Secrets (RAND_SE | SE_ID).
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timedout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS Set_Se_Binding_Data(uint8_t SE_SecretLen, uint8_t *pSE_Secret);

/**
 * \brief Get the challenge by issuing initialize update command.
 *
 * \param[out] pInitUpdateApduResp      Initialize update data. valid only if API
 *                                       status is success.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW  if the response length is more than expected
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS Get_Se_Init_Update_Apdu(phApduResponse_t *pInitUpdateApduResp);

/**
 * \brief Perform External Authenticate command with the given cryptogram.
 *
 * \param[in] CryptogramLen             Length of the cryptogram
 * \param[in] pCryptogram               Cryptogram data
 * \param[out] pExtAuthApduResp         External Authenticate data. valid only if API
 *                                 status is success.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW  if the response length is more than expected
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS Set_Se_Ext_Auth_Cmd(uint8_t CryptogramLen, uint8_t *pCryptogram, phApduResponse_t *pExtAuthApduResp);

/**
 * \brief Request SR150 to encrypt and MAC for APDU type using the SCP03 keys.
 *
 * \param[in] eApduType                    APDU TYPE
 * \param[out] pSeEncApduResp              Encrypted APDU. valid only if API
 *                                          status is success.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW  if the response length is more than expected
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS Set_SE_Apdu_Enc(eAPDU_Type_t eApduType, phApduResponse_t *pSeEncApduResp);

/**
 * \brief Request SR150 to validate the response from SE.
 *
 * \param[in] pSeResponse                 SE response APDU.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS Validate_Se_Apdu_Resp_Cmd(phApduResponse_t *pSeResponse);

/**
 * \brief Instruct SR150 to commit the computed BDI to OTP.
 *
 * \param    None
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS Commit_Bdi(void);

/**
 * \brief Stores the bdi at the end of the binding process.
 * SUS Applet needs to be in selected state before storing bdi.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS StoreBdi(void);

/**
 * \brief Reinjects the bdi at the beginning of locking process.
 * SUS Applet needs to be in selected state before storing bdi.
 * This is only used as debug support.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS ReinjectBdi(void);
#endif /* UWB_WRAPPER_H_ */
