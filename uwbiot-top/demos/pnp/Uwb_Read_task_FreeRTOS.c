/*
 * Copyright 2019,2020,2022,2023 NXP
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

/* System includes */
#include "phUwb_BuildConfig.h"

#include "phUwbTypes.h"
#include <stdint.h>
#if UWBIOT_OS_FREERTOS

/* Freescale includes */

#if UWBIOT_UWBD_SR1XXT
#if ((defined(BOARD_VERSION)) && (defined(SHIELD)) && (defined(RHODES_V4))) && \
    (((BOARD_VERSION != SHIELD) && (BOARD_VERSION != RHODES_V4)))
#include "fsl_port.h"
#include "fsl_lpspi.h"
#endif //((BOARD_VERSION != SHIELD) && (BOARD_VERSION != RHODES_V4))
#include "UWB_Hbci.h"
#endif
//#include "fsl_gpio.h"
/* UWB includes */
#include "UWB_Spi_Driver_Interface.h"

#include "UWB_Evt_Pnp.h"
#include "UwbPnpInternal.h"
#include "phOsalUwb.h"
#include "app_config.h"

OSAL_TASK_RETURN_TYPE UCI_ReaderTask(void *args)
{
    static uint8_t Buffer[HIF_MAX_PKT_SIZE];
    size_t numRead = HIF_MAX_PKT_SIZE;
    while (1) {
        numRead = HIF_MAX_PKT_SIZE;
        UWB_SpiUciRead(Buffer, &numRead);
        if (numRead == 0) {
            DEBUGOUT("Spi Read Error, Zero bytes read\n");
            continue;
        }
        DEBUGOUT("received uci rsp/ntf: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
#if ((defined(ENABLE_UWB_RESPONSE)) && (ENABLE_UWB_RESPONSE == ENABLED))
        DEBUGOUT("read returned count is %d\n", numRead);
        DEBUGOUT("UCI rsp:\n");
        for (int i = 0; i < numRead; i++) {
            DEBUGOUT(" %02x", Buffer[i]);
        }
        DEBUGOUT("\n");
#endif
        if (!Uwb_Is_Hif_Active()) {
            if ((Buffer[0] & 0xF0) == 0x40) {
                uint32_t error;
                (void)phOsalUwb_LockMutex(mHifWriteMutex);
                DEBUGOUT("sending rsp: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
                error = UWB_Hif_SendUCIRsp(Buffer, numRead);
                if (error != 0) {
                    DEBUGOUT("UCI_READER: error sending over HIF [%d]\n", error);
                }
                (void)phOsalUwb_UnlockMutex(mHifWriteMutex);
            }
            else {
                if (phOsalUwb_queueSpacesAvailable(mHifWriteQueue) > 0) {
                    phLibUwb_Message_t tlv;
                    if (numRead <= HIF_MAX_PKT_SIZE) {
                        tlv.Size     = (uint16_t)numRead;
                        tlv.pMsgData = (void *)phOsalUwb_GetMemory(tlv.Size * sizeof(uint8_t));
                        if (tlv.pMsgData != NULL) {
                            phOsalUwb_MemCopy((uint8_t *)tlv.pMsgData, Buffer, tlv.Size);
                            phOsalUwb_msgsnd(mHifWriteQueue, &tlv, NO_DELAY);
                        }
                        else {
                            DEBUGOUT("UCI_ReaderTask: Unable to Allocate Memory of %d, Memory Full:\n", tlv.Size);
                        }
                    }
                    else {
                        DEBUGOUT("UCI_ReaderTask: Invalid number of bytes read %d:\n", numRead);
                    }
                }
                else {
                    DEBUGOUT("Queue is FULL, ignoring notification\n");
                }
            }
        }
        else {
            DEBUGOUT("USB detached, USB channel reset.\n");
            DEBUGOUT("missed uci rsp/ntf: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
            Uwb_Reset_Hif_State(FALSE);
        }
    }
}
#endif //UWBIOT_OS_FREERTOS
