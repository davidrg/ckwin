@echo off
setlocal

if [%CKB_TARGET_ARCH%] == [] goto :setenv

if [%1] == [] goto :help
set CKB_OPENSSL=%1
set CKB_ZLIB=%2

set CKB_OPENSSL_PLATFORM=VC-WIN32
if [%CKB_TARGET_ARCH%] == [IA64] set CKB_OPENSSL_PLATFORM=VC-WIN64I
if [%CKB_TARGET_ARCH%] == [AMD64] set CKB_OPENSSL_PLATFORM=VC-WIN64A
if [%CKB_TARGET_ARCH%] == [ARM64] set CKB_OPENSSL_PLATFORM=VC-WIN64-ARM
if [%CKB_TARGET_ARCH%] == [ARM] set CKB_OPENSSL_PLATFORM=VC-WIN32-ARM

set CKB_OPENSSL_ZLIB_ROOT=%root%\zlib\%CKB_ZLIB%
if not exist %CKB_OPENSSL_ZLIB_ROOT%\zlib.h set CKB_OPENSSL_ZLIB_ROOT=%ZLIB_ROOT%

echo.
echo OpenSSL Build Configuration
echo ---------------------------
echo OpenSSL: %CKB_OPENSSL%
echo Platform: %CKB_OPENSSL_PLATFORM%
echo zlib: %CKB_OPENSSL_ZLIB_ROOT%
echo Target Architecture: %CKB_TARGET_ARCH%
echo.

pushd %root%\openssl\%CKB_OPENSSL%

REM for old versions of OpenSSL (<= 1.0.2) we'll enable the static engine
if exist ms\do_ms set CKB_OPENSSL_STATIC_ENGINE=enable-static-engine

REM If we have zlib, enable it.
if exist %CKB_OPENSSL_ZLIB_ROOT%\zlib.h set CKB_OPENSSL_ZLIB=zlib-dynamic --with-zlib-include=%CKB_OPENSSL_ZLIB_ROOT%

REM Visual C++ 6 is to old to do IPv6 so disable it.
cl 2>&1 | findstr /C:"Version 12.0" > nul
if %errorlevel% == 0 set CKB_OPENSSL_NO_IPV6=-DOPENSSL_USE_IPV6=0

REM run the build!
perl Configure %CKB_OPENSSL_PLATFORM% %CKB_OPENSSL_ZLIB% -D"_WIN32_WINNT=0x502" %CKB_OPENSSL_NO_IPV6%
if exist ms\do_ms goto :build_old

:build_new
REM Build OpenSSL 1.1.0 or newer
%MAKE%
goto :build_done

:build_old
REM Build 1.0.2, 1.0.1 or 1.0.0.
ms\do_nasm
REM For a non-optimised build, run ms\do_ms instead of ms\do_nasm
nmake -f ms\ntdll.mak

:build_done
popd

echo.
echo OpenSSL has built. If it built successfully, you'll want to open a new
echo terminal and run the following to use it:
echo   set root=%root%
if exist %CKB_OPENSSL_ZLIB_ROOT%\zlib.h echo   set zlib_root_override=%CKB_OPENSSL_ZLIB_ROOT%
echo   set openssl_root_override=%root%\openssl\%CKB_OPENSSL%
echo   cd %root%
echo   setenv.bat
echo alternatively, update setenv.bat and update the openssl_root path.
echo.

goto :end

:setenv
echo.
echo Error: C-Kermit build environment has not been setup. Run setenv.bat.
echo.
goto :end

:help
echo.
echo Builds OpenSSL in the specified directory under \openssl
echo.
echo Parameters
echo    openssl		Build OpenSSL in \openssl\${openssl}
echo    zlib        Use dynamic zlib in \zlib\${zlib}. Optional. If not specified,
echo                the zlib root configured by set-env.bat will be used. If no zlib
echo                can be found, OpenSSL will be built without zlib support.
echo.
echo This script must be run from within the C-Kermit build environment setup by
echo setenv.bat. You'll need nasm on your path along with perl with 
echo Text::Template installed.
echo.

:end