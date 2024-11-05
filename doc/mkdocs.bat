@echo off
REM
REM  Run with --help (or /?) for usage information
REM
setlocal enabledelayedexpansion

if [%1] == [/?] goto :help
if [%1] == [-?] goto :help
if [%1] == [--help] goto :help

REM Defaults: use git dates, build for K95 distribution (not web) with https
REM           links.
set GIT_DATES=true
set DRY_RUN=false
set DEV_MODE=false
set WEB_MODE=false
set HTTPS_MODE=true

for %%I in (switch valid OUT_DIR) do set "%%I="
for %%I in (%*) do (
    rem Value after a switch
    if defined switch (

        rem Example: /X
        if /i "!switch:~1!"=="O" set "OUT_DIR=%%~I"

        set "switch="

    ) else (

        REM check if the current parameter is a switch (starts with / or -)
        echo(%%~I | >NUL findstr "^[-/]" && (
            set "switch=%%~I"
            REM G = no git-file-dates
            REM N = dry-run
            REM D = dev-mode
            REM W = web-mode
            REM S = use-https

            rem Check for a valid switch
            for %%x in (G N D W I O) do (
                if /i "!switch:~1!"=="%%x" set "valid=true"
            )

            if not defined valid (
                echo Unrecognized switch: %%~I
                goto help
            )

            set "valid="

            if /i "!switch:~1!"=="G" (
               set "GIT_DATES=false"
               set "switch="
            )

            if /i "!switch:~1!"=="N" (
               set "DRY_RUN=true"
               set "switch="
            )

            if /i "!switch:~1!"=="D" (
               set "DEV_MODE=true"
               set "switch="
            )

            if /i "!switch:~1!"=="W" (
               set "WEB_MODE=true"
               set "switch="
            )

            if /i "!switch:~1!"=="I" (
               set "HTTPS_MODE=false"
               set "switch="
            )
        )
    )
)

goto :begin

:help
echo K95 mkdocs script
echo This script must be run from inside the build environment (run setenv.bat
echo first), and you must have already built the Kermit 95 distribution using
echo either the mkdist.bat script in the source root, or the one in \kermit\k95.
echo The distribution must be built for a compatible architecture (you can't run
echo mkdocs.bat on an x86 machine if you built K95 for ARM64), and it needs to
echo be built with REXX support. The rexxre and regutil external function
echo libraries must also be built and avaialble.
echo.
echo Options:
echo   /G    Don't get file dates from git. Timestamps will be wrong.
echo   /N    Dry run - don't actually do anything
echo   /D    Dev mode - release tag will be "DEV" rather than "BETA.7" or
echo         nothing, indicating this isn't an final/release version of the
echo         manual.
echo   /W    Build for the web. HTML files will end in .html, and the K95 main
echo         page will be index.html. If not specified, HTML files will end in
echo         .htm and index.html will be the K95 documentation index
echo   /I    Use HTTP:// links to kermitproject.org and select other sites
echo         instead of HTTPS:// - this should only be used without the /W
echo         switch when building for vintage platforms that are unlikely to
echo         have an up-to-date web browser.
echo   /O=   Set output directory
goto :end

:begin

if exist %root%\dist\k95.exe set dist_root=%root%\dist
if exist %root%\dist\k95.exe goto :mkdocs

if exist %root%\kermit\k95\dist\k95.exe set dist_root=%root%\kermit\k95\dist
if exist %root%\kermit\k95\dist\k95.exe goto :mkdocs

echo No K95 dist found. Can't mkdocs!
goto :end

:mkdocs

if "%OUT_DIR%" == "" set OUT_DIR=%dist_root%\docs\manual\

echo Parameters:
echo Git File dates: %GIT_DATES%
echo Dry Run: %DRY_RUN%
echo Dev Mode: %DEV_MODE%
echo Web Mode: %WEB_MODE%
echo HTTPS Mode: %HTTPS_MODE%
echo Out Dir: %OUT_DIR%

set docs_root=%root%\doc

echo Disting to: %dist_root%\docs\
echo Using: %dist_root%\k95.exe

set manual_dist_dir=%OUT_DIR%

REM Kermit requires the \ character to be escaped (\\ instead of \) so...
set manual_dist_dir=%manual_dist_dir:\=\\%

set mtime_file=%root%\doc\modtime.csv
set mtime_file=%mtime_file:\=\\%

REM This required so that the mkdocs.rex script can be found
set REGINA_MACROS_OLD=%REGINA_MACROS%
set REGINA_MACROS=%docs_root%

pushd %dist_root%

echo Disting the manual...

k95.exe -Y -# 127 -C "save keymap %manual_dist_dir%default.ksc,exit" > NUL:

REM Copy manual to the output directory updating version numbers, etc, as we go
REM Parameters are: source-directory destination-directory, git-file-dates dry-run dev-mode web-mode use-https
k95.exe %docs_root%\mkdocs.ksc -Y -d -# 126 = %docs_root%\manual %OUT_DIR% %GIT_DATES% %DRY_RUN% %DEV_MODE% %WEB_MODE% %HTTPS_MODE%

REM And update modified dates for anything that hasn't changed since the manual
REM was added to git
if "%DRY_RUN%" == "true" goto :skipfd
k95.exe -Y -H -# 126 -C ".manual_dir := %manual_dist_dir%,.modtime_file := %mtime_file%,rexx call setdates,exit"
:skipfd

echo manual done.
popd

set REGINA_MACROS=%REGINA_MACROS_OLD%
set REGINA_MACROS_OLD=

:end
