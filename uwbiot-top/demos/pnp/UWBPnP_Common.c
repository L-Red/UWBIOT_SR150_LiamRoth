/*
 *
 * Copyright 2021 NXP.
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

#include <UWB_Evt_Pnp.h>
#include <uwbiot_ver.h>

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif
#include "UwbPnpInternal.h"

size_t UWBPnP_GetVersionInfo(uint8_t *tlvBuf, size_t tlvBufLen, UBWPnPBoardIdentifier_t board)
{
    if (tlvBufLen < GET_VERISON_INFO_RESPONSE_SIZE) {
        return 0;
    }
    tlvBuf[0] = GET_VERISON_INFO;
    tlvBuf[1] = UWBIOTVER_STR_VER_MAJOR;
    tlvBuf[2] = UWBIOTVER_STR_VER_MINOR;
    tlvBuf[3] = UWBIOTVER_STR_VER_DEV;
    tlvBuf[4] = 0; /* RFU */

#if UWBIOT_UWBD_SR150
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR150;
#elif UWBIOT_UWBD_SR040
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR040;
#elif UWBIOT_UWBD_SR100T
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR100T;
#elif UWBIOT_UWBD_SR110T
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR110T;
#elif UWBIOT_UWBD_SR200T
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR200T;
#elif UWBIOT_UWBD_SR100S
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR100S;
#elif UWBIOT_UWBD_SR160
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR160;
#elif UWBIOT_UWBD_SR250
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR250;
#else
#error "Don't know the IC"
#endif
    tlvBuf[6] = board;

    return GET_VERISON_INFO_RESPONSE_SIZE;
}
