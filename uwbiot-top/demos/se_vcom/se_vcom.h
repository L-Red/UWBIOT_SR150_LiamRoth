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
#ifndef VCOM_H
#define VCOM_H

#ifndef UWBIOT_APP_BUILD__SE_VCOM
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__SE_VCOM
#if defined(QN9090DK6)
#include "fsl_device_registers.h"
#include "fsl_usart.h"
#include "clock_config.h"
#endif
#include "board.h"

#define RJCT_OK       0x0000
#define RJCT_ARG_FAIL 0x6000

#define WAIT_FOR_CARD 0
#define APDU_DATA     1
#define CLOSE_CONN    3

#define USB_CDC_VCOM_INTERFACE_COUNT (2)
#define SMCOM_OK                     0x9000 //!< Communication successful

void state_vcom_2_i2c(void);
void Handle_TLV(uint8_t *buffer, uint32_t length);

#endif //UWBIOT_APP_BUILD__SE_VCOM

#endif /* VCOM_H */
