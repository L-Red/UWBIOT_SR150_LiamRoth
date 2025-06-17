..
    Copyright 2023 NXP

    NXP Confidential. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms.  By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. _demo-hybrid-ranging-controlee:

=======================================================================
 Demo Hybrid Scheduled Ranging Controlee
=======================================================================

.. brief:start

This demo showcases hybrid scheduled ranging with one device configured as a Controlee - Responder
and another device configured as a Controller - Initiator [Another demo].

.. brief:end

Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- Set the application ranging parameters
- Perform normal ranging with static STS.


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Set ``UWBFTR_DataTransfer`` macro to 1

- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`

- Source:   ``demo_hybrid_ranging_controlee``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the demo_hybrid_ranging_controlee.bin file.
- Run the other demo :numref:`demo-hybrid-ranging-controller` :ref:`demo-hybrid-ranging-controller` for hybrid scheduled ranging.


Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :62000048 00000000 02000003 00640000 00010000 .. 93D080D0
    TMLUWB  :TX > :SEND                :22010004 02000003
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 02000003 0300
    TMLUWB  :TX > :SEND                :22010004 03000003
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 03000003 0300
    TMLUWB  :TX > :SEND                :22010004 04000005
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 04000005 0300
    TMLUWB  :TX > :SEND                :22010004 01000001
    TMLUWB  :RX < :RECV                :60070001 0A
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :22010004 01000001
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 01000001 0300
    TMLUWB  :TX > :SEND                :21010004 02000003
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 02000003 0100
    TMLUWB  :TX > :SEND                :21010004 03000003
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 03000003 0100
    TMLUWB  :TX > :SEND                :21010004 04000005
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 04000005 0100
    TMLUWB  :TX > :SEND                :21010004 01000001
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000001 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_hybrid_ranging_controlee/demo_hybrid_ranging_controlee.c : Success!

If such a log is not seen, re-run the program.
