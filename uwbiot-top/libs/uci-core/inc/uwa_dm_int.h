/*
 *
 * Copyright 2018-2020,2022-2023 NXP.
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

#ifndef UWA_DM_INT_H
#define UWA_DM_INT_H

#include "uwa_api.h"
#include "uwa_sys.h"
#include "uci_ext_defs.h"
#include "uci_defs.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* UWA_DM flags */
/* DM is enabled                                                        */
#define UWA_DM_FLAGS_DM_IS_ACTIVE      0x00000001
#define SES_ID_AND_NO_OF_PARAMS_OFFSET 0x05

/**GID: Proprietary group SHIFT */
#define GID_PROP_GROUP_SHIFT 0x08

/**GID: 1110(0x0E) Proprietary  group*/
#define GID_PROP_GROUP 0x0E

/**GID: 1111(0x0F) Vendor group*/
#define GID_VENDOR_GROUP 0x0F

/** GID: 1010(0x0A) IOT Proprietary  group*/
#define GID_IOT_PROP_GROUP 0x0A

/**GID: 1110(Ox0E) Proprietary  group OIDS*/
#define EXT_DM_MSG_CORE_DEVICE_INIT                                         0x00
#define EXT_DM_MSG_DBG_DATA_LOGGER_NTF                                      0x01
#define EXT_DM_MSG_DBG_GET_ERROR_LOG                                        0x02
#define EXT_DM_MSG_SE_GET_BINDING_COUNT                                     0x03
#define EXT_DM_MSG_SE_DO_TEST_LOOP                                          0x04
#define EXT_DM_MSG_SE_COMM_ERROR_NTF                                        0x05
#define EXT_DM_MSG_SE_GET_BINDING_STATUS                                    0x06
#define EXT_DM_MSG_SCHEDULER_STATUS_NTF                                     0x07
#define EXT_DM_MSG_UWB_SESSION_KDF_NTF                                      0x08
#define EXT_DM_MSG_UWB_WIFI_COEX_IND_NTF                                    0x09
#define EXT_DM_MSG_WLAN_UWB_IND_ERR_NTF                                     0x0A
#define EXT_DM_MSG_QUERY_TEMPERATURE                                        0x0B
#define EXT_DM_MSG_GENERATE_TAG                                             0x0E
#define EXT_DM_MSG_VERIFY_CALIB_DATA                                        0x0F
#define EXT_DM_MSG_CONFIGURE_AUTH_TAG_OPTIONS_CMD                           0x10
#define EXT_DM_MSG_CONFIGURE_AUTH_TAG_VERSION_CMD                           0x11
#define EXT_DM_MSG_CALIBRATION_INTEGRITY_PROTECTION                         0x12

/**GID: 1111(Ox0F) Vendor group OIDS*/
#define VENDOR_MSG_SET_VENDOR_APP_CONFIG_CMD  0x00
#define VENDOR_MSG_URSK_DELETE_CMD            0x01
#define VENDOR_MSG_GET_ALL_UWB_SESSIONS_CMD   0x02
#define VENDOR_MSG_GET_VENDOR_APP_CONFIG_CMD  0x03
#define VENDOR_MSG_DO_VCO_PLL_CALIBRATION_CMD 0x20
#define VENDOR_MSG_SET_DEVICE_CALIBRATION_CMD 0x21
#define VENDOR_MSG_GET_DEVICE_CALIBRATION_CMD 0x22
#define VENDOR_MSG_SET_SECURE_CALIBRATION_CMD 0x23
#define VENDOR_MSG_UWB_ESE_CONNECTIVITY_CMD   0x30
#define VENDOR_MSG_UWB_ESE_BINDING_CMD        0x31
#define VENDOR_MSG_UWB_ESE_BINDING_CHECK_CMD  0x32
#define VENDOR_MSG_PSDU_LOG_NTF               0x33
#define VENDOR_MSG_CIR_LOG_NTF                0x34

/** Helper Macro to fetch Particular GID and OID  for  Proprietary Group  */
#define GET_PROP_GROUP_GID_OID(PROP_OID) ((GID_PROP_GROUP << GID_PROP_GROUP_SHIFT) | (PROP_OID))

/** Helper Macro to fetch Particular GID and OID  for  Vendor Group */
#define GET_VENDOR_GROUP_GID_OID(VENDOR_OID) ((GID_VENDOR_GROUP << GID_PROP_GROUP_SHIFT) | (VENDOR_OID))

/** Helper macro to fetch Particular GID and OID for IOT Propreitary Group */
#define GET_IOT_PROP_GROUP_GID_OID(IOT_PROP_ID) ((GID_IOT_PROP_GROUP << GID_PROP_GROUP_SHIFT) | (IOT_PROP_ID))

/* DM events */
typedef enum Command_Event
{
    /* device manager local device API events */
    UWA_DM_API_ENABLE_EVT = UWA_SYS_EVT_START(UWA_ID_DM),
    UWA_DM_API_DISABLE_EVT,
    UWA_DM_API_REGISTER_EXT_CB_EVT,
    UWA_DM_API_SEND_RAW_EVT,
    UWA_DM_INTERNAL_NUM_ACTIONS,
    UWA_DM_API_CORE_GET_DEVICE_INFO_EVT,
    UWA_DM_API_CORE_SET_CONFIG_EVT,
    UWA_DM_API_CORE_GET_CONFIG_EVT,
    UWA_DM_API_CORE_DEVICE_RESET_EVT,
    UWA_DM_API_SESSION_INIT_EVT,
    UWA_DM_API_SESSION_DEINIT_EVT,
    UWA_DM_API_SESSION_GET_COUNT_EVT,
    UWA_DM_API_SESSION_SET_APP_CONFIG_EVT,
    UWA_DM_API_SESSION_GET_APP_CONFIG_EVT,
    UWA_DM_API_SESSION_START_EVT,
    UWA_DM_API_SESSION_STOP_EVT,
    UWA_DM_API_SESSION_GET_STATE_EVT,
    UWA_DM_API_CORE_GET_CAPS_INFO_EVT,
    UWA_DM_API_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_EVT,
#if !(UWBIOT_UWBD_SR040)
    UWA_DM_API_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_EVT,
    UWA_DM_API_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_EVT,
    UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_EVT,
    UWA_DM_API_SESSION_SET_HUS_CONTROLLER_CONFIG_EVT,
    UWA_DM_API_SESSION_SET_HUS_CONTROLEE_CONFIG_EVT,
    UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_EVT,
    /*    UWB RF Test API events   */
    UWA_DM_API_TEST_SET_CONFIG_EVT,
    UWA_DM_API_TEST_GET_CONFIG_EVT,
    UWA_DM_API_TEST_PERIODIC_TX_EVT,
    UWA_DM_API_TEST_PER_RX_EVT,
    UWA_DM_API_TEST_UWB_LOOPBACK_EVT,
    UWA_DM_API_TEST_RX_EVT,
    UWA_DM_API_TEST_STOP_SESSION_EVT,
    /* Proprietary event */
    UWA_DM_API_PROP_CALIB_INTEGRITY_PROTECTION,
    UWA_DM_API_PROP_GENERATE_TAG,
    UWA_DM_API_PROP_VERIFY_CALIB_DATA,
    UWA_DM_API_PROP_DO_BIND,
    UWA_DM_API_PROP_GET_BINDING_COUNT,
    UWA_DM_API_PROP_TEST_CONNECTIVITY,
    UWA_DM_API_PROP_TEST_SE_LOOP,
    UWA_DM_API_PROP_GET_BINDING_STATUS,
    /* Vendor event */
    UWA_DM_API_VENDOR_DO_VCO_PLL_CALIBRATION,
    UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION,
    UWA_DM_API_VENDOR_GET_DEVICE_CALIBRATION,
#endif //!(UWBIOT_UWBD_SR040)
    UWA_DM_API_VENDOR_GET_ALL_UWB_SESSIONS,
#if !(UWBIOT_UWBD_SR040)
    UWA_DM_API_PROP_QUERY_TEMP,
    UWA_DM_API_PROP_CONFIG_AUTH_TAG_OPTIONS,
    UWA_DM_API_PROP_CONFIG_AUTH_TAG_VERSIONS,
    UWA_DM_API_PROP_URSK_DELETION_REQUEST,
    UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT,
    UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT,
    UWA_DM_API_SESSION_SET_LOCALIZATION_ZONE_EVT,
#endif //!(UWBIOT_UWBD_SR040)
    UWA_DM_API_CORE_QUERY_UWBS_TIMESTAMP,
#if !(UWBIOT_UWBD_SR040)
    UWA_DM_API_PROP_WRITE_OTP_CALIB_DATA,
    UWA_DM_API_PROP_READ_OTP_CALIB_DATA,
#endif // !(UWBIOT_UWBD_SR040)
#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
    UWA_DM_API_TRNG_EVENT,
#endif
#if UWBIOT_UWBD_SR040
    UWA_DM_API_SUSPEND_DEVICE_EVENT,
    UWA_DM_API_SESSION_NVM_EVENT,
    UWA_DM_START_TEST_MODE_EVENT,
    UWA_DM_STOP_TEST_MODE_EVENT,
    UWA_DM_SET_CALIB_TRIM_EVENT,
    UWA_DM_GET_CALIB_TRIM_EVENT,
    UWA_MHR_IN_CCM,
    UWA_BYPASS_CURRENT_LIMITER,
#endif
#if (UWBIOT_UWBD_SR040 || UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
    UWA_DM_API_PROFILE_PARAM_EVENT,
#endif
    UWA_DM_API_SEND_DATA_EVENT,
    UWA_DM_MAX_EVT,
} eCommand_Event_t;

typedef enum Gid_Oid_Type
{
    /* UCI Core Group */
    CORE_DEVICE_RESET_GID_OID,
    CORE_GET_DEVICE_INFO_GID_OID = 0x0002,
    CORE_GET_CAPS_INFO_GID_OID   = 0x0003,
    CORE_SET_CONFIG_GID_OID,
    CORE_GET_CONFIG_GID_OID,
    CORE_QUERY_UWBS_TIMESTAMP_GID_OID = 0x0008,

    /* UWB Session Config Group */
    SESSION_INIT_GID_OID = 0x0100,
    SESSION_DEINIT_GID_OID,
    SESSION_SET_APP_CONFIG_GID_OID = 0x0103,
    SESSION_GET_APP_CONFIG_GID_OID,
    SESSION_GET_STATE_GID_OID                        = 0x0106,
    SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_GID_OID = 0x0107,
    SESSION_QUERY_DATA_SIZE_IN_RANGING_GID_OID       = 0x010B,
    SESSION_SET_HUS_CONTROLLER_CONFIG_GID_OID        = 0X010C,
    SESSION_SET_HUS_CONTROLEE_CONFIG_GID_OID         = 0X010D,
    SESSION_DATA_TRANSFER_PHASE_CONFIG_GID_OID       = 0x010E,

    /* UWB Session Control Group */
    SESSION_START_GID_OID = 0x0200,
    SESSION_STOP_GID_OID,

    /* Test Group */
    SET_TEST_CONFIG_GID_OID = 0x0D00,
    TEST_GET_CONFIG_GID_OID,
    TEST_PERIODIC_TX_GID_OID,
    TEST_PER_RX_GID_OID,
    TEST_RX_GID_OID = 0x0D05,
    TEST_UWB_LOOPBACK_GID_OID,
    TEST_STOP_SESSION_GID_OID,

/* Vendor Specific Group */
#if !(UWBIOT_UWBD_SR040)
    GET_BINDING_COUNT_GID_OID                      = GET_PROP_GROUP_GID_OID(EXT_DM_MSG_SE_GET_BINDING_COUNT),
    SESSION_UPDATE_DT_ANCHOR_RANGING_ROUND_GID_OID = 0x0108,
    SESSION_UPDATE_DT_TAG_RANGING_ROUND_GID_OID    = 0x0109,
    WRITE_OTP_CALIB_DATA_GID_OID                   = 0x0A00,
    READ_OTP_CALIB_DATA_GID_OID                    = 0x0A01,
    RANGING_TIMESTAMP_NTF_GID_OID                  = 0x0B03,
    TEST_SE_LOOP_GID_OID                           = GET_PROP_GROUP_GID_OID(EXT_DM_MSG_SE_DO_TEST_LOOP),
    CALIB_INTEGRITY_PROTECTION_GID_OID    = GET_PROP_GROUP_GID_OID(EXT_DM_MSG_CALIBRATION_INTEGRITY_PROTECTION),
    GENERATE_TAG_GID_OID                  = GET_PROP_GROUP_GID_OID(EXT_DM_MSG_GENERATE_TAG),
    VERIFY_CALIB_DATA_GID_OID             = GET_PROP_GROUP_GID_OID(EXT_DM_MSG_VERIFY_CALIB_DATA),
    SESSION_SET_VENDOR_APP_CONFIG_GID_OID = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_SET_VENDOR_APP_CONFIG_CMD),
    GET_ALL_UWB_SESSIONS_GID_OID          = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_GET_ALL_UWB_SESSIONS_CMD),
    SESSION_GET_VENDOR_APP_CONFIG_GID_OID = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_GET_VENDOR_APP_CONFIG_CMD),
    VENDOR_DO_VCO_PLL_CALIBRATION_GID_OID = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_DO_VCO_PLL_CALIBRATION_CMD),
    VENDOR_SET_DEVICE_CALIBRATION_GID_OID = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_SET_DEVICE_CALIBRATION_CMD),
    VENDOR_GET_DEVICE_CALIBRATION_GID_OID = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_GET_DEVICE_CALIBRATION_CMD),
    TEST_CONNECTIVITY_GID_OID             = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_UWB_ESE_CONNECTIVITY_CMD),
    DO_BIND_GID_OID                       = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_UWB_ESE_BINDING_CMD),
    GET_BINDING_STATUS_GID_OID            = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_UWB_ESE_BINDING_CHECK_CMD),
    PSDU_LOG_NTF_GID_OID                  = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_PSDU_LOG_NTF),
    CIR_LOG_NTF_GID_OID                   = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_CIR_LOG_NTF),

#endif //!(UWBIOT_UWBD_SR040)
    QUERY_TEMP_GID_OID               = GET_PROP_GROUP_GID_OID(EXT_DM_MSG_QUERY_TEMPERATURE),
    CONFIG_AUTH_TAG_OPTIONS_GID_OID  = GET_PROP_GROUP_GID_OID(EXT_DM_MSG_CONFIGURE_AUTH_TAG_OPTIONS_CMD),
    CONFIG_AUTH_TAG_VERSIONS_GID_OID = GET_PROP_GROUP_GID_OID(EXT_DM_MSG_CONFIGURE_AUTH_TAG_VERSION_CMD),
    URSK_DELETION_REQUEST_GID_OID    = GET_VENDOR_GROUP_GID_OID(VENDOR_MSG_URSK_DELETE_CMD),
#if UWBIOT_UWBD_SR040
    GET_TRNG_GID_OID               = 0x0E2D,
    GET_SUSPEND_DEVICE_GID_OID     = 0x0E24,
    GET_SESSION_NVM_GID_OID        = 0x0E2B,
    GET_START_TEST_MODE_GID_OID    = 0x0E20,
    GET_STOP_TEST_MODE_GID_OID     = 0x0E21,
    SET_CALIB_TRIM_MODE_GID_OID    = 0x0E26,
    GET_ALL_UWB_SESSIONS_GID_OID   = 0x0E27,
    GET_CALIB_TRIM_MODE_GID_OID    = 0x0E28,
    GID_OID_PROFILE_PARAM_MODE     = 0x0E2E,
    GID_OID_BYPASS_CURRENT_LIMITER = 0x0E2F,
#endif
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160)
    GET_TRNG_GID_OID           = 0x0A02,
    GID_OID_PROFILE_PARAM_MODE = 0x0A05,
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR160
#if UWBIOT_UWBD_SR150
    SESSION_SET_LOCALIZATION_ZONE_GID_OID = GET_IOT_PROP_GROUP_GID_OID(EXT_UCI_MSG_SESSION_SET_LOCALIZATION_ZONE_CMD),
#endif // UWBIOT_UWBD_SR150
    INVALID_GID_OID = 0xFFFF
} eGidOid_t;

/* data type for UWA_DM_API_ENABLE_EVT */
typedef struct
{
    UWB_HDR hdr;
    tUWA_DM_RSP_CBACK *p_dm_rsp_cback;
    tUWA_DM_NTF_CBACK *p_dm_ntf_cback;
#if UWBFTR_DataTransfer
    tUWA_DM_DATA_CBACK *p_dm_data_cback;
#endif //UWBFTR_DataTransfer
} tUWA_DM_API_ENABLE;

/* data type for UWA_DM_API_DISABLE_EVT */
typedef struct
{
    UWB_HDR hdr;
    bool graceful;
} tUWA_DM_API_DISABLE;

/* data type for UWA_DM_API_SEND_RAW_EVT */
typedef struct
{
    UWB_HDR hdr;
    tUWA_RAW_CMD_CBACK *p_cback;
    uint8_t oid;
    uint16_t cmd_params_len;
    uint8_t *p_cmd_params;
} tUWA_DM_API_SEND_RAW;

typedef struct
{
    UWB_HDR hdr;
    tUWA_RAW_CMD_CBACK *p_dm_ext_cback;
} tUWA_DM_API_REGISTER_EXT_CB;

typedef struct
{
    UWB_HDR hdr;
    uint16_t length;
    uint8_t *p_data;
    uint8_t pbf;
} tUCI_CMD;

/* union of all data types */
typedef union {
    /* GKI event buffer header */
    UWB_HDR hdr;
    tUWA_DM_API_ENABLE enable;          /* UWA_DM_API_ENABLE_EVT           */
    tUWA_DM_API_DISABLE disable;        /* UWA_DM_API_DISABLE_EVT          */
    tUWA_DM_API_REGISTER_EXT_CB ext_cb; /* UWA_DM_API_REGISTER_EXT_CB_EVT  */
    tUWA_DM_API_SEND_RAW send_raw;      /* UWA_DM_API_SEND_RAW_EVT         */
    tUCI_CMD sUci_cmd;
} tUWA_DM_MSG;

typedef struct
{
    uint32_t flags;                    /* UWA_DM flags (see definitions for UWA_DM_FLAGS_*)    */
    tUWA_DM_RSP_CBACK *p_dm_rsp_cback; /* UWA DM callback for response */
    tUWA_DM_NTF_CBACK *p_dm_ntf_cback; /* UWA DM callback for ntf */
#if UWBFTR_DataTransfer
    tUWA_DM_DATA_CBACK *p_dm_data_cback; /* UWA DM callback for data */
#endif                                   //UWBFTR_DataTransfer
} tUWA_DM_CB;

void uwa_dm_disable_complete(void);

/* UWA device manager control block */
extern tUWA_DM_CB uwa_dm_cb;

void uwa_dm_init(void);

/* Action function prototypes */
bool uwa_dm_enable(tUWA_DM_MSG *p_data);
bool uwa_dm_disable(tUWA_DM_MSG *p_data);
bool uwa_dm_register_ext_cb(tUWA_DM_MSG *p_data);
bool uwa_dm_act_send_raw_cmd(tUWA_DM_MSG *p_data);

/* Main function prototypes */
bool uwa_dm_evt_hdlr(UWB_HDR *p_msg);
void uwa_dm_sys_disable(void);

#endif /* UWA_DM_INT_H */
