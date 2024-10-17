REM This will clean existing dist folders

@echo off

echo Clean main dist...
if exist dist\NUL rmdir /S /Q dist
if exist dist-os2\NUL rmdir /S /Q dist-os2

echo Clean K95 dist...
cd kermit\k95
call clean-dist.bat

if "%CKF_ZINC%" == "no" goto :nozinc
echo Clean Dialer dist...
cd ..\dialer
call clean-dist.bat

:nozinc
if "%CKF_XYZ%" == "no" goto :noxyz
echo Clean p95 build...
cd ..\p95
del p95.dll p95.lib p2.dll p2.lib
:noxyz

:done
cd ..\..\
echo Done!