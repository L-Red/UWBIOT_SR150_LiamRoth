/*
 * Copyright 2012-2020,2022 NXP.
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

#ifndef PHUWBTYPES_H
#define PHUWBTYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#ifdef __SES_ARM
/* does not have memory.h */
#else
#include <memory.h>
#endif

//#include <phOsalUwb.h>

#if UWBIOT_OS_NATIVE
typedef long BaseType_t;
#endif

typedef unsigned char BOOLEAN;

#define EXTERNC extern

/** API Parameters */
#ifdef __GNUC__
#define ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define ATTRIBUTE_UNUSED
#endif

#define ENABLED  1
#define DISABLED 0

#ifndef TRUE
#define TRUE (true) /* Logical True Value */
#endif

#ifndef FALSE
#define FALSE (false) /* Logical False Value */
#endif
typedef uint8_t utf8_t;     /* UTF8 Character String */
typedef uint8_t bool_t;     /* boolean data type */
typedef uint16_t UWBSTATUS; /* Return values */
#define UWB_STATIC static

#define PHUWB_UNUSED(X) (void)(X);
/*
 * Possible Hardware Configuration exposed to upper layer.
 * Typically this should be port name (Ex:"COM1","COM2") to which SR100 is
 * connected.
 */
typedef enum
{
    ENUM_LINK_TYPE_COM1,
    ENUM_LINK_TYPE_COM2,
    ENUM_LINK_TYPE_COM3,
    ENUM_LINK_TYPE_COM4,
    ENUM_LINK_TYPE_COM5,
    ENUM_LINK_TYPE_COM6,
    ENUM_LINK_TYPE_COM7,
    ENUM_LINK_TYPE_COM8,
    ENUM_LINK_TYPE_SPI,
    ENUM_LINK_TYPE_USB,
    ENUM_LINK_TYPE_TCP,
    ENUM_LINK_TYPE_NB
} phLibUwb_eConfigLinkType;

/*
 * Deferred message. This message type will be posted to the client application
 * thread
 * to notify that a deferred call must be invoked.
 */
#define PH_LIBUWB_DEFERREDCALL_MSG (0x311)

/*
 * Deferred call declaration.
 * This type of API is called from ClientApplication ( main thread) to notify
 * specific callback.
 */
typedef void (*pphLibUwb_DeferredCallback_t)(void *);

/*
 * Deferred parameter declaration.
 * This type of data is passed as parameter from ClientApplication (main thread)
 * to the
 * callback.
 */
typedef void *pphLibUwb_DeferredParameter_t;

/*
 * Possible Hardware Configuration exposed to upper layer.
 * Typically this should be at least the communication link (Ex:"COM1","COM2")
 * the controller is connected to.
 */
typedef struct phLibUwb_sConfig
{
    uint8_t *pLogFile; /* Log File Name*/
    /* Hardware communication link to the controller */
    phLibUwb_eConfigLinkType nLinkType;
    /* The client ID (thread ID or message queue ID) */
    uintptr_t nClientId;
} phLibUwb_sConfig_t, *pphLibUwb_sConfig_t;

/*
 * UWB Message structure contains message specific details like
 * message type, message specific data block details, etc.
 */
typedef struct phLibUwb_Message
{
    uint16_t eMsgType; /* Type of the message to be posted*/
    void *pMsgData;    /* Pointer to message specific data block in case any*/
    uint16_t Size;     /* Size of the datablock*/
} phLibUwb_Message_t, *pphLibUwb_Message_t;

/**
 * \brief  UWBD  Firmware Modes.
 */
typedef enum sdkMode
{
    /** Factory Firmware */
    FACTORY_FW,
    /** Mainline Firmware */
    MAINLINE_FW
} eFirmwareMode;
/**
 * \brief  Structure lists out the Firmware Image Context
 */
typedef struct phUwbFWImageContext
{
    /** pointer to the FW image to be used*/
    const uint8_t *fwImage;
    /** size of fw image */
    uint32_t fwImgSize;
    /** fw type */
    eFirmwareMode fwMode;
} phUwbFWImageContext_t;

/*
 * Deferred message specific info declaration.
 * This type of information is packed as message data when
 * PH_LIBUWB_DEFERREDCALL_MSG
 * type message is posted to message handler thread.
 */
typedef struct phLibUwb_DeferredCall
{
    pphLibUwb_DeferredCallback_t pCallback;   /* pointer to Deferred callback */
    pphLibUwb_DeferredParameter_t pParameter; /* pointer to Deferred parameter */
} phLibUwb_DeferredCall_t;

/*
 * Definitions for supported protocol
 */

#ifdef __GNUC__
#define UWB_API_DEPCREATED __attribute__((deprecated))
#else
#define UWB_API_DEPCREATED
#endif

#endif /* PHUWBTYPES_H */
