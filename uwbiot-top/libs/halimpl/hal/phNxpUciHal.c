/*
 * Copyright (C) 2019-2023 NXP Semiconductors
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

#include "phNxpUciHal.h"
#include "UwbCoreSDK_Internal.h"
#include "phNxpUciHal_Adaptation.h"
#include "phNxpUciHal_ext.h"
#include "phTmlUwb.h"
#include "phTmlUwb_transport.h"
#include "phUwbTypes.h"
#include "phNxpLogApis_HalUci.h"
#include "uci_defs.h"
#include "uwb_types.h"
#include "phOsalUwb.h"
#include "phNxpUwbConfig.h"
#include <phTmlUwb_transport.h>

#if UWBIOT_UWBD_SR1XXT_SR2XXT
#include <UwbApi_Types_Proprietary.h>
#endif

#include "phUwbStatus.h"

#define UCI_MT_DATA 0x00

#define UCI_PBF_MASK  0x10
#define UCI_PBF_SHIFT 0x04
#define UCI_GID_MASK  0x0F
#define UCI_OID_MASK  0x3F

#define UCI_GID_CORE                   0x00
#define UCI_GID_RANGE_MANAGE           0x02 /* 0010b Range Management group */
#define UCI_OID_SESSION_INFO_NTF       0x00
#define UCI_MSG_CORE_DEVICE_STATUS_NTF 0x01

#define UCI_GID_VENDOR                  0x0F /* 1111b Vendor Group */
#define UCI_GID_PROPRIETARY             0x0E /* 1110b Proprietary Group */
#define UCI_GID_INTERNAL                0x1F /* 11111b MW Internal DM group */
#define EXT_UCI_MSG_DBG_DATA_LOGGER_NTF 0x02
#define EXT_UCI_MSG_DBG_BIN_LOG_NTF     0x03
#define EXT_UCI_MSG_DBG_CIR_LOG_NTF     0x34
#define EXT_UCI_MSG_DBG_GET_ERROR_LOG   0x09

#define UCI_NTF_PAYLOAD_OFFSET          0x04
#define NORMAL_MODE_LENGTH_OFFSET       0x03
#define EXTENDED_MODE_LEN_OFFSET        0x02
#define EXTENDED_MODE_LEN_SHIFT         0x08
#define EXTND_LEN_INDICATOR_OFFSET      0x01
#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80
#define UCI_CREDIT_NTF_STATUS_OFFSET    0x08

/**  Device State - IDLE */
#define UWB_UCI_DEVICE_INIT 0x00
/**  Device State - READY */
#define UWB_UCI_DEVICE_READY 0x01
/** Device State - ERROR */
#define UWB_UCI_DEVICE_ERROR 0xFF

bool uwb_device_initialized = FALSE;

#define SCALING_FACTOR(X) 200 + X / 4

/*********************** Global Variables *************************************/
/* UCI HAL Control structure */
phNxpUciHal_Control_t nxpucihal_ctrl;

uint32_t uwbTimeoutTimerId = 0;

static uint8_t Rx_data[UCI_MAX_DATA_LEN];
static const unsigned char nxp_config_block_names[] = {UWB_NXP_CORE_CONFIG_BLOCK_1,
    UWB_NXP_CORE_CONFIG_BLOCK_2,
    UWB_NXP_CORE_CONFIG_BLOCK_3,
    UWB_NXP_CORE_CONFIG_BLOCK_4,
    UWB_NXP_CORE_CONFIG_BLOCK_5,
    UWB_NXP_CORE_CONFIG_BLOCK_6,
    UWB_NXP_CORE_CONFIG_BLOCK_7,
    UWB_NXP_CORE_CONFIG_BLOCK_8,
    UWB_NXP_CORE_CONFIG_BLOCK_9,
    UWB_NXP_CORE_CONFIG_BLOCK_10};
/**************** local methods used in this file only ************************/

static void phNxpUciHal_open_complete(UWBSTATUS status);
static void phNxpUciHal_read_complete(void *pContext, phTmlUwb_TransactInfo_t *pInfo);
static void phNxpUciHal_close_complete(UWBSTATUS status);
static OSAL_TASK_RETURN_TYPE phNxpUciHal_client_thread(void *arg);

extern int phNxpUciHal_fw_download(void);
/* For cases when HDLL Boot is done, but FW Downlaod is skipped for SR2XX */
extern int phNxpUciHal_fw_download_SKIP_SR2XX(void);
static tHAL_UWB_STATUS phNxpUciHal_uwb_reset(void);

/******************************************************************************
 * Function         phNxpUciHal_client_thread
 *
 * Description      This function is a thread handler which handles all TML and
 *                  UCI messages.
 *
 * Returns          void
 *
 ******************************************************************************/
static OSAL_TASK_RETURN_TYPE phNxpUciHal_client_thread(void *arg)
{
    phNxpUciHal_Control_t *p_nxpucihal_ctrl = (phNxpUciHal_Control_t *)arg;
    phLibUwb_Message_t msg;

    NXPLOG_UCIHAL_D("thread started");

    p_nxpucihal_ctrl->thread_running = 1;

    /* Initialize the message */
    phOsalUwb_SetMemory(&msg, 0, sizeof(msg));

    while (p_nxpucihal_ctrl->thread_running == 1) {
        /* Fetch next message from the UWB stack message queue */
        if (phOsalUwb_msgrcv(p_nxpucihal_ctrl->gDrvCfg.nClientId, &msg, MAX_DELAY) == UWBSTATUS_FAILED) {
            NXPLOG_UCIHAL_E("UWB client received bad message");
            continue;
        }

        if (p_nxpucihal_ctrl->thread_running == 0) {
            break;
        }

        switch (msg.eMsgType) {
        case PH_LIBUWB_DEFERREDCALL_MSG: {
            phLibUwb_DeferredCall_t *deferCall = (phLibUwb_DeferredCall_t *)(msg.pMsgData);

            REENTRANCE_LOCK();
            if (deferCall->pCallback != NULL) {
                deferCall->pCallback(deferCall->pParameter);
            }
            REENTRANCE_UNLOCK();

            break;
        }

        case UCI_HAL_OPEN_CPLT_MSG: {
            REENTRANCE_LOCK();
            if (nxpucihal_ctrl.p_uwb_stack_cback != NULL) {
                /* Send the event */
                (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_OPEN_CPLT_EVT, HAL_UWB_STATUS_OK);
            }
            REENTRANCE_UNLOCK();
            break;
        }

        case UCI_HAL_CLOSE_CPLT_MSG: {
            REENTRANCE_LOCK();
            p_nxpucihal_ctrl->thread_running = 0;
            if (nxpucihal_ctrl.p_uwb_stack_cback != NULL) {
                /* Send the event */
                (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_CLOSE_CPLT_EVT, HAL_UWB_STATUS_OK);
                phOsalUwb_ProduceSemaphore(p_nxpucihal_ctrl->halClientSemaphore);
            }
            REENTRANCE_UNLOCK();
            break;
        }

        case UCI_HAL_ERROR_MSG: {
            REENTRANCE_LOCK();
            if (nxpucihal_ctrl.p_uwb_stack_cback != NULL) {
                /* Send the event */
                (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_ERROR_EVT, HAL_UWB_ERROR_EVT);
            }
            REENTRANCE_UNLOCK();
            break;
        }
        }
    }

    NXPLOG_UCIHAL_D("NxpUciHal thread stopped");

    /* Suspend task here so that it does not return in FreeRTOS
     * Task will be deleted in shutdown sequence
     */
    (void)phOsalUwb_TaskSuspend(nxpucihal_ctrl.client_thread);
}

/******************************************************************************
 * Function         phNxpUciHal_open
 *
 * Description      This function is called by libuwb-uci during the
 *                  initialization of the UWBC. It opens the physical connection
 *                  with UWBC (SR100) and creates required client thread for
 *                  operation.
 *                  After open is complete, status is informed to libuwb-uci
 *                  through callback function.
 *
 * Returns          This function return UWBSTATUS_SUCCES (0) in case of success
 *                  In case of failure returns other failure value.
 *
 ******************************************************************************/
int phNxpUciHal_open(uwb_stack_callback_t *p_cback, uwb_stack_data_callback_t *p_data_cback)
{
    phOsalUwb_Config_t tOsalConfig;
    phTmlUwb_Config_t tTmlConfig;
    UWBSTATUS wConfigStatus = UWBSTATUS_SUCCESS;
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBSTATUS status = UWBSTATUS_SUCCESS;

    if (nxpucihal_ctrl.halStatus == HAL_STATUS_OPEN) {
        NXPLOG_UCIHAL_E("phNxpUciHal_open already open");
        return UWBSTATUS_SUCCESS;
    }

    /*Create the timer for extns write response*/
    uwbTimeoutTimerId = phOsalUwb_Timer_Create(FALSE);

    if (phNxpUciHal_init_monitor() == NULL) {
        NXPLOG_UCIHAL_E("Init monitor failed");
        return UWBSTATUS_FAILED;
    }

    CONCURRENCY_LOCK();

    phOsalUwb_SetMemory(&nxpucihal_ctrl, 0x00, sizeof(nxpucihal_ctrl));
    phOsalUwb_SetMemory(&tOsalConfig, 0x00, sizeof(tOsalConfig));
    phOsalUwb_SetMemory(&tTmlConfig, 0x00, sizeof(tTmlConfig));

    status = phOsalUwb_CreateSemaphore(&nxpucihal_ctrl.halClientSemaphore, 0);
    if (status != UWBSTATUS_SUCCESS) {
        (void)CONCURRENCY_UNLOCK();
        phNxpUciHal_cleanup_monitor();
        (void)phOsalUwb_Timer_Delete(uwbTimeoutTimerId);
        uwbTimeoutTimerId = PH_OSALUWB_TIMER_ID_INVALID;
        return status;
    }
    /* By default HAL status is HAL_STATUS_OPEN */
    nxpucihal_ctrl.halStatus = HAL_STATUS_OPEN;

    nxpucihal_ctrl.p_uwb_stack_cback      = p_cback;
    nxpucihal_ctrl.p_uwb_stack_data_cback = p_data_cback;

    nxpucihal_ctrl.IsDev_suspend_enabled  = FALSE;
    nxpucihal_ctrl.IsFwDebugDump_enabled  = FALSE;
    nxpucihal_ctrl.IsCIRDebugDump_enabled = FALSE;
    nxpucihal_ctrl.fw_dwnld_mode          = FALSE;

    /* Configure hardware link */
    nxpucihal_ctrl.gDrvCfg.nClientId = phOsalUwb_msgget(configTML_QUEUE_LENGTH);
    nxpucihal_ctrl.gDrvCfg.nLinkType = ENUM_LINK_TYPE_SPI; /* For SR100 */
    tOsalConfig.dwCallbackThreadId   = (uintptr_t)nxpucihal_ctrl.gDrvCfg.nClientId;
    tOsalConfig.pLogFile             = NULL;
    tTmlConfig.dwGetMsgThreadId      = (uintptr_t)nxpucihal_ctrl.gDrvCfg.nClientId;

    /* Initialize TML layer */
    wConfigStatus = phTmlUwb_Init(&tTmlConfig);
    if (wConfigStatus != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("phTmlUwb_Init Failed");
        goto clean_and_return;
    }
    threadparams.stackdepth = CLIENT_STACK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, "CLIENT");
    threadparams.pContext = &nxpucihal_ctrl;
    threadparams.priority = CLIENT_PRIO;
    if (phOsalUwb_Thread_Create((void **)&nxpucihal_ctrl.client_thread, &phNxpUciHal_client_thread, &threadparams) !=
        UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("\n\r ---client_thread Task create failed \n");
        goto clean_and_return;
    }

    CONCURRENCY_UNLOCK();
    /* Call open complete */
    phNxpUciHal_open_complete(wConfigStatus);
    return wConfigStatus;

clean_and_return:
    CONCURRENCY_UNLOCK();
    if (PH_OSALUWB_TIMER_ID_INVALID != uwbTimeoutTimerId) {
        phOsalUwb_Timer_Delete(uwbTimeoutTimerId);
    }
    (void)phOsalUwb_DeleteSemaphore(&nxpucihal_ctrl.halClientSemaphore);
    (void)phOsalUwb_msgrelease(nxpucihal_ctrl.gDrvCfg.nClientId);
    /* Report error status */
    (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_OPEN_CPLT_EVT, HAL_UWB_ERROR_EVT);

    nxpucihal_ctrl.p_uwb_stack_cback      = NULL;
    nxpucihal_ctrl.p_uwb_stack_data_cback = NULL;
    phNxpUciHal_cleanup_monitor();
    nxpucihal_ctrl.halStatus = HAL_STATUS_CLOSE;
    return UWBSTATUS_FAILED;
}

/******************************************************************************
 * Function         phNxpUciHal_open_complete
 *
 * Description      This function inform the status of phNxpUciHal_open
 *                  function to libuwb-uci.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_open_complete(UWBSTATUS status)
{
    static phLibUwb_Message_t msg;

    if (status == UWBSTATUS_SUCCESS) {
        msg.eMsgType                   = UCI_HAL_OPEN_CPLT_MSG;
        nxpucihal_ctrl.hal_open_status = TRUE;
        nxpucihal_ctrl.halStatus       = HAL_STATUS_OPEN;
    }
    else {
        msg.eMsgType = UCI_HAL_ERROR_MSG;
    }

    msg.pMsgData = NULL;
    msg.Size     = 0;

    phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, (phLibUwb_Message_t *)&msg);

    return;
}

/******************************************************************************
 * Function         phNxpUciHal_register_data_callback
 *
 * Description      This function register data packet callback
 *
 * Returns          void
 *
 ******************************************************************************/
void phNxpUciHal_register_appdata_callback(phHalAppDataCb *appDataCb)
{
    if (NULL != appDataCb) {
        gpphTmlUwb_Context->appDataCallback = (pphTmlUwb_AppDataCb_t)appDataCb;
    }
}

/******************************************************************************
 * Function         phNxpUciHal_write
 *
 * Description      This function write the data to UWBC through physical
 *                  interface (e.g. SPI) using the SR100 driver interface.
 *
 * Returns          It returns number of bytes successfully written to UWBC.
 *
 ******************************************************************************/
int phNxpUciHal_write(uint16_t data_len, const uint8_t *p_data)
{
    uint16_t len;

    if (nxpucihal_ctrl.halStatus != HAL_STATUS_OPEN) {
        return UWBSTATUS_FAILED;
    }

    CONCURRENCY_LOCK();
    len = phNxpUciHal_write_unlocked(data_len, p_data);
    CONCURRENCY_UNLOCK();

    /* No data written */
    return len;
}

/******************************************************************************
 * Function         phNxpUciHal_write_unlocked
 *
 * Description      This is the actual function which is being called by
 *                  phNxpUciHal_write. This function writes the data to UWBC.
 *                  It waits till write callback provide the result of write
 *                  process.
 *
 * Returns          It returns number of bytes successfully written to UWBC.
 *
 ******************************************************************************/
uint16_t phNxpUciHal_write_unlocked(uint16_t data_len, const uint8_t *p_data)
{
    UWBSTATUS status;

    /* Create local copy of cmd_data */
    if (data_len <= UCI_MAX_CMD_BUF_LEN) {
        phOsalUwb_MemCopy(nxpucihal_ctrl.p_cmd_data, p_data, data_len);
        nxpucihal_ctrl.cmd_len = data_len;

        data_len = nxpucihal_ctrl.cmd_len;
        status   = phTmlUwb_Write((uint8_t *)nxpucihal_ctrl.p_cmd_data, (uint16_t)nxpucihal_ctrl.cmd_len);
        if (UWBSTATUS_SUCCESS == status) {
            NXPLOG_UCIHAL_D("%s phTmlUwb_Write success", __FUNCTION__);
        }
        else {
            NXPLOG_UCIHAL_W("%s phTmlUwb_Write Failed", __FUNCTION__);
            data_len = 0;
        }
    }
    else {
        NXPLOG_UCIHAL_E("write_unlocked buffer overflow");
        data_len = 0;
    }

    return data_len;
}

#if (NXP_UWB_EXTNS == TRUE)
/******************************************************************************
 * Function         phNxpUciHal_dump_log
 *
 * Description      This function is called whenever there is an debug logs
 *                  needs to be collected
 *
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_dump_log(uint8_t gid, uint8_t oid, uint8_t pbf)
{
    if ((gid == UCI_GID_PROPRIETARY) && (oid == EXT_UCI_MSG_DBG_BIN_LOG_NTF)) {
        NXPLOG_UCIHAL_D("debug bin ntf samples received");
        nxpucihal_ctrl.isSkipPacket = 1;
    }
    else if ((gid == UCI_GID_PROPRIETARY) && (oid == EXT_UCI_MSG_DBG_DATA_LOGGER_NTF)) {
        NXPLOG_UCIHAL_D("debug data logger ntf samples received");
        nxpucihal_ctrl.isSkipPacket = 0;
    }
    else if ((gid == UCI_GID_VENDOR) && (oid == EXT_UCI_MSG_DBG_CIR_LOG_NTF)) {
        NXPLOG_UCIHAL_D("CIR samples received");
        nxpucihal_ctrl.isSkipPacket = 0;
    }
    else if ((gid == UCI_GID_PROPRIETARY) && (oid == EXT_UCI_MSG_DBG_GET_ERROR_LOG)) {
        NXPLOG_UCIHAL_D(" error log received. ntf received");
        nxpucihal_ctrl.isSkipPacket = 1;
    }
    else {
        if ((nxpucihal_ctrl.IsCIRDebugDump_enabled) && (gid == UCI_GID_RANGE_MANAGE) &&
            (oid == UCI_OID_SESSION_INFO_NTF)) {
            NXPLOG_UCIHAL_D(" session info ntf received");
        }
    }
    if (pbf) {
        NXPLOG_UCIHAL_D("TODO: phNxpUciHal_dump_log pbf filed is not handled");
    }
    return;
}
#endif
/******************************************************************************
 * Function         phNxpUciHal_read_complete
 *
 * Description      This function is called whenever there is an UCI packet
 *                  received from UWBC. It could be RSP or NTF packet. This
 *                  function provide the received UCI packet to libuwb-uci
 *                  using data callback of libuwb-uci.
 *                  There is a pending read called from each
 *                  phNxpUciHal_read_complete so each a packet received from
 *                  UWBC can be provide to libuwb-uci.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_read_complete(void *pContext, phTmlUwb_TransactInfo_t *pInfo)
{
    UWBSTATUS status;
    uint8_t gid = 0, oid = 0, pbf, mt = 0;
    PHUWB_UNUSED(pContext);
    if (nxpucihal_ctrl.read_retry_cnt == 1) {
        nxpucihal_ctrl.read_retry_cnt = 0;
    }
    if (pInfo->wStatus == UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_D("read successful status = 0x%x", pInfo->wStatus);
        nxpucihal_ctrl.p_rx_data   = pInfo->pBuff;
        nxpucihal_ctrl.rx_data_len = pInfo->wLength;

        mt  = nxpucihal_ctrl.p_rx_data[0] & UCI_MT_MASK;
        gid = nxpucihal_ctrl.p_rx_data[0] & UCI_GID_MASK;
        oid = nxpucihal_ctrl.p_rx_data[1] & UCI_OID_MASK;
        pbf = (nxpucihal_ctrl.p_rx_data[0] & UCI_PBF_MASK) >> UCI_PBF_SHIFT;

#if (NXP_UWB_EXTNS == TRUE)
        nxpucihal_ctrl.isSkipPacket = 0;
        if (mt == UCI_MTS_NTF) {
            phNxpUciHal_dump_log(gid, oid, pbf);
        }

#endif

        if ((gid == UCI_GID_CORE) && (oid == UCI_MSG_CORE_DEVICE_STATUS_NTF)) {
            nxpucihal_ctrl.uwb_dev_status = nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET];
            if (!uwb_device_initialized) {
                if (nxpucihal_ctrl.uwb_dev_status == UWB_UCI_DEVICE_INIT ||
                    nxpucihal_ctrl.uwb_dev_status == UWB_UCI_DEVICE_READY) {
                    nxpucihal_ctrl.isSkipPacket = 1;
                    (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.dev_status_ntf_wait.sem);
                }
            }
        }

        if (gid == UCI_GID_CORE && oid == UCI_MSG_CORE_GENERIC_ERROR_NTF &&
            nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET] == UCI_STATUS_MESSAGE_RETRY) {
            nxpucihal_ctrl.ext_cb_data.status = UCI_STATUS_MESSAGE_RETRY;
            nxpucihal_ctrl.isSkipPacket       = 1;
            (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.ext_cb_data.sem);
        }

        if ((gid == UCI_GID_RANGE_MANAGE) && (oid == UCI_MSG_DATA_CREDIT_NTF)) {
            nxpucihal_ctrl.ext_cb_data.status = nxpucihal_ctrl.p_rx_data[UCI_CREDIT_NTF_STATUS_OFFSET];
            (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.ext_cb_data.sem);
        }

        if (nxpucihal_ctrl.hal_ext_enabled == 1) {
            nxpucihal_ctrl.isSkipPacket = 1;
            if (mt == UCI_MT_RSP << UCI_MT_SHIFT) {
                if (nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET] == UWBSTATUS_SUCCESS) {
                    nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_SUCCESS;
                }
                else if ((gid == UCI_GID_PROPRIETARY) && (oid == UCI_DBG_GET_ERROR_LOG_CMD)) {
                    nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_SUCCESS;
                }
                else {
                    nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_FAILED;
                    NXPLOG_UCIHAL_E(
                        "Response failed status = 0x%x", nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET]);
                }
                (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.ext_cb_data.sem);
            }
        }
        /* if Debug Notification, then skip sending to application */
        if (nxpucihal_ctrl.isSkipPacket == 0) {
            /* Read successful, send the event to higher layer */
            if ((nxpucihal_ctrl.p_uwb_stack_data_cback != NULL)
#if UWBIOT_UWBD_SR040
                && (nxpucihal_ctrl.rx_data_len <= UCI_MAX_PACKET_LEN)
#endif
            ) {
                (*nxpucihal_ctrl.p_uwb_stack_data_cback)(nxpucihal_ctrl.rx_data_len, nxpucihal_ctrl.p_rx_data);
            }
        }
    }
    else {
        NXPLOG_UCIHAL_E("read error status = 0x%x", pInfo->wStatus);
    }

    if (nxpucihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
        return;
    }
    /* Disable junk data check for each UCI packet*/
    if (nxpucihal_ctrl.fw_dwnld_mode) {
        if ((gid == UCI_GID_CORE) && (oid == UCI_MSG_CORE_DEVICE_STATUS_NTF)) {
            nxpucihal_ctrl.fw_dwnld_mode = FALSE;
        }
    }
    /* Read again because read must be pending always.*/
    status =
        phTmlUwb_Read(Rx_data, UCI_MAX_DATA_LEN, (pphTmlUwb_TransactCompletionCb_t)&phNxpUciHal_read_complete, NULL);
    if (status != UWBSTATUS_PENDING) {
        NXPLOG_UCIHAL_E("read status error status = %x", status);
        /* TODO: Not sure how to handle this ? */
    }

    return;
}

/******************************************************************************
 * Function         phNxpUciHal_close
 *
 * Description      This function close the UWBC interface and free all
 *                  resources.This is called by libuwb-uci on UWB service stop.
 *
 * Returns          Always return UWBSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpUciHal_close()
{
    UWBSTATUS status = UWBSTATUS_FAILED;

    if (nxpucihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
        NXPLOG_UCIHAL_E("phNxpUciHal_close is already closed, ignoring close");
        return UWBSTATUS_FAILED;
    }

    nxpucihal_ctrl.IsFwDebugDump_enabled  = FALSE;
    nxpucihal_ctrl.IsCIRDebugDump_enabled = FALSE;

    CONCURRENCY_LOCK();

    nxpucihal_ctrl.halStatus = HAL_STATUS_CLOSE;

    if (NULL != gpphTmlUwb_Context->pDevHandle) {
        phNxpUciHal_close_complete(UWBSTATUS_SUCCESS);
        // wait till message NCI_HAL_CLOSE_CPLT_MSG is posted to HAL Client Task
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(nxpucihal_ctrl.halClientSemaphore, HAL_MAX_CLOSE_EVENT_TIMOUT) !=
            UWBSTATUS_SUCCESS) {
            LOG_E("%s : Waiting for close event failed", __FUNCTION__);
            status = UWBSTATUS_FAILED;
        }
        /* Abort any pending read and write */
        phTmlUwb_ReadAbort();

        if (uwbTimeoutTimerId != 0) {
            (void)phOsalUwb_Timer_Stop(uwbTimeoutTimerId);
            (void)phOsalUwb_Timer_Delete(uwbTimeoutTimerId);
            uwbTimeoutTimerId = 0;
        }
        status = phTmlUwb_Shutdown();

        phOsalUwb_msgrelease(nxpucihal_ctrl.gDrvCfg.nClientId);
        phOsalUwb_Thread_Delete(nxpucihal_ctrl.client_thread);
        (void)phOsalUwb_DeleteSemaphore(&nxpucihal_ctrl.halClientSemaphore);
        phOsalUwb_SetMemory(&nxpucihal_ctrl, 0x00, sizeof(nxpucihal_ctrl));

        NXPLOG_UCIHAL_D("phNxpUciHal_close - phOsalUwb_DeInit completed");
    }

    CONCURRENCY_UNLOCK();

    phNxpUciHal_cleanup_monitor();

    /* Return success always */
    return status;
}
/******************************************************************************
 * Function         phNxpUciHal_close_complete
 *
 * Description      This function inform libuwb-uci about result of
 *                  phNxpUciHal_close.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_close_complete(UWBSTATUS status)
{
    static phLibUwb_Message_t msg;

    if (status == UWBSTATUS_SUCCESS) {
        msg.eMsgType = UCI_HAL_CLOSE_CPLT_MSG;
    }
    else {
        msg.eMsgType = UCI_HAL_ERROR_MSG;
    }
    msg.pMsgData = NULL;
    msg.Size     = 0;

    phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, &msg);

    return;
}

/******************************************************************************
 * Function         phNxpUciHal_ioctl
 *
 * Description      This function is called by jni when wired mode is
 *                  performed.First SR100 driver will give the access
 *                  permission whether wired mode is allowed or not
 *                  arg (0):
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the acutual state of operation in arg pointer
 *
 ******************************************************************************/
int phNxpUciHal_ioctl(long arg, tHAL_UWB_IOCTL *p_data)
{
    NXPLOG_UCIHAL_D("%s : enter - arg = %ld", __FUNCTION__, arg);

    int status = UWBSTATUS_FAILED;
    switch (arg) {
    case HAL_UWB_IOCTL_DUMP_FW_CRASH_LOG: {
#if !(UWBIOT_UWBD_SR040)
        status = phNxpUciHal_dump_fw_crash_log();
        if (status == UWBSTATUS_SUCCESS) {
            if (p_data == NULL) {
                NXPLOG_UCIHAL_E("%s : p_data is NULL", __FUNCTION__);
                return UWBSTATUS_INVALID_PARAMETER;
            }
            phFwCrashLogInfo_t *fwLogInfo = (phFwCrashLogInfo_t *)p_data->pCrashInfo;
            if (fwLogInfo->logLen >= (size_t)(nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_LEN_OFFSET])) {
                fwLogInfo->logLen = nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_LEN_OFFSET];
                phOsalUwb_MemCopy(fwLogInfo->pLog,
                    &nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET],
                    (uint32_t)fwLogInfo->logLen);
                return UWBSTATUS_SUCCESS;
            }
            else {
                fwLogInfo->logLen = (size_t)nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_LEN_OFFSET] - 1;
                NXPLOG_UCIHAL_E("%s : Not Enough buffer to copy FW crash log required buffer size is %d",
                    __FUNCTION__,
                    fwLogInfo->logLen);
                return UWBSTATUS_INVALID_PARAMETER;
            }
        }
        NXPLOG_UCIHAL_E("%s : phNxpUciHal_dump_fw_crash_log failed", __FUNCTION__);
#endif //!(UWBIOT_UWBD_SR040)
    } break;

    case HAL_UWB_IOCTL_SET_SUSPEND_STATE:
        nxpucihal_ctrl.IsDev_suspend_enabled = TRUE;
        break;
#if (NXP_UWB_EXTNS == TRUE)
#if !(UWBIOT_UWBD_SR040)
    case HAL_UWB_IOCTL_DUMP_FW_LOG: {
        if (p_data == NULL) {
            NXPLOG_UCIHAL_E("%s : p_data is NULL", __FUNCTION__);
            return UWBSTATUS_INVALID_PARAMETER;
        }
        InputOutputData_t *ioData            = (InputOutputData_t *)(p_data)->pIoData;
        nxpucihal_ctrl.IsFwDebugDump_enabled = ioData->enableFwDump;
        NXPLOG_UCIHAL_I("%s : Fw Dump is enabled status is %d", __FUNCTION__, ioData->enableFwDump);
        nxpucihal_ctrl.IsCIRDebugDump_enabled = ioData->enableCirDump;
        NXPLOG_UCIHAL_I("%s : Cir Dump is enabled status is %d", __FUNCTION__, ioData->enableCirDump);
        status = UWBSTATUS_SUCCESS;
    } break;
#endif //!(UWBIOT_UWBD_SR040)
#endif //(NXP_UWB_EXTNS == TRUE)
    default:
        NXPLOG_UCIHAL_E("%s : Wrong arg = %ld", __FUNCTION__, arg);
        break;
    }
    return status;
}

/******************************************************************************
 * Function         phNxpUciHal_applyVendorConfig
 *
 * Description      This function is called during init process to
 *                  apply vendor configs from the config file
 *
 * Returns          return 0 on success and status value on non-zero status value
 *
 ******************************************************************************/
int phNxpUciHal_applyVendorConfig()
{
    NXPLOG_UCIHAL_D("phNxpUciHal_applyVendorConfig Enter");
    void *p_cmd   = NULL;
    long cmd_len  = 0;
    int status    = UWBSTATUS_SUCCESS;
    uint8_t count = 0;
    if (phNxpUciHal_GetNxpNumValue(UWB_NXP_CORE_CONFIG_BLOCK_COUNT, &count, sizeof(count))) {
        NXPLOG_UCIHAL_D("phNxpUciHal_applyVendorConfig :: Value of count in %s is %x", __FUNCTION__, count);
    }
    for (int i = 0; i < count; i++) {
        if ((phNxpUciHal_GetNxpByteArrayValue(nxp_config_block_names[i], &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
            status = phNxpUciHal_send_ext_cmd((uint16_t)cmd_len, (uint8_t *)p_cmd);
            if (status != UWBSTATUS_SUCCESS) {
                break;
            }
        }
        else {
            NXPLOG_UCIHAL_D("phNxpUciHal_applyVendorConfig:: cmd_len is %ld", cmd_len);
        }
    }
    return status;
}

/******************************************************************************
 * Function         phNxpUciHal_uwbDeviceInit
 *
 * Description      This function is called to initialize UWB device. It performs
 *                  firmware download and set device configuration
 * Returns          return status
 *
 ******************************************************************************/
int phNxpUciHal_uwbDeviceInit(BOOLEAN recovery)
{
    int status;
    NXPLOG_UCIHAL_D(" Start FW download");
    nxpucihal_ctrl.fw_dwnld_mode  = TRUE; /* system in FW download mode*/
    nxpucihal_ctrl.uwb_dev_status = UWB_UCI_DEVICE_ERROR;
    uwb_device_initialized        = FALSE;
    /* Initiate semaphore */
    if (phOsalUwb_CreateSemaphore(&nxpucihal_ctrl.dev_status_ntf_wait.sem, 0) != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("Semaphore creation failed");
        return UWBSTATUS_FAILED;
    }
    if (recovery == TRUE) {
        (void)phTmlUwb_reset(0);
        phTmlUwb_suspendReader();
#if (UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
        (void)phTmlUwb_pnp_hardreset();
#endif // (UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
    }

#if (UWBIOT_UWBD_SR040) && UWBIOT_TML_SPI
    status = UWBSTATUS_SUCCESS;
    phTmlUwb_io_set(kUWBS_IO_O_RSTN, 0);
    phOsalUwb_Delay(10);
    phTmlUwb_io_set(kUWBS_IO_O_RSTN, 1);
#endif // (UWBIOT_UWBD_SR040) && UWBIOT_TML_SPI

#if UWBIOT_UWBD_SR2XXT
#if UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT
    status = phNxpUciHal_fw_download();
#else
    status = phNxpUciHal_fw_download_SKIP_SR2XX();
#endif
#endif // UWBIOT_UWBD_SR2XXT

#if UWBIOT_UWBD_SR1XXT
    LOG_I("Starting FW download");
    status = phNxpUciHal_fw_download();
#if UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
    if (status != UWBSTATUS_SUCCESS) {
        /* Retry, just once more...
             * This failure is seen in PNP PC Windows mode, where if there was no clean
             * shut down, above call seems to fail, so sending again.
             */
        status = phNxpUciHal_fw_download();
    }
#endif //UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
    if (status == UWBSTATUS_SUCCESS)
#endif //UWBIOT_UWBD_SR1XXT
    {
#if UWBIOT_UWBD_SR1XXT
        LOG_I("FW Download done.");
#endif //UWBIOT_UWBD_SR1XXT

        if (recovery == TRUE) {
            phTmlUwb_resumeReader();
        }

        status = phTmlUwb_Read(
            Rx_data, UCI_MAX_DATA_LEN, (pphTmlUwb_TransactCompletionCb_t)&phNxpUciHal_read_complete, NULL);
        if (status != UWBSTATUS_PENDING) {
            NXPLOG_UCIHAL_E("read status error status = %x", status);
        }
        else {
            status = UWBSTATUS_SUCCESS; // Reader thread started successfully
        }
    }
#if UWBIOT_UWBD_SR1XXT
    else {
        NXPLOG_UCIHAL_E("FW download is failed: status= %x", status);
        status = UWBSTATUS_FAILED;
        goto clean_and_return;
    }
#endif //UWBIOT_UWBD_SR1XXT
    if (status == UWBSTATUS_SUCCESS) {
        //Wait for device init ntf
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(
                nxpucihal_ctrl.dev_status_ntf_wait.sem, HAL_MAX_DEVICE_ST_NTF_TIMEOUT) != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("%s: Sem Timed out", __FUNCTION__);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#if UWBIOT_UWBD_SR1XXT
        if (nxpucihal_ctrl.uwb_dev_status != UWB_UCI_DEVICE_INIT) {
            NXPLOG_UCIHAL_E("%s: device status is failed %d", __FUNCTION__, nxpucihal_ctrl.uwb_dev_status);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#else
        if (nxpucihal_ctrl.uwb_dev_status != UWB_UCI_DEVICE_READY) {
            NXPLOG_UCIHAL_E("%s: device status is failed %d", __FUNCTION__, nxpucihal_ctrl.uwb_dev_status);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#endif /* UWBIOT_UWBD_SR1XXT */

#if UWBIOT_UWBD_SR1XXT_SR2XXT
        /* set board variant */
        status = phNxpUciHal_set_board_config();
        if (status != UWBSTATUS_OK) {
            NXPLOG_UCIHAL_E("%s: set board config is failed with status %d", __FUNCTION__, status);
            goto clean_and_return;
        }

        // wait for dev ready ntf
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(
                nxpucihal_ctrl.dev_status_ntf_wait.sem, HAL_MAX_DEVICE_ST_NTF_TIMEOUT) != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("%s: Sem Timed out", __FUNCTION__);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
        if (nxpucihal_ctrl.uwb_dev_status != UWB_UCI_DEVICE_READY) {
            NXPLOG_UCIHAL_E("%s: device status is failed %d", __FUNCTION__, nxpucihal_ctrl.uwb_dev_status);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#endif
        //reset device
        status = phNxpUciHal_uwb_reset();
        if (status != UWBSTATUS_OK) {
            NXPLOG_UCIHAL_E("%s: Device reset Failed", __FUNCTION__);
            goto clean_and_return;
        }

        //wait for dev ready ntf
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(
                nxpucihal_ctrl.dev_status_ntf_wait.sem, HAL_MAX_DEVICE_ST_NTF_TIMEOUT) != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("%s: Sem Timed out", __FUNCTION__);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
        if (nxpucihal_ctrl.uwb_dev_status != UWB_UCI_DEVICE_READY) {
            NXPLOG_UCIHAL_E("%s: device status is failed %d", __FUNCTION__, nxpucihal_ctrl.uwb_dev_status);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        // TODO: status to be updated once vendor configs are available
        //Apply vendor config
        if (phNxpUciHal_applyVendorConfig() != UWBSTATUS_OK) {
            NXPLOG_UCIHAL_E("%s: Apply Vendor config Failed", __FUNCTION__);
        }
#endif // UWBIOT_UWBD_SR1XXT
        uwb_device_initialized = TRUE;
    }

clean_and_return:
    phOsalUwb_DeleteSemaphore(&nxpucihal_ctrl.dev_status_ntf_wait.sem);

    return status;
}
/******************************************************************************
 * Function         phNxpUciHal_uwb_reset
 *
 * Description      This function is called to reset uwb device
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the acutual state of operation in arg pointer
 *
 ******************************************************************************/
static tHAL_UWB_STATUS phNxpUciHal_uwb_reset()
{
    tHAL_UWB_STATUS status;
    uint8_t buffer[] = {0x20, 0x00, 0x00, 0x01, 0x00};
    status           = (tHAL_UWB_STATUS)phNxpUciHal_send_ext_cmd(sizeof(buffer), buffer);
    if (status != UWBSTATUS_SUCCESS) {
        return status;
    }
    return UWBSTATUS_SUCCESS;
}

/******************************************************************************
 * Function         phNxpUciHal_SetOperatingMode
 *
 * Description      Register the UWB Operating Mode
 *
 * Returns          None
 *
 ******************************************************************************/
void phNxpUciHal_SetOperatingMode(Uwb_operation_mode_t state)
{
    nxpucihal_ctrl.operationMode = state;
}
