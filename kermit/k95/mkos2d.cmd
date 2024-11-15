@echo off
REM For compiling C-Kermit for OS/2 on an OS/2 host using Open Watcom.
REM     --> DEBUG BUILD <--
REM
REM You should have already run owsetenv.cmd to at a minimum to do the
REM following:
REM     SET WATCOM=C:\WATCOM
REM     SET PATH=%WATCOM%\BINP;%WATCOM%\BINW;%PATH%
REM And also modified and run setenv.cmd to add \kermit\k95 to the include path

mkdir os2
if exist os2\*.obj move os2\*.obj . > nul
if exist ckcmai.obj del ckcmai.obj
if exist ckuus5.obj del ckuus5.obj
wmake -h -ms -f ckoker.mak PLATFORM=OS2 MAKE="wmake -h -ms" wcos2d
move *.obj os2  > nul