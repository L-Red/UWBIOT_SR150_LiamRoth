Overview
========

This application is for internal nightly/sanity testing on different
boards.


Prepare the Demo
================

No special prepration needd. A71CH must be with Debug mode enabled.


Running the demo
================

When the Demo is RUN, following output is created.

    a71ch HostLibrary test application (Rev 1.0.0)
    **********************************************
    How about Testing the millisecond Timer
    Set timer for 1800 milliSeconds
    sm_sleep() for 1800 ms
    Platfrom/IC Timer measured : 1806
            Delta = 6 ms
            Percent Change = 0%
    ==== Milli Seconds Test Passed
    **********************************************



    microsecond delay tests start



    **********************************************
    Set timer for 2700 milliSeconds
    sm_usleep() for 2700 ms
    Platform/IC Timer measured 2810
            Delta = 110 ms
            Percent Change = 4%
    ==== Micro Seconds Test Passed
    **********************************************



    checkTimerCalibration Finished
    Connect to A71CH-SM. Chunksize at link layer = 256.
    ATR=0xB8.04.11.01.05.04.B9.02.01.01.BA.01.01.BB.0C.41.37.31.30.78.43.48.32.34.32.52.31.BC.00.
    SCI2C_HostLib Version  : 0x0130
    Applet Version   : 0x0131
    SecureBox Version: 0x0000

    ==========SELECT-DONE=========
    ==========FINISHED=========
