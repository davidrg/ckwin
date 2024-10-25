@echo off
setlocal

set RUN=N
set SUFFIX=
if [%1] == [Y] set RUN=Y
if [%2] NEQ [] set SUFFIX=-%2

@echo ==== Build CKNKER.EXE and associated utilities ====
SET PLATFORM=NT
SET K95BUILD=K95
set OUTDIR=nt%SUFFIX%
if not exist %OUTDIR%\NUL mkdir %OUTDIR%
move %OUTDIR%\*.obj . > nul
if exist ckcmai.obj del ckcmai.obj
if exist ckuus5.obj del ckuus5.obj
%MAKE% /nologo /e /f ckoker.mak msvc
move *.obj %OUTDIR%  > nul

REM Open Watcom 1.9s nmake clone doesn't seem to set errorlevel when the build
REM fails. And VC5 nmake often seems to complete successfully but report an
REM error of 1 even though we're not supplying the /K flag. So we'll check the
REM expected outputs were produced to detect failure instead:
if not exist cknker.exe goto :missingoutputs
if not exist iksd.exe goto :missingoutputs
if not exist iksdsvc.exe goto :missingoutputs
if not exist k95d.exe goto :missingoutputs
if not exist rlogin.exe goto :missingoutputs
if not exist telnet.exe goto :missingoutputs
if not exist textps.exe goto :missingoutputs
if not exist ctl3dins.exe goto :missingoutputs

if [%RUN%] == [Y] start cknker.exe

goto :end

:missingoutputs
echo.
echo.
echo ERROR: One or more outputs were not generated. Got:
dir /w /b *.exe
echo Expected: cknker.exe, iksd.exe, iksdsvc.exe, k95d.exe, rlogin.exe,
echo           telnet.exe, testps.exe, ctl3dins.exe
echo.
echo.
exit /B 1

:end