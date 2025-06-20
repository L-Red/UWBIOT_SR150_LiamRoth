/*
 * Copyright 2018-2022 NXP.
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

/**************************************     UCI Packet Format    *****************************************
***********************************************************************************************************
|                         |                       |                               |
|----3----|--1---|---4----|------2----|-----6-----|-----8----|---------8----------|       L bytes
|                         |                       |                               |
-------------------------------------------------------------------------------------------------------------
|    MT   |  PBF |  GID   |     RFU   |    OID    |   RFU    | Payload Length (L) |      Payload
-------------------------------------------------------------------------------------------------------------
|                         |                       |                               |
|       OCTET 0           |      OCTET 1          |          OCTET 2-3            |      OCTET 4 - 4+L

***********************************************************************************************************
***********************************************************************************************************


*********************     UCI Packet Format for Extended Payload  (more than 255 bytes)  *****************
***********************************************************************************************************
|                         |                                           |                               |
|----3----|--1---|---4----|-----------1----------|---1----|-----6-----|-------8--------|------8-------|       L bytes
|                         |                                           |                               |
--------------------------------------------------------------------------------------------------------------------------
|    MT   |  PBF |  GID   |   UCI Extension Bit  |  RFU   |    OID    |  Extended Payload Length (L)  |      Payload
--------------------------------------------------------------------------------------------------------------------------
|                         |                                           |                               |
|       OCTET 0           |                  OCTET 1                  |            OCTET 2-3          |      OCTET 4 - 4+L

***********************************************************************************************************
**********************************************************************************************************/

/*
 * For NON single antenna variant
 */
#ifndef _UWB_DEVICECONFIG_RV4_SR1XX_H_
#define _UWB_DEVICECONFIG_RV4_SR1XX_H_

#include <stdint.h>
#include <uwb_board.h>
#include <phNxpUwbConfig.h>
#include <nxAntennaDefine.h>

/* Set to 0 in case you are using V3 Demonstrators,
 *
 * else set to 1 */
#define USE_NAKED_BOARD 0

/* SR160 Radar required Two TX reaming requires only One*/

#if UWBIOT_UWBD_SR160
#define TX_ANTENNA_ENTRIES 0x03
#define RX_ANTENNA_ENTRIES 0x04
#else
#define TX_ANTENNA_ENTRIES 0x01
#define RX_ANTENNA_ENTRIES 0x03
#endif //UWBIOT_UWBD_SR160

#define RX_ANTEENA_PAIR 0x02

/**
 * Set the desired Channel number to configure PDoA calibrations
 *
 * \example 0x09 to select Channel 9
 */
#define PDOA_CALIBRATION_CHANNEL 0x09

#if (PDOA_CALIBRATION_CHANNEL == 0x05)
    #define MAX_CH5_CFG 4
    #define MAX_CH9_CFG 0
#elif (PDOA_CALIBRATION_CHANNEL == 0x09)
    #define MAX_CH5_CFG 0
    #define MAX_CH9_CFG 4
#else
    #error "Invalid channel selection"
#endif

/*
  * 0xE4 0x02 : DPD wakeup source : default value : 0x00
  * 0xE4 0x03 : WTX count config : default value : 20 (0x14)
  * 0xE4, 0x34: OEM XTAL start up time/clock request time
  * */

/* clang-format off */
const uint8_t phNxpUciHal_core_configs[] =
{
    0x20, 0x20, 0x04, 0x00, 0x1C, 0x06,
    0x01, 0x01, 0x01,
    0xE4, 0x02, 0x01, 0x00,
    0xE4, 0x03, 0x01, 0x14,
    0xE4, 0x04, 0x02, 0xF4, 0x01,
    0xE4, 0x28, 0x04, 0x2F, 0x2F, 0x2F, 0x00,
    0xE4, 0x33, 0x01, 0x01,
};

#if USE_NAKED_BOARD
#define GROUP_DELAY_CH5 (15078 - 42)
#define GROUP_DELAY_CH6 (15078 - 42)
#define GROUP_DELAY_CH8 (15078 - 50)
#define GROUP_DELAY_CH9 (15078 - 42)
#else
#define GROUP_DELAY_CH5 (15120)
#define GROUP_DELAY_CH6 (15120)
#define GROUP_DELAY_CH8 (15120)
#define GROUP_DELAY_CH9 (15120)
#endif

const uint8_t phNxpUciHal_rx_antennae_delay_calib_channel5[] =
{
    0x14,
    0x2F, 0x21, 0x00, 0x10,
    0x05,
    0x02,
    0x0D,
    0x04,
    0x01, 0xE6, 0x3A,
    0x02, 0xE6, 0x3A,
    0x03, 0xE6, 0x3A,
    0x04, 0xE6, 0x3A,
};

const uint8_t phNxpUciHal_rx_antennae_delay_calib_channel9[] =
{
    0x14,
    0x2F, 0x21, 0x00, 0x10,
    0x09,
    0x02,
    0x0D,
    0x04,
    0x01, 0xD0, 0x3A,
    0x02, 0xD0, 0x3A,
    0x03, 0xD0, 0x3A,
    0x04, 0xD0, 0x3A,
};

const uint8_t phNxpUciHal_rx_antennae_delay_calib_channel6[] = {};

const uint8_t phNxpUciHal_rx_antennae_delay_calib_channel8[] = {};

const uint8_t phNxpUciHal_core_antennadefs[] =
{
 /* Full length */ 4 + (1 + 3 + 3 + 3 +  ((1 + 6*AD_N_RX_ENTRIES(RX_ANTENNA_ENTRIES)) + (1 + 5*AD_N_TX_ENTRIES(TX_ANTENNA_ENTRIES)) + (1 + 6*AD_N_PAIR_ENTRIES(RX_ANTEENA_PAIR)))) ,
 /* Core set config */ 0x20, 0x04,
 /* Length */ 0x00, 1 + 3 + 3 + 3 +  ((1 + 6*AD_N_RX_ENTRIES(RX_ANTENNA_ENTRIES)) + (1 + 5*AD_N_TX_ENTRIES(TX_ANTENNA_ENTRIES)) + (1 + 6*AD_N_PAIR_ENTRIES(RX_ANTEENA_PAIR))) ,
 /* Num Configs */ 3,
AD_ANTENNA_RX_IDX_DEFINE_GPIO,
 (1 + 6*AD_N_RX_ENTRIES(RX_ANTENNA_ENTRIES)) , AD_N_RX_ENTRIES(RX_ANTENNA_ENTRIES),
#if UWBIOT_UWBD_SR160
    AD_RX_ID(1), AD_DEF_RX_PORT(0x01), AD_DEF_MASK(0x0002), AD_DEF_VAL(0x0002),
    AD_RX_ID(2), AD_DEF_RX_PORT(0x01), AD_DEF_MASK(0x0002), AD_DEF_VAL(0),
    AD_RX_ID(3), AD_DEF_RX_PORT(0x02), AD_DEF_MASK(0x0001), AD_DEF_VAL(0x0001),
    AD_RX_ID(4), AD_DEF_RX_PORT(0x02), AD_DEF_MASK(0x0001), AD_DEF_VAL(0x0),
#else
    // V AoA for Patch Array on V3 Demonstrator
    // Azimuth on the Naked RV4 board
    // Used for PCTT as well. EF2 High.
    AD_RX_ID(1), AD_DEF_RX_PORT(SR1XX_RX1_PORT), AD_DEF_MASK(kAD_GPIO_EF2), AD_DEF_VAL(kAD_GPIO_EF2),
    // H AoA for Patch Array on V3 Demonstrator.
    // NA for Naked RV4 board
    AD_RX_ID(2), AD_DEF_RX_PORT(SR1XX_RX1_PORT), AD_DEF_MASK(kAD_GPIO_EF2), AD_DEF_VAL(0),
    /* Common RX Pin for both H and V. Goes to TX/RX2 port of Helios  */
    AD_RX_ID(3), AD_DEF_RX_PORT(SR1XX_RX2_PORT), AD_DEF_MASK(kAD_GPIO_EF1), AD_DEF_VAL(kAD_GPIO_EF1),
#endif //UWBIOT_UWBD_SR160
    /* RX GPIO: Skip */ //  AD_RX_ID(5), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(6), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(7), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(8), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(9), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(10), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(11), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(12), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),


AD_ANTENNA_TX_IDX_DEFINE,
 (1 + 5*AD_N_TX_ENTRIES(TX_ANTENNA_ENTRIES)) , AD_N_TX_ENTRIES(TX_ANTENNA_ENTRIES),
#if UWBIOT_UWBD_SR160
    /** 3 TX defines  are Required for Radar */
    /* TX-1 GPIO */  AD_TX_ID(1), AD_DEF_MASK(0x05), AD_DEF_VAL(0x00),
    /* TX-2 GPIO:*/  AD_TX_ID(2), AD_DEF_MASK(0x05), AD_DEF_VAL(0x0001),
    /* TX-3 GPIO:*/  AD_TX_ID(3), AD_DEF_MASK(0x05), AD_DEF_VAL(0x0005),
#else
    /* TX GPIO: Keep */ AD_TX_ID(1), AD_DEF_MASK(kAD_GPIO_EF1), AD_DEF_VAL(0),
#endif// UWBIOT_UWBD_SR160
    /* TX GPIO: Skip */ //  AD_TX_ID(3), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* TX GPIO: Skip */ //  AD_TX_ID(4), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),

AD_ANTENNAS_RX_PAIR_DEFINE,
 (1 + 6*AD_N_PAIR_ENTRIES(RX_ANTEENA_PAIR)) , AD_N_PAIR_ENTRIES(RX_ANTEENA_PAIR),

/* 2D-AoA */
#if USE_NAKED_BOARD
    /* RX Pair: H (Naked)   */ AD_AP_ID(1), AD_AP_RX1(1 /*EF2 High */), AD_AP_RX2(3), AD_AP_RX3(0), AD_AP_FOV(0x0000), // H
#else
    /* RX Pair: H (V3 Demo) */ AD_AP_ID(1), AD_AP_RX1(2 /*EF2 LOW */), AD_AP_RX2(3), AD_AP_RX3(0), AD_AP_FOV(0x0000), // H
#endif
#if UWBIOT_UWBD_SR160
    /* RX Pair: V           */ AD_AP_ID(2), AD_AP_RX1(2), AD_AP_RX2(4), AD_AP_RX3(0), AD_AP_FOV(0x0000), // V
#else
    /* RX Pair: V           */ AD_AP_ID(2), AD_AP_RX1(1), AD_AP_RX2(3), AD_AP_RX3(0), AD_AP_FOV(0x0000), // V
#endif //UWBIOT_UWBD_SR160
    /* RX Pair: Skip */ //  AD_AP_ID(3), AD_AP_RX1(0), AD_AP_RX2(0), AD_AP_FOV(0x0000),
    /* RX Pair: Skip */ //  AD_AP_ID(4), AD_AP_RX1(0), AD_AP_RX2(0), AD_AP_FOV(0x0000),
    /* RX Pair: Skip */ //  AD_AP_ID(5), AD_AP_RX1(0), AD_AP_RX2(0), AD_AP_FOV(0x0000),
    /* RX Pair: Skip */ //  AD_AP_ID(6), AD_AP_RX1(0), AD_AP_RX2(0), AD_AP_FOV(0x0000),
    /* RX Pair: Skip */ //  AD_AP_ID(7), AD_AP_RX1(0), AD_AP_RX2(0), AD_AP_FOV(0x0000),
    /* RX Pair: Skip */ //  AD_AP_ID(8), AD_AP_RX1(0), AD_AP_RX2(0), AD_AP_FOV(0x0000),
};

const uint8_t phNxpUciHal_rx_pair_1_ch_5_pdoa_calib[] = {
    0xFB,
    0x2F, 0x21, 0x00, 0xF7,
    0x05,
    0x62,
    0xF4,
    0x01,
    0x01,
    // Pan  -60,        -48,        -36,        -24,        -12,          0,        +12,        +24,        +36,        +48,        +60,
    0x80, 0x3E, 0x75, 0x2F, 0x69, 0x1F, 0x76, 0x14, 0x0D, 0x09, 0xA6, 0xFD, 0x8C, 0xF4, 0x71, 0xED, 0x86, 0xE5, 0x5E, 0xDB, 0x7E, 0xD4,
    0x44, 0x3B, 0x78, 0x32, 0x2B, 0x24, 0xDF, 0x15, 0x5B, 0x09, 0x06, 0xFD, 0x77, 0xF2, 0x42, 0xEA, 0x84, 0xE1, 0xED, 0xD6, 0xE5, 0xD0,
    0xA4, 0x36, 0x32, 0x31, 0xB4, 0x26, 0xB8, 0x19, 0xAE, 0x0B, 0x60, 0xFE, 0x53, 0xF2, 0xFE, 0xE6, 0x68, 0xDC, 0xFB, 0xD4, 0x4B, 0xCE,
    0x71, 0x33, 0x2A, 0x2D, 0x01, 0x24, 0x04, 0x18, 0x42, 0x0B, 0x41, 0xFE, 0x3A, 0xF2, 0x1E, 0xE7, 0x91, 0xDE, 0x56, 0xD6, 0xCC, 0xCD,
    0x48, 0x34, 0x81, 0x2C, 0x3D, 0x24, 0x92, 0x1A, 0x00, 0x0E, 0x42, 0x01, 0x5D, 0xF4, 0x7B, 0xE8, 0x6C, 0xDE, 0x6C, 0xD5, 0xA5, 0xCD,
    0x9F, 0x35, 0x77, 0x2C, 0xAF, 0x22, 0x1B, 0x18, 0x29, 0x0C, 0x00, 0x00, 0xAA, 0xF5, 0x79, 0xEB, 0x10, 0xE1, 0xD1, 0xD7, 0xC5, 0xD0,
    0xF9, 0x35, 0x19, 0x2E, 0x9F, 0x25, 0xDB, 0x1A, 0x44, 0x0E, 0x48, 0x02, 0x4A, 0xF6, 0x86, 0xEC, 0xA4, 0xE3, 0x10, 0xDB, 0x4B, 0xD3,
    0x77, 0x34, 0x50, 0x2A, 0x1A, 0x1E, 0x41, 0x14, 0x27, 0x0B, 0x3F, 0x01, 0xB0, 0xF6, 0x3B, 0xEC, 0x81, 0xE1, 0xBE, 0xD8, 0x35, 0xD3,
    0x6A, 0x30, 0x68, 0x24, 0xA3, 0x1E, 0xE2, 0x17, 0x88, 0x0A, 0x85, 0x00, 0x95, 0xF8, 0xFD, 0xEF, 0x66, 0xE6, 0xF4, 0xD9, 0xF1, 0xCF,
    0xC3, 0x2B, 0x13, 0x20, 0xA0, 0x1B, 0x6A, 0x0B, 0x83, 0x00, 0x41, 0xF9, 0x95, 0xF4, 0x57, 0xEE, 0x61, 0xE7, 0x6C, 0xDC, 0x92, 0xD2,
    0xDB, 0x23, 0x1E, 0x21, 0x63, 0x12, 0xA3, 0x07, 0xD8, 0xFF, 0x7E, 0xF7, 0xE9, 0xF0, 0x47, 0xEB, 0x83, 0xE6, 0xD9, 0xE0, 0xF1, 0xD4,
};

const uint8_t phNxpUciHal_rx_pair_1_ch_9_pdoa_calib[] = {
    0xFB,
    0x2F, 0x21, 0x00, 0xF7,
    0x09,
    0x62,
    0xF4,
    0x01,
    0x01,
    // Pan  -60,        -48,        -36,        -24,        -12,          0,        +12,        +24,        +36,        +48,        +60,
    0x80, 0x49, 0x40, 0x39, 0xD4, 0x30, 0x8C, 0x20, 0xA8, 0x11, 0x18, 0xFE, 0x9B, 0xEE, 0x17, 0xE3, 0xE7, 0xD3, 0x2D, 0xCC, 0x2F, 0xC1,
    0x5A, 0x3E, 0xC1, 0x39, 0x29, 0x2E, 0x7C, 0x1E, 0x4B, 0x0E, 0x45, 0xFD, 0xE2, 0xEC, 0x56, 0xDF, 0x57, 0xD6, 0x53, 0xC8, 0x2C, 0xC3,
    0x0B, 0x3D, 0x05, 0x3A, 0xAB, 0x2C, 0xD4, 0x1E, 0x1D, 0x0E, 0x9C, 0xFC, 0x10, 0xEC, 0xA9, 0xE0, 0x2B, 0xD5, 0x2A, 0xC9, 0x39, 0xC3,
    0xDA, 0x41, 0xB5, 0x33, 0x65, 0x2B, 0x8C, 0x1C, 0xFF, 0x0D, 0x27, 0xFF, 0x93, 0xEE, 0x87, 0xDF, 0x8F, 0xD2, 0xE1, 0xCB, 0x32, 0xBB,
    0x25, 0x4A, 0xAD, 0x37, 0x4C, 0x2B, 0xF3, 0x1D, 0xA3, 0x0D, 0xCB, 0xFC, 0xDB, 0xEC, 0x0F, 0xDF, 0x30, 0xD3, 0x54, 0xC7, 0x8D, 0xBC,
    0x8B, 0x44, 0x9A, 0x39, 0x2C, 0x2B, 0x3D, 0x1D, 0x00, 0x0F, 0x00, 0x00, 0x28, 0xF0, 0x18, 0xE2, 0x0D, 0xD6, 0x0A, 0xCC, 0xC4, 0xC4,
    0x7B, 0x3D, 0x10, 0x35, 0x6C, 0x2B, 0xFE, 0x1E, 0xC8, 0x0F, 0xED, 0x00, 0x4A, 0xF1, 0x4E, 0xE4, 0xC2, 0xD9, 0xFC, 0xD0, 0xF9, 0xC9,
    0x24, 0x41, 0xBE, 0x39, 0x75, 0x2D, 0x98, 0x21, 0x9B, 0x14, 0xA8, 0x04, 0x23, 0xF3, 0x6C, 0xE4, 0x5B, 0xD6, 0x32, 0xCC, 0x6E, 0xC6,
    0x8A, 0x46, 0xBA, 0x38, 0x50, 0x2E, 0xA4, 0x1E, 0x22, 0x0F, 0x22, 0x03, 0xFF, 0xF6, 0x71, 0xEA, 0x5D, 0xDE, 0xB1, 0xCF, 0x21, 0xC1,
    0x18, 0x43, 0x0A, 0x3A, 0x96, 0x2D, 0x6F, 0x27, 0x13, 0x18, 0x29, 0x04, 0x4B, 0xF2, 0x5A, 0xE7, 0x9E, 0xDC, 0x98, 0xD2, 0x6B, 0xC4,
    0x10, 0x43, 0x77, 0x37, 0x25, 0x34, 0x7C, 0x25, 0x47, 0x12, 0xE6, 0x00, 0xCD, 0xEF, 0xEB, 0xDF, 0x3E, 0xD7, 0xEC, 0xCF, 0xBB, 0xC9,
};

const uint8_t phNxpUciHal_rx_pair_2_ch_5_pdoa_calib[] = {
    0xFB,
    0x2F, 0x21, 0x00, 0xF7,
    0x05,
    0x62,
    0xF4,
    0x01,
    0x02,
    // Pan  -60,        -48,        -36,        -24,        -12,          0,        +12,        +24,        +36,        +48,        +60,
    0x4F, 0xEC, 0x4A, 0xEF, 0x4A, 0xF2, 0xCD, 0xF5, 0x87, 0xFB, 0x79, 0x02, 0x69, 0x08, 0x8C, 0x0C, 0xDE, 0x0C, 0xE6, 0x0A, 0x48, 0x07,
    0x69, 0xDF, 0x8E, 0xE5, 0xA3, 0xEB, 0x54, 0xF1, 0x2A, 0xF8, 0x0C, 0x01, 0x1B, 0x09, 0xA6, 0x0F, 0x79, 0x11, 0x1C, 0x11, 0xFC, 0x11,
    0x92, 0xD8, 0xD9, 0xDE, 0xB3, 0xE6, 0xD9, 0xED, 0x7B, 0xF7, 0x7C, 0x01, 0xE5, 0x0B, 0x81, 0x13, 0xF8, 0x19, 0xAD, 0x20, 0xB8, 0x21,
    0x19, 0xD2, 0x97, 0xD9, 0x0B, 0xE2, 0xDB, 0xEB, 0x6C, 0xF7, 0x6E, 0x02, 0x14, 0x0F, 0x24, 0x17, 0xE7, 0x23, 0x62, 0x2D, 0xF0, 0x2E,
    0x71, 0xCA, 0x9F, 0xD5, 0x0B, 0xDE, 0x1A, 0xEA, 0xD1, 0xF6, 0x5F, 0x03, 0xC9, 0x10, 0x00, 0x1A, 0xDF, 0x29, 0xA2, 0x34, 0x49, 0x3A,
    0xC7, 0xC4, 0x1B, 0xD2, 0x8C, 0xDB, 0xF3, 0xE7, 0x16, 0xF6, 0x00, 0x00, 0x06, 0x10, 0x52, 0x1B, 0x94, 0x2A, 0xF2, 0x35, 0xA1, 0x3A,
    0x24, 0xC4, 0x25, 0xD0, 0xE4, 0xD8, 0xA3, 0xE6, 0xCC, 0xF5, 0x26, 0x03, 0xBB, 0x09, 0x4E, 0x19, 0x38, 0x24, 0x81, 0x31, 0x27, 0x34,
    0x79, 0xC7, 0x7F, 0xCF, 0x09, 0xD7, 0x5C, 0xE6, 0xF3, 0xF3, 0x44, 0x01, 0x76, 0x07, 0xF1, 0x14, 0x65, 0x1F, 0x4F, 0x2A, 0x54, 0x2D,
    0xD1, 0xC6, 0x92, 0xCE, 0xC8, 0xD6, 0xED, 0xE7, 0x15, 0xF2, 0x6D, 0xFE, 0x0D, 0x05, 0x6D, 0x0D, 0x78, 0x19, 0x2E, 0x22, 0xD0, 0x26,
    0xBA, 0xCE, 0x49, 0xD5, 0x1A, 0xDF, 0x80, 0xEA, 0xEE, 0xF0, 0xC8, 0xFB, 0xF1, 0x01, 0xE0, 0x05, 0xC3, 0x0F, 0x3A, 0x1A, 0xB9, 0x22,
    0x58, 0xE0, 0x87, 0xE4, 0x7C, 0xE8, 0x9C, 0xEE, 0xB2, 0xF3, 0xB6, 0xFB, 0x63, 0xFF, 0xED, 0x00, 0xEE, 0x03, 0xBD, 0x0D, 0xB5, 0x16,
};

const uint8_t phNxpUciHal_rx_pair_2_ch_9_pdoa_calib[] = {
    0xFB,
    0x2F, 0x21, 0x00, 0xF7,
    0x09,
    0x62,
    0xF4,
    0x01,
    0x02,
    // Pan  -60,        -48,        -36,        -24,        -12,          0,        +12,        +24,        +36,        +48,        +60,
    0x9F, 0xEC, 0x9E, 0xE3, 0x45, 0xE9, 0xE9, 0xF5, 0x32, 0x09, 0x14, 0x0E, 0xC5, 0x07, 0x44, 0x13, 0x44, 0x27, 0x38, 0x2B, 0xB0, 0x2C,
    0x17, 0xD9, 0x70, 0xDD, 0x09, 0xEB, 0xB3, 0xF0, 0x3C, 0xFB, 0xF0, 0x09, 0xEA, 0x0A, 0xAB, 0x1C, 0x61, 0x27, 0x2F, 0x2D, 0xFF, 0x39,
    0x3C, 0xD8, 0xBC, 0xDC, 0xEB, 0xE0, 0x58, 0xF0, 0x24, 0xFB, 0x85, 0x04, 0xD6, 0x0E, 0x74, 0x21, 0xC7, 0x28, 0x55, 0x35, 0x05, 0x43,
    0x51, 0xCD, 0x7F, 0xD8, 0x21, 0xE2, 0xE1, 0xE7, 0xCE, 0xF9, 0x0A, 0x02, 0x49, 0x12, 0x4F, 0x23, 0xCA, 0x2D, 0x63, 0x3D, 0x7C, 0x4A,
    0xA0, 0xD4, 0xF8, 0xD5, 0x61, 0xE0, 0xE4, 0xE3, 0xCD, 0xF4, 0x9F, 0x00, 0x0D, 0x14, 0x0C, 0x24, 0xEF, 0x2F, 0x68, 0x42, 0x50, 0x4D,
    0x03, 0xD1, 0x82, 0xD2, 0x99, 0xDB, 0xAD, 0xE2, 0xC6, 0xF0, 0x00, 0x00, 0xAB, 0x13, 0x99, 0x22, 0x94, 0x30, 0xF7, 0x41, 0x20, 0x4D,
    0xFF, 0xCD, 0x6C, 0xD0, 0x75, 0xDA, 0x52, 0xE2, 0x67, 0xF0, 0xA8, 0xFE, 0xAD, 0x0F, 0x26, 0x1E, 0x0C, 0x2F, 0x9A, 0x3C, 0x29, 0x48,
    0xF9, 0xCA, 0xA1, 0xD2, 0x16, 0xDB, 0x19, 0xE3, 0x70, 0xF0, 0x9F, 0xFC, 0x95, 0x0C, 0xCA, 0x19, 0x53, 0x2A, 0x6A, 0x37, 0x53, 0x40,
    0x49, 0xD1, 0x13, 0xD4, 0x9A, 0xDA, 0xC0, 0xE6, 0x75, 0xEE, 0x2A, 0xFB, 0xA3, 0x0A, 0xC7, 0x11, 0x81, 0x24, 0x34, 0x2D, 0x59, 0x38,
    0x91, 0xD8, 0x3D, 0xDF, 0x6B, 0xE5, 0xF2, 0xE7, 0xBB, 0xEB, 0xA4, 0xFB, 0x97, 0x07, 0x49, 0x0A, 0xF9, 0x19, 0x6F, 0x24, 0xF8, 0x2C,
    0xA5, 0xE6, 0xF3, 0xDF, 0xA3, 0xE2, 0x6E, 0xE5, 0xCB, 0xEF, 0x76, 0xFC, 0x52, 0x04, 0x19, 0x06, 0xDA, 0x0A, 0x57, 0x17, 0x6F, 0x21,
};

const uint8_t phNxpUciHal_pdoa_offset_calib_ch_5[] = {
    0x0E,
    0x2F, 0x21, 0x00, 0x0A,
    0x05,
    0x03,
    0x07,
    0x02,
    0x01, 0x26, 0x01,
    0x02, 0xB3, 0x0A,
};

const uint8_t phNxpUciHal_pdoa_offset_calib_ch_9[] = {
    0x0E,
    0x2F, 0x21, 0x00, 0x0A,
    0x09,
    0x03,
    0x07,
    0x02,
    0x01, 0x40, 0xFE,
    0x02, 0x67, 0xFD,
};

const uint8_t phNxpUciHal_aoa_threshold_pdoa_calib_ch_5[] = {
    0x0E,
    0x2F, 0x21, 0x00, 0x0A,
    0x05,
    0x66,
    0x07,
    0x02,
    0x01, 0x10, 0x59,
    0x02, 0x61, 0xB3,
};

const uint8_t phNxpUciHal_aoa_threshold_pdoa_calib_ch_9[] = {
    0x0E,
    0x2F, 0x21, 0x00, 0x0A,
    0x09,
    0x66,
    0x07,
    0x02,
    0x01, 0x7E, 0x58,
    0x02, 0xB4, 0xA7,
};

/* clang-format on */

const uint8_t phNxpUciHal_NXPCoreConfig1[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig2[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig3[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig4[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig5[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig6[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig7[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig8[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig9[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig10[] = {0x00};

const NxpParam_t phNxpUciHal_NXPConfig[] = {
    /*
     *  Ranging session period: which means, it is the duration of one 1:N
     session and the interval before start of next 1:N session. (1:N ranging
     period + interval between two consecutive  1:N cycles)
        UWB_RANG_SESSION_INTERVAL= (UWB_RANG_CYCLE_INTERVAL * No_of_anchors) +
     IDLE_BEFORE_START_OF_NEXT_1_N Value is in milliseconds This config is valid
     only in 1:N ranging session
     * */
    {UWB_RANG_SESSION_INTERVAL, TYPE_VAL, CONFIG_VAL 2000},
    /*
     *  Application session timeout: How log ranging shall continue.
        value is in milliseconds
        0 value: MW shall configure the value which is passed from application
        Non zero value: MW shall configure timeout with this config value
     provided here
     * */
    {UWB_APP_SESSION_TIMEOUT, TYPE_VAL, CONFIG_VAL 3600000},
    /*
     *  Ranging cycle interval: intreval between two consecutive SS/DS-TWR
     ranging cycle value is in milliseconds 0 value: MW shall configure the
     value which is passed from application Non zero value: MW shall configure
     interval with this config value provided here
     * */
    {UWB_RANG_CYCLE_INTERVAL, TYPE_VAL, CONFIG_VAL 200},
    /*
     *
     *  Timeout value in milliseconds for UWB standby mode.
        The range is between 5000 msec to 20000 msec and zero is to disable
     * */
    {UWB_STANDBY_TIMEOUT_VALUE, TYPE_VAL, CONFIG_VAL 0x00},
    /*
    *FW log level for each above module
     Logging Level Error            0x0001
     Logging Level Warning          0x0002
     Logging Level Timestamp        0x0004
     Logging Level Sequence Number  0x0008
     Logging Level Info-1           0x0010
     Logging Level Info-2           0x0020
     Logging Level Info-3           0x0040
     * */
    {UWB_SET_FW_LOG_LEVEL, TYPE_VAL, CONFIG_VAL 0x003},
    /*
     * Enable/disable to dump FW binary log for different Modules as below
       0x00 for disable the binary log
       Secure Thread      0x01
       Secure ISR         0x02
       Non-Secure ISR     0x04
       Shell Thread       0x08
       PHY Thread         0x10
       Ranging Thread     0x20
     * */
    {UWB_FW_LOG_THREAD_ID, TYPE_VAL, CONFIG_VAL 0x00},
    /*
     * Ranging feature:  Single Sided Two Way Ranging or Double Sided Two Way
     Ranging SS-TWR =0x00 DS-TWR =0x01
     */
    {UWB_MW_RANGING_FEATURE, TYPE_VAL, CONFIG_VAL 0x01},
    /*Board Varaints are defined below:
    BOARD_VARIANT_NXPREF    0x01
    BOARD_VARIANT_CUSTREF1  0x2A
    BOARD_VARIANT_CUSTREF2  0x2B
    BOARD_VARIANT_RHODES    0x73*/
    {UWB_BOARD_VARIANT_CONFIG, TYPE_VAL, CONFIG_VAL 0x73},
    /*
     * # Board Variant version
     * */
    {UWB_BOARD_VARIANT_VERSION, TYPE_VAL, CONFIG_VAL UWB_BOARD_VERSION},
    {UWB_CORE_CONFIG_PARAM, TYPE_DATA, phNxpUciHal_core_configs},
    /* Core config antennae defines*/
    {UWB_CORE_ANTENNAE_DEFINES, TYPE_DATA, phNxpUciHal_core_antennadefs},
    {UWB_RX_ANTENNAE_DELAY_CALIB_CH5, TYPE_DATA, phNxpUciHal_rx_antennae_delay_calib_channel5},
    {UWB_RX_ANTENNAE_DELAY_CALIB_CH6, TYPE_DATA, phNxpUciHal_rx_antennae_delay_calib_channel6},
    {UWB_RX_ANTENNAE_DELAY_CALIB_CH8, TYPE_DATA, phNxpUciHal_rx_antennae_delay_calib_channel8},
    {UWB_RX_ANTENNAE_DELAY_CALIB_CH9, TYPE_DATA, phNxpUciHal_rx_antennae_delay_calib_channel9},

    /*
     * Note: If the size of data is greater than 255 bytes, TYPE_EXTENDED_DATA has to be used.
     * TYPE_DATA must only be used when size of array is less than 255 bytes
     *
     * Example if phNxpUciHal_rx_pair_1_ch_5_pdoa_calib data is more than 255 bytes
     * {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH5, TYPE_EXTENDED_DATA, phNxpUciHal_rx_pair_1_ch_5_pdoa_calib},
     */
    {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH5, TYPE_DATA, phNxpUciHal_rx_pair_1_ch_5_pdoa_calib},
    {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH9, TYPE_DATA, phNxpUciHal_rx_pair_1_ch_9_pdoa_calib},
    {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH5, TYPE_DATA, phNxpUciHal_rx_pair_2_ch_5_pdoa_calib},
    {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH9, TYPE_DATA, phNxpUciHal_rx_pair_2_ch_9_pdoa_calib},
    {UWB_AOA_CONFIG_PDOA_OFFSET_CH5, TYPE_DATA, phNxpUciHal_pdoa_offset_calib_ch_5},
    {UWB_AOA_CONFIG_PDOA_OFFSET_CH9, TYPE_DATA, phNxpUciHal_pdoa_offset_calib_ch_9},
    {UWB_AOA_CONFIG_THRESHOLD_PDOA_CH5, TYPE_DATA, phNxpUciHal_aoa_threshold_pdoa_calib_ch_5},
    {UWB_AOA_CONFIG_THRESHOLD_PDOA_CH9, TYPE_DATA, phNxpUciHal_aoa_threshold_pdoa_calib_ch_9},
#if UWBIOT_UWBD_SR150
    {UWB_AOA_CH5_CONFIG_BLOCK_COUNT, TYPE_VAL, CONFIG_VAL MAX_CH5_CFG},
    {UWB_AOA_CH9_CONFIG_BLOCK_COUNT, TYPE_VAL, CONFIG_VAL MAX_CH9_CFG},
#else
    {UWB_AOA_CONFIG_BLOCK_COUNT, TYPE_VAL, CONFIG_VAL 8},
#endif // UWBIOT_UWBD_SR150

    /* Timeout for Firmware to enter DPD mode
     * Note: value set for UWB_DPD_ENTRY_TIMEOUT shall be in MilliSeconds.
     * Min : 100ms
     * Max : 2000ms */
    {UWB_DPD_ENTRY_TIMEOUT, TYPE_VAL, CONFIG_VAL 500},
    /* 0x00 = FIRA generic notifications (Default)
     * 0x01 = Vendor extended notifications
     * UWBS shall send any proprietary information in any response/notification
     * if NXP_EXTENDED_NTF_CONFIG is set 0x01 */
    {UWB_NXP_EXTENDED_NTF_CONFIG, TYPE_VAL, CONFIG_VAL 0x01},
    /* Firmware Low Power Mode
     * if UWB_LOW_POWER_MODE is 0, Firmware is Configured in non Low Power Mode
     * if UWB_LOW_POWER_MODE in 1, Firmware is Configured with Low Power Mode */
    {UWB_LOW_POWER_MODE, TYPE_VAL, CONFIG_VAL 0x01},

    {UWB_NXP_CORE_CONFIG_BLOCK_1, TYPE_DATA, phNxpUciHal_NXPCoreConfig1},
    {UWB_NXP_CORE_CONFIG_BLOCK_2, TYPE_DATA, phNxpUciHal_NXPCoreConfig2},
    {UWB_NXP_CORE_CONFIG_BLOCK_3, TYPE_DATA, phNxpUciHal_NXPCoreConfig3},
    {UWB_NXP_CORE_CONFIG_BLOCK_4, TYPE_DATA, phNxpUciHal_NXPCoreConfig4},
    {UWB_NXP_CORE_CONFIG_BLOCK_5, TYPE_DATA, phNxpUciHal_NXPCoreConfig5},
    {UWB_NXP_CORE_CONFIG_BLOCK_6, TYPE_DATA, phNxpUciHal_NXPCoreConfig6},
    {UWB_NXP_CORE_CONFIG_BLOCK_7, TYPE_DATA, phNxpUciHal_NXPCoreConfig7},
    {UWB_NXP_CORE_CONFIG_BLOCK_8, TYPE_DATA, phNxpUciHal_NXPCoreConfig8},
    {UWB_NXP_CORE_CONFIG_BLOCK_9, TYPE_DATA, phNxpUciHal_NXPCoreConfig9},
    {UWB_NXP_CORE_CONFIG_BLOCK_10, TYPE_DATA, phNxpUciHal_NXPCoreConfig10},

    /* Number of UWB_NXP_CORE_CONFIG_BLOCKS available in the config file */
    {UWB_NXP_CORE_CONFIG_BLOCK_COUNT, TYPE_VAL, CONFIG_VAL 10}};

#endif //_UWB_DEVICECONFIG_RV4_SR1XX_H_
