/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019,2022 NXP
 *
 * \file
 *
 * This file is the app configuration file which is pre included.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

/*! *********************************************************************************
 *  Board Configuration
 ********************************************************************************** */
#define USE_DK6_BOARD 0
#if USE_DK6_BOARD
#define BLE_ONLY       1
#define USE_SHELF_MODE 0
#define gBtnSupported_d \
    0 // DK6 board using pin20 for SFIPF data io, when demo OTA on dk6 board, please disable BtnSupport
#else
#define USE_SHELF_MODE  0
#define gBtnSupported_d 0
#endif
#define gBtn_Count_c          0
#define KEY_PRESS_CHECK_500MS 500

/* Debug only */
#define gLoggingActive_d   0
#define DBG_PWR            0
#define DBG_TMR            0
#define PostStepTickAssess 1
#define SYSTICK_DBG        0

#define gLEDsOnTargetBoardCnt_c 0

/* Defines the number of available keys for the keyboard module */
#define gKBD_KeysCount_c 0

// #define gUartDebugConsole_d 0

/*! *********************************************************************************
 *  App Configuration
 ********************************************************************************** */
/* Watch dog timer */
#ifdef DEBUG
#define APP_WWDT_ENABLE 0 /* 0 ==> disabled. 1 ==> enabled */
#elif defined(NDEBUG)
#define APP_WWDT_ENABLE 0 /* 0 ==> disabled. 1 ==> enabled */
#else
#error "Define either DEBUG or NDEBUG"
#endif

/* gatt_uuid128.h defined by application or BLE stack
 * 0 -> Defined by BLE stack
 * 1 -> Defined by application
 */
#define gGattDbApplicationDefined_d 1

#define gLEDSupported_d 0
/*! Number of connections supported by the application */
#define gAppMaxConnections_c 5

/*! Enable/disable use of uart debug log */
#define gUartDebugConsole_d 1

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d 0

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d 0

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d 0

#define gPasskeyValue_c 999999

/* Enable HS Clock to support 2Mbps PHY mode setting */
#define gBleUseHSClock2MbpsPhy_c 0

/* Enable/Disable FSCI Low Power Commands*/
#define gFSCI_IncludeLpmCommands_c 0

/*! ADV interval in Slots : 1600 matches to 1s  */
#define gAppAdvertisingInterval 1600

/*! ADV connectable - Set to 0 if ADV is not connectable*/
#define gAppConnectableAdv_d 0

#define gTimerMgrUseLpcRtc_c    1
#define gTimerMgrLowPowerTimers 1

/*! *********************************************************************************
 *  Framework Configuration
 ********************************************************************************** */
/* Use the Lowpower module from the framework :
 * Default lowpower mode for the lowpower module is WFI only
 * For full power down mode, cPWR_FullPowerDownMode shall be set to 1
 */

/* doc:start:disable-lpm */
/*
 * For PNP Mode at higher baudrate than default one (3000000) on QN9090 (MKShield/Rhodes4)
 * low power mode has to be disabled, by setting following two macros
 * (cPWR_UsePowerDownMode and cPWR_FullPowerDownMode) to 0
 * The "clean + build re-compilation is needed for the changes to take place in binary.
 */

/* Enable Power down modes
 * Need cPWR_UsePowerDownMode to be set to 1 first
 */

#define cPWR_UsePowerDownMode  1
#define cPWR_FullPowerDownMode 1
/* doc:end:disable-lpm */

/* doc:start:enable-HW-flow */
/*
 * Pnp support HW flow control support
 * To enable the HW flow control support set HW_FLOW_CONTROL_SUPPORT to 1
 * By default HW flow control is disabled
 * The "clean + build re-compilation is needed for the changes to take place in binary.
*/

/* Rhodes 4 HW flow control support */
#define HW_FLOW_CONTROL_SUPPORT 0
/* doc:end:enable-HW-flow */

/*if Power down mode is disabled Need mAppIdleHook_c to be set to 1*/
#if !(cPWR_UsePowerDownMode) || !(cPWR_FullPowerDownMode)

#define mAppIdleHook_c 1

#endif

/* Settings that apply only when cPWR_FullPowerDownMode is set */
#if cPWR_FullPowerDownMode

#define gSwdWakeupReconnect_c 0

#if USE_SHELF_MODE
#define cPWR_PowerDown_RamOffOsc32kOff 6
#endif

/* Prevent from disabling the power down mode when switching in connected mode
 * If not set, the powerdown mode will be disabled when going into connected mode */
#define cPWR_NoPowerDownDisabledOnConnect 1

/* Go to power down even if a timer (not lower timer) is running
 * Warning : timers are lost and not yet recovered on wakeup  */
#define cPWR_DiscardRunningTimerForPowerDown 1

/* Use Timebase drift compensation algo in Linklayer when power Down mode is
 * enabled (cPWR_UsePowerDownMode set to 3)
 * To be used with FRO32K (gClkUseFro32K)
 * Allow to decrease Always ON LDO voltage in powerdown for additional power saving
 * in connected mode */
#define gPWR_UseAlgoTimeBaseDriftCompensate gClkUseFro32K

/* Switch CPU clock to 48MHz FRO at startup - 32MHz (FRO or XTAL) default */
#define gPWR_CpuClk_48MHz 0

/*! Reduce the system clock frequency for  CPU / AHB bus/ SRAM during WFI
     This is particularly useful when the CPU is inactive during the Link layer events.
     However, this reduces the number of possible white-list and RAL entries that can be resolved.
     The radio timings need to be trimmed to allow WFI frequency scaling below
      0 : disabled
      32 : reduced down to 32MHz  (no effect if gPWR_CpuClk_48MHz is disabled)
      24 : reduced down to 24MHz : FRO48M clock divided by 2
      16 : reduced down to 16MHz : XTAL32M and clock divided by 2 or 48M divided by 3:  Single white-list entry
      12 : reduced down to 12MHz :
       8 : reduced down to 8MHz  :
       6 : reduced down to 6MHz  : FRO48M divided by 8
       4 : reduced down to 4MHz  : FRO48M divided by 12 or XTAL32M divided by 8    */
#define gPWR_FreqScalingWFI 0

/* Added for Power Optimization */
#define gPWR_SmartFrequencyScaling 2
#define gPWR_Xtal32MDeactMode      1
#define gPWR_LpOscOptim            1

/*! BLE Link Layer Fast Correct feature allows a one slot 625us shorter wake up advance */
#define gBleLL_FastCorrect_d (1)

#define gPWR_BleLL_EarlyEnterDsm (1)

/* Control the power saving of the Flash Controller when CPU is in WFI state
 * 1 - Power down the flash controller and Switch OFF the 48Mhz clock to Flash Controller
 * 2 - Not supported */
//#define gPWR_FlashControllerPowerDownInWFI   0

/* Apply LDO MEM voltage to 0.9V instead of 1.1V in power down
 * Shall not be used over full operating range of the device  */
#define gPWR_LDOMEM_0_9V_PD 1

/* Reduce LDO Core/Mem/Ao Voltage down to 1.0V during Active
 * except at wakeup and when increasing CPU clock */
#define gPWR_LdoVoltDynamic 1

/* Optimize Advertising interslot interval in us - Default is 1500us if not set */
#define gPWR_AdvertisingInterSlotInt 1328

/* Not supported */
#define gPWR_SerialUartRxWakeup 0

#define gLPM_UWB_RANGING 0
#endif

/* Use FRO32K instead of XTAL32K in active and power down modes - XTAL32K no longer required */
#define gClkUseFro32K 0

/* Enable/disable NTAG feature */
#define gAppNtagSupported_d 0

/* This setting impacts Slave RX window widening and consequently power consumption.
* Customer needs to charaterize the 32Khz crytal  with regards to temperature and
* aging for better power saving.  Keep it to safe value (500ppm) if not done
*/
#define gLocalSleepClkAccuracyPpm 190
/*! *********************************************************************************
 *  Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d 0

/* If set, enables Kmod data saving in PDM (requires PDM library) */
#define gRadioUsePdm_d 1

/* gUsePdm_d is not synonymous to gAppUseNvm_d because PDM is used by Radio driver independantly from NVM */
#define gUsePdm_d (gAppUseBonding_d | gAppUsePairing_d | gRadioUsePdm_d)

/* Defines Num of Serial Manager interfaces */
#define gSerialManagerMaxInterfaces_c 0

/* Defines Size for Timer Task*/
#define gTmrTaskStackSize_c 700

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define AppPoolsDetails_c                                                                   \
    _block_size_ 32 _number_of_blocks_ 60 _eol_ _block_size_ 64 _number_of_blocks_ 30 _eol_ \
        _block_size_ 128 _number_of_blocks_ 30 _eol_ _block_size_ 512 _number_of_blocks_ 20 _eol_

/* Defines number of timers needed by the application */
#define gTmrApplicationTimers_c 4

/* Defines number of timers needed by the protocol stack */
#define gTmrStackTimers_c 5

/* Set this define TRUE if the PIT frequency is an integer number of MHZ */
#define gTMR_PIT_FreqMultipleOfMHZ_d 0

/* Enable/Disable Periodical Interrupt Timer */
#define gTMR_PIT_Timestamp_Enabled_d 0

/* Enables / Disables the precision timers platform component */
#define gTimestamp_Enabled_d 0

/* Enable/Disable PANIC catch */
#define gUsePanic_c 1

/* Keyboard not supported */
#define gKeyBoardSupported_d 0

/*! *********************************************************************************
 *  RTOS Configuration
 ********************************************************************************** */
/* Defines the RTOS used */
#define FSL_RTOS_FREE_RTOS 1

/*defines the tickless mode*/
#define mAppUseTickLessMode_c 1

/* If the gTimestampUseWtimer flag is only used for the tickless mode, in order
 * to reduce the power consumption this flag could be enabled so that only the
 * wtimer1 is used for both os activity scheduling and calculation of the total
 * sleep duration.
 */
//#define gSystickUseWtimer1ForSleepDuration 1

/* Defines number of OS events used */
#define osNumberOfEvents 5

/* Defines main task stack size */
#define gMainThreadStackSize_c 1100

/* Defines controller task stack size */
#define gControllerTaskStackSize_c 2048

/* Defines total heap size used by the OS - 12k */
#define gTotalHeapSize_c 12288
/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
/* Configure the maximum number of bonded devices. If maximum bonded devices reached,
 * user should remove an old bonded device to store new bonded information. Otherwise,
 * demo application will pair with new deivce with No Bonding type.
 */
#define gMaxBondedDevices_c     16
#define gMaxResolvingListSize_c 6

/*! *********************************************************************************
 *  NVM Module Configuration - gAppUseNvm_d shall be defined aboved as 1 or 0
 ********************************************************************************** */
/* USER DO NOT MODIFY THESE MACROS DIRECTLY. */
#define gAppMemPoolId_c 0
#if gAppUseNvm_d
#define gNvmMemPoolId_c 1
#if gUsePdm_d
#define gPdmMemPoolId_c 2
#endif
#else
#if gUsePdm_d
#define gPdmMemPoolId_c 1
#endif
#endif

#if gAppUseNvm_d
#define gNvmOverPdm_d 1
/* Defines NVM pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define NvmPoolsDetails_c                                                          \
    _block_size_ 32 _number_of_blocks_ 20 _pool_id_(gNvmMemPoolId_c)               \
        _eol_ _block_size_ 60 _number_of_blocks_ 10 _pool_id_(gNvmMemPoolId_c)     \
            _eol_ _block_size_ 80 _number_of_blocks_ 10 _pool_id_(gNvmMemPoolId_c) \
                _eol_ _block_size_ 100 _number_of_blocks_ 2 _pool_id_(gNvmMemPoolId_c) _eol_

/* configure NVM module */
#define gNvStorageIncluded_d         (1)
#define gNvFragmentation_Enabled_d   (1)
#define gUnmirroredFeatureSet_d      (0)
#define gNvRecordsCopiedBufferSize_c (512)
#else
#define NvmPoolsDetails_c
#endif

#if gUsePdm_d
#define gPdmNbSegments 32 /* number of sectors contained in PDM storage */

#define PdmInternalPoolsDetails_c                                                                            \
    _block_size_ 512 _number_of_blocks_ 2 _pool_id_(gPdmMemPoolId_c) _eol_ _block_size_(gPdmNbSegments * 12) \
        _number_of_blocks_ 1 _pool_id_(gPdmMemPoolId_c) _eol_
#else
#define PdmInternalPoolsDetails_c
#endif

/*! *********************************************************************************
 *  Memory Pools Configuration
 ********************************************************************************** */

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.
 * DO NOT MODIFY THIS DIRECTLY. CONFIGURE AppPoolsDetails_c
 * If gMaxBondedDevices_c increases, adjust NvmPoolsDetails_c
*/

#if gAppUseNvm_d
#define PoolsDetails_c AppPoolsDetails_c NvmPoolsDetails_c PdmInternalPoolsDetails_c
#elif gUsePdm_d /* Radio drivers uses PDM but no NVM over PDM */
#define PoolsDetails_c AppPoolsDetails_c PdmInternalPoolsDetails_c
#else
#define PoolsDetails_c AppPoolsDetails_c
#endif

/*! *********************************************************************************
 *  Flag dependencies
 *    Define flags needed by other features to be functional
 *    DO NOT MODIFIED
 ********************************************************************************** */

#if (gAppUseBonding_d) && (!gAppUsePairing_d)
#error "Enable pairing to make use of bonding"
#endif

#if mAppUseTickLessMode_c
#if defined(gSystickUseWtimer1ForSleepDuration) && gSystickUseWtimer1ForSleepDuration
/* Not required for tickless if gSystickUseWtimer1ForSleepDuration is set to 1 */
#define gTimestampUseWtimer_c 0
#else
/* If Tickless mode is wanted, it is required to base timing on an accurate timebase
 * so counting on 32kHz tick */
#define gTimestampUseWtimer_c 1
#endif
#endif

#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
