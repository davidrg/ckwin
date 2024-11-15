@echo off
REM This will build everything that can be built:
REM  - XYZ Modem library (p95.dll)
REM  - libdes (if present and a supported compiler is being used)
REM  - C-Kermit GUI (k95g.exe)
REM  - C-Kermit Console (k95.exe) and associated utilities (iksd.exe, iksdsvc.exe, k95d.exe, rlogin.exe, telnet.exe, textps.exe, se.exe)
REM  - The Dialer (k95dial.exe) and registry tool (k95regtl.exe) - only if zinc has been built.
REM To collect build artifacts, run mkdist

echo Clean distribution folders...
call clean-dist.bat

if "%BUILD_ZINC%" == "yes" goto :buildzinc 
goto :buildp

:buildzinc
echo Building OpenZinc...
call mkzinc.bat

:buildp
if "%CKF_XYZ%" == "no" goto :buildkermit
cd kermit\p95
call mknt.bat
cd ..\..

if "%CKF_LIBDES%" NEQ "yes" goto :buildkermit
cd libdes
call mknt.bat
cd ..

:buildkermit
echo Building C-Kermit
cd kermit\k95
call mk.bat
call mkdist.bat
if "%CKF_ZINC%" == "no" goto :nozinc
echo Building the Dialer...
cd ..\dialer
call mk.bat
call mkdist.bat

:nozinc
cd ..\..\

echo Done!