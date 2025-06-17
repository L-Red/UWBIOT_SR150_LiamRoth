/*
 *
 * Copyright 2018-2020,2022 NXP.
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

#ifndef UWB_INT_H_
#define UWB_INT_H_

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include "phUwb_BuildConfig.h"
#include "uwb_target.h"
#include "UwbAdaptation.h"
#include "uci_defs.h"
#include "uwb_types.h"
#include "uwb_hal_api.h"
#include "uwa_api.h"

/*********************************************************
 * UWB Internal Group-0xF: Opcodes of command
 *********************************************************/
#define UCI_ENABLE     0x00
#define UCI_DISABLE    0x01
#define UCI_REG_EXT_CB 0x02
#define UCI_TIMEOUT    0x03

#define UWB_SEGMENT_PKT_SENT 0xFE
typedef uint8_t tUWB_RAW_EVT; /* proprietary events */

/*************************************
**  RESPONSE Callback Functions
**************************************/
typedef void(tUWB_RAW_CBACK)(uint8_t gid, tUWB_RAW_EVT event, uint16_t data_len, uint8_t *p_data);

/****************************************************************************
** UWB_TASK definitions
****************************************************************************/

/* UWB_TASK event masks */
#define UWB_TASK_EVT_TRANSPORT_READY EVENT_MASK(APPL_EVT_0)

/* UWB Timer events */
#define UWB_TTYPE_UCI_WAIT_RSP 0x00
#define UWB_WAIT_RSP_RAW_CMD   0x01

#define UWB_SAVED_HDR_SIZE 2

/* UWB Task event messages */
enum
{
    UWB_STATE_NONE,         /* not start up yet                         */
    UWB_STATE_W4_HAL_OPEN,  /* waiting for HAL_UWB_OPEN_CPLT_EVT   */
    UWB_STATE_IDLE,         /* normal operation( device is in idle state) */
    UWB_STATE_ACTIVE,       /* UWB device is in active                    */
    UWB_STATE_W4_HAL_CLOSE, /* waiting for HAL_UWB_CLOSE_CPLT_EVT  */
    UWB_STATE_CLOSING
};
typedef uint8_t tUWB_STATE;

/* This data type is for UWB task to send a UCI VS command to UCIT task */
typedef struct
{
    UWB_HDR bt_hdr;          /* the UCI command          */
    tUWB_RAW_CBACK *p_cback; /* the callback function to receive RSP   */
} tUWB_UCI_RAW_MSG;

/* This data type is for HAL event */
typedef struct
{
    UWB_HDR hdr;
    uint8_t hal_evt; /* HAL event code  */
    uint8_t status;  /* tHAL_UWB_STATUS */
} tUWB_HAL_EVT_MSG;

/* callback function pointer(8; use 8 to be safe + UWB_SAVED_CMD_SIZE(2) */
#define UWB_RECEIVE_MSGS_OFFSET (10)

/* UWB control blocks */
typedef struct
{
    tUWB_RAW_CBACK *p_ext_resp_cback;

    tUWB_STATE uwb_state;

    uint8_t last_hdr[UWB_SAVED_HDR_SIZE]; /* part of last UCI command header */
    uint8_t last_cmd[UWB_SAVED_HDR_SIZE]; /* part of last UCI command payload */

    tUWB_RAW_CBACK *p_raw_cmd_cback; /* the callback function for last raw command */

    uint16_t uci_wait_rsp_tout;   /* UCI command timeout (in ms) */
    uint16_t retry_rsp_timeout;   /* UCI command timeout during retry */
    uint8_t uci_cmd_window;       /* Number of commands the controller can accecpt
                             without waiting for response. */
    bool is_resp_pending;         /* response is pending from UWBS */
    bool is_recovery_in_progress; /* recovery in progresss  */

    const tHAL_UWB_ENTRY *p_hal;
    uint8_t rawCmdCbflag;
    uint8_t device_state;
    uint16_t cmd_retry_count;
    UWB_HDR *pLast_cmd_buf;
    bool IsConformaceTestEnabled;
    uint8_t UwbOperatinMode;
    bool isCreditNtfReceived;
} tUWB_CB;

/* UWB Task Control structure */
typedef struct Uwbtask_Control
{
    UWBOSAL_TASK_HANDLE task_handle;
    intptr_t pMsgQHandle;
    void *uwb_task_sem;
} phUwbtask_Control_t;

/*****************************************************************************
 **  EXTERNAL FUNCTION DECLARATIONS
 *****************************************************************************/

/* Global UWB data */
extern tUWB_CB uwb_cb;

/****************************************************************************
 ** Internal uwb functions
 ****************************************************************************/

extern bool uwb_ucif_process_event(UWB_HDR *p_msg);
extern void uwb_ucif_check_cmd_queue(UWB_HDR *p_buf);
extern void uwb_ucif_retransmit_cmd(UWB_HDR *p_buf);
extern void uwb_ucif_send_cmd(UWB_HDR *p_buf);
extern void uwb_ucif_update_cmd_window(void);
extern void uwb_ucif_cmd_timeout(void);
extern void uwb_ucif_uwb_recovery(void);
void uwb_ucif_dump_fw_crash_log();

/* From uwb_task.c */
extern OSAL_TASK_RETURN_TYPE uwb_task(void *args);

void uwb_task_shutdown_uwbc(void);

/* From uwb_main.c */
void uwb_set_state(tUWB_STATE uwb_state);
void uwb_main_flush_cmd_queue(void);
void uwb_main_handle_hal_evt(tUWB_HAL_EVT_MSG *p_msg);
void uwb_gen_cleanup(void);

void uwb_start_quick_timer(uint32_t timeout);
void uwb_stop_quick_timer();
void uwb_process_quick_timer_evt(uint16_t event);
void uwb_main_hal_cback(uint8_t event, tUCI_STATUS status);
void uwb_main_hal_data_cback(uint16_t data_len, uint8_t *p_data);
#endif /* UWB_INT_H_ */
