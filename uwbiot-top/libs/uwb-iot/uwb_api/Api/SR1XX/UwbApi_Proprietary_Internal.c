/*
 *
 * Copyright 2018-2024 NXP.
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

#include "UwbApi_Proprietary_Internal.h"
#include "UwbApi_Internal.h"
#include "phNxpLogApis_UwbApi.h"
#include "uwa_api.h"
#include "phOsalUwb.h"
#include "uci_ext_defs.h"
#include "UwbAdaptation.h"
#include "AppConfigParams.h"
#include "PrintUtility.h"
#include "phNxpUwbConfig.h"
#include "UwbSeApi.h"
#include "uwa_dm_int.h"
#include "uwb_int.h"
#include "UwbApi_Utility.h"
#include "uwbiot_ver.h"

/** Extended config header length (2 byte param id + 1 byte param len) */
#define UWBD_EXT_CONFIG_HEADER_LEN 0x03
/** DPD wakeup src GPIO1 */
#define DPD_WAKEUP_SRC_GPIO_1 2
/** DPD wakeup src GPIO3 */
#define DPD_WAKEUP_SRC_GPIO_3 4
/** WTX count min */
#define WTX_COUNT_MIN 20
/** WTX count max */
#define WTX_COUNT_MAX 180

/* skip sem posting op*/
BOOLEAN skip_sem_post = FALSE;

#if !(MCTT_PCTT_BIN_WITH_SSG_FW)


#if UWBIOT_UWBD_SR150

static const unsigned char aoa_ch5_config_block_names[] = {
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH5,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH5,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH5,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH5,
};

static const unsigned char aoa_ch9_config_block_names[] = {
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH9,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH9,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH9,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH9,
};

#else
static const unsigned char aoa_config_block_names[] = {
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH5,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH9,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH5,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH9,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH5,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH9,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH5,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH9,
};
#endif //

#endif //

#if UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200T
static void handle_se_com_err_ntf(uint8_t *p, uint16_t len);
#endif //UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200T

#if UWBFTR_SE_SN110
/*******************************************************************************
 **
 ** Function         reset_se_on_error
 **
 ** Description      This function is called to cold reset the SE.
 **
 ** Returns          void
 **
 *******************************************************************************/
void reset_se_on_error(void)
{
    uint16_t RespSize     = sizeof(uwbContext.rsp_data);
    uint8_t eSeRsetBuf[3] = {0x2F, 0x1E, 0x00}; // command to reset the eSE

    if (UwbSeApi_NciRawCmdSend(sizeof(eSeRsetBuf), eSeRsetBuf, &RespSize, uwbContext.rsp_data) != 0) {
        NXPLOG_UWBAPI_E("eSE reset failure");
    }
    else {
        NXPLOG_UWBAPI_D("eSE reset success");
    }
}
#endif /*UWBFTR_SE_SN110*/

/*******************************************************************************
 **
 ** Function:        DebugConfig_TlvParser
 **
 ** Description:     Debug configuration Tlv parser
 **
 ** Returns:         status
 **
 *******************************************************************************/
tUWBAPI_STATUS DebugConfig_TlvParser(
    const UWB_DebugParams_List_t *pDebugParams_List, UWB_Debug_Params_value_t *pOutput_param_value)
{
    tUWBAPI_STATUS status;
#if UWBIOT_UWBD_SR2XXT
    uint8_t *param_value = pDebugParams_List->param_value.param.param_value;
#endif // UWBIOT_UWBD_SR2XXT
    switch (pDebugParams_List->param_type) {
    case kUWB_DEBUGPARAMS_Type_u8:
        pOutput_param_value->param_len = sizeof(uint8_t);
        UWB_UINT8_TO_FIELD(pOutput_param_value->param_value, pDebugParams_List->param_value.vu8);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_DEBUGPARAMS_Type_u16:
        pOutput_param_value->param_len = sizeof(uint16_t);
        UWB_UINT16_TO_FIELD(pOutput_param_value->param_value, pDebugParams_List->param_value.vu16);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_DEBUGPARAMS_Type_u32:
        pOutput_param_value->param_len = sizeof(uint32_t);
        UWB_UINT32_TO_FIELD(pOutput_param_value->param_value, pDebugParams_List->param_value.vu32);
        status = UWBAPI_STATUS_OK;
        break;
#if UWBIOT_UWBD_SR2XXT
    case kUWB_DEBUGPARAMS_Type_au8:
        pOutput_param_value->param_len = pDebugParams_List->param_value.param.param_len;
        UWB_STREAM_TO_ARRAY(pOutput_param_value->param_value, param_value, pOutput_param_value->param_len);
        status = UWBAPI_STATUS_OK;
        break;
#endif // UWBIOT_UWBD_SR2XXT
    default:
        status = UWBAPI_STATUS_FAILED;
        break;
    }
    return status;
}

/*******************************************************************************
 **
 ** Function:        getVendorAppConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for extended application related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getVendorAppConfigTLVBuffer(uint8_t paramId, void *paramValue, uint16_t paramValueLen, uint8_t *tlvBuffer)
{
    uint32_t length     = 0;
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    /* Length 1 Byte */
#if (UWBFTR_UL_TDoA_Tag)
    case UCI_VENDOR_PARAM_ID_ULTDOA_MAC_FRAME_FORMAT:
#endif // (UWBFTR_UL_TDoA_Tag)
    case UCI_VENDOR_PARAM_ID_MAC_PAYLOAD_ENCRYPTION:
    case UCI_VENDOR_PARAM_ID_RX_ANTENNA_POLARIZATION_OPTION:
    case UCI_VENDOR_PARAM_ID_SESSION_SYNC_ATTEMPTS:
    case UCI_VENDOR_PARAM_ID_SESSION_SCHED_ATTEMPTS:
    case UCI_VENDOR_PARAM_ID_SCHED_STATUS_NTF:
    case UCI_VENDOR_PARAM_ID_TX_POWER_DELTA_FCC:
    case UCI_VENDOR_PARAM_ID_TEST_KDF_FEATURE:
    case UCI_VENDOR_PARAM_ID_ADAPTIVE_HOPPING_THRESHOLD:
    case UCI_VENDOR_PARAM_ID_RAN_MULTIPLIER:
    case UCI_VENDOR_PARAM_ID_STS_LAST_INDEX_USED:
    case UCI_VENDOR_PARAM_ID_DATA_TRANSFER_TX_STATUS_CONFIG:
#if UWBIOT_UWBD_SR1XXT
    case UCI_VENDOR_PARAM_ID_WIFI_COEX_MAX_TOLERANCE_COUNT:
    case UCI_VENDOR_PARAM_ID_CSA_MAC_MODE:
#endif // UWBIOT_UWBD_SR1XXT
#if (UWBIOT_UWBD_SR150)
    case UCI_VENDOR_PARAM_ID_CSA_FINAL_DATA2_CONFIG:
#endif // (UWBIOT_UWBD_SR150)
    case UCI_VENDOR_PARAM_ID_TX_ADAPTIVE_PAYLOAD_POWER:
    case UCI_VENDOR_PARAM_ID_AUTHENTICITY_TAG:
    case UCI_VENDOR_PARAM_ID_TX_POWER_TEMP_COMPENSATION:
    case UCI_VENDOR_PARAM_ID_MAC_CFG:
    case UCI_VENDOR_PARAM_ID_CIR_LOG_NTF:
    case UCI_VENDOR_PARAM_ID_PSDU_LOG_NTF:
    case UCI_VENDOR_PARAM_ID_SESSION_INBAND_DATA_TX_BLOCKS:
    case UCI_VENDOR_PARAM_ID_SESSION_INBAND_DATA_RX_BLOCKS:
    case UCI_VENDOR_PARAM_ID_RFRAME_LOG_NTF:
#if (UWBIOT_UWBD_SR150)
    case UCI_VENDOR_PARAM_ID_SWAP_ANTENNA_PAIR_3D_AOA:
    case UCI_VENDOR_PARAM_ID_FOV_ENABLE:
#endif // UWBIOT_UWBD_SR150
    {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;

    /* Length 2 Bytes */
    case UCI_VENDOR_PARAM_ID_CIR_CAPTURE_MODE:
    case UCI_VENDOR_PARAM_ID_RX_NBIC_CONFIG:
    case UCI_VENDOR_PARAM_ID_RML_PROXIMITY_CONFIG: {
        tlvBuffer[length++] = sizeof(uint16_t); // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;

    /* Length 4 Bytes */
    case UCI_VENDOR_PARAM_ID_RSSI_AVG_FILT_CNT: {
        tlvBuffer[length++] = sizeof(uint32_t); // Param len
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;

    /* uint8_t array */
#if (UWBIOT_UWBD_SR150)
    case UCI_VENDOR_PARAM_ID_AZIMUTH_FIELD_OF_VIEW:
#endif // UWBIOT_UWBD_SR150
    case UCI_VENDOR_PARAM_ID_ANTENNAE_CONFIGURATION_TX:
    case UCI_VENDOR_PARAM_ID_ANTENNAE_CONFIGURATION_RX:
    case UCI_VENDOR_PARAM_ID_ANTENNAE_SCAN_CONFIGURATION: {
        if ((paramValueLen + length) <= MAX_UCI_PACKET_SIZE) {
            tlvBuffer[length++] = (uint8_t)paramValueLen; // Param len
            phOsalUwb_MemCopy(&tlvBuffer[length], (uint8_t *)paramValue, paramValueLen);
            length += paramValueLen;
        }
        else {
            LOG_E("Max UCI packet size is %d bytes", MAX_UCI_PACKET_SIZE);
            length = 0;
        }
    } break;
    default:
        LOG_E("UCI Extended parameter '0x%04X' not supported", paramId);
        length = 0;
        break;
    }
    if (length > MAX_UCI_PACKET_SIZE) {
        LOG_E("Max UCI Packet size supported for the Session App/EXT configs is %d bytes and received %d bytes",
            MAX_UCI_PACKET_SIZE,
            length);
        length = 0;
    }
    return (uint8_t)length;
}

#if UWBFTR_Radar
/*******************************************************************************
 **
 ** Function:        getExtRadarAppConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for extended radar application related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getExtRadarAppConfigTLVBuffer(uint16_t paramId, void *paramValue, uint16_t paramValueLen, uint8_t *tlvBuffer)
{
    uint8_t length       = 0;
    uint8_t uci_param_id = 0;
    uci_param_id         = (uint8_t)(paramId & 0x00FF);

    if (uci_param_id == UCI_EXT_RADAR_PARAM_ID_RADAR_TEST_FCC_TESTMODE) {
        tlvBuffer[length++] = EXTENDED_RADAR_TEST_CONFIG_ID;
    }
    else {
        tlvBuffer[length++] = EXTENDED_RADAR_CONFIG_ID;
    }
    tlvBuffer[length++] = uci_param_id;

    switch (uci_param_id) {
    /* Length 1 Byte */
    case UCI_EXT_RADAR_PARAM_ID_RADAR_MODE:
    case UCI_EXT_RADAR_PARAM_ID_RADAR_SINGLE_FRAME_NTF:
    case UCI_EXT_RADAR_PARAM_ID_RADAR_CIR_START_INDEX:
    case UCI_EXT_RADAR_PARAM_ID_RADAR_CIR_NUM_SAMPLES:
    case UCI_EXT_RADAR_PARAM_ID_RADAR_CALIBRATION_SUPPORT:
    case UCI_EXT_RADAR_PARAM_ID_RADAR_CALIBRATION_NUM_SAMPLES:
    case UCI_EXT_RADAR_PARAM_ID_RADAR_TEST_FCC_TESTMODE: {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    /* Length 2 Bytes */
    case UCI_EXT_RADAR_PARAM_ID_RADAR_PRF_CFG:
    case UCI_EXT_RADAR_PARAM_ID_RADAR_RADAR_CALIBRATION_INTERVAL: {
        tlvBuffer[length++] = sizeof(uint16_t); // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;

    /* Length 4 Bytes */
    case UCI_EXT_RADAR_PARAM_ID_RADAR_RX_DELAY:
    case UCI_EXT_RADAR_PARAM_ID_RADAR_RX_GAIN: {
        tlvBuffer[length++] = sizeof(uint32_t); // Param len
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;
    /* uint8 array */
    case UCI_EXT_RADAR_PARAM_ID_RADAR_CIR_START_OFFSET: {
        tlvBuffer[length++] = paramValueLen; // Param len
        phOsalUwb_MemCopy(&tlvBuffer[length], paramValue, paramValueLen);
        length += paramValueLen;
    } break;
    default:
        LOG_E("UCI Extended parameter '0x%04X' not supported", paramId);
        length = 0;
        break;
    }

    return length;
}
#endif //UWBFTR_Radar

/*******************************************************************************
 **
 ** Function:        getExtTestConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for extended test related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getExtTestConfigTLVBuffer(uint16_t paramId, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length       = 0;
    uint8_t uci_param_id = 0;
    uci_param_id         = (uint8_t)(paramId & 0x00FF);

    tlvBuffer[length++] = EXTENDED_TEST_CONFIG_ID;
    tlvBuffer[length++] = uci_param_id;

    switch (uci_param_id) {
    case UCI_EXT_TEST_PARAM_ID_RSSI_CALIBRATION_OPTION:
    case UCI_EXT_TEST_SESSION_STS_KEY_OPTION: {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    case UCI_EXT_TEST_PARAM_ID_AGC_GAIN_VAL_RX: {
        tlvBuffer[length++] = sizeof(uint16_t); // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;
    default:
        length = 0;
        break;
    }
    return length;
}

/*******************************************************************************
 **
 ** Function:        getExtTestConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for extended test related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getExtDeviceConfigTLVBuffer(uint8_t paramId, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = 0;

    tlvBuffer[length++] = EXTENDED_DEVICE_CONFIG_ID;
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    case UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT: {
        tlvBuffer[length++] = sizeof(uint16_t); // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;
#if UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_RX_GPIO_ANTENNA_SELECTION:
#endif // UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC:
    case UCI_EXT_PARAM_ID_WTX_COUNT:
    case UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG: {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    default:
        length = 0;
        break;
    }
    return length;
}

/*******************************************************************************
 **
 ** Function:        getVendorDebugConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for Extended Debug Configs only.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getVendorDebugConfigTLVBuffer(uint16_t paramId, void *paramValue, uint16_t paramValueLen, uint8_t *tlvBuffer)
{
    uint32_t length = 0;

    tlvBuffer[length++] = paramId;

    switch (paramId) {
        /* Length 1 Byte */
#if UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_DATA_LOGGER_NTF:
#endif // UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_TEST_CONTENTION_RANGING_FEATURE:
    case UCI_EXT_PARAM_ID_RANGING_TIMESTAMP_NTF: {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    /* Length 2 Byte */
    case UCI_EXT_PARAM_ID_THREAD_SECURE:
    case UCI_EXT_PARAM_ID_THREAD_SECURE_ISR:
    case UCI_EXT_PARAM_ID_THREAD_NON_SECURE_ISR:
    case UCI_EXT_PARAM_ID_THREAD_SHELL:
    case UCI_EXT_PARAM_ID_THREAD_PHY:
    case UCI_EXT_PARAM_ID_THREAD_RANGING:
    case UCI_EXT_PARAM_ID_THREAD_SECURE_ELEMENT:
    case UCI_EXT_PARAM_ID_THREAD_UWB_WLAN_COEX: {
        tlvBuffer[length++] = sizeof(uint16_t); // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;
    /* Length 4 Byte */
    case UCI_EXT_PARAM_ID_CIR_WINDOW: {
        tlvBuffer[length++] = sizeof(uint32_t); // Param len 4
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;
#if UWBIOT_UWBD_SR2XXT
    case UCI_EXT_PARAM_ID_DATA_LOGGER_NTF: {
        if ((paramValueLen + length) <= MAX_UCI_PACKET_SIZE) {
            tlvBuffer[length++] = (uint8_t)paramValueLen; // Param len
            phOsalUwb_MemCopy(&tlvBuffer[length], (uint8_t *)paramValue, paramValueLen);
            length += paramValueLen;
        }
        else {
            LOG_E("Max UCI packet size is %d bytes", MAX_UCI_PACKET_SIZE);
            length = 0;
        }

    } break;
#endif // UWBIOT_UWBD_SR1XXT
    default:
        LOG_E("UCI Extended parameter '0x%04X' not supported", paramId);
        length = 0;
        break;
    }
    if (length > MAX_UCI_PACKET_SIZE) {
        LOG_E("Max UCI packet size is %d bytes", MAX_UCI_PACKET_SIZE);
        length = 0;
    }
    return (uint8_t)length;
}

#if UWBFTR_CCC
/*******************************************************************************
 **
 ** Function:        parseCapabilityCCCParams
 **
 ** Description:     Parse Capability CCC Params Information from the given buffer as per UCI
 **                  pDevCap:  Pointer to structure of device capability data
 **
 ** Returns:         boolean, failure when the length of the received param is incorrect,
 **                  success if the parsing is successful or an unknown Param is received.
 **
 *******************************************************************************/
BOOLEAN parseCapabilityCCCParams(
    phUwbCapInfo_t *pDevCap, uint8_t paramId, uint16_t *index, uint8_t length, uint8_t *capsInfoData)
{
    NXPLOG_UWBAPI_D("Param Id = %X\n", paramId);
    switch (paramId) {
    case SLOT_BITMASK: {
        if (length != DEVICE_CAPABILITY_LEN_1) {
            return FALSE;
        }
        pDevCap->slotBitmask = capsInfoData[(*index)++];
    } break;
    case SYNC_CODE_INDEX_BITMASK: {
        if (length != DEVICE_CAPABILITY_LEN_4) {
            return FALSE;
        }
        phOsalUwb_MemCopy(&pDevCap->syncCodeIndexBitmask, &capsInfoData[*index], length);
        *index = (uint8_t)(*index + length);
    } break;
    case HOPPING_CONFIG_BITMASK: {
        if (length != DEVICE_CAPABILITY_LEN_1) {
            return FALSE;
        }
        pDevCap->hoppingConfigBitmask = capsInfoData[(*index)++];
    } break;
    case CHANNEL_BITMASK: {
        if (length != DEVICE_CAPABILITY_LEN_1) {
            return FALSE;
        }
        pDevCap->channelBitmask = capsInfoData[(*index)++];
    } break;
    case SUPPORTED_PROTOCOL_VERSION: {
        if (length != DEVICE_CAPABILITY_LEN_2) {
            return FALSE;
        }
        phOsalUwb_MemCopy(&pDevCap->supportedProtocolVersion, &capsInfoData[*index], length);
        *index = (uint8_t)(*index + length);
    } break;
    case SUPPORTED_UWB_CONFIG_ID: {
        if (length != DEVICE_CAPABILITY_LEN_2) {
            return FALSE;
        }
        phOsalUwb_MemCopy(&pDevCap->supportedUWBConfigID, &capsInfoData[*index], length);
        *index = (uint8_t)(*index + length);
    } break;
    case SUPPORTED_PULSESHAPE_COMBO: {
        if (length == 0 || length > DEVICE_CAPABILITY_LEN_9) {
            return FALSE;
        }
        phOsalUwb_MemCopy(&pDevCap->supportedPulseShapeCombo, &capsInfoData[*index], length);
        *index = (uint8_t)(*index + length);
    } break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown ccc param Id : 0x%X", __FUNCTION__, paramId);
        *index = (uint8_t)(*index + length);
    }
    return TRUE;
}
#endif // UWBFTR_CCC

/*******************************************************************************
 **
 ** Function:        parseDeviceInfo
 **
 ** Description:     Parse Manufacturer Specific Device Information from the
 **                  Given buffer as per UCI
 **                  pdevInfo:  Pointer to structure of device information data
 **
 ** Returns:         boolean, parse success or failure
 **
 *******************************************************************************/
BOOLEAN parseDeviceInfo(phUwbDevInfo_t *pdevInfo)
{
    uint16_t index = 0;
    uint8_t paramId;
    uint8_t length;
    uint16_t manufacturerLength = uwbContext.rsp_len;
    uint8_t *manufacturerData   = uwbContext.rsp_data;

    if (manufacturerLength == 0) {
        NXPLOG_UWBAPI_E("%s: manufacturerLength is zero", __FUNCTION__);
        return FALSE;
    }

    pdevInfo->uciGenericMajor                   = manufacturerData[index++];
    pdevInfo->uciGenericMinorMaintenanceVersion = manufacturerData[index++];
    pdevInfo->macMajorVersion                   = manufacturerData[index++];
    pdevInfo->macMinorMaintenanceVersion        = manufacturerData[index++];
    pdevInfo->phyMajorVersion                   = manufacturerData[index++];
    pdevInfo->phyMinorMaintenanceVersion        = manufacturerData[index++];
    pdevInfo->mwMajor                           = UWBIOTVER_STR_VER_MAJOR;
    pdevInfo->mwMinor                           = UWBIOTVER_STR_VER_MINOR;
    pdevInfo->mwRc                              = UWBIOTVER_STR_VER_DEV;

    index = index + 2; // skip test version
    index++;           // skip extended params length

    while (index < manufacturerLength) {
        paramId = manufacturerData[index++];
        length  = manufacturerData[index++];

        NXPLOG_UWBAPI_D("Extended Device Param Id = %d", paramId);
        switch (paramId) {
        case UCI_EXT_PARAM_ID_DEVICE_NAME:
            if (length > sizeof(pdevInfo->devName)) {
                NXPLOG_UWBAPI_E("%s : device name data size is more than response buffer", __FUNCTION__);
                return FALSE;
            }
            pdevInfo->devNameLen = length;
            if (length != 0) {
                phOsalUwb_MemCopy(pdevInfo->devName, &manufacturerData[index], length);
                if (length < sizeof(pdevInfo->devName)) { /*check is added to avoid the coverity warning*/
                    pdevInfo->devName[length] = '\0';
                }
            }
            index = index + length;
            break;
        case UCI_EXT_PARAM_ID_FW_VERSION:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return FALSE;
            }
            pdevInfo->fwMajor = manufacturerData[index++];
            pdevInfo->fwMinor = manufacturerData[index++];
            pdevInfo->fwRc    = manufacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_VENDOR_UCI_VER:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return FALSE;
            }
            pdevInfo->nxpUciMajor = manufacturerData[index++];
            pdevInfo->nxpUciMinor = manufacturerData[index++];
            pdevInfo->nxpUciPatch = manufacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_UWB_CHIP_ID:
            if (length != MAX_UWB_CHIP_ID_LEN) {
                NXPLOG_UWBAPI_E("%s: nxp chip id Legnth %d is not equal to expected %d bytes",
                    __FUNCTION__,
                    length,
                    MAX_UWB_CHIP_ID_LEN);
                return FALSE;
            }
            phOsalUwb_MemCopy(pdevInfo->nxpChipId, &manufacturerData[index], length);
            index = index + length;
            break;
        case UCI_EXT_PARAM_ID_UWBS_MAX_PPM_VALUE:
            if (length != MAX_PPM_VALUE_LEN) {
                NXPLOG_UWBAPI_E("%s: PPM Value Legnth %d is not equal to expected %d bytes",
                    __FUNCTION__,
                    length,
                    MAX_PPM_VALUE_LEN);
                return FALSE;
            }
            pdevInfo->maxPpmValue = manufacturerData[index];
            index                 = index + length;
            break;
        case UCI_EXT_PARAM_ID_TX_POWER:
            if (length != MAX_TX_POWER_LEN) {
                NXPLOG_UWBAPI_E("%s: TX Power Legnth %d is not equal to expected %d bytes ",
                    __FUNCTION__,
                    length,
                    MAX_TX_POWER_LEN);
                return FALSE;
            }
            phOsalUwb_MemCopy(pdevInfo->txPowerValue, &manufacturerData[index], length);
            index = index + length;
            break;
#if UWBIOT_UWBD_SR2XXT
        case UCI_EXT_PARAM_UWBS_CAL_MODE: {
            if (length != MAX_CAL_MODE_LEN) {
                return FALSE;
            }
            pdevInfo->lifecycle[0] = manufacturerData[index++];
            pdevInfo->lifecycle[1] = manufacturerData[index++];
            pdevInfo->lifecycle[2] = manufacturerData[index++];
            pdevInfo->lifecycle[3] = manufacturerData[index++];
        } break;
#endif // UWBIOT_UWBD_SR2XXT
        case UCI_EXT_PARAM_ID_FIRA_EXT_UCI_GENERIC_VER:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return FALSE;
            }
            pdevInfo->uciGenericMajor                   = manufacturerData[index++];
            pdevInfo->uciGenericMinorMaintenanceVersion = manufacturerData[index++];
            pdevInfo->uciGenericPatch                   = manufacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_FIRA_EXT_TEST_VER:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return FALSE;
            }
            pdevInfo->uciTestMajor = manufacturerData[index++];
            pdevInfo->uciTestMinor = manufacturerData[index++];
            pdevInfo->uciTestPatch = manufacturerData[index++];
            break;
#if UWBIOT_UWBD_SR2XXT
        case UCI_EXT_PARAM_ID_UWB_FW_GIT_HASH: {
            if (length != FW_GIT_HASH_LEN) {
                NXPLOG_UWBAPI_E("%s: fwgitHash size %d bytes, is not equal to expected %d bytes",
                    __FUNCTION__,
                    length,
                    FW_GIT_HASH_LEN);
                return FALSE;
            }
            if (length != 0) {
                phOsalUwb_MemCopy(pdevInfo->fwGitHash, &manufacturerData[index], length);
            }
            index = index + length;
            break;
        }
#endif // UWBIOT_UWBD_SR2XXT
        case UCI_EXT_PARAM_ID_FW_BOOT_MODE:
            if (length != FW_BOOT_MODE_LEN) {
                NXPLOG_UWBAPI_E("%s: fwBootMode id size %d is not equal to expected %d bytes ",
                    __FUNCTION__,
                    length,
                    FW_BOOT_MODE_LEN);
                return FALSE;
            }
            pdevInfo->fwBootMode = manufacturerData[index];
            index                = index + length;
            break;
#if UWBFTR_CCC
        case UCI_EXT_PARAM_ID_UCI_CCC_VERSION: {
            if (length > sizeof(pdevInfo->uciCccVersion)) {
                NXPLOG_UWBAPI_E("%s: UCI CCC version size %d is more than response buffer", __FUNCTION__, length);
                return FALSE;
            }
            if (length != 0) {
                phOsalUwb_MemCopy(pdevInfo->uciCccVersion, &manufacturerData[index], length);
            }
            index = index + length;
        } break;

        case UCI_EXT_PARAM_ID_CCC_VERSION: {
            if (length > sizeof(pdevInfo->cccVersion)) {
                NXPLOG_UWBAPI_E("%s: CCC version size %d is more than response buffer", __FUNCTION__, length);
                return FALSE;
            }
            if (length != 0) {
                phOsalUwb_MemCopy(pdevInfo->cccVersion, &manufacturerData[index], length);
            }
            index = index + length;
        } break;

#endif // UWBFTR_CCC
        default:
            NXPLOG_UWBAPI_W("%s: unknown param Id 0x%X", __FUNCTION__, paramId);
            // Ignore the Unknown Param.
            index = index + length;
        }
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function:        parseDebugParams
 **
 ** Description:     Extracts Debug Params from the given byte array and updates
 *the structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseDebugParams(uint8_t *rspPtr, uint8_t noOfParams, UWB_DebugParams_List_t *DebugParams_List)
{
    for (int i = 0; i < noOfParams; i++) {
        uint8_t paramId = *rspPtr++;
        uint8_t length  = *rspPtr++;
        uint8_t *value  = rspPtr;
        switch (paramId) {
        case UCI_EXT_PARAM_ID_THREAD_SECURE:
            UWB_STREAM_TO_UINT16(DebugParams_List->param_value.vu16, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_SECURE_ISR:
            UWB_STREAM_TO_UINT16(DebugParams_List->param_value.vu16, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_NON_SECURE_ISR:
            UWB_STREAM_TO_UINT16(DebugParams_List->param_value.vu16, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_SHELL:
            UWB_STREAM_TO_UINT16(DebugParams_List->param_value.vu16, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_PHY:
            UWB_STREAM_TO_UINT16(DebugParams_List->param_value.vu16, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_RANGING:
            UWB_STREAM_TO_UINT16(DebugParams_List->param_value.vu16, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_SECURE_ELEMENT:
            UWB_STREAM_TO_UINT16(DebugParams_List->param_value.vu16, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_UWB_WLAN_COEX:
            UWB_STREAM_TO_UINT16(DebugParams_List->param_value.vu16, value);
            break;

        case UCI_EXT_PARAM_ID_DATA_LOGGER_NTF:
#if UWBIOT_UWBD_SR1XXT
            UWB_STREAM_TO_UINT8(DebugParams_List->param_value.vu8, value);
#else
            UWB_ARRAY_TO_STREAM(DebugParams_List->param_value.param.param_value, value, length);
#endif // UWBIOT_UWBD_SR1XXT
            break;
        case UCI_EXT_PARAM_ID_TEST_CONTENTION_RANGING_FEATURE:
            UWB_STREAM_TO_UINT8(DebugParams_List->param_value.vu8, value);
            break;
        case UCI_EXT_PARAM_ID_CIR_WINDOW:
            UWB_STREAM_TO_UINT32(DebugParams_List->param_value.vu32, value);
            break;
        case UCI_EXT_PARAM_ID_RANGING_TIMESTAMP_NTF:
            UWB_STREAM_TO_UINT8(DebugParams_List->param_value.vu8, value);
            break;
        default:
            LOG_E("UCI Extended parameter '0x%04X' not supported", paramId);
            break;
        }
        rspPtr += length;
        DebugParams_List++;
    }
}
/*******************************************************************************
 **
 ** Function:        getExtCoreDeviceConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for Ext Core Device configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getExtCoreDeviceConfigTLVBuffer(uint16_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = (uint8_t)(paramLen + UWBD_EXT_CONFIG_HEADER_LEN);
    phDdfsToneConfig_t *ddfsToneConfig;
    uint8_t uci_param_id = 0;

    if (paramValue == NULL || tlvBuffer == NULL) {
        NXPLOG_UWBAPI_E("%s: Buffer is NULL", __FUNCTION__);
        return 0;
    }

    uci_param_id = (uint8_t)(paramId & 0x00FF);
    UWB_UINT8_TO_STREAM(tlvBuffer, EXTENDED_DEVICE_CONFIG_ID);
    UWB_UINT8_TO_STREAM(tlvBuffer, uci_param_id);

    switch (uci_param_id) {
    /* 1 byte len */
    case UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC: {
        uint8_t value = *((uint8_t *)paramValue);
        if (value != DPD_WAKEUP_SRC_GPIO_1 && value != DPD_WAKEUP_SRC_GPIO_3) {
            return 0;
        }
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value);
    } break;
    case UCI_EXT_PARAM_ID_WTX_COUNT: {
        uint8_t value = *((uint8_t *)paramValue);
        if (value < WTX_COUNT_MIN || value > WTX_COUNT_MAX) {
            return 0;
        }
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value);
    } break;
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    case UCI_EXT_PARAM_ID_WIFI_CO_EX_CH_CFG: {
        uint8_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value);
    } break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    case UCI_EXT_PARAM_ID_TX_BASE_BAND_CONFIG: {
        uint8_t value = *((uint8_t *)paramValue);
        if (value != ENABLED && value != DISABLED) {
            return 0;
        }
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value);
    } break;
#if UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_RX_GPIO_ANTENNA_SELECTION: {
        uint8_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value);
    } break;
#endif //UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG: {
        uint8_t value = *((uint8_t *)paramValue);
        if (value != ENABLED && value != DISABLED) {
            return 0;
        }
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value);
    } break;

    /* Length 2 byte */
    case UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT: {
        uint16_t value = *((uint16_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint16_t));
        UWB_UINT16_TO_STREAM(tlvBuffer, value);
    } break;

    case UCI_EXT_PARAM_ID_CLK_CONFIG_CTRL: {
        phClkConfigSrc_t *value = ((phClkConfigSrc_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(phClkConfigSrc_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value->clk_src_opt);
#if UWBIOT_UWBD_SR1XXT
        UWB_UINT8_TO_STREAM(tlvBuffer, value->xtal_opt);
#elif UWBIOT_UWBD_SR2XXT
        UWB_UINT16_TO_STREAM(tlvBuffer, value->slow_clk_wait);
        UWB_UINT16_TO_STREAM(tlvBuffer, value->rf_clk_wait);
#endif // UWBIOT_UWBD_SR1XXT
    } break;

#if UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_CLOCK_PRESENT_WAITING_TIME: {
        uint16_t clockPresentWaitingTime = *((uint16_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(clockPresentWaitingTime));
        UWB_UINT16_TO_STREAM(tlvBuffer, clockPresentWaitingTime);
    } break;
#endif

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    case UCI_EXT_PARAM_ID_WIFI_COEX_UART_USER_CFG: {
        uint8_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value);
    } break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

#if UWBIOT_UWBD_SR2XXT
    case UCI_EXT_PARAM_ID_DDFS_CONFIG_PER_PULSE_SHAPE: {
        uint32_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint32_t));
        UWB_UINT32_TO_STREAM(tlvBuffer, value);
    } break;

    case UCI_EXT_PARAM_ID_AOA_MODE: {
        uint8_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value);
    } break;
#endif // UWBIOT_UWBD_SR2XXT

    case UCI_EXT_PARAM_ID_TX_PULSE_SHAPE_CONFIG: {
        uint32_t value = *((uint32_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint32_t));
        UWB_UINT32_TO_STREAM(tlvBuffer, value);
    } break;

    case UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_ABS: {
        uint16_t value = *((uint16_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint16_t));
        UWB_UINT16_TO_STREAM(tlvBuffer, value);
    } break;

    case UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_REL: {
        uint16_t value = *((uint16_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint16_t));
        UWB_UINT16_TO_STREAM(tlvBuffer, value);
    } break;

    case UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG: {
        ddfsToneConfig = (phDdfsToneConfig_t *)paramValue;
        UWB_UINT8_TO_STREAM(tlvBuffer, paramLen);
        for (int i = 0; i < NO_OF_BLOCKS; i++) {
            UWB_UINT8_TO_STREAM(tlvBuffer, ddfsToneConfig[i].channel_no);
            UWB_UINT8_TO_STREAM(tlvBuffer, ddfsToneConfig[i].tx_antenna_selection);
            UWB_UINT32_TO_STREAM(tlvBuffer, ddfsToneConfig[i].tx_ddfs_tone_0);
            UWB_UINT32_TO_STREAM(tlvBuffer, ddfsToneConfig[i].tx_ddfs_tone_1);
            UWB_UINT32_TO_STREAM(tlvBuffer, ddfsToneConfig[i].spur_duration);
            UWB_UINT8_TO_STREAM(tlvBuffer, ddfsToneConfig[i].gainval_set);
            UWB_UINT8_TO_STREAM(tlvBuffer, ddfsToneConfig[i].ddfsgainbypass_enbl);
            UWB_UINT16_TO_STREAM(tlvBuffer, ddfsToneConfig[i].periodicity);
        }
    } break;
    case UCI_EXT_PARAM_ID_PDOA_CALIB_TABLE_DEFINE: {
        phPdoaTableDef_t *value = ((phPdoaTableDef_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint16_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, value->calibStepSize);
        UWB_UINT8_TO_STREAM(tlvBuffer, value->noSteps);
    } break;
    case UCI_EXT_PARAM_ID_ANTENNA_RX_IDX_DEFINE:
    case UCI_EXT_PARAM_ID_ANTENNA_TX_IDX_DEFINE:
    case UCI_EXT_PARAM_ID_ANTENNAE_RX_PAIR_DEFINE: {
        phAntennaDefines_t *antennaDefines = (phAntennaDefines_t *)paramValue;
        if (antennaDefines->antennaDefsLen <= MAX_UCI_PACKET_SIZE - MAX_UCI_HEADER_SIZE) {
            UWB_UINT8_TO_STREAM(tlvBuffer, antennaDefines->antennaDefsLen);
            UWB_ARRAY_TO_STREAM(tlvBuffer, antennaDefines->antennaDefs, antennaDefines->antennaDefsLen);
        }
        else {
            LOG_E("%s : invalid antenna defines ", __FUNCTION__);
            return 0;
        }
    } break;

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    case UCI_EXT_PARAM_ID_WIFI_COEX_FEATURE: {
        UWB_WiFiCoEx_Ftr_t *WiFiCoEx_Ftr = (UWB_WiFiCoEx_Ftr_t *)paramValue;

        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(UWB_WiFiCoEx_Ftr_t));
        UWB_UINT8_TO_STREAM(tlvBuffer, WiFiCoEx_Ftr->UWB_WiFiCoEx_Enable);
        UWB_UINT8_TO_STREAM(tlvBuffer, WiFiCoEx_Ftr->UWB_WiFiCoEx_MinGuardDuration);
        UWB_UINT8_TO_STREAM(tlvBuffer, WiFiCoEx_Ftr->UWB_WiFiCoEx_MaxWifiBlockDuration);
        UWB_UINT8_TO_STREAM(tlvBuffer, WiFiCoEx_Ftr->UWB_WiFiCoEx_GuardDuration);

    } break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

    default:
        length = 0;
        break;
    }
    return length;
}

/*******************************************************************************
 **
 ** Function:        parseExtGetDeviceConfigResponse
 **
 ** Description:     Convert received UCI response to deviceConfig object
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseExtGetDeviceConfigResponse(uint8_t *tlvBuffer, phDeviceConfigData_t *devConfig)
{
    uint16_t paramId;
    tlvBuffer++; // skipping the extended device config ID
    UWB_STREAM_TO_UINT8(paramId, tlvBuffer);
    uint8_t totalLen = *tlvBuffer++;

    switch (paramId) {
        /* 1 byte len */
    case UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC: {
        UWB_STREAM_TO_UINT8(devConfig->dpdWakeupSrc, tlvBuffer);
    } break;
    case UCI_EXT_PARAM_ID_WTX_COUNT: {
        UWB_STREAM_TO_UINT8(devConfig->wtxCountConfig, tlvBuffer);
    } break;
    case UCI_EXT_PARAM_ID_TX_BASE_BAND_CONFIG: {
        UWB_STREAM_TO_UINT8(devConfig->txBaseBandConfig, tlvBuffer);
    } break;
#if UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_RX_GPIO_ANTENNA_SELECTION: {
        UWB_STREAM_TO_UINT8(devConfig->rxAntennaSelectionConfig, tlvBuffer);
    } break;
#endif //UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG: {
        UWB_STREAM_TO_UINT8(devConfig->nxpExtendedNtfConfig, tlvBuffer);
    } break;

    /* Length 2 byte */
    case UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT: {
        UWB_STREAM_TO_UINT16(devConfig->dpdEntryTimeout, tlvBuffer);
    } break;

    case UCI_EXT_PARAM_ID_TX_PULSE_SHAPE_CONFIG: {
        UWB_STREAM_TO_UINT8(devConfig->txPulseShapeConfig.shape_id, tlvBuffer);
        UWB_STREAM_TO_UINT8(devConfig->txPulseShapeConfig.payload_tx_shape_id, tlvBuffer);
        UWB_STREAM_TO_UINT8(devConfig->txPulseShapeConfig.sts_shape_id, tlvBuffer);
        UWB_STREAM_TO_UINT8(devConfig->txPulseShapeConfig.dac_stage_cofig, tlvBuffer);
    } break;

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    case UCI_EXT_PARAM_ID_WIFI_COEX_FEATURE: {
        UWB_STREAM_TO_UINT8(devConfig->wifiCoExFtr.UWB_WiFiCoEx_Enable, tlvBuffer);
        UWB_STREAM_TO_UINT8(devConfig->wifiCoExFtr.UWB_WiFiCoEx_MinGuardDuration, tlvBuffer);
        UWB_STREAM_TO_UINT8(devConfig->wifiCoExFtr.UWB_WiFiCoEx_MaxWifiBlockDuration, tlvBuffer);
        UWB_STREAM_TO_UINT8(devConfig->wifiCoExFtr.UWB_WiFiCoEx_GuardDuration, tlvBuffer);
    } break;

    case UCI_EXT_PARAM_ID_WIFI_CO_EX_CH_CFG: {
        UWB_STREAM_TO_UINT8(devConfig->wifiCoExChannelCfg, tlvBuffer);
    } break;

    case UCI_EXT_PARAM_ID_WIFI_COEX_UART_USER_CFG: {
        UWB_STREAM_TO_UINT8(devConfig->wifiCoexUartUserCfg, tlvBuffer);
    } break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

    case UCI_EXT_PARAM_ID_CLK_CONFIG_CTRL: {
        UWB_STREAM_TO_UINT8(devConfig->clockConfigCtrl.clk_src_opt, tlvBuffer);
#if UWBIOT_UWBD_SR1XXT
        UWB_STREAM_TO_UINT8(devConfig->clockConfigCtrl.xtal_opt, tlvBuffer);
#elif UWBIOT_UWBD_SR2XXT
        UWB_STREAM_TO_UINT16(devConfig->clockConfigCtrl.slow_clk_wait, tlvBuffer);
        UWB_STREAM_TO_UINT16(devConfig->clockConfigCtrl.rf_clk_wait, tlvBuffer);
#endif // UWBIOT_UWBD_SR1XXT
    } break;

#if UWBIOT_UWBD_SR1XXT
    case UCI_EXT_PARAM_ID_CLOCK_PRESENT_WAITING_TIME: {
        UWB_STREAM_TO_UINT16(devConfig->clockPresentWaitingTime, tlvBuffer);
    } break;
#endif

    case UCI_EXT_PARAM_ID_HOST_MAX_UCI_PAYLOAD_LENGTH: {
        UWB_STREAM_TO_UINT16(devConfig->hostMaxUCIPayloadLen, tlvBuffer);
    } break;

    case UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG: {
        for (int i = 0; i < NO_OF_BLOCKS; i++) {
            UWB_STREAM_TO_UINT8(devConfig->ddfsToneConfig[i].channel_no, tlvBuffer);
            UWB_STREAM_TO_UINT8(devConfig->ddfsToneConfig[i].tx_antenna_selection, tlvBuffer);
            UWB_STREAM_TO_UINT32(devConfig->ddfsToneConfig[i].tx_ddfs_tone_0, tlvBuffer);
            UWB_STREAM_TO_UINT32(devConfig->ddfsToneConfig[i].tx_ddfs_tone_1, tlvBuffer);
            UWB_STREAM_TO_UINT32(devConfig->ddfsToneConfig[i].spur_duration, tlvBuffer);
            UWB_STREAM_TO_UINT8(devConfig->ddfsToneConfig[i].gainval_set, tlvBuffer);
            UWB_STREAM_TO_UINT8(devConfig->ddfsToneConfig[i].ddfsgainbypass_enbl, tlvBuffer);
            UWB_STREAM_TO_UINT16(devConfig->ddfsToneConfig[i].periodicity, tlvBuffer);
        }
    } break;

#if UWBIOT_UWBD_SR2XXT
    case UCI_EXT_PARAM_ID_DDFS_CONFIG_PER_PULSE_SHAPE: {
        UWB_STREAM_TO_UINT32(devConfig->ddfsCfgPerPulseShape, tlvBuffer);
    } break;
    case UCI_EXT_PARAM_ID_AOA_MODE: {
        UWB_STREAM_TO_UINT8(devConfig->aoaMode, tlvBuffer);
    } break;
#endif // UWBIOT_UWBD_SR2XXT
    case UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_ABS: {
        UWB_STREAM_TO_UINT16(devConfig->initialRxOnOffsetAbs, tlvBuffer);
    } break;
    case UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_REL: {
        UWB_STREAM_TO_UINT16(devConfig->initialRxOnOffsetRel, tlvBuffer);
    } break;
    case UCI_EXT_PARAM_ID_PDOA_CALIB_TABLE_DEFINE: {
        UWB_STREAM_TO_UINT8(devConfig->pdoaCalibTableDef.calibStepSize, tlvBuffer);
        UWB_STREAM_TO_UINT8(devConfig->pdoaCalibTableDef.noSteps, tlvBuffer);
    } break;
    case UCI_EXT_PARAM_ID_ANTENNA_RX_IDX_DEFINE:
    case UCI_EXT_PARAM_ID_ANTENNA_TX_IDX_DEFINE:
    case UCI_EXT_PARAM_ID_ANTENNAE_RX_PAIR_DEFINE: {
        phOsalUwb_MemCopy(devConfig->antennaDefines.antennaDefs, tlvBuffer, totalLen);
        devConfig->antennaDefines.antennaDefsLen = totalLen;
    } break;
    default:
        break;
    }
}

#if UWBIOT_UWBD_SR150
/*******************************************************************************
 **
 ** Function:        parseExtGetCalibResponse
 **
 ** Description:     Convert received UCI response to phCalibRespStatus_t object
 **
 ** Returns:         None
 **
 **
 *******************************************************************************/
void parseExtGetCalibResponse(uint8_t *tlvBuffer, phCalibRespStatus_t *calibResp)
{
    uint16_t paramId;
    UWB_STREAM_TO_UINT8(calibResp->status, tlvBuffer);
    calibResp->calibState = (eCalibState)*tlvBuffer++;

    tlvBuffer++; // skipping the extended calib config ID
    UWB_STREAM_TO_UINT8(paramId, tlvBuffer);

    switch(paramId){
        case UCI_EXT_PARAM_ID_AOA_ANTENNAS_PDOA_CALIB:{
            UWB_STREAM_TO_UINT16(calibResp->length, tlvBuffer);
            /**
             * As of Now this command response is ~700 bytes and calibValueOut max buffer is 750 Bytes .
             * In case of more bytes needs to be updated ,either buffer for the calibvalue should be update , or
             * Dynamically create the memory based on the calibResp->length .
             */
            if((calibResp->length > 0 ) && (calibResp->length < MAX_API_PACKET_SIZE )){
                phOsalUwb_MemCopy(&calibResp->calibValueOut[0], tlvBuffer, calibResp->length);
            }
            else{
                LOG_E("%s Calibration Buffer Overflow or 0, recvd:%d", __FUNCTION__, calibResp->length);
            }
        }break;
        default :{
            LOG_E("%s Unknown Ext-calibration Param id Found %0x ", __FUNCTION__, paramId);
        }break;
    }
}

#endif // UWBIOT_UWBD_SR150
/*******************************************************************************
 **
 ** Function         handle_schedstatus_ntf
 **
 ** Description      This function is called to process Scheduler Status
 **                  notification
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_schedstatus_ntf(uint8_t *p, uint16_t len)
{
    phSchedStatusNtfData_t sSchedStatusNtf_data;

    if (len > 0) {
        sSchedStatusNtf_data.dataLength = len;
        if (len <= MAX_UCI_PACKET_SIZE) {
            phOsalUwb_MemCopy(sSchedStatusNtf_data.data, p, len);
        }
        else {
            LOG_E("%s Not enough buffer to store %d bytes", __FUNCTION__, len);
            return;
        }
    }
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_SCHEDULER_STATUS_NTF, &sSchedStatusNtf_data);
    }
}

#if UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200T
/*******************************************************************************
 **
 ** Function         handle_se_com_err_ntf
 **
 ** Description      This function is called to notify se comm err
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_se_com_err_ntf(uint8_t *p, uint16_t len)
{
    if (len != 0) {
        NXPLOG_UWBAPI_W("%s: SE_COMM_ERR, status %d", __FUNCTION__, (uint8_t)(*(p)));
    }
    else {
        NXPLOG_UWBAPI_W("%s Invalid length %d", __FUNCTION__, len);
    }
}

#endif //UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200T

/*******************************************************************************
 **
 ** Function         handle_do_vco_pll_calibration_ntf
 **
 ** Description      This function is called to process do calibration
 *notification
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_do_vco_pll_calibration_ntf(uint8_t *p, uint16_t len)
{
    phOsalUwb_SetMemory(&uwbContext.rsp_data[0], 0x00, sizeof(uwbContext.rsp_data));
    if ((len > 0) && (len <= sizeof(uwbContext.rsp_data))) {
        uwbContext.rsp_len = len;
        UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, p, len);
    }
    else {
        LOG_E("%s Invalid length %d", __FUNCTION__, len);
    }
}

/*******************************************************************************
 **
 ** Function         handle_read_calibration_data_ntf
 **
 ** Description      This function is called to process read calibration data
 **                  notification
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_read_calibration_data_ntf(uint8_t *p, uint16_t len)
{
    phOsalUwb_SetMemory(&uwbContext.rsp_data, 0x00, MAX_UCI_PACKET_SIZE);
    if (len > 0) {
        UWB_STREAM_TO_UINT8(uwbContext.wstatus, p);
        UWB_STREAM_TO_UINT8(uwbContext.rsp_len, p);
        UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, p, uwbContext.rsp_len);
    }
}

#if UWBFTR_UWBS_DEBUG_Dump
/*******************************************************************************
 **
 ** Function         handle_debug_ntf
 **
 ** Description      This function is called to process debug notification.
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_debug_ntf(eResponse_Ext_Ntf_Event event, uint8_t *p, uint16_t len)
{
    phDebugData_t sDebug_data;
    eNotificationType ntfType = UWBD_RANGING_DATA;
    if (len > 0) {
        if (len <= MAX_DEBUG_NTF_SIZE) {
            sDebug_data.dataLength = len;
            phOsalUwb_MemCopy(sDebug_data.data, p, len);
        }
        else {
            LOG_E("%s Not enough buffer to store %d bytes", __FUNCTION__, len);
            return;
        }
    }
    if (uwbContext.pAppCallback) {
        if (event == UWA_DM_VENDOR_CIR_LOG_NTF_EVT) {
            ntfType = UWBD_CIR_DATA_NTF;
        }
        else if (event == UWA_DM_PROP_DBG_DATA_LOGGER_NTF_EVT) {
            ntfType = UWBD_DATA_LOGGER_NTF;
        }
        else if (event == UWA_DM_VENDOR_PSDU_LOG_NTF_EVT) {
            ntfType = UWBD_PSDU_DATA_NTF;
        }
        else if (event == UWA_DM_INTERNAL_RANGING_TIMESTAMP_NTF_EVT) {
            ntfType = UWBD_RANGING_TIMESTAMP_NTF;
        }
        uwbContext.pAppCallback(ntfType, &sDebug_data);
    }
}

/*******************************************************************************
 **
 ** Function         handle_rframe_log_ntf
 **
 ** Description      This function is called to process rframe log notifications
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_rframe_log_ntf(uint8_t *p, uint16_t len)
{
    phRframeData_t sRframe_log_data;

    if (len > 0) {
        if (len <= MAX_RFRAME_PACKET_SIZE) {
            sRframe_log_data.dataLength = len;
            phOsalUwb_MemCopy(sRframe_log_data.data, p, len);
        }
        else {
            LOG_E("%s Not enough buffer to store %d bytes", __FUNCTION__, len);
            return;
        }
    }
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_RFRAME_DATA, &sRframe_log_data);
    }
}

#endif //UWBFTR_UWBS_DEBUG_Dump


#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
/*********************************************************************************
 **
 ** Function         handle_WiFiCoExInd_ntf
 **
 ** Description      This function is called to process WiFi CoEx Ind
 **                  notification
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_WiFiCoExInd_ntf(uint8_t *responsePayloadPtr)
{
    UWB_CoEx_IndNtf_t wifiCoExIndNtf;

    UWB_STREAM_TO_UINT8(wifiCoExIndNtf.status, responsePayloadPtr);
    UWB_STREAM_TO_UINT32(wifiCoExIndNtf.slot_index, responsePayloadPtr);
    UWB_STREAM_TO_UINT32(wifiCoExIndNtf.sessionHandle, responsePayloadPtr);

    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_WIFI_COEX_IND_NTF, &wifiCoExIndNtf);
    }
}

/*******************************************************************************
 **
 ** Function         handle_WiFiCoEx_ActGrantDurationNtf
 **
 ** Description      This function is called to process WiFi CoEx
 **                  Max Active Grant Duration Status notification
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_WiFiCoEx_ActGrantDurationNtf(uint8_t *responsePayloadPtr)
{
    uint8_t actGrantDuration = 0x00;
    UWB_STREAM_TO_UINT8(actGrantDuration, responsePayloadPtr);
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_MAX_ACTIVE_GRANT_DURATION_EXCEEDED_WAR_NTF, &actGrantDuration);
    }
}
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

#if !(MCTT_PCTT_BIN_WITH_SSG_FW)
/*******************************************************************************
 **
 ** Function         setDefaultAoaCalibration
 **
 ** Description      This function set the default AOA Calibration Parmeters
 **
 ** Returns          UWBAPI_STATUS_OK if successful, otherwise UWBAPI_STATUS_FAILED
 **
 *******************************************************************************/
static tUWBAPI_STATUS setDefaultAoaCalibration()
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_OK;
    void *p_cmd           = NULL;
    long cmd_len          = 0;
    uint8_t count         = 0;

    LOG_D("%s : Enter", __FUNCTION__);

#if UWBIOT_UWBD_SR150
    sAoACoreConfigs_t aoaConfigs[] = {
        {.key = UWB_AOA_CH5_CONFIG_BLOCK_COUNT, .pValue = aoa_ch5_config_block_names},
        {.key = UWB_AOA_CH9_CONFIG_BLOCK_COUNT, .pValue = aoa_ch9_config_block_names},
    };
#else
    sAoACoreConfigs_t aoaConfigs[] = {
        {.key = UWB_AOA_CONFIG_BLOCK_COUNT, .pValue = aoa_config_block_names},
    };
#endif // UWBIOT_UWBD_SR150

    for (uint8_t iter = 0; iter < GET_ARRAY_SIZE(aoaConfigs); iter++) {
        if (phNxpUciHal_GetNxpNumValue(aoaConfigs[iter].key, &count, sizeof(count))) {
            LOG_D("%s : Number of AOA calibration config count is %d", __FUNCTION__, count);
            for (int i = 0; i < count; i++) {
                if ((phNxpUciHal_GetNxpByteArrayValue(aoaConfigs[iter].pValue[i], &p_cmd, &cmd_len) == TRUE) &&
                    cmd_len > 0) {
                    status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
                    if (status != UWBAPI_STATUS_OK) {
                        LOG_E("%s : set aoa calibration for block %d failed", __FUNCTION__, count);
                        break;
                    }
                }
                else {
                    status = UWBAPI_STATUS_FAILED;
                    LOG_E("%s: calibration len for block %d is : %ld", __FUNCTION__, count, cmd_len);
                    break;
                }
            }
        }
    }
    LOG_D("%s: Exit ", __FUNCTION__);
    return status;
}
#endif // !(MCTT_PCTT_BIN_WITH_SSG_FW)

#if (UWBFTR_SE_SN110)
/**
 **
 ** Function         handle_se_binding_ntf
 **
 ** Description      This function is called to process se binding realted notifications
 **
 ** Returns          void
 **
 */
static void handle_se_binding_ntf(uint8_t *p, uint16_t len)
{
    phOsalUwb_SetMemory(&uwbContext.rsp_data, 0x00, MAX_UCI_PACKET_SIZE);
    if ((len > 0) && (len <= sizeof(uwbContext.rsp_data))) {
        uwbContext.rsp_len = len;
        UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, p, len);
    }
    else {
        LOG_E("%s Invalid length %d", __FUNCTION__, len);
    }
}
#endif //(UWBFTR_SE_SN110)

/*******************************************************************************
 **
 ** Function         processProprietaryNtf
 **
 ** Description      Process UCI NTFs in the proprietary group
 **
 ** Returns          associated event id
 **
 *******************************************************************************/
eResponse_Ext_Ntf_Event processProprietaryNtf(uint8_t oid, uint16_t responsePayloadLen, uint8_t *responsePayloadPtr)
{
    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;

    switch (oid) {
#if UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S // FIXME:
    case EXT_UCI_MSG_SE_DO_TEST_LOOP: {
        dmExtEvent = UWA_DM_PROP_SE_TEST_LOOP_NTF_EVT;
        handle_se_com_err_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    case EXT_UCI_MSG_GENERATE_TAG: {
        dmExtEvent = UWA_DM_PROP_GENERATE_TAG_NTF_EVT;
        handle_se_com_err_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#endif //UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S
#if UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200T
    case EXT_UCI_MSG_SE_COMM_ERROR_NTF: {
        dmExtEvent = UWA_DM_PROP_SE_COM_ERROR_NTF_EVT;
        handle_se_com_err_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#endif //#if UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200T
    case EXT_UCI_MSG_SCHEDULER_STATUS_NTF: {
        dmExtEvent = UWA_DM_PROP_SCHEDULER_STATUS_NTF_EVT;
        handle_schedstatus_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#if UWBFTR_UWBS_DEBUG_Dump
    case EXT_UCI_MSG_DBG_DATA_LOGGER_NTF: {
        dmExtEvent = UWA_DM_PROP_DBG_DATA_LOGGER_NTF_EVT;
        handle_debug_ntf(dmExtEvent, responsePayloadPtr, responsePayloadLen);
    } break;
#endif
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    case EXT_UCI_MSG_UWB_WIFI_COEX_IND_NTF: {
        dmExtEvent = UWA_DM_PROP_UWB_WIFI_COEX_IND_NTF_EVT;
        handle_WiFiCoExInd_ntf(responsePayloadPtr);
    } break;
    case EXT_UCI_MSG_UWB_WLAN_COEX_NTF: {
        dmExtEvent = UWA_DM_PROP_WIFI_COEX_MAX_ACTIVE_GRANT_DUARTION_EXCEEDED_WAR_NTF_EVT;
        handle_WiFiCoEx_ActGrantDurationNtf(responsePayloadPtr);
    } break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    default:
        NXPLOG_UWBAPI_W("%s: unhandled oid 0x%x", __FUNCTION__, oid);
        skip_sem_post = TRUE;
        break;
    }

    return dmExtEvent;
}

/*******************************************************************************
 **
 ** Function         processVendorNtf
 **
 ** Description      Process UCI NTFs in the vendor group
 **
 ** Returns          associated event id
 **
 *******************************************************************************/
eResponse_Ext_Ntf_Event processVendorNtf(uint8_t oid, uint16_t responsePayloadLen, uint8_t *responsePayloadPtr)
{
    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;

    switch (oid) {
    case VENDOR_UCI_MSG_DO_VCO_PLL_CALIBRATION: {
        dmExtEvent = UWA_DM_PROP_DO_VCO_PLL_CALIBRATION_NTF_EVT;
        handle_do_vco_pll_calibration_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#if UWBFTR_UWBS_DEBUG_Dump
    case VENDOR_UCI_MSG_CIR_LOG_NTF: {
        dmExtEvent = UWA_DM_VENDOR_CIR_LOG_NTF_EVT;
        handle_debug_ntf(dmExtEvent, responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_PSDU_LOG_NTF: {
        dmExtEvent = UWA_DM_VENDOR_PSDU_LOG_NTF_EVT;
        handle_debug_ntf(dmExtEvent, responsePayloadPtr, responsePayloadLen);
    } break;
#endif //UWBFTR_UWBS_DEBUG_Dump
#if (UWBFTR_SE_SN110)
    case VENDOR_UCI_MSG_SE_DO_BIND: {
        dmExtEvent = UWA_DM_VENDOR_SE_DO_BIND_NTF_EVT;
        handle_se_binding_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_SE_DO_TEST_CONNECTIVITY: {
        dmExtEvent = UWA_DM_VENDOR_SE_DO_TEST_CONNECTIVITY_NTF_EVT;
        handle_se_binding_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_ESE_BINDING_CHECK_CMD: {
        dmExtEvent = UWA_DM_VENDOR_ESE_BINDING_CHECK_NTF_EVT;
        handle_se_binding_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_URSK_DELETION_REQ: {
        dmExtEvent = UWA_DM_VENDOR_URSK_DELETION_REQ_NTF_EVT;
        handle_se_binding_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#endif //(UWBFTR_SE_SN110)

    default:
        NXPLOG_UWBAPI_W("%s: unhandled oid 0x%x", __FUNCTION__, oid);
        skip_sem_post = TRUE;
        break;
    }

    return dmExtEvent;
}

/*******************************************************************************
 **
 ** Function         processProprietarySeNtf
 **
 ** Description      Process UCI NTFs in the proprietary SE group
 **
 ** Returns          associated event id
 **
 *******************************************************************************/
eResponse_Ext_Ntf_Event processProprietarySeNtf(uint8_t oid, uint16_t responsePayloadLen, uint8_t *responsePayloadPtr)
{
    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;

    switch (oid) {
    case EXT_UCI_MSG_WRITE_CALIB_DATA_CMD: {
        dmExtEvent         = UWA_DM_PROP_SE_WRITE_CALIB_DATA_NTF_EVT;
        uwbContext.wstatus = *responsePayloadPtr;
    } break;
    case EXT_UCI_MSG_READ_CALIB_DATA_CMD: {
        dmExtEvent = UWA_DM_PROP_SE_READ_CALIB_DATA_NTF_EVT;
        handle_read_calibration_data_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    default:
        NXPLOG_UWBAPI_W("%s: unhandled oid 0x%x", __FUNCTION__, oid);
        skip_sem_post = TRUE;
        break;
    }
    return dmExtEvent;
}

/*******************************************************************************
 **
 ** Function         processInternalNtf
 **
 ** Description      Process UCI NTFs in the Internal group
 **
 ** Returns          associated event id
 **
 *******************************************************************************/
eResponse_Ext_Ntf_Event processInternalNtf(uint8_t oid, uint16_t responsePayloadLen, uint8_t *responsePayloadPtr)
{
    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;

    switch (oid) {
#if UWBFTR_UWBS_DEBUG_Dump
    case VENDOR_UCI_MSG_RANGING_TIMESTAMP_NTF: {
        dmExtEvent = UWA_DM_INTERNAL_RANGING_TIMESTAMP_NTF_EVT;
        handle_debug_ntf(dmExtEvent, responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_DBG_RFRAME_LOG_NTF: {
        dmExtEvent = UWA_DM_INTERNAL_DBG_RFRAME_LOG_NTF_EVT;
        handle_rframe_log_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#endif //UWBFTR_UWBS_DEBUG_Dump
    default:
        NXPLOG_UWBAPI_W("%s: unhandled oid 0x%x", __FUNCTION__, oid);
        skip_sem_post = TRUE;
        break;
    }

    return dmExtEvent;
}

/*******************************************************************************
 **
 ** Function:        extDeviceManagementCallback
 **
 ** Description      Process UCI NTFs in the proprietary group
 **
 ** Returns          void
 **
 *******************************************************************************/
void extDeviceManagementCallback(uint8_t gid, uint8_t oid, uint16_t paramLength, uint8_t *pResponseBuffer)
{
    uint16_t responsePayloadLen = 0;
    uint8_t *responsePayloadPtr = NULL;
    skip_sem_post               = FALSE;

    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;

    if ((paramLength > UCI_RESPONSE_STATUS_OFFSET) && (pResponseBuffer != NULL)) {
        NXPLOG_UWBAPI_D(
            "extDeviceManagementCallback: Received length data = 0x%x "
            "status = 0x%x",
            paramLength,
            pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET]);

        responsePayloadLen = (uint16_t)(paramLength - UCI_RESPONSE_STATUS_OFFSET);
        responsePayloadPtr = &pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET];

        switch (gid) {
        case UCI_GID_PROPRIETARY:
            dmExtEvent = processProprietaryNtf(oid, responsePayloadLen, responsePayloadPtr);
            break;
        case UCI_GID_VENDOR:
            dmExtEvent = processVendorNtf(oid, responsePayloadLen, responsePayloadPtr);
            break;
        case UCI_GID_PROPRIETARY_SE:
            dmExtEvent = processProprietarySeNtf(oid, responsePayloadLen, responsePayloadPtr);
            break;
        case UCI_GID_INTERNAL_GROUP:
            dmExtEvent = processInternalNtf(oid, responsePayloadLen, responsePayloadPtr);
            break;
        default:
            NXPLOG_UWBAPI_W("%s: unhandled gid 0x%x", __FUNCTION__, gid);
            skip_sem_post = TRUE;
            break;
        }
        uwbContext.receivedEventId = dmExtEvent;
        if (uwbContext.currentEventId == dmExtEvent ||
            (isCmdRespPending() && (uwbContext.dev_state == UWBD_STATUS_ERROR))) {
            if (!skip_sem_post) {
                NXPLOG_UWBAPI_D("%s: posting devMgmtSem", __FUNCTION__);
                uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
                (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
            }
            else {
                skip_sem_post = FALSE;
            }
        }
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: pResponseBuffer is NULL or paramLength is less than "
            "UCI_RESPONSE_STATUS_OFFSET",
            __FUNCTION__);
    }
}

/*******************************************************************************
 **
 ** Function:        setDefaultCoreConfigs
 **
 ** Description:     Sets all core configs. Default values are picked from
 **                  config file(phNxpUciHal_RhodesConfig.h)
 **
 ** Returns:         None
 **
 *******************************************************************************/
tUWBAPI_STATUS setDefaultCoreConfigs(void)
{
    tUWBAPI_STATUS status  = UWBAPI_STATUS_FAILED;
    void *p_cmd            = NULL;
    long cmd_len           = 0;
    uint8_t config         = 0;
    uint16_t dpdTimeout    = 0;
    uint8_t extendedConfig = 0;
    uint8_t offset         = 1;

    NXPLOG_UWBAPI_D("%s: Enter ", __FUNCTION__);
    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_CORE_CONFIG_PARAM, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting UWB Core config failed", __FUNCTION__);
            return status;
        }
    }

    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_CORE_ANTENNAE_DEFINES, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Antennae Define failed", __FUNCTION__);
            return status;
        }
    }

    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_RX_ANTENNAE_DELAY_CALIB_CH5, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting Rx Antennae delay calib for channel 5 failed", __FUNCTION__);
            return status;
        }
    }

    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_RX_ANTENNAE_DELAY_CALIB_CH9, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting Rx Antennae delay calib for channel 9 failed", __FUNCTION__);
            return status;
        }
    }

    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_RX_ANTENNAE_DELAY_CALIB_CH6, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting Rx Antennae delay calib for channel 6 failed", __FUNCTION__);
            return status;
        }
    }

    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_RX_ANTENNAE_DELAY_CALIB_CH8, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting Rx Antennae delay calib for channel 8 failed", __FUNCTION__);
            return status;
        }
    }

#if UWBIOT_UWBD_SR2XXT
    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_RX_ANTENNAE_DELAY_CALIB_CH10, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting Rx Antennae delay calib for channel 10 failed", __FUNCTION__);
            return status;
        }
    }

    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_RX_ANTENNAE_DELAY_CALIB_CH12, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting Rx Antennae delay calib for channel 12 failed", __FUNCTION__);
            return status;
        }
    }
#endif

#if !(MCTT_PCTT_BIN_WITH_SSG_FW)
    status = setDefaultAoaCalibration();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("%s: Setting default AOA calibration values failed", __FUNCTION__);
        return status;
    }
#endif

    if (phNxpUciHal_GetNxpNumValue(UWB_LOW_POWER_MODE, &config, 0x01) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_LOW_POWER_MODE value %d ", __FUNCTION__, (uint8_t)config);

        offset = (uint8_t)getCoreDeviceConfigTLVBuffer(
            UCI_PARAM_ID_LOW_POWER_MODE, sizeof(config), (void *)&config, &uwbContext.snd_data[offset]);
        uwbContext.snd_data[0] = 1; // No of parameters
        sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
        status = sendUciCommandAndWait(UWA_DM_API_CORE_SET_CONFIG_EVT, (uint16_t)(offset + 1), uwbContext.snd_data);
        if (status == UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_D("%s: low power mode config is Sucess", __FUNCTION__);
        }
        else if (status == UWBAPI_STATUS_TIMEOUT) {
            return status;
        }
        else {
            NXPLOG_UWBAPI_E("%s: low power mode config is failed", __FUNCTION__);
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: low power mode config not found", __FUNCTION__);
    }
    if (phNxpUciHal_GetNxpNumValue(UWB_DPD_ENTRY_TIMEOUT, &dpdTimeout, 0x02) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_DPD_ENTRY_TIMEOUT value %d ", __FUNCTION__, dpdTimeout);

        offset = 1;
        if ((dpdTimeout >= UWBD_DPD_TIMEOUT_MIN) && (dpdTimeout <= UWBD_DPD_TIMEOUT_MAX)) {
            offset = (uint8_t)getExtDeviceConfigTLVBuffer(
                UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT, (void *)&dpdTimeout, &uwbContext.snd_data[offset]);
            uwbContext.snd_data[0] = 1; // No of parameters
            sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
            status = sendUciCommandAndWait(UWA_DM_API_CORE_SET_CONFIG_EVT, (uint16_t)(offset + 1), uwbContext.snd_data);
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: UWBD_DPD_TIMEOUT_MIN is Success", __FUNCTION__);
            }
            else if (status == UWBAPI_STATUS_TIMEOUT) {
                return status;
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWBD_DPD_TIMEOUT_MIN is failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid Range for DPD Entry Timeout in ConfigFile", __FUNCTION__);
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: low power mode config not found", __FUNCTION__);
    }

    /* During MCTT/PCTT execution, this setting needs to be disabled.
     * Disabling NXP Extended config to be taken care in the respective application.
     */
    if (phNxpUciHal_GetNxpNumValue(UWB_NXP_EXTENDED_NTF_CONFIG, &extendedConfig, 0x01) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_NXP_EXTENDED_NTF_CONFIG value %d ", __FUNCTION__, extendedConfig);
        offset = 1;
        if ((extendedConfig == DISABLED) || (extendedConfig == ENABLED)) {
            offset = (uint8_t)getExtDeviceConfigTLVBuffer(
                UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG, (void *)&extendedConfig, &uwbContext.snd_data[offset]);
            uwbContext.snd_data[0] = 1; // No of parameters
            sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
            status = sendUciCommandAndWait(UWA_DM_API_CORE_SET_CONFIG_EVT, (uint16_t)(offset + 1), uwbContext.snd_data);
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: UWB_NXP_EXTENDED_NTF_CONFIG is Success", __FUNCTION__);
            }
            else if (status == UWBAPI_STATUS_TIMEOUT) {
                return status;
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWB_NXP_EXTENDED_NTF_CONFIG is failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid Range for nxp extended ntf config in ConfigFile", __FUNCTION__);
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: nxp extended ntf config not found", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit ", __FUNCTION__);
    return status;
}
