/* Copyright 2019-2020,2022,2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phOsalUwb.h"
#include "AppInternal.h"
//#include "UwbUsb.h"
#include "Utilities.h"
#include "AppRecovery.h"
#include "PrintUtility_RfTest.h"
#include "UwbApi_Types.h"
#include "UwbApi_Utility.h"

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if !defined(UWBIOT_APP_BUILD__DEMO_CDC)

void *perSem;
void *rangingDataSem;
void *inBandterminationSem;
void *testLoopBackNtfSem;
void *datatransferNtfSemRx;
void *datatransferNtfSemTx;
void *dataRcvNtfSemTx;
void *RadarTstAntIsoNtfSemTx;

#if !(UWBIOT_UWBD_SR040)
uint8_t dataToSend[UWBS_MAX_UCI_PACKET_SIZE];
uint8_t dataReceived[UWBS_MAX_UCI_PACKET_SIZE];
#endif

intptr_t ApduMngQueue;

/*
* Function to check whether sent data and received data are same or not
* It compares data received with sent data.
* If it matches, then only it returns success
*/
#if !(UWBIOT_UWBD_SR040)
tUWBAPI_STATUS validateReceivedData()
{
    if ((phOsalUwb_MemCompare(dataReceived, dataToSend, sizeof(dataToSend)) == UWBAPI_STATUS_OK)) {
        NXPLOG_APP_I("Data received successfully!!!");
        return UWBAPI_STATUS_OK;
    }
    else {
        NXPLOG_APP_E("Data NOT received successfully");
        return UWBAPI_STATUS_FAILED;
    }
}
#endif
void UWBDemo_Init()
{
    phOsalUwb_CreateSemaphore(&perSem, 0);
    phOsalUwb_CreateSemaphore(&rangingDataSem, 0);
    phOsalUwb_CreateSemaphore(&inBandterminationSem, 0);
    phOsalUwb_CreateSemaphore(&testLoopBackNtfSem, 0);
    phOsalUwb_CreateSemaphore(&datatransferNtfSemRx, 0);
    phOsalUwb_CreateSemaphore(&datatransferNtfSemTx, 0);
    phOsalUwb_CreateSemaphore(&dataRcvNtfSemTx, 0);
    phOsalUwb_CreateSemaphore(&RadarTstAntIsoNtfSemTx, 0);
}

void UWBDemo_DeInit()
{
    phOsalUwb_DeleteSemaphore(&perSem);
    phOsalUwb_DeleteSemaphore(&rangingDataSem);
    phOsalUwb_DeleteSemaphore(&inBandterminationSem);
    phOsalUwb_DeleteSemaphore(&testLoopBackNtfSem);
    phOsalUwb_DeleteSemaphore(&datatransferNtfSemRx);
    phOsalUwb_DeleteSemaphore(&datatransferNtfSemTx);
    phOsalUwb_DeleteSemaphore(&RadarTstAntIsoNtfSemTx);
}

/* Use for LED */
static int gToggleLEDBitCounter = 0;

static void demo_common_toggle_led_onAppCallback(void)
{
    gToggleLEDBitCounter++;
    switch (gToggleLEDBitCounter & 0x1) {
    case 0:
        UWB_BOARD_GPIO_SET_LED_O_ON();
        break;
    case 1:
        UWB_BOARD_GPIO_SET_LED_O_OFF();
        break;
        /* Could be extended with more LED Patterns */
    }
}

void AppCallback(eNotificationType opType, void *pData)
{
    switch (opType) {
    case UWBD_RANGING_DATA: {
        phRangingData_t *pRangingData = (phRangingData_t *)pData;
#if UWBIOT_UWBD_SR1XXT
#if UWBFTR_TWR // support only for DSTWR
        if (((pRangingData->ranging_meas.range_meas_twr[0].status == UWBAPI_STATUS_OK) ||
                (pRangingData->ranging_meas.range_meas_twr[0].status == UWBAPI_STATUS_OK_NEGATIVE_DISTANCE_REPORT)) &&
            ((pRangingData->ranging_meas.range_meas_twr[0].distance != 0xFFFF))) {
            phOsalUwb_ProduceSemaphore(rangingDataSem);
            printRangingData(pRangingData);
            static phLibUwb_Message_t RangingData_Info = {0};
            RangingData_Info.eMsgType                  = 0xAA;
            RangingData_Info.Size                      = 0x01;
            RangingData_Info.pMsgData                  = (void *)pData;

            (void)phOsalUwb_msgsnd(ApduMngQueue, &RangingData_Info, 0);
            break;
        }
#endif //UWBFTR_TWR
#endif // UWBIOT_UWBD_SR1XXT
        printRangingData(pRangingData);
        break;
    }
    case UWBD_TEST_MODE_LOOP_BACK_NTF: {
#if UWBIOT_UWBD_SR040
        Log("Group Delay  : %d\n", *(uint32_t *)pData);
        phOsalUwb_ProduceSemaphore(testLoopBackNtfSem);
#else
        // Handling for SR1xx/SR2xx
        phRfTestData_t *rftestdata             = (phRfTestData_t *)pData;
        phTest_Loopback_Ntf_t loopbackTestData = {0};
        // TODO: Handling to be updated for HPRF case 4K.
        uint8_t psdu[MAX_PSDU_DATA_SIZE] = {0};
        loopbackTestData.status          = rftestdata->status;
        deserializeDataFromLoopbackNtf(&loopbackTestData, rftestdata->data, psdu);
        phOsalUwb_ProduceSemaphore(testLoopBackNtfSem);
        printLoopbackRecvData(&loopbackTestData);
#endif // UWBIOT_UWBD_SR040
        break;
    }
    case UWBD_RFRAME_DATA:
    case UWBD_SCHEDULER_STATUS_NTF: {
        // Nothing to be done
    } break;
#if UWBIOT_UWBD_SR1XXT
    case UWBD_PER_SEND: {
        Log("pPerTxData->status  : %d\n", ((phPerTxData_t *)pData)->status);
        phOsalUwb_ProduceSemaphore(perSem);
    } break;
#endif // UWBIOT_UWBD_SR1XXT
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    case UWBD_PER_RCV: {
        phRfTestData_t *rftestdata      = (phRfTestData_t *)pData;
        phTestPer_Rx_Ntf_t testrecvdata = {0};
        testrecvdata.status             = rftestdata->status;
        deserializeDataFromRxPerNtf(&testrecvdata, rftestdata->data);
        phOsalUwb_ProduceSemaphore(perSem);
        printPerRecvData(&testrecvdata);
    } break;
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
#if UWBIOT_UWBD_SR1XXT_SR2XXT && UWBFTR_UWBS_DEBUG_Dump
    case UWBD_CIR_DATA_NTF:
    case UWBD_DATA_LOGGER_NTF:
    case UWBD_RANGING_TIMESTAMP_NTF:
    case UWBD_PSDU_DATA_NTF: {
        phDebugData_t *pDebugData = (phDebugData_t *)pData;
        if (opType == UWBD_CIR_DATA_NTF) {
            PRINTF("CIR data length :%d\n", pDebugData->dataLength);
            LOG_MAU8_D("CIR: ", pDebugData->data, ((pDebugData->dataLength >= 256) ? (256) : (pDebugData->dataLength)));
        }
        else if (opType == UWBD_PSDU_DATA_NTF) {
            PRINTF("PSDU data length :%d\n", pDebugData->dataLength);
        }
        else if (opType == UWBD_RANGING_TIMESTAMP_NTF) {
            PRINTF("Ranging Timestamp data length :%d\n", pDebugData->dataLength);
        }
        else {
            /* do nothing */
        }
    } break;
#endif // UWBFTR_UWBS_DEBUG_Dump && UWBIOT_UWBD_SR1XXT_SR2XXT
    case UWBD_TEST_RX_RCV: {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        phRfTestData_t *rftestdata   = (phRfTestData_t *)pData;
        phTest_Rx_Ntf_t testrecvdata = {0};
        // TODO: Handling to be updated for HPRF case 4K.
        uint8_t psdu[MAX_PSDU_DATA_SIZE] = {0};
        testrecvdata.status              = rftestdata->status;
        deserializeDataFromRxNtf(&testrecvdata, rftestdata->data, psdu);
        // TODO: Incase of HPRF this can go for a toss as the size of dataReceived is 2K.
        if ((testrecvdata.psdu_len != 0) && (testrecvdata.psdu_len <= sizeof(dataReceived))) {
            phOsalUwb_MemCopy(dataReceived, testrecvdata.pPsdu, testrecvdata.psdu_len);
            GENERATE_SEND_DATA(dataToSend, testrecvdata.psdu_len)
        }
        else {
            LOG_E("%s : Exceeding dataReceived buffer bounds", __FUNCTION__);
        }
        phOsalUwb_ProduceSemaphore(perSem);
        printrxRecvData(&testrecvdata);
#endif // UWBIOT_UWBD_SR1XXT
    } break;
    case UWBD_GENERIC_ERROR_NTF: {
        printGenericErrorStatus((phGenericError_t *)pData);
    } break;

    case UWBD_DEVICE_RESET:  // Error Recovery: cleanup all states and end all states.
        cleanUpAppContext(); // This would have called while 1. SeComError 2. other reasons
        break;

    case UWBD_RECOVERY_NTF: {
        // Error Recovery: do uwbd cleanup, fw download, move to ready state
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        phFwCrashLogInfo_t LogInfo;
        LogInfo.logLen = 255;
        LogInfo.pLog   = (uint8_t *)phOsalUwb_GetMemory((uint32_t)LogInfo.logLen * sizeof(uint8_t));
        UwbApi_GetFwCrashLog(&LogInfo);
        LOG_MAU8_I("Crash Log: ", LogInfo.pLog, LogInfo.logLen);
        phOsalUwb_FreeMemory(LogInfo.pLog);
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
        Handle_ErrorScenario(FW_CRASH);
    } break;

    case UWBD_SESSION_DATA: {
        phUwbSessionInfo_t *pSessionInfo = (phUwbSessionInfo_t *)pData;
        printSessionStatusData(pSessionInfo);
        phOsalUwb_ProduceSemaphore(inBandterminationSem);
    } break;
    case UWBD_ACTION_APP_CLEANUP: {
        Handle_ErrorScenario(APP_CLEANUP);
    } break;
#if UWBFTR_DataTransfer
    case UWBD_DATA_TRANSMIT_NTF: {
        phUwbDataTransmit_t *pTransmitNtfContext = (phUwbDataTransmit_t *)pData;
        printTransmitStatus(pTransmitNtfContext);
        phOsalUwb_ProduceSemaphore(datatransferNtfSemTx);
    } break;
    case UWBD_DATA_RCV_NTF: {
#if !(UWBIOT_UWBD_SR040)
        phUwbRcvDataPkt_t *pRcvDataPkt = (phUwbRcvDataPkt_t *)pData;
        printRcvDataStatus(pRcvDataPkt);
#endif
        static phLibUwb_Message_t shareable_config_buf = {0};
        shareable_config_buf.eMsgType                  = 0xBB;
        shareable_config_buf.Size                      = 0x01;
        shareable_config_buf.pMsgData                  = (void *)pData;

        (void)phOsalUwb_msgsnd(ApduMngQueue, &shareable_config_buf, 0);
        (void)phOsalUwb_ProduceSemaphore(dataRcvNtfSemTx);
#if !(UWBIOT_UWBD_SR040)
        if ((pRcvDataPkt->data_size != 0) && (pRcvDataPkt->data_size <= sizeof(dataReceived))) {
            phOsalUwb_MemCopy(dataReceived, pRcvDataPkt->data, pRcvDataPkt->data_size);
            GENERATE_SEND_DATA(dataToSend, pRcvDataPkt->data_size)
        }
        else {
            LOG_E("%s : Exceeding dataReceived buffer bounds", __FUNCTION__);
        }
#endif

    } break;
#endif //UWBFTR_DataTransfer
#if UWBFTR_Radar
    case UWBD_RADAR_RCV_NTF: {
        /* Application/Demo needs to allcoate the memory for CIR Data*/
        // TODO: to be fixed, as demos with limited Stack size can fail.
        static uint8_t RadarNtfBuff[MAX_RADAR_LEN];
        phUwbRadarNtf_t *pRcvRadaNtfPkt = (phUwbRadarNtf_t *)pData;
        /*This buffer later used for Pd/Bd Algo*/
        phOsalUwb_MemCopy(
            &RadarNtfBuff, pRcvRadaNtfPkt->radar_ntf.radr_cir.cirdata, pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_len);
        printRadarRecvNtf(pRcvRadaNtfPkt);
#if defined(_MSC_VER)
#if (PRESENCE_DETECTION && BREATHING_DETECTION)
#error "Both should not enable at same time"
#elif (PRESENCE_DETECTION)
        sendPresenceDetectionData(RadarNtfBuff, pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_len);
#elif (BREATHING_DETECTION)
        sendBreathingDetectionData(RadarNtfBuff, pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_len);
#endif //PRESNECE_DETECTION
#endif //_MSC_VER
    } break;
    case UWBD_TEST_RADAR_ISO_NTF: {
        phUwbRadarNtf_t *pRcvRadaNtfPkt = (phUwbRadarNtf_t *)pData;
        printRadarTestIsoNtf(pRcvRadaNtfPkt);
        (void)phOsalUwb_ProduceSemaphore(RadarTstAntIsoNtfSemTx);
    } break;
#endif //UWBFTR_Radar
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
    case UWBD_WIFI_COEX_IND_NTF: {
        UWB_CoEx_IndNtf_t *wifiCoExIndNtf = (UWB_CoEx_IndNtf_t *)pData;
        printWiFiCoExIndNtf(wifiCoExIndNtf);
    } break;
    case UWBD_MAX_ACTIVE_GRANT_DURATION_EXCEEDED_WAR_NTF: {
        LOG_D("Max Active Grant Duration Status : %X", *(uint8_t *)pData);
    } break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

#if UWBFTR_CCC
    case UWBD_RANGING_ALIRO_DATA: {
        phAliroRangingData_t *AliroRangingData = (phAliroRangingData_t *)pData;
        printAliroRangingData(AliroRangingData);
    } break;
#endif // UWBFTR_CCC

#if !(UWBIOT_UWBD_SR040)
    case UWBD_DATA_TRANSFER_PHASE_CONFIG_NTF: {
        phDataTxPhaseCfgNtf_t *pDataTxPhCfgNtf = (phDataTxPhaseCfgNtf_t *)pData;
        printDataTxPhaseCfgNtf(pDataTxPhCfgNtf);
    } break;
#endif // !(UWBIOT_UWBD_SR040)

    default:
        LOG_W("%s : Unregistered Event : 0x%X ", __FUNCTION__, opType);
        break;
    }
    demo_common_toggle_led_onAppCallback();
}
#endif // #if !defined(UWBIOT_APP_BUILD__DEMO_CDC)
