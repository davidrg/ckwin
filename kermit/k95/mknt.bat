@echo off
@echo ==== Build CKNKER.EXE and assocaited utilities ====
SET PLATFORM=NT
SET K95BUILD=K95
if not exist nt\NUL mkdir nt
move nt\*.obj . > nul
del ckcmai.obj ckuus5.obj
%MAKE% /nologo /e /f ckoker.mak msvc
move *.obj nt  > nul

REM OpenWatcom 1.9s nmake clone doesn't seem to set errorlevel when the build
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