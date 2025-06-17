..
    Copyright 2021 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-firalite-responder-iot-concurrency:

=======================================================================
 SR150 FiraLite Responder IOT Concurrency
=======================================================================

.. brief:start

This demo showcases concurrency, it communicates with iot applet to get random no and also
secure ranging with FiRaLite between two SR150-Se051W board, using FiraLite applet.
This demo will configure the device as Responder-Controlee.

In case of RTOS host platform with USB or UART, communication between initiator and
responder is handled using serial communication via PC, and python application listening
on UART com ports.
And in case of embed linux hosts communication between initiator and responder is handled
over IP, using socket, with initiator and responder devices as clients,
and a python server running on a PC in the same network.

.. brief:end

Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- This will wait for the communication from initiator
- Authenticate with initiator device using FiraLite applet
- Communicates to IOT applet parallely to get random number.
- Get the wrapped ranging data set using FiraLite applet
- Set the application ranging parameters
- Perform secure ranging with dynamic STS.


Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- SE051W should be connceted to host

  .. only:: nxp
    Refer :ref:`board-se051w`

- SR150 should be bound to SE051W. Refer :numref:`sr150-demo-binding` :ref:`sr150-demo-binding`
- Perform the ADF provisioning Refer :numref:`firalite-adf-provisioning` :ref:`firalite-adf-provisioning`
- Once ADF provisioning successful we can run ``demo_fl_responder_iot_concurrency`` binary


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

- Source:   ``demo_fl_responder_iot_concurrency``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:
Refer :numref:`demo-firalite-initiator`: :ref:`demo-firalite-initiator`.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV    :6200003D 1B010000 44332211 ... 00
    TMLUWB  :RX < :RECV    :6200003D 1C010000 44332211 ... 00
    TMLUWB  :RX < :RECV    :6200003D 1D010000 44332211 ... 00
    TMLUWB  :RX < :RECV    :6200003D 1E010000 44332211 ... 00
    TMLUWB  :TX > :SEND    :22010004 44332211
    TMLUWB  :RX < :RECV    :60070001 0A
    TMLUWB  :TX > :SEND    :22010004 44332211
    TMLUWB  :RX < :RECV    :42010001 00
    TMLUWB  :RX < :RECV    :61020006 44332211 0300
    TMLUWB  :TX > :SEND    :21010004 44332211
    TMLUWB  :RX < :RECV    :60010001 01
    TMLUWB  :RX < :RECV    :41010001 00
    TMLUWB  :RX < :RECV    :61020006 44332211 0100
    APP     :INFO :Finished ../../demos/SR150/demo_fl_responder_iot_concurrency/demo_fl_responder_iot_concurrency.c : Success!

If such a log is not seen, re-run the steps .
