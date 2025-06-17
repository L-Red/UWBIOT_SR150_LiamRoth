/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2019,2022 NXP
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
#ifndef UWBADAPT_H
#define UWBADAPT_H

#include "phNxpUciHal.h"
#include "phUwb_BuildConfig.h"
#include "uwb_hal_api.h"
#include "uwb_target.h"
#include "uwa_api.h"

#define TASK_MBOX_0_EVT_MASK 0x0001
#define TASK_MBOX_1_EVT_MASK 0x0002 // Not used at present
#define TASK_MBOX_2_EVT_MASK 0x0004
#define TASK_MBOX_3_EVT_MASK 0x0008 // Not used at present

/* Only one timer is used for command response timeout */
#define TIMER_1 1

#define TIMER_0_EVT_MASK 0x0010
#define TIMER_1_EVT_MASK 0x0020
#define TIMER_2_EVT_MASK 0x0040
#define TIMER_3_EVT_MASK 0x0080

#ifndef UWB_TASK
#define UWB_TASK 3
#endif

/******************************************************************************
**
** Timer configuration
**
******************************************************************************/

/* A conversion value for translating ticks to calculate GKI timer.  */
#ifndef TICKS_PER_SEC
#define TICKS_PER_SEC 1000
#endif

/************************************************************************
**  Utility macros converting ticks to time with user define OS ticks per sec
**/
#ifndef GKI_MS_TO_TICKS
#define GKI_MS_TO_TICKS(x) ((x) / (1000 / TICKS_PER_SEC))
#endif

#ifndef GKI_SECS_TO_TICKS
#define GKI_SECS_TO_TICKS(x) ((x) * (TICKS_PER_SEC))
#endif

#ifndef GKI_TICKS_TO_MS
#define GKI_TICKS_TO_MS(x) ((x)*1000 / TICKS_PER_SEC)
#endif

#ifndef GKI_TICKS_TO_SECS
#define GKI_TICKS_TO_SECS(x) ((x) / TICKS_PER_SEC)
#endif

/* TICK per second from OS (OS dependent change this macro accordingly to
 * various OS) */
#ifndef OS_TICKS_PER_SEC
#define OS_TICKS_PER_SEC 1000
#endif

/************************************************************************
**  Utility macros converting ticks to time with user define OS ticks per sec
**/

#ifndef GKI_OS_TICKS_TO_MS
#define GKI_OS_TICKS_TO_MS(x) ((x)*1000 / OS_TICKS_PER_SEC)
#endif

#ifndef GKI_OS_TICKS_TO_SECS
#define GKI_OS_TICKS_TO_SECS(x)   ((x) / OS_TICKS_PER_SEC))
#endif

/* delay in ticks before stopping system tick. */
#ifndef GKI_DELAY_STOP_SYS_TICK
#define GKI_DELAY_STOP_SYS_TICK 10
#endif

/* Option to guarantee no preemption during timer expiration (most system don't
 * need this) */
#ifndef GKI_TIMER_LIST_NOPREEMPT
#define GKI_TIMER_LIST_NOPREEMPT FALSE
#endif

/************************************************************************
** Mailbox definitions. Each task has 4 mailboxes that are used to
** send buffers to the task.
*/
#define TASK_MBOX_0 0
#define TASK_MBOX_2 2

#define NUM_TASK_MBOX 4

#define APPL_EVT_0 8
#define APPL_EVT_7 15

#define EVENT_MASK(evt) ((uint16_t)(0x0001u << (evt)))

#ifndef UWB_SHUTDOWN_EVT
#define UWB_SHUTDOWN_EVT APPL_EVT_7
#endif

#ifndef UWB_SHUTDOWN_EVT_MASK
#define UWB_SHUTDOWN_EVT_MASK EVENT_MASK(UWB_SHUTDOWN_EVT)
#endif

#ifndef UWB_TASK
#define UWB_TASK 3
#endif

void Initialize();
tUCI_STATUS UwbDeviceInit(bool recovery);
void Finalize();
const tHAL_UWB_ENTRY *GetHalEntryFuncs();
void HalRegisterAppCallback(phHalAppDataCb *recvDataCb);
bool isCmdRespPending();
void Hal_setOperationMode(Uwb_operation_mode_t state);
void phUwb_OSAL_send_msg(uint8_t task_id, uint16_t mbox, void *pmsg);

#endif
