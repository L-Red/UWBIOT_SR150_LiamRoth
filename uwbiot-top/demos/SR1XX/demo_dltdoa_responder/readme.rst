..
    Copyright 2021-2022 NXP

    NXP Confidential. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms. By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms. If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. _demo-Dltdoa-responder:

=======================================================================
 SR1XX DLTDOA Ranging Responder
=======================================================================

.. brief:start

This demo showcases DLTDOA ranging with two SR1XX configured as a Anchor( Initiator and responder),
and third SR1XX configured as a Mobile node(Receiver,Controlle) [Another demo].

.. brief:end

Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- Set the application ranging parameters
- Perform DLTDOA raning.


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

- Source:   ``demo_dltdoa_responder``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_dltdoa_responder.bin`` file.
- On linux platform run the built executable.
- Run the other demos :ref:`demo-Dltdoa-Tag` and :ref:`demo-Dltdoa-initiator` for DLTDOA ranging.


Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :62000048 47170000 01000000 00C80000 00010000 00000000 00000000 .. 7FD080D0
    TMLUWB  :RX < :RECV                :62000048 48170000 01000000 00C80000 00010000 00000000 00000000 .. 8ED080D0
    TMLUWB  :RX < :RECV                :62000048 49170000 01000000 00C80000 00010000 00000000 00000000 .. 8CD080D0
    TMLUWB  :RX < :RECV                :62000048 4A170000 01000000 00C80000 00010000 00000000 00000000 .. 9CD080D0
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0300
    TMLUWB  :TX > :SEND                :21010004 01000000
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_dltdoa_responder/demo_dltdoa_responder.c : Success!

If such a log is not seen, re-run the program.
