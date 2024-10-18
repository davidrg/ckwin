@echo off
REM For cross-compiling P2 for OS/2 from Windows using Open Watcom.
REM
REM You should have already run owsetenv.bat to at a minimum to do the
REM following:
REM     SET WATCOM=C:\WATCOM
REM     SET PATH=%WATCOM%\BINW;%WATCOM%\BINNT;%PATH%
REM And also modified and run setenv.bat to set at least %CKINCLUDE%
REM
REM On windows owsetenv.bat only sets up the include and library paths for
REM targeting 32bit Windows. So we'll temporarily replace these with with
REM values suitable for cross-compiling OS/2 applications and restore them
REM at the end

set OLDINCLUDE=%INCLUDE%
set INCLUDE=%CKINCLUDE%;%WATCOM%\H;%WATCOM%\H\OS2;

echo %INCLUDE%

set OLDLIB=%LIB%
set LIB=%WATCOM%\lib386\os2\;%WATCOM%\lib386

%MAKE% /nologo /f p2.mak PLATFORM=OS2

REM Restore old include and lib paths.
set INCLUDE=%OLDINCLUDE%
set LIB=%OLDLIB%


REM Open Watcom 1.9s nmake clone doesn't seem to set errorlevel when the build
REM fails. So we'll check the expected outputs were generated and if not report
REM the error ourselves.
if not exist p2.dll goto :missingoutputs

echo Build succeeded

goto :end

:missingoutputs
echo.
echo.
echo ERROR: One or more outputs were not generated. Got:
dir /w /b *.exe *.dll
echo Expected: p2.dll
echo.
echo.
exit /B 1

:end