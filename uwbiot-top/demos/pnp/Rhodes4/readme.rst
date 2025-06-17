..
    Copyright 2021 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _pnp-fw-rv4:

=======================================================================
 RV4 Plug-n-Play FW
=======================================================================

.. brief:start

This firmware is used as a PnP FW for RhodesV4 to run applications from PC.
PC would send commands over UART to the PnP firmware, which would be forwarded
to SR150 and the responses and notifications are returned to PC.

.. brief:end

How to Use
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The PnP FW supports internal FW download and also from application,
by default internal firmware download is disabled.
to enable the internal firmware download, enable following macro.

.. literalinclude:: /libs/halimpl/inc/phUwb_BuildConfig.h
    :language: c
    :start-after: /* doc:start:enable-int-fw-download */
    :end-before: /* doc:end:enable-int-fw-download */

Flash the FW on to RhodesV4 board.

Pre-built firmware is present in :file:`binaries/RhodesV4`
directory.

On PC, set environment variable ``UWBIOT_ENV_COM``
to the RhodesV4 UART *COMPORT* and execute the application.


How to run PNP at higher baudrate
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
To run the pnp on higher baud rate than 3000000, low power
mode has to be disabled on QN9090 platform which will increase the throughput

.. literalinclude:: /boards/Host/Rhodes4/app_preinclude.h
    :language: c
    :start-after: /* doc:start:disable-lpm */
    :end-before: /* doc:end:disable-lpm */

In file :file:`demos/pnp/rhodesV4/pnp_usart.c`, set ``DEMO_USART_BAUDRATE`` to use higher baud rate.

How to run PNP with Hardware flow control
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
To run the pnp with Hardware flow control enabled

.. literalinclude:: /boards/Host/Rhodes4/app_preinclude.h
    :language: c
    :start-after: /* doc:start:enable-HW-flow */
    :end-before: /* doc:end:enable-HW-flow */

Watch Dog Timer
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

See as well ``WATCHDOG_TIMEOUT_SECONDS``.

.. literalinclude:: pnp_usart.c
    :language: c
    :start-after: /* doc-start:wtd */
    :end-before: /* doc-end:wtd */
