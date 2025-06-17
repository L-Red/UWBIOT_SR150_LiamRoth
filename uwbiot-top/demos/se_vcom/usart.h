/*
 *
 * Copyright 2021 NXP.
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

#ifndef _USART_VCOM_H_
#define _USART_VCOM_H_

#include <stdint.h>
#ifndef UWBIOT_APP_BUILD__SE_VCOM
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__SE_VCOM
uint32_t transmitToUsart_vcom(uint8_t *pData, size_t size);
void USART_Initialize(void (*rcvCb)(uint8_t *, uint32_t));
void USART_RX_Data(void);

#endif // UWBIOT_APP_BUILD__SE_VCOM
#endif /* _UWB_USART_VCOM_PNP_H_ */
