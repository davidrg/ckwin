@echo off
REM For cross-compiling C-Kermit for OS/2 from Windows using Open Watcom.
REM     --> DEBUG BUILD <--
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
set INCLUDE=%WATCOM%\H;%WATCOM%\H\OS2;%CKINCLUDE%

if not exist os2\NUL mkdir os2
move os2\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /f ckoker.mak PLATFORM=OS2 wcos2d
move *.obj os2  > nul

REM Restore old include and lib paths.
set INCLUDE=%OLDINCLUDE%
