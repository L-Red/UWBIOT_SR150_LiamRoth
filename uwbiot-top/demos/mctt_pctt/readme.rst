..
    Copyright 2021 NXP

    NXP Confidential. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms.  By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.


.. _Mctt-Pctt-fw:

=======================================================================================
 MCTT & PCTT Demo (SR1XX)
=======================================================================================

.. brief:start

MCTT : MAC FIRA Conformance Test Tool
PCTT : PHY FIRA Conformance Test Tool

This demo is used as a MCTT/PCTT FW QN9090(RV4) boards to run applications from PC.
PC application has to send MCTT/PCTT compliant commands over USB/UART to the
MCTT/PCTT firmware, which would be forwarded to SR1XX and the responses and notifications
are returned to PC.

.. brief:end

To run application in MCTT/PCTT  mode, application has to perform :cpp:func:`UwbApi_Init_New` with mMcttCallback set
and mCdcCallback and mAppCallback to ``NULL`` to activate MCTT/PCTT  mode of operation. Once Init is success,
Low Power Mode and NXP extended Notification configuration to be disabled if applicable. Tx Pulse shape
config is set to 47 instead of default value 2. After this application can send MCTT/PCTT compliant
commands to FW and response and notification are received.


How to Use
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Flash the mctt_pctt firmware binary on the board.
Use COMARCH tool to test MCTT feature.
Use LitePoint tool to test PCTT feature


How to Build(Standalone project)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the project for SR1xx Using MCUXpresso IDE Project :

- Source:   ``demo_mctt_pctt``

How to Build(CMAKE project)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Build the  mctt/pctt project for SR150 Using MCUXpresso Project

- Project:  ``cmake_qn9090_project``
- Source:   ``demo_mctt_pctt``

Build the mctt/pctt  project for SR100 Using MCUXpresso Project

- Project:  ``cmake_rhodes3_project``
- Source:   ``demo_mctt_pctt``


How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project ``demo_mctt_pctt`` has package compiled for SR1xx located at
:file:`binaries/` in the project and can be run for ``SR1XX`` natively as::

    demo_mctt_pctt.bin

Steps to be followed to run:

- Flash the ``demo_mctt_pctt.bin`` file to the SR1XX device
- Use required tool for required feature.
