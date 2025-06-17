..
    Copyright 2021-2023 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _rhodesv4se-McuXpresso-project:

=======================================================================
RhodesV4-SE Standalone MCUXpresso Project
=======================================================================
.. brief:start

The QN9090_SR150 SE and Non SE based application can be build and deploy using
``RhodesV4_SE`` MCUXpresso project .

One can use this project to build new UWBIoT based Application for RhodesV4-SR150 with SE051W platform.

.. brief:end

Demo Application
=======================================================================

Below demo applications are part of this project

QN9090-SR150 supported Demos
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. :ref:`demo-inband-data-transfer-rx`
#. :ref:`demo-inband-data-transfer-tx`
#. :ref:`demo-ranging-controlee`
#. :ref:`demo-ranging-controller`
#. :ref:`demo-otp-storage-factory`
#. :ref:`demo-otp-storage-mainline`
#. :ref:`demo-UL-Tdoa-Anchor`
#. :ref:`demo-UL-TDOA-Tag`
#. :ref:`sr150-demo-nearby-interaction`
#. :ref:`sr150-demo-nearby-interaction-client`
#. :ref:`demo-test-tx`
#. :ref:`demo-test-rx`

QN9090-SR150-SE051W supported Demos
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. :ref:`sr150-demo-semslite-FiRaLite-A739-Run4`
#. :ref:`SR150-demo-FiRaLite-ADF-Provision`
#. :ref:`sr150-demo-binding`
#. :ref:`demo-firalite-initiator`
#. :ref:`demo-firalite-responder`

Prerequisites
=======================================================================

- Requires MCUXpresso IDE v11.2.0 or later

- QN9090DK6 SDK Version 2.x should be installed in MCUXpresso IDE

How to Build Demo Application
=======================================================================

For compiling any of above demo application one need to enable specific demo
from ``UWBIOT_APP_BUILD.h`` file

.. note:: only one demo application can be build & deploy at a time.
