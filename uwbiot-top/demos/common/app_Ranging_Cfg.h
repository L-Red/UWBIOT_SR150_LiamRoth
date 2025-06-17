/* Copyright 2020,2021, 2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef APP_R_CFG_H
#define APP_R_CFG_H

#define RANGING_APP_SESSION_ID 0x11223344

#define RANGING_APP_MULTI_NODE_MODE_MANY 1

#define RANGING_APP_NO_OF_ANCHORS_P2P 1

#define RANGING_APP_DEVICE_MAC_ADDRESS \
    {                                  \
        0x11, 0x22                     \
    }
#define RANGING_APP_DEST_MAC_ADDRESS_P2P \
    {                                    \
        0x33, 0x44                       \
    }

#endif /* APP_R_CFG_H */
