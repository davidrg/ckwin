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
set DEV_MODE=false
set WEB_MODE=false
set HTTPS_MODE=true

for %%I in (switch valid OUT_DIR) do set "%%I="
for %%I in (%*) do (
    rem Value after a switch
    if defined switch (

        rem /O for Output Directory
        if /i "!switch:~1!"=="O" set "OUT_DIR=%%~I"

        rem /B for banner file
        if /i "!switch:~1!"=="B" set "BANNER_FILE=%%~I"

        set "switch="

    ) else (

        REM check if the current parameter is a switch (starts with / or -)
        echo(%%~I | >NUL findstr "^[-/]" && (
            set "switch=%%~I"
            REM G = no git-file-dates
            REM D = dev-mode
            REM W = web-mode
            REM S = use-https

            rem Check for a valid switch
            for %%x in (G D W I O B) do (
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
echo   /B=   Banner file to insert at the top of every HTML file. Only the first
echo         line will be read from the file.
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
echo Dev Mode: %DEV_MODE%
echo Web Mode: %WEB_MODE%
echo HTTPS Mode: %HTTPS_MODE%
echo Out Dir: %OUT_DIR%

set docs_root=%root%\doc

set BANNER_FILE=%BANNER_FILE:\=\\%
echo Banner File: %BANNER_FILE%

echo Disting to: %OUT_DIR%
echo Using: %dist_root%\k95.exe

set manual_dist_dir=%OUT_DIR%

REM Kermit requires the \ character to be escaped (\\ instead of \) so...
set manual_dist_dir=%manual_dist_dir:\=\\%

set mtime_file=%root%\doc\manual\modtime.csv
set mtime_file=%mtime_file:\=\\%

REM This required so that the mkdocs.rex script can be found
set REGINA_MACROS_OLD=%REGINA_MACROS%
set REGINA_MACROS=%docs_root%

pushd %dist_root%

REM -# 94 is the sum of:
REM    2 - Do not load optional network DLLs     +  These four are to skip
REM    4 - Do not load optional TAPI DLLs        +  loading features we don't
REM    8 - Do not load optional Kerberos DLLs    +  need to reduce startup time
REM   16 - Do not load optional zmodem DLLs      +
REM   64 - Use stdout for output instead of the console/terminal emulator

echo Disting the manual to %OUT_DIR%...


REM Copy manual to the output directory updating version numbers, etc, as we go
REM Parameters are: source-directory destination-directory, git-file-dates dry-run dev-mode web-mode use-https
k95.exe %docs_root%\mkdocs.ksc -Y -# 94 = %docs_root%\manual %OUT_DIR% %GIT_DATES% %DEV_MODE% %WEB_MODE% %HTTPS_MODE% %BANNER_FILE% || goto :failed

REM If building for the web, put the ctlseqs document in there so it ends up
REM on the website. Its not actually part of the manual though - for the normal
REM dist process it just ends up in the docs folder.
if "%WEB_MODE%" == "true" copy %docs_root%\ctlseqs.html %OUT_DIR%

echo manual done.
goto :finished

:failed
echo K95 Manual build failed with exit status: %errorlevel%
popd

set REGINA_MACROS=%REGINA_MACROS_OLD%
set REGINA_MACROS_OLD=

exit /B 1

:finished
popd

set REGINA_MACROS=%REGINA_MACROS_OLD%
set REGINA_MACROS_OLD=

:end