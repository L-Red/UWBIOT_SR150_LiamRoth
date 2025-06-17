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
#include "UWB_Wrapper.h"
#include "uci_defs.h"
#include "uci_ext_defs.h"
#include "UwbApi_Internal.h"
#include "phNxpLogApis_UwbApi.h"
#include "SE_Wrapper.h"
#include "StateMachine.h"
#include "uwb_types.h"
/* Internal Wrapper which validates the status and copies the response data */
static tUWBAPI_STATUS ValidateResponse(tUWBAPI_STATUS status, phApduResponse_t *pApduResp)
{
    uint8_t *rspPtr = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (status == UWBAPI_STATUS_OK) {
        if (uwbContext.rsp_len > UCI_MAX_PAYLOAD_SIZE) {
            NXPLOG_UWBAPI_E("%s: Apdu response data size is more than response buffer", __FUNCTION__);
            status = UWBAPI_STATUS_BUFFER_OVERFLOW;
        }
        else {
            /* rsp_data contains complete rsp, we have to skip Header */
            rspPtr = &uwbContext.rsp_data[UCI_RESPONSE_PAYLOAD_OFFSET];
            UWB_STREAM_TO_UINT8(pApduResp->APDUPayloadLength, rspPtr);
            phOsalUwb_MemCopy(pApduResp->APDUPayloadData, rspPtr, pApduResp->APDUPayloadLength);
            NXPLOG_UWBAPI_D("%s: Apdu command successful", __FUNCTION__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Apdu Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Apdu command failed", __FUNCTION__);
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Retrieve SR150 initialize data required for binding.
 *
 * \param[out] pInitBindingData        Initiate Binding Data
 *                                (BRK_ID, HELIOS_LC | RAND_HE | HELOS_ID)
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS Get_Init_Binding_Data(phInitBindingData_t *pInitBindingData)
{
    tUWBAPI_STATUS status;
    uint8_t *pGetInitDataCmd = NULL;
    uint8_t initDataLen      = 0;
    uint8_t *pInitDataPtr    = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (pInitBindingData == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    pGetInitDataCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pGetInitDataCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pGetInitDataCmd, UCI_EXT_MSG_SE_BIND_GET_INIT_DATA);
    UWB_UINT8_TO_STREAM(pGetInitDataCmd, 0x00); //RFU
    UWB_UINT8_TO_STREAM(pGetInitDataCmd, 0x00); //No payload

    status = sendRawUci(uwbContext.snd_data, (uint16_t)UCI_MSG_HDR_SIZE);
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Get Init Binding Data successful", __FUNCTION__);
        pInitDataPtr = &uwbContext.rsp_data[UCI_RESPONSE_PAYLOAD_OFFSET];
        initDataLen  = (uint8_t)(uwbContext.rsp_len - UCI_RESPONSE_PAYLOAD_OFFSET);
        UWB_STREAM_TO_UINT8(pInitBindingData->BRK_ID, pInitDataPtr);
        initDataLen--;
        if (initDataLen == INIT_BIND_DATA_LEN) {
            phOsalUwb_MemCopy(pInitBindingData->initData, pInitDataPtr, initDataLen);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Get Init Binding data size is not as expected", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get Init Binding Data Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get Init Binding Data failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Send SE secrets to SR150, required to compute the SCP03 keys.
 *
 * \param[in] SE_SecretLen      Length of SE Secret, shall be 24 bytes.
 * \param[in] pSeSecret         SE Secrets (RAND_SE | SE_ID).
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timedout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS Set_Se_Binding_Data(uint8_t SE_SecretLen, uint8_t *pSE_Secret)
{
    tUWBAPI_STATUS status;
    uint8_t *pSetSeSecretCmd = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (pSE_Secret == NULL || SE_SecretLen != SE_SECRET_LEN) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pSetSeSecretCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pSetSeSecretCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pSetSeSecretCmd, UCI_EXT_MSG_SE_BIND_SET_SE_DATA);
    UWB_UINT8_TO_STREAM(pSetSeSecretCmd, 0x00); //RFU
    UWB_UINT8_TO_STREAM(pSetSeSecretCmd, SE_SecretLen);
    UWB_ARRAY_TO_STREAM(pSetSeSecretCmd, pSE_Secret, SE_SecretLen);

    status = sendRawUci(uwbContext.snd_data, (uint16_t)(SE_SecretLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Set SE Binding Data successful", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Set SE Binding Data Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Set SE Binding Data failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Get the challenge by issuing initialize update command.
 *
 * \param[out] pInitUpdateApduResp      Initialize update data. valid only if API
 *                                 status is success.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW  if the response length is more than expected
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS Get_Se_Init_Update_Apdu(phApduResponse_t *pInitUpdateApduResp)
{
    tUWBAPI_STATUS status;
    uint8_t payloadLen      = 0;
    uint8_t *pInitUpdateCmd = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (pInitUpdateApduResp == NULL) {
        NXPLOG_UWBAPI_E("%s: pInitUpdateApduResp is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pInitUpdateCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pInitUpdateCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pInitUpdateCmd, UCI_EXT_MSG_SE_GET_HOST_CHALLENGE_APDU);
    UWB_UINT8_TO_STREAM(pInitUpdateCmd, 0x00);
    UWB_UINT8_TO_STREAM(pInitUpdateCmd, payloadLen);

    status = sendRawUci(uwbContext.snd_data, (uint16_t)(payloadLen + UCI_MSG_HDR_SIZE));

    return ValidateResponse(status, pInitUpdateApduResp);
}

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
tUWBAPI_STATUS Set_Se_Ext_Auth_Cmd(uint8_t CryptogramLen, uint8_t *pCryptogram, phApduResponse_t *pExtAuthApduResp)
{
    tUWBAPI_STATUS status;
    uint8_t payloadLen   = sizeof(CryptogramLen) + CryptogramLen;
    uint8_t *pExtAuthCmd = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (pExtAuthApduResp == NULL) {
        NXPLOG_UWBAPI_E("%s: pExtAuthApduResp is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (payloadLen > (MAX_UCI_PACKET_SIZE - UCI_MSG_HDR_SIZE)) {
        LOG_E("%s : invalid payload", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pExtAuthCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pExtAuthCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pExtAuthCmd, UCI_EXT_MSG_SE_GET_EXTERNAL_AUTH_APDU);
    UWB_UINT8_TO_STREAM(pExtAuthCmd, 0x00);
    UWB_UINT8_TO_STREAM(pExtAuthCmd, payloadLen);
    UWB_UINT8_TO_STREAM(pExtAuthCmd, CryptogramLen);
    UWB_ARRAY_TO_STREAM(pExtAuthCmd, pCryptogram, CryptogramLen);

    status = sendRawUci(uwbContext.snd_data, (uint16_t)(payloadLen + UCI_MSG_HDR_SIZE));

    return ValidateResponse(status, pExtAuthApduResp);
}

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
tUWBAPI_STATUS Set_SE_Apdu_Enc(eAPDU_Type_t eApduType, phApduResponse_t *pSeEncApduResp)
{
    tUWBAPI_STATUS status;
    uint8_t payloadLen   = sizeof(uint8_t);
    uint8_t *pEncApduCmd = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (pSeEncApduResp == NULL) {
        NXPLOG_UWBAPI_E("%s: pExtAuthApduResp is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pEncApduCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pEncApduCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pEncApduCmd, UCI_EXT_MSG_SE_DO_ENC_APDU);
    UWB_UINT8_TO_STREAM(pEncApduCmd, 0x00);
    UWB_UINT8_TO_STREAM(pEncApduCmd, payloadLen);
    UWB_UINT8_TO_STREAM(pEncApduCmd, eApduType);
    status = sendRawUci(uwbContext.snd_data, (uint16_t)(payloadLen + UCI_MSG_HDR_SIZE));

    return ValidateResponse(status, pSeEncApduResp);
}

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
tUWBAPI_STATUS Validate_Se_Apdu_Resp_Cmd(phApduResponse_t *pSeResponse)
{
    tUWBAPI_STATUS status;
    uint8_t *pValidateSeRsp = NULL;
    uint8_t payloadlen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (pSeResponse == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    payloadlen = pSeResponse->APDUPayloadLength + 1;
    if (payloadlen > (MAX_UCI_PACKET_SIZE - UCI_MSG_HDR_SIZE)) {
        LOG_E("%s : invalid payload", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pValidateSeRsp = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pValidateSeRsp, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pValidateSeRsp, UCI_EXT_MSG_SE_RESP_APDU_VALIDATE_REQ);
    UWB_UINT8_TO_STREAM(pValidateSeRsp, 0x00); //RFU
    UWB_UINT8_TO_STREAM(pValidateSeRsp, payloadlen);
    UWB_UINT8_TO_STREAM(pValidateSeRsp, pSeResponse->APDUPayloadLength);
    UWB_ARRAY_TO_STREAM(pValidateSeRsp, pSeResponse->APDUPayloadData, pSeResponse->APDUPayloadLength);

    status = sendRawUci(uwbContext.snd_data, (uint16_t)(payloadlen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Validate Se Response Apdu Cmd successful", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Validate Se Response Apdu Cmd Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Validate Se Response Apdu Cmd failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Instruct SR150 to commit the computed BDI to OTP.
 *
 * \param    None
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS Commit_Bdi(void)
{
    tUWBAPI_STATUS status;
    uint8_t *pCommitBdi = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    pCommitBdi = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pCommitBdi, UCI_MT_CMD, UCI_GID_PROPRIETARY_SE);
    UCI_MSG_BLD_HDR1(pCommitBdi, UCI_EXT_MSG_SE_BIND_COMMIT_BDI);
    UWB_UINT8_TO_STREAM(pCommitBdi, 0x00); //RFU
    UWB_UINT8_TO_STREAM(pCommitBdi, 0x00); //No Payload

    status = sendRawUci(uwbContext.snd_data, (uint16_t)UCI_MSG_HDR_SIZE);
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Commit BDI Cmd successful", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Commit BDI Cmd Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Commit BDI Cmd failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#if SE_API_ALLOW_GET_BDI
/**
 * \brief Stores the bdi at the end of the binding process.
 * SUS Applet needs to be in selected state before storing bdi.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS StoreBdi(void)
{
    tUWBAPI_STATUS uwb_status = UWBAPI_STATUS_FAILED;
    se_status_t se_status;
    size_t outBdiLen = sizeof(uwbSeContext.bindingContext.bdi);
    uint8_t srcdata[RAND_HE_LEN + RAND_SE_LEN + SE_ID_LEN];

    phOsalUwb_MemCopy(srcdata, uwbSeContext.bindingContext.rand_HE, RAND_HE_LEN);
    phOsalUwb_MemCopy(srcdata + RAND_HE_LEN, uwbSeContext.bindingContext.rand_SE, RAND_SE_LEN);
    phOsalUwb_MemCopy(srcdata + (RAND_HE_LEN + RAND_SE_LEN), uwbSeContext.bindingContext.se_id, SE_ID_LEN);

    se_status = Se_API_GetBdi(uwbSeContext.bindingContext.helios_id,
        sizeof(uwbSeContext.bindingContext.helios_id),
        srcdata,
        sizeof(srcdata),
        uwbSeContext.bindingContext.bdi,
        &outBdiLen);

    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("Se_API_GetBdi Failed");
    }
    else {
        uwb_status = UWBAPI_STATUS_OK;
    }
    return uwb_status;
}
#endif // SE_API_ALLOW_GET_BDI

/**
 * \brief Reinjects the bdi at the beginning of locking process.
 * SUS Applet needs to be in selected state before storing bdi.
 * This is only used as debug support.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS ReinjectBdi(void)
{
    uint8_t rawCmd[UCI_MSG_HDR_SIZE + BDI_LEN] = {0x29, 0x07, 0x00, 0x10};

    /* Send raw command to reinject the Bdi */
    phOsalUwb_MemCopy(
        rawCmd + UCI_MSG_HDR_SIZE, uwbSeContext.bindingContext.bdi, sizeof(uwbSeContext.bindingContext.bdi));
    return sendRawUci(rawCmd, (uint16_t)sizeof(rawCmd));
}
