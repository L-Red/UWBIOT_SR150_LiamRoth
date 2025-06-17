/*
 * Copyright (C) 2012-2021,2023 NXP Semiconductors
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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "phNxpUciHal_utils.h"
#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"
#include "phUwb_BuildConfig.h"
#include "uwb_board.h"

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
    return (void *)malloc(dwSize);
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
        free(pMem);
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

        if (sem_init(&pSemaphoreHandle->ObjectHandle, 0, bInitialValue) == -1) {
            phOsalUwb_FreeMemory(pSemaphoreHandle);
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
            return wCreateStatus;
        }
        else {
            /* Return the semaphore handle to the caller function */
            *hSemaphore = (void *)pSemaphoreHandle;
        }
    }
    return wCreateStatus;
}

UWBSTATUS phOsalUwb_ConsumeSemaphore_WithTimeout(void *hSemaphore, uint32_t delay_ms)
{
    UWBSTATUS wConsumeStatus                     = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)hSemaphore;
    int ret;
    struct timespec abstime = {0};

    if (clock_gettime(CLOCK_REALTIME, &abstime) == -1) {
        wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    /* Check whether user passed valid parameter, and we know current time */
    if (wConsumeStatus == UWBSTATUS_SUCCESS && pSemaphoreHandle != NULL) {
        abstime.tv_sec += ((delay_ms / 1000u));
        /* ms -> us -> ns */
        abstime.tv_nsec += ((delay_ms % 1000u) * 1000u * 1000u);
        if (abstime.tv_nsec > (1000u * 1000u * 1000u)) {
            abstime.tv_sec += 1;
            abstime.tv_nsec -= (1000u * 1000u * 1000u);
        }
        /* Wait till the semaphore object is released */
        ret = sem_timedwait(&pSemaphoreHandle->ObjectHandle, &abstime);
        // LOG_D("sem_timedwait::ret=%d errno=%d delay_ms=%d\n", ret, errno, delay_ms);
        if (ret == -1) {
            /* something went wrong in waiting */
            wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CONSUME_ERROR);
        }
    }
    else {
        wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wConsumeStatus;
}

UWBSTATUS phOsalUwb_ProduceSemaphore(void *hSemaphore)
{
    UWBSTATUS wReleaseStatus                     = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)hSemaphore;

    /* Check whether user passed valid parameter */
    if (pSemaphoreHandle != NULL) {
        if (phPlatform_Is_Irq_Context()) {
            sem_post(&pSemaphoreHandle->ObjectHandle);
        }
        else {
            sem_post(&pSemaphoreHandle->ObjectHandle);
        }
    }
    else {
        wReleaseStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wReleaseStatus;
}

UWBSTATUS phOsalUwb_DeleteMutex(void **hMutex)
{
    UWBSTATUS wDeletionStatus            = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalMutex_t *pMutexHandle = (phOsalUwb_sOsalMutex_t *)*hMutex;

    /* Check whether the handle provided by user is valid */
    if (pMutexHandle != NULL) {
        pthread_mutex_destroy(&pMutexHandle->ObjectHandle);
        phOsalUwb_FreeMemory(pMutexHandle);
        *hMutex = NULL;
    }
    else {
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wDeletionStatus;
}

UWBSTATUS phOsalUwb_DeleteSemaphore(void **hSemaphore)
{
    UWBSTATUS wDeletionStatus                    = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)*hSemaphore;

    /* Check whether OSAL is initialized and user has passed a valid pointer */
    if (pSemaphoreHandle != NULL) {
        /* Check whether function was successful */
        sem_destroy(&pSemaphoreHandle->ObjectHandle);
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
        if (pthread_mutex_init(&pMutexHandle->ObjectHandle, NULL) != 0) {
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

UWBSTATUS phOsalUwb_LockMutex(void *hMutex)
{
    UWBSTATUS wLockStatus                = UWBSTATUS_SUCCESS;
    phOsalUwb_sOsalMutex_t *pMutexHandle = (phOsalUwb_sOsalMutex_t *)hMutex;

    /* Check whether handle provided by user is valid */
    if (pMutexHandle != NULL) {
        if (phPlatform_Is_Irq_Context()) {
            if ((pthread_mutex_lock(&pMutexHandle->ObjectHandle)) != 0) {
                wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_UNLOCK_ERROR);
            }
        }
        else {
            if ((pthread_mutex_lock(&pMutexHandle->ObjectHandle)) != 0) {
                wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_UNLOCK_ERROR);
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

    /* Check whether the handle provided by user is valid */
    if (pMutexHandle != NULL) {
        if (phPlatform_Is_Irq_Context()) {
            if ((pthread_mutex_unlock(&pMutexHandle->ObjectHandle)) != 0) {
                wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_UNLOCK_ERROR);
            }
        }
        else {
            if ((pthread_mutex_unlock(&pMutexHandle->ObjectHandle)) != 0) {
                wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_UNLOCK_ERROR);
            }
        }
    }
    else {
        wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wUnlockStatus;
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

        if (sem_init(&pBinSemHandle->ObjectHandle, 0, 0) == -1) {
            phOsalUwb_FreeMemory(pBinSemHandle);
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
            return wCreateStatus;
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

/*Delay  in Milli second*/
void phOsalUwb_Delay(uint32_t dwDelay)
{
    if (dwDelay >= 1000) {
        /* sleep takes seconds */
        sleep(dwDelay / 1000);
    }
    usleep((dwDelay % 1000) * 1000);
}

void phOsalUwb_GetTickCount(unsigned long *pTickCount)
{
    /* TO DO */
    struct timespec ts;
    unsigned ticks = 0U;
    clock_gettime(CLOCK_REALTIME, &ts);
    ticks = ts.tv_nsec / 1000000;
    ticks += ts.tv_sec * 1000;
    *pTickCount = ticks;
}
