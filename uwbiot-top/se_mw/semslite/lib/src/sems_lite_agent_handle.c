/*
 *
 * Copyright 2019,2020 NXP
 * SPDX-License-Identifier: Apache-2.0
 */
#include <ex_sss_boot.h>
#include <nxLog_App.h>

#include "sems_lite_agent.h"
#include "string.h" /* memset */
#if defined(SSS_HAVE_Host_PC) && (SSS_HAVE_Host_PC == 0)
#include "se05x_apis.h"
#endif
#include "sems_lite_agent_handle.h"
#include "nxEnsure.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

//#include "sems_lite_agent.pb.h"

#if SSS_HAVE_APPLET_SE05X_IOT

/* ************************************************************************** */
/* Global Variables                                                           */
/* ************************************************************************** */
extern void se05x_ic_reset();

sems_lite_status_t sems_lite_agent_reset_se(sems_lite_agent_ctx_t *sems_lite_agent_context)
{
    ENSURE_OR_RETURN_ON_ERROR((sems_lite_agent_context != NULL), kStatus_SEMS_Lite_ERR_General);

#if AX_EMBEDDED
    se05x_ic_reset();
#endif

    return kStatus_SEMS_Lite_Success;
}

#endif /* SSS_HAVE_APPLET_SE05X_IOT */
