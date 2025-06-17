/*
 * Copyright 2021-2022 NXP.
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

#ifndef __UWB_UWBS_TML_IO_SR1XX_H__
#define __UWB_UWBS_TML_IO_SR1XX_H__

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

/** @defgroup uwb_uwbs_io UWBS Specific IO Constants
 *
 * Each UWBS has differetn IO Requirements.
 *
 * In order to isolate the interfaces,
 * UWBS specific header file would allow to
 * isolate the usage of these constants.
 *
 * @{
 */

/** IO Pin High or low
 *
 */
typedef enum
{
    /** Pin is Low, logic low, zero volt */
    kUWBS_IO_State_Low = 0,

    /** Pin is High, logic high.
     * Depending on the voltage domain, 1.8v, 3.3v */
    kUWBS_IO_State_High = 1,
    /* Invalid state */
    kUWBS_IO_State_NA = 0x7F,
} uwbs_io_state_t;

/** IO PINs between Host and the UWBS
 *
 * See @ref uwb_bus_io
 */

typedef enum
{
    /** Helios Enable*/
    kUWBS_IO_O_ENABLE_HELIOS,
    /** UWBS Enable 1.8V*/
    kUWBS_IO_O_ENABLE_UWBS_1V8,
    /** Helios IRQ PIN */
    kUWBS_IO_I_UWBS_IRQ,
    /** UWBS ENABLE digital 1V8 */
    kUWBS_IO_O_ENABLE_DIG_1V8,
    /** UWBS DPD Enable */
    kUWBS_IO_O_ENABLE_UWBS_DPD,
    /** Helios Sync*/
    kUWBS_IO_O_HELIOS_SYNC,
    /** Helios RTC Sync*/
    kUWBS_IO_O_HELIOS_RTC_SYNC,
    /** Venus VEN PIN */
    kUWBS_IO_O_VENUS_VEN,
    /** Venus IRQ PIN */
    kUWBS_IO_I_VENUS_IRQ,
    /** Helios Chip Reset*/
    kUWBS_IO_O_RSTN,
} uwbs_io_t;

/** Call back function and arguemnt for helios irq
 *
 */
typedef struct
{
    void (*fn)(void *args);
    void *args;
} uwbs_io_callback;

/** @} */
#endif
