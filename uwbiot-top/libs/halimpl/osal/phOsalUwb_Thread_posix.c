/*
 * Copyright 2012-2021,2023 NXP.
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
 * \file  phOsalUwb_Thread_posix.c
 * \brief OSAL Implementation.
 */

/** \addtogroup grp_osal_uwb
    @{
 */
/*
************************* Header Files ****************************************
*/
#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"
#include "phNxpLogApis_HalUtils.h"
#include <pthread.h>
#include <errno.h>

/*
****************************** Macro Definitions ******************************
*/
/** \ingroup grp_osal_uwb
    Value indicates Failure to suspend/resume thread */
#define PH_OSALUWB_THREADPROC_FAILURE (0xFFFFFFFF)

/*
*************************** Function Definitions ******************************
*/

#if 0
static void *phOsalUwb_ThreadProcedure(void *lpParameter)
{
    phOsalUwb_sOsalThreadHandle_t *pThreadHandle = (phOsalUwb_sOsalThreadHandle_t *)lpParameter;
    pThreadHandle->ThreadFunction(pThreadHandle->Params);
    return PH_OSALUWB_RESET_VALUE;
}
#endif

UWBSTATUS phOsalUwb_Thread_Create(void **hThread, pphOsalUwb_ThreadFunction_t pThreadFunction, void *pParam)
{
    UWBSTATUS wCreateStatus = UWBSTATUS_SUCCESS;

    phOsalUwb_ThreadCreationParams_t *threadparams = (phOsalUwb_ThreadCreationParams_t *)pParam;

    if ((NULL == hThread) || (NULL == pThreadFunction)) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        pthread_t handle;
        if (pthread_create(&handle, NULL, pThreadFunction, threadparams->pContext) == 0) {
            /* Assign the Thread handle structure to the pointer provided by the user */
            *hThread = (void *)handle;
            phOsalUwb_Delay(10);
        }
        else {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_CREATION_ERROR);
        }
    }

    LOG_D("Create thread %s. Handle %X", threadparams->taskname, *hThread);

    return wCreateStatus;
}

UWBSTATUS phOsalUwb_Thread_Delete(UWBOSAL_TASK_HANDLE hThread)
{
    UWBSTATUS wDeletionStatus;

    LOG_D("Deleting thread %X", hThread);
    int ret = pthread_cancel(hThread);

    if (0 == ret || ESRCH == ret) {
        ret = pthread_join(hThread, NULL);
    }
    if (0 == ret) {
        wDeletionStatus = UWBSTATUS_SUCCESS;
        LOG_D("Terminated thread 0x%X", hThread);
    }

    if (0 != ret) {
        wDeletionStatus = UWBSTATUS_FAILED;
        LOG_W("Could not delete thread 0x%X", hThread);
    }
    return wDeletionStatus;
}

UWBOSAL_TASK_HANDLE phOsalUwb_GetTaskHandle(void)
{
    return (pthread_self());
}

void phOsalUwb_TaskResume(UWBOSAL_TASK_HANDLE xTaskToResume)
{
}

void phOsalUwb_TaskSuspend(UWBOSAL_TASK_HANDLE xTaskToSuspend)
{
}

void phOsalUwb_TaskStartScheduler(void)
{
}

void phOsalUwb_Thread_Join(UWBOSAL_TASK_HANDLE hThread)
{
    pthread_join(hThread, NULL);
}

/** @} */
