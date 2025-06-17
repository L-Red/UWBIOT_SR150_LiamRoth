..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-otp-storage-mainline:

=======================================================================
 SR1XX OTP Storage Mainline Mode
=======================================================================

.. brief:start

This demo showcases OTP Read from SR1XX in mainline mode.

.. brief:end

Internally UWB is initiailized in Mainline mode, and calibration data is read from OTP. Following sequence
of steps are handled.

- Initialize UWBD in Mainline Firmware
- Read calib param from OTP
- Set the calib data read from OTP
- Start Ranging



How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

- Source:   ``demo_otp_storage_mainline``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_otp_storage_mainline.bin`` file.
- On linux platform run the built executable.


Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :TX > :SEND                :2A010003 050100
    TMLUWB  :RX < :RECV                :4A010001 00
    TMLUWB  :RX < :RECV                :6A010004 0002B081
    TMLUWB  :TX > :SEND                :2A010003 050108
    TMLUWB  :RX < :RECV                :4A010001 00
    TMLUWB  :RX < :RECV                :6A010004 00020000
    TMLUWB  :TX > :SEND                :2F210005 050002B0 81
    TMLUWB  :RX < :RECV                :4F210001 00
    TMLUWB  :TX > :SEND                :2F210005 05610200 00
    TMLUWB  :RX < :RECV                :4F210001 00
    TMLUWB  :TX > :SEND                :2F220002 0500
    TMLUWB  :RX < :RECV                :4F220006 00000002 B081
    TMLUWB  :TX > :SEND                :2F220002 0561
    TMLUWB  :RX < :RECV                :4F220006 00006102 0000
    TMLUWB  :TX > :SEND                :2A040000
    TMLUWB  :RX < :RECV                :4A040003 000102
    APP     :INFO :Module Make ID is:
    APP     :INFO :readMMId            :0102
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_otp_storage_mainline/demo_otp_storage_mainline.c : Success!

If such a log is not seen, re-run the program.
