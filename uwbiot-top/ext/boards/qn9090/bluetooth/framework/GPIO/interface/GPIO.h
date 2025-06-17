/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
* All rights reserved.
*
* \file
*
* GPIO export interface file for ARM CORTEX-M4 processor
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef _GPIO_H_
#define _GPIO_H_

#include "EmbeddedTypes.h"
#include "board.h"

/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/

/*
* Name: GPIO_Pin_t
* Description: GPIO defined pin
*/
typedef uint8_t GPIO_Pin_t;

/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/
/******************************************************************************
* Name: GpioSet
* Description: set GPIO configured in pin to pinValue
* Param(s): - pin, pinValue
* Return: -
******************************************************************************/
void GPIO_Set(uint32_t pin, uint8_t pinValue);
/******************************************************************************
* Name: GpioGet
* Description: read GPIO pin value
* Param(s): pin
* Return: -
******************************************************************************/
uint32_t GPIO_Get(uint32_t pin);
/******************************************************************************
* Name: GPIO_InitTimer
* Description: Initialize timer for the GPIO module
* Parameters: -
* Return: -
******************************************************************************/
extern void GPIO_InitTimer(void);
/******************************************************************************
* Name: Un-initialize the GPIO module
* Description: deallocates GPIO timer
* Parameters: -
* Return: -
******************************************************************************/
extern void GPIO_UnInitTimer(void);
/******************************************************************************
* Name: GPIO_StopFlash
* Description: Stop n GPIO from flashing.
* Parameters: [IN] GPIO_Pin_t pin - GPIO pin Number (may be an OR of the list)
* Return: -
******************************************************************************/
extern void GPIO_StopFlash(GPIO_Pin_t pin);
/******************************************************************************
* Name: GPIO_StartFlashWithPeriod
* Description: Starts flashing one or more GPIOs
* Parameters: [IN] GPIO_Pin_t pin       - GPIO Number (may be an OR of the list)
              [IN] uint16_t periodMs    - GPIO flashing period in milliseconds
* Return: -
******************************************************************************/
void GPIO_StartFlashWithPeriod(GPIO_Pin_t pin, uint16_t periodMs);

#endif /* _GPIO_H_ */
