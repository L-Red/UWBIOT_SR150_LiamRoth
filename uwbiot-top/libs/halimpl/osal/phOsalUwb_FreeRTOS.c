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

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "phNxpUciHal_utils.h"
#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"
#include "phUwb_BuildConfig.h"
#include "uwb_board.h"

/*
*************************** Function Definitions ******************************
*/

/*!
 * \brief Allocates memory.
 *        This function attempts to allocate \a size bytes on the heap and
 *        returns a pointer to the allocated block.
 *
 * \param size size of the memory block to be allocated on the heap.
 *
 * \return pointer to allocated memory block or NULL in case of error.
 */
void *phOsalUwb_GetMemory(uint32_t dwSize)
{
    return (void *)pvPortMalloc(dwSize);
}

/*!
 * \brief Frees allocated memory block.
 *        This function deallocates memory region pointed to by \a pMem.
 *
 * \param pMem pointer to memory block to be freed.
 */
void phOsalUwb_FreeMemory(void *pMem)
{
    /* Check whether a null pointer is passed */
    if (NULL != pMem) {
        vPortFree(pMem);
    }
}

int32_t phOsalUwb_MemCompare(const void *pDest, const void *pSrc, uint32_t dwSize)
{
    return memcmp(pDest, pSrc, dwSize);
}

void phOsalUwb_SetMemory(void *pMem, uint8_t bVal, uint32_t dwSize)
{
    memset(pMem, bVal, dwSize);
}

void phOsalUwb_MemCopy(void *pDest, const void *pSrc, uint32_t dwSize)
{
    memcpy(pDest, pSrc, dwSize);
}

/*===============================================*/

UWBSTATUS phOsalUwb_CreateSemaphore(void **hSemaphore, uint8_t bInitialValue)
{
    UWBSTATUS wCreateStatus                      = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = NULL;

    /* Check whether user passed valid input parameters */
    if ((NULL == hSemaphore)) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Fill the Semaphore Handle structure */
        pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_sOsalSemaphore_t));
        if (pSemaphoreHandle == NULL) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
            return wCreateStatus;
        }

        pSemaphoreHandle->ObjectHandle = NULL;
        pSemaphoreHandle->ObjectHandle = xSemaphoreCreateCounting(1, (uint32_t)bInitialValue);
        if (NULL == pSemaphoreHandle->ObjectHandle) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
            phOsalUwb_FreeMemory(pSemaphoreHandle);
        }
        else {
            /* Return the semaphore handle to the caller function */
            *hSemaphore = (void *)pSemaphoreHandle;
        }
    }
    return wCreateStatus;
}

UWBSTATUS phOsalUwb_ProduceSemaphore(void *hSemaphore)
{
    UWBSTATUS wReleaseStatus                     = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)hSemaphore;
    BaseType_t xHigherPriorityTaskWoken          = pdFALSE;

    /* Check whether user passed valid parameter */
    if (pSemaphoreHandle != NULL) {
        if (phPlatform_Is_Irq_Context()) {
            if (!(xSemaphoreGiveFromISR(pSemaphoreHandle->ObjectHandle, &xHigherPriorityTaskWoken))) {
                wReleaseStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_PRODUCE_ERROR);
            }
            else {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else {
            if (!(xSemaphoreGive(pSemaphoreHandle->ObjectHandle))) {
                wReleaseStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_PRODUCE_ERROR);
            }
        }
    }
    else {
        wReleaseStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wReleaseStatus;
}

UWBSTATUS phOsalUwb_ConsumeSemaphore_WithTimeout(void *hSemaphore, uint32_t delay)
{
    UWBSTATUS wConsumeStatus                     = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)hSemaphore;
    BaseType_t xHigherPriorityTaskWoken          = pdFALSE;

    /* Check whether user passed valid parameter */
    if (pSemaphoreHandle != NULL) {
        /* Wait till the semaphore object is released */
        if (phPlatform_Is_Irq_Context()) {
            if (!(xSemaphoreTakeFromISR(pSemaphoreHandle->ObjectHandle, &xHigherPriorityTaskWoken))) {
                wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CONSUME_ERROR);
            }
            else {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else {
            if (!(xSemaphoreTake(pSemaphoreHandle->ObjectHandle, delay / portTICK_PERIOD_MS))) {
                wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CONSUME_ERROR);
            }
        }
    }
    else {
        wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wConsumeStatus;
}

UWBSTATUS phOsalUwb_DeleteSemaphore(void **hSemaphore)
{
    UWBSTATUS wDeletionStatus                    = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)*hSemaphore;

    /* Check whether OSAL is initialized and user has passed a valid pointer */
    if (pSemaphoreHandle != NULL) {
        /* Check whether function was successful */
        vSemaphoreDelete(pSemaphoreHandle->ObjectHandle);
        phOsalUwb_FreeMemory(pSemaphoreHandle);
        *hSemaphore = NULL;
    }
    else {
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wDeletionStatus;
}

UWBSTATUS phOsalUwb_CreateMutex(void **hMutex)
{
    UWBSTATUS wCreateStatus = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalMutex_t *pMutexHandle;

    if (NULL != hMutex) {
        pMutexHandle = (phOsalUwb_sOsalMutex_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_sOsalMutex_t));
        if (pMutexHandle == NULL) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
            return wCreateStatus;
        }
        pMutexHandle->ObjectHandle = NULL;
        pMutexHandle->ObjectHandle = xSemaphoreCreateMutex();
        if (NULL == pMutexHandle->ObjectHandle) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_CREATION_ERROR);
            phOsalUwb_FreeMemory(pMutexHandle);
        }
        else {
            *hMutex = (void *)pMutexHandle;
        }
    }
    else {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wCreateStatus;
}

UWBSTATUS phOsalUwb_CreateBinSem(void **hBinSem)
{
    UWBSTATUS wCreateStatus = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalSemaphore_t *pBinSemHandle;

    if (NULL != hBinSem) {
        pBinSemHandle = (phOsalUwb_sOsalSemaphore_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_sOsalSemaphore_t));
        if (pBinSemHandle == NULL) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
            return wCreateStatus;
        }
        pBinSemHandle->ObjectHandle = NULL;
        pBinSemHandle->ObjectHandle = xSemaphoreCreateBinary();
        if (NULL == pBinSemHandle->ObjectHandle) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_CREATION_ERROR);
            phOsalUwb_FreeMemory(pBinSemHandle);
        }
        else {
            *hBinSem = (void *)pBinSemHandle;
        }
    }
    else {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wCreateStatus;
}

UWBSTATUS phOsalUwb_LockMutex(void *hMutex)
{
    UWBSTATUS wLockStatus                = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalMutex_t *pMutexHandle = (phOsalUwb_sOsalMutex_t *)hMutex;
    BaseType_t xHigherPriorityTaskWoken  = pdFALSE;

    /* Check whether handle provided by user is valid */
    if (pMutexHandle != NULL) {
        if (phPlatform_Is_Irq_Context()) {
            if (!(xSemaphoreTakeFromISR(pMutexHandle->ObjectHandle, &xHigherPriorityTaskWoken))) {
                wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_LOCK_ERROR);
            }
            else {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }

        else {
            if (!(xSemaphoreTake(pMutexHandle->ObjectHandle, MAX_DELAY))) {
                wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_LOCK_ERROR);
            }
        }
    }
    else {
        wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wLockStatus;
}

UWBSTATUS phOsalUwb_UnlockMutex(void *hMutex)
{
    UWBSTATUS wUnlockStatus              = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalMutex_t *pMutexHandle = (phOsalUwb_sOsalMutex_t *)hMutex;
    BaseType_t xHigherPriorityTaskWoken  = pdFALSE;

    /* Check whether the handle provided by user is valid */
    if (pMutexHandle != NULL) {
        if (phPlatform_Is_Irq_Context()) {
            if (!(xSemaphoreGiveFromISR(pMutexHandle->ObjectHandle, &xHigherPriorityTaskWoken))) {
                wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_UNLOCK_ERROR);
            }
            else {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else {
            if (!(xSemaphoreGive(pMutexHandle->ObjectHandle))) {
                wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_UNLOCK_ERROR);
            }
        }
    }
    else {
        wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wUnlockStatus;
}

UWBSTATUS phOsalUwb_DeleteMutex(void **hMutex)
{
    UWBSTATUS wDeletionStatus            = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalMutex_t *pMutexHandle = (phOsalUwb_sOsalMutex_t *)*hMutex;

    /* Check whether the handle provided by user is valid */
    if (pMutexHandle != NULL) {
        vSemaphoreDelete(pMutexHandle->ObjectHandle);
        phOsalUwb_FreeMemory(pMutexHandle);
        *hMutex = NULL;
    }
    else {
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wDeletionStatus;
}
/* Delay  in Milli second */
void phOsalUwb_Delay(uint32_t dwDelay)
{
    vTaskDelay(pdMS_TO_TICKS(dwDelay));
}

void phOsalUwb_GetTickCount(unsigned long *pTickCount)
{
    *pTickCount = xTaskGetTickCount();
}
