@echo off
REM For compiling the C-Kermit Dialer for OS/2 on an OS/2 host using Open Watcom.
REM
REM You should have already run owsetenv.cmd to at a minimum to do the
REM following:
REM     SET WATCOM=C:\WATCOM
REM     SET PATH=%WATCOM%\BINW;%WATCOM%\BINP;%PATH%
REM And also modified and run setenv.cmd to add \kermit\k95 to the include path
REM
REM Note that OpenZinc is *required* for building the dialer. If you don't
REM already have this, grab the OpenZinc full distribution (OZ1.zip) and
REM extract it to ..\..\zinc

wmake -h -ms -f k95dial.mak PLATFORM=OS2 MAKE="wmake -h -ms" os2
