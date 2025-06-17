/* Copyright 2021 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef PHNXPUWB_GPIO_EXTENDER_H_
#define PHNXPUWB_GPIO_EXTENDER_H_

#include <stdbool.h>
#include <stdint.h>
#include <board.h>
#include "fsl_i2c.h"
#include "phUwb_BuildConfig.h"

#define GPIO_EXTENDER_RESET_RSP 0xA2

#define MASK_BIT_POS 0x5

#define GPIO_DIRECTION_OUT 0x01
#define GPIO_DIRECTION_IN  0x00

#define GENERAL_CFG_CORE_FREQ(x) (((x)&0x03) << 5)
#define GENERAL_CFG_OSC_EN       (1 << 7)

/*Outlogic of pin*/
#define GPIO_PIN_HIGH 0x01
#define GPIO_PIN_LOW  0x00

typedef enum bitPos
{
    UWB_DIG_ENABLE = 0x00,
    UWB_RF_ENABLE,
    NFC_DOWNLOAD,
    NFC_WAKEUP,
    EXT_OSC_ENABLE,
    CLKREQ_38M,
    CLKREQ_32K,
    /*Rhodes V4 Rev C supported Pins*/
    ENABLE_SN = 0x08,
    CHIP_ENABLE_SR,

} eBitPos;

typedef enum ioExpnReg
{
    /*Register Mapping*/
    /***************************************REV-C**************************************************/

    GPO_DATA_OUT_A = 0x23,
    /*Set the data for the GIPO B
         *  0 = sets output low.
         *  1 = sets output high.
        */
    GPO_DATA_OUT_B = 0x24,
    /*Set the Mode for the GIPO A
         *  0 = push/pull.
         *  1 = open drain.
        */
    GPO_OUT_MODE_A = 0x25,
    /*Set the Mode for the GIPO B
         *  0 = push/pull.
         *  1 = open drain.
        */
    GPO_OUT_MODE_B = 0x26,
    /*Set the Direction for the GIPO A
         *  0 = GPIO  is an input.
         *  1 = GPIO  is an output.
        */
    GPIO_DIRECTION_A = 0x27,
    /*Set the Direction for the GIPO B
         *  0 = GPIO  is an input.
         *  1 = GPIO  is an output.
        */
    GPIO_DIRECTION_B = 0x28,
    /***************************************REV-B**************************************************/

    /*Register Mapping*/

    /*Register 01h : Device ID and Control
        The Device ID and Control register contains the manufacturer ID and firmware revision. The Control register indicates whether
        the device has been reset and the default values have b
        een set.*/
    DEVICE_ID = 0x01,

    /*Register 03h : I/O Direction,
        The I/O Direction Register configures the direction of the I/O pins.
        If a bit in this register is set to 0, the corresponding port pin is enabled as an input
        If a bit in this register is set to 1, the corresponding port pin is enabled as an output*/
    CONFIGURE_IO_DIRECTION = 0x03,

    /*Register 05h : Output Port Register
        The Output Port Register sets the outgoing logic levels of the pins defined as outputs*/
    SET_IO_PINS = 0x05,

    /*Register 07h : Output High-Impedance
        The Output High Impedance Register determines whether pins set as output are enabled or high impedance

        When a bit in this register is set to 0, the corresponding GPIO port output state follows register the output port
        register (05h).
        When a bit in this register is set to 1, the corresponding GPIO port output is set to high*/
    CONFIGURE_IMPEDENCE = 0x07,

    /*Register 09h : Input Default State
        The Input Default State Register sets the default state of the GPIO port input for generating interrupts.

        When a bit in this register is set to 0, the default for the corresponding input is set to LOW
        When a bit in this register is set to 1, the default for the corresponding input is set to HIGH
        Bit values in this register have no effect on pins defined as outputs. In turn, reads from this register reflect the
        value that is in the flip flop controlling the default state, not the actual pin value.
        */
    DEFAULT_IO_STATE = 0x09,

    /*Register 0bh : Pull-Up/-Down Enable
        The Pull-up/-down Enable Register enables or disables the pull-up/down resistor on the GPIO-port as defined in the Pull-up
        /-down Select Register (0Dh).

        When a bit in this register is set to 0, the pull-up/down on the corresponding GPIO is disabled.
        When a bit in this register is set to 1, the pull-up/down on the corresponding GPIO is enabled.
        */
    CONFIGURE_PIN_PORT = 0X0B,

    /*Register 0Dh : Pull-Up/-Down Select
        The Pull-up/down Select Register allows the user to select either a pull-up or pull-down on the GPIO-port.
        Thisregister only selects the pull-up/down resistor on the GPIO-port, while the enabling/disabling is controlled by thePull-up/down Enable Register (0Bh).

        When a bit in this register is set to 0, the pull-down on the corresponding GPIO is selected.
        When a bit in this register is set to 1, the pull-up on the corresponding GPIO is selected.
        */
    CONFIGURE_PIN_STATE = 0X0D,

    /*Register 0Fh : Input Status Register
        The Input Status Register reflects the incoming logic levels of the GPIOs set as inputs.

        The default value, X, is determined by the externally applied logic level.
        It only acts on read operation. Attempted writes to this register have no effect.
        For GPIOs set as outputs this register will read LOW
        */
    INPUT_STATUS = 0x0F,
} eGPIOExtndrReg;

typedef struct
{
    eGPIOExtndrReg reg;
    eBitPos bitPos;
    uint8_t expanderAddr;
    bool state;
} IoExpnParamInfo_t;

void GPIOExtenderInit();
void GPIOExtenderSetPinConfig(IoExpnParamInfo_t *IoExpnParam);
uint32_t GPIOExtenderGetPinConfig(IoExpnParamInfo_t *IoExpnParam);
void GPIOExtenderSetMode(eBitPos Pin, bool direction);
void GPIOExtenderSetPin(eBitPos Pin, bool state);
bool GPIOExtenderGetPin(eBitPos Pin, uint32_t *val);
#endif //PHNXPUWB_GPIO_EXTENDER_H_
