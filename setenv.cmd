@echo off
REM C-Kermit for OS/2 Build Environment Script

REM Uncomment this and set it to where your source code lives
REM set root=C:\src

REM ================== No changes required beyond this point ===================

set include=%include%;%root%\kermit\k95

cd %root%\kermit\k95

echo C-Kermit for OS/2 Build Environment
echo.
echo Include path set to:
echo    %include%
echo.
echo Run mkos2 to build
