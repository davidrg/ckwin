@echo off
@echo === Make Distribution (OS/2) ===

@echo Create directories...
if not exist dist-os2\NUL mkdir dist-os2
if not exist dist-os2\docs\NUL mkdir dist-os2\docs
if not exist dist-os2\docs\manual\NUL mkdir dist-os2\docs\manual

@echo Move build outputs...
move *.exe dist-os2
move cko32*.dll dist-os2
move dist-os2\ckwart.exe .\
move dist-os2\osetup.exe .\
ren dist-os2\ckoker32.exe k2.exe
ren dist-os2\otelnet.exe telnet.exe
ren dist-os2\otextps.exe textps.exe
ren dist-os2\orlogin.exe rlogin.exe
ren dist-os2\ckoclip.exe k2clip.exe

@echo Copy manual...
copy ..\..\doc\manual\ckos2.htm dist-os2\docs\manual\
if exist dist\ssh.dll copy ..\..\doc\ssh-readme.md dist-os2\ssh-readme.txt

@echo Copy resources...
copy k2.ini dist-os2

REM OS/2 builds with OpenWatcom don't appear to depend on any Watcom runtime
REM libraries (ckoker32.exe runs fine on a Warp 3 VM thats never had OpenWatcom
REM installed)

REM @echo Copy enabled optional dependencies
REM for %%I in (%CK_DIST_DLLS%) do copy %%I dist-os2\

@echo Copy licenses
copy ..\..\COPYING dist-os2
if exist dist-os2\ssh.dll copy %libssh_root%\COPYING dist-os2\COPYING.libssh
if not exist dist-os2\openssl.exe goto :nossl
REM OpenSSL License was renamed in 3.0.0 to LICENSE.txt
if exist %openssl_root%\LICENSE.txt copy %openssl_root%\LICENSE.txt dist-os2\COPYING.openssl
if exist %openssl_root%\LICENSE copy %openssl_root%\LICENSE dist-os2\COPYING.openssl
:nossl