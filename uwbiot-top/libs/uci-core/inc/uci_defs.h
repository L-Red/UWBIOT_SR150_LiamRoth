/*
*
* Copyright 2019-2020,2022-2024 NXP.
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

#ifndef UWB_UCI_DEFS_H
#define UWB_UCI_DEFS_H

#include <stdint.h>

/* Define the message header size for all UCI Commands and Notifications.
*/
#define UCI_MSG_HDR_SIZE                      0x04 /* per UCI spec */
#define UCI_MAX_PAYLOAD_SIZE                  0xFF /* max control message size */
#define APP_DATA_MAX_SIZE                     0x74 /* Max Applicaiton data trasnfer size 116 bytes as per UCI Spec*/
#define UCI_DEVICE_INFO_MAX_SIZE              UCI_MAX_PAYLOAD_SIZE
#define UCI_RESPONSE_LEN_OFFSET               0x03
#define UCI_RESPONSE_STATUS_OFFSET            0x04
#define UCI_RESPONSE_PAYLOAD_OFFSET           0x05
#define UCI_CMD_PAYLOAD_OFFSET                0x04
#define UCI_CMD_SESSION_HANDLE_OFFSET         0x04
#define UCI_GET_APP_CONFIG_NO_OF_PARAM_OFFSET 0x05
#define UCI_GET_APP_CONFIG_PARAM_OFFSET       0x06
#define UCI_MAX_BPRF_PAYLOAD_SIZE             0x7F
#define UCI_MAX_DATA_PACKET_SIZE \
    4200 /* Max Radar Notificaiton Receive from the UWBS is 4106 Payload and 14 Bytes header for Bulk Transfer */
#define MAC_EXT_ADD_LEN                      8
#define MAX_NO_OF_ACTIVE_RANGING_ROUND       0xFF
#define SR040_MAX_MULTICAST_NTF_PAYLOAD_SIZE (64)

/*
 * This buffer is used to store the  EXT_PSDU_LOG_NTF.
 * Following format is used to store the data in sequence.
 * UWB_HDR    [of size `Session ID`] = 4 Octest
 * UCI Header [of size `PSDU size`] = 2 Octest
 * UCI Payload [Maximum of `PSDU_DATA` bytes] = 1023 Octest Max
 */
#define UCI_MAX_HPRF_PAYLOAD_SIZE_SR040 1029

/* UCI Command and Notification Format:
 * 4 byte message header:
 * byte 0: MT PBF GID
 * byte 1: OID
 * byte 2: RFU - To be used for extended playload length
 * byte 3: Message Length */

/* MT: Message Type (byte 0) */
#define UCI_MT_MASK  0xE0
#define UCI_MT_SHIFT 0x05
#define UCI_MT_DATA  0x00 /* (UCI_MT_DATA << UCI_MT_SHIFT) = 0x00 */
#define UCI_MT_CMD   0x01 /* (UCI_MT_CMD << UCI_MT_SHIFT) = 0x20 */
#define UCI_MT_RSP   0x02 /* (UCI_MT_RSP << UCI_MT_SHIFT) = 0x40 */
#define UCI_MT_NTF   0x03 /* (UCI_MT_NTF << UCI_MT_SHIFT) = 0x60 */
#define UCI_EXT_MASK 0x80
/* Data Packet Format for send/receive data with message type 0 */
#define UCI_DPF_SND 0x01
#define UCI_DPF_RCV 0x02

#define UCI_MTS_CMD 0x20
#define UCI_MTS_RSP 0x40
#define UCI_MTS_NTF 0x60

#define UCI_NTF_BIT 0x80 /* the tUWB_VS_EVT is a notification */
#define UCI_RSP_BIT 0x40 /* the tUWB_VS_EVT is a response     */

/* PBF: Packet Boundary Flag (byte 0) */
#define UCI_PBF_MASK       0x10
#define UCI_PBF_SHIFT      0x04
#define UCI_PBF_NO_OR_LAST 0x00 /* not fragmented or last fragment */
#define UCI_PBF_ST_CONT    0x10 /* start or continuing fragment */

/* GID: Group Identifier (byte 0) */
#define UCI_GID_MASK                    0x0F
#define UCI_GID_SHIFT                   0x00
#define UCI_GID_CORE                    0x00 /* 0000b UCI Core group */
#define UCI_GID_SESSION_MANAGE          0x01 /* 0001b Session Config commands */
#define UCI_GID_RANGE_MANAGE            0x02 /* 0010b Range Management group */
#define UCI_GID_PROP_RADAR_CONTROL      0x09 /* 1001b Data Control */
#define UCI_GID_PROPRIETARY_SE          0x0A /* 1010b Proprietary SE Group */
#define UCI_GID_INTERNAL_GROUP          0x0B /* 1011b NXP Internal Group */
#define UCI_GID_TEST                    0x0D /* 1101b RF Test Gropup */
#define UCI_GID_PROPRIETARY             0x0E /* 1110b Proprietary Group */
#define UCI_GID_VENDOR                  0x0F /* 1111b Vendor Group */
#define UCI_GID_INTERNAL                0x1F /* 11111b MW Internal DM group */

/* 0100b - 1100b RFU */

/* OID: Opcode Identifier (byte 1) */
#define UCI_OID_MASK  0x3F
#define UCI_OID_SHIFT 0x00

/* builds byte0 of UCI Command and Notification packet */
#define UCI_MSG_BLD_HDR0(p, mt, gid) *(p)++ = (uint8_t)(((mt) << UCI_MT_SHIFT) | (gid));

#define UCI_MSG_PBLD_HDR0(p, mt, pbf, gid) \
    *(p)++ = (uint8_t)(((mt) << UCI_MT_SHIFT) | ((pbf) << UCI_PBF_SHIFT) | (gid));

/* builds byte1 of UCI Command and Notification packet */
#define UCI_MSG_BLD_HDR1(p, oid) *(p)++ = (uint8_t)(((oid) << UCI_OID_SHIFT));

/* builds byte1 of UCI Command and Ext bit Packet  */
#define UCI_MSG_BLD_HDR1_EXT(p, oid) *(p)++ = (uint8_t)(((oid) << UCI_OID_SHIFT) | (UCI_EXT_MASK));

/* parse byte0 of UCI packet */
#define UCI_MSG_PRS_HDR0(p, mt, pbf, gid)       \
    mt  = (*(p)&UCI_MT_MASK) >> UCI_MT_SHIFT;   \
    pbf = (*(p)&UCI_PBF_MASK) >> UCI_PBF_SHIFT; \
    gid = *(p)++ & UCI_GID_MASK;

/* parse PBF and GID bits in byte0 of UCI packet */
#define UCI_MSG_PRS_PBF_GID(p, pbf, gid)        \
    pbf = (*(p)&UCI_PBF_MASK) >> UCI_PBF_SHIFT; \
    gid = *(p)++ & UCI_GID_MASK;

/* parse MT and PBF bits of UCI packet */
#define UCI_MSG_PRS_MT_PBF(p, mt, pbf)        \
    mt  = (*(p)&UCI_MT_MASK) >> UCI_MT_SHIFT; \
    pbf = (*(p)&UCI_PBF_MASK) >> UCI_PBF_SHIFT;

/* parse byte1 of UCI Cmd/Ntf */
#define UCI_MSG_PRS_HDR1(p, oid) \
    oid = (*(p)&UCI_OID_MASK);   \
    (p)++;

/* parse byte1 of HDR1 and get pbf field*/
#define UCI_MSG_PRS_PBF(p, pbf) pbf = (*(p)&UCI_PBF_MASK) >> UCI_PBF_SHIFT;

/* Allocate smallest possible buffer (for platforms with limited RAM) */
#define UCI_GET_CMD_BUF(paramlen) \
    ((UWB_HDR *)phOsalUwb_GetMemory((uint16_t)(UWB_HDR_SIZE + UCI_MSG_HDR_SIZE + UCI_MSG_OFFSET_SIZE + (paramlen))))

/**********************************************
 * UCI Core Group-0: Opcodes and size of commands
 **********************************************/
#define UCI_MSG_CORE_DEVICE_RESET           0x00
#define UCI_MSG_CORE_DEVICE_STATUS_NTF      0x01
#define UCI_MSG_CORE_DEVICE_INFO            0x02
#define UCI_MSG_CORE_GET_CAPS_INFO          0x03
#define UCI_MSG_CORE_SET_CONFIG             0x04
#define UCI_MSG_CORE_GET_CONFIG             0x05
#define UCI_MSG_CORE_DEVICE_SUSPEND         0x06
#define UCI_MSG_CORE_GENERIC_ERROR_NTF      0x07
#define UCI_MSG_CORE_QUERY_UWBS_TIMESTAMP   0x08
#define UCI_MSG_CORE_DEVICE_RESET_CMD_SIZE  0x01
#define UCI_MSG_CORE_DEVICE_INFO_CMD_SIZE   0x00
#define UCI_MSG_CORE_GET_CAPS_INFO_CMD_SIZE 0x00
#define UCI_MSG_CORE_UWBS_TIMESTAMP_LEN     0x08

/*********************************************************
 * UCI session config Group-2: Opcodes and size of command
 ********************************************************/
#define UCI_MSG_SESSION_INIT                             0x00
#define UCI_MSG_SESSION_DEINIT                           0x01
#define UCI_MSG_SESSION_STATUS_NTF                       0x02
#define UCI_MSG_SESSION_SET_APP_CONFIG                   0x03
#define UCI_MSG_SESSION_GET_APP_CONFIG                   0x04
#define UCI_MSG_SESSION_GET_COUNT                        0x05
#define UCI_MSG_SESSION_GET_STATE                        0x06
#define UCI_MSG_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST 0x07
#define UCI_MSG_UPDATE_ACTIVE_ROUNDS_OF_ANCHOR_DEVICE    0x08
#define UCI_MSG_UPDATE_ACTIVE_ROUNDS_OF_RECEIVER_DEVICE  0x09
#define UCI_MSG_SESSION_QUERY_DATA_SIZE_IN_RANGING       0x0B
#define UCI_MSG_SESSION_SET_HUS_CONTROLLER_CONFIG_CMD    0x0C
#define UCI_MSG_SESSION_SET_HUS_CONTROLEE_CONFIG_CMD     0x0D

/*********************************************************
 * UCI Vendor app config
 ********************************************************/
#define UCI_MSG_SESSION_VENDOR_SET_APP_CONFIG 0x00
#define UCI_MSG_SESSION_VENDOR_GET_APP_CONFIG 0x03

/* Pay load size for each command*/
#define UCI_MSG_SESSION_INIT_CMD_SIZE      0x05
#define UCI_MSG_SESSION_DEINIT_CMD_SIZE    0x04
#define UCI_MSG_SESSION_STATUS_NTF_LEN     0x06
#define UCI_MSG_SESSION_GET_COUNT_CMD_SIZE 0x00
#define UCI_MSG_SESSION_GET_STATE_SIZE     0x04

/*********************************************************
 * UWB Ranging Control Group-3: Opcodes and size of command
 *********************************************************/
#define UCI_MSG_RANGE_START             0x00
#define UCI_MSG_RANGE_STOP              0x01
#define UCI_MSG_RANGE_CTRL_REQ          0x02
#define UCI_MSG_RANGE_GET_RANGING_COUNT 0x03
#define UCI_MSG_RANGE_BLINK_DATA_TX     0x04

#define UCI_MSG_SESSION_INFO_NTF                   0x00
#define UCI_MSG_RANGE_CCC_DATA_NTF                 0x20
#define UCI_MSG_SESSION_DATA_TRANSFER_PHASE_CONFIG 0x0E

#define UCI_MSG_RANGE_START_CMD_SIZE               0x04
#define UCI_MSG_RANGE_STOP_CMD_SIZE                0x04
#define UCI_MSG_RANGE_INTERVAL_UPDATE_REQ_CMD_SIZE 0x06
#define UCI_MSG_RANGE_GET_COUNT_CMD_SIZE           0x04

/**********************************************
 * UWB DATA CONTROL Group Opcode-9 Opcodes and size of command
 **********************************************/
#define UCI_MSG_DATA_CREDIT_NTF          0x04
#define UCI_MSG_DATA_TRANSMIT_STATUS_NTF 0x05

/**********************************************
 * UCI Parameter IDs : Device Configurations
 **********************************************/
#define UCI_PARAM_ID_DEVICE_STATE   0x00
#define UCI_PARAM_ID_LOW_POWER_MODE 0x01
/*
Reserved for Extention of IDs: 0xE0-0xE2
Reserved for Proprietary use: 0xE3-0xFF
*/
/* UCI Parameter ID Length */
#define UCI_PARAM_LEN_DEVICE_STATE   0x01
#define UCI_PARAM_LEN_LOW_POWER_MODE 0x01

/*************************************************
 * UCI Parameter IDs : Application Configurations
 ************************************************/
#define UCI_PARAM_ID_DEVICE_TYPE                       0x00
#define UCI_PARAM_ID_RANGING_ROUND_USAGE               0x01
#define UCI_PARAM_ID_STS_CONFIG                        0x02
#define UCI_PARAM_ID_MULTI_NODE_MODE                   0x03
#define UCI_PARAM_ID_CHANNEL_NUMBER                    0x04
#define UCI_PARAM_ID_NO_OF_CONTROLEES                  0x05
#define UCI_PARAM_ID_DEVICE_MAC_ADDRESS                0x06
#define UCI_PARAM_ID_DST_MAC_ADDRESS                   0x07
#define UCI_PARAM_ID_SLOT_DURATION                     0x08
#define UCI_PARAM_ID_RANGING_DURATION                  0x09
#define UCI_PARAM_ID_STS_INDEX                         0x0A
#define UCI_PARAM_ID_MAC_FCS_TYPE                      0x0B
#define UCI_PARAM_ID_RANGING_ROUND_CONTROL             0x0C
#define UCI_PARAM_ID_AOA_RESULT_REQ                    0x0D
#define UCI_PARAM_ID_SESSION_INFO_NTF                  0x0E
#define UCI_PARAM_ID_NEAR_PROXIMITY_CONFIG             0x0F
#define UCI_PARAM_ID_FAR_PROXIMITY_CONFIG              0x10
#define UCI_PARAM_ID_DEVICE_ROLE                       0x11
#define UCI_PARAM_ID_RFRAME_CONFIG                     0x12
#define UCI_PARAM_ID_RSSI_REPORTING                    0x13
#define UCI_PARAM_ID_PREAMBLE_CODE_INDEX               0x14
#define UCI_PARAM_ID_SFD_ID                            0x15
#define UCI_PARAM_ID_PSDU_DATA_RATE                    0x16
#define UCI_PARAM_ID_PREAMBLE_DURATION                 0x17
#define UCI_PARAM_ID_LINK_LAYER_MODE                   0x18
#define UCI_PARAM_ID_DATA_REPETITION_COUNT             0x19
#define UCI_PARAM_ID_RANGING_TIME_STRUCT               0x1A
#define UCI_PARAM_ID_SLOTS_PER_RR                      0x1B
#define UCI_PARAM_ID_TX_ADAPTIVE_PAYLOAD_POWER         0x1C
#define UCI_PARAM_ID_AOA_BOUND_CONFIG                  0x1D
#define UCI_PARAM_ID_PRF_MODE                          0x1F
#define UCI_PARAM_ID_CAP_SIZE_RANGE                    0x20
#define UCI_PARAM_ID_SCHEDULED_MODE                    0x22
#define UCI_PARAM_ID_KEY_ROTATION                      0x23
#define UCI_PARAM_ID_KEY_ROTATION_RATE                 0x24
#define UCI_PARAM_ID_SESSION_PRIORITY                  0x25
#define UCI_PARAM_ID_MAC_ADDRESS_MODE                  0x26
#define UCI_PARAM_ID_VENDOR_ID                         0x27
#define UCI_PARAM_ID_STATIC_STS_IV                     0x28
#define UCI_PARAM_ID_NUMBER_OF_STS_SEGMENTS            0x29
#define UCI_PARAM_ID_MAX_RR_RETRY                      0x2A
#define UCI_PARAM_ID_UWB_INITIATION_TIME               0x2B
#define UCI_PARAM_ID_HOPPING_MODE                      0x2C
#define UCI_PARAM_ID_BLOCK_STRIDING                    0x2D
#define UCI_PARAM_ID_RESULT_REPORT_CONFIG              0x2E
#define UCI_PARAM_ID_IN_BAND_TERMINATION_ATTEMPT_COUNT 0x2F
#define UCI_PARAM_ID_SUB_SESSION_ID                    0x30
#define UCI_PARAM_ID_BPRF_PHR_DATA_RATE                0X31
#define UCI_PARAM_ID_MAX_NUMBER_OF_MEASUREMENTS        0x32
#define UCI_PARAM_ID_UL_TDOA_TX_INTERVAL               0X33
#define UCI_PARAM_ID_UL_TDOA_RANDOM_WINDOW             0x34
#define UCI_PARAM_ID_STS_LENGTH                        0x35
#define UCI_PARAM_ID_SUSPEND_RANGING_ROUNDS            0x36
#define UCI_PARAM_ID_UL_TDOA_NTF_REPORT_CONFIG         0x37
#define UCI_PARAM_ID_UL_TDOA_DEVICE_ID                 0x38
#define UCI_PARAM_ID_UL_TDOA_TX_TIMESTAMP              0x39
#define UCI_PARAM_ID_MIN_FRAMES_PER_RR                 0x3A
#define UCI_PARAM_ID_MTU_SIZE                          0x3B
#define UCI_PARAM_ID_INTER_FRAME_INTERVAL              0x3C
#define UCI_PARAM_ID_DLTDOA_RANGING_METHOD             0x3D
#define UCI_PARAM_ID_DLTDOA_TX_TIMESTAMP_CONF          0x3E
#define UCI_PARAM_ID_DLTDOA_INTER_CLUSTER_SYNC_PERIOD  0x3F
#define UCI_PARAM_ID_DLTDOA_ANCHOR_CFO                 0x40
#define UCI_PARAM_ID_DLTDOA_ANCHOR_LOCATION            0x41
#define UCI_PARAM_ID_DLTDOA_TX_ACTIVE_RANGING_ROUNDS   0x42
#define UCI_PARAM_ID_DL_TDOA_BLOCK_STRIDING            0x43
#define UCI_PARAM_ID_DLTDOA_TIME_REF_ANCHOR            0x44
#define UCI_PARAM_ID_SESSION_KEY                       0x45
#define UCI_PARAM_ID_SUB_SESSION_KEY                   0x46
#define UCI_PARAM_ID_DATA_TRANSFER_STATUS_NTF_CONFIG   0x47
#define UCI_PARAM_ID_SESSION_TIME_BASE                 0x48
#define UCI_PARAM_ID_DL_TDOA_RESPONDER_TOF             0x49
#define UCI_PARAM_ID_APPLICATION_DATA_ENDPOINT         0x4C
#define UCI_PARAM_ID_HOP_MODE_KEY                      0xA0
#define UCI_PARAM_ID_CCC_CONFIG_QUIRKS                 0xA1
#define UCI_PARAM_ID_RESPONDER_SLOT_INDEX              0xA2
#define UCI_PARAM_ID_RANGING_PROTOCOL_VER              0xA3
#define UCI_PARAM_ID_UWB_CONFIG_ID                     0xA4
#define UCI_PARAM_ID_PULSESHAPE_COMBO                  0xA5
#define UCI_PARAM_ID_URSK_TTL                          0xA6
#define UCI_PARAM_ID_RESPONDER_LISTEN_ONLY             0xA7
#define UCI_PARAM_ID_LAST_STS_INDEX_USED               0xA8

/* UCI Parameter ID Length */
#define UCI_PARAM_LEN_DEVICE_ROLE                0x01
#define UCI_PARAM_LEN_RANGING_METHOD             0x01
#define UCI_PARAM_LEN_STS_CONFIG                 0x01
#define UCI_PARAM_LEN_MULTI_NODE_MODE            0x01
#define UCI_PARAM_LEN_CHANNEL_NUMBER             0x01
#define UCI_PARAM_LEN_NO_OF_CONTROLEES           0x01
#define UCI_PARAM_LEN_DEVICE_MAC_ADDRESS         0x02
#define UCI_PARAM_LEN_DEST_MAC_ADDRESS           0x02
#define UCI_PARAM_LEN_SLOT_DURATION              0x02
#define UCI_PARAM_LEN_RANGING_DURATION           0x04
#define UCI_PARAM_LEN_STS_INDEX                  0x04
#define UCI_PARAM_LEN_MAC_FCS_TYPE               0x01
#define UCI_PARAM_LEN_MEASUREMENT_REPORT_REQ     0x01
#define UCI_PARAM_LEN_AOA_RESULT_REQ             0x01
#define UCI_PARAM_LEN_SESSION_INFO_NTF           0x01
#define UCI_PARAM_LEN_NEAR_PROXIMITY_CONFIG      0x02
#define UCI_PARAM_LEN_FAR_PROXIMITY_CONFIG       0x02
#define UCI_PARAM_LEN_DEVICE_TYPE                0x01
#define UCI_PARAM_LEN_RFRAME_CONFIG              0x01
#define UCI_PARAM_LEN_PREAMBLE_CODE_INDEX        0x01
#define UCI_PARAM_LEN_SFD_ID                     0x01
#define UCI_PARAM_LEN_PSDU_DATA_RATE             0x01
#define UCI_PARAM_LEN_PREAMBLE_DURATION          0x01
#define UCI_PARAM_LEN_RANGING_TIME_STRUCT        0x01
#define UCI_PARAM_LEN_AOA_BOUND_CONFIG           0x08
#define UCI_PARAM_LEN_SLOTS_PER_RR               0x01
#define UCI_PARAM_LEN_TX_POWER_ID                0x01
#define UCI_PARAM_LEN_TX_ADAPTIVE_PAYLOAD_POWER  0x01
#define UCI_PARAM_LEN_VENDOR_ID                  0x02
#define UCI_PARAM_LEN_STATIC_STS_IV              0x06
#define UCI_PARAM_LEN_NUMBER_OF_STS_SEGMENTS     0x01
#define UCI_PARAM_LEN_MAX_RR_RETRY               0x02
#define UCI_PARAM_LEN_UWB_INITIATION_TIME        0x04
#define UCI_PARAM_LEN_RANGING_ROUND_HOPPING      0x01
#define UCI_PARAM_LEN_MAX_NUMBER_OF_MEASUREMENTS 0X02
#define UCI_PARAM_LEN_UL_TDOA_TX_INTERVAL        0X04

/*************************************************
 * Status codes
 ************************************************/
/* Generic Status Codes */
#define UCI_STATUS_OK                                  0x00
#define UCI_STATUS_REJECTED                            0x01
#define UCI_STATUS_FAILED                              0x02
#define UCI_STATUS_SYNTAX_ERROR                        0x03
#define UCI_STATUS_INVALID_PARAM                       0x04
#define UCI_STATUS_INVALID_RANGE                       0x05
#define UCI_STATUS_INVALID_MSG_SIZE                    0x06
#define UCI_STATUS_UNKNOWN_GID                         0x07
#define UCI_STATUS_UNKNOWN_OID                         0x08
#define UCI_STATUS_READ_ONLY                           0x09
#define UCI_STATUS_MESSAGE_RETRY                       0x0A
#define UCI_STATUS_UNKNOWN                             0x0B
#define UCI_STATUS_DEVICE_TEMP_REACHED_THERMAL_RUNAWAY 0x54

/* UWB Session Specific Status Codes*/
#define UCI_STATUS_SESSSION_NOT_EXIST               0x11
#define UCI_STATUS_INVALID_PHASE_PARTICIPATION      0x12
#define UCI_STATUS_SESSSION_ACTIVE                  0x13
#define UCI_STATUS_MAX_SESSSIONS_EXCEEDED           0x14
#define UCI_STATUS_SESSION_NOT_CONFIGURED           0x15
#define UCI_STATUS_SESSIONS_ONGOING                 0X16
#define UCI_STATUS_SESSIONS_MULTICAST_LIST_FULL     0X17
#define UCI_STATUS_SESSIONS_ADDRESS_NOT_FOUND       0X18
#define UCI_STATUS_SESSIONS_ADDRESS_ALREADY_PRESENT 0X19

/* UWB Ranging Session Specific Status Codes */
#define UCI_STATUS_RANGING_TX_FAILED            0x20
#define UCI_STATUS_RANGING_RX_TIMEOUT           0x21
#define UCI_STATUS_RANGING_RX_PHY_DEC_FAILED    0x22
#define UCI_STATUS_RANGING_RX_PHY_TOA_FAILED    0x23
#define UCI_STATUS_RANGING_RX_PHY_STS_FAILED    0x24
#define UCI_STATUS_RANGING_RX_MAC_DEC_FAILED    0x25
#define UCI_STATUS_RANGING_RX_MAC_IE_DEC_FAILED 0x26
#define UCI_STATUS_RANGING_RX_MAC_IE_MISSING    0x27

/* UWB Data Session Specific Status Codes */
#define UCI_STATUS_DATA_TRANSFER_ERROR 0x90
#define UCI_STATUS_NO_CREDIT_AVAILABLE 0x00
#define UCI_STATUS_CREDIT_AVAILABLE    0x01
/*************************************************
* Device Role config
**************************************************/
#define UWB_CONTROLLER 0x00
#define UWB_CONTROLEE  0x01

/*************************************************
* Ranging Method config
**************************************************/
#define ONE_WAY_RANGING 0x00
#define SS_TWR_RANGING  0x01
#define DS_TWR_RANGING  0x02

/*************************************************
* Ranging Mesaurement type
**************************************************/
#define MEASUREMENT_TYPE_ONEWAY       0x00
#define MEASUREMENT_TYPE_TWOWAY       0x01
#define MEASUREMENT_TYPE_DLTDOA       0x02
#define MEASUREMENT_TYPE_OWR_WITH_AOA 0x03

/*************************************************
* Radar Mesaurement type
**************************************************/
#define RADAR_MEASUREMENT_TYPE_CIR            0x00
#define RADAR_MEASUREMENT_TYPE_TEST_ISOLATION 0x20
/*************************************************
* Mac Addressing Mode Indicator
**************************************************/
#define SHORT_MAC_ADDRESS               0x00
#define EXTENDED_MAC_ADDRESS            0x01
#define EXTENDED_MAC_ADDRESS_AND_HEADER 0x02

#define SESSION_ID_LEN     0x04
#define SESSION_HANDLE_LEN SESSION_ID_LEN
#define SHORT_ADDRESS_LEN  0x02
#if (defined(UWBIOT_UWBD_SR040) && (UWBIOT_UWBD_SR040 != 0))
#define MAX_NUM_OF_TDOA_MEASURES 1
#else
#define MAX_NUM_OF_TDOA_MEASURES 22
#endif
#define MAX_NUM_OWR_AOA_MEASURES 1
#define MAX_NUM_CONTROLLEES      8 /* max bumber of controlees for  time schedules rangng ( multicast)*/

/* UCI Response Buffer */
#define MAX_RESPONSE_DATA 0xFF // For DSTWR, TDOA ranging
/* max no of responders N 10 for dltdoa
 * N + 2 ==> 12 * 37(dltdoa ntf size) = 444 + 28 --> 472
 */
#define MAX_RADAR_LEN                4106 // MAX 8 CIRS * 128 CIR TAPS *4 (1 TAP = 4 BYTES) + 10 CIR HEADER(exclude UCI Header)
#define MAX_RESPONSE_DATA_DLTDOA_TAG 472
#define MAX_RESPONSE_DATA_DEBUG_NTF  4106 // For CIR, PSDU debug Notification
#define MAX_RESPONSE_DATA_RCV        2031 // For Data transfer, max data size which we can send and rcv is 2031
#define MAX_RESPONSE_DATA_DATA_TRANSFER \
    2048 // For Data transfer, max data size which we can send and rcv is 2031 + 16 (data header)
#if (defined(UWBFTR_Radar) && (UWBFTR_Radar != 0))
#define UCI_MAX_DATA_LEN (UCI_MSG_HDR_SIZE + MAX_RADAR_LEN)
#elif (defined(UWBFTR_DataTransfer) && (UWBFTR_DataTransfer != 0))
#define UCI_MAX_DATA_LEN (UCI_MSG_HDR_SIZE + MAX_RESPONSE_DATA_DATA_TRANSFER)
#elif (defined(UWBFTR_UWBS_DEBUG_Dump) && (UWBFTR_UWBS_DEBUG_Dump != 0))
#define UCI_MAX_DATA_LEN (UCI_MSG_HDR_SIZE + MAX_RESPONSE_DATA_DEBUG_NTF)
#elif (defined(UWBFTR_DL_TDoA_Tag) && (UWBFTR_DL_TDoA_Tag != 0))
#define UCI_MAX_DATA_LEN (UCI_MSG_HDR_SIZE + MAX_RESPONSE_DATA_DLTDOA_TAG)
#else
#define UCI_MAX_DATA_LEN (UCI_MSG_HDR_SIZE + MAX_RESPONSE_DATA)
#endif

/* UCI command buffer */
#define MAX_CMD_BUFFER_DATA_TRANSFER 2048 // For Data transfer
#define MAX_CMD_BUFFER               0xFF // For Uci commands other than data transfer
#if (defined(UWBFTR_DataTransfer) && (UWBFTR_DataTransfer != 0))
#define UCI_MAX_CMD_BUF_LEN (UWB_HDR_SIZE + UCI_MSG_HDR_SIZE + MAX_CMD_BUFFER_DATA_TRANSFER)
#else
#define UCI_MAX_CMD_BUF_LEN (UWB_HDR_SIZE + UCI_MSG_HDR_SIZE + MAX_CMD_BUFFER)
#endif

/* device status */
typedef enum
{
#if ((defined(NXP_UWB_EXTNS)) && (NXP_UWB_EXTNS == TRUE))
    UWBD_STATUS_INIT = 0x00, /* UWBD is idle */
#endif
    UWBD_STATUS_READY = 0x01,      /* UWBD is ready for  performing uwb session with non SE use cases */
    UWBD_STATUS_ACTIVE,            /* UWBD is busy running uwb session */
    UWBD_STATUS_HDP_WAKEUP = 0xFC, /* UWBD Wakeup error*/
    UWBD_STATUS_UNKNOWN    = 0xFE, /* device is unknown */
    UWBD_STATUS_ERROR      = 0xFF  /* error occured in UWBD*/
} eUWBD_DEVICE_STATUS_t;

/* Session status */
typedef enum
{
    UWB_SESSION_INITIALIZED,
    UWB_SESSION_DEINITIALIZED,
    UWB_SESSION_ACTIVE,
    UWB_SESSION_IDLE,
    UWB_SESSION_ERROR = 0xFF
} eSESSION_STATUS_t;

/** \addtogroup uwb_status
 * @{ */

/**
 * \brief Session status error reason code
 * 0x06 - 0x1C : RFU
 * 0x40 - 0x7F : RFU
 * 0x80 - 0xFF : Reserved for Vendor Specific Use
 * 0xA0 - 0xAF : Reserved for CCC Specific Use
 */
typedef enum
{
    UWB_SESSION_STATE_CHANGED                           = 0x00,
    UWB_SESSION_MAX_RR_RETRY_COUNT_REACHED              = 0x01,
    UWB_SESSION_MAX_RANGING_BLOCKS_REACHED              = 0x02,
    UWB_SESSION_SUSPENDED_DUE_TO_INBAND_SIGNAL          = 0x03,
    UWB_SESSION_RESUMED_DUE_TO_INBAND_SIGNAL            = 0x04,
    UWB_SESSION_STOPPED_DUE_TO_INBAND_SIGNAL            = 0x05,
    UWB_SESSION_INVALID_UL_TDOA_RANDOM_WINDOW           = 0x1D,
    UWB_SESSION_MIN_RFRAMES_PER_RR_NOT_SUPPORTED        = 0x1E,
    UWB_SESSION_TX_DELAY_NOT_SUPPORTED                  = 0x1F,
    UWB_SESSION_SLOT_LENTGH_NOT_SUPPORTED               = 0x20,
    UWB_SESSION_SLOTS_PER_RR_NOT_SUFFICIENT             = 0x21,
    UWB_SESSION_MAC_ADDRESS_MODE_NOT_SUPPORTED          = 0x22,
    UWB_SESSION_INVALID_RANGING_DURATION                = 0x23,
    UWB_SESSION_INVALID_STS_CONFIG                      = 0x24,
    UWB_SESSION_HUS_INVALID_RFRAME_CONFIG               = 0x25,
    UWB_SESSION_HUS_NOT_ENOUGH_SLOTS                    = 0x26,
    UWB_SESSION_HUS_CFP_PHASE_TOO_SHORT                 = 0x27,
    UWB_SESSION_HUS_CAP_PHASE_TOO_SHORT                 = 0x28,
    UWB_SESSION_HUS_OTHERS                              = 0x29,
    UWB_SESSION_STATUS_SESSION_KEY_NOT_FOUND            = 0x2A,
    UWB_SESSION_STATUS_SUB_SESSION_KEY_NOT_FOUND        = 0x2B,
    UWB_SESSION_INVALID_PREAMBLE_CODE_INDEX             = 0x2C,
    UWB_SESSION_INVALID_SFD_ID                          = 0x2D,
    UWB_SESSION_INVALID_PSDU_DATA_RATE                  = 0x2E,
    UWB_SESSION_INVALID_PHR_DATA_RATE                   = 0x2F,
    UWB_SESSION_INVALID_PREAMBLE_DURATION               = 0x30,
    UWB_SESSION_INVALID_STS_LENGTH                      = 0x31,
    UWB_SESSION_INVALID_NUM_OF_STS_SEGMENTS             = 0x32,
    UWB_SESSION_INVALID_NUM_OF_CONTROLEES               = 0x33,
    UWB_SESSION_MAX_RANGING_REPLY_TIME_EXCEEDED         = 0x34,
    UWB_SESSION_INVALID_DST_ADDRESS_LIST                = 0x35,
    UWB_SESSION_INVALID_OR_NOT_FOUND_SUB_SESSION_ID     = 0x36,
    UWB_SESSION_INVALID_RESULT_REPORT_CONFIG            = 0x37,
    UWB_SESSION_INVALID_RANGING_ROUND_CONTROL_CONFIG    = 0x38,
    UWB_SESSION_INVALID_RANGING_ROUND_USAGE             = 0x39,
    UWB_SESSION_INVALID_MULTI_NODE_MODE                 = 0x3A,
    UWB_SESSION_RDS_FETCH_FAILURE                       = 0x3B,
    UWB_SESSION_DOES_NOT_EXIST                          = 0x3C,
    UWB_SESSION_RANGING_DURATION_MISMATCH               = 0x3D,
    UWB_SESSION_INVALID_OFFSET_TIME                     = 0x3E,
    UWB_SESSION_LOST                                    = 0x3F,
    UWB_SESSION_DT_ANCHOR_RANGING_ROUNDS_NOT_CONFIGURED = 0x40,
    UWB_SESSION_DT_TAG_RANGING_ROUNDS_NOT_CONFIGURED    = 0x41,
    UWB_SESSION_ERROR_INVALID_ANTENNA_CFG               = 0x80,
    UWB_SESSION_BASEBAND_ERROR                          = 0x84,
    UWB_SESSION_TESTMODE_TERMINATED                     = 0x85,
    UWB_SESSION_INVALID_DATA_TRANSFER_MODE              = 0x86,
    UWB_SESSION_INVALID_MAC_CFG                         = 0x87,
    UWB_SESSION_INVALID_ADAPTIVE_HOPPING_THRESHOLD      = 0x88,
    UWB_SESSION_UNSUPPORTED_RANGING_LIMIT               = 0x89,
    UWB_SESSION_RNG_INVALID_DEVICE_ROLE                 = 0x8A,
    UWB_SESSION_INVALID_ANTENNA_PAIR_SWAP_CONFIGURATION = 0x94,
    UWB_SESSION_URSK_EXPIRED                            = 0xA0,
    UWB_SESSION_TERMINATION_ON_MAX_STS                  = 0xA1,
    UWB_SESSION_RADAR_FCC_LIMIT_REACHED                 = 0xA2,
    UWB_SESSION_CSA_INVALID_CFG                         = 0xA3,
} eSESSION_STATUS_REASON_CODES_t;
/** @} */ /* @addtogroup uwb_status */

// TODO: eSESSION_STATUS_REASON_CODES_t needs to be updated.
#define UWB_SESSION_HUS_NOT_ENOUGH_SLOTS UWB_SESSION_HUS_CFP_PHASE_TOO_SHORT

#endif /* UWB_UCI_DEFS_H */
