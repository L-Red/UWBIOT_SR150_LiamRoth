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

/*
 * For single antenna variant
 */

#ifndef _UWB_DEVICECONFIG_RV4_SR100S_H_
#define _UWB_DEVICECONFIG_RV4_SR100S_H_

#include <stdint.h>
#include <uwb_board.h>
#include <phNxpUwbConfig.h>
#include <nxAntennaDefine.h>

/* Set to 0 in case you are using V3 Demonstrators,
 *
 * else set to 1 */
#define USE_NAKED_BOARD 1

/*
  * 0xE4 0x02 : DPD wakeup source : default value : 0x00
  * 0xE4 0x03 : WTX count config : default value : 20 (0x14)
  * 0xE4, 0x34: OEM XTAL start up time/clock request time
  * */

/* clang-format off */
const uint8_t phNxpUciHal_core_configs[] =
{
    0x12, 0x20, 0x04, 0x00, 0x0E, 0x03,
    0xE4, 0x02, 0x01, 0x00,
    0xE4, 0x03, 0x01, 0x14,
    0xE4, 0x34, 0x02, 0xE8, 0x03,
};


#define GROUP_DELAY_CH5 (15078 - 42)
#define GROUP_DELAY_CH6 (15078 - 42)
#define GROUP_DELAY_CH8 (15078 - 50)
#define GROUP_DELAY_CH9 (15078 - 42)

const uint8_t phNxpUciHal_rx_antennae_delay_calib_channel5[] =
{
/* Over All Length */ 4 + ( 3 + (1 + 3*AD_N_RX_ENTRIES(1))),
/* Set Calib */ 0x2F, 0x21,
/* Length */ 0,  3 + (1 + 3*AD_N_RX_ENTRIES(1)),
/* Channel */ AD_CALIB_CN(5),
/* GD Calib */ AD_CALIB_CMD_GD,
/* Length */ 1 + 3*AD_N_RX_ENTRIES(1),
/* N Entries */ AD_N_RX_ENTRIES(1),

/* GD Calib: Keep */ AD_RX_ID(1), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(2), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(3), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(4), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(5), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(6), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(7), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(8), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(9), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(10), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(11), AD_CALIB_GD(GROUP_DELAY_CH5),
/* GD Calib: Skip */ //  AD_RX_ID(12), AD_CALIB_GD(GROUP_DELAY_CH5),
};

const uint8_t phNxpUciHal_rx_antennae_delay_calib_channel9[] =
{
/* Over All Length */ 4 + ( 3 + (1 + 3*AD_N_RX_ENTRIES(1))),
/* Set Calib */ 0x2F, 0x21,
/* Length */ 0,  3 + (1 + 3*AD_N_RX_ENTRIES(1)),
/* Channel */ AD_CALIB_CN(9),
/* GD Calib */ AD_CALIB_CMD_GD,
/* Length */ 1 + 3*AD_N_RX_ENTRIES(1),
/* N Entries */ AD_N_RX_ENTRIES(1),

/* GD Calib: Keep */ AD_RX_ID(1), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(2), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(3), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(4), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(5), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(6), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(7), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(8), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(9), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(10), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(11), AD_CALIB_GD(GROUP_DELAY_CH9),
/* GD Calib: Skip */ //  AD_RX_ID(12), AD_CALIB_GD(GROUP_DELAY_CH9),
};

const uint8_t phNxpUciHal_rx_antennae_delay_calib_channel6[] =
{
/* Over All Length */ 4 + ( 3 + (1 + 3*AD_N_RX_ENTRIES(1))),
/* Set Calib */ 0x2F, 0x21,
/* Length */ 0,  3 + (1 + 3*AD_N_RX_ENTRIES(1)),
/* Channel */ AD_CALIB_CN(6),
/* GD Calib */ AD_CALIB_CMD_GD,
/* Length */ 1 + 3*AD_N_RX_ENTRIES(1),
/* N Entries */ AD_N_RX_ENTRIES(1),

/* GD Calib: Keep */ AD_RX_ID(1), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(2), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(3), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(4), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(5), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(6), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(7), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(8), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(9), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(10), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(11), AD_CALIB_GD(GROUP_DELAY_CH6),
/* GD Calib: Skip */ //  AD_RX_ID(12), AD_CALIB_GD(GROUP_DELAY_CH6),
};

const uint8_t phNxpUciHal_rx_antennae_delay_calib_channel8[] =
{
/* Over All Length */ 4 + ( 3 + (1 + 3*AD_N_RX_ENTRIES(1))),
/* Set Calib */ 0x2F, 0x21,
/* Length */ 0,  3 + (1 + 3*AD_N_RX_ENTRIES(1)),
/* Channel */ AD_CALIB_CN(8),
/* GD Calib */ AD_CALIB_CMD_GD,
/* Length */ 1 + 3*AD_N_RX_ENTRIES(1),
/* N Entries */ AD_N_RX_ENTRIES(1),

/* GD Calib: Keep */ AD_RX_ID(1), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(2), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(3), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(4), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(5), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(6), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(7), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(8), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(9), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(10), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(11), AD_CALIB_GD(GROUP_DELAY_CH8),
/* GD Calib: Skip */ //  AD_RX_ID(12), AD_CALIB_GD(GROUP_DELAY_CH8),
};

const uint8_t phNxpUciHal_core_antennadefs[] =
{
 /* Full length */ 4 + (1 + 3 + 3 + ((1 + 6*AD_N_RX_ENTRIES(1)) + (1 + 5*AD_N_TX_ENTRIES(1)))),
 /* Core set config */ 0x20, 0x04,
 /* Length */ 0x00, 1 + 3 + 3 + ((1 + 6*AD_N_RX_ENTRIES(1)) + (1 + 5*AD_N_TX_ENTRIES(1))),
 /* Num Configs */ 2,
AD_ANTENNA_RX_IDX_DEFINE_GPIO,
 (1 + 6*AD_N_RX_ENTRIES(1)) , AD_N_RX_ENTRIES(1),
    /* Common TX/RX. Goes to TX/RX2 port of Helios */
    AD_RX_ID(1), AD_DEF_RX_PORT(SR1XX_RX2_PORT), AD_DEF_MASK(kAD_GPIO_EF1), AD_DEF_VAL(kAD_GPIO_EF1),
    /* RX GPIO: Skip */ //  AD_RX_ID(5), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(6), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(7), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(8), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(9), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(10), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(11), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* RX GPIO: Skip */ //  AD_RX_ID(12), AD_DEF_RX_PORT(SR1XX_RX_INVALID_PORT), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),

AD_ANTENNA_TX_IDX_DEFINE,
 (1 + 5*AD_N_TX_ENTRIES(1)) , AD_N_TX_ENTRIES(1),
    /* TX GPIO: Keep */ AD_TX_ID(1), AD_DEF_MASK(kAD_GPIO_EF1), AD_DEF_VAL(0),
    /* TX GPIO: sKip */ //  AD_TX_ID(2), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* TX GPIO: Skip */ //  AD_TX_ID(3), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
    /* TX GPIO: Skip */ //  AD_TX_ID(4), AD_DEF_MASK(0x0000), AD_DEF_VAL(0x0000),
};

const uint8_t phNxpUciHal_rx_pair_1_ch_5_pdoa_calib[] = {0};

const uint8_t phNxpUciHal_rx_pair_1_ch_9_pdoa_calib[] = {0};

const uint8_t phNxpUciHal_rx_pair_2_ch_5_pdoa_calib[] = {0};

const uint8_t phNxpUciHal_rx_pair_2_ch_9_pdoa_calib[] = {0};

const uint8_t phNxpUciHal_pdoa_offset_calib_ch_5[] = {0};

const uint8_t phNxpUciHal_pdoa_offset_calib_ch_9[] = {0};

const uint8_t phNxpUciHal_aoa_threshold_pdoa_calib_ch_5[] = {0};

const uint8_t phNxpUciHal_aoa_threshold_pdoa_calib_ch_9[] = {0};

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
    {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH5, TYPE_DATA, phNxpUciHal_rx_pair_1_ch_5_pdoa_calib},
    {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH9, TYPE_DATA, phNxpUciHal_rx_pair_1_ch_9_pdoa_calib},
    {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH5, TYPE_DATA, phNxpUciHal_rx_pair_2_ch_5_pdoa_calib},
    {UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH9, TYPE_DATA, phNxpUciHal_rx_pair_2_ch_9_pdoa_calib},
    {UWB_AOA_CONFIG_PDOA_OFFSET_CH5, TYPE_DATA, phNxpUciHal_pdoa_offset_calib_ch_5},
    {UWB_AOA_CONFIG_PDOA_OFFSET_CH9, TYPE_DATA, phNxpUciHal_pdoa_offset_calib_ch_9},
    {UWB_AOA_CONFIG_THRESHOLD_PDOA_CH5, TYPE_DATA, phNxpUciHal_aoa_threshold_pdoa_calib_ch_5},
    {UWB_AOA_CONFIG_THRESHOLD_PDOA_CH9, TYPE_DATA, phNxpUciHal_aoa_threshold_pdoa_calib_ch_9},
    {UWB_AOA_CONFIG_BLOCK_COUNT, TYPE_VAL, CONFIG_VAL 0},
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

#endif //_UWB_DEVICECONFIG_RV4_SR100S_H_
