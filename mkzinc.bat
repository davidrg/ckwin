@echo off
REM
REM Builds OpenZinc using customised makefiles from kermit\zinc-build
REM 
REM TODO: Detect compiler and set zinc_build accordingly, exiting if no suitable makefiles exist
REM In ui_env.hpp, uncomment #define ZIL_WIND0W_CTL3D
REM	-> read up about ctl3dv2.dll - who, what, when, where, why (msdn)
REM 

if "%ZINCBUILD%" NEQ "" goto :build

echo.
echo No OpenZinc build configuration available for this compiler! Set the
echo ZINCBUILD environment variable to the basename of the OpenZinc configuration
echo to use (eg, set ZINCBUILD=mvcpp400mt to use the mvcpp400mt.mak makefiles)
echo and try again.
echo.
goto :end

:build
cd %root%\zinc

if not exist source\%zincbuild%.mak call copymak.bat %zincbuild%.mak %root%\kermit\zinc-build

cd source
nmake -f %ZINCBUILD%.mak winnt || goto :error
cd ..

cd design

cd service
nmake -f %ZINCBUILD%.mak winnt || goto :error
cd ..

cd direct
nmake -f %ZINCBUILD%.mak winnt || goto :error
cd ..

cd storage
nmake -f %ZINCBUILD%.mak winnt || goto :error
cd ..

cd stredit
nmake -f %ZINCBUILD%.mak winnt || goto :error
cd ..

cd i18n
nmake -f %ZINCBUILD%.mak winnt || goto :error
cd ..

nmake -f %ZINCBUILD%.mak winnt || goto :error
cd ..
cd ..

set CKF_ZINC=yes
set CK_HAVE_ZINC_NT=yes
set BUILD_ZINC=no

echo.
echo.
echo OpenZinc built successfully - you can now build the C-Kermit for Windows Dialer!
echo.

goto :end

:error
echo OpenZinc build failed.
echo.
echo If you've previously built OpenZinc with a different compiler, run cleanall.bat
echo in the zinc subdirectory and try again.
echo.

:end
cd %root%