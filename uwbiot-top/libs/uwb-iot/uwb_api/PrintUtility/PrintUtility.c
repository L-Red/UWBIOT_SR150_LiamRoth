/*
 *
 * Copyright 2018-2020,2023,2024 NXP.
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

#include "PrintUtility.h"
#include "uwb_types.h"
#include "phNxpLogApis_App.h"
#include "phNxpLogApis_App.h"

EXTERNC void printGenericErrorStatus(const phGenericError_t *pGenericError)
{
    if (pGenericError != NULL) {
        NXPLOG_APP_I("Status                        : %hu", pGenericError->status);
    }
}

EXTERNC void printSessionStatusData(const phUwbSessionInfo_t *pSessionInfo)
{
    if (pSessionInfo != NULL) {
        NXPLOG_APP_I("pSessionInfo->sessionHandle          : %" PRIu32 "", pSessionInfo->sessionHandle);
        NXPLOG_APP_I("pSessionInfo->state               : %hu", pSessionInfo->state);
        NXPLOG_APP_I("pSessionInfo->reason_code         : %hu", pSessionInfo->reason_code);
    }
    else {
        NXPLOG_APP_E("pSessionInfo is NULL");
    }
}

EXTERNC void printUwbSessionData(const phUwbSessionsContext_t *pUwbSessionsContext)
{
    if (pUwbSessionsContext != NULL) {
        NXPLOG_APP_I("Status                        : %" PRIu32 " ", pUwbSessionsContext->status);
        NXPLOG_APP_I("Session Counter               : %d ", pUwbSessionsContext->sessioncnt);
        for (uint8_t i = 0; i < pUwbSessionsContext->sessioncnt; i++) {
            NXPLOG_APP_I(
                "Session %d ID             : %" PRIu32 " ", i, pUwbSessionsContext->pUwbSessionData[i].sessionHandle);
            NXPLOG_APP_I("Session %d Type           : %hu", i, pUwbSessionsContext->pUwbSessionData[i].session_type);
            NXPLOG_APP_I("Session %d State          : %hu", i, pUwbSessionsContext->pUwbSessionData[i].session_state);
        }
    }
    else {
        NXPLOG_APP_E("pUwbSessionsContext is NULL");
    }
}

EXTERNC void printMulticastListStatus(const phMulticastControleeListNtfContext_t *pControleeNtfContext)
{
    if (pControleeNtfContext != NULL) {
        NXPLOG_APP_I(
            "pControleeNtfContext->sessionHandle          : %" PRIu32 " ", pControleeNtfContext->sessionHandle);
        NXPLOG_APP_I("pControleeNtfContext->remaining_list      : %hu", pControleeNtfContext->remaining_list);
        NXPLOG_APP_I("pControleeNtfContext->no_of_controlees    : %hu", pControleeNtfContext->no_of_controlees);

        for (uint8_t i = 0; i < pControleeNtfContext->no_of_controlees; i++) {
#if UWBIOT_UWBD_SR1XXT
            NXPLOG_APP_I("pControleeNtfContext->controleeStatusList[%hu].controlee_mac_address  : %" PRIu16 "",
                i,
                pControleeNtfContext->controleeStatusList[i].controlee_mac_address);
#endif
            NXPLOG_APP_I("pControleeNtfContext->controleeStatusList[%hu].subsession_id          : %" PRIu32 "",
                i,
                pControleeNtfContext->controleeStatusList[i].subsession_id);
            NXPLOG_APP_I("pControleeNtfContext->controleeStatusList[%hu].status                 : %hu",
                i,
                pControleeNtfContext->controleeStatusList[i].status);
        }
    }
    else {
        LOG_E("phControleeNtfContext_t is NULL");
    }
}

EXTERNC void printRangingParams(const phRangingParams_t *pRangingParams)
{
    if (pRangingParams != NULL) {
        NXPLOG_APP_D("pRangingParams->deviceType                    : %hu", pRangingParams->deviceType);
        NXPLOG_APP_D("pRangingParams->deviceRole                    : %hu", pRangingParams->deviceRole);
        NXPLOG_APP_D("pRangingParams->multiNodeMode                 : %hu", pRangingParams->multiNodeMode);
        NXPLOG_APP_D("pRangingParams->macAddrMode                   : %hu", pRangingParams->macAddrMode);
        NXPLOG_APP_D("pRangingParams->scheduledMode                 : %hu", pRangingParams->scheduledMode);
        NXPLOG_APP_D("pRangingParams->rangingRoundUsage             : %hu", pRangingParams->rangingRoundUsage);

#if (APP_LOG_LEVEL > UWB_LOG_INFO_LEVEL)
        uint8_t addrLen = MAC_SHORT_ADD_LEN;
        if (pRangingParams->macAddrMode != SHORT_MAC_ADDRESS) { // mac addr is of 2 or 8 bytes.
            addrLen = MAC_EXT_ADD_LEN;
        }
#endif
        LOG_MAU8_D("pRangingParams->deviceMacAddr                 : ", pRangingParams->deviceMacAddr, addrLen);
    }
    else {
        NXPLOG_APP_E("pRangingParams is NULL");
    }
}
/* clang-format off */
EXTERNC void printRangingData(const phRangingData_t *pRangingData)
{
    if (pRangingData != NULL) {
        NXPLOG_APP_D("--------------Received Range Data--------------");
        NXPLOG_APP_D("pRangingData->seq_ctr                             : %" PRIu32 " ", pRangingData->seq_ctr);
        NXPLOG_APP_D("pRangingData->sessionHandle                       : 0x%08X ", pRangingData->sessionHandle);
        NXPLOG_APP_D("pRangingData->rcr_indication                      : %hu", pRangingData->rcr_indication);
        NXPLOG_APP_D("pRangingData->curr_range_interval                 : %" PRIu32 " ", pRangingData->curr_range_interval);
        NXPLOG_APP_D("pRangingData->ranging_measure_type                : %hu", pRangingData->ranging_measure_type);
        NXPLOG_APP_D("pRangingData->mac_addr_mode_indicator             : %hu", pRangingData->mac_addr_mode_indicator);
        NXPLOG_APP_D("pRangingData->sessionHandle_of_primary_session    : %x", pRangingData->sessionHandle_of_primary_session);
        NXPLOG_APP_D("pRangingData->no_of_measurements                  : %hu", pRangingData->no_of_measurements);
#if UWBIOT_UWBD_SR040
        NXPLOG_APP_D("pRangingData->antenna_info                        : %hu", pRangingData->antenna_info);
#endif //UWBIOT_UWBD_SR040

#if UWBFTR_TWR // support only for DSTWR
        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_TWOWAY) {
            for (uint8_t i = 0; i < pRangingData->no_of_measurements; i++) {
                LOG_MAU8_D("pRangingData->range_meas.mac_addr        :", pRangingData->ranging_meas.range_meas_twr[i].mac_addr,(pRangingData->mac_addr_mode_indicator * 6) + 2);
                NXPLOG_APP_D("TWR[%d].status          : %x ", i, pRangingData->ranging_meas.range_meas_twr[i].status);
                if ((pRangingData->ranging_meas.range_meas_twr[i].status == UWBAPI_STATUS_OK) || (pRangingData->ranging_meas.range_meas_twr[i].status == UWBAPI_STATUS_OK_NEGATIVE_DISTANCE_REPORT)) {
                    NXPLOG_APP_I("TWR[%d].nLos            : %hu", i, pRangingData->ranging_meas.range_meas_twr[i].nLos);

                    /* This is a good thing to report... so keep it under INFO Tag. */
                    NXPLOG_APP_I("TWR[%" PRIu16 "].distance        : %" PRIu16 "", i, pRangingData->ranging_meas.range_meas_twr[i].distance);
                    if (pRangingData->ranging_meas.range_meas_twr[i].status == UWBAPI_STATUS_OK_NEGATIVE_DISTANCE_REPORT) {
                        NXPLOG_APP_I("TWR[%" PRIu16 "]  Negative Distance Reported", i);
                    }

                    NXPLOG_APP_I("TWR[%d].aoa_azimuth: %d.%d", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_twr[i].aoa_azimuth));
                    NXPLOG_APP_D("TWR[%d].aoa_azimuth_FOM       : %d", i, pRangingData->ranging_meas.range_meas_twr[i].aoa_azimuth_FOM);
                    NXPLOG_APP_I("TWR[%d].aoa_elevation       : %d.%d", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_twr[i].aoa_elevation));
                    NXPLOG_APP_D("TWR[%d].aoa_elevation_FOM      : %d", i, pRangingData->ranging_meas.range_meas_twr[i].aoa_elevation_FOM);
                    NXPLOG_APP_D("TWR[%d].aoa_dest_azimuth  : %d.%d", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_twr[i].aoa_dest_azimuth));
                    NXPLOG_APP_D("TWR[%d].aoa_dest_azimuth_FOM      : %d", i, pRangingData->ranging_meas.range_meas_twr[i].aoa_dest_azimuth_FOM);
                    NXPLOG_APP_D("TWR[%d].aoa_dest_elevation : %d.%d", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_twr[i].aoa_dest_elevation));
                    NXPLOG_APP_D("TWR[%d].aoa_dest_elevation_FOM   : %d", i, pRangingData->ranging_meas.range_meas_twr[i].aoa_dest_elevation_FOM);
                    NXPLOG_APP_D("TWR[%d].slot_index      : %d", i, pRangingData->ranging_meas.range_meas_twr[i].slot_index);
                    NXPLOG_APP_D("TWR[%d].rssi      : %d", i, pRangingData->ranging_meas.range_meas_twr[i].rssi);
                }
            }
#if UWBIOT_UWBD_SR1XXT_SR2XXT
            if (pRangingData->vs_length != FALSE) {
                NXPLOG_APP_D("TWR.vs_length                             : %d ", pRangingData->vs_length);
                NXPLOG_APP_D("TWR.vs_data_type                          : %d ", pRangingData->vs_data_type);
                NXPLOG_APP_D("TWR.wifiCoExStatus                        : %d", pRangingData->vs_data.twr.wifiCoExStatus);
                NXPLOG_APP_D("TWR.rxMode                                : %d", pRangingData->vs_data.twr.rxInfoMesr_twr.rxMode);
                NXPLOG_APP_D("TWR.num_of_rx_antennaRxInfo               : %d", pRangingData->vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo);
                NXPLOG_APP_D("TWR.rxModeDebugNtf                        : %d",pRangingData->vs_data.twr.rxInfoDebugNtf_twr.rxModeDebugNtf);
                NXPLOG_APP_D("TWR.num_of_rx_antennaDebugNtf             : %d",pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf);
                if (pRangingData->vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo != FALSE) {
                    for (int j = 0; j < pRangingData->vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo; j++) {
                        NXPLOG_APP_D("TWR.rx_antennaIdRxInfo                    : %d", pRangingData->vs_data.twr.rxInfoMesr_twr.rx_antennaIdRxInfo[j]);
                    }
                }
                if (pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf != FALSE) {
                    for (int k = 0; k < pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf; k++) {
                        NXPLOG_APP_D("TWR.rx_antennaIdDebugNtf[%d]              : %d", k, pRangingData->vs_data.twr.rxInfoDebugNtf_twr.rx_antennaIdDebugNtf[k]);
                    }
                }
                for (uint8_t i = 0; i < pRangingData->no_of_measurements; i++) {
                    LOG_MAU8_D("Vendor Specific info for responder.mac_addr        :", pRangingData->ranging_meas.range_meas_twr[i].mac_addr,(pRangingData->mac_addr_mode_indicator * 6) + 2);
                    if (pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf != FALSE) {
                        for (int j = 0; j < pRangingData->vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo; j++) {
                            NXPLOG_APP_D("TWR[%d].angleOfArrival[%d]                : %d.%d", i, j, TO_Q_9_7(pRangingData->vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].angleOfArrival));
                            NXPLOG_APP_D("TWR[%d].pdoa[%d]                          :%d.%d", i, j, TO_Q_9_7(pRangingData->vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].pdoa));
                            NXPLOG_APP_D("TWR[%d].pdoaIndex[%d]                     : %d",i, j, pRangingData->vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].pdoaIndex);
#if UWBIOT_UWBD_SR150
                            if(pRangingData->vs_data_type == FOV_SPECIFIC_DATA_TYPE)
                            {
                                NXPLOG_APP_D("TWR[%d].aoaFovFlag[%d]                    : %d ",i, j, pRangingData->vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].aoaFovFlag);
                            }
#endif // UWBIOT_UWBD_SR150
                        }
                    }
                    if (pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf != FALSE) {
                        for (int k = 0; k < pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf; k++) {
                            NXPLOG_APP_D("TWR[%d].rxSnrFirstPath[%d]                : %d", i, k, pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rxSnrFirstPath);
                            NXPLOG_APP_D("TWR[%d].rxSnrMainPath[%d]                 : %d", i, k, pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rxSnrMainPath);
                            NXPLOG_APP_D("TWR[%d].rx_FirstPathIndex[%d]             : %d.%d", i, k, TO_Q_6_10(pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rx_FirstPathIndex));
                            NXPLOG_APP_D("TWR[%d].rx_MainPathIndex[%d]              : %d.%d", i, k, TO_Q_6_10(pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rx_MainPathIndex));
                        }
                    }
                    if ((pRangingData->vs_data.twr.rxInfoMesr_twr.rxMode == kUWBAntCfgRxMode_ToA_Rfm_Mode) || (pRangingData->vs_data.twr.rxInfoMesr_twr.rxMode == kUWBAntCfgRxMode_AoA_Rfm_Mode)) {
                        NXPLOG_APP_D("TWR[%d].distance_2                        : %d", i, pRangingData->vs_data.twr.vsMesr[i].distance_2);
                    }
                }
            }
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
        }
#endif //UWBFTR_TWR

#if UWBFTR_UL_TDoA_Anchor
        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_ONEWAY) {
            for (int i = 0; i < pRangingData->no_of_measurements; i++) {
                LOG_MAU8_D("pRangingData->range_meas.mac_addr       :",
                    pRangingData->ranging_meas.range_meas_tdoa[i].mac_addr,
                    (pRangingData->mac_addr_mode_indicator * 6) + 2);
                NXPLOG_APP_D("TDoA[%d].status                       : %d", i, pRangingData->ranging_meas.range_meas_tdoa[i].status);
                NXPLOG_APP_D("TDoA[%d].message_control              : %d",
                    i,
                    pRangingData->ranging_meas.range_meas_tdoa[i].message_control);
                NXPLOG_APP_D("TDoA[%d].frame_type                   : %d", i, pRangingData->ranging_meas.range_meas_tdoa[i].frame_type);
                NXPLOG_APP_D("TDoA[%d].nLos                         : %d", i, pRangingData->ranging_meas.range_meas_tdoa[i].nLos);
                NXPLOG_APP_D("TDoA[%" PRIi16 "].aoa_azimuth         : %d.%d",
                    i,
                    TO_Q_9_7(pRangingData->ranging_meas.range_meas_tdoa[i].aoa_azimuth));
                NXPLOG_APP_D("TDoA[%d].aoa_azimuth_FOM              : %d",
                    i,
                    pRangingData->ranging_meas.range_meas_tdoa[i].aoa_azimuth_FOM);
                NXPLOG_APP_D("TDoA[%" PRIi16 "].aoa_elevation       : %d.%d",
                    i,
                    TO_Q_9_7(pRangingData->ranging_meas.range_meas_tdoa[i].aoa_elevation));
                NXPLOG_APP_D("TDoA[%d].aoa_elevation_FOM            : %d",
                    i,
                    pRangingData->ranging_meas.range_meas_tdoa[i].aoa_elevation_FOM);
                NXPLOG_APP_D("TDoA[%d].frame_number                 : %" PRIu32 " ",
                    i,
                    pRangingData->ranging_meas.range_meas_tdoa[i].frame_number);
                LOG_MAU8_D("TDoA.rx_timestamp                       :",
                    pRangingData->ranging_meas.range_meas_tdoa[i].rx_timestamp, ULTDOA_64BIT_IN_BYTES);
                LOG_MAU8_D("TDoA.ul_tdoa_device_id                  :",
                    pRangingData->ranging_meas.range_meas_tdoa[i].ul_tdoa_device_id, ULTDOA_64BIT_IN_BYTES);
                LOG_MAU8_D("TDoA.tx_timestamp                       :",
                    pRangingData->ranging_meas.range_meas_tdoa[i].tx_timestamp, ULTDOA_64BIT_IN_BYTES);
            }
#if UWBIOT_UWBD_SR1XXT
            if (pRangingData->vs_length != FALSE) {
                NXPLOG_APP_D("TDoA.vs_length                    : %d \n", pRangingData->vs_length);
                NXPLOG_APP_D("TDoA.vendorExtLength              : %d ", pRangingData->vs_data.tdoa.vendorExtLength);
                NXPLOG_APP_D("TDoA.rssi_rx1                     : %d.%d", TO_Q_8_8(pRangingData->vs_data.tdoa.rssi_rx1));
                NXPLOG_APP_D("TDoA.rssi_rx2                     : %d.%d", TO_Q_8_8(pRangingData->vs_data.tdoa.rssi_rx2));
                NXPLOG_APP_D("TDoA.noOfPdoaMeasures             : %d", pRangingData->vs_data.tdoa.noOfPdoaMeasures);
                /** Repitition part (This fields are repeated for each Antenna Pair)*/
                if (pRangingData->vs_data.tdoa.noOfPdoaMeasures != FALSE) {
                    for (uint8_t j = 0; j < pRangingData->vs_data.tdoa.noOfPdoaMeasures; j++) {
                        NXPLOG_APP_D("TDoA.pdoaFirst[%d]                 : %d.%d",j , TO_Q_9_7(pRangingData->vs_data.tdoa.pdoa[j]));
                        NXPLOG_APP_D("TDoA.pdoaFirstIndex[%d]            : %d",j , pRangingData->vs_data.tdoa.pdoaIndex[j]);
                    }
                    PRINTF("\n");
                }
                NXPLOG_APP_D("pRangingData->antenna_pairInfo    : 0x%X ", pRangingData->antenna_pairInfo);
                NXPLOG_APP_D("pRangingData->wifiCoExStatusCode  : %d", pRangingData->wifiCoExStatusCode);
            }
#endif //UWBIOT_UWBD_SR1XXT
        }

#endif //UWBFTR_UL_TDoA_Anchor
#if (UWBIOT_UWBD_SR1XXT && UWBFTR_DL_TDoA_Tag)
        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_DLTDOA) {
            for (uint8_t i = 0; i < pRangingData->no_of_measurements; i++) {
                if (pRangingData->ranging_meas.range_meas_dltdoa[i].status == UCI_STATUS_OK) {
                    LOG_MAU8_D("pRangingData->range_meas.mac_addr        :",
                        pRangingData->ranging_meas.range_meas_dltdoa[i].mac_addr,
                        MAC_EXT_ADD_LEN);

                    NXPLOG_APP_D("DLTDoA[%d].status              : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].status);

                    NXPLOG_APP_D("DLTDoA[%d].message_type              : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].message_type);

                    NXPLOG_APP_D("DLTDoA[%d].message_control              : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].message_control);

                    NXPLOG_APP_D("DLTDoA[%d].block_index              : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].block_index);

                    NXPLOG_APP_D("DLTDoA[%d].round_index              : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].round_index);

                    NXPLOG_APP_D(
                        "DLTDoA[%d].nLoS              : %x\n", i, pRangingData->ranging_meas.range_meas_dltdoa[i].nLoS);

                    NXPLOG_APP_D("DLTDoA[%d].aoa_azimuth             : %d.%d\n",
                        i,
                        TO_Q_9_7(pRangingData->ranging_meas.range_meas_dltdoa[i].aoa_azimuth));

                    NXPLOG_APP_D("DLTDoA[%d].aoa_azimuth_fom             : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].aoa_azimuth_fom);

                    NXPLOG_APP_D("DLTDoA[%d].aoa_elevation            : %d.%d\n",
                        i,
                        TO_Q_9_7(pRangingData->ranging_meas.range_meas_dltdoa[i].aoa_elevation));

                    NXPLOG_APP_D("DLTDoA[%d].aoa_elevation_fom            : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].aoa_elevation_fom);

                    NXPLOG_APP_D(
                        "DLTDoA[%d].rssi              : %d.%d\n", i, TO_Q_7_1(pRangingData->ranging_meas.range_meas_dltdoa[i].rssi));

                    LOG_MAU8_D("pRangingData->range_meas.tx_timestamp        :",
                        pRangingData->ranging_meas.range_meas_dltdoa[i].tx_timestamp,
                        MAX_RX_TX_TIMESTAMP);

                    LOG_MAU8_D("pRangingData->range_meas.rx_timestamp        :",
                        pRangingData->ranging_meas.range_meas_dltdoa[i].rx_timestamp,
                        MAX_RX_TX_TIMESTAMP);

                    NXPLOG_APP_D("DLTDoA[%d].cfo_anchor              : %d.%d\n",
                        i,
                        TO_Q_6_10(pRangingData->ranging_meas.range_meas_dltdoa[i].cfo_anchor));

                    NXPLOG_APP_D("DLTDoA[%d].cfo              : %d.%d\n",
                        i,
                        TO_Q_6_10(pRangingData->ranging_meas.range_meas_dltdoa[i].cfo));

                    NXPLOG_APP_D("DLTDoA[%d].reply_time_initiator              : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].reply_time_initiator);

                    NXPLOG_APP_D("DLTDoA[%d].reply_time_responder              : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].reply_time_responder);

                    NXPLOG_APP_D("DLTDoA[%d].initiator_responder_tof              : %x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].initiator_responder_tof);
                }
            }
            NXPLOG_APP_D("pRangingData->wifiCoExStatusCode                    : %d ", pRangingData->wifiCoExStatusCode);
            NXPLOG_APP_D("pRangingData->antenna_pairInfo                    : %" PRIu32 " ", pRangingData->antenna_pairInfo);
        }
#endif //UWBFTR_DL_TDoA_Tag
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
        /**
         * Below fields are applicable for Observer side .
        */
        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_OWR_WITH_AOA) {
            for (int i = 0; i < pRangingData->no_of_measurements; i++) {
                NXPLOG_APP_D("OWR[%d].mac_addr                          :", i);
                for (uint8_t l = 0; l < (pRangingData->mac_addr_mode_indicator * 6) + 2;
                     l++) { // mac addr is of 2 or 8 bytes.
                    NXPLOG_APP_D("%x",
                        pRangingData->ranging_meas.range_meas_owr_aoa[i].mac_addr[l]); // mac_addr_mode_indicator:0 or 1
                }
                NXPLOG_APP_D("\n");

                NXPLOG_APP_D("OWR[%d].status                            : %x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].status);
                NXPLOG_APP_D("OWR[%d].nLos                              : %x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].nLos);
                NXPLOG_APP_D("OWR[%d].frame_seq_num                     : %x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].frame_seq_num);
                NXPLOG_APP_D("OWR[%d].block_index                       : %d\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].block_index);
                NXPLOG_APP_D("OWR[%d].aoa_azimuth                       : %d.%d\n", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_owr_aoa[i].aoa_azimuth));
                NXPLOG_APP_D("OWR[%d].aoa_azimuth_FOM                   : %x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].aoa_azimuth_FOM);
                NXPLOG_APP_D("OWR[%d].aoa_elevation                     : %d.%d\n", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_owr_aoa[i].aoa_elevation));
                NXPLOG_APP_D("OWR[%d].aoa_elevation_FOM                 : %x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].aoa_elevation_FOM);
            }
            if (pRangingData->vs_length != FALSE) {
                NXPLOG_APP_D("OWR.vs_length                             : %d \n", pRangingData->vs_length);
                NXPLOG_APP_D("OWR.vs_data_type                          : %d \n", pRangingData->vs_data_type);
                NXPLOG_APP_D("OWR.rxMode                                : %d \n", pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.rxMode);
                NXPLOG_APP_D("OWR.num_of_rx_antennaRxInfo               : %d \n", pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo);
                for (int i = 0; i < pRangingData->no_of_measurements; i++) {
                    if (pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo != FALSE) {
                        for (int m = 0; m < pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo; m++) {
                            NXPLOG_APP_D("OWR[%d].rxantennaIdRxInfo[%d]             : %d \n", i, m, pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.rx_antennaIdRxInfo[m]);
                            NXPLOG_APP_D("OWR[%d].angleOfArrival[%d]                : %d.%d \n", i, m, TO_Q_9_7(pRangingData->vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[m].angleOfArrival));
                            NXPLOG_APP_D("OWR[%d].pdoa[%d]                          : %d.%d \n", i, m, TO_Q_9_7(pRangingData->vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[m].pdoa));
                            NXPLOG_APP_D("OWR[%d].pdoaIndex[%d]                     : %d \n", i, m, pRangingData->vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[m].pdoaIndex);
#if UWBIOT_UWBD_SR150
                            if(pRangingData->vs_data_type == FOV_SPECIFIC_DATA_TYPE)
                            {
                                NXPLOG_APP_D("OWR[%d].aoaFovFlag[%d]                    : %d ", i, m, pRangingData->vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[m].aoaFovFlag);
                            }
#endif // UWBIOT_UWBD_SR150
                        }
                    }
                    NXPLOG_APP_D("OWR[%d].rssi                              : %d.%d \n", i, TO_Q_8_8(pRangingData->vs_data.owr_aoa.vsMesr[i].rssi));
                }
            }
        }
#endif //(UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    NXPLOG_APP_D("pRangingData->authInfoPrsen      : %d\n ", pRangingData->authInfoPrsen);
        if (pRangingData->authInfoPrsen != FALSE) {
                    LOG_MAU8_D("pRangingData->authenticationTag   : ", pRangingData->authenticationTag, AUTH_TAG_IN_16BYTES);
        }
#endif //UWBIOT_UWBD_SR1XXT_SR2XXT
    }
    else {
        NXPLOG_APP_D("pRangingData is NULL");
    }
}
/* clang-format on */

#if UWBFTR_DataTransfer

EXTERNC void printTransmitStatus(const phUwbDataTransmit_t *pTransmitNtfContext)
{
    if (pTransmitNtfContext != NULL) {
        NXPLOG_APP_I("pTransmitNtfContext->transmitNtf_sessionHandle       : 0x%x\n",
            pTransmitNtfContext->transmitNtf_sessionHandle);
        NXPLOG_APP_I("pTransmitNtfContext->transmitNtf_sequence_number     : %d\n",
            pTransmitNtfContext->transmitNtf_sequence_number);
        NXPLOG_APP_I(
            "pTransmitNtfContext->transmitNtf_status              : %d\n", pTransmitNtfContext->transmitNtf_status);
        NXPLOG_APP_I(
            "pTransmitNtfContext->transmitNtf_txcount             : %d\n", pTransmitNtfContext->transmitNtf_txcount);
    }
    else {
        NXPLOG_APP_E("pTransmitNtfContext is NULL");
    }
}

EXTERNC void printRcvDataStatus(const phUwbRcvDataPkt_t *pRcvDataPkt)
{
    if (pRcvDataPkt != NULL) {
        NXPLOG_APP_I("pRcvDataPkt->sessionHandle            : 0x%x", pRcvDataPkt->sessionHandle);
        NXPLOG_APP_I("pRcvDataPkt->status                   : 0x%x", pRcvDataPkt->status);
        LOG_MAU8_D("SrcMacAddr                              : ", pRcvDataPkt->src_address, MAC_EXT_ADD_LEN);
        NXPLOG_APP_I("pRcvDataPkt->sequence_number          : %d", pRcvDataPkt->sequence_number);
        if (pRcvDataPkt->status == UWBAPI_STATUS_OK) {
            NXPLOG_APP_I("data_size                             : %d", pRcvDataPkt->data_size);
            LOG_MAU8_I("DataRcv                                 : ", pRcvDataPkt->data, pRcvDataPkt->data_size);
        }
    }
    else {
        NXPLOG_APP_E("pRcvDataPkt is NULL");
    }
}
#endif //UWBFTR_DataTransfer

#if UWBFTR_Radar

EXTERNC void printRadarRecvNtf(const phUwbRadarNtf_t *pRcvRadaNtfPkt)
{
    if (pRcvRadaNtfPkt != NULL) {
        NXPLOG_APP_D("pRcvRadaNtfPkt->sessionHandle          : %x\n", pRcvRadaNtfPkt->sessionHandle);
        NXPLOG_APP_D("pRcvRadaNtfPkt->radar_status          : %x\n", pRcvRadaNtfPkt->radar_status);
        NXPLOG_APP_D("pRcvRadaNtfPkt->radar_type          : %x\n", pRcvRadaNtfPkt->radar_type);
        NXPLOG_APP_D("pRcvRadaNtfPkt->num_cirs          : %x\n", pRcvRadaNtfPkt->radar_ntf.radr_cir.num_cirs);
        NXPLOG_APP_D("pRcvRadaNtfPkt->cir_taps          : %x\n", pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_taps);
        NXPLOG_APP_D("pRcvRadaNtfPkt->rfu          : %x\n", pRcvRadaNtfPkt->radar_ntf.radr_cir.rfu);
        NXPLOG_APP_D("pRcvRadaNtfPkt->len          : %x\n", pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_len);
        LOG_MAU8_D("pRcvRadaNtfPkt->CIRDATA",
            pRcvRadaNtfPkt->radar_ntf.radr_cir.cirdata,
            pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_len);
    }
    else {
        NXPLOG_APP_E("pRcvRadaNtfPkt is NULL");
    }
}

EXTERNC void printRadarTestIsoNtf(const phUwbRadarNtf_t *pRcvRadaTstNtfPkt)
{
    if (pRcvRadaTstNtfPkt != NULL) {
        NXPLOG_APP_I("sessionHandle          : %x\n", pRcvRadaTstNtfPkt->sessionHandle);
        NXPLOG_APP_I("radar_status        : %x\n", pRcvRadaTstNtfPkt->radar_status);
        NXPLOG_APP_I("radar_type          : %x\n", pRcvRadaTstNtfPkt->radar_type);
        NXPLOG_APP_I("antenna_tx          : %x\n", pRcvRadaTstNtfPkt->radar_ntf.radar_tst_ntf.antenna_tx);
        NXPLOG_APP_I("antenna_rx          : %x\n", pRcvRadaTstNtfPkt->radar_ntf.radar_tst_ntf.antenna_rx);
        NXPLOG_APP_I("anteena_isolation   : %x\n", pRcvRadaTstNtfPkt->radar_ntf.radar_tst_ntf.anteena_isolation);
    }
    else {
        NXPLOG_APP_E("pRcvRadaNtfPkt is NULL");
    }
}
#endif //UWBFTR_Radar

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
EXTERNC void printWiFiCoExIndNtf(const UWB_CoEx_IndNtf_t *wifiCoExIndNtf)
{
    if (wifiCoExIndNtf != NULL) {
        NXPLOG_APP_D("Status                : %x\n", wifiCoExIndNtf->status);
        NXPLOG_APP_D("slot_index            : %x\n", wifiCoExIndNtf->slot_index);
        NXPLOG_APP_D("sessionHandle             : %x\n", wifiCoExIndNtf->sessionHandle);
    }
}
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

#if UWBFTR_CCC
EXTERNC void printAliroRangingData(const phAliroRangingData_t *AliroRangingData)
{
    if (AliroRangingData != NULL) {
        NXPLOG_APP_D("------------------Received Aliro Range Data------------------");
        NXPLOG_APP_D("Aliro.sessionHandle                             : 0x%x ", AliroRangingData->sessionHandle);
        NXPLOG_APP_D("Aliro.rangingStatus                             : 0x%.2X", AliroRangingData->rangingStatus);
        NXPLOG_APP_D("Aliro.stsIndex                                  : %" PRIu32 "", AliroRangingData->stsIndex);
        NXPLOG_APP_D("Aliro.rangingRoundIndex                         : %" PRIu16 "", AliroRangingData->rangingRoundIndex);
        NXPLOG_APP_D("Aliro.blockIndex                                : %" PRIu16 "", AliroRangingData->blockIndex);
        NXPLOG_APP_I("Aliro.distance                                  : %" PRIu16 "", AliroRangingData->distance);
        NXPLOG_APP_D("Aliro.uncertanityAnchorFom                      : %hu", AliroRangingData->uncertanityAnchorFom);
        NXPLOG_APP_D("Aliro.uncertanityInitiatorFom                   : %hu", AliroRangingData->uncertanityInitiatorFom);
        LOG_MAU8_D("Aliro.ccmTag                                    ", AliroRangingData->ccmTag, MAX_CCM_TAG_SIZE);
        NXPLOG_APP_D("Aliro.aoa_azimuth                               : %d.%d", TO_Q_9_7(AliroRangingData->aoa_azimuth));
        NXPLOG_APP_D("Aliro.aoa_azimuth_fom                           : %d", AliroRangingData->aoa_azimuth_FOM);
        NXPLOG_APP_D("Aliro.aoa_elevation                             : %d.%d", TO_Q_9_7(AliroRangingData->aoa_elevation));
        NXPLOG_APP_D("Aliro.aoa_elevation_fom                         : %d", AliroRangingData->aoa_elevation_FOM);

        NXPLOG_APP_D(
            "Aliro.antenna_pair_info.configMode              : %d", AliroRangingData->antenna_pair_info.configMode);
        NXPLOG_APP_D(
            "Aliro.antenna_pair_info.antPairId1              : %d", AliroRangingData->antenna_pair_info.antPairId1);
        NXPLOG_APP_D(
            "Aliro.antenna_pair_info.antPairId2              : %d", AliroRangingData->antenna_pair_info.antPairId2);

        NXPLOG_APP_D("Aliro.noOfPdoaMeasures                          : %d", AliroRangingData->noOfPdoaMeasures);
        for (size_t i = 0; i < AliroRangingData->noOfPdoaMeasures; i++) {
            NXPLOG_APP_D("Aliro.pdoaMeasurements.pdoa                     : %d.%d",
                TO_Q_9_7(AliroRangingData->pdoaMeasurements[i].pdoa));
            NXPLOG_APP_D(
                "Aliro.pdoaMeasurements.pdoaIndex                : %d", AliroRangingData->pdoaMeasurements[i].pdoaIndex);
        }
        NXPLOG_APP_D("Aliro.noOfRssiMeasurements                      : %d", AliroRangingData->noOfRssiMeasurements);
        for (size_t i = 0; i < AliroRangingData->noOfRssiMeasurements; i++) {
            NXPLOG_APP_D("Aliro.cccRssiMeasurements.rssi_rx1                     : %d.%d",
                TO_Q_8_8(AliroRangingData->cccRssiMeasurements[i].rssi_rx1));

            NXPLOG_APP_D("Aliro.cccRssiMeasurements.rssi_rx2                     : %d.%d",
                TO_Q_8_8(AliroRangingData->cccRssiMeasurements[i].rssi_rx2));
        }
        NXPLOG_APP_D("Aliro.cccSnrMeasurements                      : %d", AliroRangingData->noOfSnrMeasurements);
        for (size_t i = 0; i < AliroRangingData->noOfSnrMeasurements; i++) {
            NXPLOG_APP_D("Aliro.cccRssiMeasurements.slotIndexAndAntennaMap    : %hu", AliroRangingData->cccSnrMeasurements[i].slotIndexAndAntennaMap);
            NXPLOG_APP_D("Aliro.cccRssiMeasurements.snrFirstPath              : %hu", AliroRangingData->cccSnrMeasurements[i].snrFirstPath);
            NXPLOG_APP_D("Aliro.cccRssiMeasurements.snrMainPath               : %hu", AliroRangingData->cccSnrMeasurements[i].snrMainPath);
            NXPLOG_APP_D("Aliro.cccRssiMeasurements.snrTotal                  : %d.%d", TO_Q_8_8(AliroRangingData->cccSnrMeasurements[i].snrTotal));
        }
    }
}

#endif // UWBFTR_CCC

EXTERNC void printDataTxPhaseCfgNtf(const phDataTxPhaseCfgNtf_t *pDataTxPhCfgNtf)
{
    if (pDataTxPhCfgNtf != NULL) {
        NXPLOG_APP_I("pDataTxPhCfgNtf->sessionHandle                : 0x%x\n", pDataTxPhCfgNtf->sessionHandle);
        NXPLOG_APP_I("pDataTxPhCfgNtf->status                       : 0x%x\n", pDataTxPhCfgNtf->status);
    }
    else {
        NXPLOG_APP_E("pRcvRadaNtfPkt is NULL");
    }
}
