/****************************************************************************
 *
 *   Description:
 *     Header file for SPI specific interface to A71x family devices
 *
 ****************************************************************************
 * Copyright 2016 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
****************************************************************************/
#ifndef _SPI_A7_H
#define _SPI_A7_H

#include "sm_types.h"

#define SPI_A7_INIT_OK              100
#define SPI_A7_DEV_OPEN_FAILED      110
#define SPI_A7_SET_MODE_FAILED      140
#define SPI_A7_SET_BITS_FAILED      141
#define SPI_A7_SET_MAX_SPEED_FAILED 142
#define SPI_A7_TRANSFER_OK          150
#define SPI_A7_TRANSFER_FAILED      151

int spiA7Init(void);
/**
 * @function spiTransfer
 * @description Transfers data over spi device interface. Supports full duplex communication.
 * @param txBuf    Either NULL in case only data to be received or a buffer of at least size txLen
 * @param rxBuf    Either NULL in case only data to be sent or a buffer of at least size txLen
 * @param txLen    IN: amount of byte to be sent/received/transceived
 * @return
 */
int spiTransfer(U8 *txBuf, U8 *rxBuf, U16 txLen);

#endif /* end _SPI_A7_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
