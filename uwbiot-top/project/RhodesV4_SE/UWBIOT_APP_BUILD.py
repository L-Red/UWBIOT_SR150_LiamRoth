# Copyright 2021-2023 NXP
#
# NXP Confidential. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#


import sys
import os

DEMOS="""
    DEMO_RANGING_CONTROLLER
    DEMO_RANGING_CONTROLEE
    DEMO_BINDING
    DEMO_INBAND_DATA_TRANSFER_RX
    DEMO_INBAND_DATA_TRANSFER_TX
    DEMO_OTP_STORAGE_FACTORY
    DEMO_OTP_STORAGE_MAINLINE
    DEMO_FL_INITIATOR
    DEMO_FL_RESPONDER
    DEMO_FL_RESPONDER_IOT_CONCURRENCY
    DEMO_PNP
    DEMO_MCTT_PCTT
    DEMO_NEARBY_INTERACTION
    DEMO_NEARBY_INTERACTION_CLIENT
    DEMO_DLTDOA_INITIATOR
    DEMO_DLTDOA_RESPONDER
    DEMO_DLTDOA_TAG
    SE_VCOM
    DEMO_OWR_AOA_ADVERTISER
    DEMO_ULTDOA_ANCHOR
    DEMO_ULTDOA_SYNC_ANCHOR
    DEMO_TEST_TX
    DEMO_TEST_RX
    DEMO_HYBRID_RANGING_CONTROLLER
    DEMO_HYBRID_RANGING_CONTROLEE
    DEMO_CSA_CONTROLEE
"""



def main():
    sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..', 'scripts'))
    import bld_cfg_gen
    bld_cfg_gen.generate(sys.argv, DEMOS)


if __name__ == '__main__':
    main()
