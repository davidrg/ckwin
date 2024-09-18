@echo off
REM Environment setup script for Visual C++ 1.0 32-bit Edition on Windows NT 3.1
REM ----------------------------------------------------------------------------
REM   Use this script to setup a build environment on Windows NT 3.1 using
REM   Visual C++ 1.0 32-bit edition. This is the minimum compiler and nmake
REM   version supported by Kermit 95 - versions of the Win32 SDK
REM   older than Visual C++ 1.0 32-bit edition do not work currently.
REM 

REM Uncomment this and set it to where your source code lives
REM set root=C:\src

REM Adjust this to point to where you installed Visual C++ 1.0 32-bit edition
call C:\MSVCNT\BIN\VCVARS32.BAT

REM Optional Features
REM -----------------
REM You can uncomment these to turn things on (or off). Other features are
REM turned on automatically when the required dependencies are built.

REM debug logging ("log debug" command)
REM set CKF_DEBUG=no

REM ============================================================================
REM ================== No changes required beyond this point ===================
REM ============================================================================

REM Visual C++ 1.0 32-bit edition targets NT 3.1 always.
set CK_DETECT_COMPILER=no
set CKT_NT31=yes
set CMP=VCXX
set COMPILER=Visual C++
set MSC_VER=80
set COMPILER_VERSION=1.00 (32-bit edition)
set TARGET_PLATFORM=Windows
set TARGET_CPU=x86

REM base include path - this is required for both Windows and OS/2
set ckinclude=%root%\kermit\k95

REM This and everything else is windows-specific.
set ckwinclude=%ckinclude%;%root%\kermit\k95\kui

REM Set include path for targeting Windows.
set include=%include%;%ckwinclude%

set CK_DIST_DLLS=

REM None of these are supported on NT 3.1
set CKF_SUPERLAT=no
set CKF_ZLIB=no
set CKF_SSL=no
set CKF_SSH=no
set CKF_LIBDES=no
set CKF_CRYPTDLL=no

set CK_COMPILER_NAME=%COMPILER% %COMPILER_VERSION% %TARGET_CPU%

echo -----------------------------
echo.
echo Include path set to:
echo    %include%
echo.
echo Library path set to:
echo    %lib%
echo.
echo Compiler: %CK_COMPILER_NAME%
echo.

echo.
goto :end


:end