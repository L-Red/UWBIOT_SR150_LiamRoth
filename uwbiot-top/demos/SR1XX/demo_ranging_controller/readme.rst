..
    Copyright 2020, 2022 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-ranging-controller:

=======================================================================
 Demo Ranging Controller
=======================================================================

.. brief:start

This demo showcases normal ranging with one device configured as a Controller - Initiator
and another device configured as a Controlee - Responder [Another demo].

.. brief:end


Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- Set the application ranging parameters
- Perform normal ranging with static STS.

How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`
- For embed linux raspberry pi with crete setup :ref:`build-pi-crete`

- Source:   ``demo_ranging_controller``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the demo_ranging_controller.bin file.
- On linux platform run the built executable.
- Run the other demo :numref:`demo-ranging-controlee` :ref:`demo-ranging-controlee` for normal ranging.

TOF, 2D and 3D AoA Modes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This demo can also be used to be run for TOF single antenna, 2D Azimuth AoA, 2D Elevation AoA
and 3D AoA.

.. literalinclude:: ../../../boards/Host/Rhodes4/uwb_board.h
    :language: c
    :start-after: /* select-aoa-mode:start */
    :end-before:  /* select-aoa-mode:end */


Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :62000055 2E010000 01000000 .. 309BD080 D0
    TMLUWB  :RX < :RECV                :62000055 2F010000 01000000 .. 00000000 00
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :62000055 30010000 01000000 .. 00000000 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0300
    TMLUWB  :TX > :SEND                :21010004 01000000
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_ranging_controller/demo_ranging_controller.c : Success!

If such a log is not seen, re-run the program.
