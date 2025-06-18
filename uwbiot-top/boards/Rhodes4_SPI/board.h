/*
 * Copyright 2019-2022,2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include <stdio.h>

#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_clock.h"
#include "psector_api.h"
#include "core_cm4.h"
#include "board_utility.h"
#include "fsl_debug_console.h"
#include "fsl_adc.h"
#include "fsl_flash.h"
#include "fsl_power.h"
#include "fsl_gpio.h"
#include "fsl_xcvr.h"
#include "fsl_wwdt.h"
#include "fsl_inputmux.h"
#include "fsl_spifi.h"
#include "fsl_spi.h"
#include "fsl_spi_dma.h"
#if (FSL_FEATURE_SOC_PWM_COUNT < 1)
#include "fsl_pwm.h"
#endif
#include "fsl_pint.h"
#include "fsl_os_abstraction.h"
#include "fsl_usart_dma.h"
#include "fsl_i2c.h"
#if defined(SDK_OS_FREE_RTOS) && SDK_OS_FREE_RTOS == 1
#include "fsl_i2c_freertos.h"
#endif
#include "fsl_gpio.h"
#if (defined(gLoggingActive_d) && (gLoggingActive_d != 0)) || (defined(gAdcUsed_d) && (gAdcUsed_d != 0))
#include "fsl_wtimer.h"
#endif

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#ifndef BOARD_NAME
#define BOARD_NAME "DK6"
#endif

#define IOCON_PIO_DIGITAL_EN     0x80u   /*!<@brief Enables digital function */
#define IOCON_PIO_FUNC0          0x00u   /*!<@brief Selects pin function 0 */
#define IOCON_PIO_INPFILT_OFF    0x0100u /*!<@brief Input filter disabled */
#define IOCON_PIO_INV_DI         0x00u   /*!<@brief Input function is not inverted */
#define IOCON_PIO_MODE_PULLUP    0x00u   /*!<@brief Selects pull-up function */
#define IOCON_PIO_OPENDRAIN_DI   0x00u   /*!<@brief Open drain is disabled */
#define IOCON_PIO_SLEW0_STANDARD 0x00u   /*!<@brief Standard mode, output slew rate control is disabled */
#define IOCON_PIO_SLEW1_STANDARD 0x00u   /*!<@brief Standard mode, output slew rate control is disabled */
#define IOCON_PIO_SSEL_DI        0x00u   /*!<@brief SSEL is disabled */

#define IOCON_PIO_DIGITAL_EN    0x80u   /*!<@brief Enables digital function */
#define IOCON_PIO_FUNC7         0x07u   /*!<@brief Selects pin function 7 */
#define IOCON_PIO_INPFILT_OFF   0x0100u /*!<@brief Input filter disabled */
#define IOCON_PIO_INV_DI        0x00u   /*!<@brief Input function is not inverted */
#define IOCON_PIO_MODE_INACT    0x10u   /*!<@brief No addition pin function */
#define IOCON_PIO_MODE_PULLUP   0x00u   /*!<@brief Selects pull-up function */
#define IOCON_PIO_MODE_PULLDOWN 0x18u   /*!<@brief Selects pull-down function */
#define IOCON_PIO_OPENDRAIN_DI  0x00u   /*!<@brief Open drain is disabled */
#define IOCON_PIO_SLEW0_FAST    0x20u   /*!<@brief Fast mode, slew rate control is enabled */
#define IOCON_PIO_SLEW1_FAST    0x0200u /*!<@brief Fast mode, slew rate control is enabled */
#define IOCON_PIO_SSEL_DI       0x00u   /*!<@brief SSEL is disabled */

#define IOCON_PIO_DIGITAL_EN     0x80u   /*!<@brief Enables digital function */
#define IOCON_PIO_FUNC2          0x02u   /*!<@brief Selects pin function 2 */
#define IOCON_PIO_INPFILT_OFF    0x0100u /*!<@brief Input filter disabled */
#define IOCON_PIO_INV_DI         0x00u   /*!<@brief Input function is not inverted */
#define IOCON_PIO_MODE_INACT     0x10u   /*!<@brief No addition pin function */
#define IOCON_PIO_OPENDRAIN_DI   0x00u   /*!<@brief Open drain is disabled */
#define IOCON_PIO_SLEW0_STANDARD 0x00u   /*!<@brief Standard mode, output slew rate control is disabled */
#define IOCON_PIO_SLEW1_STANDARD 0x00u   /*!<@brief Standard mode, output slew rate control is disabled */
#define IOCON_PIO_SSEL_DI        0x00u   /*!<@brief SSEL is disabled */

#define RV4_I2CM_BASE  ((I2C_Type *)I2C0)
#define RV4_I2CM_SPEED (1000000) //1MHz
#define RV4_I2CM_IRQN  FLEXCOMM2_IRQn

/*! @brief The manufacturer name */
#define MANUFACTURER_NAME "NXP"
#ifndef gUartDebugConsole_d
#define gUartDebugConsole_d 1
#endif
#ifndef gUartAppConsole_d
#define gUartAppConsole_d 0
#endif
#ifndef gOTA_externalFlash_d
#define gOTA_externalFlash_d 0
#endif
#ifndef OLD_XTAL32M_CAL
#define OLD_XTAL32M_CAL 0
#endif
#ifndef gOtaEepromPostedOperations_d
#define gOtaEepromPostedOperations_d 0
#endif
#ifndef gDbgUseRfDiagIos
#define gDbgUseRfDiagIos 0
#endif
#ifndef PRELOAD_PFLASH
#define PRELOAD_PFLASH 0
#endif
#ifndef RdWrLogsEnable
#define RdWrLogsEnable 0
#endif

#if defined gLoggingActive_d && (gLoggingActive_d > 0)

#ifndef DBG_ADC
#define DBG_ADC 0
#endif

#ifndef DBG_INIT
#define DBG_INIT 0
#endif

#define ADC_DBG_LOG(fmt, ...)                                                      \
    if (DBG_ADC)                                                                   \
        do {                                                                       \
            DbgLogAdd(__FUNCTION__, fmt, VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__); \
        } while (0);
#define INIT_DBG_LOG(fmt, ...)                                                     \
    if (DBG_INIT)                                                                  \
        do {                                                                       \
            DbgLogAdd(__FUNCTION__, fmt, VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__); \
        } while (0);

#else
#define ADC_DBG_LOG(...)
#define INIT_DBG_LOG(...)
#endif

/* Connectivity */
#ifndef APP_SERIAL_INTERFACE_TYPE
#define APP_SERIAL_INTERFACE_TYPE (gSerialMgrUsart_c)
#endif

#ifndef APP_SERIAL_INTERFACE_INSTANCE
#define APP_SERIAL_INTERFACE_INSTANCE 1
#endif

#ifndef DEBUG_SERIAL_INTERFACE_INSTANCE
#define DEBUG_SERIAL_INTERFACE_INSTANCE 0
#endif

#ifndef APP_SERIAL_INTERFACE_SPEED
#define APP_SERIAL_INTERFACE_SPEED (3000000U)
#endif

#define IOCON_QSPI_MODE_FUNC (7U)

#define IOCON_USART0_TX_PIN      (8U)
#define IOCON_USART0_RX_PIN      (9U)
#define IOCON_DBG_UART_MODE_FUNC (2U)

#define IOCON_USART1_TX_PIN         (10U)
#define IOCON_USART1_RX_PIN         (11U)
#define IOCON_HOSTIF_UART_MODE_FUNC (2U)

#define IOCON_SWCLK_PIN     (12U)
#define IOCON_SWDIO_PIN     (13U)
#define IOCON_SWD_MODE_FUNC (2U) /* no choice for SWD */

#define IOCON_SPIFI_CS_PIN    (16U)
#define IOCON_SPIFI_CLK_PIN   (18U)
#define IOCON_SPIFI_IO0_PIN   (19U)
#define IOCON_SPIFI_IO1_PIN   (21U)
#define IOCON_SPIFI_IO2_PIN   (20U)
#define IOCON_SPIFI_IO3_PIN   (17U)
#define IOCON_SPIFI_MODE_FUNC (7U)

#define BOARD_SPIFI_CLK_RATE (8000000UL)

/* Select flash to use */
#if gOTA_externalFlash_d == 1
#define gEepromType_d gEepromDevice_MX25R8035F_c
#else
#if defined(gEepromDevice_InternalFlash_c)
#define gEepromType_d gEepromDevice_InternalFlash_c
#endif //gEepromDevice_InternalFlash_c
#endif

#define BOARD_USART_IRQ(x)         USART##x_IRQn
#define BOARD_USART_IRQ_HANDLER(x) USART##x_IRQHandler
#define BOARD_UART_BASEADDR(x)     (uint32_t) USART##x
#define BOARD_UART_RESET_PERIPH(x) kUSART##x_RST_SHIFT_RSTn
#define BOARD_UART_RX_PIN(x)       IOCON_USART##x_RX_PIN
#define BOARD_UART_TX_PIN(x)       IOCON_USART##x_TX_PIN

#if gUartAppConsole_d
#if APP_SERIAL_INTERFACE_INSTANCE == 1
#define BOARD_APP_UART_IRQ         BOARD_USART_IRQ(1)
#define BOARD_APP_UART_IRQ_HANDLER BOARD_USART_IRQ_HANDLER(1)
#define BOARD_APP_UART_BASEADDR    BOARD_UART_BASEADDR(1)
#define BOARD_APP_UART_RESET       kUSART1_RST_SHIFT_RSTn
#define BOARD_APP_UART_RX_PIN      IOCON_USART1_RX_PIN
#define BOARD_APP_UART_TX_PIN      IOCON_USART1_TX_PIN
#else
#define BOARD_APP_UART_IRQ         LPUART0_IRQn
#define BOARD_APP_UART_IRQ_HANDLER LPUART0_IRQHandler
#define BOARD_APP_UART_BASEADDR    BOARD_UART_BASEADDR(0)
#define BOARD_APP_UART_RESET       kUSART0_RST_SHIFT_RSTn
#define BOARD_APP_UART_RX_PIN      IOCON_USART0_RX_PIN
#define BOARD_APP_UART_TX_PIN      IOCON_USART0_TX_PIN
#endif
#define BOARD_APP_UART_CLK_ATTACH kOSC32M_to_USART_CLK
#define BOARD_APP_UART_CLK_FREQ   CLOCK_GetFreq(kCLOCK_Fro32M)
#endif

#if gUartDebugConsole_d

#if DEBUG_SERIAL_INTERFACE_INSTANCE == 1
#define BOARD_DEBUG_UART_IRQ         BOARD_USART_IRQ(1)
#define BOARD_DEBUG_UART_IRQ_HANDLER BOARD_USART_IRQ_HANDLER(1)
#define BOARD_DEBUG_UART_BASEADDR    BOARD_UART_BASEADDR(1)
#define BOARD_DEBUG_UART_RESET       kUSART1_RST_SHIFT_RSTn
#define BOARD_DEBUG_UART_RX_PIN      IOCON_USART1_RX_PIN
#define BOARD_DEBUG_UART_TX_PIN      IOCON_USART1_TX_PIN
#else
#define BOARD_DEBUG_UART_IRQ         LPUART0_IRQn
#define BOARD_DEBUG_UART_IRQ_HANDLER LPUART0_IRQHandler
#define BOARD_DEBUG_UART_BASEADDR    BOARD_UART_BASEADDR(0)
#define BOARD_DEBUG_UART_RESET       kUSART0_RST_SHIFT_RSTn
#define BOARD_DEBUG_UART_RX_PIN      IOCON_USART0_RX_PIN
#define BOARD_DEBUG_UART_TX_PIN      IOCON_USART0_TX_PIN
#endif

#define BOARD_DEBUG_UART_CLK_ATTACH kOSC32M_to_USART_CLK
#define BOARD_DEBUG_UART_CLK_FREQ   CLOCK_GetFreq(kCLOCK_Fro32M)

/* doc-start:uart_logging */
/* The UART to use for debug messages. */
/* when UWBIOT_LOG=Verbose, set the baudrate to 3Mbps for viewing logs in terminal emulator(ex. Tera term)*/
#define BOARD_DEBUG_UART_TYPE kSerialPort_Uart
#if (UWBIOT_LOG_VERBOSE == 1)
//#define BOARD_DEBUG_UART_BAUDRATE 3000000U //3Mbps
//#pragma message("Logging Baud rate is set to 3Mbits. Please change settings on Serial port GUI")
#define BOARD_DEBUG_UART_BAUDRATE 115200 //115200bps
#pragma message("Logging Baud rate is set to 115200bits. Please change settings on Serial port GUI")
#else
//#define BOARD_DEBUG_UART_BAUDRATE 3000000U
#define BOARD_DEBUG_UART_BAUDRATE 115200
#endif
/* doc-end:uart_logging */
#endif

//#define BOARD_DIAG_PORT_MODE           (0x00840083)
//#define BOARD_DIAG_PORT_MODE           (0x9F8B8783)
//#define BOARD_DIAG_PORT_MODE           (0x00000083)
#ifndef BOARD_DIAG_PORT_MODE
#define BOARD_DIAG_PORT_MODE 0
#endif

#define gDbgIoCfg_c 0 /* Dbg Io forbidden */
//#define gDbgIoCfg_c                     1 /* For Low Power Dbg Io use */
//#define gDbgIoCfg_c                     2 /* For General Purpose Dbg Io use */

#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
#define LpIoSet(x, y) BOARD_DbgLpIoSet(x, y)
#else
#define LpIoSet(x, y)
#endif

/* Battery voltage level pin for ADC0 */
#define gADC0BatLevelInputPin (14)

/* when Diag Port is enabled (gDbgUseLLDiagPort set to 1), define the mode to use */
/* Use IOs API for Debugging BOARD_DbgSetIoUp() and BOARD_DbgSetIoUp() in board.h */
#define gDbgUseDbgIos (gDbgIoCfg_c != 0)

/* Enable Link Layer Diag Port - enable BOARD_DbgDiagIoConf() and BOARD_DbgDiagEnable() API in board.h */
#ifndef BIT
#define BIT(x) (1 << (x))
#endif
#define gDbgLLDiagPort0Msk (BIT(7) | BIT(23))
#define gDbgLLDiagPort1Msk (BIT(15) | BIT(31))
#define gDbgUseLLDiagPort  (BOARD_DIAG_PORT_MODE & (gDbgLLDiagPort0Msk | gDbgLLDiagPort1Msk))

/* Bluetooth MAC adress size */
#define BD_ADDR_SIZE 6

#if gPWR_CpuClk_48MHz
#define BOARD_TARGET_CPU_FREQ         BOARD_MAINCLK_FRO48M
#define BOARD_BOOTCLOCKRUN_CORE_CLOCK 48000000U
#else
#define BOARD_TARGET_CPU_FREQ         BOARD_MAINCLK_XTAL32M
#define BOARD_BOOTCLOCKRUN_CORE_CLOCK 32000000U

#endif

#if gClkUseFro32K
#define CLOCK_32k_source kCLOCK_Fro32k
#else
#define CLOCK_32k_source kCLOCK_Xtal32k
#endif

#if gLoggingActive_d
#define RAM2_BASE 0x04020000
#define RAM2_SIZE (64 * 1024)
#endif

/* gAdvertisingPowerLeveldBm_c and gConnectPowerLeveldBm_c default values if not defiend otherwise
 * Valid values are in the range [-30, 10] nonetheless [-4, 3] is a reasonable range
 * e.g.  DBM(-3) for -3dBm
 * */
#define DBM(x) ((uint8_t)(x))

#ifndef gAdvertisingPowerLeveldBm_c
#define gAdvertisingPowerLeveldBm_c DBM(0)
#endif
#ifndef gConnectPowerLeveldBm_c
#define gConnectPowerLeveldBm_c DBM(0)
#endif

#define IS_DEBUG_SESSION (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* Capacitance values for 32MHz and 32kHz crystals; board-specific. Value is
   pF x 100. For example, 6pF becomes 600, 1.2pF becomes 120 */
#define CLOCK_32MfXtalIecLoadpF_x100    (600) /* 6.0pF */
#define CLOCK_32MfXtalPPcbParCappF_x100 (20)  /* 0.2pF */
#define CLOCK_32MfXtalNPcbParCappF_x100 (40)  /* 0.4pF */
#define CLOCK_32kfXtalIecLoadpF_x100    (600) /* 6.0pF */
#define CLOCK_32kfXtalPPcbParCappF_x100 (40)  /* 0.4pF */
#define CLOCK_32kfXtalNPcbParCappF_x100 (40)  /* 0.4pF */

#define LED_RED_OFF()
#define LED_GREEN_OFF()
#define LED_BLUE_OFF()
#define LED_RED_ON()
#define LED_GREEN_ON()
#define LED_BLUE_ON()

/* Set gTcxo32M_ModeEn_c if you wish to activate the 32M Xtal trimming vs temperature.
* Set gTcxo32k_ModeEn_c if you wish to activate the 32k Xtal trimming vs temperature.
* dk6 boards are equipped with a CMOS040LP 32 MHz ultra low power DCXO (Digital Controlled Xtal Oscillator).
* A temperature sweep has been performed from -40'C to +125'C in 5'C step and the frequency accuracy
* has been recorded using a frequency meter for a fixed capbank code corresponding to 6 pF IEC load,
* and the optimum IEC load giving the lowest frequency error obtained thanks to a dichotomy-based algorithm.
* The PCB parasitic capacitors have been taken into account and specified in the C header file of the SW API
* XTAL Reference: NDK NX2016SA 32MHz EXS00A-CS11213-6(IEC)
*                 NDK NX2012SA 32.768kHz EXS00A-MU01089-6(IEC)
* As a result CLOCK_ai32MXtalIecLoadFfVsTemp is setup to compensate IEC Load vs temperature for Xtal 32MHz.
*
* Similarly CLOCK_ai32kXtalIecLoadFfVsTemp has to be put together for the 32k Xtal.
* The 32k compensation turns out inefficient outside the -40'C to +80'C range.
*
*
*
**/
#define gTcxo32M_ModeEn_c (1)
#define gXo32M_Trim_c     (1 || gTcxo32M_ModeEn_c)

/* Xtal 32kHz temperature compensation is disabled because table is *not* correct: values populating the temperature
 * compensation array below are just for example TODO */
#define gTcxo32k_ModeEn_c (0)
/* 32k not temperature compensated but ATE trimming used */
#if gTcxo32k_ModeEn_c
#define gXo32k_Trim_c (1)
#else
#define gXo32k_Trim_c (0)
#endif

#define TEMP_ZERO_K           -273
#define TEMP_ZERO_K_128th_DEG (TEMP_ZERO_K * 128)

#define gAdcUsed_d 1

#define ABSOLUTE_VALUE(x) (((x) < 0) ? -(x) : (x))

#if gAdcUsed_d

/* Full scale voltage of ADC0 is 3600 mV */
#define gAdc0FullRangeVoltage (3600)

/* Full voltage value of battery is 3300 mV */
#define gBatteryFullVoltage (3300)

/* Resolution of ADC0 */
#define gAdc0MaxResolution (12)

/* Convert ADC0 output to voltage in mV */
#define ADC_TO_MV(x) (((x)*gAdc0FullRangeVoltage) >> gAdc0MaxResolution)

/* Compute voltage percentage */
#define ADC_MV_TO_PERCENT(x) (((x)*100) / gBatteryFullVoltage)

/* Temperature sensor channel of ADC */
#define ADC_TEMPERATURE_SENSOR_CHANNEL 7U

/* Battery level input channel of ADC */
#define ADC_BAT_LEVEL_CHANNEL 0x06

/* Temperature sensor driver code enable */
#define ADC_TEMP_SENSOR_DRIVER_EN 1

/* ADC initiate time */
#define ADC_WAIT_TIME_US 300

/* ADC measurements are done one over gAppADCMeasureCounter wakeup times */
#ifndef gAppADCMeasureCounter_c
#define gAppADCMeasureCounter_c 100
#endif
#else
#define ADC_TEMP_SENSOR_DRIVER_EN 0
#endif

/* Enable CPU clock to 48Mhz on critical sequences only during wakeup and lowpower entry
   to save time on wakeup (=1) and lowpower entry (=2) , keep 32Mhz XTAL otherwise
   Note : flag is useless if gPWR_CpuClk_48MHz is set to 1 already   */
#ifndef gPWR_SmartFrequencyScaling
#define gPWR_SmartFrequencyScaling (0)
#endif

/* Rhodes 4 Rev B Extender I2C Address */
#define REV_B_EXTENDER_ADDR 0x86
/* Rhodes 4 Rev C Extender I2C Address */
#define REV_C_EXTENDER_ADDR 0x68

typedef enum Rhodes4_revision
{
    RHODES4_REV_B                = 0, // With IO Expander
    RHODES4_REV_C                = 1, // With IO Expander
    RHODES4_REV_B_NO_IO_EXPANDER = 2, // Without IO Expander
} eRhodes4Revision;

/*******************************************************************************
 * API
 ******************************************************************************/
//extern flash_config_t gFlashConfig;

status_t BOARD_InitDebugConsole(void);
status_t BOARD_DeinitDebugConsole(void);
status_t BOARD_InitSwoDebugConsole(void);
status_t BOARD_DeinitSwoDebugConsole(void);
void BOARD_Get_Rhodes4Version(void);
void BOARD_I2C_Init(I2C_Type *base, uint32_t clkSrc_Hz);
status_t BOARD_I2C_Send(I2C_Type *base,
    uint8_t deviceAddress,
    uint32_t subAddress,
    uint8_t subaddressSize,
    uint8_t *txBuff,
    uint8_t txBuffSize);
status_t BOARD_I2C_Receive(I2C_Type *base,
    uint8_t deviceAddress,
    uint32_t subAddress,
    uint8_t subaddressSize,
    uint8_t *rxBuff,
    uint8_t rxBuffSize);

/* Function to initialize/deinitialize ADC on board configuration. */
void BOARD_InitAdc(void);
void BOARD_ADCWakeupInit(void);
void BOARD_CheckADCReady(void);
void BOARD_ADCMeasure(void);
void BOARD_EnableAdc(void);
void BOARD_DeInitAdc(void);

/* Function to read battery level on board configuration. */
uint8_t BOARD_GetBatteryLevel(void);

int32_t BOARD_GetTemperature(void);

/* Function called by the BLE connection manager to generate PER MCU keys */
void BOARD_GetMCUUid(uint8_t *aOutUid16B, uint8_t *pOutLen);

/* Reboot MCU */
void BOARD_MCUReset();

void hardware_init(void);

/* Function called to get the USART Clock in Hz */
extern uint32_t BOARD_GetUsartClock(int8_t instance);

/* Function called to get the CTIMER clock in Hz */
extern uint32_t BOARD_GetCtimerClock(CTIMER_Type *timer);

extern void BOARD_UnInitButtons(void);

extern void BOARD_UnInitButtons1(void);

extern void BOARD_InitFlash(void);

extern uint16_t BOARD_GetPotentiometerLevel(void);

extern void BOARD_SetFaultBehaviour(void);

extern uint32_t BOARD_GetSpiClock(uint32_t instance);
extern void BOARD_InitPMod_SPI_I2C(void);
extern void BOARD_InitSPI(void);

extern void BOARD_InitSPIFI(void);

/* For debug only */
extern void BOARD_DbgDiagIoConf(void);
extern void BOARD_DbgDiagEnable();
extern void BOARD_InitDbgIo(void);

extern void BOARD_DbgLpIoSet(int pinid, int val);
extern void BOARD_DbgIoSet(int pinid, int val);
extern void BOARD_DbgSetIoUp(int pinid);   /* Should be deprecated and replaced by BOARD_DbgLpIoSet */
extern void BOARD_DbgSetIoDown(int pinid); /* Should be deprecated and replaced by BOARD_DbgLpIoSet */
/* Passivate outputs used for driving LEDs */
extern void BOARD_SetLEDs_LowPower(void);

/* Perform preparation to let inputs acquire wake capability */
extern void BOARD_SetButtons_LowPowerEnter(void);

/* Switch to target frequency */
extern void BOARD_CpuClockUpdate(void);

extern void BOARD_common_hw_init(void);

extern bool BOARD_IsADCEnabled(void);

/* Function called to initial the pins */
extern void BOARD_InitDEBUG_UART(void);
extern void BOARD_InitHostInterface(void);
extern void BOARD_InitPins(void);
extern void BOARD_SetPinsForPowerDown(void);
extern void BOARD_SetPinsForPowerMode(void);
extern void BOARD_SetPinsForRunMode(void);
extern bool BOARD_CheckSwdAttached(void);
//void BOARD_SetPinsForRunMode(void);
extern void hardware_wdt_init(void);
extern void BOARD_CpuClockUpdate32MhzFro(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
