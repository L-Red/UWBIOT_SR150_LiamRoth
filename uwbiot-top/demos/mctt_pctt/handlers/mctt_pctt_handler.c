/* Copyright 2019-2021,2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"
#if !defined(UWBIOT_APP_BUILD__DEMO_MCTT_PCTT)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_MCTT_PCTT)
#include "mctt_pctt_handler.h"
#include "UwbApi.h"
#include "UwbHif.h"
#include "AppInternal.h"
#define MAX_CALIB_VALUE 16
static BOOLEAN recoveryMode = FALSE;

#define INBAND_DATA_PARAMVALUE 6
static BOOLEAN gInBandDataParamFlag = FALSE;

#if SSS_HAVE_APPLET_SE051_UWB
#include "smCom.h"
#include "ex_sss_boot.h"
AppContext_t appContext;
static ex_sss_boot_ctx_t ex_sss_boot_ctx;
#define PCONTEXT                 (&ex_sss_boot_ctx)
#define FIRALITE_WRAPPED_RDS_LEN (68)
uint8_t gWrappedRDS[FIRALITE_WRAPPED_RDS_LEN];
uint16_t gWrappedRDSLen = sizeof(gWrappedRDS);

#endif //SSS_HAVE_APPLET_SE051_UWB

#if UWBFTR_SE_SN110
#include "WearableCoreSDK_BuildConfig.h"
#include "SeApi.h"
#include "wearable_platform_int.h"
#include "phTmlUwb_transport.h"
extern void Enable_GPIO0_IRQ();
#endif // UWBFTR_SE_SN110

#define RF_CLK_ACCURACY_CALIB_LEN 0x07
/**
 * 1B noOfEntries
 * 5B caliberationValue
 * LEN =( noOfEntries + (numberofEntries * caliberationValue))
*/
#define TX_POWER_PER_ANT_LEN(entries) (1 + (entries * 5))
#define CHANNEL_5                     0x05
#define CHANNEL_9                     0x09

tUWBAPI_STATUS demo_setVendorAppconfigs(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status        = UWBAPI_STATUS_FAILED;
    uint8_t inBandDataParamValue = FALSE;

    /**
     * This setting are required for every session Type.
     * RX configuration settings are required for the All AOA test cases to pass.
    */
    uint8_t antennaeConfigurationRx[]                         = {kUWBAntCfgRxMode_AoA_Mode, 0x01, 0x01};
    const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_ARRAY(
            ANTENNAE_CONFIGURATION_RX, &antennaeConfigurationRx[0], sizeof(antennaeConfigurationRx)),
    };

    status = UwbApi_SetVendorAppConfigs(
        sessionHandle, sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]), &SetVendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetVendorAppConfigs FAILED");
        goto exit;
    }

    //depending upon session type applying Vendor app config value 6 or 0
    if (gInBandDataParamFlag == TRUE) {
        inBandDataParamValue = INBAND_DATA_PARAMVALUE;

        const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
            UWB_SET_VENDOR_APP_PARAM_VALUE(SESSION_INBAND_DATA_TX_BLOCKS, inBandDataParamValue),
            UWB_SET_VENDOR_APP_PARAM_VALUE(SESSION_INBAND_DATA_RX_BLOCKS, inBandDataParamValue),
        };

        status = UwbApi_SetVendorAppConfigs(sessionHandle,
            sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]),
            &SetVendorAppParamsList[0]);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_SetVendorAppConfigs FAILED");
            goto exit;
        }
    }
    else {
        status = UWBAPI_STATUS_OK;
    }
exit:
    return status;
}

#if SSS_HAVE_APPLET_SE051_UWB
tUWBAPI_STATUS demo_set_wraped_rds(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    se_rds_t seRds;
    firaLiteWrappedRds_t flWrappedRds;
    se_applet_t applet = SE_APPLET_FIRALITE;
    uint8_t wrappedRds[FIRALITE_WRAPPED_RDS_LEN];
    size_t wrappedRdsLen  = FIRALITE_WRAPPED_RDS_LEN;
    se_status_t se_status = SE_STATUS_NOT_OK;

    flWrappedRds.wrappedRDS    = &gWrappedRDS[0];
    flWrappedRds.wrappedRDSLen = gWrappedRDSLen;
    seRds.pFlWrappedRds        = &flWrappedRds;

    Se_API_DeInit();
    se_status = Se_API_GetWrappedRDS(applet, &seRds, wrappedRds, &wrappedRdsLen);
    if (se_status != SE_STATUS_OK) {
        LOG_E("Se_API_GetWrappedRDS() failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigWrappedRDS(sessionHandle, wrappedRds, wrappedRdsLen);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetAppConfigWrappedRDS Failed");
        goto exit;
    }

exit:
    return status;
}
#endif //SSS_HAVE_APPLET_SE051_UWB

tUWBAPI_STATUS mctt_pctt_common_deinit(void)
{
    tUWBAPI_STATUS status;
#if SSS_HAVE_APPLET_SE051_UWB
    ex_sss_session_close(PCONTEXT);
#elif UWBFTR_SE_SN110
    tSEAPI_STATUS se_status;
    se_status = SeApi_Shutdown();
    if (se_status != SEAPI_STATUS_OK) {
        NXPLOG_APP_E("SeApi_Init() Failed");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    se_status = SeApi_WiredEnable(FALSE);
    if (se_status != SEAPI_STATUS_OK) {
        NXPLOG_APP_E("SeApi_WiredEnable() Failed");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
#endif
    status = UwbApi_ShutDown();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ShutDown Failed");
        return UWBAPI_STATUS_FAILED;
        goto exit; //Just to avoid compilation error
    }
exit:
    return status;
}

tUWBAPI_STATUS mctt_pctt_ReadOtpData(uint8_t channel)
{
    tUWBAPI_STATUS status;
    uint16_t bitMask;
    phCalibPayload_t readCalibData = {0x00};

    uint8_t calibValues[MAX_CALIB_VALUE] = {0x00};
    uint8_t *pSetCalibValue;
    bitMask = (VCO_PLL_POS | PAPPPA_CALIB_CTRL_POS | TX_POWER_POS | XTAL_CAP_VALUES_POS);
    status  = UwbApi_ReadOtpCalibDataCmd(channel, bitMask, &readCalibData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ReadOtpCalibDataCmd Failed");
        return UWBAPI_STATUS_FAILED;
    }

    if (readCalibData.VCO_PLL != 0) {
        pSetCalibValue = calibValues;
        UWB_UINT16_TO_STREAM(pSetCalibValue, readCalibData.VCO_PLL);
        status = UwbApi_SetCalibration(channel, VCO_PLL, calibValues, sizeof(readCalibData.VCO_PLL));
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("Set Calib param VCO_PLL Failed");
            return UWBAPI_STATUS_FAILED;
        }
    }

#if !UWBIOT_UWBD_SR2XXT
    if (readCalibData.PA_PPA_CALIB_CTRL != 0) {
        pSetCalibValue = calibValues;
        UWB_UINT16_TO_STREAM(pSetCalibValue, readCalibData.PA_PPA_CALIB_CTRL);
        status =
            UwbApi_SetCalibration(channel, PA_PPA_CALIB_CTRL, calibValues, sizeof(readCalibData.PA_PPA_CALIB_CTRL));
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("Set Calib param PA_PPA_CALIB_CTRL Failed");
            return UWBAPI_STATUS_FAILED;
        }
    }
#endif // !UWBIOT_UWBD_SR2XXT

    if (readCalibData.XTAL_CAP_VALUES[0] != 0) {
        pSetCalibValue = calibValues;
        phOsalUwb_SetMemory(calibValues, 0x00, sizeof(calibValues));

        calibValues[0] = 0x03; // Number of registers(must be 0x03)
        calibValues[1] = readCalibData.XTAL_CAP_VALUES[0];
        calibValues[3] = readCalibData.XTAL_CAP_VALUES[1];
        calibValues[5] = readCalibData.XTAL_CAP_VALUES[2];
        status         = UwbApi_SetCalibration(channel, RF_CLK_ACCURACY_CALIB, calibValues, RF_CLK_ACCURACY_CALIB_LEN);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("Set Calib param RF_CLK_ACCURACY_CALIB Failed");
            return UWBAPI_STATUS_FAILED;
        }
    }
    if (readCalibData.TX_POWER_ID[0] != 0) {
        pSetCalibValue = calibValues;
        phOsalUwb_SetMemory(calibValues, 0x00, sizeof(calibValues));

        uint8_t noOfEntries = 0x02;

        calibValues[0]  = noOfEntries;                  // No. of Entries
        calibValues[1]  = 0x01;                         // Antenna Id 1
        calibValues[3]  = readCalibData.TX_POWER_ID[0]; // Tx Power Delta Peak
        calibValues[5]  = readCalibData.TX_POWER_ID[1]; // Tx Power Id RMS
        calibValues[7]  = 0x02;                         // Antenna Id 2
        calibValues[9]  = readCalibData.TX_POWER_ID[2]; // Tx Power Delta Peak
        calibValues[11] = readCalibData.TX_POWER_ID[3]; // Tx Power Id RMS

        status = UwbApi_SetCalibration(channel, TX_POWER_PER_ANTENNA, calibValues, TX_POWER_PER_ANT_LEN(noOfEntries));
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("Set Calib param TX_POWER_PER_ANTENNA Failed");
            return UWBAPI_STATUS_FAILED;
        }
    }
    return UWBAPI_STATUS_OK;
}

tUWBAPI_STATUS mctt_pctt_common_init(void)
{
    tUWBAPI_STATUS status    = UWBAPI_STATUS_FAILED;
    phUwbappContext_t appCtx = {0};
    phUwbDevInfo_t devInfo;
#if SSS_HAVE_APPLET_SE051_UWB
    sss_status_t sss_status          = kStatus_SSS_Fail;
    sss_se05x_session_t *sss_session = NULL;
    pSe05xSession_t pSeSession       = NULL;
    char *portName;
    phOsalUwb_SetMemory(PCONTEXT, 0, sizeof(*PCONTEXT));
#if AX_EMBEDDED
    portName = NULL;
#else
    sss_status = ex_sss_boot_connectstring(0, NULL, &portName);
    if (kStatus_SSS_Success != sss_status) {
        NXPLOG_APP_E("ex_sss_boot_connectstring Failed");
        goto exit;
    }

#endif // AX_EMBEDDED
    PCONTEXT->se05x_open_ctx.skip_select_applet = TRUE;
    sss_status                                  = ex_sss_boot_open(PCONTEXT, portName);
    if (kStatus_SSS_Success != sss_status) {
        NXPLOG_APP_E("ex_sss_boot_open Failed");
        goto exit;
    }
#endif //SSS_HAVE_APPLET_SE051_UWB

#if UWBFTR_SE_SN110
    tSEAPI_STATUS se_status = SEAPI_STATUS_FAILED;
    se_status               = SeApi_Init(NULL, NULL);
    if (se_status != SEAPI_STATUS_OK) {
        NXPLOG_APP_E("SeApi_Init() Failed");
        goto exit;
    }
    Enable_GPIO0_IRQ();
    se_status = SeApi_WiredEnable(TRUE);
    if (se_status != SEAPI_STATUS_OK) {
        NXPLOG_APP_E("SeApi_Init() Failed");
        goto exit;
    }
    LOG_I("SeApi_Init and wired enable success");
#endif

#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = MAINLINE_FW;
    appCtx.pMcttCallback     = NULL;
    appCtx.pCallback         = AppCallback;
    appCtx.pCdcCallback      = NULL;
#if SSS_HAVE_APPLET_SE051_UWB
    sss_session     = (sss_se05x_session_t *)&(PCONTEXT->session);
    pSeSession      = &(sss_session->s_ctx);
    appCtx.seHandle = (void *)pSeSession;
#else
    appCtx.seHandle = NULL;
#endif //SSS_HAVE_APPLET_SE051_UWB

    status = UwbApi_Init_New(&appCtx);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Init_New() Failed");
        goto exit;
    }

    status = UwbApi_GetDeviceInfo(&devInfo);
    printDeviceInfo(&devInfo);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetDeviceInfo() Failed");
        goto exit;
    }

    /** OTP read write is enabled for Sr150, SR160, and SR100S ROW
    Not Required for SR100T Mobile FW */

#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160 || UWBIOT_UWBD_SR100S
    status = mctt_pctt_ReadOtpData(CHANNEL_5);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("ReadOtpData(CHANNEL_5) Failed");
        goto exit;
    }

    status = mctt_pctt_ReadOtpData(CHANNEL_9);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("ReadOtpData(CHANNEL_9) Failed");
        goto exit;
    }
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160 || UWBIOT_UWBD_SR100S

    appCtx.pMcttCallback = McttAppDataCallback;
    appCtx.pCallback     = NULL;
    appCtx.pCdcCallback  = NULL;
    status               = UwbApi_SwitchToMCTTMode(&appCtx);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SwitchToMCTTMode() Failed");
        goto exit;
    }
exit:
    return status;
}

tUWBAPI_STATUS mctt_pctt_common_settings(void)
{
    phDeviceConfigData_t devConfig = {0};
    eDeviceConfig param_id         = LOW_POWER_MODE;
    tUWBAPI_STATUS status;
    devConfig.lowPowerMode = 0x00;
    status                 = UwbApi_SetDeviceConfig(param_id, 1, &devConfig);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetDeviceConfig FAILED");
        goto exit;
    }
    param_id                                         = TX_PULSE_SHAPE_CONFIG;
    devConfig.txPulseShapeConfig.shape_id            = 0x2F;
    devConfig.txPulseShapeConfig.payload_tx_shape_id = 0x2F;
    devConfig.txPulseShapeConfig.sts_shape_id        = 0x2F;
    devConfig.txPulseShapeConfig.dac_stage_cofig     = 0x00;
    status                                           = UwbApi_SetDeviceConfig(param_id, 4, &devConfig);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetDeviceConfig FAILED");
        goto exit;
    }
#if !(UWBIOT_UWBD_SR040)
    /* Disable NXP Extneded NTF Config */
    param_id                       = NXP_EXTENDED_NTF_CONFIG;
    devConfig.nxpExtendedNtfConfig = 0x00;
    status                         = UwbApi_SetDeviceConfig(param_id, 1, &devConfig);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetDeviceConfig FAILED");
        goto exit;
    }
#endif // End of UWBIOT_UWBD_SR040
exit:
    return status;
}

void mctt_handler(uint8_t *valueBuffer, uint16_t length, uint16_t *pRespSize, uint8_t *pRespBuf)
{
    uint8_t mt  = (*(valueBuffer)&UCI_MT_MASK) >> UCI_MT_SHIFT;
    uint8_t gid = valueBuffer[0] & UCI_GID_MASK;
    uint8_t oid = valueBuffer[1] & UCI_OID_MASK;
    tUWBAPI_STATUS status;
    switch (mt) {
    case MT_UCI_DATA: {
        if (valueBuffer[0] == MCTT_DATA_SEND) {
            *pRespSize = HIF_RESP_BUFF_SIZE; // max expected rsp size
            /*
            * For Data transmit command no need to send the response to Host.
            * As Data transmit command doesent have any response but ntf is hadling in a seperate thread.
            */
            status = UwbApi_SendRawCommand(valueBuffer, length, &pRespBuf[0], pRespSize);
            status = UWBAPI_STATUS_OK;
            phOsalUwb_SetMemory(pRespBuf, 0x00, HIF_RESP_BUFF_SIZE);
            *pRespSize = 0;
        }
        else if (valueBuffer[0] == MCTT_DATA_RECV) {
            // assume as the Data recv ntf and just copy to resp buffer
            *pRespSize = length;
            phOsalUwb_MemCopy(&pRespBuf[0], &valueBuffer[0], length);
        }
        else {
            /*
            * Added dummy error for more debugging
            */
            pRespBuf[0] = 0xEE;
            pRespBuf[1] = 0xEE;
            pRespBuf[2] = 0xEE;
            pRespBuf[3] = 0x01;
            pRespBuf[4] = 0xEE;
            *pRespSize  = 5;
        }
    } break;
    case MT_UCI_CMD: {
        //device reset command
        if (gid == UCI_GID_CORE && oid == UCI_MSG_CORE_DEVICE_RESET) {
            //Send device reset response
            pRespBuf[0] = UCI_MT_RSP << UCI_MT_SHIFT | UCI_GID_CORE;
            pRespBuf[1] = UCI_MSG_CORE_DEVICE_RESET;
            pRespBuf[2] = 0x00;
            pRespBuf[3] = 0x01;
            pRespBuf[4] = UCI_STATUS_OK;
            *pRespSize  = 5;
            UWB_Hif_SendRsp(pRespBuf, *pRespSize);
            //shutdown
            mctt_pctt_common_deinit();
            /* Do the Initilization and Apply the calibration */
            status = mctt_pctt_common_init();
            if (status != UWBAPI_STATUS_OK) {
                NXPLOG_APP_E("mctt_pctt_common_init() Failed");
                break;
            }
            /*common setting for  Mctt and Pctt */
            if (mctt_pctt_common_settings() != UWBAPI_STATUS_OK) {
                NXPLOG_APP_E("mctt_pctt_common_settings() Failed");
                break;
            }

            //send device ready ntf
            if (status == UWBAPI_STATUS_OK) {
                pRespBuf[0]  = UCI_MT_NTF << UCI_MT_SHIFT | UCI_GID_CORE;
                pRespBuf[1]  = UCI_MSG_CORE_DEVICE_STATUS_NTF;
                pRespBuf[2]  = 0x00;
                pRespBuf[3]  = 0x01;
                pRespBuf[4]  = UWBD_STATUS_READY;
                *pRespSize   = 5;
                recoveryMode = FALSE;
            }
        }
        else if (recoveryMode == TRUE) {
            //Send command rejected response
            pRespBuf[0] = UCI_MT_RSP << UCI_MT_SHIFT | gid;
            pRespBuf[1] = valueBuffer[1];
            pRespBuf[2] = 0x00;
            pRespBuf[3] = 0x01;
            pRespBuf[4] = UWBAPI_STATUS_REJECTED;
            *pRespSize  = 5;
        }
        else if (gid == UCI_GID_RANGE_MANAGE && oid == UCI_MSG_RANGE_START) {
            uint32_t sessionHandle;
            uint8_t *session_ptr;
            session_ptr = &valueBuffer[4];
            UWB_STREAM_TO_UINT32(sessionHandle, session_ptr);

            status = demo_setVendorAppconfigs(sessionHandle);
            if (status != UWBAPI_STATUS_OK) {
                NXPLOG_APP_E("demo_setVendorAppconfigs() Failed");
                goto exit;
            }

#if SSS_HAVE_APPLET_SE051_UWB
            status = demo_set_wraped_rds(sessionHandle);
            //status = UWBAPI_STATUS_OK;//
            if (status != UWBAPI_STATUS_OK) {
                NXPLOG_APP_E("demo_set_wraped_rds() Failed");
            }
#endif //SSS_HAVE_APPLET_SE051_UWB
        exit:
            if (status != UWBAPI_STATUS_OK) {
                phOsalUwb_SetMemory(pRespBuf, 0x00, HIF_RESP_BUFF_SIZE);
                // send this dummy command to the mctttool or any Host to identify on failure
                pRespBuf[0] = 0xFF;
                pRespBuf[1] = 0xFF;
                pRespBuf[2] = 0xFF;
                pRespBuf[3] = 0x01;
                pRespBuf[4] = 0xFF;
                *pRespSize  = 5;
                UWB_Hif_SendRsp(pRespBuf, *pRespSize);
                break;
            }
            else {
                *pRespSize = HIF_RESP_BUFF_SIZE; // max expected rsp size
                status     = UwbApi_SendRawCommand(valueBuffer, length, &pRespBuf[0], pRespSize);
                if (status == UWBAPI_STATUS_TIMEOUT) { // Send device error state notification
                    pRespBuf[0]  = UCI_MT_NTF << UCI_MT_SHIFT | UCI_GID_CORE;
                    pRespBuf[1]  = UCI_MSG_CORE_DEVICE_STATUS_NTF;
                    pRespBuf[2]  = 0x00;
                    pRespBuf[3]  = 0x01;
                    pRespBuf[4]  = UWBD_STATUS_ERROR;
                    *pRespSize   = 5;
                    recoveryMode = TRUE;
                }
            }
        }
        else {
            //flag for session init
            if (gid == UCI_GID_SESSION_MANAGE && oid == UCI_MSG_SESSION_INIT) {
                /** extracting session type from session init cmd*/
                uint8_t sessionTypePos = length - 1;
                //depending upon session type applying Vendor app config flag for SESSION_INBAND_DATA_TX_BLOCKS/SESSION_INBAND_DATA_RX_BLOCKS
                gInBandDataParamFlag = ((valueBuffer[sessionTypePos] == UWBD_RANGING_WITH_INBAND_DATA_TRANSFER ||
                                            valueBuffer[sessionTypePos] == UWBD_DATA_TRANSFER ||
                                            valueBuffer[sessionTypePos] == UWBD_INBAND_DATA_PHASE ||
                                            valueBuffer[sessionTypePos] == UWBD_RANGING_WITH_DATA_PHASE) ?
                                            TRUE :
                                            FALSE);
            }
            // cmds send as raw
            *pRespSize = HIF_RESP_BUFF_SIZE; // max expected rsp size
            status     = UwbApi_SendRawCommand(valueBuffer, length, &pRespBuf[0], pRespSize);
            /**
             * MW will Trigger the command Retry , if no Response from UWBS
             * Still if no  Response , MW application will indicate to the MCTT/ITT tool to Recover or Reset using Device RESET command .
            */
            if (status == UWBAPI_STATUS_TIMEOUT) { // Send UWBD_STATUS_UNKNOWN Error Response .
                pRespBuf[0]  = UCI_MT_NTF << UCI_MT_SHIFT | UCI_GID_CORE;
                pRespBuf[1]  = UCI_MSG_CORE_DEVICE_STATUS_NTF;
                pRespBuf[2]  = 0x00;
                pRespBuf[3]  = 0x01;
                pRespBuf[4]  = UWBD_STATUS_UNKNOWN;
                *pRespSize   = 5;
                recoveryMode = TRUE;
            }
        }
    } break;
    case MT_UCI_RSP: {
        *pRespSize = length;
        phOsalUwb_MemCopy(&pRespBuf[0], &valueBuffer[0], length);
    } break;
    case MT_UCI_NTF: {
        if (gid == UCI_GID_CORE && oid == UCI_MSG_CORE_DEVICE_STATUS_NTF &&
            valueBuffer[UCI_RESPONSE_STATUS_OFFSET] == UWBD_STATUS_ERROR) {
            recoveryMode = TRUE;
        }
        // ntf just copy to resp buffer
        *pRespSize = length;
        phOsalUwb_MemCopy(&pRespBuf[0], &valueBuffer[0], length);
    } break;
    case MT_ESE_CTRL_CMD: {
#if SSS_HAVE_APPLET_SE051_UWB
        /** WA
         * @brief Se_API_SendReceive excludes 2 byte status feild in the out param length
         * For mctt dynamic case we dont need that exclusion of 2 bytes hence adding + 2
         * response size
         */

        uint8_t cmdStatus;
        uint8_t dispatchEventId;
        uint8_t pDispatchEventDataBuffer_adf[272] = {0};
        size_t dispatchEventDataLen_adf           = sizeof(pDispatchEventDataBuffer_adf);

        uint8_t pDispatchEventDataBuffer_put[272] = {0};
        size_t dispatchEventDataLen_put           = sizeof(pDispatchEventDataBuffer_put);

        if (valueBuffer[4] == SELECT_ADF_CLA && valueBuffer[5] == SELECT_ADF_INS) {
            phOsalUwb_SetMemory(pRespBuf, 0x00, HIF_RESP_BUFF_SIZE);
            Se_API_Dispatch(&valueBuffer[4],
                length - 4,
                &cmdStatus,
                &pRespBuf[4],
                (size_t *)pRespSize,
                &dispatchEventId,
                pDispatchEventDataBuffer_adf,
                &dispatchEventDataLen_adf);

            pRespBuf[0] = MT_ESE_CTRL_RSP << UCI_MT_SHIFT | 0;
            pRespBuf[1] = 0x00;
            pRespBuf[2] = *pRespSize;
            pRespBuf[3] = 0x00;
            *pRespSize  = *pRespSize + HEADER_SIZE_MCTT; // response size + header + 2;
            break;
        }
        else if (valueBuffer[4] == PUT_DATA_CLA && valueBuffer[5] == PUT_DATA_INS) {
            phOsalUwb_SetMemory(pRespBuf, 0x00, HIF_RESP_BUFF_SIZE);
            Se_API_Dispatch(&valueBuffer[4],
                length - 4,
                &cmdStatus,
                &pRespBuf[4],
                (size_t *)pRespSize,
                &dispatchEventId,
                pDispatchEventDataBuffer_put,
                &dispatchEventDataLen_put);
            if (dispatchEventDataLen_put <= FIRALITE_WRAPPED_RDS_LEN) {
                gWrappedRDSLen = dispatchEventDataLen_put;
                memcpy(&gWrappedRDS[0], pDispatchEventDataBuffer_put, gWrappedRDSLen);
            }

            pRespBuf[0] = MT_ESE_CTRL_RSP << UCI_MT_SHIFT | 0;
            pRespBuf[1] = 0x00;
            pRespBuf[2] = *pRespSize;
            pRespBuf[3] = 0x00;
            *pRespSize  = *pRespSize + HEADER_SIZE_MCTT; // response size + header + 2;
            break;
        }
        else {
            Se_API_SendReceive(&valueBuffer[4], length - 4, &pRespBuf[4], (size_t *)pRespSize);
            pRespBuf[0] = MT_ESE_CTRL_RSP << UCI_MT_SHIFT | 0;
            pRespBuf[1] = 0x00;
            pRespBuf[2] = *pRespSize + 2;
            /** WA
         * @brief Se_API_SendReceive excludes 2 byte status feild in the out param length
         * For mctt dynamic case we dont need that exclusion of 2 bytes hence adding + 2
         * response size
         */
            pRespBuf[3] = 0x00;
            /**
         * @brief while transmitting to MCTT/HOST append the overall size
         */
            *pRespSize = *pRespSize + HEADER_SIZE_MCTT + 2; // response size + header + 2;
        }
#elif UWBFTR_SE_SN110
        Enable_GPIO0_IRQ();
        SeApi_WiredTransceive(&valueBuffer[4], length - 4, &pRespBuf[4], 255, pRespSize, 10000);
        phTmlUwb_io_enable_uwb_irq();
        pRespBuf[0] = MT_ESE_CTRL_RSP << UCI_MT_SHIFT | 0;
        pRespBuf[1] = 0x00;
        pRespBuf[2] = *pRespSize;
        pRespBuf[3] = 0x00;
        *pRespSize  = *pRespSize + HEADER_SIZE_MCTT; // response size + header;
#endif //UWBFTR_SE_SN110
    } break;
    default:
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
}

#endif //UWBIOT_APP_BUILD__DEMO_MCTT_PCTT
