/*
 * Copyright 2012-2020,2022 NXP.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PHTMLUWB_H
#define PHTMLUWB_H

#include "phNxpUciHal_utils.h"
#include "phUwbCommon.h"
#include "phUwb_BuildConfig.h"
#include <uwb_uwbs_tml_interface.h>

/*
 * Message posted by Reader thread uponl
 * completion of requested operation
 */
#define PH_TMLUWB_READ_MESSAGE (0xAA)

/*
 * Message posted by Writer thread upon
 * completion of requested operation
 */
#define PH_TMLUWB_WRITE_MESSAGE (0x55)

/*
 * Value indicates to reset device
 */
#define PH_TMLUWB_RESETDEVICE (0x00008001)

/*
 * read write message post reentrance max wait timeout.
 */
#define PH_TML_UWB_MAX_MESSAGE_POST_WAIT (100)

/*
 * Reader Thread max wait timeout.
 */
#define PH_TML_UWB_MAX_READER_WAIT (10000)

/*
 * Writer Thread max wait timeout.
 */
#define PH_TML_UWB_MAX_WRITER_WAIT (100)
/*
***************************Globals,Structure and Enumeration ******************
*/

/*
 * Transaction (Tx/Rx) completion information structure of TML
 *
 * This structure holds the completion callback information of the
 * transaction passed from the TML layer to the Upper layer
 * along with the completion callback.
 *
 * The value of field wStatus can be interpreted as:
 *
 *     - UWBSTATUS_SUCCESS                    Transaction performed
 * successfully.
 *     - UWBSTATUS_FAILED                     Failed to wait on Read/Write
 * operation.
 *     - UWBSTATUS_INSUFFICIENT_STORAGE       Not enough memory to store data in
 * case of read.
 *     - UWBSTATUS_BOARD_COMMUNICATION_ERROR  Failure to Read/Write from the
 * file or timeout.
 */

typedef struct phTmlUwb_TransactInfo
{
    UWBSTATUS wStatus;     /* Status of the Transaction Completion*/
    uint8_t *pBuff;        /* Response Data of the Transaction*/
    uint16_t wLength;      /* Data size of the Transaction*/
} phTmlUwb_TransactInfo_t; /* Instance of Transaction structure */

/*
 * TML transreceive completion callback to Upper Layer
 *
 * pContext - Context provided by upper layer
 * pInfo    - Transaction info. See phTmlUwb_TransactInfo
 */
typedef void (*pphTmlUwb_TransactCompletionCb_t)(void *pContext, phTmlUwb_TransactInfo_t *pInfo);

/*
 * TML Deferred callback interface structure invoked by upper layer
 *
 * This could be used for read/write operations
 *
 * dwMsgPostedThread Message source identifier
 * pParams Parameters for the deferred call processing
 */
typedef void (*pphTmlUwb_DeferFuncPointer_t)(uint32_t dwMsgPostedThread, void *pParams);

/*
 * Structure containing details related to read and write operations
 *
 */
typedef struct phTmlUwb_ReadWriteInfo
{
    volatile uint8_t bEnable; /*This flag shall decide whether to perform
                               Write/Read operation */
    uint8_t bThreadBusy;      /*Flag to indicate thread is busy on respective operation */
    /* Transaction completion Callback function */
    pphTmlUwb_TransactCompletionCb_t pThread_Callback;
    void *pContext;        /*Context passed while invocation of operation */
    uint8_t *pBuffer;      /*Buffer passed while invocation of operation */
    uint16_t wLength;      /*Length of data read/written */
    UWBSTATUS wWorkStatus; /*Status of the transaction performed */
} phTmlUwb_ReadWriteInfo_t;

/*
 * TML uci rx packet callback, In special cases application can register this to skip uci stack
 * to recieve uci rx bytestream in application
 * recvDataLen - data pkt length
 * recvData    - recv data pkt buffer
 */
typedef void (*pphTmlUwb_AppDataCb_t)(uint8_t *recvData, uint16_t recvDataLen);
/*
 *Base Context Structure containing members required for entire session
 */
typedef struct phTmlUwb_Context
{
    UWBOSAL_TASK_HANDLE readerThread;
    volatile uint8_t bThreadDone;       /*Flag to decide whether to run or abort the thread */
    phTmlUwb_ReadWriteInfo_t tReadInfo; /*Pointer to Reader Thread Structure */
    void *pDevHandle;                   /* Pointer to Device Handle */
    uintptr_t dwCallbackThreadId;       /* Thread ID to which message to be posted */
    uint8_t bEnableCrc;                 /*Flag to validate/not CRC for input buffer */
    void *rxSemaphore;
    void *postMsgSemaphore;                /* Semaphore to post message atomically by Reader &
                                             writer thread */
    pphTmlUwb_AppDataCb_t appDataCallback; /*callback to be invoked on uci rx*/
} phTmlUwb_Context_t;

/*
 * TML Configuration exposed to upper layer.
 */
typedef struct phTmlUwb_Config
{
    /* Callback Thread ID
     * This is the thread ID on which the Reader & Writer thread posts message. */
    uintptr_t dwGetMsgThreadId;
    /* Communication speed between DH and SR100
     * This is the baudrate of the bus for communication between DH and SR100 */
    uint32_t dwBaudRate;
} phTmlUwb_Config_t, *pphTmlUwb_Config_t; /* pointer to phTmlUwb_Config_t */

/*
 * TML Deferred Callback structure used to invoke Upper layer Callback function.
 */
typedef struct
{
    /* Deferred callback function to be invoked */
    pphTmlUwb_DeferFuncPointer_t pDef_call;
    /* Source identifier
   *
   * Identifier of the source which posted the message
   */
    uint32_t dwMsgPostedThread;
    /** Actual Message
   *
   * This is passed as a parameter passed to the deferred callback function
   * pDef_call. */
    void *pParams;
} phTmlUwb_DeferMsg_t; /* DeferMsg structure passed to User Thread */

/* Function declarations */
UWBSTATUS phTmlUwb_Init(pphTmlUwb_Config_t pConfig);
UWBSTATUS phTmlUwb_Shutdown(void);
UWBSTATUS phTmlUwb_Write(uint8_t *p_data, uint16_t data_len);
UWBSTATUS phTmlUwb_Read(
    uint8_t *pBuffer, uint16_t wLength, pphTmlUwb_TransactCompletionCb_t pTmlReadComplete, void *pContext);
void phTmlUwb_WriteAbort(void);
void phTmlUwb_ReadAbort(void);
void phTmlUwb_DeferredCall(uintptr_t dwThreadId, phLibUwb_Message_t *ptWorkerMsg);
#if UWBIOT_UWBD_SR2XXT
void phTmlUwb_Chip_Reset(void);
#endif
void phTmlUwb_processTmlRead(uint8_t *tmlReadData, uint16_t tmlReadDataLen);
#endif /*  PHTMLUWB_H  */

void phTmlUwb_suspendReader();
void phTmlUwb_resumeReader();