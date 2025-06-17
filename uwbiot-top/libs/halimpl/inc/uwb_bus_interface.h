/*
 * Copyright 2021-2023 NXP.
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

#ifndef __UWB_BUS_INTERFACE_H__
#define __UWB_BUS_INTERFACE_H__

#include <stdio.h>
#include <stddef.h>
#include <uwb_bus_board.h>
#include <uwb_uwbs_tml_io.h>
#include "phUwbTypes.h"
#include <phOsalUwb.h>

/** @defgroup uwb_bus UWB BUS Interface
 *
 * This layer depends on ``uwb_bus_board.h``, which is board specific.
 *
 * ``uwb_bus_board.h`` must implement / define the following
 *
 * - structure ``uwb_bus_board_ctx_t``
 *
 * The implementation of all these APIs would be board specific and
 * tightly coupled to the board.
 *
 * @{
 */

/** Max Time to wait for SPI data transfer completion */
#define MAX_UWBS_SPI_TRANSFER_TIMEOUT (1000)

/** @defgroup uwb_bus_status UWB BUS Interface Status Codes
 *
 * @{
 */

/**
 * @brief Status code for HOST
 *
 * We want to, simplify the porting and hence using simpler
 * status codes here without mapping to full UWB LIB.
 *
 */
typedef enum
{
    /** Everything went well */
    kUWB_bus_Status_OK = 0,

    /** There was a failure */
    kUWB_bus_Status_FAILED = 1,
    /** There was a ADDR NAK */
    kUWB_bus_Status_NAK = 1,
} uwb_bus_status_t;

/** @} */

/**
 * @brief
 * Initialize the nfc wrapper functions
 * which are used by wrbl-nfc-sdk
 */

/**
 * @brief
 * Wrapper function for nfc sdk
 * Calls uwb_bus_io_val_set using board ctx
 * @param gpioValue
 */
void nfc_ven_val_set(uint8_t gpioValue);

/**
 * @brief
 * Wrapper function for nfc sdk
 * Calls uwb_bus_io_val_get using board ctx
 * @param gpioValue
 * @return uint32_t
 */
uint32_t nfc_ven_val_get(uint8_t gpioValue);

/**
 * @brief
 * Wrapper function for nfc sdk
 * Calls uwb_bus_io_val_get using board ctx
 * @return uint32_t
 */
uint32_t nfc_irq_val_get();

/** @defgroup uwb_bus_ctx UWB BUS Context Management
 *
 * @{
 */

/**
 * @brief Initailize bus interface
 *
 * When this API is called
 *
 * - All requried semaphores and mutexes are created
 * - bus interface (SPI) is initialised.
 *
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_init(uwb_bus_board_ctx_t *pCtx);

/**
 * @brief De-Iniailize the bus interface and free up references and context
 *
 * When this API is called
 *
 * - All requried semaphores and mutexes are destryoed
 * - bus interface (SPI) is de-Iniailized.
 *
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_deinit(uwb_bus_board_ctx_t *pCtx);

/**
 * @brief Reset UWB BUS
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_reset(uwb_bus_board_ctx_t *pCtx);

/**
 * @brief Add Delay in micro second
 *
 * @param[in]      delay    Delay value in micro seconds
 *
 * @return     None
 */
void uwb_port_DelayinMicroSec(int delay);

/** @} */

/** @defgroup uwb_bus_io UWB BUS IO Management
 *
 * See @ref uwb_uwbs_io
 *
 * @{
 */

/**
 * @brief      Initialise GPIO's
 *
 * When this API is called
 *
 * - All IO Pins are set to their respective states
 *
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_io_init(uwb_bus_board_ctx_t *pCtx);

/**
 * @brief      De Initialise GPIO's
 *
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_io_deinit(uwb_bus_board_ctx_t *pCtx);

/**
 * @brief      Get value of a GPIO
 *
 * @param      pCtx  The context
 * @param      gpioPin  pin
 * @param      pGpioValue  value
 */
uwb_bus_status_t uwb_bus_io_val_get(uwb_bus_board_ctx_t *pCtx, uwbs_io_t gpioPin, uwbs_io_state_t *pGpioValue);

/**
 * @brief  Set value of a GPIO
 *
 * @param      pCtx  The context
 * @param      gpioPin  pin
 * @param      gpioValue  value
 */
uwb_bus_status_t uwb_bus_io_val_set(uwb_bus_board_ctx_t *pCtx, uwbs_io_t gpioPin, uwbs_io_state_t gpioValue);

/**
 * @brief  Get value of a NFC Ven pin
 *
 * @param      pCtx  The context
 * @param      gpioPin  pin
 * @param      pGpioValue  value
 */
uwb_bus_status_t uwb_bus_io_val_get(uwb_bus_board_ctx_t *pCtx, uwbs_io_t gpioPin, uwbs_io_state_t *pGpioValue);

/**
 * @brief Call back called when IRQ is triggered
 *
 * IRQ Mapping APIs are "Vendor/Platform SDK" specific.  And the SDK specific IRQ Callback implementation
 * must call this API with right context.
 *
 * @param      pCtx  The context
 */
void uwb_bus_io_irq_cb(void *pCtx);

/**
 * @brief      Enable the UWBS IRQ with board specific callback function
 *
 * @param      pCtx  The context
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_io_uwbs_irq_enable(uwb_bus_board_ctx_t *pCtx);

/**
 * @brief      Disable the UWBS IRQ with board specific callback function
 *
 * @param      pCtx  The context
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_io_uwbs_irq_disable(uwb_bus_board_ctx_t *pCtx);

/**
 * @brief      Wit for UWBS IRQ
 *
 * This is used while reading the data from UWBS
 *
 * @param      pCtx        The context
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @retval kUWB_bus_Status_OK IRQ was triggered before the timeout
 * @retval kUWB_bus_Status_FAILED IRQ was not triggered, and we timed out.
 *
 */
uwb_bus_status_t uwb_bus_io_irq_wait(uwb_bus_board_ctx_t *pCtx, uint32_t timeout_ms);

/**
 * @brief      Enable the Host IRQ
 *
 * @param      pCtx  The context
 * @param      irqPin  IRQ
 * @param      pCallback    pointer to interrupt handler
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_io_irq_en(uwb_bus_board_ctx_t *pCtx, uwbs_io_t irqPin, uwbs_io_callback *pCallback);

/**
 * @brief      Disable the Host IRQ
 *
 * @param      pCtx  The context
 * @param      irqPin  IRQ
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_io_irq_dis(uwb_bus_board_ctx_t *pCtx, uwbs_io_t irqPin);

/** @} */

/** @defgroup uwb_bus_data UWB BUS DATA Interface
 *
 * @{
 */

/**
 * @brief Transmit a data frame
 *
 * @param      pCtx    The context
 * @param[in]  pBuf    The data that we want to transmit
 * @param[in]  bufLen  The data length
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_data_tx(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t bufLen);

#if UWBIOT_UWBD_SR040
/**
 * @brief Transmit a data frame without assert config Flags for SPI.
 *
 * @param      pCtx    The context
 * @param[in]  pBuf    The data that we want to transmit
 * @param[in]  bufLen  The data length
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_data_tx_no_assert(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t bufLen);
#endif

/**
 * @brief Receive a data frame
 *
 * @param      pCtx     The context
 * @param[out] pBuf     The pointer where we copy received data
 * @param[in]  pBufLen  Input: No of bytes to read.
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_data_rx(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t pBufLen);

#if ((defined(UWBIOT_UWBD_SR2XXT)) && (UWBIOT_UWBD_SR2XXT != 0))
/**
 * @brief Send and Receive a data frame full dupex
 *
 * @param      pCtx     The context
 * @param[in]  pTxBuf     The data that we want to transmit
 * @param[out] pRxBuf     The pointer where we copy received data
 * @param[in]  pBufLen  Input: No of bytes to read.
 * @param[in]  mode  Input: spi read write mode
 *                          0 -> data write
 *                          1 -> data read
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_data_trx(uwb_bus_board_ctx_t *pCtx, uint8_t *pRxBuf, size_t pBufLen);
#endif // UWBIOT_UWBD_SR2XXT

/** @} */

#if ((defined(UWBIOT_UWBD_SR040)) && (UWBIOT_UWBD_SR040 != 0))
/** @defgroup uwb_bus_data_crc  UWB BUS DATA CRC modem apis
 *
 * @{
 */
#if ((defined(UWBIOT_HOST_S32K144_SR040)) && (UWBIOT_HOST_S32K144_SR040 != 0))
/**
 * @brief initialise crc modem
 *
 * @param      pCtx     The context
 */
uwb_bus_status_t uwb_bus_data_crc16_xmodem_init(uwb_bus_board_ctx_t *pCtx);
#endif // ((defined(UWBIOT_HOST_S32K144_SR040)) && (UWBIOT_HOST_S32K144_SR040 != 0))
/**
 * @brief Compute CRC of input data
 *
 * @param     pCtx     The context
 * @param[in] input    The pointer to input data
 * @param[in] len     length of input data
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_data_crc16_xmodem(uwb_bus_board_ctx_t *pCtx, uint8_t *input, size_t len);

/** @} */

/** @defgroup uwb_bus_reset reset helios apis
 *
 * @{
 */

/** Reset SR040 helios assert
 *
 */
void uwb_bus_reset_sr040_assert();
/** @} */
#endif //#if ((defined(UWBIOT_UWBD_SR040)) && (UWBIOT_UWBD_SR040 != 0))

/** @} */
#endif
