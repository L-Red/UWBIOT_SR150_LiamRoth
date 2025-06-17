/*
 * Copyright 2012-2020,2022,2023 NXP.
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

#ifndef _PHNXPUCIHAL_UTILS_H_
#define _PHNXPUCIHAL_UTILS_H_

#include "phUwbStatus.h"
#include "phUwb_BuildConfig.h"
#include <UWB_Assert.h>

/********************* Definitions and structures *****************************/

/* List structures */
struct listNode
{
    void *pData;
    struct listNode *pNext;
};

struct listHead
{
    struct listNode *pFirst;
    void *mutex;
};

/* Semaphore handling structure */
typedef struct phNxpUciHal_Sem
{
    /* Semaphore used to wait for callback */
    void *sem;

    /* Used to store the status sent by the callback */
    UWBSTATUS status;

    /* Used to provide a local context to the callback */
    void *pContext;

} phNxpUciHal_Sem_t;

typedef struct phNxpUciHal_DevStaus_Sem
{
    void *sem;
} phNxpUciHal_DevStatus_Sem_t;

/* 5000ms is chosen as a worst case time to get the device status notification.
 * See artf955860 */
#define HAL_MAX_DEVICE_ST_NTF_TIMEOUT (5000)
#define HAL_MAX_WRITE_TIMOUT          (1000)
/* timer of scaling factore of cmd lenght is already started, waiting for semaphore with +10 msec */
#define HAL_MAX_EXTENDED_CMD_RSP_TIMEOUT(x) ((200 + x / 4) + 1000)
#define HAL_MAX_CLOSE_EVENT_TIMOUT          (500)

/* Semaphore and mutex monitor */
typedef struct phNxpUciHal_Monitor
{
    /* Mutex protecting native library against reentrance */
    void *reentrance_binSem;

    /* Mutex protecting native library against concurrency */
    void *concurrency_binSem;

    /* List used to track pending semaphores waiting for callback */
    struct listHead sem_list;

} phNxpUciHal_Monitor_t;

/************************ Exposed functions ***********************************/
/* List functions */
int phNxpUciHal_listInit(struct listHead *pList);
int phNxpUciHal_listDestroy(struct listHead *pList);
int phNxpUciHal_listAdd(struct listHead *pList, void *pData);
int phNxpUciHal_listRemove(struct listHead *pList, void *pData);
int phNxpUciHal_listGetAndRemoveNext(struct listHead *pList, void **ppData);
void phNxpUciHal_listDump(struct listHead *pList);

/* NXP UCI HAL utility functions */
phNxpUciHal_Monitor_t *phNxpUciHal_init_monitor(void);
void phNxpUciHal_cleanup_monitor(void);
phNxpUciHal_Monitor_t *phNxpUciHal_get_monitor(void);
UWBSTATUS phNxpUciHal_init_cb_data(phNxpUciHal_Sem_t *pCallbackData, void *pContext);
void phNxpUciHal_cleanup_cb_data(phNxpUciHal_Sem_t *pCallbackData);
void phNxpUciHal_releaseall_cb_data(void);

/* Reentrance and concurrency timeout */
#define MAX_HAL_REENTRANCE_WAIT_TIMEOUT  (5000)
#define MAX_HAL_CONCURRENCY_WAIT_TIMEOUT (5000)
/* Lock unlock helper macros */
#define REENTRANCE_LOCK()                         \
    (void)phOsalUwb_ConsumeSemaphore_WithTimeout( \
        (phNxpUciHal_get_monitor()->reentrance_binSem), MAX_HAL_REENTRANCE_WAIT_TIMEOUT);
#define REENTRANCE_UNLOCK() (void)phOsalUwb_ProduceSemaphore(phNxpUciHal_get_monitor()->reentrance_binSem);
#define CONCURRENCY_LOCK()                        \
    (void)phOsalUwb_ConsumeSemaphore_WithTimeout( \
        (phNxpUciHal_get_monitor()->concurrency_binSem), MAX_HAL_CONCURRENCY_WAIT_TIMEOUT);
#define CONCURRENCY_UNLOCK() (void)phOsalUwb_ProduceSemaphore(phNxpUciHal_get_monitor()->concurrency_binSem);
#endif /* _PHNXPUCIHAL_UTILS_H_ */
