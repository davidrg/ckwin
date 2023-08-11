@echo off
setlocal

if [%1] == [] goto :help
set CKB_LIBSSH=%1
set CKB_OPENSSL=%2
set CKB_ZLIB=%3

set CKB_LIBSSH_ZLIB_ROOT=%root%\zlib\%CKB_ZLIB%
if not exist %CKB_LIBSSH_ZLIB_ROOT%\zlib.h set CKB_LIBSSH_ZLIB_ROOT=%ZLIB_ROOT%

set CKB_LIBSSH_OPENSSL_ROOT=%root%\openssl\%CKB_OPENSSL%
if not exist %CKB_LIBSSH_OPENSSL_ROOT%\NUL set CKB_LIBSSH_OPENSSL_ROOT=%openssl_root%

if exist %CKB_OPENSSL_ZLIB_ROOT%\zlib.h set CKB_LIBSSH_ZLIB_OPT=-DZLIB_ROOT:PATH=%CKB_LIBSSH_ZLIB_ROOT%

echo.
echo LibSSH Build Configuration
echo ---------------------------
echo LibSSH: %CKB_LIBSSH%
echo OpenSSH: %CKB_LIBSSH_OPENSSL_ROOT%
echo zlib: %CKB_LIBSSH_ZLIB_ROOT%
echo.

pushd %root%\libssh\%CKB_LIBSSH%

REM run the build!
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DOPENSSL_ROOT_DIR=%CKB_LIBSSH_OPENSSL_ROOT% %CKB_LIBSSH_ZLIB_OPT%
nmake
cd ..

popd

echo.
echo LibSSH has built. If it built successfully, you'll want to open a new
echo terminal and run the following to use it:
echo   set root=%root%
if exist %CKB_OPENSSL_ZLIB_ROOT%\zlib.h echo   set zlib_root_override=%CKB_OPENSSL_ZLIB_ROOT%
echo   set openssl_root_override=%CKB_OPENSSL_ROOT_DIR%
echo   set libssh_root_override=%%root%%\libssh\%CKB_LIBSSH%
echo   set libssh_build_override=%%root%%\libssh\%CKB_LIBSSH%\build
echo   cd %root%
echo   setenv.bat
echo alternatively, update setenv.bat and update the various root paths
echo.

goto :end

:setenv
echo.
echo Error: C-Kermit build environment has not been setup. Run setenv.bat.
echo.
goto :end

:help
echo.
echo Builds LibSSH in the specified directory under \libssh
echo.
echo Parameters
echo    libssh      Build LibSSH in \libssh\${libssh}
echo    openssl     Use OpenSSL in \openssl\${openssl}. Optional. If not specified,
echo                the OpenSSL root configured by set-env.bat will be used.
echo    zlib        Use zlib in \zlib\${zlib}. Optional. If not specified,
echo                the zlib root configured by set-env.bat will be used. If no zlib
echo                can be found, LibSSH will be built without zlib support.
echo.
echo This script should be run from within the C-Kermit build environment setup by
echo setenv.bat.
echo.

:end