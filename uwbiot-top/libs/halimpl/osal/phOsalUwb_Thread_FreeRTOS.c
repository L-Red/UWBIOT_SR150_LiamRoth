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
 * \file  phOsalUwb_Thread_FreeRTOS.c
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
#include "FreeRTOSConfig.h"

/*
****************************** Macro Definitions ******************************
*/
/** \ingroup grp_osal_uwb
    Value indicates Failure to suspend/resume thread */
#define PH_OSALUWB_THREADPROC_FAILURE (0xFFFFFFFF)

/*
*************************** Function Definitions ******************************
*/

UWBSTATUS phOsalUwb_Thread_Create(void **hThread, pphOsalUwb_ThreadFunction_t pThreadFunction, void *pParam)
{
    UWBSTATUS wCreateStatus                         = UWBSTATUS_SUCCESS;
    phOsalUwb_ThreadCreationParams_t *pThreadParams = (phOsalUwb_ThreadCreationParams_t *)pParam;

    if ((NULL == hThread) || (NULL == pThreadFunction) || (NULL == pThreadParams)) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Indicate the thread is created without message queue */
        if (pdPASS == xTaskCreate(pThreadFunction,
                          (const char *)&(pThreadParams->taskname[0]),
                          pThreadParams->stackdepth,
                          pThreadParams->pContext,
                          (tskIDLE_PRIORITY + pThreadParams->priority),
                          (TaskHandle_t *)hThread)) {
            configASSERT(*hThread);
        }
        else {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_CREATION_ERROR);
        }
    }
#if !(configUSE_PREEMPTION)
    if (uxTaskPriorityGet(NULL) < tskIDLE_PRIORITY + pThreadParams->priority) {
        taskYIELD();
    }
#endif

    return wCreateStatus;
}

void phOsalUwb_Thread_Join(UWBOSAL_TASK_HANDLE hThread)
{
#if ((INCLUDE_eTaskGetState == 1) || (configUSE_TRACE_FACILITY == 1) || (INCLUDE_xTaskAbortDelay == 1))
    eTaskState taskState;
    taskState = eTaskGetState(hThread);
    while (taskState < eDeleted) {
        vTaskDelay(pdMS_TO_TICKS(10));
        taskState = eTaskGetState(hThread);
    }
#endif
    return;
}

UWBSTATUS phOsalUwb_Thread_Delete(UWBOSAL_TASK_HANDLE hThread)
{
    UWBSTATUS wDeletionStatus = UWBSTATUS_SUCCESS;
    vTaskDelete(hThread);
    return wDeletionStatus;
}

UWBOSAL_TASK_HANDLE phOsalUwb_GetTaskHandle(void)
{
    return (xTaskGetCurrentTaskHandle());
}

void phOsalUwb_TaskResume(UWBOSAL_TASK_HANDLE xTaskToResume)
{
    vTaskResume(xTaskToResume);
}

void phOsalUwb_TaskSuspend(UWBOSAL_TASK_HANDLE xTaskToSuspend)
{
    vTaskSuspend(xTaskToSuspend);
}

void phOsalUwb_TaskStartScheduler(void)
{
    vTaskStartScheduler();
}

void phOsalUwb_Thread_Context_Switch(void)
{
    taskYIELD();
}

/** @} */
