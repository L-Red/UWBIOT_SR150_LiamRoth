/*
  * Copyright (C) 2021-2022 NXP Semiconductors
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

#include <uwb_uwbs_tml_interface.h>
#include <uwb_bus_board.h>
#include "phUwbTypes.h"

#include "phOsalUwb.h"
#include "phNxpUwb_Common.h"
#include "phUwb_BuildConfig.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phUwbErrorCodes.h"
#include "driver_config.h"

#if UWBIOT_UWBD_SR1XXT

#if UWBIOT_OS_NATIVE
#include <unistd.h>
#endif

#define PAYLOAD_LEN_MSB 0x02
#define PAYLOAD_LEN_LSB 0x03

#define UCI_RX_HDR_LEN             0x04
#define NORMAL_MODE_LEN_OFFSET     0x03
#define EXTND_LEN_INDICATOR_OFFSET 0x01
#define EXTENDED_LENGTH_OFFSET     0x02

#define UCI_HDR_LEN 0x04

#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80
// Wait for 10 Sec for read irq otherwise this will wake up the reader thread multiple times.
#define READ_IRQ_WAIT_TIME (10000)

#define MAX_RETRY_COUNT 0x02

UWBStatus_t uwb_uwbs_tml_init(uwb_uwbs_tml_ctx_t *pCtx)
{
    UWBStatus_t status;
    uwb_bus_status_t bus_status;

    if (pCtx == NULL) {
        LOG_E("uwbs tml context is NULL");
        status = kUWBSTATUS_INVALID_PARAMETER;
        goto exit;
    }

    phOsalUwb_SetMemory(pCtx, 0, sizeof(uwb_uwbs_tml_ctx_t));

    // by default set tml mode to UCI
    pCtx->mode = kUWB_UWBS_TML_MODE_UCI;

    status = (UWBStatus_t)phOsalUwb_CreateMutex(&(pCtx->mSyncMutex));
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("Error: uwb_uwbs_tml_init(), could not create mutex mSyncMutex\n");
        goto exit;
    }

    bus_status = uwb_bus_init(&pCtx->busCtx);
    if (bus_status != kUWB_bus_Status_OK) {
        status = kUWBSTATUS_CONNECTION_FAILED;
        LOG_E("Error: uwb_uwbs_tml_init(), uwb bus initialisation failed");
        goto exit;
    }
    status = kUWBSTATUS_SUCCESS;
exit:
    return status;
}

UWBStatus_t uwb_uwbs_tml_setmode(uwb_uwbs_tml_ctx_t *pCtx, uwb_uwbs_tml_mode_t mode)
{
    if (pCtx == NULL) {
        LOG_E("uwbs tml context is NULL");
        return kUWBSTATUS_INVALID_PARAMETER;
    }
    pCtx->mode = mode;
    return kUWBSTATUS_SUCCESS;
}

UWBStatus_t uwb_uwbs_tml_deinit(uwb_uwbs_tml_ctx_t *pCtx)
{
    phOsalUwb_DeleteMutex(&(pCtx->mSyncMutex));
    uwb_bus_deinit(&pCtx->busCtx);
    phOsalUwb_SetMemory(pCtx, 0, sizeof(uwb_uwbs_tml_ctx_t));
    return kUWBSTATUS_SUCCESS;
}

UWBStatus_t uwb_uwbs_tml_data_tx(uwb_uwbs_tml_ctx_t *pCtx, uint8_t *pBuf, size_t bufLen)
{
    uwb_bus_status_t bus_status;
    UWBStatus_t status = kUWBSTATUS_FAILED;

    if (pCtx == NULL) {
        LOG_E("uwbs tml context is NULL");
        status = kUWBSTATUS_INVALID_PARAMETER;
        return status;
    }

    pCtx->noOfBytesWritten = -1;

    if (pBuf == NULL || bufLen == 0) {
        LOG_E("write buffer is Null or bufLen is 0");
        status = kUWBSTATUS_INVALID_PARAMETER;
        return status;
    }

    phOsalUwb_LockMutex(pCtx->mSyncMutex);

    if (pCtx->mode == kUWB_UWBS_TML_MODE_HBCI) {
        bus_status = uwb_bus_data_tx(&pCtx->busCtx, pBuf, bufLen);
        if (bus_status == kUWB_bus_Status_FAILED) {
            LOG_E("uwb_uwbs_tml_data_tx writing HBCI Command failed");
            goto end;
        }
    }
    else if (pCtx->mode == kUWB_UWBS_TML_MODE_UCI) {
        /*pre write delay to allow the device to recover from the last batch of data */
        phOsalUwb_Delay(2);
        bus_status = uwb_bus_data_tx(&pCtx->busCtx, pBuf, UCI_HDR_LEN);
        if (bus_status == kUWB_bus_Status_FAILED) {
            LOG_E("uwb_uwbs_tml_data_tx writing UCI header failed");
            goto end;
        }

        uwb_port_DelayinMicroSec(80);
        pCtx->noOfBytesWritten = (uint16_t)(UCI_HDR_LEN);

        /* Send payload */
        if (bufLen > UCI_HDR_LEN) {
            bus_status = uwb_bus_data_tx(&pCtx->busCtx, &pBuf[UCI_HDR_LEN], (size_t)(bufLen - UCI_HDR_LEN));
            if (bus_status == kUWB_bus_Status_FAILED) {
                LOG_E("uwb_uwbs_tml_data_tx writing UCI command failed");
                pCtx->noOfBytesWritten = -1;
                goto end;
            }
            else {
                pCtx->noOfBytesWritten = bufLen;
            }
        }
    }
    else {
        LOG_E("uwb_uwbs_tml_data_tx : tml mode not supported");
        goto end;
    }
    status = kUWBSTATUS_SUCCESS;
end:
    phOsalUwb_UnlockMutex(pCtx->mSyncMutex);
    return status;
}

UWBStatus_t uwb_uwbs_tml_data_rx(uwb_uwbs_tml_ctx_t *pCtx, uint8_t *pBuf, size_t *pBufLen)
{
    uwb_bus_status_t bus_status;
    UWBStatus_t status = kUWBSTATUS_FAILED;
    size_t payloadLen;
    size_t totalBtyesToRead   = 0;
    uint8_t count             = 0;
    uwbs_io_state_t gpioValue = kUWBS_IO_State_NA;

    if (pCtx == NULL) {
        LOG_E("uwb_uwbs_tml_data_rx : uwbs tml context is NULL");
        return kUWBSTATUS_INVALID_PARAMETER;
    }

    if (pBuf == NULL || pBufLen == NULL) {
        LOG_E("uwb_uwbs_tml_data_rx : read buffer is Null");
        return kUWBSTATUS_INVALID_PARAMETER;
    }

start:
    bus_status = uwb_bus_io_irq_wait(&pCtx->busCtx, READ_IRQ_WAIT_TIME);

    if (bus_status == kUWB_bus_Status_FAILED) {
        status   = kUWBSTATUS_RESPONSE_TIMEOUT;
        *pBufLen = 0;
        goto exit;
    }
    if (pCtx->mode == kUWB_UWBS_TML_MODE_HBCI) {
        if (*pBufLen >= HBCI_HEADER_SIZE) {
            bus_status = uwb_bus_io_val_get(&pCtx->busCtx, kUWBS_IO_I_UWBS_IRQ, &gpioValue);
            if (bus_status != kUWB_bus_Status_OK) {
                LOG_E("uwb_uwbs_tml_data_rx : get heios irq status failed");
                *pBufLen = 0;
                goto end;
            }
            if (!(gpioValue)) {
                goto start;
            }
            bus_status = uwb_bus_data_rx(&pCtx->busCtx, pBuf, HBCI_HEADER_SIZE);
            if (bus_status != kUWB_bus_Status_OK) {
                LOG_E("uwb_uwbs_tml_data_rx : reading from helios failed");
                *pBufLen = 0;
                goto exit;
            }
            payloadLen = (uint16_t)((pBuf[PAYLOAD_LEN_MSB] << 8) | pBuf[PAYLOAD_LEN_LSB]);
            if (payloadLen != 0) {
                if (*pBufLen >= (HBCI_HEADER_SIZE + payloadLen)) {
                    bus_status = uwb_bus_data_rx(&pCtx->busCtx, &pBuf[HBCI_HEADER_SIZE], payloadLen);
                    if (bus_status != kUWB_bus_Status_OK) {
                        LOG_E("uwb_uwbs_tml_data_rx : reading from helios failed");
                        *pBufLen = 0;
                        goto exit;
                    }
                }
                else {
                    LOG_E("uwb_uwbs_tml_data_trx not enough receive buffer");
                    *pBufLen = 0;
                    goto exit;
                }
            }
            *pBufLen = (HBCI_HEADER_SIZE + payloadLen);
        }
        else {
            LOG_E("uwb_uwbs_tml_data_trx not enough receive buffer");
            *pBufLen = 0;
            goto exit;
        }
        status = kUWBSTATUS_SUCCESS;
    }
    else if (pCtx->mode == kUWB_UWBS_TML_MODE_UCI) {
        if (*pBufLen < UCI_RX_HDR_LEN) {
            LOG_E("%s Not enough RX buffer", __FUNCTION__);
            *pBufLen = 0;
            goto exit;
        }

        phOsalUwb_LockMutex(pCtx->mSyncMutex);
        uwb_port_DelayinMicroSec(1);

        bus_status = uwb_bus_io_val_get(&pCtx->busCtx, kUWBS_IO_I_UWBS_IRQ, &gpioValue);
        if (bus_status != kUWB_bus_Status_OK) {
            LOG_E("uwb_uwbs_tml_data_rx : get heios irq status failed");
            *pBufLen = 0;
            goto end;
        }
        if (!(gpioValue)) {
            phOsalUwb_UnlockMutex(pCtx->mSyncMutex);
            goto start;
        }

        /*Read Ready Indication from HOST to Helios*/
        bus_status = uwb_bus_io_val_set(&pCtx->busCtx, kUWBS_IO_O_HELIOS_SYNC, kUWBS_IO_State_High);
        if (bus_status == kUWB_bus_Status_FAILED) {
            LOG_E("uwb_uwbs_tml_data_rx : setting HELIOS irq SYNC failed");
            *pBufLen = 0;
            goto end;
        }

        /* IRQ asserted again */
        bus_status = uwb_bus_io_irq_wait(&pCtx->busCtx, READ_IRQ_WAIT_TIME);
        if (bus_status == kUWB_bus_Status_FAILED) {
            *pBufLen = 0;
            goto end;
        }

        bus_status = uwb_bus_data_rx(&pCtx->busCtx, pBuf, UCI_RX_HDR_LEN);
        if (bus_status != kUWB_bus_Status_OK) {
            LOG_E("uwb_uwbs_tml_data_rx : reading from helios failed");
            *pBufLen = 0;
            goto end;
        }

        /* For data packet, we get 2 bytes of data always so extended len handling is not required */
        totalBtyesToRead = pBuf[NORMAL_MODE_LEN_OFFSET];
        if ((pBuf[0] == 0x02) /* Data packet*/ ||
            (pBuf[EXTND_LEN_INDICATOR_OFFSET] & EXTND_LEN_INDICATOR_OFFSET_MASK) /* Extended length */
        ) {
            totalBtyesToRead = (uint16_t)((totalBtyesToRead << 8) | pBuf[EXTENDED_LENGTH_OFFSET]);
        }

        payloadLen = totalBtyesToRead;

        if (payloadLen != 0) {
            if (*pBufLen >= (payloadLen + UCI_RX_HDR_LEN)) {
                bus_status = uwb_bus_data_rx(&pCtx->busCtx, &pBuf[UCI_RX_HDR_LEN], payloadLen);
                if (bus_status != kUWB_bus_Status_OK) {
                    LOG_E("uwb_uwbs_tml_data_rx : reading from helios failed");
                    *pBufLen = 0;
                    goto end;
                }
            }
            else {
                LOG_E("%s Not enough RX buffer", __FUNCTION__);
                *pBufLen = 0;
                goto end;
            }
        }

        *pBufLen = (UCI_RX_HDR_LEN + payloadLen);
        count    = 0;
        uwb_bus_io_val_get(&pCtx->busCtx, kUWBS_IO_I_UWBS_IRQ, &gpioValue);
        while (gpioValue) {
            uwb_bus_io_val_get(&pCtx->busCtx, kUWBS_IO_I_UWBS_IRQ, &gpioValue);
            if (count >= 20) {
                *pBufLen = 0;
                goto end;
            }
            // Sleep of 500us * 20 = 10ms as per artf786394
            uwb_port_DelayinMicroSec(500);
            count++;
        }
        status = kUWBSTATUS_SUCCESS;
    end:
        if ((uwb_bus_io_val_set(&pCtx->busCtx, kUWBS_IO_O_HELIOS_SYNC, kUWBS_IO_State_Low)) == kUWB_bus_Status_FAILED) {
            LOG_E("uwb_uwbs_tml_data_rx : uwb_bus_io_val_set failed");
            *pBufLen = 0;
            status   = kUWBSTATUS_FAILED;
        }
        phOsalUwb_UnlockMutex(pCtx->mSyncMutex);
    }
    else {
        LOG_E("uwb_uwbs_tml_data_tx : tml mode not supported");
    }
exit:
    return status;
}

UWBStatus_t uwb_uwbs_tml_data_trx(
    uwb_uwbs_tml_ctx_t *pCtx, uint8_t *pTxBuf, size_t txBufLen, uint8_t *pRxBuf, size_t *pRxBufLen)
{
    UWBStatus_t status = kUWBSTATUS_FAILED;

    if (pCtx->mode == kUWB_UWBS_TML_MODE_HBCI) {
        status = uwb_uwbs_tml_data_tx(pCtx, pTxBuf, txBufLen);
        if (status != kUWBSTATUS_SUCCESS) {
            LOG_E("uwb_uwbs_tml_hbci_data_trx write data failed");
            goto end;
        }
        //LOG_MAU8_I("HBCI Tx >", pTxBuf, txBufLen);
        status = uwb_uwbs_tml_data_rx(pCtx, pRxBuf, pRxBufLen);
        if (status != kUWBSTATUS_SUCCESS) {
            LOG_E("uwb_uwbs_tml_hbci_data_trx read data failed");
        }
        //LOG_MAU8_I("HBCI Rx >", pRxBuf, *pRxBufLen);
    end:
        uwb_port_DelayinMicroSec(50);
    }
    else {
        /* not needed for UCI Mode */
    }
    return status;
}

UWBStatus_t uwb_uwbs_tml_data_trx_with_Len(
    uwb_uwbs_tml_ctx_t *pCtx, uint8_t *pTxBuf, size_t txBufLen, uint8_t *pRxBuf, size_t rxBufLen)
{
    UWBStatus_t status = kUWBSTATUS_FAILED;
    uwb_bus_status_t bus_status;

    if (pRxBuf == NULL || rxBufLen == 0) {
        LOG_E("Response buffer is NULL or bytes to read is zero");
        return status;
    }

    if (pCtx->mode == kUWB_UWBS_TML_MODE_HBCI) {
        status = uwb_uwbs_tml_data_tx(pCtx, pTxBuf, txBufLen);
        if (status != kUWBSTATUS_SUCCESS) {
            LOG_E("uwb_uwbs_tml_hbci_data_trx write data failed");
            goto end;
        }

    start:
        status     = kUWBSTATUS_FAILED;
        bus_status = uwb_bus_io_irq_wait(&pCtx->busCtx, READ_IRQ_WAIT_TIME);
        if (bus_status == kUWB_bus_Status_FAILED) {
            goto start;
        }
        bus_status = uwb_bus_data_rx(&pCtx->busCtx, pRxBuf, rxBufLen);
        if (bus_status != kUWB_bus_Status_OK) {
            LOG_E("uwb_uwbs_tml_data_rx : reading from helios failed");
            goto end;
        }
        //LOG_MAU8_I("HBCI Rx >", pRxBuf , *pBufLen);
        status = kUWBSTATUS_SUCCESS;
    end:
        uwb_port_DelayinMicroSec(50);
    }
    else {
        /* not needed for UCI Mode */
    }
    return status;
}

UWBStatus_t uwb_uwbs_tml_reset(uwb_uwbs_tml_ctx_t *pCtx)
{
    if (kUWB_bus_Status_OK != uwb_bus_reset(&pCtx->busCtx)) {
        return kUWBSTATUS_FAILED;
    }
    return kUWBSTATUS_SUCCESS;
}

void uwb_uwbs_tml_flush_read_buffer(uwb_uwbs_tml_ctx_t *pCtx)
{
    /* Not needed here*/
    return;
}

UWBStatus_t uwb_uwbs_tml_helios_hardreset(uwb_uwbs_tml_ctx_t *pCtx)
{
    /* Dummy function. Not needed here. */
    return kUWBSTATUS_SUCCESS;
}

#endif // UWBIOT_UWBD_SR1XXT
