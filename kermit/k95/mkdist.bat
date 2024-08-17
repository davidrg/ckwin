@echo off
set local
@echo === Make Distribution ===

@echo Create directories...
if not exist dist\NUL mkdir dist
if not exist dist\docs\NUL mkdir dist\docs
if not exist dist\docs\manual\NUL mkdir dist\docs\manual
REM if not exist dist\icons mkdir dist\icons
REM if not exist dist\fonts mkdir dist\fonts

REM These directories would normally go in: C:\ProgramData\Kermit 95
REM if not exist dist\certs\NUL mkdir dist\certs
REM if not exist dist\crls\NUL mkdir dist\crls
if not exist dist\keymaps\NUL mkdir dist\keymaps
REM if not exist dist\phones\NUL mkdir dist\phones
REM if not exist dist\printer\NUL mkdir dist\printer
if not exist dist\public\NUL mkdir dist\public
if not exist dist\scripts\NUL mkdir dist\scripts
REM if not exist dist\ssh\NUL mkdir dist\ssh
if not exist dist\users\NUL mkdir dist\users

@echo Move build outputs...
move *.exe dist
move k95ssh*.dll dist
if exist k95crypt.dll move k95crypt.dll dist
copy *.manifest dist
copy iksd.ksc dist\iksd.ksc.sample
ren dist\cknker.exe k95.exe
ren dist\cknker.exe.manifest k95.exe.manifest
del dist\cknker.exe.manifest
REM del dist\ctl3dins.exe   -- this can trip up virus scanners but its required by the dialer
move dist\ckwart.exe .\

if "%CKF_XYZ%" == "no" goto :nop
if exist ..\p95\p95.dll copy ..\p95\p95.dll dist\
:nop

@echo Copy resources...
copy k95d.cfg dist

REM The following would go in "C:\ProgramData\Kermit 95" if installed:
copy k95.ini dist
copy k95custom.ini dist\k95custom.sample
copy welcome.txt dist
copy hostmode.bat dist
REM (k95custom.sample should be renamed to k95custom.ini upon installation)

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

@echo Copy manual...
copy ..\..\doc\manual\ckwin.htm dist\docs\manual\
copy hostmode.txt dist\docs\
if exist dist\ssh.dll copy ..\..\doc\ssh-readme.md dist\ssh-readme.txt

REM --- The following belongs in "C:\ProgramData\Kermit 95" if "installed" ---

REM CERTS directory
REM Empty directory in K-95

REM CRLS directory
REM Empty directory in K-95

REM KEYMAPS directory
@echo Copy keymaps...
set CK_DIST_KEYMAPS=vt220.ksc keycodes.txt
REM These also used to be distributed but aren't very useful anymore:
REM       emacs.ksc keypad.ksc sni.ksc  wp50.ksc wp51.ksc
REM They *were* available at:
REM   ftp://kermit.columbia.edu/pub/kermit/archives/k95keymaps.zip
REM Now archived at:
REM   https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu-2/pub/kermit/archives/k95keymaps.zip

REM These files are only useful on Windows 95 so don't bother including them on builds
REM that can't run there:
REM         capslock.ksc ctrl2cap.txt ctrl2cap.vxd
REM They're also available at:
REm   https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu/pub/kermit/k95/
if "%CKB_9X_COMPATIBLE%" == "yes" set CK_DIST_KEYMAPS=%CK_DIST_KEYMAPS% capslock.ksc

for %%I in (%CK_DIST_KEYMAPS%) do copy %%I dist\keymaps\
copy keymaps-readme.txt dist\keymaps\readme.txt

REM Generate the default keymap. This will fail if we're cross-compiling for an
REM architecture incompatible with this machine so skip it in that case.
if "%CKB_CROSS_COMPATIBLE%" == "no" goto :skipkm
cd dist
k95.exe -Y -# 127 -C "save keymap keymaps/default.ksc,exit" > NUL:
cd ..
:skipkm

REM PHONES
REM Contains dialing directories. All the files previously distributed here are
REM completely obsolete - none of the dial-in BBS still exist 20+ years later.

REM PRINTER directory
REM originally contained:
REM     pcaprint.sh, pcprint.com, pcprint.man, pcprint.sh
REM         Utilities for printing from a unix or VMS host to a local printer
REM         via K95
REM     textps.txt
REM         Documentation for the textps utility
REM     readme.txt
REM         Document describing the contents of this directory

REM PUBLIC directory
@echo Copy public files...
set CK_DIST_PUBLIC=hostuser.txt
for %%I in (%CK_DIST_PUBLIC%) do copy %%I dist\public\

REM SCRIPTS directory
@echo Copy scripts...
set CK_DIST_SCRIPTS=apage.ksc autotel.ksc iksdpy.ksc login.ksc host.ksc hostcom.ksc hostmdm.ksc hostmode.ksc hosttcp.ksc
set CK_DIST_SCRIPTS=%CK_DIST_SCRIPTS% npage.ksc recover.ksc review.ksc rgrep.ksc host.cfg
for %%I in (%CK_DIST_SCRIPTS%) do copy %%I dist\scripts\
copy scripts-readme.txt dist\scripts\readme.txt

REM SSH directory
REM Empty directory in K-95, location not used by the new K95 SSH subsystem

REM USERS directory
@echo Copy User files...
copy hostmode-greeting.txt dist\users\greeting.txt
copy hostmode-help.txt dist\users\hostmode.txt