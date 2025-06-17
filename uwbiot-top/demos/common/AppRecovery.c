/* Copyright 2020,2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

/*
 * Recovery.c
 *
 *  Created on: Mar 9, 2020
 *      Author: nxf50460
 */
#include "phUwb_BuildConfig.h"

#include "AppRecovery.h"
#include "AppInternal.h"
#include "phOsalUwb.h"
#if UWBIOT_UWBD_SR1XXT
#include "UwbApi_Proprietary_Fm.h"
#endif

extern UWBOSAL_TASK_HANDLE testTaskHandle;
static UWBOSAL_TASK_HANDLE recoveryTaskHandle;

static intptr_t mErrorHandlerQueue;
phLibUwb_Message_t receive_message;

extern UWBOSAL_TASK_HANDLE uwb_demo_start(void);
static OSAL_TASK_RETURN_TYPE AppRecoveryTask(void *args);
static OSAL_TASK_RETURN_TYPE recovery_handler(errorScenario_t error_scenario);

#if UWBIOT_OS_NATIVE
uint8_t isRecovery = false;
#endif

static OSAL_TASK_RETURN_TYPE AppRecoveryTask(void *args)
{
    LOG_D("Started AppRecoveryTask");

    mErrorHandlerQueue = phOsalUwb_msgget(2);
    if (!mErrorHandlerQueue) {
        Log("Error: main, could not create queue mErrorHandlerQueue\n");
        while (1)
            ;
    }

    while (1) {
        phLibUwb_Message_t message;
        errorScenario_t scenario;
        message.eMsgType = 0;
        if (phOsalUwb_msgrcv(mErrorHandlerQueue, &message, MAX_DELAY) == UWBSTATUS_FAILED) {
            continue;
        }
        if (message.eMsgType != 0) {
            scenario = (errorScenario_t)message.eMsgType;
            recovery_handler(scenario);
        }
    }
}

static OSAL_TASK_RETURN_TYPE recovery_handler(errorScenario_t error_scenario)
{
    switch (error_scenario) {
    case TIME_OUT:
    case APP_CLEANUP:
    case FW_CRASH:
        Log("Recovery Started for Scenario : %d\n", error_scenario);
#if UWBIOT_UWBD_SR1XXT
        if (UwbApi_ShutDown() != UWBAPI_STATUS_OK) {
            LOG_E("recovery handler : UwbApi_ShutDown failed");
        }
#endif /* UWBIOT_UWBD_SR1XXT */
#if UWBIOT_OS_NATIVE
        isRecovery = true;
        pthread_cancel(testTaskHandle);
#elif UWBIOT_OS_FREERTOS
        phOsalUwb_Thread_Delete(testTaskHandle);
        testTaskHandle = uwb_demo_start();
#endif
        break;
    default:
        Log("Recovery Started for Default Scenario. \n");
        break;
    }
}

void Start_AppRecoveryTask()
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    int pthread_create_status = 0;

    threadparams.stackdepth = RECOVERY_TASK_STACK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, RECOVERY_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = RECOVERY_TASK_PRIORITY;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&recoveryTaskHandle, &AppRecoveryTask, &threadparams);
    if (0 != pthread_create_status) {
        Log("AppRecoveryTask created Failed");
    }
}

void Stop_AppRecoveryTask()
{
    phOsalUwb_msgrelease(mErrorHandlerQueue);
    phOsalUwb_Thread_Delete(recoveryTaskHandle);
}

void Handle_ErrorScenario(errorScenario_t scenario)
{
    receive_message.eMsgType = scenario;
    (void)phOsalUwb_msgsnd(mErrorHandlerQueue, &receive_message, NO_DELAY);
}
