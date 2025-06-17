..
    Copyright 2021,2023 NXP

    NXP Confidential. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms. By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms. If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. _demo-test-rx:

=======================================================================
 SR1XX Demo Test Rx
=======================================================================

.. brief:start

This demo showcases how to test SR1XX in Test mode.
This demo is tested with session ID ``0x00000000`` and used to check RF parameters.
Here data is received over RF (over the air).

.. brief:end

Following sequence of steps are handled.

- Intialize UWBD.
- Initialize the test session and set the RF test parameters.
- Set test configuration.
- Start the RF test and check the parameters of RF test.

This test is generally used to measure characteristics like signal power, bandwith, etc.

How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_test_rx.bin`` file.
- On linux platform run the built executable.
- Run the other demo :numref:`demos-test-tx` :ref:`demos-test-tx` for normal ranging.

.. note:: By default BPRF configuration is set. To enable HPRF configuration set ``APP_INTERNAL_USE_HPRF`` to 1.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
At the end of program execution, log message like this must be seen::

    TMLUWB  :TX > :SEND                :21000005 00000000 D0
    TMLUWB  :RX < :RECV                :41000005 00010000 D0
    TMLUWB  :RX < :RECV                :61020006 010000D0 0000
    TMLUWB  :TX > :SEND                :2D000032 010000D0 09000428 00000001 04A00F00 000204C2 01000003 04EE0200 00040100 05010006 04000000 00070400 00000008 0100
    TMLUWB  :RX < :RECV                :4D000002 0000
    TMLUWB  :TX > :SEND                :21030010 010000D0 03050101 06021111 07022222
    TMLUWB  :RX < :RECV                :41030002 0000
    TMLUWB  :RX < :RECV                :61020006 010000D0 0300
    TMLUWB  :TX > :SEND                :2103001A 010000D0 07040109 15010214 010A1201 00160100 1701011F 0100
    TMLUWB  :RX < :RECV                :41030002 0000
    TMLUWB  :TX > :SEND                :21030008 010000D0 01290100
    TMLUWB  :RX < :RECV                :41030002 0000
    TMLUWB  :TX > :SEND                :2D000009 010000D0 01E50301 01
    TMLUWB  :RX < :RECV                :4D000002 0000
    TMLUWB  :TX > :SEND                :2D030004 00000000
    TMLUWB  :RX < :RECV                :4D030001 00
    TMLUWB  :RX < :RECV                :60010001 02
    TMLUWB  :RX < :RECV                :61020006 010000D0 0200
    TMLUWB  :RX < :RECV                :6D030041 00280000 002C0000 00040000 00000000 00280000 00000000 00280000 00000000 00280000 00000000 00280000 00000000 00280000 000A0000 00010178 C8362812 00
    TMLUWB  :RX < :RECV                :61020006 010000D0 0300
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :TX > :SEND                :21010004 010000D0
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :21010004 010000D0
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :21010004 010000D0
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :21010004 010000D0
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :21010004 010000D0
    UWBAPI  :ERROR:sendUciCommandAndWait : event timedout
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_test_rx/demo_test_rx.c : Success!

If such a log is not seen, re-run the program.