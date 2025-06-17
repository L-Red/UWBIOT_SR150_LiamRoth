/*
 * Copyright 2012-2020 NXP.
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

#ifndef _PHNXPUCIHAL_ADAPTATION_H_
#define _PHNXPUCIHAL_ADAPTATION_H_

typedef uint8_t uwb_event_t;
typedef uint8_t uwb_status_t;
#include "uwb_types.h"
#include "uwb_hal_api.h"
/*`
 * The callback passed in from the UWB stack that the HAL
 * can use to pass events back to the stack.
 */
typedef void(uwb_stack_callback_t)(uwb_event_t event, uwb_status_t event_status);

/*
 * The callback passed in from the UWB stack that the HAL
 * can use to pass incomming data to the stack.
 */
typedef void(uwb_stack_data_callback_t)(uint16_t data_len, uint8_t *p_data);

/*
 * The callback is passed in from the application
 * callback to be invoked on data packet reception
 */
typedef void(phHalAppDataCb)(uint8_t *recvBuf, uint16_t dataLen);

/* NXP HAL functions */
int phNxpUciHal_open(uwb_stack_callback_t *p_cback, uwb_stack_data_callback_t *p_data_cback);
int phNxpUciHal_write(uint16_t data_len, const uint8_t *p_data);
void phNxpUciHal_register_appdata_callback(phHalAppDataCb *appDataCb);
int phNxpUciHal_close();
int phNxpUciHal_ioctl(long arg, tHAL_UWB_IOCTL *p_data);
int phNxpUciHal_applyVendorConfig();
int phNxpUciHal_uwbDeviceInit(BOOLEAN recovery);
void phNxpUciHal_SetOperatingMode(Uwb_operation_mode_t mode);
#endif /* _PHNXPUCIHAL_ADAPTATION_H_ */
