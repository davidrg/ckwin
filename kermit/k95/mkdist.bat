@echo off
@echo === Make Distribution ===

@echo Create directories...
if not exist dist\NUL mkdir dist
if not exist dist\docs\NUL mkdir dist\docs
if not exist dist\docs\manual\NUL mkdir dist\docs\manual

@echo Move build outputs...
move *.exe dist
copy *.manifest dist
ren dist\cknker.exe k95.exe
ren dist\cknker.exe.manifest k95.exe.manifest
del dist\cknker.exe.manifest
del dist\ctl3dins.exe
move dist\ckwart.exe .\

@echo Copy manual...
copy ..\..\doc\manual\ckwin.htm dist\docs\manual\
if exist dist\ssh.dll copy ..\..\doc\ssh-readme.md dist\ssh-readme.txt

if "%CKF_XYZ%" == "no" goto :nop
if exist ..\p95\p95.dll copy ..\p95\p95.dll dist\
:nop

@echo Copy resources...
copy k95.ini dist
copy k95custom.ini dist
copy k95d.cfg dist
copy welcome.txt dist

@echo Copy runtime libraries
if defined WATCOM copy %WATCOM%\binnt\mt7r*.dll dist
if defined WATCOM copy %WATCOM%\binnt\clbr*.dll dist
if defined WATCOM copy %WATCOM%\binnt\plbr*.dll dist

@echo Copy enabled optional dependencies
for %%I in (%CK_DIST_DLLS%) do copy %%I dist\

@echo Copy licenses
copy ..\..\COPYING dist
if exist dist\ssh.dll copy %libssh_root%\COPYING dist\COPYING.libssh
if not exist dist\openssl.exe goto :nossl
REM OpenSSL License was renamed in 3.0.0 to LICENSE.txt
if exist %openssl_root%\LICENSE.txt copy %openssl_root%\LICENSE.txt dist\COPYING.openssl
if exist %openssl_root%\LICENSE copy %openssl_root%\LICENSE dist\COPYING.openssl
:nossl