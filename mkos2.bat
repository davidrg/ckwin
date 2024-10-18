@echo off
REM This will build everything that can be built:
REM  - XYZ Modem library (p95.dll)
REM  - libdes (if present and a supported compiler is being used)
REM  - Kermit/2 (k2.exe) and associated utilities (k2clip, k2dc, rlogin, telnetpm, textps, setup)
REM  - Any TC/IP backends that can be built (cko32i41.dll, cko32i20.dll)
REM  - The Dialer (k95dial.exe) - only if zinc is available
REM To collect build artifacts, run mkdist-os2

if "%BUILD_ZINC%" == "yes" goto :buildzinc
goto :buildp

:buildzinc
REM The OS/2 binaries and libs distributed with OpenZinc *seem* fine for Watcom
REM echo Building OpenZinc...
REM call mkzinc.bat

:buildp
if "%CKF_XYZ%" == "no" goto :buildkermit
cd kermit\p95
call mkos2.bat
cd ..\..

if "%CKF_LIBDES%" NEQ "yes" goto :buildkermit
REM How do we build this?
REM cd libdes
REM call mknt.bat
REM cd ..

:buildkermit
echo Building Kermit/2
cd kermit\k95
call mkos2.bat
call mkdist-os2.bat
if "%CKF_ZINC%" == "no" goto :nozinc
echo Building the Dialer...
cd ..\dialer
call mkos2.bat
call mkdist-os2.bat

:nozinc
cd ..\..\

echo Done!