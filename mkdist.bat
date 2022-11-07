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
echo Done.