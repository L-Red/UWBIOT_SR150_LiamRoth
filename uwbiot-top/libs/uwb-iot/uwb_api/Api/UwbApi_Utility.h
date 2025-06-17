/*
 *
 * Copyright 2021-2023 NXP.
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
#ifndef UWB_CORE_UWBAPI_API_UWBAPI_UTILITY_H_
#define UWB_CORE_UWBAPI_API_UWBAPI_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "UwbApi_Types.h"
#include <UwbApi_Types_Proprietary.h>
#include "uwb_types.h"
#include "UwbApi_Internal.h"

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
EXTERNC tUWBAPI_STATUS sendUciCommandAndWait(uint16_t event, uint16_t cmdLen, uint8_t *pCmd);

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
EXTERNC tUWBAPI_STATUS sendUciCommand(uint16_t event, uint16_t cmdLen, uint8_t *pCmd, uint8_t pbf);

/*******************************************************************************
**
** Function         serializeSessionInitPayload
**
** Description      serialize Session Init Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeSessionInitPayload(uint32_t sessionHandle, eSessionType sessionType, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeGetCoreConfigPayload
**
** Description      serialize Get Core Config Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeGetCoreConfigPayload(
    uint8_t noOfParams, uint8_t paramLen, uint8_t *paramId, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeSessionHandlePayload
**
** Description      serialize Session Handle Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeSessionHandlePayload(uint32_t sessionHandle, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeAppConfigPayload
**
** Description      serialize App Config Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeAppConfigPayload(
    uint32_t sessionHandle, uint8_t noOfParams, uint16_t paramLen, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeUpdateControllerMulticastListPayload
**
** Description      serialize Update Controller MulticastList Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeUpdateControllerMulticastListPayload(
    phMulticastControleeListContext_t *pControleeContext, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeTestDataPayload
**
** Description      serialize Test Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeTestDataPayload(uint16_t psduLen, uint8_t psduData[], uint8_t *pCmdBuf);

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
EXTERNC uint16_t serializedoVcoPllCalibPayload(uint8_t channel, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeWriteOtpCalibDataPayload
**
** Description      serialize Write Otp Calib Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeWriteOtpCalibDataPayload(
    uint8_t channel, uint8_t writeOption, uint8_t writeDataLen, uint8_t *writeData, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeReadOtpCalibDataPayload
**
** Description      serialize Read Otp Calib Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeReadOtpCalibDataPayload(
    uint8_t channel, uint8_t readOption, eOtpCalibParam calibParam, uint8_t *pCmdBuf);

#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

/*******************************************************************************
**
** Function         serializeSetCalibPayload
**
** Description      serialize Set Calib Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeSetCalibPayload(
    uint8_t channel, eCalibParam paramId, uint8_t *calibrationValue, uint16_t length, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeGetCalibPayload
**
** Description      serialize Get Calib Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeGetCalibPayload(uint8_t channel, eCalibParam paramId, uint8_t rxAntenaPair, uint8_t *pCmdBuf);

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
EXTERNC uint16_t serializeGetExtCalibPayload(uint8_t channel, eCalibParam paramId, uint8_t rxAntenaPair, uint8_t *pCmdBuf);
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
EXTERNC uint16_t serializeSeLoopTestPayload(uint16_t loopCnt, uint16_t timeInterval, uint8_t *pCmdBuf);
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
EXTERNC uint16_t serializecalibIntegrityProtectionPayload(
    eCalibTagOption tagOption, uint16_t calibBitMask, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeVerifyCalibDataPayload
**
** Description      serialize Verify Calib Data Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeVerifyCalibDataPayload(
    uint8_t *pCmacTag, uint8_t tagOption, uint16_t tagVersion, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeConfigureAuthTagOptionsPayload
**
** Description      serialize Configure Auth Tag Options Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeConfigureAuthTagOptionsPayload(
    uint8_t deviceTag, uint8_t modelTag, uint16_t labelValue, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeConfigureAuthTagVersionPayload
**
** Description      serialize Configure Auth Tag Version Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeConfigureAuthTagVersionPayload(uint16_t labelValue, uint8_t *pCmdBuf);
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
EXTERNC uint16_t serializeUrskDeletionRequestPayload(
    uint8_t noOfSessionHandle, uint32_t *pSessionHandleList, uint8_t *pCmdBuf);
#endif //(UWBFTR_SE_SN110)
#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
/*******************************************************************************
**
** Function         serializeTrngtPayload
**
** Description      serialize Trng payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeTrngtPayload(uint8_t trng_size, uint8_t *pCmdBuf);
#endif // (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

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
EXTERNC uint16_t serializeSessionNvmPayload(
    esessionNvmManage sesNvmManageTag, uint32_t sessionHandle, uint8_t *pCmdBuf);

#endif // UWBIOT_UWBD_SR040

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR160)
/*******************************************************************************
**
** Function         serializeSetProfileParamsPayload
**
** Description      serialize set profile params payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeSetProfileParamsPayload(
    phUwbProfileInfo_t *pProfileInfo, uint16_t blobSize, uint8_t *pProfileBlob, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeUwbDeviceConfigData
**
** Description      serializes dset profile params payload
**
** Returns          Length of the serialized data
**
*******************************************************************************/
uint16_t serializeUwbDeviceConfigData(UwbDeviceConfigData_t *pUwbDeviceConfig, uint8_t *pCmdBuf);
/*******************************************************************************
**
** Function         serializeUwbPhoneConfigData
**
** Description      deserializes phone configuration data
**
**
*******************************************************************************/
void serializeUwbPhoneConfigData(UwbPhoneConfigData_t *pUwbPhoneConfig, uint8_t *pCmdBuf);

#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR160

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
uint16_t serializeControllerHusSessionPayload(phControllerHusSessionConfig_t *pHusSessionCfg, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeControleeHusSessionPayload
**
** Description      serialize HUS Session Payload of Controlee
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeControleeHusSessionPayload(phControleeHusSessionConfig_t *pHusSessionCfg, uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeDtpcmPayload
**
** Description      serialize DTPCM Payload
**
** Returns          Length of payload
**
*******************************************************************************/
uint16_t serializeDtpcmPayload(phDataTxPhaseConfig_t *phDataTxPhaseCfg, uint8_t *pCmdBuf);

#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

#if UWBIOT_UWBD_SR1XXT_SR2XXT
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
EXTERNC uint16_t serializeUpdateActiveRoundsAnchorPayload(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    UWB_MacAddressMode_t macAddressingMode,
    const phActiveRoundsConfig_t roundConfigList[],
    uint8_t *pCmdBuf);

/*******************************************************************************
**
** Function         serializeUpdateActiveRoundsReceiverPayload
**
** Description      serialize update active rounds receiver Payload
**
** Returns          Length of payload
**
*******************************************************************************/
EXTERNC uint16_t serializeUpdateActiveRoundsReceiverPayload(
    uint32_t sessionHandle, uint8_t nActiveRounds, const uint8_t roundConfigList[], uint8_t *pCmdBuf);
#endif //(UWBFTR_DL_TDoA_Anchor || UWBFTR_DL_TDoA_Tag)
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
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
EXTERNC uint16_t serializeSendDataPayload(phUwbDataPkt_t *pSendData, uint8_t *pCmdBuf);

#endif //UWBFTR_DataTransfer

#if UWBIOT_UWBD_SR1XXT_SR2XXT
void deserializeDataFromRxPerNtf(phTestPer_Rx_Ntf_t *ptestrecvdata, uint8_t *pRespBuf);
void deserializeDataFromRxNtf(phTest_Rx_Ntf_t *pRfTestRecvData, uint8_t *pRespBuf, uint8_t *pPsduBuf);
void deserializeDataFromLoopbackNtf(phTest_Loopback_Ntf_t *pRfTestRecvData, uint8_t *pRespBuf, uint8_t *pPsdu);
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

#if UWBIOT_UWBD_SR150
/*******************************************************************************
**
** Function         serializeSessionSetLocZoneCmd
**
** Description      serialize Session Set Localization Zone command.
**
** Returns          Length of command
**
*******************************************************************************/
EXTERNC uint16_t serializeSessionSetLocZoneCmd(phSessionSetLocZone_t *pSetLocZone, uint8_t *pCmdBuf);
#endif // UWBIOT_UWBD_SR150

void PRINTF_WITH_TIME(const char *fmt, ...);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* UWB_CORE_UWBAPI_API_UWBAPI_UTILITY_H_ */
