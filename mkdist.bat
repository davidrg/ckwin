@echo off
if exist kermit\k95\dist\k95.exe goto :mkdist 

echo Running build...
call mk.bat

:mkdist
echo Collecting build artifcats...
if exist dist\NUL rmdir /S /Q dist
move kermit\k95\dist .\

if "%CKF_ZINC%" == "no" goto :nozinc
cd dist
move ..\kermit\dialer\dist\* .\
cd ..

:nozinc

:zipdist
REM make distribution zip if we can
if not exist %infozip_root%\zip.exe goto :nozip

cd dist
echo Making distribution archive K95PACK.ZIP
%infozip_root%\zip.exe -r -5 -v -o k95pack * -x setup.exe

if exist %infozip_root%\%CKB_TARGET_ARCH%\unzipsfx.exe goto :exedist
echo Could not find %CKB_TARGET_ARCH% version of Info-Zip unzipsfx.exe at %infozip_root%\%CKB_TARGET_ARCH%\unzipsfx.exe
echo Skipping creation of EXE distribution archive
cd ..
goto :zipdone

:exedist
REM This is for use with setup.exe, CD-ROM or Network install
copy /b %infozip_root%\%CKB_TARGET_ARCH%\unzipsfx.exe + k95pack.zip K2PACK.EXE
cd ..

goto :zipdone

:nozip
echo Could not find Info-Zip zip.exe at %infozip_root%\zip.exe
echo Skipping creation of distribution archive



:zipdone

echo Done.