/* Copyright 2021-2023 NXP
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
#if !defined(UWBIOT_APP_BUILD__DEMO_PNP)

#include "board.h"
#include "fsl_usart_dma.h"
#include "usart_vcom_qn9090.h"
#include "phOsalUwb.h"
#include "phUwbStatus.h"
#include "phNxpLogApis_App.h"
#include <assert.h>
#include "UwbHif.h"

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

/*FTDI has TX/Rx FIFO of 512 Bytes */
#define MAX_TRANSFER_COUNT              512
#define MAX_UWBS_SPI_TRANSFER_TIMEOUT   (1000)
#define USART_DEVICE_INTERRUPT_PRIORITY (3U)
#define USART_RCV_HDR_NON_BLOCKING(BUFFER, SIZE, RCVD_BYTES_PTR) \
    gRcvXfer.data     = BUFFER;                                  \
    gRcvXfer.dataSize = SIZE;                                    \
    USART_TransferReceiveDMA(DEMO_USART, &g_uartDmaHandle, &gRcvXfer);

usart_dma_handle_t g_uartDmaHandle;
dma_handle_t g_uartTxDmaHandle;
dma_handle_t g_uartRxDmaHandle;
void *mHifNtfnSem = NULL;
void *mHifRspSem  = NULL;
static uint8_t gRxBuffer[4096];
static usart_transfer_t gSendXfer;
static usart_transfer_t gRcvXfer;

static void (*usartRcvCb)(uint8_t *, uint32_t *);
void USART_UserCallback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData);

void Uwb_USART_Init(void (*rcvCb)(uint8_t *, uint32_t *))
{
    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending UCI resp from Rhodes*/
    if (phOsalUwb_CreateBinSem(&mHifRspSem) != UWBSTATUS_SUCCESS) {
        PRINTF("Error: main, could not create semaphore mHifRspSem\n");
        while (1)
            ;
    }

    usart_config_t config;

    uint8_t uartHeaderSize = HEADER_SIZE_CDC;
    if (HifGetMode() == kUWB_MODE_MCTT) {
        uartHeaderSize = HEADER_SIZE_MCTT;
    }

    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 3000000;
    config.enableTx     = TRUE;
    config.enableRx     = TRUE;
#if ((defined(HW_FLOW_CONTROL_SUPPORT)) && (HW_FLOW_CONTROL_SUPPORT != 0))
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

    USART_RCV_HDR_NON_BLOCKING(gRxBuffer, uartHeaderSize, &rxReceivedBytes);
}

void USART_UserCallback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData)
{
    uint8_t uartHeaderSize = HEADER_SIZE_CDC;
    if (HifGetMode() == kUWB_MODE_MCTT) {
        uartHeaderSize = HEADER_SIZE_MCTT;
    }
    userData = userData;

    if (kStatus_USART_TxIdle == status) {
        (void)phOsalUwb_ProduceSemaphore(mHifRspSem);
    }

    if (kStatus_USART_RxIdle == status) {
        uint32_t payloadLength = 0;
        if (gRxBuffer[MCTT_GID_INDEX] == MCTT_SE_GID_HEARDER) {
            payloadLength = gRxBuffer[MCTT_DATA_PAYLOD_LENGTH_INDEX];
        }
        else if ((gRxBuffer[MCTT_GID_INDEX] == MCTT_DATA_SEND) || (gRxBuffer[MCTT_GID_INDEX] == MCTT_LL_DATA_SEND)) {
            payloadLength = gRxBuffer[MCTT_DATA_PAYLOD_LENGTH_INDEX] + (gRxBuffer[MCTT_DATA_PAYLOD_LENGTH_INDEX + 1] << 8);
        }
        else {
            payloadLength = (gRxBuffer[uartHeaderSize - 2] << (8 * 1)) | (gRxBuffer[uartHeaderSize - 1] << (8 * 0));
        }

        if (payloadLength > 0) {
            status = USART_ReadBlocking(base, (&gRxBuffer[uartHeaderSize]), payloadLength);
            if (status != kStatus_Success) {
                assert(0);
            }
        }
        payloadLength += uartHeaderSize;
        if (usartRcvCb) {
            usartRcvCb(gRxBuffer, &payloadLength);
        }
        phOsalUwb_SetMemory(gRxBuffer, 0, sizeof(gRxBuffer));
        USART_RCV_HDR_NON_BLOCKING(gRxBuffer, uartHeaderSize, NULL);
    }
}

uint32_t transmitToUsart(uint8_t *pData, size_t size)
{
    status_t status;
    if (size == 0) {
        return 1;
    }
    if (size <= DMA_MAX_TRANSFER_COUNT) {
        gSendXfer.data     = pData;
        gSendXfer.dataSize = size;
        status             = USART_TransferSendDMA(DEMO_USART, &g_uartDmaHandle, &gSendXfer);
        if (status == kStatus_Success) {
            if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifRspSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                UWBSTATUS_SUCCESS) {
                PRINTF("transmitToUsart timeout");
                return kStatus_Fail;
            }
        }
    }
    else {
        int nchunks = 0, remainigBytes = 0;
        nchunks       = size / DMA_MAX_TRANSFER_COUNT;
        remainigBytes = size - nchunks * DMA_MAX_TRANSFER_COUNT;
        for (int i = 0; i < nchunks; i++) {
            gSendXfer.data     = pData + (i * DMA_MAX_TRANSFER_COUNT);
            gSendXfer.dataSize = DMA_MAX_TRANSFER_COUNT;
            status             = USART_TransferSendDMA(DEMO_USART, &g_uartDmaHandle, &gSendXfer);
            if (status == kStatus_Success) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifRspSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    PRINTF("transmitToUsart timeout");
                    return kStatus_Fail;
                }
            }
        }
        if (remainigBytes != 0) {
            gSendXfer.data     = pData + (nchunks * DMA_MAX_TRANSFER_COUNT);
            gSendXfer.dataSize = remainigBytes;
            status             = USART_TransferSendDMA(DEMO_USART, &g_uartDmaHandle, &gSendXfer);
            if (status == kStatus_Success) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifRspSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    PRINTF("transmitToUsart timeout");
                    return kStatus_Fail;
                }
                return (uint32_t)kStatus_Success;
            }
        }
    }
    return (uint32_t)kStatus_Success;
}

void Uwb_USART_DeInit()
{
    phOsalUwb_DeleteSemaphore(&mHifRspSem);
    USART_Deinit(DEMO_USART);
}
#endif // !defined(UWBIOT_APP_BUILD__DEMO_PNP)
