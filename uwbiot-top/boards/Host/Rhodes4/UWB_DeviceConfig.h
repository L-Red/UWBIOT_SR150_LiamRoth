/*
 * Copyright 2018-2022 NXP.
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

#ifndef _UWB_DEVICECONFIG_RV4_H_
#define _UWB_DEVICECONFIG_RV4_H_

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#if UWBIOT_UWBD_SR100S
#include <UWB_DeviceConfig_SR100S.h>
#else
#include <UWB_DeviceConfig_SR1XX.h>
#endif

#endif //_UWB_DEVICECONFIG_RV4_H_
