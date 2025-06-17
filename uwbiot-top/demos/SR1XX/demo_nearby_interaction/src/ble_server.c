/*! *********************************************************************************
* \addtogroup Private Profile Server
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2021-2023 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the QPP Server application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */
#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* #undef CR_INTEGER_PRINTF to force the usage of the sprintf() function provided
 * by the compiler in this file. The sprintf() function is #included from
 * the <stdio.h> file. */
#ifdef CR_INTEGER_PRINTF
#undef CR_INTEGER_PRINTF
#endif

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

/* TLV includes */
#include "TLV_Types_i.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "PWR_Configuration.h"

/* Framework / Drivers */
#include "stdio.h"
#include "driver_config.h"
#include "RNG_Interface.h"
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "MemManager.h"
#include "Panic.h"
#include "PWRLib.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "private_profile_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "demo_ble_server.h"

#include "UWBT_PowerMode.h"
#include "UWBT_Config.h"
#include "uwb_types.h"

#if (gAppNtagSupported_d)
#include "app_ntag.h"
#endif

#if gBtnSupported_d
#include "gpio_pins.h"
#include "ButtonScan.h"
#endif

#include "phTmlUwb_transport.h"
#include "phOsalUwb.h"

void UWB_Interrupt_ISR_Init(void);
void UWB_Interrupt_ISR(void);
extern QueueHandle_t tlvMngQueue;
extern uint32_t mSessionHandle;

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mAttNotifHeaderSize_c              (3) /* ATT op code + ATT handle  */
#define BLE_CONN_CB_MAX_REENTRANCE_TIMEOUT (0x2000)

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum
{
#if gAppUseBonding_d
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d > 0)
    fastWhiteListAdvState_c,
#endif // defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d > 0)
#endif // gAppUseBonding_d
    fastAdvState_c,
    slowAdvState_c,
    defaultAdvState_c
} advType_t;

typedef struct advState_tag
{
    bool_t advOn;
    advType_t advType;
} advState_t;

typedef struct appPeerInfo_tag
{
    uint8_t deviceId;
    uint8_t ntf_cfg;
} appPeerInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
/* Adv State */
static advState_t mAdvState;
//static bool_t      mRestartAdv;
static uint32_t mAdvTimeout;
/* Service Data*/
static qppsConfig_t qppServiceConfig = {service_qpps};

static uint16_t cpHandles[1]     = {value_qpps_rx};
static uint16_t cpReadHandles[1] = {value_nearby_data};

/* Application specific data*/
static appPeerInfo_t mPeerInformation[gAppMaxConnections_c];

#if TAG_BUILD_CFG == MANUF_TEST_BUILD || TAG_BUILD_CFG == VALIDATION_V3_BUILD
extern bool bleConnection;
#endif

/* UWB operation status */
extern QueueHandle_t mTlvMutex;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent);
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent);
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent);
static void BleApp_Config(void);
static void BleApp_Advertise(void);
static void BleApp_ReceivedDataHandler(deviceId_t deviceId, uint8_t *aValue, uint16_t valueLength);

#if gBtnSupported_d
/*! *********************************************************************************
* \brief        Handles button events.
*
* \param[in]    events    button event structure.
********************************************************************************** */
static void App_ButtonCallBack(uint8_t events);
#endif
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void)
{
#if gBtnSupported_d && (gBtn_Count_c > 0)
    Btn_Init(App_ButtonCallBack);
#endif
    UWB_Interrupt_ISR_Init();

#if defined(CPU_JN518X) && (cPWR_UsePowerDownMode)
    TMR_TimeStampInit();
    PWR_RegisterLowPowerExitCallback(UWBT_ExitPowerDownCb);
    PWR_RegisterLowPowerEnterCallback(UWBT_EnterLowPowerCb);
#endif

    // Nothing to do here at the moment
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
#if gAppUseBonding_d
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d > 0)
    if (gcBondedDevices > 0) {
        mAdvState.advType = fastWhiteListAdvState_c;
    }
    else
#endif
    {
#endif
        mAdvState.advType = defaultAdvState_c;
#if gAppUseBonding_d
    }
#endif

#if (gAppNtagSupported_d)
    NtagApp_NdefPairingWr(PERIPHERAL_AND_CENTRAL_ROLE, NTAG_LOCAL_DEV_NAME, strlen(NTAG_LOCAL_DEV_NAME));
#endif
    BleApp_Advertise();
}

#if gBtnSupported_d
/*! *********************************************************************************
* \brief        Handles button events.
*
* \param[in]    events    button event structure.
********************************************************************************** */
void App_ButtonCallBack(uint8_t events)
{
    PRINTF("App_ButtonCallBack\n\r");
    switch (events) {
    case gBtn_EventPB1_c:
#if ((defined APP_WWDT_ENABLE) && (APP_WWDT_ENABLE))
        PRINTF(
            "button single short pressed -->>\n\r"
            "system would to reset after wdt overflow \r\n");
#else
        PRINTF("button single short pressed\r\n");
#endif
        break;
    case gBtn_EventPB1Double_c:
        PRINTF("button double short pressed\n\r");
        break;
    case gBtn_EventOneSecondPB1_c:
        PRINTF("button 1s long pressed\n\r");
        break;
    case gBtn_EventSevenHalfSecondPB1_c:
        PRINTF("button 7.5s long pressed\n\r");
        break;
    default:
        break;
    }
}
#endif
/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    switch (events) {
    case gKBD_EventPressPB1_c: {
        break;
    }
    case gKBD_EventLongPB1_c: {
        break;
    }
    case gKBD_EventLongPB2_c: {
        break;
    }
    default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback(gapGenericEvent_t *pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType) {
    case gInitializationComplete_c: {
        BleApp_Config();
    } break;

    case gAdvertisingParametersSetupComplete_c: {
        (void)Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);
    } break;

    case gAdvertisingDataSetupComplete_c: {
        (void)Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    } break;

    case gTxPowerLevelSetComplete_c: {
        (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
    }

    case gAdvertisingSetupFailed_c: {
        //Serial_Print(gAppSerMgrIf, "\r\ngAdvertisingSetupFailed_c\r\n", gNoBlock_d);
        //panic(0,0,0,0);
    } break;

    case gBondCreatedEvent_c: {
        PRINTF("Binding done\n");
    } break;

    default:
        break;
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config()
{
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
    GattServer_RegisterHandlesForReadNotifications(NumberOfElements(cpReadHandles), cpReadHandles);
    App_RegisterGattServerCallback(BleApp_GattServerCallback);

    /* TODO: Load required BLE configurations from flash*/

    mAdvState.advOn = FALSE;
    for (uint8_t i = 0; i < gAppMaxConnections_c; i++) {
        mPeerInformation[i].deviceId = gInvalidDeviceId_c;
    }

    Qpp_Start(&qppServiceConfig);

    mAdvState.advType = defaultAdvState_c;
    BleApp_Advertise();
    PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
    PWR_AllowDeviceToSleep();
    //    PWR_PreventEnterLowPower(false);
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    switch (mAdvState.advType) {
#if gAppUseBonding_d
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d > 0)
    case fastWhiteListAdvState_c: {
        gAdvParams.minInterval  = gFastConnMinAdvInterval_c;
        gAdvParams.maxInterval  = gFastConnMaxAdvInterval_c;
        gAdvParams.filterPolicy = gProcessWhiteListOnly_c;
        mAdvTimeout             = gFastConnWhiteListAdvTime_c;
    } break;
#endif
#endif
    case fastAdvState_c: {
        gAdvParams.minInterval  = gFastConnMinAdvInterval_c;
        gAdvParams.maxInterval  = gFastConnMaxAdvInterval_c;
        gAdvParams.filterPolicy = gProcessAll_c;
        mAdvTimeout             = gFastConnAdvTime_c - gFastConnWhiteListAdvTime_c;
    } break;

    case slowAdvState_c: {
        gAdvParams.minInterval  = gReducedPowerMinAdvInterval_c;
        gAdvParams.maxInterval  = gReducedPowerMinAdvInterval_c;
        gAdvParams.filterPolicy = gProcessAll_c;
        mAdvTimeout             = gReducedPowerAdvTime_c;
    } break;
    case defaultAdvState_c: {
        gAdvParams.minInterval  = UWBT_CfgReadBleInterval();
        gAdvParams.maxInterval  = UWBT_CfgReadBleInterval();
        gAdvParams.filterPolicy = gProcessAll_c;
        mAdvTimeout             = gReducedPowerAdvTime_c;
    } break;
    }

    /* Set advertising parameters*/
    Gap_SetAdvertisingParameters(&gAdvParams);
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent)
{
    switch (pAdvertisingEvent->eventType) {
    case gAdvertisingStateChanged_c: {
        mAdvState.advOn = !mAdvState.advOn;
        PRINTF("BLE Start adv\r\n");
        if (mAdvState.advOn) {
            PWR_AllowDeviceToSleep();
        }
    } break;

    case gAdvertisingCommandFailed_c: {
        //Serial_Print(gAppSerMgrIf, "\r\ngAdvertisingCommandFailed_c\r\n", gNoBlock_d);
        //panic(0,0,0,0);
    } break;

    default:
        break;
    }
}

static uint8_t howManyPeer(void)
{
    uint8_t nb_peer = 0;
    for (uint8_t i = 0; i < gAppMaxConnections_c; i++) {
        if (mPeerInformation[i].deviceId != gInvalidDeviceId_c)
            nb_peer++;
    }
    return nb_peer;
}

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent)
{
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType) {
    case gConnEvtConnected_c: {
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        UWBT_PowerModeEnter(UWBT_RUN_MODE);
#endif /* cPWR_UsePowerDownMode */
        mPeerInformation[peerDeviceId].deviceId = peerDeviceId;

        if (phOsalUwb_ConsumeSemaphore_WithTimeout(mTlvMutex, BLE_CONN_CB_MAX_REENTRANCE_TIMEOUT) !=
            UWBSTATUS_SUCCESS) {
            LOG_E("BleApp_ConnectionCallback timeout");
        }
        if (!handleDeviceInit()) {
            LOG_E("Device init failed");
        }
        (void)phOsalUwb_ProduceSemaphore(mTlvMutex);

        /* Subscribe client*/
        (void)Qpp_Subscribe(peerDeviceId);

        LOG_I("BLE Connected to peer #%d, %d peers connected\r\n", peerDeviceId + 1, howManyPeer());
        /* Restart Advertising while max connection is not reached */
        if (howManyPeer() < gAppMaxConnections_c) {
            BleApp_Start();
        }

#if TAG_BUILD_CFG == MANUF_TEST_BUILD || TAG_BUILD_CFG == VALIDATION_V3_BUILD
        bleConnection = TRUE;
#endif

    } break;

    case gConnEvtEncryptionChanged_c: {
        PRINTF("Encrypted BLE connection\n");
    } break;

    case gConnEvtDisconnected_c: {
        /* qpps Unsubscribe client */
        Qpp_Unsubscribe();
        mPeerInformation[peerDeviceId].ntf_cfg  = QPPS_VALUE_NTF_OFF;
        mPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
        PRINTF("BLE Disconnected\r\n");
        if (!handleStopSession((uint8_t)peerDeviceId)) {
            LOG_E("Stopping session failed");
            break;
        }
        else {
            LOG_I("BLE Disconnected peer %d, %d peers conenctd\r\n", peerDeviceId + 1, howManyPeer());
        }
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(mTlvMutex, BLE_CONN_CB_MAX_REENTRANCE_TIMEOUT) !=
            UWBSTATUS_SUCCESS) {
            LOG_E("BleApp_ConnectionCallback timeout");
        }
        /* if not more peer connected, shutdown UWB */
        if (howManyPeer() == 0) {
            if (!handleShutDown()) {
                LOG_E("Stack Deinit failed");
            }
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
            UWBT_PowerModeEnter(UWBT_POWER_DOWN_MODE);
            PWR_AllowDeviceToSleep();
#endif /* cPWR_UsePowerDownMode */
        }
        (void)phOsalUwb_ProduceSemaphore(mTlvMutex);
        /* in any case restart advertising since a connection has been released */
        BleApp_Start();

#if TAG_BUILD_CFG == MANUF_TEST_BUILD || TAG_BUILD_CFG == VALIDATION_V3_BUILD
        bleConnection = FALSE;
#endif
    } break;

#if defined(gAppUsePairing_d) && (gAppUsePairing_d)
    case gConnEvtPairingComplete_c: {
        if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful) {
            LOG_I("BLE Pairing to peer #%d success\r\n", peerDeviceId + 1);
        }
        else {
            LOG_E("BLE Pairing to peer #%d failed\r\n", peerDeviceId + 1);
        }
    } break;
#endif // defined(gAppUsePairing_d) && (gAppUsePairing_d)

    default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent)
{
    uint16_t handle;
    uint8_t status;

    switch (pServerEvent->eventType) {
    case gEvtMtuChanged_c: {
        //uint8_t notifMaxPayload = 0;
        //notifMaxPayload = pServerEvent->eventData.mtuChangedEvent.newMtu - mAttNotifHeaderSize_c;
    } break;

    case gEvtAttributeWritten_c: {
        handle = pServerEvent->eventData.attributeWrittenEvent.handle;
        status = gAttErrCodeNoError_c;
        GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
        if (handle == value_qpps_rx) {
            BleApp_ReceivedDataHandler(deviceId,
                pServerEvent->eventData.attributeWrittenEvent.aValue,
                pServerEvent->eventData.attributeWrittenEvent.cValueLength);
        }
    } break;

    case gEvtAttributeWrittenWithoutResponse_c: {
        handle = pServerEvent->eventData.attributeWrittenEvent.handle;

        if (handle == value_qpps_rx) {
            BleApp_ReceivedDataHandler(deviceId,
                pServerEvent->eventData.attributeWrittenEvent.aValue,
                pServerEvent->eventData.attributeWrittenEvent.cValueLength);
        }
    } break;

    case gEvtCharacteristicCccdWritten_c: {
        handle = pServerEvent->eventData.charCccdWrittenEvent.handle;
        if (handle == cccd_qpps_tx) {
            mPeerInformation[deviceId].ntf_cfg = pServerEvent->eventData.charCccdWrittenEvent.newCccd;
        }
    } break;

    case gEvtAttributeRead_c: {
        handle = pServerEvent->eventData.attributeReadEvent.handle;
        LOG_I("Attribute read by peer device \r\n");
        GattServer_SendAttributeReadStatus(deviceId, handle, gAttErrCodeNoError_c);
    } break;

    default:
        break;
    }
}

static void BleApp_ReceivedDataHandler(deviceId_t deviceId, uint8_t *aValue, uint16_t valueLength)
{
    tlvRecv((uint8_t)deviceId, UWB_HIF_BLE, aValue, valueLength);
}

void UWB_Interrupt_ISR_Init(void)
{
    UWBT_InterruptISRInit(UWB_Interrupt_ISR);
}

void UWB_Interrupt_ISR(void)
{
    /** these re-initialization is not required here,
     * when device wakes up from Low Power Mode.
     *
     * phTmlUwb_open_and_configure(NULL, NULL);
     * phTmlUwb_helios_irq_enable();
     *
     */
}
#endif /* UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION */
/*! *********************************************************************************
* @}
********************************************************************************** */
