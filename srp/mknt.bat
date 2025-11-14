@echo off
nmake /f srp.mak CFG="Release"
nmake /f tconf.mak CFG="Release"
nmake /f krypto.mak CFG="Release"

REM Not sure what in the normal SRP build process creates the include directory,
REM so we'll just populate it with the headers K95 needs.
mkdir include
cd include
copy ..\libsrp\t_pwd.h
copy ..\libsrp\cstr.h
copy ..\libsrp\t_client.h
copy ..\libsrp\t_server.h
copy ..\libsrp\t_sha.h
copy ..\libsrp\srp.h
copy ..\libsrp\srp_aux.h
copy ..\libkrypto\krypto.h
cd ..

REM Stanford SRP
if "%CKF_SRP%" == "yes" goto :havesrp
if not exist %srp_root%\win32\libsrp_openssl\Release\srp.lib goto :nosrp
echo Found Stanford SRP
set CKF_SRP=yes
set INCLUDE=%INCLUDE%;%srp_root%\include
set LIB=%LIB%;%srp_root%\win32\libsrp_openssl\Release\
set CK_SRP_DIST_DLLS=%srp_root%\win32\libsrp_openssl\Release\srp.dll %srp_root%\win32\libsrp_openssl\Release\tconf.exe %srp_root%\win32\libkrypto_openssl\Release\krypto.dll
set CK_DIST_DLLS=%CK_DIST_DLLS% %CK_SRP_DIST_DLLS%
goto :havesrp
:nosrp
echo Could not find srp.lib in %srp_root%\win32\libsrp_openssl\Release\
:havesrp
