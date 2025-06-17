/*
 * Copyright 2019,2020 NXP
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
#include "phUwb_BuildConfig.h"
#ifndef APP_TEST_BINDING_SE_TEST_H_
#define APP_TEST_BINDING_SE_TEST_H_
#include "UwbApi.h"

// Factory mode is enabled
#if UWBFTR_FactoryMode
/**
 * \brief Factory test status
 */
typedef struct
{
    uint8_t testStatus;
    uint8_t shutDownStatus;
    uint8_t fwDlStatus;
    uint8_t seInitStatus;
    uint8_t connectivityTestStatus;
#if (UWBFTR_SE_SN110)
    SeConnectivityStatus_t eseTestConnnectivity;
    phSeDoBindStatus_t doBindStatus;
    phSeGetBindingCount_t getBindingCount;
#endif //(UWBFTR_SE_SN110)
} factoryFwTestStatus_t;
#endif // UWBFTR_FactoryMode
/**
 * \brief Mainline test status
 */
typedef struct
{
    uint8_t testStatus;
    uint8_t shutDownStatus;
    uint8_t fwDlStatus;
    uint8_t seInitStatus;
    phTestLoopData_t testLoopData;
#if (UWBFTR_SE_SN110)
    phSeGetBindingStatus_t getBindingStatus;
#endif //(UWBFTR_SE_SN110)
} mainLineFwTestStatus_t;

mainLineFwTestStatus_t doMainlineFwTest();
// Factory mode is enabled
#if UWBFTR_FactoryMode
factoryFwTestStatus_t doFactoryFwTest();
#endif // UWBFTR_FactoryMode
#endif /* APP_TEST_BINDING_SE_TEST_H_ */
