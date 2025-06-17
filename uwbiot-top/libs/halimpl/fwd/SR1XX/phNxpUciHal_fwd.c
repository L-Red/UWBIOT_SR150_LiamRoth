/*
 *
 * Copyright 2018-2020,2022 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

/*************************************************************************************/
/*   INCLUDES */
/*************************************************************************************/

#include "phNxpUciHal_fwd.h"

#include "phUwb_BuildConfig.h"
#include "phNxpLogApis_FwDnld.h"
#include <driver_config.h>
#include <phNxpUciHal_utils.h>
#include <phTmlUwb_transport.h>

#include "uwb_fwdl_provider.h"

uwb_fwdl_provider_t fwdlCtx;

#if UWBIOT_UWBD_SR1XXT

#define ENABLE_FW_DOWNLOAD_LOG FALSE

phHbci_MosiApdu_t gphHbci_MosiApdu;
phHbci_MisoApdu_t gphHbci_MisoApdu;
Options_t gOpts;

/*************************************************************************************/
/*   LOCAL FUNCTIONS */
/*************************************************************************************/
static void setOpts(void)
{
    gOpts.link     = Link_Default;
    gOpts.mode     = Mode_Default;
    gOpts.capture  = Capture_Default;
    gOpts.imgFile  = NULL;
    gOpts.mosiFile = (char *)"Mosi.bin";
    gOpts.misoFile = (char *)"Miso.bin";
}

void UWB_HeliosCE(bool set)
{
    (void)phTmlUwb_io_set(kUWBS_IO_O_ENABLE_HELIOS, set);
    (void)phTmlUwb_io_set(kUWBS_IO_O_HELIOS_RTC_SYNC, set);
}
static int init(void)
{
    UWB_HeliosCE(0);
    phOsalUwb_Delay(5); // Delay in Millisecond/
    UWB_HeliosCE(1);

    phOsalUwb_Delay(5);

    if (Capture_Off != gOpts.capture) {
        NXPLOG_UWB_FWDNLD_D("Not Capture_Off...");
    }

    return 0;
}

phHbci_Status_t phHbci_GetStatus(void)
{
    size_t rcvlen = PHHBCI_MAX_LEN_DATA_MISO;
    NXPLOG_UWB_FWDNLD_D("phHbci_GetStatus Enter");
    gphHbci_MosiApdu.len = 0;
    gphHbci_MisoApdu.len = PHHBCI_LEN_HDR;

    if ((phTmlUwb_hbci_transcive(
            (uint8_t *)&gphHbci_MosiApdu, PHHBCI_LEN_HDR, (uint8_t *)&gphHbci_MisoApdu, &rcvlen)) ==
        kUWBSTATUS_SUCCESS) {
        gphHbci_MisoApdu.len = (uint16_t)rcvlen;
        return phHbci_Success;
    }
    gphHbci_MisoApdu.len = (uint16_t)rcvlen;
    return phHbci_Failure;
}

phHbci_Status_t phHbci_GeneralStatus(phHbci_General_Command_t mode)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_GeneralStatus Enter");
    switch (gphHbci_MisoApdu.cls) {
    case phHbci_Class_General | phHbci_SubClass_Answer:
        switch (gphHbci_MisoApdu.ins) {
        case phHbci_General_Ans_HBCI_Ready:
            if (!mode) {
                return phHbci_Success;
            }

            NXPLOG_UWB_FWDNLD_E("ERROR: Unexpected General Status 0x%02x In Mode 0x%02x", gphHbci_MisoApdu.ins, mode);
            break;

        case phHbci_General_Ans_Mode_Patch_ROM_Ready:
            if (phHbci_General_Cmd_Mode_Patch_ROM == mode) {
                return phHbci_Success;
            }

            NXPLOG_UWB_FWDNLD_E("ERROR: Unexpected General Status 0x%02x In Mode 0x%02x", gphHbci_MisoApdu.ins, mode);
            break;

        case phHbci_General_Ans_Mode_HIF_Image_Ready:
            if (phHbci_General_Cmd_Mode_HIF_Image == mode) {
                return phHbci_Success;
            }

            NXPLOG_UWB_FWDNLD_E("ERROR: Unexpected General Status 0x%02x In Mode 0x%02x", gphHbci_MisoApdu.ins, mode);
            break;

        case phHbci_General_Ans_HBCI_Fail:
        case phHbci_General_Ans_Boot_Autoload_Fail:
        case phHbci_General_Ans_Boot_GPIOConf_CRC_Fail:
        case phHbci_General_Ans_Boot_TRIM_CRC_Fail:
        case phHbci_General_Ans_Boot_GPIOTRIM_CRC_Fail:
            NXPLOG_UWB_FWDNLD_E("ERROR: HBCI Interface Failed With 0x%02x", gphHbci_MisoApdu.ins);
            break;

        case phHbci_General_Ans_Mode_Patch_ROM_Fail:
            NXPLOG_UWB_FWDNLD_E("ERROR: Patch ROM Mode Failed!");
            break;

        case phHbci_General_Ans_Mode_HIF_Image_Fail:
            NXPLOG_UWB_FWDNLD_E("ERROR: HIF Image Mode Failed!");
            break;

        default:
            NXPLOG_UWB_FWDNLD_E("ERROR: Unknown General Status 0x%02x", gphHbci_MisoApdu.ins);
            break;
        }
        break;

    case phHbci_Class_General | phHbci_SubClass_Ack:
        switch (gphHbci_MisoApdu.ins) {
        case phHbci_Invlaid_Class:
            NXPLOG_UWB_FWDNLD_E("ERROR: Invalid Class Error From Slave!");
            break;

        case phHbci_Invalid_Instruction:
            NXPLOG_UWB_FWDNLD_E("ERROR: Invalid Instruction Error From Slave!");
            break;

        default:
            NXPLOG_UWB_FWDNLD_E("ERROR: Unexpected Instruction From Slave 0x%02x", gphHbci_MisoApdu.ins);
            break;
        }
        break;

    default:
        NXPLOG_UWB_FWDNLD_E("ERROR: Unknown General Class 0x%02x", gphHbci_MisoApdu.cls);
        break;
    }

    return phHbci_Failure;
}

static phHbci_Status_t phHbci_MasterPatchROM()
{
    NXPLOG_UWB_FWDNLD_D("phHbci_MasterPatchROM enter");
    phHbci_Status_t ret = phHbci_Failure;

    gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Query);
    gphHbci_MosiApdu.ins = (uint8_t)phHbci_General_Qry_Status;

    while (1) {
        if (phHbci_Success != (ret = phHbci_GetStatus())) {
            return ret;
        }

        switch (gphHbci_MisoApdu.cls) {
        case phHbci_Class_General | phHbci_SubClass_Answer:
        case phHbci_Class_General | phHbci_SubClass_Ack:
            if (phHbci_Success != (ret = phHbci_GeneralStatus(phHbci_General_Cmd_Mode_Patch_ROM))) {
                return ret;
            }

            gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_Patch_ROM | phHbci_SubClass_Command);
            gphHbci_MosiApdu.ins = (uint8_t)phHbci_Patch_ROM_Cmd_Download_Patch;

            fwdlCtx.uwb_fwdl_MosiApdu = gphHbci_MosiApdu;
            fwdlCtx.uwb_fwdl_MisoApdu = gphHbci_MisoApdu;
            if (kUWBSTATUS_SUCCESS != uwb_fwdl_downloadFw(&fwdlCtx)) {
                return phHbci_Failure;
            }

            gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_Patch_ROM | phHbci_SubClass_Query);
            gphHbci_MosiApdu.ins = (uint8_t)phHbci_Patch_ROM_Qry_Patch_Status;
            break;

        case phHbci_Class_Patch_ROM | phHbci_SubClass_Answer:
            switch (gphHbci_MisoApdu.ins) {
            case phHbci_Patch_ROM_Ans_Patch_Success:
                NXPLOG_UWB_FWDNLD_D("Patch ROM Transfer Complete.");
                ret = phHbci_Success;
                break;

            case phHbci_Patch_ROM_Ans_File_Too_Large:
            case phHbci_Patch_ROM_Ans_Invalid_Patch_File_Marker:
            case phHbci_Patch_ROM_Ans_Too_Many_Patch_Table_Entries:
            case phHbci_Patch_ROM_Ans_Invalid_Patch_Code_Size:
            case phHbci_Patch_ROM_Ans_Invalid_Global_Patch_Marker:
            case phHbci_Patch_ROM_Ans_Invalid_Signature_Size:
            case phHbci_Patch_ROM_Ans_Invalid_Signature:
                NXPLOG_UWB_FWDNLD_E("EROOR: Patch ROM Transfer Failed With 0x%02x!", gphHbci_MisoApdu.ins);
                ret = phHbci_Failure;
                break;

            default:
                NXPLOG_UWB_FWDNLD_E("ERROR: Unknown Patch ROM Status 0x%02x", gphHbci_MisoApdu.ins);
                ret = phHbci_Failure;
                break;
            }
            return ret;

        default:
            NXPLOG_UWB_FWDNLD_E("ERROR: Unknown Class 0x%02x", gphHbci_MisoApdu.cls);
            return phHbci_Failure;
        }
    }

    return phHbci_Success;
}

static phHbci_Status_t phHbci_MasterHIFImage()
{
    NXPLOG_UWB_FWDNLD_D("phHbci_MasterHIFImage enter");
    phHbci_Status_t ret = phHbci_Failure;

    gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Query);
    gphHbci_MosiApdu.ins = (uint8_t)phHbci_General_Qry_Status;

    while (1) {
        if (phHbci_Success != (ret = phHbci_GetStatus())) {
            return ret;
        }

        switch (gphHbci_MisoApdu.cls) {
        case phHbci_Class_General | phHbci_SubClass_Answer:
        case phHbci_Class_General | phHbci_SubClass_Ack:
            if (phHbci_Success != (ret = phHbci_GeneralStatus(phHbci_General_Cmd_Mode_HIF_Image))) {
                return ret;
            }

            gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_HIF_Image | phHbci_SubClass_Command);
            gphHbci_MosiApdu.ins = (uint8_t)phHbci_HIF_Image_Cmd_Download_Image;

            fwdlCtx.uwb_fwdl_MosiApdu = gphHbci_MosiApdu;
            fwdlCtx.uwb_fwdl_MisoApdu = gphHbci_MisoApdu;
            if (kUWBSTATUS_SUCCESS != uwb_fwdl_downloadFw(&fwdlCtx)) {
                return phHbci_Failure;
            }

            phOsalUwb_Delay(60);

            gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_HIF_Image | phHbci_SubClass_Query);
            gphHbci_MosiApdu.ins = (uint8_t)phHbci_HIF_Image_Qry_Image_Status;
            break;

        case phHbci_Class_HIF_Image | phHbci_SubClass_Answer:
            switch (gphHbci_MisoApdu.ins) {
            case phHbci_HIF_Image_Ans_Image_Success:
                NXPLOG_UWB_FWDNLD_D("HIF Image Transfer Complete.");
                /*Check FW download throughput measurement*/
                return phHbci_Success;

            case phHbci_HIF_Image_Ans_Header_Too_Large:
            case phHbci_HIF_Image_Ans_Header_Parse_Error:
            case phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Crypto:
            case phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Hash:
            case phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Curve:
            case phHbci_HIF_Image_Ans_Invalid_ECC_Key_Length:
            case phHbci_HIF_Image_Ans_Invalid_Payload_Description:
            case phHbci_HIF_Image_Ans_Invalid_Firmware_Version:
            case phHbci_HIF_Image_Ans_Invalid_ECID_Mask:
            case phHbci_HIF_Image_Ans_Invalid_ECID_Value:
            case phHbci_HIF_Image_Ans_Invalid_Encrypted_Payload_Hash:
            case phHbci_HIF_Image_Ans_Invalid_Header_Signature:
            case phHbci_HIF_Image_Ans_Install_Settings_Too_Large:
            case phHbci_HIF_Image_Ans_Install_Settings_Parse_Error:
            case phHbci_HIF_Image_Ans_Payload_Too_Large:
            case phHbci_HIF_Image_Ans_Quickboot_Settings_Parse_Error:
            case phHbci_HIF_Image_Ans_Invalid_Static_Hash:
            case phHbci_HIF_Image_Ans_Invalid_Dynamic_Hash:
            case phHbci_HIF_Image_Ans_Execution_Settings_Parse_Error:
            case phHbci_HIF_Image_Ans_Key_Read_Error:
                NXPLOG_UWB_FWDNLD_E("EROOR: HIF Image Transfer Failed With 0x%02x!", gphHbci_MisoApdu.ins);
                return phHbci_Failure;
            default:
                NXPLOG_UWB_FWDNLD_E("ERROR: Unknown HIF Status 0x%02x", gphHbci_MisoApdu.ins);
                return phHbci_Failure;
            }
            break;

        default:
            NXPLOG_UWB_FWDNLD_E("ERROR: Unknown Class 0x%02x", gphHbci_MisoApdu.cls);
            return phHbci_Failure;
        }
    }

    return phHbci_Success;
}
static phHbci_Status_t phHbci_MasterSetMode(phHbci_General_Command_t mode)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_MasterSetMode enter");
    size_t rcvlen        = PHHBCI_MAX_LEN_DATA_MISO;
    gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Command);
    gphHbci_MosiApdu.ins = (uint8_t)mode;

    gphHbci_MosiApdu.len = 0;
    gphHbci_MisoApdu.len = PHHBCI_LEN_HDR;

    if ((phTmlUwb_hbci_transcive(
            (uint8_t *)&gphHbci_MosiApdu, PHHBCI_LEN_HDR, (uint8_t *)&gphHbci_MisoApdu, &rcvlen)) ==
        kUWBSTATUS_SUCCESS) {
        gphHbci_MisoApdu.len = (uint16_t)rcvlen;
        return phHbci_Success;
    }
    gphHbci_MisoApdu.len = (uint16_t)rcvlen;
    return phHbci_Failure;
}

/*********************************************************************************************************************/
/*   GLOBAL FUNCTIONS */
/*********************************************************************************************************************/
phHbci_Status_t phHbci_Master(phHbci_General_Command_t mode)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_Master Enter");
    phHbci_Status_t ret = phHbci_Failure;

    gphHbci_MosiApdu.cls     = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Query);
    gphHbci_MosiApdu.ins     = (uint8_t)phHbci_General_Qry_Status;
    gphHbci_MosiApdu.payload = (uint8_t *)phOsalUwb_GetMemory(PHHBCI_MAX_LEN_PAYLOAD_MOSI);
    if (gphHbci_MosiApdu.payload == NULL) {
        return ret;
    }
    if (phHbci_Success != (ret = phHbci_GetStatus())) {
        goto exit;
    }
    if (phHbci_Success != (ret = phHbci_GeneralStatus((phHbci_General_Command_t)0))) {
        goto exit;
    }

    if (phHbci_Success != (ret = phHbci_MasterSetMode(mode))) {
        goto exit;
    }

    switch (mode) {
    case phHbci_General_Cmd_Mode_Patch_ROM: {
        ret = phHbci_MasterPatchROM();
    } break;

    case phHbci_General_Cmd_Mode_HIF_Image: {
        ret = phHbci_MasterHIFImage();
    } break;

    default:
        NXPLOG_UWB_FWDNLD_E("ERROR: Undefined mode 0x%02x", mode);
        break;
    }

exit:
    phOsalUwb_FreeMemory(gphHbci_MosiApdu.payload);
    gphHbci_MosiApdu.payload = NULL;
    return ret;
}

/*********************************************************************************************************************/
/*   GLOBAL FUNCTIONS */
/*********************************************************************************************************************/
uint8_t phHbci_CalcLrc(uint8_t *pBuf, uint16_t bufSz)
{
    uint8_t lrc = 0;
    uint16_t i;

    if (!pBuf || !bufSz) {
        return lrc;
    }

    /* ISO 1155:1978 Information processing -- Use of longitudinal parity to
   * detect errors in information messages */
    for (i = 0; i < bufSz; i++) {
        lrc = (uint8_t)(lrc + *pBuf++);
    }

    lrc ^= 0xFF;
    lrc = (uint8_t)(lrc + 1);

    return lrc;
}

/******************************************************************************
 * Function         phNxpUciHal_fw_download
 *
 * Description      This function is called by jni when wired mode is
 *                  performed.First SR100 driver will give the access
 *                  permission whether wired mode is allowed or not
 *                  arg (0):
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the acutual state of operation in arg pointer
 *
 ******************************************************************************/
int phNxpUciHal_fw_download()
{
    uint32_t err = 0;
    phHbci_General_Command_t cmd;

    NXPLOG_UWB_FWDNLD_D("phNxpUciHal_fw_download enter and FW download started...");
    setOpts();

    if (init()) {
        NXPLOG_UWB_FWDNLD_E("INIT Failed...");
        return 1;
    }

    switch (gOpts.mode) {
    case Mode_Patch_ROM:
        cmd = phHbci_General_Cmd_Mode_Patch_ROM;
        break;

    case Mode_HIF_Image:
        cmd = phHbci_General_Cmd_Mode_HIF_Image;
        break;

    default:
        NXPLOG_UWB_FWDNLD_E("ERROR: Undefined Master Mode = %u", gOpts.mode);
        return 1;
    }

    if (cmd == phHbci_General_Cmd_Mode_HIF_Image) {
        NXPLOG_UWB_FWDNLD_D("HIF Image mode.");
    }

    phTmlUwb_set_hbci_mode();
    gphHbci_MisoApdu.payload = (uint8_t *)phOsalUwb_GetMemory(PHHBCI_MAX_LEN_PAYLOAD_MISO);
    if (gphHbci_MisoApdu.payload == NULL) {
        NXPLOG_UWB_FWDNLD_E("Unable to allocate buffer");
        return 1;
    }
    if (phHbci_Success != phHbci_Master(cmd)) {
        NXPLOG_UWB_FWDNLD_E("Failure!");
        err = 1;
    }
    phTmlUwb_set_uci_mode();

    phOsalUwb_FreeMemory(gphHbci_MisoApdu.payload);
    gphHbci_MisoApdu.payload = NULL;
    return err;
}

#endif // UWBIOT_UWBD_SR1XXT
