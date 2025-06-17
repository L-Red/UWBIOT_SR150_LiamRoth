# Copyright 2019 NXP
#
# SPDX-License-Identifier: Apache-2.0
#
#
# Manually create project. mbedTLS has it's own CMakeLists.txt
#
PROJECT(mbedtls)

FILE(
    GLOB
    mbedtls_sources
    mbedtls.cmake
    mbedtls/library/*.c
    mbedtls/library/*.h
    mbedtls/include/mbedtls/*.h
)

IF(WithmbedTLS_ALT_A71CH)
    FILE(
        GLOB
        mbedtls_alt
        ${SIMW_TOP_DIR}/hostlib/hostLib/mbedtls/src/*.c
        ${SIMW_TOP_DIR}/hostlib/hostLib/mbedtls/src/*.h
        ${SIMW_TOP_DIR}/hostlib/hostLib/mbedtls/inc/*.h
    )
ENDIF()

IF(WithmbedTLS_ALT_SSS AND NOT UWBIOT_HOST_NRF52840_MK)
    FILE(
        GLOB
        mbedtls_alt
        ${SIMW_TOP_DIR}/sss/plugin/mbedtls/ecdh_alt_ax.c
        ${SIMW_TOP_DIR}/sss/plugin/mbedtls/ecp_alt.c
        ${SIMW_TOP_DIR}/sss/plugin/mbedtls/ecp_alt_sss.c
        ${SIMW_TOP_DIR}/sss/plugin/mbedtls/sss_mbedtls.c
        ${SIMW_TOP_DIR}/sss/plugin/mbedtls/sss_mbedtls_rsa.c
        ${SIMW_TOP_DIR}/sss/plugin/mbedtls/ecdsa_verify_alt.c
        ${SIMW_TOP_DIR}/hostlib/hostLib/mbedtls/src/*_alt.c
        mbedtls/port/ksdk/ecp_curves_alt.c
        mbedtls/port/ksdk/ecp_alt.c
    )
ENDIF()

IF(WithmbedTLS_ALT_A71CH)
    FILE(
        GLOB
        mbedtls_alt
        ${SIMW_TOP_DIR}/hostlib/hostLib/mbedtls/src/*_alt.c
        ${SIMW_TOP_DIR}/hostlib/hostLib/mbedtls/src/*_alt_ax.c
        mbedtls/port/ksdk/ecp_curves_alt.c
    )
ENDIF()

IF((WithKSDK OR SSS_HAVE_MBEDTLS_ALT_SSS OR SSS_HAVE_MBEDTLS_ALT_A71CH) AND NOT UWBIOT_HOST_NRF52840_MK)
    FILE(
        GLOB
        mbedtls_ksdk_sources
        mbedtls/port/ksdk/*.c
        mbedtls/port/ksdk/*.h
    )
ENDIF()

IF(WithHost_Android)
    ADD_LIBRARY(
        ${PROJECT_NAME} SHARED
        ${mbedtls_ksdk_sources}
        ${mbedtls_sources}
        ${mbedtls_alt}
    )
ELSE()
    ADD_LIBRARY(
        ${PROJECT_NAME}
        ${mbedtls_ksdk_sources}
        ${mbedtls_sources}
        ${mbedtls_alt}
    )
ENDIF()

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PUBLIC mbedtls/include
    PUBLIC mbedtls/library
)

IF(WithKSDK)
    IF(NOT UWBIOT_HOST_NRF52840_MK)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC mbedtls/port/ksdk)
        TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC MBEDTLS_CONFIG_FILE=\"ksdk_mbedtls_config.h\")
        TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC MBEDTLS_USER_CONFIG_FILE=\"sss_ksdk_mbedtls_config.h\")
    ELSE()
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${UWBIOT_BOARD_NAME})
        TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC MBEDTLS_USER_CONFIG_FILE=\"se_mbedtls_config.h\")
    ENDIF()

    IF(SSS_HAVE_MBEDTLS_ALT_A71CH)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${SIMW_TOP_DIR}/hostlib/hostLib/mbedtls/inc)
        TARGET_LINK_LIBRARIES(
            ${PROJECT_NAME}
            smCom
            SSS_APIs
        )
    ELSEIF(SSS_HAVE_MBEDTLS_ALT_SSS)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${SIMW_TOP_DIR}/sss/plugin/mbedtls)
        TARGET_LINK_LIBRARIES(
            ${PROJECT_NAME}
            smCom
            SSS_APIs
        )
    ENDIF()

    # UWB MW does not have board,
    # Only P&T MW has it.
    IF (NOT (UWBIOT_OS_FREERTOS OR UWBIOT_OS_NATIVE))
        TARGET_LINK_LIBRARIES(
            ${PROJECT_NAME}
            board
        )
    ENDIF()
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        ${UWBIOT_BOARD_NAME}
    )

    IF(NOT(UWBIOT_HOST_QN9090_MK_SHIELD_V4 OR UWBIOT_HOST_RHODES4 OR UWBIOT_HOST_CRETE OR UWBIOT_HOST_NRF52840_MK))
        TARGET_LINK_LIBRARIES(
            ${PROJECT_NAME}
            _mmcau
        )
    ENDIF()
ELSE() # KSDK
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC MBEDTLS_USER_CONFIG_FILE=\"sss_x86_mbedtls_config.h\")
    IF(WithmbedTLS_ALT_A71CH)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
            PUBLIC
            ${SIMW_TOP_DIR}/hostlib/hostLib/mbedtls/inc
            ${SIMW_TOP_DIR}/sss/inc
            ${SIMW_TOP_DIR}/hostlib/hostLib/libCommon/infra
            ${SIMW_TOP_DIR}/hostlib/hostLib/inc
            )
    ELSEIF(WithmbedTLS_ALT_SSS)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
            PUBLIC
            ${SIMW_TOP_DIR}/sss/plugin/mbedtls
            ${SIMW_TOP_DIR}/sss/inc
            ${SIMW_TOP_DIR}/hostlib/hostLib/libCommon/infra
            ${SIMW_TOP_DIR}/hostlib/hostLib/inc
            )
    ENDIF()
ENDIF()

IF(
    "${CMAKE_C_COMPILER}"
    MATCHES
    ".*clang"
    OR "${CMAKE_CXX_COMPILER_ID}"
       STREQUAL
       "AppleClang"
)
    # MESSAGE(STATUS "-- No warning for mbedtls")
    TARGET_COMPILE_OPTIONS(
        ${PROJECT_NAME}
        PRIVATE -Wno-unused-function
        PRIVATE -Wno-error=pointer-sign
        PRIVATE -Wno-error=format
        PRIVATE -Wno-format
        PRIVATE -Wno-error=unused-const-variable
        PRIVATE -Wno-unused-const-variable
        PRIVATE -Wno-unused-but-set-variable
        PRIVATE -Wno-sign-compare
    )
ENDIF()

IF(
    "${CMAKE_CXX_COMPILER_ID}"
    MATCHES
    "MSVC"
)
    IF(NXPInternal)
        TARGET_COMPILE_OPTIONS(
            ${PROJECT_NAME}
            PRIVATE /wd4245 # '=': conversion from 'int' to 'mbedtls_mpi_uint', signed/unsigned misma
            PRIVATE /wd4310 # cast truncates constant value
            PRIVATE /wd4389 # '==': signed/unsigned mismatch
            PRIVATE /wd4132 # const object should be initialized
            PUBLIC /wd4127 # conditional expression is constant
            PRIVATE /wd4701 # potentially uninitialized local variable
            PRIVATE /wd4477 # 'printf' : format string '%d'
            PRIVATE /wd4200 # zero-sized array in struct/union
        )
    ENDIF()
ENDIF()

IF(
    "${CMAKE_CXX_COMPILER_ID}"
    STREQUAL
    "GNU"
)
    TARGET_COMPILE_OPTIONS(
        ${PROJECT_NAME}
        PRIVATE -Wno-unused-function
        PRIVATE -Wno-error=pointer-sign
        PRIVATE -Wno-error=format
        PRIVATE -Wno-format
    )

    SET(GCC_VERSION_WITH_UNUSED_CONST 6.3.0)
    IF(
        GCC_VERSION_WITH_UNUSED_CONST
        VERSION_LESS
        CMAKE_CXX_COMPILER_VERSION
    )
        TARGET_COMPILE_OPTIONS(
            ${PROJECT_NAME}
            PRIVATE -Wno-implicit-function-declaration
            PRIVATE -Wno-error=unused-const-variable
            PRIVATE -Wno-unused-const-variable
            PRIVATE -Wno-stringop-overflow
        )
    ENDIF()
ENDIF()
