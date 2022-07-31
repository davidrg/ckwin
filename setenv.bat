@echo off

REM Uncomment this and set it to where your source code lives
REM set root=C:\src

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
set zlib_root=%root%\zlib\1.2.12

REM openssl - set to the folder where the include folder and .lib and .dll files
REM live.
set openssl_root=%root%\openssl\1.1.1q

REM libssh - set to the folder where the include directory lives.
set libssh_root=%root%\libssh\0.9.6

REM libssh - set to where you built libssh. This should have a subdirectory
REM called 'src' containing ssh.dll and ssh.lib
set libssh_build=%libssh_root%\build

REM ================== No changes required beyond this point ===================

REM base include path
set include=%include%;%root%\kermit\k95
set include=%include%;%root%\kermit\k95\kui

REM Add on any optional dependencies

set CKF_ZLIB=no
set CKF_SSL=no
set CKF_SSH=no

set CK_DIST_DLLS=

if not defined vcpkg_installed goto :end

REM Look for dependencies in the vcpkg folders

set include=%include%;%vcpkg_installed%\include
set lib=%lib%;%vcpkg_installed%\lib

if exist %vcpkg_installed%\lib\zlib.lib set CKF_ZLIB=yes
REM Not currently required -
REM     if exist %vcpkg_installed%\bin\zlib1.dll set CK_DIST_DLLS=%CK_DIST_DLLS% %vcpkg_installed%\bin\zlib1.dll

if exist %vcpkg_installed%\lib\libssl.lib set CKF_SSL=yes
REM Not currently required -
REM     if exist %vcpkg_installed%\bin\zlib1.dll set CK_DIST_DLLS=%CK_DIST_DLLS% %vcpkg_installed%\bin\libssl3.dll

if exist %vcpkg_installed%\lib\ssh.lib set CKF_SSH=yes
if exist %vcpkg_installed%\bin\ssh.dll set CK_DIST_DLLS=%CK_DIST_DLLS% %vcpkg_installed%\bin\ssh.dll %vcpkg_installed%\bin\pthreadVC3.dll

:end

REM Look for optional dependencies

REM zlib:
if exist %zlib_root%\zlib.h set include=%include%;%zlib_root%
if exist %zlib_root%\zlib.lib set lib=%lib%;%zlib_root%
if exist %zlib_root%\zlib.lib set CKF_ZLIB=yes
REM Not currently required -
REM     if exist %zlib_root%\zlib1.dll set CK_DIST_DLLS=%CK_DIST_DLLS% %zlib_root%\zlib1.dll

REM OpenSSL:
if exist %openssl_root%\include\NUL set include=%include%;%openssl_root%\include
if exist %openssl_root%\libssl.lib set lib=%lib%;%openssl_root%
if exist %openssl_root%\libssl.lib set CKF_SSL=yes
if exist %openssl_root%\libcrypto-1_1.dll set CK_DIST_DLLS=%CK_DIST_DLLS% %openssl_root%\libcrypto-1_1.dll
REM libcrypto is only needed for libssh

REM libssh:
if exist %libssh_root%\include\NUL set include=%include%;%libssh_root%\include;%libssh_build%\include
if exist %libssh_build%\src\ssh.lib set lib=%lib%;%libssh_build%\src
if exist %libssh_build%\src\ssh.lib set CKF_SSH=yes
if exist %libssh_build%\src\ssh.dll set CK_DIST_DLLS=%CK_DIST_DLLS% %libssh_build%\src\ssh.dll

echo Include path set to:
echo    %include%
echo.
echo Library path set to:
echo    %lib%
echo.
echo Optional Dependencies Found:
echo    zlib: %CKF_ZLIB%
echo OpenSSL: %CKF_SSL%
echo  libssh: %CKF_SSH%