@echo off
REM For cross-compiling C-Kermit for OS/2 from Windows using Open Watcom.
REM
REM You should have already run owsetenv.bat to at a minimum to do the
REM following:
REM     SET WATCOM=C:\WATCOM
REM     SET PATH=%WATCOM%\BINNT;%WATCOM%\BINW;%PATH%
REM And also modified and run setenv.bat to set at least %CKINCLUDE%
REM
REM The owsetenv.bat sets up the include paths for targeting host system.
REM So we'll temporarily replace these with with values suitable for 
REM cross-compiling OS/2 applications and restore them at the end

set OLDINCLUDE=%INCLUDE%
set INCLUDE=%CKINCLUDE%;%WATCOM%\H;%WATCOM%\H\OS2;

echo %INCLUDE%

if not exist os2\NUL mkdir os2
move os2\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /f ckoker.mak PLATFORM=OS2 wcos2
move *.obj os2  > nul

REM Restore old include and lib paths.
set INCLUDE=%OLDINCLUDE%


REM Open Watcom 1.9s nmake clone doesn't seem to set errorlevel when the build
REM fails. So we'll check the expected outputs were generated and if not report
REM the error ourselves.
if not exist ckoclip.exe goto :missingoutputs
if not exist ckoker32.exe goto :missingoutputs
if not exist orlogin.exe goto :missingoutputs
if not exist osetup.exe goto :missingoutputs
if not exist otelnet.exe goto :missingoutputs
if not exist otextps.exe goto :missingoutputs

goto :end

:missingoutputs
echo.
echo.
echo ERROR: One or more outputs were not generated. Got:
dir /w /b *.exe
echo Expected: ckoclip.exe, ckoker32.exe, orlogin.exe, osetup.exe, otelnet.exe,
echo           otextps.exe
echo.
echo.
exit /B 1

:end