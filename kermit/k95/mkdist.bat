@echo off
set local
@echo === Make Distribution ===

@echo Create directories...
REM These should normally go in: %PROGRAMFILES%\Kermit 95
if not exist dist\NUL mkdir dist
if not exist dist\docs\NUL mkdir dist\docs
if not exist dist\docs\manual\NUL mkdir dist\docs\manual
REM if not exist dist\icons mkdir dist\icons
REM if not exist dist\fonts mkdir dist\fonts

REM These directories would normally go in: %ALLUSERSPROFILE%\Kermit 95
REM if not exist dist\certs\NUL mkdir dist\certs
REM if not exist dist\crls\NUL mkdir dist\crls
if not exist dist\keymaps\NUL mkdir dist\keymaps
REM if not exist dist\phones\NUL mkdir dist\phones
if not exist dist\printer\NUL mkdir dist\printer
if not exist dist\public\NUL mkdir dist\public
if not exist dist\scripts\NUL mkdir dist\scripts
REM if not exist dist\ssh\NUL mkdir dist\ssh
if not exist dist\users\NUL mkdir dist\users

@echo Move build outputs...
REM All of this should go in: %PROGRAMFILES%\Kermit 95
move *.exe dist
if exist *.pdb move *.pdb dist
if exist dist\nullssh.pdb del dist\nullssh.pdb
if exist k95ssh*.dll move k95ssh*.dll dist
if exist k95crypt.dll move k95crypt.dll dist
copy *.manifest dist
copy iksd.ksc dist\iksd.ksc.sample
ren dist\cknker.exe k95.exe
if exist dist\cknker.pdb ren dist\cknker.pdb k95.pdb
ren dist\cknker.exe.manifest k95.exe.manifest
if exist dist\cknker.exe.manifest del dist\cknker.exe.manifest
REM del dist\ctl3dins.exe   -- this can trip up virus scanners but its required by the dialer
move dist\ckwart.exe .\
move dist\telnet-old.* .\
move dist\rlogin-old.* .\
if "%CKF_SSH%" == "no" move dist\ssh.exe .\

if "%CKF_XYZ%" == "no" goto :nop
if exist ..\p95\p95.dll copy ..\p95\p95.dll dist\
:nop

@echo Copy resources...
copy k95d.cfg dist

REM ----------------------------------------------------------
REM The following should go in: %ALLUSERSPROFILE%\Kermit 95
copy k95.ini dist
copy k95custom.ini dist\k95custom.sample
copy welcome.txt dist
copy hostmode.bat dist
REM (k95custom.sample should be renamed to k95custom.ini upon installation)
REM k95site.ini should also be included once an installer exists
REM    And perhaps we should update it to copy %ALLUSERSPROFILE%\k95custom.ini
REM    to %APPDATA%\k95custom.ini if it doesn't already exist.
REM ca_certs.pem and ca_certs.license
REM dialinf.dat when we have that (this is the dialer templates, etc)
REM ----------------------------------------------------------

@echo Copy Open Watcom DLL run-time libraries
if "%WATCOM%"=="" goto :noowrtdll
if "%CKB_STATIC_CRT_NT%"=="yes" goto :noowrtdll
copy %WATCOM%\binnt\mt7r*.dll dist
copy %WATCOM%\binnt\clbr*.dll dist
copy %WATCOM%\binnt\plbr*.dll dist
:noowrtdll

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

if exist dist\regina.dll copy %rexx_root%\COPYING-LIB dist\COPYING.regina
if exist dist\regina.dll copy %rexx_root%\doc\*.pdf dist\docs\

if exist dist\rexxre.dll copy %rexxre_root%\rexxre.pdf dist\docs\

@echo Copy manual...
copy hostmode.txt dist\docs\
copy ..\..\doc\ctlseqs.html dist\docs\
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
echo Generate default.ksc...
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
@echo Copy printer files...
set CK_DIST_PRINTER=pcprint.sh pcprint.man pcprint.com pcaprint.sh textps.txt
REM TODO: readme.txt
for %%I in (%CK_DIST_PRINTER%) do copy %%I dist\printer\
copy printer-readme.txt dist\printer\readme.txt

REM PUBLIC directory
@echo Copy public files...
set CK_DIST_PUBLIC=hostuser.txt
REM TODO: iksd.txt
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

REM Install layout on Windows NT
REM $(user)\AppData\Kermit 95\:
REM     Certs\ - empty
REM     Crls\ - empty
REM     Keymaps\ - empty
REM     Phones\ - empty
REM     Scripts\ - empty
REM     Ssh\ - empty
REM     Dialusr.dat
REM All Users\AppData\Kermit 95\:
REM     Certs\ - empty
REM     Crls\ - empty
REM     Keymaps\  -- dist\keymaps
REM     phones\  -- now empty, previously had phone and network directories for services that no longer exist
REM     printer\  -- dist\printer
REM     public\  -- dist\public     (hostuser.txt, iksd.txt)
REM     scripts\  -- dist\scripts
REM     users\  -- dist\users
REM     cacerts.pem
REM     dialinf.dat
REM     hostmode.bat
REM     k95.ini
REM     k95custom.ini
REM     k95site.ini