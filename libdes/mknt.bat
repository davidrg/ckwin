@echo off

%make% /f libdes.mak CFG="Release"

if "%CKF_LIBDES%" == "yes" goto :havelibdes
if exist %libdes_root%\des\des.h set CKF_LIBDES=yes
if exist %libdes_root%\des\des.h set CKF_CRYPTDLL=yes
if exist %libdes_root%\des\des.h set INCLUDE=%include%;%libdes_root%
:havelibdes

if "%CKF_LIBDES_BUILT%" == "yes" goto :libdesbuilt
if exist %libdes_root%\Release\libdes.lib set CKF_LIBDES_BUILT=yes
if exist %libdes_root%\Debug\libdes.lib set CKF_LIBDES_BUILT=yes
if exist %libdes_root%\Release\libdes.lib set lib=%lib%;%libdes_root%\Release\
if exist %libdes_root%\Debug\libdes.lib set lib=%lib%;%libdes_root%\Debug\
:libdesbuilt