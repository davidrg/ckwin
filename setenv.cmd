@echo off
REM C-Kermit for OS/2 Build Environment Script

REM Uncomment this and set it to where your source code lives
REM set root=C:\src
REM

REM To build the dialer, you must extract the OpenZinc distribution to .\zinc
REM Either grab OZ1.zip (the full distribution), or OZ1eng.zip (engine) and
REM OZ1os2.zip (OS/2 files).

set ZINCBUILD=
wcc386 . <nul 2>&1 > nul
if not %errorlevel% == 0 goto :unsupported
wcc386 . <nul 2>&1 | findstr /C:"Version 1.9" > nul
if %errorlevel% == 0 set ZINCBUILD=ow19
wcc386 . <nul 2>&1 | findstr /C:"Version 2.0" > nul
if %errorlevel% == 0 set ZINCBUILD=ow20

REM ================== No changes required beyond this point ===================

if "%ZINCBUILD%" == "" goto :unsupported
set include=%include%;%root%\kermit\k95
set include=%include%;%root%\zinc\include
set lib=%root%\zinc\lib\%ZINCBUILD%;%lib%

cd %root%\kermit\k95

echo C-Kermit for OS/2 Build Environment
echo.
echo Include path set to:
echo    %include%
echo Lib path set to:
echo    %lib%
echo.
echo Run mkos2 to build Kermit/2
echo cd ..\dialer and run mkos2 to build the Kermit/2 Dialer

:unsupported
