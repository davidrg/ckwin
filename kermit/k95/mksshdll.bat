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
if exist %OUTDIR%\*.obj move %OUTDIR%\*.obj . > nul
%MAKE% /nologo /e /f ckoker.mak msvc-sshdll
if exist ckolssh.obj del ckolssh.obj
if exist ckolsshs.obj del ckolsshs.obj
move *.obj %OUTDIR%  > nul

REM Open Watcom 1.9s nmake clone doesn't seem to set errorlevel when the build
REM fails. And VC5 nmake often seems to complete successfully but report an
REM error of 1 even though we're not supplying the /K flag. So we'll check the
REM expected outputs were produced to detect failure instead:
if not exist k95ssh.dll goto :missingoutputs

if [%RUN%] == [Y] start cknker.exe

goto :end

:missingoutputs
echo.
echo.
echo ERROR: One or more outputs were not generated. Got:
dir /w /b *.exe
echo Expected: k95ssh.dll
echo.
echo.
exit /B 1

:end