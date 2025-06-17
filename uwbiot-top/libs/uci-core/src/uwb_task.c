/*
 *
 * Copyright 2018-2023 NXP.
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
 *  Entry point for UWB_TASK
 *
 ******************************************************************************/
#include "UwbAdaptation.h"
#include "uwb_target.h"
#include "uwb_hal_api.h"
#include "uwb_int.h"
#include "uci_hmsgs.h"

#include "uwa_sys.h"
#include "uwa_dm_int.h"
#include "phNxpLogApis_UciCore.h"

phUwbtask_Control_t *gp_uwbtask_ctrl;

static uint32_t TimerHandle;

/*******************************************************************************
**
** Function         uwb_start_quick_timer
**
** Description      Start a timer for the specified amount of time.
**                  NOTE: The timeout resolution depends on including modules.
**                  QUICK_TIMER_TICKS_PER_SEC should be used to convert from
**                  time to ticks.
**
**
** Returns          void
**
*******************************************************************************/

static void quick_timer_callback(uint32_t timerid, void *pContext)
{
    static uint16_t quickEvent = UWB_TTYPE_UCI_WAIT_RSP;
    (void)phOsalUwb_Timer_Stop(timerid);
    phUwb_OSAL_send_msg(UWB_TASK, TIMER_1_EVT_MASK, &quickEvent);
    (void)phOsalUwb_Timer_Delete(timerid);
}

void uwb_start_quick_timer(uint32_t timeout)
{
    UCI_TRACE_D("uwb_start_quick_timer enter: timeout: %d", timeout);
    // [TODO] Timer creation to be optimized
    TimerHandle = phOsalUwb_Timer_Create(FALSE);

    if (TimerHandle != PH_OSALUWB_TIMER_ID_INVALID) {
        if (phOsalUwb_Timer_Start(TimerHandle,
                (uint32_t)((GKI_SECS_TO_TICKS(timeout) / QUICK_TIMER_TICKS_PER_SEC)),
                quick_timer_callback,
                NULL) != 0) {
            // BT_ERROR_TRACE_0(TRACE_LAYER_GKI, "Timer Start failed");
        }
    }
    else {
        UCI_TRACE_E("%s : Invalid Timer ID", __FUNCTION__);
    }
}

/*******************************************************************************
**
** Function         uwb_stop_quick_timer
**
** Description      Stop a timer.
**
** Returns          void
**
*******************************************************************************/
void uwb_stop_quick_timer()
{
    UCI_TRACE_D("uwb_stop_quick_timer: enter");

    (void)phOsalUwb_Timer_Stop(TimerHandle);
    (void)phOsalUwb_Timer_Delete(TimerHandle);
}

/*******************************************************************************
**
** Function         uwb_process_quick_timer_evt
**
** Description      Process quick timer event
**
** Returns          void
**
*******************************************************************************/
void uwb_process_quick_timer_evt(uint16_t event)
{
    switch (event) {
    case UWB_TTYPE_UCI_WAIT_RSP:
        uwb_ucif_cmd_timeout();
        break;

    default:
        UCI_TRACE_D("uwb_process_quick_timer_evt: event (0x%04x)", event);
    }
}

/*******************************************************************************
**
** Function         uwb_task_shutdown_uwbc
**
** Description      Handle UWB shutdown
**
** Returns          nothing
**
*******************************************************************************/
void uwb_task_shutdown_uwbc(void)
{
    uwb_gen_cleanup();

    uwb_set_state(UWB_STATE_W4_HAL_CLOSE);
    uwb_cb.p_hal->close();

    /* Stop the timers */
    uwb_stop_quick_timer();
}

#define UWB_TASK_ARGS void *args

#if UWBIOT_OS_FREERTOS
#define RET_VALUE return
#else
#define RET_VALUE return 0
#endif

/*******************************************************************************
**
** Function         uwb_task
**
** Description      UWB event processing task
**
** Returns          nothing
**
*******************************************************************************/
OSAL_TASK_RETURN_TYPE uwb_task(UWB_TASK_ARGS)
{
    uint32_t event;
    bool free_buf  = FALSE;
    UWB_HDR *p_msg = NULL;
    phLibUwb_Message_t msg;
    gp_uwbtask_ctrl    = (phUwbtask_Control_t *)args;
    tUCI_STATUS status = UCI_STATUS_FAILED;

    /* Initialize the uwb control block */
    phOsalUwb_SetMemory(&uwb_cb, 0, sizeof(tUWB_CB));

    /* Initialize the message */
    phOsalUwb_SetMemory(&msg, 0, sizeof(msg));

    UCI_TRACE_D("UWB_TASK started.");

    /* main loop */
    while (1) {
        if (phOsalUwb_msgrcv(gp_uwbtask_ctrl->pMsgQHandle, &msg, MAX_DELAY) == UWBSTATUS_FAILED) {
            continue;
        }
        event = msg.eMsgType;
        /* Handle UWB_TASK_EVT_TRANSPORT_READY from UWB HAL */
        if (event & UWB_TASK_EVT_TRANSPORT_READY) {
            UCI_TRACE_D("UWB_TASK got UWB_TASK_EVT_TRANSPORT_READY.");

            /* Reset the UWB controller. */
            uwb_set_state(UWB_STATE_IDLE);
            status = UCI_STATUS_OK;
            (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_ENABLE, sizeof(tUCI_STATUS), &status);
        }

        if (event & UWB_SHUTDOWN_EVT_MASK) {
            break;
        }

        if (event & UWB_MBOX_EVT_MASK) {
            /* Process all incoming UCI messages */
            p_msg    = (UWB_HDR *)msg.pMsgData;
            free_buf = TRUE;

            /* Determine the input message type. */
            if (p_msg != NULL) {
                switch (p_msg->event & UWB_EVT_MASK) {
                case BT_EVT_TO_UWB_UCI:
                    free_buf = uwb_ucif_process_event(p_msg);
                    break;

                case BT_EVT_TO_UWB_MSGS:
                    uwb_main_handle_hal_evt((tUWB_HAL_EVT_MSG *)p_msg);
                    break;

                default:
                    UCI_TRACE_E("uwb_task: unhandle mbox message, event=%04x", p_msg->event);
                    break;
                }
                if (free_buf) {
                    phOsalUwb_FreeMemory(p_msg);
                }
            }
        }

        /* Process quick timer tick */
        if (event & UWB_QUICK_TIMER_EVT_MASK) {
            uwb_process_quick_timer_evt(*((uint16_t *)msg.pMsgData));
        }

        if (event & UWA_MBOX_EVT_MASK) {
            uwa_sys_event(&(((tUWA_DM_API_ENABLE *)msg.pMsgData)->hdr));
        }
    }

    UCI_TRACE_D("uwb_task terminated");
    phOsalUwb_ProduceSemaphore(gp_uwbtask_ctrl->uwb_task_sem);

    /* Suspend task here so that it does not return in FreeRTOS
     * Task will be deleted in shutdown sequence
     */
    (void)phOsalUwb_TaskSuspend(gp_uwbtask_ctrl->task_handle);
}
