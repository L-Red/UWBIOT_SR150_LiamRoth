/*
 * Copyright 2012-2022 NXP.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "phNxpUciHal_ext.h"
#include "phNxpUciHal.h"
#include "phNxpUwbConfig.h"
#include "phTmlUwb.h"
#include "phUwbCommon.h"
#include "phUwb_BuildConfig.h"
#include "phUwbTypes.h"
#include "phNxpLogApis_HalUci.h"
#include <phTmlUwb_transport.h>
#include <uwb_board.h>

#define MAX_PROPERTY_SIZE (PROPERTY_VALUE_MAX)
/* Timeout value to wait for response from SR100 */
#define HAL_EXTNS_WRITE_RSP_TIMEOUT     (100)
#define HAL_EXTNS_WRITE_RSP_SEM_TIMEOUT (HAL_EXTNS_WRITE_RSP_TIMEOUT + 1900)
#define BOARD_VARIANT_NXPREF_V1         0x01
#define BOARD_VARIANT_NXPREF_V2         0x02
#define BOARD_VARIANT_SSG_V1            0x03
#define BOARD_VARIANT_SSG_ANCHOR_V2     0x06

#define UCI_CORE_DEVICE_INIT_CMD     0x00
#define UCI_CORE_DEVICE_INIT_CMD_LEN 0x02
#define UCI_MTS_CMD                  0x20

/******************* Global variables *****************************************/
extern phNxpUciHal_Control_t nxpucihal_ctrl;

extern uint32_t cleanup_timer;
extern uint32_t uwbTimeoutTimerId;

/************** HAL extension functions ***************************************/
static void hal_extns_write_rsp_timeout_cb(uint32_t TimerId, void *pContext);

/******************************************************************************
 * Function         phNxpUciHal_process_ext_cmd_rsp
 *
 * Description      This function process the extension command response. It
 *                  also checks the received response to expected response.
 *
 * Returns          returns UWBSTATUS_SUCCESS if response is as expected else
 *                  returns failure.
 *
 ******************************************************************************/
static UWBSTATUS phNxpUciHal_process_ext_cmd_rsp(uint16_t cmd_len, uint8_t *p_cmd)
{
    UWBSTATUS status          = UWBSTATUS_FAILED;
    uint16_t data_written     = 0;
    uint8_t ext_cmd_retry_cnt = 0;

    /* Create the local semaphore */
    if (phNxpUciHal_init_cb_data(&nxpucihal_ctrl.ext_cb_data, NULL) != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_D("Create ext_cb_data failed");
        return UWBSTATUS_FAILED;
    }

    do {
        nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_SUCCESS;
        /* Send ext command */
        data_written = phNxpUciHal_write_unlocked(cmd_len, p_cmd);
        if (data_written != cmd_len) {
            NXPLOG_UCIHAL_D("phNxpUciHal_write failed for hal ext");
            goto clean_and_return;
        }
        ext_cmd_retry_cnt++;
        /* Start timer */
        status = phOsalUwb_Timer_Start(
            uwbTimeoutTimerId, HAL_EXTNS_WRITE_RSP_TIMEOUT, &hal_extns_write_rsp_timeout_cb, NULL);
        if (UWBSTATUS_SUCCESS == status) {
            NXPLOG_UCIHAL_D("Response timer started");
        }
        else {
            NXPLOG_UCIHAL_E("Response timer not started!!!");
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }

        /* Wait for rsp */
        NXPLOG_UCIHAL_D("Waiting after ext cmd sent");
        /* timer already started with HAL_EXTNS_WRITE_RSP_TIMEOUT, wait for + 10 msec for semaphore. */
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(nxpucihal_ctrl.ext_cb_data.sem, HAL_EXTNS_WRITE_RSP_SEM_TIMEOUT) !=
            UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("p_hal_ext->ext_cb_data.sem semaphore error");
            goto clean_and_return;
        }
    } while ((nxpucihal_ctrl.ext_cb_data.status == UWBSTATUS_RESPONSE_TIMEOUT ||
                 nxpucihal_ctrl.ext_cb_data.status == UCI_STATUS_MESSAGE_RETRY) &&
             ext_cmd_retry_cnt < MAX_RETRY_COUNT);

    /* Stop Timer */
    status = phOsalUwb_Timer_Stop(uwbTimeoutTimerId);

    if (UWBSTATUS_SUCCESS == status) {
        NXPLOG_UCIHAL_D("Response timer stopped");
    }
    else {
        NXPLOG_UCIHAL_E("Response timer stop ERROR!!!");
        status = UWBSTATUS_FAILED;
        goto clean_and_return;
    }

    if (nxpucihal_ctrl.ext_cb_data.status != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E(
            "Callback Status is failed!! Timer Expired!! Couldn't read it! 0x%x", nxpucihal_ctrl.ext_cb_data.status);
        status = UWBSTATUS_FAILED;
        goto clean_and_return;
    }
    NXPLOG_UCIHAL_D("Checking response");

    status = UWBSTATUS_SUCCESS;

clean_and_return:
    phNxpUciHal_cleanup_cb_data(&nxpucihal_ctrl.ext_cb_data);

    return status;
}

/******************************************************************************
 * Function         phNxpUciHal_send_ext_cmd
 *
 * Description      This function send the extension command to UWBC. No
 *                  response is checked by this function but it waits for
 *                  the response to come.
 *
 * Returns          Returns UWBSTATUS_SUCCESS if sending cmd is successful and
 *                  response is received.
 *
 ******************************************************************************/
UWBSTATUS phNxpUciHal_send_ext_cmd(uint16_t cmd_len, uint8_t *p_cmd)
{
    UWBSTATUS status;

    HAL_ENABLE_EXT();
    nxpucihal_ctrl.cmd_len = cmd_len;
    phOsalUwb_MemCopy(nxpucihal_ctrl.p_cmd_data, p_cmd, cmd_len);
    status = phNxpUciHal_process_ext_cmd_rsp(nxpucihal_ctrl.cmd_len, nxpucihal_ctrl.p_cmd_data);
    HAL_DISABLE_EXT();

    return status;
}

/******************************************************************************
 * Function         hal_extns_write_rsp_timeout_cb
 *
 * Description      Timer call back function
 *
 * Returns          None
 *
 ******************************************************************************/
static void hal_extns_write_rsp_timeout_cb(uint32_t timerId, void *pContext)
{
    PHUWB_UNUSED(timerId);
    PHUWB_UNUSED(pContext);
    NXPLOG_UCIHAL_W("hal_extns_write_rsp_timeout_cb - write timeout!!!");
    nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_RESPONSE_TIMEOUT;
    phOsalUwb_Delay(1);
    (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.ext_cb_data.sem);

    return;
}
#if (NXP_UWB_EXTNS == TRUE)
#if UWBIOT_UWBD_SR1XXT_SR2XXT
/******************************************************************************
 * Function         phNxpUciHal_set_board_config
 *
 * Description      This function is called to set the board varaint config
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the acutual state of operation in arg pointer
 *
 ******************************************************************************/
UWBSTATUS phNxpUciHal_set_board_config()
{
    UWBSTATUS status;
    uint8_t buffer[] = {
        (UCI_MTS_CMD | UCI_GID_PROPRIETARY), UCI_CORE_DEVICE_INIT_CMD, 0x00, UCI_CORE_DEVICE_INIT_CMD_LEN, 0x01, 0x01};
    uint8_t boardConfig, boardVersion;

    if (phNxpUciHal_GetNxpNumValue(UWB_BOARD_VARIANT_CONFIG, &boardConfig, sizeof(boardConfig))) {
        NXPLOG_UCIHAL_D("%s: UWB_BOARD_VARIANT_CONFIG: %x", __FUNCTION__, boardConfig);
    }
    else
        boardConfig = BOARD_VARIANT;

#if UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
    /*
    * In Case of PNP, host PC is unaware of the target Board where pnp FW is actually running
    * This API help in fetching Board version from PNP FW
    */
    status = uwb_uwbs_tml_query_board(&gUwbsTmlCtx);
    if (status != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("uwb_uwbs_tml_query_board FAILED");
    }
    boardVersion = gUwbsTmlCtx.boardVersion;
#else
    if (phNxpUciHal_GetNxpNumValue(UWB_BOARD_VARIANT_VERSION, &boardVersion, sizeof(boardVersion))) {
        NXPLOG_UCIHAL_D("%s: UWB_BOARD_VARIANT_VERSION: %x", __FUNCTION__, boardVersion);
    }
    else
        boardVersion = UWB_BOARD_VERSION;
#endif
    buffer[4] = boardConfig;
    buffer[5] = boardVersion;

    status = phNxpUciHal_send_ext_cmd(sizeof(buffer), buffer);
    return status;
}
#endif // #if UWBIOT_UWBD_SR1XXT
#if !(UWBIOT_UWBD_SR040)
/******************************************************************************
 * Function         phNxpUciHal_dump_fw_crash_log
 *
 * Description      dump FW crash log when fw is crashed
 *
 * Returns          Returns UWBSTATUS_SUCCESS if fw crash log dumpped
 *successfully
 *
 ******************************************************************************/
UWBSTATUS phNxpUciHal_dump_fw_crash_log()
{
    UWBSTATUS status;
    uint8_t cmd_buff[] = {(UCI_MTS_CMD | UCI_GID_PROPRIETARY), UCI_DBG_GET_ERROR_LOG_CMD, 0x00, 0x00};
    uint8_t cmd_len;

    NXPLOG_UCIHAL_D("phNxpUciHal_dump_fw_crash_log: Enter");
    cmd_len = sizeof(cmd_buff);

    status = phNxpUciHal_send_ext_cmd(cmd_len, cmd_buff);

    NXPLOG_UCIHAL_D("phNxpUciHal_dump_fw_crash_log: Exit");
    return status;
}
#endif //!(UWBIOT_UWBD_SR040)

#endif //#if (NXP_UWB_EXTNS == TRUE)
