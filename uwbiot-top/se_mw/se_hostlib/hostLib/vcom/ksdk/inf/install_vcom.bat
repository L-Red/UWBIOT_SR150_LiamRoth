REM @echo off
@REM Copyright 2018 NXP
@REM
@REM NXP Confidential. This software is owned or controlled by NXP and may only
@REM be used strictly in accordance with the applicable license terms.  By
@REM expressly accepting such terms or by downloading, installing, activating
@REM and/or otherwise using the software, you are agreeing that you have read,
@REM and that you agree to comply with and are bound by, such license terms.  If
@REM you do not agree to be bound by the applicable license terms, then you may
@REM not retain, install, activate or otherwise use the software.
@REM

@REM

@REM This file has to be run as Adminstrator
cd /d %~dp0

@echo Installing VCOM Driver

call %SystemRoot%\system32\PnPutil.exe -i -a fsl_ucwxp.inf

pause
