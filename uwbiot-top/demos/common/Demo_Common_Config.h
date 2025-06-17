/* Copyright 2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef __DEMO_COMMON_CONFIG__
#define __DEMO_COMMON_CONFIG__

#include "phUwb_BuildConfig.h"
#include "UwbApi.h"

tUWBAPI_STATUS demo_set_common_app_config(uint32_t sessionHandle, UWB_StsConfig_t sts_config);

#if (UWBFTR_SE_SN110)
tUWBAPI_STATUS UwbApi_setFixedSessionKey(uint32_t sessionHandle);
#endif //UWBFTR_SE_SN110

#endif // __DEMO_COMMON_CONFIG__
