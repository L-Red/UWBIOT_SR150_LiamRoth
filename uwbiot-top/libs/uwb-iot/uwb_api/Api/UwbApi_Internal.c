/*
 * Copyright 2018-2020,2022-2024 NXP.
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

#include "UwbApi_Internal.h"
#include "phNxpLogApis_UwbApi.h"
#include "PrintUtility.h"
#include "uwa_api.h"
#include "uwa_dm_int.h"
#include "phOsalUwb.h"
#include "uci_defs.h"
#include "uci_ext_defs.h"
#include "uci_test_defs.h"
#include "UwbAdaptation.h"
#include "UwbApi_Proprietary_Internal.h"
#include "AppConfigParams.h"
#include <UwbApi_Types.h>

#if UWBIOT_UWBD_SR1XXT
#include "UwbApi_RfTest.h"
#endif
// #include "uwb_api.h"
#include "uwb_int.h"
#include "UwbApi_Utility.h"
/* Context variable */

/* 4Bytes(sessionHandle) + 1Byte(radarstatus) + 1Byte(radar_type) + 2Bytes(num_cirs) + 1Byte(cir_taps) + 1Byte(RFU)*/
#define RADAR_CIR_NTF_HEADER 0x0A

phUwbApiContext_t uwbContext;

#if !(UWBIOT_UWBD_SR040)
tUWBAPI_STATUS recoverUWBS()
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    /*Resetting Device State to Device Init */
    uwbContext.dev_state = UWBD_STATUS_INIT;
    sep_SetWaitEvent(UWA_DM_DEVICE_STATUS_NTF_EVT);
    status = (uint8_t)UwbDeviceInit(TRUE);
    NXPLOG_UWBAPI_D("%s: uwb device init status: %d", __FUNCTION__, status);
    if (status == UWBAPI_STATUS_OK)
    {
        status = setDefaultCoreConfigs();
        if (status != UWBAPI_STATUS_OK)
        {
            NXPLOG_UWBAPI_E("%s: setDefaultCoreConfigs is failed:", __FUNCTION__);
            return status;
        }

        // Update UWBC device info
        status = getDeviceInfo();
    }
    else
    {
        sep_SetWaitEvent(DEFAULT_EVENT_TYPE);
        NXPLOG_UWBAPI_E("%s: DownloadFirmware is failed:", __FUNCTION__);
        return status;
    }
    return status;
}
#endif // !(UWBIOT_UWBD_SR040)

/*******************************************************************************
 **
 ** Function:        cleanUp
 **
 ** Description:     CleanUp all the Semaphores and Timers
 **
 ** Returns:         None
 **
 *******************************************************************************/
void cleanUp()
{
    phOsalUwb_DeleteSemaphore(&uwbContext.devMgmtSem);
    Finalize(); // disable GKI, UCI task, UWB task
    phOsalUwb_SetMemory(&uwbContext, 0x00, sizeof(phUwbApiContext_t));
}

/*******************************************************************************
 **
 ** Function:        uwbInit
 **
 ** Description:     Perform UwbInit with the callback
 **
 ** Returns:         Status
 **
 *******************************************************************************/
tUWBAPI_STATUS uwbInit(tUwbApi_AppCallback *pCallback, Uwb_operation_mode_t mode)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    const tHAL_UWB_ENTRY *halFuncEntries = NULL;
    phUwb_LogInit();
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == TRUE)
        return UWBAPI_STATUS_OK;
    uwbContext.sessionInfo.state = UWBAPI_SESSION_ERROR;
    uwbContext.pAppCallback = pCallback;
    uwbContext.receivedEventId = DEFAULT_EVENT_TYPE;
    if (phOsalUwb_CreateSemaphore(&uwbContext.devMgmtSem, 0) != UWBSTATUS_SUCCESS)
    {
        return status;
    }
    NXPLOG_UWBAPI_D("UWA_Init");
    Initialize();
    halFuncEntries = GetHalEntryFuncs();
    UWA_Init(halFuncEntries);

    NXPLOG_UWBAPI_D("UfaEnable");
    sep_SetWaitEvent(UWA_DM_ENABLE_EVT);
#if UWBFTR_DataTransfer
    status =
        UWA_Enable(&ufaDeviceManagementRspCallback, &ufaDeviceManagementNtfCallback, &ufaDeviceManagementDataCallback);
#else
    status = UWA_Enable(&ufaDeviceManagementRspCallback, &ufaDeviceManagementNtfCallback);
#endif // UWBFTR_DataTransfer
    if (status == UWBAPI_STATUS_OK)
    {
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT) !=
            UWBSTATUS_SUCCESS)
        {
            LOG_E("%s : UWA_DM_ENABLE_EVT timedout", __FUNCTION__);
            status = UWBAPI_STATUS_TIMEOUT;
            goto Error;
        }
        status = uwbContext.wstatus;
        if (status == UWBAPI_STATUS_OK)
        {
            sep_SetWaitEvent(UWA_DM_REGISTER_EXT_CB_EVT);
            status = UWA_RegisterExtCallback(&extDeviceManagementCallback);
            if (status == UWBAPI_STATUS_OK)
            {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT) !=
                    UWBSTATUS_SUCCESS)
                {
                    LOG_E("%s : UWA_DM_REGISTER_EXT_CB_EVT timedout", __FUNCTION__);
                    status = UWBAPI_STATUS_TIMEOUT;
                    goto Error;
                }
                uwbContext.isUfaEnabled = TRUE;
                sep_SetWaitEvent(UWA_DM_DEVICE_STATUS_NTF_EVT);

                status = (uint8_t)UwbDeviceInit(FALSE);
                NXPLOG_UWBAPI_D("%s: DownloadFirmware status: %d", __FUNCTION__, status);
                /* Set operating mode */
                Hal_setOperationMode(mode);
                if (status == UWBAPI_STATUS_OK)
                {
                    status = setDefaultCoreConfigs();
                    if (status != UWBAPI_STATUS_OK)
                        goto Error;
#if UWBFTR_DataTransfer
                    // Update UWBS capability info
                    phUwbCapInfo_t devCap;
                    status = getCapsInfo();
                    if (status == UWBAPI_STATUS_OK)
                    {
                        if (parseCapabilityInfo(&devCap) == FALSE)
                        {
                            NXPLOG_UWBAPI_E("%s: Parsing Capability Information Failed", __FUNCTION__);
                            status = UWBAPI_STATUS_FAILED;
                        }
                    }
                    else
                        goto Error;
#endif // UWBFTR_DataTransfer
                }
                else
                {
                    sep_SetWaitEvent(DEFAULT_EVENT_TYPE);
                    goto Error;
                }
            }
            else
            {
                NXPLOG_UWBAPI_D("%s: UWA_Enable status: %d", __FUNCTION__, status);
                return status;
            }
        }
        else
        {
            return status;
        }
        return status;
    }
Error:
    uwbContext.isUfaEnabled = FALSE;
    sep_SetWaitEvent(UWA_DM_DISABLE_EVT);
    if (UWA_Disable(FALSE) == UWBAPI_STATUS_OK)
    {
        if (UWBSTATUS_SUCCESS !=
            phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT))
        {
            LOG_E("%s : phOsalUwb_ConsumeSemaphore_WithTimeout failed", __FUNCTION__);
        }
    }
    else
    {
        NXPLOG_UWBAPI_E("%s: UFA Disable is failed:", __FUNCTION__);
    }
    cleanUp();
    NXPLOG_UWBAPI_D("%s: exit with status %d", __FUNCTION__, status);
    return status;
}

/*******************************************************************************
 **
 ** Function:        parseRangingParams
 **
 ** Description:     Extracts Ranging Params from the given byte array and updates the structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseRangingParams(uint8_t *rspPtr, uint8_t noOfParams, phRangingParams_t *pRangingParams)
{
    for (int i = 0; i < noOfParams; i++)
    {
        uint8_t paramId = *rspPtr++;
        uint8_t length = *rspPtr++;
        uint8_t *value = rspPtr;
        switch (paramId)
        {
        case UCI_PARAM_ID_DEVICE_ROLE:
            /*  Device Role */
            UWB_STREAM_TO_UINT8(pRangingParams->deviceRole, value);
            break;
        case UCI_PARAM_ID_MULTI_NODE_MODE:
            /*  Multi Node Mode */
            UWB_STREAM_TO_UINT8(pRangingParams->multiNodeMode, value);
            break;
        case UCI_PARAM_ID_MAC_ADDRESS_MODE:
            /*  Mac addr mode */
            UWB_STREAM_TO_UINT8(pRangingParams->macAddrMode, value);
            break;
#if !(UWBIOT_UWBD_SR040)
        case UCI_PARAM_ID_SCHEDULED_MODE:
            /*  Scheduled Mode */
            UWB_STREAM_TO_UINT8(pRangingParams->scheduledMode, value);
            break;
#endif // !(UWBIOT_UWBD_SR040)
        case UCI_PARAM_ID_RANGING_ROUND_USAGE:
            /* Ranging Round Usage */
            UWB_STREAM_TO_UINT8(pRangingParams->rangingRoundUsage, value);
            break;
        case UCI_PARAM_ID_DEVICE_MAC_ADDRESS:
            /*  Device Mac Address */
            UWB_STREAM_TO_ARRAY(&pRangingParams->deviceMacAddr[0], value, length);
            break;
        case UCI_PARAM_ID_DEVICE_TYPE:
            /*  Device Type */
            UWB_STREAM_TO_UINT8(pRangingParams->deviceType, value);
            break;
        default:
            break;
        }
        rspPtr += length;
    }
}

/*******************************************************************************
 **
 ** Function         processInternalRsp
 **
 ** Description      Process UCI responses in the internal group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Rsp_Event processInternalRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    /* process the message based on the opcode and message type */
    switch (oid)
    {
    case UCI_ENABLE: /* enable */
        dmEvent = UWA_DM_ENABLE_EVT;
        uwbContext.wstatus = *eventData;
        uwa_dm_cb.flags |= UWA_DM_FLAGS_DM_IS_ACTIVE;
        break;
    case UCI_DISABLE: /* disable */
        dmEvent = UWA_DM_DISABLE_EVT;
        uwbContext.wstatus = UWBAPI_STATUS_OK;
        uwa_dm_cb.flags &= (uint32_t)(~UWA_DM_FLAGS_DM_IS_ACTIVE);
        break;
    case UCI_REG_EXT_CB: /* register external CB */
        dmEvent = UWA_DM_REGISTER_EXT_CB_EVT;
        uwbContext.wstatus = UWBAPI_STATUS_OK;
        break;
    case UCI_TIMEOUT: /* response timeout event */
        dmEvent = UWA_DM_UWBD_RESP_TIMEOUT_EVT;
        uwbContext.wstatus = UWBAPI_STATUS_TIMEOUT;
        break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/*******************************************************************************
 **
 ** Function         processCoreRsp
 **
 ** Description      Process UCI responses in the CORE group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Rsp_Event processCoreRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL))
    {
        uint16_t timestampLen;

        /* process the message based on the opcode and message type */
        switch (oid)
        {
        case UCI_MSG_CORE_DEVICE_RESET:
            dmEvent = UWA_DM_DEVICE_RESET_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_CORE_DEVICE_INFO:
        {
            dmEvent = UWA_DM_CORE_GET_DEVICE_INFO_RSP_EVT;
            uint16_t device_info_len = (uint16_t)(len - sizeof(uint8_t)); // exclude status length
            uwbContext.wstatus = *eventData++;
            if (uwbContext.wstatus == UCI_STATUS_OK && device_info_len <= sizeof(uwbContext.rsp_data))
            {
                uwbContext.rsp_len = device_info_len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, device_info_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_CORE_DEVICE_CAP failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;
        case UCI_MSG_CORE_GET_CAPS_INFO:
        {
            dmEvent = UWA_DM_GET_CORE_DEVICE_CAP_RSP_EVT;
            uint16_t cap_info_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            uwbContext.wstatus = *eventData++;
            if (uwbContext.wstatus == UCI_STATUS_OK && cap_info_len <= sizeof(uwbContext.rsp_data))
            {
                eventData++; // skip no of TLVs
                uwbContext.rsp_len = cap_info_len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, cap_info_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_CORE_DEVICE_CAP failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;
        case UCI_MSG_CORE_GET_CONFIG:
        {
            /* Result of UWA_GetCoreConfig */
            dmEvent = UWA_DM_CORE_GET_CONFIG_RSP_EVT;
            uint16_t core_info_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            uwbContext.wstatus = *eventData++;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && core_info_len <= sizeof(uwbContext.rsp_data))
            {
                eventData++; // skip no of TLVs
                uwbContext.rsp_len = core_info_len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, core_info_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_CORE_CONFIG failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;
        case UCI_MSG_CORE_SET_CONFIG:
            /* Result of UWA_SetCoreConfig */
            dmEvent = UWA_DM_CORE_SET_CONFIG_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_CORE_QUERY_UWBS_TIMESTAMP:
            dmEvent = UWA_DM_PROP_QUERY_TIMESTAMP_RESP_EVT;
            uwbContext.wstatus = *eventData;
            timestampLen = len - sizeof(uint8_t); // Exclude the status
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                if (timestampLen == UCI_MSG_CORE_UWBS_TIMESTAMP_LEN)
                {
                    uwbContext.rsp_len = len;
                    phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
                }
                else
                {
                    NXPLOG_UWBAPI_E("%s: Invalid response", __FUNCTION__);
                    uwbContext.rsp_len = 0;
                }
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UCI_MSG_CORE_QUERY_UWBS_TIMESTAMP failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
            break;
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/*******************************************************************************
 **
 ** Function         processSessionManagementRsp
 **
 ** Description      Process UCI responses in the Session Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Rsp_Event processSessionManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL) && (len < (MAX_UCI_HEADER_SIZE + MAX_API_PACKET_SIZE)))
    {
        switch (oid)
        {
        case UCI_MSG_SESSION_INIT:
            dmEvent = UWA_DM_SESSION_INIT_RSP_EVT;
            uwbContext.wstatus = *eventData;
            uwbContext.rsp_len = len;
            phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            break;
        case UCI_MSG_SESSION_DEINIT:
            dmEvent = UWA_DM_SESSION_DEINIT_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_SESSION_GET_APP_CONFIG:
        {
            dmEvent = UWA_DM_SESSION_GET_CONFIG_RSP_EVT;
            uint16_t get_config_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            uwbContext.wstatus = *eventData++;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && get_config_len <= sizeof(uwbContext.rsp_data))
            {
                eventData++; // skip no of TLVs
                uwbContext.rsp_len = get_config_len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, get_config_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_APP_CONFIG failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;
        case UCI_MSG_SESSION_SET_APP_CONFIG:
            dmEvent = UWA_DM_SESSION_SET_CONFIG_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_SESSION_GET_STATE:
            dmEvent = UWA_DM_SESSION_GET_STATE_RSP_EVT;
            uwbContext.wstatus = *eventData++;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                uwbContext.rsp_data[0] = *eventData;
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: get session state Request is failed", __FUNCTION__);
            }
            break;
        case UCI_MSG_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST:
            dmEvent = UWA_DM_SESSION_MC_LIST_UPDATE_RSP_EVT;
            uwbContext.wstatus = *eventData;
            NXPLOG_UWBAPI_D("%s: Received Multicast List Status.\n", __FUNCTION__);
            break;
#if UWBIOT_UWBD_SR1XXT
        case UCI_MSG_UPDATE_ACTIVE_ROUNDS_OF_ANCHOR_DEVICE:
            dmEvent = UWA_DM_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_RSP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED)
            {
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, (uint32_t)(len - 1));
                uwbContext.rsp_len = len;
            }
            break;
        case UCI_MSG_UPDATE_ACTIVE_ROUNDS_OF_RECEIVER_DEVICE:
            dmEvent = UWA_DM_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_RSP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED)
            {
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, (uint32_t)(len - 1));
                uwbContext.rsp_len = len;
            }
            break;
#endif // UWBIOT_UWBD_SR1XXT
        case UCI_MSG_SESSION_SET_HUS_CONTROLLER_CONFIG_CMD:
            dmEvent = UWA_DM_SESSION_SET_HUS_CONTROLLER_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData);
            if (uwbContext.wstatus != UWBAPI_STATUS_OK)
            {
                NXPLOG_UWBAPI_E(
                    "%s: Received HUS Controller Config Response Error: %d\n", __FUNCTION__, uwbContext.wstatus);
            }
            break;
        case UCI_MSG_SESSION_SET_HUS_CONTROLEE_CONFIG_CMD:
            dmEvent = UWA_DM_SESSION_SET_HUS_CONTROLEE_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData);
            if (uwbContext.wstatus != UWBAPI_STATUS_OK)
            {
                NXPLOG_UWBAPI_E(
                    "%s: Received HUS Controlee Config Response Error: %d\n", __FUNCTION__, uwbContext.wstatus);
            }
            break;
        case UCI_MSG_SESSION_DATA_TRANSFER_PHASE_CONFIG:
        {
            dmEvent = UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData);
            if (uwbContext.wstatus != UWBAPI_STATUS_OK)
            {
                NXPLOG_UWBAPI_E("%s: Received DTPCM Config Response Error: %d\n", __FUNCTION__, uwbContext.wstatus);
            }
        }
        break;
#if !(UWBIOT_UWBD_SR040)
        case UCI_MSG_SESSION_QUERY_DATA_SIZE_IN_RANGING:
            dmEvent = UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_RSP_EVT;
            /**
             * Response for Query data size command update with 3 fields
             * 1: Session Handle --> 4 bytes.
             * 2: Status -->1 Byte .
             * 3: data size --> 2 bytes
             */
            /** Increment  the pointer with 4 hence it will point to the status */
            uwbContext.wstatus = *(eventData + 4);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                if (len <= sizeof(uwbContext.rsp_data))
                {
                    phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
                    uwbContext.rsp_len = len;
                }
                else
                {
                    LOG_E("%s : Not enough buffer to store %d bytes", __FUNCTION__, len);
                    uwbContext.rsp_len = 0;
                }
            }
            else
            {
                NXPLOG_UWBAPI_E(
                    "%s: Received Query status  Config Response Error: %d\n", __FUNCTION__, uwbContext.wstatus);
                uwbContext.rsp_len = 0;
            }
            break;
#endif // !(UWBIOT_UWBD_SR040)
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/*******************************************************************************
 **
 ** Function         processRangeManagementRsp
 **
 ** Description      Process UCI responses in the Ranging Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Rsp_Event processRangeManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL))
    {
        switch (oid)
        {
        case UCI_MSG_RANGE_START:
            dmEvent = UWA_DM_RANGE_START_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_RANGE_STOP:
            dmEvent = UWA_DM_RANGE_STOP_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_RANGE_BLINK_DATA_TX:
            dmEvent = UWA_DM_SEND_BLINK_DATA_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/*******************************************************************************
 **
 ** Function         processTestManagementRsp
 **
 ** Description      Process UCI responses in the test Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Rsp_Event processTestManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL))
    {
        switch (oid)
        {
        case UCI_MSG_TEST_GET_CONFIG:
        {
            dmEvent = UWA_DM_TEST_GET_CONFIG_RSP_EVT;
            uint16_t test_config_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            uwbContext.wstatus = *eventData++;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && test_config_len <= sizeof(uwbContext.rsp_data))
            {
                eventData++; // skip no of TLVs
                uwbContext.rsp_len = test_config_len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, test_config_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_TEST_GET_CONFIG_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;
        case UCI_MSG_TEST_SET_CONFIG:
            dmEvent = UWA_DM_TEST_SET_CONFIG_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_TEST_PERIODIC_TX:
            dmEvent = UWA_DM_TEST_PERIODIC_TX_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_TEST_PER_RX:
            dmEvent = UWA_DM_TEST_PER_RX_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_TEST_LOOPBACK:
            dmEvent = UWA_DM_TEST_LOOPBACK_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_TEST_RX:
            dmEvent = UWA_DM_TEST_RX_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_TEST_STOP_SESSION:
            dmEvent = UWA_DM_TEST_STOP_SESSION_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/*******************************************************************************
 **
 ** Function         processProprietaryRsp
 **
 ** Description      Process UCI responses in the propriotory group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Rsp_Event processProprietaryRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL))
    {
        switch (oid)
        {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        case EXT_UCI_MSG_SE_GET_BINDING_COUNT:
        {
            dmEvent = UWA_DM_PROP_GET_BINDING_COUNT_RESP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_PROP_GET_BINDING_COUNT_RESP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;

        case EXT_UCI_MSG_QUERY_TEMPERATURE:
        {
            dmEvent = UWA_DM_PROP_QUERY_TEMPERATURE_RESP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_PROP_QUERY_TEMPERATURE_RESP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;

#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

#if (UWBIOT_UWBD_SR100T)
        case EXT_UCI_MSG_GENERATE_TAG:
        {
            dmEvent = UWA_DM_PROP_GENERATE_TAG_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case EXT_UCI_MSG_CALIBRATION_INTEGRITY_PROTECTION:
        {
            dmEvent = UWA_DM_PROP_CALIB_INTEGRITY_PROTECTION_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case EXT_UCI_MSG_VERIFY_CALIB_DATA:
        {
            dmEvent = UWA_DM_PROP_VERIFY_CALIB_DATA_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case EXT_UCI_MSG_CONFIGURE_AUTH_TAG_OPTIONS_CMD:
        {
            dmEvent = UWA_DM_PROP_CONFIGURE_AUTH_TAG_OPTION_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case EXT_UCI_MSG_CONFIGURE_AUTH_TAG_VERSION_CMD:
        {
            dmEvent = UWA_DM_PROP_CONFIGURE_AUTH_TAG_VERSION_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

#if UWBFTR_SE_SN110
        case EXT_UCI_MSG_SE_DO_TEST_LOOP:
        {
            dmEvent = UWA_DM_PROP_SE_TEST_LOOP_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;
#endif // UWBFTR_SE_SN110
#endif // UWBIOT_UWBD_SR100T
#if UWBIOT_UWBD_SR040
        case EXT_UCI_MSG_GET_ALL_UWB_SESSIONS:
            dmEvent = UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
            break;
        case EXT_UCI_MSG_GET_TRNG:
            dmEvent = UWA_DM_PROP_TRNG_RESP_EVENT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_PROP_TRNG_RESP_EVENT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
            break;
        case EXT_UCI_MSG_DEVICE_SUSPEND_CMD:
        {
            dmEvent = UWA_DM_PROP_SUSPEND_DEVICE_RSP_ENVT;
            uwbContext.wstatus = *eventData;
        }
        break;
        case EXT_UCI_MSG_SESSION_NVM_MANAGE_CMD:
        {
            dmEvent = UWA_DM_SESSION_NVM_PAYLOAD_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        }
        break;
        case EXT_UCI_MSG_TEST_START_CMD:
        {
            dmEvent = UWA_DM_START_TEST_MODE_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        }
        break;
        case EXT_UCI_MSG_TEST_STOP_CMD:
        {
            dmEvent = UWA_DM_STOP_TEST_MODE_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        }
        break;
        case EXT_UCI_MSG_SET_TRIM_VALUES_CMD:
        {
            dmEvent = UWA_DM_SET_CALIB_TRIM_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        }
        break;
        case EXT_UCI_MSG_GET_TRIM_VALUES_CMD:
        {
            dmEvent = UWA_DM_GET_CALIB_TRIM_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        }
        break;
        case EXT_UCI_MSG_SET_PROFILE:
        {
            dmEvent = UWA_DM_PROP_PROFILE_BLOB_RSP_EVENT;
            uwbContext.wstatus = *eventData;
            /* catch the length in the response of the PROP_SET_PROFILE_CMD*/
            if ((uwbContext.wstatus == UWBAPI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data)))
            {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
        }
        break;
        case EXT_UCI_MSG_BYPASS_CURRENT_LIMITER_CMD:
        {
            dmEvent = UWA_DM_GET_BYPASS_CURRENT_LIMITER;
            uwbContext.wstatus = *eventData;
        }
        break;
#endif // UWBIOT_UWBD_SR040
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}
#if UWBIOT_UWBD_SR1XXT_SR2XXT
/*******************************************************************************
 **
 ** Function         processVendorRsp
 **
 ** Description      Process UCI responses in the propriotory group
 **
 ** Returns          eResponse_Rsp_Event
 **
 *******************************************************************************/
eResponse_Rsp_Event processVendorRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL) && (len < (MAX_UCI_HEADER_SIZE + MAX_API_PACKET_SIZE)))
    {
        switch (oid)
        {
        case VENDOR_UCI_MSG_DO_VCO_PLL_CALIBRATION:
        {
            dmEvent = UWA_DM_VENDOR_DO_VCO_PLL_CALIBRATION_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;
        case UCI_MSG_SESSION_VENDOR_GET_APP_CONFIG:
        {
            dmEvent = UWA_DM_SESSION_GET_VENDOR_CONFIG_RSP_EVT;
            uint16_t get_config_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            uwbContext.wstatus = *eventData++;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && get_config_len <= sizeof(uwbContext.rsp_data))
            {
                eventData++; // skip no of TLVs
                uwbContext.rsp_len = get_config_len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, get_config_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_APP_CONFIG failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;
        case UCI_MSG_SESSION_VENDOR_SET_APP_CONFIG:
            dmEvent = UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;

        case VENDOR_UCI_MSG_SET_DEVICE_CALIBRATION:
        {
            dmEvent = UWA_DM_VENDOR_SET_DEVICE_CALIBRATION_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case VENDOR_UCI_MSG_GET_DEVICE_CALIBRATION:
        {
            dmEvent = UWA_DM_VENDOR_GET_DEVICE_CALIBRATION_RESP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                if (sizeof(uwbContext.rsp_data) >= (size_t)len)
                {
                    uwbContext.rsp_len = len;
                    phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
                }
                else
                {
                    LOG_E("%s : Not enough buffer to store %d bytes", __FUNCTION__, len);
                    uwbContext.rsp_len = 0;
                }
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_VENDOR_GET_DEVICE_CALIBRATION_RESP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;

        case VENDOR_UCI_MSG_GET_ALL_UWB_SESSIONS:
        {
            dmEvent = UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK)
            {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: VENDOR_GET_ALL_UWB_SESSION failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        }
        break;

#if (UWBFTR_SE_SN110)
        case VENDOR_UCI_MSG_SE_DO_TEST_CONNECTIVITY:
        {
            dmEvent = UWA_DM_PROP_TEST_CONNECTIVITY_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case VENDOR_UCI_MSG_ESE_BINDING_CHECK_CMD:
        {
            dmEvent = UWA_DM_PROP_GET_BINDING_STATUS_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case VENDOR_UCI_MSG_URSK_DELETION_REQ:
        {
            dmEvent = UWA_DM_PROP_URSK_DELETION_REQUEST_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case VENDOR_UCI_MSG_SE_DO_BIND:
        {
            dmEvent = UWA_DM_PROP_DO_BIND_RESP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

#endif // (UWBFTR_SE_SN110)
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

#if UWBIOT_UWBD_SR1XXT
/*******************************************************************************
 **
 ** Function         processProprietarySERsp
 **
 ** Description      Process UCI responses in the propriotorySE group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Rsp_Event processProprietarySeRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL))
    {
        switch (oid)
        {
#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160
        case EXT_UCI_MSG_SET_PROFILE:
        {
            dmEvent = UWA_DM_PROP_PROFILE_BLOB_RSP_EVENT;
            uwbContext.wstatus = *eventData;
            /* catch the sessionHandle in the response of the PROP_SET_PROFILE_CMD*/
            if ((uwbContext.wstatus == UWBAPI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data)))
            {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
        }
        break;
        case EXT_UCI_MSG_GET_TRNG:
            dmEvent = UWA_DM_PROP_TRNG_RESP_EVENT;
            uwbContext.wstatus = *eventData;
            if ((uwbContext.wstatus == UWBAPI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data)))
            {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else
            {
                NXPLOG_UWBAPI_E("%s: UWA_DM_PROP_TRNG_RESP_EVENT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
            break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
        case EXT_UCI_MSG_WRITE_CALIB_DATA_CMD:
        {
            dmEvent = UWA_DM_PROP_WRITE_OTP_CALIB_DATA_RSP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

        case EXT_UCI_MSG_READ_CALIB_DATA_CMD:
        {
            dmEvent = UWA_DM_PROP_READ_OTP_CALIB_DATA_RSP_EVT;
            uwbContext.wstatus = *eventData;
        }
        break;

#if UWBIOT_UWBD_SR150
        case EXT_UCI_MSG_SESSION_SET_LOCALIZATION_ZONE_CMD:
        {
            dmEvent = UWA_DM_SESSION_SET_LOCALIZATION_ZONE_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        }
        break;
#endif // UWBIOT_UWBD_SR150

        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}
#endif
/*******************************************************************************
 **
 ** Function         processCoreManagementNtf
 **
 ** Description      Process UCI responses in the core Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Ntf_Event processCoreManagementNtf(uint8_t oid, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    switch (oid)
    {
    case UCI_MSG_CORE_GENERIC_ERROR_NTF:
    {
        dmEvent = UWA_DM_CORE_GEN_ERR_STATUS_EVT;
        if (UCI_STATUS_MESSAGE_RETRY == *eventData)
        {
            /* Nothing to do */
        }
        else
        {
            NXPLOG_UWBAPI_E("%s: UWA_DM_CORE_GEN_ERR_STATUS_EVT status %d", __FUNCTION__, *eventData);
        }
#if (UWBIOT_UWBD_SR1XXT)
        /*
         * Notify application if STATUS_DEVICE_TEMP_REACHED_THERMAL_RUNAWAY is received.
         */
        if (UCI_STATUS_DEVICE_TEMP_REACHED_THERMAL_RUNAWAY == *eventData)
        {
            if (uwbContext.pAppCallback)
            {
                uwbContext.pAppCallback(UWBD_OVER_TEMP_REACHED, NULL);
            }
        }
#endif
    }
    break;
    case UCI_MSG_CORE_DEVICE_STATUS_NTF:
    {
        dmEvent = UWA_DM_DEVICE_STATUS_NTF_EVT;
        // uwb_ucif_proc_core_device_status(eventData, len);
        uwbContext.dev_state = (eUWBD_DEVICE_STATUS_t)*eventData;
        if (uwbContext.dev_state == UWBD_STATUS_ERROR)
        {
            if (isCmdRespPending())
            {
                uwbContext.wstatus = UWBAPI_STATUS_TIMEOUT;
            }
            else
            {
                // in case of uwb device err firmware crash, send recovery signal to app
                if (uwbContext.pAppCallback)
                {
                    uwbContext.pAppCallback(UWBD_RECOVERY_NTF, &uwbContext.wstatus);
                }
            }
        }
        else if (uwbContext.dev_state == UWBAPI_STATUS_HPD_WAKEUP)
        {
            uwbContext.wstatus = UWBAPI_STATUS_HPD_WAKEUP;
            /* Keeping below code for future use in case we want to change the handling of HPD wakeup*/
            /* If Device Status Notification is 0xFC, then inform application to perform clean up */
            // if (uwbContext.pAppCallback) {
            //     uwbContext.pAppCallback(UWBD_ACTION_APP_CLEANUP, NULL);
            // }
        }
#if UWBIOT_UWBD_SR040
        else if (uwbContext.dev_state == UWBAPI_STATUS_LOW_POWER_ERROR)
        {
            uwbContext.wstatus = UWBAPI_STATUS_LOW_POWER_ERROR;
        }
#endif // UWBIOT_UWBD_SR040
    }
    break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/*******************************************************************************
 **
 ** Function         processSessionManagementNtf
 **
 ** Description      Process UCI responses in the session Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Ntf_Event processSessionManagementNtf(uint8_t oid, uint8_t *eventData, BOOLEAN *skip_sem_post)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    switch (oid)
    {
    case UCI_MSG_SESSION_STATUS_NTF:
    {

        NXPLOG_UWBAPI_D("Got SESSION_STATUS_NTF");
        dmEvent = UWA_DM_SESSION_STATUS_NTF_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.sessionInfo.sessionHandle, eventData);
        UWB_STREAM_TO_UINT8(uwbContext.sessionInfo.state, eventData);
        UWB_STREAM_TO_UINT8(uwbContext.sessionInfo.reason_code, eventData);

#if (defined(UWBFTR_Radar) && (UWBFTR_Radar != 0))
        if ((uwbContext.sessionInfo.reason_code == UWB_SESSION_STOPPED_DUE_TO_INBAND_SIGNAL) ||
            (uwbContext.sessionInfo.reason_code == UWB_SESSION_RADAR_FCC_LIMIT_REACHED))
        {
#else
        if (uwbContext.sessionInfo.reason_code == UWB_SESSION_STOPPED_DUE_TO_INBAND_SIGNAL)
        {
#endif                             // (defined(UWBFTR_Radar) && (UWBFTR_Radar != 0))
            *skip_sem_post = TRUE; // Skip posting session status in-band termination
        }

        if (uwbContext.sessionInfo.reason_code != UWB_SESSION_STATE_CHANGED)
        {
            if (uwbContext.pAppCallback)
            {
                uwbContext.pAppCallback(UWBD_SESSION_DATA, &uwbContext.sessionInfo);
            }
        }
    }
    break;
    case UCI_MSG_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST:
    {
        dmEvent = UWA_DM_SESSION_MC_LIST_UPDATE_NTF_EVT;
        phMulticastControleeListNtfContext_t pControleeNtfContext;
        UWB_STREAM_TO_UINT32(pControleeNtfContext.sessionHandle, eventData);
        UWB_STREAM_TO_UINT8(pControleeNtfContext.remaining_list, eventData);
        UWB_STREAM_TO_UINT8(pControleeNtfContext.no_of_controlees, eventData);

        if (pControleeNtfContext.no_of_controlees > MAX_NUM_CONTROLLEES)
        {
            NXPLOG_UWBAPI_E("%s: wrong number of controless : %d", __FUNCTION__, pControleeNtfContext.no_of_controlees);
            break;
        }
        for (uint8_t i = 0; i < pControleeNtfContext.no_of_controlees; i++)
        {
#if UWBIOT_UWBD_SR1XXT
            UWB_STREAM_TO_UINT16(pControleeNtfContext.controleeStatusList[i].controlee_mac_address, eventData);
#endif
            UWB_STREAM_TO_UINT32(pControleeNtfContext.controleeStatusList[i].subsession_id, eventData);
            UWB_STREAM_TO_UINT8(pControleeNtfContext.controleeStatusList[i].status, eventData);
        }

        if (uwbContext.pAppCallback)
        {
            uwbContext.pAppCallback(UWBD_MULTICAST_LIST_NTF, &pControleeNtfContext);
        }

        uwbContext.wstatus = *eventData;
        NXPLOG_UWBAPI_D("%s: Received Multicast List data.\n", __FUNCTION__);
    }
    break;
    case UCI_MSG_SESSION_DATA_TRANSFER_PHASE_CONFIG:
    {
        dmEvent = UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_NTF_EVT;
        phDataTxPhaseCfgNtf_t pDataTxPhCfgNtf;
        UWB_STREAM_TO_UINT32(pDataTxPhCfgNtf.sessionHandle, eventData);
        UWB_STREAM_TO_UINT8(pDataTxPhCfgNtf.status, eventData);
        if (uwbContext.pAppCallback)
        {
            uwbContext.pAppCallback(UWBD_DATA_TRANSFER_PHASE_CONFIG_NTF, &pDataTxPhCfgNtf);
        }
    }
    break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/* clang-format off */
#if UWBFTR_TWR // support only for DSTWR
/*******************************************************************************
 **
 ** Function:        parseTwoWayRangingNtf
 **
 ** Description:     Extracts Ranging Params from the given byte array for two way ranging
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseTwoWayRangingNtf(uint8_t *p, uint16_t len)
{
    uint8_t i = 0;
    if (uwbContext.rangingData.no_of_measurements > MAX_NUM_RESPONDERS) {
        NXPLOG_UWBAPI_E(
            "%s: Wrong number of measurements received:%d", __FUNCTION__, uwbContext.rangingData.no_of_measurements);
        return;
    }
    for (i = 0; i < uwbContext.rangingData.no_of_measurements; i++) {
        if (uwbContext.rangingData.mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
            UWB_STREAM_TO_ARRAY(
                &uwbContext.rangingData.ranging_meas.range_meas_twr[i].mac_addr[0], p, MAC_SHORT_ADD_LEN);
        }
        else if (uwbContext.rangingData.mac_addr_mode_indicator == EXTENDED_MAC_ADDRESS) {
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_twr[i].mac_addr[0], p, MAC_EXT_ADD_LEN);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid mac addressing indicator", __FUNCTION__);
            return;
        }
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_twr[i].status, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_twr[i].nLos, p);
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_twr[i].distance, p);
        UWB_STREAM_TO_INT16(uwbContext.rangingData.ranging_meas.range_meas_twr[i].aoa_azimuth, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_twr[i].aoa_azimuth_FOM, p);
        UWB_STREAM_TO_INT16(uwbContext.rangingData.ranging_meas.range_meas_twr[i].aoa_elevation, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_twr[i].aoa_elevation_FOM, p);
        UWB_STREAM_TO_INT16(uwbContext.rangingData.ranging_meas.range_meas_twr[i].aoa_dest_azimuth, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_twr[i].aoa_dest_azimuth_FOM, p);
        UWB_STREAM_TO_INT16(uwbContext.rangingData.ranging_meas.range_meas_twr[i].aoa_dest_elevation, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_twr[i].aoa_dest_elevation_FOM, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_twr[i].slot_index, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_twr[i].rssi, p);
        /** Skip the RFU bytes
         * if mac address format is short, then skip 11 bytes
         * if mac address format is extended, then skip 5 bytes */
        if (uwbContext.rangingData.mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
            p   = p + RFU_SHORT_MAC_ADD;
        }
        else {
            p   = p + RFU_EXT_MAC_ADD;
        }
        /* Update the received notification length */
        len = (uint16_t)(len - MAX_TWR_RNG_DATA_NTF_OFFSET);
    }
    /*check whether vendor specific data is recieved or not*/
    if (len > 0) {
#if UWBIOT_UWBD_SR040
        uint16_t vendor_spec_length = 0;
        UWB_STREAM_TO_UINT16(vendor_spec_length, p);
        if (vendor_spec_length > 0 && vendor_spec_length == 1) {
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.antenna_info, p);
        }
#endif // #if UWBIOT_UWBD_SR040
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        //vendor specific data length
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.vs_length, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        /*check whether vendor specific data is recieved or not*/
        if ((uwbContext.rangingData.vs_length <= sizeof(VENDORSPECIFIC_MEAS)) && (uwbContext.rangingData.vs_length > 0)) {
            //vendor specific data type
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data_type, p);
            len = (uint16_t)(len - sizeof(uint8_t));
            /** NXP Specific Data (FIXED PART)*/
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.twr.wifiCoExStatus, p);
            len = (uint16_t)(len - sizeof(uint8_t));
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.twr.rxInfoMesr_twr.rxMode, p);
            len = (uint16_t)(len - sizeof(uint8_t));
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo, p);
            len = (uint16_t)(len - sizeof(uint8_t));
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.vs_data.twr.rxInfoMesr_twr.rx_antennaIdRxInfo[0], p, uwbContext.rangingData.vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo);
            len = (uint16_t)(len - (sizeof(uint8_t)) * uwbContext.rangingData.vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo);
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.twr.rxInfoDebugNtf_twr.rxModeDebugNtf, p);
            len = (uint16_t)(len - sizeof(uint8_t));
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf, p);
            len = (uint16_t)(len - sizeof(uint8_t));
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.vs_data.twr.rxInfoDebugNtf_twr.rx_antennaIdDebugNtf[0], p, uwbContext.rangingData.vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf);
            len = (uint16_t)(len - (sizeof(uint8_t)) * uwbContext.rangingData.vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf);
            /** Repitition part (This fields are repeated for each responder)*/
            for (i = 0; i < uwbContext.rangingData.no_of_measurements; i++) {
                for (int j = 0; j < uwbContext.rangingData.vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo; j++) {
                    /** AoA / PDoA measurements per RX entry*/
                    UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].angleOfArrival, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                    UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].pdoa, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                    UWB_STREAM_TO_UINT16(uwbContext.rangingData.vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].pdoaIndex, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
#if UWBIOT_UWBD_SR150
                    /** FoV Specific Data */
                    if(uwbContext.rangingData.vs_data_type == FOV_SPECIFIC_DATA_TYPE)
                    {
                        UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].aoaFovFlag, p);
                        len = (uint16_t)(len - sizeof(uint8_t));
                    }
#endif // UWBIOT_UWBD_SR150
                }
                for (int k = 0; k < uwbContext.rangingData.vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf; k++) {
                    /** SNRFirst / SNRMain / FirstIndex / Main Index : measurements per RX entry*/
                    UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rxSnrFirstPath, p);
                    len = (uint16_t)(len - sizeof(uint8_t));
                    UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rxSnrMainPath, p);
                    len = (uint16_t)(len - sizeof(uint8_t));
                    UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rx_FirstPathIndex, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                    UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rx_MainPathIndex, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                }

                if ((uwbContext.rangingData.vs_data.twr.rxInfoMesr_twr.rxMode == kUWBAntCfgRxMode_ToA_Rfm_Mode) || (uwbContext.rangingData.vs_data.twr.rxInfoMesr_twr.rxMode == kUWBAntCfgRxMode_AoA_Rfm_Mode)) {
                    UWB_STREAM_TO_UINT16(uwbContext.rangingData.vs_data.twr.vsMesr[i].distance_2, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                }
            }
        }
        else {
            NXPLOG_UWBAPI_E(
                "%s: session info ntf vendor specific length exceeds the buffer limit value %d : received length "
                "%d",
                __FUNCTION__,
                sizeof(VENDORSPECIFIC_MEAS),
                uwbContext.rangingData.vs_length);
            return;
        }
        /** TODO: Need to handle as mandatory field in future*/
        if (len >= sizeof(uint8_t)) {
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.authInfoPrsen, p);
            len = (uint16_t)(len - sizeof(uint8_t));
            if (uwbContext.rangingData.authInfoPrsen != 0) {
                UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.authenticationTag[0], p, AUTH_TAG_IN_16BYTES);
                len = (uint16_t)(len - AUTH_TAG_IN_16BYTES);
            }
        }
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
    }
}
#endif // UWBFTR_TWR

#if UWBIOT_UWBD_SR1XXT
#if UWBFTR_DL_TDoA_Tag
/*******************************************************************************
 **
 ** Function:        parseDlTDoARangingNtf
 **
 ** Description:     Extracts Ranging Params from the given byte array for Dl TDOA ranging
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseDlTDoARangingNtf(uint8_t *p, uint16_t len)
{
    uint16_t actRngRounds = 0x00; /** Number of active ranging */

    for (uint8_t i = 0; i < uwbContext.rangingData.no_of_measurements; i++) {
        if (uwbContext.rangingData.mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].mac_addr[0], p, MAC_SHORT_ADD_LEN);
            len = (uint16_t)(len - MAC_SHORT_ADD_LEN);
        }
        else if (uwbContext.rangingData.mac_addr_mode_indicator == EXTENDED_MAC_ADDRESS) {
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].mac_addr[0], p, MAC_EXT_ADD_LEN);
            len = (uint16_t)(len - MAC_EXT_ADD_LEN);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid mac addressing indicator", __FUNCTION__);
            return;
        }
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].status, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_type, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].block_index, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].round_index, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].nLoS, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_INT16(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].aoa_azimuth, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].aoa_azimuth_fom, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_INT16(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].aoa_elevation, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].aoa_elevation_fom, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].rssi, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        /** 64-bit TX timestamp */
        if ((uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control & TX_TIMESTAMP_LEN) ==
            TX_TIMESTAMP_LEN) {
            /* 8 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].tx_timestamp[0], p, MAX_RX_TX_TIMESTAMP);
            len = (uint16_t)(len - MAX_RX_TX_TIMESTAMP);
        }
        /** 40-bit TX timestamp */
        else if ((uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control & TX_TIMESTAMP_LEN) == 0) {
            /* 5 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].tx_timestamp[0], p, MAX_RX_TX_TIMESTAMP - 3);
            len = (uint16_t)(len - (MAX_RX_TX_TIMESTAMP - 3));
        }
        /** 64-bit RX timestamp */
        if ((uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control & RX_TIMESTAMP_LEN) == RX_TIMESTAMP_LEN) {
            /* 8 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].rx_timestamp[0], p, MAX_RX_TX_TIMESTAMP);
            len = (uint16_t)(len - MAX_RX_TX_TIMESTAMP);
        }
        /** 40-bit RX timestamp */
        else if ((uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control & RX_TIMESTAMP_LEN) == 0) {
            /* 5 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].rx_timestamp[0], p, MAX_RX_TX_TIMESTAMP - 3);
            len = (uint16_t)(len - (MAX_RX_TX_TIMESTAMP - 3));
        }

        UWB_STREAM_TO_INT16(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].cfo_anchor, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_INT16(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].cfo, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_UINT32(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].reply_time_initiator, p);
        len = (uint16_t)(len - sizeof(uint32_t));
        UWB_STREAM_TO_UINT32(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].reply_time_responder, p);
        len = (uint16_t)(len - sizeof(uint32_t));
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].initiator_responder_tof, p);
        len = (uint16_t)(len - sizeof(uint16_t));

        if ((uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control & ANCHOR_LOCATION_WGS84) == ANCHOR_LOCATION_WGS84) {
            /** DT-Anchor location is included in WGS-84 coordinate system - 12 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].anchor_location[0], p, MAX_ANCHOR_LOCATIONS);
            len = (uint16_t)(len - MAX_ANCHOR_LOCATIONS);
        }
        else if ((uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control & ANCHOR_LOCATION_REL) == ANCHOR_LOCATION_REL) {
            /** DT-Anchor location is included in a relative coordinate system - 10 Octets*/
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].anchor_location[0], p, (MAX_ANCHOR_LOCATIONS - 2));
            len = (uint16_t)(len - (MAX_ANCHOR_LOCATIONS - 2));
        }
        else if ((uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control & ANCHOR_LOCATION_WGS84) == 0) {
            /** DT-Anchor location is not included - 0 Octets */
        }
        actRngRounds = (uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].message_control >> ACTIVE_RR_OFSET) & MAX_ACTIVE_RR;
        UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_dltdoa[i].active_ranging_rounds[0], p, actRngRounds);
        len = (uint16_t)(len - actRngRounds);
    }
    /*check whether data is present or not*/
    /** TODO: Need to handle as mandatory field in future*/
    if (len >=  sizeof(uint32_t)) {
        /** Antenna Rx Configuration information used in current ranging round*/
        UWB_STREAM_TO_UINT32(uwbContext.rangingData.antenna_pairInfo, p);
        len = (uint16_t)(len - sizeof(uint32_t));
    }
    if (len >=  sizeof(uint8_t)) {
        /** Status code for WLAN during ranging RR*/
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.wifiCoExStatusCode, p);
        len = (uint16_t)(len - sizeof(uint8_t));
    }
    if (len >=  sizeof(uint8_t)) {
        /** Indicator for presence of Authentication Tag*/
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.authInfoPrsen, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        if (uwbContext.rangingData.authInfoPrsen != 0) {
            /**Authentication Tag*/
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.authenticationTag[0], p, AUTH_TAG_IN_16BYTES);
            len = (uint16_t)(len - AUTH_TAG_IN_16BYTES);
        }
    }
}
#endif // UWBFTR_DL_TDoA_Tag
#endif // UWBIOT_UWBD_SR1XXT

#if UWBFTR_UL_TDoA_Anchor
/*******************************************************************************
 **
 ** Function:        parseOneWayRangingNtf
 **
 ** Description:     Extracts Ranging Params from the given byte array for one way ranging
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseOneWayRangingNtf(uint8_t *p, uint16_t len)
{
    uint8_t message_control = 0x00;
    //LOG_MAU8_I(" IN API ->",p,len);

    for (uint8_t i = 0; i < uwbContext.rangingData.no_of_measurements; i++) {
        if (uwbContext.rangingData.mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].mac_addr[0], p, MAC_SHORT_ADD_LEN);
            len = (uint16_t)(len - MAC_SHORT_ADD_LEN);
        }
        else if (uwbContext.rangingData.mac_addr_mode_indicator == EXTENDED_MAC_ADDRESS) {
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].mac_addr[0], p, MAC_EXT_ADD_LEN);
            len = (uint16_t)(len - MAC_EXT_ADD_LEN);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid MAC addressing indicator", __FUNCTION__);
            return;
        }
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].status, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].message_control, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].frame_type, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].nLos, p);
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].aoa_azimuth, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].aoa_azimuth_FOM, p);
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].aoa_elevation, p);
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].aoa_elevation_FOM, p);
        UWB_STREAM_TO_UINT32(uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].frame_number, p);
        len = (uint16_t)(len - ONEWAY_RFU_BYTE_OFFSET);

        message_control = uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].message_control;
        // Set initial values as 0, if not present for Tx timestamp and device ID
        phOsalUwb_SetMemory(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].tx_timestamp[0], 0, ULTDOA_64BIT_IN_BYTES);
        phOsalUwb_SetMemory(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].ul_tdoa_device_id[0], 0, ULTDOA_64BIT_IN_BYTES);

        if ((message_control & ULTDOA_64BIT_RX_TIMESTAMP_MASK) == ULTDOA_64BIT_RX_TIMESTAMP_MASK) {
            /* 8 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].rx_timestamp[0], p, ULTDOA_64BIT_IN_BYTES);
            len = (uint16_t)(len - sizeof(uint64_t));
        }
        else {
            /* 5 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].rx_timestamp[0], p, ULTDOA_40BIT_IN_BYTES);
            len = (uint16_t)(len - ULTDOA_40BIT_IN_BYTES);
        }

        if ((message_control & ULTDOA_DEVICE_ID_MASK) == ULTDOA_DEVICE_ID_16BIT_VALUE) {
             /* 2 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].ul_tdoa_device_id[0], p, ULTDOA_16BIT_IN_BYTES);
            len = (uint16_t)(len - sizeof(uint16_t));
        }
        else if ((message_control & ULTDOA_DEVICE_ID_MASK) == ULTDOA_DEVICE_ID_32BIT_VALUE) {
             /* 4 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].ul_tdoa_device_id[0], p, ULTDOA_32BIT_IN_BYTES);
            len = (uint16_t)(len - sizeof(uint32_t));
        }
        else if ((message_control & ULTDOA_DEVICE_ID_MASK) == ULTDOA_DEVICE_ID_64BIT_VALUE) {
             /* 8 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].ul_tdoa_device_id[0], p, ULTDOA_64BIT_IN_BYTES);
            len = (uint16_t)(len - sizeof(uint64_t));
        }

        if ((message_control & ULTDOA_40BIT_TX_TIMESTAMP_MASK) == ULTDOA_40BIT_TX_TIMESTAMP_MASK) {
            /* 5 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].tx_timestamp[0], p, ULTDOA_40BIT_IN_BYTES);
            len = (uint16_t)(len - ULTDOA_40BIT_IN_BYTES);
        }
        if ((message_control & ULTDOA_64BIT_TX_TIMESTAMP_MASK) == ULTDOA_64BIT_TX_TIMESTAMP_MASK) {
             /* 8 Octets */
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.ranging_meas.range_meas_tdoa[i].tx_timestamp[0], p, ULTDOA_64BIT_IN_BYTES);
            len = (uint16_t)(len - sizeof(uint64_t));
        }
    }
    if(len > 0) {
#if UWBIOT_UWBD_SR040
        uint16_t vendor_spec_length = 0;
        /*check whether vendor specific data is received or not*/
        UWB_STREAM_TO_UINT16(vendor_spec_length, p);

        if (vendor_spec_length > 0 && vendor_spec_length == 1) {
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.antenna_info, p);
        }
#endif // UWBIOT_UWBD_SR040

#if UWBIOT_UWBD_SR1XXT
        //vendor specific data length
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.vs_length, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        /*check whether vendor specific data is received or not*/
        if (uwbContext.rangingData.vs_length > 0) {
            /** Vendor specific Extended data length*/
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.tdoa.vendorExtLength, p);
            len = (uint16_t)(len - sizeof(uint8_t));

            if((uwbContext.rangingData.vs_data.tdoa.vendorExtLength > 0))
            {
                /** NXP Specific Data (FIXED PART)*/
                UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.tdoa.rssi_rx1, p);
                len = (uint16_t)(len - sizeof(uint16_t));
                UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.tdoa.rssi_rx2, p);
                len = (uint16_t)(len - sizeof(uint16_t));
                UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.tdoa.noOfPdoaMeasures, p);
                len = (uint16_t)(len - sizeof(uint8_t));
                /** Repitition part (This fields are repeated for each Antenna Pair)*/
                for (uint8_t k = 0; k < uwbContext.rangingData.vs_data.tdoa.noOfPdoaMeasures; k++) {
                    /** AoA / PDoA measurements per RX entry*/
                    UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.tdoa.pdoa[k], p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                    UWB_STREAM_TO_UINT16(uwbContext.rangingData.vs_data.tdoa.pdoaIndex[k], p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                }
            }

            /** TODO: Need to handle as mandatory field in future*/
            if (len >=  sizeof(uint32_t)) {
                /** Antenna Rx Configuration information used in current ranging round*/
                UWB_STREAM_TO_UINT32(uwbContext.rangingData.antenna_pairInfo, p);
                len = (uint16_t)(len - sizeof(uint32_t));
            }
            if (len >=  sizeof(uint8_t)) {
                /** Status code for WLAN during ranging RR*/
                UWB_STREAM_TO_UINT8(uwbContext.rangingData.wifiCoExStatusCode, p);
                len = (uint16_t)(len - sizeof(uint8_t));
            }
            if (len >=  sizeof(uint8_t)) {
                /** Indicator for presence of Authentication Tag*/
                UWB_STREAM_TO_UINT8(uwbContext.rangingData.authInfoPrsen, p);
                len = (uint16_t)(len - sizeof(uint8_t));
                if (uwbContext.rangingData.authInfoPrsen != 0) {
                    /**Authentication Tag*/
                    UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.authenticationTag[0], p, AUTH_TAG_IN_16BYTES);
                    len = (uint16_t)(len - AUTH_TAG_IN_16BYTES);
                }
            }
        }
        else {
            NXPLOG_UWBAPI_E(
                "%s: session info ntf vendor specific length exceeds the buffer limit value %d : received length "
                "%d",
                __FUNCTION__,
                sizeof(VENDORSPECIFIC_MEAS),
                uwbContext.rangingData.vs_length);
            return;
        }
#endif // UWBIOT_UWBD_SR1XXT
    }
}

#endif // UWBFTR_UL_TDoA_Anchor
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
/*******************************************************************************
 **
 ** Function:        parseOwrWithAoaNtf
 **
 ** Description:     Extracts Ranging Params from the given byte array for OWR with AoA
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseOwrWithAoaNtf(uint8_t *p, uint16_t len)
{
    for (uint8_t i = 0; i < uwbContext.rangingData.no_of_measurements; i++) {
        if (uwbContext.rangingData.mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
            UWB_STREAM_TO_ARRAY(
                &uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].mac_addr[0], p, MAC_SHORT_ADD_LEN);
            len = (uint16_t)(len - MAC_SHORT_ADD_LEN);
        }
        else if (uwbContext.rangingData.mac_addr_mode_indicator == EXTENDED_MAC_ADDRESS) {
            UWB_STREAM_TO_ARRAY(
                &uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].mac_addr[0], p, MAC_EXT_ADD_LEN);
            len = (uint16_t)(len - MAC_EXT_ADD_LEN);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid mac addressing indicator", __FUNCTION__);
            return;
        }
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].status, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].nLos, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].frame_seq_num, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].block_index, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].aoa_azimuth, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].aoa_azimuth_FOM, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].aoa_elevation, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_meas.range_meas_owr_aoa[i].aoa_elevation_FOM, p);
        len = (uint16_t)(len - sizeof(uint8_t));
    }
    if(len > 0)
    {
        /** Vendor specific data Length*/
        UWB_STREAM_TO_UINT16(uwbContext.rangingData.vs_length, p);
        len = (uint16_t)(len - sizeof(uint16_t));
        /*check whether vendor specific data is recieved or not*/
        if ((uwbContext.rangingData.vs_length <= sizeof(VENDORSPECIFIC_MEAS)) && (uwbContext.rangingData.vs_length > 0)) {
            /** Vendor specific data type*/
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data_type, p);
            len = (uint16_t)(len - sizeof(uint8_t));
            /** NXP Specific Data (FIXED PART)*/
            /** Rx Antenna Info for AoA Measurements*/
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.owr_aoa.rxInfoMesr_owr.rxMode,p);
            len = (uint16_t)(len - sizeof(uint8_t));
            UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo,p);
            len = (uint16_t)(len - sizeof(uint8_t));
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.vs_data.owr_aoa.rxInfoMesr_owr.rx_antennaIdRxInfo[0],p,uwbContext.rangingData.vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo);
            len = (uint16_t)(len - (sizeof(uint8_t) * uwbContext.rangingData.vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo));

            for (uint8_t i = 0; i < uwbContext.rangingData.no_of_measurements; i++) {
                for (int j = 0; j < uwbContext.rangingData.vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo;j++) {
                    /** Repitition part(This fields are repeated for each responder)*/
                    /** AoA / PDoA measurements per RX entry*/
                    /** Angle of arrival*/
                    UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[j].angleOfArrival, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                    /** Phase difference of arrival*/
                    UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[j].pdoa, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
                    /** Phase difference of arrival index in the whole CIR */
                    UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[j].pdoaIndex, p);
                    len = (uint16_t)(len - sizeof(uint16_t));
#if UWBIOT_UWBD_SR150
                /** FoV Specific Data */
                    if(uwbContext.rangingData.vs_data_type == FOV_SPECIFIC_DATA_TYPE)
                    {
                        UWB_STREAM_TO_UINT8(uwbContext.rangingData.vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[j].aoaFovFlag, p);
                        len = (uint16_t)(len - sizeof(uint8_t));
                    }
#endif // UWBIOT_UWBD_SR150
                }
                /**RSSI*/
                UWB_STREAM_TO_INT16(uwbContext.rangingData.vs_data.owr_aoa.vsMesr[i].rssi, p);
                len = (uint16_t)(len - sizeof(uint16_t));
            }
        }
        else {
        NXPLOG_UWBAPI_E("%s: session info ntf vendor specific length exceeds the buffer limit value %d : received length ""%d", __FUNCTION__, sizeof(VENDORSPECIFIC_MEAS), uwbContext.rangingData.vs_length);
        return;
        }
    }

    /*check whether data is present or not*/
    /** TODO: Need to handle as mandatory field in future*/
    if (len >= sizeof(uint8_t)) {
        UWB_STREAM_TO_UINT8(uwbContext.rangingData.authInfoPrsen, p);
        len = (uint16_t)(len - sizeof(uint8_t));
        if (uwbContext.rangingData.authInfoPrsen != 0) {
            UWB_STREAM_TO_ARRAY(&uwbContext.rangingData.authenticationTag[0], p, AUTH_TAG_IN_16BYTES);
            len = (uint16_t)(len - AUTH_TAG_IN_16BYTES);
        }
    }
}
#endif // (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
/* clang-format off */

/*******************************************************************************
 **
 ** Function:        parseRangingNtf
 **
 ** Description:     Extracts Ranging Params from the given byte array and updates the structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseRangingNtf(uint8_t *p, uint16_t len)
{
    UWB_STREAM_TO_UINT32(uwbContext.rangingData.seq_ctr, p);
    UWB_STREAM_TO_UINT32(uwbContext.rangingData.sessionHandle, p);
    UWB_STREAM_TO_UINT8(uwbContext.rangingData.rcr_indication, p);
    UWB_STREAM_TO_UINT32(uwbContext.rangingData.curr_range_interval, p);
    UWB_STREAM_TO_UINT8(uwbContext.rangingData.ranging_measure_type, p);
    p++; // skip rfu byte
    UWB_STREAM_TO_UINT8(uwbContext.rangingData.mac_addr_mode_indicator, p);
    UWB_STREAM_TO_UINT32(uwbContext.rangingData.sessionHandle_of_primary_session, p);
    p = p + RESERVED_LEN; // skip reserved bytes
    UWB_STREAM_TO_UINT8(uwbContext.rangingData.no_of_measurements, p);
    len = (uint16_t)(len - MAC_ADDR_OFFSET);

    if ((uwbContext.rangingData.ranging_measure_type != MEASUREMENT_TYPE_TWOWAY) &&
        (uwbContext.rangingData.ranging_measure_type != MEASUREMENT_TYPE_ONEWAY) &&
        (uwbContext.rangingData.ranging_measure_type != MEASUREMENT_TYPE_DLTDOA) &&
        (uwbContext.rangingData.ranging_measure_type != MEASUREMENT_TYPE_OWR_WITH_AOA)) {
        NXPLOG_UWBAPI_E("%s: Measurement type not matched", __FUNCTION__);
    }

#if UWBFTR_TWR // support only for DSTWR
    if (uwbContext.rangingData.ranging_measure_type == MEASUREMENT_TYPE_TWOWAY) {
        parseTwoWayRangingNtf(p, len);
    }
#endif // UWBFTR_TWR
#if (UWBFTR_UL_TDoA_Anchor)
    if (uwbContext.rangingData.ranging_measure_type == MEASUREMENT_TYPE_ONEWAY) {
        parseOneWayRangingNtf(p, len);
    }
#endif // UWBFTR_UL_TDoA_Anchor
#if (UWBIOT_UWBD_SR1XXT && UWBFTR_DL_TDoA_Tag)
    if (uwbContext.rangingData.ranging_measure_type == MEASUREMENT_TYPE_DLTDOA) {
        parseDlTDoARangingNtf(p, len);
    }
#endif // UWBFTR_DL_TDoA_Tag
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
    if (uwbContext.rangingData.ranging_measure_type == MEASUREMENT_TYPE_OWR_WITH_AOA) {
        parseOwrWithAoaNtf(p, len);
    }
#endif // (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_RANGING_DATA, &uwbContext.rangingData);
    }
}

#if UWBFTR_Radar
/*******************************************************************************
 **
 ** Function:        parseRadarCirNtf
 **
 ** Description:     Extracts Radar Params from the given byte array for Radar CIR notification structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseRadarCirNtf(uint8_t *p, uint16_t len)
{
    uwbContext.RadarNtf.radar_ntf.radr_cir.cir_len = len - RADAR_CIR_NTF_HEADER;
    UWB_STREAM_TO_UINT16(uwbContext.RadarNtf.radar_ntf.radr_cir.num_cirs, p);
    UWB_STREAM_TO_UINT8(uwbContext.RadarNtf.radar_ntf.radr_cir.cir_taps, p);
    UWB_STREAM_TO_UINT8(uwbContext.RadarNtf.radar_ntf.radr_cir.rfu, p);
    /*Application/Demo needs to allcoate the memory for CIR Data. It will not happen in the API context*/
    uwbContext.RadarNtf.radar_ntf.radr_cir.cirdata = p;
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_RADAR_RCV_NTF, &uwbContext.RadarNtf);
    }
}

/*******************************************************************************
 **
 ** Function:        parseRadarTestNtf
 **
 ** Description:     Extracts Radar Params from the given byte array for Radar Test Isolation notification structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseRadarTestNtf(uint8_t *p, uint16_t len)
{
    UWB_STREAM_TO_UINT8(uwbContext.RadarNtf.radar_ntf.radar_tst_ntf.antenna_tx, p);
    UWB_STREAM_TO_UINT8(uwbContext.RadarNtf.radar_ntf.radar_tst_ntf.antenna_rx, p);
    UWB_STREAM_TO_UINT16(uwbContext.RadarNtf.radar_ntf.radar_tst_ntf.anteena_isolation, p);
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_TEST_RADAR_ISO_NTF, &uwbContext.RadarNtf);
    }
}

/*******************************************************************************
 **
 ** Function:        parseRadarNtf
 **
 ** Description:     Extracts Radar Params from the given byte array and updates the structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseRadarNtf(uint8_t *p, uint16_t len)
{
    UWB_STREAM_TO_UINT32(uwbContext.RadarNtf.sessionHandle, p);
    UWB_STREAM_TO_UINT8(uwbContext.RadarNtf.radar_status, p);
    UWB_STREAM_TO_UINT8(uwbContext.RadarNtf.radar_type, p);
    if (uwbContext.RadarNtf.radar_type == RADAR_MEASUREMENT_TYPE_CIR) {
        parseRadarCirNtf(p, len);
    }
    else if (uwbContext.RadarNtf.radar_type == RADAR_MEASUREMENT_TYPE_TEST_ISOLATION) {
        parseRadarTestNtf(p, len);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Measurement type not matched", __FUNCTION__);
    }
}

#endif // UWBFTR_Radar

#if UWBFTR_CCC
/*******************************************************************************
 **
 ** Function:        parseCccRangingNtf
 **
 ** Description:     Extracts CCC Ranging Params from the given byte array and updates the structure
  ** Returns:         None
 **
 *******************************************************************************/
void parseCccRangingNtf(uint8_t *p, uint16_t len)
{
    UWB_STREAM_TO_UINT32(uwbContext.AliroRangingData.sessionHandle, p);
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.rangingStatus, p);
    UWB_STREAM_TO_UINT32(uwbContext.AliroRangingData.stsIndex, p);
    UWB_STREAM_TO_UINT16(uwbContext.AliroRangingData.rangingRoundIndex, p);
    UWB_STREAM_TO_UINT16(uwbContext.AliroRangingData.blockIndex, p);
    UWB_STREAM_TO_UINT16(uwbContext.AliroRangingData.distance, p);
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.uncertanityAnchorFom, p);
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.uncertanityInitiatorFom, p);
    UWB_STREAM_TO_ARRAY(uwbContext.AliroRangingData.ccmTag, p, MAX_CCM_TAG_SIZE);
    UWB_STREAM_TO_INT16(uwbContext.AliroRangingData.aoa_azimuth, p);
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.aoa_azimuth_FOM, p);
    UWB_STREAM_TO_INT16(uwbContext.AliroRangingData.aoa_elevation, p);
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.aoa_elevation_FOM, p);
    /** Antenna Pair Info*/
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.antenna_pair_info.configMode, p);
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.antenna_pair_info.antPairId1, p);
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.antenna_pair_info.antPairId2, p);
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.antenna_pair_info.rfu, p);
    /** PDoA Measurements*/
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.noOfPdoaMeasures, p);
    for (size_t i = 0; i < uwbContext.AliroRangingData.noOfPdoaMeasures; i++) {
        UWB_STREAM_TO_INT16(uwbContext.AliroRangingData.pdoaMeasurements[i].pdoa, p);
        UWB_STREAM_TO_UINT16(uwbContext.AliroRangingData.pdoaMeasurements[i].pdoaIndex, p);
    }
    /** RSSI Measurements */
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.noOfRssiMeasurements, p);
    if (uwbContext.AliroRangingData.noOfRssiMeasurements <= MAX_NO_OF_CCC_RSSI_MEASUREMENTS) {
        for (size_t i = 0; i < uwbContext.AliroRangingData.noOfRssiMeasurements; i++) {
            UWB_STREAM_TO_INT16(uwbContext.AliroRangingData.cccRssiMeasurements[i].rssi_rx1, p);
            UWB_STREAM_TO_INT16(uwbContext.AliroRangingData.cccRssiMeasurements[i].rssi_rx2, p);
        }
    }
    else {
        NXPLOG_UWBAPI_W("%s: Invalid Range of RSSI Measurements : %d, Expected was %d",
            __FUNCTION__,
            uwbContext.AliroRangingData.noOfRssiMeasurements,
            MAX_NO_OF_CCC_RSSI_MEASUREMENTS);
    }
    /** SNR Measurements */
    UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.noOfSnrMeasurements, p);
    if (uwbContext.AliroRangingData.noOfSnrMeasurements <= MAX_NO_OF_CCC_SNR_MEASUREMENTS) {
        for (size_t i = 0; i < uwbContext.AliroRangingData.noOfSnrMeasurements; i++) {
            UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.cccSnrMeasurements[i].slotIndexAndAntennaMap, p);
            UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.cccSnrMeasurements[i].snrFirstPath, p);
            UWB_STREAM_TO_UINT8(uwbContext.AliroRangingData.cccSnrMeasurements[i].snrMainPath, p);
            UWB_STREAM_TO_UINT16(uwbContext.AliroRangingData.cccSnrMeasurements[i].snrTotal, p);
        }
    }
    else {
        NXPLOG_UWBAPI_W("%s: Invalid Range of SNR Measurements : %d, Expected was %d",
            __FUNCTION__,
            uwbContext.AliroRangingData.noOfSnrMeasurements,
            MAX_NO_OF_CCC_SNR_MEASUREMENTS);
    }

    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_RANGING_ALIRO_DATA, &uwbContext.AliroRangingData);
    }
}
#endif // UWBFTR_CCC

/*******************************************************************************
 **
 ** Function         processRangeManagementNtf
 **
 ** Description      Process UCI responses in the range Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Ntf_Event processRangeManagementNtf(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    switch (oid) {
#if UWBFTR_DataTransfer
    case UCI_MSG_DATA_TRANSMIT_STATUS_NTF: {
        dmEvent = UWA_DM_DATA_TRANSMIT_STATUS_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.dataTransmit.transmitNtf_sessionHandle, eventData);
        UWB_STREAM_TO_UINT16(uwbContext.dataTransmit.transmitNtf_sequence_number, eventData);
        UWB_STREAM_TO_UINT8(uwbContext.dataTransmit.transmitNtf_status, eventData);
        UWB_STREAM_TO_UINT8(uwbContext.dataTransmit.transmitNtf_txcount, eventData);
    } break;
    case UCI_MSG_DATA_CREDIT_NTF: {
        dmEvent = UWA_DM_DATA_CREDIT_STATUS_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.dataCredit.sessionHandle, eventData);
        UWB_STREAM_TO_UINT8(uwbContext.dataCredit.credit_availability, eventData);
    } break;
#endif // UWBFTR_DataTransfer
    case UCI_MSG_SESSION_INFO_NTF:
        dmEvent = UWA_DM_SESSION_INFO_NTF_EVT;
        parseRangingNtf(eventData, len);
        break;
#if UWBFTR_CCC
    case UCI_MSG_RANGE_CCC_DATA_NTF: {
        dmEvent = UWA_DM_RANGE_CCC_DATA_NTF_EVT;
        parseCccRangingNtf(eventData, len);
    } break;
#endif // UWBFTR_CCC
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    return dmEvent;
}

#if UWBFTR_Radar

/*******************************************************************************
 **
 ** Function         processDataControlNtf
 **
 ** Description      Process UCI responses in the data Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Ntf_Event processDataControlNtf(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    switch (oid) {
    case EXT_UCI_MSG_RADAR_NTF: {
        if (len < RADAR_CIR_NTF_HEADER) {
            NXPLOG_UWBAPI_E("%s: Invalid  Radar Notificaiton Length :0x%x", __FUNCTION__, len);
            break;
        }
        parseRadarNtf(eventData, len);
    } break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    return dmEvent;
}

#endif// UWBFTR_Radar

/*******************************************************************************
 **
 ** Function         processTestManagementNtf
 **
 ** Description      Process UCI responses in the test Management group
 **
 ** Returns          void
 **
 *******************************************************************************/
eResponse_Ntf_Event processTestManagementNtf(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    switch (oid) {
    case UCI_MSG_TEST_PERIODIC_TX: {
        dmEvent = UWA_DM_TEST_PERIODIC_TX_NTF_EVT;
        phPerTxData_t pPerTxData;
        UWB_STREAM_TO_UINT8(pPerTxData.status, eventData);
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_PER_SEND, &pPerTxData);
        }
    } break;
    case UCI_MSG_TEST_PER_RX:
    case UCI_MSG_TEST_RX:
    case UCI_MSG_TEST_LOOPBACK: {
        phRfTestData_t pRfTestData;
        if (len >= 1) {
            UWB_STREAM_TO_UINT8(pRfTestData.status, eventData);
            pRfTestData.dataLength = (uint16_t)(len - 1);
        }
        else {
            LOG_E("%s: Invalid RF test data", __FUNCTION__);
            break;
        }
        if ((pRfTestData.dataLength != 0) && (pRfTestData.dataLength <= MAX_UCI_PACKET_SIZE)) {
            phOsalUwb_MemCopy(pRfTestData.data, eventData, pRfTestData.dataLength);
        }
        else {
            // TODO: need to handle HPRF case where the data can be of 4K+ .
            phOsalUwb_SetMemory(&pRfTestData.data, 0, MAX_UCI_PACKET_SIZE);
            pRfTestData.dataLength = (uint16_t)(MAX_UCI_PACKET_SIZE);
        }

        if (uwbContext.pAppCallback) {
            switch (oid) {
            case UCI_MSG_TEST_PER_RX:
                dmEvent = UWA_DM_TEST_PER_RX_NTF_EVT;
                uwbContext.pAppCallback(UWBD_PER_RCV, &pRfTestData);
                break;
            case UCI_MSG_TEST_RX:
                dmEvent = UWA_DM_TEST_RX_NTF_EVT;
                uwbContext.pAppCallback(UWBD_TEST_RX_RCV, &pRfTestData);
                break;
            case UCI_MSG_TEST_LOOPBACK:
                dmEvent = UWA_DM_TEST_LOOPBACK_NTF_EVT;
                uwbContext.pAppCallback(UWBD_TEST_MODE_LOOP_BACK_NTF, &pRfTestData);
                break;
            }
        }
    } break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    return dmEvent;
}

/*******************************************************************************
 **
 ** Function:        ufaDeviceManagementRspCallback
 **
 ** Description:     Receive device management response events from stack.
 **                  dmEvent: Device-management event ID.
 **                  eventData: Data associated with event ID.
 **
 ** Returns:         None
 **
 *******************************************************************************/

void ufaDeviceManagementRspCallback(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;

    //NXPLOG_UWBAPI_D("%s: enter; event=0x%X status %d", __FUNCTION__, dmEvent, *eventData);

    switch (gid) {
    case UCI_GID_INTERNAL:
        dmEvent = processInternalRsp(oid, len, eventData);
        break;
    case UCI_GID_CORE: /* 0000b UCI Core group */
        dmEvent = processCoreRsp(oid, len, eventData);
        break;
    case UCI_GID_SESSION_MANAGE: /* 0001b UCI Session Config group */
        dmEvent = processSessionManagementRsp(oid, len, eventData);
        break;
    case UCI_GID_RANGE_MANAGE: /* 0010b UCI Range group */
        dmEvent = processRangeManagementRsp(oid, len, eventData);
        break;
    case UCI_GID_TEST: /* 1101b test group */
        dmEvent = processTestManagementRsp(oid, len, eventData);
        break;
    case UCI_GID_PROPRIETARY:
        dmEvent = processProprietaryRsp(oid, len, eventData);
        break;
#if UWBIOT_UWBD_SR1XXT
    case UCI_GID_PROPRIETARY_SE:
        dmEvent = processProprietarySeRsp(oid, len, eventData);
        break;
#endif
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    case UCI_GID_VENDOR:
        dmEvent = processVendorRsp(oid, len, eventData);
        break;
#endif
    default:
        NXPLOG_UWBAPI_E("ufaDeviceManagementRspCallback: Unknown gid:%d", gid);
        break;
    }

    uwbContext.receivedEventId = dmEvent;

    if (uwbContext.currentEventId == dmEvent || dmEvent == UWA_DM_UWBD_RESP_TIMEOUT_EVT ||
        (isCmdRespPending() && (uwbContext.dev_state == UWBD_STATUS_ERROR))) {
        NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __FUNCTION__);
        uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
        (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
    }
}

/*******************************************************************************
 **
 ** Function:        ufaDeviceManagementNtfCallback
 **
 ** Description:     Receive device management response events from stack.
 **                  dmEvent: Device-management event ID.
 **                  eventData: Data associated with event ID.
 **
 ** Returns:         None
 **
 *******************************************************************************/

void ufaDeviceManagementNtfCallback(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    BOOLEAN skip_sem_post = FALSE;

    if ((len != 0) && (eventData != NULL)) {
        NXPLOG_UWBAPI_D("%s: enter; event=0x%X status %d", __FUNCTION__, dmEvent, *eventData);

        switch (gid) {
        case UCI_GID_CORE:
            dmEvent = processCoreManagementNtf(oid, eventData);
            break;
        case UCI_GID_SESSION_MANAGE: /* 0010b UCI management group */
            dmEvent = processSessionManagementNtf(oid, eventData, &skip_sem_post);
            break;
        case UCI_GID_RANGE_MANAGE: /* 0011b UCI Range management group */
            dmEvent = processRangeManagementNtf(oid, len, eventData);
            break;
#if UWBFTR_Radar
        case UCI_GID_PROP_RADAR_CONTROL: /* 1001b UCI DATA control group */
            dmEvent = processDataControlNtf(oid, len, eventData);
#if UWBFTR_Radar
            if (oid == EXT_UCI_MSG_RADAR_NTF) {
                skip_sem_post = TRUE;
            }
#endif
            break;
#endif                     // UWBFTR_DataTransfer ||UWBFTR_Radar
        case UCI_GID_TEST: /* 1101b test group */
            dmEvent = processTestManagementNtf(oid, len, eventData);
            break;
#if UWBIOT_UWBD_SR2XXT
        case 0x0B: // With CIRs it's 0x0B. TBC
            break;
#endif
        default:
            NXPLOG_UWBAPI_E("ufaDeviceManagementNtfCallback: UWB Unknown gid:%d", gid);
            break;
        }
    }
    uwbContext.receivedEventId = dmEvent;
    if (uwbContext.currentEventId == dmEvent || (isCmdRespPending() && (uwbContext.dev_state == UWBD_STATUS_ERROR))
#if UWBIOT_UWBD_SR040
        || (uwbContext.dev_state == UWBD_STATUS_HDP_WAKEUP)
#endif
    ) {
        if (!skip_sem_post) {
            NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __FUNCTION__);
            uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
            (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
        }
    }
}
#if UWBFTR_DataTransfer
/*******************************************************************************
 **
 ** Function:        ufaDeviceManagementDataCallback
 **
 ** Description:     Receive device management response events from stack.
 **                  dmEvent: Device-management event ID.
 **                  eventData: Data associated with event ID.
 **
 ** Returns:         None
 **
 *******************************************************************************/

void ufaDeviceManagementDataCallback(uint8_t dpf, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;

    BOOLEAN skip_sem_post = FALSE;
    if ((len != 0) && (eventData != NULL)) {
        NXPLOG_UWBAPI_D("%s: enter; event=0x%X status %d", __FUNCTION__, dmEvent, *eventData);

        switch (dpf) {
        case UCI_DPF_RCV: /* Data Packet Format for receive data with message type 0 */
            dmEvent = UWA_DM_DATA_RCV_NTF_EVT;

            UWB_STREAM_TO_UINT32(uwbContext.rcvDataPkt.sessionHandle, eventData);
            UWB_STREAM_TO_UINT8(uwbContext.rcvDataPkt.status, eventData);
            UWB_STREAM_TO_ARRAY(uwbContext.rcvDataPkt.src_address, eventData, MAC_EXT_ADD_LEN);
            UWB_STREAM_TO_UINT16(uwbContext.rcvDataPkt.sequence_number, eventData);
            UWB_STREAM_TO_UINT16(uwbContext.rcvDataPkt.data_size, eventData);
            UWB_STREAM_TO_ARRAY(uwbContext.rcvDataPkt.data, eventData, uwbContext.rcvDataPkt.data_size);

            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_DATA_RCV_NTF, &uwbContext.rcvDataPkt);
            }
            break;
        default:
            NXPLOG_UWBAPI_E("%s : UWB Unknown dpf:%d", __FUNCTION__, dpf);
            break;
        }
    }
    uwbContext.receivedEventId = dmEvent;
    if (uwbContext.currentEventId == dmEvent || (isCmdRespPending() && (uwbContext.dev_state == UWBD_STATUS_ERROR))) {
        if (!skip_sem_post) {
            NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __FUNCTION__);
            uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
            (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
        }
    }
}
#endif // UWBFTR_DataTransfer
/*******************************************************************************
 **
 ** Function:        sep_SetWaitEvent
 **
 ** Description:     Update the current event ID in Context with given event ID
 **                  eventID:  event ID.
 **
 ** Returns:         None
 **
 *******************************************************************************/
void sep_SetWaitEvent(uint16_t eventID)
{
    uwbContext.currentEventId = eventID;
}

/*******************************************************************************
 **
 ** Function:        RawCommandResponse_Cb
 **
 ** Description:     Receive response from the stack for raw command sent from
 **                  UWB API.
 **                  gid: gid
 **                  event:  event ID.
 **                  param_len: length of the response
 **                  p_param: pointer to data
 **
 ** Returns:         None
 **
 *******************************************************************************/
static void rawCommandResponse_Cb(uint8_t gid, uint8_t event, uint16_t param_len, uint8_t *p_param)
{
    NXPLOG_UWBAPI_D(
        "NxpResponse_Cb Received length data = 0x%x status = 0x%x", param_len, p_param[UCI_RESPONSE_STATUS_OFFSET]);
    uwbContext.wstatus = UWBAPI_STATUS_FAILED;
    uwbContext.rsp_len = param_len;
    if (event == UWB_SEGMENT_PKT_SENT) {
        uwbContext.wstatus = UWBAPI_STATUS_PBF_PKT_SENT;
    }
    else if (param_len > 0 && p_param != NULL) {
        uwbContext.wstatus = p_param[UCI_RESPONSE_STATUS_OFFSET];
        phOsalUwb_MemCopy(uwbContext.rsp_data, p_param, param_len);
    }
    NXPLOG_UWBAPI_D("%s: posting devMgmtSem", __FUNCTION__);
    uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
    (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
}

/*******************************************************************************
 **
 ** Function:        sendRawUci
 **
 ** Description:     Internal function to Send Raw Command
 **
 ** Returns:         Status
 **
 *******************************************************************************/
tUWBAPI_STATUS sendRawUci(uint8_t *p_cmd_params, uint16_t cmd_params_len)
{
    uint8_t cmd_gid, cmd_oid, rsp_gid, rsp_oid;

    cmd_gid                 = p_cmd_params[0] & UCI_GID_MASK;
    cmd_oid                 = p_cmd_params[1] & UCI_OID_MASK;
    tUWBAPI_STATUS status;

    sep_SetWaitEvent(UWA_DM_UWBD_RESP_TIMEOUT_EVT);
    status = UWA_SendRawCommand(cmd_params_len, p_cmd_params, &rawCommandResponse_Cb);
    if (status == UCI_STATUS_OK) {
        /** Data transfer command using the SendRaw
         * uwb_cb.uci_cmd_window++; is required to unblock the Next Write.
         * uwbContext.wstatus = UWBAPI_STATUS_OK; is required as  uwbContext.wstatus is global context .
         * for data transfer did not recevive the any response hence make it success and safe exit.
         */
        if (p_cmd_params[0] == UCI_DPF_SND) {
            NXPLOG_UWBAPI_W("%s:For data transfer command no need to wait for response", __FUNCTION__);
            uwb_cb.uci_cmd_window++;
            uwbContext.wstatus = UWBAPI_STATUS_OK;
            status = UWBAPI_STATUS_OK;
            goto exit;
        }
        status = phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, 3000); // Wait for only 3000ms or 3sec
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: semaphore wait timed out", __FUNCTION__);
            status = UWBAPI_STATUS_TIMEOUT;
            goto exit;
        }
        NXPLOG_UWBAPI_D("%s: Success UWA_SendRawCommand", __FUNCTION__);
        status = uwbContext.wstatus;
    }
    else {
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

    if (uwbContext.wstatus == UWBAPI_STATUS_TIMEOUT) {
        status = UWBAPI_STATUS_TIMEOUT;
    }
    else if (uwbContext.wstatus == UWBAPI_STATUS_PBF_PKT_SENT) {
        status = UWBAPI_STATUS_PBF_PKT_SENT;
    }
    else if (uwbContext.wstatus == UWBAPI_STATUS_HPD_WAKEUP) {
        status = UWBAPI_STATUS_HPD_WAKEUP;
    }
#if UWBIOT_UWBD_SR040
    else if (status == UWBAPI_STATUS_LOW_POWER_ERROR) {
        status = UWBAPI_STATUS_LOW_POWER_ERROR;
        NXPLOG_UWBAPI_W("%s: Low Power Mode Error : Status : %d", __FUNCTION__, status);
    }
#endif // UWBIOT_UWBD_SR040
    else {
        rsp_gid = uwbContext.rsp_data[0] & UCI_GID_MASK;
        rsp_oid = uwbContext.rsp_data[1] & UCI_OID_MASK;
        if ((cmd_gid != rsp_gid) || (cmd_oid != rsp_oid)) {
            LOG_E(
                "Status = %d: Error, Received gid/oid other than what is sent, sent %x%x recv "
                "%x%x",
                status,
                cmd_gid,
                cmd_oid,
                rsp_gid,
                rsp_oid);
            status = UWBAPI_STATUS_FAILED;
        }
    }
exit :
    return status;
}

/*******************************************************************************
 **
 ** Function:        waitforNotification
 **
 ** Description:     waits for the notification for the specified event time out value.
 **                  waitEventId: Device-management event ID.
 **                  waitEventNtftimeout: Event associated time out value.
 **
 ** Returns:         status
 **
 *******************************************************************************/
tUWBAPI_STATUS waitforNotification(uint16_t waitEventId, uint32_t waitEventNtftimeout)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    /*
     * Check the waiting notification is already received or not.
     */
    if (uwbContext.receivedEventId != waitEventId) {
        /*
         *  Wait for the event notification
         */
        sep_SetWaitEvent(waitEventId);

        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, waitEventNtftimeout) == UWBSTATUS_SUCCESS) {
            status = UWBAPI_STATUS_OK;
        }
        else {
            /*
             * A scenario can happen when waiting for session status notification.
             * Session status notification comes prior to device status notification.
             * In that case, status to be set to ok since the notification is
             * already received. In any case, session state checking is done in the
             * Session init/deinit related API's.
             */
            if (UWA_DM_SESSION_STATUS_NTF_EVT == waitEventId) {
                status = UWBAPI_STATUS_OK;
            }
            else {
                NXPLOG_UWBAPI_E("%s: Wait timed-out for EventId: %d", __FUNCTION__, waitEventId);
            }
        }
    }
    else {
        status = UWBAPI_STATUS_OK;
    }
    /*
     * Reset the received event id to default event.
     */
    uwbContext.receivedEventId = DEFAULT_EVENT_TYPE;

    return status;
}

/*******************************************************************************
 **
 ** Function:        getAppConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for Application related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getAppConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = 0;

    tlvBuffer[length++] = paramId;

    if (paramLen > (MAX_UCI_PACKET_SIZE - 2 /* 1 byte of paramId + 1 byte of param_len */)) {
        LOG_E("%s, app config value len is greater than max uci payload size", __FUNCTION__);
        length = 0;
        return length;
    }

    switch (paramId) {
        /* Length 1 Byte */
    case UCI_PARAM_ID_DEVICE_ROLE:
    case UCI_PARAM_ID_RANGING_ROUND_USAGE:
    case UCI_PARAM_ID_STS_CONFIG:
    case UCI_PARAM_ID_MULTI_NODE_MODE:
    case UCI_PARAM_ID_CHANNEL_NUMBER:
    case UCI_PARAM_ID_NO_OF_CONTROLEES:
    case UCI_PARAM_ID_SESSION_INFO_NTF:
    case UCI_PARAM_ID_DEVICE_TYPE:
    case UCI_PARAM_ID_MAC_FCS_TYPE:
    case UCI_PARAM_ID_RANGING_ROUND_CONTROL:
    case UCI_PARAM_ID_AOA_RESULT_REQ:
    case UCI_PARAM_ID_RFRAME_CONFIG:
    case UCI_PARAM_ID_RSSI_REPORTING:
    case UCI_PARAM_ID_PREAMBLE_CODE_INDEX:
    case UCI_PARAM_ID_SFD_ID:
    case UCI_PARAM_ID_PSDU_DATA_RATE:
    case UCI_PARAM_ID_PREAMBLE_DURATION:
    case UCI_PARAM_ID_RANGING_TIME_STRUCT:
    case UCI_PARAM_ID_SLOTS_PER_RR:
#if (UWBIOT_UWBD_SR040)
    case UCI_PARAM_ID_TX_ADAPTIVE_PAYLOAD_POWER:
#endif // UWBIOT_UWBD_SR040
    case UCI_PARAM_ID_PRF_MODE:
    case UCI_PARAM_ID_SCHEDULED_MODE:
    case UCI_PARAM_ID_KEY_ROTATION:
    case UCI_PARAM_ID_KEY_ROTATION_RATE:
    case UCI_PARAM_ID_SESSION_PRIORITY:
    case UCI_PARAM_ID_MAC_ADDRESS_MODE:
    case UCI_PARAM_ID_NUMBER_OF_STS_SEGMENTS:
    case UCI_PARAM_ID_HOPPING_MODE:
    case UCI_PARAM_ID_IN_BAND_TERMINATION_ATTEMPT_COUNT:
    case UCI_PARAM_ID_RESULT_REPORT_CONFIG:
    case UCI_PARAM_ID_STS_LENGTH:
    case UCI_PARAM_ID_UL_TDOA_TX_TIMESTAMP:
    case UCI_PARAM_ID_BPRF_PHR_DATA_RATE:
    case UCI_PARAM_ID_BLOCK_STRIDING:
    case UCI_PARAM_ID_DLTDOA_RANGING_METHOD:
    case UCI_PARAM_ID_DLTDOA_TX_TIMESTAMP_CONF:
    case UCI_PARAM_ID_DLTDOA_INTER_CLUSTER_SYNC_PERIOD:
    case UCI_PARAM_ID_DLTDOA_ANCHOR_CFO:
    case UCI_PARAM_ID_DLTDOA_TX_ACTIVE_RANGING_ROUNDS:
    case UCI_PARAM_ID_DL_TDOA_BLOCK_STRIDING:
#if UWBFTR_CCC
    case UCI_PARAM_ID_CCC_CONFIG_QUIRKS:
    case UCI_PARAM_ID_PULSESHAPE_COMBO:
    case UCI_PARAM_ID_RESPONDER_LISTEN_ONLY:
    case UCI_PARAM_ID_RESPONDER_SLOT_INDEX :
#endif // UWBFTR_CCC
    case UCI_PARAM_ID_SUSPEND_RANGING_ROUNDS:
    case UCI_PARAM_ID_DLTDOA_TIME_REF_ANCHOR:
    case UCI_PARAM_ID_APPLICATION_DATA_ENDPOINT:
    case UCI_PARAM_ID_DL_TDOA_RESPONDER_TOF:
    case UCI_PARAM_ID_DATA_TRANSFER_STATUS_NTF_CONFIG: {
        tlvBuffer[length++] = 1; // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
#if UWBFTR_DataTransfer
    case UCI_PARAM_ID_LINK_LAYER_MODE:
    case UCI_PARAM_ID_DATA_REPETITION_COUNT:
#endif // UWBFTR_DataTransfer
    case UCI_PARAM_ID_MIN_FRAMES_PER_RR:
    case UCI_PARAM_ID_INTER_FRAME_INTERVAL: {
        tlvBuffer[length++] = 1; // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;

    /* Length 2 Bytes */
    case UCI_PARAM_ID_CAP_SIZE_RANGE: // Contention based ranging.
    case UCI_PARAM_ID_NEAR_PROXIMITY_CONFIG:
    case UCI_PARAM_ID_FAR_PROXIMITY_CONFIG:
    case UCI_PARAM_ID_SLOT_DURATION:
    case UCI_PARAM_ID_MAX_RR_RETRY:
    case UCI_PARAM_ID_VENDOR_ID:
#if UWBFTR_CCC
    case UCI_PARAM_ID_RANGING_PROTOCOL_VER:
    case UCI_PARAM_ID_UWB_CONFIG_ID:
    case UCI_PARAM_ID_URSK_TTL:
#endif // UWBFTR_CCC
    case UCI_PARAM_ID_MAX_NUMBER_OF_MEASUREMENTS:
    case UCI_PARAM_ID_MTU_SIZE: {
        tlvBuffer[length++] = 2; // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;

    /* Length 4 Byte */
    case UCI_PARAM_ID_UL_TDOA_TX_INTERVAL:
    case UCI_PARAM_ID_UL_TDOA_RANDOM_WINDOW:
    case UCI_PARAM_ID_STS_INDEX:
#if UWBFTR_CCC
    case UCI_PARAM_ID_LAST_STS_INDEX_USED:
    case UCI_PARAM_ID_HOP_MODE_KEY:
#endif // UWBFTR_CCC
    case UCI_PARAM_ID_SUB_SESSION_ID:
    case UCI_PARAM_ID_RANGING_DURATION: {
        tlvBuffer[length++] = 4; // Param len
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;

    /* Length Array of 1 Bytes */
#if !(UWBIOT_UWBD_SR040)
    case UCI_PARAM_ID_SESSION_TIME_BASE:
#endif
    case UCI_PARAM_ID_UL_TDOA_NTF_REPORT_CONFIG:
    case UCI_PARAM_ID_UL_TDOA_DEVICE_ID:
    case UCI_PARAM_ID_STATIC_STS_IV:
    case UCI_PARAM_ID_DEVICE_MAC_ADDRESS:
    case UCI_PARAM_ID_DST_MAC_ADDRESS:
    case UCI_PARAM_ID_UWB_INITIATION_TIME:
    case UCI_PARAM_ID_DLTDOA_ANCHOR_LOCATION:
    case UCI_PARAM_ID_SESSION_KEY:
    case UCI_PARAM_ID_SUB_SESSION_KEY:
    case UCI_PARAM_ID_AOA_BOUND_CONFIG: {
        uint8_t *value      = (uint8_t *)paramValue;
        tlvBuffer[length++] = paramLen; // Param len
        for (uint8_t i = 0; i < (paramLen / sizeof(uint8_t)); i++) {
            tlvBuffer[length++] = *value++;
        }
    } break;
    default:
        length = 0;
        break;
    }

    return length;
}

#if !(UWBIOT_UWBD_SR040)
/*******************************************************************************
 **
 ** Function:        getTestConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for test configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getTestConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = 0;

    tlvBuffer[length++] = paramId;

    switch (paramId) {
    /* Length 1 Byte */
    case UCI_TEST_PARAM_ID_RANDOMIZE_PSDU:
    case UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR:
    case UCI_TEST_PARAM_ID_PHR_RANGING_BIT: {
        tlvBuffer[length++] = 1; // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;

    /* Length 4 Byte */
    case UCI_TEST_PARAM_ID_NUM_PACKETS:
    case UCI_TEST_PARAM_ID_T_GAP:
    case UCI_TEST_PARAM_ID_T_START:
    case UCI_TEST_PARAM_ID_T_WIN:
    case UCI_TEST_PARAM_ID_RMARKER_TX_START:
    case UCI_TEST_PARAM_ID_RMARKER_RX_START: {
        tlvBuffer[length++] = 4; // Param len
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;
    default:
        length = 0;
        break;
    }
    return length;
}

/*******************************************************************************
 **
 ** Function:        VendorAppConfig_TlvParser
 **
 ** Description:     Application configuration Tlv parser for vendor specific params
 **
 ** Returns:         status
 **
 *******************************************************************************/
tUWBAPI_STATUS VendorAppConfig_TlvParser(
    const UWB_VendorAppParams_List_t *pAppParams_List, UWB_AppParams_value_au8_t *pOutput_param_value)
{
    tUWBAPI_STATUS status;
    uint8_t *param_value = pAppParams_List->param_value.au8.param_value;

    switch (pAppParams_List->param_type) {
    case kUWB_APPPARAMS_Type_u32:
        pOutput_param_value->param_len = 4;
        UWB_UINT32_TO_FIELD(pOutput_param_value->param_value, pAppParams_List->param_value.vu32);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_APPPARAMS_Type_au8:
        pOutput_param_value->param_len = pAppParams_List->param_value.au8.param_len;
        UWB_STREAM_TO_ARRAY(pOutput_param_value->param_value, param_value, pOutput_param_value->param_len);
        status = UWBAPI_STATUS_OK;
        break;
    default:
        status = UWBAPI_STATUS_FAILED;
        break;
    }
    return status;
}
#endif // !(UWBIOT_UWBD_SR040)

/*******************************************************************************
 **
 ** Function:        getCoreDeviceConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for Core Device configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
uint8_t getCoreDeviceConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = 0;
    if (paramValue == NULL || tlvBuffer == 0) {
        NXPLOG_UWBAPI_E("%s: Buffer is NULL", __FUNCTION__);
        return 0;
    }
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    case UCI_PARAM_ID_DEVICE_STATE:
    case UCI_PARAM_ID_LOW_POWER_MODE:
#if UWBIOT_UWBD_SR040
    case UCI_EXT_PARAM_ID_MHR_IN_CCM:
    case UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG_ENABLE:
#endif
    {
        tlvBuffer[length++] = 1; // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        if (value != ENABLED && value != DISABLED) {
            return 0;
        }
        tlvBuffer[length++] = value;
    } break;
    default:
        length = 0;
        break;
    }
    return length;
}

/*******************************************************************************
 **
 ** Function:        parseCoreGetDeviceConfigResponse
 **
 ** Description:     Convert received UCI response to deviceConfig object
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseCoreGetDeviceConfigResponse(uint8_t *tlvBuffer, phDeviceConfigData_t *devConfig)
{
    uint16_t paramId;
    UWB_STREAM_TO_UINT8(paramId, tlvBuffer);
    tlvBuffer++; // skipping the length
    switch (paramId) {
    /* 1 byte len */
    case UCI_PARAM_ID_LOW_POWER_MODE: {
        UWB_STREAM_TO_UINT8(devConfig->lowPowerMode, tlvBuffer);
    } break;
    }
}

/*******************************************************************************
 **
 ** Function:        getDeviceInfo
 **
 ** Description:     Gets Device Info from FW and sets in to context
 **
 ** Returns:         None
 **
 *******************************************************************************/
tUWBAPI_STATUS getDeviceInfo(void)
{
    tUWBAPI_STATUS status;
    sep_SetWaitEvent(UWA_DM_CORE_GET_DEVICE_INFO_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_CORE_GET_DEVICE_INFO_EVT, 0, NULL);

    return status;
}

/*******************************************************************************
 **
 ** Function:        getCapsInfo
 **
 ** Description:     Gets capability Info from FW and sets in to context
 **
 ** Returns:         status
 **
 *******************************************************************************/
tUWBAPI_STATUS getCapsInfo(void)
{
    tUWBAPI_STATUS status;
    sep_SetWaitEvent(UWA_DM_GET_CORE_DEVICE_CAP_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_CORE_GET_CAPS_INFO_EVT, 0, NULL);
    return status;
}

/*******************************************************************************
 **
 ** Function:        AppConfig_TlvParser
 **
 ** Description:     Application configuration Tlv parser
 **
 ** Returns:         status
 **
 *******************************************************************************/
tUWBAPI_STATUS AppConfig_TlvParser(
    const UWB_AppParams_List_t *pAppParams_List, UWB_AppParams_value_au8_t *pOutput_param_value)
{
    tUWBAPI_STATUS status;
    uint8_t *param_value = pAppParams_List->param_value.au8.param_value;

    switch (pAppParams_List->param_type) {
    case kUWB_APPPARAMS_Type_u32:
        pOutput_param_value->param_len = 4;
        UWB_UINT32_TO_FIELD(pOutput_param_value->param_value, pAppParams_List->param_value.vu32);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_APPPARAMS_Type_au8:
        pOutput_param_value->param_len = pAppParams_List->param_value.au8.param_len;
        UWB_STREAM_TO_ARRAY(pOutput_param_value->param_value, param_value, pOutput_param_value->param_len);
        status = UWBAPI_STATUS_OK;
        break;
    default:
        status = UWBAPI_STATUS_FAILED;
        break;
    }
    return status;
}

/*******************************************************************************
 **
 ** Function:        parseUwbSessionParams
 **
 ** Description:     Extracts All Sessions Data Parameters and updates the
 *structure
 **
 ** Returns:         status
 **
 *******************************************************************************/
tUWBAPI_STATUS parseUwbSessionParams(uint8_t *rspPtr, phUwbSessionsContext_t *pUwbSessionsContext)
{
    tUWBAPI_STATUS status;
    // Validation of all the parameters needs to be added.
    const uint8_t maxAvailableCount = pUwbSessionsContext->sessioncnt;
    if (maxAvailableCount == 0) {
        LOG_W("pUwbSessionsContext->sessioncnt is not set");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    else if (maxAvailableCount > 10) {
        LOG_W("Seems pUwbSessionsContext->sessioncnt is garbage");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    else {
        UWB_STREAM_TO_UINT8(pUwbSessionsContext->sessioncnt, rspPtr);

        if (maxAvailableCount < pUwbSessionsContext->sessioncnt) {
            LOG_W("Param Error: Not all Values returned for session. ");
            pUwbSessionsContext->sessioncnt = maxAvailableCount;
            pUwbSessionsContext->status     = kUWBSTATUS_BUFFER_TOO_SMALL;
        }

        for (uint8_t i = 0; i < pUwbSessionsContext->sessioncnt; i++) {
            UWB_STREAM_TO_UINT32(pUwbSessionsContext->pUwbSessionData[i].sessionHandle, rspPtr);
            UWB_STREAM_TO_UINT8(pUwbSessionsContext->pUwbSessionData[i].session_type, rspPtr);
            UWB_STREAM_TO_UINT8(pUwbSessionsContext->pUwbSessionData[i].session_state, rspPtr);
        }
        printUwbSessionData(pUwbSessionsContext);
        status = UWBAPI_STATUS_OK;
    }
exit:
    return status;
}

/**
 * \brief API to get all uwb sessions.
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetAllUwbSessions(phUwbSessionsContext_t *pUwbSessionsContext)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pUwbSessionsContext == NULL) {
        NXPLOG_UWBAPI_E("%s: UwbSessionsContext is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_GET_ALL_UWB_SESSIONS, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: GetAllUWBSessions successful", __FUNCTION__);
        /* rsp_data contains complete rsp, we have to skip Header */
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        UWB_STREAM_TO_UINT8(pUwbSessionsContext->status, rspPtr);

        if (pUwbSessionsContext->status == UWBAPI_STATUS_OK) {
            /*
             * Parse all the response parameters are correct or not.
             */
            status = parseUwbSessionParams(rspPtr, pUwbSessionsContext);
            if (status != UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_E("%s: parseUwbSessionParams failed", __FUNCTION__);
            }
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: GetAllUWBSessions Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: GetAllUWBSessions failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

/*******************************************************************************
 **
 ** Function:        parseCapabilityInfo
 **
 ** Description:     Parse Capability Information from the given buffer as per UCI
 **                  pDevCap:  Pointer to structure of device capability data
 **
 ** Returns:         boolean, failure when the length of the received param is incorrect,
 **                  success if the parsing is successful or an unknown Param is received.
 **
 *******************************************************************************/
BOOLEAN parseCapabilityInfo(phUwbCapInfo_t *pDevCap)
{
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    uint8_t paramId = 0;
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
    BOOLEAN status = TRUE;
    uint16_t index        = 0;
    uint8_t extParamId    = 0;
    uint8_t length        = 0;
    uint8_t capsInfoLen   = uwbContext.rsp_len;
    uint8_t *capsInfoData = uwbContext.rsp_data;
    if ((capsInfoLen == 0) || (capsInfoData == NULL)) {
        NXPLOG_UWBAPI_E("%s: capsInfoLen is zero or capsInfoData is NULL", __FUNCTION__);
        return FALSE;
    }

    while (index < capsInfoLen) {
        // Store Ext Param Id in case of 0xE0, 0xE1,..or Param Id in case of 0xA0, 0xA1,..0x01, 0x02,...
        extParamId = capsInfoData[index++];

        if ((extParamId & EXTENDED_PARAM_ID_MASK) == CCC_INFO_ID) {
            length = capsInfoData[index++];
#if UWBFTR_CCC
            if (parseCapabilityCCCParams(pDevCap, extParamId, &index, length, &capsInfoData[index]) == FALSE) {
                NXPLOG_UWBAPI_W("Error Parsing CCC Params.");
                status = FALSE;
            }
#else
            NXPLOG_UWBAPI_W("%s: unknown param Id 0x%X", __FUNCTION__, extParamId);
            index = (uint8_t)(index + length); // Skip CCC Params
#endif
        }
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        else if (extParamId == EXTENDED_CAP_INFO_ID) {
            paramId = capsInfoData[index++]; // store the Param Id
            length  = capsInfoData[index++];

            NXPLOG_UWBAPI_D("Param Id = %d\n", paramId);
            switch (paramId) {
            case UCI_EXT_PARAM_ID_UWBS_MAX_UCI_PAYLOAD_LENGTH: {
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->maxUciPayloadLength, &capsInfoData[index], length);
                index = (uint8_t)(index + length);
            } break;
            case UCI_EXT_PARAM_ID_UWBS_INBAND_DATA_BUFFER_BLOCK_SIZE: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->inbandDataBlockSize = capsInfoData[index++];
            } break;
            case UCI_EXT_PARAM_ID_UWBS_INBAND_DATA_MAX_BLOCKS: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->inbandDataMaxBlock = capsInfoData[index++];
            } break;
            default:
                NXPLOG_UWBAPI_W("%s: unknown Param Id : 0x%X", __FUNCTION__, paramId);
                index  = (uint8_t)(index + length);
            } // End Of Switch Case
        }
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
        // Skipping E0 to E9 CCC Parameters
        else if ((extParamId & EXTENDED_PARAM_ID_MASK) == CCC_EXT_PARAM_ID) {
            length = capsInfoData[index++];
            index  = (uint8_t)(index + length);
        }
        else {
            length = capsInfoData[index++];

            NXPLOG_UWBAPI_D("Param Id = %d\n", extParamId);
            switch (extParamId) {
            case MAX_MESSAGE_SIZE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->maxMessageSize, &capsInfoData[index], length);
#if UWBFTR_DataTransfer
                uwbContext.maxMessageSize = pDevCap->maxMessageSize;
#endif // UWBFTR_DataTransfer
                index = (uint8_t)(index + length);
            } break;
            case MAX_DATA_PACKET_PAYLOAD_SIZE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->maxDataPacketPayloadSize, &capsInfoData[index], length);
#if UWBFTR_DataTransfer
                uwbContext.maxDataPacketPayloadSize = pDevCap->maxDataPacketPayloadSize;
#endif // UWBFTR_DataTransfer
                index = (uint8_t)(index + length);
            } break;
            case FIRA_PHY_VERSION_RANGE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_4) {
                    return FALSE;
                }
                pDevCap->firaPhyLowerRangeMajorVersion             = capsInfoData[index++];
                pDevCap->firaPhyLowerRangeMinorMaintenanceVersion  = capsInfoData[index++];
                pDevCap->firaPhyHigherRangeMajorVersion            = capsInfoData[index++];
                pDevCap->firaPhyHigherRangeMinorMaintenanceVersion = capsInfoData[index++];
            } break;
            case FIRA_MAC_VERSION_RANGE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_4) {
                    return FALSE;
                }
                pDevCap->firaMacLowerRangeMajorVersion             = capsInfoData[index++];
                pDevCap->firaMacLowerRangeMinorMaintenanceVersion  = capsInfoData[index++];
                pDevCap->firaMacHigherRangeMajorVersion            = capsInfoData[index++];
                pDevCap->firaMacHigherRangeMinorMaintenanceVersion = capsInfoData[index++];
            } break;

            case DEVICE_ROLES_ID: {
#if UWBIOT_UWBD_SR040
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->deviceRoles = capsInfoData[index++];
#else  // !(UWBIOT_UWBD_SR040)
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->deviceRoles, &capsInfoData[index], length);
                index += length;
#endif // UWBIOT_UWBD_SR040
            } break;

            case RANGING_METHOD_ID: {
#if UWBIOT_UWBD_SR040
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->rangingMethod = capsInfoData[index++];
#else  // !(UWBIOT_UWBD_SR040)
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->rangingMethod, &capsInfoData[index], length);
                index += length;
#endif // UWBIOT_UWBD_SR040
            } break;

#if !(UWBIOT_UWBD_SR040)
            case DEVICE_TYPE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->deviceTypes = capsInfoData[index++];
            } break;
#endif //  !(UWBIOT_UWBD_SR040)

            case STS_CONFIG_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->stsConfig = capsInfoData[index++];
            } break;
            case MULTI_NODE_MODE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->multiNodeMode = capsInfoData[index++];
            } break;
            case RANGING_TIME_STRUCT_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->rangingTimeStruct = capsInfoData[index++];
            } break;
            case SCHEDULED_MODE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->scheduledMode = capsInfoData[index++];
            } break;
            case HOPPING_MODE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->hoppingMode = capsInfoData[index++];
            } break;
            case BLOCK_STRIDING_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->blockStriding = capsInfoData[index++];
            } break;
            case UWB_INITIATION_TIME_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->uwbInitiationTime = capsInfoData[index++];
            } break;
            case CHANNELS_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->channels = capsInfoData[index++];
            } break;
            case RFRAME_CONFIG_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->rframeConfig = capsInfoData[index++];
            } break;
            case CC_CONSTRAINT_LENGTH_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->ccConstraintLength = capsInfoData[index++];
            } break;
            case BPRF_PARAMETER_SETS_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->bprfParameterSets = capsInfoData[index++];
            } break;
            case HPRF_PARAMETER_SETS_ID: {
                if (length != DEVICE_CAPABILITY_LEN_5) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(pDevCap->hprfParameterSets, &capsInfoData[index], length);
                index = (uint8_t)(index + length);
            } break;
            case AOA_SUPPORT_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->aoaSupport = capsInfoData[index++];
            } break;
            case EXTENDED_MAC_ADDR_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->extendedMacAddress = capsInfoData[index++];
            } break;
#if !(UWBIOT_UWBD_SR040)
            case SUSPEND_RANGING_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->suspendRanging = capsInfoData[index++];
            } break;
            case SESSION_KEY_LEN_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->sessionKeyLen = capsInfoData[index++];
            } break;
            case DT_ANCHOR_MAX_ACTIVE_RR_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->ancorMaxRrActive = capsInfoData[index++];
            } break;
            case DT_TAG_MAX_ACTIVE_RR_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->tagMaxRrActive = capsInfoData[index++];
            } break;
            case DT_TAG_BLOCK_SKIPPING_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->tagBlockSkipping = capsInfoData[index++];
            } break;
            case PSDU_LENGTH_SUPPORT_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->psduLengthSupport = capsInfoData[index++];
            }break;
#endif // !(UWBIOT_UWBD_SR040)
            default:
                NXPLOG_UWBAPI_W("%s: unknown param Id 0x%X", __FUNCTION__, extParamId);
                index  = (uint8_t)(index + length);
            }
        }
    } // End Of While

    return status;
}
