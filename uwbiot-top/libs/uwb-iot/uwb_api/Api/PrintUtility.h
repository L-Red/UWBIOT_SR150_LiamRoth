/*
 *
 * Copyright 2018-2020,2023 NXP.
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

#ifndef _PRINT_UTILITY_
#define _PRINT_UTILITY_

#ifdef __cplusplus
extern "C" {
#endif

#include "phUwbTypes.h"
#include <inttypes.h>
#include "phUwb_BuildConfig.h"
#include "UwbApi_Types.h"
#include "PrintUtility_Proprietary.h"

/** Q-format : Q(x.y)
 * Q(x.y) : where x is number of integer bits + 1 (sign bit)
 * And y is number of fractional bits
 * For Signed integer take 2's complement
*/
#define TO_Q_8_8(X) ((X >> 8 & 0x80) ? -((~X + 1) >> 8) : (X >> 8)) ,((X >> 8 & 0x80) ? ((~X + 1) & 0xFF) : (X & 0xFF))

/** Here for integer bits x : x >> 0x0B
 *  Now for fractional bits y : y & 0x3FF
 *  where 2^11 = 2048 : which ≈ 0-2047(0x7FF in hex)
*/
#define TO_Q_5_11(X) ((X >> 8 & 0x80) ? -((~X + 1) >> 0x0B) : (X >> 0x0B)) ,((X >> 8 & 0x80) ? ((~X + 1) & 0x7FF) : (X & 0x7FF))

/** Here for integer bits x : x >> 0x0A
 *  Now for fractional bits y : y & 0x3FF
 *  where 2^10 = 1024 : which ≈ 0-1023(0x3FF in hex)
*/
#define TO_Q_6_10(X) ((X >> 8 & 0x80) ? -((~X + 1) >> 0x0A) : (X >> 0x0A)) ,((X >> 8 & 0x80) ? ((~X + 1) & 0x3FF) : (X & 0x3FF))

/** Here for integer bits x : x >> 7
 *  Now for fractional bits y : y & 0x7F
 *  where 2^7 = 128 : which ≈ 0-127(0x7F in hex)
*/
#define TO_Q_9_7(X) ((X >> 8 & 0x80) ? -((~X + 1) >> 7) : (X >> 7)) ,((X >> 8 & 0x80) ? ((~X + 1) & 0x7F) : (X & 0x7F))

/** Here for integer bits x : x >> 1
 *  Now for fractional bits y : y & 0x01
 *  where 2^1 = 2 : which ≈ 0-1(0x01 in hex)
*/
#define TO_Q_7_1(X) ((X & 0x80) ? -((~X + 1) >> 1) : (X >> 1)) ,((X & 0x80) ? ((~X + 1) & 0x01) : (X & 0x01))

EXTERNC void printGenericErrorStatus(const phGenericError_t *pGenericError);
EXTERNC void printSessionStatusData(const phUwbSessionInfo_t *pSessionInfo);
EXTERNC void printUwbSessionData(const phUwbSessionsContext_t *pUwbSessionsContext);
EXTERNC void printMulticastListStatus(const phMulticastControleeListNtfContext_t *pControleeNtfContext);
EXTERNC void printRangingData(const phRangingData_t *pRangingData);
EXTERNC void printRangingParams(const phRangingParams_t *pRangingParams);
EXTERNC void printTwoWayRangingData(const phRangingData_t *pRangingData);
EXTERNC void printoneWayRangingData(const phRangingData_t *pRangingData);
EXTERNC void printDltdoaRangingData(const phRangingData_t *pRangingData);
EXTERNC void printDataTxPhaseCfgNtf(const phDataTxPhaseCfgNtf_t *pDataTxPhCfgNtf);
#if UWBFTR_DataTransfer
EXTERNC void printRcvDataStatus(const phUwbRcvDataPkt_t *pRcvDataPkt);
EXTERNC void printTransmitStatus(const phUwbDataTransmit_t *pTransmitNtfContext);
#endif //UWBFTR_DataTransfer

#if UWBFTR_Radar
EXTERNC void printRadarRecvNtf(const phUwbRadarNtf_t *pRcvRadaNtfPkt);
EXTERNC void printRadarTestIsoNtf(const phUwbRadarNtf_t *pRcvRadaTstNtfPkt);
#endif //UWBFTR_Radar

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
EXTERNC void printWiFiCoExIndNtf(const UWB_CoEx_IndNtf_t *wifiCoExIndNtf);
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR160)
#if UWBFTR_CCC
EXTERNC void printAliroRangingData(const phAliroRangingData_t *AliroRangingData);
#endif // UWBFTR_CCC

#ifdef __cplusplus
} // closing brace for extern "C"
#endif // __cplusplus

#endif // _PRINT_UTILITY_
