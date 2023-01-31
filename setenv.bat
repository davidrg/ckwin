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
set openssl_root=%root%\openssl\1.1.1s

REM libssh - set to the folder where the include directory lives.
set libssh_root=%root%\libssh\0.10.3

REM libssh - set to where you built libssh. This should have a subdirectory
REM called 'src' containing ssh.dll and ssh.lib
set libssh_build=%libssh_root%\build

REM libdes - required for building k95crypt.dll and enabling a few features that
REm rely on obsolete and easily broken encryption
set libdes_root=%root%\libdes

REM Stanford SRP - Optional SRP Authentication for Telnet and FTP connections
REM Extract srp-2.1.2.tar.gz to the following location:
set srp_root=%root%\srp

REM Kerberos for Windows, support for which probably hasn't been built since 2002
set k4w_root=%root%\kerberos\kfw-2.2-beta-2

REM ============================================================================
REM ================== No changes required beyond this point ===================
REM ============================================================================

echo Searching for Optional Dependencies...

REM base include path - this is required for both Windows and OS/2
set ckinclude=%root%\kermit\k95

REM See if the user has the SuperLAT SDK (*extremely* unlikely)
set CKF_SUPERLAT=no
if exist %root%\superlat\include\latioc.h set CKF_SUPERLAT=yes
if exist %root%\superlat\include\latioc.h set ckinclude=%ckinclude%;%root%\superlat\include

REM This and everything else is windows-specific.
set ckwinclude=%ckinclude%;%root%\kermit\k95\kui

REM Set include path for targeting Windows.
set include=%include%;%ckwinclude%

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
if exist %vcpkg_installed%\bin\libcrypto-3.dll set CK_SSL_DIST_DLLS=%vcpkg_installed%\bin\libcrypto-3.dll %vcpkg_installed%\bin\libssl-3.dll
if exist %vcpkg_installed%\bin\libcrypto-1_1.dll set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\libcrypto-1_1.dll %openssl_root%\libssl-1_1.dll
if exist %vcpkg_installed%\tools\openssl\openssl.exe set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\tools\openssl\openssl.exe
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
if "%CKF_ZLIB%" == "no" echo Skipping check for ZLIB
if "%CKF_ZLIB%" == "no" goto :nozlib
set CKF_ZLIB=no
if exist %zlib_root%\zlib.h set include=%include%;%zlib_root%
if exist %zlib_root%\zlib.lib set lib=%lib%;%zlib_root%
if exist %zlib_root%\zlib.lib set CKF_ZLIB=yes
if exist %zlib_root%\zlib.lib echo Found zlib
if exist %zlib_root%\zlib1.dll set CK_ZLIB_DIST_DLLS=%zlib_root%\zlib1.dll
:nozlib

REM OpenSSL
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
if exist %openssl_root%\libssl.lib echo Found OpenSSL 1.1.x or 3.0.x
if exist %openssl_root%\libssl.lib set CKF_SSL_LIBS=libssl.lib libcrypto.lib
if exist %openssl_root%\apps\openssl.exe set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\apps\openssl.exe

REM OpenSSL 3.0.x
if exist %openssl_root%\libcrypto-3.dll set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\libcrypto-3.dll %openssl_root%\libssl-3.dll

REM OpenSSL 1.1.x
if exist %openssl_root%\libcrypto-1_1.dll set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\libcrypto-1_1.dll %openssl_root%\libssl-1_1.dll

REM OpenSSL 0.9.8, 1.0.x:
if exist %openssl_root%\out32dll\ssleay32.lib set lib=%lib%;%openssl_root%\out32dll
if exist %openssl_root%\out32dll\ssleay32.lib set CKF_SSL=yes
if exist %openssl_root%\out32dll\ssleay32.lib echo Found OpenSSL 0.9.8 or 1.0.x
if exist %openssl_root%\out32dll\ssleay32.lib set CKF_SSL_LIBS=ssleay32.lib libeay32.lib
if exist %openssl_root%\out32dll\ssleay32.dll set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\out32dll\ssleay32.dll %openssl_root%\out32dll\libeay32.dll
if exist %openssl_root%\out32dll\openssl.exe set CK_SSL_DIST_DLLS=%CK_SSL_DIST_DLLS% %openssl_root%\out32dll\openssl.exe
:nossl

REM libssh:
if "%CKF_SSH%" == "no" echo Skipping check for libssh
if "%CKF_SSH%" == "no" goto :nossh
set CKF_SSH=no
if exist %libssh_root%\include\NUL set include=%include%;%libssh_root%\include;%libssh_build%\include
if exist %libssh_build%\src\ssh.lib set lib=%lib%;%libssh_build%\src
if exist %libssh_build%\src\ssh.lib set CKF_SSH=yes
if exist %libssh_build%\src\ssh.lib echo Found libssh
if exist %libssh_build%\src\ssh.dll set CK_SSH_DIST_DLLS=%libssh_build%\src\ssh.dll
:nossh

REM libdes:
if "%CKF_LIBDES%" == "no" echo Skipping check for libdes
if "%CKF_LIBDES%" == "no" goto :nolibdes
set CKF_LIBDES=no
if not exist %libdes_root%\des\des.h goto :nolibdes
echo Found libdes
set CKF_LIBDES=yes
set CKF_CRYPTDLL=yes
set INCLUDE=%include%;%libdes_root%
if exist %libdes_root%\Release\libdes.lib set lib=%lib%;%libdes_root%\Release\
if exist %libdes_root%\Debug\libdes.lib set lib=%lib%;%libdes_root%\Debug\
:nolibdes

REM Stanford SRP
if "%CKF_LIBDES%" == "no" echo Skipping check for SRP (libdes is required but not available)
if "%CKF_LIBDES%" == "no" goto :nosrp
if "%CKF_SRP%" == "no" echo Skipping check for SRP
if "%CKF_SRP%" == "no" goto :nosrp
set CKF_SRP=no
if not exist %srp_root%\win32\libsrp_openssl\Release\srp.lib echo Stanford SRP not found.
if not exist %srp_root%\win32\libsrp_openssl\Release\srp.lib goto :nosrp
echo Found Stanford SRP
set CKF_SRP=yes
set INCLUDE=%INCLUDE%;%srp_root%\include
set LIB=%LIB%;%srp_root%\win32\libsrp_openssl\Release\
set CK_SRP_DIST_DLLS=%srp_root%\win32\libsrp_openssl\Release\srp.dll %srp_root%\win32\libsrp_openssl\Release\tconf.exe %srp_root%\win32\libkrypto_openssl\Release\krypto.dll
:nosrp

REM Kerberos for Windows
if "%CKF_K4W%" == "no" echo Skipping check for K4W
if "%CKF_K4W%" == "no" goto :nok4w
set CKF_K4W=no
if not exist %k4w_root%\target\lib\i386\rel\wshload.lib echo Kerberos for Windows (K4W) not found.
if not exist %k4w_root%\target\lib\i386\rel\wshload.lib goto :nok4w
echo Found Kerberos for Windows (K4W)
set CKF_K4W=yes
set INCLUDE=$INCLUDE%;%k4w_root%\athena\wshelper\include
set INCLUDE=$INCLUDE%;%k4w_root%\athena\auth\krb5\src\include
set lib=%lib%;%k4w_root%\target\lib\i386\rel
:nok4w


REM --------------------------------------------------------------
REM Detect compiler so the OpenZinc build environment can be setup 
REM --------------------------------------------------------------

echo Attempting to identify compiler (this may take a moment)...

REM Now figure out what compiler we're using - we need to find this out so we'll
REM know where to look for OpenZinc and if we're able to build it if it can't be
REM found (not all supported compilers have OpenZinc makefiles available)
set CK_COMPILER_NAME=unknown
set ZINCBUILD=
set CKF_ZINC=no
set BUILD_ZINC=no

REM We can't look at OpenWatcoms help output for a version number because it
REM waits for input ("Press any key to continue:"), so we'll just detect it by
REM the presence of its environment variables.
if exist %WATCOM%\binnt\wcl386.exe goto :watcomc
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
cl 2>&1 | findstr /C:"Version 10.0" > nul
if %errorlevel% == 0 goto :vc4
cl 2>&1 | findstr /C:"Version 9.0" > nul
if %errorlevel% == 0 goto :vc2
cl 2>&1 | findstr /R /C:"32-bit.*Version 8\.0" > nul
if %errorlevel% == 0 goto :vc1
cl 2>&1 | findstr /C:"Version 8.00" > nul
if %errorlevel% == 0 goto :vc116

goto :unsupported

:watcomc
REM TODO - ideally we should try and detect the version of OpenWatcom - at least 1.9 vs 2.0
set CK_COMPILER_NAME=OpenWatcom
set ZINCBUILD=ow19

REM OpenWatcom doesn't include TAPI headers to we bundle them with CKW. Add them to the include
REM path so the dialer can find them.
set include=%include%;%root%\kermit\k95\ow
goto :cvcdone

:vc116
set CK_COMPILER_NAME=Visual C++ 1.0 (16-bit)
set ZINCBUILD=mvcpp150
set CKF_SUPERLAT=unsupported
set CKF_SSH=unsupported
set CKF_SSL=unsupported
goto :semisupported

:vc1
set CK_COMPILER_NAME=Visual C++ 1.0 32-bit Edition
goto :unsupported

:vc2
set CK_COMPILER_NAME=Visual C++ 2.0
REM TODO - try to find msvcrt20.dll and add it to distdlls
set ZINCBUILD=mvcpp200mt
set CKF_SSH=unsupported
set CKF_SSL=unsupported
goto :cvcdone

:vc4
set CK_COMPILER_NAME=Visual C++ 4.0
set ZINCBUILD=mvcpp400mt
set CKF_SSH=unsupported
set CKF_SSL=unsupported
goto :cvcdone

:vc5
set CK_COMPILER_NAME=Visual C++ 5.0 (Visual Studio 97)
set ZINCBUILD=mvcpp500mt
set CKF_SSH=unsupported
set CKF_SSL=unsupported
goto :cvcdone

:vc6
set CK_COMPILER_NAME=Visual C++ 6.0 (Visual Studio 6)
set ZINCBUILD=mvcpp600mt
goto :cvcdone

:vc7
set CK_COMPILER_NAME=Visual C++ 2002 (7.0)
set ZINCBUILD=mvcpp700mt
goto :cvcdone

:vc71
set CK_COMPILER_NAME=Visual C++ 2003 (7.1)
goto :cvcdone

:vc8
set CK_COMPILER_NAME=Visual C++ 2005 (8.0)
goto :cvcdone

:vc9
set CK_COMPILER_NAME=Visual C++ 2008 (9.0)
set CKF_SUPERLAT=unsupported
goto :cvcdone

:vc10
set CK_COMPILER_NAME=Visual C++ 2010 (10.0)
set ZINCBUILD=mvcpp10
set CKF_SUPERLAT=unsupported
goto :cvcdone

:vc11
set CK_COMPILER_NAME=Visual C++ 2012 (11.0)
set CKF_SUPERLAT=unsupported
goto :cvcdone

:vc12
set CK_COMPILER_NAME=Visual C++ 2013 (12.0)
set CKF_SUPERLAT=unsupported
goto :cvcdone

:vc140
set CK_COMPILER_NAME=Visual C++ 2015 (14.0)
set CKF_SUPERLAT=unsupported
goto :cvcdone

:vc141
set CK_COMPILER_NAME=Visual C++ 2017 (14.1)
set CKF_SUPERLAT=unsupported
goto :cvcdone

:vc142
set CK_COMPILER_NAME=Visual C++ 2019 (14.2)
set CKF_SUPERLAT=unsupported
goto :cvcdone

:vc143
set CK_COMPILER_NAME=Visual C++ 2022 (14.3)
set CKF_SUPERLAT=unsupported
goto :cvcdone

REM Meridian SuperLAT requires Visual C++ 2005 or older as the product itself
REM requires Windows NT 3.5x or 4.0 (2000 and newer are unsupported)

:unsupported
echo.
echo -- Unsupported compiler: %CK_COMPILER_NAME% --
echo C-Kermit for Windows has not been tested with this compiler and may not build.
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
REM Zinc is supported for this compiler so add it to the include and lib path
set lib=%lib%;%root%\zinc\lib\%ZINCBUILD%
set include=%include%;%root%\zinc\include

REM Then check to see if its already built.
if exist %root%\zinc\lib\%ZINCBUILD%\wnt_zil.lib goto :have_zinc

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
echo OpenZinc found!
goto :cvcend

:cvcend

if "%CK_K95CINIT%" == "yes" goto :build_k95cinit

REM TODO - if we're using an old compiler, force things like SSH off
REM        and remove their dist files.

set CK_DIST_DLLS=%CK_ZLIB_DIST_DLLS% %CK_SSL_DIST_DLLS% %CK_SSH_DIST_DLLS% %CK_SRP_DIST_DLLS%

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
echo  OpenSSL: %CKF_SSL%
echo   libssh: %CKF_SSH%
echo     zinc: %CKF_ZINC%
echo   libdes: %CKF_LIBDES%
echo SuperLAT: %CKF_SUPERLAT%
echo      SRP: %CKF_SRP%
echo      K4W: %CKF_K4W%
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