..
    Copyright 2021-2023 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. include:: <isonum.txt>

.. _sr150-demo-nearby-interaction:

=======================================================================
 Demo nearby interaction
=======================================================================

.. brief:start

This demo showcases ranging via Bluetooth LE |trade| with background(pairing) feature with SR150 configured as
a either Controller - Initiator or Controlee - Responder.

.. brief:end

For details on Peer-to-Peer ranging sequence and configuration details
for SR150 refer to :ref:`p2p-ranging`.

IOS
.............
    #) By default the UWB spec version is set to v1.1. To enable v1.0, we shall set the ``UWB_IOS_SPEC_VERSION_MINOR`` to ``0x00, 0x00`` in :file:UwbApi_types.h.

    #) By default the Bonding and pairing capability is disabled, To enable them we shall set the below macros to ``1``

        ``#define gAppUseBonding_d 1``

        ``#define gAppUsePairing_d 1``

Android
.............
    #) By default the UWB spec version is set to v1.0 in :file:UwbApi_types.h.

* For multiple connection using ble, gAppMaxConnections_c define needs to updated to max number of connections allowed in :file:`app_preinclude.h`, at location ``boards/Host/Rhodes4/app_preinclude.h``, by default it's defined to 5. Clean build is needed after updating the :file:`app_preinclude.h`

App Ranging with Bluetooth LE |trade| on QN9090-SR150
=======================================================================

In this demo, a smartphone first asks to pair the UWB device, once paired it sends commands over Bluetooth LE |trade| to QN9090
to initialize and configure session and start ranging.

By Default the Low Power Mode is Enabled, To Disable the Low Power Mode we need to set the below macros as ``0``
inside :file:`uwbiot-top/boards/Host/Rhodes4/app_preinclude.h`

``#define cPWR_UsePowerDownMode 0``

``#define cPWR_FullPowerDownMode 0``

For UWB and App developer specific changes refer
``/*Define for App developer*/`` & ``/*Define for UWB developer*/`` section in
``TLV_Mng.c`` file.

.. note::
    ``gatt_uuid128.h`` file is moved from q9090 sdk to application as ``gatt_uuid128_uwb_ble.h``, to update service UUID
    to match ios expection.

    Similarly ``gatt_db.h`` file is moved to application as ``gatt_db_uwb_ble.h`` to update qpps rx characteristic.


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the Bluetooth LE |trade| based Ranging project for SR150 Using QN9090
MCUXpresso Project:

- Project:  ``RhodesV4_SE``
- Source:   ``demo_nearby_interaction``

Refer :numref:`rhodesv4se-McuXpresso-project` :ref:`rhodesv4se-McuXpresso-project`.

SR150 configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

According to specification here are the configuration that can be changed:

- ``TLV_Types_i.h``
  -   DEVICE_ROLE::

        #define DEMO_DEVICE_TYPE kUWB_DeviceType_Controller     // Configure accessory as Controller
        #define DEMO_DEVICE_ROLE kUWB_DeviceRole_Initiator      // Configure accessory as Initiator
        // #define DEMO_DEVICE_TYPE kUWB_DeviceType_Controlee   // Configure accessory as Controlee
        // #define DEMO_DEVICE_ROLE kUWB_DeviceRole_Responder   // Configure accessory as Responder


- ``TLV_Mng.c``::

    uint8_t SpecMajorVersion[2] = {0x01, 0x00};                 // Spec major version
    uint8_t SpecMinorVersion[2] = {0x01, 0x00};                 // Spec minor version
    configData.PreferedUpdateRate                               // Prefered Update data rate (use PreferedUpdateRate_t definition)

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

After program execution, log message like this must be seen::

    #################################################
    ## Demo Nearby Interaction (BLE tracker) : SR150
    ## UWBIOT_v04.05.05
    #################################################
    Device_MAC_ID = 00:60:37:0f:1f:cf
    BLE Start adv
    APP     :WARN :device init
    |
    |
    TMLUWB  :RX < :RECV                :62000048 13000000 01000000 .. 89D080D0
    TMLUWB  :RX < :RECV                :62000048 14000000 01000000 .. 9DD080D0
    BLE Disconnected
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :60070001 0A
    TMLUWB  :RX < :RECV                :62000048 15000000 01000000 .. 8AD080D0
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0300
    TMLUWB  :TX > :SEND                :21010004 01000000
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0100
    APP     :INFO :BLE Disconnected peer 1, 0 peers conenctd
    APP     :WARN :device deinit

If such a log is not seen, re-run the program.