@echo off
setlocal

set RUN=N
set SUFFIX=
if [%1] == [Y] set RUN=Y
if [%2] NEQ [] set SUFFIX=-%2

@echo ==== Build K95G.EXE ====
SET KERMITDIR=..
SET OUTDIR=.\win95%SUFFIX%
SET NODEBUG=1
SET PLATFORM=NT
cd kui
%MAKE% /e /f makefile || exit /B %errorlevel%
cd ..
REM UNSET NODEBUG
SET OUTDIR=.\kui\win95%SUFFIX%
SET K95BUILD=K95
move kui\win95%SUFFIX%\ck*.obj . > nul
move kui\win95%SUFFIX%\p_*.obj . > nul
del ckcmai.obj ckuus5.obj
%MAKE% /nologo /e /f ckoker.mak k95gd
move *.obj kui\win95%SUFFIX% > nul

REM Open Watcom 1.9s nmake clone doesn't seem to set errorlevel when the build
REM fails. So we'll check the expected outputs were produced too:
if not exist k95g.exe goto :missingoutputs

if [%RUN%] == [Y] start k95g.exe

goto :end

:missingoutputs
echo.
echo.
echo ERROR: One or more outputs were not generated. Got:
dir /w /b *.exe
echo Expected: k95g.exe
echo.
echo.
set errorlevel=1

:end



