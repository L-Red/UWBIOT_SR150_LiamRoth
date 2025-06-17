/*
 *
 * Copyright 2021-2023 NXP.
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
#include "smCom.h"
#include "phOsalUwb.h"
#include "phNxpLogApis_App.h"
#include "fsl_sss_se05x_apis.h"

#ifndef UWBIOT_APP_BUILD__DEMO_FL_RESPONDER_IOT_CONCURRENCY
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_FL_RESPONDER_IOT_CONCURRENCY

#define DEMO_FIRALITE_IOT_TASK_SIZE   768
#define DEMO_FIRALITE_IOT_TASK_NAME   "firalite_responder_iot_concurrency"
#define DEMO_FIRALITE_IOT_TASK_PRIO   4
#define DEMO_FIRALITE_WRAPPED_RDS_LEN (68)

#define DEMO_RANGING_APP_NO_OF_ANCHORS_P2P 1

static const uint8_t gkDeviceMacAddrShort[2] = {0x22, 0x22};
static const uint8_t gkDSTMacAddrShort[2]    = {0x11, 0x11};

uint8_t gExtDeviceMacAddr[8] = {0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x1, 0x1};
uint8_t gExtDSTMacAddr[8]    = {0xd, 0x7, 0x6, 0x5, 0x4, 0x2, 0x2, 0xd};

#define DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT 0x0
#define DEMO_SE_STATE_WRAPPED_RDS_AVAILABLE        (3)

AppContext_t appContext;
static ex_sss_boot_ctx_t ex_sss_boot_ctx;
#define PCONTEXT            (&ex_sss_boot_ctx)
#define DEMO_CHANNEL_1_MASK (0x01)

intptr_t mHifCommandQueue;
static UWBOSAL_TASK_HANDLE hifTaskHandle;
static uint8_t UWB_HandleTLV(phLibUwb_Message_t *tlv, void *pSeHandle);

se_status_t doselectFiraLite(
    void *pSeHandle, uint8_t *pSelectData, size_t selectDataLen, uint8_t *pRespBuf, size_t *pRespSize);
se_status_t doCallDispatch(uint8_t *pDispatchData, size_t dispatchDataLen, uint8_t *pRespBuf, size_t *pRespSize);
static tUWBAPI_STATUS doStartRanging(void);

static uint8_t gState = 0;
uint8_t gWrappedRDS[DEMO_FIRALITE_WRAPPED_RDS_LEN];
uint16_t gWrappedRDSLen = sizeof(gWrappedRDS);

UWBOSAL_TASK_HANDLE demoTaskHandle1;
UWBOSAL_TASK_HANDLE demoTaskHandle2;
#define DEMO_SIMULT_ACCESS_TASK_SIZE 2048
#define DEMO_SIMULT_ACCESS_TASK_NAME "SimultaneousAccess"
#define DEMO_SIMULT_ACCESS_TASK_PRIO 3

static sss_status_t generateRandomNumber(uint8_t *rndData, size_t rndDataLen);

/* ************************************************************************** */
/* Function declarations                                                      */
/* ************************************************************************** */
static int create_thread(void **taskHandle, pphOsalUwb_ThreadFunction_t pThreadFunction, char *taskname);

OSAL_TASK_RETURN_TYPE uwbfiratask(void *args);
OSAL_TASK_RETURN_TYPE iottask(void *args);

tUWBAPI_STATUS gUwb_status_task1 = UWBAPI_STATUS_FAILED;
tUWBAPI_STATUS gUwb_status_task2 = UWBAPI_STATUS_FAILED;

int create_thread(void **taskHandle, pphOsalUwb_ThreadFunction_t pThreadFunction, char *taskname)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    int pthread_create_status = 0;
    threadparams.stackdepth   = DEMO_SIMULT_ACCESS_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, taskname);
    threadparams.pContext = NULL;
    threadparams.priority = 4;
    pthread_create_status = phOsalUwb_Thread_Create(taskHandle, pThreadFunction, &threadparams);
    if (0 != pthread_create_status) {
        LOG_E("Failed to create task %s", taskname);
    }
    return pthread_create_status;
}

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Demo FiraLite Responder IOT Concurrency");

    tUWBAPI_STATUS status     = UWBAPI_STATUS_FAILED;
    int pthread_create_status = 0;
    sss_status_t sss_status   = kStatus_SSS_Fail;
    char *portName;

    phOsalUwb_SetMemory(PCONTEXT, 0, sizeof(*PCONTEXT));

#if AX_EMBEDDED
    portName = NULL;
#else
    sss_status = ex_sss_boot_connectstring(0, NULL, &portName);
    if (kStatus_SSS_Success != sss_status) {
        LOG_E("ex_sss_boot_connectstring Failed");
        goto cleanup;
    }
#endif // AX_EMBEDDED

    sss_status = ex_sss_boot_open(PCONTEXT, portName);

    if (kStatus_SSS_Success != sss_status) {
        LOG_E("ex_sss_boot_open Failed");
        goto cleanup;
    }

    pthread_create_status = create_thread((void **)&demoTaskHandle1, &uwbfiratask, "UWB-TASK");
    ENSURE_OR_GO_CLEANUP(pthread_create_status == 0);

    pthread_create_status = create_thread((void **)&demoTaskHandle2, &iottask, "IOT-TASK");
    ENSURE_OR_GO_CLEANUP(pthread_create_status == 0);

    phOsalUwb_Thread_Join(demoTaskHandle1);
    phOsalUwb_Thread_Join(demoTaskHandle2);

    if ((gUwb_status_task1 == UWBAPI_STATUS_OK) && (gUwb_status_task2 == UWBAPI_STATUS_OK)) {
        status = UWBAPI_STATUS_OK;
    }
cleanup:
    ex_sss_session_close(PCONTEXT);
    UWBIOT_EXAMPLE_END(status);
}

OSAL_TASK_RETURN_TYPE uwbfiratask(void *args)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    phOsalUwb_ThreadCreationParams_t threadparams;
    //onst uint32_t session_id = RANGING_APP_SESSION_ID;
    sss_se05x_session_t *sss_session = (sss_se05x_session_t *)&(PCONTEXT->session);
    uint8_t ret                      = 0;
    UWBSTATUS osalStatus;
    int pthread_create_status = 0;
    phDeviceConfigData_t devConfig;

    phLibUwb_Message_t evt;
    pSe05xSession_t pSeSession = &(sss_session->s_ctx);

    phUwbappContext_t appCtx = {0};
    phOsalUwb_SetMemory(&appCtx, 0, sizeof(phUwbappContext_t));
#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = MAINLINE_FW;
    appCtx.pCallback         = AppCallback;

    appCtx.seHandle = (void *)pSeSession;

#if defined(__linux__)
    uint8_t respID[3];
    /* Responder identify command for server */
    respID[0] = 0xB0;
    respID[1] = 0x00;
    respID[2] = 0x00;
#endif

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

    LOG_I("Responder Starting OOB Session");

#if AX_EMBEDDED
    /* DeInitialize UART Debug console */
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
    UWB_Hif_SendRsp(respID, sizeof(respID));
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
            /* FiraLite Operation complete move to reanging */
            break;
        }
        osalStatus = phOsalUwb_msgrcv(mHifCommandQueue, &evt, MAX_DELAY);
        if (osalStatus != UWBSTATUS_SUCCESS) {
            continue;
        }
        ret = UWB_HandleTLV(&evt, pSeSession->conn_ctx);
        if (ret == 0) {
            /* SE Operation failed */
            LOG_E("SE Operation failed");
            goto exit;
        }
    }

    phOsalUwb_Thread_Delete(hifTaskHandle);
#if AX_EMBEDDED
    BOARD_DeinitSwoDebugConsole();
    BOARD_InitDebugConsole();
#endif

    status = doStartRanging();
exit:
    if (Se_API_DeInit() != SE_STATUS_OK) {
        LOG_E("Se_API_DeInit() Failed");
    }

    if (UwbApi_ShutDown() != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_ShutDown Failed");
    }
    if (status == UWBAPI_STATUS_TIMEOUT) {
        Handle_ErrorScenario(TIME_OUT);
    }
    phOsalUwb_Thread_Delete(demoTaskHandle1);
}

static uint8_t UWB_HandleTLV(phLibUwb_Message_t *tlv, void *pSeHandle)
{
    static uint8_t respBuffer[HIF_RESP_BUFF_SIZE];
    uint8_t respTagType   = TLV_TYPE_END;
    bool validPacket      = FALSE;
    uint8_t ret           = 0;
    se_status_t se_status = SE_STATUS_NOT_OK;
    uint8_t *data         = (uint8_t *)tlv->pMsgData;
    uint16_t dataLength   = (uint16_t)(tlv->Size);
    uint8_t *respPayload  = &respBuffer[3];
    size_t respSize       = HIF_RESP_BUFF_SIZE - 3;

    switch (tlv->eMsgType) {
    case SE_SELECT_APPLET: {
        se_status = doselectFiraLite(pSeHandle, data, dataLength, respPayload, &respSize);
        ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
        respTagType = SE_DISPATCH;
        validPacket = TRUE;
    } break;
    case SE_DISPATCH: {
        se_status = doCallDispatch(data, dataLength, respPayload, &respSize);
        ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
        respTagType = tlv->eMsgType;
        validPacket = TRUE;
    } break;
    case SE_START_RANGING: {
        gState = DEMO_SE_STATE_WRAPPED_RDS_AVAILABLE;
        HifDeInit();
        validPacket = FALSE;
        ret         = TRUE;
    } break;
    case UART_ECHO: {
        respBuffer[CMD_TYPE_OFFSET] = UART_ECHO;
        //respSize++;
        respBuffer[LSB_LENGTH_OFFSET] = (uint8_t)dataLength;
        respBuffer[MSB_LENGTH_OFFSET] = (uint8_t)(dataLength >> MSB_LENGTH_MASK);
        respSize                      = (uint16_t)(dataLength + HIF_RSP_HEADER_SIZE);
        memcpy(respPayload, data, dataLength);
        LOG_MAU8_I("Echo Received", respBuffer, respSize);
        respPayload[0] -= 1;
        respPayload[1] -= 1;
        LOG_MAU8_I("Echo Sending", respBuffer, respSize);
        UWB_Hif_SendRsp(respBuffer, respSize);
        ret = TRUE;
    } break;
    default: {
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
    }

    if (validPacket) {
        ret = TRUE;
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
exit:
    return ret;
}

se_status_t doselectFiraLite(
    void *pSeHandle, uint8_t *pSelectData, size_t selectDataLen, uint8_t *pRespBuf, size_t *pRespSize)
{
    static uint8_t pOpenChannelCmd[] = {0x00, 0x70, 0x00, 0x00, 0x01};
    smStatus_t sm_status;
    uint32_t RespDataLen  = 256;
    se_status_t se_status = SE_STATUS_NOT_OK;
    smStatus_t respstat   = SM_NOT_OK;

    // responder: open logical channel
    sm_status = (smStatus_t)smCom_TransceiveRaw(
        pSeHandle, pOpenChannelCmd, (U16)sizeof(pOpenChannelCmd), pRespBuf, (U32 *)&RespDataLen);

    ENSURE_OR_GO_EXIT(sm_status == SM_OK);
    sm_status = (pRespBuf[RespDataLen - 2] << 8) | (pRespBuf[RespDataLen - 1]);
    ENSURE_OR_GO_EXIT(sm_status == SM_OK);

    pSelectData[0U] |= DEMO_CHANNEL_1_MASK;
    respstat = (smStatus_t)smCom_TransceiveRaw(pSeHandle, pSelectData, (U16)selectDataLen, pRespBuf, (U32 *)pRespSize);
    ENSURE_OR_GO_EXIT(respstat == SM_OK);
    se_status = SE_STATUS_OK;

exit:
    return se_status;
}

se_status_t doCallDispatch(uint8_t *pDispatchData, size_t dispatchDataLen, uint8_t *pRespBuf, size_t *pRespSize)
{
    se_status_t se_status = SE_STATUS_NOT_OK;
    uint8_t cmdStatus;
    uint8_t dispatchEventId;
    uint8_t pDispatchEventDataBuffer[FIRALITE_MAX_BUF_SIZE_RSP] = {
        0,
    };
    size_t dispatchEventDataLen = sizeof(pDispatchEventDataBuffer);

    se_status = Se_API_Dispatch(pDispatchData,
        dispatchDataLen,
        &cmdStatus,
        pRespBuf,
        pRespSize,
        &dispatchEventId,
        pDispatchEventDataBuffer,
        &dispatchEventDataLen);

    ENSURE_OR_GO_EXIT(se_status == SE_STATUS_OK);
    se_status = SE_STATUS_NOT_OK;
    ENSURE_OR_GO_EXIT(cmdStatus == FIRALITE_STATUS_COMMAND_PROCESSED_RETURN_TO_COUNTERPART_DEVICE);
    se_status = SE_STATUS_OK;
    if (dispatchEventId == 2) {
        LOG_MAU8_I("Responder WRDS:", pDispatchEventDataBuffer, dispatchEventDataLen);
        if (dispatchEventDataLen <= DEMO_FIRALITE_WRAPPED_RDS_LEN) {
            gWrappedRDSLen = dispatchEventDataLen;
            memcpy(&gWrappedRDS[0], pDispatchEventDataBuffer, gWrappedRDSLen);
        }
        else {
            LOG_E("Invalid Wrapped RDS");
            se_status = SE_STATUS_NOT_OK;
        }
    }
exit:
    return se_status;
}

static tUWBAPI_STATUS doStartRanging(void)
{
    tUWBAPI_STATUS status  = UWBAPI_STATUS_FAILED;
    uint32_t session_id    = 0;
    uint32_t sessionHandle = 0;

    phRangingParams_t inRangingParams;

    se_rds_t seRds;
    firaLiteWrappedRds_t flWrappedRds;
    se_applet_t applet = SE_APPLET_FIRALITE;
    uint8_t wrappedRds[DEMO_FIRALITE_WRAPPED_RDS_LEN];
    size_t wrappedRdsLen = DEMO_FIRALITE_WRAPPED_RDS_LEN;
    se_status_t se_status;

    const UWB_AppParams_List_t SetAppParamsList[] = {

        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_RANGING_APP_NO_OF_ANCHORS_P2P),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gkDSTMacAddrShort, MAC_SHORT_ADD_LEN),
    };

    flWrappedRds.wrappedRDS    = &gWrappedRDS[0];
    flWrappedRds.wrappedRDSLen = gWrappedRDSLen;
    seRds.pFlWrappedRds        = &flWrappedRds;

    se_status = Se_API_GetWrappedRDS(applet, &seRds, wrappedRds, &wrappedRdsLen);
    if (se_status != SE_STATUS_OK) {
        LOG_E("Se_API_GetWrappedRDS() failed");
        goto exit;
    }
    // TODO: Need To Discuss.
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

    inRangingParams.deviceRole        = kUWB_DeviceRole_Responder;
    inRangingParams.multiNodeMode     = kUWB_MultiNodeMode_UniCast;
    inRangingParams.macAddrMode       = DEMO_RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    inRangingParams.deviceType        = kUWB_DeviceType_Controlee;
    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    inRangingParams.deviceMacAddr[0] = gkDeviceMacAddrShort[0];
    inRangingParams.deviceMacAddr[1] = gkDeviceMacAddrShort[1];

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
        Log("UwbApi_SessionDeinit() Failed");
        goto exit;
    }
exit:
    return status;
}

OSAL_TASK_RETURN_TYPE iottask(void *args)
{
    sss_status_t status = kStatus_SSS_Success;
    int itr             = 0;
    uint8_t rndData[32] = {0};
    size_t rndDataLen   = sizeof(rndData);

    for (itr = 0; itr <= 50; itr++) {
        status = generateRandomNumber(rndData, rndDataLen);
        if (status != kStatus_SSS_Success) {
            goto exit;
        }
    }

    LOG_I("IOT Task successfully completed");
    gUwb_status_task2 = UWBAPI_STATUS_OK;

exit:
    if (status != kStatus_SSS_Success) {
        gUwb_status_task2 = UWBAPI_STATUS_FAILED;
    }
    phOsalUwb_Thread_Delete(demoTaskHandle2);
}

static sss_status_t generateRandomNumber(uint8_t *rndData, size_t rndDataLen)
{
    sss_status_t status = kStatus_SSS_Fail;
    sss_rng_context_t rng;
    status = sss_rng_context_init(&rng, &PCONTEXT->session /* Session */);
    if (kStatus_SSS_Success != status) {
        LOG_E("Random Generator Init Context Failed!!!");
        goto exit;
    }
    status = sss_rng_get_random(&rng, rndData, rndDataLen);

    if (kStatus_SSS_Success == status) {
        LOG_MAU8_D("Random data from IOT applet", rndData, rndDataLen);
    }
    else {
        LOG_E("Get random failed!!");
    }

    sss_rng_context_free(&rng);
exit:
    return status;
}

/*
 * Interface which will be called from Main to create the required task with its own parameters.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;
    threadparams.stackdepth   = DEMO_FIRALITE_IOT_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_FIRALITE_IOT_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_FIRALITE_IOT_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        LOG_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_FL_RESPONDER_IOT_CONCURRENCY
