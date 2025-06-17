/******************************************************************************
*
* Copyright (C) 2010-2014 Broadcom Corporation
* Copyright 2018-2022 NXP.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************/

/******************************************************************************
*
*  UWA interface for device management
*
******************************************************************************/
#include "phNxpLogApis_UciCore.h"

#include "uwa_api.h"
#include "uwa_sys.h"
#include "uwa_dm_int.h"
#include "uwb_int.h"

tHAL_UWB_CONTEXT hal_Initcntxt;

/*****************************************************************************
**  APIs
*****************************************************************************/
/*******************************************************************************
**
** Function         UWA_Init
**
** Description      This function initializes control blocks for UWA
**
**                  p_hal_entry_tbl points to a table of HAL entry points
**
**                  NOTE: the buffer that p_hal_entry_tbl points must be
**                  persistent until UWA is disabled.
**
** Returns          none
**
*******************************************************************************/
void UWA_Init(const tHAL_UWB_ENTRY *p_hal_entry_tbl)
{
    UCI_TRACE_D(__FUNCTION__);
    hal_Initcntxt.hal_entry_func = p_hal_entry_tbl;
    uwa_sys_init();
    uwa_dm_init();
    /* Clear uwb control block */
    phOsalUwb_SetMemory(&uwb_cb, 0, sizeof(tUWB_CB));
    uwb_cb.p_hal                   = hal_Initcntxt.hal_entry_func;
    uwb_cb.uwb_state               = UWB_STATE_NONE;
    uwb_cb.uci_cmd_window          = UCI_MAX_CMD_WINDOW;
    uwb_cb.retry_rsp_timeout       = ((UWB_CMD_RETRY_TIMEOUT * QUICK_TIMER_TICKS_PER_SEC) / 1000);
    uwb_cb.uci_wait_rsp_tout       = ((UWB_CMD_CMPL_TIMEOUT * QUICK_TIMER_TICKS_PER_SEC) / 1000);
    uwb_cb.pLast_cmd_buf           = NULL;
    uwb_cb.is_resp_pending         = FALSE;
    uwb_cb.cmd_retry_count         = 0;
    uwb_cb.is_recovery_in_progress = FALSE;
    uwb_cb.IsConformaceTestEnabled = FALSE;
    uwb_cb.UwbOperatinMode         = 0;
    uwb_cb.isCreditNtfReceived     = false;
}

/*******************************************************************************
**
** Function         UWA_Enable
**
** Description      This function enables UWB. Prior to calling UWA_Enable,
**                  the UWBC must be powered up, and ready to receive commands.
**                  This function enables the tasks needed by UWB, opens the UCI
**                  transport, resets the UWB controller, downloads patches to
**                  the UWBC (if necessary), and initializes the UWB subsystems.
**
** Returns          UCI_STATUS_OK if successfully initiated
**                  UCI_STATUS_FAILED otherwise
**
*******************************************************************************/
#if UWBFTR_DataTransfer
tUCI_STATUS UWA_Enable(
    tUWA_DM_RSP_CBACK *p_dm_rsp_cback, tUWA_DM_NTF_CBACK *p_dm_ntf_cback, tUWA_DM_DATA_CBACK *p_dm_data_cback)
#else
tUCI_STATUS UWA_Enable(tUWA_DM_RSP_CBACK *p_dm_rsp_cback, tUWA_DM_NTF_CBACK *p_dm_ntf_cback)
#endif //UWBFTR_DataTransfer
{
    tUWA_DM_API_ENABLE *p_msg;

    UCI_TRACE_D(__FUNCTION__);

    /* Validate parameters */
    if (!p_dm_rsp_cback) {
        UCI_TRACE_E("error null rsp callback");
        return (UCI_STATUS_FAILED);
    }

    /* Validate parameters */
    if (!p_dm_ntf_cback) {
        UCI_TRACE_E("error null ntf callback");
        return (UCI_STATUS_FAILED);
    }
#if UWBFTR_DataTransfer
    /* Validate parameters */
    if (!p_dm_data_cback) {
        UCI_TRACE_E("error null data callback");
        return (UCI_STATUS_FAILED);
    }
#endif //UWBFTR_DataTransfer
    if ((p_msg = (tUWA_DM_API_ENABLE *)phOsalUwb_GetMemory(sizeof(tUWA_DM_API_ENABLE))) != NULL) {
        p_msg->hdr.event      = UWA_DM_API_ENABLE_EVT;
        p_msg->p_dm_rsp_cback = p_dm_rsp_cback;
        p_msg->p_dm_ntf_cback = p_dm_ntf_cback;
#if UWBFTR_DataTransfer
        p_msg->p_dm_data_cback = p_dm_data_cback;
#endif //UWBFTR_DataTransfer
        uwa_sys_sendmsg(p_msg);

        return (UCI_STATUS_OK);
    }

    return (UCI_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_Disable
**
** Description      This function is called to shutdown UWB. The tasks for UWB
**                  are terminated, and clean up routines are performed. This
**                  function is typically called during platform shut-down, or
**                  when UWB is disabled from a settings UI. When the UWB
**                  shutdown procedure is completed, an uwa_dm_disable_EVT is
**                  returned to the application using the tUWA_DM_CBACK.
**
** Returns          UCI_STATUS_OK if successfully initiated
**                  UCI_STATUS_FAILED otherwise
**
*******************************************************************************/
tUCI_STATUS UWA_Disable(bool graceful)
{
    tUWA_DM_API_DISABLE *p_msg;

    UCI_TRACE_D("UWA_Disable (graceful=%i)", graceful);

    if ((p_msg = (tUWA_DM_API_DISABLE *)phOsalUwb_GetMemory(sizeof(tUWA_DM_API_DISABLE))) != NULL) {
        p_msg->hdr.event = UWA_DM_API_DISABLE_EVT;
        p_msg->graceful  = graceful;

        uwa_sys_sendmsg(p_msg);

        return (UCI_STATUS_OK);
    }

    return (UCI_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_RegisterExtCallback
**
** Description      This function enables proprietary callbacks.
**
** Returns          UCI_STATUS_OK if successfully initiated
**                  UCI_STATUS_FAILED otherwise
**
*******************************************************************************/
tUCI_STATUS UWA_RegisterExtCallback(tUWA_RAW_CMD_CBACK *p_dm_ext_cback)
{
    tUWA_DM_API_REGISTER_EXT_CB *p_msg;

    UCI_TRACE_D(__FUNCTION__);

    /* Validate parameters */
    if (!p_dm_ext_cback) {
        UCI_TRACE_E("error null callback");
        return (UCI_STATUS_FAILED);
    }

    if ((p_msg = (tUWA_DM_API_REGISTER_EXT_CB *)phOsalUwb_GetMemory(sizeof(tUWA_DM_API_REGISTER_EXT_CB))) != NULL) {
        p_msg->hdr.event      = UWA_DM_API_REGISTER_EXT_CB_EVT;
        p_msg->p_dm_ext_cback = p_dm_ext_cback;
        uwa_sys_sendmsg(p_msg);

        return (UCI_STATUS_OK);
    }

    return (UCI_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_SendUciCommand
**
** Description      Send Uci command
**
** Returns          UCI_STATUS_OK on success
**                  UCI_STATUS_FAILED otherwise
**
*******************************************************************************/
tUCI_STATUS UWA_SendUciCommand(uint16_t event, uint16_t cmdLen, uint8_t *pCmd, uint8_t pbf)
{
    UCI_TRACE_D("UWA_SendUciCommand()");
    tUCI_CMD *p_msg = (tUCI_CMD *)phOsalUwb_GetMemory((uint16_t)(sizeof(tUCI_CMD) + (uint16_t)cmdLen));
    if (p_msg != NULL) {
        p_msg->hdr.event = event;
        p_msg->length    = cmdLen;
        p_msg->p_data    = (uint8_t *)(p_msg + 1);
        if ((pCmd != NULL) && (cmdLen != 0)) {
            phOsalUwb_MemCopy(p_msg->p_data, pCmd, cmdLen);
        }
        p_msg->pbf = pbf;
        uwa_sys_sendmsg(p_msg);
        return (UCI_STATUS_OK);
    }
    return (UCI_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_SendRawCommand
**
** Description      This function is called to send raw Vendor Specific
**                  command to Helios.
**
**                  cmd_params_len  - The command parameter len
**                  p_cmd_params    - The command parameter
**                  p_cback         - The callback function to receive the
**                                    command
**
** Returns          UCI_STATUS_OK if successfully initiated
**                  UCI_STATUS_FAILED otherwise
**
*******************************************************************************/
tUCI_STATUS UWA_SendRawCommand(uint16_t cmd_params_len, uint8_t *p_cmd_params, tUWA_RAW_CMD_CBACK *p_cback)
{
    if (cmd_params_len == 0x00 || p_cmd_params == NULL || p_cback == NULL) {
        return UCI_STATUS_INVALID_PARAM;
    }
    uint16_t size               = (uint16_t)(sizeof(tUWA_DM_API_SEND_RAW) + cmd_params_len);
    tUWA_DM_API_SEND_RAW *p_msg = (tUWA_DM_API_SEND_RAW *)phOsalUwb_GetMemory(size);

    if (p_msg != NULL) {
        p_msg->hdr.event = UWA_DM_API_SEND_RAW_EVT;
        p_msg->p_cback   = p_cback;
        if (cmd_params_len && p_cmd_params) {
            p_msg->cmd_params_len = cmd_params_len;
            p_msg->p_cmd_params   = (uint8_t *)(p_msg + 1);
            phOsalUwb_MemCopy(p_msg->p_cmd_params, p_cmd_params, cmd_params_len);
        }
        uwa_sys_sendmsg(p_msg);

        return UCI_STATUS_OK;
    }

    return UCI_STATUS_FAILED;
}