/*
 * Copyright 2021-2023 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 */

#include "StateMachine.h"
#include "SE_Wrapper.h"
#include "UWB_Wrapper.h"
#include "phNxpLogApis_UwbApi.h"
#include "UwbApi.h"

/**
  * \brief Performs Locking only if the current state is bound and
  *        Unlocked. This is only supported with Helios Mainline Firmware.
  *
  * \retval #UWBAPI_STATUS_OK               on success
  * \retval #UWBAPI_STATUS_FAILED           otherwise
  */
tUWBAPI_STATUS locking_Process(void)
{
    se_status_t se_status;
    tUWBAPI_STATUS uwb_status;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    uwbSeContext.applet                = SE_APPLET_SUS;
    uwbSeContext.bindState.boundStatus = SE_Not_Bound;
    /* This is used as the response legnth from SE side */
    uwbSeContext.seRspLen = sizeof(uwbSeContext.ApduResp.APDUPayloadData);

    se_status = Se_API_Init(uwbSeContext.applet, SE_CHANNEL_1);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("Applet Selection Failed");
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

    /* First check the device bound state. If state is bound and
     * Unlocked then only initiate locking process.
     */
    se_status = Se_API_GetBindingState(&uwbSeContext.bindState);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("SE get binding state failed");
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

    if (SE_Bound_And_Unlocked == uwbSeContext.bindState.boundStatus) {
#if 0
        uwb_status = ReinjectBdi();
        if (uwb_status != UWBAPI_STATUS_OK) {
           NXPLOG_UWBAPI_E("ReinjectBdi Failed");
           goto exit;
        }
#endif
        /* Now establish SCP channel. */
        uwb_status = Authenticate_Scp03(NONBINDING);
        if (uwb_status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("SCP channel establishemnt failed");
            goto exit;
        }
    }
    else {
        if (SE_Bound_And_Locked == uwbSeContext.bindState.boundStatus) {
            NXPLOG_UWBAPI_W("Already in Locked State %d", uwbSeContext.bindState.boundStatus);
            uwb_status = UWBAPI_STATUS_OK;
        }
        else {
            NXPLOG_UWBAPI_W("Can't perform Locking. First do binding then followed by locking as current state is %d",
                uwbSeContext.bindState.boundStatus);
            uwb_status = UWBAPI_STATUS_FAILED;
        }
        goto exit;
    }

    /* Initiate Lock Binding on the UWB side */
    uwb_status = Set_SE_Apdu_Enc(LOCK_BIND, &uwbSeContext.ApduCmd);
    if (uwb_status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UWB Lock Binding failed");
        goto exit;
    }

    /* Initiate Lock Binding on the SE side */
    se_status = Se_API_SendReceive(&uwbSeContext.ApduCmd.APDUPayloadData[0],
        (size_t)uwbSeContext.ApduCmd.APDUPayloadLength,
        &uwbSeContext.ApduResp.APDUPayloadData[0],
        &uwbSeContext.seRspLen);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("SE Lock Binding failed");
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    /* Use the response length from SE side. */
    uwbSeContext.ApduResp.APDUPayloadLength = (uint8_t)uwbSeContext.seRspLen;

    /* Validate the response from SE side for finalization */
    // Include 9000 as well
    uwbSeContext.ApduResp.APDUPayloadData[uwbSeContext.ApduResp.APDUPayloadLength++] = 0x90;
    uwbSeContext.ApduResp.APDUPayloadData[uwbSeContext.ApduResp.APDUPayloadLength++] = 0x00;
    uwb_status = Validate_Se_Apdu_Resp_Cmd(&uwbSeContext.ApduResp);
    if (uwb_status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UWB Validate Apdu response failed");
        goto exit;
    }

    /* Close the channel and Deselect the applet */
    /* Required because Get Binding state command will not work
       if secure channel is established. */
    Se_API_DeInit();

    se_status = Se_API_Init(uwbSeContext.applet, SE_CHANNEL_1);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("Applet Selection Failed");
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

    /* Validate the state. */
    se_status = Se_API_GetBindingState(&uwbSeContext.bindState);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("SE get binding state failed");
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    if (uwbSeContext.bindState.boundStatus != SE_Bound_And_Locked) {
        NXPLOG_UWBAPI_E("Locking Failed");
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    NXPLOG_UWBAPI_D("Locking Successful");

exit:
    /* Close the channel */
    Se_API_DeInit();

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return uwb_status;
}
