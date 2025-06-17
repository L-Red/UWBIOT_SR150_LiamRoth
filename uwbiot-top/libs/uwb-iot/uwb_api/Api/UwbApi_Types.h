/*
 *
 * Copyright 2018-2020,2022-2024 NXP.
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

#ifndef UWBAPI_TYPES_H
#define UWBAPI_TYPES_H

#include "phUwbTypes.h"
#include "uci_defs.h"
#include <uwb_board.h>

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

/**
 *  \brief Constants used by UWBD
 */

/** \addtogroup Uwb_Constants
 * @{ */

#if defined(UWBIOT_TIMING_RSTU) && (UWBIOT_TIMING_RSTU == 1)
/* Convert micro seconds to RSTU if Needed */
#define UWBIOT_USEC_TIMING(V) ((V)*6 / 5)
#define UWBIOT_RSTU_TIMING(V) (V)
#else
/* KEEP USEC to USEC */
#define UWBIOT_USEC_TIMING(V) (V)
#define UWBIOT_RSTU_TIMING(V) ((V)*5 / 6)
#endif

/**
 *  \brief Constants used by UWB API layer
 */

/**  MAX UCI header size */
#define MAX_UCI_HEADER_SIZE 4
/**  MAX UCI packet size */
#define MAX_UCI_PACKET_SIZE 255
/**  MAX API transfer packet size
 *  Needed 768 bytes for set and get calibration
*/
#define MAX_API_PACKET_SIZE 768
/** MAX Number of Responders, for Conteniton based Ranging it can go upto 12 */
#define MAX_NUM_RESPONDERS 12
/**  max app data size */
#define MAX_APP_DATA_SIZE 116
/**  Max psdu data size
 * MAX 128 for BPRF
 * MAX 4096 for HPRF
 * TODO: needs to be updated for HPRF.
*/
#define MAX_PSDU_DATA_SIZE 128
/** Max debug ntf size */
#define MAX_DEBUG_NTF_SIZE 4100
/**  MAX Number of Controlees for Physical Access */
#define MAX_NUM_PHYSICAL_ACCESS_CONTROLEES 8
/** MAX Profile Blob Size v1.0*/
#define TOTAL_PROFILE_BLOB_SIZE_v1_0 28
/** MAX Profile Blob Size v1.1*/
#define TOTAL_PROFILE_BLOB_SIZE_v1_1 30
/** LENGTH OF THE MAC ADDRESS*/
#define MAC_ADDR_LENGTH 8
/** Number of channels */
#define NO_OF_CHANNELS 4
/** Number of blocks */
#define NO_OF_BLOCKS 4
/**  Short MAC Address Mode */
#define SHORT_MAC_ADDRESS_MODE 0x00
/**  Extended MAC Address Mode */
#define EXTENDED_MAC_ADDRESS_MODE_WITH_HEADER 0x02
/**  Short MAC Mode Address length */
#define MAC_SHORT_ADD_LEN 2
/**  Extended MAC Mode Address length */
#define MAC_EXT_ADD_LEN 8
/**  AOA Phase difference length */
#define AOA_PHASE_DIFF_LEN 4
/**  AOA UCI Field length */
#define AOA_LEN 4
/**  Single AOA length */
#define SINGLE_AOA_LEN 4
/** sub session Handle length */
#define SUBSESSION_HANDLE_LEN 4
/** add a controllee to multicast addr list */
#define MULTICAST_LIST_ADD_CONTROLEE 0
/** delete a controlee from multicast addr list */
#define MULTICAST_LIST_DEL_CONTROLEE 1
#define DEFAULT_EVENT_TYPE           0xFF
/**  UWB command time out 2 Sec */
#define UWB_CMD_TIMEOUT 2000
/**  UWB notification timeout 2000ms */
#define UWB_NTF_TIMEOUT 2000
/**  UWB transmission status timeout */
#define UWBD_LOCK_TIMEOUT 2000
/**  UWB device bind lock timeout 2 sec */
#define UWBD_TRANSMIT_NTF_TIMEOUT 10000
/**  timeout for SE APIs */
#define UWBD_SE_TIMEOUT 10000
/**  timeout for SE Ranging APIs */
#define UWBD_SE_RANGING_TIMEOUT 40000
/**  Time out value for calibration notification */
#define UWBD_CALIB_NTF_TIMEOUT 2000
/**  Time out value for do calibration notification */
#define UWBD_DO_CALIB_NTF_TIMEOUT 2000
/**  Key Fetch Error Retry Count */
#define UWB_KEY_FETCH_ERROR_RETRY_COUNT 0x01
/**  Time out value for Generate Tag notification */
#define UWBD_GENERATE_TAG_NTF_TIMEOUT 2000
/**  Binding Success Status value */
#define STATUS_BINDING_SUCCESS 0x50
/** MAX Number of Anchor Locations */
#define MAX_ANCHOR_LOCATIONS 12
/** Max RX-TX Timestamp */
#define MAX_RX_TX_TIMESTAMP 8
/** MAX Active Ranging Rounds */
#define MAX_ACTIVE_RR 0xF
/** TX timestamp type */
#define TX_TIMESTAMP_TYPE 1
/** TX timestamp length */
#define TX_TIMESTAMP_LEN 2
/** RX timestamp length */
#define RX_TIMESTAMP_LEN 8
/**WGS-84 Coordinates system for DT-Anchor location */
#define ANCHOR_LOCATION_WGS84 32
/** Relative Coordinate system for DT-Anchor location */
#define ANCHOR_LOCATION_REL 64
/** Number of Active Ranging Rounds */
#define ACTIVE_RR_OFSET 7
/** Maximum number of sessions **/
#define MAXIMUM_SESSION_COUNT 5
/** 16 Bytes Sub-Session Key length */
#define SUB_SESSION_KEY_LEN_16B 16
/** 32 Bytes Sub-Session Key length */
#define SUB_SESSION_KEY_LEN_32B 32
/** Max Sub-session Key Size */
#define MAX_SUB_SESSION_KEY_LEN SUB_SESSION_KEY_LEN_32B
/** Max UWBD binding Count */
#define MAX_UWBD_BINDING_CNT 3
/** UWBD binding Status */
#define UWBD_NOT_BOUND 0
/** Failed to create thread */
#define THREAD_CREATION_FAILED 0
/** Maximun number of phases, 1 octet */
#define MAX_PHASE_COUNT 6
/** Update Time, 8 octet */
#define HUS_CONFIG_UPDATE_TIME_OCTETS 8
/** Max Destination Address List Size */
#define MAX_DST_ADDR_LIST_SIZE 5
/** Max allowed range slots */
#define MAX_ALLOWED_SLOT_INDEX 32768
/** Maximum slot bitmap size */
#define MAX_SLOT_BITMAP_SIZE 32
/** Session Handle offset in response */
#define SESSION_HANDLE_OFFSET 1
/** Session Handle offset length, 1-octate status + 4-octates SessionHandle */
#define SESSION_HANDLE_OFFSET_LEN (1 + 4)
/** MAX CCM TAG SIZE */
#define MAX_CCM_TAG_SIZE 8
/** Max Spec Version Length */
#define MAX_SPEC_VER_LEN 2
/* Max sharable data header length for Android  */
#define SHAREABLE_DATA_HEADER_LENGTH_ANDROID 14

/** Skip MAC Address mode offset [b0] */
#define SKIP_MAC_ADDR_OFFSET 1
/** Ranging Slot Offset [b1-b3] */
#define RANGING_SLOT_OFFSET 1
/** Get Ranging Slots for slot bitmap */
#define GET_RANGING_SLOTS(x) (RANGING_SLOT_OFFSET << (x >> SKIP_MAC_ADDR_OFFSET))

/* Debive Capability Lengths */
#define DEVICE_CAPABILITY_LEN_1 1
#define DEVICE_CAPABILITY_LEN_2 2
#define DEVICE_CAPABILITY_LEN_3 3
#define DEVICE_CAPABILITY_LEN_4 4
#define DEVICE_CAPABILITY_LEN_5 5
#define DEVICE_CAPABILITY_LEN_6 6
#define DEVICE_CAPABILITY_LEN_9 9

/** Extented Calibration Param Id*/
#define EXTENTED_CALIB_PARAM_ID   0xE0

/*Device Info Parameters*/
#if UWBIOT_UWBD_SR040
#define FIRA_PHY_VERSION_RANGE_ID       0x00
#define FIRA_MAC_VERSION_RANGE_ID       0x01
#define DEVICE_ROLES_ID                 0x02
#define RANGING_METHOD_ID               0x03
#define STS_CONFIG_ID                   0x04
#define MULTI_NODE_MODE_ID              0x05
#define RANGING_TIME_STRUCT_ID          0x06
#define SCHEDULED_MODE_ID               0x07
#define HOPPING_MODE_ID                 0x08
#define BLOCK_STRIDING_ID               0x09
#define UWB_INITIATION_TIME_ID          0x0A
#define CHANNELS_ID                     0x0B
#define RFRAME_CONFIG_ID                0x0C
#define CC_CONSTRAINT_LENGTH_ID         0x0D
#define BPRF_PARAMETER_SETS_ID          0x0E
#define HPRF_PARAMETER_SETS_ID          0x0F
#define AOA_SUPPORT_ID                  0x10
#define EXTENDED_MAC_ADDR_ID            0x11
#define MAX_MESSAGE_SIZE_ID             0x12
#define MAX_DATA_PACKET_PAYLOAD_SIZE_ID 0x13

#else // !(UWBIOT_UWBD_SR040)

#define MAX_MESSAGE_SIZE_ID             0x00
#define MAX_DATA_PACKET_PAYLOAD_SIZE_ID 0x01
#define FIRA_PHY_VERSION_RANGE_ID       0x02
#define FIRA_MAC_VERSION_RANGE_ID       0x03
#define DEVICE_TYPE_ID                  0x04
#define DEVICE_ROLES_ID                 0x05
#define RANGING_METHOD_ID               0x06
#define STS_CONFIG_ID                   0x07
#define MULTI_NODE_MODE_ID              0x08
#define RANGING_TIME_STRUCT_ID          0x09
#define SCHEDULED_MODE_ID               0x0A
#define HOPPING_MODE_ID                 0x0B
#define BLOCK_STRIDING_ID               0x0C
#define UWB_INITIATION_TIME_ID          0x0D
#define CHANNELS_ID                     0x0E
#define RFRAME_CONFIG_ID                0x0F
#define CC_CONSTRAINT_LENGTH_ID         0x10
#define BPRF_PARAMETER_SETS_ID          0x11
#define HPRF_PARAMETER_SETS_ID          0x12
#define AOA_SUPPORT_ID                  0x13
#define EXTENDED_MAC_ADDR_ID            0x14
#define SUSPEND_RANGING_ID              0x15
#define SESSION_KEY_LEN_ID              0x16
#define DT_ANCHOR_MAX_ACTIVE_RR_ID      0x17
#define DT_TAG_MAX_ACTIVE_RR_ID         0x18
#define DT_TAG_BLOCK_SKIPPING_ID        0x19
#define PSDU_LENGTH_SUPPORT_ID          0x1A
#endif // UWBIOT_UWBD_SR040

#if UWBIOT_UWBD_SR1XXT_SR2XXT
#define SLOT_BITMASK               0xA0
#define SYNC_CODE_INDEX_BITMASK    0xA1
#define HOPPING_CONFIG_BITMASK     0xA2
#define CHANNEL_BITMASK            0xA3
#define SUPPORTED_PROTOCOL_VERSION 0xA4
#define SUPPORTED_UWB_CONFIG_ID    0xA5
#define SUPPORTED_PULSESHAPE_COMBO 0xA6
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT

/**  EXTENDED PARAM ID MASK */
#define EXTENDED_PARAM_ID_MASK 0xF0
/** CCC PARAM ID */
#define CCC_INFO_ID 0xA0
/*CCC EXT PARAM ID*/
#define CCC_EXT_PARAM_ID 0xE0
/** Maximum number of PDoA Measurements */
#define MAX_NO_OF_PDOA_MEASUREMENTS 255
/** Maximum number of ccc RSSI Measurements */
#define MAX_NO_OF_CCC_RSSI_MEASUREMENTS 2
/** Maximum number of ccc SNR Measurements
 * for H1:
 *  - poll rx1
 *  - poll rx2
 *  - final rx1
 *  - final rx2
 * */
#define MAX_NO_OF_CCC_SNR_MEASUREMENTS 4

/**Number of aoa fine calib ctrl pairs, per ch, per pol per ant, 8**/
#define NO_OF_CALIB_PAIRS 4
/**Number of ang sweeps for aoa fine calib**/
#define AOA_FINE_CALIB_HOR_SWEEPS 11
#define AOA_FINE_CALIB_VER_SWEEPS 11

#define UWB_IOS_SPEC_VERSION_MAJOR \
    {                              \
        0x01, 0x00                 \
    }
#define UWB_IOS_SPEC_VERSION_MINOR \
    {                              \
        0x01, 0x00                 \
    }

#define UWB_ANDROID_SPEC_VERSION_MAJOR \
    {                                  \
        0x01, 0x00                     \
    }
#define UWB_ANDROID_SPEC_VERSION_MINOR \
    {                                  \
        0x00, 0x00                     \
    }

#define MANUFACTURER_ID        \
    {                          \
        0x32, 0x11, 0x10, 0x00 \
    } // NXP Manufacturer ID

/** Definitions for the UWB configuration IDs */
#define UWB_CONFIG_ID_1_MASK (uint32_t)(1 << 1)
#define UWB_CONFIG_ID_2_MASK (uint32_t)(1 << 2)
#define UWB_CONFIG_ID_3_MASK (uint32_t)(1 << 3)

/** Definitions for the UWB configuration IDs */
#define UWB_CONFIG_ID_1 0x01U
#define UWB_CONFIG_ID_2 0x02U
#define UWB_CONFIG_ID_3 0x03U

/* Max number of confgis */
#define MAX_CONFIG_ID_NUM_CONFIGS 10

/** Defines supported UWB configuration IDs by the device */
#define UWB_SUPPORTED_CONFIG_IDS (uint32_t)(UWB_CONFIG_ID_1_MASK | UWB_CONFIG_ID_2_MASK | UWB_CONFIG_ID_3_MASK)

/** Definitions for the supported UWB roles */
/** UWB device supports Controller device type and Initiator device role */
#define UWB_DEVICE_CONTROLLER (uint8_t)(1 << 0)
/** UWB device supports Controlee device type and Responder device role */
#define UWB_DEVICE_CONTROLEE (uint8_t)(1 << 1)

/** Defines the set of UWB Ranging role configurations supported by the UWB device */
#if UWBIOT_UWBD_SR040
#define UWB_SUPPORTED_DEVICE_RANGING_ROLES (uint8_t)(UWB_DEVICE_CONTROLLER)
#else
#define UWB_SUPPORTED_DEVICE_RANGING_ROLES (uint8_t)(UWB_DEVICE_CONTROLLER | UWB_DEVICE_CONTROLEE)
#endif //UWBIOT_UWBD_SR040

/** Macro to get the Array size */
#define GET_ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

/*
 *  \brief Deprecated related macros.
 */
#define __DEPRECATED 1

/** @} */ /* @addtogroup uwb_const */

/** \addtogroup uwb_status
 * @{ */
/**
 * \brief Status Codes, as per UCI.
 */
/**  Command succeeded */
#define UWBAPI_STATUS_OK 0x00
/**  Request is rejected */
#define UWBAPI_STATUS_REJECTED 0x01
/**  Command Failed */
#define UWBAPI_STATUS_FAILED 0x02
/**  API called without UCI being initialized */
#define UWBAPI_STATUS_NOT_INITIALIZED 0x03
/**  Invalid parameter provided */
#define UWBAPI_STATUS_INVALID_PARAM 0x04
/** Invalid value range provided */
#define UWBAPI_STATUS_INVALID_RANGE 0x05
/** Session wrt SESSION Handle Does not exist */
#define UWBAPI_STATUS_SESSION_NOT_EXIST 0x11
/**  Invalid Phase Participation values in SESSION_SET_HUS_CONFIG_CMD */
#define UWBAPI_STATUS_INVALID_PHASE_PARTICIPATION 0x12
/**  Session active */
#define UWBAPI_STATUS_SESSION_ACTIVE 0x13
/**  MAX Sessions exceeded */
#define UWBAPI_STATUS_MAX_SESSIONS_EXCEEDED 0x14
/**  Operation is started with out configuring required parameters for Session */
#define UWBAPI_STATUS_SESSION_NOT_CONFIGURED 0x15
/**  Sessions ongoing */
#define UWBAPI_STATUS_SESSIONS_ONGOING 0X16
/**  Indicates when multicast list is full during one to many ranging */
#define UWBAPI_STATUS_MULTICAST_LIST_FULL 0x17
/** Negative distance was reported */
#define UWBAPI_STATUS_OK_NEGATIVE_DISTANCE_REPORT 0x1B
/**  ESE Rest happened during command processing. */
#define UWBAPI_STATUS_ESE_RESET 0x71
/** Unrecoverable data transfer error */
#define UWBAPI_STATUS_DATA_TRANSFER_ERROR 0x30
/** Credit not available for Data Packet */
#define UWBAPI_STATUS_NO_CREDIT_AVAILABLE 0x31
/** Given round index couldn't be activated */
#define UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED 0x28
/** Given round exceeds the possible number of ranging rounds */
#define UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED 0x29
/** The role for the configured ranging round index is not Initiator and therefore RDM list cannot be set */
#define UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_SET_AS_INITIATOR 0x2A
/** Device address not matching */
#define UWBAPI_STATUS_DLTDOA_DEVICE_ADDRESS_NOT_MATCHING_IN_REPLY_TIMELIST 0x30
/**  Buffer overflow */
#define UWBAPI_STATUS_BUFFER_OVERFLOW 0xFA
/**  Status PBF=1 CMD SENT */
#define UWBAPI_STATUS_PBF_PKT_SENT 0xFB
/**  Device is woken up from HPD */
#define UWBAPI_STATUS_HPD_WAKEUP 0xFC
/**  Command failed with time out */
#define UWBAPI_STATUS_TIMEOUT 0xFD
/**  SE Error */
#define UWBAPI_STATUS_ESE_ERROR 0xFF
/**  Ranging suspended */
#define UWBAPI_STATUS_SUSPEND 0x8B
/** DTPCM Status Ok, reported when DTPCM is configured for given MAC Address */
#define UWBAPI_STATUS_OK_DTPCM_CONFIG_SUCCESS
/** INVALID_SLOT_BITMAP Shall be reported when configured slot bit map size exceeds RANGING_DURATION */
#define UWBAPI_STATUS_ERROR_INVALID_SLOT_BITMAP
/** DUPLICATE_SLOT_ASSIGMENT Shall be reported when configured slot assignments is inconsistent, i.e., one slot is assigned to more than one FiRA device */
#define UWBAPI_STATUS_ERROR_DUPLICATE_SLOT_ASSIGMENT
/** INVALID_MAC_ADDRESS Shall be reported when given MAC address is not found */
#define UWBAPI_STATUS_ERROR_INVALID_MAC_ADDRESS
/** It is not known whether the intended operation was failed or successful. */
#define UWBAPI_STATUS_UNKNOWN 0x0B
/**
 * \brief Device Status codes, as per UCI.
 */
/** Device State - INIT */
#define UWBAPI_UCI_DEV_INIT 0x00
/**  Device State - READY */
#define UWBAPI_UCI_DEV_READY 0x01
/**  Device State - ACTIVE */
#define UWBAPI_UCI_DEV_ACTIVE 0x02
/** Device State - ERROR */
#define UWBAPI_UCI_DEV_ERROR 0xFF

/**
 * \brief Session State codes, as per UCI.
 */
/**  Session State - Session Init Success */
#define UWBAPI_SESSION_INIT_SUCCESS 0x00
/**  Session State - Session DeInit Success */
#define UWBAPI_SESSION_DEINIT_SUCCESS 0x01
/**  Session State - Session Activated */
#define UWBAPI_SESSION_ACTIVATED 0x02
/**  Session State - Session Idle */
#define UWBAPI_SESSION_IDLE 0x03
/**  Session State - Error */
#define UWBAPI_SESSION_ERROR 0xFF

/**
 * \brief Data Transfer related status codes.
 */

/** Data transmission is ongoing.*/
#define UWBAPI_DATA_TRANSFER_STATUS_REPETITION_OK 0x00
/** Data transmission is completed.*/
#define UWBAPI_DATA_TRANSFER_STATUS_OK 0x01
/** Application Data could not be sent due to an unrecoverable error*/
#define UWBAPI_DATA_TRANSFER_STATUS_ERROR 0x02
/** DATA_MESSAGE_SND is not accepted as no credit is available */
#define UWBAPI_DATA_TRANSFER_STATUS_NO_CREDIT_AVAILABLE 0x03
/** DATA_MESSAGE_SND packet sent in wrong state or Application Data Size exceeds the maximum size that can be sent in one Ranging Round.*/
#define UWBAPI_DATA_TRANSFER_STATUS_REJECTED 0x04
/** Data transfer is not supported for given session type.*/
#define UWBAPI_DATA_TRANSFER_STATUS_TYPE_NOT_SUPPORTED 0x05
/** Application Data is being transmitted and the number of configured DATA_REPETITION_COUNT transmissions is not yet completed */
#define UWBAPI_DATA_TRANSFER_DATA_TRANSFER_IS_ONGOING 0x06
/** The format of the command DATA_MESSAGE_SND associated with this notification is incorrect (e.g, a parameter is missing, a parameter value is invalid).*/
#define UWBAPI_DATA_TRANSFER_STATUS_INVALID_FORMAT 0x07
/**
 * \brief UL-TDoA related definitions.
 */
/**  UL-TDoA Message Control Mask */
#define ULTDOA_MESSAGE_CONTROL_MASK (0x3F)
/** 16-bit UL-TDoA Device ID */
#define ULTDOA_DEVICE_ID_16BIT_VALUE (0x01)
/** 32-bit UL-TDoA Device ID */
#define ULTDOA_DEVICE_ID_32BIT_VALUE (0x02)
/** 64-bit UL-TDoA Device ID */
#define ULTDOA_DEVICE_ID_64BIT_VALUE (0x03)
/** MASK for above values. */
#define ULTDOA_DEVICE_ID_MASK (0x03)

/** 40-bit TX timestamp */
#define ULTDOA_40BIT_TX_TIMESTAMP_MASK (0x04)
/** 64-bit TX timestamp */
#define ULTDOA_64BIT_TX_TIMESTAMP_MASK (0x08)

/** 64-bit RX timestamp */
#define ULTDOA_64BIT_RX_TIMESTAMP_MASK (0x10)

/** Length in bytes for 16-bits */
#define ULTDOA_16BIT_IN_BYTES (0x02)

/** Length in bytes for 32-bits */
#define ULTDOA_32BIT_IN_BYTES (0x04)

/** Length in bytes for 40-bits */
#define ULTDOA_40BIT_IN_BYTES (0x05)

/** Length in bytes for 64-bits */
#define ULTDOA_64BIT_IN_BYTES (0x08)

/** Length in Bytes for 128-bits*/
#define AUTH_TAG_IN_16BYTES (0x10)

/**  Session State - Session Init Success */
#define UWBAPI_SESSION_INIT_SUCCESS 0x00

/** Max number of supported rx antenna ID/pair*/
#define MAX_NUM_ANTENNA_PAIR 8

/** FoV specific Vendor Data type*/
#define FOV_SPECIFIC_DATA_TYPE 0xA0

/** @} */ /* @addtogroup uwb_status */

/** \addtogroup uwb_types
 * @{ */

/**
 *  \brief Types used by Uwb APIs
 */

/**
 * \brief  Status used by UWB APIs.
 */
typedef uint8_t tUWBAPI_STATUS;

/**
 * \brief       RESET Modes.
 */
typedef enum resetType
{
    /** UWB Device Reset */
    UWBD_RESET = 0,
    /** RFU */
    RFU
} eDevResetMode;

/**
 * \brief  UWBD Session Type.
 */
typedef enum session_type
{
    /** Ranging Session */
    UWBD_RANGING_SESSION = 0x00,
    /** Inband data transfer */
    UWBD_RANGING_WITH_INBAND_DATA_TRANSFER = 0x01,
    /** Data Transfer Session */
    UWBD_DATA_TRANSFER = 0x02,
    /** Ranging Only Phase */
    UWBD_RANGING_ONLY_PHASE = 0x03,
    /** Inband Data Phase */
    UWBD_INBAND_DATA_PHASE = 0x04,
    /** Ranging With Data Phase */
    UWBD_RANGING_WITH_DATA_PHASE = 0x05,
    /** CCC Session */
    UWBD_CCC_SESSION = 0xA0,
    /** Aliro Session */
    UWBD_CSA_SESSION = 0xA2,
    /** Test Session */
    UWBD_RFTEST = 0xD0,
    /** Radar Trasnfer Session */
    UWBD_RADAR_TRANSFER = 0xF0,
    /** RFU */
    UWBD_RFU = 0xFF,
} eSessionType;

/**
 * \brief Structure for storing  Session Data.
 */
typedef struct phUwbSessionData
{
    /** Session Handle */
    uint32_t sessionHandle;
    /** Session Type */
    uint8_t session_type;
    /** Session State */
    uint8_t session_state;
} phUwbSessionData_t;

/**
 * \brief Structure for storing  Session Context.
 */
typedef struct phUwbSessionsContext
{
    /** Status */
    uint8_t status;
    /** [Input/Output] Session Count */
    uint8_t sessioncnt;
    /** Pointer to Session Data */
    phUwbSessionData_t *pUwbSessionData;
} phUwbSessionsContext_t;

/**
 *  \brief Modes for setting LINK_LAYER_MODE app config, supported in UWB API layer.
 */
typedef enum linkLayerModes
{
    /** Bypass mode. (Default) */
    Link_Layer_Mode_Bypass = 0,
    /** Connection-Less mode. */
    Link_Layer_Mode_ConnectionLess,
    /** Connection mode. */
    Link_Layer_Mode_ConnectionOriented,
} eLinkLayerModes;

/**
 * \brief  UWBD notification Type.
 */
typedef enum notification_type
{
    /** Ranging Data Notification */
    UWBD_RANGING_DATA,
    /** Data Transmit Notification */
    UWBD_DATA_TRANSMIT_NTF,
    /** PER Packet Sent Notification */
    UWBD_PER_SEND,
    /** PER receive operation completed notification */
    UWBD_PER_RCV,
    /** Generic Error Notification */
    UWBD_GENERIC_ERROR_NTF,
    /** Upon Receiving Device Reset, Application needs to
     * clear all session context and re-initiate all the
     * sessions */
    UWBD_DEVICE_RESET,
    /** RFRAME Data Notification */
    UWBD_RFRAME_DATA,
    /** Upon receiving Recovery Notification, Application has
     * to invoke Recovery API non main/application thread
     * context. */
    UWBD_RECOVERY_NTF,
    /** UWBS shall send SCHED_STATUS_NTF notification
     * when scheduler meets either
     * SESSION_SCHEDULER_ATTEMPTS or
     * SESSION_SYNC_ATTEMPTS configuration criteria. */
    UWBD_SCHEDULER_STATUS_NTF,
    /** Session Data Notification */
    UWBD_SESSION_DATA,
    /** Multicast list notification */
    UWBD_MULTICAST_LIST_NTF,
    /** Over Temperature reached notification */
    UWBD_OVER_TEMP_REACHED,
    /** Blink data tx notification */
    UWBD_BLINK_DATA_TX_NTF,
    /** Data Tx Phase Configuration Notification */
    UWBD_DATA_TRANSFER_PHASE_CONFIG_NTF,
    /** TEST RX notification */
    /** Perform Application Clean Up & Restart upon receiving this notification */
    UWBD_ACTION_APP_CLEANUP,
    /** Loopback Status Data */
    UWBD_TEST_MODE_LOOP_BACK_NTF,
    /** PHY LOG NTF */
    UWB_TEST_PHY_LOG_NTF,
    /** TEST RX notification */
    UWBD_TEST_RX_RCV,
    /** Data receive */
    UWBD_DATA_RECV_NTF,
    /** CIR notification data */
    UWBD_CIR_DATA_NTF,
    /** Data logger ntf */
    UWBD_DATA_LOGGER_NTF,
    /** PSDU log ntf */
    UWBD_PSDU_DATA_NTF,
    /** RANGING timestamp ntf*/
    UWBD_RANGING_TIMESTAMP_NTF,
#if UWBFTR_DataTransfer
    /** Data receive notification */
    UWBD_DATA_RCV_NTF,
#endif //UWBFTR_DataTransfer
#if UWBFTR_Radar
    /** Radar rcv ntf */
    UWBD_RADAR_RCV_NTF,
    /** Test radar Isolation ntf */
    UWBD_TEST_RADAR_ISO_NTF,
#endif //UWBFTR_Radar
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    UWBD_WIFI_COEX_IND_NTF,
    /** Max Active Grant Duration Exceeded Warning NTF */
    UWBD_MAX_ACTIVE_GRANT_DURATION_EXCEEDED_WAR_NTF,
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#if UWBFTR_CCC
    /** Ranging Aliro Data Notification */
    UWBD_RANGING_ALIRO_DATA,
#endif //UWBFTR_CCC
} eNotificationType;

/**
 * \brief  Structure lists out the ranging measurement information.
 *         Ranging measurements array -- TWR
 */
typedef struct phRangingMesr
{
    /** Mac address of the participating device, addr can be
     * of short 2 byte or extended 8 byte modes */
    uint8_t mac_addr[MAC_ADDR_LENGTH];
    /** Status */
    uint8_t status;
    /** Indicates if the ranging measurement was in Line of sight or
     * non-line of sight */
    uint8_t nLos;
    /** AOA Azimuth FOM */
    uint8_t aoa_azimuth_FOM;
    /** AOA elevation FOM */
    uint8_t aoa_elevation_FOM;
    /** AOA destination azimuth FOM */
    uint8_t aoa_dest_azimuth_FOM;
    /** AOA destination elevation FOM */
    uint8_t aoa_dest_elevation_FOM;
    /** Status to the slot number within the ranging round
     *  In time schedule mode, in case of a failure, this field indicates the slot number
     *  within the ranging round where the failure has occurred
     *  In contention-based ranging mode, this field can be used to indicate
     *  the slot selected by the controlee device.
     *  If the Slot Index field is not used, then it shall be set to 0. */
    uint8_t slot_index;
    /** Rssi  */
    uint8_t rssi;
    /** Distance in centimeters */
    uint16_t distance;
    /** AOA Azimuth */
    int16_t aoa_azimuth;
    /** AOA Elevation */
    int16_t aoa_elevation;
    /** AOA destination azimuth */
    int16_t aoa_dest_azimuth;
    /** AOA destination elevation */
    int16_t aoa_dest_elevation;
} phRangingMesr_t;

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief  Structure lists out the Vendor speicifc Rx Antenna Info for AoA Measurements
 */
typedef struct phRxInfoMesr
{
    /** RX mode */
    uint8_t rxMode;
    /** Number of RX antenna to follow */
    uint8_t num_of_rx_antennaRxInfo;
    /** RX antenna pair */
    uint8_t rx_antennaIdRxInfo[MAX_NUM_ANTENNA_PAIR];
} phRxInfoMesr_t;

/**
 * \brief  Structure lists out the Vendor speicifc Rx Antenna Info For Debug Notifications
 */
typedef struct phRxInfoDebugNtf
{
    /** RX mode */
    uint8_t rxModeDebugNtf;
    /** Number of RX antenna to follow */
    uint8_t num_of_rx_antennaDebugNtf;
    /** RX antenna pair */
    uint8_t rx_antennaIdDebugNtf[MAX_NUM_ANTENNA_PAIR];
} phRxInfoDebugNtf_t;

/**
 * \brief  Structure lists out the Vendor speicifc information for AoA / PDoA measurements per RX
 */
typedef struct phAoaPdoaMesr
{
    /** Angle of arrival */
    int16_t angleOfArrival;
    /** Phase difference of arrival */
    int16_t pdoa;
    /** Phase difference of arrival index in the whole CIR */
    uint16_t pdoaIndex;
#if UWBIOT_UWBD_SR150
    /** This parameter indicates the presence or absence
     * of the peer device in FoV for user configured
     * Horizontal Rx Antenna Pair as defined in 'AZIMUTH_FIELD_OF_VIEW'
     * 0x00: Peer device is not present in FoV
     * 0x01: Peer device is present in FoV
     * This field would be displayed only when NXP Specific Data Type is 0xA0
    */
    uint8_t aoaFovFlag;
#endif
} phAoaPdoaMesr_t;

/**
 * \brief  Structure lists out the Vendor speicifc Rx Antenna information SNRFirst / SNRMain /FirstIndex :
 *  Main Index measurements per RX entry
 */
typedef struct phSnrPathIndexMesr
{
    /** Signal-to-Noise (SNR) of the First Path in dB*/
    uint8_t rxSnrFirstPath;
    /** Signal-to-Noise (SNR) of the Main Path in dB */
    uint8_t rxSnrMainPath;
    /** First path index in the whole CIR */
    int16_t rx_FirstPathIndex;
    /** Main path index in the whole CIR */
    int16_t rx_MainPathIndex;
} phSnrPathIndexMesr_t;

/**
 * \brief  Structure lists out the TWR Vendor speicifc measurements for each responder
*/
typedef struct phTwoWayRangingVsMesr
{
    /** AoA / PDoA measurements per RX */
    phAoaPdoaMesr_t aoaPdoaMesr_twr[MAX_NUM_ANTENNA_PAIR];
    /** SNRFirst / SNRMain / FirstIndex / Main Index measurements per RX */
    phSnrPathIndexMesr_t snrPathIndexMesr_twr[MAX_NUM_ANTENNA_PAIR];
    /** Range or distance between the device and target
     *  Set to 0 for Initiator: distance-2 is reported using the RFM
     *  pair on responder only in AOA_RFM mode */
    uint16_t distance_2;
} phTwoWayRangingVsMesr_t;

/**
 * \brief  Structure lists out the Vendor speicifc information for TWR
*/
typedef struct phTwoWayRangingVsData
{
    /** Vendor Specific Data*/
    /** WiFi co-existence status*/
    uint8_t wifiCoExStatus;
    /** Rx Antenna Info for AoA Measurements*/
    phRxInfoMesr_t rxInfoMesr_twr;
    /** Rx Antenna Info For Debug Notifications*/
    phRxInfoDebugNtf_t rxInfoDebugNtf_twr;
    /** twr vs measurment for each responders  */
    phTwoWayRangingVsMesr_t vsMesr[MAX_NUM_RESPONDERS];
} phTwoWayRangingVsData_t;

/**
 * \brief  Structure lists out the OWR Vendor speicifc measurements for each responder
 */
typedef struct phOneWayRangingVsMesr
{
    /** AoA / PDoA measurements per RX */
    phAoaPdoaMesr_t aoaPdoaMesr_owr[MAX_NUM_ANTENNA_PAIR];
    /** RSSI in dB **/
    int16_t rssi;
} phOneWayRangingVsMesr_t;

/**
 * \brief  Structure lists out the Vendor speicifc information for OWR
 */
typedef struct phOneWayRangingVsData
{
    /** Vendor Specific Data*/
    /** Rx Antenna Info for AoA Measurements*/
    phRxInfoMesr_t rxInfoMesr_owr;
    /** owr vs measurment for each responders  */
    phOneWayRangingVsMesr_t vsMesr[MAX_NUM_OF_TDOA_MEASURES];
} phOneWayRangingVsData_t;

#endif //UWBIOT_UWBD_SR1XXT_SR2XXT

#if UWBFTR_UL_TDoA_Anchor
/**
 * \brief  Structure lists out the TDoA ranging measurement information -- one way ranging.
 */
typedef struct phRangingMesrTdoa
{
    /** Mac address of the participating device,  addr can be
     * of short 2 byte or extended 8 byte modes */
    uint8_t mac_addr[MAC_ADDR_LENGTH];
    /** Status */
    uint8_t status;
    /** This signifies the presence of Device ID, Rx and Tx timestamps */
    uint8_t message_control;
    /** Type of frame */
    uint8_t frame_type;
    /** Indicates if the ranging measurement was in Line of sight or
     * non-line of sight */
    uint8_t nLos;
    /** AOA Azimuth */
    int16_t aoa_azimuth;
    /** AOA Azimuth FOM */
    uint8_t aoa_azimuth_FOM;
    /** AOA elevation */
    int16_t aoa_elevation;
    /** AOA elevation FOM */
    uint8_t aoa_elevation_FOM;
    /** Number received in the Payload of the blink UTM from the UT-Tag
     *   or UTM from the UT-Synchronization Anchor. */
    uint32_t frame_number;
    /** Local RX timestamp of the received UWB RFRAME as measured by
     *  the device that is sending this SESSION_INFO_NTF. The unit is
     *  2^(-7) of the 499.2 MHz chipping period, which is
     *  approximately 15.65ps.  */
    uint8_t rx_timestamp[ULTDOA_64BIT_IN_BYTES];
    /** UL-TDoA Device ID of the sender of the received UTM as listed
     *  in the Blink UTM from the UT-Tag or the Synchronization UTM
     *  from the UT-Synchronization Anchor. */
    uint8_t ul_tdoa_device_id[ULTDOA_64BIT_IN_BYTES];
    /** TX timestamp of the UWB RFRAME as listed in the Blink UTM
     *  from the UT-Tag or the Synchronization UTM from UT-Synchronization
     *  Anchor. The unit is 2^(-7) of the 499.2 MHz chipping period,
     *   which is approximately 15.65ps. */
    uint8_t tx_timestamp[ULTDOA_64BIT_IN_BYTES];
} phRangingMesrTdoa_t;

/**
 * \brief  Structure lists out the Vendor speicifc information for tdoa
 */
typedef struct phTdoaRangingVsData
{
    /** Vendor Specific Data*/
    /** Vendor extension length for TDOA Measurements*/
    uint8_t vendorExtLength;
    /** RSSI RX1 */
    int16_t rssi_rx1;
    /** RSSI RX2 */
    int16_t rssi_rx2;
    /** No of pdoA measurement */
    uint8_t noOfPdoaMeasures;
    /** Estimation of phase difference for antenna pair N */
    int16_t pdoa[MAX_NUM_ANTENNA_PAIR];
    /** CIR Index estimate at which pdoa has been detected */
    uint16_t pdoaIndex[MAX_NUM_ANTENNA_PAIR];
} phTdoaRangingVsData_t;
#endif //UWBFTR_UL_TDoA_Anchor

#if UWBFTR_DL_TDoA_Tag && UWBIOT_UWBD_SR1XXT
/**
 * \brief  Structure lists out the TDoA ranging measurement information.
 */
typedef struct phRangingMesrDlTdoa
{
    /** MAC address */
    uint8_t mac_addr[MAC_ADDR_LENGTH];
    /** Status */
    uint8_t status;
    /** Message type */
    uint8_t message_type;
    /** Message Control */
    uint16_t message_control;
    /** Block index */
    uint16_t block_index;
    /** Round index */
    uint8_t round_index;
    /** Nlos */
    uint8_t nLoS;
    /** AoA Azimuth */
    int16_t aoa_azimuth;
    /** AoA Azimuth FOM */
    uint8_t aoa_azimuth_fom;
    /** AoA Elevation */
    int16_t aoa_elevation;
    /** AoA Elevation FOM*/
    uint8_t aoa_elevation_fom;
    /** RSSI measured by DT-tag **/
    uint8_t rssi;
    /** Tx Timestamp */
    uint8_t tx_timestamp[MAX_RX_TX_TIMESTAMP];
    /**Rx Timestamp */
    uint8_t rx_timestamp[MAX_RX_TX_TIMESTAMP];
    /** CFO Anchor */
    int16_t cfo_anchor;
    /** Clock frequency offset */
    int16_t cfo;
    /** Reply time of initiator */
    uint32_t reply_time_initiator;
    /** Reply time of responder */
    uint32_t reply_time_responder;
    /** Initiator-Responder TOF */
    uint16_t initiator_responder_tof;
    /** Anchor Location */
    uint8_t anchor_location[MAX_ANCHOR_LOCATIONS];
    /** Active Ranging Rounds */
    uint8_t active_ranging_rounds[MAX_ACTIVE_RR];

} phRangingMesrDlTdoa_t;
#endif //UWBFTR_DL_TDoA_Tag && UWBIOT_UWBD_SR1XXT

#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
/**
 * \brief  Structure lists out the OWR with AoA ranging measurement information.
 */
typedef struct
{
    /** Mac address of the participating device,  addr can be
   * of short 2 byte or extended 8 byte modes */
    uint8_t mac_addr[8];
    /** Status */
    uint8_t status;
    /** Indicates if the ranging measurement was in Line of sight or
   * non-line of sight */
    uint8_t nLos;
    /** frame sequence number */
    uint8_t frame_seq_num;
    /** block index */
    uint16_t block_index;
    /** AOA Azimuth */
    int16_t aoa_azimuth;
    /** AOA Azimuth FOM */
    uint8_t aoa_azimuth_FOM;
    /** AOA Elevation */
    int16_t aoa_elevation;
    /** AOA elevation FOM */
    uint8_t aoa_elevation_FOM;
} phRangingMesrOwrAoa_t;
#endif //(UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
/**
 * \brief  Union for TWR ranging and TDoA ranging measurement information.
 */
typedef union {
    /** Ranging measurements array */
#if UWBFTR_TWR
    phRangingMesr_t range_meas_twr[MAX_NUM_RESPONDERS];
#endif //UWBFTR_TWR
#if UWBFTR_UL_TDoA_Anchor
    /** Ranging measurements TDoA array */
    /** One Way Ranging Ntf*/
    phRangingMesrTdoa_t range_meas_tdoa[MAX_NUM_OF_TDOA_MEASURES];
#endif //UWBFTR_UL_TDoA_Anchor
#if UWBIOT_UWBD_SR1XXT
#if UWBFTR_DL_TDoA_Tag
    /** Ranging measurements DLTDoA array */
    phRangingMesrDlTdoa_t range_meas_dltdoa[MAX_NUM_OF_TDOA_MEASURES];
#endif //UWBFTR_DL_TDoA_Tag
#endif //UWBIOT_UWBD_SR1XXT
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
    /** Ranging measurements OWR with AoA */
    phRangingMesrOwrAoa_t range_meas_owr_aoa[MAX_NUM_OWR_AOA_MEASURES];
#endif //(UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
} RANGING_MEAS;

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief  Union for vendor speific Information of TWR ranging and TDoA ranging.
 */
typedef union {
    /** Ranging measurements array */
#if UWBFTR_TWR
    phTwoWayRangingVsData_t twr;
#endif //UWBFTR_TWR
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    /** One Way Ranging Ntf*/
    phOneWayRangingVsData_t owr_aoa;
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
#if UWBFTR_UL_TDoA_Anchor
    /** One Way Ranging TDoA Ntf*/
    phTdoaRangingVsData_t tdoa;
#endif //UWBFTR_UL_TDoA_Anchor
} VENDORSPECIFIC_MEAS;
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT

/**
 * \brief  Structure lists out the ranging notification information.
 */
typedef struct phRangingData
{
    /** RCR Indication */
    uint8_t rcr_indication;
    /** Ranging Measurement Type */
    uint8_t ranging_measure_type;
    /** Mac addr mode indicator,
     *
     * - 0: short 2 byte,
     * - 1: extended 8 byte mode */
    uint8_t mac_addr_mode_indicator;
    /** Number of ranging measurements */
    uint8_t no_of_measurements;
#if UWBIOT_UWBD_SR040
    /* Antenna Configuration information in current ranging round */
    uint8_t antenna_info;
#endif // UWBIOT_UWBD_SR040
    /** keep track of the notifications */
    uint32_t seq_ctr;
    /** Session Handle of the ranging round */
    uint32_t sessionHandle;
    /** Current ranging interval setting in milli seconds */
    uint32_t curr_range_interval;
    /** Session Handle of Primary Session  */
    uint32_t sessionHandle_of_primary_session;
    /** Ranging measures array */
    RANGING_MEAS ranging_meas;
#if (UWBIOT_UWBD_SR1XXT_SR2XXT)
    /** Vendor specific data type */
    uint8_t vs_data_type;
    /** Vendor specific data Length*/
    uint16_t vs_length;
    /** Vendor specific data*/
    VENDORSPECIFIC_MEAS vs_data;
#endif // (UWBIOT_UWBD_SR1XXT_SR2XXT)
#if ((UWBFTR_UL_TDoA_Anchor || UWBFTR_DL_TDoA_Tag) && UWBIOT_UWBD_SR1XXT_SR2XXT)
    /** Status code for WLAN during ranging RR*/
    uint8_t wifiCoExStatusCode;
    /** Antenna Rx Configuration information used in current ranging round*/
    uint32_t antenna_pairInfo;
#endif //((UWBFTR_UL_TDoA_Anchor || UWBFTR_DL_TDoA_Tag) && UWBIOT_UWBD_SR1XXT_SR2XXT)
#if (!(UWBIOT_UWBD_SR040))
    /** Indicator for presence of Authentication Tag*/
    uint8_t authInfoPrsen;
    /**Authentication Tag*/
    uint8_t authenticationTag[AUTH_TAG_IN_16BYTES];
#endif //(!(UWBIOT_UWBD_SR040))
} phRangingData_t;

#if UWBFTR_CCC

/**
 * \brief  Structure lists out the PDoA Measurements for CCC Range Data Ntf .
 */
typedef struct phCccPdoaMeasurements
{
    /** Estimation of phase difference in degrees from antenna pair [1…N] */
    int16_t pdoa;
    /** CIR index estimate at which PDoA [1…N] has been detected */
    uint16_t pdoaIndex;
} phCccPdoaMeasurements_t;

/**
 * \brief  Structure lists out the Antenna Pair Info for CCC Range Data Ntf .
 */
typedef struct antenna_pair_info
{
    /** Antenna Rx Configuration information used in current ranging round */

    /** Configuration Mode as per ANTENNAS_CONFIGURATION_RX
     *  supported configuration modes are 0 ,1, 5 and 6
     *  0 : Configuration Mode 0: ToF Only Mode
     *  1 : Configuration Mode 1: Configuration Mode for 3D/2D AoA, test/loopback session and implicit ToA mode usecase
     *  5 : Configuration Mode 5: Configuration Mode for CSA ToA mode use case
     *  6 : Configuration Mode 6: Configuration Mode for CSA AoA mode use case
     *
    */
    uint8_t configMode;
    /**
     * Config mode(s) 0:
     *    Antenna ID 1 as per ANTENNA_RX_IDX_DEFINE
     * Config mode(s) 1:
     *    Antenna Pair ID as per ANTENNAS_RX_PAIR_DEFINE
     * Config mode(s) 5:
     *    Antenna ID as per ANTENNA_RX_IDX_DEFINE mapped to current active RR
     * Config mode(s) 6:
     *    Antenna Pair ID as per ANTENNAS_RX_PAIR_DEFINE mapped to current active RR
     */
    uint8_t antPairId1;
    /**
     * Config mode(s) 0:
     *    Antenna ID 2 as per ANTENNA_RX_IDX_DEFINE
     * Config mode(s) 1:
     *    Antenna Pair ID as per ANTENNAS_RX_PAIR_DEFINE
     * Config mode(s) 5:
     *    Antenna ID as per ANTENNA_RX_IDX_DEFINE mapped to current active RR
     * Config mode(s) 6:
     *    Antenna Pair ID as per ANTENNAS_RX_PAIR_DEFINE mapped to current active RR
     */
    uint8_t antPairId2;
    /** RFU */
    uint8_t rfu;

} phCccAntennaPairInfo_t;

/**
 * \brief  Structure lists out the RSSI Measurements for CCC Range Data Ntf .
*/
typedef struct cccRssiMeasurements
{
    /** RSSI Measurement from RX1
     * It is signed value expressed in Q8.8 format
    */
    int16_t rssi_rx1;
    /** RSSI Measurement from RX2
     * It is signed value expressed in Q8.8 format
    */
    int16_t rssi_rx2;
} phCccRssiMeasurements_t;

/**
 * \brief  Structure lists out the SNR Measurements for CCC Range Data Ntf .
*/
typedef struct cccSnrMeasurements
{
    /** Slot index and Antenna mapping details
     *  bits[5:0] - RFRAME slot index
     *  bits[7:6] for H1
     *  bit[6] - RFU
     *  bit[7] - 0: RX1, 1: RX2 for H1
     *  bits[7:6] : For H2
     *  - 0b00: RXC
     *  - 0b01: RXB
     *  - 0b10: RFU
     *  - 0b11: RXA
     */
    uint8_t slotIndexAndAntennaMap;
    /** Signal-to-Noise Ratio (SNR) of the First path and it is reported as unsigned value in dB*/
    uint8_t snrFirstPath;
    /** Signal-to-Noise Ratio (SNR) of the main path and it is reported as unsigned value in dB*/
    uint8_t snrMainPath;
    /** Signal-to-Noise Ratio (SNR) of configured RX*/
    uint16_t snrTotal;
} phCccSnrMeasurements_t;

/**
 * \brief  Structure lists out the Aliro ranging notification information.
 */
typedef struct phAliroRangingData
{
    /** Session Handle of the CCC ranging round */
    uint32_t sessionHandle;
    /** Ranging Status of the CCC Session, refer eCccRangingStatus */
    uint8_t rangingStatus;
    /** STS Index */
    uint32_t stsIndex;
    /** Ranging Round Index */
    uint16_t rangingRoundIndex;
    /** Index of the Ranging block for which the following measurement result applies */
    uint16_t blockIndex;
    /** Distance between Initiator and Anchor in cm */
    uint16_t distance;
    /** Ranging timestamp uncertainty of controllee */
    uint8_t uncertanityAnchorFom;
    /** Ranging timestamp uncertainty of controller */
    uint8_t uncertanityInitiatorFom;
    /** CCM Tag  */
    uint8_t ccmTag[MAX_CCM_TAG_SIZE];
    /** AOA Azimuth */
    int16_t aoa_azimuth;
    /** AOA Azimuth FOM */
    uint8_t aoa_azimuth_FOM;
    /** AOA Elevation */
    int16_t aoa_elevation;
    /** AOA elevation FOM */
    uint8_t aoa_elevation_FOM;
    /** Antenna Pair Info */
    phCccAntennaPairInfo_t antenna_pair_info;
    /** Number of PDoA Measurements */
    uint8_t noOfPdoaMeasures;
    /** PDoA Measurements */
    phCccPdoaMeasurements_t pdoaMeasurements[MAX_NO_OF_PDOA_MEASUREMENTS];
    /** Number of RSSI Measurements
     * This field shall be set 0 when SWAP_ANTENNA_PAIR_3D_AOA parameter is set to 0x00
    */
    uint8_t noOfRssiMeasurements;
    /** RSSI Measurements */
    phCccRssiMeasurements_t cccRssiMeasurements[MAX_NO_OF_CCC_RSSI_MEASUREMENTS];
    /** Number of SNR Measurements
     * This field shall be set 0 and SNR Measurement field will not be present.
    */
    uint8_t noOfSnrMeasurements;
    /** SNR Measurements */
    phCccSnrMeasurements_t cccSnrMeasurements[MAX_NO_OF_CCC_SNR_MEASUREMENTS];
} phAliroRangingData_t;

/**
 * \brief  enum lists out the CCC ranging notification information status.
 * For the CCC session, the lower nibble represents the ranging status of the controller and the higher nibble represents the ranging status of the controllee.
 * For other sessions, this contains only the status of the controllee.
 */
typedef enum cccRangingStatus
{
    CCC_STATUS_SUCCESS          = 0x0, /* Success */
    CCC_STATUS_TRANSACTION_OVFL = 0x1, /* Transaction overflow */
    CCC_STATUS_TRANSACTION_EXPD = 0x2, /* Transaction expired */
    CCC_STATUS_INCORRECT_FRAME  = 0x3, /* Incorrect frame */
    CCC_STATUS_RESP_LISTEN      = 0xD, /* Responder is in listen only mode (Not available for the controller) */
    CCC_STATUS_CTRL_MSG_LOST    = 0xF, /* Ranging Control Message lost (Not available for the controller) */

} eCccRangingStatus;

#endif // UWBFTR_CCC

/**
 * \brief  Structure lists out the mandatory configurations to be set for
 * ranging.
 */
typedef struct phRangingParams
{
    /** Device Role
     *
        * kUWB_DeviceRole_Responder      = 0,
        * kUWB_DeviceRole_Initiator      = 1,
        * kUWB_DeviceRole_UT_Sync_Anchor = 2,
        * kUWB_DeviceRole_UT_Anchor      = 3,
        * kUWB_DeviceRole_UT_Tag         = 4,
        * kUWB_DeviceRole_Advertiser     = 5,
        * kUWB_DeviceRole_Observer       = 6,
        * kUWB_DeviceRole_DlTDoA_Anchor  = 7,
        * kUWB_DeviceRole_DlTDoA_Tag     = 8,
     */
    uint8_t deviceRole;
    /** Multi Node Mode,
     *
     * - 0x00: Single device to Single device (Unicast),
     * - 0x01: One to Many,
     * - 0x02: Many to Many,
     * - 0x03: Reserved */
    uint8_t multiNodeMode;
    /** Device Mac Address mode 0:2 bytes,1:8 bytes mac addr with
     * 2 bytes in header, 2: 8 bytes in mac addr and header */
    uint8_t macAddrMode;
    /** Device Mac Address, 2 bytes or 8 bytes
     * addr is supported. */
    uint8_t deviceMacAddr[MAC_EXT_ADD_LEN];
    /** Device Type, 0x00: Controlee, 0x01: Controller, 0x02: Advertiser, 0x03: Observer */
    uint8_t deviceType;
    /** Ranging Round Usage */
    uint8_t rangingRoundUsage;
    /** Scheduled Mode */
    uint8_t scheduledMode;
} phRangingParams_t;

/**
 * \brief  Structure lists out the preamble pulse shape config
 */
typedef struct phTxTelecConfig
{
    /** Preamble pulse shape id */
    uint8_t shape_id;
    /** Payload tx pulse shape id */
    uint8_t payload_tx_shape_id;
    /** STS Tx pulse shape id */
    uint8_t sts_shape_id;
    /** DAC Stage config */
    uint8_t dac_stage_cofig;
} phTxPulseShapeConfig_t;

/**
 * \brief  Structure lists out the configurations to be get from
 *  Accessory Config Data.
 */
typedef struct AccessoryConfigDataContent
{
    /** Total length */
    uint8_t length;
    /** Specification Major version */
    uint8_t uwb_spec_ver_major[MAX_SPEC_VER_LEN];
    /** Specification Minor version */
    uint8_t uwb_spec_ver_minor[MAX_SPEC_VER_LEN];
    /** Manufacture id for device specific*/
    uint8_t manufacturer_id[4];
    /** Model id for device specific*/
    uint8_t model_id[4];
    /** MW  version*/
    uint8_t mw_version[4];
    /** Device Role */
    uint8_t ranging_role;
    /** Source mac address*/
    uint8_t device_mac_addr[MAC_SHORT_ADD_LEN];
    /** CLock frequency drift value */
    uint8_t clock_drift[2];
} AccessoryUwbConfigDataContent_t;

/**
 * \brief Structure lists our the configurations
 * to be loaded on the device sent from the phone counterpart
 */
typedef struct UwbPhoneConfigData
{
    uint8_t spec_ver_major[MAX_SPEC_VER_LEN];
    uint8_t spec_ver_minor[MAX_SPEC_VER_LEN];
    uint32_t session_id;
    uint8_t preamble_id;
    uint8_t channel_number;
    uint8_t config_id;
    uint8_t device_ranging_role;
    uint8_t phone_mac_address[SHORT_ADDRESS_LEN];
} UwbPhoneConfigData_t;

/**
 * \brief  Structure lists out the configurations to be get from
 *  Accessory Config Data.
 */
typedef struct UwbDeviceConfigData
{
    /** Specification Major version */
    uint8_t spec_ver_major[2];
    /** Specification Minor version */
    uint8_t spec_ver_minor[2];
    /** UWB Chip identifier */
    uint8_t chip_id[2];
    /** UWB Chip firmware version */
    uint8_t chip_fw_version[2];
    /** MW  version*/
    uint8_t mw_version[3];
    /** Range of supported profiles by the device */
    uint32_t supported_config_ids;
    /** Device Role */
    uint8_t ranging_role;
    /** Source mac address*/
    uint8_t device_mac_addr[MAC_SHORT_ADD_LEN];
} UwbDeviceConfigData_t;

/**
 * \brief  Structure lists out the mandatory configurations to be set for
 *  User Accessory Config Data.
 */
typedef struct UserAccessoryConfigData_iOS
{
    uint8_t customerSpecMajorVer[2];
    uint8_t customerSpecMinorVer[2];
    uint8_t preferedUpdateRate;
    uint8_t RFU[10];
    AccessoryUwbConfigDataContent_t UwbConfigData;
} UserAccessoryConfigData_iOS_t;
#if UWBFTR_DataTransfer
/**
 * \brief  Structure lists out the data control transmit notification.
 */
typedef struct phDataTransmit
{
    /** Session Handle */
    uint32_t transmitNtf_sessionHandle;
    /** Sequence number */
    uint16_t transmitNtf_sequence_number;
    /** Status */
    uint8_t transmitNtf_status;
    /** Tx count*/
    uint8_t transmitNtf_txcount;
} phUwbDataTransmit_t;

/**
 * \brief  Structure lists out the data credit notification.
 */
typedef struct phDataCredit
{
    /** Session Handle */
    uint32_t sessionHandle;
    /** Credit availability */
    uint8_t credit_availability;
} phUwbDataCredit_t;
#endif //UWBFTR_DataTransfer

#if UWBFTR_Radar
/**
 * \brief  Structure lists out the Radar Cir notification.
 */
typedef struct phUwbRadarcirNtf
{
    /** Num of Cirs  */
    uint16_t num_cirs;
    /** Cir Taps  */
    uint8_t cir_taps;
    /** Rfu */
    uint8_t rfu;
    /** Length of the Cir */
    uint16_t cir_len;
    /** Cir data */
    uint8_t *cirdata;
} phUwbRadarcirNtf_t;

/**
 * \brief  Structure lists out the Test Radar Antenna Isolation.
 */
typedef struct phUwbRadarTestIsoNtf
{
    /** Tx Antenna select */
    uint8_t antenna_tx;
    /** Rx Antenna select */
    uint8_t antenna_rx;
    /** Radar type */
    uint16_t anteena_isolation;
} phUwbRadarTestIsoNtf_t;

/**
 * \brief  Union for Radar and Anteena isolation measurement information.
 */
typedef union {
    /** Radar CIR notification Structure */
    phUwbRadarcirNtf_t radr_cir;
    /** Radar CIR Antenna test isolation Structure */
    phUwbRadarTestIsoNtf_t radar_tst_ntf;
} RADAR_MEAS;

/**
 * \brief  Structure lists out the Radar notificaiton.
 */
typedef struct phUwbRadarNtf
{
    /** Session Handle */
    uint32_t sessionHandle;
    /** Status of the radar*/
    uint8_t radar_status;
    /** Radar type */
    uint8_t radar_type;
    /** Radar measures */
    RADAR_MEAS radar_ntf;
} phUwbRadarNtf_t;

#endif //UWBFTR_Radar

/**
 * \brief  Enumeration lists out the chip specific type used.
 */
typedef enum
{
    kChipType_NA    = 0x00,
    kChipType_SR150 = 0x01,
    kChipType_SR040 = 0x02,
    kChipType_SR160 = 0x03
} demo_chip_type_t;

/**
 * \brief  Enumeration lists out the board specific type used.
 */
typedef enum
{
    kBoardType_NA       = 0x00,
    kBoardType_Shield   = 0x01,
    kBoardType_FinderV3 = 0x03
} demo_board_type_t;

/**
 * \brief  Enumeration lists out the Preferred specific rate type used.
 */
typedef enum
{
    kUpdateRate_Automatic       = 0,
    kUpdateRate_Infrequent      = 10,
    kUpdateRate_UserInteractive = 20
} PreferedUpdateRate_t;

#if UWBIOT_UWBD_SR040
/**
 * @brief  Enumeration lists out the Preferred Antenna used.
 *
 */
typedef enum
{
    kAntennaSecect_AntDefault = 0x00,
    kAntennaSecect_AntTop     = 0x01,
    kAntennaSecect_AntBottom  = 0x02,
} eAntennaSelect_t;
#endif // UWBIOT_UWBD_SR040

#if UWBIOT_UWBD_SR150
#define MODELID_BOARD_TYPE kBoardType_Shield
#define MODELID_CHIP_TYPE  kChipType_SR150
#endif //UWBIOT_UWBD_SR150

#if UWBIOT_UWBD_SR160
#define MODELID_BOARD_TYPE kBoardType_Shield
#define MODELID_CHIP_TYPE  kChipType_SR160
#endif //UWBIOT_UWBD_SR160

#if UWBIOT_UWBD_SR040
#define MODELID_BOARD_TYPE kChipType_SR040
#define MODELID_CHIP_TYPE  kChipType_SR040
#endif                   //UWBIOT_UWBD_SR040
#define MODELID_RFU 0x00 // RFU
#define CLOCK_DRIFT 100

/**
 * \brief  Enumeration lists out the channel numbers used.
 */
typedef enum UWB_ChannelNumber
{
    kUWB_ChannelNumber_5 = 5,
    kUWB_ChannelNumber_6 = 6,
    kUWB_ChannelNumber_8 = 8,
    kUWB_ChannelNumber_9 = 9,
} UWB_ChannelNumber_t;

/**
 * \brief  Enumeration lists out the MAC FCS type used.
 */
typedef enum UWB_MacFcsType
{
    kUWB_MacFcsType_CRC16 = 0,
    kUWB_MacFcsType_CRC32 = 1,
} UWB_MacFcsType_t;

/**
 * \brief  Enumeration lists out the SFD ID's used.
 */
typedef enum UWB_SfdId
{
    kUWB_SfdId_BPRF_0 = 0, //BPRF
    kUWB_SfdId_BPRF_2 = 2, //BPRF
    kUWB_SfdId_HPRF_1 = 1, //HPRF
    kUWB_SfdId_HPRF_2 = 2, //HPRF
    kUWB_SfdId_HPRF_3 = 3, //HPRF
} UWB_SfdId_t;

/**
 * \brief  Enumeration lists out the Preamble Index codes used.
 */
typedef enum UWB_PreambleIndxCode
{
    kUWB_PreambleIndxCode_BPRF_09 = 9, // [9.. 12] BPRF
    kUWB_PreambleIndxCode_BPRF_10 = 10,
    kUWB_PreambleIndxCode_BPRF_11 = 11,
    kUWB_PreambleIndxCode_BPRF_12 = 12,
    kUWB_PreambleIndxCode_HPRF_25 = 25, // [25.. 32] HPRF
    kUWB_PreambleIndxCode_HPRF_26 = 26,
    kUWB_PreambleIndxCode_HPRF_27 = 27,
    kUWB_PreambleIndxCode_HPRF_28 = 28,
    kUWB_PreambleIndxCode_HPRF_29 = 29,
    kUWB_PreambleIndxCode_HPRF_30 = 30,
    kUWB_PreambleIndxCode_HPRF_31 = 31,
    kUWB_PreambleIndxCode_HPRF_32 = 32,
} UWB_PreambleIndxCode_t;

/**
 * \brief  Enumeration lists out the Preamble duration used.
 */
typedef enum UWB_PreambleDuration
{
    kUWB_PreambleDuration_32Symbols = 0, //Both BPRF and HPRF
    kUWB_PreambleDuration_64Symbols = 1, //BPRF Only
} UWB_PreambleDuration_t;

/**
 * \brief  Enumeration lists out the Ranging Data Notifications used.
 */
typedef enum UWB_Session_InfoNtf
{
    kUWB_DisableSession_Info_Ntf                           = 0x00,
    kUWB_EnableSession_Info_Ntf                            = 0x01,
    kUWB_SessionInfo_Ntf_Proximity                         = 0x02,
    kUWB_SessionInfo_Ntf_AOABounds                         = 0x03,
    kUWB_SessionInfo_Ntf_AOABounds_n_Proximity             = 0x04,
    kUWB_SessionInfo_Ntf_Enter_Leave_Proximity             = 0x05,
    kUWB_SessionInfo_Ntf_Enter_Leave_AOABounds             = 0x06,
    kUWB_SessionInfo_Ntf_Enter_Leave_AOABounds_n_Proximity = 0x07,

} UWB_Session_InfoNtf_t;

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
/**
 * \brief Enumeration lists out the CoEx Interface to be selected.
*/
typedef enum UWB_CoEx_InterfaceSelection
{
    /** UWB_WiFiCoEx_AdvGrantDuration */
    kUWB_CoEx_Gpio_Interface = 0x00,
    /** Set CoEx Interface as Uart Interface */
    kUWB_CoEx_Uart_Interface = 0x10,
    /** Set CoEx Interface as One Wire Interface. */
    kUWB_CoEx_OneWire_Interface = 0x20,

} UWB_CoEx_InterfaceSelection_t;

/**
 * \brief Enumeration lists out the configurations for the WiFi CoEx feature Notifications.
*/
typedef enum UWB_CoEx_NtfSelection
{
    /** Disable Wifi CoEx Feature (default) */
    kUWB_CoEx_Disable = 0x00,
    /** Enable CoEx Interface without Debug and without Warning Verbose. */
    kUWB_CoEx_En_WoDebug_WoWarning = 0x01,
    /** Enable CoEx Interface with Debug Verbose only */
    kUWB_CoEx_En_Debug = 0x02,
    /** Enable CoEx Interface with Warnings Verbose only */
    kUWB_CoEx_En_Warning = 0x03,
    /** Enable CoEx Interface with both Debug and Warning Verbose */
    kUWB_CoEx_En_Debug_Warning = 0x04,

} UWB_CoEx_NtfSelection_t;

/**
 * \brief Enumeration lists out the configurations for the WiFi CoEx feature.
*/
typedef enum UWB_CoEx_ChannelCfg
{
    /** Enable Wifi CoEx on Channel 5 */
    kUWB_CoEx_CH5 = 0x01,
    /** Enable Wifi CoEx on Channel 6 */
    kUWB_CoEx_CH6 = 0x02,
    /** Enable Wifi CoEx on Channel 8 */
    kUWB_CoEx_CH8 = 0x04,
    /** Enable Wifi CoEx on Channel 9 */
    kUWB_CoEx_CH9 = 0x08,
    /** Enable Wifi CoEx for all channels i.e ch5,ch6,ch8 and ch9 */
    kUWB_CoEx_AllCH = 0x0F,

} UWB_CoEx_ChannelCfg_t;
/**
 * \brief Structure for storing WiFiCoEx IND Ntf Context.
 */
typedef struct UWB_CoEx_IndNtf
{
    /** UWB_WLAN_IND Status */
    uint8_t status;
    /** Slot Index where the GPIO change occured */
    uint32_t slot_index;
    /** Session Handle to which UWB_WIFI_COEX_IND_NTF belongs */
    uint32_t sessionHandle;

} UWB_CoEx_IndNtf_t;

#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
/**
 * \brief  Enumeration lists out the PRF modes used.
 */
typedef enum UWB_PrfMode
{
    /** BPRF */
    kUWB_PrfMode_62_4MHz = 0,
    /** HPRF */
    kUWB_PrfMode_124_8MHz = 1,
    /** HPRF mode with data rate 27.2 and 31.2 Mbps */
    kUWB_PrfMode_249_6MHz = 2
} UWB_PrfMode_t;

/**
 * \brief  Enumeration lists out the PSDU data rates used.
 */
typedef enum UWB_PsduDataRate
{
    kUWB_PsduDataRate_6_81Mbps = 0,
    kUWB_PsduDataRate_7_80Mbps = 1,
    kUWB_PsduDataRate_27_2Mbps = 2,
    kUWB_PsduDataRate_31_2Mbps = 3,
    kUWB_PsduDataRate_850Kbps  = 4
} UWB_PsduDataRate_t;

/**
 * \brief  Enumeration lists out the RFrame Config sts values.
 */
typedef enum UWB_RfFrameConfig
{
    kUWB_RfFrameConfig_No_Sts               = 0,
    kUWB_RfFrameConfig_SP0                  = kUWB_RfFrameConfig_No_Sts,
    kUWB_RfFrameConfig_Sfd_Sts              = 1,
    kUWB_RfFrameConfig_SP1                  = kUWB_RfFrameConfig_Sfd_Sts,
    kUWB_RfFrameConfig_Psdu_Sts             = 2,
    kUWB_RfFrameConfig_Sfd_Sts_NoPhr_NoPsdu = 3,
    kUWB_RfFrameConfig_SP3                  = kUWB_RfFrameConfig_Sfd_Sts_NoPhr_NoPsdu,
} UWB_RfFrameConfig_t;

/**
 * \brief  Enumeration lists out the Device role values.
 */
typedef enum
{
    kUWB_DeviceRole_Responder      = 0,
    kUWB_DeviceRole_Initiator      = 1,
    kUWB_DeviceRole_UT_Sync_Anchor = 2,
    kUWB_DeviceRole_UT_Anchor      = 3,
    kUWB_DeviceRole_UT_Tag         = 4,
    kUWB_DeviceRole_Advertiser     = 5,
    kUWB_DeviceRole_Observer       = 6,
    kUWB_DeviceRole_DlTDoA_Anchor  = 7,
    kUWB_DeviceRole_DlTDoA_Tag     = 8,
} UWB_DeviceRole_t;

/**
 * \brief  Enumeration lists out the Multicast mode values.
 */
typedef enum UWB_MultiNodeMode
{
    kUWB_MultiNodeMode_UniCast    = 0,
    kUWB_MultiNodeMode_OnetoMany  = 1,
    kUWB_MultiNodeMode_ManytoMany = 2,
} UWB_MultiNodeMode_t;

/**
 * \brief  Enumeration lists out the Device type values.
 */
typedef enum UWB_DeviceType
{
    kUWB_DeviceType_Controlee  = 0,
    kUWB_DeviceType_Controller = 1,
} UWB_DeviceType_t;

/**
* \brief  Enumeration lists out the Ranging Round Usage values.
*/
typedef enum UWB_RangingRoundUsage
{
    /* One Way Ranging UL-TDoA */
    kUWB_RangingRoundUsage_TDoA = 0,
    /* SS-TWR with Deferred Mode */
    kUWB_RangingRoundUsage_SS_TWR = 1,
    /* DS-TWR with Deferred Mode */
    kUWB_RangingRoundUsage_DS_TWR = 2,
    /* SS-TWR with Non-deferred Mode*/
    kUWB_RangingRoundUsage_SS_TWR_nd = 3,
    /* Double Sided TWR Non Deferred*/
    kUWB_RangingRoundUsage_DS_TWR_nd = 4,
    /* One Way Ranging DL-TDOA*/
    kUWB_RangingRoundUsage_DL_TDOA = 5,
    /* OWR for AoA Measurement*/
    kUWB_RangingRoundUsage_OWR_AOA = 6,
    /* eSS-TWR with Non-deferred Mode for Contention-based ranging*/
    kUWB_RangingRoundUsage_eSS_TWR = 7,
    /* aDS-TWR for Contention-based ranging*/
    kUWB_RangingRoundUsage_aDS_TWR = 8,
    /* Data transfer mode*/
    kUWB_RangingRoundUsage_DTx = 9,

} UWB_RangingRoundUsage_t;

typedef UWB_RangingRoundUsage_t UWB_RangingMethod_t;

#define kUWB_RangingMethod_TDoA      kUWB_RangingRoundUsage_TDoA
#define kUWB_RangingMethod_SS_TWR    kUWB_RangingRoundUsage_SS_TWR
#define kUWB_RangingMethod_DS_TWR    kUWB_RangingRoundUsage_DS_TWR
#define kUWB_RangingMethod_SS_TWR_ND kUWB_RangingRoundUsage_SS_TWR_nd
#define kUWB_RangingMethod_DS_TWR_ND kUWB_RangingRoundUsage_DS_TWR_nd

/***
 *  \brief Enumeration lists out the AOA_RESULT_REQ values.
 */
typedef enum UWB_AOA_Result_Req
{
    kUWB_AOA_Result_Req_Disable   = 0,
    kUWB_AOA_Result_Req_Enable    = 1,
    kUWB_AOA_Result_Req_Azimuth   = 2,
    kUWB_AOA_Result_Req_Elevation = 3,

} UWB_AOA_Result_Req_t;

/**
 * \brief  Enumeration lists out the STS Config values.
 */
typedef enum UWB_StsConfig
{
    kUWB_StsConfig_StaticSts               = 0,
    kUWB_StsConfig_DynamicSts              = 1,
    kUWB_StsConfig_DynamicSts_Ctrlee_key   = 2,
    kUWB_StsConfig_ProvisionSts            = 3,
    kUWB_StsConfig_ProvisionSts_Ctrlee_key = 4,

} UWB_StsConfig_t;

/**
 * \brief  Enumeration lists out the MAC address mode.
 */
typedef enum
{
    kUWB_MacAddressMode_2bytes = 0,
    kUWB_MacAddressMode_8bytes = 2,
} UWB_MacAddressMode_t;

/**
 * \brief  Enumeration lists out the scheduled Mode values.
 */
typedef enum UWB_ScheduledMode
{
    /** Contention based Ranging Scheduling */
    kUWB_ScheduledMode_ContentionBased = 0,
    /** Time based Ranging Scheduling */
    kUWB_ScheduledMode_TimeScheduled = 1,
    /** Hybrid based Scheduling */
    kUWB_ScheduledMode_HybridBased = 2,
} UWB_ScheduledMode_t;

/**
 * \brief  Enumeration lists out the Radar Mode values.
 *
 * 0x01: Medium distance, e.g. used for vital sign detection;
 * 0x02: Close distance, e.g. used for hand gesture recognition
 * 0x03: Far distance, e.g. used for presence detection
 * 0x04 - RFU
 * 0x05: Medium distance,   e.g. used for static object support
 * 0x06: Far distance, e.g. used for static object support
 */
typedef enum UWB_RadarMode
{
    kUWB_RadarMode_Medium_Distance        = 1,
    kUWB_RadarMode_Close_Distance         = 2,
    kUWB_RadarMode_Far_Distance           = 3,
    kUWB_RadarMode_Static_Medium_Distance = 5,
    kUWB_RadarMode_Static_Far_Distance    = 6,
    kUWB_RadarMode_Test_Isolation         = 0x20
} UWB_RadarMode_t;

/**
 * \brief  Enumeration lists out the Phase Participation values of Controller.
 *
 * 0x00 = No participation in the phase
 * 0x01 = Participate in the phase as a device role configured by DEVICE_ROLE
 * 0x02 = The UWBS shall transmit DTPCM in this phase
 * 0x03 = The UWBS shall Receive DTPCM in this phase
 * Note : 0x02 and 0x03 are applicable only for Data Transfer Phase.
 */
typedef enum UWB_phaseParticipationCtrl
{
    kUWB_CtrlNoParticipation         = 0x00,
    kUWB_CtrlDeviceRoleParticipation = 0x01,
    kUWB_CtrlDtpcmTxParticipation    = 0x02,
    kUWB_CtrlDtpcmRxParticipation    = 0x03,
} UWB_phaseParticipationCtrl_t;

/**
 * \brief  Enumeration lists out the Phase Participation values of Controlee.
 *
 * 0x00 = No participation in the phase.
 * 0x01 = Participate as a Responder Role if MAC Address of this phase is not present in the RMM.
 * 0x02 = Participate in the phase as a device role configured by DEVICE_ROLE.
 */
typedef enum UWB_phaseParticipationClee
{
    kUWB_CleeNoParticipation            = 0x00,
    kUWB_CleeResponderRoleParticipation = 0x01,
    kUWB_CleeDeviceRoleParticipation    = 0x02,
} UWB_phaseParticipationClee_t;

/**
 * \brief Structure for storing List of Phases of Controller.
 */
typedef struct phCtlrPhaseList
{
    /** Secondary Session Handle */
    uint32_t phase_sessionHandle;
    /** Start Slot Index */
    uint16_t start_slot_index;
    /** End Slot Index */
    uint16_t end_slot_index;
    /** Phase Participation to indicate whether the
     * device shall participate or not in the phase
     */
    uint8_t phase_participation;
    /** MAC address of the participating device in the current phase */
    uint8_t mac_addr[MAC_EXT_ADD_LEN];

} phCtlrPhaseList_t;

/**
 * \brief Structure for storing Controller HUS Session Configurations.
 */
typedef struct phControllerHusSessionConfig
{
    /** Primary Session Handle */
    uint32_t sessionHandle;

    /** Number of Phases */
    uint8_t phase_count;

    /** Update Time */
    uint8_t update_time[HUS_CONFIG_UPDATE_TIME_OCTETS];

    /** Phase List */
    phCtlrPhaseList_t phase_list[MAX_PHASE_COUNT];

} phControllerHusSessionConfig_t;

/**
 * \brief Structure for storing List of Phases of Controlee.
 */
typedef struct phCleePhaseList
{
    /** Secondary Session Handle */
    uint32_t phase_sessionHandle;
    /** Phase Participation to indicate whether the
     * device shall participate or not in the phase
     */
    uint8_t phase_participation;

} phCleePhaseList_t;

/**
 * \brief Structure for storing Controlee HUS Session Configurations.
 */
typedef struct phControleeHusSessionConfig
{
    /** Primary Session Handle */
    uint32_t sessionHandle;

    /** Number of Phases */
    uint8_t phase_count;

    /** Phase List */
    phCleePhaseList_t phase_list[MAX_PHASE_COUNT];

} phControleeHusSessionConfig_t;

/** Set the Slot Bitmap
 * [b3-b1]
 * 0 = 8 ranging slots
 * 1 = 16 ranging slots
 * 2 = 32 ranging slots
 * 3 = 64 ranging slots
 * 4 = 128 ranging slots
 * 5 = 256 ranging slots
 * 6-7 = RFU
 */
typedef enum phSlotBitmap
{
    ranging_slots_8   = 0x00,
    ranging_slots_16  = 0x02,
    ranging_slots_32  = 0x04,
    ranging_slots_64  = 0x06,
    ranging_slots_128 = 0x08,
    ranging_slots_256 = 0x0A,
    /* RFU */
} phSlotBitmap_t;

/**
 * \brief Structure to store Data Transfer Phase Management List.
 */
typedef struct phDataTxPhaseMngList
{
    /** MAC address for which Data Tx slots are configured  */
    uint8_t mac_addr[MAC_EXT_ADD_LEN];
    /** Slot Bitmap */
    uint8_t slot_bitmap[MAX_SLOT_BITMAP_SIZE];
} phDataTxPhaseMngList_t;

/**
 * \brief structure to store Data Transfer Phase Configurations.
 */
typedef struct phDataTxPhaseConfig
{
    /** Session Handle to which Data Transfer phase to be configured */
    uint32_t dtpcm_SessionHandle;

    /** Data Transfer Phase Control Message Repetition */
    uint8_t dtpcm_Repetition;

    /** Data Transfer Control */
    uint8_t dataTransferCtrl;

    /** Data Transfer Phase Management List Size */
    uint8_t dtpml_size;

    /** Data Transfer Phase Management List */
    phDataTxPhaseMngList_t dtpml[MAX_PHASE_COUNT];

} phDataTxPhaseConfig_t;

/**
 * \brief Structure to store Data Transfer Phase Config notification values.
 */
typedef struct phDataTxPhaseCfgNtf
{
    /** Session Handle to which the DataTx phase is configured */
    uint32_t sessionHandle;
    /** Data Tx phase Status */
    uint8_t status;
} phDataTxPhaseCfgNtf_t;

/**
 * \brief  Generic callback function registered by Application to receive data
 * from Rhodes SDK. This is a common callback for All session types
 *             1. Ranging
 *             2. App Data Transfer
 *             3. PER Exchange
 *             4. Generic Error Notification
 *             5. Device Reset Notification
 *             6. RFrame Data Notification
 *             7. Recovery Notification
 *             8. Scheduler status Notification
 *
 *             This data is provided by the stack, to the user.  The user should
 *             implement this callback if they want to receive the NTF
 *             information.
 *
 *             - If the callback function pointer
 *             is NULL, the user will not receive the notifications.
 *
 * \param opType  Type of Notification
 * \param pData   Data received
 */
typedef void(tUwbApi_AppCallback)(eNotificationType opType, void *pData);

/**
 * \brief  Generic callback function registered by Application to receive data packet
 * from Rhodes SDK.
 * If the callback function pointer is NULL, the user will not receive the data packet.
 *
 * \param recvData       pointer to received data
 * \param recvDataLen    len of data received
 */
typedef void(tUwbApi_AppDataCallback)(uint8_t *recvData, uint16_t recvDataLen);

/**
 * \brief  Structure lists out the UWB-IoT init context
 */
typedef struct phUwbappContext
{
    /** Standalone mode callback function */
    tUwbApi_AppCallback *pCallback;
    /** Application specific callback function*/
    tUwbApi_AppDataCallback *pMcttCallback;
    /** CDC mode callback function */
    tUwbApi_AppCallback *pCdcCallback;
    /** FW Image context */
    phUwbFWImageContext_t fwImageCtx;
    /** SE communication handle
     * Can be set to Null if SE is not present */
    void *seHandle;
} phUwbappContext_t;

/**
 * \brief  Structure lists out the Generic Error notification
 */
typedef struct phGenericError
{
    /** Status */
    uint8_t status;
} phGenericError_t;

/**
 * \brief  Structure lists out session information.
 */
typedef struct phUwbSessionInfo
{
    /** Session Handle */
    uint32_t sessionHandle;
    /** Session state */
    uint8_t state;
    /** Reason code */
    uint8_t reason_code;
} phUwbSessionInfo_t;

/**
 * \brief Enumeration to list out the Actions of the Controller's Multicast List.
 */
typedef enum multicastControllerActions
{
    kUWB_AddControlee        = 0x00,
    kUWB_DelControlee        = 0x01,
    KUWB_Add16BSubSessionKey = 0x02,
    KUWB_Add32BSubSessionKey = 0x03,
} eMulticastControllerActions_t;

/**
 * \brief Structure List out Controlee info.
 */
typedef struct phControleeList
{
    /** Short address*/
    uint16_t short_address;
    /** Sub Session Handle */
    uint32_t subsession_id;
    /**Controlee specific Sub-session Key 16/32 Bytes*/
    uint8_t subsession_key[MAX_SUB_SESSION_KEY_LEN];

} phControleeList_t;

/**
 * \brief Structure for storing Multicast Controlee List Context.
 */
typedef struct phMulticastControleeListContext
{
    uint32_t sessionHandle;
    /** Action */
    uint8_t action;
    /** Controlee Count */
    uint8_t no_of_controlees;
    /** Controlee List */
    phControleeList_t controlee_list[MAX_NUM_PHYSICAL_ACCESS_CONTROLEES];

} phMulticastControleeListContext_t;

/**
 * \brief Structure for storing Multicast Controlee Status List Context.
 */
typedef struct phMulticastControleeStatusList
{
#if UWBIOT_UWBD_SR1XXT
    uint16_t controlee_mac_address;
#endif
    uint32_t subsession_id;
    uint8_t status;
} phMulticastControleeStatusList_t;

/**
 * \brief Structure for storing Multicast Controlee List Ntf Context.
 */
typedef struct phMulticastControleeListNtfContext
{
    uint32_t sessionHandle;
    uint8_t remaining_list;
    uint8_t no_of_controlees;
    phMulticastControleeStatusList_t controleeStatusList[MAX_NUM_PHYSICAL_ACCESS_CONTROLEES];
} phMulticastControleeListNtfContext_t;


/**
 * \brief Structure for storing Blink Data Tx Ntf Context.
 */
typedef struct phBlinkDataTxNtfContext
{
    /** Repetition Count Status */
    uint8_t repetition_count_status;
} phBlinkDataTxNtfContext_t;
#if (UWBFTR_DataTransfer)
/**
 * \brief Structure for SendData.
 */
typedef struct phUwbDataPkt
{
    /** Session Handle */
    uint32_t sessionHandle;
    /** MAC Address */
    uint8_t mac_address[MAC_EXT_ADD_LEN];
    /** Sequence Number */
    uint16_t sequence_number;
    /** Data Size */
    uint16_t data_size;
    /** Application Data */
    uint8_t *data;
} phUwbDataPkt_t;
#endif //(UWBFTR_DataTransfer)

#if UWBFTR_DataTransfer
/**
 * \brief Structure for RcvData.
 */
typedef struct phUwbRcvDataPkt
{
    /** Session Handle */
    uint32_t sessionHandle;
    /** Status */
    uint8_t status;
    /** MAC Address */
    uint8_t src_address[MAC_EXT_ADD_LEN];
    /** Sequence Number */
    uint16_t sequence_number;
    /** Data Size */
    uint16_t data_size;
    /** Application Data */
    uint8_t data[MAX_RESPONSE_DATA_RCV];
} phUwbRcvDataPkt_t;
#endif //UWBFTR_DataTransfer

/**
 * \brief Structure for storing Data Control Transmit Ntf Context.
 */
typedef struct phDataControlTransmitNtfContext
{
    /** Session Handle */
    uint32_t sessionHandle;
    /** Status */
    uint8_t status;

} phDataControlTransmitNtfContext_t;

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR160)
/**
 * \brief  Enumeration lists out the Vendor specific Profile ID
 */
typedef enum UWB_ProfileID
{
    kUWB_Profile_1 = 0x0B,

} eUWB_ProfileID_t;

/**
 * \brief Structure lists out Profile information.
 */
typedef struct phUwbProfileInfo
{
    /** Session Handle is out param */
    uint32_t sessionHandle;
    /** Source mac address*/
    uint8_t mac_addr[MAC_SHORT_ADD_LEN];
    /** Profile ID */
    eUWB_ProfileID_t profileId;
    /** Device Role
     *
     * - 0x00: Responder
     * - 0x01: Initiator
     * - 0x02: Master Anchor */
    uint8_t deviceRole;
    /** Device Type, 0x00: Controlee, 0x01: Controller */
    uint8_t deviceType;

} phUwbProfileInfo_t;
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR160)

/**
 * \brief  Structure lists out the UWB Device Info Parameters.
 *
 */
typedef struct phUwbCapInfo
{
    /** FIRA phy lower range major version */
    uint8_t firaPhyLowerRangeMajorVersion;
    /** FIRA phy lower range minor maintenance version */
    uint8_t firaPhyLowerRangeMinorMaintenanceVersion;
    /** FIRA phy higher range major version */
    uint8_t firaPhyHigherRangeMajorVersion;
    /** FIRA phy higher range minor maintenance version */
    uint8_t firaPhyHigherRangeMinorMaintenanceVersion;
    /** FIRA mac lower range major version */
    uint8_t firaMacLowerRangeMajorVersion;
    /** FIRA mac lower range minor maintenance version */
    uint8_t firaMacLowerRangeMinorMaintenanceVersion;
    /** FIRA mac higher range major version */
    uint8_t firaMacHigherRangeMajorVersion;
    /** FIRA mac higher range minor maintenance version */
    uint8_t firaMacHigherRangeMinorMaintenanceVersion;
    /** Device types */
    uint8_t deviceTypes;
    /** Device roles */
    uint16_t deviceRoles;
    /** Ranging method */
    uint16_t rangingMethod;
    /** STS config */
    uint8_t stsConfig;
    /** Multinode mode */
    uint8_t multiNodeMode;
    /** Ranging time struct */
    uint8_t rangingTimeStruct;
    /** Scheduled mode */
    uint8_t scheduledMode;
    /** Hopping mode */
    uint8_t hoppingMode;
    /** Block striding */
    uint8_t blockStriding;
    /** UWB initiation time */
    uint8_t uwbInitiationTime;
    /** FIRA phy ver range */
    uint8_t channels;
    /** RFRAME config */
    uint8_t rframeConfig;
    /** CC constraint length */
    uint8_t ccConstraintLength;
    /** BPRF parameter sets */
    uint8_t bprfParameterSets;
    /** HPRF parameter sets */
    uint8_t hprfParameterSets[DEVICE_CAPABILITY_LEN_5];
    /** AOA support */
    uint8_t aoaSupport;
    /** Extended mac address */
    uint8_t extendedMacAddress;
    /* Suspend Ranging */
    uint8_t suspendRanging;
    /* Session Key Len */
    uint8_t sessionKeyLen;
    /* Achor Max Active RR */
    uint8_t ancorMaxRrActive;
    /* TAG Max Active RR */
    uint8_t tagMaxRrActive;
    /** Max message size */
    uint16_t maxMessageSize;
    /** Max data packet payload size */
    uint16_t maxDataPacketPayloadSize;
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    /** Slot bitmask */
    uint8_t slotBitmask;
    /** Sync code index bitmask */
    uint8_t syncCodeIndexBitmask[DEVICE_CAPABILITY_LEN_4];
    /** Hopping config bitmask */
    uint8_t hoppingConfigBitmask;
    /** Channel bitmask */
    uint8_t channelBitmask;
    /** Supported protocol version */
    uint16_t supportedProtocolVersion;
    /** Supported UWB config ID */
    uint16_t supportedUWBConfigID;
    /** Supported pulse shape combo */
    uint8_t supportedPulseShapeCombo[DEVICE_CAPABILITY_LEN_9];
    /** Maximum UCI payload size that can handle by UWBS */
    uint16_t maxUciPayloadLength;
    /** Data buffer block size in bytes which the UWBS manages
     *  for the overall in-band data transfer memory pool.
     */
    uint8_t inbandDataBlockSize;
    /** Parameter to indicate the number of blocks available
     *  in the overall in-band data transfer memory pool.
     */
    uint8_t inbandDataMaxBlock;
    /* Indicates the block skipping capability for DT-Tag */
    uint8_t tagBlockSkipping;
    /* Indicates the supported PSDU Length */
    uint8_t psduLengthSupport;
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
} phUwbCapInfo_t;

#if UWBFTR_UWBS_DEBUG_Dump
/**
 * \brief  Structure lists out the RFRAME Log Notification Parameters.
 *
 */
typedef struct phUwbRframeLogNtf
{
    uint8_t mapping;
    uint8_t decodeStatus;
    uint8_t nLos;
    uint16_t firstPathIndex;
    uint16_t mainpathIndex;
    uint8_t snrMainPath;
    uint8_t snrFirstPath;
    uint16_t snrTotal;
    uint16_t rssi;
    uint32_t cirMainPwr;
    uint32_t cirFirstPathPwr;
    uint16_t noiseVariance;
    uint16_t cfo;
    uint16_t aoaPhase;
    uint8_t cirSamples[64];

} phUwbRframeLogNtf_t;
#endif // UWBFTR_UWBS_DEBUG_Dump

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief   Structure lists out the Query Data Size Parameters.
 *
 */
typedef struct phUwbQueryDataSize
{
    /* In param to fetch Session Handle */
    uint32_t sessionHandle;
    /* out param to catch Max Data Size in a single ranging round received in response */
    uint16_t dataSize;
} phUwbQueryDataSize_t;
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

/** @} */ /* @addtogroup Uwb_Types */

#ifndef UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT
#error UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT must be defined by uwb_board.h
#endif //UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT

/** TODO: These definitions are added for backward compatibility, To be removed once VnV adapts*/
#define DUAL_AOA_PREAMBLE_STS    (eAppConfig)0xE30A
#define MAX_CONTENTION_PHASE_LEN (eAppConfig)0xE311

/* TODO: These are added teporarily to make the builds to pass, these are to be removed when RADAR related AppConfigs are taken */
#if UWBFTR_Radar
/**
     * Parameter to test FCC 10 Secs Rule
     * Default: 0(Test Mode Off)
     *          1(Test Mode On)
     */
#define RADAR_FCC_TEST_MODE (eAppConfig)0xE480
/**
    * This sets the RADAR mode which should
    * Modes:
    * 0x00: user defined (not available on
    * product release version, only internally)
    * 0x01: Medium distance, e.g. used for vital sign detection
    * 0x02: Close distance, e.g. used for hand gesture recognition
    * 0x03: Far distance, e.g. used for presence detection
    * 0x04: legacy LPRF mode
    * 0x05: legacy BPRF mode
    * default = 0x01
    **/
#define RADAR_MODE (eAppConfig)0xE500
/**
    * This maps to TX_PARAMS_PRMBL register with following fields:
    * bits[12:10] - Log2 of the sync symbol upsampling factor (delta_L)
    * bits[9:0] - Number of symbols in the sync divided by 8 Overwrites PREAMBLE_DURATION
    * default = 0x810
    **/
#define RADAR_PRF_CFG (eAppConfig)0xE501
/**
    * This maps to RX_ADC_CTRL register with following fields:
    * bits[22:20] - RX2 pulse polarity delay in units of ~8 ns
    * bits[18:16] - RX1 pulse polarity delay in units of ~8 ns
    * bits[10:8] - RX2 ADC sample delay in ns
    * bits[2:0] - RX1 ADC sample delay in ns
    * default = 0
    **/
#define RADAR_RX_DELAY (eAppConfig)0xE502
/**
     * Define the RX gain in radar mode
     *   bits[31:24]:
     *     0: enable AGC,
     *     1: force gain index values,
     *     2: force AGC to be re-executed
     *   bits[23:16] - Gain index for RXA
     *   bits[15:8] - Gain index for RXB
     *   bits[7:0] - Gain index for RXC
     * default: 0x00000000
     **/
#define RADAR_RX_GAIN (eAppConfig)0xE504
/**
     * Defines the mode used to transmit CIR samples.
     * 0x00: use a buffer to transmit the CIR samples if the buffer is full or session switch will be performed.
     * 0x01: transmit every single CIR sample
     * default = 0
     **/
#define RADAR_SINGLE_FRAME_NTF (eAppConfig)0xE505
/**
     * Capture offset in 128-TAP CIR
     * (default = 0)
     **/
#define RADAR_CIR_START_INDEX (eAppConfig)0xE506
/**
      * Number of TAPs to be captured from RADAR_CIR_START_INDEX
      * default = 128, min. = 16, max.= 128
      */
#define RADAR_CIR_NUM_SAMPLES (eAppConfig)0xE507
/**
     * Defines the start index of the CIR in case of
     * Radar modes using BPRF/HPRF
     * bits[31:16] start index for RX2
     * bits[15:0] start index for RX1
     * default = 0
     **/
#define RADAR_CIR_START_OFFSET (eAppConfig)0xE508
/**
     * Flag which to set Calibration support in the
     * current Radar Session
     * 0x00 (Disabled)
     * Default: 0x1 (Enabled)
     */
#define RADAR_CALIBRATION_SUPPORT (eAppConfig)0xE509
/**
     * Number of Samples (LPRF+HPRF) which will be used for calibration
     * default: 32
     */
#define RADAR_CALIBRATION_NUM_SAMPLES (eAppConfig)0xE50A
/**
     * The interval at which Calibration frames will be sent. The interval is defined in terms of Radar Rounds
     * Default: 0
     */
#define RADAR_CALIBRATION_INTERVAL (eAppConfig)0xE50B

#endif //UWBFTR_Radar

/* TODO: status code for Duplicate session no longer exists, adding to support backward compatibility */
#define UWBAPI_STATUS_SESSION_DUPLICATE 0x12

/* TODO: End of Backward Compatibility */

#endif //UWBAPI_TYPES_H
