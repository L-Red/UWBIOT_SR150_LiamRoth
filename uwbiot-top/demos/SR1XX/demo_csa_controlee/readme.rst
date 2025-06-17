..
    Copyright 2022-2023 NXP

    NXP Confidential. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms.  By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. _demo-csa-controlee:

=======================================================================
 Demo CSA Controlee
=======================================================================

.. brief:start

This demo showcases CSA scenario with one device (SR150) configured as a Controlee - Responder
and another device (SR100) configured as a Controller - Initiator.

.. brief:end

Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- Create Session
- Configure Session
- Start Session

How to Set Wrapped Rds(optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This section explain how to form and call the wrapped Rds.
Uncomment below to enable this feature.

.. literalinclude:: demo_csa_controlee.c
    :language: c
    :start-after: /* doc:start:call wrapped rds Api */
    :end-before: /* doc:end:call wrapped rds Api */
    :dedent: 4

.. literalinclude:: demo_csa_controlee.c
    :language: c
    :start-after: /* doc:start:form wrapped rds */
    :end-before: /* doc:end:form wrapped rds */

How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`
- For embed linux raspberry pi with crete setup :ref:`build-rpi-crete`

- Source:   ``demo_csa_controlee``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the demo_ranging_controlee.bin file.
- On linux platform run the built executable.


Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :62200033 010000A0 00C34902 00000020 .. D2DFDF17 D39AE0
    TMLUWB  :RX < :RECV                :62200033 010000A0 00F34902 00000049 .. D361E0D5 D19AE0
    TMLUWB  :RX < :RECV                :62200033 010000A0 00234A02 0000001C .. D1A9E05D D2F1DF
    TMLUWB  :TX > :SEND                :22010004 010000A0
    TMLUWB  :RX < :RECV                :60070001 0A
    TMLUWB  :RX < :RECV                :62200033 010000A0 00534A02 00000051 .. D0DCDF4D D258E0
    TMLUWB  :RX < :RECV                :62200033 010000A0 F0534A02 .. D0DCDF4D D258E0
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :22010004 010000A0
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 010000A0 0300
    TMLUWB  :TX > :SEND                :21010004 010000A0
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 010000A0 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_csa_controlee/demo_csa_controlee.c : Success!

If such a log is not seen, re-run the program.