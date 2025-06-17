/*
 * Copyright 2019-2021,2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "pin_mux.h"
#include "board.h"
#include "TimersManager.h"
#include "UWB_GPIOExtender.h"
/*${header:end}*/

/*${function:start}*/
void BOARD_InitClocks(void)
{
    /*!< Set up the clock sources */
    //CLOCK_EnableClock(kCLOCK_Fro32M);     /*!< Ensure FRO 32MHz is on */
    CLOCK_EnableClock(kCLOCK_Fro48M);     /*!< Ensure FRO 48MHz is on */
    CLOCK_EnableClock(kCLOCK_Fro12M);     /*!< Ensure FRO 12MHz is on */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK); /*!< Switch to FRO 12MHz first to ensure we can change the clock setting */

    CLOCK_EnableClock(kCLOCK_Xtal32k); //#define CLOCK_32k_source kCLOCK_Xtal32k

    //CLOCK_EnableAPBBridge();           /* The Async_APB clock is enabled. */
    //CLOCK_EnableClock(kCLOCK_Xtal32M); /*!< Enable XTAL 32 MHz output */
    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false);     /*!< Set AHBCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk, 1U, false); /*!< Set SYSTICKCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivTraceClk, 1U, false);   /*!< Set TRACECLKDIV divider to value 1 */

    /*!< Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kXTAL32M_to_MAIN_CLK);   /*!< Switch MAIN_CLK to XTAL32M */
    CLOCK_AttachClk(kMAIN_CLK_to_ASYNC_APB); /*!< Switch ASYNC_APB to MAIN_CLK */
    CLOCK_AttachClk(kXTAL32M_to_OSC32M_CLK); /*!< Switch OSC32M_CLK to XTAL32M */
    CLOCK_AttachClk(kOSC32M_to_PWM_CLK);     /*!< Switch PWM_CLK to OSC32M */
    CLOCK_AttachClk(kOSC32M_to_USART_CLK);
    CLOCK_AttachClk(kOSC32M_to_I2C_CLK); /*!< Switch I2C_CLK to OSC32M */

#if ((defined APP_WWDT_ENABLE) && (APP_WWDT_ENABLE))
    CLOCK_AttachClk(kOSC32K_to_WDT_CLK); /*!< Switch WDT_CLK to OSC32K */
#endif

    /*!< Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
}

void hardware_init(void)
{
    BOARD_common_hw_init();
    BOARD_Get_Rhodes4Version();
    GPIOExtenderInit();
}

/*!
 * @brief Configure the wake-up timer for run time stats.
 */
void RTOS_AppConfigureTimerForRuntimeStats(void)
{
#if gTimestampUseWtimer_c
    CLOCK_EnableClock(CLOCK_32k_source);
    Timestamp_Init();
#endif
}

/*!
 * @brief Get run counter from wake-up timer.
 */
uint32_t RTOS_AppGetRuntimeCounterValue(void)
{
#if gTimestampUseWtimer_c
    return Timestamp_GetCounter32bit();
#else
    return 0;
#endif
}

/*${function:end}*/
