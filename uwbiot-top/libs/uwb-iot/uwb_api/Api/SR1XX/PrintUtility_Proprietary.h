/*
 *
 * Copyright 2018-2020,2022,2023 NXP.
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

#ifndef _PRINT_UTILITY_PROPRIETARY_H
#define _PRINT_UTILITY_PROPRIETARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <UwbApi_Types_Proprietary.h>

EXTERNC void printDistance_Aoa(const phRangingData_t *pRangingData);
EXTERNC void printDebugParams(uint8_t noOfParams, const UWB_DebugParams_List_t *DebugParams_List);
EXTERNC void printDeviceInfo(const phUwbDevInfo_t *pdevInfo);
#if (UWBFTR_SE_SN110)
EXTERNC void printDoBindStatus(const phSeDoBindStatus_t *pDoBindStatus);
EXTERNC void printGetBindingStatus(const phSeGetBindingStatus_t *pGetBindingStatus);
EXTERNC void printGetEseTestConnectivityStatus(const SeConnectivityStatus_t *pGetSeConnectivityStatus);
#endif //(UWBFTR_SE_SN110)
EXTERNC void printTestLoopNtfData(const phTestLoopData_t *pTestLoopData);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif
