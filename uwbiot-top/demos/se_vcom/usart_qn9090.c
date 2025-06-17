/*
 *
 * Copyright 2021-2022 NXP.
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

#if defined(QN9090DK6)
#include "board.h"
#include "fsl_usart.h"
#include "usart.h"
#include <stdbool.h>
#include "phOsalUwb.h"
//#include <UwbPnpInternal.h>

#include "fsl_debug_console.h"
#include "se_vcom.h"
#ifndef UWBIOT_APP_BUILD__SE_VCOM
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__SE_VCOM
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART          USART0
#define DEMO_USART_CLK_SRC  kCLOCK_Fro32M
#define DEMO_USART_CLK_FREQ CLOCK_GetFreq(DEMO_USART_CLK_SRC)

#define UART_HEADER_SIZE 4

usart_handle_t g_uart_Handle;
static uint8_t buffer[4096];
static usart_transfer_t rcvXfer;
volatile bool txOnGoing = FALSE;

void USART_Initialize(void (*rcvCb)(uint8_t *, uint32_t))
{
    rcvXfer.data     = buffer;
    rcvXfer.dataSize = UART_HEADER_SIZE;

    usart_config_t config;

    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 3000000;
    config.enableTx     = TRUE;
    config.enableRx     = TRUE;

    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
    USART_TransferCreateHandle(DEMO_USART, &g_uart_Handle, NULL, NULL);
}

void USART_RX_Data()
{
    status_t status = USART_ReadBlocking(DEMO_USART, (&buffer[0]), UART_HEADER_SIZE);
    if (kStatus_Success != status) {
        return;
    }
    uint32_t payloadLength = (buffer[UART_HEADER_SIZE - 2] << (8 * 1)) | (buffer[UART_HEADER_SIZE - 1] << (8 * 0));
    if (payloadLength > 0) {
        status = USART_ReadBlocking(DEMO_USART, (&buffer[UART_HEADER_SIZE]), payloadLength);
        if (kStatus_Success != status) {
            return;
        }
    }
    payloadLength += UART_HEADER_SIZE;
    Handle_TLV(buffer, payloadLength);
}

uint32_t transmitToUsart_vcom(uint8_t *pData, size_t size)
{
    if (size == 0) {
        return (uint32_t)kStatus_Fail;
    }

    USART_WriteBlocking(DEMO_USART, pData, size);
    return (uint32_t)kStatus_Success;
}
#endif // UWBIOT_APP_BUILD__SE_VCOM
#endif // #if defined(QN9090DK6)