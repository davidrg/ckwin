REM This will build everything that can be built:
REM  - C-Kermit GUI (k95g.exe)
REM  - C-Kermit Console (k95.exe) and associated utilities (iksd.exe, iksdsvc.exe, k95d.exe, rlogin.exe, telnet.exe, textps.exe, se.exe)
REM  - The Dialer (k95dial.exe) and registry tool (k95regtl.exe) - only if zinc has been built.
REM To collect build artifacts, run mkdist 

@echo off

if "%BUILD_ZINC%" == "yes" goto :buildzinc 
goto :buildkermit 

:buildzinc
echo Building OpenZinc...
call mkzinc.bat

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