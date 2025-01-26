@echo off

call clean-dist.bat
call mkos2.bat

:mkdist
echo Collecting build artifcats...
if exist dist\NUL rmdir /S /Q dist-os2
move kermit\k95\dist-os2 .\

if "%CKF_ZINC%" == "no" goto :nozinc
cd dist-os2
move ..\kermit\dialer\dist-os2\* .\
cd ..
goto :zipdist

echo ZINC is not available - distribution will not include the dialer!

:nozinc

:zipdist
REM make distribution zip
if not exist %infozip_root%\os2\unzipsfx.exe goto :nounzipsfx
if not exist %infozip_root%\zip.exe goto :nozip
cd dist-os2
echo Making full distribution archive K2PACK.EXE
REM This is for use with setup.exe, CD-ROM or Network install
%infozip_root%\zip.exe -r -5 -v -o k2pack * -x setup.exe
copy /b %infozip_root%\os2\unzipsfx.exe + k2pack.zip K2PACK.EXE

echo Making minimal distribution K2MINI.EXE
REM This is basically everything but the dialer. Its small enough to fit on a
REM single floppy disk alongside setup.exe
%infozip_root%\zip.exe -r -5 -v -o k2pack-mini k2.* k2clip.exe cko32*.dll p*.dll pcfonts.dll rlogin.exe telnetpm*.exe textps.exe docs download eas icons keymaps phones printer public scripts tmp users
copy /b %infozip_root%\os2\unzipsfx.exe + k2pack-mini.zip K2MINI.EXE

cd ..

goto :zipdone
:nozip
echo Could not find Info-Zip zip.exe at %infozip_root%\zip.exe
echo Skipping creation of OS/2 distribution archive

goto :zipdone
:nounzipsfx
echo Could not find OS/2 version of Info-Zip unzipsfx.exe at %infozip_root%\os2\unzipsfx.exe
echo Skipping creation of OS/2 distribution archive

:zipdone

echo Done.