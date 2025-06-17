/*
 * Copyright (C) 2010-2021,2023 NXP Semiconductors
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

/*
 * OSAL independent message queue implementation for Android (can be used under
 * Linux too)
 */

#include <errno.h>
#include <linux/ipc.h>
#include "phOsalUwb.h"
#ifndef PHUWBTYPES_H
#include "phUwbTypes.h"
#endif
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

typedef struct phOsalUwb_message_queue_item
{
    phLibUwb_Message_t nMsg;
    struct phOsalUwb_message_queue_item *pPrev;
    struct phOsalUwb_message_queue_item *pNext;
} phOsalUwb_message_queue_item_t;

typedef struct phOsalUwb_message_queue
{
    phOsalUwb_message_queue_item_t *pItems;
    pthread_mutex_t nCriticalSectionMutex;
    sem_t nProcessSemaphore;

} phOsalUwb_message_queue_t;

/*******************************************************************************
**
** Function         phOsalUwb_msgget
**
** Description      Allocates message queue
**
** Parameters       Ignored, included only for Linux queue API compatibility
**
** Returns          (int) value of pQueue if successful
**                  -1, if failed to allocate memory or to init mutex
**
*******************************************************************************/
intptr_t phOsalUwb_msgget(uint32_t queueLength)
{
    PHUWB_UNUSED(queueLength);
    phOsalUwb_message_queue_t *pQueue;
    pQueue = (phOsalUwb_message_queue_t *)malloc(sizeof(phOsalUwb_message_queue_t));
    if (pQueue == NULL)
        return (intptr_t)NULL;
    phOsalUwb_SetMemory(pQueue, 0, sizeof(phOsalUwb_message_queue_t));
    if (pthread_mutex_init(&pQueue->nCriticalSectionMutex, NULL) == -1) {
        free(pQueue);
        return (intptr_t)NULL;
    }
    if (sem_init(&pQueue->nProcessSemaphore, 0, 0) == -1) {
        free(pQueue);
        return (intptr_t)NULL;
    }

    return ((intptr_t)pQueue);
}

/*******************************************************************************
**
** Function         phOsalUwb_msgrelease
**
** Description      Releases message queue
**
** Parameters       msqid - message queue handle
**
** Returns          None
**
*******************************************************************************/
void phOsalUwb_msgrelease(intptr_t msqid)
{
    phOsalUwb_message_queue_t *pQueue = (phOsalUwb_message_queue_t *)msqid;

    if (pQueue != NULL) {
        sem_post(&pQueue->nProcessSemaphore);
        usleep(3000);
        if (sem_destroy(&pQueue->nProcessSemaphore)) {
            printf("Failed to destroy semaphore (errno=0x%08x)", errno);
        }
        pthread_mutex_destroy(&pQueue->nCriticalSectionMutex);

        free(pQueue);
    }

    return;
}

/*******************************************************************************
**
** Function         phOsalUwb_msgctl
**
** Description      Destroys message queue
**
** Parameters       msqid - message queue handle
**                  cmd, buf - ignored, included only for Linux queue API
**                  compatibility
**
** Returns          0,  if successful
**                  -1, if invalid handle is passed
**
*******************************************************************************/
int phOsalUwb_msgctl(intptr_t msqid, int cmd, void *buf)
{
    phOsalUwb_message_queue_t *pQueue;
    phOsalUwb_message_queue_item_t *p;
    PHUWB_UNUSED(cmd);
    PHUWB_UNUSED(buf);
    if (msqid == 0)
        return -1;

    pQueue = (phOsalUwb_message_queue_t *)msqid;
    pthread_mutex_lock(&pQueue->nCriticalSectionMutex);
    if (pQueue->pItems != NULL) {
        p = pQueue->pItems;
        while (p->pNext != NULL) {
            p = p->pNext;
        }
        while (p->pPrev != NULL) {
            p = p->pPrev;
            free(p->pNext);
            p->pNext = NULL;
        }
        free(p);
    }
    pQueue->pItems = NULL;
    pthread_mutex_unlock(&pQueue->nCriticalSectionMutex);
    pthread_mutex_destroy(&pQueue->nCriticalSectionMutex);
    free(pQueue);

    return 0;
}

/*******************************************************************************
**
** Function         phOsalUwb_msgsnd
**
** Description      Sends a message to the queue. The message will be added at
**                  the end of the queue as appropriate for FIFO policy
**
** Parameters       msqid  - message queue handle
**                  msg   - message to be sent
**                  waittimeout - ignored
**
** Returns          UWBSTATUS_SUCCESS,  if successful
**                  UWBSTATUS_FAILED, if invalid parameter passed or failed to allocate memory
**
*******************************************************************************/
UWBSTATUS phOsalUwb_msgsnd(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout)
{
    phOsalUwb_message_queue_t *pQueue;
    phOsalUwb_message_queue_item_t *p;
    phOsalUwb_message_queue_item_t *pNew;
    PHUWB_UNUSED(waittimeout);
    if ((msqid == 0) || (msg == NULL))
        return UWBSTATUS_FAILED;

    pQueue = (phOsalUwb_message_queue_t *)msqid;
    pNew   = (phOsalUwb_message_queue_item_t *)malloc(sizeof(phOsalUwb_message_queue_item_t));
    if (pNew == NULL)
        return UWBSTATUS_FAILED;
    phOsalUwb_SetMemory(pNew, 0, sizeof(phOsalUwb_message_queue_item_t));
    phOsalUwb_MemCopy(&pNew->nMsg, msg, sizeof(phLibUwb_Message_t));
    pthread_mutex_lock(&pQueue->nCriticalSectionMutex);

    if (pQueue->pItems != NULL) {
        p = pQueue->pItems;
        while (p->pNext != NULL) {
            p = p->pNext;
        }
        p->pNext    = pNew;
        pNew->pPrev = p;
    }
    else {
        pQueue->pItems = pNew;
    }
    pthread_mutex_unlock(&pQueue->nCriticalSectionMutex);

    sem_post(&pQueue->nProcessSemaphore);

    return UWBSTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         phOsalUwb_msgrcv
**
** Description      Gets the oldest message from the queue.
**                  If the queue is empty the function waits (blocks on a mutex)
**                  until a message is posted to the queue with phOsalUwb_msgsnd
**
** Parameters       msqid  - message queue handle
**                  msg    - message to be received
**                  waittimeout - ignored
**
** Returns          UWBSTATUS_SUCCESS,  if successful
**                  UWBSTATUS_FAILED, if invalid parameter passed
**
*******************************************************************************/
UWBSTATUS phOsalUwb_msgrcv(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout)
{
    phOsalUwb_message_queue_t *pQueue;
    phOsalUwb_message_queue_item_t *p;
    PHUWB_UNUSED(waittimeout);
    if ((msqid == 0) || (msg == NULL))
        return UWBSTATUS_FAILED;

    pQueue = (phOsalUwb_message_queue_t *)msqid;

    if (sem_wait(&pQueue->nProcessSemaphore) != 0) {
        printf("Failed to wait semaphore (errno=0x%08x)", errno);
    }

    pthread_mutex_lock(&pQueue->nCriticalSectionMutex);

    if (pQueue->pItems != NULL) {
        phOsalUwb_MemCopy(msg, &(pQueue->pItems)->nMsg, sizeof(phLibUwb_Message_t));
        p = pQueue->pItems->pNext;
        free(pQueue->pItems);
        pQueue->pItems = p;
    }
    pthread_mutex_unlock(&pQueue->nCriticalSectionMutex);

    return UWBSTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         phOsalUwb_queueSpacesAvailable
**
** Description      Dummy function added to avoid build failure on
**                  Linux system.
**
** Parameters       msqid  - message queue handle
**
** Returns          0
**
*******************************************************************************/
int phOsalUwb_queueSpacesAvailable(intptr_t msqid)
{
    return 0;
}
