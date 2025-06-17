/*
 *
 * Copyright 2021-2022,2023 NXP.
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
#include "UwbApi_Utility.h"
#include "uci_ext_defs.h"
#include "phOsalUwb.h"
#include "phNxpLogApis_UwbApi.h"

/*******************************************************************************
**
** Function         sendUciCommandAndWait
**
** Description      Send Uci command and wait on semaphore
**
** Returns          UWBAPI_STATUS_OK on success
**                  UWBAPI_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWBAPI_STATUS sendUciCommandAndWait(uint16_t event, uint16_t cmdLen, uint8_t *pCmd)
{
    tUWBAPI_STATUS status;

    status = UWA_SendUciCommand(event, cmdLen, pCmd, 0);

    if (UWBAPI_STATUS_OK == status) {
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT) !=
            UWBSTATUS_SUCCESS) {
            LOG_E("%s : event timedout", __FUNCTION__);
            return UWBAPI_STATUS_TIMEOUT;
        }
        status = uwbContext.wstatus;
    }
    return status;
}

/*******************************************************************************
**
** Function         sendUciCommand
**
** Description      Send Uci command
**
** Returns          UWBAPI_STATUS_OK on success
**                  UWBAPI_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWBAPI_STATUS sendUciCommand(uint16_t event, uint16_t cmdLen, uint8_t *pCmd, uint8_t pbf)
{
    tUWBAPI_STATUS status;

    status = UWA_SendUciCommand(event, cmdLen, pCmd, pbf);

    if (UWBAPI_STATUS_OK == status) {
        status = uwbContext.wstatus;
    }
    return status;
}

/*******************************************************************************
**
** Function         serializeSessionInitPayload
**
** Description      serialize Session Init Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeSessionInitPayload(uint32_t sessionHandle, eSessionType sessionType, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT32_TO_STREAM(pCmdBuf, sessionHandle);
    offset = (uint16_t)(offset + sizeof(sessionHandle));

    UWB_UINT8_TO_STREAM(pCmdBuf, sessionType);
    offset = (uint16_t)(offset + sizeof(uint8_t));
    return offset;
}

/*******************************************************************************
**
** Function         serializeGetCoreConfigPayload
**
** Description      serialize Get Core Config Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeGetCoreConfigPayload(uint8_t noOfParams, uint8_t paramLen, uint8_t *paramId, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    pCmdBuf[0]      = noOfParams;
    offset          = (uint16_t)(offset + sizeof(noOfParams) + paramLen);
    if (noOfParams > 0) {
        phOsalUwb_MemCopy(&pCmdBuf[1], paramId, paramLen);
    }
    return offset;
}

/*******************************************************************************
**
** Function         serializeSessionHandlePayload
**
** Description      serialize Session Handle Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeSessionHandlePayload(uint32_t sessionHandle, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT32_TO_STREAM(pCmdBuf, sessionHandle);
    offset = (uint16_t)(offset + sizeof(sessionHandle));
    return offset;
}

/*******************************************************************************
**
** Function         serializeAppConfigPayload
**
** Description      serialize App Config Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeAppConfigPayload(uint32_t sessionHandle, uint8_t noOfParams, uint16_t paramLen, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT32_TO_STREAM(pCmdBuf, sessionHandle);
    offset = (uint16_t)(offset + sizeof(sessionHandle));

    UWB_UINT8_TO_STREAM(pCmdBuf, noOfParams);
    offset = (uint16_t)(offset + sizeof(noOfParams) + paramLen);

    return offset;
}

/*******************************************************************************
**
** Function         serializeUpdateControllerMulticastListPayload
**
** Description      serialize Update Controller MulticastList Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeUpdateControllerMulticastListPayload(
    phMulticastControleeListContext_t *pControleeContext, uint8_t *pCmdBuf)
{
    uint16_t offset              = 0;
    uint16_t subSessionKeyOffset = 0;
    UWB_UINT32_TO_STREAM(pCmdBuf, pControleeContext->sessionHandle);
    offset = (uint16_t)(offset + sizeof(pControleeContext->sessionHandle));

    UWB_UINT8_TO_STREAM(pCmdBuf, pControleeContext->action);
    offset = (uint16_t)(offset + sizeof(pControleeContext->action));

    UWB_UINT8_TO_STREAM(pCmdBuf, pControleeContext->no_of_controlees);
    offset = (uint16_t)(offset + sizeof(pControleeContext->no_of_controlees));

    if (pControleeContext->no_of_controlees > 0) {
        for (uint8_t i = 0; i < pControleeContext->no_of_controlees; i++) {
            UWB_UINT16_TO_STREAM(pCmdBuf, pControleeContext->controlee_list[i].short_address);
            UWB_UINT32_TO_STREAM(pCmdBuf, pControleeContext->controlee_list[i].subsession_id);

            if (pControleeContext->action == KUWB_Add16BSubSessionKey) {
                UWB_ARRAY_TO_STREAM(
                    pCmdBuf, pControleeContext->controlee_list[i].subsession_key, SUB_SESSION_KEY_LEN_16B);
                subSessionKeyOffset += SUB_SESSION_KEY_LEN_16B;
            }
            else if (pControleeContext->action == KUWB_Add32BSubSessionKey) {
                UWB_ARRAY_TO_STREAM(
                    pCmdBuf, pControleeContext->controlee_list[i].subsession_key, SUB_SESSION_KEY_LEN_32B);
                subSessionKeyOffset += SUB_SESSION_KEY_LEN_32B;
            }
        }
        offset = (uint16_t)(offset + pControleeContext->no_of_controlees * SHORT_ADDRESS_LEN +
                            pControleeContext->no_of_controlees * SUBSESSION_HANDLE_LEN + subSessionKeyOffset);
    }

    return offset;
}

/*******************************************************************************
**
** Function         serializeTestDataPayload
**
** Description      serialize Test Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeTestDataPayload(uint16_t psduLen, uint8_t psduData[], uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    if (psduLen > 0) {
        phOsalUwb_MemCopy(pCmdBuf, psduData, psduLen);
        offset = (uint16_t)(offset + psduLen);
    }
    return offset;
}
#if UWBIOT_UWBD_SR1XXT_SR2XXT
/*******************************************************************************
**
** Function         serializedoVcoPllCalibPayload
**
** Description      serialize do Calib Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializedoVcoPllCalibPayload(uint8_t channel, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT8_TO_STREAM(pCmdBuf, channel);
    offset = (uint16_t)(offset + sizeof(channel));
    return offset;
}

/*******************************************************************************
**
** Function         serializeSetCalibPayload
**
** Description      serialize Set Calib Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeSetCalibPayload(
    uint8_t channel, eCalibParam paramId, uint8_t *calibrationValue, uint16_t length, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT8_TO_STREAM(pCmdBuf, channel);
    offset = (uint16_t)(offset + sizeof(channel));

    if (((paramId >> 8) & 0xFF) != EXTENTED_CALIB_PARAM_ID){
        UWB_UINT8_TO_STREAM(pCmdBuf, paramId);
        offset = (uint16_t)(offset + sizeof(uint8_t));

        UWB_UINT8_TO_STREAM(pCmdBuf, length);
        offset = (uint16_t)(offset + sizeof(uint8_t));
    }
#if UWBIOT_UWBD_SR150
    else{
        UWB_UINT8_TO_STREAM(pCmdBuf, (paramId>>8));
        UWB_UINT8_TO_STREAM(pCmdBuf, (paramId & 0x00FF));
        offset = (uint16_t)(offset + sizeof(uint16_t));

        UWB_UINT16_TO_STREAM(pCmdBuf, length);
        offset = (uint16_t)(offset + sizeof(uint16_t));
    }
#endif // UWBIOT_UWBD_SR150

    UWB_ARRAY_TO_STREAM(pCmdBuf, calibrationValue, length);
    offset = (uint16_t)(offset + length);

    return offset;
}

/*******************************************************************************
**
** Function         serializeGetCalibPayload
**
** Description      serialize Get Calib Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeGetCalibPayload(uint8_t channel, eCalibParam paramId, uint8_t rxAntenaPair, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT8_TO_STREAM(pCmdBuf, channel);
    offset = (uint16_t)(offset + sizeof(channel));

    UWB_UINT8_TO_STREAM(pCmdBuf, paramId);
    offset = (uint16_t)(offset + sizeof(uint8_t));

#if UWBIOT_UWBD_SR1XXT
    if (paramId == AOA_ANTENNAS_PDOA_CALIB) {
        UWB_UINT8_TO_STREAM(pCmdBuf, rxAntenaPair);
        offset = (uint16_t)(offset + sizeof(uint8_t));
    }
    else
#endif
    {
        PHUWB_UNUSED(rxAntenaPair);
    }

    return offset;
}

#if UWBIOT_UWBD_SR150
/*******************************************************************************
**
** Function         serializeGetExtCalibPayload
**
** Description      serialize Get Ext Calib Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeGetExtCalibPayload(uint8_t channel, eCalibParam paramId, uint8_t rxAntenaPair, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    uint16_t paramSubId;
    /**
     * 1:  1 Byte channel Id
     * 2:  2 Bytes Param Id (Ext Id E0 + 00 subParamId)
     * 3:  1 Byte rxAntenaPair Based on the Use case
     */
    UWB_UINT8_TO_STREAM(pCmdBuf, channel);
    offset = (uint16_t)(offset + sizeof(channel));

    UWB_UINT8_TO_STREAM(pCmdBuf, (paramId>>8));
    paramSubId = (paramId & 0x00FF);
    UWB_UINT8_TO_STREAM(pCmdBuf, (paramId & 0x00FF));
    offset = (uint16_t)(offset + sizeof(uint16_t));
    switch(paramSubId){
        case UCI_EXT_PARAM_ID_AOA_ANTENNAS_PDOA_CALIB:{
            UWB_UINT8_TO_STREAM(pCmdBuf, rxAntenaPair);
            offset = (uint16_t)(offset + sizeof(uint8_t));
        }break;
        default :{
        	PHUWB_UNUSED(rxAntenaPair);
        }break;
    }

    return offset;
}
#endif // UWBIOT_UWBD_SR150
#if (UWBFTR_SE_SN110)
/*******************************************************************************
**
** Function         serializeSeLoopTestPayload
**
** Description      serialize Se Loop Test Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeSeLoopTestPayload(uint16_t loopCnt, uint16_t timeInterval, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT16_TO_STREAM(pCmdBuf, loopCnt);
    offset = (uint16_t)(offset + sizeof(loopCnt));

    UWB_UINT16_TO_STREAM(pCmdBuf, timeInterval);
    offset = (uint16_t)(offset + sizeof(timeInterval));

    return offset;
}
#endif //(UWBFTR_SE_SN110)
#if (UWBIOT_UWBD_SR100T)
/*******************************************************************************
**
** Function         serializecalibIntegrityProtectionPayload
**
** Description      serialize calib Integrity Protection Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializecalibIntegrityProtectionPayload(eCalibTagOption tagOption, uint16_t calibBitMask, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT8_TO_STREAM(pCmdBuf, tagOption);
    offset = (uint16_t)(offset + sizeof(uint8_t));

    UWB_UINT16_TO_STREAM(pCmdBuf, calibBitMask);
    offset = (uint16_t)(offset + sizeof(calibBitMask));
    return offset;
}

/*******************************************************************************
**
** Function         serializeVerifyCalibDataPayload
**
** Description      serialize Verify Calib Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeVerifyCalibDataPayload(uint8_t *pCmacTag, uint8_t tagOption, uint16_t tagVersion, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_ARRAY_TO_STREAM(pCmdBuf, pCmacTag, UCI_TAG_CMAC_LENGTH);
    offset = (uint16_t)(offset + UCI_TAG_CMAC_LENGTH);

    UWB_UINT8_TO_STREAM(pCmdBuf, tagOption);
    offset = (uint16_t)(offset + sizeof(tagOption));

    UWB_UINT16_TO_STREAM(pCmdBuf, tagVersion);
    offset = (uint16_t)(offset + sizeof(tagVersion));
    return offset;
}

/*******************************************************************************
**
** Function         serializeConfigureAuthTagOptionsPayload
**
** Description      serialize Configure Auth Tag Options Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeConfigureAuthTagOptionsPayload(
    uint8_t deviceTag, uint8_t modelTag, uint16_t labelValue, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT8_TO_STREAM(pCmdBuf, deviceTag);
    offset = (uint16_t)(offset + sizeof(deviceTag));

    UWB_UINT8_TO_STREAM(pCmdBuf, modelTag);
    offset = (uint16_t)(offset + sizeof(modelTag));

    UWB_UINT16_TO_STREAM(pCmdBuf, labelValue);
    offset = (uint16_t)(offset + sizeof(labelValue));

    return offset;
}

/*******************************************************************************
**
** Function         serializeConfigureAuthTagVersionPayload
**
** Description      serialize Configure Auth Tag Version Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeConfigureAuthTagVersionPayload(uint16_t labelValue, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT16_TO_STREAM(pCmdBuf, labelValue);
    offset = (uint16_t)(offset + sizeof(labelValue));

    return offset;
}
#endif //UWBIOT_UWBD_SR100T
#if (UWBFTR_SE_SN110)
/*******************************************************************************
**
** Function         serializeUrskDeletionRequestPayload
**
** Description      serialize Ursk Deletion Request Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeUrskDeletionRequestPayload(uint8_t noOfSessionHandle, uint32_t *pSessionHandleList, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT8_TO_STREAM(pCmdBuf, noOfSessionHandle);
    offset = (uint16_t)(offset + sizeof(noOfSessionHandle));

    for (int i = 0; i < noOfSessionHandle; i++) {
        UWB_UINT32_TO_STREAM(pCmdBuf, pSessionHandleList[i]);
        offset = (uint16_t)(offset + sizeof(uint32_t));
    }

    return offset;
}
#endif //(UWBFTR_SE_SN110)

/*******************************************************************************
**
** Function         serializeWriteOtpCalibDataPayload
**
** Description      serialize Write Otp Calib Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeWriteOtpCalibDataPayload(
    uint8_t channel, uint8_t writeOption, uint8_t writeDataLen, uint8_t *writeData, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT8_TO_STREAM(pCmdBuf, channel);
    offset = (uint16_t)(offset + sizeof(channel));

    UWB_UINT8_TO_STREAM(pCmdBuf, writeOption);
    offset = (uint16_t)(offset + sizeof(writeOption));

    UWB_UINT8_TO_STREAM(pCmdBuf, writeDataLen);
    offset = (uint16_t)(offset + sizeof(writeDataLen));

    UWB_ARRAY_TO_STREAM(pCmdBuf, writeData, writeDataLen);
    offset = (uint16_t)(offset + writeDataLen);

    return offset;
}

/*******************************************************************************
**
** Function         serializeReadOtpCalibDataPayload
**
** Description      serialize Read Otp Calib Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeReadOtpCalibDataPayload(
    uint8_t channel, uint8_t readOption, eOtpCalibParam calibParam, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT8_TO_STREAM(pCmdBuf, channel);
    offset = (uint16_t)(offset + sizeof(channel));

    UWB_UINT8_TO_STREAM(pCmdBuf, readOption);
    offset = (uint16_t)(offset + sizeof(readOption));

    UWB_UINT8_TO_STREAM(pCmdBuf, calibParam);
    offset = (uint16_t)(offset + sizeof(uint8_t));

    return offset;
}

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/*******************************************************************************
**
** Function         serializeControllerHusSessionPayload
**
** Description      serialize HUS Session Payload of Controller
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeControllerHusSessionPayload(phControllerHusSessionConfig_t *pHusSessionCfg, uint8_t *pCmdBuf)
{
    uint16_t offset              = 0;
    uint8_t update_time_bytesize = 8;

    UWB_UINT32_TO_STREAM(pCmdBuf, pHusSessionCfg->sessionHandle);
    offset = (uint16_t)(offset + sizeof(pHusSessionCfg->sessionHandle));

    UWB_UINT8_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_count);
    offset = (uint16_t)(offset + sizeof(pHusSessionCfg->phase_count));

    UWB_ARRAY_TO_STREAM(pCmdBuf, pHusSessionCfg->update_time, update_time_bytesize);
    offset = (uint16_t)(offset + update_time_bytesize);

    for (int i = 0; i < pHusSessionCfg->phase_count; i++) {
        UWB_UINT32_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_list[i].phase_sessionHandle);
        offset = (uint16_t)(offset + sizeof(pHusSessionCfg->phase_list[i].phase_sessionHandle));

        UWB_UINT16_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_list[i].start_slot_index);
        offset = (uint16_t)(offset + sizeof(pHusSessionCfg->phase_list[i].start_slot_index));

        UWB_UINT16_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_list[i].end_slot_index);
        offset = (uint16_t)(offset + sizeof(pHusSessionCfg->phase_list[i].end_slot_index));

        UWB_UINT8_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_list[i].phase_participation);
        offset = (uint16_t)(offset + sizeof(pHusSessionCfg->phase_list[i].phase_participation));

        UWB_ARRAY_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_list[i].mac_addr, MAC_SHORT_ADD_LEN);
        offset = (uint16_t)(offset + MAC_SHORT_ADD_LEN);
    }

    return offset;
}

/*******************************************************************************
**
** Function         serializeControleeHusSessionPayload
**
** Description      serialize HUS Session Payload of Controlee
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeControleeHusSessionPayload(phControleeHusSessionConfig_t *pHusSessionCfg, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT32_TO_STREAM(pCmdBuf, pHusSessionCfg->sessionHandle);
    offset = (uint16_t)(offset + sizeof(pHusSessionCfg->sessionHandle));

    UWB_UINT8_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_count);
    offset = (uint16_t)(offset + sizeof(pHusSessionCfg->phase_count));

    for (int i = 0; i < pHusSessionCfg->phase_count; i++) {
        UWB_UINT32_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_list[i].phase_sessionHandle);
        offset = (uint16_t)(offset + sizeof(pHusSessionCfg->phase_list[i].phase_sessionHandle));

        UWB_UINT8_TO_STREAM(pCmdBuf, pHusSessionCfg->phase_list[i].phase_participation);
        offset = (uint16_t)(offset + sizeof(pHusSessionCfg->phase_list[i].phase_participation));
    }

    return offset;
}

/*******************************************************************************
**
** Function         serializeDtpcmPayload
**
** Description      serialize DTPCM Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeDtpcmPayload(phDataTxPhaseConfig_t *phDataTxPhaseCfg, uint8_t *pCmdBuf)
{
    uint16_t offset        = 0;
    uint16_t ranging_slots = 0;

    UWB_UINT32_TO_STREAM(pCmdBuf, phDataTxPhaseCfg->dtpcm_SessionHandle);
    offset = (uint16_t)(offset + sizeof(phDataTxPhaseCfg->dtpcm_SessionHandle));

    UWB_UINT8_TO_STREAM(pCmdBuf, phDataTxPhaseCfg->dtpcm_Repetition);
    offset = (uint16_t)(offset + sizeof(phDataTxPhaseCfg->dtpcm_Repetition));

    UWB_UINT8_TO_STREAM(pCmdBuf, phDataTxPhaseCfg->dataTransferCtrl);
    offset = (uint16_t)(offset + sizeof(phDataTxPhaseCfg->dataTransferCtrl));

    UWB_UINT8_TO_STREAM(pCmdBuf, phDataTxPhaseCfg->dtpml_size);
    offset = (uint16_t)(offset + sizeof(phDataTxPhaseCfg->dtpml_size));

    ranging_slots = GET_RANGING_SLOTS(phDataTxPhaseCfg->dataTransferCtrl);

    for (int i = 0; i < phDataTxPhaseCfg->dtpml_size; i++) {
        /* b0 of dataTransferCtrl indicates the mac addr mode 0: short & 1: extended */
        /* Save the address mode in the context to bve used while copying the mac addr from the command's ntf */
        uwbContext.dtpcm_addr_mode = phDataTxPhaseCfg->dataTransferCtrl & 0x01;
        if (phDataTxPhaseCfg->dataTransferCtrl & 0x01) {
            UWB_ARRAY_TO_STREAM(pCmdBuf, phDataTxPhaseCfg->dtpml[i].mac_addr, MAC_EXT_ADD_LEN);
            offset = (uint16_t)(offset + MAC_EXT_ADD_LEN);
        }
        else {
            UWB_ARRAY_TO_STREAM(pCmdBuf, phDataTxPhaseCfg->dtpml[i].mac_addr, MAC_SHORT_ADD_LEN);
            offset = (uint16_t)(offset + MAC_SHORT_ADD_LEN);
        }
        UWB_ARRAY_TO_STREAM(pCmdBuf, phDataTxPhaseCfg->dtpml[i].slot_bitmap, ranging_slots);
        offset = (uint16_t)(offset + ranging_slots);
    }

    return offset;
}
#endif

#if (UWBFTR_DL_TDoA_Anchor || UWBFTR_DL_TDoA_Tag)
/*******************************************************************************
**
** Function         serializeUpdateActiveRoundsAnchorPayload
**
** Description      serialize update active rounds Anchor Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeUpdateActiveRoundsAnchorPayload(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    UWB_MacAddressMode_t macAddressingMode,
    const phActiveRoundsConfig_t roundConfigList[],
    uint8_t *pCmdBuf)
{
    uint16_t offset           = 0;
    uint16_t dst_mac_add_len  = 0;
    uint16_t responderSlotLen = 0;

    /** adding parameter session ID */
    UWB_UINT32_TO_STREAM(pCmdBuf, sessionHandle);
    offset = (uint16_t)(offset + sizeof(sessionHandle));

    /** adding parameter nActiveRounds */
    UWB_UINT8_TO_STREAM(pCmdBuf, nActiveRounds);
    offset = (uint16_t)(offset + sizeof(nActiveRounds));

    for (int i = 0; i < nActiveRounds; i++) {
        /** adding parameter nActiveRounds */
        UWB_UINT8_TO_STREAM(pCmdBuf, roundConfigList[i].roundIndex);
        offset = (uint16_t)(offset + sizeof(roundConfigList[i].roundIndex));
        /** adding parameter rangingRole */
        UWB_UINT8_TO_STREAM(pCmdBuf, roundConfigList[i].rangingRole);
        offset = (uint16_t)(offset + sizeof(roundConfigList[i].rangingRole));
        /** if ranging role is Initiator adding Subsequent fields */
        if (roundConfigList[i].rangingRole == 1) {
            /** adding parameter noofResponders */
            UWB_UINT8_TO_STREAM(pCmdBuf, roundConfigList[i].noofResponders);
            offset = (uint16_t)(offset + sizeof(roundConfigList[i].noofResponders));
            /** depending upon the macAddressingMode adding  responderMacAddressList*/
            if (macAddressingMode == kUWB_MacAddressMode_2bytes) {
                dst_mac_add_len = MAC_SHORT_ADD_LEN * roundConfigList[i].noofResponders;
            }
            else if (macAddressingMode == kUWB_MacAddressMode_8bytes) {
                dst_mac_add_len = MAC_EXT_ADD_LEN * roundConfigList[i].noofResponders;
            }
            else {
                offset = 0;
                NXPLOG_UWBAPI_E("%s: %d: Invalid MacAddressingMode ", __FUNCTION__, __LINE__);
                break;
            }
            UWB_ARRAY_TO_STREAM(pCmdBuf, roundConfigList[i].responderMacAddressList, dst_mac_add_len);
            offset = (uint16_t)(offset + dst_mac_add_len);
            /** adding parameter responderSlotScheduling
             *  depending upon responderSlotScheduling *responderSlots will be added
            */
            if (roundConfigList[i].responderSlotScheduling == 0) {
                UWB_UINT8_TO_STREAM(pCmdBuf, roundConfigList[i].responderSlotScheduling);
                offset = (uint16_t)(offset + sizeof(uint8_t));
            }
            else if ((roundConfigList[i].responderSlots == NULL) ||
                     (roundConfigList[i].responderMacAddressList == NULL)) {
                offset = 0;
                break;
            }
            else {
                responderSlotLen = roundConfigList[i].noofResponders;
                UWB_ARRAY_TO_STREAM(pCmdBuf, roundConfigList[i].responderSlots, responderSlotLen);
                offset = (uint16_t)(offset + responderSlotLen);
            }
        }
    }

    return offset;
}

/*******************************************************************************
**
** Function         serializeUpdateActiveRoundsReceiverPayload
**
** Description      serialize update active rounds Receiver Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeUpdateActiveRoundsReceiverPayload(
    uint32_t sessionHandle, uint8_t nActiveRounds, const uint8_t roundIndexList[], uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT32_TO_STREAM(pCmdBuf, sessionHandle);
    offset = (uint16_t)(offset + sizeof(sessionHandle));

    UWB_UINT8_TO_STREAM(pCmdBuf, nActiveRounds);
    offset = (uint16_t)(offset + sizeof(nActiveRounds));

    for (int i = 0; i < nActiveRounds; i++) {
        UWB_UINT8_TO_STREAM(pCmdBuf, roundIndexList[i]);
        offset = (uint16_t)(offset + sizeof(roundIndexList[i]));
    }

    return offset;
}
#endif //UWBFTR_DL_TDoA_Anchor || UWBFTR_DL_TDoA_Tag
#endif

/*******************************************************************************
**
** Function         serializeTrngtPayload
**
** Description      serialize Trng payload
**
** Returns          Length of payload
**
*******************************************************************************/
#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
uint16_t serializeTrngtPayload(uint8_t trng_size, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT8_TO_STREAM(pCmdBuf, trng_size);
    offset = (uint16_t)(offset + sizeof(trng_size));
    return offset;
}
#endif //(UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
#if UWBIOT_UWBD_SR040
/*******************************************************************************
**
** Function         serializeSessionNvmPayload
**
** Description      serialize NVM payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeSessionNvmPayload(esessionNvmManage sesNvmManageTag, uint32_t sessionHandle, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    switch (sesNvmManageTag) {
    case SESSION_NVM_MANAGE_PERSIST:
#if 0
    case SESSION_NVM_MANAGE_DELETE: /* Not available */
#endif

        UWB_UINT8_TO_STREAM(pCmdBuf, sesNvmManageTag);
        offset = (uint16_t)(offset + sizeof(uint8_t));
        UWB_UINT32_TO_STREAM(pCmdBuf, sessionHandle);
        offset = (uint16_t)(offset + sizeof(sessionHandle));
        break;
    case SESSION_NVM_MANAGE_DELETE_ALL:
        UWB_UINT8_TO_STREAM(pCmdBuf, sesNvmManageTag);
        offset = (uint16_t)(offset + sizeof(uint8_t));
        break;
    default:
        return offset;
    }

    return offset;
}

#endif

#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
/*******************************************************************************
**
** Function         serializeSetProfileParamsPayload
**
** Description      serialize set profile params payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeSetProfileParamsPayload(
    phUwbProfileInfo_t *pProfileInfo, uint16_t blobSize, uint8_t *pProfileBlob, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_UINT8_TO_STREAM(pCmdBuf, pProfileInfo->profileId);
    offset = (uint16_t)(offset + sizeof(uint8_t));
    UWB_UINT8_TO_STREAM(pCmdBuf, pProfileInfo->deviceType);
    offset = (uint16_t)(offset + sizeof(pProfileInfo->deviceType));
    UWB_ARRAY_TO_STREAM(pCmdBuf, pProfileInfo->mac_addr, MAC_SHORT_ADD_LEN);
    offset = (uint16_t)(offset + MAC_SHORT_ADD_LEN);
    UWB_UINT8_TO_STREAM(pCmdBuf, pProfileInfo->deviceRole);
    offset = (uint16_t)(offset + sizeof(pProfileInfo->deviceRole));
    UWB_ARRAY_TO_STREAM(pCmdBuf, pProfileBlob, blobSize);
    offset = (uint16_t)(offset + blobSize);
    return offset;
}

/*******************************************************************************
**
** Function         serializeUwbDeviceConfigData
**
** Description      serializes device configuration data
**
** Returns          Length of the serialized data
**
*******************************************************************************/
uint16_t serializeUwbDeviceConfigData(UwbDeviceConfigData_t *pUwbDeviceConfig, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;
    UWB_ARRAY_TO_BE_STREAM(pCmdBuf, pUwbDeviceConfig->spec_ver_major, sizeof(pUwbDeviceConfig->spec_ver_major));
    offset = (uint16_t)(offset + sizeof(pUwbDeviceConfig->spec_ver_major));
    UWB_ARRAY_TO_BE_STREAM(pCmdBuf, pUwbDeviceConfig->spec_ver_minor, sizeof(pUwbDeviceConfig->spec_ver_minor));
    offset = (uint16_t)(offset + sizeof(pUwbDeviceConfig->spec_ver_minor));
    UWB_ARRAY_TO_BE_STREAM(pCmdBuf, pUwbDeviceConfig->chip_id, sizeof(pUwbDeviceConfig->chip_id));
    offset = (uint16_t)(offset + sizeof(pUwbDeviceConfig->chip_id));
    UWB_ARRAY_TO_BE_STREAM(pCmdBuf, pUwbDeviceConfig->chip_fw_version, sizeof(pUwbDeviceConfig->chip_fw_version));
    offset = (uint16_t)(offset + sizeof(pUwbDeviceConfig->chip_fw_version));
    UWB_ARRAY_TO_BE_STREAM(pCmdBuf, pUwbDeviceConfig->mw_version, sizeof(pUwbDeviceConfig->mw_version));
    offset = (uint16_t)(offset + sizeof(pUwbDeviceConfig->mw_version));
    UWB_UINT32_TO_BE_STREAM(pCmdBuf, pUwbDeviceConfig->supported_config_ids);
    offset = (uint16_t)(offset + sizeof(pUwbDeviceConfig->supported_config_ids));
    UWB_UINT8_TO_BE_STREAM(pCmdBuf, pUwbDeviceConfig->ranging_role);
    offset = (uint16_t)(offset + sizeof(pUwbDeviceConfig->ranging_role));
    UWB_ARRAY_TO_BE_STREAM(pCmdBuf, pUwbDeviceConfig->device_mac_addr, MAC_SHORT_ADD_LEN);
    offset = (uint16_t)(offset + MAC_SHORT_ADD_LEN);
    return offset;
}
/*******************************************************************************
**
** Function         serializeUwbPhoneConfigData
**
** Description      serializes phone configuration data
**
**
*******************************************************************************/
void serializeUwbPhoneConfigData(UwbPhoneConfigData_t *pUwbPhoneConfig, uint8_t *pCmdBuf)
{
    UWB_BE_STREAM_TO_ARRAY(pCmdBuf, &pUwbPhoneConfig->spec_ver_major[0], MAX_SPEC_VER_LEN);
    UWB_BE_STREAM_TO_ARRAY(pCmdBuf, &pUwbPhoneConfig->spec_ver_minor[0], MAX_SPEC_VER_LEN);
    UWB_BE_STREAM_TO_UINT32(pUwbPhoneConfig->session_id, pCmdBuf);
    UWB_STREAM_TO_UINT8(pUwbPhoneConfig->preamble_id, pCmdBuf);
    UWB_STREAM_TO_UINT8(pUwbPhoneConfig->channel_number, pCmdBuf);
    UWB_STREAM_TO_UINT8(pUwbPhoneConfig->config_id, pCmdBuf);
    UWB_STREAM_TO_UINT8(pUwbPhoneConfig->device_ranging_role, pCmdBuf);
    UWB_BE_STREAM_TO_ARRAY(pCmdBuf, &pUwbPhoneConfig->phone_mac_address[0], SHORT_ADDRESS_LEN);
}
#endif // (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)

#if UWBFTR_DataTransfer
/*******************************************************************************
**
** Function         serializeSendDataPayload
**
** Description      serialize Send Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeSendDataPayload(phUwbDataPkt_t *pSendData, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT32_TO_STREAM(pCmdBuf, pSendData->sessionHandle);
    offset = (uint16_t)(offset + sizeof(pSendData->sessionHandle));

    UWB_ARRAY_TO_STREAM(pCmdBuf, pSendData->mac_address, MAC_EXT_ADD_LEN);
    offset = (uint16_t)(offset + MAC_EXT_ADD_LEN);

    UWB_UINT16_TO_STREAM(pCmdBuf, pSendData->sequence_number);
    offset = (uint16_t)(offset + sizeof(pSendData->sequence_number));

    UWB_UINT16_TO_STREAM(pCmdBuf, pSendData->data_size);
    offset = (uint16_t)(offset + sizeof(pSendData->data_size));

    if (pSendData->data_size > 0) {
        phOsalUwb_MemCopy(pCmdBuf, pSendData->data, pSendData->data_size);
        offset = (uint16_t)(offset + pSendData->data_size);
    }

    return offset;
}

#endif //UWBFTR_DataTransfer

#if UWBIOT_UWBD_SR1XXT_SR2XXT
void deserializeDataFromRxPerNtf(phTestPer_Rx_Ntf_t *pRfTestRecvData, uint8_t *pRespBuf)
{
    UWB_STREAM_TO_UINT32(pRfTestRecvData->attempts, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->acq_Detect, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->acq_Reject, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->rxfail, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->sync_cir_ready, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->sfd_fail, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->sfd_found, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->phr_dec_error, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->phr_bit_error, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->psdu_dec_error, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->psdu_bit_error, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->sts_found, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->eof, pRespBuf);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->vs_data_len, pRespBuf);
    if (pRfTestRecvData->vs_data_len > 0) {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data_type, pRespBuf);
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data.rx_mode, pRespBuf);
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data.no_of_rx_antenna, pRespBuf);
        UWB_STREAM_TO_ARRAY(
            &pRfTestRecvData->vs_data.rx_antenna_id[0], pRespBuf, pRfTestRecvData->vs_data.no_of_rx_antenna);
        for (int j = 0; j < pRfTestRecvData->vs_data.no_of_rx_antenna; j++) {
            UWB_STREAM_TO_INT16(pRfTestRecvData->vs_data.rssi_rx[j], pRespBuf);
        }
        for (int k = 0; k < pRfTestRecvData->vs_data.no_of_rx_antenna; k++) {
            UWB_STREAM_TO_INT16(pRfTestRecvData->vs_data.snr_rx[k], pRespBuf);
        }
        UWB_STREAM_TO_INT16(pRfTestRecvData->rx_cfo_est, pRespBuf);
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
    }
}

void deserializeDataFromLoopbackNtf(phTest_Loopback_Ntf_t *pRfTestRecvData, uint8_t *pRespBuf, uint8_t *pPsdu)
{
    UWB_STREAM_TO_UINT32(pRfTestRecvData->tx_ts_int, pRespBuf);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->tx_ts_frac, pRespBuf);
    UWB_STREAM_TO_UINT32(pRfTestRecvData->rx_ts_int, pRespBuf);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->rx_ts_frac, pRespBuf);
    UWB_STREAM_TO_INT16(pRfTestRecvData->aoa_azimuth, pRespBuf);
    UWB_STREAM_TO_INT16(pRfTestRecvData->aoa_elevation, pRespBuf);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->phr, pRespBuf);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->psdu_len, pRespBuf);
    pRfTestRecvData->pPsdu = pPsdu;
    UWB_STREAM_TO_ARRAY(&pRfTestRecvData->pPsdu[0], pRespBuf, pRfTestRecvData->psdu_len);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->vs_data_len, pRespBuf);
    if (pRfTestRecvData->vs_data_len > 0) {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data_type, pRespBuf);
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data.rx_mode, pRespBuf);
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data.no_of_rx_antenna, pRespBuf);
        UWB_STREAM_TO_ARRAY(
            &pRfTestRecvData->vs_data.rx_antenna_id[0], pRespBuf, pRfTestRecvData->vs_data.no_of_rx_antenna);
        for (int j = 0; j < pRfTestRecvData->vs_data.no_of_rx_antenna; j++) {
            UWB_STREAM_TO_INT16(pRfTestRecvData->vs_data.rssi_rx[j], pRespBuf);
        }
        for (int k = 0; k < pRfTestRecvData->vs_data.no_of_rx_antenna; k++) {
            UWB_STREAM_TO_INT16(pRfTestRecvData->vs_data.snr_rx[k], pRespBuf);
        }
        UWB_STREAM_TO_INT16(pRfTestRecvData->rx_cfo_est, pRespBuf);
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
    }
}

void deserializeDataFromRxNtf(phTest_Rx_Ntf_t *pRfTestRecvData, uint8_t *pRespBuf, uint8_t *pPsduBuf)
{
    UWB_STREAM_TO_UINT32(pRfTestRecvData->rx_done_ts_int, pRespBuf);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->rx_done_ts_frac, pRespBuf);
    UWB_STREAM_TO_INT16(pRfTestRecvData->aoa_azimuth, pRespBuf);
    UWB_STREAM_TO_INT16(pRfTestRecvData->aoa_elevation, pRespBuf);
    UWB_STREAM_TO_UINT8(pRfTestRecvData->toa_gap, pRespBuf);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->phr, pRespBuf);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->psdu_len, pRespBuf);
    pRfTestRecvData->pPsdu = pPsduBuf;
    UWB_STREAM_TO_ARRAY(&pRfTestRecvData->pPsdu[0], pRespBuf, pRfTestRecvData->psdu_len);
    UWB_STREAM_TO_UINT16(pRfTestRecvData->vs_data_len, pRespBuf);
    if (pRfTestRecvData->vs_data_len > 0) {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data_type, pRespBuf);
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data.rx_mode, pRespBuf);
        UWB_STREAM_TO_UINT8(pRfTestRecvData->vs_data.no_of_rx_antenna, pRespBuf);
        UWB_STREAM_TO_ARRAY(
            &pRfTestRecvData->vs_data.rx_antenna_id[0], pRespBuf, pRfTestRecvData->vs_data.no_of_rx_antenna);
        for (int j = 0; j < pRfTestRecvData->vs_data.no_of_rx_antenna; j++) {
            UWB_STREAM_TO_INT16(pRfTestRecvData->vs_data.rssi_rx[j], pRespBuf);
        }
        for (int k = 0; k < pRfTestRecvData->vs_data.no_of_rx_antenna; k++) {
            UWB_STREAM_TO_INT16(pRfTestRecvData->vs_data.snr_rx[k], pRespBuf);
        }
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
    }
}
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

#if UWBIOT_UWBD_SR150
uint16_t serializeSessionSetLocZoneCmd(phSessionSetLocZone_t *pSetLocZone, uint8_t *pCmdBuf)
{
    uint16_t offset = 0;

    UWB_UINT32_TO_STREAM(pCmdBuf, pSetLocZone->setLocZone_SessionHandle);
    offset = (uint16_t)(offset + sizeof(pSetLocZone->setLocZone_SessionHandle));

    UWB_UINT16_TO_STREAM(pCmdBuf, pSetLocZone->setLocZone_RangingBlockIndex);
    offset = (uint16_t)(offset + sizeof(pSetLocZone->setLocZone_RangingBlockIndex));

    UWB_UINT8_TO_STREAM(pCmdBuf, pSetLocZone->setLocZone_LocZone);
    offset = (uint16_t)(offset + sizeof(pSetLocZone->setLocZone_LocZone));

    return offset;
}
#endif // UWBIOT_UWBD_SR150

void PRINTF_WITH_TIME(const char *fmt, ...)
{
    /* No printing here */
}
