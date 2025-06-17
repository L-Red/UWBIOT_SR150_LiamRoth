/* Copyright 2021,2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

/* demos\rhodes\demo_test_tx\app_Test_Cfg.h */

#ifndef APP_R_CFG_H
#define APP_R_CFG_H

#include <AppInternal.h>

#if APP_INTERNAL_USE_HPRF

#define DEMO_RF_TEST_PRF_MODE            (kUWB_PrfMode_124_8MHz)
#define DEMO_RF_TEST_PREAMBLE_CODE_INDEX (25)
#define DEMO_RF_TEST_SFD_ID              (2)
#define DEMO_RF_TEST_PSDU_DATA_RATE \
    (APP_INTERNAL_USE_HPRF_PSDU_DATARATE_78M == 1 ? kUWB_PsduDataRate_7_80Mbps : kUWB_PsduDataRate_6_81Mbps)
#define DEMO_RF_TEST_PREAMBLE_DURATION (kUWB_PreambleDuration_64Symbols)
#else // BPRF
#define DEMO_RF_TEST_PRF_MODE            (kUWB_PrfMode_62_4MHz)
#define DEMO_RF_TEST_PREAMBLE_CODE_INDEX (10)
#define DEMO_RF_TEST_SFD_ID              (2)
#define DEMO_RF_TEST_PSDU_DATA_RATE      kUWB_PsduDataRate_6_81Mbps
#define DEMO_RF_TEST_PREAMBLE_DURATION   kUWB_PreambleDuration_64Symbols
#endif // HPRF or BPRF

#define DEMO_RF_TEST_CHANNEL_ID    9
/* Do not use Enums in Preprocessor directives especially during comparisons */
/* For SP0 Frame type explicitly Set 0
 * For SP1 Frame type explicitly set 1
 * For SP3 Frame type explicitly set 3
 */
#define DEMO_RF_TEST_RFRAME_CONFIG (0)

#if (defined(DEMO_RF_TEST_RFRAME_CONFIG) && (DEMO_RF_TEST_RFRAME_CONFIG == 0))
// SP0, no STS segments
#define DEMO_RF_TEST_NUMBER_OF_STS_SEGMENTS (0)
#else
#define DEMO_RF_TEST_NUMBER_OF_STS_SEGMENTS (1) // 1 or 2
#endif

#endif /* APP_R_CFG_H */
