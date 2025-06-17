/* Copyright 2021-2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "UWB_GPIOExtender.h"
#include "phNxpLogApis_App.h"
#include "phUwb_BuildConfig.h"
#include "phUwbErrorCodes.h"

#define I2CM_BASE  ((I2C_Type *)I2C0)
#define I2CM_SPEED (1000000) //1MHz
#define I2CM_IRQN  FLEXCOMM2_IRQn

extern eRhodes4Revision gRhodesV4_Rev;

static void GPIOExtenderSoftReset(void)
{
    uint8_t buff[] = {0x01, 0x01};
    BOARD_I2C_Send(RV4_I2CM_BASE, REV_B_EXTENDER_ADDR, 0, 0, &buff[0], 02);
    BOARD_I2C_Receive(RV4_I2CM_BASE, REV_B_EXTENDER_ADDR, 0, 0, &buff[0], 01);
    if (buff[0] != GPIO_EXTENDER_RESET_RSP) {
        NXPLOG_APP_I("IO Expansion soft reset fail");
    }
}

static void GPIOExtenderSetImpedence(void)
{
    uint8_t buff[] = {CONFIGURE_IMPEDENCE, GPIO_PIN_LOW};
    BOARD_I2C_Send(RV4_I2CM_BASE, REV_B_EXTENDER_ADDR, 0, 0, &buff[0], 02);
}

void GPIOExtenderSetPinConfig(IoExpnParamInfo_t *IoExpnParam)
{
    uint8_t buff[2] = {0};

    buff[0] = IoExpnParam->reg;
    /*check which pins are set*/
    BOARD_I2C_Send(RV4_I2CM_BASE, IoExpnParam->expanderAddr, 0, 0, &buff[0], 01);
    BOARD_I2C_Receive(RV4_I2CM_BASE, IoExpnParam->expanderAddr, 0, 0, &buff[1], 01);
    /*set bitpos*/
    if (IoExpnParam->state == GPIO_PIN_HIGH) {
        buff[1] = buff[1] | (1 << IoExpnParam->bitPos);
    }
    /*reset bitpos*/
    else if (IoExpnParam->state == GPIO_PIN_LOW) {
        buff[1] = buff[1] & ~(1 << IoExpnParam->bitPos);
    }

    BOARD_I2C_Send(RV4_I2CM_BASE, IoExpnParam->expanderAddr, 0, 0, &buff[0], 02);
}

uint32_t GPIOExtenderGetPinConfig(IoExpnParamInfo_t *IoExpnParam)
{
    uint8_t buff[2]     = {0};
    uint32_t gpio_value = 0;

    buff[0] = IoExpnParam->reg;
    /*check which pins are set*/
    BOARD_I2C_Send(RV4_I2CM_BASE, IoExpnParam->expanderAddr, 0, 0, &buff[0], 01);
    BOARD_I2C_Receive(RV4_I2CM_BASE, IoExpnParam->expanderAddr, 0, 0, &buff[1], 01);
    /*get bitpos value*/
    gpio_value = 1 & (buff[1] >> IoExpnParam->bitPos);
    return gpio_value;
}

void GPIOExtenderSetMode(eBitPos Pin, bool direction)
{
    IoExpnParamInfo_t IoExpnParam = {0};
    IoExpnParam.state             = direction;
    switch (Pin) {
    case UWB_DIG_ENABLE:
    case UWB_RF_ENABLE:
    case NFC_DOWNLOAD:
    case NFC_WAKEUP:
    case EXT_OSC_ENABLE: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPIO_DIRECTION_A;
            IoExpnParam.bitPos       = Pin;
        }
        if (gRhodesV4_Rev == RHODES4_REV_B) {
            IoExpnParam.expanderAddr = REV_B_EXTENDER_ADDR;
            IoExpnParam.reg          = CONFIGURE_IO_DIRECTION;
            IoExpnParam.bitPos       = Pin;
        }

    } break;
    case CLKREQ_38M:
    case CLKREQ_32K: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPIO_DIRECTION_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
        if (gRhodesV4_Rev == RHODES4_REV_B) {
            IoExpnParam.expanderAddr = REV_B_EXTENDER_ADDR;
            IoExpnParam.reg          = CONFIGURE_IO_DIRECTION;
            IoExpnParam.bitPos       = Pin;
        }
    } break;
    case ENABLE_SN:
    case CHIP_ENABLE_SR: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPIO_DIRECTION_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
    } break;
    default:
        LOG_E("Selected Pin is not correct ");
        return;
    }
    GPIOExtenderSetPinConfig(&IoExpnParam);
}

void GPIOExtenderSetPin(eBitPos Pin, bool state)
{
    IoExpnParamInfo_t IoExpnParam = {0};
    IoExpnParam.state             = state;
    switch (Pin) {
    case UWB_DIG_ENABLE:
    case UWB_RF_ENABLE:
    case NFC_DOWNLOAD:
    case NFC_WAKEUP:
    case EXT_OSC_ENABLE: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPO_DATA_OUT_A;
            IoExpnParam.bitPos       = Pin;
        }
        if (gRhodesV4_Rev == RHODES4_REV_B) {
            IoExpnParam.expanderAddr = REV_B_EXTENDER_ADDR;
            IoExpnParam.reg          = SET_IO_PINS;
            IoExpnParam.bitPos       = Pin;
        }
    } break;
    case CLKREQ_38M:
    case CLKREQ_32K: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPO_DATA_OUT_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
        if (gRhodesV4_Rev == RHODES4_REV_B) {
            IoExpnParam.expanderAddr = REV_B_EXTENDER_ADDR;
            IoExpnParam.reg          = SET_IO_PINS;
            IoExpnParam.bitPos       = Pin;
        }
    } break;
    case ENABLE_SN:
    case CHIP_ENABLE_SR: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPO_DATA_OUT_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
    } break;
    default:
        LOG_E("Selected Pin is not correct ");
        return;
    }
    GPIOExtenderSetPinConfig(&IoExpnParam);
}

bool GPIOExtenderGetPin(eBitPos Pin, uint32_t *val)
{
    IoExpnParamInfo_t IoExpnParam = {0};

    switch (Pin) {
    case UWB_DIG_ENABLE:
    case UWB_RF_ENABLE:
    case NFC_DOWNLOAD:
    case NFC_WAKEUP:
    case EXT_OSC_ENABLE: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPO_DATA_OUT_A;
            IoExpnParam.bitPos       = Pin;
        }
        if (gRhodesV4_Rev == RHODES4_REV_B) {
            IoExpnParam.expanderAddr = REV_B_EXTENDER_ADDR;
            IoExpnParam.reg          = SET_IO_PINS;
            IoExpnParam.bitPos       = Pin;
        }
    } break;
    case CLKREQ_38M:
    case CLKREQ_32K: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPO_DATA_OUT_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
        if (gRhodesV4_Rev == RHODES4_REV_B) {
            IoExpnParam.expanderAddr = REV_B_EXTENDER_ADDR;
            IoExpnParam.reg          = SET_IO_PINS;
            IoExpnParam.bitPos       = Pin;
        }
    } break;
    case ENABLE_SN:
    case CHIP_ENABLE_SR: {
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            IoExpnParam.expanderAddr = REV_C_EXTENDER_ADDR;
            IoExpnParam.reg          = GPO_DATA_OUT_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
    } break;
    default:
        LOG_E("Selected Pin is not correct ");
        return FALSE;
    }
    *val = GPIOExtenderGetPinConfig(&IoExpnParam);
    return TRUE;
}

void GPIOExtenderInit()
{
    if (gRhodesV4_Rev == RHODES4_REV_B_NO_IO_EXPANDER) {
        return;
    }
    if (gRhodesV4_Rev == RHODES4_REV_B) {
        GPIOExtenderSoftReset();
        GPIOExtenderSetImpedence();
    }

    GPIOExtenderSetMode(UWB_DIG_ENABLE, GPIO_DIRECTION_OUT);
    GPIOExtenderSetMode(UWB_RF_ENABLE, GPIO_DIRECTION_OUT);
    GPIOExtenderSetPin(UWB_DIG_ENABLE, GPIO_PIN_HIGH);
    GPIOExtenderSetPin(UWB_RF_ENABLE, GPIO_PIN_HIGH);
    GPIOExtenderSetMode(NFC_DOWNLOAD, GPIO_DIRECTION_OUT);
    GPIOExtenderSetMode(NFC_WAKEUP, GPIO_DIRECTION_OUT);
    GPIOExtenderSetPin(NFC_DOWNLOAD, GPIO_PIN_HIGH);
    GPIOExtenderSetPin(NFC_WAKEUP, GPIO_PIN_HIGH);

    if (gRhodesV4_Rev == RHODES4_REV_C) {
        // Set Direction
        GPIOExtenderSetMode(CHIP_ENABLE_SR, GPIO_DIRECTION_OUT);
        GPIOExtenderSetMode(ENABLE_SN, GPIO_DIRECTION_OUT);
    }
}
