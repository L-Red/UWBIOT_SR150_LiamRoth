/* Copyright 2019,2020 NXP
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SEMS_LITE_AGENT_CONTEXT_INC
#define SEMS_LITE_AGENT_CONTEXT_INC

#include <sm_types.h>

#include "fsl_sss_api.h"
#include "sems_lite_agent_common.h"
#include "se05x_tlv.h"

//#include "fsl_sss_api.h"
#if SSS_HAVE_APPLET_SE05X_IOT
//#include "fsl_sss_se05x_types.h"
#endif

/* *****************************************************************************************************************
 * Types/Structure Declarations
 * ***************************************************************************************************************** */

/**
 * @addtogroup sems_lite_agent_types
 * @{
 */

typedef struct
{
    int current_command_index;
    int checkpoint_index;
    int retry_count;
} sems_lite_agent_request_ctx_t;

typedef struct
{
    /** Boot context */
    Se05xSession_t *pS05x_Ctx;
    /** Status word of last R-APDU. */
    uint32_t status_word;
    /** Flag for get recovery script */
    bool recovery_executed;

    /** Skip following commands
     *
     * Internal state variable.
     *
     * When one APDU in this APDU stream has failed,
     * skip through all the next APDUs (do not send them)
     * and return back to the caller.
     */
    bool skip_next_commands;
    /** Flag for session status: Open/Close */
    bool session_opened;

#ifdef SEMS_LITE_AGENT_CHANNEL_1
    /** FIXME: TODO
     *
     * The currnt used logical channel.
     *
     * We use Channel 0 for all normal communication (To the IoT Applet)
     * And we use Channel 1 for communcation to the SEMS Lite Applet.
     *
     * This variable helps to know on which channel communication is
     * happening, and it should continue there.  Selecting the SEMS Lite applet
     * at two channels is not possible.
     *
     * In case we have not switched to channel 1, for actual upload of the
     * package, we can still use chanel 0 to talk to SEMS Lite Applet and
     * also to the IoT Applet.
     *
     */
    uint8_t n_logical_channel;
#endif
    uint32_t freePHeapCentralGap;
    uint32_t freeTransient; // COD Info from JCOP
} sems_lite_agent_ctx_t;

/**
 *@}
 */ /* end of sems_lite_agent_types */

/* *****************************************************************************************************************
 * Types/Structure Declarations
 * ***************************************************************************************************************** */

#endif // !SEMS_LITE_AGENT_CONTEXT_INC
