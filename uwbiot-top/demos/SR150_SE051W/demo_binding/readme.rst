..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _sr150-demo-binding:

=======================================================================
 demo_binding
=======================================================================

.. brief:start

This demo showcases SR150 Binding with SE051W in production process.

Note:
    1. Binding process shall be done only once during the lifetime of the IC
    2. Binding is an irresversible process. Once done, the IC state cannot be reverted to unbound state.

.. brief:end


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- For RTOS based platform refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`
- For embed linux platform refer :numref:`build-rpi-mk-shield` :ref:`build-rpi-mk-shield`

- Source:   ``demo_binding``


How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

- For embedded RTOS device flash the ``demo_binding.bin`` file.
- On linux platform run the built executable.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

   TMLUWB  :TX > :SEND                :29000000
   TMLUWB  :RX < :RECV                :4900001B 008C0000 01020304 05060757 33365030 2D3934B1 31473447 001B00
   TMLUWB  :TX > :SEND                :29010018 009B9B7F F5209D44 4790D321 47007474 6E6E6E62 62620000
   TMLUWB  :RX < :RECV                :49010001 00
   TMLUWB  :TX > :SEND                :29030000
   TMLUWB  :RX < :RECV                :49030010 000E8150 00000800 01020304 05060700
   App     :INFO :pOutData            :00010203 04050607 0000FF03 607CD913 CC843A81 26348691 5F6C1263 68
   TMLUWB  :TX > :SEND                :29040011 107CD913 CC843A81 26348691 5F6C1263 68
   TMLUWB  :RX < :RECV                :49040018 00168582 33001046 5F12FFE8 28929FBC AEC21F9F C9425E00
   App     :INFO :pOutData            :
   TMLUWB  :TX > :SEND                :29050001 30
   TMLUWB  :RX < :RECV                :49050010 000E8530 000008A9 81B6ED6C 33528E00
   App     :INFO :pOutData            :AD18723E CCDEFFBF
   TMLUWB  :TX > :SEND                :2906000B 0AAD1872 3ECCDEFF BF9000
   TMLUWB  :RX < :RECV                :49060001 00
   TMLUWB  :TX > :SEND                :29020000
   TMLUWB  :RX < :RECV                :49020001 00
   hostLib :INFO :rspBuf              :6F218410 A0000003 96545300 00000104 02000000 A50DBF0C 0A9F7E02 00074C03 000000
   App     :INFO :Applet is Selected successfully!!!! .
   App     :INFO :SUS Applet is Bound and in unlocked state
   APP     :INFO :
   Finished uwbiot-top\demos\SR150\demo_binding\demo_binding.c : Success!

If such a log is not seen, re-run the program.
