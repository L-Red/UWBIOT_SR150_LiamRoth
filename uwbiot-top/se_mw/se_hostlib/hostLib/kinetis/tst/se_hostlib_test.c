/* Copyright 2018 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#define CHECK_A71CH_CONNECTION      1

#define CHECK_TIMER_CALIBRATION     1

#if defined(IMX_RT)
#define DEBUGGING_1050_TIMER        1
#endif

#include "app_boot.h"
#include "ax_api.h"
#include "axHostCrypto.h"
#include "sm_timer.h"
#include "sm_printf.h"
#include "board.h"


#if CHECK_TIMER_CALIBRATION

    #if FSL_FEATURE_SIM_OPT_HAS_FTM
    #   include "fsl_ftm.h"
    #elif FSL_FEATURE_SIM_OPT_HAS_TPM
    #   include "fsl_tpm.h"
    #elif FSL_FEATURE_SOC_PIT_COUNT
    #   include "fsl_pit.h"
    #elif FSL_FEATURE_SOC_RIT_COUNT
    #include "fsl_rit.h"
    #endif


#endif /* CHECK_TIMER_CALIBRATION */


#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * 1.1.0 : Added support for PIT
 */
#define TST_APP_VERSION "1.1.1"

#ifndef LED_RED_OFF
#   define LED_RED_OFF()
#   define LED_GREEN_OFF()
#   define LED_BLUE_OFF()
#   define LED_RED_ON()
#   define LED_GREEN_ON()
#   define LED_BLUE_ON()
#   define LED_RED_TOGGLE()
#   define LED_GREEN_TOGGLE()
#   define LED_BLUE_TOGGLE()
#endif

#define LOOP_COUNT          3
#define DELAY_MILLI_SECONDS 300


#if FSL_FEATURE_SIM_OPT_HAS_FTM

/* The Flextimer instance/channel used for board */
#define BOARD_FTM_BASEADDR FTM3

/* Interrupt number and interrupt handler for the FTM instance used */
#define BOARD_FTM_IRQ_NUM FTM3_IRQn
#define BOARD_FTM_HANDLER FTM3_IRQHandler

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/4)

#elif FSL_FEATURE_SIM_OPT_HAS_TPM

/* define instance */
#define BOARD_TPM TPM2
/* Interrupt number and interrupt handler for the TPM instance used */
#define BOARD_TPM_IRQ_NUM TPM2_IRQn
#define BOARD_TPM_HANDLER TPM2_IRQHandler
/* Get source clock for TPM driver */
#define TPM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_McgFllClk)/4)

#elif FSL_FEATURE_SOC_PIT_COUNT
#define BOARD_PIT_HANDLER PIT_IRQHandler
#define PIT_IRQ_ID PIT_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_IpgClk)

#elif FSL_FEATURE_SOC_RIT_COUNT
#define RIT_IRQ_ID RIT_IRQn
/* Get source clock for RIT driver */
#define RIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_CoreSysClk)
//#define LED_INIT() LED1_INIT(LOGIC_LED_ON)
//#define LED_TOGGLE() LED1_TOGGLE()
#define BOARD_RIT_HANDLER RIT_IRQHandler

#endif



/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if FSL_FEATURE_SIM_OPT_HAS_FTM
static void init_StartFTM_Timer(void);
#elif FSL_FEATURE_SIM_OPT_HAS_TPM
static void init_StartTPM_Timer(void);
#elif   FSL_FEATURE_SOC_PIT_COUNT
static void init_StartPIT_Timer(void);
#elif FSL_FEATURE_SOC_RIT_COUNT
static void init_StartRIT_Timer();
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t milisecondCounts = 0U;
#if   FSL_FEATURE_SOC_PIT_COUNT
volatile bool pitIsrFlag = false;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

void checkTimerCalibration(void);

/*!
 * @brief Main function
 */
int main(void)
{

    app_boot_Init();

#if DEBUGGING_1050_TIMER
    USER_LED_INIT(LOGIC_LED_OFF);
#endif
    PRINTF( "a71ch HostLibrary test application (Rev %s)\r\n", TST_APP_VERSION);
    PRINTF( "**********************************************\r\n");


#if CHECK_TIMER_CALIBRATION
    checkTimerCalibration();
#endif

#if CHECK_A71CH_CONNECTION
{
    U16 connectStatus = 0;
    SmCommState_t commState;

    PRINTF("Connect to A71CH-SM. Chunksize at link layer = %d.\r\n", MAX_CHUNK_LENGTH_LINK);
    connectStatus = app_boot_Connect(&commState, NULL);

    if (connectStatus != 0)
    {
        PRINTF("Select failed. SW = 0x%04X\r\n", connectStatus);
        LED_RED_ON(); LED_BLUE_OFF(); LED_GREEN_OFF();
        return 5;
    }
    PRINTF("==========FINISHED=========\r\n");
    LED_GREEN_ON(); LED_BLUE_OFF(); LED_RED_OFF();
}
#endif
    while (1)
    {
#if !defined(IMX_RT)
        __WFI();
#endif
    }
}

#if CHECK_TIMER_CALIBRATION

void checkTimerCalibration() {
    int i;
    volatile uint32_t startTimerCount = 0;
    volatile uint32_t endTimerCount = 0;
    uint32_t delayTested = 0;
    uint32_t elapsedTime = 0;
    int32_t diffTime = 0;
    int32_t tolerance = 0;

    PRINTF("How about Testing the millisecond Timer %d \r\n", MAX_CHUNK_LENGTH_LINK);

#if FSL_FEATURE_SIM_OPT_HAS_FTM
    init_StartFTM_Timer();
#elif FSL_FEATURE_SIM_OPT_HAS_TPM
    init_StartTPM_Timer();
#elif   FSL_FEATURE_SOC_PIT_COUNT
    init_StartPIT_Timer();
#elif FSL_FEATURE_SOC_RIT_COUNT
    init_StartRIT_Timer();
#endif

    delayTested = (2 * LOOP_COUNT * DELAY_MILLI_SECONDS );
    PRINTF("Set timer for %lu milliSeconds\r\n", delayTested);

    {

        LED_GREEN_OFF();
        LED_RED_ON();

        __disable_irq();
        startTimerCount = milisecondCounts;
        __enable_irq();

        for ( i = 0 ; i < LOOP_COUNT ; i ++ ) {

            sm_sleep(DELAY_MILLI_SECONDS);
            LED_RED_TOGGLE();
            sm_sleep(DELAY_MILLI_SECONDS);
        }
        __disable_irq();
         endTimerCount = milisecondCounts;
         __enable_irq();
         LED_RED_OFF();
         LED_GREEN_ON();
         elapsedTime = endTimerCount - startTimerCount;

         PRINTF("sm_sleep() for %lu ms\r\n", delayTested);
         PRINTF("Platfrom/IC Timer measured : %lu\r\n", elapsedTime);
         diffTime = elapsedTime - delayTested;
         PRINTF("\tDelta = %lu ms\r\n", diffTime);
         if ( diffTime < 0 )
             tolerance =((-diffTime)*1000) / delayTested;
         else
             tolerance =(diffTime*1000) / delayTested;

         PRINTF("\tPercent Change: %li%%\r\n", (tolerance/10));

         if((tolerance/10)<=10)
         {
             PRINTF("==== Milli Seconds Test Passed\r\n");
         }
         else
         {
             PRINTF("!!!!! Milli Seconds Test Failed!!!\r\n");
             PRINTF("\tmilliSeconds Timer is %s by %li%%\r\n", ((diffTime < 0)?"Faster":"Slower"),(tolerance/10));
         }

         PRINTF( "**********************************************\r\n\r\n\r\n\r\n");
    }


    PRINTF( "microsecond delay tests start\r\n\r\n\r\n\r\n");
    PRINTF( "**********************************************\r\n");

    delayTested = (3 * LOOP_COUNT * DELAY_MILLI_SECONDS );
    PRINTF("Set timer for %lu milliSeconds\r\n", delayTested);
    LED_RED_OFF();
    LED_GREEN_OFF();
    {
        int i, j;
        __disable_irq();
        startTimerCount = milisecondCounts;
        __enable_irq();

        for ( i = 0 ; i < LOOP_COUNT ; i ++ ) {
            for ( j = 0 ; j < DELAY_MILLI_SECONDS; j++) {
                sm_usleep(1000);
            }
            LED_BLUE_TOGGLE();
            for ( j = 0 ; j < DELAY_MILLI_SECONDS; j++) {
                sm_usleep(1000);
            }
            LED_BLUE_TOGGLE();
            for ( j = 0 ; j < DELAY_MILLI_SECONDS; j++) {
                sm_usleep(1000);
            }
        }


         __disable_irq();
         endTimerCount = milisecondCounts;
         __enable_irq();
         LED_RED_OFF();
         LED_GREEN_ON();
         elapsedTime = endTimerCount - startTimerCount;

         PRINTF("sm_usleep() for %lu ms\r\n", delayTested);
         PRINTF("Platform/IC Timer measured %lu\r\n", elapsedTime);
         diffTime = elapsedTime - delayTested;
         PRINTF("\tDelta = %lu ms\r\n", diffTime);
         if ( diffTime < 0 )
             tolerance =((-diffTime)*1000) / delayTested;
         else
             tolerance =(diffTime*1000) / delayTested;

         PRINTF("\tPercent Change: %li%%\r\n", (tolerance/10));

         if((tolerance/10)<=10)
         {
             PRINTF("==== Micro Seconds Test Passed\r\n");
         }
         else
         {
             PRINTF("!!!!! Micro Seconds Test Failed!!!\r\n");
             PRINTF("\tMicroSecond Timer is %s by %lu%%\r\n", ((diffTime < 0)?"Faster":"Slower"),(tolerance/10));
         }

         PRINTF( "**********************************************\r\n\r\n\r\n\r\n");


    }
    LED_RED_OFF();

    LED_BLUE_ON();

    printf("%s Finished\r\n",__FUNCTION__);

}
#endif /* CHECK_TIMER_CALIBRATION */


#if FSL_FEATURE_SIM_OPT_HAS_FTM && CHECK_TIMER_CALIBRATION
static void init_StartFTM_Timer(void)
{
    ftm_config_t ftmInfo;
    FTM_GetDefaultConfig(&ftmInfo);

    /* Divide FTM clock by 4 */
    ftmInfo.prescale = kFTM_Prescale_Divide_4;

    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    /*
     * Set timer period.
    */
    FTM_SetTimerPeriod(BOARD_FTM_BASEADDR, USEC_TO_COUNT(1000U, FTM_SOURCE_CLOCK));

    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(BOARD_FTM_IRQ_NUM);
    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
}


#elif FSL_FEATURE_SIM_OPT_HAS_TPM && CHECK_TIMER_CALIBRATION
static void init_StartTPM_Timer(void)
{
    tpm_config_t tpmInfo;
    /* Select the clock source for the TPM counter as MCGPLLCLK */
    CLOCK_SetTpmClock(1U);

    TPM_GetDefaultConfig(&tpmInfo);

    /* TPM clock divide by 4 */
    tpmInfo.prescale = kTPM_Prescale_Divide_4;

    /* Initialize TPM module */
    TPM_Init(BOARD_TPM, &tpmInfo);

    /*
     * Set timer period.
     */
    TPM_SetTimerPeriod(BOARD_TPM, USEC_TO_COUNT(1000U, TPM_SOURCE_CLOCK));

    TPM_EnableInterrupts(BOARD_TPM, kTPM_TimeOverflowInterruptEnable);

    EnableIRQ(BOARD_TPM_IRQ_NUM);

    TPM_StartTimer(BOARD_TPM, kTPM_SystemClock);
}
#elif   FSL_FEATURE_SOC_PIT_COUNT && CHECK_TIMER_CALIBRATION
static void init_StartPIT_Timer(void)
{
    /* Structure of initialize PIT */
    pit_config_t pitConfig;

    /*
    * pitConfig.enableRunInDebug = false;
    */
    PIT_GetDefaultConfig(&pitConfig);

    /* Init pit module */
    PIT_Init(PIT, &pitConfig);

    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(500U, PIT_SOURCE_CLOCK));

    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(PIT_IRQ_ID);

    /* Start channel 0 */
    PIT_StartTimer(PIT, kPIT_Chnl_0);

}


#endif

#if FSL_FEATURE_SIM_OPT_HAS_FTM && CHECK_TIMER_CALIBRATION
void BOARD_FTM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, kFTM_TimeOverflowFlag);
    milisecondCounts++;
}

#elif FSL_FEATURE_SIM_OPT_HAS_TPM && CHECK_TIMER_CALIBRATION
void BOARD_TPM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    TPM_ClearStatusFlags(BOARD_TPM, kTPM_TimeOverflowFlag);
    milisecondCounts++;
}

#elif   FSL_FEATURE_SOC_PIT_COUNT && CHECK_TIMER_CALIBRATION

void BOARD_PIT_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    milisecondCounts++;
#if DEBUGGING_1050_TIMER
    USER_LED_TOGGLE();
#endif
}

#endif

#if FSL_FEATURE_SOC_RIT_COUNT && CHECK_TIMER_CALIBRATION

void BOARD_RIT_HANDLER(void)
{
    RIT_ClearStatusFlags(RIT, kRIT_TimerFlag);
    milisecondCounts++;

//  LED_TOGGLE();
}

static void init_StartRIT_Timer()
{
    rit_config_t ritConfig;
    RIT_GetDefaultConfig(&ritConfig);

    /* Init rit module */
    RIT_Init(RIT, &ritConfig);

    /* Set timer period for Compare register. */
    RIT_SetTimerCompare(RIT, USEC_TO_COUNT(1000U, RIT_SOURCE_CLOCK));

    /* Set the register reset to zero when the counter value equals the set period. */
    RIT_ClearCounter(RIT, true);

    /* Start counting */
    RIT_StartTimer(RIT);

    /* Enable at the NVIC */
    EnableIRQ(RIT_IRQ_ID);
}
#endif
