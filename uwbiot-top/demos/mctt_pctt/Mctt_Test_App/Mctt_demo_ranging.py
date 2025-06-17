# Copyright 2020 NXP
#
# NXP Confidential. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

import Mctt_Core as pnp

#########################################################################################################################################################################
#################################################################### Common Configurations ##############################################################################
#########################################################################################################################################################################

SESSION_ID = [0x44, 0x33, 0x22, 0x11]

UWB_Command_Reset = [0x20, 0x00, 0x00, 0x01, 0x00]

UWB_CoreGetConfig = [0x20, 0x02, 0x00, 0x00]

UWB_SessionInit_Ranging = [0x21, 0x00, 0x00, 0x05] + SESSION_ID + [0x00]

UWB_SetAppConfig_RFRAME_CONFIG = [0x21, 0x03, 0x00, 0x08] + SESSION_ID + [
                                  0x01,             # Number of configs
                                  0x12, 0x01, 0x03  # STS follows SFD, PPDU has no PHR or PSDU
                                  ]

UWB_SetAppConfig_SLOTS_PER_RR = [0x21, 0x03, 0x00, 0x08
                                 ] + SESSION_ID + [
                                 0x01,              # Number of configs
                                 0x1B, 0x01, 0x19   # 25 slots
                                 ]

UWB_SetAppConfig_STS_CONFIG = [0x21, 0x03, 0x00, 0x08 ] + SESSION_ID + [
                               0x01,              # Number of configs
                               0x02, 0x01, 0x01   # STS_CONFIG => Dynamic STS
                              ]

UWB_SetAppConfig_RANGING_DURATION = [0x21, 0x03, 0x00, 0x0B] + SESSION_ID + [
                                     0x01,                                  # Number of configs
                                     0x09, 0x04, 0xC8, 0x00, 0x00, 0x00     # 200 ms
                                     ]

UWB_SetAppConfig_SFD_ID = [0x21, 0x03, 0x00, 0x08] + SESSION_ID + [
                           0x01,                # Number of configs
                           0x15, 0x01, 0x02     # BPRF
                           ]

UWB_SetAppConfig_DST_MAC_ADDR = [0x21, 0x03, 0x00, 0x09] + SESSION_ID + [
                                 0x01,                      # Number of configs
                                 0x07, 0x02, 0x22, 0x22     # DST_MAC_ADDRESS    => 0x2222
                                 ]

UWB_SetAppConfig_NO_OF_CONTROLEES = [0x21, 0x03, 0x00, 0x08] + SESSION_ID + [
                                     0x01,                  # Number of configs
                                     0x05, 0x01, 0x01       # NUMBER_OF_ANCHORS  => 1
                                    ]

UWB_SetAppConfig_Mandatory = [0x21, 0x03, 0x00, 0x18] + SESSION_ID + [
                              0x06,                       # Number of configs
                              0x11, 0x01, 0x01,           # DEVICE_ROLE           => Initiator
                              0x03, 0x01, 0x00,           # MULTI_NODE_MODE       => Unicast
                              0x06, 0x02, 0x11, 0x11,     # DEVICE_MAC_ADDRESS    => 0x1111
                              0x00, 0x01, 0x01,           # DEVICE_TYPE           => Controller
                              0x22, 0x01, 0x01,           # SCHEDULED_MODE        => Time Scheduled
                              0x01, 0x01, 0x02            # RANGING_ROUND_USAGE   => DSTWR
                              ]

UWB_MCTT_SetApp_Configs = [0x21 ,0x03 ,0x00 ,0x7c ] + SESSION_ID + [
                           0x22 ,                                               # Number of configs
                           0x11 ,0x01 ,0x01 ,                                   # DEVICE_ROLE
                           0x00 ,0x01 ,0x01 ,                                   # DEVICE_TYPE
                           0x32 ,0x02 ,0x14 ,0x00 ,                             # MAX_NUMBER_OF_MEASUREMENTS
                           0x01 ,0x01 ,0x02 ,                                   # RANGING_ROUND_USAGE
                           0x03 ,0x01 ,0x00 ,                                   # MULTI_NODE_MODE
                           0x22 ,0x01 ,0x01 ,                                   # SCHEDULED_MODE
                           0x0c ,0x01 ,0x02 ,                                   # RANGING_ROUND_CONTROL
                           0x12 ,0x01 ,0x03 ,                                   # RFRAME_CONFIG
                           0x2e ,0x01 ,0x00 ,                                   # RESULT_REPORT_CONFIG
                           0x02 ,0x01 ,0x01 ,                                   # STS_CONFIG
                           0x2c ,0x01 ,0x00 ,                                   # RANGING_ROUND_HOPPING
                           0x2d ,0x01 ,0x00 ,                                   # BLOCK_STRIDE_LEN
                           0x09 ,0x04 ,0xc8 ,0x00 ,0x00 ,0x00 ,                 # RANGING_DURATION
                           0x1b ,0x01 ,0x19 ,                                   # SLOTS_PER_RR
                           0x08 ,0x02 ,0x60 ,0x09 ,                             # SLOT_DURATION
                           0x04 ,0x01 ,0x09 ,                                   # CHANNEL_ID
                           0x14 ,0x01 ,0x09 ,                                   # PREAMBLE_CODE_INDEX
                           0x1f ,0x01 ,0x00 ,                                   # PRF_MODE
                           0x17 ,0x01 ,0x01 ,                                   # PREAMBLE_DUR
                           0x15 ,0x01 ,0x02 ,                                   # SFD_ID
                           0x29 ,0x01 ,0x01 ,                                   # NUMBER_OF_STS_SEGMENTS
                           0x16 ,0x01 ,0x00 ,                                   # PSDU_DATA_RATE
                           0x35 ,0x01 ,0x01 ,                                   # STS_LENGTH
                           0x2a ,0x02 ,0x0a ,0x00 ,                             # MAX_RR_RETRY
                           0x2b ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,                 # UWB_INITIATION_TIME
                           0x23 ,0x01 ,0x01 ,                                   # KEY_ROTATION
                           0x24 ,0x01 ,0x00 ,                                   # KEY_ROTATION_RATE
                           0x0b ,0x01 ,0x00 ,                                   # MAC_TYPE
                           0x26 ,0x01 ,0x00 ,                                   # MAC_ADDRESS_MODE
                           0x06 ,0x02 ,0xa1 ,0xaa ,                             # SRC_MAC_ADDRESS
                           0x27 ,0x02 ,0xfe ,0x6d ,                             # VENDOR_ID
                           0x28 ,0x06 ,0xc4 ,0x25 ,0xcb ,0xe0 ,0x92 ,0xef ,     # STATIC_STS_IV
                           0x05 ,0x01 ,0x01 ,                                   # NUMBER_OF_CONTROLEES
                           0x07 ,0x02 ,0xa2 ,0xaa                               # DST_MAC_ADDRESS_LIST
                           ]

UWB_StartRanging = [0x22, 0x00, 0x00, 0x04] + SESSION_ID

# These is replica of MCTT
select = [0x80,0x00,0x0e,0x00,0x00,0xa4,0x04,0x00,0x09,0xa0,0x00,0x00,0x08,0x67,0x46,0x41,0x50,0x00]

adf = [0x80, 0x00, 0x18, 0x00,0x80,0xA5,0x04,0x00,0x0F,0x80,0x01,0x00,0x06,0x0A,0x60,0x86,0x48,0x01,0x86,0xFF,0x13,0x01,0x01,0x01,0x00,0x00,0x00,0x00]
put_data= [0x80, 0x00, 0x19, 0x00,0x00,0xdb,0x3f,0xff,0x14,0xbf,0x78,0x11,0x81,0x04,0xf7,0xc8,0xb4,0xa2,0xa5,0x06,0x80,0x04,0x31,0x22,0xd3,0xda,0x87,0x01,0x01]

#########################################################################################################################################################################

get_app_command = [0x21, 0x04, 0x00, 0x05] + SESSION_ID + [0x00]

SR1XX_Normal_Ranging = [
    UWB_CoreGetConfig,
    UWB_SessionInit_Ranging,
    get_app_command,
    UWB_SetAppConfig_RFRAME_CONFIG,
    UWB_SetAppConfig_SLOTS_PER_RR,
    UWB_SetAppConfig_RANGING_DURATION,
    UWB_SetAppConfig_SFD_ID,
    UWB_SetAppConfig_DST_MAC_ADDR,
    UWB_SetAppConfig_NO_OF_CONTROLEES,
    UWB_SetAppConfig_Mandatory,
    UWB_StartRanging,
]

SR1XX_Secure_Ranging = [
    UWB_CoreGetConfig,
    UWB_SessionInit_Ranging,
    select,
    adf,
    put_data,
    get_app_command,
    UWB_SetAppConfig_RFRAME_CONFIG,
    UWB_SetAppConfig_STS_CONFIG,
    UWB_SetAppConfig_SLOTS_PER_RR,
    UWB_SetAppConfig_RANGING_DURATION,
    UWB_SetAppConfig_SFD_ID,
    UWB_SetAppConfig_DST_MAC_ADDR,
    # UWB_MCTT_SetApp_Configs,
    UWB_SetAppConfig_NO_OF_CONTROLEES,
    UWB_SetAppConfig_Mandatory,
    UWB_StartRanging,
]

SR1xx_only_se = [
    select,
    adf,
    put_data,
]


def main():

    pnp.run(SR1XX_Normal_Ranging, SESSION_ID)
    # pnp.run(SR1XX_Secure_Ranging, SESSION_ID)
    # pnp.run(SR1xx_only_se, SESSION_ID)


if __name__ == '__main__':
    main()
