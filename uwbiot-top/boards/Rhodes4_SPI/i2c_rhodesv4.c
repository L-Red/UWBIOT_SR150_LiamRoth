/*
 *
 * Copyright 2016-2018,2022 NXP
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * I2C implmentation for ICs related to Freedom Family
 */

#include <board.h>
#include "board.h"
#if defined(FSL_FEATURE_SOC_I2C_COUNT) && FSL_FEATURE_SOC_I2C_COUNT > 0 && defined(QN9090DK6)
#include "i2c_rhodesv4.h"

//#define I2C_BAUDRATE (3400u * 1000u) // 3.4. Not used by default
#define I2C_BAUDRATE (1000u * 1000u) // 1 MHZ

//#define I2C_DEBUG
//#define DELAY_I2C_US          (T_CMDG_USec)
#define DELAY_I2C_US (0)

#define I2C_LOG_PRINTF PRINTF

#ifdef QN9090DK6
#define AX_I2CM                    ((I2C_Type *)I2C0)
#define I2C_MASTER_CLOCK_FREQUENCY CLOCK_GetFreq(kCLOCK_Fro48M)
// #   define AX_I2C_CLK_SRC       I2C0_CLK_SRC
#define AX_I2CM_IRQN FLEXCOMM2_IRQn
#endif

#define BACKOFF_DELAY_RATE (1000 / configTICK_RATE_HZ)

#if defined(I2C_DEBUG) || 0
#define DEBUG_PRINT_KINETIS_I2C(Operation, status)                                                \
    if (result == kStatus_Success) { /* I2C_LOG_PRINTF(Operation " OK\r\n");                   */ \
    }                                                                                             \
    else if (result == kStatus_I2C_Addr_Nak) { /* I2C_LOG_PRINTF(Operation " A-Nak\r\n");  */     \
    }                                                                                             \
    else if (result == kStatus_I2C_Busy)                                                          \
        I2C_LOG_PRINTF(Operation " Busy\r\n");                                                    \
    else if (result == kStatus_I2C_Idle)                                                          \
        I2C_LOG_PRINTF(Operation " Idle\r\n");                                                    \
    else if (result == kStatus_I2C_Nak)                                                           \
        I2C_LOG_PRINTF(Operation " Nak\r\n");                                                     \
    else if (result == kStatus_I2C_Timeout)                                                       \
        I2C_LOG_PRINTF(Operation " T/O\r\n");                                                     \
    else if (result == kStatus_I2C_ArbitrationLost)                                               \
        I2C_LOG_PRINTF(Operation " ArbtnLost\r\n");                                               \
    else                                                                                          \
        I2C_LOG_PRINTF(Operation " ERROR  : 0x%02lX\r\n", status);
#else
#define DEBUG_PRINT_KINETIS_I2C(Operation, status)
#endif

/* Handle NAK from the A71CH */
static int gBackoffDelay;

void axI2CResetBackoffDelay()
{
    gBackoffDelay = 0;
}

/*as configTICK_RATE_HZ for qn9090 is 200Hz 1 tick will happen after 5msec*/
static void BackOffDelay_Wait()
{
    if (gBackoffDelay < 1000)
        gBackoffDelay = gBackoffDelay + BACKOFF_DELAY_RATE;
    vTaskDelay(1 >= pdMS_TO_TICKS(gBackoffDelay) ? 1 : pdMS_TO_TICKS(gBackoffDelay));
}

static i2c_error_t kinetisI2cStatusToAxStatus(status_t kinetis_i2c_status)
{
    i2c_error_t retStatus;
    switch (kinetis_i2c_status) {
    case kStatus_Success:
        axI2CResetBackoffDelay();
        retStatus = I2C_OK;
        break;
    case kStatus_I2C_Busy:
        retStatus = I2C_BUSY;
        break;
    case kStatus_I2C_Idle:
        retStatus = I2C_BUSY;
        break;
    case kStatus_I2C_Nak:
        BackOffDelay_Wait();
        retStatus = I2C_NACK_ON_DATA;
        break;
    case kStatus_I2C_ArbitrationLost:
        retStatus = I2C_ARBITRATION_LOST;
        break;
    case kStatus_I2C_Timeout:
        retStatus = I2C_TIME_OUT;
        break;
    case kStatus_I2C_Addr_Nak:
        //BackOffDelay_Wait();
        retStatus = I2C_NACK_ON_ADDRESS;
        break;
    case kStatus_I2C_UnexpectedState:
        retStatus = I2C_BUSY;
        break;
    default:
        retStatus = I2C_FAILED;
        break;
    }
    return retStatus;
}

#define RETURN_ON_BAD_kinetisI2cStatus(kinetis_i2c_status)                      \
    {                                                                           \
        i2c_error_t ax_status = kinetisI2cStatusToAxStatus(kinetis_i2c_status); \
        if (ax_status != I2C_OK)                                                \
            return ax_status;                                                   \
    }

#if defined(SDK_OS_FREE_RTOS) && SDK_OS_FREE_RTOS == 1
i2c_rtos_handle_t gmaster_rtos_handle;
#endif

i2c_error_t axI2CInit(void **conn_ctx, const char *pDevName)
{
    i2c_master_config_t masterConfig;

    /*
     * Default configuration:
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.enableHighDrive = false;
     * masterConfig.enableStopHold = false;
     * masterConfig.glitchFilterWidth = 0U;
     * masterConfig.enableMaster = true;
     */

    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = I2C_BAUDRATE;
    uint32_t sourceClock      = I2C_MASTER_CLOCK_FREQUENCY; //CLOCK_GetFreq(AX_I2C_CLK_SRC);
#if defined(SDK_OS_FREE_RTOS) && SDK_OS_FREE_RTOS == 1
    NVIC_SetPriority(AX_I2CM_IRQN, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    EnableIRQ(AX_I2CM_IRQN);
    I2C_RTOS_Init(&gmaster_rtos_handle, AX_I2CM, &masterConfig, sourceClock);
#else
    I2C_MasterInit(AX_I2CM, &masterConfig, sourceClock);
#endif
    return I2C_OK;
}

void axI2CTerm(void *conn_ctx, int mode)
{
#if defined(SDK_OS_FREE_RTOS) && SDK_OS_FREE_RTOS == 1
    I2C_RTOS_Deinit(&gmaster_rtos_handle);
#endif
}

#if defined(SDK_OS_FREE_RTOS) && SDK_OS_FREE_RTOS == 1
#define I2CM_TX() result = I2C_RTOS_Transfer(&gmaster_rtos_handle, &masterXfer)
#else
#define I2CM_TX() result = I2C_MasterTransferBlocking(AX_I2CM, &masterXfer)
#endif

unsigned int axI2CWrite(
    void *conn_ctx, unsigned char bus_unused_param, unsigned char addr, unsigned char *pTx, unsigned short txLen)
{
    status_t result;
    i2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer)); //clear values

    if (pTx == NULL || txLen > MAX_DATA_LEN) {
        return I2C_FAILED;
    }

    masterXfer.slaveAddress   = addr >> 1; // the address of the A70CM
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = 0;
    masterXfer.subaddressSize = 0;
    masterXfer.data           = pTx;
    masterXfer.dataSize       = txLen;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    I2CM_TX();

    DEBUG_PRINT_KINETIS_I2C("WR", result);
    RETURN_ON_BAD_kinetisI2cStatus(result);

    /* RETURN_ON_BAD_kinetisI2cStatus return the i2c_error_t status hence returning I2C_OK here */
    return I2C_OK;
}

unsigned int axI2CRead(void *conn_ctx, unsigned char bus, unsigned char addr, unsigned char *pRx, unsigned short rxLen)
{
    i2c_master_transfer_t masterXfer;
    status_t result;
    memset(&masterXfer, 0, sizeof(masterXfer)); //clear values

    if (pRx == NULL || rxLen > MAX_DATA_LEN) {
        return I2C_FAILED;
    }

    masterXfer.slaveAddress = addr >> 1; // the address of the A70CM
    //masterXfer.slaveAddress = addr;
    masterXfer.direction      = kI2C_Read;
    masterXfer.subaddress     = 0;
    masterXfer.subaddressSize = 0;
    masterXfer.data           = pRx;
    masterXfer.dataSize       = rxLen;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    I2CM_TX();

    DEBUG_PRINT_KINETIS_I2C("RD", result);
    RETURN_ON_BAD_kinetisI2cStatus(result);

    /* RETURN_ON_BAD_kinetisI2cStatus return the i2c_error_t status hence returning I2C_OK here */
    return I2C_OK;
}

#endif /* FSL_FEATURE_SOC_I2C_COUNT */
