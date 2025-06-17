/* Copyright 2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "uwb_board_flash_interface.h"

#include "board.h"
#include "pin_mux.h"
#include "phNxpLogApis_App.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static bool uwb_bus_flash_read_cmd_set(uwb_bus_flash_ctx_t *pCtx);
static bool uwb_bus_flash_write_cmd_set(uwb_bus_flash_ctx_t *pCtx, uint32_t StartAddr);

/*******************************************************************************
 * Variables
 ******************************************************************************/

spifi_command_t cmmd[SPIFI_CMD_NUM] = {
    /* READ */
    {EXT_FLASH_PAGE_SIZE,
        false,
        kSPIFI_DataInput,
        0,
        kSPIFI_CommandAllSerial,
        kSPIFI_CommandOpcodeAddrThreeBytes,
        UWB_BUS_FLASH_READ_OPR},
    /* PROGRAM_PAGE */
    {EXT_FLASH_PAGE_SIZE,
        false,
        kSPIFI_DataOutput,
        0,
        kSPIFI_CommandAllSerial,
        kSPIFI_CommandOpcodeAddrThreeBytes,
        UWB_BUS_FLASH_PROG_PAGE_OPR},
    /* GET_STATUS */
    {1, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, UWB_BUS_FLASH_GET_STATUS_OPR},
    /* ERASE_SECTOR */
    {0,
        false,
        kSPIFI_DataOutput,
        0,
        kSPIFI_CommandAllSerial,
        kSPIFI_CommandOpcodeAddrThreeBytes,
        UWB_BUS_FLASH_ERASE_SECTOR_OPR},
    /* WRITE_ENABLE */
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, UWB_BUS_FLASH_WRITE_ENABLE_OPR},
    /* WRITE_REGISTER */
    {4,
        false,
        kSPIFI_DataOutput,
        0,
        kSPIFI_CommandAllSerial,
        kSPIFI_CommandOpcodeOnly,
        UWB_BUS_FLASH_WRITE_REGISTER_OPR},
    /* RESET_ENABLE */
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, UWB_BUS_FLASH_RESET_ENABLE_OPR},
    /* RESET */
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, UWB_BUS_FLASH_RESET_OPR},
    /*WRITE DISABLE*/
    {0,
        false,
        kSPIFI_DataOutput,
        0,
        kSPIFI_CommandAllSerial,
        kSPIFI_CommandOpcodeOnly,
        UWB_BUS_FLASH_WRITE_DISABLE_OPR},
    /*ERASE BLOCK*/
    {0,
        false,
        kSPIFI_DataOutput,
        0,
        kSPIFI_CommandAllSerial,
        kSPIFI_CommandOpcodeAddrThreeBytes,
        UWB_BUS_FLASH_ERASE_BLOCK_OPR},
    /*FAST READ*/
    {EXT_FLASH_PAGE_SIZE,
        false,
        kSPIFI_DataInput,
        1,
        kSPIFI_CommandAllSerial,
        kSPIFI_CommandOpcodeAddrThreeBytes,
        UWB_BUS_FLASH_FAST_READ_OPR},
};

uwb_bus_status_t uwb_bus_flash_init(uwb_bus_flash_ctx_t *pCtx)
{
    memset(&pCtx->flash_config, 0, sizeof(pCtx->flash_config));

    uint32_t sourceClockFreq;

    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    phOsalUwb_MemCopy(pCtx->command, cmmd, (sizeof(spifi_command_t) * SPIFI_CMD_NUM));

    /* Set SPIFI clock source */
    CLOCK_AttachClk(CONFIG_SPIFI_CLK_SRC);
    sourceClockFreq = CLOCK_GetSpifiClkFreq();
    /* Set the clock divider */
    {
        uint32_t divisor;
        /* Do not set null divisor value */
        divisor = sourceClockFreq / CONFIG_SPI_BAUDRATE;
        CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, divisor ? divisor : 1, false);
    }

    RV4_Init_SPIFIPins();

    /* Initialize SPIFI */
    SPIFI_GetDefaultConfig(&pCtx->flash_config);
    pCtx->flash_config.spiMode = (spifi_spi_mode_t)kSPIFI_QuadMode;
    SPIFI_Init(CONFIG_SPIFI, &pCtx->flash_config);

    EnableIRQ(SPIFI0_IRQn);
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_flash_deinit(uwb_bus_flash_ctx_t *pCtx)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    SPIFI_Deinit(CONFIG_SPIFI);
    DisableIRQ(SPIFI0_IRQn);

    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_flash_reset(uwb_bus_flash_ctx_t *pCtx)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    SPIFI_ResetCommand(CONFIG_SPIFI);

    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_flash_status(uwb_bus_flash_ctx_t *pCtx)
{
    uint32_t val = 0;

    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    /* Check WIP bit */
    do {
        SPIFI_SetCommand(CONFIG_SPIFI, &pCtx->command[UWB_BUS_FLASH_GET_STATUS_CMD]);
        while ((CONFIG_SPIFI->STAT & SPIFI_STAT_INTRQ_MASK) == 0U) {
        }
        val = SPIFI_ReadPartialWord(CONFIG_SPIFI, pCtx->command[UWB_BUS_FLASH_GET_STATUS_CMD].dataLen);
    } while (val & 0x1);

    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_flash_write(
    uwb_bus_flash_ctx_t *pCtx, const uint8_t *pBuf, uint32_t bufLen, uint32_t StartAddr)
{
    uint32_t word = 0, byte = 0, wrCount = 0;
    uint32_t page = 0, sector = 0, block = 0, data = 0;
    uint32_t flashStartAddr = 0;

    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    if (pBuf == NULL || bufLen == 0) {
        return kUWB_bus_Status_FAILED;
    }

    LOG_D("Writing %d bytes", bufLen);
    uwb_bus_flash_reset(pCtx);
    /* Program page */
    while (block < EXT_FLASH_SIZE / EXT_FLASH_BLOCK_SIZE) {
        sector = 0;
        while (sector < EXT_FLASH_BLOCK_SIZE / EXT_FLASH_SECTOR_SIZE) {
            page = 0;
            while (page < (EXT_FLASH_SECTOR_SIZE / EXT_FLASH_PAGE_SIZE)) {
                flashStartAddr = StartAddr + (page * EXT_FLASH_PAGE_SIZE) + (sector * EXT_FLASH_SECTOR_SIZE) +
                                 (block * EXT_FLASH_BLOCK_SIZE);
                uwb_bus_flash_write_cmd_set(pCtx, flashStartAddr);

                for (word = 0; word < EXT_FLASH_PAGE_SIZE; word += 4) {
                    for (byte = 0; byte < 4; byte++) {
                        if (wrCount < (bufLen)) {
                            data |=
                                ((uint32_t)(pBuf[word + byte + (page * EXT_FLASH_PAGE_SIZE) +
                                                 (sector * EXT_FLASH_SECTOR_SIZE) + (block * EXT_FLASH_BLOCK_SIZE)]))
                                << (byte * 8);
                        }
                        else {
                            data |= 0xff << (byte * 8); /*since we write a complete page*/
                        }
                        wrCount++; /* number of bytes written */
                    }
#if RdWrLogsEnable
                    LOG_I("addr: 0x%x      data:%x\r\n",
                        (word + (page * EXT_FLASH_PAGE_SIZE) + (sector * EXT_FLASH_SECTOR_SIZE) +
                            (block * EXT_FLASH_BLOCK_SIZE)),
                        data);
#endif //RdWrLogsEnable

                    SPIFI_WriteData(CONFIG_SPIFI, data);
                    data = 0;
                }
                page++;
                uwb_bus_flash_status(pCtx);
                if (page > (bufLen / EXT_FLASH_PAGE_SIZE)) {
                    goto PPexit;
                }
            }
            sector++;
        }
        block++;
    }
PPexit:
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_flash_read(uwb_bus_flash_ctx_t *pCtx, uint8_t *pBuf, uint32_t bufLen, uint32_t StartAddr)
{
    uint8_t *val;
    static bool readCmdSet = FALSE;

    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    if (readCmdSet == FALSE)
        uwb_bus_flash_read_cmd_set(pCtx);

    for (uint32_t i = 0; i < bufLen; i++) {
        val     = (uint8_t *)(StartAddr + i);
        pBuf[i] = *val;
    }

    return kUWB_bus_Status_OK;
}

static bool uwb_bus_flash_read_cmd_set(uwb_bus_flash_ctx_t *pCtx)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return FALSE;
    }
    SPIFI_SetMemoryCommand(CONFIG_SPIFI, &pCtx->command[UWB_BUS_FLASH_READ_CMD]);
    return TRUE;
}

uwb_bus_status_t uwb_bus_flash_erase(uwb_bus_flash_ctx_t *pCtx, uint32_t StartAddr, uint32_t FwLen)
{
    uint32_t i = 0;

    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    LOG_I("Erasing Block..................!");
    /*Erase Blocks*/
    for (i = 0; i <= (FwLen + EXT_FLASH_PAGE_SIZE) / EXT_FLASH_BLOCK_SIZE; i++) {
        /* Write enable */
        SPIFI_SetCommand(CONFIG_SPIFI, &pCtx->command[UWB_BUS_FLASH_WRITE_ENABLE_CMD]);
        /* Set address */
        SPIFI_SetCommandAddress(CONFIG_SPIFI, StartAddr + (i * EXT_FLASH_BLOCK_SIZE));
        /* Erase sector */
        SPIFI_SetCommand(CONFIG_SPIFI, &pCtx->command[UWB_BUS_FLASH_ERASE_BLOCK_CMD]);
        /* Check if finished */
        uwb_bus_flash_status(pCtx);
    }
    return kUWB_bus_Status_OK;
}

static bool uwb_bus_flash_write_cmd_set(uwb_bus_flash_ctx_t *pCtx, uint32_t StartAddr)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus flash context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    SPIFI_SetCommand(CONFIG_SPIFI, &pCtx->command[UWB_BUS_FLASH_WRITE_ENABLE_CMD]);
    SPIFI_SetCommandAddress(CONFIG_SPIFI, StartAddr);
    SPIFI_SetCommand(CONFIG_SPIFI, &pCtx->command[UWB_BUS_FLASH_PROGRAM_PAGE_CMD]);

    return kUWB_bus_Status_OK;
}
