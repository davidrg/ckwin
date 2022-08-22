@echo off
REM For compiling C-Kermit for OS/2 on an OS/2 host using OpenWatcom.
REM
REM You should have already run owsetenv.cmd to at a minimum to do the
REM following:
REM     SET WATCOM=C:\WATCOM
REM     SET PATH=%WATCOM%\BINW;%WATCOM%\BINP;%PATH%
REM And also modified and run setenv.cmd to add \kermit\k95 to the include path

del ckcmai.obj ckuus5.obj
wmake -h -ms -f ckoker.mak PLATFORM=OS2 MAKE="wmake -h -ms" wcos2