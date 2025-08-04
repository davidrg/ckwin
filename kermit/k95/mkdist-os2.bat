@echo off
@echo === Make Distribution (OS/2) ===

if exist dist-os2\NUL rmdir /S /Q dist-os2

@echo Create directories...
if not exist dist-os2\NUL mkdir dist-os2
if not exist dist-os2\DOCS\NUL mkdir dist-os2\DOCS
if not exist dist-os2\DOCS\manual\NUL mkdir dist-os2\DOCS\manual
if not exist dist-os2\DOWNLOAD\NUL mkdir dist-os2\DOWNLOAD
REM TODO: Kermit 95 2.1.2 had an EAS directory - what is it for? Is it still
REM       needed?
if not exist dist-os2\EAS\NUL mkdir dist-os2\EAS
if not exist dist-os2\ICONS\NUL mkdir dist-os2\ICONS
if not exist dist-os2\KEYMAPS\NUL mkdir dist-os2\KEYMAPS
if not exist dist-os2\PHONES\NUL mkdir dist-os2\PHONES
if not exist dist-os2\PRINTER\NUL mkdir dist-os2\PRINTER
if not exist dist-os2\PUBLIC\NUL mkdir dist-os2\PUBLIC
if not exist dist-os2\SCRIPTS\NUL mkdir dist-os2\SCRIPTS
if not exist dist-os2\TMP\NUL mkdir dist-os2\TMP
if not exist dist-os2\USERS\NUL mkdir dist-os2\USERS

@echo Move build outputs...
move *.exe dist-os2
move cko32*.dll dist-os2
move pcfonts.dll dist-os2
move nullssh.dll dist-os2
move dist-os2\ckwart.exe .\
ren dist-os2\osetup.exe setup.exe
ren dist-os2\ckoker32.exe k2.exe
ren dist-os2\otelnet.exe telnet.exe
ren dist-os2\otextps.exe textps.exe
ren dist-os2\orlogin.exe rlogin.exe
ren dist-os2\ckoclip.exe k2clip.exe
copy dist-os2\telnet.exe dist-os2\telnetpm.exe

if "%CKF_XYZ%" == "no" goto :nop
if exist ..\p95\p2.dll copy ..\p95\p2.dll dist-os2\
:nop

@echo Copy manual...
copy ..\..\DOCS\manual\ckos2.htm dist-os2\DOCS\manual\
if exist dist-os2\ssh.dll copy ..\..\doc\ssh-readme.md dist-os2\ssh-readme.txt

@echo Copy resources...
copy k2.ini dist-os2

if "%WATCOM%"=="" goto :nortdll
if "%CKB_STATIC_CRT_OS2%"=="yes" goto :noowrtdll
@echo Copy Open Watcom DLL run-time libraries
copy %WATCOM%\binp\dll\mt7r*.dll dist-os2
copy %WATCOM%\binp\dll\clbr*.dll dist-os2
copy %WATCOM%\binp\dll\plbr*.dll dist-os2
:noowrtdll

REN DOCS download
REM TODO: DOCS\readme.txt

REM DOWNLOAD directory
copy download-readme.txt dist-os2\DOWNLOAD\readme.txt

REM EAS directory
REM k2.eas, k2dial.eas, k2reg.eas  - Extended Attributes. Made with eautil.

REM ICONS directory
@echo Copy icons...
copy k95f_os2.ico dist-os2\ICONS\
copy k95g_os2.ico dist-os2\ICONS\
copy ckoclip.ico dist-os2\ICONS\k2clip.ico
copy ckermit.ico dist-os2\ICONS\k2.ico
copy icons-readme.txt dist-os2\ICONS\readme.txt

REM KEYMAPS directory
@echo Copy keymaps...
set CK_DIST_KEYMAPS=vt220.ksc keycodes.txt capslock.ksc
REM These also used to be distributed but aren't very useful anymore:
REM       emacs.ksc keypad.ksc sni.ksc  wp50.ksc wp51.ksc keypad.ksc
REM They *were* available at:
REM   ftp://kermit.columbia.edu/pub/kermit/archives/k95keymaps.zip
REM Now archived at:
REM   https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu-2/pub/kermit/archives/k95keymaps.zip

for %%I in (%CK_DIST_KEYMAPS%) do copy %%I dist-os2\KEYMAPS\
copy keymaps-readme.txt dist-os2\KEYMAPS\readme.txt

REM Ideally we'd generate default.ksc here, but we can't run OS/2 binaries
REM on Windows.
REM TODO: Is default.ksc from a windows build identical?

REM PHONES directory
REM Contains dialing directories. All the files previously distributed here are
REM completely obsolete - none of the dial-in BBS still exist 20+ years later.
REM TODO: PHONES\readme.txt

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
copy printer-readme.txt dist-os2\printer\readme.txt

REM PUBLIC directory
@echo Copy public files...
copy hostuser.txt dist-os2\PUBLIC\

REM SCRIPTS directory
@echo Copy scripts...
set CK_DIST_SCRIPTS=apage.ksc autotel.ksc iksdpy.ksc login.ksc host.ksc
set CK_DIST_SCRIPTS=%CK_DIST_SCRIPTS% hostcom.ksc hostmdm.ksc hostmode.ksc hosttcp.ksc
set CK_DIST_SCRIPTS=%CK_DIST_SCRIPTS% npage.ksc recover.ksc review.ksc rgrep.ksc host.cfg
for %%I in (%CK_DIST_SCRIPTS%) do copy %%I dist-os2\SCRIPTS\
copy scripts-readme.txt dist-os2\SCRIPTS\readme.txt

REM TMP directory
REM TODO: TMP\readme.txt

REM USERS directory
@echo Copy User files...
copy hostmode-greeting.txt dist-os2\USERS\greeting.txt
copy hostmode-help.txt dist-os2\USERS\hostmode.txt

REM TODO: Copy over k95custom.sample as k2custom.sample