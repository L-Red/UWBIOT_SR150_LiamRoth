/* Copyright 2020 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef __UWB_VCOM_PNP_H_RV4__
#define __UWB_VCOM_PNP_H_RV4__

uint32_t transmitToUsart(uint8_t *pData, size_t size);
void Uwb_USART_Init(void (*rcvCb)(uint8_t *, uint32_t *));
bool Uwb_USART_TrasnferStatus(USART_Type *base);

#define MAX_UWBS_SPI_TRANSFER_TIMEOUT   (1000)
#define USART_DEVICE_INTERRUPT_PRIORITY (3U)

#define Uwb_Vcom_Init        Uwb_USART_Init
#define UWB_Vcom_UciSendNtfn transmitToUsart
#define UWB_Vcom_SendUCIRsp  transmitToUsart
#define UWB_Vcom_SendRsp     transmitToUsart

#endif /* __UWB_VCOM_PNP_H_RV4__ */
