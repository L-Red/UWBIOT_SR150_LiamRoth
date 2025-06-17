/*! *********************************************************************************
* \addtogroup Private Profile Server
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2023 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the QPP Server application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */
#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION_CLIENT
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION_CLIENT
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
#include "ble_service_discovery.h"

#include "board.h"
#include "ApplMain.h"
#include "ble_client.h"
#include "demo_device_config.h"

#include "UWBT_PowerMode.h"
#include "UWBT_Config.h"
#include "uwb_types.h"
#include "UwbApi_Types.h"

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
typedef enum appEvent_tag
{
    mAppEvt_PeerConnected_c,
    mAppEvt_PairingComplete_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattRspReceived_c,
    mAppEvt_GattProcError_c
} appEvent_t;

typedef enum appState_tag
{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppServiceDisc_c,
    mAppEnableNotifications_c,
    mAppRunning_c,
    mAppInitUwb_c,
    mAppStartUwb_c,
} appState_t;

typedef struct appCustomInfo_tag
{
    qppConfig_t qppClientConfig;
} appCustomInfo_t;

typedef struct appPeerInfo_tag
{
    bleDeviceAddress_t bleAddress;
    deviceId_t deviceId;
    appCustomInfo_t customInfo;
    bool_t isBonded;
    appState_t appState;
    uint64_t bytsReceivedPerInterval;
    uint64_t bytsSentPerInterval;
    uint32_t SessionHandle;
    uint8_t AccRole;
    uint16_t DevMacAdd;
    uint16_t AccMacAdd;
} appPeerInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
uint8_t gAppSerMgrIf;
static appPeerInfo_t mPeerInformation[gAppMaxConnections_c];
const uint8_t DUMMY_ADD[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#if gAppUseBonding_d
static bool_t mRestoringBondedLink = FALSE;
#if gAppUsePrivacy_d
static bool_t mAttemptRpaResolvingAtConnect = FALSE;
#endif
#endif

static bool_t mScanningOn           = FALSE;
static bool_t mFoundDeviceToConnect = FALSE;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t *mpCharProcBuffer = NULL;
static gattCharacteristic_t mCharBuffer;

/* UWB operation status */
extern QueueHandle_t mTlvMutex;

static uint8_t Rx_buffer[255];
static uint16_t Rx_buffer_len;

extern void InitUwb(void);
extern void DeInitUwb(void);
extern bool StartUwbSession(uint32_t *session_id, uint8_t acc_role, uint16_t dev_mac_add);
extern void StopUwbSession(uint32_t sessionHandle);
extern uint8_t gDstMacAddr[2];

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
static void BleApp_ScanningCallback(gapScanningEvent_t *pScanningEvent);

static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent);

static void BleApp_GattClientCallback(deviceId_t serverDeviceId,
    gattProcedureType_t procedureType,
    gattProcedureResult_t procedureResult,
    bleResult_t error);

static void BleApp_GattNotificationCallback(
    deviceId_t serverDeviceId, uint16_t characteristicValueHandle, uint8_t *aValue, uint16_t valueLength);

static void BleApp_ServiceDiscoveryCallback(deviceId_t deviceId, servDiscEvent_t *pEvent);

static void BleApp_Config(void);

void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event);

static bool_t CheckScanEvent(gapScannedDevice_t *pData);

static void BleApp_StoreServiceHandles(deviceId_t peerDeviceId, gattService_t *pService);

static bleResult_t Send_OOB_cmd(uint8_t deviceId, uint8_t *cmd, uint16_t sz);

static uint8_t howManyPeer(void)
{
    uint8_t nb_peer = 0;
    for (uint8_t i = 0; i < gAppMaxConnections_c; i++) {
        if (mPeerInformation[i].deviceId != gInvalidDeviceId_c)
            nb_peer++;
    }
    return nb_peer;
}

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
    UWB_Interrupt_ISR_Init();

    RNG_Init();

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
    if (!mScanningOn) {
        App_StartScanning(&gScanParams,
            BleApp_ScanningCallback,
            gGapDuplicateFilteringEnable_c,
            gGapScanContinuously_d,
            gGapScanPeriodicDisabled_d);
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
    /* Configure as GAP Central */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    App_RegisterGattClientNotificationCallback(BleApp_GattNotificationCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

    for (uint8_t i = 0; i < gAppMaxConnections_c; i++) {
        mPeerInformation[i].appState                = mAppIdle_c;
        mPeerInformation[i].deviceId                = gInvalidDeviceId_c;
        mPeerInformation[i].bytsReceivedPerInterval = 0;
        mPeerInformation[i].bytsSentPerInterval     = 0;
    }

    mScanningOn           = FALSE;
    mFoundDeviceToConnect = FALSE;

    BleApp_Start();
}

/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback(gapScanningEvent_t *pScanningEvent)
{
#if gAppUsePrivacy_d && gAppUseBonding_d
    uint8_t bondedDevicesCnt = 0;
#endif

    switch (pScanningEvent->eventType) {
    case gDeviceScanned_c: {
        if (mFoundDeviceToConnect == FALSE) {
            mFoundDeviceToConnect = CheckScanEvent(&pScanningEvent->eventData.scannedDevice);

            if (mFoundDeviceToConnect == TRUE) {
                gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                FLib_MemCpy(gConnReqParams.peerAddress,
                    pScanningEvent->eventData.scannedDevice.aAddress,
                    sizeof(bleDeviceAddress_t));

                (void)Gap_StopScanning();
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
                gConnReqParams.usePeerIdentityAddress =
                    pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
            }
        }
    } break;

    case gScanStateChanged_c: {
        mScanningOn = !mScanningOn;
        if (mScanningOn) {
            mFoundDeviceToConnect = FALSE;
            LED_StopFlashingAllLeds();
            Led1Flashing();
            PRINTF("\r\nScanning...\r\n");
        }
        else /* Scanning is turned OFF */
        {
            if (mFoundDeviceToConnect == TRUE) {
#if gAppUsePrivacy_d
                if (gConnReqParams.peerAddressType == gBleAddrTypeRandom_c) {
#if gAppUseBonding_d
                    /* Check if there are any bonded devices */
                    Gap_GetBondedDevicesCount(&bondedDevicesCnt);

                    if (bondedDevicesCnt == 0) {
                        /* display the unresolved RPA address */
                        Serial_PrintHex(
                            gAppSerMgrIf, gConnReqParams.peerAddress, gcBleDeviceAddressSize_c, gPrtHexNoFormat_c);
                    }
                    else {
                        mAttemptRpaResolvingAtConnect = TRUE;
                    }
#else
                    /* If bonding is disabled and we receive an RPA address there is nothing to do but display it */
                    Serial_PrintHex(
                        gAppSerMgrIf, gConnReqParams.peerAddress, gcBleDeviceAddressSize_c, gPrtHexNoFormat_c);
#endif /* gAppUseBonding_d */
                }
                else {
                    /* display the public/resolved address */
                    Serial_PrintHex(
                        gAppSerMgrIf, gConnReqParams.peerAddress, gcBleDeviceAddressSize_c, gPrtHexNoFormat_c);
                }
#else
                /* Display the peer address */
                //Serial_PrintHex(gAppSerMgrIf, gConnReqParams.peerAddress, gcBleDeviceAddressSize_c, gPrtHexNoFormat_c);
#endif /* gAppUsePrivacy_d */

                (void)App_Connect(&gConnReqParams, BleApp_ConnectionCallback);
            }
        }
    } break;

    case gScanCommandFailed_c: {
    }
    default:
        break;
    }
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
    BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType) {
    case gConnEvtConnected_c: {
#if gAppUsePrivacy_d && gAppUseBonding_d
        if (mAttemptRpaResolvingAtConnect == TRUE) {
            /* If the peer RPA was resolved, the IA is displayed, otherwise the peer RPA address is displayed */
            Serial_PrintHex(gAppSerMgrIf,
                pConnectionEvent->eventData.connectedEvent.peerAddress,
                gcBleDeviceAddressSize_c,
                gPrtHexNoFormat_c);
            /* clear the flag */
            mAttemptRpaResolvingAtConnect = FALSE;
        }
#endif
        /* UI */
        LED_StopFlashingAllLeds();
        Led1On();
        PRINTF("\r\nConnected!\r\n");

        memcpy(mPeerInformation[peerDeviceId].bleAddress,
            pConnectionEvent->eventData.connectedEvent.peerAddress,
            sizeof(bleDeviceAddress_t));
        mPeerInformation[peerDeviceId].deviceId = peerDeviceId;
        mPeerInformation[peerDeviceId].isBonded = FALSE;

        if (howManyPeer() < gAppMaxConnections_c) {
            /* Restart scanning */
            App_StartScanning(&gScanParams,
                BleApp_ScanningCallback,
                gGapDuplicateFilteringEnable_c,
                gGapScanContinuously_d,
                gGapScanPeriodicDisabled_d);
        }

#if gAppUseBonding_d
        Gap_CheckIfBonded(peerDeviceId, &mPeerInformation[peerDeviceId].isBonded, NULL);

        if ((mPeerInformation[peerDeviceId].isBonded) &&
            (gBleSuccess_c ==
                Gap_LoadCustomPeerInformation(
                    peerDeviceId, (void *)&mPeerInformation[peerDeviceId].customInfo, 0, sizeof(appCustomInfo_t)))) {
            mRestoringBondedLink = TRUE;
            /* Restored custom connection information. Encrypt link */
            Gap_EncryptLink(peerDeviceId);
        }
#endif
        BleApp_StateMachineHandler(mPeerInformation[peerDeviceId].deviceId, mAppEvt_PeerConnected_c);
    } break;

    case gConnEvtDisconnected_c: {
        memcpy(mPeerInformation[peerDeviceId].bleAddress, DUMMY_ADD, sizeof(bleDeviceAddress_t));
        mPeerInformation[peerDeviceId].deviceId                = gInvalidDeviceId_c;
        mPeerInformation[peerDeviceId].appState                = mAppIdle_c;
        mPeerInformation[peerDeviceId].bytsReceivedPerInterval = 0;
        mPeerInformation[peerDeviceId].bytsSentPerInterval     = 0;

        StopUwbSession(mPeerInformation[peerDeviceId].SessionHandle);

        /* Reset Service Discovery to be sure*/
        BleServDisc_Stop(peerDeviceId);
        for (uint8_t i = 0; i < gAppMaxConnections_c; i++) {
            if (mPeerInformation[i].deviceId != gInvalidDeviceId_c)
                break;
        }

        /* No more accessory connected */
        if (howManyPeer() == 0) {
            DeInitUwb();
        }

        /* UI */
        LED_TurnOffAllLeds();
        LED_StartFlash(LED_ALL);
        PRINTF("\r\nDisconnected!\r\n");

        if (!mScanningOn) {
            App_StartScanning(&gScanParams,
                BleApp_ScanningCallback,
                gGapDuplicateFilteringEnable_c,
                gGapScanContinuously_d,
                gGapScanPeriodicDisabled_d);
        }
    } break;

#if gAppUsePairing_d
    case gConnEvtPairingComplete_c: {
        if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful) {
            BleApp_StateMachineHandler(mPeerInformation[peerDeviceId].deviceId, mAppEvt_PairingComplete_c);
        }
    } break;

    case gConnEvtEncryptionChanged_c: {
        if (mRestoringBondedLink) {
            mRestoringBondedLink = FALSE;
            uint16_t value       = gCccdNotification_c;
            GattClient_WriteCharacteristicDescriptor(
                mPeerInformation[peerDeviceId].deviceId, mpCharProcBuffer, sizeof(value), (void *)&value);
        }
    } break;

    case gConnEvtAuthenticationRejected_c: {
        /* Start Pairing Procedure */
        (void)Gap_Pair(peerDeviceId, &gPairingParameters);
    } break;

#endif /* gAppUsePairing_d */

    default:
        break;
    }
}

static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t *pEvent)
{
    switch (pEvent->eventType) {
    case gServiceDiscovered_c: {
        BleApp_StoreServiceHandles(peerDeviceId, pEvent->eventData.pService);
    } break;

    case gDiscoveryFinished_c: {
        if (pEvent->eventData.success) {
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryComplete_c);
        }
        else {
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryFailed_c);
        }
    } break;

    default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles GATT client callback from host stack.
*
* \param[in]    serverDeviceId      GATT Server device ID.
* \param[in]    procedureType       Procedure type.
* \param[in]    procedureResult     Procedure result.
* \param[in]    error               Callback result.
********************************************************************************** */
static void BleApp_GattClientCallback(deviceId_t serverDeviceId,
    gattProcedureType_t procedureType,
    gattProcedureResult_t procedureResult,
    bleResult_t error)
{
    if (procedureResult == gGattProcError_c) {
        attErrorCode_t attError = (attErrorCode_t)(error & 0xFF);
        if (attError == gAttErrCodeInsufficientEncryption_c || attError == gAttErrCodeInsufficientAuthorization_c ||
            attError == gAttErrCodeInsufficientAuthentication_c) {
            /* Start Pairing Procedure */
            Gap_Pair(serverDeviceId, &gPairingParameters);
        }

        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
    }
    else if (procedureResult == gGattProcSuccess_c) {
        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
}

/*! *********************************************************************************
* \brief        Handles GATT client notification callback from host stack.
*
* \param[in]    serverDeviceId                  GATT Server device ID.
* \param[in]    characteristicValueHandle           Handle.
* \param[in]    aValue                      Pointer to value.
* \param[in]    valueLength                 Value length.
********************************************************************************** */

static void BleApp_GattNotificationCallback(
    deviceId_t serverDeviceId, uint16_t characteristicValueHandle, uint8_t *aValue, uint16_t valueLength)
{
    if (characteristicValueHandle == mPeerInformation[serverDeviceId].customInfo.qppClientConfig.hTxData) {
        Rx_buffer_len = valueLength;
        memcpy(Rx_buffer, aValue, valueLength);
        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattRspReceived_c);
    }
}

/*! *********************************************************************************
* \brief        Stores handles for the specified service.
*
* \param[in]    pService    Pointer to gattService_t.
********************************************************************************** */
static void BleApp_StoreServiceHandles(deviceId_t peerDeviceId, gattService_t *pService)
{
    uint8_t i, j;
    static uint8_t base_uuid[16] = {
        0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (pService->uuidType == gBleUuidType16_c) {
        base_uuid[12] = pService->uuid.uuid16;
        base_uuid[13] = pService->uuid.uuid16 >> 8;
    }

    if ((FLib_MemCmp(base_uuid, uuid_service_qpps, 16)) ||
        (FLib_MemCmp(pService->uuid.uuid128, uuid_service_qpps, 16))) {
        /* Found QPPS Service */
        mPeerInformation[peerDeviceId].customInfo.qppClientConfig.hService = pService->startHandle;

        for (i = 0; i < pService->cNumCharacteristics; i++) {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType128_c) &&
                FLib_MemCmp(pService->aCharacteristics[i].value.uuid.uuid128, uuid_qpps_characteristics_tx, 16)) {
                /* Found QPPS TX Char */
                mPeerInformation[peerDeviceId].customInfo.qppClientConfig.hTxData =
                    pService->aCharacteristics[i].value.handle;

                for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++) {
                    if ((pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c) &&
                        (pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16 == gBleSig_CCCD_d)) {
                        mPeerInformation[peerDeviceId].customInfo.qppClientConfig.hTxCccd =
                            pService->aCharacteristics[i].aDescriptors[j].handle;
                    }
                }
            }

            else if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType128_c) &&
                     FLib_MemCmp(pService->aCharacteristics[i].value.uuid.uuid128, uuid_qpps_characteristics_rx, 16)) {
                /* Found QPPS RX Char */
                mPeerInformation[peerDeviceId].customInfo.qppClientConfig.hRxData =
                    pService->aCharacteristics[i].value.handle;
            }
        }
    }
}

/*! *********************************************************************************
* \brief        Check for specific information in advertising data
*
* \param[in]    pElement        Pointer to the advertising data structure.
* \param[in]    pData           Pointer to the the data to be matched.
* \param[in]    iDataLen        Length of the data to be matched.
*
* \return       TRUE if a match was found. Else FALSE.
********************************************************************************** */
static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement, void *pData, uint8_t iDataLen)
{
    uint8_t i;
    bool_t status = FALSE;

    for (i = 0; i < (pElement->length - 1); i += iDataLen) {
        if (FLib_MemCmp(pData, &pElement->aData[i], iDataLen)) {
            status = TRUE;
            break;
        }
    }
    return status;
}

/*! *********************************************************************************
* \brief        Advertising report handler
*
* \param[in]    pData        Pointer to the device informaion.
*
* \return       TRUE if a device with QPP UUID was found. Else FALSE.
********************************************************************************** */
static bool_t CheckScanEvent(gapScannedDevice_t *pData)
{
    uint8_t i, index = 0;
    uint8_t name[16];
    uint8_t nameLength = 0U;
    bool_t foundMatch  = FALSE;

    /* Check if discovered device is already connected */
    for (i = 0; i < gAppMaxConnections_c; i++) {
        if (!memcmp(mPeerInformation[i].bleAddress, pData->aAddress, sizeof(bleDeviceAddress_t))) {
            //PRINTF("\r\nAlready connected device found, ignoring\r\n");
            goto end;
        }
    }

    while (index < pData->dataLength) {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1];
        adElement.aData  = &pData->data[index + 2];

        /* Search for Temperature Custom Service */
        if ((adElement.adType == gAdIncomplete128bitServiceList_c) ||
            (adElement.adType == gAdComplete128bitServiceList_c)) {
            foundMatch = MatchDataInAdvElementList(&adElement, &uuid_service_qpps, 16);
        }

        if ((adElement.adType == gAdShortenedLocalName_c) || (adElement.adType == gAdCompleteLocalName_c)) {
            nameLength = MIN(adElement.length, 16);
            FLib_MemCpy(name, adElement.aData, nameLength);
        }

        /* Move on to the next AD elemnt type */
        index += adElement.length + sizeof(uint8_t);
    }

    if (foundMatch) {
        /* UI */
        PRINTF("\r\nFound device: %s\r\n", name);
    }
end:
    return foundMatch;
}

static bleResult_t Send_OOB_cmd(uint8_t deviceId, uint8_t *cmd, uint16_t sz)
{
    mCharBuffer.value.handle = mPeerInformation[deviceId].customInfo.qppClientConfig.hRxData;
    return GattClient_SimpleCharacteristicWrite(deviceId, &mCharBuffer, sz, cmd);
}

/*! *********************************************************************************
* \brief        Application state machine
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Received event
********************************************************************************** */
void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event)
{
    bleResult_t result = gBleInvalidParameter_c;

    switch (mPeerInformation[peerDeviceId].appState) {
    case mAppIdle_c: {
        if (event == mAppEvt_PeerConnected_c) {
            uint8_t testData         = 0xA5;
            mCharBuffer.value.handle = mPeerInformation[peerDeviceId].customInfo.qppClientConfig.hRxData;
            result =
                GattClient_CharacteristicWriteWithoutResponse(peerDeviceId, &mCharBuffer, sizeof(testData), &testData);
            if (result == gBleSuccess_c) {
                mPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
            }
        }
    } break;

    case mAppExchangeMtu_c: {
        if (event == mAppEvt_GattProcComplete_c) {
            (void)Gap_UpdateLeDataLength(peerDeviceId, gBleMaxTxOctets_c, gBleMaxTxTime_c);

            /* Moving to Service Discovery State*/
            mPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

            /* Start Service Discovery*/
            BleServDisc_Start(peerDeviceId);
        }
        else if (event == mAppEvt_GattProcError_c) {
            Gap_Disconnect(peerDeviceId);
        }
    } break;

    case mAppServiceDisc_c: {
        if (event == mAppEvt_ServiceDiscoveryComplete_c) {
            mPeerInformation[peerDeviceId].appState = mAppEnableNotifications_c;
            if (mPeerInformation[peerDeviceId].customInfo.qppClientConfig.hTxCccd) {
                mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + 23);
                if (!mpCharProcBuffer)
                    return;
                mpCharProcBuffer->handle  = mPeerInformation[peerDeviceId].customInfo.qppClientConfig.hTxCccd;
                mpCharProcBuffer->paValue = (uint8_t *)(mpCharProcBuffer + 1);
                GattClient_ReadCharacteristicDescriptor(mPeerInformation[peerDeviceId].deviceId, mpCharProcBuffer, 23);
            }
        }
        else if (event == mAppEvt_ServiceDiscoveryFailed_c) {
            Gap_Disconnect(peerDeviceId);
        }
    } break;

    case mAppEnableNotifications_c: {
        if (event == mAppEvt_GattProcComplete_c) {
            if (mpCharProcBuffer &&
                (mpCharProcBuffer->handle == mPeerInformation[peerDeviceId].customInfo.qppClientConfig.hTxCccd)) {
                uint16_t value = gCccdNotification_c;
                /* Moving to Running State*/
                mPeerInformation[peerDeviceId].appState = mAppRunning_c;
                GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer, sizeof(value), (void *)&value);
            }
        }
        else if (event == mAppEvt_PairingComplete_c) {
            /* Continue after pairing is complete */
            GattClient_ReadCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer, 23);
        }
    } break;

    case mAppRunning_c: {
        if (event == mAppEvt_GattProcComplete_c) {
            /* Tag connected trigger UWB scenario */
            uint8_t cmd[]      = {0xA5};
            bleResult_t result = Send_OOB_cmd(peerDeviceId, cmd, sizeof(cmd));
            if (result == gBleSuccess_c) {
                mPeerInformation[peerDeviceId].appState = mAppInitUwb_c;
            }
            else {
                Gap_Disconnect(peerDeviceId);
            }

#if gAppUseBonding_d
            /* Write data in NVM */
            Gap_SaveCustomPeerInformation(mPeerInformation[peerDeviceId].deviceId,
                (void *)&mPeerInformation[peerDeviceId].customInfo,
                0,
                sizeof(appCustomInfo_t));
#endif
        }
        else if (event == mAppEvt_PairingComplete_c) {
            if (mpCharProcBuffer) {
                uint16_t value = gCccdNotification_c;
                GattClient_WriteCharacteristicDescriptor(
                    mPeerInformation[peerDeviceId].deviceId, mpCharProcBuffer, sizeof(value), (void *)&value);
            }
        }
    } break;

    case mAppInitUwb_c: {
        if (event == mAppEvt_GattRspReceived_c) {
            if ((Rx_buffer_len == 19) && (Rx_buffer[0] == 0x01)) {
                uint32_t devMacAddr;
                RNG_GetRandomNo(&mPeerInformation[peerDeviceId].SessionHandle);
                RNG_GetRandomNo(&devMacAddr);
                if (Rx_buffer[16] & UWB_DEVICE_CONTROLEE) {
                    mPeerInformation[peerDeviceId].AccRole = UWB_DEVICE_CONTROLEE;
                }
                else {
                    mPeerInformation[peerDeviceId].AccRole = UWB_DEVICE_CONTROLLER;
                }
                mPeerInformation[peerDeviceId].DevMacAdd = devMacAddr & 0xFFFF;
                gDstMacAddr[0]                           = Rx_buffer[17];
                gDstMacAddr[1]                           = Rx_buffer[18];
                // mPeerInformation[peerDeviceId].AccMacAdd = Rx_buffer[17] + (Rx_buffer[18] << 8);

                /* Sends UWB (Android phone like) configuration */
                uint8_t cmd[] = {0x0B,
                    0x01,
                    0x00,
                    0x00,
                    0x00, // Spec major - minor
                    0xFF,
                    0xFF,
                    0xFF,
                    0xFF, // Session ID
                    0x0A, // Preamble ID
                    0x09, // Channel number
                    DEMO_SET_CONFIG_ID, // Profile ID
                    0x02, // Accessory ranging role
                    0xFF,
                    0xFF}; // MAC address
                cmd[5]        = mPeerInformation[peerDeviceId].SessionHandle >> 24;
                cmd[6]        = mPeerInformation[peerDeviceId].SessionHandle >> 16;
                cmd[7]        = mPeerInformation[peerDeviceId].SessionHandle >> 8;
                cmd[8]        = mPeerInformation[peerDeviceId].SessionHandle;
                cmd[12]       = mPeerInformation[peerDeviceId].AccRole;
                cmd[13]       = mPeerInformation[peerDeviceId].DevMacAdd;
                cmd[14]       = mPeerInformation[peerDeviceId].DevMacAdd >> 8;

                bleResult_t result = Send_OOB_cmd(peerDeviceId, cmd, sizeof(cmd));
                if (result == gBleSuccess_c) {
                    mPeerInformation[peerDeviceId].appState = mAppStartUwb_c;
                }
                else {
                    Gap_Disconnect(peerDeviceId);
                }
            }
            else {
                Gap_Disconnect(peerDeviceId);
            }
        }
    } break;

    case mAppStartUwb_c: {
        if (event == mAppEvt_GattRspReceived_c) {
            if ((Rx_buffer_len == 1) && (Rx_buffer[0] == 0x02)) {
                //if (howManyPeer()==0){
                InitUwb();
                //}
                uint32_t *sessionHandle = &mPeerInformation[peerDeviceId].SessionHandle;

                /* Start UWB */
                if (StartUwbSession(sessionHandle,
                        mPeerInformation[peerDeviceId].AccRole,
                        mPeerInformation[peerDeviceId].DevMacAdd)) {
                    /* ok */
                }
                else {
                    Gap_Disconnect(peerDeviceId);
                }
            }
            else {
                Gap_Disconnect(peerDeviceId);
            }
        }
    } break;
    }
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
#endif /* UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION_CLIENT */
/*! *********************************************************************************
* @}
********************************************************************************** */
