/*
 *
 * Copyright 2018-2020,2022-2023 NXP.
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

#include "UwbApi.h"
#include "AppConfigParams.h"
#include "uci_ext_defs.h"
#include "uwa_api.h"
#include "phNxpLogApis_UwbApi.h"
#include "phOsalUwb.h"
#include "uwa_api.h"
#include "phNxpUwbConfig.h"
#include "UwbAdaptation.h"
#include "uwa_dm_int.h"

#if (UWBFTR_SE_SE051W)
#include "SE_Wrapper.h"
#endif

#include "UwbApi_Utility.h"
#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary_Internal.h"
#include "uwb_int.h"

#if UWBIOT_UWBD_SR1XXT
#include <Mainline_Firmware.h>
#include "uwb_fwdl_provider.h"
#endif // UWBIOT_UWBD_SR1XXT

#define MAX_SUPPORTED_TDOA_REPORT_FREQ 22
#define MIN_TRNG_SIZE                  0x01
#define MAX_TRNG_SIZE                  0x10
#define CHANNEL_5                      5
#define CHANNEL_6                      6
#define CHANNEL_8                      8
#define CHANNEL_9                      9
#define SEND_DATA_HEADER_LEN           16 // 4(Session Handle) + 8(mac address) + 1(dst endpoint) + 1(seq no) + 2(data size)

/* Minimum number of controlees for time based ranging is 1*/
/*In case of contention based ranging, this could be 0*/
#define MIN_NUM_OF_CONTROLEES 1

/**
 * \brief Initialize the UWB Device stack in the required mode. Operating mode
 *        will be set as per the Callback functions. Operating Modes supported include
 *        Standalone mode [default mode], CDC mode and MCTT mode.
 *        Atleast one of the call backs shall not be NULL.
 *        When all the callbacks are set then "Standalone" mode will take precedence.
 *
 * \param[in] pAppCtx   Pointer to \ref phUwbappContext_t strucutre
 *
 * \retval #UWBAPI_STATUS_OK             on success
 * \retval #UWBAPI_STATUS_TIMEOUT        if command is timeout
 * \retval #UWBAPI_STATUS_FAILED         otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_Init_New(phUwbappContext_t *pAppCtx)
{
    tUWBAPI_STATUS status                 = UWBAPI_STATUS_INVALID_PARAM;
    tUwbApi_AppCallback *pCallbackGeneric = NULL;
    /*By Defaulut Operating mode Should be Default*/
    Uwb_operation_mode_t eOperatingMode = kOPERATION_MODE_default;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if (pAppCtx == NULL) {
        NXPLOG_UWBAPI_E("pAppCtx is Null");
        goto exit;
    }

    if ((pAppCtx->pCallback == NULL) && (pAppCtx->pMcttCallback == NULL) && (pAppCtx->pCdcCallback == NULL)) {
        NXPLOG_UWBAPI_E("Atleast 1 appcallback should be set");
        goto exit;
    }

#if UWBIOT_UWBD_SR1XXT
    if (uwb_fwdl_getFwImage(&pAppCtx->fwImageCtx) != kUWBSTATUS_SUCCESS) {
        NXPLOG_UWBAPI_E("uwb_fwdl_getFwImage failed");
        return UWBAPI_STATUS_FAILED;
    }
#endif // UWBIOT_UWBD_SR1XXT

#if (UWBFTR_SE_SE051W)
    se_status_t se_status;
    if (pAppCtx->seHandle) {
        se_status = Se_API_SetHandle(pAppCtx->seHandle);
        if (se_status != SE_STATUS_OK) {
            NXPLOG_UWBAPI_E("Se_API_SetHandle failed");
            status = UWBAPI_STATUS_FAILED;
            goto exit;
        }
    }
    else {
        LOG_D("UwbApi_Init_New : Not using SE");
    }
#endif

    if (pAppCtx->pCallback != NULL) {
        /*Register the Genric callback*/
        pCallbackGeneric = pAppCtx->pCallback;
        eOperatingMode   = kOPERATION_MODE_default;
    }
    else if (pAppCtx->pCdcCallback != NULL) {
        /*Register the CDC callback*/
        pCallbackGeneric = pAppCtx->pCdcCallback;
        eOperatingMode   = kOPERATION_MODE_cdc;
    }

    else if (pAppCtx->pMcttCallback != NULL) {
        eOperatingMode = kOPERATION_MODE_mctt;
    }

    status = uwbInit(pCallbackGeneric, eOperatingMode);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("uwbInit failed");
        goto exit;
    }
    // Fira Test mode is enabled
    if (pAppCtx->pMcttCallback != NULL) {
        /* Do HAL call back register only after Uwb Init */
        HalRegisterAppCallback(pAppCtx->pMcttCallback);
    }

    /* Set operating mode */
    Hal_setOperationMode(eOperatingMode);

exit:
    NXPLOG_UWBAPI_D("%s: exit ", __FUNCTION__);
    return status;
}

/**
 * \brief To switch the Operating mode to MCTT
 *
 * \param[in] pAppCtx   Pointer to \ref phUwbappContext strucutre
 *
 * \retval #UWBAPI_STATUS_OK             on success
 * \retval #UWBAPI_STATUS_FAILED         otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_SwitchToMCTTMode(phUwbappContext_t *pAppCtx)
{
    // Fira Test mode is enabled
    if (pAppCtx->pMcttCallback != NULL) {
        /* Do HAL call back register only after Uwb Init */
        HalRegisterAppCallback(pAppCtx->pMcttCallback);
    }
    else {
        return UWBAPI_STATUS_FAILED;
    }

    /* Set operating mode */
    Hal_setOperationMode(kOPERATION_MODE_mctt);

    return UWBAPI_STATUS_OK;
}

/**
 * \brief Initialize the UWB Middleware stack in standalone mode.
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
EXTERNC tUWBAPI_STATUS UwbApi_Init(tUwbApi_AppCallback *pCallback)
{
#if UWBIOT_UWBD_SR1XXT
    phUwbFWImageContext_t fwImageCtx;
    fwImageCtx.fwImage   = (uint8_t *)heliosEncryptedMainlineFwImage;
    fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
    fwImageCtx.fwMode    = MAINLINE_FW;
    if (uwb_fwdl_getFwImage(&fwImageCtx) != kUWBSTATUS_SUCCESS) {
        NXPLOG_UWBAPI_E("uwb_fwdl_getFwImage failed");
        return UWBAPI_STATUS_FAILED;
    }

#endif //UWBIOT_UWBD_SR1XXT
    /** Initialize with Default mode. */
    return uwbInit(pCallback, kOPERATION_MODE_default);
}

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
EXTERNC tUWBAPI_STATUS UwbApi_ShutDown()
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_NOT_INITIALIZED;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_D("%s: UWB device is already  deinitialized", __FUNCTION__);
        return status;
    }
    sep_SetWaitEvent(UWA_DM_DISABLE_EVT);
    status = UWA_Disable(TRUE); /* gracefull exit */
    if (status == UWBAPI_STATUS_OK) {
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT) !=
            UWBSTATUS_SUCCESS) {
            LOG_E("%s : UWA_DM_DISABLE_EVT timedout", __FUNCTION__);
            status = UWBAPI_STATUS_TIMEOUT;
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: De-Init is failed:", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }
    /*Cleanup done only once */
    cleanUp();
    phUwb_LogDeInit();
    /* clear the Se handle in SE wrapper*/
#if (UWBFTR_SE_SE051W)
    Se_API_ResetHandle();
#endif
    return status;
}

#if !(UWBIOT_UWBD_SR040)
/**
 * \brief API to recover from crash, cmd timeout.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_RecoverUWBS()
{
#if UWBIOT_UWBD_SR1XXT
    phUwbFWImageContext_t fwImageCtx;
    fwImageCtx.fwImage   = (uint8_t *)heliosEncryptedMainlineFwImage;
    fwImageCtx.fwImgSize = sizeof(heliosEncryptedMainlineFwImage);
    fwImageCtx.fwMode    = MAINLINE_FW;
    if (uwb_fwdl_getFwImage(&fwImageCtx) != kUWBSTATUS_SUCCESS) {
        NXPLOG_UWBAPI_E("uwb_fwdl_getFwImage failed");
        return UWBAPI_STATUS_FAILED;
    }
#endif //UWBIOT_UWBD_SR1XXT

    return recoverUWBS();
}

#endif //!(UWBIOT_UWBD_SR040)

/**
 * \brief Resets UWBD device to Ready State
 *
 * \param[in] resetConfig   Supported Value: UWBD_RESET
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_UwbdReset(uint8_t resetConfig)
{
    tUWBAPI_STATUS status;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_DEVICE_RESET_RSP_EVT);
    uwbContext.dev_state   = UWBAPI_UCI_DEV_ERROR;
    uwbContext.snd_data[0] = resetConfig;
    status = sendUciCommandAndWait(UWA_DM_API_CORE_DEVICE_RESET_EVT, sizeof(resetConfig), uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_DEVICE_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.dev_state != UWBAPI_UCI_DEV_READY) {
            status = UWBAPI_STATUS_FAILED;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Gets UWB Device State
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if parameter is invalid
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetUwbDevState(uint8_t *pDeviceState)
{
    tUWBAPI_STATUS status;
    tUWA_PMID configParam[] = {UCI_PARAM_ID_DEVICE_STATE};
    uint16_t cmdLen         = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (pDeviceState == NULL) {
        NXPLOG_UWBAPI_E("%s: pDeviceState is NULL\n", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB Device is not initialized\n", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_CORE_GET_CONFIG_RSP_EVT);
    cmdLen = serializeGetCoreConfigPayload(1, sizeof(configParam), configParam, uwbContext.snd_data);

    status = sendUciCommandAndWait(UWA_DM_API_CORE_GET_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK && uwbContext.rsp_data[0] == UCI_PARAM_ID_DEVICE_STATE) {
        *pDeviceState = uwbContext.rsp_data[2];
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get UWB DEV state is failed\n", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Initializes session for a Type(Ranging/Data/Per)
 *
 * \param[in]   sessionId           Session ID.
 * \param[in]   sessionType         Type of Session(Ranging/Data/Per).
 * \param[out]  sessionHandle       Session Handle.
 *
 * \retval #UWBAPI_STATUS_OK                     on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED        if UWB stack is not initialized
 *
 * \retval #UWBAPI_STATUS_MAX_SESSIONS_EXCEEDED  if more than 5 sessions are exceeded
 * \retval #UWBAPI_STATUS_TIMEOUT                if command is timeout
 * \retval #UWBAPI_STATUS_FAILED                 otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionInit(uint32_t sessionId, eSessionType sessionType, uint32_t *sessionHandle)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (sessionType == UWBD_RFTEST) {
        if (sessionId != 0x00) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
    }
    if (sessionHandle == NULL) {
        NXPLOG_UWBAPI_E("%s: SessionHandle is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_INIT_RSP_EVT);
    cmdLen = serializeSessionInitPayload(sessionId, sessionType, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_INIT_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK && uwbContext.rsp_len > SESSION_HANDLE_OFFSET) {
        uint8_t *rspPtr = uwbContext.rsp_data;
        // skip the status from response.
        rspPtr++;
        // copy the Session Handle received through response.
        UWB_STREAM_TO_UINT32(*sessionHandle, rspPtr);
    }
    else {
        *sessionHandle = sessionId;
    }

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != *sessionHandle ||
            uwbContext.sessionInfo.state != UWB_SESSION_INITIALIZED) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_INITIALIZED notification", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);

    return status;
}

/**
 * \brief De-initialize based on Session Handle
 *
 * \param[in] sessionHandle   Initialized Session Handle
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionDeinit(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_SESSION_DEINIT_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_DEINIT_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle ||
            uwbContext.sessionInfo.state != UWB_SESSION_DEINITIALIZED) {
            return UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Set session specific ranging parameters.
 *
 *
 * \param[in] sessionHandle       Initialized Session Handle
 * \param[in] pRangingParam   Pointer to \ref phRangingParams
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetRangingParams(uint32_t sessionHandle, const phRangingParams_t *pRangingParam)
{
    tUWBAPI_STATUS status;
    uint8_t paramLen          = 0;
    uint8_t addrLen           = 0;
    uint8_t noOfRangingParams = 0;
    uint16_t cmdLen           = 0;
    uint8_t payloadOffset     = SES_ID_AND_NO_OF_PARAMS_OFFSET;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRangingParam == NULL) {
        NXPLOG_UWBAPI_E("%s: pRangingParam is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_ROLE,
                                        sizeof(pRangingParam->deviceRole),
                                        (void *)&pRangingParam->deviceRole,
                                        &uwbContext.snd_data[payloadOffset]));
    ++noOfRangingParams; // Increment the number of ranging params count

    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_MULTI_NODE_MODE,
                                        sizeof(pRangingParam->multiNodeMode),
                                        (void *)&pRangingParam->multiNodeMode,
                                        &uwbContext.snd_data[payloadOffset + paramLen]));
    ++noOfRangingParams; // Increment the number of ranging params count

    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_MAC_ADDRESS_MODE,
                                        sizeof(pRangingParam->macAddrMode),
                                        (void *)&pRangingParam->macAddrMode,
                                        &uwbContext.snd_data[payloadOffset + paramLen]));
    ++noOfRangingParams; // Increment the number of ranging params count

#if !(UWBIOT_UWBD_SR040)
    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_SCHEDULED_MODE,
                                        sizeof(pRangingParam->scheduledMode),
                                        (void *)&pRangingParam->scheduledMode,
                                        &uwbContext.snd_data[payloadOffset + paramLen]));
    ++noOfRangingParams; // Increment the number of ranging params count
#endif                   // !(UWBIOT_UWBD_SR040)

    if (pRangingParam->macAddrMode == SHORT_MAC_ADDRESS) {
        addrLen = (uint8_t)MAC_SHORT_ADD_LEN;
    }
    else if (pRangingParam->macAddrMode == EXTENDED_MAC_ADDRESS ||
             pRangingParam->macAddrMode == EXTENDED_MAC_ADDRESS_AND_HEADER) {
        addrLen = (uint8_t)MAC_EXT_ADD_LEN;
    }

    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_MAC_ADDRESS,
                                        addrLen,
                                        (void *)&pRangingParam->deviceMacAddr,
                                        &uwbContext.snd_data[payloadOffset + paramLen]));
    ++noOfRangingParams; // Increment the number of ranging params count

    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_RANGING_ROUND_USAGE,
                                        sizeof(pRangingParam->rangingRoundUsage),
                                        (void *)&pRangingParam->rangingRoundUsage,
                                        &uwbContext.snd_data[payloadOffset + paramLen]));
    ++noOfRangingParams; // Increment the number of ranging params count

    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_TYPE,
                                        sizeof(pRangingParam->deviceType),
                                        (void *)&pRangingParam->deviceType,
                                        &uwbContext.snd_data[payloadOffset + paramLen]));
    ++noOfRangingParams; // Increment the number of ranging params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfRangingParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);
#if UWBIOT_OS_NATIVE
    phOsalUwb_Delay(100);
#endif

    if ((status == UWBAPI_STATUS_OK) && (uwbContext.sessionInfo.state != UWB_SESSION_IDLE)) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_IDLE notification", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Get session specific ranging parameters

 * \param[in] sessionHandle       Initialized Session Handle
 * \param[out] pRangingParam   Pointer to \ref phRangingParams
 *
 * \retval #UWBAPI_STATUS_OK on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 */

EXTERNC tUWBAPI_STATUS UwbApi_GetRangingParams(uint32_t sessionHandle, phRangingParams_t *pRangingParams)
{
    tUWBAPI_STATUS status;
    uint8_t *pGetRangingCommand = NULL;
    uint16_t index              = 0;
    uint8_t paramId             = 0;
    uint8_t noOfParams;
    uint16_t cmdLen      = 0;
    uint8_t payloadOffet = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRangingParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pRangingParams is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    noOfParams         = uciRangingParamIds_len / sizeof(uint8_t);
    pGetRangingCommand = &uwbContext.snd_data[payloadOffet];
    for (index = 0; index < noOfParams; index++) {
        paramId = uciRangingParamIds[index];
        UWB_UINT8_TO_STREAM(pGetRangingCommand, paramId);
        NXPLOG_UWBAPI_D("%s: App ID: %02X", __FUNCTION__, paramId);
    }
    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);

    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, noOfParams, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        parseRangingParams(rspPtr, noOfParams, pRangingParams);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

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
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 *
 * Note : FOR SR1XXT and SR2XXT this API can only be used to set FIRA-specific AppCfgs.
 */

EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfig(uint32_t sessionHandle, eAppConfig param_id, uint32_t param_value)
{
    tUWBAPI_STATUS status;
    uint8_t noOfParams    = 1;
    uint8_t paramLen      = 0;
    uint16_t cmdLen       = 0;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    if (!(param_id < END_OF_SUPPORTED_APP_CONFIGS)) {
        NXPLOG_UWBAPI_W("Parameter can not be set using UwbApi_SetAppConfig. Use UwbApi_SetAppConfigMultipleParams");
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    else if ((param_id == STATIC_STS_IV) || (param_id == UWB_INITIATION_TIME)) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#elif UWBIOT_UWBD_SR040
    if (!((param_id >= RANGING_ROUND_USAGE && param_id < END_OF_SUPPORTED_APP_CONFIGS) ||
            ((param_id >> 4) >= EXTENDED_APP_CONFIG_ID && param_id < END_OF_SUPPORTED_EXT_CONFIGS))) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    else if (param_id == STATIC_STS_IV) {
        NXPLOG_UWBAPI_W(
            "STATIC_STS_IV can not be set using UwbApi_SetAppConfig. Use UwbApi_SetAppConfigMultipleParams");
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#endif // UWBIOT_UWBD_SR040

    /* TODO: to be removed */
    if (param_id == CHANNEL_NUMBER) {
        uint8_t channel = param_value & 0xff;
        if ((channel != CHANNEL_5) && (channel != CHANNEL_6) && (channel != CHANNEL_8) && (channel != CHANNEL_9)) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
    }

#if UWBIOT_UWBD_SR040
    if ((param_id >> 4) >= EXTENDED_APP_CONFIG_ID) {
        paramLen = getExtTLVBuffer(param_id, (void *)&param_value, &uwbContext.snd_data[payloadOffset]);
    }
    else
#endif // UWBIOT_UWBD_SR040
    {
        paramLen = getAppConfigTLVBuffer(param_id, 0, (void *)&param_value, &uwbContext.snd_data[payloadOffset]);
    }

    if (paramLen == 0) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
tUWBAPI_STATUS UwbApi_SetAppConfigMultipleParams(
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_AppParams_List_t *AppParams_List)
{
    uint16_t paramLen = 0;
    uint16_t cmdLen   = 0;
    eAppConfig paramId;
    tUWBAPI_STATUS status;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    UWB_AppParams_value_au8_t output_param_value;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((AppParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Assign the buffer for storing all the configs */
    output_param_value.param_value = uwbContext.snd_data;

    for (uint32_t LoopCnt = 0; LoopCnt < noOfparams; ++LoopCnt) {
        paramId = AppParams_List[LoopCnt].param_id;
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        if (!(paramId < END_OF_SUPPORTED_APP_CONFIGS)) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#elif UWBIOT_UWBD_SR040
        if (!((paramId >= RANGING_ROUND_USAGE && paramId < END_OF_SUPPORTED_APP_CONFIGS) ||
                ((AppParams_List[LoopCnt].param_id >> 4) >= EXTENDED_APP_CONFIG_ID &&
                    paramId < END_OF_SUPPORTED_EXT_CONFIGS))) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#endif // UWBIOT_UWBD_SR040
        /* parse and get input length and pointer */
        if (AppConfig_TlvParser(&AppParams_List[LoopCnt], &output_param_value) != UWBAPI_STATUS_OK) {
            return UWBAPI_STATUS_FAILED;
        }

#if UWBIOT_UWBD_SR040
        if ((AppParams_List[LoopCnt].param_id >> 4) >= EXTENDED_APP_CONFIG_ID) {
            /* Safe check for array indexing. Coverity issue fix. */
            paramLen = (uint16_t)(paramLen + getExtTLVBuffer(paramId,
                                                 (void *)(output_param_value.param_value),
                                                 &uwbContext.rsp_data[payloadOffset + paramLen]));
        }
        else
#endif // UWBIOT_UWBD_SR040
        {
            paramLen = (uint16_t)(paramLen + getAppConfigTLVBuffer(paramId,
                                                 (uint8_t)(output_param_value.param_len),
                                                 output_param_value.param_value,
                                                 &uwbContext.rsp_data[payloadOffset + paramLen]));
        }
    }

    if (paramLen == 0) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, paramLen, uwbContext.rsp_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.rsp_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
tUWBAPI_STATUS UwbApi_SetVendorAppConfigs(
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_VendorAppParams_List_t *vendorAppParams_List)
{
    uint16_t paramLen = 0;
    uint16_t cmdLen   = 0;
    eVendorAppConfig paramId;
    tUWBAPI_STATUS status;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    UWB_AppParams_value_au8_t output_param_value;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((vendorAppParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Assign the buffer for storing all the configs */
    output_param_value.param_value = uwbContext.snd_data;

    for (uint32_t LoopCnt = 0; LoopCnt < noOfparams; ++LoopCnt) {
        paramId = vendorAppParams_List[LoopCnt].param_id;
        /* parse and get input length and pointer */
        if (VendorAppConfig_TlvParser(&vendorAppParams_List[LoopCnt], &output_param_value) != UWBAPI_STATUS_OK) {
            return UWBAPI_STATUS_FAILED;
        }
#if (UWBFTR_SE_SE051W)
        if (paramId == WRAPPED_RDS) {
            uwbContext.rsp_data[paramLen++] = UCI_VENDOR_PARAM_ID_WRAPPED_RDS;
            uwbContext.rsp_data[paramLen++] = output_param_value.param_len;
            phOsalUwb_MemCopy(
                &uwbContext.rsp_data[paramLen], (void *)(output_param_value.param_value), output_param_value.param_len);
            paramLen += output_param_value.param_len;
        }
        else
#endif // UWBFTR_SE_SE051W
        {
            paramLen = (uint16_t)(paramLen + getVendorAppConfigTLVBuffer(paramId,
                                                 (void *)(output_param_value.param_value),
                                                 output_param_value.param_len,
                                                 &uwbContext.rsp_data[payloadOffset + paramLen]));
        }
    }

    if (paramLen == 0) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, paramLen, uwbContext.rsp_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.rsp_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
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
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 *
 * Note : FOR SR1XXT and SR2XXT this API can only be used to get FIRA-specific AppCfgs.
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetAppConfig(uint32_t sessionHandle, eAppConfig param_id, uint32_t *param_value)
{
    uint8_t len             = 0;
    uint8_t offset          = 0;
    uint8_t noOfParams      = 1;
    uint8_t paramLen        = 1;
    uint16_t cmdLen         = 0;
    uint8_t *pConfigCommand = NULL;
    uint8_t payloadOffset   = SES_ID_AND_NO_OF_PARAMS_OFFSET;

    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (param_value == NULL) {
        NXPLOG_UWBAPI_E("%s: param_value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

#if UWBIOT_UWBD_SR1XXT_SR2XXT
    if (!(param_id < END_OF_SUPPORTED_APP_CONFIGS)) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#elif UWBIOT_UWBD_SR040
    if (!((param_id >= RANGING_ROUND_USAGE && param_id < END_OF_SUPPORTED_APP_CONFIGS) ||
            ((param_id >> 4) >= EXTENDED_APP_CONFIG_ID && param_id < END_OF_SUPPORTED_EXT_CONFIGS))) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#endif // UWBIOT_UWBD_SR040

    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);
    pConfigCommand = &uwbContext.snd_data[payloadOffset];

#if UWBIOT_UWBD_SR040
    if ((param_id >> 4) >= EXTENDED_APP_CONFIG_ID) {
        UWB_UINT8_TO_STREAM(pConfigCommand, param_id);
    }
    else
#endif // UWBIOT_UWBD_SR040
    {
        UWB_UINT8_TO_STREAM(pConfigCommand, param_id);
    }
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        /* rsp_data contains rsp, starting from the paramId, so skip paramId field*/
        offset++;
        len             = uwbContext.rsp_data[offset++];
        uint8_t *rspPtr = &uwbContext.rsp_data[offset];
        if (len == sizeof(uint8_t)) {
            UWB_STREAM_TO_UINT8(*param_value, rspPtr);
        }
        else if (len == sizeof(uint16_t)) {
            UWB_STREAM_TO_UINT16(*param_value, rspPtr);
        }
        else if (len == (sizeof(uint32_t) - 1)) {
            UWB_STREAM_TO_UINT24(*param_value, rspPtr);
        }
        else if (len == sizeof(uint32_t)) {
            UWB_STREAM_TO_UINT32(*param_value, rspPtr);
        }
        else {
            NXPLOG_UWBAPI_W(
                "%s: API limitation, API Shall not be called for parameters having length more than 4 bytes",
                __FUNCTION__);
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
    uint32_t sessionHandle, uint8_t noOfparams, UWB_AppParams_List_t *AppParams_List)
{
    tUWBAPI_STATUS status;
    uint8_t i = 0;
    eAppConfig paramId;
    uint8_t *pConfigCommand = NULL;
    uint8_t payloadOffset   = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    uint16_t cmdLen         = 0;
    uint8_t offset          = 0;
    uint8_t *rspPtr         = NULL;
    uint16_t len            = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((AppParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pConfigCommand = &uwbContext.snd_data[payloadOffset];
    for (i = 0; i < noOfparams; i++) {
        paramId = AppParams_List[i].param_id;
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        if (!(paramId >= RANGING_ROUND_USAGE && paramId < END_OF_SUPPORTED_APP_CONFIGS)) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#endif
#if UWBIOT_UWBD_SR040
        if (!((paramId >= RANGING_ROUND_USAGE && paramId < END_OF_SUPPORTED_APP_CONFIGS) ||
                ((AppParams_List[i].param_id >> 4) >= EXTENDED_APP_CONFIG_ID &&
                    paramId < END_OF_SUPPORTED_EXT_CONFIGS))) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#endif

#if UWBIOT_UWBD_SR040
        if ((paramId >> 4) >= EXTENDED_APP_CONFIG_ID) {
            pConfigCommand[cmdLen++] = (paramId & 0xFF);
        }
        else
#endif // UWBIOT_UWBD_SR040
        {
            pConfigCommand[cmdLen++] = (uint8_t)(paramId & 0xFF);
        }
    }
    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, cmdLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        for (i = 0; i < noOfparams; i++) {
            /* rsp_data contains rsp, starting from the paramId, so skip paramId field*/
            offset++;
            len    = uwbContext.rsp_data[offset++];
            rspPtr = &uwbContext.rsp_data[offset];
            /* increment the offset according to the size of previous param value */
            offset += len;
            if (AppParams_List[i].param_type == kUWB_APPPARAMS_Type_u32) {
                if (len == sizeof(uint8_t)) {
                    UWB_STREAM_TO_UINT8(AppParams_List[i].param_value.vu32, rspPtr);
                }
                else if (len == sizeof(uint16_t)) {
                    UWB_STREAM_TO_UINT16(AppParams_List[i].param_value.vu32, rspPtr);
                }
                else if (len == (sizeof(uint32_t) - 1)) {
                    UWB_STREAM_TO_UINT24(AppParams_List[i].param_value.vu32, rspPtr);
                }
                else if (len == sizeof(uint32_t)) {
                    UWB_STREAM_TO_UINT32(AppParams_List[i].param_value.vu32, rspPtr);
                }
            }
            else if (AppParams_List[i].param_type == kUWB_APPPARAMS_Type_au8) {
                if (AppParams_List[i].param_value.au8.param_len >= len) {
                    phOsalUwb_MemCopy(AppParams_List[i].param_value.au8.param_value, rspPtr, len);
                }
                else {
                    NXPLOG_UWBAPI_E("%s: Not enough buffer to store app config value", __FUNCTION__);
                    status = UWBAPI_STATUS_BUFFER_OVERFLOW;
                }
                AppParams_List[i].param_value.au8.param_len = len;
            }
        }
    }
    return status;
}

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
tUWBAPI_STATUS UwbApi_GetVendorAppConfigs(
    uint32_t sessionHandle, uint8_t noOfparams, UWB_VendorAppParams_List_t *vendorAppParams_List)
{
    tUWBAPI_STATUS status;
    uint8_t i = 0;
    eVendorAppConfig paramId;
    uint8_t *pConfigCommand = NULL;
    uint8_t payloadOffset   = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    uint16_t cmdLen         = 0;
    uint8_t offset          = 0;
    uint8_t *rspPtr         = NULL;
    uint16_t len            = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((vendorAppParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pConfigCommand = &uwbContext.snd_data[payloadOffset];
    for (i = 0; i < noOfparams; i++) {
        paramId = vendorAppParams_List[i].param_id;

        // TODO: Add check to validate given vendor param is in list or not
        pConfigCommand[cmdLen++] = (uint8_t)(paramId & 0xFF);
    }
    // TODO: wait for GET_VENDOR_APP_CONFIG_RSP
    sep_SetWaitEvent(UWA_DM_SESSION_GET_VENDOR_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, cmdLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        for (i = 0; i < noOfparams; i++) {
            offset++;
            len    = uwbContext.rsp_data[offset++];
            rspPtr = &uwbContext.rsp_data[offset];
            offset += len;
            if (vendorAppParams_List[i].param_type == kUWB_APPPARAMS_Type_u32) {
                if (len == sizeof(uint8_t)) {
                    UWB_STREAM_TO_UINT8(vendorAppParams_List[i].param_value.vu32, rspPtr);
                }
                else if (len == sizeof(uint16_t)) {
                    UWB_STREAM_TO_UINT16(vendorAppParams_List[i].param_value.vu32, rspPtr);
                }
                else if (len == sizeof(uint32_t)) {
                    UWB_STREAM_TO_UINT32(vendorAppParams_List[i].param_value.vu32, rspPtr);
                }
            }
            else if (vendorAppParams_List[i].param_type == kUWB_APPPARAMS_Type_au8) {
                if (vendorAppParams_List[i].param_value.au8.param_len >= len) {
                    phOsalUwb_MemCopy(vendorAppParams_List[i].param_value.au8.param_value, rspPtr, len);
                }
                else {
                    NXPLOG_UWBAPI_E("%s: Not enough buffer to store app config value", __FUNCTION__);
                    status = UWBAPI_STATUS_BUFFER_OVERFLOW;
                }
                vendorAppParams_List[i].param_value.au8.param_len = len;
            }
        }
    }
    return status;
}
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT

/**
 * \brief Sets session specific app config parameters Vendor ID and Static STS
 * IV.
 *
 * \param[in] sessionHandle       Initialized Session Handle
 * \param[in] vendorId        App Config Parameter Vendor Id
 * \param[in] staticStsIv     Param value for App config param static Sts Iv
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST  if session is not initialized with
 *                                           sessionHandle
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetStaticSts(uint32_t sessionHandle, uint16_t vendorId, uint8_t const *const staticStsIv)
{
    tUWBAPI_STATUS status;
    uint8_t noOfParams    = 0;
    uint8_t paramLen      = 0;
    uint16_t cmdLen       = 0;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (staticStsIv == NULL) {
        NXPLOG_UWBAPI_E("%s: Static Sts Iv is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    paramLen = getAppConfigTLVBuffer(
        UCI_PARAM_ID_VENDOR_ID, UCI_PARAM_LEN_VENDOR_ID, (void *)&vendorId, &uwbContext.snd_data[payloadOffset]);

    ++noOfParams; // Increment the number of params count

    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_STATIC_STS_IV,
                                        UCI_PARAM_LEN_STATIC_STS_IV,
                                        (void *)staticStsIv,
                                        &uwbContext.snd_data[payloadOffset + paramLen]));

    ++noOfParams; // Increment the number of params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Sets device configuration
 *
 * \param[in] paramId        device configuration param id
 * \param[in] paramLen       Parameter length
 * \param[in] paramValue     Param value
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetDeviceConfig(
    eDeviceConfig param_id, uint8_t param_len, phDeviceConfigData_t *param_value)
{
    uint8_t offset = 1;
    tUWBAPI_STATUS status;
#if UWBIOT_UWBD_SR040
    uint8_t ext_param_id = 0;
#endif

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (param_value == NULL || param_len == 0) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

#if UWBIOT_UWBD_SR1XXT_SR2XXT
    if (((param_id >> 8) & 0xFF) == EXTENDED_DEVICE_CONFIG_ID) {
        offset = getExtCoreDeviceConfigTLVBuffer(param_id, param_len, param_value, &uwbContext.snd_data[offset]);
    }
    else {
        offset = getCoreDeviceConfigTLVBuffer(param_id, param_len, param_value, &uwbContext.snd_data[offset]);
    }
#endif // UWBIOT_UWBD_SR1XXT

#if UWBIOT_UWBD_SR040
    ext_param_id = (uint8_t)(param_id >> 4);
    /* Checking for the second Nibble */
    if (ext_param_id >= EXTENDED_DEVICE_CONFIG_ID) {
        /*No Mapping need */
        offset =
            getExtCoreDeviceConfigTLVBuffer(param_id, param_len, (void *)param_value, &uwbContext.snd_data[offset]);
    }
    else {
        /*No Mapping need */
        offset = getCoreDeviceConfigTLVBuffer(param_id, param_len, param_value, &uwbContext.snd_data[offset]);
    }
#endif // UWBIOT_UWBD_SR040

    if (offset == 0) {
        return UWBAPI_STATUS_FAILED;
    }
    sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
    uwbContext.snd_data[0] = 1;
    status = sendUciCommandAndWait(UWA_DM_API_CORE_SET_CONFIG_EVT, (uint16_t)(offset + 1), uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Get device config parameters.
 *
 * \param[in] param_id   Device Config Parameter Id
 * \param[in,out] devConfig   Param value structure for device config param id
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetDeviceConfig(eDeviceConfig param_id, phDeviceConfigData_t *devConfig)
{
    uint8_t offset     = 0;
    uint8_t noOfParams = 1;
    uint8_t paramLen   = 1;
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (devConfig == NULL) {
        NXPLOG_UWBAPI_E("%s: param_value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_CORE_GET_CONFIG_RSP_EVT);
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    if ((uint8_t)(param_id >> 8) == EXTENDED_DEVICE_CONFIG_ID) {
        paramLen++;
        param_id = (eDeviceConfig)((param_id >> 8) | (param_id << 8));
    }
#endif // UWBIOT_UWBD_SR1XXT

    cmdLen = serializeGetCoreConfigPayload(noOfParams, paramLen, (uint8_t *)&param_id, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_CORE_GET_CONFIG_EVT, cmdLen, uwbContext.snd_data);
    if (status == UWBAPI_STATUS_OK) {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        if ((uint8_t)param_id == EXTENDED_DEVICE_CONFIG_ID) {
            parseExtGetDeviceConfigResponse(&uwbContext.rsp_data[offset], devConfig);
        }
        else {
            parseCoreGetDeviceConfigResponse(&uwbContext.rsp_data[offset], devConfig);
        }
#endif // UWBIOT_UWBD_SR1XXT

#if UWBIOT_UWBD_SR040
        if ((uint8_t)(param_id >> 4) >= EXTENDED_DEVICE_CONFIG_ID) {
            parseExtGetDeviceConfigResponse(&uwbContext.rsp_data[offset], devConfig);
        }
        else {
            parseCoreGetDeviceConfigResponse(&uwbContext.rsp_data[offset], devConfig);
        }
#endif // UWBIOT_UWBD_SR040
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_StartRangingSession(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
#if UWBFTR_SE_SN110
    uint8_t KeyFetchErrorRetryCnt = 0U;
#endif
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
#if UWBFTR_SE_SN110
RetryUponKeyFetchError:
#endif //UWBFTR_SE_SN110
    sep_SetWaitEvent(UWA_DM_RANGE_START_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_START_EVT, cmdLen, uwbContext.snd_data);
#if UWBFTR_SE_SN110
    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWBD_SE_RANGING_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle ||
            uwbContext.sessionInfo.state != UWB_SESSION_ACTIVE) {
            /*
         * Hanlde the Key Fetch Error Handling. Only till Key Fetch error retry
         * count.
         */
            if (uwbContext.sessionInfo.state == UCI_SESSION_FAILED_WITH_KEY_FETCH_ERROR) {
                status = UCI_STATUS_ESE_RECOVERY_FAILURE;
                while (KeyFetchErrorRetryCnt < UWB_KEY_FETCH_ERROR_RETRY_COUNT) {
                    /*
                   * Increment the Key Fetch Error Retry Count.
                   */
                    ++KeyFetchErrorRetryCnt;
                    /*
                     * Wait for SE COMM ERROR NTF.
                     */
                    status = waitforNotification(UWA_DM_PROP_SE_COM_ERROR_NTF_EVT, UWB_NTF_TIMEOUT);
                    if (status == UWBAPI_STATUS_OK) {
                        /*
                         * If the Recovery is successful during se_comm_error notification
                         * then Resend start ranging command.
                         */
                        if (uwbContext.wstatus == UCI_STATUS_ESE_RECOVERY_SUCCESS) {
                            goto RetryUponKeyFetchError;
                        }
                        else if (uwbContext.wstatus == UCI_STATUS_ESE_RECOVERY_FAILURE) {
                            /*
                             * Reset the eSE.[Do the cold reset of eSE]
                             */
                            reset_se_on_error();
                            status = UWBAPI_STATUS_ESE_RESET;
                            break;
                        }
                    }
                }
            }
            else if (uwbContext.sessionInfo.state == UCI_SESSION_FAILED_WITH_NO_RNGDATA_IN_SE) {
                status = UWBAPI_STATUS_SESSION_NOT_EXIST;
            }
            else {
                status = UWBAPI_STATUS_FAILED;
            }
        }
    }
#else  /* UWBFTR_SE_SN110 */
    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (status != UWBAPI_STATUS_OK || uwbContext.sessionInfo.sessionHandle != sessionHandle ||
            uwbContext.sessionInfo.state != UWB_SESSION_ACTIVE) {
        	if (status != UWBAPI_STATUS_OK)
                NXPLOG_UWBAPI_E("status != UWBAPI_STATUS_OK", __FUNCTION__);
        	if (uwbContext.sessionInfo.sessionHandle != sessionHandle)
        		NXPLOG_UWBAPI_E("uwbContext.sessionInfo.sessionHandle != sessionHandle", __FUNCTION__);
        	if (uwbContext.sessionInfo.state != UWB_SESSION_ACTIVE)
        		NXPLOG_UWBAPI_E("uwbContext.sessionInfo.state != UWB_SESSION_ACTIVE: %d", __FUNCTION__, uwbContext.sessionInfo.state);
            NXPLOG_UWBAPI_E("%s: waitforNotification for event %d Failed", __FUNCTION__, UWA_DM_SESSION_STATUS_NTF_EVT);
            status = UWBAPI_STATUS_FAILED;
        }
    }
#endif /* UWBFTR_SE_SN110 */

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Stop Ranging for a session
 *
 * \param[in] sessionHandle   Initialized Session Handle
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_StopRangingSession(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_RANGE_STOP_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_STOP_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            status = UWBAPI_STATUS_FAILED;
        }
    }
#if UWBIOT_UWBD_SR040
    /* Wait for 1 more NTF, 6001000101.
     * Else there's Writer thread takes mutex of IO interface and read will always be pending, and write will always fail.
     * Cleaner handling would be to ensure writer thread is able to unblock reader thread.
     */
    phOsalUwb_Delay(10);
#endif

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_EnableRangingDataNtf(
    uint32_t sessionHandle, uint8_t enableRangingDataNtf, uint16_t proximityNear, uint16_t proximityFar)
{
    uint8_t noOfParam     = 1;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    uint8_t paramLen      = 0;
    uint16_t cmdLen       = 0;
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (enableRangingDataNtf > 2) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_SESSION_INFO_NTF,
                                        UCI_PARAM_LEN_SESSION_INFO_NTF,
                                        &enableRangingDataNtf,
                                        &uwbContext.snd_data[payloadOffset]));

    if (enableRangingDataNtf == 2) {
        noOfParam = (uint8_t)(noOfParam + 2);
        paramLen  = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_NEAR_PROXIMITY_CONFIG,
                                            UCI_PARAM_LEN_NEAR_PROXIMITY_CONFIG,
                                            &proximityNear,
                                            &uwbContext.snd_data[payloadOffset + paramLen]));

        paramLen = (uint8_t)(paramLen + getAppConfigTLVBuffer(UCI_PARAM_ID_FAR_PROXIMITY_CONFIG,
                                            UCI_PARAM_LEN_FAR_PROXIMITY_CONFIG,
                                            &proximityFar,
                                            &uwbContext.snd_data[payloadOffset + paramLen]));
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParam, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Send UCI RAW command.
 *
 * \param[in] data       UCI Command to be sent
 * \param[in] data_len   Length of the UCI Command
 * \param[out] resp       Response Received
 * \param[out] respLen    Response length
 *
 * \retval #UWBAPI_STATUS_OK               if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if wrong parameter is passed
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW  if response buffer is not sufficient
 *                                          to hold the response
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SendRawCommand(uint8_t data[], uint16_t data_len, uint8_t *pResp, uint16_t *pRespLen)
{
    tUWBAPI_STATUS status;
    uint8_t pbf;
    NXPLOG_UWBAPI_D("%s: enter; ", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        *pRespLen = 0;
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (data == NULL || data_len <= 0 || pResp == NULL || pRespLen == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (*pRespLen == 0) {
        NXPLOG_UWBAPI_E("%s: pRespLen is Zero", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = sendRawUci(data, data_len);
    if (status == UWBAPI_STATUS_PBF_PKT_SENT) {
        *pRespLen = 0;
        return status;
    }

    if (uwbContext.rsp_len > *pRespLen) {
        UCI_MSG_PRS_PBF(&uwbContext.rsp_data[0], pbf)
        if (pbf) {
            NXPLOG_UWBAPI_E(
                "%s: Response data size is more than response buffer due to chaining rsp expected for the command ",
                __FUNCTION__);
            NXPLOG_UWBAPI_W("Max Response size of respone expecting is pResp[%d] bytes:", uwbContext.rsp_len);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Response data size is more than response buffer", __FUNCTION__);
        }
        *pRespLen = 0;
        status    = UWBAPI_STATUS_BUFFER_OVERFLOW;
    }
    else {
        *pRespLen = uwbContext.rsp_len;
        phOsalUwb_MemCopy(pResp, uwbContext.rsp_data, uwbContext.rsp_len);
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/**
 * \brief Get Session State
 *
 * \param[in] sessionHandle      Initialized Session Handle
 * \param[out] sessionState   Session Status
 *
 * \retval #UWBAPI_STATUS_OK                on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           if command is timeout
 *
 * if API returns UWBAPI_STATUS_OK, Session State would be one of the below
 * values
 * \n #UWBAPI_SESSION_INIT_SUCCESS     - Session is Initialized
 * \n #UWBAPI_SESSION_DEINIT_SUCCESS   - Session is De-initialized
 * \n #UWBAPI_SESSION_ACTIVATED        - Session is Busy
 * \n #UWBAPI_SESSION_IDLE             - Session is Idle
 * \n #UWBAPI_SESSION_ERROR            - Session Not Found
 *
 * if API returns not UWBAPI_STATUS_OK, Session State is set to
 * UWBAPI_SESSION_ERROR
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetSessionState(uint32_t sessionHandle, uint8_t *sessionState)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: enter; ", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (sessionState == NULL) {
        NXPLOG_UWBAPI_E("%s: sessionState is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_GET_STATE_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_STATE_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        *sessionState = uwbContext.rsp_data[0];
    }
    else {
        *sessionState = UWBAPI_SESSION_ERROR;
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Update Controller Multicast List.
 *
 * \param[in] pControleeContext   Controlee Context
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_UpdateControllerMulticastList(phMulticastControleeListContext_t *pControleeContext)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pControleeContext == NULL) {
        NXPLOG_UWBAPI_E("%s: pControleeContext is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (pControleeContext->action == MULTICAST_LIST_DEL_CONTROLEE) {
        for (uint8_t i = 0; i < pControleeContext->no_of_controlees; i++) {
            if (pControleeContext->controlee_list[i].subsession_id != 0x0) {
                return UWBAPI_STATUS_INVALID_PARAM; //for deletion, sub Session Handle must be zero
            }
        }
    }

    sep_SetWaitEvent(UWA_DM_SESSION_MC_LIST_UPDATE_RSP_EVT);
    cmdLen = serializeUpdateControllerMulticastListPayload(pControleeContext, uwbContext.snd_data);
    status =
        sendUciCommandAndWait(UWA_DM_API_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)

/**
 * \brief Get TRNG  api.
 *
 * \param[in] trng size   param in \ref GetTrng
 * \param[out] ptrng   : the size of this buffer shall be equal to trng size.
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED    if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM      if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT            if command is timeout
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetTrng(uint8_t trng_size, uint8_t *ptrng)
{
    tUWBAPI_STATUS status;
    uint8_t *pResponse = NULL;
    uint16_t cmdLen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (ptrng == NULL) {
        NXPLOG_UWBAPI_E("%s: trng data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (trng_size > MAX_TRNG_SIZE || trng_size < MIN_TRNG_SIZE) {
        NXPLOG_UWBAPI_E("%s: Trng size  is invalid it should be Between 0x01-0x10 ", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_RANGE;
    }
    sep_SetWaitEvent(UWA_DM_PROP_TRNG_RESP_EVENT);
    cmdLen = serializeTrngtPayload(trng_size, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_TRNG_EVENT, cmdLen, uwbContext.snd_data);
    if (status == UWBAPI_STATUS_OK) {
        pResponse = &uwbContext.rsp_data[0]; //Response
        UWB_STREAM_TO_UINT8(status, pResponse);
        if (status == UWBAPI_STATUS_OK) {
            /* Fetch the TRNG bytes */
            phOsalUwb_MemCopy(ptrng, pResponse, trng_size);
        }
        else {
            NXPLOG_UWBAPI_E("%s: UwbApi_GetTrng failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetProfileParams(
    uint8_t *pProfileBlob, uint16_t blobSize, phUwbProfileInfo_t *pProfileInfo)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pProfileBlob == NULL) {
        NXPLOG_UWBAPI_E("%s: profile blob buffer is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pProfileInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: profile info is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if ((blobSize != TOTAL_PROFILE_BLOB_SIZE_v1_1) && (blobSize != TOTAL_PROFILE_BLOB_SIZE_v1_0)) {
        NXPLOG_UWBAPI_E("%s: profile blob size should be %d or %d bytes",
            __FUNCTION__,
            TOTAL_PROFILE_BLOB_SIZE_v1_0,
            TOTAL_PROFILE_BLOB_SIZE_v1_1);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_PROFILE_BLOB_RSP_EVENT);
    cmdLen = serializeSetProfileParamsPayload(pProfileInfo, blobSize, pProfileBlob, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_PROFILE_PARAM_EVENT, cmdLen, uwbContext.snd_data);

#if !(UWBIOT_UWBD_SR040)
    /* set the sessionHandle received from the PROP_SET_PROFILE_CMD */
    if (status == UWBAPI_STATUS_OK &&
        (uwbContext.rsp_len > SESSION_HANDLE_OFFSET && uwbContext.rsp_len <= SESSION_HANDLE_OFFSET_LEN)) {
        uint8_t *rspPtr = uwbContext.rsp_data;
        // skip the status from response.
        rspPtr++;
        // copy the Session Handle received through response.
        UWB_STREAM_TO_UINT32(pProfileInfo->sessionHandle, rspPtr);
    }
#endif // !(UWBIOT_UWBD_SR040)

    if (status == UWBAPI_STATUS_OK) {
        status = uwbContext.wstatus;
        if (status == UWBAPI_STATUS_OK) {
            status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
            if (status == UWBAPI_STATUS_OK && uwbContext.sessionInfo.state == UWB_SESSION_IDLE) {
                /** In case of SessionHandle is Disabled */
                if (uwbContext.rsp_len == SESSION_HANDLE_OFFSET) {
                    pProfileInfo->sessionHandle = uwbContext.sessionInfo.sessionHandle;
                }

                if (pProfileInfo->sessionHandle != uwbContext.sessionInfo.sessionHandle) {
                    status = UWBAPI_STATUS_FAILED;
                }
            }
            else {
                status = UWBAPI_STATUS_FAILED;
            }
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Fill Accessory UWB related configuration data.
 * \param device_role                -[in] device role
 * \param uwb_data_content           -[Out] Pointer to structure of AccessoryUwbConfigDataContent_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
tUWBAPI_STATUS UwbApi_GetUwbConfigData_iOS(
    UWB_DeviceRole_t device_role, AccessoryUwbConfigDataContent_t *uwb_data_content)
{
    tUWBAPI_STATUS status;
    static phUwbDevInfo_t devInfo    = {0};
    uint8_t uwb_spec_version_major[] = UWB_IOS_SPEC_VERSION_MAJOR;
    uint8_t uwb_spec_version_minor[] = UWB_IOS_SPEC_VERSION_MINOR;
    uint8_t manufacturer_id[]        = MANUFACTURER_ID;

    phUwbSessionData_t sessionData[MAXIMUM_SESSION_COUNT] = {0};
    phUwbSessionsContext_t uwbSessionsContext             = {0};
    uwbSessionsContext.sessioncnt                         = 5;
    uwbSessionsContext.pUwbSessionData                    = sessionData;
    uwbSessionsContext.status                             = 0;

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    uint8_t readMMIdLen        = MODULE_MAKER_ID_MAX_SIZE;
    eOtpParam_Type_t paramType = kUWB_OTP_ModuleMakerInfo;
#endif //UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (uwb_data_content == NULL) {
        NXPLOG_UWBAPI_E("%s: uwb_data_content is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if ((uwb_spec_version_minor[0] == 0x00) && (uwb_spec_version_minor[1] == 0x00)) {
        NXPLOG_UWBAPI_D(" Following spec 1.0");
        uwb_data_content->length =
            sizeof(AccessoryUwbConfigDataContent_t) - 1 - 2 /* clock drift not sent in spec 1.0 */;
    }
    else if ((uwb_spec_version_minor[0] == 0x01) && (uwb_spec_version_minor[1] == 0x00)) {
        NXPLOG_UWBAPI_D(" Following spec 1.1");
        uwb_data_content->length = sizeof(AccessoryUwbConfigDataContent_t) - 1;
    }
    else {
        NXPLOG_UWBAPI_D(" Unknown Spec");
        uwb_data_content->length = 0;
    }

    /* Generate mac address */
    status = UwbApi_GetTrng(MAC_SHORT_ADD_LEN, uwb_data_content->device_mac_addr);

    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_GetTrng() Failed");
        goto exit;
    }

    /* Fill in UWB spec version */
    phOsalUwb_MemCopy(uwb_data_content->uwb_spec_ver_major, uwb_spec_version_major, sizeof(uwb_spec_version_major));
    phOsalUwb_MemCopy(uwb_data_content->uwb_spec_ver_minor, uwb_spec_version_minor, sizeof(uwb_spec_version_minor));

    /* Get info of all UWBsessions */
    status = UwbApi_GetAllUwbSessions(&uwbSessionsContext);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_GetAllUwbSessions Failed");
        goto exit;
    }
    /* check if there is no session present, then only call UwbApi_GetDeviceInfo() */
    if (uwbSessionsContext.sessioncnt == 0) {
        status = UwbApi_GetDeviceInfo(&devInfo);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_GetDeviceInfo failed");
            goto exit;
        }
    }
    /* Fill in the MW version*/
    uwb_data_content->mw_version[3] = devInfo.mwMajor;
    uwb_data_content->mw_version[2] = devInfo.mwMinor;
    /* First two bytes are zero default */
    uwb_data_content->mw_version[1] = 0;
    uwb_data_content->mw_version[0] = 0;
    /* Fill in the Device Model ID */
    uwb_data_content->model_id[0] = devInfo.fwMajor;

#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160
    /* Check the chip type */
    if (phOsalUwb_MemCompare(devInfo.devName, "SR150_PROD_IOT_ROW", devInfo.devNameLen) != 0) {
        uwb_data_content->model_id[1] = 0xFF;
    }
    else {
        uwb_data_content->model_id[1] = MODELID_CHIP_TYPE;
    }
#elif UWBIOT_UWBD_SR040
    /* Check the chip type */
    if (phOsalUwb_MemCompare(devInfo.devName, "SR040", 5) != 0) {
        uwb_data_content->model_id[1] = 0xFF;
    }
    else {
        uwb_data_content->model_id[1] = MODELID_CHIP_TYPE;
    }
#endif //UWBIOT_UWBD_SR040
    /* Fill in the Manufacturer Version */
    phOsalUwb_MemCopy(uwb_data_content->manufacturer_id, manufacturer_id, sizeof(manufacturer_id));

    /* Fill in the device role */
    uwb_data_content->ranging_role = device_role;

    if ((uwb_spec_version_minor[0] == 0x01) && (uwb_spec_version_minor[1] == 0x00)) {
        /* Fill in the clock drift value */
        uwb_data_content->clock_drift[0] = CLOCK_DRIFT;
        uwb_data_content->clock_drift[1] = (CLOCK_DRIFT >> 8);
    }

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    status = UwbApi_ReadOtpCmd(paramType, &uwb_data_content->model_id[2], &readMMIdLen);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_ReadOtpCmd Failed");
        goto exit;
    }
#elif UWBIOT_UWBD_SR040
    uwb_data_content->model_id[2] = MODELID_BOARD_TYPE;
    uwb_data_content->model_id[3] = MODELID_RFU;
#endif //UWBIOT_UWBD_SR040

exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Fill Accessory UWB related configuration data.
 * \param uwb_data_content           -[Out] Pointer to structure of AccessoryUwbConfigDataContent_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
tUWBAPI_STATUS UwbApi_GetUwbConfigData_Android(UwbDeviceConfigData_t *uwb_data_content)
{
    tUWBAPI_STATUS status;
    static phUwbDevInfo_t devInfo    = {0};
    uint8_t uwb_spec_version_major[] = UWB_ANDROID_SPEC_VERSION_MAJOR;
    uint8_t uwb_spec_version_minor[] = UWB_ANDROID_SPEC_VERSION_MINOR;

    phUwbSessionData_t sessionData[MAXIMUM_SESSION_COUNT] = {0};
    phUwbSessionsContext_t uwbSessionsContext             = {0};
    uwbSessionsContext.sessioncnt                         = 5;
    uwbSessionsContext.pUwbSessionData                    = sessionData;
    uwbSessionsContext.status                             = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (uwb_data_content == NULL) {
        NXPLOG_UWBAPI_E("%s: uwb_data_content is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Fill in UWB spec version */
    phOsalUwb_MemCopy(uwb_data_content->spec_ver_major, uwb_spec_version_major, sizeof(uwb_spec_version_major));
    phOsalUwb_MemCopy(uwb_data_content->spec_ver_minor, uwb_spec_version_minor, sizeof(uwb_spec_version_minor));

    /* Get info of all UWBsessions */
    status = UwbApi_GetAllUwbSessions(&uwbSessionsContext);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_GetAllUwbSessions Failed");
        goto exit;
    }
    /* check if there is no session present, then only call UwbApi_GetDeviceInfo() */
    if (uwbSessionsContext.sessioncnt == 0) {
        status = UwbApi_GetDeviceInfo(&devInfo);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_GetDeviceInfo failed");
            goto exit;
        }
    }
    /* Fill in UWB chip FW version */
    uwb_data_content->chip_fw_version[0] = devInfo.fwMajor;
    uwb_data_content->chip_fw_version[1] = devInfo.fwMinor;

    /* Fill in the MW version */
    uwb_data_content->mw_version[0] = devInfo.mwMajor;
    uwb_data_content->mw_version[1] = devInfo.mwMinor;
    uwb_data_content->mw_version[2] = 0x00;

#if UWBIOT_UWBD_SR150
    /* Check the chip type */
    if (phOsalUwb_MemCompare(devInfo.devName, "SR150_PROD_IOT_ROW", devInfo.devNameLen) != 0) {
        uwb_data_content->chip_id[1] = 0xFF;
    }
    else {
        uwb_data_content->chip_id[1] = MODELID_CHIP_TYPE;
    }
#elif UWBIOT_UWBD_SR040
    /* Check the chip type */
    if (phOsalUwb_MemCompare(devInfo.devName, "SR040", 5) != 0) {
        uwb_data_content->chip_id[1] = 0xFF;
    }
    else {
        uwb_data_content->chip_id[1] = MODELID_CHIP_TYPE;
    }
#endif //UWBIOT_UWBD_SR040

    /* Fill in supported UWB profile Ids */
    uwb_data_content->supported_config_ids = UWB_SUPPORTED_CONFIG_IDS;

    /* Fill in the supported uwb profile ids */
    uwb_data_content->ranging_role = UWB_SUPPORTED_DEVICE_RANGING_ROLES;

    /* Generate mac address */
    status = UwbApi_GetTrng(MAC_SHORT_ADD_LEN, uwb_data_content->device_mac_addr);

    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_GetTrng() Failed");
        goto exit;
    }

exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
 */
tUWBAPI_STATUS UwbApi_GetDeviceCapability(phUwbCapInfo_t *pDevCap)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pDevCap == NULL) {
        NXPLOG_UWBAPI_E("%s: pDevCap is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = getCapsInfo();

    if (status == UWBAPI_STATUS_OK) {
        if (parseCapabilityInfo(pDevCap) == FALSE) {
            NXPLOG_UWBAPI_E("%s: Parsing Capability Information Failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Parsing Capability Information Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Parsing Capability Information failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
 */
EXTERNC tUWBAPI_STATUS UwbApi_SendData(phUwbDataPkt_t *pSendData)
{
    tUWBAPI_STATUS status    = UWBAPI_STATUS_FAILED;
    uint16_t cmdLen          = 0;
    uint16_t packetLength    = 0;
    uint8_t offset           = 0;
    bool isFirstSegment      = true;
    uint8_t pbf              = 1;
    uint8_t hdrLen           = SEND_DATA_HEADER_LEN;
    uint8_t flag             = 0; // This flag is used to indicate 3rd packet
    bool dataTransferOngoing = false;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((pSendData == NULL) || (pSendData->data == NULL)) {
        NXPLOG_UWBAPI_E("%s: pSendData is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pSendData->data_size + SEND_DATA_HEADER_LEN > uwbContext.maxDataPacketPayloadSize) {
        NXPLOG_UWBAPI_E("%s: pSendData length is more than %d", __FUNCTION__, uwbContext.maxDataPacketPayloadSize);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    cmdLen = serializeSendDataPayload(pSendData, uwbContext.snd_data);

    while (cmdLen > 0) {
        // Only for the first packet we should send header data
        if (!isFirstSegment) {
            hdrLen = 0;
        }

        // For the first packet in case of fragmentation, we should send header data along with MAX_DATA_PACKET_PAYLOAD_SIZE
        if (cmdLen > uwbContext.maxDataPacketPayloadSize + hdrLen) {
            packetLength = uwbContext.maxDataPacketPayloadSize + hdrLen;
        }
        else {
            // Incase of single packet, we should send the entire length including header
            pbf          = 0;
            packetLength = cmdLen;
        }

        // For 1st packet offset should be 0, for 2nd packet offset should be MAX_DATA_PACKET_PAYLOAD_SIZE + 16
        if (flag == 0) {
            if (!isFirstSegment) {
                offset += uwbContext.maxDataPacketPayloadSize + SEND_DATA_HEADER_LEN;
                flag = 1;
            }
        }
        else {
            // From 3rd packet onwards, offset should be MAX_DATA_PACKET_PAYLOAD_SIZE
            offset += uwbContext.maxDataPacketPayloadSize;
        }

        status = sendUciCommand(UWA_DM_API_SEND_DATA_EVENT, packetLength, &uwbContext.snd_data[offset], pbf);
        if (status == UWBAPI_STATUS_OK) {
            /**
             * Normal flow handling is skipped
             */
            if (pbf) {
                status = waitforNotification(UWA_DM_DATA_CREDIT_STATUS_EVT, UWB_NTF_TIMEOUT);
                if ((status != UWBAPI_STATUS_OK) || (uwbContext.dataCredit.sessionHandle != pSendData->sessionHandle) ||
                    (uwbContext.dataCredit.credit_availability != UCI_STATUS_CREDIT_AVAILABLE)) {
                    status = UWBAPI_STATUS_FAILED;
                    NXPLOG_UWBAPI_E("%s: status: %d", __FUNCTION__, status);
                    break;
                }
            }
        }
        else {
            status = UWBAPI_STATUS_FAILED;
            NXPLOG_UWBAPI_E("%s: status: %d", __FUNCTION__, status);
            break;
        }

        cmdLen -= packetLength;
        isFirstSegment = false;
    }
    if (status == UWBAPI_STATUS_OK) {
        /** As Per CR-490
         *  If SESSION_DATA_TRANSFER_STATUS_NTF is disabled, then the UWBS shall not send SESSION_DATA_TRANSFER_STATUS_NTF for every Application Data transmission except for last.
         *  SESSION_DATA_TRANSFER_STATUS_NTF is 'enabled' and 'DATA_REPETITION_COUNT > 0' it indicates that one Data transmission is completed in a RR.
         *  if above configs are enabled then UWBS shall send 'STATUS_REPETITION_OK' after each Application Data transmission completion except for the last Application Data transmission.
         * EX: SESSION_DATA_TRANSFER_STATUS_NTF = 1 and DATA_REPETITION_COUNT = 5
         *     We receive 5 data Transmit notifications , in which 4 are with status 'STATUS_REPETITION_OK' and last ntf with STATUS_OK.
         *     uwbContext.dataTransmit.txcount is 5 (DATA_REPETITION_COUNT = 5).
         *  by default 'dataTransferOngoing' is enabled to check at least one Transmit notification.
        */
        do {
            /* wait fot the  Data Transmit ntf (0x62(GID), 0x05(OID))*/
            status = waitforNotification(UWA_DM_DATA_TRANSMIT_STATUS_EVT, UWBD_TRANSMIT_NTF_TIMEOUT);
            if (status == UWBAPI_STATUS_OK) {
                /**
                 * check for the  Data transmission status.
                 * status is UWBAPI_DATA_TRANSFER_STATUS_OK then Data transmission is completed.
                */
                if ((uwbContext.dataTransmit.transmitNtf_status == UWBAPI_DATA_TRANSFER_STATUS_OK) &&
                    (uwbContext.dataTransmit.transmitNtf_sessionHandle == pSendData->sessionHandle) &&
                    (uwbContext.dataTransmit.transmitNtf_sequence_number == pSendData->sequence_number)) {
                    /* disable dataTransferOngoing flag it is last Data transmission". */
                    NXPLOG_UWBAPI_I("%s: Tansmit-status: %d txcount:%d",
                        __FUNCTION__,
                        uwbContext.dataTransmit.transmitNtf_status,
                        uwbContext.dataTransmit.transmitNtf_txcount);
                    dataTransferOngoing = FALSE;
                }
                else if (uwbContext.dataTransmit.transmitNtf_status == UWBAPI_DATA_TRANSFER_STATUS_REPETITION_OK) {
                    /* Transmission status UWBAPI_DATA_TRANSFER_STATUS_REPETITION_OK indicates Data transmission is ongoing.
                    *  Enable dataTransferOngoing flag  till last Data transmission with status code "UWBAPI_DATA_TRANSFER_STATUS_OK".
                    */
                    dataTransferOngoing = TRUE;
                    NXPLOG_UWBAPI_I("%s: Tansmit-status: %d txcount:%d",
                        __FUNCTION__,
                        uwbContext.dataTransmit.transmitNtf_status,
                        uwbContext.dataTransmit.transmitNtf_txcount);
                }
                else {
                    /* Apart from 'STATUS_REPETITION_OK' and 'STATUS_REPETITION_OK' rest of the status treated as failure .*/
                    NXPLOG_UWBAPI_E(
                        "%s: UWB TRANSMIT STATUS : %d", __FUNCTION__, uwbContext.dataTransmit.transmitNtf_status);
                    dataTransferOngoing = FALSE;
                    status              = uwbContext.dataTransmit.transmitNtf_status;
                }
            }
            else {
                /**
                 * NTF timeout(UWBD_TRANSMIT_NTF_TIMEOUT) occured .
                 * Not able to receive the UWA_DM_DATA_TRANSMIT_STATUS_EVT 0x62(GID), 0x05(OID) notification.
                 *
                */
                NXPLOG_UWBAPI_E("%s: UWBD_TRANSMIT_NTF_TIMEOUT with staus: %d", __FUNCTION__, status);
                dataTransferOngoing = FALSE;
                status              = UWBAPI_STATUS_FAILED;
            }
        } while (dataTransferOngoing);

        /* check for the status success,if failure exit */
        ENSURE_AND_GOTO_EXIT;
        status = waitforNotification(UWA_DM_DATA_CREDIT_STATUS_EVT, UWB_NTF_TIMEOUT);
        if ((status != UWBAPI_STATUS_OK) || (uwbContext.dataCredit.sessionHandle != pSendData->sessionHandle)) {
            status = UWBAPI_STATUS_FAILED;
            NXPLOG_UWBAPI_E(
                "%s: UWB_NTF_TIMEOUT for UWA_DM_DATA_CREDIT_STATUS_EVT with status: %d", __FUNCTION__, status);
            goto exit;
        }
        if (NULL != uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_DATA_TRANSMIT_NTF, (void *)&uwbContext.dataTransmit);
        }
        else {
            NXPLOG_UWBAPI_E("%s: %d pAppCallback is NULL", __FUNCTION__, __LINE__);
        }
    }
exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
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
tUWBAPI_STATUS UwbApi_SessionQueryDataSize(phUwbQueryDataSize_t *pQueryDataSize)
{
    tUWBAPI_STATUS status;
    uint32_t rspSessionId;
    uint8_t *pResponse = NULL;
    uint16_t cmdLen    = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (pQueryDataSize == NULL) {
        NXPLOG_UWBAPI_E("%s: pDataSize is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(pQueryDataSize->sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        /* Catch the Response of 6 bytes */
        pResponse = &uwbContext.rsp_data[0];
        /* Copy the sessionHandle Received */
        UWB_STREAM_TO_UINT32(rspSessionId, pResponse);
        /** Skip the status code */
        pResponse++;
        /* Compare the Receved Session Handle with Sent Session Handle */
        if (pQueryDataSize->sessionHandle == rspSessionId) {
            /* Copy the DataSize of 2 bytes*/
            UWB_STREAM_TO_UINT16(pQueryDataSize->dataSize, pResponse);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid Session Handle", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Timed Out with status :%d", __FUNCTION__, status);
    }
    else if (status == UWBAPI_STATUS_UNKNOWN) {
        NXPLOG_UWBAPI_E("%s: Unknown with status :%d", __FUNCTION__, status);
    }
    else {
        NXPLOG_UWBAPI_E("%s: failed with status :%d", __FUNCTION__, status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
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
EXTERNC tUWBAPI_STATUS UwbApi_SetControllerHusSession(phControllerHusSessionConfig_t *pHusSessionCfg)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pHusSessionCfg == NULL) {
        NXPLOG_UWBAPI_E("%s: pHusSessionCfg is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pHusSessionCfg->phase_count > MAX_PHASE_COUNT) {
        NXPLOG_UWBAPI_E("%s: pHusSessionCfg max phase count exceedded.", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_HUS_CONTROLLER_CONFIG_RSP_EVT);
    cmdLen = serializeControllerHusSessionPayload(pHusSessionCfg, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_HUS_CONTROLLER_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

/**
 * \brief Frames the HUS session config in TLV format for Controlee.
 *
 * \param[in]   pHusSessionCfg              : Pointer to structure of device HUS Controlee session config data.
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
EXTERNC tUWBAPI_STATUS UwbApi_SetControleeHusSession(phControleeHusSessionConfig_t *pHusSessionCfg)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pHusSessionCfg == NULL) {
        NXPLOG_UWBAPI_E("%s: pHusSessionCfg is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pHusSessionCfg->phase_count > MAX_PHASE_COUNT) {
        NXPLOG_UWBAPI_E("%s: pHusSessionCfg max phase count exceedded.", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_HUS_CONTROLEE_CONFIG_RSP_EVT);
    cmdLen = serializeControleeHusSessionPayload(pHusSessionCfg, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_HUS_CONTROLEE_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

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
EXTERNC tUWBAPI_STATUS UwbApi_SessionDataTxPhaseConfig(phDataTxPhaseConfig_t *phDataTxPhaseCfg)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (phDataTxPhaseCfg == NULL) {
        NXPLOG_UWBAPI_E("%s: phDataTxPhaseCfg is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_RSP_EVT);
    cmdLen = serializeDtpcmPayload(phDataTxPhaseCfg, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_EVT, cmdLen, uwbContext.snd_data);
    if (status == UWBAPI_STATUS_OK) {
        /* DTPCM Notification will only be received in active state */
        if (uwbContext.sessionInfo.state == UWB_SESSION_ACTIVE) {
            status = waitforNotification(UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_NTF_EVT, UWB_NTF_TIMEOUT);
            if (status != UWBAPI_STATUS_OK) {
                return UWBAPI_STATUS_FAILED;
            }
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
