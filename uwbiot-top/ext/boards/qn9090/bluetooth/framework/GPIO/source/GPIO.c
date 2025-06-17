/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
* All rights reserved.
*
* \file
*
* GPIO implementation file for ARM CORTEX-M4 processor
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#include "GPIO.h"
#include "TimersManager.h"
#include <driver_config.h>
#include "pin_mux.h"
#include "fsl_os_abstraction.h"
#include "fsl_gpio.h"

#define PORT 0

static void GPIO_FlashTimeout(void *timerId);

/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/
/* Flashing Mode: indicates how many GPIOs are in flashing mode */
static uint32_t mFlashingGPIOs = 0;  /* flashing GPIOs */
#if gTMR_Enabled_d
/* GPIO timer ID */
static tmrTimerID_t mGPIOTimerID = gTmrInvalidTimerID_c;
#endif

/******************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/******************************************************************************
* Name: GPIO_InitTimer
* Description: Initialize timer for the GPIO module
* Parameters: -
* Return: -
******************************************************************************/
void GPIO_InitTimer
(
    void
)
{
    /* allocate a timer for use in flashing GPIOs */
#if gTMR_Enabled_d
    mGPIOTimerID = TMR_AllocateTimer();
#endif
}
/******************************************************************************
* Name: Un-initialize the GPIO module
* Description: frees GPIO timer
* Parameters: -
* Return: -
******************************************************************************/
void GPIO_UnInitTimer
(
void
)
{
#if gTMR_Enabled_d
    /* free the timer used for flashing mode */
    TMR_FreeTimer(mGPIOTimerID);
#endif
}
/******************************************************************************
* Name: GPIO_Set
* Description: set GPIO configured in pin to pinValue
* Param(s): -
* Return: -
******************************************************************************/
void GPIO_Set
(
    uint32_t pin,
    uint8_t pinValue
)
{
    if((1 << pin) & mFlashingGPIOs){
        GPIO_StopFlash(pin);
    }

    GPIO_PinWrite(GPIO, PORT, pin, pinValue);
}
/******************************************************************************
* Name: GpioGet
* Description: read GPIO pin value
* Param(s): pin
* Return: value
******************************************************************************/
uint32_t GPIO_Get(uint32_t pin){

    return GPIO_PinRead(GPIO, PORT, pin);
}
/******************************************************************************
* Name: GPIO_StartFlashWithPeriod
* Description: Starts flashing one or more GPIOs
* Parameters: [IN] GPIO_Pin_t pin       - GPIO Number (may be an OR of the list)
              [IN] uint16_t periodMs    - GPIO flashing period in milliseconds
* Return: -
******************************************************************************/
void GPIO_StartFlashWithPeriod
(
    GPIO_Pin_t pin,
    uint16_t periodMs
)
{

#if gTMR_Enabled_d

    /* indicate which GPIOs are flashing */
    mFlashingGPIOs |= (1 << pin);

    /* start the timer */
    if(!TMR_IsTimerActive(mGPIOTimerID))
    {
        TMR_StartIntervalTimer(mGPIOTimerID, periodMs, (pfTmrCallBack_t)GPIO_FlashTimeout, (void*)((uint32_t)mGPIOTimerID));
    }
#endif
}

/******************************************************************************
* Name: GPIO_StopFlash
* Description: Stop an GPIO from flashing.
* Parameters: [IN] GPIO_Pin_t pin - GPIO Number (may be an OR of the list)
* Return: -
******************************************************************************/
void GPIO_StopFlash
(
    GPIO_Pin_t pin
)
{
    /* stop flashing on one or more GPIOs */
    mFlashingGPIOs &= (~(1 << pin));

#if gTMR_Enabled_d
    /* if ALL GPIOs have stopped flashing, then stop timer */
    if(!mFlashingGPIOs)
    {
        TMR_StopTimer(mGPIOTimerID);
    }
#endif
}
/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

#if gTMR_Enabled_d
/******************************************************************************
* Name: GPIO_FlashTimeout
* Description: timer callback function that is called each time the timer
*              expires
* Param(s): [IN] timerId - the timer ID
* Return: -
******************************************************************************/
static void GPIO_FlashTimeout
(
	void *timerId
)
{
    (void) timerId;
    GPIO_PortToggle(GPIO, PORT, mFlashingGPIOs);
}
#endif /* gTMR_Enabled_d */
