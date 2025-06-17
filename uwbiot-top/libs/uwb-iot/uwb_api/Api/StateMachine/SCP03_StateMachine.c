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
#include <phOsalUwb.h>

/**
 * \brief Sets up Secure channel depending on the operation type.
 * During Bidning, extra step of initiating binding process will be done.
 * Initialize Update and External authenticate will be done to
 * establish the secure channel.
 * Before calling this interface, applet selection shall be done by the caller.
 * Upon any failure, deselection of the applets shall be done by the caller.
 *
 * \param[in] bOpType                 Operation Type Binding or nonbinding.
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS Authenticate_Scp03(eOperation_Type_t bOpType)
{
    se_status_t se_status;
    tUWBAPI_STATUS uwb_status;
    /* Contains response for Initiate Binding data from uwb */
    phInitBindingData_t InitBindingData;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    /* This is used as the response length from SE side */
    uwbSeContext.seRspLen = sizeof(uwbSeContext.ApduResp.APDUPayloadData);

    /* For binding Process, initiate binding and then establish SCP channel. */
    if (BINDING == bOpType) {
        /* Do get initiate binding */
        uwb_status = Get_Init_Binding_Data(&InitBindingData);
        if (uwb_status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UWB Init Binding data Failed");
            goto exit;
        }

        /* Save the Binding context */
        phOsalUwb_MemCopy(uwbSeContext.bindingContext.rand_HE, &InitBindingData.initData[RAND_HE_OFFSET], RAND_HE_LEN);
        phOsalUwb_MemCopy(
            uwbSeContext.bindingContext.helios_id, &InitBindingData.initData[HELOS_ID_OFFSET], HELOS_ID_LEN);

        /* Do SE Side Initiate Binding */
        se_status = Se_API_InitiateBinding(InitBindingData.initData[HELIOS_LC_OFFSET],
            InitBindingData.BRK_ID,
            &InitBindingData.initData[RAND_HE_OFFSET],
            INIT_BIND_DATA_LEN - HELIOS_LC_LEN, // Need to exclude Helios LC Length
            &uwbSeContext.ApduResp.APDUPayloadData[0],
            &uwbSeContext.seRspLen);
        if (se_status != SE_STATUS_OK) {
            NXPLOG_UWBAPI_E("SE Initiate Binding data Failed");
            uwb_status = UWBAPI_STATUS_FAILED;
            goto exit;
        }

        /* Save the Binding context */
        phOsalUwb_MemCopy(uwbSeContext.bindingContext.rand_SE, &uwbSeContext.ApduResp.APDUPayloadData[0], RAND_SE_LEN);
        phOsalUwb_MemCopy(
            uwbSeContext.bindingContext.se_id, &uwbSeContext.ApduResp.APDUPayloadData[SE_ID_OFFSET], SE_ID_LEN);

        /* Use the response length from SE side. */
        uwbSeContext.ApduResp.APDUPayloadLength = (uint8_t)uwbSeContext.seRspLen;

        /* Set the SE secret */
        uwb_status =
            Set_Se_Binding_Data(uwbSeContext.ApduResp.APDUPayloadLength, &uwbSeContext.ApduResp.APDUPayloadData[0]);
        if (uwb_status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UWB Set Secret data Failed");
            goto exit;
        }
    }
    /* Now do Init Update followed by External Authenticate */
    /* Do Initialize Update Command */
    uwb_status = Get_Se_Init_Update_Apdu(&uwbSeContext.ApduCmd);
    if (uwb_status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UWB Initialize Update Failed");
        goto exit;
    }
    uwbSeContext.seRspLen = UCI_MAX_PAYLOAD_SIZE;
    /* Call the SE side Initialize Update interface */
    se_status = Se_API_SendReceive(&uwbSeContext.ApduCmd.APDUPayloadData[0],
        (size_t)uwbSeContext.ApduCmd.APDUPayloadLength,
        &uwbSeContext.ApduResp.APDUPayloadData[0],
        &uwbSeContext.seRspLen);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("SE Initialize Update Failed");
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    /* Need to exclude first 13 bytes from the response.
     * Only card and Cryptogram length shall be used. */
    uwbSeContext.ApduResp.APDUPayloadLength = (CARD_CHALLENGE_LEN + CRYPTOGRAM_CHALLENGE_LEN);

    /* Call the SE side External Authenticate interface */
    uwb_status = Set_Se_Ext_Auth_Cmd(uwbSeContext.ApduResp.APDUPayloadLength,
        &uwbSeContext.ApduResp.APDUPayloadData[CARD_CRYPTOGRAM_OFFSET],
        &uwbSeContext.ApduCmd);
    if (uwb_status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UWB External Authentication Failed");
        goto exit;
    }
    /* Call the SE side External Authenticate interface */
    se_status = Se_API_SendReceive(&uwbSeContext.ApduCmd.APDUPayloadData[0],
        (size_t)uwbSeContext.ApduCmd.APDUPayloadLength,
        &uwbSeContext.ApduResp.APDUPayloadData[0],
        (size_t *)&uwbSeContext.ApduResp.APDUPayloadLength);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_UWBAPI_E("SE External Authentication Failed");
        uwb_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
exit:
    if (uwb_status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("SCP Success");
    }
    else {
        /* Update the status as only failed irrespective of the failure */
        uwb_status = UWBAPI_STATUS_FAILED;
        NXPLOG_UWBAPI_E("SCP Failed");
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return uwb_status;
}
