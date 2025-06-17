/* Copyright 2021 NXP
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

#ifndef _UWB_TLV_HANDLERS_H_
#define _UWB_TLV_HANDLERS_H_

#include "PrintUtility.h"
#include "UwbApi.h"
#include "mctt_pctt_app.h"
#include "uwb_types.h"

void mctt_handler(uint8_t *valueBuffer, uint16_t length, uint16_t *pRespSize, uint8_t *pRespBuf);
extern void McttAppDataCallback(uint8_t *recvData, uint16_t recvDataLen);
tUWBAPI_STATUS mctt_pctt_ReadOtpData(uint8_t channel);
tUWBAPI_STATUS mctt_pctt_common_settings(void);
tUWBAPI_STATUS mctt_pctt_common_init(void);
tUWBAPI_STATUS mctt_pctt_common_deinit();
tUWBAPI_STATUS demo_set_wraped_rds(uint32_t session_id);

#endif // _UWB_TLV_HANDLERS_H_
