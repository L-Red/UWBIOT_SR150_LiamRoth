/*
 *
 * Copyright 2018-2023 NXP.
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

#pragma once

#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "phUwb_BuildConfig.h"

#include <uwb_board_values.h>

// TODO: Check if this is valid for QN9090
#define phPlatform_Is_Irq_Context() (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)

/* Select Board version here */
#define UWB_BOARD_VERSION UWB_BOARD_RHODES_V4

#define UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT 1

#if UWBIOT_UWBD_SR100S
#define UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD 1
#else
#define UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD 0
#endif

/* select-aoa-mode:start */

#if UWBIOT_UWBD_SR100S
#define UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL UWB_BOARD_RX_ANTENNA_CONFIG_MODE_TOF
#else
// On Naked board, it's 2D AoA, Post packaging, it wil be set to 3D AoA
#define UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL UWB_BOARD_RX_ANTENNA_CONFIG_MODE_3DAOA
#endif

/* select-aoa-mode:end */

/* TODO : Map to LEDs on board */
#define UWB_BOARD_DEFINE_LED_APIS(THE_LED)                      \
    static inline void UWB_BOARD_GPIO_SET_##THE_LED##_ON(void)  \
    {                                                           \
    }                                                           \
    static inline void UWB_BOARD_GPIO_SET_##THE_LED##_OFF(void) \
    {                                                           \
    }

UWB_BOARD_DEFINE_LED_APIS(LED_R)
UWB_BOARD_DEFINE_LED_APIS(LED_O)
UWB_BOARD_DEFINE_LED_APIS(LED_B)
UWB_BOARD_DEFINE_LED_APIS(LED_G)
