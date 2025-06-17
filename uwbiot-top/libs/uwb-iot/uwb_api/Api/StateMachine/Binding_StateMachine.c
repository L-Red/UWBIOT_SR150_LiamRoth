/*
 * Copyright 2021-2022 NXP.
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

/* SE Context variable */
phUwbSeContext_t uwbSeContext;

/**
 * \brief Performs Factory Binding only if the current state is not bound and
 * not locked.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
tUWBAPI_STATUS Binding_Process(void)
{
    tUWBAPI_STATUS uwb_status = UWBAPI_STATUS_FAILED;
    se_status_t se_status;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    uwbSeContext.applet                = SE_APPLET_SUS;
    uwbSeContext.bindState.boundStatus = SE_Not_Bound;
    /* This is used as the response length from SE side */
    uwbSeContext.seRspLen = sizeof(uwbSeContext.ApduResp.APDUPayloadData);

    se_status = Se_API_Init(uwbSeContext.applet, SE_CHANNEL_1);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("Applet Selection Failed");
        goto exit;
    }

    /* First check the device state. If it only unbound then
     * initiate binding process.
     */
    se_status = Se_API_GetBindingState(&uwbSeContext.bindState);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("SE get binding state failed");
        goto exit;
    }
    if (SE_Bound_And_Locked == uwbSeContext.bindState.boundStatus) {
        NXPLOG_UWBAPI_W("Already in the Bound and Locked state %d cannot bind", uwbSeContext.bindState.boundStatus);
        uwb_status = UWBAPI_STATUS_OK;
        goto exit;
    }

    /* Now establish SCP channel. */
    uwb_status = Authenticate_Scp03(BINDING);
    if (uwb_status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("SCP channel establishemnt failed");
        goto exit;
    }

    /* Finalize the binding process */
    /* Initiate Binding Finalize on the uwb side */
    uwb_status = Set_SE_Apdu_Enc(FINALIZE, &uwbSeContext.ApduCmd);
    if (uwb_status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UWB Binding FINALIZE failed");
        goto exit;
    }

    /* Initiate Binding Finalize on the se side */
    se_status = Se_API_SendReceive(&uwbSeContext.ApduCmd.APDUPayloadData[0],
        (size_t)uwbSeContext.ApduCmd.APDUPayloadLength,
        &uwbSeContext.ApduResp.APDUPayloadData[0],
        &uwbSeContext.seRspLen);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("SE Binding FINALIZE failed");
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

    /* Finally do the commit BDI Operation */
    uwb_status = Commit_Bdi();
    if (uwb_status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UWB Binding FINALIZE failed");
        goto exit;
    }

#if 0
    /* Calculate BDI */
    uwb_status = StoreBdi();
    if (uwb_status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("StoreBdi failed");
        goto exit;
    }
#endif
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
    if (SE_Bound_And_Unlocked != uwbSeContext.bindState.boundStatus) {
        NXPLOG_UWBAPI_E("Not in Bound and unlocked state %d", uwbSeContext.bindState.boundStatus);
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    NXPLOG_UWBAPI_D("Binding Flow Success");

exit:
    /* Close the channel and Deselect the applet */
    Se_API_DeInit();

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return uwb_status;
}
