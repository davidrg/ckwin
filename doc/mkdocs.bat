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

REM This required so that the mkdocs.rex script can be found
set REGINA_MACROS_OLD=%REGINA_MACROS%
set REGINA_MACROS=%docs_root%

pushd %dist_root%
dir *.dll

echo Disting the manual...
REM Parameters are: source-directory destination-directory, git-file-dates dry-run dev-mode
k95.exe %docs_root%\mkdocs.ksc -Y -# 126 = %docs_root%\manual %dist_root%\docs\manual true false true
echo manual done.
popd

set REGINA_MACROS=%REGINA_MACROS_OLD%
set REGINA_MACROS_OLD=

REM TODO: if we're doing this for dist rather than web:
REM       Rename index.html to k95manual.htm
REM       Replace all references to index.htm with k95manual.htm
REM         (eg, in fontsize.html)
REM       Rename manualindex.html to index.htm

REM TODO: Deal with the fact git doesn't preserve modified dates. Once the
REM       manual is built, any files that haven't been modified since the manual
REM       was added to git (mostly images/screenshots) will have a modified date
REM       of 24 July 2024. For these files, the correct (or at least a less
REM       wrong) modified date can be found in modtime.csv. We should update
REM       these files with timestamps from modtime.csv to preserve this
REM       information.

:end
