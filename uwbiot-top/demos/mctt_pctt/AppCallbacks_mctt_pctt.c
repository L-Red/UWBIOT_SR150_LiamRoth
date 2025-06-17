/* Copyright 2019-2021 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "AppInternal.h"
#include "AppRecovery.h"
#include "phOsalUwb.h"
#include "Utilities.h"
#include "UwbHif.h"

#if !defined(UWBIOT_APP_BUILD__DEMO_MCTT_PCTT)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_MCTT_PCTT)

#if UWBIOT_UWBD_SR040
#define MAX_MCTT_PKT_SIZE 256
#else
#define MAX_MCTT_PKT_SIZE 1024
#endif

#define MAX_NTFS 10

static uint8_t ntfDataBuf[MAX_MCTT_PKT_SIZE * MAX_NTFS]; // buf enough to hold max MAX_NTFS MAX_PKTs in queue
static uint16_t dataBufIdx = 0;                          // bufIdx intially at 0
void *rfTestSem            = NULL;

uint8_t *getCdcBuf(uint16_t size)
{
    uint16_t memPtr = dataBufIdx;
    // if dataBuf exceeds its limit to hold incoming size, reset bufIdx to 0
    if ((memPtr + size + HIF_RSP_HEADER_SIZE) > MAX_MCTT_PKT_SIZE * MAX_NTFS) {
        memPtr     = 0;
        dataBufIdx = 0;
    }
    dataBufIdx = (uint16_t)(dataBufIdx + size); // update bufIdx to size of incoming data
    return &ntfDataBuf[memPtr];                 //return initial addr of cur buf which can hold data of len 'size'
}

void McttAppDataCallback(uint8_t *recvData, uint16_t recvDataLen)
{
    (void)phOsalUwb_LockMutex(mCmdMutex);
    phLibUwb_Message_t tlv;
    tlv.eMsgType = UWB_MCTT_UCI_READY;
    tlv.Size     = recvDataLen;
    tlv.pMsgData = getCdcBuf(recvDataLen);

    phOsalUwb_MemCopy(tlv.pMsgData, recvData, recvDataLen);
    phOsalUwb_msgsnd(mHifCommandQueue, &tlv, MAX_DELAY);

    (void)phOsalUwb_UnlockMutex(mCmdMutex);
}

#endif //UWBIOT_APP_BUILD__DEMO_MCTT_PCTT
