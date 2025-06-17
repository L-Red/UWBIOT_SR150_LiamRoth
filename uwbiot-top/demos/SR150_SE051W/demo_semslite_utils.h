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

#include <fsl_sss_se05x_apis.h>
#include <nxEnsure.h>
#include <string.h>
#include <global_platf.h>
#include <smCom.h>

#include "sems_lite_api.h"

#define DEMO_SEMSLITE_OEF_A693      (0xA693)
#define DEMO_SEMSLITE_OEF_A739      (0xA739)
#define DEMO_SEMSLITE_OEF_A739_RUN3 (0x03)

sss_status_t delete_package(sems_lite_agent_ctx_t *psems_ctx, multicast_package_t *pPackage);
sss_status_t demo_semslite_updateApplet(sems_lite_agent_ctx_t *psems_ctx, multicast_package_t *pPackage);
sss_status_t JCOP4_GetDataIdentify(void *conn_ctx, uint8_t *rspBuf, uint16_t *rspBufLen);
U32 demo_semslite_secom_txrx_raw(void *conn_ctx, uint8_t *pTx, uint16_t txLen, U8 *pRx, U32 *pRrxLen);
