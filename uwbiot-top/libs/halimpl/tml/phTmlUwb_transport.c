/*
 * Copyright 2012-2020,2022-2023 NXP.
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
#include <stdlib.h>

#include "phNxpUciHal.h"
#include "phNxpUciHal_utils.h"

#include "phTmlUwb_transport.h"
#include "phUwbStatus.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phUwb_BuildConfig.h"
#include <uwb_uwbs_tml_interface.h>
#include "uwb_uwbs_tml_io.h"

/*********************** Global Variables *************************************/
/* UCI HAL Control structure */
static int gInitDone = 0xFF;
/** UWB Subsystem tml interface context */
uwb_uwbs_tml_ctx_t gUwbsTmlCtx;
static void helios_irq_cb(void *args)
{
#if UWBIOT_UWBD_SR040
    // Disable interrupt
    uwb_uwbs_tml_ctx_t *pCtx = (uwb_uwbs_tml_ctx_t *)args;
    uwb_bus_io_irq_dis(&pCtx->busCtx, kUWBS_IO_I_UWBS_IRQ);
#else
    // Nothing to do
#endif
}

/*******************************************************************************
**
** Function         phTmlUwb_open_and_configure
**
** Description      Open and configure helios device
**
** Parameters       pConfig     - hardware information
**                  pLinkHandle - device handle
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - open_and_configure operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_open_and_configure(pphTmlUwb_Config_t pConfig, void **pLinkHandle)
{
    UWBSTATUS retStatus = UWBSTATUS_INVALID_DEVICE;

    if (uwb_uwbs_tml_init(&gUwbsTmlCtx) == kUWBSTATUS_SUCCESS) {
        gInitDone = 0;
        retStatus = UWBSTATUS_SUCCESS;
    }
    if (pLinkHandle != NULL) {
        /*
         *  Bus io init should be done only once.
         *  Null check added because for pnp and mctt and SR040(SWUP) we are calling this api with NUll.
         *  once we call from the params NULL we dont want to re initialize the IO configs.
         */
        if (uwb_bus_io_init(&gUwbsTmlCtx.busCtx) == kUWB_bus_Status_OK) {
            retStatus = UWBSTATUS_SUCCESS;
        }
        *pLinkHandle = (void *)&gInitDone;
    }
    return retStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_io_init
**
** Description      initialise uwbs io
**
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_io_init()
{
    UWBSTATUS retStatus = UWBSTATUS_INVALID_DEVICE;
    if (uwb_bus_io_init(&gUwbsTmlCtx.busCtx) == kUWB_bus_Status_OK) {
        retStatus = UWBSTATUS_SUCCESS;
    }
    return retStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_io_deinit
**
** Description      De initialise uwbs io
**
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_io_deinit()
{
    UWBSTATUS retStatus = UWBSTATUS_INVALID_DEVICE;
    if (uwb_bus_io_deinit(&gUwbsTmlCtx.busCtx) == kUWB_bus_Status_OK) {
        retStatus = UWBSTATUS_SUCCESS;
    }
    return retStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_io_set
**
** Description      set uwbs io
**
** Parameters       ioPim      - pin to set
**                  value         - pin value
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_io_set(uwbs_io_t ioPin, bool_t value)
{
    if ((uwb_bus_io_val_set(&gUwbsTmlCtx.busCtx, ioPin, (uwbs_io_state_t)value)) == kUWB_bus_Status_OK) {
        return UWBSTATUS_SUCCESS;
    }
    return UWBSTATUS_INVALID_DEVICE;
}

/*******************************************************************************
**
** Function         phTmlUwb_io_enable_irq
**
** Description      enable uwbs irq
**
** Parameters       irqPin     - irq to enable
**                  cb         - callback
**                  args       - callback args
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
void phTmlUwb_io_enable_irq(uwbs_io_t irqPin, void (*cb)(void *args), void *args)
{
    uwbs_io_callback callback;
    callback.fn   = cb;
    callback.args = args;
    if (kUWB_bus_Status_OK == (uwb_bus_io_irq_en(&gUwbsTmlCtx.busCtx, irqPin, &callback))) {
        LOG_D("phTmlUwb_io_enable_irq : enabled irq successfully");
    }
    else {
        LOG_E("phTmlUwb_io_enable_irq : failed to enable irq");
    }
}

//TODO: Temporary for sn110 and sr1xx simultaneous irq enblement
#if UWBFTR_SE_SN110
void phTmlUwb_io_enable_uwb_irq()
{
    uwb_bus_io_uwbs_irq_enable(&gUwbsTmlCtx.busCtx);
}
#endif
/*******************************************************************************
**
** Function         phTmlUwb_io_disable_irq
**
** Description      disable uwbs irq
**
** Parameters       irqPin     - irq to disable
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
void phTmlUwb_io_disable_irq(uwbs_io_t irqPin)
{
    if (kUWB_bus_Status_OK == (uwb_bus_io_irq_dis(&gUwbsTmlCtx.busCtx, irqPin))) {
        LOG_D("phTmlUwb_io_disable_irq : disabled irq successfully");
    }
    else {
        LOG_E("phTmlUwb_io_disable_irq : failed to disable irq");
    }
}

/*******************************************************************************
**
** Function         phTmlUwb_close
**
** Description      SPI Cleanup
**
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_close()
{
    if (kUWBSTATUS_SUCCESS != uwb_uwbs_tml_deinit(&gUwbsTmlCtx)) {
        LOG_E("phTmlUwb_close : uwb_uwbs_tml_deinit failed");
    }
    gInitDone = 0xFF;
}

/*******************************************************************************
**
** Function         phTmlUwb_uci_read
**
** Description      Reads requested number of bytes from SR100 device into given
**                  buffer
**
** Parameters       pBuffer          - buffer for read data
**                  nNbBytesToRead   - number of bytes requested to be read
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*******************************************************************************/
int phTmlUwb_uci_read(uint8_t *pBuffer, int nNbBytesToRead)
{
    int bufLen = nNbBytesToRead;
    UWBStatus_t status = uwb_uwbs_tml_data_rx(&gUwbsTmlCtx, pBuffer, (size_t *)&bufLen);
    if (status != kUWBSTATUS_SUCCESS) {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        if (status == kUWBSTATUS_RESPONSE_TIMEOUT) {
            LOG_D("phTmlUwb_uci_read : Read IRQ Timedout");
            bufLen = UWBSTATUS_IRQ_READ_TIMEOUT;
        }
        else
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
        {
            LOG_D("phTmlUwb_uci_read : uwb_uwbs_tml_data_rx failed");
            bufLen = 0;
        }
    }
    return bufLen;
}

#if UWBIOT_UWBD_SR040
/*******************************************************************************
**
** Function         phTmlUwb_rci_read
**
** Description      Reads requested number of bytes from SR040 device into given
**                  buffer using SWUP protocol
**
** Parameters       pBuffer          - buffer for read data
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*******************************************************************************/
int phTmlUwb_rci_read(uint8_t *pBuffer, int nNbBytesToRead)
{
    size_t bufLen = (size_t)nNbBytesToRead;
    uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_SWUP);
    if (uwb_uwbs_tml_data_rx(&gUwbsTmlCtx, pBuffer, &bufLen) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_rci_read : uwb_uwbs_tml_data_rx failed");
    }
    return (int)bufLen;
}
#endif //UWBIOT_UWBD_SR040

/*******************************************************************************
**
** Function         phTmlUwb_uci_write
**
** Description      Writes requested number of bytes from given buffer into
**                  SR100 device
**
** Parameters       pBuffer          - buffer for read data
**                  nNbBytesToWrite  - number of bytes requested to be written
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*******************************************************************************/
int phTmlUwb_uci_write(uint8_t *pBuffer, uint16_t nNbBytesToWrite)
{
    int numWrote                  = 0;
    const UWBStatus_t writeStatus = uwb_uwbs_tml_data_tx(&gUwbsTmlCtx, pBuffer, nNbBytesToWrite);

    if (writeStatus == kUWBSTATUS_SUCCESS) {
        numWrote = gUwbsTmlCtx.noOfBytesWritten;
    }
    else if (writeStatus == kUWBSTATUS_BUSY) {
        LOG_D("phTmlUwb_uci_write : uwb_uwbs_tml_data_tx failed");
        numWrote = -2;
    }
    else {
        LOG_D("phTmlUwb_uci_write : uwb_uwbs_tml_data_tx failed");
        numWrote = -1;
    }

    return numWrote;
}

#if UWBIOT_UWBD_SR040
/*******************************************************************************
**
** Function         phTmlUwb_rci_write
**
** Description      Writes requested number of bytes from given buffer into
**                  SR040 device in SWUP mode
**
** Parameters       pBuffer          - buffer to write
**                  nNbBytesToWrite  - number of to write
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*******************************************************************************/
int phTmlUwb_rci_write(uint8_t *pBuffer, uint16_t nNbBytesToWrite)
{
    int numWrote = 0;
    uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_SWUP);
    if (uwb_uwbs_tml_data_tx(&gUwbsTmlCtx, pBuffer, nNbBytesToWrite) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_rci_write : uwb_uwbs_tml_data_tx failed");
        return -1;
    }
    numWrote = gUwbsTmlCtx.noOfBytesWritten;

    if (numWrote <= 0) {
        return -1;
    }
    else {
        return 0;
    }
}
#endif //UWBIOT_UWBD_SR040

#if UWBIOT_UWBD_SR1XXT
/*******************************************************************************
**
** Function         phTmlUwb_hbci_transcive
**
** Description      HBCI Write read operation for SR1xxT devices for FW download
**
** Parameters       pWriteBuf        - buffer to write
**                  writeBufLen  - number of bytes to written
**                  pRespBuf     - response buffer
**                  pRspBufLen   - response bufferLen
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_hbci_transcive operation success
**                  UWBSTATUS_FAILED - phTmlUwb_hbci_transcive failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_hbci_transcive(uint8_t *pWriteBuf, size_t writeBufLen, uint8_t *pRespBuf, size_t *pRspBufLen)
{
    UWBSTATUS status = UWBSTATUS_FAILED;
    if ((uwb_uwbs_tml_data_trx(&gUwbsTmlCtx, pWriteBuf, writeBufLen, pRespBuf, pRspBufLen)) == kUWBSTATUS_SUCCESS) {
        status = UWBSTATUS_SUCCESS;
    }
    return status;
}

/*******************************************************************************
**
** Function         phTmlUwb_hbci_transcive_with_len
**
** Description      HBCI Write read operation for SR1XXT with known receive length.
**
** Parameters       pDevHandle       - valid device handle
**                  pWriteBuf        - buffer to write
**                  writeBufLen  - number of bytes to written
**                  pRespBuf     - response buffer
**                  rspBufLen   - response bufferLen
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_hbci_transcive_with_len operation success
**                  UWBSTATUS_FAILED - phTmlUwb_hbci_transcive_with_len failure
**
*******************************************************************************/
#if UWBIOT_TML_SPI
UWBSTATUS phTmlUwb_hbci_transcive_with_len(uint8_t *pWriteBuf, size_t writeBufLen, uint8_t *pRespBuf, size_t rspBufLen)
{
    UWBSTATUS status = UWBSTATUS_FAILED;
    if ((uwb_uwbs_tml_data_trx_with_Len(&gUwbsTmlCtx, pWriteBuf, writeBufLen, pRespBuf, rspBufLen)) ==
        kUWBSTATUS_SUCCESS) {
        status = UWBSTATUS_SUCCESS;
    }
    return status;
}
#endif
#endif //UWBIOT_UWBD_SR1XXT

#if UWBIOT_UWBD_SR2XXT
/*******************************************************************************
**
** Function         phTmlUwb_hdll_read
**
** Description      Reads requested number of bytes from SR040 device into given
**                  buffer using SWUP protocol
**
** Parameters       pBuffer          - buffer for read data
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*******************************************************************************/
int phTmlUwb_hdll_read(uint8_t *pBuffer, uint16_t *pRspBufLen)
{
    size_t bufLen = 0;
    uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_HDLL);
#if UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
    if (uwb_uwbs_tml_helios_get_hdll_edl_ntf(&gUwbsTmlCtx, pBuffer, &bufLen) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_hdll_read : uwb_uwbs_tml_data_rx failed");
        return -1;
    }
#else
    if (uwb_uwbs_tml_data_rx(&gUwbsTmlCtx, pBuffer, &bufLen) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_hdll_read : uwb_uwbs_tml_data_rx failed");
        return -1;
    }
#endif
    *pRspBufLen = (uint16_t)bufLen;
    return 0;
}
/*******************************************************************************
**
** Function         phTmlUwb_hdll_write
**
** Description      Writes requested number of bytes from given buffer into
**                  SR200 device in HDLL mode
**
** Parameters       pBuffer          - buffer to write
**                  nNbBytesToWrite  - number of to write
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*******************************************************************************/
int phTmlUwb_hdll_write(uint8_t *pBuffer, uint16_t nNbBytesToWrite)
{
    int numWrote = 0;
    uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_HDLL);
    if (uwb_uwbs_tml_data_tx(&gUwbsTmlCtx, pBuffer, nNbBytesToWrite) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_hdll_write : uwb_uwbs_tml_data_tx failed");
    }
    numWrote = gUwbsTmlCtx.noOfBytesWritten;

    if (numWrote <= 0) {
        return -1;
    }
    else {
        return 0;
    }
}

/*******************************************************************************
**
** Function         phTmlUwb_hdll_transcive
**
** Description      HDLL Write read operation for SR2XXT devices for FW download
**
** Parameters       pWriteBuf        - buffer to write
**                  writeBufLen  - number of bytes to written
**                  pRespBuf     - response buffer
**                  pRspBufLen   - response bufferLen
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_hdll_transcive operation success
**                  UWBSTATUS_FAILED - phTmlUwb_hdll_transcive failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_hdll_transcive(uint8_t *pWriteBuf, size_t writeBufLen, uint8_t *pRespBuf, size_t *pRspBufLen)
{
    UWBSTATUS status = UWBSTATUS_FAILED;
    uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_HDLL);
    if ((uwb_uwbs_tml_data_trx(&gUwbsTmlCtx, pWriteBuf, writeBufLen, pRespBuf, pRspBufLen)) == kUWBSTATUS_SUCCESS) {
        status = UWBSTATUS_SUCCESS;
    }
    return status;
}

/*******************************************************************************
**
** Function         phTmlUwb_hdll_reset
**
** Description      Send HDLL Reset Command for SR2XXT devices. Resets device back to UCI mode
**
** Parameters       NULL
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_hdll_transcive operation success
**                  UWBSTATUS_FAILED - phTmlUwb_hdll_transcive failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_hdll_reset()
{
    UWBSTATUS status = UWBSTATUS_FAILED;
    uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_HDLL);
    if (uwb_uwbs_tml_helios_reset(&gUwbsTmlCtx) == kUWBSTATUS_SUCCESS) {
        status = UWBSTATUS_SUCCESS;
    }
    return status;
}
#endif //UWBIOT_UWBD_SR2XXT

#if UWBIOT_UWBD_SR1XXT_SR2XXT
/*******************************************************************************
**
** Function         phTmlUwb_hdll_hardreset
**
** Description      Send Reset Command for SR1XXT/SR2XXT devices. Used when device
**                  needs to be kept in HDLL mode after reset for SR2XXT devices or
**                  Reset device for SR1XXT devices.
**
** Parameters       NULL
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_hdll_transcive operation success
**                  UWBSTATUS_FAILED - phTmlUwb_hdll_transcive failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_hdll_hardreset()
{
    UWBSTATUS status = UWBSTATUS_FAILED;
    if (uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_HDLL) == kUWBSTATUS_SUCCESS) {
        if (uwb_uwbs_tml_helios_hardreset(&gUwbsTmlCtx) == kUWBSTATUS_SUCCESS) {
            status = UWBSTATUS_SUCCESS;
        }
        else {
            LOG_E("%s : uwb_uwbs_tml_helios_hardreset failed", __FUNCTION__);
        }
    }
    return status;
}
#if (UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
/*******************************************************************************
**
** Function         phTmlUwb_pnp_hardreset
**
** Description      Send Reset Command for PNP HOST. Used when
**                  Reset the device for SR1XXT devices.
**
** Parameters       NULL
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - Hard Rest operation success
**                  UWBSTATUS_FAILED -  failure on other cases
**
*******************************************************************************/
UWBSTATUS phTmlUwb_pnp_hardreset()
{
    UWBSTATUS status = UWBSTATUS_FAILED;
    phTmlUwb_set_hbci_mode();
    if (uwb_uwbs_tml_helios_hardreset(&gUwbsTmlCtx) == kUWBSTATUS_SUCCESS) {
        status = UWBSTATUS_SUCCESS;
    }
    return status;
}
#endif // (UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT

/*******************************************************************************
**
** Function         phTmlUwb_reset
**
** Description      Reset SR100 device, using VEN pin
**
** Parameters       level          - reset level
**
** Returns           0   - reset operation success
**                  -1   - reset operation failure
**
*******************************************************************************/
int phTmlUwb_reset(long level)
{
    NXPLOG_UWB_TML_D("phTmlUwb_reset(), VEN level %ld", level);
    phTmlUwb_ReadAbort();
#if UWBIOT_OS_NATIVE
    /** Read abort for the Kernel mode */
    if (uwb_bus_io_uwbs_irq_disable(&gUwbsTmlCtx.busCtx) != kUWB_bus_Status_OK) {
        LOG_E("uwb_bus_io_uwbs_irq_disable failed");
    }
#endif // UWBIOT_OS_NATIVE
    return 1;
}

/*******************************************************************************
**
** Function         phTmlUwb_set_uci_mode
**
** Description      Set TML transfer mode to UCI
**
*******************************************************************************/
void phTmlUwb_set_uci_mode()
{
    if (uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_UCI) != kUWBSTATUS_SUCCESS) {
        LOG_E("phTmlUwb_set_uci_mode : uwb_uwbs_tml_setmode failed");
    }
}

/*******************************************************************************
**
** Function         phTmlUwb_set_hbci_mode
**
** Description      Set TML transfer mode to HBCI
**
*******************************************************************************/
void phTmlUwb_set_hbci_mode()
{
    if (uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_HBCI) != kUWBSTATUS_SUCCESS) {
        LOG_E("phTmlUwb_set_uci_mode : uwb_uwbs_tml_setmode failed");
    }
}

#if UWBIOT_UWBD_SR040
/*******************************************************************************
**
** Function         phTmlUwb_flush_read_buffer
**
** Description      flush tml read buffer
**
**
**
*******************************************************************************/
void phTmlUwb_flush_read_buffer()
{
    uwb_uwbs_tml_flush_read_buffer(&gUwbsTmlCtx);
}

/*******************************************************************************
**
** Function         phTmlUwb_reset_uwbs
**
** Description      Reset UWBS device
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_reset_uwbs operation success
**                  UWBSTATUS_FAILED - phTmlUwb_reset_uwbs failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_reset_uwbs()
{
    UWBSTATUS status = UWBSTATUS_SUCCESS;
    if (uwb_uwbs_tml_reset(&gUwbsTmlCtx) != kUWBSTATUS_SUCCESS) {
        LOG_E("phTmlUwb_reset_uwbs: uwb_uwbs_tml_reset failed");
        status = UWBSTATUS_FAILED;
    }
    return status;
}
#endif //UWBIOT_UWBD_SR040

/*******************************************************************************
**
** Function         phTmlUwb_helios_irq_enable
**
** Description      enable uwbs irq
**
**
**
*******************************************************************************/
void phTmlUwb_helios_irq_enable()
{
    uwbs_io_callback callback;
    callback.fn   = &helios_irq_cb;
    callback.args = (void *)&gUwbsTmlCtx;
    if (uwb_bus_io_irq_en(&gUwbsTmlCtx.busCtx, kUWBS_IO_I_UWBS_IRQ, &callback) != kUWB_bus_Status_OK) {
        LOG_E("phTmlUwb_helios_irq_enable : uwb_bus_io_irq_en failed");
    }
}

/*******************************************************************************
**
** Function         phTmlUwb_helios_interupt_status
**
** Description      get uwbs irq pin status
**
**
**
*******************************************************************************/
bool phTmlUwb_helios_interupt_status()
{
    uwbs_io_state_t gpioValue = kUWBS_IO_State_NA;
    if (uwb_bus_io_val_get(&gUwbsTmlCtx.busCtx, kUWBS_IO_I_UWBS_IRQ, &gpioValue) != kUWB_bus_Status_OK) {
        LOG_E("phTmlUwb_helios_interupt_status : uwb_bus_io_val_get failed");
    }
    return (bool)gpioValue;
}

#if UWBIOT_UWBD_SR040
/*******************************************************************************
**
** Function         phTmlUwb_rdy_read
**
** Description      get uwbs ready pin status
**
**
**
*******************************************************************************/
bool phTmlUwb_rdy_read()
{
    uwbs_io_state_t gpioValue = kUWBS_IO_State_NA;
    if (uwb_bus_io_val_get(&gUwbsTmlCtx.busCtx, kUWBS_IO_I_UWBS_READY_IRQ, &gpioValue) != kUWB_bus_Status_OK) {
        LOG_E("phTmlUwb_rdy_read : uwb_bus_io_val_get failed");
    }
    return (bool)gpioValue;
}
#endif

#if UWBIOT_TML_S32UART || UWBIOT_UWBD_SR2XXT
/*******************************************************************************
**
** Function         phTmlUwb_switch_protocol
**
** Description      switch uwbs protocl
**
** Parameters       protocol       - protocol SWUP or UCI
**
*******************************************************************************/

void phTmlUwb_switch_protocol(uint8_t protocol)
{
#if UWBIOT_UWBD_SR2XXT
    uwb_uwbs_tml_setmode(&gUwbsTmlCtx, (uwb_uwbs_tml_mode_t)protocol);
#elif UWBIOT_TML_S32UART
    uwb_uwbs_tml_switch_protocol(&gUwbsTmlCtx, (uwb_uwbs_tml_mode_t)protocol);
#endif
}
#endif //UWBIOT_TML_S32UART || UWBIOT_UWBD_SR2XXT
