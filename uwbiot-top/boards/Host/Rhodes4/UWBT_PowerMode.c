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

#include "PWR_Interface.h"
#include "UWBT_PowerMode.h"
#include "phTmlUwb_transport.h"
#include "PWR_Configuration.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

typedef void (*pIntCallback_t)(void);
static pIntCallback_t pIntCallback;

extern void vPortSetupTimerInterrupt(void);

static void UWBT_EnterRunMode(void)
{
    vPortSetupTimerInterrupt();
    PWR_DisallowDeviceToSleep();
    // BOARD_SetPinsForRunMode(); // This function would be called in hw_init from BLE
}

static void UWBT_EnterPowerDown(void)
{
    /* configure pins for power down mode */
    PWR_AllowDeviceToSleep();
    phTmlUwb_io_deinit();
}

struct
{
    const char *modeName;
    void (*enterFn)(void);
} powerModeTable[] = {
    [UWBT_RUN_MODE]        = {"RUN_MODE", UWBT_EnterRunMode},
    [UWBT_POWER_DOWN_MODE] = {"POWER_DOWN", UWBT_EnterPowerDown},
};

void UWBT_PowerModeEnter(UWBT_PowerMode_t mode)
{
#if defined(CPU_JN518X) && (cPWR_UsePowerDownMode)
    assert(mode <= UWBT_POWER_DOWN_MODE);
    powerModeTable[mode].enterFn();
#endif
}

#if defined(CPU_JN518X) && (cPWR_UsePowerDownMode)
void UWBT_ExitPowerDownCb(void)
{
    // Do nothing. This will make MCU enter power down again unless action is taken
    if (pIntCallback) {
        pIntCallback();
    }
    /* Initialize application support for drivers */
    BOARD_ADCWakeupInit();
}

void UWBT_EnterLowPowerCb(void)
{
    BOARD_SetPinsForPowerDown();
}
#endif

void UWBT_InterruptISRInit(void *cb)
{
    pIntCallback = (pIntCallback_t)cb;
}
