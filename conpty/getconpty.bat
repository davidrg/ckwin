@echo off

REM Microsoft-signed ConPTY binaries are available from nuget. That means we
REM need Nuget in order to fetch them

if "%NUGET_URL%" == "" set NUGET_URL=https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
if "%CONPTY_VERSION%" == "" set CONPTY_VERSION=1.24.260512001

where nuget.exe >nul 2>nul
if %errorlevel% equ 0 (
    echo Found Nuget
    goto :getpkg
)

echo Nuget not found
where curl.exe >nul 2>nul
if %errorlevel% equ 0 (
    REM but we have url! Fetch nuget
    echo Downloading nuget with curl...
    curl --output nuget.exe %NUGET_URL%
    if exist nuget.exe goto :getpkg
)

echo Curl not found. Download:
echo    https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
echo And place it in:
echo    %root%\conpty
echo Then re-run this script
goto :end

:getpkg
echo Getting package...
nuget install Microsoft.Windows.Console.ConPTY -version %CONPTY_VERSION% -OutputDirectory %root%\conpty
echo Done!

if exist Microsoft.Windows.Console.ConPTY.%CONPTY_VERSION% (
    if exist Microsoft.Windows.Console.ConPTY rmdir /S /Q Microsoft.Windows.Console.ConPTY
    ren Microsoft.Windows.Console.ConPTY.%CONPTY_VERSION%  Microsoft.Windows.Console.ConPTY
)

if not "%CK_DIST_DLLS%"=="%CK_DIST_DLLS:conpty.dll=%" (
    echo DIST DLLs already up-to-date.
    goto :end
)

set CONPTY_ARCH=none
if "%CKB_TARGET_ARCH%" == "AMD64" set CONPTY_ARCH=x64
if "%CKB_TARGET_ARCH%" == "ARM64" set CONPTY_ARCH=arm64
if "%CKB_TARGET_ARCH%" == "x86" set CONPTY_ARCH=x86
if "%CONPTY_ARCH%" == "none" goto :end

set CK_DIST_DLLS=%CK_DIST_DLLS% %root%\conpty\Microsoft.Windows.Console.ConPTY\runtimes\win-%CONPTY_ARCH%\native\conpty.dll
set CK_DIST_DLLS=%CK_DIST_DLLS% %root%\conpty\Microsoft.Windows.Console.ConPTY\build\native\runtimes\%CONPTY_ARCH%\OpenConsole.exe

echo DIST DLLs updated:
echo %CK_DIST_DLLS%

:end
set CONPTY_VERSION=
set NUGET_URL=
set CONPTY_ARCH=