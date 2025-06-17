/*
 *
 * Copyright 2018-2021 NXP.
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

#include <stdbool.h>
#include <string.h>

#include "UWBT_Config.h"
#include "NVM_Interface.h"
#include "board.h"

#define BLE_DFLT_INTERVAL 0x0140

typedef struct
{
    uint8_t bleAddr[6];
    uint8_t role;
    uint16_t interval;
} UWBT_WakeupInfo_t;

#define nvmId_BleName_c     0x3001U
#define nvmId_BleInterval_c 0x3002U
#define nvmId_Wakeupinfo_c  0x3003U

static char mBleName[BLE_NAME_SIZE + 1];
static uint16_t mBleInterval;
static UWBT_WakeupInfo_t mWakeupInfo;

NVM_RegisterDataSet(mBleName, 1, BLE_NAME_SIZE + 1, nvmId_BleName_c, gNVM_MirroredInRam_c);
NVM_RegisterDataSet(&mBleInterval, 1, sizeof(uint16_t), nvmId_BleInterval_c, gNVM_MirroredInRam_c);
NVM_RegisterDataSet(&mWakeupInfo, 1, sizeof(UWBT_WakeupInfo_t), nvmId_Wakeupinfo_c, gNVM_MirroredInRam_c);

const char *UWBT_CfgReadBleName()
{
    if (NvRestoreDataSet(mBleName, false) != gNVM_OK_c) {
        return NULL;
    }

    return mBleName;
}

uint32_t UWBT_CfgReadBleInterval()
{
    if (NvRestoreDataSet(&mBleInterval, false) == gNVM_OK_c) {
        return mBleInterval;
    }
    else {
        return BLE_DFLT_INTERVAL;
    }
}

bool UWBT_CfgWriteBleName(uint8_t *name, uint16_t nameLen)
{
    if (NvIsDataSetDirty(mBleName) || nameLen > BLE_NAME_SIZE) {
        return false;
    }

    memcpy(mBleName, name, nameLen);
    if (mBleName[nameLen - 1] != 0) {
        // Force 0-termination
        mBleName[nameLen] = 0;
    }

    return NvSaveOnIdle(mBleName, false) == gNVM_OK_c;
}

bool UWBT_CfgWriteBleInterval(uint16_t interval)
{
    bool ok = false;

    uint16_t oldValue = mBleInterval;
    if (NvIsDataSetDirty(&mBleInterval)) {
        goto end;
    }

    mBleInterval = interval;

    if (NvSaveOnIdle(&mBleInterval, false) != gNVM_OK_c) {
        goto end;
    }

    ok = true;

end:
    if (!ok) {
        mBleInterval = oldValue;
    }

    return ok;
}

bool UWBT_CfgInit(void)
{
    bool ok = true;
    if (NvRestoreDataSet(&mBleInterval, false) != gNVM_OK_c) {
        mBleInterval = BLE_DFLT_INTERVAL;
        if (NvSaveOnIdle(&mBleInterval, false) != gNVM_OK_c) {
            goto end;
        }
    }

    ok = true;
end:
    return ok;
}
