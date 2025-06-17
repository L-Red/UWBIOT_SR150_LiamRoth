/* Copyright 2021,2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef _UWB_USART_VCOM_H_
#define _UWB_USART_VCOM_H_

#include <stdint.h>
#include "fsl_usart.h"
#include "LED.h"
#include "GPIO.h"
#include "driver_config.h"
#include "fsl_gpio.h"
#include "phUwb_BuildConfig.h"

uint32_t transmitToUsart(uint8_t *pData, size_t size);
uint32_t USART_SendBlocking(uint8_t *pData, size_t size);
void Uwb_USART_Init(void (*rcvCb)(uint8_t *, uint32_t *));
void Uwb_USART_DeInit();

#define UWB_Serial_Com_Init             Uwb_USART_Init
#define UWB_Serial_Com_DeInit           Uwb_USART_DeInit
#define UWB_Serial_Com_SendRsp          transmitToUsart
#define USART_DEVICE_INTERRUPT_PRIORITY (3U)

#define SET_LED_RED()     GPIO_PinWrite(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, 0)
#define SET_LED_BLUE()    GPIO_PinWrite(GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, 0)
#define SET_LED_GREEN()   GPIO_PinWrite(GPIO, BOARD_LED_GREEN_GPIO_PORT, BOARD_LED_GREEN_GPIO_PIN, 0)
#define CLEAR_LED_RED()   GPIO_PinWrite(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, 1)
#define CLEAR_LED_BLUE()  GPIO_PinWrite(GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, 1)
#define CLEAR_LED_GREEN() GPIO_PinWrite(GPIO, BOARD_LED_GREEN_GPIO_PORT, BOARD_LED_GREEN_GPIO_PIN, 1)

#endif /* _UWB_USART_VCOM_PNP_H_ */
