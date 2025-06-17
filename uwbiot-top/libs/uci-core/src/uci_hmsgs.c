/*
 *
 * Copyright 2018-2020,2022,2023 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

/******************************************************************************
 *
 *  This file contains function of the UCI unit to format and send UCI
 *  commands (for DH).
 *
 ******************************************************************************/

#include "uwb_target.h"
#include "UwbAdaptation.h"
#include "uci_defs.h"
#include "uci_test_defs.h"
#include "uci_hmsgs.h"
#include "uwb_int.h"
#include "uwa_sys.h"
#include "uwb_hal_int.h"
#include "phNxpLogApis_UciCore.h"
#include "uwa_dm_int.h"
#if (NXP_UWB_EXTNS == TRUE)
#include "uci_ext_defs.h"
#endif

/*******************************************************************************
**
** Function         getGidOid
**
** Description      To get the gid & oid based on the event
**
** Returns         gid & oid based on the event
**
*******************************************************************************/
uint16_t getGidOid(uint16_t eventId)
{
    uint16_t gidOid = INVALID_GID_OID;
    switch (eventId) {
    case UWA_DM_API_SESSION_INIT_EVT:
        gidOid = (uint16_t)SESSION_INIT_GID_OID;
        break;
    case UWA_DM_API_CORE_SET_CONFIG_EVT:
        gidOid = (uint16_t)CORE_SET_CONFIG_GID_OID;
        break;
    case UWA_DM_API_CORE_GET_DEVICE_INFO_EVT:
        gidOid = (uint16_t)CORE_GET_DEVICE_INFO_GID_OID;
        break;
    case UWA_DM_API_CORE_GET_CAPS_INFO_EVT:
        gidOid = (uint16_t)CORE_GET_CAPS_INFO_GID_OID;
        break;
    case UWA_DM_API_CORE_DEVICE_RESET_EVT:
        gidOid = (uint16_t)CORE_DEVICE_RESET_GID_OID;
        break;
    case UWA_DM_API_CORE_GET_CONFIG_EVT:
        gidOid = (uint16_t)CORE_GET_CONFIG_GID_OID;
        break;
    case UWA_DM_API_SESSION_DEINIT_EVT:
        gidOid = (uint16_t)SESSION_DEINIT_GID_OID;
        break;
    case UWA_DM_API_SESSION_SET_APP_CONFIG_EVT:
        gidOid = (uint16_t)SESSION_SET_APP_CONFIG_GID_OID;
        break;
    case UWA_DM_API_SESSION_GET_APP_CONFIG_EVT:
        gidOid = (uint16_t)SESSION_GET_APP_CONFIG_GID_OID;
        break;
    case UWA_DM_API_SESSION_STOP_EVT:
        gidOid = (uint16_t)SESSION_STOP_GID_OID;
        break;
    case UWA_DM_API_SESSION_START_EVT:
        gidOid = (uint16_t)SESSION_START_GID_OID;
        break;
    case UWA_DM_API_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_EVT:
        gidOid = (uint16_t)SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_GID_OID;
        break;
#if !(UWBIOT_UWBD_SR040)
    case UWA_DM_API_TEST_SET_CONFIG_EVT:
        gidOid = (uint16_t)SET_TEST_CONFIG_GID_OID;
        break;
    case UWA_DM_API_TEST_GET_CONFIG_EVT:
        gidOid = (uint16_t)TEST_GET_CONFIG_GID_OID;
        break;
    case UWA_DM_API_TEST_PERIODIC_TX_EVT:
        gidOid = (uint16_t)TEST_PERIODIC_TX_GID_OID;
        break;
    case UWA_DM_API_TEST_PER_RX_EVT:
        gidOid = (uint16_t)TEST_PER_RX_GID_OID;
        break;
    case UWA_DM_API_TEST_UWB_LOOPBACK_EVT:
        gidOid = (uint16_t)TEST_UWB_LOOPBACK_GID_OID;
        break;
    case UWA_DM_API_TEST_RX_EVT:
        gidOid = (uint16_t)TEST_RX_GID_OID;
        break;
    case UWA_DM_API_TEST_STOP_SESSION_EVT:
        gidOid = (uint16_t)TEST_STOP_SESSION_GID_OID;
        break;
#endif //!(UWBIOT_UWBD_SR040)
    case UWA_DM_API_SESSION_GET_STATE_EVT:
        gidOid = (uint16_t)SESSION_GET_STATE_GID_OID;
        break;
#if (UWBIOT_UWBD_SR100T)
    case UWA_DM_API_PROP_GENERATE_TAG:
        gidOid = (uint16_t)GENERATE_TAG_GID_OID;
        break;
    case UWA_DM_API_PROP_CALIB_INTEGRITY_PROTECTION:
        gidOid = (uint16_t)CALIB_INTEGRITY_PROTECTION_GID_OID;
        break;
    case UWA_DM_API_PROP_VERIFY_CALIB_DATA:
        gidOid = (uint16_t)VERIFY_CALIB_DATA_GID_OID;
        break;
    case UWA_DM_API_PROP_DO_BIND:
        gidOid = (uint16_t)DO_BIND_GID_OID;
        break;
    case UWA_DM_API_PROP_TEST_CONNECTIVITY:
        gidOid = (uint16_t)TEST_CONNECTIVITY_GID_OID;
        break;
    case UWA_DM_API_PROP_TEST_SE_LOOP:
        gidOid = (uint16_t)TEST_SE_LOOP_GID_OID;
        break;
    case UWA_DM_API_PROP_GET_BINDING_STATUS:
        gidOid = (uint16_t)GET_BINDING_STATUS_GID_OID;
        break;
    case UWA_DM_API_PROP_CONFIG_AUTH_TAG_OPTIONS:
        gidOid = (uint16_t)CONFIG_AUTH_TAG_OPTIONS_GID_OID;
        break;
    case UWA_DM_API_PROP_CONFIG_AUTH_TAG_VERSIONS:
        gidOid = (uint16_t)CONFIG_AUTH_TAG_VERSIONS_GID_OID;
        break;
    case UWA_DM_API_PROP_URSK_DELETION_REQUEST:
        gidOid = (uint16_t)URSK_DELETION_REQUEST_GID_OID;
        break;
#endif //UWBIOT_UWBD_SR100T

#if !(UWBIOT_UWBD_SR040)
    case UWA_DM_API_VENDOR_DO_VCO_PLL_CALIBRATION:
        gidOid = (uint16_t)VENDOR_DO_VCO_PLL_CALIBRATION_GID_OID;
        break;
    case UWA_DM_API_PROP_GET_BINDING_COUNT:
        gidOid = (uint16_t)GET_BINDING_COUNT_GID_OID;
        break;
    case UWA_DM_API_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_EVT:
        gidOid = (uint16_t)SESSION_UPDATE_DT_ANCHOR_RANGING_ROUND_GID_OID;
        break;
    case UWA_DM_API_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_EVT:
        gidOid = (uint16_t)SESSION_UPDATE_DT_TAG_RANGING_ROUND_GID_OID;
        break;
    case UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_EVT:
        gidOid = (uint16_t)SESSION_QUERY_DATA_SIZE_IN_RANGING_GID_OID;
        break;
    case UWA_DM_API_PROP_WRITE_OTP_CALIB_DATA:
        gidOid = (uint16_t)WRITE_OTP_CALIB_DATA_GID_OID;
        break;
    case UWA_DM_API_PROP_READ_OTP_CALIB_DATA:
        gidOid = (uint16_t)READ_OTP_CALIB_DATA_GID_OID;
        break;
    case UWA_DM_API_SESSION_SET_HUS_CONTROLLER_CONFIG_EVT:
        gidOid = (uint16_t)SESSION_SET_HUS_CONTROLLER_CONFIG_GID_OID;
        break;
    case UWA_DM_API_SESSION_SET_HUS_CONTROLEE_CONFIG_EVT:
        gidOid = (uint16_t)SESSION_SET_HUS_CONTROLEE_CONFIG_GID_OID;
        break;
    case UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_EVT:
        gidOid = SESSION_DATA_TRANSFER_PHASE_CONFIG_GID_OID;
        break;
    case UWA_DM_API_PROP_QUERY_TEMP:
        gidOid = (uint16_t)QUERY_TEMP_GID_OID;
        break;
    case UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION:
        gidOid = (uint16_t)VENDOR_SET_DEVICE_CALIBRATION_GID_OID;
        break;
    case UWA_DM_API_VENDOR_GET_DEVICE_CALIBRATION:
        gidOid = (uint16_t)VENDOR_GET_DEVICE_CALIBRATION_GID_OID;
        break;
    case UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT:
        gidOid = (uint16_t)SESSION_SET_VENDOR_APP_CONFIG_GID_OID;
        break;
    case UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT:
        gidOid = (uint16_t)SESSION_GET_VENDOR_APP_CONFIG_GID_OID;
        break;
#endif //!(UWBIOT_UWBD_SR040)
#if (UWBIOT_UWBD_SR150)
    case UWA_DM_API_SESSION_SET_LOCALIZATION_ZONE_EVT:
        gidOid = (uint16_t)SESSION_SET_LOCALIZATION_ZONE_GID_OID;
        break;
#endif // (UWBIOT_UWBD_SR150)
    case UWA_DM_API_CORE_QUERY_UWBS_TIMESTAMP:
        gidOid = (uint16_t)CORE_QUERY_UWBS_TIMESTAMP_GID_OID;
        break;
    case UWA_DM_API_VENDOR_GET_ALL_UWB_SESSIONS:
        gidOid = (uint16_t)GET_ALL_UWB_SESSIONS_GID_OID;
        break;
#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
    case UWA_DM_API_TRNG_EVENT:
        gidOid = (uint16_t)GET_TRNG_GID_OID;
        break;
#endif // UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160
#if UWBIOT_UWBD_SR040
    case UWA_DM_API_SUSPEND_DEVICE_EVENT:
        gidOid = (uint16_t)GET_SUSPEND_DEVICE_GID_OID;
        break;
    case UWA_DM_API_SESSION_NVM_EVENT:
        gidOid = (uint16_t)GET_SESSION_NVM_GID_OID;
        break;
    case UWA_DM_START_TEST_MODE_EVENT:
        gidOid = (uint16_t)GET_START_TEST_MODE_GID_OID;
        break;
    case UWA_DM_STOP_TEST_MODE_EVENT:
        gidOid = (uint16_t)GET_STOP_TEST_MODE_GID_OID;
        break;
    case UWA_DM_SET_CALIB_TRIM_EVENT:
        gidOid = (uint16_t)SET_CALIB_TRIM_MODE_GID_OID;
        break;
    case UWA_DM_GET_CALIB_TRIM_EVENT:
        gidOid = (uint16_t)GET_CALIB_TRIM_MODE_GID_OID;
        break;
    case UWA_BYPASS_CURRENT_LIMITER:
        gidOid = (uint16_t)GID_OID_BYPASS_CURRENT_LIMITER;
        break;
#endif
#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
    case UWA_DM_API_PROFILE_PARAM_EVENT:
        gidOid = (uint16_t)GID_OID_PROFILE_PARAM_MODE;
        break;
#endif // UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160
    default:
        break;
    }
    return gidOid;
}

/*******************************************************************************
**
** Function         uci_snd_cmd_interface
**
** Description     Interface that internally calls UCI SEND command
**
** Returns         True in order to free the allocated memory in event handler
**
*******************************************************************************/
bool uci_snd_cmd_interface(uint16_t eventId, uint16_t length, uint8_t *data, uint8_t pbf)
{
    uint8_t status;

    status = uci_snd_cmd(eventId, length, data, pbf);
    if (status != UCI_STATUS_OK) {
        UCI_TRACE_E("uci_snd_cmd(): failed ,status=0x%X", status);
    }
    else {
        UCI_TRACE_D("uci_snd_cmd(): success ,status=0x%X", status);
    }
    return (TRUE);
}

/*******************************************************************************
**
** Function         uci_snd_cmd
**
** Description      compose and send command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_cmd(uint16_t eventId, uint16_t length, uint8_t *data, uint8_t pbf)
{
    UCI_TRACE_D(__FUNCTION__);
    UWB_HDR *p;
    uint8_t *pp;
    uint16_t gidOid     = INVALID_GID_OID;
    uint16_t payloadLen = length;
    uint16_t offset     = 0;

    do {
        // For Send Data, use pbf from api layer
#if UWBIOT_UWBD_SR040
        if ((eventId != UWA_DM_API_SEND_DATA_EVENT) && (eventId != UWA_DM_START_TEST_MODE_EVENT)) {
#else
        if (eventId != (UWA_DM_API_SEND_DATA_EVENT) && (eventId != UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION))
        {
#endif //UWBIOT_UWBD_SR040
            if (length > UCI_MAX_PAYLOAD_SIZE) {
                payloadLen = UCI_MAX_PAYLOAD_SIZE;
                pbf        = 1;
            }
            else {
                payloadLen = length;
                pbf        = 0;
            }
        }

        if ((p = UCI_GET_CMD_BUF(payloadLen)) == NULL)
            return (UCI_STATUS_FAILED);

        p->event          = BT_EVT_TO_UWB_UCI;
        p->len            = (uint16_t)(UCI_MSG_HDR_SIZE + payloadLen);
        p->offset         = UCI_MSG_OFFSET_SIZE;
        p->layer_specific = 0;
        pp                = (uint8_t *)(p + 1) + p->offset;

        if (eventId == UWA_DM_API_SEND_DATA_EVENT) {
            // For Send Data, MT should be data type
            UCI_MSG_PBLD_HDR0(pp, UCI_MT_DATA, pbf, UCI_DPF_SND);
            UCI_MSG_BLD_HDR1(pp, 0x00); // there is no OID for send data
            UWB_UINT8_TO_STREAM(pp, payloadLen & 0xFF);
            UWB_UINT8_TO_STREAM(pp, payloadLen >> 8);
            UWB_ARRAY_TO_STREAM(pp, data, payloadLen);
            uwb_ucif_send_cmd(p);
            break;
        }
#if !(UWBIOT_UWBD_SR040)
        else if(eventId == UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION){
            gidOid = getGidOid(eventId);
            UCI_MSG_PBLD_HDR0(pp, UCI_MT_CMD, pbf, (uint8_t)(gidOid >> 8));
            if (length > UCI_MAX_PAYLOAD_SIZE){
            	UCI_MSG_BLD_HDR1_EXT(pp, (uint8_t)gidOid);
                UWB_UINT8_TO_STREAM(pp, payloadLen & 0xFF);
                UWB_UINT8_TO_STREAM(pp, payloadLen >> 8)
            }
            else{
                UCI_MSG_BLD_HDR1(pp, (uint8_t)gidOid)
                UWB_UINT8_TO_STREAM(pp, 0x00);
                UWB_UINT8_TO_STREAM(pp, payloadLen);
            };
            UWB_ARRAY_TO_STREAM(pp, data, payloadLen);
            uwb_ucif_send_cmd(p);
            break;
        }
#endif // !(UWBIOT_UWBD_SR040)
        else {
            gidOid = getGidOid(eventId);
            UCI_MSG_PBLD_HDR0(pp, UCI_MT_CMD, pbf, (uint8_t)(gidOid >> 8));
            UCI_MSG_BLD_HDR1(pp, (uint8_t)gidOid);
            UWB_UINT8_TO_STREAM(pp, 0x00);
            UWB_UINT8_TO_STREAM(pp, payloadLen);
            if (payloadLen != 0) {
                UWB_ARRAY_TO_STREAM(pp, (&data[offset]), payloadLen);
            }
        }
        offset += payloadLen;
        length -= payloadLen;
        uwb_ucif_send_cmd(p);
    } while (length != 0);

    return (UCI_STATUS_OK);
}

/*******************************************************************************
 **
 ** Function         uci_proc_raw_cmd_rsp
 **
 ** Description      Process RAW CMD responses
 **
 ** Returns          void
 **
 *******************************************************************************/
void uci_proc_raw_cmd_rsp(uint8_t *p_buf, uint16_t len)
{
    tUWB_RAW_CBACK *p_cback = uwb_cb.p_raw_cmd_cback;

    UCI_TRACE_D(" uci_proc_raw_cmd_rsp:"); // for debug

    /* If there's a pending/stored command, restore the associated address of the
   * callback function */
    if (p_cback == NULL) {
        UCI_TRACE_E("p_raw_cmd_cback is null");
    }
    else {
        /**
          *Invokes rawCommandResponse_Cb with Raw UCI Data in UCI.
        */
        (*p_cback)(0, 0 /*unused in this case*/, len, p_buf);
        uwb_cb.p_raw_cmd_cback = NULL;
    }
    uwb_cb.rawCmdCbflag = FALSE;
    uwb_ucif_update_cmd_window();
}

/*******************************************************************************
 **
 ** Function         uci_proc_proprietary_ntf
 **
 ** Description      Process UCI notifications in the proprietary Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
void uci_proc_proprietary_ntf(uint8_t gid, uint8_t op_code, uint8_t *p_buf, uint16_t len)
{
    if (len > 0) {
        UCI_TRACE_D(" uci_proc_raw_cmd_rsp:"); // for debug

        if (uwb_cb.p_ext_resp_cback == NULL) {
            UCI_TRACE_E("ext response callback is null");
        }
        else {
            switch (op_code) {
#if UWBIOT_UWBD_SR100T
                /* Perform Reset incase of SE Communication Failure*/

            case EXT_UCI_MSG_SE_COMM_ERROR_NTF: {
                uint8_t *status_ptr = p_buf + UCI_RESPONSE_STATUS_OFFSET;
                uint8_t status      = *status_ptr;
                if (status == UCI_STATUS_ESE_RECOVERY_FAILURE) {
                    tHAL_UWB_IOCTL ioCtl;
                    uwb_uci_IoctlInOutData_t inpOutData;
                    uint8_t stat;
                    ioCtl.pIoData = &inpOutData;
                    stat          = uwb_cb.p_hal->ioctl(HAL_UWB_IOCTL_ESE_RESET, &ioCtl);
                    if (stat == UCI_STATUS_OK) {
                        UCI_TRACE_D("%s: Set ESE RESET successful", __FUNCTION__);
                    }
                    else {
                        UCI_TRACE_E("%s: Set ESE RESET Failed", __FUNCTION__);
                    }
                }
            } break;
#endif //UWBIOT_UWBD_SR100T
            default:
                break;
            }

            /* Invokes extDeviceManagementCallBack with Raw UCI Data in JNI */
            (*uwb_cb.p_ext_resp_cback)(gid, (tUWB_RAW_EVT)(op_code), len, p_buf);
        }
    }
    else {
        UCI_TRACE_E("%s: len is zero", __FUNCTION__);
    }
}
