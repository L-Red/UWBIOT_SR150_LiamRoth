/*
 *
 * Copyright 2018-2020,2022 NXP.
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

#if UWBFTR_SE_SN110
#include "SeApi.h"
#include "UwbSeApi.h"

UINT8 UwbSeApi_NciRawCmdSend(UINT16 dataLen, UINT8 *pData, UINT16 *pRespSize, UINT8 *pRespBuf)
{
    return SeApi_NciRawCmdSend(dataLen, pData, pRespSize, pRespBuf);
}
#endif
