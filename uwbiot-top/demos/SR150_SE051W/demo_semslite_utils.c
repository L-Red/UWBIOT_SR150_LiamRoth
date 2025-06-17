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
#include "AppRecovery.h"
#include "UwbApi.h"
#include <AppInternal.h>
#include <ex_sss.h>
#include <ex_sss_boot.h>
#include <demo_semslite_utils.h>
#include "smCom.h"

/* Execute and decode GET DATA IDENTIFY as specified for SE05x JCOP
*
* @warn After this call, the applet is deselected and applet commands won't work.
*       The applet session needs to be re-established
*       (select applet, establish optional session, like in the sss_session_open()
*/

sss_status_t JCOP4_GetDataIdentify(void *conn_ctx, uint8_t *rspBuf, uint16_t *rspBufLen)
{
    sss_status_t status = kStatus_SSS_Fail;
    smStatus_t rxStatus;
    const uint8_t cmd[] = {
        0x80, // CLA '80' / '00' GlobalPlatform / ISO / IEC
        0xCA, // INS 'CA' GET DATA(IDENTIFY)
        0x00, // P1 '00' High order tag value
        0xFE, // P2 'FE' Low order tag value - proprietary data
        0x02, // Lc '02' Length of data field
        0xDF,
        0x28, // Data 'DF28' Card identification data
        0x00  // Le '00' Length of response data
    };

    U32 prspLen         = *rspBufLen;
    U16 dummyResponse16 = *rspBufLen;
    /* Select card manager / ISD
    * (ReUsing same dummy buffers) */
    rxStatus = GP_Select(conn_ctx, (uint8_t *)rspBuf /* dummy */, 0, (uint8_t *)rspBuf, &dummyResponse16);
    if (rxStatus != SM_OK) {
        LOG_E("Could not select ISD.");
        goto cleanup;
    }
    rxStatus = smCom_TransceiveRaw(conn_ctx, (uint8_t *)cmd, sizeof(cmd), (uint8_t *)rspBuf, &prspLen);
    if (rxStatus == SM_OK) {
        LOG_I("JCOP4_GetDataIdentify Successful");
    }
    else {
        LOG_E("Error in retreiving the JCOP Identifier. Response is as follows");
        LOG_AU8_E(rspBuf, prspLen);
        goto cleanup;
    }
    status = kStatus_SSS_Success;

cleanup:
    *rspBufLen = prspLen;
    return status;
}

sss_status_t delete_package(sems_lite_agent_ctx_t *psems_ctx, multicast_package_t *pPackage)
{
    return demo_semslite_updateApplet(psems_ctx, pPackage);
}

sss_status_t demo_semslite_updateApplet(sems_lite_agent_ctx_t *psems_ctx, multicast_package_t *pPackage)
{
    sems_lite_status_t sems_status;
    sss_status_t sss_status = kStatus_SSS_Fail;

    sems_status = sems_lite_agent_load_package(psems_ctx, pPackage);
    if (sems_status == kStatus_SEMS_Lite_Success) {
        sss_status = kStatus_SSS_Success;
    }
    else {
        LOG_E("sems_lite_agent_load_package failed");
    }
    return sss_status;
}

U32 demo_semslite_secom_txrx_raw(void *conn_ctx, uint8_t *pTx, uint16_t txLen, U8 *pRx, U32 *pRrxLen)
{
    U32 smCom_ret;

    smCom_ret = smCom_TransceiveRaw(conn_ctx, (uint8_t *)(pTx), (uint16_t)(txLen), pRx, pRrxLen);
    if (smCom_ret != SMCOM_OK) {
        LOG_E("smCom_TransceiveRaw failed!!!");
    }
    return smCom_ret;
}
