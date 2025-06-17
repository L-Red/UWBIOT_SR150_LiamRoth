/*
 *
 * Copyright 2021,2022,2023 NXP.
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

#include "phUwb_BuildConfig.h"
#include "AppInternal.h"
#include "AppRecovery.h"

#include <uwb_board.h>
#include "UwbApi.h"
#include "UwbHif.h"
#include <SE_Wrapper.h>
#include "nxEnsure.h"
#include "ex_sss_boot.h"
#include "phOsalUwb.h"
#include "phNxpLogApis_App.h"

#ifndef UWBIOT_APP_BUILD__DEMO_FL_INITIATOR
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_FL_INITIATOR

#define DEMO_FIRALITE_TASK_SIZE 2048
#define DEMO_FIRALITE_TASK_NAME "firalite_initiator"
#define DEMO_FIRALITE_TASK_PRIO 4

#define DEMO_RANGING_APP_NO_OF_ANCHORS_P2P 1

#define DEMO_SE_STATE_AUTH_DONE             (0)
#define DEMO_SE_STATE_REMOTE_GET_DATA_DONE  (1)
#define DEMO_SE_STATE_REMOTE_PUT_DATA_DONE  (2)
#define DEMO_SE_STATE_WRAPPED_RDS_AVAILABLE (3)
#define DEMO_FIRALITE_WRAPPED_RDS_LEN       (68)
#define DEMO_FIRALITE_ADF_OID1                                     \
    {                                                              \
        0x60, 0x86, 0x48, 0x01, 0x86, 0xFF, 0x13, 0x01, 0x01, 0x01 \
    }

static const uint8_t gkDeviceMacAddrShort[2] = {0x22, 0x22};
static const uint8_t gkDSTMacAddrShort[2]    = {0x11, 0x11};

uint8_t gExtDeviceMacAddr[8] = {0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x1, 0x1};
uint8_t gExtDSTMacAddr[8]    = {0xd, 0x7, 0x6, 0x5, 0x4, 0x2, 0x2, 0xd};

#define DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT 0x0

AppContext_t appContext;
static ex_sss_boot_ctx_t ex_sss_boot_ctx;
#define PCONTEXT (&ex_sss_boot_ctx)

intptr_t mHifCommandQueue;
static UWBOSAL_TASK_HANDLE hifTaskHandle;
static uint8_t UWB_HandleTLV(phLibUwb_Message_t *tlv);

static se_status_t doGenAuthentication(uint8_t *pRespBuf, size_t *pRespSize);
static se_status_t doCallDispatch(uint8_t *pDispatchData, size_t dispatchDataLen, uint8_t *pRespBuf, size_t *pRespSize);
static se_status_t doCallRemoteGetData(uint8_t *pRespBuf, size_t *pRespSize);
static se_status_t doCallRemotePutData(uint8_t *pRespBuf, size_t *pRespSize);
static tUWBAPI_STATUS doStartRanging(void);

static uint8_t gState = 0;
uint8_t gWrappedRDS[DEMO_FIRALITE_WRAPPED_RDS_LEN];
uint16_t gWrappedRDSLen = sizeof(gWrappedRDS);

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Demo FiraLite Initiator");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    phOsalUwb_ThreadCreationParams_t threadparams;
    int pthread_create_status = 0;
    sss_status_t sss_status   = kStatus_SSS_Fail;
    char *portName;
    phLibUwb_Message_t evt;
    sss_se05x_session_t *sss_session = NULL;
    pSe05xSession_t pSeSession       = NULL;
    phUwbappContext_t appCtx         = {0};
    phDeviceConfigData_t devConfig;

    UWBSTATUS osalStatus = UWBSTATUS_FAILED;
    phOsalUwb_SetMemory(PCONTEXT, 0, sizeof(*PCONTEXT));
    uint8_t ret = 0;
#if defined(__linux__)
    uint8_t initID[3];
    /* Initiator identify command for server */
    initID[0] = 0xB0;
    initID[1] = 0x01;
    initID[2] = 0x01;
#endif
#if AX_EMBEDDED
    portName = NULL;
#else
    sss_status = ex_sss_boot_connectstring(0, NULL, &portName);
    if (kStatus_SSS_Success != sss_status) {
        LOG_E("ex_sss_boot_connectstring Failed");
        goto exit;
    }
#endif // AX_EMBEDDED

    PCONTEXT->se05x_open_ctx.skip_select_applet = TRUE;

    sss_status = ex_sss_boot_open(PCONTEXT, portName);

    if (kStatus_SSS_Success != sss_status) {
        LOG_E("ex_sss_boot_open Failed");
        goto exit;
    }

    sss_session = (sss_se05x_session_t *)&(PCONTEXT->session);

    pSeSession = &(sss_session->s_ctx);

    phOsalUwb_SetMemory(&appCtx, 0, sizeof(phUwbappContext_t));
#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = MAINLINE_FW;
    appCtx.pCallback         = AppCallback;

    appCtx.seHandle = (void *)pSeSession;

    status = UwbApi_Init_New(&appCtx);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_Init_New Failed");
        goto exit;
    }

    /* Disable the low power mode */
    devConfig.lowPowerMode = DISABLED;
    status                 = UwbApi_SetDeviceConfig(LOW_POWER_MODE, 1, &devConfig);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("Disabling Low power mode failed");
        goto exit;
    }

    status = UwbApi_PerformLocking();
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_PerformLocking Failed");
        goto exit;
    }

    /* Enable the low power mode */
    devConfig.lowPowerMode = ENABLED;
    status                 = UwbApi_SetDeviceConfig(LOW_POWER_MODE, 1, &devConfig);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("%s : UwbApi_SetDeviceConfig() failed");
        goto exit;
    }

    LOG_I("Initiator Starting OOB Session");

#if AX_EMBEDDED
    /* DeInitialize UART Debug instance */
    BOARD_DeinitDebugConsole();
    /* Enable the SWO Debug consle for only in debug session */
    if (IS_DEBUG_SESSION) {
        BOARD_InitSwoDebugConsole();
    }
#endif
    /*SET CDC MODE*/
    HifSetMode(kUWB_MODE_CDC);
#if defined(__linux__)
    HifInit(kUWB_COMM_Socket);
    UWB_Hif_SendRsp(initID, sizeof(initID));
#else
    HifInit(kUWB_COMM_Serial);
#endif

    threadparams.stackdepth = HIF_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, HIF_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = HIF_TASK_PRIORITY;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&hifTaskHandle, &UWB_HIFTask, &threadparams);
    if (0 != pthread_create_status) {
        LOG_E("Failed to create task %s", threadparams.taskname);
    }

    while (TRUE) {
        if (gState == DEMO_SE_STATE_WRAPPED_RDS_AVAILABLE) {
            /* FiraLite Operations completed move to ranging */
            break;
        }
        osalStatus = phOsalUwb_msgrcv(mHifCommandQueue, &evt, MAX_DELAY);
        if (osalStatus != UWBSTATUS_SUCCESS) {
            continue;
        }
        ret = UWB_HandleTLV(&evt);
        if (ret == 0) {
            /* SE Operation failed */
            LOG_E("SE Operation failed");
            goto exit;
        }
    }

    /* wait for python reader to close as HifDeInit writes data to uart */
    phOsalUwb_Delay(3000);
    phOsalUwb_Thread_Delete(hifTaskHandle);
    HifDeInit();
#if AX_EMBEDDED
    if (IS_DEBUG_SESSION) {
        BOARD_DeinitSwoDebugConsole();
    }
    BOARD_InitDebugConsole();
#endif
    status = doStartRanging();
exit:
    if (Se_API_DeInit() != SE_STATUS_OK) {
        LOG_E("Se_API_DeInit() Failed");
    }
    ex_sss_session_close(PCONTEXT);

    if (UwbApi_ShutDown() != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_ShutDown Failed");
    }
    if (status == UWBAPI_STATUS_TIMEOUT) {
        Handle_ErrorScenario(TIME_OUT);
    }
    UWBIOT_EXAMPLE_END(status);
}

static tUWBAPI_STATUS doStartRanging(void)
{
    uint32_t session_id    = 0;
    uint32_t sessionHandle = 0;

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    phRangingParams_t inRangingParams;

    se_rds_t seRds;
    firaLiteWrappedRds_t flWrappedRds;
    se_applet_t applet = SE_APPLET_FIRALITE;
    uint8_t wrappedRds[DEMO_FIRALITE_WRAPPED_RDS_LEN];
    size_t wrappedRdsLen  = DEMO_FIRALITE_WRAPPED_RDS_LEN;
    se_status_t se_status = SE_STATUS_NOT_OK;

    const UWB_AppParams_List_t SetAppParamsList[] = {

        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_RANGING_APP_NO_OF_ANCHORS_P2P),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gkDeviceMacAddrShort, MAC_SHORT_ADD_LEN),
    };

    flWrappedRds.wrappedRDS    = &gWrappedRDS[0];
    flWrappedRds.wrappedRDSLen = gWrappedRDSLen;
    seRds.pFlWrappedRds        = &flWrappedRds;

    se_status = Se_API_GetWrappedRDS(applet, &seRds, wrappedRds, &wrappedRdsLen);
    if (se_status != SE_STATUS_OK) {
        LOG_E("Se_API_GetWrappedRDS() failed");
        goto exit;
    }
    // TODO: Need to discuss.
    session_id = ((wrappedRds[0] << 24) & 0xFFFFFFFF);
    session_id |= ((wrappedRds[1] << 16) & 0xFFFFFFFF);
    session_id |= ((wrappedRds[2] << 8) & 0xFFFFFFFF);
    session_id |= (wrappedRds[3] & 0xFFFFFFFF);

    status = UwbApi_SessionInit(session_id, UWBD_RANGING_SESSION, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    inRangingParams.deviceRole        = kUWB_DeviceRole_Initiator;
    inRangingParams.multiNodeMode     = kUWB_MultiNodeMode_UniCast;
    inRangingParams.macAddrMode       = DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    inRangingParams.deviceType        = kUWB_DeviceType_Controller;
    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    inRangingParams.deviceMacAddr[0] = gkDSTMacAddrShort[0];
    inRangingParams.deviceMacAddr[1] = gkDSTMacAddrShort[1];

    status = UwbApi_SetRangingParams(sessionHandle, &inRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    status = demo_set_common_app_config(sessionHandle, kUWB_StsConfig_DynamicSts);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("demo_set_common_app_config() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigWrappedRDS(sessionHandle, wrappedRds, wrappedRdsLen);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetAppConfigWrappedRDS Failed");
        goto exit;
    }

    status = UwbApi_StartRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /* Delay 5 Mins for Ranging MILLISECONDS = MINUTES * 60 * 1000 */
    phOsalUwb_Delay(5 * 60 * 1000);

    status = UwbApi_StopRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_StopRangingSession() Failed");
        goto exit;
    }

    status = UwbApi_SessionDeinit(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SessionDeinit() Failed");
        goto exit;
    }
exit:
    return status;
}

static uint8_t UWB_HandleTLV(phLibUwb_Message_t *tlv)
{
    uint8_t ret                            = 0;
    uint8_t respBuffer[HIF_RESP_BUFF_SIZE] = {0};
    uint8_t respTagType                    = TLV_TYPE_END;
    bool returnResp                        = FALSE;

    uint8_t *data         = (uint8_t *)tlv->pMsgData;
    uint16_t dataLength   = (uint16_t)(tlv->Size);
    uint8_t *respPayload  = &respBuffer[3];
    size_t respSize       = sizeof(respBuffer) - 3;
    se_status_t se_status = SE_STATUS_NOT_OK;
    switch (tlv->eMsgType) {
    case TLV_TYPE_START: {
        se_status = doGenAuthentication(respPayload, &respSize);
        ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
        respTagType = SE_SELECT_APPLET;
        returnResp  = TRUE;
    } break;
    case SE_DISPATCH: {
        se_status = doCallDispatch(data, dataLength, respPayload, &respSize);
        ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
        respTagType = tlv->eMsgType;
        returnResp  = TRUE;
    } break;
    case UART_ECHO: {
        respBuffer[CMD_TYPE_OFFSET] = UART_ECHO;
        //respSize++;
        respBuffer[LSB_LENGTH_OFFSET] = (uint8_t)dataLength;
        respBuffer[MSB_LENGTH_OFFSET] = (uint8_t)(dataLength >> MSB_LENGTH_MASK);
        respSize                      = (uint16_t)(dataLength + HIF_RSP_HEADER_SIZE);
        memcpy(respPayload, data, dataLength);
        LOG_MAU8_I("Echo Received", respBuffer, respSize);
        respPayload[0] += 1;
        respPayload[1] += 1;
        LOG_MAU8_I("Echo Sending", respBuffer, respSize);
        UWB_Hif_SendRsp(respBuffer, respSize);
        ret        = TRUE;
        returnResp = FALSE;
    } break;
    default: {
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
    }
    if (returnResp) {
        ret = TRUE;
        if (gState == DEMO_SE_STATE_WRAPPED_RDS_AVAILABLE) {
            /* No need to send to remote device if Put data is done send START RANGING
            to responder */
            respBuffer[CMD_TYPE_OFFSET] = SE_START_RANGING;
            //respSize++;
            respBuffer[LSB_LENGTH_OFFSET] = (uint8_t)00;
            respBuffer[MSB_LENGTH_OFFSET] = (uint8_t)00;
            respSize                      = (uint16_t)HIF_RSP_HEADER_SIZE;
            UWB_Hif_SendRsp(respBuffer, respSize);
        }
        else {
            // Build Response
            respBuffer[CMD_TYPE_OFFSET] = respTagType; // TLV TYPE
            //respSize++;                                                             // Subtype
            respBuffer[LSB_LENGTH_OFFSET] = (uint8_t)(respSize);                    // PAYLOAD SIZE LSB
            respBuffer[MSB_LENGTH_OFFSET] = (uint8_t)(respSize >> MSB_LENGTH_MASK); // PAYLOAD SIZE MSB
            //respBuffer[CMD_SUB_TYPE_OFFSET] = (uint8_t)tlv->value[0];                 // TLV SUB TYPE then Payload follows

            // Add 4 Bytes Header size to respSize to send Over USB
            respSize = (uint16_t)(respSize + HIF_RSP_HEADER_SIZE);

            // Send Response Over Host interface
            UWB_Hif_SendRsp(respBuffer, respSize);
        }
    }
exit:
    return ret;
}

static se_status_t doGenAuthentication(uint8_t *pRespBuf, size_t *pRespSize)
{
    uint8_t pOid1[]                      = DEMO_FIRALITE_ADF_OID1;
    const se_firalite_optsa_t optsA      = se_firalite_optsa_symm_crypto;
    se_firelite_oid_entry oid_entries[1] = {{pOid1, sizeof(pOid1)}};
    const size_t oid_entries_count       = sizeof(oid_entries) / sizeof(se_firelite_oid_entry);
    se_firelite_selectadf_reponse_t selectAdfResponse;
    uint8_t cmdStatus;
    se_status_t se_status = SE_STATUS_NOT_OK;

    // initiator: open logical channel and select FiRaLite applet
    LOG_I("initiator: Open logical channel 1 and select FiRaLite applet");
    se_status = Se_API_Init(SE_APPLET_FIRALITE, SE_CHANNEL_1);
    ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);

    // initiator: select ADF
    LOG_I("initiator: select ADF");
    se_status = Se_API_SelectADF(optsA, oid_entries, oid_entries_count, &selectAdfResponse);
    ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);

    // initiator: initiate transaction
    LOG_I("initiator: initiate transaction");
    se_status = Se_API_InitiateTransaction(oid_entries, oid_entries_count, NULL, 0, &cmdStatus, pRespBuf, pRespSize);

    ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
    if (cmdStatus != FIRALITE_STATUS_COMMAND_PROCESSED_RETURN_TO_COUNTERPART_DEVICE) {
        se_status = SE_STATUS_NOT_OK;
    }

exit:
    return se_status;
}

static se_status_t doCallDispatch(uint8_t *pDispatchData, size_t dispatchDataLen, uint8_t *pRespBuf, size_t *pRespSize)
{
    uint8_t cmdStatus;
    uint8_t dispatchEventId;
    uint8_t pDispatchEventDataBuffer[FIRALITE_MAX_BUF_SIZE_RSP] = {
        0,
    };
    size_t dispatchEventDataLen = sizeof(pDispatchEventDataBuffer);
    se_status_t se_status;
    size_t dispatchRspSize = *pRespSize;

    se_status = Se_API_Dispatch(pDispatchData,
        dispatchDataLen,
        &cmdStatus,
        pRespBuf,
        &dispatchRspSize,
        &dispatchEventId,
        pDispatchEventDataBuffer,
        &dispatchEventDataLen);

    ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
    if (cmdStatus == FIRALITE_STATUS_COMMAND_PROCESSED_RETURN_TO_COUNTERPART_DEVICE) {
        //continue with counterpart device
        *pRespSize = dispatchRspSize;
        goto exit;
    }

    else if (cmdStatus == FIRALITE_STATUS_COMMAND_PROCESSED_RETURN_TO_HOST_APP) {
        memset(pRespBuf, 0x00, *pRespSize);
        switch (gState) {
        case DEMO_SE_STATE_AUTH_DONE:
            se_status = doCallRemoteGetData(pRespBuf, pRespSize);
            ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
            gState = DEMO_SE_STATE_REMOTE_GET_DATA_DONE;
            break;
        case DEMO_SE_STATE_REMOTE_GET_DATA_DONE:
            se_status = doCallRemotePutData(pRespBuf, pRespSize);
            ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
            gState = DEMO_SE_STATE_REMOTE_PUT_DATA_DONE;
            break;
        case DEMO_SE_STATE_REMOTE_PUT_DATA_DONE:
            /*start ranging*/
            LOG_MAU8_I("WRDS:", pDispatchEventDataBuffer, dispatchEventDataLen);
            if (dispatchEventDataLen <= DEMO_FIRALITE_WRAPPED_RDS_LEN) {
                gWrappedRDSLen = dispatchEventDataLen;
                memcpy(&gWrappedRDS[0], pDispatchEventDataBuffer, gWrappedRDSLen);
            }
            else {
                LOG_E("Invalid Wrapped RDS");
                return SE_STATUS_NOT_OK;
            }
            gState = DEMO_SE_STATE_WRAPPED_RDS_AVAILABLE;
            break;
        default:
            se_status = SE_STATUS_NOT_OK;
            break;
        }
    }
exit:
    return se_status;
}

static se_status_t doCallRemoteGetData(uint8_t *pRespBuf, size_t *pRespSize)
{
    uint8_t pRemoteGetDataCmd[] = {0x4D, 0x05, 0xBF, 0x70, 0x02, 0xA3, 0x80};

    uint8_t cmdStatus;
    se_status_t se_status;
    se_status = Se_API_RemoteGetData(pRemoteGetDataCmd, sizeof(pRemoteGetDataCmd), &cmdStatus, pRespBuf, pRespSize);
    ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
    if (cmdStatus != FIRALITE_STATUS_COMMAND_PROCESSED_RETURN_TO_COUNTERPART_DEVICE) {
        se_status = SE_STATUS_NOT_OK;
    }

exit:
    return se_status;
}

static se_status_t doCallRemotePutData(uint8_t *pRespBuf, size_t *pRespSize)
{
    uint8_t cmdStatus;
    uint8_t pRemotePutDataCmd[] = {
        0xBF,
        0x78, // tag TAG_UWB_SESSION_DATA_TRANSIENT
        0x02, // sub-tag length
        0x87, // sub-sub-tag TAG_MAKE_RDS_AVAILABLE
        0x00, // sub-sub-tag length
    };
    se_status_t se_status;
    se_status = Se_API_RemotePutData(pRemotePutDataCmd, sizeof(pRemotePutDataCmd), &cmdStatus, pRespBuf, pRespSize);

    ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
    if (cmdStatus != FIRALITE_STATUS_COMMAND_PROCESSED_RETURN_TO_COUNTERPART_DEVICE) {
        se_status = SE_STATUS_NOT_OK;
    }

exit:
    return se_status;
}

/*
 * Interface which will be called from Main to create the required task with its own parameters.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;
    threadparams.stackdepth   = DEMO_FIRALITE_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_FIRALITE_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_FIRALITE_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        LOG_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // UWBIOT_APP_BUILD__DEMO_FL_INITIATOR
