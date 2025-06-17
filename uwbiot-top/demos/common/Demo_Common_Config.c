/* Copyright 2022,2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

/**
 * @file Demo_Common_Config.c
 * @author nxf18919
 * @brief This file is use for common application configs setting and common api calls in multiple demos
 *
 */
#include "Demo_Common_Config.h"
#include "phNxpLogApis_App.h"
#include "uwb_board.h"

#if (UWBFTR_SE_SN110)
#include "SeApi.h"
#include "wearable_platform_int.h"
#include "phTmlUwb_transport.h"

#define SUS_TEST_AID                                                                                   \
    {                                                                                                  \
        0xA0, 0x00, 0x00, 0x03, 0x96, 0x54, 0x53, 0x00, 0x00, 0x00, 0x01, 0x04, 0xF2, 0x00, 0x00, 0x00 \
    }
#define SUS_AID                                                                                        \
    {                                                                                                  \
        0xA0, 0x00, 0x00, 0x03, 0x96, 0x54, 0x53, 0x00, 0x00, 0x00, 0x01, 0x04, 0x02, 0x00, 0x00, 0x00 \
    }
#define ROOT_SESSION_KEY                                                                               \
    {                                                                                                  \
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 \
    }

#define SELECT_APPLET_CMD            \
    {                                \
        0x00, 0xA4, 0x04, 0x00, 0x00 \
    }

#define SET_SESSION_ID_CMD           \
    {                                \
        0x80, 0xA0, 0x00, 0x00, 0x00 \
    }

#define GET_SESSION_ID_CMD     \
    {                          \
        0x80, 0xCA, 0x00, 0x47 \
    }

extern void Enable_GPIO0_IRQ();

#endif //UWBFTR_SE_SN110

/* Currently not used for SR040 */
#if (UWBIOT_UWBD_SR1XXT_SR2XXT)

/* Helper macro for ToA Mode */
#define TOA_ID_X(H, V) kUWBAntCfgRxMode_AoA_Mode, 0x02, (H), (V)
#define TOA_ID_Y(Y)    kUWBAntCfgRxMode_AoA_Mode, 0x01, (Y)
/* Helper macro for ToA Mode */
#define TOF_ID_X(X) kUWBAntCfgRxMode_ToA_Mode, 0x01, (X)

/* Helper macro to select 2D AoA Azimuth Antenna */
#define ANT_ID_AOA_2D_AZIMUTH TOA_ID_Y(1)

/* Helper macro to select 3D AoA Azimuth and Elevation
 * configuration of the antenna.
 *
 * When using this, ensure
 * - ``USE_NAKED_BOARD`` is set to ``0`` in ``boards/Host/Rhodes4/UWB_DeviceConfig.h``
 */
#define ANT_ID_AOA_3D_AoA TOA_ID_X(1, 2)

#if (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_2DAOA)
#define SET_ANTENNA_CONFIG (1)
#elif (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_3DAOA)
#define SET_ANTENNA_CONFIG (1)
#elif (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_TOF)
#define SET_ANTENNA_CONFIG (0)
#else
#error "Select TOF, 2D AOA or 3D AoA"
#endif

tUWBAPI_STATUS demo_set_common_app_config(uint32_t sessionHandle, UWB_StsConfig_t sts_config)
{
    tUWBAPI_STATUS status;

#if (UWBIOT_UWBD_SR1XXT)

#if (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_2DAOA)
    uint8_t antennaeConfigurationRx[] = {ANT_ID_AOA_2D_AZIMUTH};
#elif (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_3DAOA)
    uint8_t antennaeConfigurationRx[] = {ANT_ID_AOA_3D_AoA};
#elif (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_TOF)
    /* No Need to define antennaeConfigurationsRX */
#else
#error "Select TOF, 2D AOA or 3D AoA"
#endif

    const UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(SFD_ID, 2),
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 25),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 25 * 8),
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, sts_config),
        UWB_SET_APP_PARAM_VALUE(NUMBER_OF_STS_SEGMENTS, 1),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, 3),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
    };

#if SET_ANTENNA_CONFIG
    const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_ARRAY(
            ANTENNAE_CONFIGURATION_RX, &antennaeConfigurationRx[0], sizeof(antennaeConfigurationRx)),
    };
#endif // SET_ANTENNA_CONFIG
#endif // UWBIOT_UWBD_SR1XXT

#if (UWBIOT_UWBD_SR2XXT)
    const UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(RANGING_ROUND_USAGE, kUWB_RangingRoundUsage_DS_TWR),
        UWB_SET_APP_PARAM_VALUE(SFD_ID, 2),
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 25),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 25 * 8),
        /*static STS*/
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, 0),
        UWB_SET_APP_PARAM_VALUE(NUMBER_OF_STS_SEGMENTS, 1),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, kUWB_RfFrameConfig_SP3),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
        UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, 0),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, 10),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_DURATION, 1),
        UWB_SET_APP_PARAM_VALUE(PSDU_DATA_RATE, 0),
        UWB_SET_APP_PARAM_VALUE(PRF_MODE, 0),
        UWB_SET_APP_PARAM_VALUE(BPRF_PHR_DATA_RATE, 0),
        UWB_SET_APP_PARAM_VALUE(STS_LENGTH, 1),
    };
#endif

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

#if (UWBIOT_UWBD_SR1XXT)
#if SET_ANTENNA_CONFIG
    status = UwbApi_SetVendorAppConfigs(
        sessionHandle, sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]), &SetVendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetVendorAppConfigs() Failed");
        goto exit;
    }
#endif //SET_ANTENNA_CONFIG
#endif //UWBIOT_UWBD_SR1XXT
exit:
    return status;
}

#if (UWBFTR_SE_SN110)

tUWBAPI_STATUS UwbApi_setFixedSessionKey(uint32_t sessionHandle)
{
    tSEAPI_STATUS status       = 0;
    uint8_t txBuffer[255]      = {0};
    uint8_t rxBuffer[255]      = {0};
    uint16_t rxRecvBufferLen   = 0;
    uint8_t SUSTestApplet[]    = SUS_TEST_AID;
    uint8_t SUSApplet[]        = SUS_AID;
    uint8_t rootSessionKey[]   = ROOT_SESSION_KEY;
    uint8_t selectAppletCmd[]  = SELECT_APPLET_CMD;
    uint8_t sessionInjectCmd[] = SET_SESSION_ID_CMD;
    uint8_t getSessionId[]     = GET_SESSION_ID_CMD;
    uint32_t recvSessionId;

    /* Enable SN110 Irq */
    //TODO: This is the temparary fix for SR1XX and SN110 IRQ enablament
    Enable_GPIO0_IRQ();

    SeApi_WiredEnable(TRUE);

    /* Select SUS Test Applet */
    phOsalUwb_MemCopy(&txBuffer[0], &selectAppletCmd[0], sizeof(selectAppletCmd));
    txBuffer[4] = sizeof(SUSTestApplet);
    phOsalUwb_MemCopy(&txBuffer[5], &SUSTestApplet[0], sizeof(SUSTestApplet));

    LOG_MAU8_I("SE Tx >", txBuffer, sizeof(selectAppletCmd) + sizeof(SUSApplet));
    status = SeApi_WiredTransceive(
        txBuffer, sizeof(selectAppletCmd) + sizeof(SUSApplet), rxBuffer, 255, &rxRecvBufferLen, 10000);
    if (status != SEAPI_STATUS_OK) {
        LOG_E("Failed to select applet");
        return UWBAPI_STATUS_FAILED;
    }

    LOG_MAU8_I("SE Rx <", rxBuffer, rxRecvBufferLen);

    /* Set Fixed Session Key  */
    phOsalUwb_MemCopy(&txBuffer[0], &sessionInjectCmd[0], sizeof(sessionInjectCmd));
    txBuffer[4]  = sizeof(rootSessionKey) + 4 + 4;
    txBuffer[5]  = 0xC0;
    txBuffer[6]  = sizeof(rootSessionKey);
    txBuffer[23] = 0xCF;
    txBuffer[24] = 0x04;
    phOsalUwb_MemCopy(&txBuffer[7], &rootSessionKey[0], sizeof(rootSessionKey));
    UWB_UINT32_TO_BE_FIELD(&txBuffer[25], sessionHandle);

    LOG_MAU8_I("SE Tx >", txBuffer, txBuffer[4] + 5);

    status = SeApi_WiredTransceive(txBuffer, txBuffer[4] + 5, rxBuffer, 255, &rxRecvBufferLen, 10000);
    if (status != SEAPI_STATUS_OK) {
        LOG_E("Failed to select applet");
        return UWBAPI_STATUS_FAILED;
    }
    LOG_MAU8_I("SE Rx <", rxBuffer, rxRecvBufferLen);

    /* select SUS Applet */
    phOsalUwb_MemCopy(&txBuffer[0], &selectAppletCmd[0], sizeof(selectAppletCmd));
    txBuffer[4] = sizeof(SUSApplet);
    phOsalUwb_MemCopy(&txBuffer[5], &SUSApplet[0], sizeof(SUSApplet));

    LOG_MAU8_I("SE Tx >", txBuffer, sizeof(selectAppletCmd) + sizeof(SUSApplet));
    status = SeApi_WiredTransceive(
        txBuffer, sizeof(selectAppletCmd) + sizeof(SUSApplet), rxBuffer, 255, &rxRecvBufferLen, 10000);
    if (status != SEAPI_STATUS_OK) {
        LOG_E("Failed to select applet");
        return UWBAPI_STATUS_FAILED;
    }

    LOG_MAU8_I("SE Rx <", rxBuffer, rxRecvBufferLen);

    /* Verify Session ID */
    phOsalUwb_MemCopy(&txBuffer[0], &getSessionId[0], sizeof(getSessionId));

    LOG_MAU8_I("SE Tx >", txBuffer, txBuffer[4] + 5);

    status = SeApi_WiredTransceive(txBuffer, 4, rxBuffer, 255, &rxRecvBufferLen, 10000);
    if (status != SEAPI_STATUS_OK) {
        LOG_E("Failed to select applet");
        return UWBAPI_STATUS_FAILED;
    }
    LOG_MAU8_I("SE Rx <", rxBuffer, rxRecvBufferLen);

    recvSessionId = (((uint32_t)(rxBuffer[5])) + (((uint32_t)(rxBuffer[4])) << 8) + (((uint32_t)(rxBuffer[3])) << 16) +
                     (((uint32_t)(rxBuffer[2])) << 24));

    if (recvSessionId != sessionHandle) {
        LOG_E("Session Key mismatch");
        return UWBAPI_STATUS_FAILED;
    }

    /* Enable SR1XX Irq */
    //TODO: This is the temparary fix for SR1XX and SN110 IRQ enablament
    phTmlUwb_io_enable_uwb_irq();

    return UWBAPI_STATUS_OK;
}
#endif //UWBFTR_SE_SN110

#endif // (UWBIOT_UWBD_SR1XXT_SR2XXT)
