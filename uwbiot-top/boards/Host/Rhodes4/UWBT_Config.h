/*
 *
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

#ifndef _UWBT_CONFIG_H_
#define _UWBT_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>

#include "gap_types.h"

/* Flash framework */
#include "Flash_Adapter.h"
#include "board.h"
#include "UWBT_BuildConfig.h"

// BLE_NAME
#define BLE_NAME_SIZE 24

typedef enum
{
    BLE_NAME = 1,
    BLE_INTERVAL,
} UWBT_ConfigId_t;

bool UWBT_CfgInit(void);

bool UWBT_CfgWriteBleName(uint8_t *name, uint16_t nameLen);
bool UWBT_CfgWriteBleInterval(uint16_t interval);

uint32_t UWBT_CfgReadBleInterval(void);
const char *UWBT_CfgReadBleName(void);

#endif /* _UWBT_CONFIG_H_ */
