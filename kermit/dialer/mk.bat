@echo off
if "%CK_HAVE_ZINC_NT%" == "no" goto :nozinc

%MAKE% -f k95dial.mak PLATFORM=NT winnt

REM The registry tool isn't useful on NT 3.x so don't build it for compilers
REM that only support NT 3.x
if "%ZINCBUILD%" == "mvcpp200mt" goto :end

cd registry
%MAKE% -f registry.mak PLATFORM=NT winnt
cd ..

goto :end

:nozinc
echo.
echo OpenZinc has not been configured in the Kermit 95 build environment!
echo This is normally done by setenv.bat when it detects a build of OpenZinc for your
echo current compiler, or by mkzinc.bat when it successfully builds OpenZinc for your
echo current compiler.
echo.
echo To force a build anyway, run:
echo    nmake -f k95dial.mak winnt
echo.

:end
