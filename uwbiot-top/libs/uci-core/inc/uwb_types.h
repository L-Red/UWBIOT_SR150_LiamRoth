/******************************************************************************
 *
 *  Copyright (C) 1999-2014 Broadcom Corporation
 *  Copyright 2018-2020,2022 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#ifndef UWB_TYPES_H
#define UWB_TYPES_H

/****************************************************************************
** UWB_HDR header definition for UWB messages
*****************************************************************************/
typedef struct
{
    uint16_t event;
    uint16_t len;
    uint16_t offset;
    uint16_t layer_specific;
} UWB_HDR;
#define UWB_HDR_SIZE (sizeof(UWB_HDR))

/* Mask for UWB_HDR event field */
#define UWB_EVT_MASK     0xFF00
#define UWB_SUB_EVT_MASK 0x00FF

/**
 *  \brief UWB operating mode.
 */
typedef enum Uwb_operation_mode
{
    /** Default UWB operation mode */
    kOPERATION_MODE_default = 0,
    /** cdc operation mode */
    kOPERATION_MODE_cdc = 1,
    /** mctt operation mode */
    kOPERATION_MODE_mctt = 2,
} Uwb_operation_mode_t;

/*****************************************************************************
** Macros to get and put bytes to and from a stream (Little Endian format).
*****************************************************************************/
#define UWB_UINT64_TO_STREAM(p, u64)     \
    {                                    \
        *(p)++ = (uint8_t)(u64);         \
        *(p)++ = (uint8_t)((u64) >> 8);  \
        *(p)++ = (uint8_t)((u64) >> 16); \
        *(p)++ = (uint8_t)((u64) >> 24); \
        *(p)++ = (uint8_t)((u64) >> 32); \
        *(p)++ = (uint8_t)((u64) >> 40); \
        *(p)++ = (uint8_t)((u64) >> 48); \
        *(p)++ = (uint8_t)((u64) >> 56); \
    }
#define UWB_UINT32_TO_STREAM(p, u32)     \
    {                                    \
        *(p)++ = (uint8_t)(u32);         \
        *(p)++ = (uint8_t)((u32) >> 8);  \
        *(p)++ = (uint8_t)((u32) >> 16); \
        *(p)++ = (uint8_t)((u32) >> 24); \
    }
#define UWB_UINT24_TO_STREAM(p, u24)     \
    {                                    \
        *(p)++ = (uint8_t)(u24);         \
        *(p)++ = (uint8_t)((u24) >> 8);  \
        *(p)++ = (uint8_t)((u24) >> 16); \
    }
#define UWB_UINT16_TO_STREAM(p, u16)    \
    {                                   \
        *(p)++ = (uint8_t)(u16);        \
        *(p)++ = (uint8_t)((u16) >> 8); \
    }
#define UWB_INT16_TO_STREAM(p, i16)    \
    {                                  \
        *(p)++ = (int8_t)(i16);        \
        *(p)++ = (int8_t)((i16) >> 8); \
    }
#define UWB_UINT8_TO_STREAM(p, u8) \
    {                              \
        *(p)++ = (uint8_t)(u8);    \
    }
#define UWB_INT8_TO_STREAM(p, u8) \
    {                             \
        *(p)++ = (int8_t)(u8);    \
    }
#define UWB_ARRAY32_TO_STREAM(p, a)        \
    {                                      \
        register int ijk;                  \
        for (ijk = 0; ijk < 32; ijk++)     \
            *(p)++ = (uint8_t)a[31 - ijk]; \
    }
#define UWB_ARRAY16_TO_STREAM(p, a)        \
    {                                      \
        register int ijk;                  \
        for (ijk = 0; ijk < 16; ijk++)     \
            *(p)++ = (uint8_t)a[15 - ijk]; \
    }
#define UWB_ARRAY8_TO_STREAM(p, a)        \
    {                                     \
        register int ijk;                 \
        for (ijk = 0; ijk < 8; ijk++)     \
            *(p)++ = (uint8_t)a[7 - ijk]; \
    }
#define UWB_BDADDR_TO_STREAM(p, a)                      \
    {                                                   \
        register int ijk;                               \
        for (ijk = 0; ijk < BD_ADDR_LEN; ijk++)         \
            *(p)++ = (uint8_t)a[BD_ADDR_LEN - 1 - ijk]; \
    }
#define UWB_LAP_TO_STREAM(p, a)                     \
    {                                               \
        register int ijk;                           \
        for (ijk = 0; ijk < LAP_LEN; ijk++)         \
            *(p)++ = (uint8_t)a[LAP_LEN - 1 - ijk]; \
    }
#define UWB_DEVCLASS_TO_STREAM(p, a)                      \
    {                                                     \
        register int ijk;                                 \
        for (ijk = 0; ijk < DEV_CLASS_LEN; ijk++)         \
            *(p)++ = (uint8_t)a[DEV_CLASS_LEN - 1 - ijk]; \
    }
#define UWB_ARRAY_TO_STREAM(p, a, len)       \
    {                                        \
        register int ijk;                    \
        for (ijk = 0; ijk < (int)len; ijk++) \
            *(p)++ = (uint8_t)a[ijk];        \
    }
#define UWB_REVERSE_ARRAY_TO_STREAM(p, a, len)  \
    {                                           \
        register int ijk;                       \
        for (ijk = 0; ijk < len; ijk++)         \
            *(p)++ = (uint8_t)a[len - 1 - ijk]; \
    }
#define UWB_STREAM_TO_UINT8(u8, p) \
    {                              \
        u8 = (uint8_t)(*(p));      \
        (p) += 1;                  \
    }

#define UWB_STREAM_TO_INT8(u8, p) \
    {                             \
        u8 = (int8_t)(*(p));      \
        (p) += 1;                 \
    }
#define UWB_STREAM_TO_UINT16(u16, p)                                            \
    {                                                                           \
        u16 = (uint16_t)(((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8))); \
        (p) += 2;                                                               \
    }
#define UWB_STREAM_TO_INT16(i16, p)                                          \
    {                                                                        \
        i16 = (int16_t)((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); \
        (p) += 2;                                                            \
    }
#define UWB_STREAM_TO_UINT24(u32, p)                                                                         \
    {                                                                                                        \
        u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16)); \
        (p) += 3;                                                                                            \
    }
#define UWB_STREAM_TO_UINT32(u32, p)                                                                         \
    {                                                                                                        \
        u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16) + \
               ((((uint32_t)(*((p) + 3)))) << 24));                                                          \
        (p) += 4;                                                                                            \
    }
#define UWB_STREAM_TO_UINT64(u64, p)                                                                         \
    {                                                                                                        \
        u64 = (((uint64_t)(*(p))) + ((((uint64_t)(*((p) + 1)))) << 8) + ((((uint64_t)(*((p) + 2)))) << 16) + \
               ((((uint64_t)(*((p) + 3)))) << 24) + ((((uint64_t)(*((p) + 4)))) << 32) +                     \
               ((((uint64_t)(*((p) + 5)))) << 40) + ((((uint64_t)(*((p) + 6)))) << 48) +                     \
               ((((uint64_t)(*((p) + 7)))) << 56));                                                          \
        (p) += 8;                                                                                            \
    }
#define UWB_STREAM_TO_BDADDR(a, p)                               \
    {                                                            \
        register int ijk;                                        \
        register uint8_t *pbda = (uint8_t *)a + BD_ADDR_LEN - 1; \
        for (ijk = 0; ijk < BD_ADDR_LEN; ijk++)                  \
            *pbda-- = *p++;                                      \
    }
#define UWB_STREAM_TO_ARRAY32(a, p)                \
    {                                              \
        register int ijk;                          \
        register uint8_t *_pa = (uint8_t *)a + 31; \
        for (ijk = 0; ijk < 32; ijk++)             \
            *_pa-- = *p++;                         \
    }
#define UWB_STREAM_TO_ARRAY16(a, p)                \
    {                                              \
        register int ijk;                          \
        register uint8_t *_pa = (uint8_t *)a + 15; \
        for (ijk = 0; ijk < 16; ijk++)             \
            *_pa-- = *p++;                         \
    }
#define UWB_STREAM_TO_ARRAY8(a, p)                \
    {                                             \
        register int ijk;                         \
        register uint8_t *_pa = (uint8_t *)a + 7; \
        for (ijk = 0; ijk < 8; ijk++)             \
            *_pa-- = *p++;                        \
    }
#define UWB_STREAM_TO_DEVCLASS(a, p)                              \
    {                                                             \
        register int ijk;                                         \
        register uint8_t *_pa = (uint8_t *)a + DEV_CLASS_LEN - 1; \
        for (ijk = 0; ijk < DEV_CLASS_LEN; ijk++)                 \
            *_pa-- = *p++;                                        \
    }
#define UWB_STREAM_TO_LAP(a, p)                              \
    {                                                        \
        register int ijk;                                    \
        register uint8_t *plap = (uint8_t *)a + LAP_LEN - 1; \
        for (ijk = 0; ijk < LAP_LEN; ijk++)                  \
            *plap-- = *p++;                                  \
    }
#define UWB_STREAM_TO_ARRAY(a, p, len)       \
    {                                        \
        register int ijk;                    \
        for (ijk = 0; ijk < (int)len; ijk++) \
            ((uint8_t *)a)[ijk] = *p++;      \
    }
#define UWB_REVERSE_STREAM_TO_ARRAY(a, p, len)          \
    {                                                   \
        register int ijk;                               \
        register uint8_t *_pa = (uint8_t *)a + len - 1; \
        for (ijk = 0; ijk < len; ijk++)                 \
            *_pa-- = *p++;                              \
    }

/*****************************************************************************
** Macros to get and put bytes to and from a field (Little Endian format).
** These are the same as to stream, except the pointer is not incremented.
*****************************************************************************/

#define UWB_UINT32_TO_FIELD(p, u32)                     \
    {                                                   \
        *(uint8_t *)(p)       = (uint8_t)(u32);         \
        *((uint8_t *)(p) + 1) = (uint8_t)((u32) >> 8);  \
        *((uint8_t *)(p) + 2) = (uint8_t)((u32) >> 16); \
        *((uint8_t *)(p) + 3) = (uint8_t)((u32) >> 24); \
    }
#define UWB_UINT24_TO_FIELD(p, u24)                     \
    {                                                   \
        *(uint8_t *)(p)       = (uint8_t)(u24);         \
        *((uint8_t *)(p) + 1) = (uint8_t)((u24) >> 8);  \
        *((uint8_t *)(p) + 2) = (uint8_t)((u24) >> 16); \
    }
#define UWB_UINT16_TO_FIELD(p, u16)                    \
    {                                                  \
        *(uint8_t *)(p)       = (uint8_t)(u16);        \
        *((uint8_t *)(p) + 1) = (uint8_t)((u16) >> 8); \
    }
#define UWB_UINT8_TO_FIELD(p, u8)        \
    {                                    \
        *(uint8_t *)(p) = (uint8_t)(u8); \
    }

/*****************************************************************************
** Macros to get and put bytes to and from a stream (Big Endian format)
*****************************************************************************/

#define UWB_UINT32_TO_BE_STREAM(p, u32)  \
    {                                    \
        *(p)++ = (uint8_t)((u32) >> 24); \
        *(p)++ = (uint8_t)((u32) >> 16); \
        *(p)++ = (uint8_t)((u32) >> 8);  \
        *(p)++ = (uint8_t)(u32);         \
    }
#define UWB_UINT24_TO_BE_STREAM(p, u24)  \
    {                                    \
        *(p)++ = (uint8_t)((u24) >> 16); \
        *(p)++ = (uint8_t)((u24) >> 8);  \
        *(p)++ = (uint8_t)(u24);         \
    }
#define UWB_UINT16_TO_BE_STREAM(p, u16) \
    {                                   \
        *(p)++ = (uint8_t)((u16) >> 8); \
        *(p)++ = (uint8_t)(u16);        \
    }
#define UWB_UINT8_TO_BE_STREAM(p, u8) \
    {                                 \
        *(p)++ = (uint8_t)(u8);       \
    }
#define UWB_ARRAY_TO_BE_STREAM(p, a, len)    \
    {                                        \
        register int ijk;                    \
        for (ijk = 0; ijk < (int)len; ijk++) \
            *(p)++ = (uint8_t)a[ijk];        \
    }
#define UWB_BE_STREAM_TO_UINT8(u8, p) \
    {                                 \
        u8 = (uint8_t)(*(p));         \
        (p) += 1;                     \
    }
#define UWB_BE_STREAM_TO_UINT16(u16, p)                                     \
    {                                                                       \
        u16 = (uint16_t)(((uint16_t)(*(p)) << 8) + (uint16_t)(*((p) + 1))); \
        (p) += 2;                                                           \
    }
#define UWB_BE_STREAM_TO_UINT24(u32, p)                                                              \
    {                                                                                                \
        u32 = (((uint32_t)(*((p) + 2))) + ((uint32_t)(*((p) + 1)) << 8) + ((uint32_t)(*(p)) << 16)); \
        (p) += 3;                                                                                    \
    }
#define UWB_BE_STREAM_TO_UINT32(u32, p)                                                                  \
    {                                                                                                    \
        u32 = ((uint32_t)(*((p) + 3)) + ((uint32_t)(*((p) + 2)) << 8) + ((uint32_t)(*((p) + 1)) << 16) + \
               ((uint32_t)(*(p)) << 24));                                                                \
        (p) += 4;                                                                                        \
    }
#define UWB_BE_STREAM_TO_ARRAY(p, a, len) \
    {                                     \
        register int ijk;                 \
        for (ijk = 0; ijk < len; ijk++)   \
            ((uint8_t *)a)[ijk] = *p++;   \
    }

/*****************************************************************************
** Macros to get and put bytes to and from a field (Big Endian format).
** These are the same as to stream, except the pointer is not incremented.
*****************************************************************************/

#define UWB_UINT32_TO_BE_FIELD(p, u32)                  \
    {                                                   \
        *(uint8_t *)(p)       = (uint8_t)((u32) >> 24); \
        *((uint8_t *)(p) + 1) = (uint8_t)((u32) >> 16); \
        *((uint8_t *)(p) + 2) = (uint8_t)((u32) >> 8);  \
        *((uint8_t *)(p) + 3) = (uint8_t)(u32);         \
    }
#define UWB_UINT24_TO_BE_FIELD(p, u24)                  \
    {                                                   \
        *(uint8_t *)(p)       = (uint8_t)((u24) >> 16); \
        *((uint8_t *)(p) + 1) = (uint8_t)((u24) >> 8);  \
        *((uint8_t *)(p) + 2) = (uint8_t)(u24);         \
    }
#define UWB_UINT16_TO_BE_FIELD(p, u16)                 \
    {                                                  \
        *(uint8_t *)(p)       = (uint8_t)((u16) >> 8); \
        *((uint8_t *)(p) + 1) = (uint8_t)(u16);        \
    }
#define UWB_UINT8_TO_BE_FIELD(p, u8)     \
    {                                    \
        *(uint8_t *)(p) = (uint8_t)(u8); \
    }
#define UWB_STREAM_TO_BE_ARRAY(p, a, len)              \
    {                                                  \
        register int ijk;                              \
        for (ijk = 0; ijk < len; ijk++)                \
            ((uint8_t *)a)[ijk] = ((uint8_t *)p)[ijk]; \
    }

/*****************************************************************************
** Define trace levels
*****************************************************************************/

/* No trace messages to be generated    */
#define BT_TRACE_LEVEL_NONE 0
/* Error condition trace messages       */
#define BT_TRACE_LEVEL_ERROR 1
/* Warning condition trace messages     */
#define BT_TRACE_LEVEL_WARNING 2
/* API traces                           */
#define BT_TRACE_LEVEL_API 3
/* Debug messages for events            */
#define BT_TRACE_LEVEL_EVENT 4
/* Full debug messages                  */
#define BT_TRACE_LEVEL_DEBUG 5

#define TRACE_CTRL_GENERAL 0x00000000
#define TRACE_LAYER_UCI    0x00280000
#define TRACE_LAYER_HAL    0x00310000
#define TRACE_LAYER_GKI    0x001a0000
#define TRACE_ORG_STACK    0x00000000
#define TRACE_ORG_GKI      0x00000400

#define TRACE_TYPE_ERROR   0x00000000
#define TRACE_TYPE_WARNING 0x00000001
#define TRACE_TYPE_API     0x00000002
#define TRACE_TYPE_EVENT   0x00000003
#define TRACE_TYPE_DEBUG   0x00000004

#define TRACE_TYPE_GENERIC 0x00000008

#endif /* UWB_TYPES_H */
