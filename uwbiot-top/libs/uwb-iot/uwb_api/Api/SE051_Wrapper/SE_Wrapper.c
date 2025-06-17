/* Copyright 2021-2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include <ex_sss.h>
#include "phNxpLogApis_SE_Wrapper.h"
#include "SE_Wrapper.h"

#include <string.h>
#include <nxEnsure.h>

#include "smCom.h"

static se_SusSession_t gsus_ctx;

static se_status_t Se_parseRspSelectADF(
    uint8_t *pBuffer, const size_t bufferLen, se_firelite_selectadf_reponse_t *pResponse);

static se_status_t Se_parseRspInitiateTransaction(
    uint8_t *pBuffer, const size_t bufferLen, uint8_t *pStatus, uint8_t *pDataBuffer, size_t *pDataLen);

static se_status_t Se_parseRspDispatch(uint8_t *pBuffer,
    const size_t bufferLen,
    uint8_t *pStatus,
    uint8_t *pDataBuffer,
    size_t *pDataLen,
    uint8_t *pEventId,
    uint8_t *pEventDataBuffer,
    size_t *pEventDataLen);

static se_status_t Se_parseRspTunnel(
    uint8_t *pBuffer, const size_t bufferLen, uint8_t *pStatus, uint8_t *pDataBuffer, size_t *pDataLen);

static bool Se_checkHandle(pSe05xSession_t se_ctx)
{
    bool result = TRUE;

    if ((se_ctx == NULL) || (se_ctx->conn_ctx == NULL)) {
        LOG_E("SE Wrapper handle is not set");
        LOG_W("First open a communication with SE using sss_session_open(),");
        LOG_W("then set the handle using Se_API_SetHandle");
    }
    else {
        result = FALSE;
    }

    return result;
}

#if SE_API_ALLOW_GET_BDI
/* srcData = RAND_HE || RAND_SE || SE ID */
se_status_t Se_API_GetBdi(
    uint8_t *pkey, size_t keyLen, uint8_t *srcData, size_t srcDataLen, uint8_t *outBdi, size_t *outBdiLen)
{
    sss_type_t subsystem = kType_SSS_SubSystem_NONE;
    const char *cpFolder = "./cp";
    sss_session_t session;
    sss_key_store_t ks;
    sss_object_t key;
    sss_status_t status;
    sss_mac_t mac;
    uint32_t keyId        = __LINE__;
    se_status_t se_status = SE_STATUS_NOT_OK;

    sss_algorithm_t algorithm = kAlgorithm_SSS_CMAC_AES;
    sss_mode_t mode           = kMode_SSS_Mac;

    ENSURE_OR_GO_EXIT(pkey);
    ENSURE_OR_GO_EXIT(srcData);
    ENSURE_OR_GO_EXIT(outBdi);
    ENSURE_OR_GO_EXIT(outBdiLen);

#if SSS_HAVE_MBEDTLS
    subsystem = kType_SSS_mbedTLS;
#ifndef MBEDTLS_FS_IO
    cpFolder = NULL;
#endif
#endif
#if SSS_HAVE_OPENSSL
    subsystem = kType_SSS_OpenSSL;
#endif
    status = sss_session_open(&session, subsystem, 0, kSSS_ConnectionType_Plain, (void *)cpFolder);
    if (kStatus_SSS_Success != status) {
        LOG_E("Open Session Failed!!!");
        goto exit;
    }

    status = sss_key_store_context_init(&ks, &session);
    if (kStatus_SSS_Success != status) {
        LOG_E("sss_key_store_context_init Failed!!!");
        goto exit;
    }

    status = sss_key_store_allocate(&ks, __LINE__);
    if (kStatus_SSS_Success != status) {
        LOG_E("sss_key_store_context_init Failed!!!");
        goto exit;
    }

    status = sss_key_object_init(&key, &ks);
    if (kStatus_SSS_Success != status) {
        LOG_E("sss_key_object_init for derived key Failed");
        goto exit;
    }

    status = sss_key_object_allocate_handle(
        &key, keyId, kSSS_KeyPart_Default, kSSS_CipherType_CMAC, keyLen, kKeyObject_Mode_Persistent);
    if (kStatus_SSS_Success != status) {
        LOG_E("sss_key_object_allocate_handle Failed");
        goto exit;
    }

    status = sss_key_store_set_key(&ks, &key, pkey, keyLen, keyLen * 8, NULL, 0);
    if (kStatus_SSS_Success != status) {
        LOG_E("sss_key_object_allocate_handle Failed");
        goto exit;
    }

    status = sss_mac_context_init(&mac, &session, &key, algorithm, mode);
    if (kStatus_SSS_Success != status) {
        LOG_E(" sss_mac_context_init Failed!!!");
        goto exit;
    }

    status = sss_mac_one_go(&mac, srcData, srcDataLen, outBdi, outBdiLen);
    if (kStatus_SSS_Success != status) {
        LOG_E(" sss_mac_one_go Failed!!!");
        goto exit;
    }

    sss_key_object_free(&key);
    sss_mac_context_free(&mac);
    sss_key_store_context_free(&ks);
    if (srcDataLen == *outBdiLen) {
        se_status = SE_STATUS_OK;
    }

exit:
    sss_session_close(&session);
    sss_session_delete(&session);
    return se_status;
}
#endif /* SE_API_ALLOW_GET_BDI */

se_status_t Se_API_SetHandle(void *se_ctx)
{
    se_status_t status = SE_STATUS_NOT_OK;
    if ((se_ctx == NULL) || (((pSe05xSession_t)se_ctx)->conn_ctx == NULL)) {
        LOG_E("se_ctx is Null, Se_API_SetHandle failed");
    }
    else {
        gsus_ctx.se_ctx = (pSe05xSession_t)se_ctx;
        status          = SE_STATUS_OK;
        LOG_D("Set SE Handle successfully!!!");
    }

    return status;
}

void Se_API_ResetHandle(void)
{
    gsus_ctx.se_ctx = NULL;
    LOG_D("Reset SE Handle to NULL successfully!!!");
}

se_status_t Se_API_Init(se_applet_t pApplet, uint8_t logical_channel)
{
    se_status_t status     = SE_STATUS_NOT_OK;
    smStatus_t retStatus   = SM_NOT_OK;
    pSusSession_t pSus_ctx = &gsus_ctx;

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    switch (pApplet) {
    case SE_APPLET_SUS: /* fall through */
    case SE_APPLET_SUS_CLIENT:
        retStatus = SUS_API_Init(pSus_ctx->se_ctx, (uint8_t)pApplet, logical_channel);
        break;
    case SE_APPLET_FIRALITE:
        retStatus = se_FiRaLite_API_Select(pSus_ctx->se_ctx, logical_channel);
        break;
    default:
        break;
    }

    if (retStatus != SM_OK) {
        LOG_E("Could not select !!!.");
        goto cleanup;
    }

    LOG_I("Applet is Selected successfully!!!! .");
    status = SE_STATUS_OK;

cleanup:
    return status;
}

se_status_t Se_API_GetBindingState(se_bindState_t *pBindState)
{
    se_status_t status                   = SE_STATUS_NOT_OK;
    smStatus_t smstatus                  = SM_NOT_OK;
    uint8_t rspBuf[SUS_MAX_BUF_SIZE_RSP] = {0u};
    pSusSession_t pSus_ctx               = &gsus_ctx;
    size_t rspbufLen                     = sizeof(rspBuf);
    ENSURE_OR_GO_EXIT(pBindState);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    smstatus = SUS_API_GetData(pSus_ctx->se_ctx, (uint8_t)SUS_TAG_GET_BINDING_STATE, rspBuf, &rspbufLen);
    if ((smstatus == SM_OK) && (rspbufLen == SUS_GET_BINDING_STATE_RSP)) {
        pBindState->boundStatus         = (se_boundStatus_t)rspBuf[0];
        pBindState->factoryResetCounter = rspBuf[1];
        pBindState->bindingAttempts     = rspBuf[2];
        status                          = SE_STATUS_OK;
        switch (rspBuf[0]) {
        case 0:
            LOG_D("SUS Applet is Not Bound to any uwb system");
            break;
        case 1:
            LOG_D("SUS Applet is Bound and in unlocked state");
            break;
        case 2: {
            LOG_D("SUS Applet is Bound and in locked state");
            if (pBindState->bindingAttempts != 0) {
                LOG_E("bindingAttempts should be zero if applet is in bound and locked state");
                status = SE_STATUS_NOT_OK;
            }
        } break;
        default:
            LOG_E("Se_API_GetBindingState returned invalid state");
            status = SE_STATUS_NOT_OK;
            break;
        }
    }
    else {
        LOG_E("Se_API_GetBindingState Get state failed");
    }
exit:
    return status;
}

se_status_t Se_API_GetVersion(uint8_t *verString, size_t *verStringLen)
{
    se_status_t status     = SE_STATUS_NOT_OK;
    smStatus_t smstatus    = SM_NOT_OK;
    pSusSession_t pSus_ctx = &gsus_ctx;

    ENSURE_OR_GO_EXIT(verString);
    ENSURE_OR_GO_EXIT(verStringLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    smstatus = SUS_API_GetData(pSus_ctx->se_ctx, (uint8_t)SUS_TAG_GET_VERSION, verString, verStringLen);
    if ((smstatus == SM_OK) && (*verStringLen == SUS_GET_VERSION_RSP)) {
        status = SE_STATUS_OK;
        LOG_I("Current Applet version: %02d-%02d-%02d", verString[0], verString[1], verString[2]);
    }
    else {
        LOG_E("Se_API_GetVersion failed !!!");
    }

exit:
    return status;
}

se_status_t Se_API_GetFiraLiteVersion(
    uint8_t *verString, size_t *verStringLen, char *verDiscription, size_t *verDiscriptionLen)
{
    se_status_t status                   = SE_STATUS_NOT_OK;
    uint8_t rspBuf[SUS_MAX_BUF_SIZE_RSP] = {0u};
    size_t rspbufLen                     = sizeof(rspBuf);

    ENSURE_OR_GO_EXIT(verString);
    ENSURE_OR_GO_EXIT(verStringLen);
    ENSURE_OR_GO_EXIT(verDiscription);
    ENSURE_OR_GO_EXIT(verDiscriptionLen);

    status = Se_API_LocalGetData(
        (uint8_t)FIRALITE_TAG_GET_VERSION_MSB, (uint8_t)FIRALITE_TAG_GET_VERSION_LSB, rspBuf, &rspbufLen);
    if ((status == SE_STATUS_OK) && (rspbufLen > FIRALITE_VERSION_LEN)) {
        LOG_I("FiraLite Applet version: %02d-%02d-%02d-%02d", rspBuf[0], rspBuf[1], rspBuf[2], rspBuf[3]);
        LOG_I("FiraLite Descriptive version: %s", &rspBuf[4]);
        if ((*verStringLen >= FIRALITE_VERSION_LEN) && (*verDiscriptionLen >= rspbufLen - FIRALITE_VERSION_LEN)) {
            status = SE_STATUS_OK;
            phOsalUwb_MemCopy(verString, rspBuf, (uint32_t)FIRALITE_VERSION_LEN);
            phOsalUwb_MemCopy(
                verDiscription, &rspBuf[FIRALITE_VERSION_LEN], (uint32_t)(rspbufLen - FIRALITE_VERSION_LEN));
        }
        else {
            LOG_E("Not Enough output Buffer");
        }
        *verStringLen      = FIRALITE_VERSION_LEN;
        *verDiscriptionLen = rspbufLen - FIRALITE_VERSION_LEN;
    }
    else {
        LOG_E("Se_API_GetFiraLiteVersion failed !!!");
    }

exit:
    return status;
}

se_status_t Se_API_GetLifeCycle(se_lifeCycleStatus_t *pLcState)
{
    se_status_t status                   = SE_STATUS_NOT_OK;
    smStatus_t smstatus                  = SM_NOT_OK;
    uint8_t rspBuf[SUS_MAX_BUF_SIZE_RSP] = {0u};
    pSusSession_t pSus_ctx               = &gsus_ctx;
    size_t rspbufLen                     = sizeof(rspBuf);
    se_lifeCycleStatus_t seLc            = SE_None;

    ENSURE_OR_GO_EXIT(pLcState);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    smstatus = SUS_API_GetData(pSus_ctx->se_ctx, (uint8_t)SUS_TAG_GET_APPLET_LIFE_CYCLE, rspBuf, &rspbufLen);
    if (smstatus == SM_OK && rspbufLen == SUS_GET_APPLET_LIFE_CYCLE_RSP) {
        seLc = (se_lifeCycleStatus_t)rspBuf[0];
        if ((seLc >= SE_Personalized) && (seLc <= SE_Locked)) {
            *pLcState = seLc;
            status    = SE_STATUS_OK;
        }
        else {
            LOG_E("Se_API_GetLifeCycle received invalid SE Lifecycle state !!!");
        }
    }
    else {
        LOG_E("Se_API_GetLifeCycle failed !!!");
    }

exit:
    return status;
}

se_status_t Se_API_GetBindingHistory(pBindHistry_t pBindHistry)
{
    se_status_t status                   = SE_STATUS_NOT_OK;
    smStatus_t smstatus                  = SM_NOT_OK;
    uint8_t rspBuf[SUS_MAX_BUF_SIZE_RSP] = {0u};
    pSusSession_t pSus_ctx               = &gsus_ctx;
    size_t rspbufLen                     = sizeof(rspBuf);

    ENSURE_OR_GO_EXIT(pBindHistry);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    smstatus = SUS_API_GetData(pSus_ctx->se_ctx, (uint8_t)SUS_TAG_GET_BINDING_HISTORY, rspBuf, &rspbufLen);
    if ((smstatus == SM_OK) && (rspbufLen == SE_MAX_BINDING_HISTORY_SIZE)) {
        status                       = SE_STATUS_OK;
        int i                        = 0;
        pBindHistry->totalBindingCnt = rspBuf[i++] << 8;
        pBindHistry->totalBindingCnt |= rspBuf[i++];
        pBindHistry->uwbSystmCnt = rspBuf[i++];
        phOsalUwb_MemCopy((uint8_t *)&(pBindHistry->uwbSystm1), &rspBuf[i], SUS_GET_DATA_UWB_SYSTEM_HISTORY_SIZE);
        i += SUS_GET_DATA_UWB_SYSTEM_HISTORY_SIZE;
        phOsalUwb_MemCopy((uint8_t *)&(pBindHistry->uwbSystm2), &rspBuf[i], SUS_GET_DATA_UWB_SYSTEM_HISTORY_SIZE);
        i += SUS_GET_DATA_UWB_SYSTEM_HISTORY_SIZE;
        phOsalUwb_MemCopy((uint8_t *)&(pBindHistry->uwbSystm3), &rspBuf[i], SUS_GET_DATA_UWB_SYSTEM_HISTORY_SIZE);
    }
    else {
        LOG_E("Se_API_GetBindingHistory failed");
    }

exit:
    return status;
}

se_status_t Se_API_InitiateBinding(
    uint8_t Lc, uint8_t brk, uint8_t *pbindInData, size_t bindInDataLen, uint8_t *pbindOutData, size_t *pbindOutDataLen)
{
    se_status_t status     = SE_STATUS_NOT_OK;
    smStatus_t smstatus    = SM_NOT_OK;
    pSusSession_t pSus_ctx = &gsus_ctx;

    ENSURE_OR_GO_EXIT(pbindInData);
    ENSURE_OR_GO_EXIT(pbindOutData);
    ENSURE_OR_GO_EXIT(pbindOutDataLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    smstatus =
        SUS_API_InitiateBinding(pSus_ctx->se_ctx, Lc, brk, pbindInData, bindInDataLen, pbindOutData, pbindOutDataLen);

    if (smstatus == SM_OK) {
        status = SE_STATUS_OK;
    }
    else {
        LOG_E("Se_API_InitiateBinding failed !!!");
    }

exit:
    return status;
}

se_status_t Se_API_GetWrappedRDS(se_applet_t applet, se_rds_t *pRds, uint8_t *pWrappedRds, size_t *pWrappedRdsLen)
{
    se_status_t se_status                            = SE_STATUS_NOT_OK;
    size_t offset                                    = 0;
    uint8_t wrappedRDS[SUS_MAX_WRAPPED_RDS_RSP_SIZE] = {0};
    size_t wrappedRDSLen                             = SUS_MAX_WRAPPED_RDS_RSP_SIZE;
    size_t len                                       = 0;
    size_t rdslen                                    = 0;
    uint8_t *pRdsBuf                                 = NULL;
    size_t rdsOffset                                 = 0;
    int tlvRet;
    switch (applet) {
    case SE_APPLET_SUS_CLIENT: {
        se_status = Se_API_Init(applet, SE_CHANNEL_1);
        if (se_status != SE_STATUS_OK) {
            LOG_E("Applet Selection Failed");
            goto exit;
        }
        se_status = Se_API_WrapData(pRds->pSusPlaindRds, wrappedRDS, &wrappedRDSLen);
        if (se_status != SE_STATUS_OK) {
            LOG_E("Se API WrapData failed");
            goto exit;
        }
        rdslen  = wrappedRDSLen;
        pRdsBuf = &wrappedRDS[0];
    } break;
    case SE_APPLET_FIRALITE: {
        /* Neglect first 6 bytes as disapatch is coded as
         * 04 SessionId, TotalLen, Tag LenId SessionID .......*/
        rdslen    = (pRds->pFlWrappedRds->wrappedRDSLen) - 6;
        pRdsBuf   = &(pRds->pFlWrappedRds->wrappedRDS[6]);
        se_status = SE_STATUS_OK;
    } break;
    default:
        LOG_E("Wrong Applet given, applet should be FireLite or SUS Client");
        goto exit;
    }
    len    = *pWrappedRdsLen;
    tlvRet = tlvGet_u8buf(pRdsBuf, &offset, rdslen, kSE05x_SUS_TAG_SESSION_ID, &pWrappedRds[rdsOffset], &len);
    if (tlvRet != 0) {
        goto exit;
    }
    rdsOffset += len;
    len    = *pWrappedRdsLen - len;
    tlvRet = tlvGet_u8buf(pRdsBuf, &offset, rdslen, kSE05x_SUS_TAG_RANDOM_NUM, &pWrappedRds[rdsOffset], &len);
    if (tlvRet != 0) {
        goto exit;
    }
    rdsOffset += len;
    len    = *pWrappedRdsLen - len;
    tlvRet = tlvGet_u8buf(pRdsBuf, &offset, rdslen, kSE05x_SUS_TAG_WRDS, &pWrappedRds[rdsOffset], &len);
    if (tlvRet != 0) {
        goto exit;
    }
    rdsOffset += len;
    *pWrappedRdsLen = rdsOffset;
exit:
    if (applet == SE_APPLET_SUS_CLIENT) {
        if (Se_API_DeInit() != SE_STATUS_OK) {
            LOG_E("Se API DeInit failed");
            se_status = SE_STATUS_NOT_OK;
        }
    }
    return se_status;
}

se_status_t Se_API_WrapData(se_plainRDS *pRdsData, uint8_t *pWrappedRds, size_t *pWrappedRdsLen)
{
    se_status_t status     = SE_STATUS_NOT_OK;
    smStatus_t smstatus    = SM_NOT_OK;
    pSusSession_t pSus_ctx = &gsus_ctx;

    ENSURE_OR_GO_EXIT(pRdsData);
    ENSURE_OR_GO_EXIT(pWrappedRds);
    ENSURE_OR_GO_EXIT(pWrappedRdsLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    smstatus = SUS_API_WrapData(pSus_ctx->se_ctx, pRdsData, pWrappedRds, pWrappedRdsLen);
    if (smstatus == SM_OK) {
        status = SE_STATUS_OK;
    }
    else {
        LOG_E("Se_API_WrapData failed !!!");
    }

exit:
    return status;
}

se_status_t Se_API_SendReceive(uint8_t *pInData, size_t InDataLen, uint8_t *pOutData, size_t *pOutDataLen)
{
    uint32_t retStatus     = SM_NOT_OK;
    se_status_t status     = SE_STATUS_NOT_OK;
    pSusSession_t pSus_ctx = &gsus_ctx;
    U32 u32RXLen           = (U32)*pOutDataLen;

    ENSURE_OR_GO_EXIT(pInData);
    ENSURE_OR_GO_EXIT(pOutData);
    ENSURE_OR_GO_EXIT(pOutDataLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    retStatus = smCom_TransceiveRaw(pSus_ctx->se_ctx->conn_ctx, pInData, (U16)InDataLen, pOutData, &u32RXLen);
    if ((retStatus == SM_OK) && (u32RXLen >= 2)) {
        retStatus    = (pOutData[u32RXLen - 2] << 8) | (pOutData[u32RXLen - 1]);
        *pOutDataLen = u32RXLen - 2;
        if (retStatus == SM_OK) {
            status = SE_STATUS_OK;
        }

        LOG_AU8_I(pOutData, *pOutDataLen);
    }
    else {
        LOG_E("Se_API_SendReceive failed !!!");
    }

exit:
    return status;
}

se_status_t Se_API_DeInit(void)
{
    pSusSession_t pSus_ctx = &gsus_ctx;
    uint8_t closeCmd[]     = {0x00, 0x70, 0x80, pSus_ctx->se_ctx->logical_channel};
    uint32_t retStatus     = SM_NOT_OK;
    se_status_t status     = SE_STATUS_NOT_OK;
    U16 closeCmdLen        = sizeof(closeCmd);
    uint8_t closeResp[32]  = {0x00};
    U32 closeRespLen       = sizeof(closeResp);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return status;
    }

    /* close the channel */
    retStatus =
        smCom_TransceiveRaw(pSus_ctx->se_ctx->conn_ctx, (uint8_t *)closeCmd, closeCmdLen, closeResp, &closeRespLen);
    if ((retStatus == SM_OK) && (closeRespLen == 2)) {
        retStatus = (closeResp[closeRespLen - 2] << 8) | (closeResp[closeRespLen - 1]);
        if (retStatus == SM_OK) {
            status = SE_STATUS_OK;
        }
        else {
            LOG_E("Unable to close Logical Channel 1");
        }
    }

    return status;
}

se_status_t Se_API_SelectADF(const se_firalite_optsa_t optsA,
    se_firelite_oid_entry *pOid_entries,
    const size_t oid_entries_count,
    se_firelite_selectadf_reponse_t *pResponse)
{
    se_status_t retVal                        = SE_STATUS_NOT_OK;
    se_status_t seStatus                      = SE_STATUS_NOT_OK;
    uint8_t rspBuf[FIRALITE_MAX_BUF_SIZE_RSP] = {0u};
    smStatus_t smStatus                       = SM_NOT_OK;
    pSusSession_t pSus_ctx                    = &gsus_ctx;
    size_t rspLen                             = sizeof(rspBuf);

    ENSURE_OR_GO_EXIT(pOid_entries);
    ENSURE_OR_GO_EXIT(oid_entries_count);
    ENSURE_OR_GO_EXIT(pResponse);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus = se_FiRaLite_API_SelectADF(pSus_ctx->se_ctx, optsA, pOid_entries, oid_entries_count, rspBuf, &rspLen);

    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    seStatus = Se_parseRspSelectADF(rspBuf, rspLen, pResponse);

    ENSURE_OR_GO_EXIT(SE_STATUS_OK == seStatus);

    retVal = SE_STATUS_OK;

exit:

    return retVal;
}

se_status_t Se_API_InitiateTransaction(se_firelite_oid_entry *pOid_entries,
    const size_t oid_entries_count,
    uint8_t *pSessionId,
    const size_t sessionIdLen,
    uint8_t *pStatus,
    uint8_t *pDataBuffer,
    size_t *pDataLen)
{
    se_status_t retVal                        = SE_STATUS_NOT_OK;
    se_status_t seStatus                      = SE_STATUS_NOT_OK;
    uint8_t rspBuf[FIRALITE_MAX_BUF_SIZE_RSP] = {0u};
    smStatus_t smStatus                       = SM_NOT_OK;
    pSusSession_t pSus_ctx                    = &gsus_ctx;
    size_t rspLen                             = sizeof(rspBuf);

    ENSURE_OR_GO_EXIT(pOid_entries);
    ENSURE_OR_GO_EXIT(oid_entries_count);
    ENSURE_OR_GO_EXIT(pStatus);
    ENSURE_OR_GO_EXIT(pDataBuffer);
    ENSURE_OR_GO_EXIT(pDataLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus = se_FiRaLite_API_InitiateTransaction(
        pSus_ctx->se_ctx, pOid_entries, oid_entries_count, pSessionId, sessionIdLen, rspBuf, &rspLen);

    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    seStatus = Se_parseRspInitiateTransaction(rspBuf, rspLen, pStatus, pDataBuffer, pDataLen);

    ENSURE_OR_GO_EXIT(SE_STATUS_OK == seStatus);

    retVal = SE_STATUS_OK;

exit:
    return retVal;
}

se_status_t Se_API_Dispatch(uint8_t *pDispatchData,
    size_t dispatchDataLen,
    uint8_t *pStatus,
    uint8_t *pDataBuffer,
    size_t *pDataLen,
    uint8_t *pEventId,
    uint8_t *pEventDataBuffer,
    size_t *pEventDataLen)
{
    se_status_t retVal                        = SE_STATUS_NOT_OK;
    se_status_t seStatus                      = SE_STATUS_NOT_OK;
    uint8_t rspBuf[FIRALITE_MAX_BUF_SIZE_RSP] = {0u};
    smStatus_t smStatus                       = SM_NOT_OK;
    pSusSession_t pSus_ctx                    = &gsus_ctx;
    size_t rspLen                             = sizeof(rspBuf);

    ENSURE_OR_GO_EXIT(pDispatchData);
    ENSURE_OR_GO_EXIT(pStatus);
    ENSURE_OR_GO_EXIT(pDataBuffer);
    ENSURE_OR_GO_EXIT(pDataLen);
    ENSURE_OR_GO_EXIT(pEventId);
    ENSURE_OR_GO_EXIT(pEventDataBuffer);
    ENSURE_OR_GO_EXIT(pEventDataLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus = se_FiRaLite_API_Dispatch(pSus_ctx->se_ctx, pDispatchData, dispatchDataLen, rspBuf, &rspLen);

    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    seStatus =
        Se_parseRspDispatch(rspBuf, rspLen, pStatus, pDataBuffer, pDataLen, pEventId, pEventDataBuffer, pEventDataLen);

    ENSURE_OR_GO_EXIT(SE_STATUS_OK == seStatus);

    retVal = SE_STATUS_OK;

exit:
    return retVal;
}

se_status_t Se_API_RemoteGetData(
    const uint8_t *pInBuf, const size_t inBufLen, uint8_t *pStatus, uint8_t *pDataBuffer, size_t *pDataLen)
{
    se_status_t retVal                        = SE_STATUS_NOT_OK;
    se_status_t seStatus                      = SE_STATUS_NOT_OK;
    uint8_t cmdBuf[FIRALITE_MAX_BUF_SIZE_CMD] = {0u};
    uint8_t rspBuf[FIRALITE_MAX_BUF_SIZE_RSP] = {0u};
    smStatus_t smStatus                       = SM_NOT_OK;
    pSusSession_t pSus_ctx                    = &gsus_ctx;
    size_t cmdLen                             = sizeof(cmdBuf);
    size_t rspLen                             = sizeof(rspBuf);

    ENSURE_OR_GO_EXIT(pInBuf);
    ENSURE_OR_GO_EXIT(inBufLen != 0);
    ENSURE_OR_GO_EXIT(pStatus);
    ENSURE_OR_GO_EXIT(pDataBuffer);
    ENSURE_OR_GO_EXIT(pDataLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus = se_FiRaLite_API_RemoteGetData(pInBuf, inBufLen, cmdBuf, &cmdLen);

    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    smStatus = se_FiRaLite_API_Tunnel(pSus_ctx->se_ctx, cmdBuf, cmdLen, rspBuf, &rspLen);
    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    seStatus = Se_parseRspTunnel(rspBuf, rspLen, pStatus, pDataBuffer, pDataLen);

    ENSURE_OR_GO_EXIT(SE_STATUS_OK == seStatus);

    retVal = SE_STATUS_OK;

exit:
    return retVal;
}

se_status_t Se_API_RemotePutData(
    const uint8_t *pInBuf, const size_t inBufLen, uint8_t *pStatus, uint8_t *pDataBuffer, size_t *pDataLen)
{
    se_status_t retVal                        = SE_STATUS_NOT_OK;
    se_status_t seStatus                      = SE_STATUS_NOT_OK;
    uint8_t cmdBuf[FIRALITE_MAX_BUF_SIZE_CMD] = {0u};
    uint8_t rspBuf[FIRALITE_MAX_BUF_SIZE_RSP] = {0u};
    smStatus_t smStatus                       = SM_NOT_OK;
    pSusSession_t pSus_ctx                    = &gsus_ctx;
    size_t cmdLen                             = sizeof(cmdBuf);
    size_t rspLen                             = sizeof(rspBuf);

    ENSURE_OR_GO_EXIT(pInBuf);
    ENSURE_OR_GO_EXIT(inBufLen != 0);
    ENSURE_OR_GO_EXIT(pStatus);
    ENSURE_OR_GO_EXIT(pDataBuffer);
    ENSURE_OR_GO_EXIT(pDataLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus = se_FiRaLite_API_RemotePutData(pInBuf, inBufLen, cmdBuf, &cmdLen);

    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    smStatus = se_FiRaLite_API_Tunnel(pSus_ctx->se_ctx, cmdBuf, cmdLen, rspBuf, &rspLen);
    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    seStatus = Se_parseRspTunnel(rspBuf, rspLen, pStatus, pDataBuffer, pDataLen);

    ENSURE_OR_GO_EXIT(SE_STATUS_OK == seStatus);

    retVal = SE_STATUS_OK;

exit:
    return retVal;
}

se_status_t Se_API_LocalGetData(const uint8_t tagp1, const uint8_t tagp2, uint8_t *pDataBuffer, size_t *pDataLen)
{
    se_status_t retVal     = SE_STATUS_NOT_OK;
    smStatus_t smStatus    = SM_NOT_OK;
    pSusSession_t pSus_ctx = &gsus_ctx;

    ENSURE_OR_GO_EXIT(pDataBuffer);
    ENSURE_OR_GO_EXIT(pDataLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus = se_FiRaLite_API_LocalGetData(pSus_ctx->se_ctx, tagp1, tagp2, pDataBuffer, pDataLen);

    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    retVal = SE_STATUS_OK;

exit:
    return retVal;
}

se_status_t Se_API_LocalPutData(const uint8_t *pInBuf, const size_t inBufLen)
{
    se_status_t retVal     = SE_STATUS_NOT_OK;
    smStatus_t smStatus    = SM_NOT_OK;
    pSusSession_t pSus_ctx = &gsus_ctx;

    ENSURE_OR_GO_EXIT(pInBuf);
    ENSURE_OR_GO_EXIT(inBufLen != 0);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus = se_FiRaLite_API_LocalPutData(pSus_ctx->se_ctx, pInBuf, inBufLen);

    ENSURE_OR_GO_EXIT(SM_OK == smStatus);

    retVal = SE_STATUS_OK;

exit:
    return retVal;
}

se_status_t Se_API_RemotePutData_WithoutTunnel(
    const uint8_t *pInBuf, const size_t inBufLen, uint8_t *pRspBuffer, size_t *pRspBufLen)
{
    se_status_t retVal = SE_STATUS_NOT_OK;
    smStatus_t smStatus;
    pSusSession_t pSus_ctx = &gsus_ctx;
    size_t cmdLen          = *pRspBufLen;

    ENSURE_OR_GO_EXIT(pInBuf);
    ENSURE_OR_GO_EXIT(inBufLen != 0);
    ENSURE_OR_GO_EXIT(pRspBuffer);
    ENSURE_OR_GO_EXIT(pRspBufLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus    = se_FiRaLite_API_RemotePutData(pInBuf, inBufLen, pRspBuffer, &cmdLen);
    *pRspBufLen = cmdLen;

    if (SM_OK == smStatus) {
        retVal = SE_STATUS_OK;
    }
exit:
    return retVal;
}

se_status_t Se_API_RemoteGetData_WithoutTunnel(
    const uint8_t *pInBuf, const size_t inBufLen, uint8_t *pRspBuffer, size_t *pRspBufLen)
{
    se_status_t retVal = SE_STATUS_NOT_OK;
    smStatus_t smStatus;
    pSusSession_t pSus_ctx = &gsus_ctx;
    size_t cmdLen          = *pRspBufLen;

    ENSURE_OR_GO_EXIT(pInBuf);
    ENSURE_OR_GO_EXIT(inBufLen != 0);
    ENSURE_OR_GO_EXIT(pRspBuffer);
    ENSURE_OR_GO_EXIT(pRspBufLen);

    if (Se_checkHandle(pSus_ctx->se_ctx)) {
        return retVal;
    }

    smStatus    = se_FiRaLite_API_RemoteGetData(pInBuf, inBufLen, pRspBuffer, &cmdLen);
    *pRspBufLen = cmdLen;

    if (SM_OK == smStatus) {
        retVal = SE_STATUS_OK;
    }
exit:
    return retVal;
}

static se_status_t Se_parseRspSelectADF(
    uint8_t *pBuffer, const size_t bufferLen, se_firelite_selectadf_reponse_t *pResponse)
{
    se_status_t retVal = SE_STATUS_NOT_OK;
    int tlvRet;
    size_t rspIndex = 0;

    ENSURE_OR_GO_CLEANUP(pBuffer);
    ENSURE_OR_GO_CLEANUP(pResponse);

    tlvRet = tlvGet_ValueIndex(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_FCI_TEMPLATE);
    if (0 != tlvRet) {
        goto cleanup;
    }

    pResponse->aidLen = sizeof(pResponse->pAid);
    tlvRet = tlvGet_u8buf(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_AID, pResponse->pAid, &pResponse->aidLen);
    if (0 != tlvRet) {
        goto cleanup;
    }

    if (rspIndex < bufferLen) {
        /* optional proprietary information */
        size_t propLen = sizeof(pResponse->pPropInfo);
        tlvRet         = tlvGet_u8buf(
            pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_PROPRIETARY, pResponse->pPropInfo, &propLen);
        if (0 != tlvRet || propLen != sizeof(pResponse->pPropInfo)) {
            goto cleanup;
        }

        pResponse->is_privacy_enabled = TRUE;
    }
    else {
        pResponse->is_privacy_enabled = FALSE;
    }

    if (rspIndex == bufferLen) {
        retVal = SE_STATUS_OK;
    }

cleanup:
    return retVal;
}

static se_status_t Se_parseRspInitiateTransaction(
    uint8_t *pBuffer, const size_t bufferLen, uint8_t *pStatus, uint8_t *pDataBuffer, size_t *pDataLen)
{
    se_status_t retVal = SE_STATUS_NOT_OK;
    int tlvRet;
    size_t rspIndex = 0;

    ENSURE_OR_GO_CLEANUP(pBuffer);
    ENSURE_OR_GO_CLEANUP(pStatus);
    ENSURE_OR_GO_CLEANUP(pDataBuffer);
    ENSURE_OR_GO_CLEANUP(pDataLen);

    tlvRet = tlvGet_ValueIndex(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_PROP_RSP_TEMPLATE);
    if (0 != tlvRet) {
        goto cleanup;
    }

    tlvRet = tlvGet_U8(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_STATUS, pStatus);
    if (0 != tlvRet) {
        goto cleanup;
    }

    tlvRet =
        tlvGet_u8buf(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_COMMAND_OR_RESPONSE, pDataBuffer, pDataLen);
    if (0 != tlvRet) {
        goto cleanup;
    }

    if (rspIndex == bufferLen) {
        retVal = SE_STATUS_OK;
    }

cleanup:
    return retVal;
}

static se_status_t Se_parseRspDispatch(uint8_t *pBuffer,
    const size_t bufferLen,
    uint8_t *pStatus,
    uint8_t *pDataBuffer,
    size_t *pDataLen,
    uint8_t *pEventId,
    uint8_t *pEventDataBuffer,
    size_t *pEventDataLen)
{
    se_status_t retVal = SE_STATUS_NOT_OK;
    int tlvRet;
    size_t rspIndex = 0;

    ENSURE_OR_GO_CLEANUP(pStatus);
    ENSURE_OR_GO_CLEANUP(pDataBuffer);
    ENSURE_OR_GO_CLEANUP(pDataLen);
    ENSURE_OR_GO_CLEANUP(pEventId);
    ENSURE_OR_GO_CLEANUP(pEventDataBuffer);
    ENSURE_OR_GO_CLEANUP(pEventDataLen);

    tlvRet = tlvGet_ValueIndex(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_PROP_RSP_TEMPLATE);
    if (0 != tlvRet) {
        goto cleanup;
    }

    tlvRet = tlvGet_U8(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_STATUS, pStatus);
    if (0 != tlvRet) {
        goto cleanup;
    }

    if (rspIndex < bufferLen) {
        /* optional part */
        if (kSE05x_FIRALITE_TAG_COMMAND_OR_RESPONSE == pBuffer[rspIndex]) {
            /* optional data */
            tlvRet = tlvGet_u8buf(
                pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_COMMAND_OR_RESPONSE, pDataBuffer, pDataLen);
            if (0 != tlvRet) {
                goto cleanup;
            }
        }

        if (rspIndex < bufferLen) {
            uint8_t notifFormat;

            /* optional notification */
            tlvRet = tlvGet_ValueIndex(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_NOTIFICATION);
            if (0 != tlvRet) {
                goto cleanup;
            }

            tlvRet = tlvGet_U8(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_NOTIFICATION_FORMAT, &notifFormat);
            if (0 != tlvRet) {
                goto cleanup;
            }

            tlvRet = tlvGet_U8(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_EVENT_ID, pEventId);
            if (0 != tlvRet) {
                goto cleanup;
            }

            if (rspIndex < bufferLen) {
                /* optional event data */
                tlvRet = tlvGet_u8buf(
                    pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_EVENT_DATA, pEventDataBuffer, pEventDataLen);
                if (0 != tlvRet) {
                    goto cleanup;
                }
            }
        }
    }

    if (rspIndex == bufferLen) {
        retVal = SE_STATUS_OK;
    }

cleanup:
    return retVal;
}

static se_status_t Se_parseRspTunnel(
    uint8_t *pBuffer, const size_t bufferLen, uint8_t *pStatus, uint8_t *pDataBuffer, size_t *pDataLen)
{
    se_status_t retVal = SE_STATUS_NOT_OK;
    int tlvRet;
    size_t rspIndex = 0;

    ENSURE_OR_GO_CLEANUP(pBuffer);
    ENSURE_OR_GO_CLEANUP(pStatus);
    ENSURE_OR_GO_CLEANUP(pDataBuffer);
    ENSURE_OR_GO_CLEANUP(pDataLen);
    ENSURE_OR_GO_CLEANUP(bufferLen > 0u);

    tlvRet = tlvGet_ValueIndex(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_PROP_RSP_TEMPLATE);
    if (0 != tlvRet) {
        goto cleanup;
    }

    tlvRet = tlvGet_U8(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_STATUS, pStatus);
    if (0 != tlvRet) {
        goto cleanup;
    }

    tlvRet =
        tlvGet_u8buf(pBuffer, &rspIndex, bufferLen, kSE05x_FIRALITE_TAG_COMMAND_OR_RESPONSE, pDataBuffer, pDataLen);
    if (0 != tlvRet) {
        goto cleanup;
    }

    if (rspIndex == bufferLen) {
        retVal = SE_STATUS_OK;
    }

cleanup:
    return retVal;
}
