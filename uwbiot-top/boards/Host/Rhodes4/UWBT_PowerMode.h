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

#ifndef UWBT_POWER_MODES
#define UWBT_POWER_MODES

typedef enum
{
    UWBT_RUN_MODE,
    UWBT_POWER_DOWN_MODE,
    UWBT_RUN_MODE_UNINIT,
} UWBT_PowerMode_t;

void UWBT_PowerModeEnter(UWBT_PowerMode_t mode);
void UWBT_ExitPowerDownCb(void);
void UWBT_EnterLowPowerCb(void);
void UWBT_InterruptISRInit(void *cb);
#endif
