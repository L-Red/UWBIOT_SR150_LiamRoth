/* Copyright 2021-2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"

#include "UwbApi.h"
#include <AppInternal.h>
#include "phOsalUwb.h"

#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION

#if defined(CPU_JN518X)
#include "ApplMain.h"
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#endif // cPWR_UsePowerDownMode
#endif // CPU_JN518X
#include "TLV_Types_i.h"

extern uint8_t ucHeap[configTOTAL_HEAP_SIZE];

/********************************************************************************/
#define DEMO_UWB_BLE_SR150i_TASK_SIZE 1024
#define DEMO_UWB_BLE_SR150i_TASK_NAME "DemoDataTransferRx"
#define DEMO_UWB_BLE_SR150i_TASK_PRIO 4

#if DEMO_UWB_BLE_SR150i_TASK_SIZE > 1024
#pragma message("DEMO_UWB_BLE_SR150i_TASK_SIZE > 1024 : BLE demos will not work")
#endif // DEMO_UWB_BLE_SR150i_TASK_SIZE

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Demo Nearby Interaction (BLE tracker)");
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

#if DEMO_UWB_BLE_SR150i_TASK_SIZE > 1024
    status = UWBAPI_STATUS_OK;
    LOG_W("Skipping demo as DEMO_UWB_BLE_SR150i_TASK_SIZE > 1024");
#else

    if (!tlvBuilderInit()) {
        LOG_E("Failed to initialize TLV builder");
        goto initialize_ble;
    }

    if (!tlvMngInit()) {
        LOG_E("Failed to initialize manager");
        goto initialize_ble;
    }

initialize_ble:
#if defined(CPU_JN518X)
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    PWR_Init();
    /* Inform the PWR module to keep the FreeRTOS heap in retention */
    PWR_vAddRamRetention((uint32_t)&ucHeap[0], sizeof(ucHeap));
#endif
    main_task(0);
#endif //CPU_JN518X
#endif // DEMO_UWB_BLE_SR150i_TASK_SIZE
    UWBIOT_EXAMPLE_END(status);
}

/*
 * Interface which will be called from Main to create the required task with its own parameters.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;
    threadparams.stackdepth   = DEMO_UWB_BLE_SR150i_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_UWB_BLE_SR150i_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_UWB_BLE_SR150i_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
