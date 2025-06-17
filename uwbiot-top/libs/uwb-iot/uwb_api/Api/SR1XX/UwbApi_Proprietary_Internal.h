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

#ifndef UWBAPI_PROPRIETARY_INTERNAL_SR1XX_H
#define UWBAPI_PROPRIETARY_INTERNAL_SR1XX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "phUwbTypes.h"
#include "UwbApi_Types.h"
#include <UwbApi_Types_Proprietary.h>

#if UWBFTR_SE_SN110
#include "UwbSeApi.h"
EXTERNC void reset_se_on_error(void);
#endif // UWBFTR_SE_SN110
/*
* below checks are required for Windows build fix and avoid coverity warning in 'setDefaultAoaCalibration' .
*/
#if (UWBIOT_TML_SPI)
#if UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S
#define MCTT_PCTT_BIN_WITH_SSG_FW 1
#else
#define MCTT_PCTT_BIN_WITH_SSG_FW 0
#endif
#else
#define MCTT_PCTT_BIN_WITH_SSG_FW 0
#endif //UWBIOT_HOST_PCWINDOWS

/* UWA_DM PROP callback events */
#define UWA_DM_EXT_NTF_EVENT 0xD0


EXTERNC uint8_t getVendorAppConfigTLVBuffer(
    uint8_t paramId, void *paramValue, uint16_t paramValueLen, uint8_t *tlvBuffer);
EXTERNC uint8_t getVendorDebugConfigTLVBuffer(
    uint16_t paramId, void *paramValue, uint16_t paramValueLen, uint8_t *tlvBuffer);
EXTERNC uint8_t getExtDeviceConfigTLVBuffer(uint8_t paramId, void *paramValue, uint8_t *tlvBuffer);
EXTERNC uint8_t getExtTestConfigTLVBuffer(uint16_t paramId, void *paramValue, uint8_t *tlvBuffer);
EXTERNC void parseDebugParams(uint8_t *rspPtr, uint8_t noOfParams, UWB_DebugParams_List_t *DebugParams_List);
EXTERNC void extDeviceManagementCallback(uint8_t gid, uint8_t event, uint16_t paramLength, uint8_t *pResponseBuffer);
EXTERNC tUWBAPI_STATUS DebugConfig_TlvParser(
    const UWB_DebugParams_List_t *pDebugParams_List, UWB_Debug_Params_value_t *pOutput_param_value);
EXTERNC BOOLEAN parseDeviceInfo(phUwbDevInfo_t *pdevInfo);
EXTERNC uint8_t getExtCoreDeviceConfigTLVBuffer(
    uint16_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer);
EXTERNC void parseExtGetDeviceConfigResponse(uint8_t *tlvBuffer, phDeviceConfigData_t *devConfig);
EXTERNC tUWBAPI_STATUS setDefaultCoreConfigs(void);
#if UWBIOT_UWBD_SR150
EXTERNC void parseExtGetCalibResponse(uint8_t *tlvBuffer, phCalibRespStatus_t *calibResp);
#endif // UWBIOT_UWBD_SR150
#if UWBFTR_Radar
EXTERNC uint8_t getExtRadarAppConfigTLVBuffer(
    uint16_t paramId, void *paramValue, uint16_t paramValueLen, uint8_t *tlvBuffer);
#endif // UWBFTR_Radar//

#if UWBFTR_CCC
EXTERNC BOOLEAN parseCapabilityCCCParams(
    phUwbCapInfo_t *pDevCap, uint8_t paramId, uint16_t *index, uint8_t length, uint8_t *capsInfoData);
#endif // UWBFTR_CCC

/* UWA_DM callback events for UWB EXT NTF events */
typedef enum Response_Ext_Ntf_Event
{
    /* Result of SE Test Loop ntf event */
    UWA_DM_PROP_SE_TEST_LOOP_NTF_EVT = UWA_DM_EXT_NTF_EVENT,
    /* Result of Generate Tag ntf event */
    UWA_DM_PROP_GENERATE_TAG_NTF_EVT,
    /* Result of SE COM ERROR ntf event */
    UWA_DM_PROP_SE_COM_ERROR_NTF_EVT,
    /* Scheduler status ntf event*/
    UWA_DM_PROP_SCHEDULER_STATUS_NTF_EVT,
#if UWBFTR_UWBS_DEBUG_Dump
    /* Data Logger ntf event*/
    UWA_DM_PROP_DBG_DATA_LOGGER_NTF_EVT,
#endif //UWBFTR_UWBS_DEBUG_Dump
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    /* WiFi CoEx Ind ntf event*/
    UWA_DM_PROP_UWB_WIFI_COEX_IND_NTF_EVT,
    /* WiFi CoEx Maximum active grant duration excced warning ntf event*/
    UWA_DM_PROP_WIFI_COEX_MAX_ACTIVE_GRANT_DUARTION_EXCEEDED_WAR_NTF_EVT,
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#if UWBFTR_UWBS_DEBUG_Dump
    /* Vendor CIR Log nt event*/
    UWA_DM_VENDOR_CIR_LOG_NTF_EVT,
    /* Vendor PSDU Log nt event*/
    UWA_DM_VENDOR_PSDU_LOG_NTF_EVT,
#endif //UWBFTR_UWBS_DEBUG_Dump
#if (UWBFTR_SE_SN110)
    /* Vendor se binding ntf event*/
    UWA_DM_VENDOR_SE_DO_BIND_NTF_EVT,
    /* Vendor se test connectivity ntf event*/
    UWA_DM_VENDOR_SE_DO_TEST_CONNECTIVITY_NTF_EVT,
    /* Vendor ese binfin check ntf event*/
    UWA_DM_VENDOR_ESE_BINDING_CHECK_NTF_EVT,
    /* Vendor ursk deletion req ntf event*/
    UWA_DM_VENDOR_URSK_DELETION_REQ_NTF_EVT,
#endif //(UWBFTR_SE_SN110)
    /* Vendor Do Calibration ntf event*/
    UWA_DM_PROP_DO_VCO_PLL_CALIBRATION_NTF_EVT,
    /* Internal Ranging Timestamp Log nt event*/
    UWA_DM_INTERNAL_RANGING_TIMESTAMP_NTF_EVT,
    /* Internal rframe log ntf event*/
    UWA_DM_INTERNAL_DBG_RFRAME_LOG_NTF_EVT,
    /* Write calibration data ntf event*/
    UWA_DM_PROP_SE_WRITE_CALIB_DATA_NTF_EVT,
    /* Read calibration data ntf event*/
    UWA_DM_PROP_SE_READ_CALIB_DATA_NTF_EVT,
    /* Result of invalid ntf event*/
    UWA_EXT_DM_INVALID_NTF_EVT = 0xFF
} eResponse_Ext_Ntf_Event;

/**
 * \brief Structure lists out the fields for the PDAoA Configurations.
 */
typedef struct
{
    /** Key used to find the values against it
     * \ref NxpUwbConfig
     */
    uint8_t key;
    /**
     * Value for the key
     */
    const uint8_t *pValue;
}sAoACoreConfigs_t;


#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif // UWBAPI_PROPRIETARY_INTERNAL_SR1XX_H
