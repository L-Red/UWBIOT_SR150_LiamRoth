..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-otp-storage-factory:

=======================================================================
 SR1XX OTP Storage Factory Mode
=======================================================================

.. brief:start

This demo showcases OTP Write/Read operation for SR1XX in factory mode.

.. brief:end

Internally UWB is initiailized in Factory mode, and calibration data write and read is done in OTP. Following sequence
of steps are handled.

- Initialize UWBD in Factory Firmware
- Write calib param in OTP (be careful while writing the data. Data written to OTP can not be reset)
- Read calib data from OTP


Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- UWBS programmed with OTP implemented factory mode firmware


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_otp_storage_factory.bin`` file.
- On linux platform run the built executable.

- Source:   ``demo_otp_storage_factory``

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::
    This demo will success only on new device.

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20040004 01010101
    TMLUWB  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20040006 01E40402 F401
    TMLUWB  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20020000
    TMLUWB  :RX < :RECV                :4002003D 00011039 ... 00
    TMLUWB  :TX > :SEND                :2A010003 050100
    TMLUWB  :RX < :RECV                :4A010001 00
    TMLUWB  :RX < :RECV                :6A010004 00020100
    APP     :INFO :Read Calib data VCO_PLL: 0x0001
    TMLUWB  :TX > :SEND                :2A000006 05010300 0100
    TMLUWB  :RX < :RECV                :4A000001 00
    TMLUWB  :RX < :RECV                :6A000001 00
    TMLUWB  :TX > :SEND                :2A010003 050100
    TMLUWB  :RX < :RECV                :4A010001 00
    TMLUWB  :RX < :RECV                :6A010004 00020100
    APP     :INFO :Read after Write VCO_PLL: 0x0001
    APP     :INFO :Read and Write data are same
    APP     :INFO :
    Finished :<PROJECT_PATH>\uwbiot-top\demos\SR1XX\demo_otp_storage_factory\demo_otp_storage_factory.c : Success!

If such a log is not seen, re-run the program.
