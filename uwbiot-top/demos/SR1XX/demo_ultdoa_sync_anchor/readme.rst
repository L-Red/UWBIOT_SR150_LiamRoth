..
    Copyright 2022,2023 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-UL-Tdoa-Sync-Anchor:

=======================================================================
 TDOA Sync Anchor UCI 2.0
=======================================================================

.. brief:start

This demo showcases TDOA Special use case App ranging with device configured as a Controller - Initiator (UT-Synchronization Anchor)
and UWBS configured as a Controlee - Responder.

For details on Peer-to-Peer ranging sequence and configuration details
for UWBS, refer to :ref:`p2p-ranging`.

.. brief:end

How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

- Source:   ``demo_ultdoa_sync_anchor``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_ultdoa_sync_anchor.bin`` file.
- On linux platform run the built executable.
- Run the other demo :ref:`demo-UL-TDOA-Tag` for normal ranging.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :6200003F 92000000 00000000 00FFFFFF .. 012E2E
    TMLUWB  :RX < :RECV                :6200003F 93000000 00000000 00FFFFFF .. 012F2F
    TMLUWB  :RX < :RECV                :6200003F 94000000 00000000 00FFFFFF .. 012F2F
    TMLUWB  :TX > :SEND                :22010004 00000000
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 00000000 0300
    TMLUWB  :TX > :SEND                :21010004 00000000
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 00000000 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_ultdoa_sync_anchor/demo_ultdoa_sync_anchor.c : Success!

If such a log is not seen, re-run the program. 