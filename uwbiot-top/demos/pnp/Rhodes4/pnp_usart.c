/* Copyright 2020, 2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_PNP)

#include "board.h"
#include "fsl_usart_dma.h"
#include "Uwb_Vcom_Pnp.h"
#include <stdio.h>
#include <stdbool.h>
#include "phOsalUwb.h"
#include <UwbPnpInternal.h>
#include "phUwbStatus.h"
#include "phNxpLogApis_App.h"
#include "UwbPnpInternal.h"
#include "fsl_wwdt.h"
#include "UwbApi_Utility.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART           USART0
#define DEMO_USART_CLK_SRC   kCLOCK_Fro32M
#define DEMO_USART_CLK_FREQ  CLOCK_GetFreq(DEMO_USART_CLK_SRC)
#define DEMO_USART_IRQn      USART0_IRQn
#define USART_TX_DMA_CHANNEL 1
#define USART_RX_DMA_CHANNEL 0
#define UART_DMA_BASEADDR    DMA0
#define FTDI_BUFFER_SIZE     256

/* doc-start:wtd */
/*
 * Reset the full board for recovery.
 *
 * If there's no communicaiton between the PnP FW
 * and host for these much time, the PnP FW would
 * auto reset.
 *
 * /!\ IMPORTANT /!\
 * /!\
 * /!\ A side effect of this Watchdog reset is that the Host has to
 * /!\ flush out and read all the previous packets from the RV4 board
 * /!\ on a new connection.
 * /!\
 * /!\ These packets are either buffered at FTDI, or due to the WatchDog
 * /!\ reset of QN9090, some garbage values are sensed/fetched by the
 * /!\ FTDI.
 * /!\
 * /!\ IMPORTANT /!\
 *
 * Set this to 0 to disable the Watch Dog Timer.
 *
 * Set this to non zero value to enable the watch dog timer behavior.
 *
 */
#define WATCHDOG_TIMEOUT_SECONDS 0
/* doc-end:wtd */

/* Supported Baudrate
 * 115200
 * 1000000 (1 Mbps)
 * 1500000 (1.5 Mbps)
 * 3000000 (3 Mbps)
 */
#define DEMO_USART_BAUDRATE 3000000

#if (cPWR_UsePowerDownMode) || (cPWR_FullPowerDownMode)
#pragma message( \
    "DEMO_USART_BAUDRATE = 3Mbps : Disable low power mode from app_preinclude.h file & perform clean build AND always use release build to work with higher baud-rate")
#endif //(cPWR_UsePowerDownMode) || (cPWR_FullPowerDownMode)

#define UART_HEADER_SIZE 3

/*FTDI has TX/Rx FIFO of 512 Bytes */
#define MAX_TRANSFER_COUNT 512

#define USART_RCV_HDR_NON_BLOCKING(BUFFER, SIZE, RCVD_BYTES_PTR) \
    rcvXfer.data     = BUFFER;                                   \
    rcvXfer.dataSize = SIZE;                                     \
    USART_TransferReceiveDMA(DEMO_USART, &g_uartDmaHandle, &rcvXfer);

usart_dma_handle_t g_uartDmaHandle;
dma_handle_t g_uartTxDmaHandle;
dma_handle_t g_uartRxDmaHandle;
void *mHifNtfnSem = NULL;
void *mHifRspSem  = NULL;
static uint8_t buffer[4096];
static usart_transfer_t sendXfer;
static usart_transfer_t rcvXfer;
volatile bool mError = FALSE;

static void (*usartRcvCb)(uint8_t *, uint32_t *);
void USART_UserCallback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData);

#define WDT_CLK_FREQ CLOCK_GetFreq(kCLOCK_WdtOsc)
#define APP_WDT_IRQn WDT_BOD_IRQn

#if WATCHDOG_TIMEOUT_SECONDS
void uwb_pnp_board_watchdog_init(uint32_t timout_seconds)
{
    wwdt_config_t config;
    uint32_t wdtFreq;
    WWDT_GetDefaultConfig(&config);

#if ((defined APP_WWDT_ENABLE) && (APP_WWDT_ENABLE))
    /* done */
#else
    CLOCK_AttachClk(kOSC32K_to_WDT_CLK); /*!< Switch WDT_CLK to OSC32K */
#endif

    /* The WDT divides the input frequency into it by 4 */
    wdtFreq = WDT_CLK_FREQ / 4;

    config.timeoutValue = wdtFreq * timout_seconds;
    //    config.warningValue = 512;
    //    config.windowValue  = wdtFreq * 1;
    /* Configure WWDT to reset on timeout */
    config.enableWatchdogReset = TRUE;
    /* Setup watchdog clock frequency(Hz). */
    config.clockFreq_Hz = WDT_CLK_FREQ;
    WWDT_Init(WWDT, &config);

    NVIC_EnableIRQ(APP_WDT_IRQn);
}

void uwb_pnp_board_watchdog_deferred_init()
{
    static bool uwb_pnp_board_watchdog_initialized = FALSE;
    if (!uwb_pnp_board_watchdog_initialized) {
        uwb_pnp_board_watchdog_init(WATCHDOG_TIMEOUT_SECONDS);
        uwb_pnp_board_watchdog_initialized = TRUE;
    }
}

void uwb_pnp_board_watchdog_refresh()
{
    uwb_pnp_board_watchdog_deferred_init();
    WWDT_Refresh(WWDT);
}

#endif /* WATCHDOG_TIMEOUT_SECONDS */

void Uwb_USART_Init(void (*rcvCb)(uint8_t *, uint32_t *))
{
    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending Notifications from Rhodes*/
    if (phOsalUwb_CreateBinSem(&mHifNtfnSem) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifNtfnSem\n");
        while (1)
            ;
    }
    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending UCI resp from Rhodes*/
    if (phOsalUwb_CreateBinSem(&mHifRspSem) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifRspSem\n");
        while (1)
            ;
    }

    usart_config_t config;

    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = DEMO_USART_BAUDRATE;
    config.enableTx     = TRUE;
    config.enableRx     = TRUE;
#if HW_FLOW_CONTROL_SUPPORT
    config.enableHardwareFlowControl = TRUE;
#endif
    usartRcvCb = rcvCb;

    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
    /*NVIC priority dor DMA IRQn is set in spi init so disabling here*/
    //NVIC_SetPriority(DMA0_IRQn, USART_DEVICE_INTERRUPT_PRIORITY + 2);

    /* DMA Init is done SPI init so disabling here*/
    //DMA_Init(UART_DMA_BASEADDR);
    DMA_EnableChannel(UART_DMA_BASEADDR, USART_TX_DMA_CHANNEL);
    DMA_EnableChannel(UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

    DMA_CreateHandle(&g_uartTxDmaHandle, UART_DMA_BASEADDR, USART_TX_DMA_CHANNEL);
    DMA_CreateHandle(&g_uartRxDmaHandle, UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

    /* Create UART DMA handle. */
    USART_TransferCreateHandleDMA(
        DEMO_USART, &g_uartDmaHandle, USART_UserCallback, NULL, &g_uartTxDmaHandle, &g_uartRxDmaHandle);

    USART_RCV_HDR_NON_BLOCKING(buffer, UART_HEADER_SIZE, &rxReceivedBytes);

    //uwb_pnp_board_watchdog_init(3);
}

void USART_UserCallback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_USART_TxIdle == status) {
        (void)phOsalUwb_ProduceSemaphore(mHifRspSem);
    }

    if (kStatus_USART_RxIdle == status) {
        uint32_t payloadLength = (buffer[UART_HEADER_SIZE - 2] << (8 * 1)) | (buffer[UART_HEADER_SIZE - 1] << (8 * 0));

        if (payloadLength > 0) {
            if (payloadLength < (FTDI_BUFFER_SIZE - UART_HEADER_SIZE)) {
                // uwb_pnp_reload_timer();
            }
            else if (payloadLength > (sizeof(buffer) - UART_HEADER_SIZE)) {
                /* protocol error */
                uwb_pnp_board_protocol_error_handler();
            }

            status = USART_ReadBlocking(base, (&buffer[UART_HEADER_SIZE]), payloadLength);
        }
        payloadLength += UART_HEADER_SIZE;
        if (usartRcvCb) {
            usartRcvCb(buffer, &payloadLength);
        }
        phOsalUwb_SetMemory(buffer, 0, sizeof(buffer));
#if WATCHDOG_TIMEOUT_SECONDS
        uwb_pnp_board_watchdog_refresh();
#endif // WATCHDOG_TIMEOUT_SECONDS
        USART_RCV_HDR_NON_BLOCKING(buffer, UART_HEADER_SIZE, NULL);
    }
}

uint32_t transmitToUsart(uint8_t *pData, size_t size)
{
    status_t status;
    if (size == 0) {
        return 1;
    }

#if WATCHDOG_TIMEOUT_SECONDS
    uwb_pnp_board_watchdog_refresh();
#endif

    if (size <= DMA_MAX_TRANSFER_COUNT) {
        sendXfer.data     = pData;
        sendXfer.dataSize = size;
        status            = USART_TransferSendDMA(DEMO_USART, &g_uartDmaHandle, &sendXfer);
        if (status == kStatus_Success) {
            if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifRspSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                UWBSTATUS_SUCCESS) {
                PRINTF_WITH_TIME("transmitToUsart timeout");
                return kStatus_Fail;
            }
        }
    }
    else {
        int nchunks = 0, remainigBytes = 0;
        nchunks       = size / DMA_MAX_TRANSFER_COUNT;
        remainigBytes = size - nchunks * DMA_MAX_TRANSFER_COUNT;
        for (int i = 0; i < nchunks; i++) {
            sendXfer.data     = pData + (i * DMA_MAX_TRANSFER_COUNT);
            sendXfer.dataSize = DMA_MAX_TRANSFER_COUNT;
            status            = USART_TransferSendDMA(DEMO_USART, &g_uartDmaHandle, &sendXfer);
            if (status == kStatus_Success) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifRspSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    PRINTF_WITH_TIME("transmitToUsart timeout");
                    return kStatus_Fail;
                }
            }
        }
        if (remainigBytes != 0) {
            sendXfer.data     = pData + (nchunks * DMA_MAX_TRANSFER_COUNT);
            sendXfer.dataSize = remainigBytes;
            status            = USART_TransferSendDMA(DEMO_USART, &g_uartDmaHandle, &sendXfer);
            if (status == kStatus_Success) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifRspSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    PRINTF_WITH_TIME("transmitToUsart timeout");
                    return kStatus_Fail;
                }
                return (uint32_t)kStatus_Success;
            }
        }
    }
    return (uint32_t)kStatus_Success;
}

void uwb_pnp_board_protocol_error_handler()
{
    RESET_SystemReset();
}

#endif // defined(UWBIOT_APP_BUILD__DEMO_PNP)
