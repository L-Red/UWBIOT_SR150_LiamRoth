/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2019,2022,2023 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#include "UwbAdaptation.h"
#include "UwbCoreSDK_Internal.h"
#include "phNxpUciHal_Adaptation.h"
#include "phNxpUwbConfig.h"
#include "phOsalUwb.h"
#include "uwa_api.h"
#include "uwb_hal_int.h"
#include "uwb_int.h"
#include "uwb_logging.h"
#include "uwb_target.h"
#include "phUwb_BuildConfig.h"
#include "phNxpLogApis_UwbApi.h"

static phUwbtask_Control_t uwb_ctrl;

static void HalOpen(tHAL_UWB_CBACK *p_hal_cback, tHAL_UWB_DATA_CBACK *p_data_cback);
static void HalClose(void);
static void HalWrite(uint16_t data_len, uint8_t *p_data);
static tUCI_STATUS HalIoctl(long arg, tHAL_UWB_IOCTL *p_data);

#if (UWBIOT_UWBD_SR1XXT_SR2XXT)
static tUCI_STATUS HalApplyVendorConfigs(void);
static const tHAL_UWB_ENTRY mHalEntryFuncs = {&HalOpen, &HalClose, &HalWrite, &HalIoctl, &HalApplyVendorConfigs};
#elif (UWBIOT_UWBD_SR040)
static const tHAL_UWB_ENTRY mHalEntryFuncs = {&HalOpen, &HalClose, &HalWrite, &HalIoctl, NULL};
#endif

#define MAX_TIMEOUT_UWB_TASK_SEM (50)

uint32_t StartUwbTask()
{
    phOsalUwb_ThreadCreationParams_t threadparams;

    threadparams.stackdepth = UWBTASK_STACK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, "UWB_TASK");
    threadparams.pContext = &uwb_ctrl;
    threadparams.priority = 5;
    if (phOsalUwb_Thread_Create((void **)&uwb_ctrl.task_handle, &uwb_task, &threadparams) != 0) {
        return UCI_STATUS_FAILED;
    }

    return UCI_STATUS_OK;
}

extern phUwbtask_Control_t *gp_uwbtask_ctrl;

#define TIMER_1_EVT_MASK 0x0020

void phUwb_OSAL_send_msg(uint8_t task_id, uint16_t mbox, void *pmsg)
{
    phLibUwb_Message_t msg;
    intptr_t pMsgQ = 0;

    switch (mbox) {
    case TIMER_1_EVT_MASK:
    case UWB_TASK_EVT_TRANSPORT_READY:
    case UWB_SHUTDOWN_EVT_MASK:
        msg.eMsgType = (uint16_t)mbox;
        break;
    default:
        msg.eMsgType = (uint16_t)EVENT_MASK(mbox);
    }

    msg.pMsgData = pmsg;
    msg.Size     = 0;

    if (task_id == UWB_TASK) {
        pMsgQ = gp_uwbtask_ctrl->pMsgQHandle;
    }

    if (phOsalUwb_msgsnd(pMsgQ, &msg, NO_DELAY) != UWBSTATUS_SUCCESS) {
        // Error Message
    }
}

/*******************************************************************************
**
** Function:    Initialize()
**
**
** Returns:     none
**
*******************************************************************************/
void Initialize()
{
    uint8_t mConfig;

    uwb_ctrl.pMsgQHandle = phOsalUwb_msgget(configTML_QUEUE_LENGTH);
    (void)phOsalUwb_CreateSemaphore(&uwb_ctrl.uwb_task_sem, 0);

    if (StartUwbTask() == UCI_STATUS_OK) {
        (void)phNxpUciHal_GetNxpNumValue(UWB_FW_LOG_THREAD_ID, &mConfig, sizeof(mConfig));
    }
}

/*******************************************************************************
**
** Function:    Finalize()
**
** Returns:     none
**
*******************************************************************************/
void Finalize()
{
    phUwb_OSAL_send_msg(UWB_TASK, UWB_SHUTDOWN_EVT_MASK, NULL);
    if (UWBSTATUS_SUCCESS != phOsalUwb_ConsumeSemaphore_WithTimeout(uwb_ctrl.uwb_task_sem, MAX_TIMEOUT_UWB_TASK_SEM)) {
        LOG_E("%s : phOsalUwb_ConsumeSemaphore_WithTimeout failed", __FUNCTION__);
    }
    phOsalUwb_DeleteSemaphore(&uwb_ctrl.uwb_task_sem);
    phOsalUwb_msgrelease(uwb_ctrl.pMsgQHandle);
    (void)phOsalUwb_Thread_Delete(uwb_ctrl.task_handle);
}

/*******************************************************************************
**
** Function:    GetHalEntryFuncs()
**
** Description: Get the set of HAL entry points.
**
** Returns:     Functions pointers for HAL entry points.
**
*******************************************************************************/
const tHAL_UWB_ENTRY *GetHalEntryFuncs()
{
    return &mHalEntryFuncs;
}

#if !(UWBIOT_UWBD_SR040)
/*******************************************************************************
**
** Function:    HalApplyVendorConfigs
**
** Description: Apply the vendor configurations.
**
** Returns:     None.
**
*******************************************************************************/
static tUCI_STATUS HalApplyVendorConfigs()
{
    tUCI_STATUS status = 0;
    status             = (tUCI_STATUS)phNxpUciHal_applyVendorConfig();
    return status;
}
#endif //!(UWBIOT_UWBD_SR040)

/*******************************************************************************
**
** Function:    HalOpen
**
** Description: Turn on controller, download firmware.
**
** Returns:     None.
**
*******************************************************************************/
static void HalOpen(ATTRIBUTE_UNUSED tHAL_UWB_CBACK *p_hal_cback, ATTRIBUTE_UNUSED tHAL_UWB_DATA_CBACK *p_data_cback)
{
    (void)phNxpUciHal_open(p_hal_cback, p_data_cback);
}

/*******************************************************************************
**
** Function:    HalClose
**
** Description: Turn off controller.
**
** Returns:     None.
**
*******************************************************************************/
static void HalClose()
{
    (void)phNxpUciHal_close();
}

/*******************************************************************************
**
** Function:    HalWrite
**
** Description: Write UCI message to the controller.
**
** Returns:     None.
**
*******************************************************************************/
static void HalWrite(ATTRIBUTE_UNUSED uint16_t data_len, ATTRIBUTE_UNUSED uint8_t *p_data)
{
    (void)phNxpUciHal_write(data_len, p_data);
}

/*******************************************************************************
**
** Function:    HalRegisterAppCallback
**
** Description: registers app data call back in tml context.
**
** Returns:     None.
**
*******************************************************************************/
void HalRegisterAppCallback(phHalAppDataCb *recvDataCb)
{
    phNxpUciHal_register_appdata_callback(recvDataCb);
}

/*******************************************************************************
**
** Function:    HalIoctl
**
** Description: Calls ioctl to the Uwb driver.
**              If called with a arg value of 0x01 than wired access requested,
**              status of the requst would be updated to p_data.
**              If called with a arg value of 0x00 than wired access will be
**              released, status of the requst would be updated to p_data.
**              If called with a arg value of 0x02 than current p61 state would
*be
**              updated to p_data.
**
** Returns:     -1 or 0.
**
*******************************************************************************/
static tUCI_STATUS HalIoctl(ATTRIBUTE_UNUSED long arg, ATTRIBUTE_UNUSED tHAL_UWB_IOCTL *p_data)
{
    tUCI_STATUS status = 0;
    status             = (tUCI_STATUS)phNxpUciHal_ioctl(arg, p_data);
    return status;
}

/*******************************************************************************
**
** Function:    UwbDeviceInit
**
** Description: Download firmware patch files and apply device configs.
**
** Returns:     None.
**
*******************************************************************************/
tUCI_STATUS UwbDeviceInit(bool recovery)
{
    tUCI_STATUS status = UWBSTATUS_SUCCESS;
    if (phNxpUciHal_uwbDeviceInit(recovery) != UWBSTATUS_SUCCESS) {
        LOG_E("%s : phNxpUciHal_uwbDeviceInit failed", __FUNCTION__);
        status = UWBSTATUS_FAILED;
    }
    return status;
}

/*******************************************************************************
**
** Function:    isCmdRespPending
**
** Description: This function is get the Response status for the current command sent to fw
**
** Returns:     TRUE if response is pending, FALSE otherwise.
**
*******************************************************************************/
bool isCmdRespPending()
{
    return uwb_cb.is_resp_pending;
}

/*******************************************************************************
**
** Function:    Hal_setOperationMode
**
** Description: This function is get the Register the Operation Mode as follows
                1: MCTT
                2: CDC
                3: STANDALONE(DEFAULT)
** Returns:     None.
**
*******************************************************************************/
void Hal_setOperationMode(Uwb_operation_mode_t state)
{
    /* register the state of the user mode in Hal */
    phNxpUciHal_SetOperatingMode(state);
    // Fira Test mode is enabled
    if (state == kOPERATION_MODE_mctt) {
        /* enable the conformance test mode to avoid uci packet chaining */
        uwb_cb.IsConformaceTestEnabled = TRUE;
    }
    /*register the operating mode in uwb context to use it in Api*/
    uwb_cb.UwbOperatinMode = state;
}
