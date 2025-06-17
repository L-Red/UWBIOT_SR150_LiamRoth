/*
*
* Copyright 2019-2020,2022-2023 NXP.
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

/******************************************************************************
 *
 *  This file contains the definition from UCI specification
 *
 ******************************************************************************/

#ifndef UWB_UCI_EXT_DEFS_H
#define UWB_UCI_EXT_DEFS_H

#include <stdint.h>

/**********************************************
 * UCI proprietary group(UCI_GID_PROPRIETARY)- 7: Opcodes and size of commands
 **********************************************/
#define EXT_UCI_MSG_CORE_DEVICE_INIT                                         0x00
#define EXT_UCI_MSG_DBG_DATA_LOGGER_NTF                                      0x01
#define EXT_UCI_MSG_DBG_GET_ERROR_LOG                                        0x02
#define EXT_UCI_MSG_SE_GET_BINDING_COUNT                                     0x03
#define EXT_UCI_MSG_SE_DO_TEST_LOOP                                          0x04
#define EXT_UCI_MSG_SE_COMM_ERROR_NTF                                        0x05
#define EXT_UCI_MSG_SE_GET_BINDING_STATUS                                    0x06
#define EXT_UCI_MSG_SCHEDULER_STATUS_NTF                                     0x07
#define EXT_UCI_MSG_UWB_SESSION_KDF_NTF                                      0x08
#define EXT_UCI_MSG_UWB_WIFI_COEX_IND_NTF                                    0x09
#define EXT_UCI_MSG_WLAN_UWB_IND_ERR_NTF                                     0x0A
#define EXT_UCI_MSG_QUERY_TEMPERATURE                                        0x0B
#define EXT_UCI_MSG_GENERATE_TAG                                             0x0E
#define EXT_UCI_MSG_VERIFY_CALIB_DATA                                        0x0F
#define EXT_UCI_MSG_CONFIGURE_AUTH_TAG_OPTIONS_CMD                           0x10
#define EXT_UCI_MSG_CONFIGURE_AUTH_TAG_VERSION_CMD                           0x11
#define EXT_UCI_MSG_CALIBRATION_INTEGRITY_PROTECTION                         0x12
#define EXT_UCI_MSG_UWB_WLAN_COEX_NTF                                        0x13

/**********************************************
 * UCI proprietary group(UCI_GID_PROPRIETARY_SE)- 0x0A: Opcodes and size of commands
 **********************************************/
#define EXT_UCI_MSG_WRITE_CALIB_DATA_CMD 0x00
#define EXT_UCI_MSG_READ_CALIB_DATA_CMD  0x01
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
#define EXT_UCI_MSG_GET_TRNG              0x02
#define EXT_UCI_MSG_WRITE_MODULE_MAKER_ID 0x03
#define EXT_UCI_MSG_READ_MODULE_MAKER_ID  0x04
#define EXT_UCI_MSG_SET_PROFILE           0x05
#define EXT_UCI_MSG_GET_HIF_I2C_CFG       0x10
#define EXT_UCI_MSG_SET_HIF_I2C_CFG       0x11
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
#if UWBIOT_UWBD_SR150
#define EXT_UCI_MSG_SESSION_SET_LOCALIZATION_ZONE_CMD 0x12
#endif // UWBIOT_UWBD_SR150

/**********************************************
 * UWB RADAR CONTROL Group Opcode-A(10d) Opcodes and size of command
 **********************************************/
#define EXT_UCI_MSG_RADAR_NTF 0x0A

/**********************************************************
 * UCI Extention Parameter IDs : Device Information
 *********************************************************/
#define UCI_EXT_PARAM_ID_DEVICE_NAME              0x00
#define UCI_EXT_PARAM_ID_FW_VERSION               0x01
#define UCI_EXT_PARAM_ID_VENDOR_UCI_VER           0x02
#define UCI_EXT_PARAM_ID_UWB_CHIP_ID              0x03
#define UCI_EXT_PARAM_ID_UWBS_MAX_PPM_VALUE       0x04
#define UCI_EXT_PARAM_ID_TX_POWER                 0x05
#define UCI_EXT_PARAM_ID_FIRA_EXT_UCI_GENERIC_VER 0x60
#define UCI_EXT_PARAM_ID_FIRA_EXT_TEST_VER        0x61
#define UCI_EXT_PARAM_ID_FW_BOOT_MODE             0x63

#define UCI_EXT_PARAM_ID_UCI_CCC_VERSION 0XA0
#define UCI_EXT_PARAM_ID_CCC_VERSION     0XA1

/**********************************************************
 * UCI Extension Parameter IDs : Capability Information
 *********************************************************/
#define EXTENDED_CAP_INFO_ID 0xE3

#define UCI_EXT_PARAM_ID_UWBS_MAX_UCI_PAYLOAD_LENGTH        0x00
#define UCI_EXT_PARAM_ID_UWBS_INBAND_DATA_BUFFER_BLOCK_SIZE 0x01
#define UCI_EXT_PARAM_ID_UWBS_INBAND_DATA_MAX_BLOCKS        0x02

#define UCI_EXT_PARAM_ID_INBAND_DATA_BUFFER_BLOCK_SIZE_LEN 0x01
#define UCI_EXT_PARAM_ID_INBAND_DATA_MAX_BLOCKS_LEN        0x01
#define UCI_EXT_PARAM_ID_UWBS_MAX_UCI_PAYLOAD_LENGTH_LEN   0x02

/**********************************************************
 * UCI Extension Parameter IDs : Device Configurations
 *********************************************************/
#define EXTENDED_DEVICE_CONFIG_ID 0xE4

#define UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC    0x02
#define UCI_EXT_PARAM_ID_WTX_COUNT         0x03
#define UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT 0x04
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#define UCI_EXT_PARAM_ID_WIFI_COEX_FEATURE 0x05
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#define UCI_EXT_PARAM_ID_RX_GPIO_ANTENNA_SELECTION   0x06
#define UCI_EXT_PARAM_ID_TX_BASE_BAND_CONFIG         0x26
#define UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG            0x27
#define UCI_EXT_PARAM_ID_TX_PULSE_SHAPE_CONFIG       0x28
#define UCI_EXT_PARAM_ID_CLK_CONFIG_CTRL             0x30
#define UCI_EXT_PARAM_ID_HOST_MAX_UCI_PAYLOAD_LENGTH 0x31
#define UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG     0x33
#define UCI_EXT_PARAM_ID_CLOCK_PRESENT_WAITING_TIME  0x34
#define UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_ABS    0x35
#define UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_REL    0x36
#define UCI_EXT_PARAM_ID_WIFI_COEX_UART_USER_CFG     0x37
#define UCI_EXT_PARAM_ID_PDOA_CALIB_TABLE_DEFINE     0x46
#define UCI_EXT_PARAM_ID_ANTENNA_RX_IDX_DEFINE       0x60
#define UCI_EXT_PARAM_ID_ANTENNA_TX_IDX_DEFINE       0x61
#define UCI_EXT_PARAM_ID_ANTENNAE_RX_PAIR_DEFINE     0x62
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#define UCI_EXT_PARAM_ID_WIFI_CO_EX_CH_CFG 0x64
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

#define UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC_LEN 0x01
#define UCI_EXT_PARAM_ID_WTX_COUNT_LEN      0x01
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#define UCI_EXT_PARAM_ID_WIFI_COEX_FEATURE_LEN 0x04
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#define UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT_LEN           0x02
#define UCI_EXT_PARAM_ID_TX_BASE_BAND_CONFIG_LEN         0x01
#define UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG_LEN            0x48
#define UCI_EXT_PARAM_ID_TX_PULSE_SHAPE_CONFIG_LEN       0x04
#define UCI_EXT_PARAM_ID_HOST_MAX_UCI_PAYLOAD_LENGTH_LEN 0x02
#define UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG_LEN     0x01
#define UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_ABS_LEN    0x02
#define UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_REL_LEN    0x02
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#define UCI_EXT_PARAM_ID_WIFI_CO_EX_CH_CFG_LEN 0x01
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

/***********************************************************
 * UCI Vendor Config Parameter IDs : Application Configurations
 **********************************************************/
#define UCI_VENDOR_PARAM_ID_MAC_PAYLOAD_ENCRYPTION         0x00
#define UCI_VENDOR_PARAM_ID_ANTENNAE_CONFIGURATION_TX      0x02
#define UCI_VENDOR_PARAM_ID_ANTENNAE_CONFIGURATION_RX      0x03
#define UCI_VENDOR_PARAM_ID_RAN_MULTIPLIER                 0x20
#define UCI_VENDOR_PARAM_ID_STS_LAST_INDEX_USED            0x21
#define UCI_VENDOR_PARAM_ID_CIR_LOG_NTF                    0x30
#define UCI_VENDOR_PARAM_ID_PSDU_LOG_NTF                   0x31
#define UCI_VENDOR_PARAM_ID_RSSI_AVG_FILT_CNT              0x40
#define UCI_VENDOR_PARAM_ID_CIR_CAPTURE_MODE               0x60
#define UCI_VENDOR_PARAM_ID_RX_ANTENNA_POLARIZATION_OPTION 0x61
#define UCI_VENDOR_PARAM_ID_SESSION_SYNC_ATTEMPTS          0x62
#define UCI_VENDOR_PARAM_ID_SESSION_SCHED_ATTEMPTS         0x63
#define UCI_VENDOR_PARAM_ID_SCHED_STATUS_NTF               0x64
#define UCI_VENDOR_PARAM_ID_TX_POWER_DELTA_FCC             0x65
#define UCI_VENDOR_PARAM_ID_TEST_KDF_FEATURE               0x66
#define UCI_VENDOR_PARAM_ID_TX_POWER_TEMP_COMPENSATION     0x67
#define UCI_VENDOR_PARAM_ID_WIFI_COEX_MAX_TOLERANCE_COUNT  0x68
#define UCI_VENDOR_PARAM_ID_ADAPTIVE_HOPPING_THRESHOLD     0x69
#define UCI_VENDOR_PARAM_ID_AUTHENTICITY_TAG               0x6E
#define UCI_VENDOR_PARAM_ID_RX_NBIC_CONFIG                 0x6F
#define UCI_VENDOR_PARAM_ID_MAC_CFG                        0x70
#define UCI_VENDOR_PARAM_ID_SESSION_INBAND_DATA_TX_BLOCKS  0x71
#define UCI_VENDOR_PARAM_ID_SESSION_INBAND_DATA_RX_BLOCKS  0x72
#define UCI_VENDOR_PARAM_ID_ANTENNAE_SCAN_CONFIGURATION    0x74
#define UCI_VENDOR_PARAM_ID_DATA_TRANSFER_TX_STATUS_CONFIG 0x75
#if (UWBFTR_UL_TDoA_Tag)
#define UCI_VENDOR_PARAM_ID_ULTDOA_MAC_FRAME_FORMAT        0x76
#endif // #if (UWBFTR_UL_TDoA_Tag)
#define UCI_VENDOR_PARAM_ID_WRAPPED_RDS                    0x79
#define UCI_VENDOR_PARAM_ID_RFRAME_LOG_NTF                 0x7B
#define UCI_VENDOR_PARAM_ID_TX_ADAPTIVE_PAYLOAD_POWER      0x7F
#define UCI_VENDOR_PARAM_ID_SWAP_ANTENNA_PAIR_3D_AOA       0x80
#define UCI_VENDOR_PARAM_ID_RML_PROXIMITY_CONFIG           0x81
#define UCI_VENDOR_PARAM_ID_CSA_MAC_MODE                   0x82
#if (UWBIOT_UWBD_SR150)
#define UCI_VENDOR_PARAM_ID_FOV_ENABLE            0x84
#define UCI_VENDOR_PARAM_ID_AZIMUTH_FIELD_OF_VIEW 0x85
#define UCI_VENDOR_PARAM_ID_CSA_FINAL_DATA2_CONFIG 0x90
#endif // (UWBIOT_UWBD_SR150)

/**********************************************
 * UCI Vendor group(UCI_GID_VENDOR)- 0x0F: Opcodes and size of commands
 **********************************************/
#define VENDOR_UCI_MSG_URSK_DELETION_REQ       0x01
#define VENDOR_UCI_MSG_GET_ALL_UWB_SESSIONS    0x02
#define VENDOR_UCI_MSG_DO_VCO_PLL_CALIBRATION  0x20
#define VENDOR_UCI_MSG_SET_DEVICE_CALIBRATION  0x21
#define VENDOR_UCI_MSG_GET_DEVICE_CALIBRATION  0x22
#define VENDOR_UCI_MSG_SE_DO_TEST_CONNECTIVITY 0x30
#define VENDOR_UCI_MSG_SE_DO_BIND              0x31
#define VENDOR_UCI_MSG_ESE_BINDING_CHECK_CMD   0x32
#define VENDOR_UCI_MSG_PSDU_LOG_NTF            0x33
#define VENDOR_UCI_MSG_CIR_LOG_NTF             0x34

/**********************************************
 * UCI Internal group (UCI_GID_INTERNAL_GROUP)- 0x0B: Opcodes and size of commands
 **********************************************/
#define VENDOR_UCI_MSG_RANGING_TIMESTAMP_NTF 0x03
#define VENDOR_UCI_MSG_DBG_RFRAME_LOG_NTF    0x22

/***********************************************************
 * UCI Extention Parameter IDs : Debug Configurations
 **********************************************************/
#define UCI_EXT_PARAM_ID_DATA_LOGGER_NTF                 0x7A
#define UCI_EXT_PARAM_ID_TEST_CONTENTION_RANGING_FEATURE 0x7C
#define UCI_EXT_PARAM_ID_CIR_WINDOW                      0x7D
#define UCI_EXT_PARAM_ID_RANGING_TIMESTAMP_NTF           0x7E
#define UCI_EXT_PARAM_ID_THREAD_SECURE                   0xD0
#define UCI_EXT_PARAM_ID_THREAD_SECURE_ISR               0xD1
#define UCI_EXT_PARAM_ID_THREAD_NON_SECURE_ISR           0xD2
#define UCI_EXT_PARAM_ID_THREAD_SHELL                    0xD3
#define UCI_EXT_PARAM_ID_THREAD_PHY                      0xD4
#define UCI_EXT_PARAM_ID_THREAD_RANGING                  0xD5
#define UCI_EXT_PARAM_ID_THREAD_SECURE_ELEMENT           0xD6
#define UCI_EXT_PARAM_ID_THREAD_UWB_WLAN_COEX            0xD7

/***********************************************************
 * UCI Extention Test Parameter IDs : Test Configurations
 **********************************************************/
#define EXTENDED_TEST_CONFIG_ID 0xE5

#define UCI_EXT_TEST_PARAM_ID_RSSI_CALIBRATION_OPTION 0x01
#define UCI_EXT_TEST_PARAM_ID_AGC_GAIN_VAL_RX         0x02
#define UCI_EXT_TEST_SESSION_STS_KEY_OPTION           0x03

/**********************************************************
 * UCI Extension Parameter IDs : Radar Configurations
 **********************************************************/
#define EXTENDED_RADAR_CONFIG_ID      0xE5
#define EXTENDED_RADAR_TEST_CONFIG_ID 0xE4

#define UCI_EXT_RADAR_PARAM_ID_RADAR_MODE                       0x00
#define UCI_EXT_RADAR_PARAM_ID_RADAR_PRF_CFG                    0x01
#define UCI_EXT_RADAR_PARAM_ID_RADAR_RX_DELAY                   0x02
#define UCI_EXT_RADAR_PARAM_ID_RADAR_RX_GAIN                    0x03
#define UCI_EXT_RADAR_PARAM_ID_RADAR_SINGLE_FRAME_NTF           0x05
#define UCI_EXT_RADAR_PARAM_ID_RADAR_CIR_START_INDEX            0x06
#define UCI_EXT_RADAR_PARAM_ID_RADAR_CIR_NUM_SAMPLES            0x07
#define UCI_EXT_RADAR_PARAM_ID_RADAR_CIR_START_OFFSET           0x08
#define UCI_EXT_RADAR_PARAM_ID_RADAR_CALIBRATION_SUPPORT        0x09
#define UCI_EXT_RADAR_PARAM_ID_RADAR_CALIBRATION_NUM_SAMPLES    0x0A
#define UCI_EXT_RADAR_PARAM_ID_RADAR_RADAR_CALIBRATION_INTERVAL 0x0B
#define UCI_EXT_RADAR_PARAM_ID_RADAR_TEST_FCC_TESTMODE          0x80

/***********************************************************
 * UCI Parameter IDs : SR150 specific Configurations
 **********************************************************/
#define UCI_EXT_MSG_SE_BIND_GET_INIT_DATA      0x08
#define UCI_EXT_MSG_SE_BIND_SET_SE_DATA        0x09
#define UCI_EXT_MSG_SE_BIND_COMMIT_BDI         0x0A
#define UCI_EXT_MSG_SE_GET_HOST_CHALLENGE_APDU 0x0B
#define UCI_EXT_MSG_SE_GET_EXTERNAL_AUTH_APDU  0x0C
#define UCI_EXT_MSG_SE_DO_ENC_APDU             0x0D
#define UCI_EXT_MSG_SE_RESP_APDU_VALIDATE_REQ  0x0E

/** UCI Extention calibration Param */

#define UCI_EXT_PARAM_ID_AOA_ANTENNAS_PDOA_CALIB     0x00

/* Binary log levels*/
#define DBG_LOG_LEVEL_DISABLE 0x0000
#define DBG_LOG_LEVEL_ERROR   0x0001
#define DBG_LOG_LEVEL_WARNG   0x0002
#define DBG_LOG_LEVEL_TMSTAMP 0x0004
#define DBG_LOG_LEVEL_SEQNUM  0x0008
#define DBG_LOG_LEVEL_INFO1   0x0010
#define DBG_LOG_LEVEL_INFO2   0x0020
#define DBG_LOG_LEVEL_INFO3   0x0040

/* UWB proprietary status codes */
#define UCI_STATUS_BINDING_SUCCESS                     0x50
#define UCI_STATUS_BINDING_FAILURE                     0x51
#define UCI_STATUS_BINDING_LIMIT_REACHED               0x52
#define UCI_STATUS_CALIBRATION_IN_PROGRESS             0x53
#define UCI_STATUS_DEVICE_TEMP_REACHED_THERMAL_RUNAWAY 0x54
#define UCI_STATUS_NO_ESE                              0x70
#define UCI_STATUS_ESE_RSP_TIMEOUT                     0x71
#define UCI_STATUS_ESE_RECOVERY_FAILURE                0x72
#define UCI_STATUS_ESE_RECOVERY_SUCCESS                0x73
#define UCI_STATUS_SE_APDU_CMD_FAIL                    0x74
#define UCI_STATUS_SE_AUTH_FAIL                        0x75

/*Binding status notification on boot | (0x04-0xFF : RFU)*/
#define UCI_BINDING_STATUS_NOT_BOUND      0x00
#define UCI_BINDING_STATUS_BOUND_UNLOCKED 0x01
#define UCI_BINDING_STATUS_BOUND_LOCKED   0x02
#define UCI_BINDING_STATUS_UNKNOWN        0x03

#define UCI_BINDING_STATUS_LEN 0x01

/* UWB Generate Tag command related length fields */
#define UCI_TAG_CMAC_LENGTH 0x10U

/* UWB Query Timestamp command related length fields */
#define UCI_QUERY_TIMESTAMP_LENGTH 0x08U

/* Session State codes, as per UCI*/
/**  Session State - Session status notification with no rng data error */
#define UCI_SESSION_FAILED_WITH_NO_RNGDATA_IN_SE 0x80
/**  Session State - Session status notification with key fetch error */
#define UCI_SESSION_FAILED_WITH_KEY_FETCH_ERROR 0x81
/**  Session State - Session status notification with dynamic sts not supported error */
#define UCI_SESSION_FAILIED_DYNAMIC_STS_NOT_SUPPORTED 0x82
/**  Session State - Session status notification with in band stop received error */
#define UCI_SESSION_IN_BAND_STOP_RECEIVED 0x83

/* Session reason codes, as per UCI*/
/**  Session State - Session status notification with slot length not supported */
#define UCI_SESSION_FAILED_WITH_SLOT_LEN_NOT_SUPPORTED 0x20
/**  Session State - Session status notification with slot per ranging round insufficient */
#define UCI_SESSION_FAILED_WITH_SLOT_RR_INSUFFICIENT 0x21
/** Session State - Session state notification with key rotation enabled during Dynamic STS ranging */
#define UCI_SESSION_FAILED_WITH_KEY_ROTATION_INVALID_STS_CONFIG 0x4B

#endif /* UWB_UCI_EXT_DEFS_H */
