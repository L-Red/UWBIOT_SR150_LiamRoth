/*
 *
 * Copyright 2021,2022-2023 NXP.
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

#include <stdio.h>
#include <AppInternal.h>
#include "se_vcom.h"
#include "usart.h"
#include <board.h>
#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160
#include <sm_types.h>
#include "phNxpEse_Internal.h"
#include "smComT1oI2C.h"
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

#if UWBFTR_SE_SN110
#include "SeApi.h"
#include "wearable_platform_int.h"
#endif // UWBFTR_SE_SN110

#ifdef FLOW_VERBOSE
#define NX_LOG_ENABLE_SMCOM_DEBUG 1
#endif

#ifndef UWBIOT_APP_BUILD__SE_VCOM
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__SE_VCOM

#define HIF_TASK_PRIO        3
#define PNP_APP_TASK_PRIO    3
#define UCI_READER_TASK_PRIO 1
#define HIF_WRITER_TASK_PRIO 2

#define DEMO_VCOM_TASK_SIZE 2048
#define DEMO_VCOM_TASK_NAME "VCOM"
#define DEMO_VCOM_TASK_PRIO 4

#ifndef ERR_COMM_ERROR
#define ERR_COMM_ERROR (0x7003) //!< Generic communication error
#endif

static uint8_t g_sendresp_vcom        = 0;
static uint8_t s_FrameParseInProgress = 0;
static uint16_t curr_index            = 0;

uint32_t selectResponseDataLen               = 0;
static uint8_t selectResponseData[2048 + 20] = {0};
static uint8_t targetBuffer[2048 + 20]       = {0};
uint16_t targetBufferLen                     = 0;
static uint32_t nExpectedPayload             = 0;
static uint8_t s_RecvBuff[2048 + 10]         = {0};
#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160
extern phNxpEse_Context_t gnxpese_ctxt;
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)

// #if UWBFTR_SE_SN110
// int8_t GeneralState = 0;
// #endif // UWBFTR_SE_SN110

uint16_t vcomPackageApduResponse(uint8_t messageType,
    uint8_t nodeAddress,
    uint8_t *payload,
    uint16_t payloadLen,
    uint8_t *targetBuf,
    uint16_t *targetBufLen);
uint16_t SM_SendAPDUVcom(uint8_t *cmd, uint16_t cmdLen, uint8_t *resp, uint16_t *respLen);

uint16_t vcomPackageApduResponse(uint8_t messageType,
    uint8_t nodeAddress,
    uint8_t *payload,
    uint16_t payloadLen,
    uint8_t *targetBuf,
    uint16_t *targetBufLen)
{
    if (*targetBufLen < (4 + payloadLen)) {
        return RJCT_ARG_FAIL;
    }

    targetBuf[0] = messageType;
    targetBuf[1] = nodeAddress;
    targetBuf[2] = (payloadLen >> 8) & 0x00FF;
    targetBuf[3] = payloadLen & 0x00FF;
    memcpy(&targetBuf[4], payload, payloadLen);
    *targetBufLen = 4 + payloadLen;
    return RJCT_OK;
}

/*USB header is handled here as per designed doc*/
void Handle_TLV(uint8_t *buffer, uint32_t length)
{
    uint16_t sw          = 0x9000;
    uint16_t statusValue = 0;

    if (s_FrameParseInProgress == 0) {
        nExpectedPayload = (buffer[2] << 8) + buffer[3];
    }

    if (nExpectedPayload > ((length) + curr_index - 4)) {
        s_FrameParseInProgress = 1;
        memcpy(&s_RecvBuff[curr_index], buffer, (length));
        curr_index += (length);
    }
    else {
        s_FrameParseInProgress = 0;
        memcpy(&s_RecvBuff[curr_index], buffer, (length));
        curr_index += (length);
    }

    if ((curr_index == (nExpectedPayload + 4)) && (s_FrameParseInProgress == 0)) {
        curr_index = 0;
        switch (buffer[0]) {
        case WAIT_FOR_CARD: {
            uint8_t Atr[64];
            uint16_t AtrLen = sizeof(Atr);

            AtrLen = sizeof(Atr);
#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160
            sw = smComT1oI2C_Open(NULL, 0x00, 0x00, Atr, &AtrLen);
            if (sw == SW_OK) {
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#if UWBFTR_SE_SN110
                SeApi_WiredEnable(TRUE);
                sw = SeApi_WiredGetAtr(Atr, &AtrLen);
                if (sw == SEAPI_STATUS_OK) {
#endif
                    targetBufferLen = sizeof(targetBuffer);
                    statusValue     = vcomPackageApduResponse(0x00, 0x00, Atr, AtrLen, targetBuffer, &targetBufferLen);
                    if (statusValue == RJCT_OK) {
                        memcpy(selectResponseData, targetBuffer, targetBufferLen);
                        selectResponseDataLen = targetBufferLen;
                        g_sendresp_vcom       = 1;
                    }
                }
                else {
                    Atr[0]          = 0x69;
                    Atr[1]          = 0x82;
                    AtrLen          = 2;
                    targetBufferLen = sizeof(targetBuffer);
                    statusValue     = vcomPackageApduResponse(0x00, 0x00, Atr, AtrLen, targetBuffer, &targetBufferLen);
                    if ((statusValue == RJCT_OK) && (targetBufferLen <= sizeof(selectResponseData))) {
                        memcpy(selectResponseData, targetBuffer, targetBufferLen);
                        selectResponseDataLen = targetBufferLen;
                        g_sendresp_vcom       = 1;
                    }
                }
            }
            break;
        case APDU_DATA:
            if (s_RecvBuff[4] == 0xFF) {
                uint32_t time_ms;
                time_ms = (s_RecvBuff[6] << 24);
                time_ms |= (s_RecvBuff[7] << 16);
                time_ms |= (s_RecvBuff[8] << 8);
                time_ms |= s_RecvBuff[9];
#if SSS_HAVE_SE05X || SSS_HAVE_LOOPBACK
#if FSL_FEATURE_SOC_PIT_COUNT > 0
                se_pit_SetTimer(time_ms);
                g_sendresp_vcom = 1;
#endif
#endif
            }
            else {
                state_vcom_2_i2c();
            }
            break;
        case CLOSE_CONN:
#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160
            sw = smComT1oI2C_Close(&gnxpese_ctxt, 0);
            if (sw == SW_OK) {
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#if UWBFTR_SE_SN110
                /* Implement 	5. SeApi_WiredEnable(FALSE) */
                sw = SeApi_WiredEnable(FALSE);
                if (sw == SEAPI_STATUS_OK) {
#endif
                    targetBufferLen = sizeof(targetBuffer);
                    statusValue     = vcomPackageApduResponse(
                        CLOSE_CONN, 0x00, (uint8_t *)&sw, sizeof(sw), targetBuffer, &targetBufferLen);
                    if (statusValue == RJCT_OK) {
                        memcpy(selectResponseData, targetBuffer, targetBufferLen);
                        selectResponseDataLen = targetBufferLen;
                        g_sendresp_vcom       = 1;
                    }
                }
                else {
                    uint8_t resp[2];
                    uint16_t respLen = sizeof(resp);
                    resp[0]          = 0x69;
                    resp[1]          = 0x82;
                    targetBufferLen  = sizeof(targetBuffer);
                    statusValue =
                        vcomPackageApduResponse(CLOSE_CONN, 0x00, resp, respLen, targetBuffer, &targetBufferLen);
                    if (statusValue == RJCT_OK) {
                        memcpy(selectResponseData, targetBuffer, targetBufferLen);
                        selectResponseDataLen = targetBufferLen;
                        g_sendresp_vcom       = 1;
                    }
                }
                break;
            default:
                break;
            }
        }

            if (g_sendresp_vcom) {
                g_sendresp_vcom = 0;
                //send data to usart
                statusValue = transmitToUsart_vcom(selectResponseData, selectResponseDataLen);
            }
            else if (s_FrameParseInProgress == 1) {
                // Continue sending data to usart
                statusValue = transmitToUsart_vcom(NULL, 0);
            }
        }

        uint16_t SM_SendAPDUVcom(uint8_t * cmd, uint16_t cmdLen, uint8_t * resp, uint16_t * respLen)
        {
            uint32_t status       = 0;
            uint32_t respLenLocal = *respLen;
#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160
            status = smCom_TransceiveRaw(NULL, cmd, cmdLen, resp, &respLenLocal);
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#if UWBFTR_SE_SN110
            status = SeApi_WiredTransceive(cmd, cmdLen, resp, 255, (uint16_t *)&respLenLocal, 10000);
#endif
            *respLen = (uint16_t)respLenLocal;
            return (uint16_t)status;
        }

        void state_vcom_2_i2c()
        {
            uint16_t sw;
            uint16_t uint16_tRespLen;
            memset(selectResponseData, 0, sizeof(selectResponseData));
            selectResponseDataLen = sizeof(selectResponseData) - 4;
            uint16_tRespLen       = (uint16_t)selectResponseDataLen;
            sw = SM_SendAPDUVcom(&s_RecvBuff[4], nExpectedPayload, selectResponseData, &uint16_tRespLen);
            selectResponseDataLen = uint16_tRespLen;
#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160
            if (sw == SW_OK) {
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#if UWBFTR_SE_SN110
                if (sw == SEAPI_STATUS_OK) {
#endif
                    targetBufferLen = sizeof(targetBuffer);
                    sw              = vcomPackageApduResponse(
                        APDU_DATA, 0x00, selectResponseData, selectResponseDataLen, targetBuffer, &targetBufferLen);
                    if ((sw == RJCT_OK) && (targetBufferLen <= sizeof(selectResponseData))) {
                        memcpy(selectResponseData, targetBuffer, targetBufferLen);
                        selectResponseDataLen = targetBufferLen;
                        g_sendresp_vcom       = 1;
                    }
                }
                else {
                    uint16_t statusValue = 0;
                    uint8_t resp[2];
                    uint16_t respLen = sizeof(resp);
                    resp[0]          = ERR_COMM_ERROR >> 8;
                    resp[1]          = ERR_COMM_ERROR & 0xFF;
                    targetBufferLen  = sizeof(targetBuffer);
                    statusValue =
                        vcomPackageApduResponse(APDU_DATA, 0x00, resp, respLen, targetBuffer, &targetBufferLen);
                    if ((statusValue == RJCT_OK) && (targetBufferLen <= sizeof(selectResponseData))) {
                        memcpy(selectResponseData, targetBuffer, targetBufferLen);
                        selectResponseDataLen = targetBufferLen;
                        g_sendresp_vcom       = 1;
                    }
                }
            }

            OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
            {
#if defined(QN9090DK6) || defined(NORDIC_MCU)
                /* DeInitialize UART Debug instance */
                BOARD_DeinitDebugConsole();
                /* Enable the SWO Debug consle for only in debug session */
                if (IS_DEBUG_SESSION) {
                    BOARD_InitSwoDebugConsole();
                }
#endif
#if defined(CPU_MIMXRT1176DVMAA)
                BOARD_DeInitUsbDebugConsole();
                BOARD_InitDebugConsole();
#endif
                PRINT_APP_NAME("Demo VCOM");
                USART_Initialize(&Handle_TLV);
#if UWBFTR_SE_SN110
                SeApi_Init(NULL, NULL);
#endif
                while (1) {
                    USART_RX_Data();
                }
            }

            /*
 * @brief   Application entry point.
 */
            UWBOSAL_TASK_HANDLE uwb_demo_start(void)
            {
                phOsalUwb_ThreadCreationParams_t threadparams;
                UWBOSAL_TASK_HANDLE taskHandle;
                int pthread_create_status = 0;
                threadparams.stackdepth   = DEMO_VCOM_TASK_SIZE;
                PHOSALUWB_SET_TASKNAME(threadparams, DEMO_VCOM_TASK_NAME);
                threadparams.pContext = NULL;
                threadparams.priority = DEMO_VCOM_TASK_PRIO;
                pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
                if (0 != pthread_create_status) {
                    NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
                }
                return taskHandle;
            }

#endif //UWBIOT_APP_BUILD__SE_VCOM
