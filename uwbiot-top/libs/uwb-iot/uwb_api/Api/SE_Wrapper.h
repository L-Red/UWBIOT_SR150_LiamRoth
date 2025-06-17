/* Copyright 2021-2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef _SE_WRAPPER_
#define _SE_WRAPPER_

#ifdef __cplusplus
extern "C" {
#endif

#include <sus_APDU.h>
#include "se_FiRaLite_API.h"
#include "phOsalUwb.h"

#define SUS_TAG_GET_VERSION                  (0x40)
#define SUS_TAG_GET_APPLET_LIFE_CYCLE        (0x42)
#define SUS_TAG_GET_BINDING_HISTORY          (0x45)
#define SUS_TAG_GET_BINDING_STATE            (0x46)
#define SUS_GET_DATA_UWB_SYSTEM_HISTORY_SIZE (17)

#define SE_MAX_APPLET_LIFE_CYCLE_STATE_SIZE (16)
#define SE_MAX_BINDING_HISTORY_SIZE         (54)
#define SE_MAX_BINDING_STATE_SIZE           (16)

#define SUS_GET_VERSION_RSP           (0x03)
#define SUS_GET_BINDING_STATE_RSP     (0x03)
#define SUS_GET_APPLET_LIFE_CYCLE_RSP (0x01)

#define SUS_GET_VERSION_RSP           (0x03)
#define SUS_GET_BINDING_STATE_RSP     (0x03)
#define SUS_GET_APPLET_LIFE_CYCLE_RSP (0x01)

#define FIRALITE_TAG_GET_VERSION_MSB (0x9F)
#define FIRALITE_TAG_GET_VERSION_LSB (0x7E)
#define FIRALITE_VERSION_LEN         (0x04)

#define SE_CHANNEL_1 (0x01)

/** \addtogroup se_apis
 * \{ */

/**
 * \brief       FiRaLite Status.
 */
/* @{ */
typedef enum
{
    FIRALITE_STATUS_TRANSACTION_COMPLETE_WITH_NO_ERRORS            = 0x00,
    FIRALITE_STATUS_COMMAND_PROCESSED_RETURN_TO_COUNTERPART_DEVICE = 0x80,
    FIRALITE_STATUS_COMMAND_PROCESSED_RETURN_TO_HOST_APP           = 0x81,
    FIRALITE_STATUS_TRANSACTION_COMPLETE_WITH_ERRORS               = 0xFF
} firalite_status;
/* @}*/

/**
 * \brief       SE Bind State.
 */
/* @{ */
typedef enum
{
    SE_STATUS_OK     = 0x9000,
    SE_STATUS_NOT_OK = 0xFFFF,
} se_status_t;
/* @}*/

/**
 * \brief       SE Bound status.
 */
/* @{ */
typedef enum
{
    SE_Not_Bound = 0x00,
    SE_Bound_And_Unlocked,
    SE_Bound_And_Locked,
    SE_Bound_NA = 0xFF,
} se_boundStatus_t;
/* @}*/

/**
 * \brief       SE Binding State.
 */
/* @{ */
typedef struct
{
    /* bound status see :cpp:type:'se_boundStatus_t' */
    se_boundStatus_t boundStatus;
    /* There is no factory reset. There is a factory reset counter for
    compatibility, which should always say 0 factory reset possible */
    uint8_t factoryResetCounter;
    /* remaining binding attempts (only when bound & unlocked). This value
    shall be set to '00' if bound Status is 2 (Bound & Locked).*/
    uint8_t bindingAttempts;
} se_bindState_t;
/* @}*/

/**
 * \brief       Applet Life Cycle.
 */
/* @{ */
typedef enum
{
    SE_None         = 0x00,
    SE_Personalized = 0x02,
    SE_Ready,
    SE_Locked,
} se_lifeCycleStatus_t;
/*@}*/

/**
 * \brief        Applet types.
 */
/* @{ */
typedef enum
{
    /* Select SUS applet*/
    SE_APPLET_SUS = 0x00,
    /* Select SUSClient applet */
    SE_APPLET_SUS_CLIENT = 0x01,
    /* Select FiRaLite applet */
    SE_APPLET_FIRALITE = 0x02,
} se_applet_t;
/* @}*/

/**
 * \brief  FiraLite Wrapped RDS, command buffer from dispatch.
 */
/* @{ */
typedef struct
{
    uint8_t *wrappedRDS;
    size_t wrappedRDSLen;
} firaLiteWrappedRds_t;
/* @}*/

/**
 * \brief RDS structure to use depending on applet SUSClient/FiraLite.
 */
/* @{ */
typedef union {
    firaLiteWrappedRds_t *pFlWrappedRds;
    se_plainRDS *pSusPlaindRds;
} se_rds_t;
/* @}*/

/**
 * \brief    UWB subystem entry.
 */
/* @{ */
typedef struct
{
    /* 16 bytes UWB subsystem ID */
    uint8_t uwbSystmId[16];
    /* Binding count */
    uint8_t bindCnt;
} uwbSystmHistory_t;
/* @}*/

/**
 * \brief       UWB-SE Binding History Context.
 */
/* @{ */
typedef struct
{
    /* total number of times the SUS was bound to a UWB subsystem.
    This count increases by one after each binding.*/
    uint16_t totalBindingCnt;
    /* number of distinct UWB subsystems which SUS was bound to. This value increases
     by one after each binding with a UWB subsystem ID different than the current UWB subsystem. */
    uint8_t uwbSystmCnt;
    /* The UWB subsystem entries (up to 3 entries) */
    uwbSystmHistory_t uwbSystm1;
    uwbSystmHistory_t uwbSystm2;
    uwbSystmHistory_t uwbSystm3;
} se_bindingHistory_t;

typedef se_bindingHistory_t *pBindHistry_t;
/* @}*/

/**
 * \brief       SE Session Context.
 */
/* @{ */
typedef struct
{
    pSe05xSession_t se_ctx;
    se_applet_t applet;
} se_SusSession_t;

typedef se_SusSession_t *pSusSession_t;
/* @}*/

/**
 * \brief APIs exposed to application to access SE051 UWB Functionality.
 */

/** \brief Initializes Communication.
 *  This function sets the SE handle for communication.
 *
 * \param[in] se_ctx connection context
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_SetHandle(void *se_ctx);

/** \brief DeInitialise Communication.
 *  This function clears the SE handle for communication..
 */
EXTERNC void Se_API_ResetHandle(void);

/** \brief Select the desired applet.
 *  The function selects the Fira applet for further communication.
 *
 * \param[in] applet  applet to select See :cpp:type:`se_applet_t`
 * \param[in] logical_channel  Logical channel to be used for SE communication
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_Init(se_applet_t applet, uint8_t logical_channel);

/** \brief Get SE bind state.
 *  The function gets the SE bind state.
 *
 * \param[out] pBindState returns binding State. see :cpp:type:'se_bindState_t'
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_GetBindingState(se_bindState_t *pBindState);

/** \brief Get SUS Applet Version.
 *  The function gets the version of SUS applet.
 *
 * \param[out] verString returns applet version string.
 * \param[out] verStringLen returns len of applet version string.
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_GetVersion(uint8_t *verString, size_t *verStringLen);

/** \brief Get FiraLite Applet Version.
 *  The function gets the version of FiraLite applet.
 *
 * \param[out] verString returns applet version string.
 * \param[out] verStringLen returns len of applet version string.
 * \param[out] verDiscription returns version description in ASCII format.
 * \param[out] verDiscriptionLen returns len of version description.
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_GetFiraLiteVersion(
    uint8_t *verString, size_t *verStringLen, char *verDiscription, size_t *verDiscriptionLen);

/** \brief Get SUS Applet Life Cycle State.
 *  The function gets the Life cycle state.
 *
 * \param[out] pLcState returns applet life cycle state See :cpp:type:`se_lifeCycleStatus_t`.
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_GetLifeCycle(se_lifeCycleStatus_t *pLcState);

/** \brief Get Binding History.
 *  The function gets the binding history of applet with UWB subsystem.
 *
 * \param[out] pBindHistry returns binding history See :cpp:type:`se_bindingHistory_t`.
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_GetBindingHistory(pBindHistry_t pBindHistry);

/** \brief Send Receive to SE.
 *  The function is used to send and received data to SE.
 *
 * \param[in] pInData Input buffer
 * \param[in] InDataLen Length of the input in bytes
 * \param[out] pOutData Output buffer
 * \param[out] pOutDataLen Length of the output in bytes
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_SendReceive(uint8_t *pInData, size_t InDataLen, uint8_t *pOutData, size_t *pOutDataLen);

/** \brief Send Initiate Binding APDU to SE .
 *  The function is used to start the binding process  to SE.
 *
 * \param[in] Lc Lifecycle state as input
 * \param[in] brk brk Id as input
 * \param[in] pbindInData Input buffer
 * \param[in] bindInDataLen Length of the input in bytes
 * \param[out] pbindOutData Output buffer
 * \param[out] pbindOutDataLen Length of the output in bytes
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_InitiateBinding(uint8_t Lc,
    uint8_t brk,
    uint8_t *pbindInData,
    size_t bindInDataLen,
    uint8_t *pbindOutData,
    size_t *pbindOutDataLen);

/** \brief Utility function to get Wrapped RDS from SUSClient/FiraLite
 *  In case of FiraLite Get the wrapped RDS from dispatch buffer
 *
 * \param[in] applet Applet FiraLite/SUS Client
 * \param[in] pRds Union containing rds data for FiraLite/SUS Client
 * \param[out] pWrappedRds wrapped RDS as Output buffer
 * \param[out] pWrappedRdsLen Length of the output in bytes
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_GetWrappedRDS(
    se_applet_t applet, se_rds_t *pRds, uint8_t *pWrappedRds, size_t *pWrappedRdsLen);

/** \brief Send plain RDS Data to SE.
 *  The function is used to get wrapped RDS from SUS Client.
 *  se_plainRDS is converted into final RDS TLV payload.
 *  The maximum TLV Length of the plain RDS is allowed to 223 bytes.
 *  Please refer applet spec.
 *
 * \param[in] pRdsData plain RDS Data as input
 * \param[out] pWrappedRds wrapped RDS as Output buffer
 * \param[out] pWrappedRdsLen Length of the output in bytes
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_WrapData(se_plainRDS *pRdsData, uint8_t *pWrappedRds, size_t *pWrappedRdsLen);

/** \brief Close the channel and deselect the applet.
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_DeInit(void);

#define SE_API_ALLOW_GET_BDI 0

#if SE_API_ALLOW_GET_BDI

/** Temporary api to get bdi
 * \param[in] pkey  heliosId
 * \param[in] keyLen  length of heliosId
 * \param[in] srcData  RAND_HE || RAND_SE || SE ID
 * \param[in] srcDataLen  length of srcData content
 * \param[out] outBdi  outbuffer holding bdi
 * \param[out] outBdiLen  length of bdi
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_GetBdi(
    uint8_t *pkey, size_t keyLen, uint8_t *srcData, size_t srcDataLen, uint8_t *outBdi, size_t *outBdiLen);
#endif

/** \brief Send SelectADF command to SE.
 * \param[in]  optsA  type of crypto
 * \param[in]  oid_entries  reference to OID entries
 * \param[in]  oid_entries_count  number of OID entries
 * \param[out] pResponse  buffer containing the response
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_SelectADF(const se_firalite_optsa_t optsA,
    se_firelite_oid_entry *oid_entries,
    const size_t oid_entries_count,
    se_firelite_selectadf_reponse_t *pResponse);

/** \brief Send InitiateTransaction command to SE.
 * \param[in]  oid_entries  reference to OID entries
 * \param[in]  oid_entries_count  number of OID entries
 * \param[in]  pSessionId  Optional reference to the UWB session id used in case of Multicast Ranging
 * \param[in]  sessionIdLen  length of the UWB session id if pSessionId is not null
 * \param[out] pStatus  pointer to the status
 * \param[out] pDataBuffer  buffer containing CAPDU (must be large enough to hold SE response)
 * \param[out] pDataLen  length of the CAPDU
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_InitiateTransaction(se_firelite_oid_entry *oid_entries,
    const size_t oid_entries_count,
    uint8_t *pSessionId,
    const size_t sessionIdLen,
    uint8_t *pStatus,
    uint8_t *pDataBuffer,
    size_t *pDataLen);

/** \brief Send Dispatch command to SE.
 * \param[in] pDispatchData  pointer to the data to be dispatched
 * \param[in] dispatchDataLen  length of the data to be dispatched
 * \param[out] pStatus  pointer to the status
 * \param[out] pDataBuffer  buffer containing the response data
 * \param[out] pDataLen  length of the response data (0 if not part of response)
 * \param[out] pEventId  pointer to the event Id
 * \param[out] pEventDataBuffer  buffer containing the event data
 * \param[out] pEventDataLen  length of the event data
 *
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
 */
EXTERNC se_status_t Se_API_Dispatch(uint8_t *pDispatchData,
    size_t dispatchDataLen,
    uint8_t *pStatus,
    uint8_t *pDataBuffer,
    size_t *pDataLen,
    uint8_t *pEventId,
    uint8_t *pEventDataBuffer,
    size_t *pEventDataLen);

/** \brief Send GetData command over tunnel to SE.
* \param[in]  pInBuf  command Buffer
* \param[in]  inBufLen  command Buffer length
* \param[out] pStatus  pointer to the status
* \param[out] pDataBuffer  buffer containing the response data
* \param[out] pDataLen  length of the response data
*
* \returns Status of the operation
* \retval SE_STATUS_OK The operation has completed successfully.
* \retval SE_STATUS_NOT_OK The operation has failed.
*/
EXTERNC se_status_t Se_API_RemoteGetData(
    const uint8_t *pInBuf, const size_t inBufLen, uint8_t *pStatus, uint8_t *pDataBuffer, size_t *pDataLen);

/** \brief Send PutData command over tunnel to SE.
* \param[in]  pInBuf  command Buffer
* \param[in]  inBufLen  command Buffer length
* \param[out] pStatus  pointer to the status
* \param[out] pDataBuffer  buffer containing the response data
* \param[out] pDataLen  length of the response data
*
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
*/
EXTERNC se_status_t Se_API_RemotePutData(
    const uint8_t *pInBuf, const size_t inBufLen, uint8_t *pStatus, uint8_t *pDataBuffer, size_t *pDataLen);

/** \brief Send GetData command to local SE.
* \param[in]  tagp1  Tag P1
* \param[in]  tagp2  Tag P2
* \param[out] pDataBuffer  buffer containing the response data
* \param[out] pDataLen  length of the response data
*
 * \returns Status of the operation
 * \retval SE_STATUS_OK The operation has completed successfully.
 * \retval SE_STATUS_NOT_OK The operation has failed.
*/
EXTERNC se_status_t Se_API_LocalGetData(
    const uint8_t tagp1, const uint8_t tagp2, uint8_t *pDataBuffer, size_t *pDataLen);
/** \brief Send PutData command to local SE.
* \param[in]  pInBuf  command Buffer
* \param[in]  inBufLen  command Buffer length
*
* \returns Status of the operation
* \retval SE_STATUS_OK The operation has completed successfully.
* \retval SE_STATUS_NOT_OK The operation has failed.
*/
EXTERNC se_status_t Se_API_LocalPutData(const uint8_t *pInBuf, const size_t inBufLen);

/** \brief Send Get Data command to Remote SE. To be sent in plain without tunnel.
* \param[in]  pInBuf  command Buffer
* \param[in]  inBufLen  command Buffer length
* \param[out] pRspBuffer buffer containing CAPDU will be sent OOB (must be large enough to hold SE response)
* \param[out] pRspBufLen  length of the CAPDU
*
* \returns Status of the operation
* \retval SE_STATUS_OK The operation has completed successfully.
* \retval SE_STATUS_NOT_OK The operation has failed.
*/
EXTERNC se_status_t Se_API_RemoteGetData_WithoutTunnel(
    const uint8_t *pInBuf, const size_t inBufLen, uint8_t *pRspBuffer, size_t *pRspBufLen);

/** \brief Send Put Data command to Remote SE. To be sent in plain without tunnel.
* \param[in]  pInBuf  command Buffer
* \param[in]  inBufLen  command Buffer length
* \param[out] pRspBuffer buffer containing CAPDU will be sent OOB (must be large enough to hold SE response)
* \param[out] pRspBufLen  length of the CAPDU
*
* \returns Status of the operation
* \retval SE_STATUS_OK The operation has completed successfully.
* \retval SE_STATUS_NOT_OK The operation has failed.
*/
EXTERNC se_status_t Se_API_RemotePutData_WithoutTunnel(
    const uint8_t *pInBuf, const size_t inBufLen, uint8_t *pRspBuffer, size_t *pRspBufLen);

/**
 * @}
 */

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif
