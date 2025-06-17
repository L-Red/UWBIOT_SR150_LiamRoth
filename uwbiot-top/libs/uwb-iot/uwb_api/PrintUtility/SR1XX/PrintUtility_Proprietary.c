/*
 *
 * Copyright 2018-2020,2022,2023 NXP.
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
#include "phNxpLogApis_App.h"

EXTERNC void printDeviceInfo(const phUwbDevInfo_t *pdevInfo)
{
    if (pdevInfo != NULL) {
        NXPLOG_APP_I("UCI Generic Version            : %02X.%02X\n",
            pdevInfo->uciGenericMajor,
            pdevInfo->uciGenericMinorMaintenanceVersion);
        NXPLOG_APP_D("MAC Version                    : %02X.%02X\n",
            pdevInfo->macMajorVersion,
            pdevInfo->macMinorMaintenanceVersion);
        NXPLOG_APP_D("PHY Version                    : %02X.%02X\n",
            pdevInfo->phyMajorVersion,
            pdevInfo->phyMinorMaintenanceVersion);
        NXPLOG_APP_D("Device Name Length                 : %d\n", pdevInfo->devNameLen);
        NXPLOG_APP_I("Device Name                        : %s\n", pdevInfo->devName);
        NXPLOG_APP_I("Firmware Version                   : %02X.%02X.%02X\n",
            pdevInfo->fwMajor,
            pdevInfo->fwMinor,
            pdevInfo->fwRc);
        NXPLOG_APP_I("NXP UCI Version                    : %02X.%02X.%02x\n",
            pdevInfo->nxpUciMajor,
            pdevInfo->nxpUciMinor,
            pdevInfo->nxpUciPatch);
        LOG_MAU8_D("NXP Chip ID                        :", pdevInfo->nxpChipId, sizeof(pdevInfo->nxpChipId));

        NXPLOG_APP_I("Middleware Version                 : %02X.%02X.%02X\n",
            pdevInfo->mwMajor,
            pdevInfo->mwMinor,
            pdevInfo->mwRc);
    }
}

#if (UWBFTR_SE_SN110)
EXTERNC void printDoBindStatus(const phSeDoBindStatus_t *pDoBindStatus)
{
    if (pDoBindStatus != NULL) {
        NXPLOG_APP_D("pDoBindStatus->status                : %0x \n", pDoBindStatus->status);
        NXPLOG_APP_D("pDoBindStatus->count_remaining       : %0x \n", pDoBindStatus->count_remaining);
        NXPLOG_APP_D("pDoBindStatus->binding_state         : %0x \n", pDoBindStatus->binding_state);
        NXPLOG_APP_D("pDoBindStatus->se_instruction_code   : %0x \n", pDoBindStatus->se_instruction_code);
        NXPLOG_APP_D("pDoBindStatus->se_error_status       : %0x \n", pDoBindStatus->se_error_status);
    }
    else {
        NXPLOG_APP_E("pDoBindStatus is NULL");
    }
}

EXTERNC void printGetBindingStatus(const phSeGetBindingStatus_t *pGetBindingStatus)
{
    if (pGetBindingStatus != NULL) {
        NXPLOG_APP_D("pGetBindingStatus->status                 : %0x \n", pGetBindingStatus->status);
        NXPLOG_APP_D("pGetBindingStatus->se_binding_count       : %0x \n", pGetBindingStatus->se_binding_count);
        NXPLOG_APP_D("pGetBindingStatus->uwbd_binding_count     : %0x \n", pGetBindingStatus->uwbd_binding_count);
        NXPLOG_APP_D("pGetBindingStatus->se_instruction_code    : %0x \n", pGetBindingStatus->se_instruction_code);
        NXPLOG_APP_D("pGetBindingStatus->se_error_status        : %0x \n", pGetBindingStatus->se_error_status);
    }
    else {
        NXPLOG_APP_E("printGetBindingStatus is NULL");
    }
}

EXTERNC void printGetEseTestConnectivityStatus(const SeConnectivityStatus_t *pGetSeConnectivityStatus)
{
    if (pGetSeConnectivityStatus != NULL) {
        NXPLOG_APP_D("pGetSeConnectivityStatus->status                 : %0x \n", pGetSeConnectivityStatus->status);
        NXPLOG_APP_D(
            "pGetSeConnectivityStatus->se_instruction_code    : %0x \n", pGetSeConnectivityStatus->se_instruction_code);
        NXPLOG_APP_D(
            "pGetSeConnectivityStatus->se_error_status        : %0x \n", pGetSeConnectivityStatus->se_error_status);
    }
    else {
        NXPLOG_APP_E("printGetEseTestConnectivityStatus is NULL");
    }
}

#endif //(UWBFTR_SE_SN110)

EXTERNC void printDistance_Aoa(const phRangingData_t *pRangingData)
{
    if (pRangingData != NULL) {
        NXPLOG_APP_D("--------------Received Range Data--------------\n");
        LOG_D("pRangingData->sessionHandle                     : %" PRIu32 " \n", pRangingData->sessionHandle);
        for (uint8_t i = 0; i < pRangingData->no_of_measurements; i++) {
#if UWBFTR_TWR // support only for DSTWR
            NXPLOG_APP_D("pRangingData->range_meas[%" PRIu8 "].status          : %" PRIu8 " \n",
                i,
                pRangingData->ranging_meas.range_meas_twr[i].status);
            if (pRangingData->ranging_meas.range_meas_twr[i].status == UWBAPI_STATUS_OK) {
                NXPLOG_APP_D("pRangingData->range_meas[%" PRIu16 "].distance        : %" PRIu16 " \n",
                    i,
                    pRangingData->ranging_meas.range_meas_twr[i].distance);
                NXPLOG_APP_D("pRangingData->range_meas[%" PRIu16 "].aoaFirst             : %" PRIu16 " \n",
                    i,
                    pRangingData->ranging_meas.range_meas_twr[i].aoa_elevation_FOM);
            }
#endif //UWBFTR_TWR // support only for DSTWR
        }
    }
    else {
        NXPLOG_APP_D("pRangingData is NULL");
    }
}

EXTERNC void printDebugParams(uint8_t noOfParams, const UWB_DebugParams_List_t *DebugParams_List)
{
    if (DebugParams_List != NULL) {
        for (uint8_t LoopCnt = 0; LoopCnt < noOfParams; LoopCnt++) {
            switch (DebugParams_List->param_id) {
            case kUWB_SR1XX_DBG_CFG_THREAD_SECURE:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_THREAD_SECURE: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_THREAD_SECURE_ISR:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_THREAD_SECURE_ISR: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_THREAD_NON_SECURE_ISR:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_THREAD_NON_SECURE_ISR: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_THREAD_SHELL:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_THREAD_SHELL: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_THREAD_PHY:
                NXPLOG_APP_D(
                    "DebugParams_List->kUWB_SR1XX_DBG_CFG_THREAD_PHY: 0x%" PRIx16 " \n", DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_THREAD_RANGING:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_THREAD_RANGING: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_THREAD_SECURE_ELEMENT:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_THREAD_SECURE_ELEMENT: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_THREAD_UWB_WLAN_COEX:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_THREAD_UWB_WLAN_COEX: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_DATA_LOGGER_NTF:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_DATA_LOGGER_NTF: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_TEST_CONTENTION_RANGING_FEATURE:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_TEST_CONTENTION_RANGING_FEATURE: 0x%" PRIx16 " \n ",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_CIR_CAPTURE_WINDOW:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_CIR_CAPTURE_WINDOW: 0x%" PRIx32 "\n",
                    DebugParams_List->param_value);
                break;
            case kUWB_SR1XX_DBG_CFG_RANGING_TIMESTAMP_NTF:
                NXPLOG_APP_D("DebugParams_List->kUWB_SR1XX_DBG_CFG_RANGING_TIMESTAMP_NTF: 0x%" PRIx16 " \n",
                    DebugParams_List->param_value);
                break;
            default:
                break;
            }
            DebugParams_List++;
        }
    }
    else {
        NXPLOG_APP_D("DebugParams_List is NULL");
    }
}

EXTERNC void printTestLoopNtfData(const phTestLoopData_t *pTestLoopData)
{
    if (pTestLoopData != NULL) {
        NXPLOG_APP_D("Status                        : %" PRIu8 " \n", pTestLoopData->status);
        NXPLOG_APP_D("Loop Count                    : %" PRIu8 " \n", pTestLoopData->loop_cnt);
        NXPLOG_APP_D("Loop Pass Count               : %" PRIu8 " \n", pTestLoopData->loop_pass_count);
    }
    else {
        NXPLOG_APP_D("pTestLoopData is NULL");
    }
}
