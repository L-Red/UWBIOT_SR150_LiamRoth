/*
 * Copyright 2012,2021-2024 NXP.
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
#include <ctype.h>
#include "phTmlUwb.h"
#include "UwbCoreSDK_Internal.h"
#include "phNxpUciHal.h"
#include "phNxpUciHal_utils.h"

#include "phOsalUwb.h"
#include "phTmlUwb_transport.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phUwb_BuildConfig.h"
#include "phUwbTypes.h"

extern phNxpUciHal_Control_t nxpucihal_ctrl;

/* Value to reset variables of TML  */
#define PH_TMLUWB_RESET_VALUE (0x00)

/* Indicates a Initial or offset value */
#define PH_TMLUWB_VALUE_ONE         (0x01)
#define TML_READ_WRITE_SYNC_TIMEOUT (10)

/* Initialize Context structure pointer used to access context structure */
phTmlUwb_Context_t *gpphTmlUwb_Context = NULL;

/* Local Function prototypes */
static UWBSTATUS phTmlUwb_StartThread(void);
static void phTmlUwb_CleanUp(void);
static void phTmlUwb_ReadDeferredCb(void *pParams);

static OSAL_TASK_RETURN_TYPE phTmlUwb_TmlReaderThread(void *pParam);

/* for debugging purpose, print log messages from FW */
#define SR040_PRINT_DEBUG_LOG_MESSAGES 1
#if UWBIOT_UWBD_SR040
/* Extended handling to print debug messages from SR040 */
static void phTmlUwb_PrintRecevedMessage(const uint8_t *const pBuffer, const uint16_t wLength);
#endif
/* Function definitions */

/*******************************************************************************
**
** Function         phTmlUwb_Init
**
** Description      Provides initialization of TML layer and hardware interface
**                  Configures given hardware interface and sends handle to the
**                  caller
**
** Parameters       pConfig - TML configuration details as provided by the upper
**                            layer
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - initialization successful
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_FAILED - initialization failed (for example,
**                                     unable to open hardware interface)
**                  UWBSTATUS_INVALID_DEVICE - device has not been opened or has
**                                             been disconnected
**
*******************************************************************************/
UWBSTATUS phTmlUwb_Init(pphTmlUwb_Config_t pConfig)
{
    UWBSTATUS wInitStatus = UWBSTATUS_SUCCESS;

    /* Check if TML layer is already Initialized */
    if (NULL != gpphTmlUwb_Context) {
        /* TML initialization is already completed */
        wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_ALREADY_INITIALISED);
    }
    /* Validate Input parameters */
    else if ((NULL == pConfig) || (PH_TMLUWB_RESET_VALUE == pConfig->dwGetMsgThreadId)) {
        /*Parameters passed to TML init are wrong */
        wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Allocate memory for TML context */
        gpphTmlUwb_Context = (phTmlUwb_Context_t *)phOsalUwb_GetMemory(sizeof(phTmlUwb_Context_t));

        if (NULL == gpphTmlUwb_Context) {
            wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
        }
        else {
            /* Initialise all the internal TML variables */
            phOsalUwb_SetMemory(gpphTmlUwb_Context, PH_TMLUWB_RESET_VALUE, sizeof(phTmlUwb_Context_t));
            /* Make sure that the thread runs once it is created */
            gpphTmlUwb_Context->bThreadDone = 1;

            /* Open the device file to which data is read/written */
            wInitStatus = phTmlUwb_open_and_configure(pConfig, &(gpphTmlUwb_Context->pDevHandle));

            if (UWBSTATUS_SUCCESS != wInitStatus) {
                wInitStatus                    = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_DEVICE);
                gpphTmlUwb_Context->pDevHandle = NULL;
            }
            else {
                gpphTmlUwb_Context->tReadInfo.bEnable     = 0;
                gpphTmlUwb_Context->tReadInfo.bThreadBusy = FALSE;

                if (0 != phOsalUwb_CreateSemaphore(&gpphTmlUwb_Context->rxSemaphore, 0)) {
                    wInitStatus = UWBSTATUS_FAILED;
                }
                else if (0 != phOsalUwb_CreateSemaphore(&gpphTmlUwb_Context->postMsgSemaphore, 0)) {
                    wInitStatus = UWBSTATUS_FAILED;
                }
                else {
                    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->postMsgSemaphore);
                    /* Start TML thread (to handle write and read operations) */
                    if (UWBSTATUS_SUCCESS != phTmlUwb_StartThread()) {
                        wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
                    }
                    else {
                        /* Store the Thread Identifier to which Message is to be posted */
                        gpphTmlUwb_Context->dwCallbackThreadId = pConfig->dwGetMsgThreadId;
                    }
                }
            }
        }
    }
    /* Clean up all the TML resources if any error */
    if (UWBSTATUS_SUCCESS != wInitStatus) {
        /* Clear all handles and memory locations initialized during init */
        phTmlUwb_CleanUp();
    }

    return wInitStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_StartThread
**
** Description      Initializes comport, reader and writer threads
**
** Parameters       None
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - threads initialized successfully
**                  UWBSTATUS_FAILED - initialization failed due to system error
**
*******************************************************************************/
static UWBSTATUS phTmlUwb_StartThread(void)
{
    UWBSTATUS wStartStatus = UWBSTATUS_SUCCESS;
    phOsalUwb_ThreadCreationParams_t threadparams;
    int pthread_create_status = 0;

    /* Create Reader and Writer threads */
    threadparams.stackdepth = TMLREADER_STACK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, "TMLREAD");
    threadparams.pContext = NULL;
    threadparams.priority = TMLREADER_PRIO;
    pthread_create_status =
        phOsalUwb_Thread_Create((void **)&gpphTmlUwb_Context->readerThread, &phTmlUwb_TmlReaderThread, &threadparams);
    if (0 != pthread_create_status) {
        wStartStatus = UWBSTATUS_FAILED;
        NXPLOG_UWB_TML_E("\n\r ---phTmlUwb_TmlReaderThread Task create failed ");
    }

    return wStartStatus;
}

static OSAL_TASK_RETURN_TYPE phTmlUwb_TmlReaderThread(void *pParam)
{
    UWBSTATUS wStatus     = UWBSTATUS_SUCCESS;
    int32_t dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;
    static uint8_t temp[UCI_MAX_DATA_LEN];
    static uint8_t *temp_payload = &temp[4];
    // Fira Test mode is enabled
    static uint8_t chain_packet_counter = 0;
    uint8_t pbf;
    /* Transaction info 2060 to be passed to Callback Thread */
    static phTmlUwb_TransactInfo_t tTransactionInfo;
    /* Structure containing Tml callback function and parameters to be invoked
     by the callback thread */
    static phLibUwb_DeferredCall_t tDeferredInfo;
    /* Initialize Message structure to post message onto Callback Thread */
    static phLibUwb_Message_t tMsg;
    PHUWB_UNUSED(pParam);
    NXPLOG_UWB_TML_D("Tml Reader Thread Started...");
    /* Writer thread loop shall be running till shutdown is invoked */
    while (gpphTmlUwb_Context->bThreadDone) {
        /* If Tml write is requested */
        /* Set the variable to success initially */
        wStatus = UWBSTATUS_SUCCESS;
        /* Reader is always enabled by upper layer wait for 100 ms */
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(gpphTmlUwb_Context->rxSemaphore, PH_TML_UWB_MAX_READER_WAIT) !=
            UWBSTATUS_SUCCESS) {
            NXPLOG_UWB_TML_D("Tml Reader Thread : Fail to acquire rx semaphore");
        }
        /* If Tml read is requested */
        if (1 == gpphTmlUwb_Context->tReadInfo.bEnable) {
            NXPLOG_UWB_TML_D("Read requested...");
            /* Set the variable to success initially */
            wStatus = UWBSTATUS_SUCCESS;

            /* Variable to fetch the actual number of bytes read */
            dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;

            /* Read the data from the file onto the buffer */
            if (NULL != gpphTmlUwb_Context->pDevHandle) {
                NXPLOG_UWB_TML_D("Invoking Read...");
                dwNoBytesWrRd = phTmlUwb_uci_read(temp_payload, UCI_MAX_DATA_LEN);

                if (gpphTmlUwb_Context->bThreadDone == 0) {
                    break;
                }

                if (dwNoBytesWrRd > UCI_MAX_DATA_LEN) {
                    NXPLOG_UWB_TML_E("Number of bytes read exceeds the limit ...");
                    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
                }
#if UWBIOT_UWBD_SR1XXT_SR2XXT
                else if (UWBSTATUS_IRQ_READ_TIMEOUT == dwNoBytesWrRd) {
                    NXPLOG_UWB_TML_D("%s : Read IRQ Timedout, re-read the packet", __FUNCTION__);
                    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
                }
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
                else if (0 == dwNoBytesWrRd) {
#if UWBIOT_TML_S32UART || UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
                    /* Dont' warn */
#else
                    NXPLOG_UWB_TML_D("Empty packet Read, Ignore read and try new read...");
#endif
                    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
                }
                else {
                    // Fira Test mode is enabled
                    if (nxpucihal_ctrl.operationMode == kOPERATION_MODE_mctt) {
                        // print whole log
                        LOG_RX("RECV ", temp_payload, (uint16_t)dwNoBytesWrRd);
                        if (UCI_MT_RSP == ((temp_payload[0] & UCI_MT_MASK) >> UCI_MT_SHIFT)) {
                            // Response packet received
                            pbf = (temp_payload[0] & UCI_PBF_MASK) >> UCI_PBF_SHIFT;
                            if ((pbf == TRUE) && (chain_packet_counter == 0)) {
                                // Chained packet start. Copy to pBuffer and continue normal flow
                                phOsalUwb_MemCopy(
                                    gpphTmlUwb_Context->tReadInfo.pBuffer, temp_payload, (uint32_t)dwNoBytesWrRd);
                                chain_packet_counter++;
                            }
                            else if (chain_packet_counter > 0) {
                                if (gpphTmlUwb_Context->appDataCallback != NULL) {
                                    // Chained packet continue. Send to appDataCallback
                                    gpphTmlUwb_Context->appDataCallback(temp_payload, (uint16_t)(dwNoBytesWrRd));
                                    // End of chained packet?
                                    if (pbf == TRUE) {
                                        chain_packet_counter++;
                                    }
                                    else {
                                        chain_packet_counter = 0; //last packet
                                    }
                                    // Produce semaphore as data has already been sent to appDataCallback
                                    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
                                    continue;
                                }
                                else {
                                    // appDataCallback not registered. Continue normal flow
                                    phOsalUwb_MemCopy(
                                        gpphTmlUwb_Context->tReadInfo.pBuffer, temp_payload, (uint32_t)dwNoBytesWrRd);
                                }
                            }
                            else if ((pbf == FALSE) && (chain_packet_counter == 0)) {
                                // Normal packet. Continue normal flow
                                phOsalUwb_MemCopy(
                                    gpphTmlUwb_Context->tReadInfo.pBuffer, temp_payload, (uint32_t)dwNoBytesWrRd);
                            }
                        }
                        else {
                            // NTF packet received
                            if (gpphTmlUwb_Context->appDataCallback != NULL) {
                                // Send to appDataCallback
                                gpphTmlUwb_Context->appDataCallback(temp_payload, (uint16_t)(dwNoBytesWrRd));
                                phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
                                continue;
                            }
                            else {
                                // appDataCallback not registered. Continue normal flow
                                phOsalUwb_MemCopy(
                                    gpphTmlUwb_Context->tReadInfo.pBuffer, temp_payload, (uint32_t)dwNoBytesWrRd);
                            }
                        }
                    }
                    else {
                        phOsalUwb_MemCopy(gpphTmlUwb_Context->tReadInfo.pBuffer, temp_payload, (uint32_t)dwNoBytesWrRd);
                    }
                    NXPLOG_UWB_TML_D("Read successful...");
                    /* This has to be reset only after a successful read */
                    gpphTmlUwb_Context->tReadInfo.bEnable = 0;

                    /* Update the actual number of bytes read including header */
                    gpphTmlUwb_Context->tReadInfo.wLength = (uint16_t)(dwNoBytesWrRd);

                    dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;

                    /* Fill the Transaction info structure to be passed to Callback
                    * Function */
                    tTransactionInfo.wStatus = wStatus;
                    tTransactionInfo.pBuff   = gpphTmlUwb_Context->tReadInfo.pBuffer;
                    /* Actual number of bytes read is filled in the structure */
                    tTransactionInfo.wLength = gpphTmlUwb_Context->tReadInfo.wLength;

                    /* Read operation completed successfully. Post a Message onto Callback
                    * Thread*/
                    /* Prepare the message to be posted on User thread */
                    tDeferredInfo.pCallback  = &phTmlUwb_ReadDeferredCb;
                    tDeferredInfo.pParameter = &tTransactionInfo;
                    tMsg.eMsgType            = PH_LIBUWB_DEFERREDCALL_MSG;
                    tMsg.pMsgData            = &tDeferredInfo;
                    tMsg.Size                = sizeof(tDeferredInfo);
#if UWBIOT_UWBD_SR040
                    phTmlUwb_PrintRecevedMessage(temp_payload, gpphTmlUwb_Context->tReadInfo.wLength);
#else
                    if (nxpucihal_ctrl.operationMode != kOPERATION_MODE_mctt) {
                        if (gpphTmlUwb_Context->tReadInfo.wLength > RX_LOG_MAX_NUMBER_OF_BYTES) {
                            LOG_RX("RECV ", temp_payload, RX_LOG_MAX_NUMBER_OF_BYTES);
                        }
                        else {
                            LOG_RX("RECV ", temp_payload, gpphTmlUwb_Context->tReadInfo.wLength);
                        }
                    }
#endif
                    NXPLOG_UWB_TML_D("Posting read message...");
                    phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, &tMsg);
                }
            }
            else {
                NXPLOG_UWB_TML_D("SR100 -gpphTmlUwb_Context->pDevHandle is NULL");
            }
        }
        else {
            NXPLOG_UWB_TML_D("read request NOT enabled");
            phOsalUwb_Delay(10);
        }
    } /* End of While loop */

    /* Suspend task here so that it does not return in FreeRTOS
     * Task will be deleted in shutdown sequence
     */
    (void)phOsalUwb_TaskSuspend(gpphTmlUwb_Context->readerThread);
}
#if UWBIOT_UWBD_SR040
/* Print ASCII messages coming from FW */
static void phTmlUwb_PrintRecevedMessage(const uint8_t *const pBuffer, const uint16_t wLength)
{
#if defined(DEBUG) && UWBIOT_UWBD_SR040 && SR040_PRINT_DEBUG_LOG_MESSAGES && defined(_MSC_VER)
    if ((wLength >= 5)                       /* */
        && (pBuffer[0] == 0x6E)              /* */
        && (pBuffer[1] == 0x00)              /* */
        && (pBuffer[2] == 0x00)              /* */
        && (pBuffer[3] == (wLength - 4))     /* */
        && (pBuffer[4] == 0x01)              /* */
        && (pBuffer[5] == 0x0A)              /* */
        && (pBuffer[6] == (wLength - 4 - 3)) /* */
    ) {
        for (int i = 7; i < wLength; i++) {
            if (isspace(pBuffer[i])) {
                /* No new lines */
                PUTCHAR(' ');
            }
            else {
                PUTCHAR(pBuffer[i]);
            }
        }
        PUTCHAR('\n');
    }
    else
#endif
    {
        LOG_RX("RECV ", pBuffer, wLength);
    }
}
#endif

/*******************************************************************************
**
** Function         phTmlUwb_CleanUp
**
** Description      Clears all handles opened during TML initialization
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_CleanUp(void)
{
    if (NULL == gpphTmlUwb_Context) {
        return;
    }
    if (NULL != gpphTmlUwb_Context->pDevHandle) {
        (void)phTmlUwb_reset(0);
        gpphTmlUwb_Context->bThreadDone = 0;
    }
    phTmlUwb_close();
    phOsalUwb_DeleteSemaphore(&gpphTmlUwb_Context->rxSemaphore);
    phOsalUwb_DeleteSemaphore(&gpphTmlUwb_Context->postMsgSemaphore);
    gpphTmlUwb_Context->pDevHandle = NULL;
    /* Clear memory allocated for storing Context variables */
    phOsalUwb_FreeMemory((void *)gpphTmlUwb_Context);
    /* Set the pointer to NULL to indicate De-Initialization */
    gpphTmlUwb_Context = NULL;
    return;
}

/*******************************************************************************
**
** Function         phTmlUwb_Shutdown
**
** Description      Uninitializes TML layer and hardware interface
**
** Parameters       None
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - TML configuration released successfully
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_FAILED - un-initialization failed (example: unable
**                                     to close interface)
**
*******************************************************************************/
UWBSTATUS phTmlUwb_Shutdown(void)
{
    UWBSTATUS wShutdownStatus = UWBSTATUS_SUCCESS;

    /* Check whether TML is Initialized */
    if (NULL != gpphTmlUwb_Context) {
        /* Reset thread variable to terminate the thread */
        gpphTmlUwb_Context->bThreadDone = 0;

        /* Clear All the resources allocated during initialization */
        (void)phTmlUwb_reset(ABORT_READ_PENDING);
        (void)phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
        phOsalUwb_Delay(1);
        (void)phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->postMsgSemaphore);
        phOsalUwb_Delay(1);

        (void)phOsalUwb_Thread_Delete(gpphTmlUwb_Context->readerThread);

        NXPLOG_UWB_TML_D("bThreadDone == 0");

        phTmlUwb_CleanUp();
    }
    else {
        wShutdownStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
    }

    return wShutdownStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_Write
**
** Description      Synchronously writes given data block to hardware
**                  interface/driver.
**
** Parameters       p_data - data to be sent
**                  data_len - length of data buffer
**
** Returns          wStatus:
**                  UWBSTATUS_SUCCESS - Data written successfully
**                  UWBSTATUS_WRITE_FAILED - Failed to write data
**
*******************************************************************************/
UWBSTATUS phTmlUwb_Write(uint8_t *p_data, uint16_t data_len)
{
    UWBSTATUS wStatus     = UWBSTATUS_SUCCESS;
    int32_t dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;

    /* Check whether TML is Initialized */
    if (NULL != gpphTmlUwb_Context) {
        if ((NULL != gpphTmlUwb_Context->pDevHandle) && (NULL != p_data) && (PH_TMLUWB_RESET_VALUE != data_len)) {
            NXPLOG_UWB_TML_D("Invoking Write...");
            if (nxpucihal_ctrl.IsDev_suspend_enabled) {
                /* TBD: to be checked for its purpose */
                NXPLOG_UWB_TML_D("writing dummy packet during standby.");
                dwNoBytesWrRd = phTmlUwb_uci_write(p_data, NORMAL_MODE_HEADER_LEN);
                if (dwNoBytesWrRd < 0) {
                    NXPLOG_UWB_TML_D("Error in write...");
                }
                nxpucihal_ctrl.IsDev_suspend_enabled = FALSE;
                phOsalUwb_Delay(40);
            }
            dwNoBytesWrRd = phTmlUwb_uci_write(p_data, data_len);
            if (dwNoBytesWrRd == -2) {
                wStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_BUSY);
            }
            else if (dwNoBytesWrRd < 0) {
                NXPLOG_UWB_TML_W("Command error in Write...");
                wStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
                LOG_TX("FAIL ", p_data, data_len);
                wStatus = UWBSTATUS_WRITE_FAILED;
            }
            else {
                LOG_TX("SEND ", p_data, data_len);
            }
        }
        else {
            wStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
        }
    }
    else {
        wStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
    }
    return wStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_Read
**
** Description      Asynchronously reads data from the driver
**                  Number of bytes to be read and buffer are passed by upper
**                  layer.
**                  Enables reader thread if there are no read requests pending
**                  Returns successfully once read operation is completed
**                  Notifies upper layer using callback mechanism
**
** Parameters       pBuffer - location to send read data to the upper layer via
**                            callback
**                  wLength - length of read data buffer passed by upper layer
**                  pTmlReadComplete - pointer to the function to be invoked
**                                     upon completion of read operation
**                  pContext - context provided by upper layer
**
** Returns          UWB status:
**                  UWBSTATUS_PENDING - command is yet to be processed
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_BUSY - read request is already in progress
**
*******************************************************************************/
UWBSTATUS phTmlUwb_Read(
    uint8_t *pBuffer, uint16_t wLength, pphTmlUwb_TransactCompletionCb_t pTmlReadComplete, void *pContext)
{
    UWBSTATUS wReadStatus;

    /* Check whether TML is Initialized */
    if (NULL != gpphTmlUwb_Context) {
        if ((gpphTmlUwb_Context->pDevHandle != NULL) && (NULL != pBuffer) && (PH_TMLUWB_RESET_VALUE != wLength) &&
            (NULL != pTmlReadComplete)) {
            if (!gpphTmlUwb_Context->tReadInfo.bThreadBusy) {
                /* Setting the flag marks beginning of a Read Operation */
                gpphTmlUwb_Context->tReadInfo.bThreadBusy = TRUE;
                /* Copy the buffer, length and Callback function,
                   This shall be utilized while invoking the Callback function in thread
                   */
                gpphTmlUwb_Context->tReadInfo.pBuffer          = pBuffer;
                gpphTmlUwb_Context->tReadInfo.wLength          = wLength;
                gpphTmlUwb_Context->tReadInfo.pThread_Callback = pTmlReadComplete;
                gpphTmlUwb_Context->tReadInfo.pContext         = pContext;
                wReadStatus                                    = UWBSTATUS_PENDING;

                /* Set event to invoke Reader Thread */
                gpphTmlUwb_Context->tReadInfo.bEnable = 1;                   // To be enabled later
                phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore); // To be enabled later
                // NXPLOG_UWB_TML_I("UWB context not null phTmlUwb_Read \n ");
            }
            else {
                wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_BUSY);
            }
        }
        else {
            wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
        }
    }
    else {
        wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
        // NXPLOG_UWB_TML_I("UWB context QUIT \n ");
    }

    return wReadStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_ReadAbort
**
** Description      Aborts pending read request (if any)
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_ReadAbort(void)
{
    gpphTmlUwb_Context->tReadInfo.bEnable = 0;

    /*Reset the flag to accept another Read Request */
    gpphTmlUwb_Context->tReadInfo.bThreadBusy = FALSE;
}
/*******************************************************************************
**
** Function         phTmlUwb_DeferredCall
**
** Description      Posts message on upper layer thread
**                  upon successful read or write operation
**
** Parameters       dwThreadId  - id of the thread posting message
**                  ptWorkerMsg - message to be posted
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_DeferredCall(uintptr_t dwThreadId, phLibUwb_Message_t *ptWorkerMsg)
{
    PHUWB_UNUSED(dwThreadId);
    /* Post message on the user thread to invoke the callback function */
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(
            gpphTmlUwb_Context->postMsgSemaphore, PH_TML_UWB_MAX_MESSAGE_POST_WAIT) != UWBSTATUS_SUCCESS) {
        NXPLOG_UWB_TML_E("phTmlUwb_DeferredCall: consume semaphore error");
        (void)phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->postMsgSemaphore);
        return;
    }
    (void)phOsalUwb_msgsnd(gpphTmlUwb_Context->dwCallbackThreadId, ptWorkerMsg, NO_DELAY);
    (void)phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->postMsgSemaphore);
}

/*******************************************************************************
**
** Function         phTmlUwb_ReadDeferredCb
**
** Description      Read thread call back function
**
** Parameters       pParams - context provided by upper layer
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_ReadDeferredCb(void *pParams)
{
    /* Transaction info buffer to be passed to Callback Function */
    phTmlUwb_TransactInfo_t *pTransactionInfo = (phTmlUwb_TransactInfo_t *)pParams;

    /* Reset the flag to accept another Read Request */
    gpphTmlUwb_Context->tReadInfo.bThreadBusy = FALSE;
    gpphTmlUwb_Context->tReadInfo.pThread_Callback(gpphTmlUwb_Context->tReadInfo.pContext, pTransactionInfo);

    return;
}

#if UWBIOT_UWBD_SR2XXT
/*******************************************************************************
**
** Function         phTmlUwb_Chip_Reset
**
** Description      Invoke this API to Chip enable/Disable
**
** Parameters       None
**
** Returns          void
**
*******************************************************************************/
void phTmlUwb_Chip_Reset(void)
{
    phTmlUwb_io_set(kUWBS_IO_O_RSTN, 0);
    phOsalUwb_Delay(10);
    phTmlUwb_io_set(kUWBS_IO_O_RSTN, 1);
    phOsalUwb_Delay(10);
}
#endif // UWBIOT_UWBD_SR2XXT

/*******************************************************************************
**
** Function         phTmlUwb_suspendReader
**
** Description      Suspend the Reader Thread
**
** Parameters       None
**
** Returns          void
**
*******************************************************************************/
void phTmlUwb_suspendReader()
{
    phOsalUwb_TaskSuspend(gpphTmlUwb_Context->readerThread);
}

/*******************************************************************************
**
** Function         phTmlUwb_resumeReader
**
** Description      Resume the suspended Reader Thread
**
** Parameters       None
**
** Returns          void
**
*******************************************************************************/
void phTmlUwb_resumeReader()
{
    phOsalUwb_TaskResume(gpphTmlUwb_Context->readerThread);
}
