/*
 * Copyright 2021-2022 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 */
#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SE_Wrapper.h"
#include "UWB_Wrapper.h"

/**
  * \brief  Enumeration to define the Operation type.
  */
typedef enum Operation_Type
{
    BINDING    = 0x01,
    NONBINDING = 0x02
} eOperation_Type_t;

/**
  * \brief Structure for storing  UWB SE Context.
  */
typedef struct phUwbSeContext
{
    /* Contains command that needs to be sent to SE from uwb */
    phApduResponse_t ApduCmd;
    /* Contains response that needs to be sent to uwb from SE */
    phApduResponse_t ApduResp;
    /* This is used as the response legnth from SE side */
    size_t seRspLen;
    /** Get Binding state */
    se_bindState_t bindState;
    /** FIRA Applet */
    se_applet_t applet;
    /* Contains BDI related params. */
    bdi_ctx_t bindingContext;

} phUwbSeContext_t;

EXTERNC phUwbSeContext_t uwbSeContext;

/**
 * \brief Performs Factory Binding only if the current state is not bound and
 * not locked.
 *
 * \retval #UWBAPI_STATUS_OK               on success
 * \retval #UWBAPI_STATUS_FAILED           otherwise
 */
EXTERNC tUWBAPI_STATUS Binding_Process(void);

/**
  * \brief Performs Locking only if the current state is bound and
  *        Unlocked. This is only supported with Helios Mainline Firmware.
  *
  * \retval #UWBAPI_STATUS_OK               on success
  * \retval #UWBAPI_STATUS_FAILED           otherwise
  */
EXTERNC tUWBAPI_STATUS locking_Process(void);

/**
 * \brief Sets up Secure channel depending on the operation type.
 * During Bidning, extra step of initiating binding process will be done.
 * Initialize Update and External authenticate will be done to
 * establish the secure channel.
 * Before calling this interface, applet selection shall be done by the caller.
 * Upon any failure, deselection of the applets shall be done by the caller.
 *
 * \param[in] bOpType                 Operation Type Binding or nonbinding.
 *
 * \retval #UWBAPI_STATUS_OK                 on success
 * \retval #UWBAPI_STATUS_FAILED             otherwise
 */
tUWBAPI_STATUS Authenticate_Scp03(eOperation_Type_t bOpType);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif // STATEMACHINE_H
