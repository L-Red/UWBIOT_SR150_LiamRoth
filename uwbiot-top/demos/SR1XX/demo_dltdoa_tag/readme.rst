..
    Copyright 2021-2022 NXP

    NXP Confidential. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms. By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms. If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. _demo-Dltdoa-Tag:

=======================================================================
 SR1XX DLTDOA Ranging Tag
=======================================================================

.. brief:start

This demo showcases DLTDOA ranging with one SR1XX configured as a Tag node(Tag,Controlle),
second SR1XX configured as Slave Anchor( Initiator and responder, Controlee) [Another demo]
and third SR1XX configured as a  Master anchor ( Initiator and Responder,Controller)[Another demo].

.. brief:end

Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- Set the application ranging parameters
- Perform DLTDOA raning.


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

- Source:   ``demo_dltdoa_tag``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_dltdoa_tag.bin`` file.
- On linux platform run the built executable.
- Run the other demos :ref:`demo-Dltdoa-initiator` and :ref:`demo-Dltdoa-responder` for DLTDOA ranging.


Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :6200001B 4A170000 01000000 00C80000 00020000 00000000 00000000 000000
    TMLUWB  :RX < :RECV                :6200001B 4B170000 01000000 00C80000 00020000 00000000 00000000 000000
    TMLUWB  :RX < :RECV                :6200001B 4C170000 01000000 00C80000 00020000 00000000 00000000 000000
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0300
    TMLUWB  :TX > :SEND                :21010004 01000000
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_dltdoa_tag/demo_dltdoa_tag.c : Success!

If such a log is not seen, re-run the program.
