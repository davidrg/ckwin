@echo off
REM For cross-compiling the C-Kermit for OS/2 Dialer from Windows using
REM Open Watcom.
REM
REM You should have already run owsetenv.bat to at a minimum to do the
REM following:
REM     SET WATCOM=C:\WATCOM
REM     SET PATH=%WATCOM%\BINNT;%WATCOM%\BINW;%PATH%
REM And also modified and run setenv.bat to set at least %CKINCLUDE%
REM
if "%CK_HAVE_ZINC_OS2%" == "no" goto :nozinc

REM The owsetenv.bat sets up the include paths for targeting host system.
REM So we'll temporarily replace these with with values suitable for 
REM cross-compiling OS/2 applications and restore them at the end

set OLDINCLUDE=%INCLUDE%
set INCLUDE=%WATCOM%\H;%WATCOM%\H\OS2;%CKINCLUDE%;%ck_zinc_include%

set OLDLIB=%LIB%
set LIB=%ck_zinc_lib%

nmake -f k95dial.mak PLATFORM=OS2 os2

REM Restore old include and lib paths.
set INCLUDE=%OLDINCLUDE%
set LIB=%OLDLIB%

REM because Open Watcom 1.9s nmake clone doesn't seem to set errorlevel when the
REM build fails.
if not exist k2dial.exe goto :missingoutputs

goto :end

:missingoutputs
echo.
echo.
echo ERROR: k2dial.exe was not generated. Got:
dir /w /b *.exe
echo Expected: k2dial.exe
echo.
echo.
exit /B 1

:nozinc
echo.
echo OpenZinc has not been configured in the C-Kermit for OS/2 build environment!
echo This is normally done by setenv.bat when it detects a build of OpenZinc for your
echo current compiler. Download the OpenZinc OS/2 package (OZ1os2.zip) or build
echo OS/2 OpenZinc for your compiler.
echo.
echo To force a build anyway, run:
echo    nmake -f k95dial.mak PLATFORM=OS2 os2
echo.
exit /B 1

:end
