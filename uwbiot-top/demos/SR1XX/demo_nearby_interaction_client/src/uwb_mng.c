/* Copyright 2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

/* TLV */
#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION_CLIENT
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION_CLIENT

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Driver includes */
#include "phTmlUwb_transport.h"
#include "AppInternal.h"
#include "UWBT_PowerMode.h"
#include "UwbApi_Utility.h"
#include "demo_device_config.h"

bool gDeviceInitialized = FALSE;
bool gSessionStarted    = FALSE;
#define DEMO_BLE_CLIENT_NO_OF_ANCHORS_P2P 1
uint8_t gDstMacAddr[2];

void InitUwb(void)
{
    tUWBAPI_STATUS uwb_status = UWBAPI_STATUS_OK;
    if (!gDeviceInitialized) {
        LOG_W("device init");
        uwb_status = UwbApi_Init(AppCallback);
        if (uwb_status != UWBAPI_STATUS_OK) {
            LOG_E("UwbApi_Init failed");
        }
        else {
            gDeviceInitialized = TRUE;
        }
    }
}
void DeInitUwb(void)
{
    tUWBAPI_STATUS uwb_status = UWBAPI_STATUS_OK;
    if (gDeviceInitialized) {
        uwb_status = UwbApi_ShutDown();
        if (uwb_status == UWBAPI_STATUS_OK) {
            LOG_W("device deinit");
            gDeviceInitialized = FALSE;
        }
        else {
            LOG_E("Error shutting down: %02X", uwb_status);
        }
    }
}

/**
 * \brief Wrapper function to initialize, configure and start a session.
 *
 * \param[inout]    session_id          // To input the sessionId and take out the SessionHandle.
 * \param[in]       acc_role            // To specify the role of the device.
 * \param[in]       dev_mac_add         // To pass the Device MAC Address.
 *
 */
bool StartUwbSession(uint32_t *session_id, uint8_t acc_role, uint16_t dev_mac_add)
{
    tUWBAPI_STATUS status;
    phRangingParams_t inRangingParams = {0};
    uint32_t sessionHandle            = 0;

    LOG_I("mac_addr :          :%01X", dev_mac_add);

    if (gDeviceInitialized) {
        // Initialize the session with the session_id.
        status = UwbApi_SessionInit(*session_id, UWBD_RANGING_SESSION, &sessionHandle);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_SessionInit() Failed");
            return FALSE;
        }
        // update the session_id field with the received sessionHandle.
        *session_id = sessionHandle;

        switch (acc_role) {
        case UWB_DEVICE_CONTROLEE:
            inRangingParams.deviceRole = kUWB_DeviceRole_Initiator;
            inRangingParams.deviceType = kUWB_DeviceType_Controller;
            break;
#if !(UWBIOT_UWBD_SR040)
        case UWB_DEVICE_CONTROLLER:
            inRangingParams.deviceRole = kUWB_DeviceRole_Responder;
            inRangingParams.deviceType = kUWB_DeviceType_Controlee;
            break;
#endif
        default:
            NXPLOG_APP_E("Unsupported accessory role");
        }

#if ((DEMO_SET_CONFIG_ID == UWB_CONFIG_ID_1) || (DEMO_SET_CONFIG_ID == UWB_CONFIG_ID_3))
        inRangingParams.multiNodeMode = kUWB_MultiNodeMode_UniCast;
#elif (DEMO_SET_CONFIG_ID == UWB_CONFIG_ID_2)
        inRangingParams.multiNodeMode = kUWB_MultiNodeMode_OnetoMany;
#endif // CONFIG_ID
        inRangingParams.macAddrMode       = 0;
        inRangingParams.deviceMacAddr[0]  = dev_mac_add;
        inRangingParams.deviceMacAddr[1]  = dev_mac_add >> 8;
        inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
        inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

        status = UwbApi_SetRangingParams(sessionHandle, &inRangingParams);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
            return FALSE;
        }

        const uint8_t stsStatic[]                     = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        const uint8_t vendorId[]                      = {0x08, 0x07};
        const UWB_AppParams_List_t SetAppParamsList[] = {

#if ((DEMO_SET_CONFIG_ID == UWB_CONFIG_ID_1) || (DEMO_SET_CONFIG_ID == UWB_CONFIG_ID_3))
#if UWBIOT_UWBD_SR040
            UWB_SET_APP_PARAM_VALUE(RANGING_INTERVAL, 240),
#else
            UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 240),
#endif // UWBIOT_UWBD_SR040
            UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10),
#elif (DEMO_SET_CONFIG_ID == UWB_CONFIG_ID_2)
            UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 200),
            UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10),
#endif // CONFIG_ID
            UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 2400),
            UWB_SET_APP_PARAM_ARRAY(VENDOR_ID, &vendorId[0], sizeof(vendorId)),
            UWB_SET_APP_PARAM_ARRAY(STATIC_STS_IV, &stsStatic[0], sizeof(stsStatic)),
            UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, 0x0A),
            UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 0x09),
            UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_BLE_CLIENT_NO_OF_ANCHORS_P2P),
            UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gDstMacAddr, MAC_SHORT_ADD_LEN),
        };

        status = UwbApi_SetAppConfigMultipleParams(
            sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
        if (status != UWBAPI_STATUS_OK) {
            LOG_E("UwbApi_SetAppConfigMultipleParams() Failed");
            return FALSE;
        }

        status = UwbApi_StartRangingSession(sessionHandle);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_StartRangingSession() Failed");
            return FALSE;
        }
        gSessionStarted = TRUE;
    }
    return TRUE;
}

void StopUwbSession(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status;

    if (gDeviceInitialized && gSessionStarted) {
        status = UwbApi_StopRangingSession(sessionHandle);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_StopRangingSession() Failed");
        }
    }
}
#endif //UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION_CLIENT
