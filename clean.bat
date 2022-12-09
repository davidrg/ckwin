REM This will build both C-Kermit and, when possible, the dialer.
REM To collect build artifacts, run mkdist 

@echo off
echo Clean C-Kermit build...
cd kermit\k95
call clean.bat

if "%CKF_ZINC%" == "no" goto :nozinc
echo Clean Dialer build...
cd ..\dialer
call clean.bat

:nozinc
if "%CKF_XYZ%" == "no" goto :done
echo Clean p95 build...
cd ..\p95
call clean.bat

:done
cd ..\..\
echo Done!