/* Copyright 2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef UWBT_BUILD_CONFIG
#define UWBT_BUILD_CONFIG

#define DEFAULT_BUILD              0
#define MANUF_TEST_BUILD           1
#define STANDALONE_INITATOR_BUILD  2
#define STANDALONE_RESPONDER_BUILD 3
#define VALIDATION_V3_BUILD        4

#define TAG_BUILD_CFG DEFAULT_BUILD

#define TAG_PROTO 2

// #define TAG_PROTO                   1
#define TAG_V2        3
#define TAG_V3        4
#define SHIELD        5
#define RHODES_V4     6
#define BOARD_VERSION RHODES_V4

#define PIN_LEVEL_HIGH 1
#define PIN_LEVEL_LOW  0

#define CHIP_VERSION 0x01
#define MW_VERSION   0x02

#endif
