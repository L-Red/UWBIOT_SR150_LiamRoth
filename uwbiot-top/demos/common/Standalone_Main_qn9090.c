/* Copyright 2019,2020,2022,2023 NXP
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

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if !defined(UWBIOT_APP_BUILD__DEMO_PNP)
#include <stdio.h>

#include <uwb_board.h>

#if defined(QN9090_SERIES)

// Kinetis includes
#include "board.h"
#include "peripherals.h"

#include "UWBT_PowerMode.h"
#include "LED.h"
#include "GPIO.h"
#include "AppInternal.h"
#include "AppRecovery.h"
#include "phOsalUwb.h"
#include "UWBT_BuildConfig.h"

#if UWBFTR_SE_SE051W
#define HAVE_KSDK
#include "ex_sss_main_inc_ksdk.h"
#endif

#if UWBFTR_SE_SN110
#include <firmware_sn100.h>
#endif

int gapp_argc              = 1;
const char gapp_argv[2][4] = {"NA", ""};

UWBOSAL_TASK_HANDLE testTaskHandle;
void UWBDemo_Init();
UWBOSAL_TASK_HANDLE uwb_demo_start(void);

#if UWBFTR_SE_SN110
int8_t GeneralState = 0;
#endif // UWBFTR_SE_SN110

/* Allocate the memory for the heap. */
uint8_t __attribute__((section(".bss.$SRAM1"))) ucHeap[configTOTAL_HEAP_SIZE];
/*
 * @brief   Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
#if UWBFTR_SE_SE051W
    ex_sss_main_ksdk_bm();
#else
    hardware_init();
#endif // UWBFTR_SE_SE051W

    LED_Init();
    GPIO_InitTimer();

    UWBDemo_Init();

    Start_AppRecoveryTask();

#if UWBIOT_UWBD_SR040
    /* Configure accelerometer */
    ACCEL_Configure();
#endif // UWBIOT_UWBD_SR040

    testTaskHandle = uwb_demo_start();

    phOsalUwb_TaskStartScheduler();
    return 0;
}

#ifndef BOARD_VERSION
#error BOARD_VERSION must be defined
#endif
#ifndef SHIELD
#error SHIELD must be defined
#endif
#ifndef RHODES_V4
#error RHODES_V4 must be defined
#endif

#endif // defined(QN9090_SERIES)

#endif /* !defined(UWBIOT_APP_BUILD__DEMO_PNP) */
