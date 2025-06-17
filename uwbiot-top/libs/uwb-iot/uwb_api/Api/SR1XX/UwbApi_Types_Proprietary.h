/*
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

#ifndef UWBAPI_TYPES_PROPRIETARY_SR1XX_H
#define UWBAPI_TYPES_PROPRIETARY_SR1XX_H

#include "UwbApi_Types.h"
#include "phUwb_BuildConfig.h"

/** \addtogroup uwb_apis_sr1xx
 *
 * APIs for SR100 and SR150
 *
 * @{ */

/* DPD Timeout Range */
/**  Minimum timeout Supported by UWBS */
#define UWBD_DPD_TIMEOUT_MIN 100
/**  Maximum timeout Supported by UWBS */
#define UWBD_DPD_TIMEOUT_MAX 5000
/**  Maximum length of version Supported by UWBS */
#define UWBD_VERSION_LENGTH_MAX 0x3
/* UWB Generate Tag command related length fields */
#define UWB_TAG_CMAC_LENGTH 0x10U
/**  UWBS MAX UCI packet size */
#define UWBS_MAX_UCI_PACKET_SIZE 2048
/**  HOST MAX UCI packet size */
#define HOST_MAX_UCI_PACKET_SIZE 2048
/**  MIN DEVICE packet size */
#define MIN_DEVICE_PACKET_SIZE 255
/**  EXTENDED PARAM ID MASK */
#define EXTENDED_PARAM_ID_MASK 0xF0
/** CCC PARAM ID */
#define CCC_INFO_ID 0xA0
/** MAX UCI CCC Version Length */
#define MAX_UCI_CCC_VERSION_LEN 2
/** MAX CCC Version Length */
#define MAX_CCC_VERSION_LEN 8
/**  MODULE MAKER ID MAX SIZE */
#define MODULE_MAKER_ID_MAX_SIZE    2
#define MODULE_MAKER_ID_MAX_SIZE_FW 8

/** MAX UWB CHID ID Length */
#define MAX_UWB_CHIP_ID_LEN 16
/** MAX UWBS PPM VALUE Length */
#define MAX_PPM_VALUE_LEN 1
/** MAX TX Power Lenght*/
#define MAX_TX_POWER_LEN 2
/** MAX FW BOOT Length*/
#define FW_BOOT_MODE_LEN 1

/* Session ID (4 octets) || Random (12 octets) || Wrapped RDS from SE (Maximum 40 octets) */
#define RANDOM_KEY_LEN  12
#define SESSION_KEY_LEN 40
#define WRAPPED_RDS_LEN (SESSION_ID_LEN + RANDOM_KEY_LEN + SESSION_KEY_LEN)
/** Max RDS List Size */
#define MAX_RDS_LIST_SIZE 5

#define VCO_PLL_POS                   0x0001
#define TX_POWER_POS                  0x0002
#define XTAL_CAP_VALUES_POS           0x0004
#define RSSI_CONSTANT1_POS            0x0008
#define RSSI_CONSTANT2_POS            0x0010
#define TX_POWER_PARAMS_POS           0x0040
#define PAPPPA_CALIB_CTRL_POS         0x0100
#define TX_TEMP_COMP_POS              0x0200
#define DELAY_CALIB_POS               0x0B00
#define PDOA_MFG_OFFSET_POS           0x1000
#define PDOA_AOA_ANT_MULTIPOINT_CALIB 0x2000

/**
 * \brief antenna selection gpio bit mask
 *
 */

/** Antenna selection gpio bit mask for EF1 */
#define UWB_ANTENNA_SELECTION_GPIO_BIT_EF1_MASK 1
/** Antenna selection gpio bit mask for EF2 */
#define UWB_ANTENNA_SELECTION_GPIO_BIT_EF2_MASK 2
/** Antenna selection gpio bit mask for GPIO6 */
#define UWB_ANTENNA_SELECTION_GPIO_BIT_GPIO6_MASK 4
/** Antenna selection gpio bit mask for GPIO7 */
#define UWB_ANTENNA_SELECTION_GPIO_BIT_GPIO7_MASK 8
/** Antenna selection gpio bit mask for GPIO9 */
#define UWB_ANTENNA_SELECTION_GPIO_BIT_GPIO9_MASK 16
/** Antenna selection gpio bit mask for GPIO10 */
#define UWB_ANTENNA_SELECTION_GPIO_BIT_GPIO10_MASK 32
/** Antenna selection gpio bit mask for GPIO11 */
#define UWB_ANTENNA_SELECTION_GPIO_BIT_GPIO11_MASK 64
/** Antenna selection gpio bit mask for GPIO14 */
#define UWB_ANTENNA_SELECTION_GPIO_BIT_GPIO14_MASK 128

/** MAX RFRAME Measurements */
#define MAX_RFRAME_MEAS 2
#if UWBFTR_UWBS_DEBUG_Dump
/**  MAX RFRAME packet size */
#define MAX_RFRAME_PACKET_SIZE (sizeof(phUwbRframeLogNtf_t) * MAX_RFRAME_MEAS * MAX_NUM_RESPONDERS)
#endif // UWBFTR_UWBS_DEBUG_Dump

/**
 *  \brief OTP Read Write Options.
 */
typedef enum otpRWOption
{
    /* bit 0 set to 1b : Calibration Parameter */
    CALIB_PARAM = 0x01,
#if 0
    /* bit 1 set to 1b : Calibration Parameter bit mask value */
    BIT_MASK_VALUE = 0x02,
    /* bit 2 set to 1b : CMAC Tag */
    CMAC_TAG = 0x04
#endif
    /* Bit 2 - 7 : RFU */
} otpRWOption;

/**
 *  \brief Calib params payload.
 */
typedef struct phCalibPayload
{
    /** VCO PLL code **/
    uint16_t VCO_PLL;
    /** Tx power Id **/
    uint8_t TX_POWER_ID[4];
    /** XTAL cap values. It Can be set only once.
     * Channel independent and remains same for each channel.
     * Octet [0]: 38.4 MHz XTAL CAP1
     * Octet [1]: 38.4 MHz XTAL CAP2
     * Octet [2]: 38.4 MHz XTAL GM CURRNT CONTROL
     * Values : [0x00-0xFF] for Octet[1:0]
     * Values : [0x00-0x3F] for Octet[2]
    **/
    uint8_t XTAL_CAP_VALUES[3];
    /** RSSI CONSTANT1(4*2). this parameter is channel and antenna pair dependent.
     *  Channel number to be provide in calibration commands to set this parameter.
     * 4 antenna pairs with 2 RX each
     * 2 RX: RX1 and RX2
     **/
    uint8_t RSSI_CONSTANT1[8];
    /** RSSI CONSTANT2(4*2). this parameter is channel and antenna pair dependent.
     *  Channel number to be provide in calibration commands to set this parameter.
     * 4 antenna pairs with 2 RX each
     * 2 RX: RX1 and RX2
     **/
    uint8_t RSSI_CONSTANT2[8];
    /** Tx power parameters (4*4)
     * 4 Antenna pairs
     * 4 parameters for each antenna pairs
     *    Octet[0]: PA_GAIN
     *    Octet[1]: PA_DRIVE_GAIN
     *    Octet[2]: DIG_GAIN
     *    Octet[3]: TX_DAC_GAIN
     **/
    uint8_t TX_POWER_PARAMS[16];
    /** PA output capacitor control **/
    uint16_t PA_PPA_CALIB_CTRL;
    /** Tx tempreature comp. This parameter is dependent on the chosen channel and Tx antenna.
     * (2*4*2):
     *  2 bytes value description
     *      Octet [0]:TX_POWER_TEMP_UPPER_BOUND
     *      Octet [1]:TX_POWER_GAIN_INDEX
     *  4 bytes: 2 Octets repeated 4 times to allow up to 4 different temperature ranges
     *      Octet[1-0]: Temperature range 1 and gain index
     *      Octet[3-2]: Temperature range 2  and gain index
     *      Octet[5-4]: Temperature range 3 and gain index
     *      Octet[7-6]: Temperature range 4 and gain index
     *  2 bytes: Number of Tx antenna selection options
     **/
    uint8_t TX_TEMP_COMP[16];
    /** Delay calibration. It Can be set only once. Same value will be applied for all channels **/
    uint16_t DELAY_CALIB_VALUE;
    /* First 2 byte corresponds to RX Antenna pair
       ID 1 and next 2 bytes corresponds to RX
       Antenna pair ID 2.*/
    uint8_t PDOA_MFG_0_OFFSET_CALIB[4];
    /* ( Each multipoint size
        is 4b and 2 points are supported per pair
        = 4b each mulipoint vlaue * 2 points *2 RX
        antenna pairs = 16)
        Nomenclature to be followed by host to store
        and retreive is
        First 8 bytes ( 4b multipoint-1 + 4b multipoi
        nt-2) corresponds RX Antenna pair ID 1 and
        next 8 bytes ( 4b multipoint-1 + 4b multipoi
        nt-2) corresponds to RX Antenna pair ID 2*/
    uint8_t AOA_ANT_MULTIPOINT_CALIB[16];

} phCalibPayload_t;

/**
 *  \brief SetApp Configuration parameters supported in UWB API layer.
 */
typedef enum appConfig
{
    /** Devce Type
     * 0x00 = Controlee
     * 0x01 = Controller
     */
    /* UWB Prefix added to avoid potential redefinition with platform macros. */
    UWB_DEVICE_TYPE = 0x00,
    /** Ranging Round Usage
     * 0x00 = One Way Ranging UL-TDoA
     * 0x01 = SS-TWR with Deferred Mode
     * 0x02 = DS-TWR with Deferred Mode
     * 0x03 = SS-TWR with Non-deferred Mode
     * 0x04 = DS-TWR with Non-deferred Mode
     * 0x05 = One Way Ranging DL-TDOA
     * 0x06 = OWR for AoA Measurement
     * 0x07 = eSS-TWR with Non-deferred Mode for Contention-based ranging
     * 0x08 = aDS-TWR for Contention-based ranging
     * 0x09 = Data Transfer Mode
     */
    RANGING_ROUND_USAGE = 0x01,
    /** STS Config
     *  0x00:Static STS
     *  0x01:Dynamic STS
     *  0x02:Dynamic STS for controlee individual key
     *  0x03:Provisioned STS
     *  0x04:Provisioned STS for Responder specific Sub-session Key
     *  0xA0:To be set at Anchor and User device to distinguish the transition from Static STS to Dynamic STS
     *  0x05 to 0xFF except 0xA0 : RFU
     */
    STS_CONFIG,
    /** 0x00 = Single device to Single device (Unicast)
	 *  0x01 = One to Many
	 *  0x02 = Many to Many
	 * Values 0x03 to 0xFF = RFU
	 */
    MULTI_NODE_MODE = 0x03,
    /** Possible values are {5, 6, 8, 9, 10, 12, 13, 14}
     * (default = 9)
     */
    CHANNEL_NUMBER = 0x04,
    /** Number of Controlees
     * To be configured by Host when MULTI_NODE_MODE is set other than 0x00.
     * Number of Controlees(N)
     * 1<=N<=8
     * (Default is 1)
     */
    NO_OF_CONTROLEES = 0x05,
    /** Device MAC Address
     * MAC Address of the UWBS itself participating in UWB session.
     * Size of this config is based on the MAC_ADDRESS_MODE.
     *
     * Note : In case of Extended MAC Addr mode, this config is to be set through UwbApi_SetAppConfigMultipleParams.
    */
    DEV_MAC_ADDRESS = 0x06,
    /** Destination MAC Addr
     * MAC Address of the UWBS itself participating in UWB session.
     * Size of this config is based on the MAC_ADDRESS_MODE.
     *
     * Note : In case of Extended MAC Addr mode, this config is to be set through UwbApi_SetAppConfigMultipleParams.
     */
    DST_MAC_ADDRESS = 0x07,
    /**
     * Unsigned integer that specifies duration of a ranging slot in the unit of RSTU (Ranging/Radar Standard Time Unit)
     * (default = 2400)
     */
    SLOT_DURATION = 0x08,
    /** Ranging Duration in ms
     * Ranging duration in the unit of 1200 RSTU which is 1 ms.
     * (default = 200)
     */
    RANGING_DURATION,
    /** STS index init value */
    STS_INDEX,
    /** MAC FCS TYPE
     * 0x00 = CRC 16 (default)
     * 0x01 = CRC 32
     */
    MAC_FCS_TYPE,
    /** 1:Enable, 0:Disable
     * Below bits are applicable when SCHEDULED_MODE is set to 0x01(Time scheduled ranging)
     * b0 - Measurement Report Phase
     * b1 - Control Phase
     * b2 - Configuration of RCP in Non-deferred Mode TWR
     * b3 : b5 - RFU
     * b6 - Measurement Report Phase (MRP) [UWBS shall ignore this bit]
     * b7 - Measurement Report Message (MRM)
     * (default = 0x03)
     * Below bits are applicable when SCHEDULED_MODE is set to 0x00(Contention-based ranging)
     * b0 - Ranging Result Report Message (RRRM) UWBS shall ignore this bit
     * b1 - 1 (Controller shall send a CM in-band and a Controlee shall expect a CM in-band)
     * b2 - 1 (RCP is excluded in Ranging Round)
     * b5 : b3 = RFU
     * b6 - Measurement Report Phase (MRP) ; If set to 0, MRP is not present (default) ; If set to 1, MRP is present
     * b7 - Measurement Report Message (MRM) UWBS shall ignore this bit.
     * (default = 0x06)
     */
    RANGING_ROUND_CONTROL,
    /** AOA RESULT REQ
     * 0x00 = AoA results are disabled.
     * 0x01 = AoA results are enabled(default), return all the AOA type supported by the device
     * 0x02 = Only AoA Azimuth is enabled
     * 0x03 = Only AOA Elevation is enabled
     */
    AOA_RESULT_REQ,
    /** SESSION_INFO_NTF
     * 0x00 = Disable range data SESSION_INFO_NTF
     * 0x01 = Enable range data SESSION_INFO_NTF (default)
     * 0x02 = Enable range data SESSION_INFO_NTF while inside proximity range
     * 0x03 = Enable range data SESSION_INFO_NTF while inside AoA (upper and lower) bounds
     * 0x04 = Enable range data SESSION_INFO_NTF while inside AoA bounds as well as inside proximity range
     * 0x05 = Enable range data SESSION_INFO_NTF only when entering and leaving proximity range.
     * 0x06 = Enable range data SESSION_INFO_NTF only when entering and leaving AoA (upper and lower) bound
     * 0x07 = Enable range data SESSION_INFO_NTF only when entering and leaving AoA bounds as well as entering and leaving proximity range.
     */
    SESSION_INFO_NTF,
    /** Proximity near in cm
     * (default = 0)
    */
    NEAR_PROXIMITY_CONFIG,
    /** Proximity far in cm
     * (default = 20000)
    */
    FAR_PROXIMITY_CONFIG,
    /** Devive Role
     * 0x00 = Responder
     * 0x01 = Initiator
     * 0x02 = Assigned
     * 0x03 = Assigned
     * 0x04 = Assigned
     * 0x05 = Advertiser
     * 0x06 = Observer
     * 0x07 = DT-Anchor
     * 0x08 = DT-Tag
     */
    DEVICE_ROLE,
    /** Activate or deactivate RSSI reporting
     * 0x00 = SP0 (Reserved value for test purpose)
     * 0x01 = SP1
     * 0x02 = RFU
     * 0x03 = SP3
     * Values 0x04 to 0xFF = RFU
     * (default = 0x03)
     */
    RFRAME_CONFIG = 0x12,
    /** Activate or deactivate RSSI reporting
     * 0x00 : Disable(default)
     * 0x01 : Enable
     * 0x02-0xFF :RFU
    */
    RSSI_REPORTING = 0x13,
    /** Preamble Code Index
     * [9-12]   - BPRF,
     * [25-32]  - HPRF, used for RADAR_MODE 1, 2 and 3
     * [95-102] - LPRF used for RADAR_MODE 5 and 6
     * (default = 10)
     */
    PREAMBLE_CODE_INDEX = 0x14,
    /** [0,2]:BPRF, [1,2,3,4]:HPRF (default = 2) */
    SFD_ID,
    /** PSDU DATA RATE
     * 0x00 = 6.81 Mbps (default)
     * 0x01 = 7.80 Mbps
     * 0x02 = 27.2 Mbps
     * 0x03 = 31.2 Mbps
     * 0x04 = 850 Kbps
     * Values 0x00, 0x02, 0x04 map to K=3 and 0x01, 0x03 map to K=7.
     */
    PSDU_DATA_RATE,
    /** 0:32 symbols, 1:64 symbols, 0xA0:1024 symbols */
    PREAMBLE_DURATION,
    /**  0x00 – Bypass Mode (Default), 0x01 – Connection-Less data transfer */
    LINK_LAYER_MODE,
    /**  0x00 – No repetition (Default), 0xFF – Repeat infinite number of times */
    DATA_REPETITION_COUNT,
    /**  1:Block based (default) & 0:[2-255]:RFU */
    RANGING_TIME_STRUCT,
    /** Number of slots for per ranging round (default = 25) */
    SLOTS_PER_RR,
    /** AOA_BOUND_CONFIG
     * 1:0 AOA_BOUND_CONFIG_LOWER_BOUND_AOA_AZIMUTH range -180 (default) to 180
     * 3:2 AOA_BOUND_CONFIG_UPPER_BOUND_AOA_AZIMUTH range -180 to 180 (default)
     * 5:4 AOA_BOUND_CONFIG_LOWER_BOUND_AOA_ELEVATION -90 (default) to 90
     * 7:6 AOA_BOUND_CONFIG_UPPER_BOUND_AOA_ELEVATION -90 to 90 (default)
    */
    AOA_BOUND_CONFIG = 0x1D,
    /** PRF MODE
     * 0x00 = 62.4 MHz PRF. BPRF mode (default)
     * 0x01 = 124.8 MHz PRF. HPRF mode.
     * 0x02 = 249.6 MHz PRF. HPRF mode with data rate 27.2 and 31.2 Mbps
     */
    PRF_MODE = 0x1F,
    /** configuration parameter sets the minimum and maximum CAP size to be used by the
	 * Controller/Initiator in the Contention-based ranging session, in the units of Ranging Slots.
	 *
	 * Octet [0] - represents the maximum CAP size. Default = SLOTS_PER_RR-1.
	 * Octet [1] - represents the minimum CAP size. Default = 5.
	 */
    CAP_SIZE_RANGE = 0x20,
    /** Parameter is used to set the multi-node Ranging Type.
     * 0x00 = Contention-based ranging
     * 0x01 = Time scheduled ranging (default)
     * 0x02 = Hybrid-based ranging
     * Values 0x03 to 0xFF = RFU
     */
    SCHEDULED_MODE = 0x22,
    /** 1: Enable, 0:Disable [Default] */
    KEY_ROTATION,
    /** Key Rotation rate 2^n where 0<=n<=15 */
    KEY_ROTATION_RATE,
    /** Session Priority 1-100, default : 50 */
    SESSION_PRIORITY,
    /** MAC address mode Default 0 [SHORT] */
    MAC_ADDRESS_MODE,
    /** Unique vendor Id*/
    VENDOR_ID,
    /** Vendor defined static sts*/
    STATIC_STS_IV,
    /** Number of STS segments in the frame. 0x00:No STS
     * Segments(if PPDU_COFIG is 0). If PPDU Config is
     * set to 1 or 3 then 0x01:1 STS Segment(default),
     * 0x02:2 STS Segments */
    NUMBER_OF_STS_SEGMENTS,
    /** Number of Failed Ranging Round attempts before terminating
     * the session. Default : 0 */
    MAX_RR_RETRY,
    /** Indicates when ranging operation shall start after
     * ranging start request is issued from AP. Default : 0 */
    UWB_INITIATION_TIME,
    /** Modes for the hopping.
     * 0x00: No hopping
     * 0x02: Adaptive hopping using MODULO
     * 0x03: continuous hopping using MODULO
     * 0x04: adaptive hopping using AES
     * 0x05: continuous hopping using AES
     * Default : 0 */
    HOPPING_MODE,
    /** Block Stride Length. 0x00:Default, [0x01-0xFF]:Application use case specific value **/
    BLOCK_STRIDE_LENGTH,
    /** Config to enable result report, 0: Disable, This is applicable only RANGING_ROUND_CONTROL enabled */
    RESULT_REPORT_CONFIG,
    /** Indicates how many times in-band termination signal needs to be sent by controller/initiator to a controlee device. Default : 1 */
    IN_BAND_TERMINATION_ATTEMPT_COUNT,
    /** Sub-Session Handle for the controlee device.
     * This config is mandatory and it is applicable if STS_CONFIG is set to 2 for controlee device */
    SUB_SESSION_ID,
    /** PHR coding rate Default : 0(850kpbs) */
    BPRF_PHR_DATA_RATE,
    /**Maximum Number of ranging blocks to executed in a session  Default : 0*/
    MAX_NUMBER_OF_MEASUREMENTS,
    /** This parameter specifies the average transmission interval of Blink UTMs from
     *  UT-Tags and/or Synchronization UTMs from UT-Synchronization Anchors, as defined
     *  by the UL-TDoA TX Interval MAC configuration parameter.
     *
     *  By default, UL_TDOA_TX_INTERVAL = 2000ms.
     */
    UL_TDOA_TX_INTERVAL = 0x33,
    /** Length of the randomization window within which Blink and Synchronization UTMs may be transmitted. */
    UL_TDOA_RANDOM_WINDOW = 0x34,
    /** STS length 0: 32 symbols , 1: 64 symbols(Default), 2: 128 symbols */
    STS_LENGTH = 0x35,
    /** configuration allows the Ranging Rounds to be suspended */
    SUSPEND_RANGING_ROUNDS = 0x36,
    /** UT-Anchor configuration to specify if UL-TDoA related SESSION_INFO_NTF */
    UL_TDOA_NTF_REPORT_CONFIG = 0x37,
    /** This value shall be used to specify the length and presence of the UL-TDoA Device ID in UTMs. */
    UL_TDOA_DEVICE_ID = 0x38,
    /** Presence and length of TX timestamps in UTMs. */
    UL_TDOA_TX_TIMESTAMP = 0x39,

    /** Minimum number of frames to be transmitted in a ranging round.(default = 4) */
    MIN_FRAMES_PER_RR = 0x3A,
    /** Maximum Transfer Unit (MTU) Size represents the maximum size of allowed payload size to be transmitted in a frame */
    MTU_SIZE = 0x3B,
    /** The configuration in units of 1200 RSTU to configure the interval between the RFRAMES transmitted in the “OWR for AoA Measurement” ranging round.(default = 1) */
    INTER_FRAME_INTERVAL = 0x3C,
    /** DL-TdoA ranging round 0: SS-TWR, 1:DS-TWR(default) */
    DLTDOA_RANGING_METHOD = 0x3D,
    /** DL-TdoA Tx timestamp conf b0: TX timestamp type b1-2: TX timestamp length , Default: 00000011*/
    DLTDOA_TX_TIMESTAMP_CONF,
    /** controls cluster sync field 1: inter cluster sync filed in every poll DTM */
    DL_TDOA_HOP_COUNT,
    /** DL-TdoA anchor CFO 0: not included, 1:Anchor CFO included(default) */
    DLTDOA_ANCHOR_CFO,
    /** DL-TdoA anchor location */
    DLTDOA_ANCHOR_LOCATION,
    /** DL-TdoA tx active ranging rounds 0: not present (default) 1: present */
    DLTDOA_TX_ACTIVE_RANGING_ROUNDS,
    /** To configure number of blocks that shall be skipped by a DT-Tag between two active ranging blocks,
     * 0x00      : No blocks striding(default)
     * 0x01-0xFF : no. of blocks to be skipped by DT-Tag*/
    DL_TDOA_BLOCK_STRIDING = 0x43,
    /** global time rference of dl-tdoa network 0: Disable 1: Set global metric time */
    DLTDOA_TIME_REF_ANCHOR = 0x44,
    /**Session Key provided for Provisioned STS mode (STS_CONFIG equal to 0x03 or 0x04)
     * If the Session Key is not provided by the Host in Provisioned STS mode, the UWBS shall fetch the Session Key from the Secure Component.
     * This parameter is valid only in Provisioned STS mode and shall be ignored otherwise.*/
    SESSION_KEY = 0x45,
    /**Sub-session Key provided for Provisioned STS for Responder specific Key mode (STS_CONFIG equal to 0x04).
     * If the Sub-session Key is provided by the Host, the Host shall also provide the SESSION_KEY.
     * If the Sub-session Key is not provided by the Host for Provisioned STS for Responder specific Key mode, the UWBS shall fetch the Sub-session Key from the Secure Component.
     * This parameter is valid only in Provisioned STS for Responder specific Key mode and shall be ignored otherwise.*/
    SUB_SESSION_KEY = 0x46,
    /**This parameter is used to configure the SESSION_DATA_TRANSFER_STATUS_NTF.
     * 0x00 = Disable SESSION_DATA_TRANSFER_STATUS_NTF (Default).
     * 0x01 = Enable SESSION_DATA_TRANSFER_STATUS_NTF.
     * If SESSION_DATA_TRANSFER_STATUS_NTF is disabled, then the UWBS shall not send SESSION_DATA_TRANSFER_STATUS_NTF for every Application Data transmission except for last transmission.
    */
    SESSION_DATA_TRANSFER_STATUS_NTF_CONFIG = 0x47,
    /** Configures a reference time base for the given session.
     * Octet 0:
     * b0: Reference time base feature
     *       0b0 = Enable
     *       0b1 = Disable (default)
     * b1: continue/stop the session(s) when reference session is not in SESSION_STATE_ACTIVE Session State
     *       0b0 = stop (default)
     *       0b1 = continue
     * b2: Resync time grid in case the reference session will become active again after it has been inactive.
     *       0b0 = No resync (default)
     *       0b1 = Resync
     * Octet 1-5: Session Handle of the reference session
     * Octet 5-8: Session offset time in microseconds
    */
    SESSION_TIME_BASE = 0x48,
    /**This parameter specifies whether a DT-Anchor with the Responder role in a given ranging round
     * shall include the estimated Responder ToF Result in a Response DTM.
     * 0x00: Responder ToF Result shall not be added to Response DTMs (default).
     * 0x01: Responder ToF Result shall be added to Response DTMs.
     */
    DL_TDOA_RESPONDER_TOF = 0x49,
    /**Local endpoint configuration of the session
     * It defines which endpoint is used by the UWBS for Application data exchange using the non-secure or secure message connection.
     * When using the Bypass mode, all data shall be exchanged using the Non-secure endpoint
     * Values
     * b3-b0: Non-secure end point configuration
     * 0x0 : Host  (default)
     * 0x1: Secure Component
     * 0xF- 0x2 :  RFU
     * b7-b4: Secure end point configuration
     * 0x0 : Host (default)
     * 0x1: Secure Component
     * 0xF- 0x2 :  RFU
     */
    APPLICATION_DATA_ENDPOINT = 0x4C,
#if UWBFTR_CCC
    /** Key to generate hopping sequence.
     * This value is used for both AES and MODULO hopping formula.
     * For MODULO hopping, only first 4 bytes are used as converted to 4byte integer.
     * (default key for AES hopping formula = 0x4c,0x57,0x72,0xbc)
     * (default key for MODULO hopping formula = 0xbc72574c)
     * */
    HOP_MODE_KEY = 0xA0,
    /**
     * 0x00: Key Identifier Field of Auxiliary Security Header of the MAC Header as per CCC Digital Key Release 3, Version 1.0.0
     * 0x01: Key Identifier Field formatted as: Source Short Address || Source PAN ID (default = 0x01)
     */
    CCC_CONFIG_QUIRKS = 0xA1,
    /**  1: responder1,...,N: responder N */
    RESPONDER_SLOT_INDEX = 0xA2,
    /** Version of the ranging protocol (defined by CCC)
     * [0x0000 – 0xFFFF]
     * (default = 0x0100)
     */
    RANGING_PROTOCOL_VER = 0xA3,
    /** UWB Configuration ID
     * [0x0000 – 0xFFFF]
     * (default = 0x0001)
     */
    UWB_CONFIG_ID = 0xA4,
    /** Pulse Shape Combinations.
     * Possible combinations are written in format:
     * Pulse shape combo value - Initiator transmit pulse shape - Responder transmit pulse shape
     * 0x00 - 0x0 - 0x0
     * 0x01 - 0x0 - 0x1
     * 0x02 - 0x0 - 0x2
     * 0x10 - 0x1 - 0x0
     * 0x11 - 0x1 - 0x1
     * 0x12 - 0x1 - 0x2
     * 0x20 - 0x2 - 0x0
     * 0x21 - 0x2 - 0x1
     * 0x22 - 0x2 - 0x2
     * Support for value 0x00 is mandatory.
     * (default = 0x00)
     */
    PULSESHAPE_COMBO = 0xA5,
    /** URSK expiration time, in minutes (max 12 hours).
     * After this time from setting URSK, the session will go to idle.
     * [0x001 - 0x2D0]
     * (default = 0x2D0)
     */
    URSK_TTL = 0xA6,
    /** Responder is in listen only mode.
     * If it is enabled then the responder will not send range frame. In this mode, the responder will report 0xD (responder is in listen only mode)
     * as higher nibble of ranging status in RANGE_CCC_DATA_NTF
     * 0x00: Responder is in normal mode.
     * 0x01: Responder is in listen only mode
     * (default = 0x00)
     */
    RESPONDER_LISTEN_ONLY = 0xA7,
    /** Parameter used to get the STS index of the UWB session.
     * When SESSION_GET_APP_CONFIG_CMD issued for this config during SESSION_STATE_ACTIVE the UWBS shall return the last STS Index of the latest completed ranging block.
     */
    LAST_STS_INDEX_USED = 0xA8,

#endif // UWBFTR_CCC

    /** End of App Configs*/
    END_OF_SUPPORTED_APP_CONFIGS,
} eAppConfig;

/* DEPRECATED enums. These defines will be removed in future releases */
#define RANGE_DATA_NTF_BOUND_AOA    AOA_BOUND_CONFIG
#define RNG_DATA_NTF_PROXIMITY_FAR  FAR_PROXIMITY_CONFIG
#define RNG_DATA_NTF_PROXIMITY_NEAR NEAR_PROXIMITY_CONFIG
#define RNG_DATA_NTF                SESSION_INFO_NTF

/**
 *  \brief Set Get Vendor App Configuration parameters supported in UWB API layer.
 */
typedef enum vendorAppConfig
{
    /** This parameter shall enable disable encryption of Payload data
     * 0x00 - Plain Text
     * 0x01 - Encrypted(default)
    */
    MAC_PAYLOAD_ENCRYPTION = 0x00,
    /**
     * The antenna used for TX.
     * If Octet[0] of ANTENNAE_CONFIGURATION_TX is 0 and Octet[1] of ANTENNAE_CONFIGURATION_RX is
     * 0, FW automatically enters Scan Phase.
     * Octet[0] - Define number of TX Antennas to follow (default value is 1).
     * Octet[1] - Tx Antennas ID as defined by ANTENNA_TX_DEFINE (default value is 1). So we transmit by
     * default with Antennas ID 1 (As a pre requisite: an antenna with ID 1 using ANTENNA_TX_IDX_DEFINE must
     * be explicitly pre-defined).
     * Octet[2] - Tx Antennas ID as defined by ANTENNA_TX_DEFINE Must be 0 for SR100T, SR150 & SR160..
     */
    ANTENNAE_CONFIGURATION_TX = 0x02,
    /**
     * The session specific antenna configuration for Rx
     * If Octet[0] of ANTENNAE_CONFIGURATION_TX is 0, 0, and Octet[0] of ANTENNAE_CONFIGURATION_RX
     * is 0,0 FW automatically enters Scan Phase.
     * Octet [0] : Mode of RX operation
     *  – 0 : Configuration ToA Mode
     *      – ToF Only Mode
     *  – 1 : Configuration AoA Mode
     *      – Dual / Single AoA usecase
     *      – ToA Mode with implicit Rx mode as per ANTENNAS_RX_PAIR_DEFINE
     *      – Test / Loopback mode usecase
     *  – 2 : Configuration Mode 2: Radar Mode
     *      – Default 0x01 (Note: SR100S only)
     *  – 3 : Configuration Mode 3: ToA usecase using different Rx antenna pair for RFM
     *  – 4 : Configuration Mode 4: AoA usecase using different Rx antenna pair for RFM
     *  – 5 : Configuration Mode 5: ToA mode for CSA
     *  – 6 : Configuration Mode 6: AoA mode for CSA
     * Octet [1] : Number of Antennas or Antenna pairs to follow
     * Default Value:
     *  Generic Session will have
     *  Octet[0] : 0x00
     *  Octet[1] : 0x01
     * Test Session will have
     *  Octet[0] : 0x01
     *  Octet[1] : 0x01
     *
     * Note : Please refer the spec for more details on each configuration mode.
     */
    ANTENNAE_CONFIGURATION_RX = 0x03,
    /**
     * Return the possible RAN multiplier value for a new session
     */
    RAN_MULTIPLIER = 0x20,
    /**
     * Parameter used to get the STS index of the UWB session.
     * When GET_VENDOR_APP_CONFIG_CMD issued for this config during SESSION_STATE_ACTIVE the
     * UWBS shall return the last
     */
    STS_LAST_INDEX_USED = 0x21,
    /**
     * 0x00: Disable (default)
     * 0x01: Enable
     */
    CIR_LOG_NTF = 0x30,
    /**
     * 0x00: Disable (default)
     * 0x01: Enable
     */
    PSDU_LOG_NTF = 0x31,
    /** This parameter is used to filter out the outliers in RSSI measurements in PER RX Test.
     * If the RSSI filtering count is set to N and total packet count is set to M then UWBS shall report the average of
     * (M-2N) RSSI values in TEST_PER_RX_NTF excluding(N) maximum and (N) minimum RSSI values.
     * Note: M is the total packet count to be received in PER Rx test
     * (default = 0)
      */
    RSSI_AVG_FILT_CNT = 0x40,
    /**
     * CIR sampling position for incoming UWB packet
     * bits[7:4] - CIR1 capture mode
     * bits[3:0] - CIR0 capture mode
     * CIR capture modes:
     * 0x0 - Pre SYNC RX1
     * 0x1 - Pre SYNC RX2
     * 0x2 - Pre STS RX1
     * 0x3 - Pre STS RX2
     * 0x4 - Post SYNC RX1
     * 0x5 - Post SYNC RX2
     * 0x6 - Post STS RX1
     * 0x7 - Post STS RX2
     * 0x8 - 0xF - RFU
     * (default = 0x76)
     *
     */
    CIR_CAPTURE_MODE = 0x60,
    /**
     * This parameter is used to choose the antenna polarization option when AOA measurement is enabled (AOA_
     * RESULT_REQ = 1). This parameter is not applicable when AOA_RESULT_REQ = 0.
     * bit[0]: Polarization option to be used for the first AoA computation
     * [0]: Straight (Default)
     * [1]: Reverse
     * bit[1]: Polarization option to be used for the second AoA computation when enabled either via DUAL_AOA_
     * PREAMBLE_STS or NUMBER_OF_STS_SEGMENTS = 2. The value is not applicable when second AoA
     * computation is not enabled
     * [0]: Straight (Default)
     * [1]: Reverse
     * bit[7:2]: RFU
     *
     */
    RX_ANTENNA_POLARIZATION_OPTION = 0x61,
    /**
     * Number of times scheduler shall attempt to sync in controlee session before reporting error notification. This
     * config is applicable for controlee session only.
     * Range: [3 : 255] (Default: 3)
     *
     */
    SESSION_SYNC_ATTEMPTS = 0x62,
    /**
     * Number of times scheduler shall attempt to schedule ranging round before reporting error notification
     * Range: [1 : 255] (Default: 3)
     *
     */
    SESSION_SCHED_ATTEMPTS = 0x63,
    /**
     * Enable/disable SCHEDULER_STATUS_NTF
     * 0x00 - Disable (default)
     * 0x01 - Enable for including all the sessions information in notification
     * 0x02 - Enable for include only failure sessions information in notification
     * 0x03-0xFF: RFU
     *
     */
    SCHED_STATUS_NTF = 0x64,
    /**
     * Session specific Tx power ID offset applied on top of Tx POWER calibration parameter. configured via SET_
     * DEVICE_CALIBRATION_CMD.
     * 0:No offset (default)
     * 1 to 127: Attenuation (0.25 dB per steps)
     * 128 to 255: RFU
     *
     */
    TX_POWER_DELTA_FCC = 0x65,
    /**
     * This parameter is used to enable/disable KDF notification generation.
     * 0x00: Disable (default)
     * 0x01: Enable
     *
     */
    TEST_KDF_FEATURE = 0x66,
    /**
     * This parameter is used to enable/disable Tx power temperature compensation
     * 0x00: Disable (Default)
     * 0x01: Enable
     *
     */
    TX_POWER_TEMP_COMPENSATION = 0x67,
    /**
     * WiFi-CoEx maximum tolerance count, after the expiry of the number of count the UWBS shall make the
     * "Medium Grant Request" with priority field set to "Critical".
     * This parameter can be modified when session is in SESSION_STATE_ACTIVE Session State.
     * Range: [1 : 25] (Default: 3)
     *
     */
    WIFI_COEX_MAX_TOLERANCE_COUNT = 0x68,
    /**
     * This parameter can be used to configure the required number of successful responses(T) from Responders to
     * conclude a successful ranging round.
     * If numbers of responses is less than this given threshold(T) when Initiator device acting as Controller then
     * initiator device triggers a hop to a different round index within the next block.
     * Range: [0<T<= NUMBER_OF_CONTROLEES] (Default: NUMBER_OF_CONTROLEES)
     * Note: This parameter is applicable when HOPPING_MODE = 0xA0 (NXP Adaptive Hopping mode is Enabled)
     */
    ADAPTIVE_HOPPING_THRESHOLD = 0x69,
    /**
     * Config to enable/disable authenticity tag in RANGE_DATA_NTF. This config is applicable when STS_CONFIG
     * is set to Dynamic STS.
     * 0x00 = Disable (Default)
     * 0x01 = Enable
     *
     */
    AUTHENTICITY_TAG = 0x6E,
    /**
     * Octet [0] : Used to configure NBIC settings
     * b[0]: Enable / Disable NBIC.
     *   0 = Disable (default)
     *   1 = Enable
     * b[1:2]: Content of register MA_FILTER_BW_SET. Filter bandwidth setting (default: 0x3)
     * b[3:4]: Content of register MA_FILTER_BW_START_SET. Starting filter bandwidth setting for estimation
     *   (default: 0x3)
     * b[5:7]: RFU
     * Octet [1]: Content of register PSD_WEIGHT_SET
     *   (Default: 0x14 )
     *   (Default: 0x40 only applicable when NBIC is enabled )
     *
     */
    RX_NBIC_CONFIG = 0x6F,
    /**
     * Config is used to configure the MAC Header and MAC Footer
     * b[0]: MAC Header present
     * b[1]: MAC Footer present
     * b[7:2]: RFU
     *   (Default value: 0x03 for FIRA Session)
     *   (Default value: 0x00 for Test Mode Session)
     *
     */
    MAC_CFG = 0x70,
    /**
     * Amount of blocks which should be reserved for the given session for inband data transfer for transmitting
     * data. If set to 0, transmitting inband data is not allowed for this session.
     * Note: The sum of this value for all active session must not exceed UWBS_INBAND_DATA_MAX_BLOCKS
     * and will prevent the first session which is causing to exceed the limit to get started.
     * Default Value:
     *   0 for Ranging Session
     *   UWBS_INBAND_DATA_BUFFER_BLOCK_SIZE for Data Session with DEVICE_TYPE set to Controlee
     *   0 for Data Session with DEVICE_TYPE set to Controller
     *
     */
    SESSION_INBAND_DATA_TX_BLOCKS = 0x71,
    /**
     * Amount of blocks which should be reserved for the given session for inband data transfer for receiving data. If
     * set to 0, receiving inband data is not allowed for this session.
     * Note: The sum of this value for all active session must not exceed UWBS_INBAND_DATA_MAX_BLOCKS
     * and will prevent the first session which is causing to exceed the limit to get started.
     * Default Value:
     *   0 for Ranging Session
     *   UWBS_INBAND_DATA_BUFFER_BLOCK_SIZE for Data Session with DEVICE_TYPE set to Controller
     *   0 for Data Session with DEVICE_TYPE set to Controlee
     *
     */
    SESSION_INBAND_DATA_RX_BLOCKS = 0x72,
    /**
     * List of scanning pairs for Antennas.
     * Assuming we have Anteannes East, West, North, South(E, W, N, S)
     * Octet[0 + 0] E-TX for 1st Round
     * Octet[0 + 1] E-RX1 for 1st Round
     * Octet[0 + 2] W-RX2 for 1st Round
     * Octet[3 + 0] N-TX for 2nd Round
     * Octet[3 + 1] N-RX1 for 2nd Round
     * Octet[3 + 2] S-RX2 for 2nd Round
     * More entries as needed.
     * Antennas IDs as defined by ANTENNA_TX_IDX_DEFINE and ANTENNA_RX_IDX_DEFINE.
     * Once FW detects which Antenna has
     * In case of 3D AoA, Even IDs of RX Pair are for H and Odd IDs are for V Configuration as an enforced
     * convention.
     *
     */
    ANTENNAE_SCAN_CONFIGURATION = 0x74,
    /**
     * This configuration shall be used to configure DATA_TRANSMISSION_STATUS_NTF indication
     * 0x00 : Always ON
     * 0x01 : Always OFF
     * 0x02 : Notify when error
     * (Default: 0x00)
     * Note:
     * The UWBS shall always send DATA_TRANSMISSION_STATUS_NTF whenever it receives DATA_
     * MESSAGE_SND, the subsequent DATA_TRANSFER_TX_STATUS_NTF on RF transmit shall be sent based
     * on DATA_TRANSFER_TX_STATUS_CONFIG configuration
     *
     */
    DATA_TRANSFER_TX_STATUS_CONFIG = 0x75,
#if (UWBFTR_UL_TDoA_Tag)
    /**
     * Parameter to select MAC frame format for UL-TDOA Tag device
     * 0x00: FIRA (Default)
     * 0x01: Vendor-specific MAC format
     * Note: This parameter is only applicable when RANGING_ROUND_USAGE = 0x00
     * (One Way Ranging UL-TDoA) and DEVICE_ROLE = 0x04 (UT-Tag)
     *
     */
    ULTDOA_MAC_FRAME_FORMAT = 0x76,
#endif // (UWBFTR_UL_TDoA_Tag)
#if (UWBFTR_SE_SE051W)
    /**
     * Session ID (4 octets) || Random (12 octets) || Wrapped RDS from SE (Maximum 40 octets)
     *
     */
    WRAPPED_RDS = 0x79,
#endif
    /**
    * This configuration is used to enable/disable RFRAME LOG NTF.
    * 0x00 = Disable (default)
    * 0x01 = Enable
    * Values 0x02 to 0xFF = RFU
    */
    RFRAME_LOG_NTF = 0x7B,
    /**
    * This configuration is used to enable/disable adaptive payload power for TX.
    * 0x00 = Disable
    * 0x01 = Enable (default)
    * Values 0x02 to 0xFF = RFU
    */
    TX_ADAPTIVE_PAYLOAD_POWER = 0x7F,
#if (UWBIOT_UWBD_SR150)
    /** Session specific configuration parameter is used to swap the antenna pair for RFM reception.
     *
     * 0x00 =  not swap ( Same pairs are used for all message reception) (Default)
     * 0x01 = Pair1 and Pair 2 are swapped for RFM reception
     *
     * When SWAP_ANTENNA_PAIR_3D_AOA is set to 0x01 then RSSI measurements shall be report for all pairs in the RANGE_DATA_NTF.
     * Applicable only for Responder.
     */
    SWAP_ANTENNA_PAIR_3D_AOA = 0x80,
#endif // UWBIOT_UWBD_SR150
    /**
     * Octet 0: RML_NEAR_PROXIMITY (default = 0)
     * This parameter sets the lower bound in meters where the discovered devices are added into the RML list.
     * Should be less than or equal to  RML_FAR_ _CONFIG value.
     *
     * Octet 1: RML_FAR_PROXIMITY (default = 5)
     * This parameter sets the upper bound in meters above which the RML list is not added with the discovered devices.
     * Should be greater than or equal to  RML_NEAR_CONFIG value.
     *
     */
    RML_PROXIMITY_CONFIG = 0x81,
    /**
     * This configuration is used to configure:
     *      1.The number of active ranging rounds in a RANGING_DURATION.
     *      2.Offset between two active ranging rounds in a RANGING_DURATION.
     * [b7-b6]: Number of active ranging round(s)
     * 0 = One active ranging round (default).
     * 1 = Two active ranging rounds (CSA use case).
     * 2 and 3 = RFU.
     * [b5-b0]: Offset between two active ranging rounds.
     * 1 to (Nround - 1)
     *
     * Note:
     *  Bits [b5, b0] SHALL be set if [b7, b6] set to decimal value 1. Otherwise, bits [b5, b0] will be ignored.
     *  Nround is calculated based on RANGING_DURATION, SLOTS_PER_RR and SLOT_DURATION.
     *
     */
    CSA_MAC_MODE = 0x82,
#if (UWBIOT_UWBD_SR150)
    /** 2D AoA FoV Processing Enable/Disable
     * This parameter decides whether to enable or disable 2D AoA FoV Processing.
     * 0x00 : 2D AoA FoV Processing Disabled (Default)
     * 0x01 : 2D AoA FoV Processing Enabled
    */
    FOV_ENABLE = 0x84,
    /** Field of View for Horizontal Antenna Pair.
    * This parameter indicates if the peer device is in the configured FoV of the device or not.
    * Octet[0] : Horizontal RX Antenna Pair ID as defined in 'ANTENNAS_RX_PAIR_DEFINE UCI parameter'.
    * Value 0 shall be rejected.
    * Octet[1] : FoV Coverage in degrees.
    */
    AZIMUTH_FIELD_OF_VIEW = 0x85,
#endif
#if (UWBIOT_UWBD_SR150)
    /**
    * Configuration to enable/disable transmission of Final Data 2 from Responder to Initiator.
    * 0x00 = Responder shall not transmit Final Data 2 message (default).
    * 0x01 = Responder shall transmit the Final Data 2 message
    */
    CSA_FINAL_DATA2_CONFIG = 0x90,
#endif // (UWBIOT_UWBD_SR150)
} eVendorAppConfig;

/**
 *  \brief Debug Configuration parameters supported in UWB API layer.
 */
typedef enum UWB_SR1XX_DBG_CFG
{
    kUWB_SR1XX_DBG_CFG_DATA_LOGGER_NTF                 = 0x7A,
    kUWB_SR1XX_DBG_CFG_TEST_CONTENTION_RANGING_FEATURE = 0x7C,
    kUWB_SR1XX_DBG_CFG_CIR_CAPTURE_WINDOW              = 0x7D,
    kUWB_SR1XX_DBG_CFG_RANGING_TIMESTAMP_NTF           = 0x7E,
    kUWB_SR1XX_DBG_CFG_THREAD_SECURE                   = 0xD0,
    kUWB_SR1XX_DBG_CFG_THREAD_SECURE_ISR               = 0xD1,
    kUWB_SR1XX_DBG_CFG_THREAD_NON_SECURE_ISR           = 0xD2,
    kUWB_SR1XX_DBG_CFG_THREAD_SHELL                    = 0xD3,
    kUWB_SR1XX_DBG_CFG_THREAD_PHY                      = 0xD4,
    kUWB_SR1XX_DBG_CFG_THREAD_RANGING                  = 0xD5,
    kUWB_SR1XX_DBG_CFG_THREAD_SECURE_ELEMENT           = 0xD6,
    kUWB_SR1XX_DBG_CFG_THREAD_UWB_WLAN_COEX            = 0xD7,
    /** End of Ext Debug Configs*/
    END_OF_SUPPORTED_EXT_DEBUG_CONFIGS,
} UWB_SR1XX_DBG_CFG_t;
/**
 *  \brief Set/Get App Configuration parameters type supported in UWB API layer.
 */
typedef enum UWB_AppParams_type
{
    /** We don't know the type */
    kUWB_APPPARAMS_Type_Unknown = 0,
    /** It's a 32 bit value */
    kUWB_APPPARAMS_Type_u32 = 4,
    /** It's an array of 8 bit values*/
    kUWB_APPPARAMS_Type_au8 = 5,
} UWB_AppParams_type_t;

/**
 *  \brief Set/Get Debug Configuration parameters type supported in UWB API layer.
 */
typedef enum UWB_DebugParams_type
{
    /** It's a 8 bit value */
    kUWB_DEBUGPARAMS_Type_u8 = 1,
    /** It's a 16 bit value */
    kUWB_DEBUGPARAMS_Type_u16 = 2,
    /** It's a 32 bit value */
    kUWB_DEBUGPARAMS_Type_u32 = 4,

} UWB_DebugParams_type_t;

/**
 *  \brief Set/Get App Configuration parameters value type supported in UWB API layer.
 */
typedef struct UWB_AppParams_value_au8
{
    uint8_t *param_value;
    uint16_t param_len; // Just to  handle parameter of any length
} UWB_AppParams_value_au8_t;

/**
 *  \brief Set/Get App Configuration parameters value structure supported in UWB API layer.
 */
typedef union UWB_AppParams_value {
    uint32_t vu32; // All values u8, u16 and u32 are processed here
    UWB_AppParams_value_au8_t au8;
} UWB_AppParams_value_t;

/**
 *  \brief Set/Get Debug Configuration parameters value type supported in UWB API layer.
 */
typedef struct UWB_Debug_Params_value
{
    uint8_t *param_value;
    uint16_t param_len; // Just to  handle parameter of any length
} UWB_Debug_Params_value_t;
/**
 *  \brief Set/Get Debug Configuration parameters value structure supported in UWB API layer.
 */
typedef union UWB_DebugParams_value {
    uint8_t vu8;   //values u8 are processed here
    uint16_t vu16; //values u16 are processed here
    uint32_t vu32; //values u32 are processed here
    UWB_Debug_Params_value_t param;
} UWB_DebugParams_value_t;

/**
 *  \brief Set/Get App Configuration parameters list supported in UWB API layer.
 */
typedef struct UWB_AppParams_List
{
    /** Input: search this tag */
    eAppConfig param_id;
    /** Filled Implicitly: Expected type. */
    UWB_AppParams_type_t param_type;
    /** Input: Parameter Value */
    UWB_AppParams_value_t param_value;
} UWB_AppParams_List_t;

/**
 *  \brief Set/Get Vendor App Configuration parameters list supported in UWB API layer.
 */
typedef struct UWB_VendorAppParams_List
{
    /** Input: search this tag */
    eVendorAppConfig param_id;
    /** Filled Implicitly: Expected type. */
    UWB_AppParams_type_t param_type;
    /** Input: Parameter Value */
    UWB_AppParams_value_t param_value;
} UWB_VendorAppParams_List_t;

/**
 *  \brief  This configuration is used to configure the WiFi CoEx feature.
 */
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
typedef struct UWB_WiFiCoEx_Ftr
{
    /* Enable/Disable CoEx functionality with appropriate interface and verbose */
    uint8_t UWB_WiFiCoEx_Enable;
    /* Enable/Disable CoEx functionality with appropriate interface and verbose */
    uint8_t UWB_WiFiCoEx_MinGuardDuration;
    /* Maximum duration for which the UWB can request for medium access */
    uint8_t UWB_WiFiCoEx_MaxWifiBlockDuration;
    /* OneWire CoEx Guard Duration before which UWBS can perform Tx/Rx, applied before start of every co-ex session */
    uint8_t UWB_WiFiCoEx_GuardDuration;

} UWB_WiFiCoEx_Ftr_t;

#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

/* Deprecated. Following Set app parmas specific defines are changed,
 * use above defines, defined here for backward compatibility */

typedef UWB_AppParams_type_t SetAppParams_type_t;

#define kAPPPARAMS_Type_Unknown kUWB_APPPARAMS_Type_Unknown
#define kAPPPARAMS_Type_u32     kUWB_APPPARAMS_Type_u32
#define kAPPPARAMS_Type_au8     kUWB_APPPARAMS_Type_au8

typedef UWB_AppParams_value_au8_t SetAppParams_value_au8_t;
typedef UWB_AppParams_value_t SetAppParams_value_t;
typedef UWB_AppParams_List_t SetAppParams_List_t;

/* End of deprecated defines */

/**
 *  \brief Macro to set SetApp Configuration parameters value supported in UWB API layer.
 */
#define UWB_SET_APP_PARAM_VALUE(PARAM, VALUE)                                                      \
    {                                                                                              \
        .param_id = (PARAM), .param_type = (kUWB_APPPARAMS_Type_u32), .param_value.vu32 = (VALUE), \
    }

/**
 *  \brief Macro to set GetApp Configuration parameters value supported in UWB API layer.
 */
#define UWB_SET_GETAPP_PARAM(PARAM)                                   \
    {                                                                 \
        .param_id = (PARAM), .param_type = (kUWB_APPPARAMS_Type_u32), \
    }

/**
 *  \brief Macro to set SetApp/GetApp Configuration parameters array value supported in UWB API layer.
 */
#define UWB_SET_APP_PARAM_ARRAY(PARAM, ARRAY, LENGTH)                                                        \
    {                                                                                                        \
        .param_id = (PARAM), .param_type = (kUWB_APPPARAMS_Type_au8), .param_value.au8.param_len = (LENGTH), \
        .param_value.au8.param_value = (uint8_t *)(ARRAY)                                                    \
    }

#define UWB_SET_VENDOR_APP_PARAM_VALUE UWB_SET_APP_PARAM_VALUE
#define UWB_SET_GETVENDOR_APP_PARAM    UWB_SET_GETAPP_PARAM
#define UWB_SET_VENDOR_APP_PARAM_ARRAY UWB_SET_APP_PARAM_ARRAY

/**
 *  \brief Set/Get App Configuration parameters list supported in UWB API layer.
 */
typedef struct UWB_DebugParams_List
{
    /** Input: search this tag */
    UWB_SR1XX_DBG_CFG_t param_id;
    /** Filled Implicitly: Expected type. */
    UWB_DebugParams_type_t param_type;
    /** Input: Parameter Value */
    UWB_DebugParams_value_t param_value;
} UWB_DebugParams_List_t;

/**
 *  \brief Macro to set Set Debug Configuration parameters value supported in UWB API layer.
 */
#define UWB_SET_DEBUG_PARAM_VALUE_u8(PARAM, VALUE)                                                 \
    {                                                                                              \
        .param_id = (PARAM), .param_type = (kUWB_DEBUGPARAMS_Type_u8), .param_value.vu8 = (VALUE), \
    }

#define UWB_SET_DEBUG_PARAM_VALUE_u16(PARAM, VALUE)                                                  \
    {                                                                                                \
        .param_id = (PARAM), .param_type = (kUWB_DEBUGPARAMS_Type_u16), .param_value.vu16 = (VALUE), \
    }

#define UWB_SET_DEBUG_PARAM_VALUE_u32(PARAM, VALUE)                                                  \
    {                                                                                                \
        .param_id = (PARAM), .param_type = (kUWB_DEBUGPARAMS_Type_u32), .param_value.vu32 = (VALUE), \
    }
/**
 *  \brief Macro to set Get Debug Configuration parameters value supported in UWB API layer.
 */
#define UWB_SET_GETDEBUG_PARAM_u8(PARAM)                               \
    {                                                                  \
        .param_id = (PARAM), .param_type = (kUWB_DEBUGPARAMS_Type_u8), \
    }
#define UWB_SET_GETDEBUG_PARAM_u16(PARAM)                               \
    {                                                                   \
        .param_id = (PARAM), .param_type = (kUWB_DEBUGPARAMS_Type_u16), \
    }
#define UWB_SET_GETDEBUG_PARAM_u32(PARAM)                               \
    {                                                                   \
        .param_id = (PARAM), .param_type = (kUWB_DEBUGPARAMS_Type_u32), \
    }

/**
 *   Device Configuration parameters supported in UWB API layer.
 */
typedef enum deviceConfig
{
    /** 0:DISABLE, 1:ENABLE */
    LOW_POWER_MODE = 0X01,
    /** DPD wakeup source
    * bit1: GPIO1, bit3: GPIO3 */
    DPD_WAKEUP_SRC = 0xE402,
    /** WTX count, 20>= wtx count <=120 */
    WTX_COUNT_CONFIG = 0xE403,
    /** DPD entry timeout in ms (default = 500ms)*/
    DPD_ENTRY_TIMEOUT = 0xE404,
/** This configuration is used to configure the wifi co-ex feature.
     *
     * Octet[0]: Enable/Disable wifi co-ex feature
     *      0x00 : To Disable(default)
     *      b3-b0: Enable/Disable functionality CoEx
     *              1: Enable CoEx Interface without Debug and without Warning Verbose
     *              2: Enable CoEx Interface with Debug Verbose only GPIO2 toggle status before start of
     *                  ranging round and end of ranging round  will be indicated via UWB_WIFI_COEX_IND_NTF
     *              3: Enable CoEx Interface with Warnings Verbose only UWB_WLAN_COEX_MAX_ACTIVE_GRANT_DUARTION_EXCEEDED_WAR_NTF
     *                  will be send when WLAN max grant duration is exceeded
     *              4: Enable CoEx Interface with both Debug and Warning Verbose
     *      b7-b4: CoEx Interface (GPIO/UART/One Wire) selection:
     *              0: GPIO Interface
     *              1: Uart Interface
     *              2: One Wire Interface(Proposal new)
     * Octet[1]: MIN_GUARD_DURATION. RFU for one wire interface, FW force this to 0 to manage internal scheduler logic.
     * Octet[2]: MAX_GRANT_DURATION / MAX_WIFI_BLOCK_DURATION(will be renamed for One Wire Co-ex)
     *              Maximum duration for which the UWB can request for medium access Default is 30ms,  Maximum = 255ms
     * Octet[3]: ADVANCED GRANT DURATION / GUARD DURATION(will be renamed for One Wire Co-ex).
     *              OneWire Co-ex - Guard Duration before which UWBS can perform Tx/Rx,
     *              applied before start of every co-ex session, default = 1ms, Minimum =1ms, Maximum = 255ms
     *
     * */
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    WIFI_COEX_FEATURE = 0xE405,
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    /** This configuration is used to indicate whether the RX Antenna selection should
     * take place w.r.t EF2 or GPIO14 (for Antenna switching)
     * 0 : EF2 based Antenna selection (Default)
     * 1 : GPIO14 based Antenna selection */
    RX_GPIO_ANTENNA_SELECTION = 0xE406,
    /** 0:DISABLE, 1:ENABLE */
    TX_BASE_BAND_CONFIG = 0xE426,
    /** DDFS tone config (4*8 bytes repeated for 4 channels)
    * 18 bytes value description:
    *     Octet[0]: channel number
    *     Octet[1]: Tx antenna selection. Possible values are 1 or 2.
    *     Octet[5:2]: Content of register TX_DDFS_TONE_0
    *     Octet[9:6]: Content of register TX_DDFS_TONE_1
    *     Octet[13:10]: Duration of the spur, in 124.8 MHz resolution (~ 8 ns)
    *     Octet[14]: Content of register GAINVAL_SET
    *     Octet[15]: Content of register DDFSGAINBYPASS_ENBL
    *     Octet[17:16]: Periodicity of spur in terms of gap interval in the PER command.
    * 4 Blocks: 18 Octets  repeated for each block
    *     Octets[17:0] correspond to Block1
    *     Octets[35:18] correspond to Block2
    *     Octets[53:36] correspond to Block3
    *     Octets[71:54] correspond to Block4 */
    DDFS_TONE_CONFIG = 0xE427,
    /** Preamble pulse shape setting
    *     Octet[0]: Preamble pulse shape id
    *     Octet[1]: Payload Tx pulse shape id
    *     Octet[2]: STS Tx pulse shape ID
    *     Octet[3]: DAC Stage Config
    *  Values:
    *     Octet[0-2] =  [2(default), 30, 34, 36 and 37]
    *     Octet[3] = Value is defined as below
    *       bit0: To set the DAC gain
    *          0: Unchanged( UWBS Keeps previous assigned value)  .
    *          1: UWBS set to 0x24
    *       bit1:  To set LPF( Tx DAC C)
    *          0: UWBS shall set to 0
    *          1: UWBS shall set to 0x5F
    *       bit7-bit2: RFU
    * */
    TX_PULSE_SHAPE_CONFIG = 0xE428,
    /**
     * Octet[0]: Clock source option
     * - b[0]: RF Clock option
     * - [0]: Use on board crystal (default)
     * - [1]: Use external Clock
     * - b[1]: Slow clock option
     * - [0]: Use on board crystal
     * - [1]: Use external 32.768 KHz Clock
     * - b[2:7]: RFU
     * Octet[1]: Crystal Option for RF clock
     * - b[0]: Crystal Option
     * - [0]: Use 38.4MHz crystal (default)
     * - [1]: Use 26 MHz crystal
     * - b[1:7]: RFU
     */
    CLK_CONFIG_CTRL = 0xE430,
    /** Parameter is used to set host capability of handling max UCI payload.
     *  FW shall use this parameter to send the UCI responses/notification to host.
     *  FW shall use PBF bit if UCI payload goes more than HOST_MAX_UCI_PAYLOAD_LENGTH size.
     *  255<= HOST_MAX_UCI_PAYLOAD_LENGTH<= UWBS_MAX_UCI_PAYLOAD_LENGTH(capability parameter)
     *  (Default = 255) */
    HOST_MAX_UCI_PAYLOAD_LENGTH = 0xE431,
    /** 0x00 = FIRA generic Response/Notification (Default)
    0x01 = Vendor extended Response/Notification */
    NXP_EXTENDED_NTF_CONFIG = 0xE433,
    /** Maximum waiting time until clock is present. The value is given in microseconds and
     *  defaults to 1000us.
     *  If Octet [0] of CLK_CONFIG_CTRL is set to 0 (on board crystal), this time is indicating
     *  the maximum waiting time until XTAL oscillator becomes stable.
     *  If Octet [0] of CLK_CONFIG_CTRL is set to 1 (external clock), this time is indicating
     *  the guard time between clock request GPIO (GPIO1) going high until the platform has
     *  to provide a stable clock.
     */
    CLOCK_PRESENT_WAITING_TIME = 0xE434,
    /** Negative offset when Rx should be enabled to receive the first message
    of a ranging round on Controlee compared to expected reception time. Default = 100 us */
    INITIAL_RX_ON_OFFSET_ABS = 0xE435,
    /** Negative offset when Rx should be enabled to receive the first message
    of a ranging round on Controlee compared to expected reception time. Default = 100 ppm */
    INITIAL_RX_ON_OFFSET_REL = 0xE436,

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    /**
     * UART based WiFi-CoEx Interface User Configuration.
     * Default UWB WLAN Coex-Config : 0x01
     * Valid ranging 1 to 255 (seconds): The Home channel information request and Band
     * channel information requests will be sent by UWB after specified duration.
     */
    WIFI_COEX_UART_USER_CFG = 0xE437,
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    /** PdoA Calibration Table Definition :
     *  Octet[0] - Calibration Step Size : The calibration table step size in degrees which
     *  indicates the step size between two consecutive points.
     *  Allowed Range : 10° to 15° (default = 12°)
     *
     *  Example : For maximum span of 60°, with step size 12° then the calibration table would
     *  look like[-60°, -48°, -36° , ..., 0°, ..., 36°, 48°, 60°].
     *  Octet [1] - Number of Steps: The number of calibration steps needs to be an odd
     *  number.
     *
     *  Allowed Range : 3<=M<=21(to include 0°)(default = 11)
     *  Example : With number of step size as 10° and number of steps as 13, the achieved
     *  calibration span is -60, -50, -40, ...,  -10, 0, 10, ..., 40, 50, 60
     *  Note:
     *
     *  Total Calibration Span = ((Number of Steps - 1) * Step Size). Example :
     *  If steps are 11 and step size is 12° then total span is 120°(-60° to 60° including 0°).
     *  If steps are 13 and step size is 15° then total span is 180°(-90° to 90° including 0°)
     */
    PDOA_CALIB_TABLE_DEFINE = 0xE446,
    /**
     * To define/create antenna identifier for RX
     * Octet[0] : Number of Entries. (N). This must be equal to "MAX_N".
     *
     * Array of entries:
     * Octet[X + 0] : RX Antennae ID.
     * Index of the Antennae.
     * Value 0 shall be Invalid.
     * Value shall from 1 to MAX_N
     *
     * Octet[X + 2, X + 1]: GPIO Filter Mask
     * This mask defines which GPIOs shall be changed during the state transition,
     * if a GPIO bit is set to 0 the GPIO shall not change its state.
     * It's a 2 byte value in Little Endian format.
     *
     * Octet[X + 4, X + 3]: GPIO State / Value
     * This mask defines the GPIO state, if the corresponding GPIO bit is 0 in the GPIO Filter mask,
     * the state shall be ignored and not changed
     * It's a 2 byte value in Little Endian format.
     *
     */
    ANTENNA_RX_IDX_DEFINE = 0xE460,
    /**
     * To define/create antenna identifier for TX
     *
     * Octet[0] : N Entries.
     *
     * Array of entries:
     * In case, same Antenna is used for Tx/Rx Use the same ID.
     * In case few GPIOs have to be switched for toggling,
     * Octet[1] and Octet[2] would be different when TX/RX antennae are defined.
     *
     * Octet[0] : Antennae ID.
     * Index of the Antennae.
     * Value 0 shall be Invalid.
     * Value shall from 1 to N.
     *
     * Octet[1]: GPIO Filter Mask
     * This mask defines which GPIOs shall be changed during the state transition,
     * if a GPIO bit is set to 0 the GPIO shall not change its state.
     *
     * Octet[2]: GPIO State / Value
     * This mask defines the GPIO state, if the corresponding GPIO bit is 0 in the GPIO Filter mask,
     * the state shall be ignored and not changed
     *
     * Octet[6:3] Group Delay
     *
     * Max number of entries per product variant for max value of N is same as ANTENNA_RX_IDX_DEFINE
     * For Calibration of TX Power, use TX_POWER from Calibration Parameters
     * */
    ANTENNA_TX_IDX_DEFINE = 0xE461,
    /** To define/create antenna identifier for RX Pair
     *
     * Octet[0] : N Entries.
     *
     * Array of entries:
     * This may be an H or V Combination. Repeat of [
     * Octet[0] : Antennae PAIR ID
     * This IDx is used along with ANTENNAE_CONFIGURATION(Session Config) and for reporting.
     * ID 0 shall not be used.
     * For non-scanning mode/for backward compatibility, Default ID for Horizontal has to be 1.
     * And ID for Vertical has to be 2.
     * Use Odd Values for Horizonal Pair And Even Values for Vertical Paris.
     * So, FW can decide to move to lower or higher Pair ID.
     * FW can also use IDs X, X+1 to identify which ID is making a group for 3D Configuration.
     *
     * Octet[1] : RX1 Antennae ID as defined by ANTENNAE_RX_IDX_DEFINE
     *
     * Octet[2] : RX2 Antennae ID as defined by ANTENNAE_RX_IDX_DEFINE.
     *
     * Octet[4:3]: PDOA Zero Offset
     * - 2 bytes PDOA1_OFFSET, PDOA2_OFFSET
     *
     * Octet[6:5] : Relative Angle of View / Field of View.
     * Assuming we have 4 Antennae pairs symmetrically placed on a UWB System.
     * For a given selected Antennae pair, the peer object may be at 0°,
     * but reported AoA has to be either 0°, 90°, 180° or 270° depending on this relative Angle of View.
     *
     * When this is set to 0, then Host has to use RX Antennae info and derive the relative angle.
     * When this is set to 0, the FW has no IDEA which antennae group to switch to in case of antennae
     * moves out of some configuration.
     *
     * Max number of entries per product variant for max value of N is same as ANTENNA_RX_IDX_DEFINE
     * */
    ANTENNAE_RX_PAIR_DEFINE = 0xE462,
/** To select the WIFI Co-ex channel
     *
     * b0: Channel 5, Set to 1 enable Wifi Co-ex on Channel 5
     * b1: Channel 6, Set to 1 enable Wifi Co-ex on Channel 6
     * b2: Channel 8, Set to 1 enable Wifi Co-ex on Channel 8
     * b3: Channel 9, Set to 1 enable Wifi Co-ex on Channel 9
     * b4:b7 - RFU
     * 0x0F, enable co-ex for all channels i.e ch5,ch6,ch8 and ch9
     *
     * */
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    WIFI_CO_EX_CH_CFG = 0xE464,
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

    /** End of device Configs*/
    END_OF_SUPPORTED_DEVICE_CONFIGS,
} eDeviceConfig;

/** Antenna Configuration and slection mode */
typedef enum
{
    /* For Time Of Arrival */
    kUWBAntCfgRxMode_ToA_Mode = 0,
    /* For Angle Of Arrival */
    kUWBAntCfgRxMode_AoA_Mode = 1,
    /** For Rx RADAR */
    kUWBAntCfgRxMode_Radar_Mode,
    /**For Rx TOA RFM*/
    kUWBAntCfgRxMode_ToA_Rfm_Mode,
    /**For Rx AOA RFM*/
    kUWBAntCfgRxMode_AoA_Rfm_Mode,
    /** ToA mode for CSA */
    kUWBAntCfgRxMode_CSA_ToA_Mode,
    /** AoA mode for CSA */
    kUWBAntCfgRxMode_CSA_AoA_Mode,

} kUWBAntCfgRxMode_t;

/**
 * \brief  Structure lists out the UWB Device Info Parameters.
 */
typedef struct phUwbDevInfo
{
    /** Mac Major version */
    uint8_t macMajorVersion;
    /** Mac Minor version */
    uint8_t macMinorMaintenanceVersion;
    /** Phy Major version */
    uint8_t phyMajorVersion;
    /** Phy Minor version */
    uint8_t phyMinorMaintenanceVersion;
    /** Device Name length*/
    uint8_t devNameLen;
    /** Device Name */
    uint8_t devName[48];
    /** Fw Major Version */
    uint8_t fwMajor;
    /** Fw Minor Version */
    uint8_t fwMinor;
    /** Fw Rc Version */
    uint8_t fwRc;
    /** NXP UCI Major Version */
    uint8_t nxpUciMajor;
    /** NXP UCI Minor Version */
    uint8_t nxpUciMinor;
    /** NXP UCI Patch Version */
    uint8_t nxpUciPatch;
    /** NXP Chip Id */
    uint8_t nxpChipId[MAX_UWB_CHIP_ID_LEN];
    /** Max PPM Value*/
    uint8_t maxPpmValue;
    /** TX Power Value*/
    int16_t txPowerValue[MAX_TX_POWER_LEN];
    /** MW Major Version */
    uint8_t mwMajor;
    /** MW Minor Version */
    uint8_t mwMinor;
    /** Mw Rc Version */
    uint8_t mwRc;
    /** NXP FIRA UCI generic major version */
    uint8_t uciGenericMajor;
    /** NXP FIRA UCI generic minor version */
    uint8_t uciGenericMinorMaintenanceVersion;
    /** NXP FIRA UCI generic patch version */
    uint8_t uciGenericPatch;
    /** NXP FIRA UCI test major version */
    uint8_t uciTestMajor;
    /** NXP FIRA UCI test minor version */
    uint8_t uciTestMinor;
    /** NXP FIRA UCI test patch version */
    uint8_t uciTestPatch;
    /** Fw Boot Mode */
    uint8_t fwBootMode;
#if UWBFTR_CCC
    /** UCI CCC VERSION */
    uint8_t uciCccVersion[MAX_UCI_CCC_VERSION_LEN];
    /** CCC VERSION */
    uint8_t cccVersion[MAX_CCC_VERSION_LEN];
#endif // UWBFTR_CCC
} phUwbDevInfo_t;

/**
 *  \brief OTP Read Write Configuration parameters supported in UWB API layer.
 */
typedef enum otpParam_Type
{
    /** 2 bytes of Module maker info */
    kUWB_OTP_ModuleMakerInfo = 0x00,
} eOtpParam_Type_t;

/**
 *  \brief Calibrations Configuration parameters supported in UWB API layer.
 */
typedef enum calibParam
{
    /** VCO_PLL calibration - Channel dependent */
    VCO_PLL = 0x00,
    /** RF_CLK_ACCURACY_CALIB - Channel independent */
    RF_CLK_ACCURACY_CALIB = 0x01,
    /** Delay Calibration for each RX Antenna - Channel dependent */
    RX_ANT_DELAY_CALIB = 0x02,
    /** PDOA Offset Calibration - Channel dependent */
    PDOA_OFFSET_CALIB = 0x03,
    /** TX_POWER - Channel dependent */
    TX_POWER_PER_ANTENNA = 0x04,
    /** MANUAL_TX_POW_CTRL */
    MANUAL_TX_POW_CTRL = 0x60,
    /** TX PA and PPA setting */
    PA_PPA_CALIB_CTRL = 0x61,
    /** PDOA Calibration tables */
    AOA_ANTENNAS_PDOA_CALIB = 0x62,
    /** Multi point ('N') PDoA manufacturing offset calibration */
    AOA_ANTENNAS_MULTIPOINT_CALIB = 0x63,
    /** Zero offset Manufacturing PDOA Calibration */
    PDOA_MANUFACT_ZERO_OFFSET_CALIB = 0x65,
    /** AoA Threshold PDOA */
    AOA_THRESHOLD_PDOA = 0x66,
    /** TX temperature compensation per antenna */
    TX_TEMPERATURE_COMP_PER_ANTENNA = 0x67,
    /** SNR Calibration per RX Antenna - Channel dependent */
    SNR_CALIB_CONSTANT_PER_ANTENNA = 0x68,
    /** RSSI Offset per RX Antenna for High Power - Channel dependent */
    RSSI_CALIB_CONSTANT_HIGH_PWR = 0x69,
    /** RSSI Offset per RX Antenna for Lower Power - Channel dependent */
    RSSI_CALIB_CONSTANT_LOW_PWR = 0x6A,
#if UWBIOT_UWBD_SR150
    /** This parameter is used to set the PDoA Calibration Table for 90 Fov*/
    AOA_ANTENNAS_PDOA_CALIB_EXTENDED_SUPPORT = 0xE000,
#endif // UWBIOT_UWBD_SR150
} eCalibParam;

/* Deprecated Enums
 * TODO: Will be removed in future releases
 */
#define AOA_ANTENNAE_PDOA_CALIB AOA_ANTENNAS_PDOA_CALIB

/**
 *  \brief Calibrations Configuration parameters supported in UWB API layer.
 */
typedef enum otpCalibParam
{
    /** VCO PLL Calibration, channel dependent */
    OTP_CALIB_VCO_PLL = 0x00,
    /** TX POWER Calibration, channel and antenna dependent */
    OTP_CALIB_TX_POWER = 0x01,
    /** 38.4MHz XTAL, channel and antenna Independent */
    OTP_CALIB_RF_XTAL_CAP = 0x02,
    /** RSSI Calibration, channel and antenna dependent */
    OTP_CALIB_RSSI_CALIB_CONST1 = 0x03,
    /** RSSI Calibration, channel and antenna dependent */
    OTP_CALIB_RSSI_CALIB_CONST2 = 0x04,
    /** power control parameters */
    OTP_CALIB_MANUAL_TX_POW_CTRL = 0x06,
    /** PA_PPA Calibration control */
    OTP_CALIB_PAPPPA_CALIB_CTRL = 0x08,
    /** Tx Temperature Compensation */
    OTP_CALIB_TX_TEMPARATURE_COMP = 0x09,
    /** Delay calibration value to adjust the distance measurement from ranging*/
    OTP_CALIB_DELAY_CALIB = 0x0B,
    /** PDOA mfg zero offset calibration*/
    OTP_PDOA_MFG_ZERO_OFFSET_CALIB = 0x0C,
    /** Point calibration entries*/
    OTP_AOA_ANT_MULTIPOINT_CALIB = 0x0D,
} eOtpCalibParam;

/**
 * Calibration integrity protection tag options.
 */

typedef enum calibTagOption
{
    /** Device Specific tag option */
    DEVICE_SPECIFIC = 0x0,
    /** Model Specific tag option */
    MODEL_SPECIFIC
} eCalibTagOption;

/**
 * \brief  Calibration Parameter States
 */
typedef enum
{
    /** Calibration parameter carries default value by UWBS */
    DEFAULT = 0x00,
    /** Calibration parameter is not integrity protected either by a
     *  Device specific or Model specific authentication tag. The parameter
     *  is applied in UWBS for usage.
     */
    CUSTOM_NOT_INTEGRITY_PROTECTED = 0x01,
    /** Calibration parameter integrity check is pending either by a Device
     *  specific or Model specific tag. Mainline FW: Until the tag verification
     *  finishes, the parameter will not be applied
     */
    CUSTOM_AUTH_PENDING = 0x02,
    /** Calibration parameter integrity check is verified successfully by a
     *  Device specific tag.
     */
    CUSTOM_DEVICE_SPECIFIC_TAG_AUTHENTICATED = 0x03,
    /** Calibration parameter integrity check is verified successfully by a
     *  Model specific tag
     */
    CUSTOM_MODEL_SPECIFIC_TAG_AUTHENTICATED = 0x04,
} eCalibState;

/**
 * \brief  Structure lists out the calibration command response/notification.
 */
typedef struct phCalibRespStatus
{
    /** Status */
    uint8_t status;
    /** Calibration State in case of get_calib command
     * and not used in case of do_calib command */
    eCalibState calibState;
    /** Calibration antenna pair only for aoa antennae pdoa calib*/
    uint8_t inRxAntennaPair;
    /** Calibration value out */
    uint8_t calibValueOut[MAX_API_PACKET_SIZE];
    /** Calibration value length */
    uint16_t length;
} phCalibRespStatus_t;

/**
 * \brief  Structure lists out the Generate Tag command response/notification.
 */

typedef struct phGenerateTagRespStatus
{
    /** Status */
    uint8_t status;
    /** CMAC Tag */
    uint8_t cmactag[UWB_TAG_CMAC_LENGTH];
} phGenerateTagRespStatus_t;

/**
 * \brief  Structure lists out the blink command response/notification.
 */
typedef struct phBlinkRespStatus
{
    /** repetition count status */
    uint8_t repetition_count_status;
} phBlinkRespStatus_t;

#if (UWBFTR_SE_SN110)
/**
 * \brief UWBD Type for SE_DO_BIND Notification.
 */
typedef struct phSeDoBindStatus
{
    /** Binding status*/
    /** 0x00: Not Bound,*/
    /** 0x01: Bound Unlocked,*/
    /** 0x02: Bound Locked,*/
    /** 0x03: Unknown ( if any error occurred during getting binding state from SE) */
    uint8_t status;
    /** Remaining Binding Count */
    uint8_t count_remaining;
    /** Binding state */
    uint8_t binding_state;
    /** command for which SE communication is failed (Itshall indicate last APDU while binding procedure) */
    uint16_t se_instruction_code;
    /** status codes (SW1 SW2) */
    uint16_t se_error_status;
} phSeDoBindStatus_t;

/**
 * \brief UWBD Type for SE_GET_BINDING_STATUS Notification.
 */
typedef struct phSeGetBindingStatus
{
    /** Binding status*/
    /** 0x00: Not Bound,*/
    /** 0x01: Bound Unlocked,*/
    /** 0x02: Bound Locked,*/
    /** 0x03: Unknown ( if any error occurred during getting binding state from SE) */
    uint8_t status;
    /** Remaining binding count in SE */
    uint8_t se_binding_count;
    /** Remaining binding count in uwb device */
    uint8_t uwbd_binding_count;
    /** command for which SE communication is failed (Itshall indicate last APDU while binding procedure) */
    uint16_t se_instruction_code;
    /** status codes (SW1 SW2) */
    uint16_t se_error_status;
} phSeGetBindingStatus_t;

/**
 * \brief UWBD Type for UWB_ESE_CONNECTIVITY_CMD.
 */
typedef struct SeConnectivityStatus
{
    /** 0x00 : Success */
    /** 0x01 : SE Error */
    /** 0x02 : Time-out */
    /** 0x03 : I2C interface error between UWB and eSE */
    /** 0x04 : No Applet in eSE */
    /** 0x74 : APDU command is rejected by eSE */
    /** 0x75 : Authentication to eSE failed */
    /** 0x76 : I2C write fail */
    /** 0x77 : I2C Read fail with IRQ low */
    /** 0x78 : I2C Read fail with IRQ high */
    /** 0x79 : I2C timeout */
    /** 0x7A : I2C write time out with IRQ high */
    uint8_t status;
    /** command for which SE communication is failed (Itshall indicate last APDU while binding procedure) */
    uint16_t se_instruction_code;
    /** status codes (SW1 SW2) */
    uint16_t se_error_status;
} SeConnectivityStatus_t;

#endif // (UWBFTR_SE_SN110)
/**
 * \brief UWBD Type for URSK_DELETION_REQ Notification.
 */

typedef struct
{
    /** Session Handle */
    uint32_t sessionHandle;
    /** Status */
    uint8_t status;
} phSessionHandleList_t;

/**
 * \brief UWBD Type for URSK_DELETION_REQ Notification.
 */
typedef struct
{
    /** Status */
    uint8_t status;
    /** No of Session Handles */
    uint8_t noOfSessionHandles;
    /** Session Handle list */
    phSessionHandleList_t *sessionHandleList;
} phUrskDeletionRequestStatus_t;

/**
 * \brief UWBD Type for SE_GET_BINDING_COUNT_RSP.
 */
typedef struct hSeGetBindingCount
{
    /** Binding Status */
    uint8_t bindingStatus;
    /** Remaining binding count in uwb device */
    uint8_t uwbdBindingCount;
    /** Remaining binding count in SE.
     *
     * @warning for SR150, this field has to be 0.
     */
    uint8_t seBindingCount;
} phSeGetBindingCount_t;

#if UWBFTR_UWBS_DEBUG_Dump
/**
 * \brief  Structure lists out the additional rframe ranging notification
 * information.
 */
typedef struct phRframeData
{
    /** Data Length */
    uint16_t dataLength;
    /** Data */
    uint8_t data[MAX_RFRAME_PACKET_SIZE];
} phRframeData_t;

/**
 * \brief  Structure lists out the debug notification information.
 */
typedef struct phDebugData
{
    /** Data Length */
    uint16_t dataLength;
    /** Data */
    uint8_t data[MAX_DEBUG_NTF_SIZE];
} phDebugData_t;
#endif // UWBFTR_UWBS_DEBUG_Dump

/**
 * \brief  Structure lists out the scheduler status notification information.
 */
typedef struct phSchedStatusNtfData
{
    /** Data Length */
    uint16_t dataLength;
    /** Data */
    uint8_t data[MAX_UCI_PACKET_SIZE];
} phSchedStatusNtfData_t;

/**
 * \brief  Structure lists out the SE test loop information.
 */
typedef struct phSeTestLoopData
{
    /** Status */
    uint8_t status;
    /** No of times loop was run */
    uint16_t loop_cnt;
    /** No of times loop successfully completed */
    uint16_t loop_pass_count;
} phTestLoopData_t;

/**
 * \brief  Structure lists out the SE Comm Error notification.
 */
typedef struct phSeCommError
{
    /** Status */
    uint8_t status;
    /** T=1 command for which SE communication is failed. */
    uint16_t cla_ins;
    /** T=1 status codes(SW1SW2) */
    uint16_t t_eq_1_status;
} phSeCommError_t;

/**
 * channel for aoa fine calibration, supported parameters
 */
typedef enum phChannel
{
    /** channel 5*/
    CH_5 = 5,
    /**channel 9*/
    CH_9 = 9
} ephChannel;

/**
 * Antenna pair for aoa fine calibration, supported parameters
 */
typedef enum antPair
{
    ANT_1 = 1,
    ANT_2 = 2
} eAntPair;

/**
 * \brief  Structure lists out the pdoa table define config.
 */
typedef struct phPdoaTableDef
{
    /** The calibration table step size in degrees which indicates the step size between two consecutive points */
    uint8_t calibStepSize;
    /* The number of calibration steps needs to be an odd number. */
    uint8_t noSteps;
} phPdoaTableDef_t;

/**
 * \brief Clock config parameters
 */
typedef struct clkConfigSrc
{
    /** Clock source option */
    uint8_t clk_src_opt;
    /** Crystal option for RF clock */
    uint8_t xtal_opt;
} phClkConfigSrc_t;

/**
 * \brief  Structure lists out the ddfs tone config.
 */
typedef struct phDdfsToneConfig
{
    /** channel no */
    uint8_t channel_no;
    /** Tx antenna selection */
    uint8_t tx_antenna_selection;
    /** content of TX_DDFS_TONE_0 */
    uint32_t tx_ddfs_tone_0;
    /** content of TX_DDFS_TONE_1 */
    uint32_t tx_ddfs_tone_1;
    /** spur duration */
    uint32_t spur_duration;
    /** value of GAINVAL_SET */
    uint8_t gainval_set;
    /** content of DDFSGAINBYPASS_ENBL */
    uint8_t ddfsgainbypass_enbl;
    /** periodicity */
    uint16_t periodicity;
} phDdfsToneConfig_t;

/**
 * \brief  Structure lists out the aoa Calibration average threshold.
 */
typedef struct phAoACalibCtrlAvgThreshPdoa
{
    /** avg threshold pdoa for all angle sweeps */
    uint16_t avg_thresh_pdoa[NO_OF_CALIB_PAIRS];
} phAoACalibCtrlAvgThreashPdoa_t;

/**
 * \brief  Structure for antenna Identifier defines
 */
typedef struct phAntennaDefines
{
    /* Buffer should includes the number of entries */
    uint8_t *antennaDefs;
    uint8_t antennaDefsLen;
} phAntennaDefines_t;

/**
 * \brief Device config data
 */
typedef union phDeviceConfig {
    /** low power mode */
    uint8_t lowPowerMode;
    /** DPD wakeup src */
    uint8_t dpdWakeupSrc;
    /** WTX count config */
    uint8_t wtxCountConfig;
    /** DPD entry timeout */
    uint16_t dpdEntryTimeout;
    /** DDFS tone config enable */
    uint8_t txBaseBandConfig;
    /** Anttena selection Config*/
    uint8_t rxAntennaSelectionConfig;
    /** nxp extended ntf config */
    uint8_t nxpExtendedNtfConfig;
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    /** WiFi CoEx Feature Confg */
    UWB_WiFiCoEx_Ftr_t wifiCoExFtr;
    /** WiFi CoEx Channel confg */
    uint8_t wifiCoExChannelCfg;
    /** UART based WiFi-CoEx Interface User Configuration */
    uint8_t wifiCoexUartUserCfg;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    /** DDFS Tone confg */
    phDdfsToneConfig_t ddfsToneConfig[NO_OF_CHANNELS];
    /** Tx Telec config */
    phTxPulseShapeConfig_t txPulseShapeConfig;
    /** Host Max UCI Payload Len */
    uint16_t hostMaxUCIPayloadLen;
    /* PdoA Calibration Table Definition */
    phPdoaTableDef_t pdoaCalibTableDef;
    /** PDoA threshold values for all the calib pairs*/
    phAoACalibCtrlAvgThreashPdoa_t aoaCalibThresholdPdoa;
    /** Negative offset when Rx should be enabled to receive the first message
    of a ranging round on Controlee compared to expected reception time. Default = 100 us */
    uint16_t initialRxOnOffsetAbs;
    /** Negative offset when Rx should be enabled to receive the first message
    of a ranging round on Controlee compared to expected reception time. Default = 100 ppm */
    uint16_t initialRxOnOffsetRel;
    /** To Define/Create all antenna Identifier for RX/TX/RX Pair */
    phAntennaDefines_t antennaDefines;
    /** Clock config parameters */
    phClkConfigSrc_t clockConfigCtrl;
    /** Maximum waiting time until clock is present */
    uint16_t clockPresentWaitingTime;
} phDeviceConfigData_t;

#if (UWBFTR_DL_TDoA_Anchor || UWBFTR_DL_TDoA_Tag)
/**
 * \brief Structure for storing Active round config Context.
 */
typedef struct phActiveRoundsConfig
{
    /** Active Round Index */
    uint8_t roundIndex;
    /** Device role within the given round index. See \ref UWB_DeviceRole_t */
    uint8_t rangingRole;
    /** Number M of Responder MAC Addresses, Possible values are between 1 to 8.*/
    uint8_t noofResponders;
    /** Responder MAC Address List for the specified ranging round as Initiator DT-Anchor.*/
    uint8_t *responderMacAddressList;
    /** Responder slot presence
        Possible values are:
        0x00: implicit scheduling, i.e., responder slots are not present;
        0x01: responder slots are present; therefore, M octets shall follow, specifying the assigned slot for each Responder DT-Anchor.
        0x02-0xFF: RFU */
    uint8_t responderSlotScheduling;
    /** Responder slot index assigned for responder transmissions */
    uint8_t *responderSlots;

} phActiveRoundsConfig_t;

/**
 * \brief Structure for storing active round config rsp data in case of #UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED.
 */
typedef struct phNotActivatedRounds
{
    /** No of index */
    uint8_t noOfIndex;
    /** Index list */
    uint8_t indexList[MAX_NO_OF_ACTIVE_RANGING_ROUND];

} phNotActivatedRounds_t;
#endif //(UWBFTR_DL_TDoA_Anchor || UWBFTR_DL_TDoA_Tag)

/**
 * \brief Structure for storing Firmware Crash Info, allocate pLog buffer with max response
 *
 * length value and update the logLen filed
 */
typedef struct phFwCrashLogInfo
{
    uint8_t *pLog;
    size_t logLen;
} phFwCrashLogInfo_t;

#if UWBIOT_UWBD_SR150
/**
 * \brief Structure for storing fields of Session Set Localization Zone Command.
 */
typedef struct phSessionSetLocZone
{
    /** Session Handle of the Session for which localization zone is estimated. */
    uint32_t setLocZone_SessionHandle;
    /** Ranging Block Index for which localization zone is estimated. */
    uint16_t setLocZone_RangingBlockIndex;
    /** The estimated localization zone.
     * \ref eLocalizationZone_t
    */
    uint8_t setLocZone_LocZone;
} phSessionSetLocZone_t;

/**
 * \brief Enumurator lists out the possible values of Localization Zones
 */
typedef enum localizationZone
{
    eLocZone_Undefined = 0x00,
    eLocZone_Frontside = 0x01,
    eLocZone_Backside  = 0x02,
} eLocalizationZone_t;
#endif // UWBIOT_UWBD_SR150

/** @}  */ // uwb_apis_sr1xx

#endif // UWBAPI_TYPES_PROPRIETARY_SR1XX_H
