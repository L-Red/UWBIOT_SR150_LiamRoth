/* Copyright 2021-2023 NXP
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
#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION

#include "TLV_Types_i.h"
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Driver includes */

#include "RNG_Interface.h"
#include "phTmlUwb_transport.h"
#include "phOsalUwb.h"
#include "AppInternal.h"
#include "UWBT_PowerMode.h"
#include "private_profile_interface.h"
#include "UwbApi_Utility.h"

#define TLV_MNG_STACK_SIZE 400
#define TLV_MNG_PRIO       (tskIDLE_PRIORITY + 4)

intptr_t tlvMngQueue;
void *mTlvMutex = NULL;

static UWBOSAL_TASK_HANDLE mTlvMngHnd;
static void tlvMngTask(void *args);
static void handleTLV(uint8_t deviceId, uint8_t *data);
UwbHandlerState mSessionState[gAppMaxConnections_c] = {notCreated};
uint32_t mSessionHandle[gAppMaxConnections_c];
uint8_t mDevice[gAppMaxConnections_c];
uint16_t mMacAddr[gAppMaxConnections_c];
bool gDeviceInitialized = FALSE;

bool tlvMngInit(void)
{
    phOsalUwb_ThreadCreationParams_t threadParams;
    tlvMngQueue = phOsalUwb_msgget(1);

    if (!tlvMngQueue) {
        LOG_E("Could not create queue tlvMngQueue");
        return FALSE;
    }

    if (phOsalUwb_CreateMutex(&mTlvMutex) != UWBSTATUS_SUCCESS) {
        LOG_E("Could not create TLV mutex");
        return FALSE;
    }

    threadParams.stackdepth = TLV_MNG_STACK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadParams, "TlvMng");
    threadParams.pContext = NULL;
    threadParams.priority = TLV_MNG_PRIO;
    phOsalUwb_Thread_Create((void **)&mTlvMngHnd, &tlvMngTask, &threadParams);
    if (!mTlvMngHnd) {
        LOG_E("Could not create tlvMng task");
        return FALSE;
    }
    return TRUE;
}

void tlvMngTask(void *args)
{
    while (1) {
        uint8_t data[50]       = {0};
        phLibUwb_Message_t evt = {0};
        if (phOsalUwb_msgrcv(tlvMngQueue, &evt, MAX_DELAY) == UWBSTATUS_FAILED) {
            continue;
        }
        phOsalUwb_MemCopy(data, (uint8_t *)evt.pMsgData, evt.Size);
        LOG_AU8_D(data, evt.Size);
        handleTLV((uint8_t)evt.eMsgType, data);
    }
}

/*
 * Here is the entry point for the application
 * handleTLV() is managing the state machine to handle
 * the data received/sent by the phone application
 */
void handleTLV(uint8_t deviceId, uint8_t *data)
{
    tUWBAPI_STATUS uwb_status = UWBAPI_STATUS_FAILED;
    (void)phOsalUwb_LockMutex(mTlvMutex);
    /* Fill-in input structure with device role/type and device mac address*/
    phUwbProfileInfo_t profileInfo;
    static UserAccessoryConfigData_iOS_t UserConfigData_iOS = {0};
    uint8_t BLEmessage_iOS[1 + sizeof(UserAccessoryConfigData_iOS_t)];
    /* Keep the config data stored in the variable for multiple devices (sessions) to use
       As get device info can not be done if any session is active */
    static UwbDeviceConfigData_t UwbDeviceConfigData = {0};
    uint8_t response;

    if (data == NULL) {
        LOG_W("handleTLV data is NULL");
    }

    switch (*data) {
    case kMsg_ConfigureAndStart: {
        mSessionState[deviceId] = notStarted;

        /* Received configure command with Shareable data
         * Apply the Shareable data and start UWB ranging
         *
         * Host can also use this API to set DebugParams as Well as AppParams
         * the following parameters are suported:
         * CIR_CAPTURE_WINDOW
         * ANT_CONFIG_RX
         * TX_ADAPTIVE_PAYLOAD_POWER
         *
         * To easily set the AppParams list, following macros have been defined.
         *   UWB_SET_APP_PARAM_VALUE(Parameter, Value) AND UWB_SET_APP_PARAM_ARRAY(Parameter, ArrayValue, Length)
         *
         * The session specific antenna configuration for Rx.
         *
         * To configure ToA mode
         *  Octet [0] : Mode of RX operation
         *  Octet [1] : Number of Antennas or Antenna pairs to follow
         *  Octet [2] : Rx Antenna ID
         *
         * Helper macro for ToA Mode
         *  #define TOA_ID_X(X) (0x00, 0x01, (X))
         *
         * uint8_t antennaeConfigurationRx[] = {TOA_ID_X(1)};
         *
         * To configure AoA mode
         *  Octet [0] : Mode of RX operation
         *  Octet [1] : Number of Antennas or Antenna pairs to follow
         *  Octet [2] : Rx Antenna ID of Horizontal Pair
         *  Octet [3] : Rx Antenna ID of Vertical Pair
         *
         * Helper macro for AoA Mode
         *  #define AOA_ID_X(H, V) (0x01, 0x02, (H), (V))
         *
         * uint8_t antennaeConfigurationRx[] = {AOA_ID_X(1, 2)};
         *
         * const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
         *     UWB_SET_APP_PARAM_ARRAY(
         *         ANTENNAE_CONFIGURATION_RX, &antennaeConfigurationRx[0], sizeof(antennaeConfigurationRx)),
         * };
         *
         *
         * UwbApi_ConfigureData_iOS(data + 1,
         *         *(data + SHAREABLE_DATA_LENGTH_OFFSET) + SHAREABLE_DATA_HEADER_LENGTH,
         *         &profileInfo,
         *         sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]),
         *         &SetVendorAppParamsList[0],
         *         sizeof(SetDebugParamsList) / sizeof(SetDebugParamsList[0]),
         *         &SetDebugParamsList[0]);
         *
         *                      OR
         *
         * UwbApi_ConfigureData_Android(
         *         data + 1, SHAREABLE_DATA_HEADER_LENGTH_ANDROID,
         *         &profileInfo,
         *         sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]),
         *         &SetVendorAppParamsList[0],
         *         sizeof(SetDebugParamsList) / sizeof(SetDebugParamsList[0]),
         *         &SetDebugParamsList[0]);
         *
         */

         uint8_t antennaeConfigurationRx[] = {
                 0x01, // Mode of Operation
                 0x02, // size of configuration data
                 0x01, // RX Pair H
                 0x02, // RX Pair V
         };

         const UWB_VendorAppParams_List_t   SetAppParamsList[] = {
                 UWB_SET_APP_PARAM_VALUE(TX_ADAPTIVE_PAYLOAD_POWER, 1),
                 UWB_SET_APP_PARAM_ARRAY(
                    ANTENNAE_CONFIGURATION_RX, &antennaeConfigurationRx[0], sizeof(antennaeConfigurationRx)),
         };

        if (mDevice[deviceId] == kDev_android_c) {
            profileInfo.mac_addr[0] = mMacAddr[deviceId] >> 8 & 0xFF;
            profileInfo.mac_addr[1] = mMacAddr[deviceId] >> 0 & 0xFF;

            LOG_MAU8_I("mac addr :", profileInfo.mac_addr, 2);

            uwb_status = UwbApi_ConfigureData_Android(
                data + 1, SHAREABLE_DATA_HEADER_LENGTH_ANDROID,
                &profileInfo,
                sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]),
                &SetAppParamsList[0],
                0,
                NULL);
            if (uwb_status != UWBAPI_STATUS_OK) {
                LOG_E("Shareable data not configured");
            }
            else {
                LOG_D("Shareable data configured");
                mSessionHandle[deviceId] = profileInfo.sessionHandle;
                mSessionState[deviceId]  = Started;
                response                 = kRsp_AccessoryUwbDidStart;
                if (!tlvSendRaw(deviceId, &response, sizeof(response))) {
                    LOG_E("Failed to send UWB start response");
                }
            }
        }
        else if (mDevice[deviceId] == kDev_iPhone_c) {
            /* Fill-in input structure with device role/type and device mac address*/
            profileInfo.deviceRole  = DEMO_DEVICE_ROLE;
            profileInfo.deviceType  = DEMO_DEVICE_TYPE;
            profileInfo.mac_addr[0] = mMacAddr[deviceId] >> 8 & 0xFF;
            profileInfo.mac_addr[1] = mMacAddr[deviceId] >> 0 & 0xFF;

            LOG_MAU8_I("mac addr :", profileInfo.mac_addr, 2);

            uwb_status = UwbApi_ConfigureData_iOS(data + 1,
                *(data + SHAREABLE_DATA_LENGTH_OFFSET) + SHAREABLE_DATA_HEADER_LENGTH,
                &profileInfo,
                sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]),
                &SetAppParamsList[0],
                0,
                NULL);
            if (uwb_status != UWBAPI_STATUS_OK) {
                LOG_E("Shareable data not configured");
            }
            else {
                LOG_D("Shareable data configured");
                mSessionHandle[deviceId] = profileInfo.sessionHandle;
                mSessionState[deviceId]  = Started;
                response                 = kRsp_AccessoryUwbDidStart;
                if (!tlvSendRaw(deviceId, &response, sizeof(response))) {
                    LOG_E("Failed to send start accessory");
                }
                // API is called to erase Service characteristic in GATT as ranging is already started
                if ((UserConfigData_iOS.UwbConfigData.uwb_spec_ver_minor[0] == 0x01) &&
                    (UserConfigData_iOS.UwbConfigData.uwb_spec_ver_minor[1] == 0x00)) {
                    if (Erase_Nearby(deviceId) != 0) {
                        LOG_E("Erase GATT failure");
                    }
                }
            }
        }
        else {
            uwb_status = UWBAPI_STATUS_FAILED; // Unknown platform detected
            LOG_E("Unknown platform detected");
        }
    } break;
    case kMsg_Initialize_iOS: {
        /* Start command received
         * Fill the ConfigData and send it over BLE to the phone application
         */

        /* Application related definitions */
        uint8_t SpecMajorVersion[] = SPEC_VERSION_MAJOR;
        uint8_t SpecMinorVersion[] = SPEC_VERSION_MINOR;

        phOsalUwb_MemCopy(UserConfigData_iOS.customerSpecMajorVer, SpecMajorVersion, sizeof(SpecMajorVersion));
        phOsalUwb_MemCopy(UserConfigData_iOS.customerSpecMinorVer, SpecMinorVersion, sizeof(SpecMinorVersion));
        UserConfigData_iOS.preferedUpdateRate = kUpdateRate_UserInteractive;

        mDevice[deviceId] = kDev_iPhone_c;

        /* UWB related definitions */
        uwb_status = UwbApi_GetUwbConfigData_iOS(DEMO_DEVICE_ROLE, &(UserConfigData_iOS.UwbConfigData));
        if (uwb_status != UWBAPI_STATUS_OK) {
            LOG_E("UwbApi_GetUwbConfigData_iOS configuration failed");
        }
        /* Build BLE Message, to be build depending on the iOS application message stream
         * In example application, it contains Message ID + Accessory configuration data */
        BLEmessage_iOS[0] = kRsp_AccessoryConfigurationData; /* Response ID */

        /* Warning: this copy works correctly because the UserConfigData_iOS does not need
         * padding, if more fields were to be added and don't fit the memory alignment then extra
         * bytes could be added and more data than required would be sent. To fix that a serialize
         * utils function would be required or use the packed attribute
         */
        phOsalUwb_MemCopy(&BLEmessage_iOS[1], (uint8_t *)&UserConfigData_iOS, sizeof(UserConfigData_iOS));

        mMacAddr[deviceId] = ((UserConfigData_iOS.UwbConfigData.device_mac_addr[0] << 8) & 0xFF00) |
                             ((UserConfigData_iOS.UwbConfigData.device_mac_addr[1] << 0) & 0xFF);

        if ((UserConfigData_iOS.UwbConfigData.uwb_spec_ver_minor[0] == 0x01) &&
            (UserConfigData_iOS.UwbConfigData.uwb_spec_ver_minor[1] == 0x00)) {
            LOG_I(" Following spec 1.1");
            /* Spec 1.1 required to update GATT server
        	 Update the GATT server with the same BLEmessage (only removing Response ID that is not part of the original definition) */
            if (Update_Nearby(deviceId, sizeof(BLEmessage_iOS) - 1, BLEmessage_iOS + 1) != 0) {
                LOG_E("%s failed to update the GATT server with the BLE msg", __FUNCTION__);
            }

            /* Need to send the exact data from UserConfigData_iOS  over ble */
            LOG_MAU8_I("Sent over BLE: ", BLEmessage_iOS, sizeof(BLEmessage_iOS));
            if (!tlvSendRaw(deviceId, BLEmessage_iOS, sizeof(BLEmessage_iOS))) {
                LOG_E("Failed to send accessory config");
            }
        }
        else if ((UserConfigData_iOS.UwbConfigData.uwb_spec_ver_minor[0] == 0x00) &&
                 (UserConfigData_iOS.UwbConfigData.uwb_spec_ver_minor[1] == 0x00)) {
            LOG_I(" Following spec 1.0");
            /* Spec 1.0 support, clock drift not sent over BLE. BLE message size must  */
            LOG_MAU8_I(
                "Sent over BLE: ", BLEmessage_iOS, sizeof(BLEmessage_iOS) - 2 /* clock drift not sent in spec 1.0 */);
            if (!tlvSendRaw(deviceId, BLEmessage_iOS, sizeof(BLEmessage_iOS) - 2)) {
                LOG_E("Failed to send accessory config");
            }
        }
        else {
            LOG_I(" Unknown spec");
        }
    } break;

    case kMsg_Initialize_Android: {
        /* Start command received
         * Fill the ConfigData and send it over BLE to the phone application
         */
        mDevice[deviceId] = kDev_android_c;

        /* UWB related definitions */
        uwb_status = UwbApi_GetUwbConfigData_Android(&UwbDeviceConfigData);
        if (uwb_status != UWBAPI_STATUS_OK) {
            LOG_E("UwbApi_GetUwbConfigData_iOS configuration failed");
        }

        /* Store generated own device UWB MAC address */
        mMacAddr[deviceId] = ((UwbDeviceConfigData.device_mac_addr[0] << 8) & 0xFF00) |
                             ((UwbDeviceConfigData.device_mac_addr[1] << 0) & 0xFF);

        /* Build BLE Message, to be build depending on the Android application message stream
         * In example application, it contains Message ID + Accessory configuration data */

        uint8_t BLEmessage_Android[1 + sizeof(UwbDeviceConfigData_t)];
        uint16_t dataLen      = serializeUwbDeviceConfigData(&UwbDeviceConfigData, &BLEmessage_Android[1]);
        BLEmessage_Android[0] = kRsp_AccessoryConfigurationData; /* Response ID */
        dataLen               = (uint16_t)(dataLen + 1);

        /* Need to send the exact data from ConfigData  over ble */
        if (!tlvSendRaw(deviceId, BLEmessage_Android, dataLen)) {
            LOG_E("Failed to send accessory config");
        }
    } break;
    case kMsg_Stop: {
        /* Stop command received
         * Stop UWB and send back the response to the phone
         */
        LOG_I("Received stop message");
        if (!handleStopSession(deviceId)) {
            LOG_E("Stop session failed");
        }
        else {
            uwb_status = UWBAPI_STATUS_OK;
        }
        response = kRsp_AccessoryUwbDidStop;
        if (!tlvSendRaw(deviceId, &response, sizeof(response))) {
            LOG_E("Failed to send start accessory");
        }
    } break;
    default:
        LOG_W("Unknown command, skipping");
        break;
    }
    (void)phOsalUwb_UnlockMutex(mTlvMutex);
}

//FIXME: // Required?
void handleDisconnection(uint8_t deviceId)
{
    mSessionState[deviceId]  = notCreated;
    mSessionHandle[deviceId] = (uint32_t)0xA5A5A5A5;
    mDevice[deviceId]        = kDev_unknown_c;
    mMacAddr[deviceId]       = (uint16_t)0xA5A5;
}

bool handleStopSession(uint8_t deviceId)
{
    bool status              = TRUE;
    tUWBAPI_STATUS operation = UWBAPI_STATUS_OK;

    while (mSessionState[deviceId] != notCreated) {
        if (uwbContext.sessionInfo.reason_code == UWB_SESSION_STOPPED_DUE_TO_INBAND_SIGNAL) {
            /* On Inband Termination there is no need to stop the session again
            * as the session is already stopped, so skipping started case*/
#if !(UWBIOT_LOG_SILENT) && DEBUG
            uint32_t devSessionId = 0x00;
            uint8_t *pSessId      = (uint8_t *)&mSessionHandle[deviceId];
            UWB_BE_STREAM_TO_UINT32(devSessionId, pSessId);
            LOG_W("Inband Termination for: %04X", devSessionId);
#endif // !(UWBIOT_LOG_SILENT) && DEBUG
            mSessionState[deviceId] = notStarted;
        }
        switch (mSessionState[deviceId]) {
        case notStarted:
            LOG_D("Deleting session: %02X", mSessionHandle[deviceId]);
            operation = UwbApi_SessionDeinit(mSessionHandle[deviceId]);
            if (operation == UWBAPI_STATUS_OK || operation == UWBAPI_STATUS_SESSION_NOT_EXIST) {
                mSessionState[deviceId] = notCreated;
                status                  = TRUE;
            }
            else {
                status = FALSE;
            }
            break;
        case Started:
            LOG_D("Stopping session: %02X", mSessionHandle[deviceId]);
            operation = UwbApi_StopRangingSession(mSessionHandle[deviceId]);
            if (operation == UWBAPI_STATUS_OK || operation == UWBAPI_STATUS_SESSION_NOT_EXIST) {
                mSessionState[deviceId] = notStarted;
                status                  = TRUE;
            }
            else {
                status = FALSE;
            }
            break;
        default:
            LOG_E("Stop session wrong state: %d", mSessionState[deviceId]);
            status = FALSE;
            break;
        }
    }
    return status;
}

bool handleShutDown(void)
{
    bool status              = TRUE;
    tUWBAPI_STATUS operation = UWBAPI_STATUS_OK;

    operation = UwbApi_ShutDown();
    if (operation == UWBAPI_STATUS_OK) {
        LOG_W("device deinit");
        gDeviceInitialized = FALSE;
        memset(mSessionState, notCreated, gAppMaxConnections_c);
        memset(mSessionHandle, (uint32_t)0xA5A5A5A5, gAppMaxConnections_c);
        memset(mDevice, kDev_unknown_c, gAppMaxConnections_c);
        memset(mMacAddr, (uint16_t)0xA5A5, gAppMaxConnections_c);
    }
    else {
        status = FALSE;
        LOG_E("Error shutting down: %02X", operation);
    }
    return status;
}

bool handleDeviceInit(void)
{
    tUWBAPI_STATUS uwb_status = UWBAPI_STATUS_OK;
    if (!gDeviceInitialized) {
        LOG_W("device init");
        uwb_status = UwbApi_Init(AppCallback);
        if (uwb_status != UWBAPI_STATUS_OK) {
            LOG_E("UwbApi_Init failed");
            return FALSE;
        }
        else {
            gDeviceInitialized = TRUE;

            // TX_POWER_ID
            uint16_t bitmask = (1<<1); // TX_POWER_ID
            phCalibPayload_t calib;
    /*  calib.TX_POWER_ID[4]
        [0]: TX_RMS_POWER_IND for TX ID1
        [1]: TX_Peak_POWER_IND for TX ID1
        [2]: TX_RMS_POWER_IND for TX ID2
        [3]: TX_Peak_POWER_IND for TX ID2
    */
            uint8_t channels[] = {5, 9};
            uint8_t channel;
            uint8_t TX_POWER[11] = {0x02 /* Number of parameters */,
                                    0x01 /* for TX ID 1 */, 0x00, 0x00, 0x00, 0x00,
                                    0x02 /* for TX ID 2 */, 0x00, 0x00, 0x00, 0x00};
    /* TX_POWER[11]
       [0]: Number of paramters
       [1]: TX ID 1
       [2][3]: TX_POWER_DELTA_PEAK for TX ID 1
       [4][5]: TX_POWER_ID_RMS for TX ID 1
       [6]: TX ID 2
       [7][8]: TX_POWER_DELTA_PEAK for TX ID 2
       [9][10]: TX_POWER_ID_RMS for TX ID 2
    */
            uint8_t CLK_ACCURACY[7] = { 0x03 /* Number of parameters */,
                                        0x00, 0x00 /* CAP1 */,
                                        0x00, 0x00 /* CAP2 */,
                                        0x00, 0x00 /* GM CURRNT CONTROL */};

            int i;
            for(i = 0; i < sizeof(channels); i++) {
                channel = channels[i];
                UwbApi_ReadOtpCalibDataCmd(channel, bitmask, &calib);
                TX_POWER[4] = calib.TX_POWER_ID[0] + (2.1-0.6+0.5)*4;
                TX_POWER[2] = calib.TX_POWER_ID[1];
                UwbApi_SetCalibration(channel, TX_POWER_PER_ANTENNA, TX_POWER, sizeof(TX_POWER));
            }

            // XTAL_CAP
            bitmask = (1<<2); // XTAL_CAP
            UwbApi_ReadOtpCalibDataCmd(9, bitmask, &calib);    // channel 9 is dummy. not used for XTAL_CAP
            CLK_ACCURACY[1] = calib.XTAL_CAP_VALUES[0];
            CLK_ACCURACY[3] = calib.XTAL_CAP_VALUES[1];
            CLK_ACCURACY[5] = calib.XTAL_CAP_VALUES[2];
            UwbApi_SetCalibration(9, RF_CLK_ACCURACY_CALIB, CLK_ACCURACY, sizeof(CLK_ACCURACY));
        }
    }
    return TRUE;
}

#endif //UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
