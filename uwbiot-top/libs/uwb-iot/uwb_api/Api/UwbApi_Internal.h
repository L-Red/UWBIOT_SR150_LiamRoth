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

#ifndef UWBAPI_INTERNAL_H
#define UWBAPI_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "phUwb_BuildConfig.h"
#include "phUwbTypes.h"
#include "UwbApi_Types.h"
#include "UwbApi_Types_RfTest.h"
#include <UwbApi_Types_Proprietary.h>
#include "uwa_api.h"
#include "uwb_types.h"

#define MAC_SHORT_ADD_LEN 2
#define MAC_EXT_ADD_LEN   8
#define TIMESTAMP_LEN     8
#define RFU_SHORT_MAC_ADD 11
#define RFU_EXT_MAC_ADD   5
#define RESERVED_LEN      4
/*these offset are used for tracing rcv data ntf length*/
#define MAC_ADDR_OFFSET 25
/** Range Data Notification Offset for TWR
 * For Short Addressing Mode : MacAddr(2B) + OtherFields(18B) + RFU(11B)
 * For Ext Addressing Mode : MacAddr(8B) + OtherFields(18B) + RFU(5B)
 * In both Addrressing Modes the length sums out to be 31B
*/
#define MAX_TWR_RNG_DATA_NTF_OFFSET 31

#define ONEWAY_RFU_BYTE_OFFSET (12)
#define TLV_BUFFER_OFFSET      2

/* Session ID RF Test */
/**  Session ID used for RF TEST Mode */
#define SESSION_ID_RFTEST 0x00000000

/* UWA_DM callback events */
#define UWA_DM_RSP_EVENT 0x00
#define UWA_DM_NTF_EVENT 0xA0

/* Device management response timeout */
#if UWBIOT_UWBD_SR2XXT
#define UWB_MAX_DEV_MGMT_RSP_TIMEOUT (2000 + 500)
#else
#define UWB_MAX_DEV_MGMT_RSP_TIMEOUT (1000 + 500)
#endif

typedef enum Response_Rsp_Event
{
    /* Result of UWA_Enable             */
    UWA_DM_ENABLE_EVT = UWA_DM_RSP_EVENT,
    /* Result of UWA_Disable            */
    UWA_DM_DISABLE_EVT,
    /* Result of UWA Register Ext Callback */
    UWA_DM_REGISTER_EXT_CB_EVT,
    /* Result of command response timeout */
    UWA_DM_UWBD_RESP_TIMEOUT_EVT,
    /* Result of get device info */
    UWA_DM_CORE_GET_DEVICE_INFO_RSP_EVT,
    /* Result of UWA_SetCoreConfig          */
    UWA_DM_CORE_SET_CONFIG_RSP_EVT,
    /* Result of get core config          */
    UWA_DM_CORE_GET_CONFIG_RSP_EVT,
    /* Result of Core Dev Reset */
    UWA_DM_DEVICE_RESET_RSP_EVT,
    /* Result of session Init cmd */
    UWA_DM_SESSION_INIT_RSP_EVT,
    /* Result of session Deinit cmd */
    UWA_DM_SESSION_DEINIT_RSP_EVT,
    /* Result of set app config */
    UWA_DM_SESSION_SET_CONFIG_RSP_EVT,
    /* Result of get app config */
    UWA_DM_SESSION_GET_CONFIG_RSP_EVT,
    /* Result of get session count */
    UWA_DM_SESSION_GET_COUNT_RSP_EVT,
    /* Result of get session count */
    UWA_DM_SESSION_GET_STATE_RSP_EVT,
    /* Result of range start cmd */
    UWA_DM_RANGE_START_RSP_EVT,
    /* Result of range stop cmd */
    UWA_DM_RANGE_STOP_RSP_EVT,
    /* Result of get core device capability  */
    UWA_DM_GET_CORE_DEVICE_CAP_RSP_EVT = 17, /* changed as it would clash with OID SE comm error */
    /* Session Update Multicast List resp event*/
    UWA_DM_SESSION_MC_LIST_UPDATE_RSP_EVT,
    /* Blink Data Tx event*/
    UWA_DM_SEND_BLINK_DATA_RSP_EVT = 20, /* changed as it would clash with OID SE bind error */
    /* Result of update active rounds anchor rsp event */
    UWA_DM_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_RSP_EVT,
    /* Result of update active rounds receiver rsp event */
    UWA_DM_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_RSP_EVT,
#if !(UWBIOT_UWBD_SR040)
    /* Result of Get Data Size in Ranging */
    UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_RSP_EVT,
#endif // !(UWBIOT_UWBD_SR040)
    /* Result of test set config event */
    UWA_DM_TEST_SET_CONFIG_RSP_EVT,
    /* Result of test get config event */
    UWA_DM_TEST_GET_CONFIG_RSP_EVT,
    /* Result of test stop session response event */
    UWA_DM_TEST_STOP_SESSION_RSP_EVT,
    /* Result of test periodic tx response event */
    UWA_DM_TEST_PERIODIC_TX_RSP_EVT,
    /* Result of test per rx response event */
    UWA_DM_TEST_PER_RX_RSP_EVT,
    /* Result of test Loop Back response event */
    UWA_DM_TEST_LOOPBACK_RSP_EVT,
    /* Result of test RX Test resp event*/
    UWA_DM_TEST_RX_RSP_EVT,
    /* Result of Calibration Integrity Protection Resp event */
    UWA_DM_PROP_CALIB_INTEGRITY_PROTECTION_RESP_EVT,
    /* Result of Generate Tag Resp event */
    UWA_DM_PROP_GENERATE_TAG_RESP_EVT,
    /* Result of Verify Calib Data Resp event */
    UWA_DM_PROP_VERIFY_CALIB_DATA_RESP_EVT,
    /* Result of Do Bind Resp event */
    UWA_DM_PROP_DO_BIND_RESP_EVT,
    /* Result of Get Binding Count Resp event */
    UWA_DM_PROP_GET_BINDING_COUNT_RESP_EVT,
    /* Result of Test Connectivity Resp event */
    UWA_DM_PROP_TEST_CONNECTIVITY_RESP_EVT,
    /* Result of SE Test Loop Resp event */
    UWA_DM_PROP_SE_TEST_LOOP_RESP_EVT,
    /* Result of Get Binding Status Resp event */
    UWA_DM_PROP_GET_BINDING_STATUS_RESP_EVT,
    /* Result of Query Temperature Resp event */
    UWA_DM_PROP_QUERY_TEMPERATURE_RESP_EVT,
    /* Result of Configure Auth Tag Options Resp event */
    UWA_DM_PROP_CONFIGURE_AUTH_TAG_OPTION_RESP_EVT,
    /* Result of Configure Auth Tag Version Resp event */
    UWA_DM_PROP_CONFIGURE_AUTH_TAG_VERSION_RESP_EVT,
    /* Result of URSK Deletion Request Resp event */
    UWA_DM_PROP_URSK_DELETION_REQUEST_RESP_EVT,
    /* Result of Query UWB Timestamp Resp event */
    UWA_DM_PROP_QUERY_TIMESTAMP_RESP_EVT,
    /* Result of get all uwb sessions rsp event */
    UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT,
    /* Result of write otp calib data rsp event */
    UWA_DM_PROP_WRITE_OTP_CALIB_DATA_RSP_EVT,
    /* Result of read otp calib data rsp event */
    UWA_DM_PROP_READ_OTP_CALIB_DATA_RSP_EVT,
    /* Result of Data Transfer Phase Configuration Resp event */
    UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_RSP_EVT,
    /* Result of get session */
    UWA_DM_SESSION_SET_HUS_CONTROLLER_CONFIG_RSP_EVT,
    /* Result of get session */
    UWA_DM_SESSION_SET_HUS_CONTROLEE_CONFIG_RSP_EVT,
    /* Result of Vendor set app config */
    UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT,
    /* Result of vendor get app config */
    UWA_DM_SESSION_GET_VENDOR_CONFIG_RSP_EVT,
#if !UWBIOT_UWBD_SR040
    /* Result of do Calibration Resp event */
    UWA_DM_VENDOR_DO_VCO_PLL_CALIBRATION_RESP_EVT,
    /* Result of Set Device Calibration Resp event */
    UWA_DM_VENDOR_SET_DEVICE_CALIBRATION_RESP_EVT,
    /* Result of Set Calibration Resp event */
    UWA_DM_VENDOR_GET_DEVICE_CALIBRATION_RESP_EVT,
#endif // !UWBIOT_UWBD_SR040
#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
    /* Result of TRNG  rsp event */
    UWA_DM_PROP_TRNG_RESP_EVENT,
#endif //(UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
#if UWBIOT_UWBD_SR040
    /* Result of suspend device rsp event */
    UWA_DM_PROP_SUSPEND_DEVICE_RSP_ENVT,
    /* Result of NVM payload rsp event */
    UWA_DM_SESSION_NVM_PAYLOAD_RSP_EVENT,
    /* Result of start test mode rsp event */
    UWA_DM_START_TEST_MODE_RSP_EVENT,
    /* Result of stop test mode rsp event */
    UWA_DM_STOP_TEST_MODE_RSP_EVENT,
    /* Result of set calibration rsp event */
    UWA_DM_SET_CALIB_TRIM_RSP_EVENT,
    /* Result of get calibration rsp event */
    UWA_DM_GET_CALIB_TRIM_RSP_EVENT,
    /* Result of get calibration rsp event */
    UWA_DM_GET_BYPASS_CURRENT_LIMITER,
#endif
#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
    /* Result of profile blob rsp event */
    UWA_DM_PROP_PROFILE_BLOB_RSP_EVENT,
#endif // UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160
    /* results of SESSION_SET_LOCALIZATION_ZONE_CMD rsp event  */
    UWA_DM_SESSION_SET_LOCALIZATION_ZONE_RSP_EVENT,
    /* Result of invalid rsp event*/
    UWA_DM_INVALID_RSP_EVT = 0xFF
} eResponse_Rsp_Event;

/* UWA_DM callback events for UWB RF events */
typedef enum Response_Ntf_Event
{
    /* Result of device status ntf */
    UWA_DM_DEVICE_STATUS_NTF_EVT = UWA_DM_NTF_EVENT,
    /* Result of core generic error status */
    UWA_DM_CORE_GEN_ERR_STATUS_EVT,
    /* Result of session NTF  */
    UWA_DM_SESSION_STATUS_NTF_EVT,
    /* Result of session info ntf */
    UWA_DM_SESSION_INFO_NTF_EVT,
    /* Result of data credit notification*/
    UWA_DM_DATA_CREDIT_STATUS_EVT,
    /* Result of data transmit status notification*/
    UWA_DM_DATA_TRANSMIT_STATUS_EVT,
    /* Session Update Multicast List ntf event*/
    UWA_DM_SESSION_MC_LIST_UPDATE_NTF_EVT,
    /* Blink Data Tx ntf event*/
    UWA_DM_SEND_BLINK_DATA_NTF_EVT,
    /* Data Transfer Phase Configuration NTF event */
    UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_NTF_EVT,
    /* Result of test periodic tx NTF event */
    UWA_DM_TEST_PERIODIC_TX_NTF_EVT,
    /* Result of test per rx NTF event */
    UWA_DM_TEST_PER_RX_NTF_EVT,
    /* Result of test Loop Back NTF event */
    UWA_DM_TEST_LOOPBACK_NTF_EVT,
    /* Result of test RX Test NTF event*/
    UWA_DM_TEST_RX_NTF_EVT,
#if UWBFTR_DataTransfer
    /* Result of test RX Test NTF event*/
    UWA_DM_DATA_RCV_NTF_EVT,
#endif //UWBFTR_DataTransfer
#if UWBFTR_CCC
    /* Result of range CCC data ntf */
    UWA_DM_RANGE_CCC_DATA_NTF_EVT,
#endif // UWBFTR_CCC
    /* Result of invalid ntf event*/
    UWA_DM_INVALID_NTF_EVT = 0xFF
} eResponse_Ntf_Event;

/**
 * \brief Structure for storing  UWB API Context.
 */
typedef struct phUwbApiContext
{
    /** Data to be send to device. This includes both UCI Header and UCI Payload */
#if UWBFTR_DataTransfer
    uint8_t snd_data[MAX_UCI_HEADER_SIZE + MAX_CMD_BUFFER_DATA_TRANSFER];
#else
    /*  This is needed for Set calibration.
      *  Max set calibration buffer needed is 19*19*2 + 2 */
    uint8_t snd_data[MAX_UCI_HEADER_SIZE + MAX_API_PACKET_SIZE];
#endif //UWBFTR_DataTransfer
    /** Common response buffer to store *
     * response/ntfn received. This includes both UCI Header and UCI Payload */
    uint8_t rsp_data[MAX_UCI_HEADER_SIZE + MAX_API_PACKET_SIZE];
    /** Current device state *
     * UCI_DEV_IDLE/UCI_DEV_READY/UCI_DEV_BUSY/UCI_DEV_ERROR */
    uint8_t dev_state;
    /** Holds status of the current ongoing operations */
    tUWBAPI_STATUS wstatus;
    /** This is set after UWB stack is initialized */
    BOOLEAN isUfaEnabled;
    /** Current Event on which UWB API  is waiting for
     * response/ntfn */
    uint16_t currentEventId;
    uint16_t receivedEventId;
    /** Length of the response received */
    uint16_t rsp_len;
    /** Ranging Count */
    int32_t rangingCnt;
    /** Semaphore used for making UWB APIs synchronous, it will *
     * be signaled when UCI response/notification  is received */
    void *devMgmtSem;
    /** Generic callback registered by Application */
    tUwbApi_AppCallback *pAppCallback;
    /** Session data */
    phUwbSessionInfo_t sessionInfo;
#if UWBIOT_UWBD_SR040
    /** Calibration status notification data */
    phCalibrationStatus_t calibrationStatus;
    /** Test Loopback status data */
    phTestLoopbackData_t testLoopbackStatus;
    /** Test Initiator Range data */
    phTestInitiatorRangekData_t testInitiatorRangeStatus;
    phPhyLogNtfnData_t testPhyLogNtfnData;

#endif //UWBIOT_UWBD_SR040
    /** Ranging data notification data */
    phRangingData_t rangingData;
#if UWBFTR_DataTransfer
    uint16_t maxDataPacketPayloadSize;
    uint16_t maxMessageSize;
    phUwbDataCredit_t dataCredit;
    phUwbDataTransmit_t dataTransmit;
    phUwbRcvDataPkt_t rcvDataPkt;
#endif //UWBFTR_DataTransfer
#if UWBFTR_Radar
    phUwbRadarNtf_t RadarNtf;
#endif //UWBFTR_Radar
#if UWBFTR_CCC
    phAliroRangingData_t AliroRangingData;
#endif //UWBFTR_CCC
    uint8_t dtpcm_addr_mode;
} phUwbApiContext_t;

EXTERNC phUwbApiContext_t uwbContext;

void cleanUp();
EXTERNC tUWBAPI_STATUS uwbInit(tUwbApi_AppCallback *pCallback, Uwb_operation_mode_t mode);
#if !(UWBIOT_UWBD_SR040)
EXTERNC tUWBAPI_STATUS recoverUWBS();
#endif //!(UWBIOT_UWBD_SR040)
EXTERNC void sep_SetWaitEvent(uint16_t eventID);
EXTERNC tUWBAPI_STATUS getDeviceInfo(void);
EXTERNC tUWBAPI_STATUS getCapsInfo(void);
EXTERNC tUWBAPI_STATUS sendRawUci(uint8_t *p_cmd_params, uint16_t cmd_params_len);
EXTERNC tUWBAPI_STATUS waitforNotification(uint16_t waitEventId, uint32_t waitEventNtftimeout);
EXTERNC uint8_t getAppConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer);
#if !(UWBIOT_UWBD_SR040)
EXTERNC uint8_t getTestConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer);
EXTERNC tUWBAPI_STATUS VendorAppConfig_TlvParser(
    const UWB_VendorAppParams_List_t *pAppParams_List, UWB_AppParams_value_au8_t *pOutput_param_value);
#endif //!(UWBIOT_UWBD_SR040)
EXTERNC uint8_t getCoreDeviceConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer);
EXTERNC void parseCoreGetDeviceConfigResponse(uint8_t *tlvBuffer, phDeviceConfigData_t *devConfig);
EXTERNC void parseRangingParams(uint8_t *rspPtr, uint8_t noOfParams, phRangingParams_t *pRangingParams);
EXTERNC void ufaDeviceManagementRspCallback(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData);
EXTERNC tUWBAPI_STATUS AppConfig_TlvParser(
    const UWB_AppParams_List_t *pAppParams_List, UWB_AppParams_value_au8_t *pOutput_param_value);

#if UWBIOT_UWBD_SR040
EXTERNC void ufaDeviceManagementNtfCallback(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData);
#endif
EXTERNC void ufaDeviceManagementNtfCallback(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData);
#if UWBFTR_DataTransfer
EXTERNC void ufaDeviceManagementDataCallback(uint8_t dpf, uint16_t len, uint8_t *eventData);
#endif //UWBFTR_DataTransfer
EXTERNC tUWBAPI_STATUS UwbApi_GetAllUwbSessions(phUwbSessionsContext_t *pUwbSessionsContext);
EXTERNC tUWBAPI_STATUS parseUwbSessionParams(uint8_t *rspPtr, phUwbSessionsContext_t *pUwbSessionsContext);
EXTERNC BOOLEAN parseCapabilityInfo(phUwbCapInfo_t *pDevCap);
#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif
