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

#include "board.h"
#include <uwb_bus_interface.h>
#include <QN9090.h>
#include "phNxpLogApis_Board.h"
#include "UWB_GPIOExtender.h"

extern eRhodes4Revision gRhodesV4_Rev;

static uwbs_io_callback mCallbacks[FSL_FEATURE_PINT_NUMBER_OF_CONNECTED_OUTPUTS];

static void uwb_bus_io_pint_generic_cb(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    if (mCallbacks[pintr].fn) {
        mCallbacks[pintr].fn(mCallbacks[pintr].args);
    }
}

void uwb_bus_io_irq_cb(void *args)
{
    uwb_bus_board_ctx_t *pCtx = (uwb_bus_board_ctx_t *)args;
    // Signal TML read task
    phOsalUwb_ProduceSemaphore(pCtx->mIrqWaitSem);
}

uwb_bus_status_t uwb_bus_io_deinit(uwb_bus_board_ctx_t *pCtx)
{
    phOsalUwb_SetMemory(mCallbacks, 0, sizeof(uwbs_io_callback) * FSL_FEATURE_PINT_NUMBER_OF_CONNECTED_OUTPUTS);
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_io_init(uwb_bus_board_ctx_t *pCtx)
{
    uwb_bus_io_uwbs_irq_enable(pCtx);
    return kUWB_bus_Status_OK;
}

/**
 * @brief
 * Wrapper function for nfc sdk
 * Calls uwb_bus_io_val_set using board ctx
 * @param gpioValue
 */
void nfc_ven_val_set(uint8_t gpioValue)
{
    uwbs_io_state_t gpioSetValue = (uwbs_io_state_t)gpioValue;
    uwb_bus_board_ctx_t board_ctx;
    if (uwb_bus_io_val_set(&board_ctx, kUWBS_IO_O_VENUS_VEN, gpioSetValue) != kUWB_bus_Status_OK) {
        LOG_E("%s : uwb_bus_io_val_set failed", __FUNCTION__);
    }
}

/**
 * @brief
 * Wrapper function for nfc sdk
 * Calls uwb_bus_io_val_get using board ctx
 * @param gpioValue
 * @return uint32_t
 */
uint32_t nfc_ven_val_get(uint8_t gpioValue)
{
    uwbs_io_state_t gpio_val = kUWBS_IO_State_NA;
    uwb_bus_board_ctx_t board_ctx;
    if (uwb_bus_io_val_get(&board_ctx, kUWBS_IO_O_VENUS_VEN, &gpio_val) != kUWB_bus_Status_OK) {
        LOG_E("%s : uwb_bus_io_val_get failed", __FUNCTION__);
    }
    return (uint32_t)gpio_val;
}

/**
 * @brief
 * Wrapper function for nfc sdk
 * Calls uwb_bus_io_val_get using board ctx
 * @return uint32_t
 */
uint32_t nfc_irq_val_get()
{
    uwbs_io_state_t irq_val = kUWBS_IO_State_NA;
    uwb_bus_board_ctx_t board_ctx;
    if (uwb_bus_io_val_get(&board_ctx, kUWBS_IO_I_VENUS_IRQ, &irq_val) != kUWB_bus_Status_OK) {
        LOG_E("%s : uwb_bus_io_val_get failed", __FUNCTION__);
    }
    return (uint32_t)irq_val;
}

uwb_bus_status_t uwb_bus_io_val_set(uwb_bus_board_ctx_t *pCtx, uwbs_io_t gpioPin, uwbs_io_state_t gpioValue)
{
    uwb_bus_status_t return_status;
    GPIO_Type *base;
    uint32_t port;
    uint32_t pin;
    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    switch (gpioPin) {
    case kUWBS_IO_O_HELIOS_SYNC: {
        base = RV4_INIT_HELIOSPINS_HELIOS_GPIO3_SYNC_GPIO;
        port = RV4_INIT_HELIOSPINS_HELIOS_GPIO3_SYNC_PORT;
        pin  = RV4_INIT_HELIOSPINS_HELIOS_GPIO3_SYNC_PIN;
        GPIO_PinWrite(base, port, pin, gpioValue);
        return_status = kUWB_bus_Status_OK;
    } break;
    case kUWBS_IO_O_ENABLE_HELIOS: {
        if ((gRhodesV4_Rev == RHODES4_REV_B) || (gRhodesV4_Rev == RHODES4_REV_B_NO_IO_EXPANDER)) {
            base = RV4_INIT_HELIOSCEPIN_HELIOS_CE_GPIO;
            port = RV4_INIT_HELIOSCEPIN_HELIOS_CE_PORT;
            pin  = RV4_INIT_HELIOSCEPIN_HELIOS_CE_PIN;
            GPIO_PinWrite(base, port, pin, gpioValue);
        }
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            GPIOExtenderSetPin(CHIP_ENABLE_SR, gpioValue);
        }
        return_status = kUWB_bus_Status_OK;
    } break;
    case kUWBS_IO_O_HELIOS_RTC_SYNC:
        /* Not needed for RV4 */
        return_status = kUWB_bus_Status_OK;
        break;
    case kUWBS_IO_O_VENUS_VEN: {
        if ((gRhodesV4_Rev == RHODES4_REV_B) || (gRhodesV4_Rev == RHODES4_REV_B_NO_IO_EXPANDER)) {
            GPIO_PinWrite(RV4_INIT_VENUSPINS_VENUS_VEN_GPIO,
                RV4_INIT_VENUSPINS_VENUS_VEN_PORT,
                RV4_INIT_VENUSPINS_VENUS_VEN_PIN,
                gpioValue);
            return_status = kUWB_bus_Status_OK;
        }
        else if (gRhodesV4_Rev == RHODES4_REV_C) {
            GPIOExtenderSetPin(ENABLE_SN, gpioValue);
            return_status = kUWB_bus_Status_OK;
        }
        else {
            LOG_E("Unexpected value for Rhodes V4 Revision.");
            return_status = kUWB_bus_Status_FAILED;
        }
    } break;
    default:
        LOG_E("UWBD IO GPIO Pin not supported");
        return_status = kUWB_bus_Status_FAILED;
        break;
    }
    return return_status;
}

uwb_bus_status_t uwb_bus_io_val_get(uwb_bus_board_ctx_t *pCtx, uwbs_io_t gpioPin, uwbs_io_state_t *pGpioValue)
{
    uwb_bus_status_t return_status = kUWB_bus_Status_OK;
    GPIO_Type *base;
    uint32_t port;
    uint32_t pin;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return_status = kUWB_bus_Status_FAILED;
    }
    else {
        switch (gpioPin) {
        case kUWBS_IO_I_UWBS_IRQ: {
            base        = RV4_INIT_HELIOSPINS_HELIOS_GPIO5_HOST_INT_GPIO;
            port        = RV4_INIT_HELIOSPINS_HELIOS_GPIO5_HOST_INT_PORT;
            pin         = RV4_INIT_HELIOSPINS_HELIOS_GPIO5_HOST_INT_PIN;
            *pGpioValue = (uwbs_io_state_t)GPIO_PinRead(base, port, pin);
        } break;
        case kUWBS_IO_O_HELIOS_SYNC: {
            base        = RV4_INIT_HELIOSPINS_HELIOS_GPIO3_SYNC_GPIO;
            port        = RV4_INIT_HELIOSPINS_HELIOS_GPIO3_SYNC_PORT;
            pin         = RV4_INIT_HELIOSPINS_HELIOS_GPIO3_SYNC_PIN;
            *pGpioValue = (uwbs_io_state_t)GPIO_PinRead(base, port, pin);
        } break;
        case kUWBS_IO_O_ENABLE_HELIOS: {
            if ((gRhodesV4_Rev == RHODES4_REV_B) || (gRhodesV4_Rev == RHODES4_REV_B_NO_IO_EXPANDER)) {
                base        = RV4_INIT_HELIOSCEPIN_HELIOS_CE_GPIO;
                port        = RV4_INIT_HELIOSCEPIN_HELIOS_CE_PORT;
                pin         = RV4_INIT_HELIOSCEPIN_HELIOS_CE_PIN;
                *pGpioValue = (uwbs_io_state_t)GPIO_PinRead(base, port, pin);
            }
            else if (gRhodesV4_Rev == RHODES4_REV_C) {
                uint32_t gpioValue;
                if (GPIOExtenderGetPin(CHIP_ENABLE_SR, &gpioValue) == FALSE) {
                    LOG_E("GPIOExtenderGetPin read fail.");
                    *pGpioValue   = kUWBS_IO_State_NA;
                    return_status = kUWB_bus_Status_FAILED;
                }
                else {
                    if (gpioValue) {
                        *pGpioValue = kUWBS_IO_State_High;
                    }
                    else {
                        *pGpioValue = kUWBS_IO_State_Low;
                    }
                }
            }
            else {
                LOG_E("Unexpected value for Rhodes V4 Revision.");
                return_status = kUWB_bus_Status_FAILED;
            }
        } break;
        case kUWBS_IO_I_VENUS_IRQ: {
            if ((gRhodesV4_Rev == RHODES4_REV_B) || (gRhodesV4_Rev == RHODES4_REV_B_NO_IO_EXPANDER)) {
                base        = RV4_INIT_REVB_VENUSIRQPINS_VENUS_IRQ_GPIO;
                port        = RV4_INIT_REVB_VENUSIRQPINS_VENUS_IRQ_PORT;
                pin         = RV4_INIT_REVB_VENUSIRQPINS_VENUS_IRQ_PIN;
                *pGpioValue = ((uwbs_io_state_t)GPIO_PinRead(base, port, pin));
            }
            else if (gRhodesV4_Rev == RHODES4_REV_C) {
                base        = RV4_INIT_REVC_VENUSIRQPINS_VENUS_IRQ_GPIO;
                port        = RV4_INIT_REVC_VENUSIRQPINS_VENUS_IRQ_PORT;
                pin         = RV4_INIT_REVC_VENUSIRQPINS_VENUS_IRQ_PIN;
                *pGpioValue = ((uwbs_io_state_t)GPIO_PinRead(base, port, pin));
            }
            else {
                LOG_E("Unexpected value for Rhodes V4 Revision.");
                return_status = kUWB_bus_Status_FAILED;
            }
        } break;
        case kUWBS_IO_O_VENUS_VEN: {
            if ((gRhodesV4_Rev == RHODES4_REV_B) || (gRhodesV4_Rev == RHODES4_REV_B_NO_IO_EXPANDER)) {
                base        = RV4_INIT_VENUSPINS_VENUS_VEN_GPIO;
                port        = RV4_INIT_VENUSPINS_VENUS_VEN_PORT;
                pin         = RV4_INIT_VENUSPINS_VENUS_VEN_PIN;
                *pGpioValue = ((uwbs_io_state_t)GPIO_PinRead(base, port, pin));
            }
            else if (gRhodesV4_Rev == RHODES4_REV_C) {
                uint32_t gpioValue;
                if (GPIOExtenderGetPin(ENABLE_SN, &gpioValue) == FALSE) {
                    LOG_E("GPIOExtenderGetPin read fail.");
                    *pGpioValue   = kUWBS_IO_State_NA;
                    return_status = kUWB_bus_Status_FAILED;
                }
                else {
                    if (gpioValue) {
                        *pGpioValue = kUWBS_IO_State_High;
                    }
                    else {
                        *pGpioValue = kUWBS_IO_State_Low;
                    }
                }
            }
            else {
                LOG_E("Unexpected value for Rhodes V4 Revision.");
                return_status = kUWB_bus_Status_FAILED;
            }
        } break;
        default:
            LOG_E("UWBD IO GPIO Pin not supported");
            return_status = kUWB_bus_Status_FAILED;
        }
    }
    return return_status;
}

uwb_bus_status_t uwb_bus_io_irq_dis(uwb_bus_board_ctx_t *pCtx, uwbs_io_t irqPin)
{
    uwb_bus_status_t status = kUWB_bus_Status_FAILED;
    pint_pin_int_t intrType;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return status;
    }

    __disable_irq();

    switch (irqPin) {
    case kUWBS_IO_I_UWBS_IRQ: {
        intrType = kPINT_PinInt0;
    } break;
    case kUWBS_IO_I_VENUS_IRQ: {
        intrType = kPINT_PinInt1;
    } break;
    default:
        LOG_E("UWBD IO GPIO Pin Interrupt not supported");
        goto end;
    }

    PINT_DisableCallbackByIndex(PINT, intrType);
    RESET_PeripheralReset(kPINT_RST_SHIFT_RSTn);
    mCallbacks[intrType].fn   = NULL;
    mCallbacks[intrType].args = NULL;
    status                    = kUWB_bus_Status_OK;
end:
    __enable_irq();
    __ISB();
    return status;
}

uwb_bus_status_t uwb_bus_io_irq_en(uwb_bus_board_ctx_t *pCtx, uwbs_io_t irqPin, uwbs_io_callback *pCallback)
{
    uwb_bus_status_t status = kUWB_bus_Status_FAILED;
    pint_pin_enable_t enable;
    pint_pin_int_t intrType;
    uint8_t prio;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return status;
    }

    /*
     * Note: __disable_irq()/_enable_irq()+__ISB() apparently cause no
     * trouble, but don't match FreeRTOS's portDISABLE_INTERRUPTS/portENABLE_INTERRUPTS
     * definition. Consider using FreeRTOS's macros in case of trouble
     */
    __disable_irq();

    switch (irqPin) {
    case kUWBS_IO_I_UWBS_IRQ: {
        mCallbacks[kPINT_PinInt0].fn   = pCallback->fn;
        mCallbacks[kPINT_PinInt0].args = pCallback->args;
        enable                         = kPINT_PinIntEnableRiseEdge;
        intrType                       = kPINT_PinInt0;
        prio                           = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;

        INPUTMUX_Init(INPUTMUX);
        RV4_Init_HeliosIRQPins();
        /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
        INPUTMUX_Deinit(INPUTMUX);
    } break;
    case kUWBS_IO_I_VENUS_IRQ: {
        mCallbacks[kPINT_PinInt1].fn   = pCallback->fn;
        mCallbacks[kPINT_PinInt1].args = pCallback->args;
        enable                         = kPINT_PinIntEnableHighLevel;
        intrType                       = kPINT_PinInt1;
        prio                           = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;

        INPUTMUX_Init(INPUTMUX);
        if ((gRhodesV4_Rev == RHODES4_REV_B) || (gRhodesV4_Rev == RHODES4_REV_B_NO_IO_EXPANDER)) {
            RV4_Init_RevB_VenusIRQPins();
        }
        if (gRhodesV4_Rev == RHODES4_REV_C) {
            RV4_Init_RevC_VenusIRQPins();
        }
        /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
        INPUTMUX_Deinit(INPUTMUX);
    } break;
    default:
        LOG_E("UWBD IO GPIO Pin Interrupt not supported");
        goto end;
    }
    PINT_PinInterruptConfig(PINT, intrType, enable, uwb_bus_io_pint_generic_cb);
    PINT_EnableCallbackByIndexAndPriority(PINT, intrType, prio);

    status = kUWB_bus_Status_OK;
end:
    __enable_irq();
    __ISB();
    return status;
}

uwb_bus_status_t uwb_bus_io_irq_wait(uwb_bus_board_ctx_t *pCtx, uint32_t timeout_ms)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(pCtx->mIrqWaitSem, timeout_ms) != UWBSTATUS_SUCCESS) {
        LOG_D("phOsalUwb_ConsumeSemaphore_WithTimeout failed");
        return kUWB_bus_Status_FAILED;
    }
    return kUWB_bus_Status_OK;
}

/* This api is needed only for PNP Firmware and SN110 */
uwb_bus_status_t uwb_bus_io_uwbs_irq_enable(uwb_bus_board_ctx_t *pCtx)
{
    uwbs_io_callback callback;
    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    callback.fn   = uwb_bus_io_irq_cb;
    callback.args = pCtx;
    return uwb_bus_io_irq_en(pCtx, kUWBS_IO_I_UWBS_IRQ, &callback);
}
