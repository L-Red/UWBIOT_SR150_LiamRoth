..
    Copyright 2022 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-owr-aoa-advertiser:

=======================================================================
 OWR AoA advertiser
=======================================================================

.. brief:start

This demo showcases OWR Special use case App ranging with SR150 configured
owr aoa advertiser and sends data packets to a owr aoa observer.

.. brief:end


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

- Source:   ``demo_owr_aoa_advertiser``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_owr_aoa_advertiser.bin`` file.
- On linux platform run the built executable.
- Run the counterpart demo :numref:`demo-owr-aoa-observer` :ref:`demo-owr-aoa-observer` for normal ranging.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :62000038 4F3A0000 01000001 00C80000 ... 4AC12F2F
    TMLUWB  :RX < :RECV                :62000038 503A0000 01000001 00C80000 ... A6C02A2A
    TMLUWB  :RX < :RECV                :62000038 513A0000 01000001 00C80000 ... DCC03030
    APP     :INFO :Data received successfully!!!
    TMLUWB  :TX > :SEND                :22010004 01000001
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 01000001 0300
    TMLUWB  :TX > :SEND                :21010004 01000001
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000001 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/SR1XX/demo_owr_aoa_observer/demo_owr_aoa_observer.c : Success!

If such a log is not seen, re-run the program.