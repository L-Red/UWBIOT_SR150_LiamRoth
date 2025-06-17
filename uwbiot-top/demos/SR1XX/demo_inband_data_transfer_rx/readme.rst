..
    Copyright 2021,2023 NXP.

    NXP Confidential. This software is owned or controlled by NXP and may only be
    used strictly in accordance with the applicable license terms. By expressly
    accepting such terms or by downloading,installing, activating and/or otherwise
    using the software, you are agreeing that you have read,and that you agree to
    comply with and are bound by, such license terms. If you do not agree to be
    bound by the applicable license terms, then you may not retain, install, activate
    or otherwise use the software.

.. _demo-inband-data-transfer-rx:

=======================================================================
 SR150 In Band Data Transfer Rx
=======================================================================

.. brief:start

This demo showcases data transfer feature with one SR150 configured as a Controllee - Responder
and another SR150 configured as a Controller - Initiator [Another demo].


.. brief:end

Following sequence of steps are handled.

- Intialize UWBD in Mainline Firmware.
- Initialize the ranging session and Set the application configuration parameters
- Start the ranging session to move the device to ACTIVE state
- Invoke Send data API to receive the data and wait for Data Rx notification

Limitations:

-  Maximum of 2031 bytes can be transferred.


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

- Source:   ``demo_inband_data_transfer_rx``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_inband_data_transfer_rx.bin`` file.
- On linux platform run the built executable.
- Run the other demo :numref:`demo-inband-data-transfer-tx` :ref:`demo-inband-data-transfer-tx` for normal ranging.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :02002501 01000001 00111100 00000000 00000014
    |
    |
    APP     :INFO :Data Receive NTF Received
    APP     :INFO :Data received successfully!!!
    |
    |
    TMLUWB  :RX < :RECV                :62000048 42000000 01000001 .. 8ED080D0
    TMLUWB  :RX < :RECV                :62000048 43000000 01000001 .. 5CD040D0
    TMLUWB  :RX < :RECV                :61020006 01000001 0305
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :TX > :SEND                :21010004 01000001
    TMLUWB  :RX < :RECV                :60070001 0A
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :21010004 01000001
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000001 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_inband_data_transfer_rx/demo_inband_data_transfer_rx.c : Success!

If such a log is not seen, re-run the program.