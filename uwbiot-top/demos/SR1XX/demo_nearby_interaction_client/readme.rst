..
    Copyright 2023 NXP

    NXP Confidential. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms.  By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. include:: <isonum.txt>

.. _sr150-demo-nearby-interaction-client:

=======================================================================
 Demo nearby interaction Client
=======================================================================

.. brief:start

This demo showcases ranging via Bluetooth LE |trade| with background(pairing) feature with SR150 configured as a Controller - Initiator.
This is the counter part of the : :numref:`sr150-demo-nearby-interaction` :ref:`sr150-demo-nearby-interaction`

.. brief:end

For details on Peer-to-Peer ranging sequence and configuration details
for SR150 refer to :ref:`p2p-ranging`.

Android
.............
    #) By default the UWB spec version is set to v1.0 in :file:`UwbApi_types.h`.

IOS
..........
    #) Not Supported Yet.
App Ranging with Bluetooth LE |trade| on QN9090-SR150
=======================================================================

In this demo, first asks to pair the UWB device, once paired it sends commands over Bluetooth LE |trade| to QN9090 (:numref:`sr150-demo-nearby-interaction` :ref:`sr150-demo-nearby-interaction`)
to initialize and configure session and start ranging.

By Default the Low Power Mode is Enabled, to disable low power mode we need to set the below macros as ``0``
inside :file:`uwbiot-top/boards/Host/Rhodes4/app_preinclude.h`

``#define cPWR_UsePowerDownMode 0``

``#define cPWR_FullPowerDownMode 0``

.. note::
    ``gatt_uuid128.h`` file is moved from q9090 sdk to application as ``gatt_uuid128_uwb_ble.h``, to update service UUID
    to match ios expection.

    Similarly ``gatt_db.h`` file is moved to application as ``gatt_db_uwb_ble.h`` to update qpps rx characteristic.


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the Bluetooth LE |trade| based Ranging project for SR150 Using QN9090
MCUXpresso Project:

- Project:  ``RhodesV4_SE``
- Source:   ``demo_nearby_interaction_client``

Refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

After program execution, log message like this must be seen::

    #################################################
    ## Demo BLE Tracker counterpart : SR150
    ## UWBIOT_v04.05.05
    #################################################
    Device_MAC_ID = 00:60:37:29:bd:4c
    Scanning...
    Found device: Tag
    Connected!
    Scanning...
    APP     :WARN :device init
    FWDNLD  :INFO :FWDL Directly from host
    HALUCI  :INFO :Starting FW download
    HALUCI  :INFO :FW Download done.
    TMLUWB  :RX < :RECV                :60010001 00
    |
    |
    TMLUWB  :RX < :RECV                :62000048 11000000 01000000 00F00000 .. 98D080D0
    APP     :INFO :TWR[0].distance        : 21
    TMLUWB  :RX < :RECV                :62000048 12000000 01000000 00F00000 .. 51D040D0
    APP     :INFO :TWR[0].distance        : 18
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :60070001 0A
    TMLUWB  :RX < :RECV                :62000048 3C000000 01000000 00F00000 .. 00000000
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :62000048 3D000000 01000000 00F00000 .. 00000000
    TMLUWB  :RX < :RECV                :61020006 01000000 0300
    APP     :WARN :device deinit

If such a log is not seen, re-run the program.
