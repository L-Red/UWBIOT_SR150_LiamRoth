/*
  * Copyright (C) 2012-2022 NXP Semiconductors
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *      http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

/* System includes */

#include <stdint.h>
#include <uwb_bus_board.h>
#include <uwb_bus_interface.h>

/* Freescale includes*/
#include "board.h"

/* UWB includes */
#include "driver_config.h"
#include "phUwbTypes.h"

#include "phOsalUwb.h"
#include "phUwb_BuildConfig.h"
#include "phUwbErrorCodes.h"
#include "phNxpLogApis_TmlUwb.h"

#define SPI_MASTER_CLK_SRC  kCLOCK_Fro32M
#define SPI_MASTER_CLK_FREQ CLOCK_GetFreq(SPI_MASTER_CLK_SRC)
#define SPI_MASTER_IRQ      SPI1_IRQn

#define DMA_BASE DMA0

#define DMA_DEVICE_INTERRUPT_PRIORITY (4U)

#define SPI_MASTER_RX_CHANNEL   10
#define SPI_MASTER_TX_CHANNEL   11
#define EXAMPLE_MASTER_SPI_SPOL kSPI_SpolActiveAllLow

static status_t gtransferStatus;

/* This semaphore is signaled when SPI write is completed successfully*/
void *mSpiTransferSem = NULL;

void SPI_MasterUserCallback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData)
{
    gtransferStatus = status;
    (void)phOsalUwb_ProduceSemaphore(mSpiTransferSem);
}

uwb_bus_status_t uwb_bus_init(uwb_bus_board_ctx_t *pCtx)
{
    spi_master_config_t masterConfig;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    CLOCK_AttachClk(kOSC32M_to_SPI_CLK); /*!< Switch SPI_CLK to OSC32M */

    /* Init CE */
    if (kUWB_bus_Status_OK != uwb_bus_io_val_set(pCtx, kUWBS_IO_O_ENABLE_HELIOS, kUWBS_IO_State_Low)) {
        LOG_E("Error %s: could not set io value", __FUNCTION__);
        return kUWB_bus_Status_FAILED;
    }

    if (kUWB_bus_Status_OK != uwb_bus_io_val_set(pCtx, kUWBS_IO_O_HELIOS_SYNC, kUWBS_IO_State_Low)) {
        LOG_E("Error %s: could not set io value", __FUNCTION__);
        return kUWB_bus_Status_FAILED;
    }

    /* DMA init */
    DMA_Init(DMA_BASE);

    /*CPOL =0, CPHA = 0*/
    /* Master config */
    SPI_MasterGetDefaultConfig(&masterConfig);

    masterConfig.baudRate_Bps = UWB_SPI_BAUDRATE;
    masterConfig.dataWidth    = kSPI_Data8Bits;
    masterConfig.polarity     = kSPI_ClockPolarityActiveHigh;
    masterConfig.phase        = kSPI_ClockPhaseFirstEdge;
    masterConfig.direction    = kSPI_MsbFirst;

    masterConfig.delayConfig.preDelay      = 15U;
    masterConfig.delayConfig.postDelay     = 15U;
    masterConfig.delayConfig.frameDelay    = 15U;
    masterConfig.delayConfig.transferDelay = 15U;

    masterConfig.sselPol = kSPI_SpolActiveAllLow;
    masterConfig.sselNum = UWB_SPI_SSEL;

    masterConfig.txWatermark = kSPI_TxFifo0;
    masterConfig.rxWatermark = kSPI_RxFifo1;

    masterConfig.enableLoopback = false;
    masterConfig.enableMaster   = true;

    SPI_MasterInit(UWB_SPI_BASEADDR, &masterConfig, CLOCK_GetFreq(kCLOCK_Fro32M));

    NVIC_SetPriority(DMA0_IRQn, DMA_DEVICE_INTERRUPT_PRIORITY + 2);

    DMA_EnableChannel(DMA_BASE, SPI_MASTER_TX_CHANNEL);
    DMA_EnableChannel(DMA_BASE, SPI_MASTER_RX_CHANNEL);
    // DMA_SetChannelPriority(DMA_BASE, SPI_MASTER_TX_CHANNEL, kDMA_ChannelPriority0);
    // DMA_SetChannelPriority(DMA_BASE, SPI_MASTER_RX_CHANNEL, kDMA_ChannelPriority1);
    DMA_CreateHandle(&pCtx->masterTxHandle, DMA_BASE, SPI_MASTER_TX_CHANNEL);
    DMA_CreateHandle(&pCtx->masterRxHandle, DMA_BASE, SPI_MASTER_RX_CHANNEL);

    /* Set up spi master */
    SPI_MasterTransferCreateHandleDMA(UWB_SPI_BASEADDR,
        &pCtx->masterHandle,
        SPI_MasterUserCallback,
        NULL,
        &pCtx->masterTxHandle,
        &pCtx->masterRxHandle);

    /* This semaphore is signaled when SPI data is send out completely Rhodes*/
    if (phOsalUwb_CreateBinSem(&mSpiTransferSem) != UWBSTATUS_SUCCESS) {
        LOG_E("Error: uwb_bus_init(), could not create semaphore mSpiTransferSem\n");
        return kUWB_bus_Status_FAILED;
    }

    /*This semaphore is signaled in the ISR context.*/
    if (phOsalUwb_CreateSemaphore(&pCtx->mIrqWaitSem, 0) != kUWBSTATUS_SUCCESS) {
        LOG_E("Error: uwb_uwbs_tml_init(), could not create semaphore mWaitIrqSem\n");
        return kUWB_bus_Status_FAILED;
    }

    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_deinit(uwb_bus_board_ctx_t *pCtx)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    SPI_Deinit(UWB_SPI_BASEADDR);

    phOsalUwb_DeleteSemaphore(&mSpiTransferSem);
    phOsalUwb_ProduceSemaphore(pCtx->mIrqWaitSem);
    phOsalUwb_Delay(2);
    phOsalUwb_DeleteSemaphore(&pCtx->mIrqWaitSem);
    mSpiTransferSem = NULL;
    phOsalUwb_SetMemory(pCtx, 0, sizeof(uwb_bus_board_ctx_t));
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_data_tx(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t bufLen)
{
    spi_transfer_t xfer;
    status_t status;
    uwb_bus_status_t bus_status = kUWB_bus_Status_FAILED;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        goto end;
    }

    if (pBuf == NULL || bufLen == 0) {
        goto end;
    }

    /* Send header */
    xfer.rxData      = NULL;
    xfer.configFlags = kSPI_FrameAssert;

    if (bufLen <= DMA_MAX_TRANSFER_COUNT) {
        xfer.txData   = pBuf;
        xfer.dataSize = bufLen;
        status        = SPI_MasterTransferDMA(UWB_SPI_BASEADDR, &pCtx->masterHandle, &xfer);
        if (status == kStatus_Success) {
            if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                UWBSTATUS_SUCCESS) {
                LOG_E("%s : spi transfer timeout", __FUNCTION__);
                goto end;
            }
            if (gtransferStatus != kStatus_Success) {
                goto end;
            }
            bus_status = kUWB_bus_Status_OK;
        }
    }
    else {
        int nchunks = 0, remainigBytes = 0;
        nchunks       = bufLen / DMA_MAX_TRANSFER_COUNT;
        remainigBytes = bufLen - nchunks * DMA_MAX_TRANSFER_COUNT;

        for (int i = 0; i < nchunks; i++) {
            xfer.txData   = &pBuf[i * DMA_MAX_TRANSFER_COUNT];
            xfer.dataSize = DMA_MAX_TRANSFER_COUNT;
            status        = SPI_MasterTransferDMA(UWB_SPI_BASEADDR, &pCtx->masterHandle, &xfer);
            if (status == kStatus_Success) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    LOG_E("%s : spi transfer timeout", __FUNCTION__);
                    goto end;
                }
                if (gtransferStatus != kStatus_Success) {
                    goto end;
                }
            }
        }
        if (remainigBytes != 0) {
            xfer.txData   = &pBuf[nchunks * DMA_MAX_TRANSFER_COUNT];
            xfer.dataSize = remainigBytes;
            status        = SPI_MasterTransferDMA(UWB_SPI_BASEADDR, &pCtx->masterHandle, &xfer);
            if (status == kStatus_Success) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    LOG_E("%s : spi transfer timeout", __FUNCTION__);
                    goto end;
                }
                if (gtransferStatus != kStatus_Success) {
                    goto end;
                }
                bus_status = kUWB_bus_Status_OK;
            }
        }
        else {
            bus_status = kUWB_bus_Status_OK;
        }
    }

end:
    return bus_status;
}

uwb_bus_status_t uwb_bus_data_rx(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t pBufLen)
{
    spi_transfer_t xfer;
    status_t status;
    uwb_bus_status_t bus_status = kUWB_bus_Status_FAILED;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        goto end;
    }

    if (pBuf == NULL || pBufLen == 0) {
        goto end;
    }

    /* Send header */
    xfer.txData      = NULL;
    xfer.configFlags = kSPI_FrameAssert;

    if (pBufLen <= DMA_MAX_TRANSFER_COUNT) {
        xfer.rxData   = pBuf;
        xfer.dataSize = pBufLen;
        status        = SPI_MasterTransferDMA(UWB_SPI_BASEADDR, &pCtx->masterHandle, &xfer);
        if (status == kStatus_Success) {
            if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                UWBSTATUS_SUCCESS) {
                LOG_E("%s : spi transfer timeout", __FUNCTION__);
                goto end;
            }
            if (gtransferStatus != kStatus_Success) {
                goto end;
            }
            bus_status = kUWB_bus_Status_OK;
        }
    }
    else {
        int nchunks = 0, remainigBytes = 0;
        nchunks       = pBufLen / DMA_MAX_TRANSFER_COUNT;
        remainigBytes = pBufLen - nchunks * DMA_MAX_TRANSFER_COUNT;

        for (int i = 0; i < nchunks; i++) {
            xfer.rxData   = &pBuf[i * DMA_MAX_TRANSFER_COUNT];
            xfer.dataSize = DMA_MAX_TRANSFER_COUNT;
            status        = SPI_MasterTransferDMA(UWB_SPI_BASEADDR, &pCtx->masterHandle, &xfer);
            if (status == kStatus_Success) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    LOG_E("%s : spi transfer timeout", __FUNCTION__);
                    goto end;
                }
                if (gtransferStatus != kStatus_Success) {
                    goto end;
                }
            }
        }
        if (remainigBytes != 0) {
            xfer.rxData   = &pBuf[nchunks * DMA_MAX_TRANSFER_COUNT];
            xfer.dataSize = remainigBytes;
            status        = SPI_MasterTransferDMA(UWB_SPI_BASEADDR, &pCtx->masterHandle, &xfer);
            if (status == kStatus_Success) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    LOG_E("%s : spi transfer timeout", __FUNCTION__);
                    goto end;
                }
                if (gtransferStatus != kStatus_Success) {
                    goto end;
                }
                bus_status = kUWB_bus_Status_OK;
            }
        }
        else {
            bus_status = kUWB_bus_Status_OK;
        }
    }

end:
    return bus_status;
}

uwb_bus_status_t uwb_bus_reset(uwb_bus_board_ctx_t *pCtx)
{
    /* Not needed here */
    return kUWB_bus_Status_OK;
}

//TODO : this API is not yet calibrated for platform
void uwb_port_DelayinMicroSec(int delay)
{
    volatile int cnt = 6 * delay;
    while (cnt--)
        ;
}