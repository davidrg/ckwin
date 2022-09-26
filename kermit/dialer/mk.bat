@echo off
if "%CKF_ZINC%" == "no" goto :nozinc

nmake -f k95dial.mak winnt
cd registry
nmake -f registry.mak winnt
cd ..

goto :end

:nozinc
echo.
echo OpenZinc has not been configured in the C-Kermit for Windows build environment!
echo This is normally done by setenv.bat when it detects a build of OpenZinc for your
echo current compiler, or by mkzinc.bat when it successfully builds OpenZinc for your
echo current compiler.
echo.
echo To force a build anyway, run:
echo    nmake -f k95dial.mak winnt
echo.

:end
