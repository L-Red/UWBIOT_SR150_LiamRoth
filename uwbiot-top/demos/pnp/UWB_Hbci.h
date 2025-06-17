/*
 * Copyright 2019,2020 NXP
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
#include "phUwb_BuildConfig.h"

#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)

#ifndef UWB_HBCI_H
#define UWB_HBCI_H

#define FW_CHUNK_LEN 2048

#define MAX_HBCI_LEN 1024
#define HBCI_HDR_LEN 4

/* HBCI GENERAL COMMAND SET */
/* CLA definitions */
#define GENERAL_QRY_CLA 0x01
#define GENERAL_ANS_CLA 0x02
#define GENERAL_CMD_CLA 0x03
#define GENERAL_ACK_CLA 0x04

/* QRY INS */
#define QRY_STATUS_INS     0x21
#define QRY_CHIP_ID_INS    0x31
#define QRY_HELIOS_ID_INS  0x32
#define QRY_CA_ROOT_PK_INS 0x33
#define QRY_NXP_PK_INS     0x34

/* ANS INS */
#define ANS_HBCI_READY_INS           0x21
#define ANS_MODE_TEST_OS_READY_INS   0x22
#define ANS_MODE_PATCH_ROM_READY_INS 0x23
#define ANS_MODE_PATCH_HIF_READY_INS 0x24
#define ANS_MODE_PATCH_IM4_READY_INS 0x25

/* CMD INS */
#define CMD_MODE_TEST_OS_INS   0x22
#define CMD_MODE_PATCH_ROM_INS 0x23
#define CMD_MODE_HIF_INS       0x24
#define CMD_MODE_IM4_INS       0x25

/* ACK INS */
#define ACK_VALID_APDU_INS   0x01
#define ACK_LRC_MISMATCH_INS 0x82
#define ACK_INVALID_CLA_INS  0x83
#define ACK_INVALID_INS_INS  0x84
#define ACK_INVALID_LEN_INS  0x85

/* HBCI TESTOS  COMMAND SET */
/* Class defitions */
#define TEST_OS_QRY_CLA 0x11
#define TEST_OS_ANS_CLA 0x12
#define TEST_OS_CMD_CLA 0x13

/* Query INS definitions */
#define TEST_OS_WRITE_STATUS_INS    0x1
#define TEST_OS_AUTH_STATUS_INS     0x2
#define TEST_OS_JTAG2AHB_STATUS_INS 0x3
#define TEST_OS_PAYLOAD_STATUS_INS  0x4
#define TEST_OS_DEVICE_STATUS_INS   0x8
#define TEST_OS_ATTEMPTS_REM_INS    0x9

/* Answer INS definitions */
#define TEST_OS_WRITE_SUCCESS_INS        0x1
#define TEST_OS_AUTH_SUCCESS_INS         0x2
#define TEST_OS_JTAG2AHB_SUCCESS_INS     0x3
#define TEST_OS_PAYLOAD_SUCCESS_INS      0x4
#define TEST_OS_DEVICE_UNLOCKED_INS      0x8
#define TEST_OS_OTP_FULL_INS             0x81
#define TEST_OS_INVALID_PASSWORD_LEN_INS 0x82
#define TEST_OS_AUTH_FAIL_INS            0x83
#define TEST_OS_DEVICE_LOCKED_INS        0x84
#define TEST_OS_JTAG2AHB_FAIL_INS        0x85
#define TEST_OS_PAYLOAD_FAIL_INS         0x86

/* Command INS definitions */
#define TEST_OS_WRITE_PASSWORD_INS   0x1
#define TEST_OS_AUTH_PASSWORD_INS    0x2
#define TEST_OS_ENABLE_JTAG2AHB_INS  0x3
#define TEST_OS_DOWNLOAD_PAYLOAD_INS 0x4

bool UWB_HbciEncryptedFwDownload(void);

#endif // UWB_HBCI_H

#endif // (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
