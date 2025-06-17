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

#ifndef __UWB_BUS_FLASH_BOARD_H__
#define __UWB_BUS_FLASH_BOARD_H__

#include "board.h"
#include "uwb_bus_interface.h"

#define MAINLINE_FW_CRC_START_ADDR (0x10000000)
#define MAINLINE_FW_LEN_START_ADDR (0x10000002)
#define MAINLINE_FW_START_ADDR     (0x10000100)

#define FACTORY_FW_CRC_START_ADDR (0x10050000)
#define FACTORY_FW_LEN_START_ADDR (0x10050002)
#define FACTORY_FW_START_ADDR     (0x10050100)

/** Max Time to wait for SPI data transfer completion */
#define MAX_UWBS_SPIFI_TRANSFER_TIMEOUT (1000)

#define CLOCK_ABSTRACTION
/* Abstract attaching the clock */
#define CONFIG_SPIFI_CLK_SRC         (kMAIN_CLK_to_SPIFI) // (kMAIN_CLK_to_SPIFI)
#define CONFIG_SPIFI_ATTACH_MAIN_CLK (CLOCK_AttachClk(CONFIG_SPIFI_CLK_SRC))
/* Abstract getting the clock */
#define CONFIG_SPIFI_CLK        (kCLOCK_Spifi)
#define CONFIG_SPIFI_CLOCK_FREQ (CLOCK_GetFreq(CONFIG_SPIFI_CLK))

#define CONFIG_SPIFI        (SPIFI)
#define EXT_FLASH_PAGE_SIZE (256U) // 1 page size is  256 Bytes

#define EXT_FLASH_SECTOR_SIZE \
    (16 * EXT_FLASH_PAGE_SIZE) // 1 sector contains 16 pages, Sector Size = (16 * EXT_FLASH_PAGE_SIZE)

#define EXT_FLASH_BLOCK_SIZE \
    (16 * EXT_FLASH_SECTOR_SIZE) // 1 Block contains 16 Sectors, Block size = (16 * EXT_FLASH_SECTOR_SIZE)

#define EXT_FLASH_SIZE \
    (32 * EXT_FLASH_BLOCK_SIZE) // Total Flash contain 32 Block, Flash Size = ( 32 * EXT_FLASH_BLOCK_SIZE)

#define CONFIG_SPI_BAUDRATE (8000000L)

#define SPIFI_CMD_NUM (11)

/** Board Specific FLASH Interface for the Host HAL */

/** Flash Operations.
 *
 *
 */
typedef enum flash_operation
{
    UWB_BUS_FLASH_READ_CMD,           /* Read Command */
    UWB_BUS_FLASH_PROGRAM_PAGE_CMD,   /* Program Page Command */
    UWB_BUS_FLASH_GET_STATUS_CMD,     /* Get Status Command */
    UWB_BUS_FLASH_ERASE_SECTOR_CMD,   /* Erase Sector Command */
    UWB_BUS_FLASH_WRITE_ENABLE_CMD,   /* Write Enable Command */
    UWB_BUS_FLASH_WRITE_REGISTER_CMD, /* Write Register Command */
    UWB_BUS_FLASH_RESET_ENABLE_CMD,   /* Reset Enable Command */
    UWB_BUS_FLASH_RESET_CMD,          /* Reset Command */
    UWB_BUS_FLASH_WRITE_DISABLE_CMD,  /* Write Disable Command */
    UWB_BUS_FLASH_ERASE_BLOCK_CMD,    /* Erase Block Command */
    UWB_BUS_FLASH_FAST_READ_CMD       /* Fast Read Command */

} eFlashOperation;

/** Flash Commands.
 *
 *
 */
typedef enum flash_command
{
    UWB_BUS_FLASH_READ_OPR           = 0x03, /* Read Operation */
    UWB_BUS_FLASH_PROG_PAGE_OPR      = 0x02, /* Page Program Operation */
    UWB_BUS_FLASH_GET_STATUS_OPR     = 0x05, /* Get Status Operation (check WIP bit) */
    UWB_BUS_FLASH_ERASE_SECTOR_OPR   = 0x20, /* Erase Sector Operation */
    UWB_BUS_FLASH_WRITE_ENABLE_OPR   = 0x06, /* Write Enable Operation */
    UWB_BUS_FLASH_WRITE_REGISTER_OPR = 0x01, /* Write Register Operation */
    UWB_BUS_FLASH_RESET_ENABLE_OPR   = 0x66, /* Reset Enable Operation */
    UWB_BUS_FLASH_RESET_OPR          = 0x99, /* Reset Operation */
    UWB_BUS_FLASH_WRITE_DISABLE_OPR  = 0x04, /* Write Disable Operation */
    UWB_BUS_FLASH_ERASE_BLOCK_OPR    = 0xD8, /* Erase Block Operation */
    UWB_BUS_FLASH_FAST_READ_OPR      = 0x0B  /* Fast Read Operation */
} eFlashCommand;

/** Board Specific FLASH Interface for the Host HAL */
typedef struct
{
    /* SPIFI flash config */
    spifi_config_t flash_config;
    /* SPIFI command structure */
    spifi_command_t command[SPIFI_CMD_NUM];
} uwb_bus_flash_ctx_t;

/** @} */
#endif // __UWB_BUS_FLASH_BOARD_H__
