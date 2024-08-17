@echo off

REM Uncomment this and set it to where your source code lives
REM set root=C:\src

REM Optional Features
REM -----------------
REM You can uncomment these to turn things on (or off). Other features are
REM turned on automatically when the required dependencies are built.

REM debug logging ("log debug" command)
REM set CKF_DEBUG=no

REM Target NT 3.50 when building with OpenWatcom (this is automatic when building
REM with Visual C++ 2.0)
REM set CKT_NT31=yes

REM Optional Dependencies - zlib, openssl, libssh.
REM ----------------------------------------------
REM If you've built or installed these, uncomment and set a few more paths to
REM get more features like built-in SSH!

REM If you've installed these dependencies using vcpkg, set the following
REM to the vcpkg installed directory (where the include, lib and bin
REM subdirectories are) and ignore the rest.
REM set vcpkg_installed=C:\dev\vcpkg\installed\x86-windows

REM If you build these dependencies yourself using the instructions in
REM doc\optional-dependencies.md, the following should be correct. If you've
REM built them elsewhere (or built different versions of them) you'll need to
REM update the paths. If you're using vcpkg to get these dependencies you can
REM comment out the four SET lines below.

REM zlib - set to the folder where zlib.h and the .lib and .dll files live:
set zlib_root=%root%\zlib\1.2.13

REM openssl - set to the folder where the include folder and .lib and .dll files
REM live.
set openssl_root=%root%\openssl\1.1.1u

REM libssh - set to the folder where the include directory lives.
set libssh_root=%root%\libssh\0.10.5

REM libssh - set to where you built libssh. This should have a subdirectory
REM called 'src' containing ssh.dll and ssh.lib
set libssh_build=%libssh_root%\build

REM libdes - required for building k95crypt.dll and enabling a few features that
REm rely on obsolete and easily broken encryption
set libdes_root=%root%\libdes

REM Stanford SRP - Optional SRP Authentication for Telnet and FTP connections
REM Extract srp-2.1.2.tar.gz to the following location:
set srp_root=%root%\srp

REM Kerberos for Windows. Some examples of what you should find in the k4w_root:
REM    target\bin\i386\rel\wshelp32.dll
REM    target\lib\i386\rel\wshload.lib
REM    athena\wshelper\include\wshelper.h
REM Kermit 95 was last built with v2.2-beta2. K95 is known to work with 2.6.0.
REM
REM You can also point this at the root directory for the Kerberos for Windows
REM 3.x or 4.x SDK.
set k4w_root=%root%\kerberos\kfw-2.2-beta-2

REM Make program must be something sufficiently compatible with nmake 1.40 (Visual C++ 1.1). Jom is recommended
REM for doing parallel builds.
set make=nmake

REM ============================================================================
REM ================== No changes required beyond this point ===================
REM ============================================================================

echo Checking target architecture...

REM Figure out if we're doing a 64bit build or a 32bit one (this affects our
REM choices for some libraries later on), and what our target architecture is
REM (this affects whether we can run the generated binary on this machine)

REM This should match the %PROCESSOR_ARCHITECTURE% on the target machine
set CKB_TARGET_ARCH=x86

set CKB_OPENSSL_SUFFIX=
if exist %WATCOM%\binnt\wcl386.exe goto :bitcheckdone
cl 2>&1 | findstr /C:"for x64" > nul
if %errorlevel% == 0 goto :x64

cl 2>&1 | findstr /C:"for AMD64" > nul
if %errorlevel% == 0 goto :x64

cl 2>&1 | findstr /C:"for Itanium" > nul
if %errorlevel% == 0 goto :ia64

cl 2>&1 | findstr /C:"for IA-64" > nul
if %errorlevel% == 0 goto :ia64

cl 2>&1 | findstr /C:"for ARM64" > nul
if %errorlevel% == 0 goto :arm64

cl 2>&1 | findstr /C:"for ARM" > nul
if %errorlevel% == 0 goto :arm

REM Microsoft (R) & Digital (TM) AXP C/C++ Optimizing Compiler Version 8.03.JFa
cl 2>&1 | findstr /C:"AXP" > nul
if %errorlevel% == 0 goto :axp

REM Microsoft (R) & Digital (TM) Alpha C/C++ Optimizing Compiler Version 13.00.8499
cl 2>&1 | findstr /C:"Alpha" > nul
if %errorlevel% == 0 goto :axp

cl 2>&1 | findstr /C:"for MIPS R-Series" > nul
if %errorlevel% == 0 goto :mips

REM Win32 SDK Final Release MIPS compiler (NT 3.1)
REM Microsoft (R) C Centaur Optimizing Compiler Version 8.00.081
cl 2>&1 | findstr /C:"Microsoft (R) C Centaur Optimizing Compiler" > nul
if %errorlevel% == 0 goto :mips

cl 2>&1 | findstr /C:"for PowerPC" > nul
if %errorlevel% == 0 goto :ppc

REM Yes, the 64bit Windows for Alpha compiler exists. No, you can't run its output
REM on anything (unless you happen to work for Microsoft)
cl 2>&1 | findstr /R /C:"Digital.*Alpha.*Version 13.0" > nul
if %errorlevel% == 0 goto :bits64

REM Else something unknown or x86-32
goto :bits32

:arm
REM 32bit ARM (Windows RT)
REM TODO: Check
set CKB_TARGET_ARCH=ARM
goto :bits32

:axp
REM Catch the 64bit compiler:
REM Microsoft (R) & Digital (TM) Alpha C/C++ Optimizing Compiler Version 13.00.8499
cl 2>&1 | findstr /C:"13.00" > nul
if %errorlevel% == 0 goto :axp64

REM Alpha AXP Windows NT - 32bits
set CKB_TARGET_ARCH=ALPHA
goto :bits32

:axp64
REM Alpha AXP Windows 2000/XP - 64bits
REM TODO: Check
set CKB_TARGET_ARCH=ALPHA64
goto :bits64

:mips
REM MIPS Windows NT - 32bits
REM TODO: Check
set CKB_TARGET_ARCH=MIPS
goto :bits32

:ppc
REM PowerPC Windows NT - 32bits
REM TODO: Check
set CKB_TARGET_ARCH=PPC
goto :bits32

:x64
set CKB_OPENSSL_SUFFIX=-x64
set CKB_TARGET_ARCH=AMD64
goto :bits64

:arm64
set CKB_OPENSSL_SUFFIX=-arm64

REM TODO: Check
set CKB_TARGET_ARCH=ARM64
goto :bits64

:ia64
set CKB_OPENSSL_SUFFIX=-ia64
set CKB_TARGET_ARCH=IA64

REM libssh won't build for any compiler that can target IA64 windows
REM due to lack of C99 support. Force it off.
set CKF_SSH=no

REM Also no ZLIB support on Itanium (couldn't get it to easily
REM cross-compile) - not that K95 actually uses zlib for anything.
set CKF_ZLIB=no
goto :bits64

:bits64
REM Targeting a 64bit host (x86-64, IA64, ARM64, AXP64)
set CKB_BITS_64=yes
echo Targeting 64bit Windows

REM These libraries aren't supported for 64bit targets - force them off
set CKF_SRP=no
set CKF_SUPERLAT=no
goto :bitcheckdone

:bits32
set CKB_BITS_64=no
echo Targeting 32bit Windows
goto :bitcheckdone

:bitcheckdone

echo Processor Architecture: %PROCESSOR_ARCHITECTURE%
echo Target Architecture: %CKB_TARGET_ARCH%

REM Figure out if we can run the binaries we're compiling on this machine. This is
REM true if: we're targeting the same architecture as this machine, or we're targeting
REM a compatible architecture (targeting x86 on an ADM64 machine)
set CKB_CROSS_COMPATIBLE=no
if "%PROCESSOR_ARCHITECTURE%" == "%CKB_TARGET_ARCH%" set CKB_CROSS_COMPATIBLE=yes
if "%PROCESSOR_ARCHITECTURE%-%CKB_TARGET_ARCH%" == "AMD64-x86" set CKB_CROSS_COMPATIBLE=yes
REM TODO: is ARM64-x86 and ARM64-AMD64 ok? Will that run under emulation, or will it
REM       just fail like AMD64-IA64 does (hangs the build server)

if "%CKB_CROSS_COMPATIBLE%" == "no" echo Cannot run compiled binaries on this machine

REM Assume the toolchain we're using is not Windows 9x-compatible (we'll update
REM this later if we discover otherwise)
Set CKB_9X_COMPATIBLE=no

echo Searching for Optional Dependencies...

REM base include path - this is required for both Windows and OS/2
set ckinclude=%root%\kermit\k95

REM See if the user has the SuperLAT SDK (*extremely* unlikely)
if "%CKF_SUPERLAT%" == "no" echo Skipping check for SuperLAT
if "%CKF_SUPERLAT%" == "no" goto :nosuperlat
set CKF_SUPERLAT=no
if exist %root%\superlat\include\latioc.h set CKF_SUPERLAT=yes
if exist %root%\superlat\include\latioc.h set ckinclude=%ckinclude%;%root%\superlat\include
:nosuperlat

REM This and everything else is windows-specific.
set k95include=%ckinclude%;%root%\kermit\k95\kui

REM Set include path for targeting Windows.
set include=%include%;%k95include%

REM Handle path overrides. These are to allow the build server to override any
REM hard-coded definitions in here without having to modify.
if not "%zlib_root_override%"=="" set zlib_root=%zlib_root_override%
if not "%openssl_root_override%"=="" set openssl_root=%openssl_root_override%
if not "%libssh_root_override%"=="" set libssh_root=%libssh_root_override%
if not "%libssh_build_override%"=="" set libssh_build=%libssh_build_override%
if not "%libdes_root_override%"=="" set libdes_root=%libdes_root_override%
if not "%srp_root_override%"=="" set srp_root=%srp_root_override%
if not "%k4w_root_override%"=="" set k4w_root=%k4w_root_override%
if not "%make_override%"=="" set make=%make_override%

REM The OpenWatcom 1.9 linker can't handle %LIB% starting with a semicolon which
REM is what we get when we do "set LIB=%LIB%;C:\somewhere" when LIB starts out
REM empty. So we need to make sure there is always at least *something* in the
REM lib path before we go appending things to it. That something doesn't
REM actually have to exist, so when starting with an empty LIB path we'll just
REM add something thats sure not to have any libs in it. For reference, the
REM error you get if you don't do this is:
REM   Internal error on line 323 of ..\..\c\fuzzy.c. Please contact the Open Watcom maintainers at http://www.openwatcom.com/"
if not defined lib set lib=%root%\dummy
if "%lib%" == "" set lib=%root%\dummy

REM Add on any optional dependencies

set CK_DIST_DLLS=

if not defined vcpkg_installed goto :end_vcpkg

REM ------------------------------------------
REM Look for dependencies in the vcpkg folders
REM ------------------------------------------
set include=%include%;%vcpkg_installed%\include
set lib=%lib%;%vcpkg_installed%\lib

if "%CKF_ZLIB%" == "no" echo Skipping check for ZLIB
if "%CKF_ZLIB%" == "no" goto :novcpkgzlib
if exist %vcpkg_installed%\lib\zlib.lib set CKF_ZLIB=yes
if exist %vcpkg_installed%\bin\zlib1.dll set CK_ZLIB_DIST_DLLS=%vcpkg_installed%\bin\zlib1.dll
:novcpkgzlib

if "%CKF_SSL%" == "no" echo Skipping check for OpenSSL
if "%CKF_SSL%" == "no" goto :novcpkgssl
if exist %vcpkg_installed%\lib\libssl.lib set CKF_SSL=yes
if exist %vcpkg_installed%\tools\openssl\openssl.exe set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\tools\openssl\openssl.exe
if exist %vcpkg_installed%\bin\libcrypto-1_1%CKB_OPENSSL_SUFFIX%.dll set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\libcrypto-1_1%CKB_OPENSSL_SUFFIX%.dll %openssl_root%\libssl-1_1-%CKB_OPENSSL_SUFFIX%.dll
if exist %vcpkg_installed%\bin\libcrypto-3%CKB_OPENSSL_SUFFIX%.dll set CK_SSL_DIST_DLLS=%vcpkg_installed%\bin\libcrypto-3%CKB_OPENSSL_SUFFIX%.dll %vcpkg_installed%\bin\libssl-3%CKB_OPENSSL_SUFFIX%.dll
:novcpkgssl

if "%CKF_SSH%" == "no" echo Skipping check for libssh
if "%CKF_SSH%" == "no" goto :novcpkgssh
if exist %vcpkg_installed%\lib\ssh.lib set CKF_SSH=yes
if exist %vcpkg_installed%\bin\ssh.dll set CK_SSH_DIST_DLLS=%CK_DIST_DLLS% %vcpkg_installed%\bin\ssh.dll %vcpkg_installed%\bin\pthreadVC3.dll
:novcpkgssh

:end_vcpkg

REM ------------------------------------------------------------
REM Look for optional dependencies in the manual-build locations
REM ------------------------------------------------------------

REM zlib:
echo.
if "%CKF_ZLIB%" == "no" echo Skipping check for ZLIB
if "%CKF_ZLIB%" == "no" goto :nozlib
set CKF_ZLIB=no
if exist %zlib_root%\zlib.h set include=%include%;%zlib_root%
if exist %zlib_root%\zlib.lib set lib=%lib%;%zlib_root%
if exist %zlib_root%\zlib.lib set CKF_ZLIB=yes
if exist %zlib_root%\zlib.lib echo Found zlib: %zlib_root%\zlib.lib
if exist %zlib_root%\zlib1.dll set CK_ZLIB_DIST_DLLS=%zlib_root%\zlib1.dll
:nozlib

REM OpenSSL
echo.
set CKF_OPENSSL_VERSION=not found
if "%CKF_SSL%" == "no" echo Skipping check for OpenSSL
if "%CKF_SSL%" == "no" goto :nossl
set CKF_SSL=no
REM OpenSSL 0.9.8, 1.0.0, 1.1.0, 1.1.1 and 3.0.x use this:
if exist %openssl_root%\include\openssl\NUL set include=%include%;%openssl_root%\include
REM OpenSSL 1.0.1 and 1.0.2 uses this:
if exist %openssl_root%\inc32\openssl\NUL set include=%include%;%openssl_root%\inc32

REM OpenSSL 1.1.x, 3.0.x
if exist %openssl_root%\libssl.lib set lib=%lib%;%openssl_root%
if exist %openssl_root%\libssl.lib set CKF_SSL=yes
if exist %openssl_root%\libssl.lib echo Found OpenSSL 1.1.x or 3.0.x: %openssl_root%\libssl.lib
if exist %openssl_root%\libssl.lib set CKF_SSL_LIBS=libssl.lib libcrypto.lib
if exist %openssl_root%\apps\openssl.exe set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\apps\openssl.exe

REM OpenSSL 3.0.x
if exist %openssl_root%\libcrypto-3%CKB_OPENSSL_SUFFIX%.dll set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\libcrypto-3%CKB_OPENSSL_SUFFIX%.dll %openssl_root%\libssl-3%CKB_OPENSSL_SUFFIX%.dll
if exist %openssl_root%\libcrypto-3%CKB_OPENSSL_SUFFIX%.dll set CKF_OPENSSL_VERSION=3.x

REM OpenSSL 1.1.x
if exist %openssl_root%\libcrypto-1_1%CKB_OPENSSL_SUFFIX%.dll set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\libcrypto-1_1%CKB_OPENSSL_SUFFIX%.dll %openssl_root%\libssl-1_1%CKB_OPENSSL_SUFFIX%.dll
if exist %openssl_root%\libcrypto-1_1%CKB_OPENSSL_SUFFIX%.dll set CKF_OPENSSL_VERSION=1.1.x

REM OpenSSL 0.9.8, 1.0.x:
if exist %openssl_root%\out32dll\ssleay32.lib set lib=%lib%;%openssl_root%\out32dll
if exist %openssl_root%\out32dll\ssleay32.lib set CKF_SSL=yes
if exist %openssl_root%\out32dll\ssleay32.lib set CKF_OPENSSL_VERSION=0.9.8 or 1.0.x
if exist %openssl_root%\out32dll\ssleay32.lib echo Found OpenSSL 0.9.8 or 1.0.x: %openssl_root%\out32dll\ssleay32.lib
if exist %openssl_root%\out32dll\ssleay32.lib set CKF_SSL_LIBS=ssleay32.lib libeay32.lib
if exist %openssl_root%\out32dll\ssleay32.dll set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\out32dll\ssleay32.dll %openssl_root%\out32dll\libeay32.dll
if exist %openssl_root%\out32dll\openssl.exe set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\out32dll\openssl.exe
:nossl

REM libssh:
echo.
if "%CKF_SSH%" == "no" echo Skipping check for libssh
if "%CKF_SSH%" == "no" goto :nossh
set CKF_SSH=no
if exist %libssh_root%\include\NUL set include=%include%;%libssh_root%\include;%libssh_build%\include
if exist %libssh_build%\src\ssh.lib set lib=%lib%;%libssh_build%\src
if exist %libssh_build%\src\ssh.lib set CKF_SSH=yes
if exist %libssh_build%\src\ssh.lib echo Found libssh: %libssh_build%\src\ssh.lib
if exist %libssh_build%\src\ssh.dll set CK_SSH_DIST_DLLS=%libssh_build%\src\ssh.dll

if exist %libssh_build%\src\out\ssh.lib set lib=%lib%;%libssh_build%\src\out
if exist %libssh_build%\src\out\ssh.lib echo Found libssh: %libssh_build%\src\out\ssh.lib
if exist %libssh_build%\src\out\ssh.lib set CKF_SSH=yes
if exist %libssh_build%\src\out\ssh.lib set lib=%lib%;%libssh_build%\src\out
if exist %libssh_build%\src\out\ssh.dll set CK_SSH_DIST_DLLS=%libssh_build%\src\out\ssh.dll
if exist %libssh_build%\src\out\sshg.dll set CK_SSH_DIST_DLLS=%CK_SSH_DIST_DLLS% %libssh_build%\src\out\sshg.dll
if exist %libssh_build%\src\out\sshx.dll set CK_SSH_DIST_DLLS=%CK_SSH_DIST_DLLS% %libssh_build%\src\out\sshx.dll
if exist %libssh_build%\src\out\sshgx.dll set CK_SSH_DIST_DLLS=%CK_SSH_DIST_DLLS% %libssh_build%\src\out\sshgx.dll
if "%CKF_SSH%" == "no" echo Could not find libssh (%libssh_build%\src\ssh.lib or %libssh_build%\src\out\ssh.lib)
if "%CKF_SSH%" == "no" goto :nossh
:nossh

REM libdes:
echo.
if "%CKF_LIBDES%" == "no" echo Skipping check for libdes
if "%CKF_LIBDES%" == "no" goto :nolibdes
set CKF_LIBDES=no
if not exist %libdes_root%\des\des.h echo Could not find libdes (%libdes_root%\des\des.h)
if not exist %libdes_root%\des\des.h goto :nolibdes
echo Found libdes: %libdes_root%\des\des.h
set CKF_LIBDES=yes
set CKF_CRYPTDLL=yes
set INCLUDE=%include%;%libdes_root%
if exist %libdes_root%\Release\libdes.lib set lib=%lib%;%libdes_root%\Release\
if exist %libdes_root%\Debug\libdes.lib set lib=%lib%;%libdes_root%\Debug\
:nolibdes

REM Stanford SRP
echo.
if "%CKF_SSL%" == "no" echo Skipping check for SRP (OpenSSL is required but not available)
if "%CKF_SSL%" == "no" set CKF_SRP=no
if "%CKF_SSL%" == "no" goto :nosrp

if "%CKF_LIBDES%" == "no" echo Skipping check for SRP (libdes is required but not available)
if "%CKF_LIBDES%" == "no" set CKF_SRP=no
if "%CKF_LIBDES%" == "no" goto :nosrp

if "%CKF_SRP%" == "no" echo Skipping check for SRP
if "%CKF_SRP%" == "no" goto :nosrp
set CKF_SRP=no
if not exist %srp_root%\win32\libsrp_openssl\Release\srp.lib echo Stanford SRP not found (%srp_root%\win32\libsrp_openssl\Release\srp.lib)
if not exist %srp_root%\win32\libsrp_openssl\Release\srp.lib goto :nosrp
echo Found Stanford SRP
set CKF_SRP=yes
set INCLUDE=%INCLUDE%;%srp_root%\include
set LIB=%LIB%;%srp_root%\win32\libsrp_openssl\Release\
set CK_SRP_DIST_DLLS=%srp_root%\win32\libsrp_openssl\Release\srp.dll %srp_root%\win32\libsrp_openssl\Release\tconf.exe %srp_root%\win32\libkrypto_openssl\Release\krypto.dll
:nosrp

REM Kerberos for Windows
echo.
set CKF_K4W_WSHELPER=no
set CKF_K4W_KRB4=no
if "%CKF_K4W%" == "no" echo Skipping check for KFW
if "%CKF_K4W%" == "no" goto :nok4w
set CKF_K4W=no
if "%CKF_K4W_SSL%" == "" set CKF_K4W_SSL=no

REM Check kerberos compiled from source (ca 2.6)
set CK_KRB_INCLUDE=%k4w_root%\athena\auth\krb5\src\include
echo Searching for Kerberos: %CK_KRB_INCLUDE%\krb5.h
if exist "%CK_KRB_INCLUDE%\krb5.h" goto :havekerberos

REM Check kerberos SDK (ca 3.2.2)
set CK_KRB_INCLUDE=%k4w_root%\inc\krb5
echo Searching for Kerberos: %CK_KRB_INCLUDE%\krb5.h
if exist "%CK_KRB_INCLUDE%\krb5.h" goto :havekerberos

REM Check KFW 4.x SDK
set CK_KRB_INCLUDE=%k4w_root%\include
echo Searching for Kerberos: %CK_KRB_INCLUDE%\krb5.h
if exist "%CK_KRB_INCLUDE%\krb5.h" goto :havekerberos

REM No Kerberos :(
echo Kerberos for Windows not found.
echo Kerberos root is:
dir /B %k4w_root%
goto :nok4w

:havekerberos
echo Found Kerberos for Windows (KFW)
echo Include: %CK_KRB_INCLUDE%
set CKF_K4W=yes
set INCLUDE=%INCLUDE%;%CK_KRB_INCLUDE%
if "%CKF_OPENSSL_VERSION%" neq "0.9.8 or 1.0.x" set CKF_K4W_SSL=unsupported

echo Checking for Kerberos IV support...
if exist "%CK_KRB_INCLUDE%\KerberosIV\krb.h" set CKF_K4W_KRB4=yes
if exist "%CK_KRB_INCLUDE%\KerberosIV\krb.h" echo Found Kerberos IV support.

REM This will only work when KFW is built from source (wshload isn't included
REM in any of the SDKs)
echo Searching for Kerberos wshload (for DNS-SRV support)...
if not exist "%k4w_root%\target\lib\i386\rel\wshload.lib" goto :nowshload
echo Found wshload, enabling DNS-SRV
set CKF_K4W_WSHELPER=yes
set lib=%lib%;%k4w_root%\target\lib\i386\rel
set INCLUDE=%INCLUDE%;%k4w_root%\athena\wshelper\include
:nowshload

REM Dist the kerberos binaries if we're building against a from-source thing.
REM if we're building against an SDK then the user can just use the associated
REm installer to get this stuff on their system.
set K4WBINS=%k4w_root%\target\bin\i386\rel
set CK_K4W_DIST_FILES="%K4WBINS%\comerr32.dll" "%K4WBINS%\gssapi32.dll" "%K4WBINS%\k524init.exe" "%K4WBINS%\kclnt32.dll"
set CK_K4W_DIST_FILES=%CK_K4W_DIST_FILES% "%K4WBINS%\klist.exe" "%K4WBINS%\krb524.dll" "%K4WBINS%\krb5_32.dll"
set CK_K4W_DIST_FILES=%CK_K4W_DIST_FILES% "%K4WBINS%\krbv4w32.dll" "%K4WBINS%\leash32.exe" "%K4WBINS%\leash32.hlp"
set CK_K4W_DIST_FILES=%CK_K4W_DIST_FILES% "%K4WBINS%\leashw32.dll" "%K4WBINS%\ms2mit.exe" "%K4WBINS%\wshelp32.dll"
set CK_K4W_DIST_FILES=%CK_K4W_DIST_FILES% "%K4WBINS%\kdestroy.exe" "%K4WBINS%\kinit.exe" "%K4WBINS%\krbcc32s.exe"
set CK_K4W_DIST_FILES=%CK_K4W_DIST_FILES% "%K4WBINS%\krbcc32.dll" "%K4WBINS%\leash32.chm" "%K4WBINS%\xpprof32.dll"
REM TODO: also need the MFC DLLs
for %%I in (%CK_K4W_DIST_FILES%) do set CK_K4W_DIST=%CK_K4W_DIST% %%I
:nok4w


REM --------------------------------------------------------------
REM Detect compiler so the OpenZinc build environment can be setup 
REM --------------------------------------------------------------

echo.
echo Attempting to identify compiler (this may take a moment)...

REM Now figure out what compiler we're using - we need to find this out so we'll
REM know where to look for OpenZinc and if we're able to build it if it can't be
REM found (not all supported compilers have OpenZinc makefiles available)
set CK_COMPILER_NAME=unknown
set ZINCBUILD=
set CKF_ZINC=no
set BUILD_ZINC=no
set CKB_9X_COMPATIBLE=no
set CKB_NT_COMPATIBLE=no
set CKB_XP_COMPATIBLE=no

REM We can't look at OpenWatcoms help output for a version number because it
REM waits for input ("Press any key to continue:"), so we'll just detect it by
REM the presence of its environment variables.
if exist %WATCOM%\binnt\wcl386.exe goto :watcomc
cl 2>&1 | findstr /C:"Version 19.4" > nul
if %errorlevel% == 0 goto :vc144
cl 2>&1 | findstr /C:"Version 19.3" > nul
if %errorlevel% == 0 goto :vc143
cl 2>&1 | findstr /C:"Version 19.2" > nul
if %errorlevel% == 0 goto :vc142
cl 2>&1 | findstr /C:"Version 19.1" > nul
if %errorlevel% == 0 goto :vc141
cl 2>&1 | findstr /C:"Version 19.0" > nul
if %errorlevel% == 0 goto :vc140
cl 2>&1 | findstr /C:"Version 18.0" > nul
if %errorlevel% == 0 goto :vc12
cl 2>&1 | findstr /C:"Version 17.0" > nul
if %errorlevel% == 0 goto :vc11
cl 2>&1 | findstr /C:"Version 16.0" > nul
if %errorlevel% == 0 goto :vc10
cl 2>&1 | findstr /C:"Version 15.0" > nul
if %errorlevel% == 0 goto :vc9
cl 2>&1 | findstr /C:"Version 14.0" > nul
if %errorlevel% == 0 goto :vc8
cl 2>&1 | findstr /C:"Version 13.1" > nul
if %errorlevel% == 0 goto :vc71
cl 2>&1 | findstr /C:"Version 13.0" > nul
if %errorlevel% == 0 goto :vc7
cl 2>&1 | findstr /C:"Version 12.0" > nul
if %errorlevel% == 0 goto :vc6
cl 2>&1 | findstr /C:"Version 11.0" > nul
if %errorlevel% == 0 goto :vc5
cl 2>&1 | findstr /C:"Version 10.2" > nul
if %errorlevel% == 0 goto :vc42
cl 2>&1 | findstr /C:"Version 10.1" > nul
if %errorlevel% == 0 goto :vc41
cl 2>&1 | findstr /C:"Version 10.0" > nul
if %errorlevel% == 0 goto :vc4
cl 2>&1 | findstr /C:"Version 9.1" > nul
if %errorlevel% == 0 goto :vc21
cl 2>&1 | findstr /C:"Version 9.0" > nul
if %errorlevel% == 0 goto :vc2
cl 2>&1 | findstr /R /C:"32-bit.*Version 8\.0" > nul
if %errorlevel% == 0 goto :vc1
cl 2>&1 | findstr /R /C:"AXP.*Version 8\.0" > nul
if %errorlevel% == 0 goto :vc1axp
cl 2>&1 | findstr /R /C:"C Centaur.*Version 8\.00" > nul
if %errorlevel% == 0 goto :vc1mips
cl 2>&1 | findstr /C:"Version 8.00" > nul
if %errorlevel% == 0 goto :vc116

goto :unsupported

:watcomc
REM TODO - ideally we should try and detect the version of OpenWatcom - at least 1.9 vs 2.0
set CK_COMPILER_NAME=OpenWatcom
set ZINCBUILD=ow19
set CKF_SSH=unsupported
set CKF_SSL=unsupported
set CKF_LIBDES=unsupported
set CKF_K4W=unsupported
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes

REM For openwatcom we have to use its nmake clone
set MAKE=nmake

REM OpenWatcom doesn't include TAPI headers to we bundle them with K95. Add them to the include
REM path so the dialer can find them.
set include=%include%;%root%\kermit\k95\ow
goto :cvcdone

:vc116
set CK_COMPILER_NAME=Visual C++ 1.0 (16-bit)
set ZINCBUILD=mvcpp150
set CKF_SUPERLAT=unsupported
set CKF_SSH=unsupported
set CKF_SSL=unsupported
set CKF_LIBDES=unsupported
set CKF_K4W=unsupported
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
goto :semisupported

:vc1
set CK_COMPILER_NAME=Visual C++ 1.0 32-bit Edition
set CKF_SSH=unsupported
set CKF_SSL=unsupported
set CKF_LIBDES=unsupported
set CKF_CRYPTDLL=no
set CKF_K4W=unsupported
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc1axp
REM This is in the NT 3.50 Win32 SDK. Doesn't include Visual C++ libs/runtime (msvcrt)
set CK_COMPILER_NAME=Visual C++ 1.0 for Alpha AXP
set CKF_SSH=unsupported
set CKF_SSL=unsupported
set CKF_LIBDES=unsupported
set CKF_CRYPTDLL=no
set CKF_K4W=unsupported
set CKB_NT_COMPATIBLE=yes

REM As this compiler doesn't include msvcrt...
set CKB_STATIC_CRT=yes

goto :cvcdone

:vc1mips
REM This is in the Win32 SDK Final Release (NT 3.1) compiler.
REM Doesn't include Visual C++ libs/runtime (msvcrt)
set CK_COMPILER_NAME=Win32 SDK Final Release (MIPS) Centaur C 8.0
set CKF_SSH=unsupported
set CKF_SSL=unsupported
set CKF_LIBDES=unsupported
set CKF_CRYPTDLL=no
set CKF_K4W=unsupported
set CKB_STATIC_CRT=yes
set CKB_NT_COMPATIBLE=yes
goto :cvcdone


:vc2
:vc21
set CK_COMPILER_NAME=Visual C++ 2.x
REM TODO - try to find msvcrt20.dll and add it to distdlls
set ZINCBUILD=mvcpp200mt
set CKF_SSH=unsupported
set CKF_SSL=unsupported
set CKF_LIBDES=unsupported
set CKF_CRYPTDLL=no
set CKF_K4W=unsupported
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes

if "%CKB_TARGET_ARCH%" == "ALPHA" set ZINCBUILD=mvcpp400mt-alpha
if "%CKB_TARGET_ARCH%" == "MIPS" set ZINCBUILD=mvcpp400mt-mips

goto :cvcdone

:vc4
:vc41
:vc42
set CK_COMPILER_NAME=Visual C++ 4.x
set ZINCBUILD=mvcpp400mt
set CKF_SSH=unsupported
set CKF_SSL=unsupported
set CKF_LIBDES=unsupported
set CKF_CRYPTDLL=no
set CKF_K4W=unsupported
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes

if "%CKB_TARGET_ARCH%" == "PPC" set ZINCBUILD=mvcpp400mt-ppc
if "%CKB_TARGET_ARCH%" == "ALPHA" set ZINCBUILD=mvcpp400mt-alpha
if "%CKB_TARGET_ARCH%" == "MIPS" set ZINCBUILD=mvcpp400mt-mips

goto :cvcdone

:vc5
set CK_COMPILER_NAME=Visual C++ 5.0 (Visual Studio 97)
set ZINCBUILD=mvcpp500mt
set CKF_SSH=unsupported
set CKF_SSL=unsupported
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes

if "%CKB_TARGET_ARCH%" == "ALPHA" set ZINCBUILD=mvcpp500mt-alpha

goto :cvcdone

:vc6
set CK_COMPILER_NAME=Visual C++ 6.0 (Visual Studio 6)
set ZINCBUILD=mvcpp600mt
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes

if "%CKB_TARGET_ARCH%" == "ALPHA" set ZINCBUILD=mvcpp600mt-alpha

goto :cvcdone

:vc7
set CK_COMPILER_NAME=Visual C++ 2002 (7.0)
set ZINCBUILD=mvcpp700mt
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes

REM libdes won't build for Alpha64
if "%CKB_TARGET_ARCH%" == "ALPHA64" set CKF_LIBDES=unsupported
if "%CKB_TARGET_ARCH%" == "ALPHA64" set CKF_CRYPTDLL=no

goto :cvcdone

:vc71
set CK_COMPILER_NAME=Visual C++ 2003 (7.1)
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc8
set CK_COMPILER_NAME=Visual C++ 2005 (8.0)
set CKB_9X_COMPATIBLE=yes
set CKB_NT_COMPATIBLE=yes
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc9
set CK_COMPILER_NAME=Visual C++ 2008 (9.0)
set CKF_SUPERLAT=unsupported
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc10
set CK_COMPILER_NAME=Visual C++ 2010 (10.0)
set ZINCBUILD=mvcpp10
set CKF_SUPERLAT=unsupported
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc11
set CK_COMPILER_NAME=Visual C++ 2012 (11.0)
set CKF_SUPERLAT=unsupported
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc12
set CK_COMPILER_NAME=Visual C++ 2013 (12.0)
set CKF_SUPERLAT=unsupported
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc140
set CK_COMPILER_NAME=Visual C++ 2015 (14.0)
set CKF_SUPERLAT=unsupported
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc141
set CK_COMPILER_NAME=Visual C++ 2017 (14.1)
set CKF_SUPERLAT=unsupported
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

:vc142
set CK_COMPILER_NAME=Visual C++ 2019 (14.2)
set CKF_SUPERLAT=unsupported
set CKB_XP_COMPATIBLE=yes
goto :cvcdone

REM Visual C++ 2022 and newer are not suitable for targeting XP.

:vc143
set CK_COMPILER_NAME=Visual C++ 2022 (14.3)
set CKF_SUPERLAT=unsupported
goto :cvcdone

:vc144
set CK_COMPILER_NAME=Visual C++ 2022 17.10+ (14.4)
set CKF_SUPERLAT=unsupported
goto :cvcdone

REM Meridian SuperLAT requires Visual C++ 2005 or older as the product itself
REM requires Windows NT 3.5x or 4.0 (2000 and newer are unsupported)

:unsupported
echo.
echo -- Unsupported compiler: %CK_COMPILER_NAME% --
echo Kermit 95 has not been tested with this compiler and may not build.
echo.
goto :cvcend

:semisupported
REM This compiler is only supported for building k95cinit.exe, a utility which
REM requires Zinc for Windows 3.x

set CK_K95CINIT=yes

goto :check_zinc16

:cvcdone
REM Compiler detection finished. If Zinc is supported for this compiler,
REM go set it up.
echo Compiler: %CK_COMPILER_NAME%
if "%ZINCBUILD%" == "" echo Can not setup Zinc for this compiler
if "%ZINCBUILD%" NEQ "" goto :check_zinc

goto :cvcend

:check_zinc
REM Zinc is supported for this compiler - check to see if we have it.
echo Checking for OpenZinc in %root%\zinc\lib\%ZINCBUILD%\

set ck_zinc_lib=%root%\zinc\lib\%ZINCBUILD%
set ck_zinc_include=%root%\zinc\include
set lib=%lib%;%ck_zinc_lib%
set include=%include%;%ck_zinc_include%

REM Then check to see if its already built.
set CK_HAVE_ZINC_OS2=no
set CK_HAVE_ZINC_NT=no
if exist %root%\zinc\lib\%ZINCBUILD%\os2_zil.lib set CK_HAVE_ZINC_OS2=yes
if exist %root%\zinc\lib\%ZINCBUILD%\wnt_zil.lib set CK_HAVE_ZINC_NT=yes

if "%CK_HAVE_ZINC_OS2%" == "yes" goto :have_zinc
if "%CK_HAVE_ZINC_NT%" == "yes" goto :have_zinc

echo No zinc binaries found. Checking for sources...
if exist %root%\zinc\INCLUDE\ui_win.h goto :have_zinc_src

echo OpenZinc source code not found at %root%\zinc. Extract the OpenZinc
echo distribution in this location for building the dialer.

:have_zinc_src
REM It is not built, but it can be!
set BUILD_ZINC=yes
goto :cvcend

:check_zinc16
REM 16bit Zinc is supported for this compiler so add it to the include and lib path
set lib=%lib%;%root%\zinc\lib\%ZINCBUILD%
set include=%include%;%root%\zinc\include

REM Then check to see if its already built.
if exist %root%\zinc\lib\%ZINCBUILD%\win_zil.lib goto :have_zinc

REM It is not built, but it can be!
set BUILD_ZINC=yes
goto :cvcend

:have_zinc
REM Looks like we've got a suitable compiled copy of OpenZinc.
set CKF_ZINC=yes
set BUILD_ZINC=no
if "%CK_HAVE_ZINC_OS2%" == "yes" echo OpenZinc for OS/2 found!
if "%CK_HAVE_ZINC_NT%" == "yes" echo OpenZinc for Win32 found!

goto :cvcend

:cvcend

if "%CK_K95CINIT%" == "yes" goto :build_k95cinit

REM TODO - if we're using an old compiler, force things like SSH off
REM        and remove their dist files.

set CK_DIST_DLLS=%CK_ZLIB_DIST_DLLS% %CK_SSL_DIST_DLLS% %CK_SSH_DIST_DLLS% %CK_SRP_DIST_DLLS% %CK_K4W_DIST_FILES%

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
echo Dist files set to:
echo    %CK_DIST_DLLS%
echo.
echo Optional Dependencies:
echo     zlib: %CKF_ZLIB%
echo  OpenSSL: %CKF_SSL% (Version: %CKF_OPENSSL_VERSION%)
echo   libssh: %CKF_SSH%
echo     zinc: %CKF_ZINC%
echo   libdes: %CKF_LIBDES%
echo SuperLAT: %CKF_SUPERLAT%
echo      SRP: %CKF_SRP%
echo Kerberos: %CKF_K4W% (Kerberos+SSL: %CKF_K4W_SSL%, DNS-SRV: %CKF_K4W_WSHELPER%, KRB4: %CKF_K4W_KRB4%)
echo.
if "%BUILD_ZINC%" == "yes" echo OpenZinc is required for building the dialer. You can build it by extracting
if "%BUILD_ZINC%" == "yes" echo the OpenZinc distribution to %root%\zinc and running
if "%BUILD_ZINC%" == "yes" echo %root%\mkzinc.bat
if "%BUILD_ZINC%" == "yes" echo.
goto :end

:build_k95cinit
echo.
echo Your compiler is: %CK_COMPILER_NAME%
echo This compiler is only supported for building the k95cinit.exe utility.

cd %root%\kermit\dialer\init
if "%BUILD_ZINC%" == "yes" goto :build_zinc_k95cinit

echo You can build that now by running mk.bat
echo.

goto :end

:build_zinc_k95cinit
echo Zinc is required to build this utility. You can build Zinc now by running
echo %root%\mkzinc.bat
echo.
echo Once Zinc has been built for your compiler, you can build k95cinit.exe by
echo running mk.bat in %root%\kermit\dialer\init\
echo.

:end