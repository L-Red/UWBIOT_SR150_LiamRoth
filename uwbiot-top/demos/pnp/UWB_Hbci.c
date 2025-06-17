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

#if ((INTERNAL_FIRMWARE_DOWNLOAD == ENABLED) && UWBIOT_UWBD_SR1XXT)

#include <stdint.h>
#include "phUwbTypes.h"
#include <stdio.h>

#include "UWB_Spi_Driver_Interface.h"

#include <UwbApi.h>

#define UWB_HBCI_TAG "UWB_HBCI"
#define FW_CHUNK_LEN 2048
#define HBCI_HDR_LEN 4
#define MAX_HBCI_LEN (FW_CHUNK_LEN + HBCI_HDR_LEN + 1)

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

/* HBCI Fw DOWNLOAD COMMAND SET */

/* Class defitions */
#define FW_DWNLD_QRY_CLA 0x51
#define FW_DWNLD_ANS_CLA 0x52
#define FW_DWNLD_CMD_CLA 0x53

/* Query INS definitions */
#define FW_DWNLD_QRY_IMAGE_STATUS 0x1

/* ANS INS defintions */
#define FW_DWNLD_IMAGE_SUCCESS                  0x01
#define FW_DWNLD_HEADER_SUCCESS                 0x04
#define FW_DWNLD_QUICKBOOT_SETTINGS_SUCCESS     0x05
#define FW_DWNLD_HEADER_TOO_LARGE               0x81
#define FW_DWNLD_HEADER_PARSE_ERR               0x82
#define FW_DWNLD_INVALID_CIPHER_TYPE_CRYPTO     0x83
#define FW_DWNLD_INVALID_CIPHER_TYPE_MODE       0x84
#define FW_DWNLD_INVALID_CIPHER_TYPE_HASH       0x85
#define FW_DWNLD_INVALID_CIPHER_TYPE_CURVE      0x86
#define FW_DWNLD_INVALID_ECC_KEY_LENGTH         0x87
#define FW_DWNLD_INVALID_PAYLOAD_DESCRIPTION    0x88
#define FW_DWNLD_INVALID_FW_VERSION             0x89
#define FW_DWNLD_INVALID_ECID_MASK              0x8A
#define FW_DWNLD_INVALID_ECID_VALUE             0x8B
#define FW_DWNLD_INVALID_ENCRYPTED_PAYLOAD_HASH 0x8C
#define FW_DWNLD_INVALID_HEADER_SIGNATURE       0x8D
#define FW_DWNLD_INSTALL_SETTINGS_TOO_LARGE     0x8E
#define FW_DWNLD_PAYLOAD_TOO_LARGE              0x8F
#define FW_DWNLD_SETTINGS_PARSE_ERR             0x90
#define FW_DWNLD_QUICKBOOT_SETTINGS_PARSE_ERR   0x91
#define FW_DWNLD_INVALID_STATIC_HASH            0x92
#define FW_DWNLD_INVALID_DYNAMIC_HASH           0x93
#define FW_DWNLD_EXECUTION_SETTINGS_ERR         0x94
#define FW_DWNLD_KEY_READ_ERR                   0x95

/* CMD INS defintions */
#define FW_DWNLD_DWNLD_IMAGE 0x01

#define SEG_PACKET   0x08
#define FINAL_PACKET 0x00

#define HBCI_HDR(arr, CLA, INS, SEG) \
    {                                \
        arr[0] = CLA;                \
        arr[1] = INS;                \
        arr[3] = ((SEG) << 4);       \
    }

typedef struct
{
    uint8_t *data;
    uint16_t len;
    uint8_t seg;
    uint8_t crc;
} hbci_packet_t;

static uint8_t mHbciSndBuf[MAX_HBCI_LEN];
static uint8_t mHbciRcvBuf[MAX_HBCI_LEN];

static bool hbci_check(hbci_packet_t *packet, uint8_t cla, uint8_t ins, uint8_t seg)
{
    bool ok = (packet != NULL) && (packet->len > 0) && (packet->data[0] == cla) && (packet->data[1] == ins) &&
              ((packet->data[2] >> 4) == seg);

    if (!ok) {
        PRINTF("Unexpected CLA/INS/SEG : %02x %02x %02x\n", packet->data[0], packet->data[1], packet->data[2] >> 4);
    }
    return ok;
}
static void hbci_prepare(hbci_packet_t *packet, uint8_t cla, uint8_t ins, uint8_t seg)
{
    HBCI_HDR(packet->data, cla, ins, seg);
    packet->seg = seg;
    packet->len = HBCI_HDR_LEN;
    packet->crc = 0;
}

static bool hbci_add(hbci_packet_t *packet, const uint8_t *payload, uint16_t size)
{
    if (packet->len + size > MAX_HBCI_LEN) {
        return FALSE;
    }
    phOsalUwb_MemCopy(&packet->data[packet->len], payload, size);
    packet->len += size;

    return TRUE;
}

static bool hbci_done(hbci_packet_t *packet)
{
    if (packet->seg == FINAL_PACKET) {
        if (packet->len == HBCI_HDR_LEN) {
            packet->data[2] = (packet->len - HBCI_HDR_LEN) & 0xFF;
            packet->data[3] |= ((packet->len - HBCI_HDR_LEN) >> 8) & 0x0F;
        }
        else {
            packet->data[2] = (packet->len + 1 - HBCI_HDR_LEN) & 0xFF;
            packet->data[3] |= ((packet->len + 1 - HBCI_HDR_LEN) >> 8) & 0x0F;
        }
    }
    else {
        packet->data[2] = 0;
        packet->data[3] &= 0xF0;
    }

    if (packet->len > HBCI_HDR_LEN) {
        //Add CRC. Be aware that CRC is not included in the payload size in the packet header
        packet->crc = 0;
        for (int i = 0; i < packet->len; i++) {
            packet->crc += packet->data[i];
        }
        packet->crc               = (packet->crc ^ 0xFF) + 1;
        packet->data[packet->len] = packet->crc;
        packet->len++;
    }

#if 0
    PRINTF("PACKET Tx:\n");
    for(int i = 0; i < packet->len; i++) {
        PRINTF("%02x", packet->data[i]);
    }
    PRINTF("\n");
#endif
    return TRUE;
}

static void hbci_transceive(hbci_packet_t *snd, int sndBegin, int sndLen, hbci_packet_t *rcv)
{
    rcv->len = MAX_HBCI_LEN;
    if (kUWBSTATUS_SUCCESS == UWB_SpiHbciXfer(&snd->data[sndBegin], sndLen, rcv->data, (size_t *)&rcv->len)) {
#if 0
    PRINTF("PACKET Rx:\n");
    for(int i = 0; i < rcv->len; i++) {
        PRINTF("%02x", rcv->data[i]);
    }
    PRINTF("\n");
#endif
    }
    else {
        PRINTF("hbci_transceive(): ERROR\n");
        rcv->len  = 0;
        rcv->data = 0;
    }
}

static void hbci_wait_ready(void)
{
    hbci_GPIOwait_ready();
}

static void hbci_transceive_hdr(hbci_packet_t *snd, hbci_packet_t *rcv)
{
    hbci_transceive(snd, 0, HBCI_HDR_LEN, rcv);
}

static void hbci_transceive_payload(hbci_packet_t *snd, hbci_packet_t *rcv)
{
    hbci_transceive(snd, HBCI_HDR_LEN, snd->len - HBCI_HDR_LEN, rcv);
}

bool UWB_HbciEncryptedFwDownload(void)
{
    hbci_packet_t snd;
    hbci_packet_t rcv;
    int fwSize, i, numChunks;

    // Prepare output buffer
    snd.data = mHbciSndBuf;
    rcv.data = mHbciRcvBuf;
    /* set HBCI MODE */
    UWB_Tml_Set_Mode(kUWB_UWBS_TML_MODE_HBCI);
    // HBCI QUERY
    hbci_prepare(&snd, GENERAL_QRY_CLA, QRY_STATUS_INS, FINAL_PACKET);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, GENERAL_ANS_CLA, ANS_HBCI_READY_INS, FINAL_PACKET)) {
        // Wrong packet header
        PRINTF("Wrong response to [GENERAL_QRY_CLA, QRY_STATUS_INS]\n");
        return FALSE;
    }

    // HIF MODE
    hbci_prepare(&snd, GENERAL_CMD_CLA, CMD_MODE_HIF_INS, FINAL_PACKET);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, GENERAL_ACK_CLA, ACK_VALID_APDU_INS, FINAL_PACKET)) {
        // Wrong packet header
        PRINTF("Wrong response to [GENERAL_CMD_CLA, CMD_MODE_HIF_INS]\n");
        return FALSE;
    }

    // HIF MODE STATUS QUERY
    hbci_prepare(&snd, GENERAL_QRY_CLA, QRY_STATUS_INS, FINAL_PACKET);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, GENERAL_ANS_CLA, ANS_MODE_PATCH_HIF_READY_INS, FINAL_PACKET)) {
        // Wrong packet header
        PRINTF("Wrong response to [GENERAL_QRY_CLA, QRY_STATUS_INS]\n");
        return FALSE;
    }

    // Download FW
    fwSize = heliosEncryptedMainlineFwImageLen;
    if (fwSize == 0 || fwSize < 0) {
        PRINTF("Invalid fw image size\n");
        return FALSE;
    }
    numChunks = fwSize / FW_CHUNK_LEN + ((fwSize % FW_CHUNK_LEN == 0) ? 0 : 1);

    for (i = 0; i < numChunks; i++) {
        uint8_t seg;
        uint16_t chunkLen;
        //PRINTF("FW Image %d/%d\n", (i+1), numChunks);

        // DEVICE STATUS
        if (i < numChunks - 1) {
            seg      = SEG_PACKET;
            chunkLen = FW_CHUNK_LEN;
        }
        else {
            seg      = FINAL_PACKET;
            chunkLen = fwSize - (numChunks - 1) * FW_CHUNK_LEN;
        }

        hbci_prepare(&snd, FW_DWNLD_CMD_CLA, FW_DWNLD_DWNLD_IMAGE, seg);
        if (!hbci_add(&snd, &heliosEncryptedMainlineFwImage[i * FW_CHUNK_LEN], chunkLen)) {
            PRINTF("Error adding payload to packet\n");
            return FALSE;
        }

        hbci_done(&snd);
        hbci_transceive_hdr(&snd, &rcv);
        /*FW download stuck here if logs are disable so adding some delay*/
        phOsalUwb_Delay(10);

        if (!hbci_check(&rcv, GENERAL_ACK_CLA, ACK_VALID_APDU_INS, FINAL_PACKET)) {
            // Wrong packet header
            PRINTF("Wrong response to [FW_DWNLD_CMD_CLA, FW_DWNLD_DWNLD_IMAGE]\n");
            return FALSE;
        }

        uwb_port_DelayinMicroSec(250); // WAR for B2

        hbci_transceive_payload(&snd, &rcv);
        if (!hbci_check(&rcv, GENERAL_ACK_CLA, ACK_VALID_APDU_INS, FINAL_PACKET)) {
            // Wrong packet header
            PRINTF("Wrong response to payload\n");
            return FALSE;
        }
    }
#if UWBIOT_OS_FREERTOS
    hbci_wait_ready();
#else
    phOsalUwb_Delay(60);
#endif

    // HBCI QUERY
    hbci_prepare(&snd, FW_DWNLD_QRY_CLA, FW_DWNLD_QRY_IMAGE_STATUS, FINAL_PACKET);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, FW_DWNLD_ANS_CLA, FW_DWNLD_IMAGE_SUCCESS, FINAL_PACKET)) {
        // Wrong packet header
        PRINTF("Wrong response to [FW_DWNLD_QRY_CLA, FW_DWNLD_QRY_IMAGE_STATUS]\n");
        return FALSE;
    }
    /* set back to UCI MODE */
    UWB_Tml_Set_Mode(kUWB_UWBS_TML_MODE_UCI);
    PRINTF("HELIOS FW download completed\n");
    return TRUE;
}

#if 0
bool UWB_HbciFwDownload(void) {
    hbci_packet_t snd;
    hbci_packet_t rcv;

    // Prepare output buffer
    snd.data = mHbciSndBuf;

    int fwSize, i, numChunks;

    // HBCI QUERY
    hbci_prepare(&snd, GENERAL_QRY_CLA, QRY_STATUS_INS, FINAL_PACKET);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, GENERAL_ANS_CLA,  ANS_HBCI_READY_INS, FINAL_PACKET)) {
        // Wrong packet header
        PRINTF("Wrong response to [GENERAL_QRY_CLA, QRY_STATUS_INS]\n");
        return FALSE;
    }

    // TEST_OS MODE
    hbci_prepare(&snd, GENERAL_CMD_CLA, CMD_MODE_TEST_OS_INS, FINAL_PACKET);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, GENERAL_ACK_CLA,  ACK_VALID_APDU_INS, FINAL_PACKET)) {
        // Wrong packet header
        PRINTF("Wrong response to [GENERAL_CMD_CLA, CMD_MODE_TEST_OS_INS]\n");
        return FALSE;
    }

    // TEST_OS MODE STATUS QUERY
    hbci_prepare(&snd, GENERAL_QRY_CLA, QRY_STATUS_INS, 0);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, GENERAL_ANS_CLA,  ANS_MODE_TEST_OS_READY_INS, 0)) {
        // Wrong packet header
        PRINTF("Wrong response to [GENERAL_QRY_CLA, QRY_STATUS_INS]\n");
        return FALSE;
    }

    // DEVICE STATUS
    hbci_prepare(&snd, TEST_OS_QRY_CLA, TEST_OS_DEVICE_STATUS_INS, FINAL_PACKET);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, TEST_OS_ANS_CLA, TEST_OS_DEVICE_UNLOCKED_INS , FINAL_PACKET)) {
        // Wrong packet header
        PRINTF("Wrong response to [TEST_OS_QRY_CLA, TEST_OS_DEVICE_STATUS_INS]\n");
        return FALSE;
    }

    // Download FW
    fwSize = sizeof(heliosFwImage) / sizeof(heliosFwImage[0]);
    if (fwSize == 0 || fwSize < 0) {
        PRINTF("Invalid fw image size\n");
        return FALSE;
    }
    numChunks = fwSize / FW_CHUNK_LEN + ((fwSize % FW_CHUNK_LEN == 0) ? 0 : 1);

    for(i = 0; i < numChunks; i++) {
        uint8_t seg;
        uint16_t chunkLen;
        PRINTF("FW Image %d/%d\n", (i+1), numChunks);

        // DEVICE STATUS
        if (i < numChunks - 1) {
            seg = SEG_PACKET;
            chunkLen = FW_CHUNK_LEN;
        } else {
            seg = FINAL_PACKET;
            chunkLen = fwSize - (numChunks - 1) * FW_CHUNK_LEN;
        }
        hbci_prepare(&snd, TEST_OS_CMD_CLA, TEST_OS_DOWNLOAD_PAYLOAD_INS, seg);

        if (!hbci_add(&snd, &heliosFwImage[i * FW_CHUNK_LEN], chunkLen)) {
            PRINTF("Error adding payload to packet\n");
            return FALSE;
        }

        hbci_done(&snd);
        hbci_transceive_hdr(&snd, &rcv);

        if (!hbci_check(&rcv, GENERAL_ACK_CLA, ACK_VALID_APDU_INS , FINAL_PACKET)) {
            // Wrong packet header
            PRINTF("Wrong response to [TEST_OS_CMD_CLA, TEST_OS_DOWNLOAD_PAYLOAD_INS]\n");
            return FALSE;
        }

        hbci_transceive_payload(&snd, &rcv);
        if (!hbci_check(&rcv, GENERAL_ACK_CLA, ACK_VALID_APDU_INS , FINAL_PACKET)) {
            // Wrong packet header
            PRINTF("Wrong response to payload\n");
            return FALSE;
        }
    }

    // HBCI QUERY
    hbci_prepare(&snd, TEST_OS_QRY_CLA, TEST_OS_PAYLOAD_STATUS_INS, FINAL_PACKET);
    hbci_done(&snd);
    hbci_transceive_hdr(&snd, &rcv);
    if (!hbci_check(&rcv, TEST_OS_ANS_CLA,  TEST_OS_PAYLOAD_SUCCESS_INS, FINAL_PACKET)) {
        // Wrong packet header
        PRINTF("Wrong response to [TEST_OS_QRY_CLA, TEST_OS_PAYLOAD_STATUS_INS]\n");
        return FALSE;
    }

    return TRUE;
}
#endif // 0

#endif // INTERNAL_FIRMWARE_DOWNLOAD
