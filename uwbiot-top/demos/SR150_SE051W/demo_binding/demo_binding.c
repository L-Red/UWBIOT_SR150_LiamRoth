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

#include "phUwb_BuildConfig.h"
#include "AppRecovery.h"
#include "UwbApi.h"
#include <AppInternal.h>
#include "UwbApi_Proprietary_Fm.h"
#include "ex_sss_boot.h"
#ifndef UWBIOT_APP_BUILD__DEMO_BINDING
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_BINDING

/*
  * Below list contains the application configs which are only related to default
  * configuration.
  */

/********************************************************************************/
#define DEMO_BINDING_TASK_SIZE 1536
#define DEMO_BINDING_TASK_NAME "DemoBinding"
#define DEMO_BINDING_TASK_PRIO 4

AppContext_t appContext;
static ex_sss_boot_ctx_t ex_sss_boot_ctx;
#define PCONTEXT (&ex_sss_boot_ctx)

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Demo SE Binding");
    tUWBAPI_STATUS status   = UWBAPI_STATUS_FAILED;
    sss_status_t sss_status = kStatus_SSS_Fail;
    char *portName;
    phOsalUwb_SetMemory(PCONTEXT, 0, sizeof(*PCONTEXT));
    phSeGetBindingCount_t uwbd_BindingCtx = {0};
    se_bindState_t se_bindState           = {0};
    se_status_t se_status                 = SE_STATUS_NOT_OK;

#if AX_EMBEDDED
    portName = NULL;
#else
    sss_status = ex_sss_boot_connectstring(0, NULL, &portName);
    if (kStatus_SSS_Success != sss_status) {
        NXPLOG_APP_E("ex_sss_boot_connectstring Failed");
        goto exit;
    }

#endif // AX_EMBEDDED
    PCONTEXT->se05x_open_ctx.skip_select_applet = TRUE;
    sss_status                                  = ex_sss_boot_open(PCONTEXT, portName);
    if (kStatus_SSS_Success != sss_status) {
        NXPLOG_APP_E("ex_sss_boot_open Failed");
        goto exit;
    }
// Factory mode is enabled
#if UWBFTR_FactoryMode
    sss_se05x_session_t *sss_session = (sss_se05x_session_t *)&(PCONTEXT->session);
    pSe05xSession_t pSeSession       = &(sss_session->s_ctx);
    phUwbappContext_t appCtx         = {0};

#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedFactoryFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedFactoryFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = FACTORY_FW;
    appCtx.pCallback         = AppCallback;
    appCtx.pCdcCallback      = NULL;
    appCtx.pMcttCallback     = NULL;
    appCtx.seHandle          = (void *)pSeSession;
    status                   = UwbApi_Init_New(&appCtx);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Init_New Failed");
        goto exit;
    }
#endif // UWBFTR_FactoryMode
    status = UwbApi_GetBindingCount(&uwbd_BindingCtx);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetBindingCount Failed");
        goto exit;
    }
    se_status = Se_API_Init(SE_APPLET_SUS, SE_CHANNEL_1);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_APP_E("Applet Selection Failed");
        /* Close the channel and Deselect the applet */
        Se_API_DeInit();
        goto exit;
    }
    Se_API_GetBindingState(&se_bindState);
    if (se_status != SE_STATUS_OK) {
        NXPLOG_APP_E("Se_API_GetBindingState Failed");
        /* Close the channel and Deselect the applet */
        Se_API_DeInit();
        goto exit;
    }
    /* Close the channel and Deselect the applet */
    Se_API_DeInit();

    /* This check is added to avoid more than one binding attempt,
     * If you want to test more than one binding disable this check
     */
    if ((uwbd_BindingCtx.bindingStatus == UWBD_NOT_BOUND) ||
        ((se_bindState.boundStatus == SE_Not_Bound) && (uwbd_BindingCtx.uwbdBindingCount < MAX_UWBD_BINDING_CNT))) {
        status = UwbApi_PerformBinding();
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_PerformBinding Failed");
            goto exit;
        }
        NXPLOG_APP_I("Binding Done");
    }
    else {
        LOG_I("UWBD's remaining binding count:%d, binding status:%d",
            uwbd_BindingCtx.uwbdBindingCount,
            uwbd_BindingCtx.bindingStatus);

        LOG_I("Secure Element's remaining attempts:%d, binding status:%d",
            se_bindState.bindingAttempts,
            se_bindState.boundStatus);
    }

exit:
    ex_sss_session_close(PCONTEXT);

    if (UwbApi_ShutDown() != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ShutDown Failed");
    }
    if (status == UWBAPI_STATUS_TIMEOUT) {
        Handle_ErrorScenario(TIME_OUT);
    }

    UWBIOT_EXAMPLE_END(status);
}

/*
 * Interface which will be called from Main to create the required task with its own parameters.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;
    threadparams.stackdepth   = DEMO_BINDING_TASK_SIZE;
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_BINDING_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_BINDING_TASK_PRIO;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_BINDING
