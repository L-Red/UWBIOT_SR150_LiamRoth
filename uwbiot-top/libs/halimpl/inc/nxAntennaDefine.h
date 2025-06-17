/* Copyright 2021-2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

/* Define atennaes
 *
 * Everything in this file starts with
 *
 *  AD : Antenna Define
 */

#ifndef NX_ANTENNA_DEFINE_H
#define NX_ANTENNA_DEFINE_H

#pragma once

/** Little endian, U16 to U8
 *
 * (Helper macro)
 */
#define AD_U16_TO_AU8_LE(X) (0xFFu & (X)), (0xFFu & ((X) >> 8u))

/** How many RX GPIO Define entries are there */
#define AD_N_RX_ENTRIES(X) ((X))

/** How many TX GPIO Define entries are there */
#define AD_N_TX_ENTRIES(X) ((X))

/** How many Pairs are defined */
#define AD_N_PAIR_ENTRIES(X) ((X))

/** Define RX Antennae */
#define AD_ANTENNA_RX_IDX_DEFINE_GPIO 0xE4, 0x60

/** Define TX Antennae  */
#define AD_ANTENNA_TX_IDX_DEFINE 0xE4, 0x61

/** Define Antennae Pair */
#define AD_ANTENNAS_RX_PAIR_DEFINE 0xE4, 0x62

/** Define ID for RX GPIO */
#define AD_RX_ID(X) ((X))
/** Define ID for TX GPIO and Entry */
#define AD_TX_ID(X) ((X))
/** Define ID for RX Atenna Pair */
#define AD_AP_ID(X) ((X))
/** Pin for RX1 for Antenna Pair */
#define AD_AP_RX1(X) ((X))
/** Pin for RX2 for Antenna Pair */
#define AD_AP_RX2(X) ((X))
/** Pin for RX3 for Antenna Pair */
#define AD_AP_RX3(X) ((X))

#if defined(UWBIOT_UWBD_SR100S) && UWBIOT_UWBD_SR100S
/* In case we are compiling for Single Antenna/ToF Variant, just undef all Antenna Pair Macros */
#undef AD_AP_ID
#undef AD_AP_RX1
#undef AD_AP_RX2
#undef AD_AP_RX3
#endif

/** Channel Number for Antenna Define */
#define AD_CALIB_CN(X) ((X))

/** UCI Command for Calibration params */
#if !(UWBIOT_UWBD_SR040)
#define AD_CALIB_CMD_GD                 0x02
#define AD_CALIB_CMD_PDOA_CALIB         0x62
#define AD_CALIB_CMD_PDOA_OFFSET        0x03
#define AD_CALIB_CMD_AOA_THRESHOLD_PDOA (0x66)
#else
#define AD_CALIB_CMD_GD                 0x0F
#define AD_CALIB_CMD_PDOA_CALIB         0x0C
#define AD_CALIB_CMD_PDOA_OFFSET        0x10
#define AD_CALIB_CMD_AOA_THRESHOLD_PDOA (0x12)
#endif // !(UWBIOT_UWBD_SR040)
#define AD_CALIB_LEN_PDOA_CALIB (11 * 11 * 2)

/* Calibration values */
#define AD_CALIB_GD(X)             AD_U16_TO_AU8_LE((X))
#define AD_CALIB_PDOA_OFFSET(X)    AD_U16_TO_AU8_LE((X))
#define AD_CALIB_THRESHOLD_PDOA(X) AD_U16_TO_AU8_LE((X))

/* RX Receiver port used */
#define AD_DEF_RX_PORT(X) ((X))
#define AD_DEF_MASK(X)    AD_U16_TO_AU8_LE((X))
#define AD_DEF_VAL(X)     AD_U16_TO_AU8_LE((X))
#define AD_AP_FOV(X)      AD_U16_TO_AU8_LE((X))

#define SR1XX_RX1_PORT (0x01)
#define SR1XX_RX2_PORT (0x02)
/* Invalid port only used for skipped defines add new value when required */
#define SR1XX_RX_INVALID_PORT (0x00)

/** Configuration Mode 0: Configuration mode for Dual/Single AoA usecase, ToA Mode with
implicit Rx mode as per ANTENNAS_RX_PAIR_DEFINE and Test/Loopback mode usecase **/

#define AD_MODE_DEFAULT 0
#define AD_MODE_TOF     1
#define AD_MODE_RADAR   2

/** GPIO Masks and values */
enum AD_GPIO_Configs
{
    kAD_GPIO_EF1 = 1u << 0u,
    kAD_GPIO_EF2 = 1u << 1u,
    kAD_GPIO_6   = 1u << 2u,
    kAD_GPIO_7   = 1u << 3u,
    kAD_GPIO_9   = 1u << 4u,
    kAD_GPIO_10  = 1u << 5u,
    kAD_GPIO_11  = 1u << 6u,
    kAD_GPIO_12  = 1u << 7u,
    kAD_GPIO_14  = 1u << 8u,
};

#endif /* NX_ANTENNA_DEFINE_H */
