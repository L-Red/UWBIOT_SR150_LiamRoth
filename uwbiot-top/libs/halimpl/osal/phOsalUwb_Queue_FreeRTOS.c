/*
 * Copyright 2012-2023 NXP.
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

/**
 * \file  phOsalUwb_Queue_FreeRTOS.c
 * \brief OSAL Implementation.
 */

/*
************************* Header Files ****************************************
*/

#include "phOsalUwb.h"
#include "FreeRTOSConfig.h"
#include "phOsalUwb_Internal.h"
#include "uwb_board.h"

typedef struct phOsalUwb_Queue
{
    QueueHandle_t pQueue;
} phOsalUwb_Queue_t;

/*******************************************************************************
**
** Function         phOsalUwb_msgget
**
** Description      Allocates message queue
**
** Parameters       queueLength - Length of the queue
**
** Returns          (int) value of pQueue if successful
**                  -1, if failed to allocate memory or to init mutex
**
*******************************************************************************/
intptr_t phOsalUwb_msgget(uint32_t queueLength)
{
    phOsalUwb_Queue_t *pMsgQ;
    pMsgQ = (phOsalUwb_Queue_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_Queue_t));
    if (pMsgQ == NULL) {
        return (intptr_t)NULL;
    }
    pMsgQ->pQueue = xQueueCreate((UBaseType_t)queueLength, sizeof(phLibUwb_Message_t));
    if (pMsgQ->pQueue == NULL) {
        phOsalUwb_FreeMemory(pMsgQ);
        return (intptr_t)NULL;
    }

    return ((intptr_t)pMsgQ);
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
    phOsalUwb_Queue_t *pMsgQ = (phOsalUwb_Queue_t *)msqid;

    if (pMsgQ != NULL) {
        if (pMsgQ->pQueue != NULL) {
            vQueueDelete(pMsgQ->pQueue);
            pMsgQ->pQueue = NULL;
        }
        phOsalUwb_FreeMemory(pMsgQ);
    }
    return;
}

/*******************************************************************************
**
** Function         phOsalUwb_msgsnd
**
** Description      Sends a message to the queue. The message will be added at
**                  the end of the queue as appropriate for FIFO policy
**
** Parameters       msqid  - message queue handle
**                  msg    - message to be sent
**                  waittimeout - timeout in ms
**
** Returns          UWBSTATUS_SUCCESS,  if successful
**                  UWBSTATUS_FAILED, if invalid parameter passed or failed to allocate memory
**
*******************************************************************************/
UWBSTATUS phOsalUwb_msgsnd(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout)
{
    TickType_t delay;
    phOsalUwb_Queue_t *pMsgQ            = (phOsalUwb_Queue_t *)msqid;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if ((pMsgQ == NULL) || (pMsgQ->pQueue == NULL) || (msg == NULL))
        return UWBSTATUS_FAILED;

    if (waittimeout != portMAX_DELAY && waittimeout != NO_DELAY)
        delay = pdMS_TO_TICKS(waittimeout);
    else
        delay = waittimeout;

    /* Wait till the semaphore object is released */
    if (phPlatform_Is_Irq_Context()) {
        if (!(xQueueSendToBackFromISR(pMsgQ->pQueue, msg, &xHigherPriorityTaskWoken))) {
            return UWBSTATUS_FAILED;
        }
        else {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else {
        if (!(xQueueSendToBack(pMsgQ->pQueue, msg, delay))) {
            return UWBSTATUS_FAILED;
        }
    }

    return UWBSTATUS_SUCCESS;
}

#if UWBIOT_UWBD_SR040

UWBSTATUS phOsalUwb_msgreset(intptr_t msqid)
{
    phOsalUwb_Queue_t *pMsgQ = (phOsalUwb_Queue_t *)msqid;
    xQueueReset(pMsgQ->pQueue);
    return UWBSTATUS_SUCCESS;
}
#endif //UWBIOT_UWBD_SR040

/*******************************************************************************
**
** Function         phOsalUwb_msgrcv
**
** Description      Gets the oldest message from the queue.
**                  If the queue is empty the function waits (blocks on a mutex)
**                  until a message is posted to the queue with
**                  phOsalUwb_msgsnd.
**
** Parameters       msqid  - message queue handle
**                  msg    - container for the message to be received
**                  waittimeout - max wait time for task in ms
**
** Returns          UWBSTATUS_SUCCESS,  if successful
**                  UWBSTATUS_FAILED, if invalid parameter passed
**
*******************************************************************************/
UWBSTATUS phOsalUwb_msgrcv(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout)
{
    UBaseType_t returnVal = FALSE;
    TickType_t delay;
    phOsalUwb_Queue_t *pMsgQ = (phOsalUwb_Queue_t *)msqid;
    if ((pMsgQ == NULL) || (pMsgQ->pQueue == NULL) || (msg == NULL))
        return UWBSTATUS_FAILED;
    if (waittimeout != portMAX_DELAY && waittimeout != NO_DELAY)
        delay = pdMS_TO_TICKS(waittimeout);
    else
        delay = waittimeout;
    returnVal = xQueueReceive(pMsgQ->pQueue, msg, delay);
    if (returnVal == pdTRUE)
        return UWBSTATUS_SUCCESS;
    else
        return UWBSTATUS_FAILED;
}

/*******************************************************************************
**
** Function         phOsalUwb_queueSpacesAvailable
**
** Description      Gets the space available in queue
**
** Parameters       msqid  - message queue handle
**
** Returns          space available,  if successful
**                  -1, if invalid parameter passed
**
*******************************************************************************/
int phOsalUwb_queueSpacesAvailable(intptr_t msqid)
{
    phOsalUwb_Queue_t *pMsgQ = (phOsalUwb_Queue_t *)msqid;
    if (pMsgQ->pQueue == NULL)
        return -1;

    return (int)uxQueueSpacesAvailable(pMsgQ->pQueue);
}
