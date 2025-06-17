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

/** @file uwb_board_values.h
 *
 * Values used by uwb_board.h */

#ifndef UWB_BAORD_VALUES_H
#define UWB_BAORD_VALUES_H

#pragma once

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

/** Board Variants
 *
 * @{ */

// Board Variants are defined below:
#define BOARD_VARIANT_NXPREF 0x01

#define BOARD_VARIANT_CUSTREF1 0x2A

#define BOARD_VARIANT_CUSTREF2 0x2B

#define BOARD_VARIANT_RHODES 0x73

#define UWB_BOARD__BOARD_VARIANT_NXPREF 1

#define UWB_BOARD_RHODES_V3 2

#define UWB_BOARD_RHODES_V4 4

/** @} */

/* Used by uwb_board.h to select what is the mode of RX for the Session.
 *
 * See as well @ref kUWBAntCfgRxMode_t
 *
 * @{
 */

/** Single antenna mode */
#define UWB_BOARD_RX_ANTENNA_CONFIG_MODE_TOF 1

/** 2D AoA mode */
#define UWB_BOARD_RX_ANTENNA_CONFIG_MODE_2DAOA 2

/** 3D AoA Mode. Only for V3 Demonstrator */
#define UWB_BOARD_RX_ANTENNA_CONFIG_MODE_3DAOA 3

/** @} */

#endif // UWB_BAORD_VALUES_H
