@echo off
if exist %root%\dist\k95.exe set dist_root=%root%\dist
if exist %root%\dist\k95.exe goto :mkdocs

if exist %root%\kermit\k95\dist\k95.exe set dist_root=%root%\kermit\k95\dist
if exist %root%\kermit\k95\dist\k95.exe goto :mkdocs

echo No K95 dist found. Can't mkdocs!
goto :end

:mkdocs

set docs_root=%root%\doc

echo Disting to: %dist_root%\docs\
echo Using: %dist_root%\k95.exe

set manual_dist_dir=%dist_root%\docs\manual\

REM Kermit requires the \ character to be escaped (\\ instead of \) so...
set manual_dist_dir=%manual_dist_dir:\=\\%

set mtime_file=%root%\doc\modtime.csv
set mtime_file=%mtime_file:\=\\%

REM This required so that the mkdocs.rex script can be found
set REGINA_MACROS_OLD=%REGINA_MACROS%
set REGINA_MACROS=%docs_root%

pushd %dist_root%

echo Disting the manual...

REM Copy manual to the output directory updating version numbers, etc, as we go
REM Parameters are: source-directory destination-directory, git-file-dates dry-run dev-mode
k95.exe %docs_root%\mkdocs.ksc -Y -# 126 = %docs_root%\manual %dist_root%\docs\manual true false true

REM And update modified dates for anything that hasn't changed since the manual
REM was added to git
k95.exe -Y -H -# 126 -C ".manual_dir := %manual_dist_dir%,.modtime_file := %mtime_file%,echo Foo \m(manual_dir),rexx call setdates,exit"

echo manual done.
popd

set REGINA_MACROS=%REGINA_MACROS_OLD%
set REGINA_MACROS_OLD=

REM TODO: if we're doing this for dist rather than web:
REM       Rename index.html to k95manual.htm
REM       Replace all references to index.htm with k95manual.htm
REM         (eg, in fontsize.html)
REM       Rename manualindex.html to index.htm

:end
