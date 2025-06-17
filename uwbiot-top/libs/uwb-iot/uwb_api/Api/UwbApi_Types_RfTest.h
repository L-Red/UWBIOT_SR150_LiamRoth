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

#ifndef UWBAPI_TYPES_RF_TEST_H
#define UWBAPI_TYPES_RF_TEST_H

#include "UwbApi_Types.h"

/**
 *  \brief Types used by UWB RX and TX Performance Tests
 */

/** @addtogroup Uwb_RfTest_Types
 *
 * @{ */

/**
 *  \name Test Configuration parameters supported in UWB API layer.
 */
/* @{ */
typedef enum testConfig
{
    /** NUM_OF_PACKETS */
    NUM_PACKETS,
    /** T_GAP */
    T_GAP,
    /** T_START */
    T_START,
    /** T_WIN */
    T_WIN,
    /** RANDOMIZE_PSDU */
    RANDOMIZE_PSDU,
    /** PHR_RANGING_BIT */
    PHR_RANGING_BIT,
    /** RMARKER_TX_START */
    RMARKER_TX_START,
    /** RMARKER_RX_START */
    RMARKER_RX_START,
    /** STS_INDEX_AUTO_INCR */
    STS_INDEX_AUTO_INCR,
    /** 0:Calibration level 1, 1: Calibration level 2 */
    RSSI_CALIBRATION_OPTION = 0xE501,
    /** AGC gain setting of Rx, applicable only when
     * RSSI_CALIBRATION_OPTION = 0. */
    AGC_GAIN_VAL_RX,
    /**Chosse Between the IEEE or Proprietary
      * 0:Proprietary option for key, data values
      * 1:IEEE option for key (default)
      * bit[1-7]: RFU */
    TEST_SESSION_STS_KEY_OPTION,
} eTestConfig;
/* @} */

/**
 * \brief  Structure lists out the mandatory configurations to be set for PER RF
 * test.
 */
/* @{ */
typedef struct phRfTestParams
{
    /** Start time of TX in 124.8MHz snapshot ticks */
    uint32_t rmarkerTxStart;
    /** Start time of RX in 124.8MHz snapshot ticks */
    uint32_t rmarkerRxStart;
    /** num Of Pckts */
    uint32_t numOfPckts;
    /** t Gap, tGap >> packet len */
    uint32_t tGap;
    /** t Start in us */
    uint32_t tStart;
    /** t Win, tWin > TStart */
    uint32_t tWin;
    /** randomized Size, 0: No randomization, 1: Take 1st
     * byte of PSDU data as seed */
    uint8_t randomizedSize;
    /** This configures Ranging bit field of PHR in both BPRF and HPRF modes.
     * 0x00 - Disable(default) 0x01 - Enable */
    uint8_t phrRangingBit;
    /** Sts Index Auto Increment, 0x00: fixed sts index
     * value, 0x01: increment for every frame in PER */
    uint8_t stsIndexAutoIncr;

} phRfTestParams_t;
/* @}*/

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/**
 * \brief  Structure lists out the vendor specific data of TEST_PER_RX_NTF, TEST_RX_NTF & TEST_LOOPBACK_NTF.
 * test.
 */
/* @{ */
typedef struct
{
    /** RX mode*/
    uint8_t rx_mode;
    /** Number of RX antenna to follow*/
    uint8_t no_of_rx_antenna;
    /** RX antenna pair*/
    uint8_t rx_antenna_id[MAX_NUM_ANTENNA_PAIR];
    /** Average value of the computed RSSI value on RxN path */
    int16_t rssi_rx[MAX_NUM_ANTENNA_PAIR];
    /** Average value of the computed SNR value on RxN path. */
    int16_t snr_rx[MAX_NUM_ANTENNA_PAIR];
} phRfVendorData_t;
/* @}*/
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT

/**
 * \brief  Structure lists out the TEST_PER_RX_NTF.
 */
/* @{ */
typedef struct
{
    /** Status */
    uint8_t status;
    /** No. of RX attempts */
    uint32_t attempts;
    /** No. of times signal was detected */
    uint32_t acq_Detect;
    /** No. of times signal was rejected */
    uint32_t acq_Reject;
    /** No. of times RX did not go beyond ACQ stage */
    uint32_t rxfail;
    /** No. of times sync CIR ready event was received */
    uint32_t sync_cir_ready;
    /** No. of time RX was stuck at either ACQ detect or sync CIR ready */
    uint32_t sfd_fail;
    /** No. of times SFD was found */
    uint32_t sfd_found;
    /** No. of times PHR decode failed */
    uint32_t phr_dec_error;
    /** No. of times PHR bits in error */
    uint32_t phr_bit_error;
    /** No. of times PHR decode failed */
    uint32_t psdu_dec_error;
    /** No. of times payload bits in error */
    uint32_t psdu_bit_error;
    /** STS Found */
    uint32_t sts_found;
    /** No. of times end of frame event was triggered */
    uint32_t eof;
    /** Vendor specific data len */
    uint16_t vs_data_len;
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    /** Vendor specific data type */
    uint8_t vs_data_type;
    /** Vendor specific data */
    phRfVendorData_t vs_data;
    /** Average value of the computed carrier frequency offset estimate from the reception */
    int16_t rx_cfo_est;
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
} phTestPer_Rx_Ntf_t;
/* @} */

/**
 * \brief  Structure lists out the TEST_RX_NTF.
 */
/* @{ */
typedef struct
{
    /** Status */
    uint8_t status;
    /** Integer part of timestamp 1/124.8Mhz ticks. */
    uint32_t rx_done_ts_int;
    /** Fractional part of timestamp in 1/128 * 499.2Mhz ticks */
    uint16_t rx_done_ts_frac;
    /** Angle of Arrival in degrees for Preamble  */
    int16_t aoa_azimuth;
    /** AoA Elevation in degrees and it is a signed value in Q9.7 format.
     * This field is zero if AOA_RESULT_REQ = 0.  */
    int16_t aoa_elevation;
    /* ToA of main path minus ToA of first path in nanoseconds */
    uint8_t toa_gap;
    /** Received PHR (bits 0-12 as per IEEE spec) */
    uint16_t phr;
    /** psdu data length */
    uint16_t psdu_len;
    /** Received PSDU Data[0:N] bytes. Must be allocated by the application */
    uint8_t *pPsdu;
    /** Vendor specific data len */
    uint16_t vs_data_len;
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    /** Vendor specific data type */
    uint8_t vs_data_type;
    /** Vendor specific data */
    phRfVendorData_t vs_data;
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
} phTest_Rx_Ntf_t;
/* @} */

/**
 * \brief  Structure lists out the TEST_LOOPBACK_NTF.
 */
/* @{ */
typedef struct
{
    /** Status */
    uint8_t status;
    /** Integer part of timestamp 1/124.8Mhz ticks. */
    uint32_t tx_ts_int;
    /** Fractional part of timestamp in 1/128 * 499.2Mhz ticks */
    uint16_t tx_ts_frac;
    /** Integer part of timestamp 1/124.8Mhz ticks. */
    uint32_t rx_ts_int;
    /** Fractional part of timestamp in 1/128 * 499.2Mhz ticks */
    uint16_t rx_ts_frac;
    /** Angle of Arrival in degrees for Preamble  */
    int16_t aoa_azimuth;
    /** AoA Elevation in degrees and it is a signed value in Q9.7 format.
     * This field is zero if AOA_RESULT_REQ = 0.  */
    int16_t aoa_elevation;
    /** Received PHR (bits 0-12 as per IEEE spec) */
    uint16_t phr;
    /** psdu data length */
    uint16_t psdu_len;
    /** Received PSDU Data[0:N] bytes. Must be allocated by the application */
    uint8_t *pPsdu;
    /** Vendor specific data len */
    uint16_t vs_data_len;
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    /** Vendor specific data type */
    uint8_t vs_data_type;
    /** Vendor specific data */
    phRfVendorData_t vs_data;
    /** Average value of the computed carrier frequency offset estimate from the reception */
    int16_t rx_cfo_est;
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
} phTest_Loopback_Ntf_t;
/* @} */

/**
 * \brief  Structure lists out the per rx notification.
 */
/* @{ */
typedef struct phRfTestData
{
    /** Status */
    uint8_t status;
    /** Data length */
    uint16_t dataLength;
    /** Data received */
    // TODO: IOT13 HPRF requires 4k+ buffer
    uint8_t data[MAX_UCI_PACKET_SIZE];
} phRfTestData_t;
/* @}*/

/**
 * \brief  Structure lists out the per tx notification.
 */
/* @{ */
typedef struct phPerTxData
{
    /** Status */
    uint8_t status;
} phPerTxData_t;
/* @}*/

/** @} */ /* @addtogroup Uwb_RfTest_Types */

#endif
