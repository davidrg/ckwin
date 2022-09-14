# Change Log
This document covers what has changed in each release of C-Kermit for Windows 
(formerly known as Kermit 95). For a more in-depth look at what has changed, 
check the git commit log.

## C-Kermit for Windows 10.0b4 beta 3 - 14 September 2022
This release focused on improving SSH support, returning SSL support, minor
enhancements, porting to new platforms (NT 3.50, OS/2) and new compilers
(Visual C++ 2.0, OpenWatcom 2.0, OpenWatcom 1.9 for OS/2)

### New Features:
* Idle SSH sessions can now be prevented from timing out by supplying some
  interval to the "set ssh heartbeat" command, for example: set ssh heartbeat 60
* Added support for "user@host" syntax to SSH command. "ssh root@myhost" should
  do the same as "ssh myhost /user:root" now. The implementation is pretty basic
  and may not handle weird input well but when it works it should be less
  confusing to new users.
* Added mouse wheel support. By default, it scrolls one line at a time, or one
  screen at a time when holding Ctrl. You can remap this to whatever you like
  via the new "set mouse wheel" command which works like "set mouse button".
* File save dialogs are now the modern (normal) type on Windows ME, 2000 and
  newer. Windows 95, 98 and NT4 retain the old Windows 95-look file dialogs as
  before.
* The Shell Execute utility, se.exe, is back. Documentation is here:
  https://kermitproject.org/k95manual/url.html#urlsexe
* SSL and TLS support has returned. The http command can now make https
  connections, secure telnet (telnet-ssl) works again, as does ftps
* SSH is now supported on Windows XP (for now - it will probably disappear in a
  year or so when OpenSSL drops XP support)
* The screen update interval is no longer fixed at 100ms - you can now change it
  with the "set terminal screen-update fast" command. Smaller intervals will
  feel smoother. If the interval is too small for your computer elements that
  are supposed to blink (such as the cursor if noblink is not set) may not
  blink or may not blink consistently.
* The /subsystem qualifier now works on the SSH command, as does the "skermit"
  command allowing you to use kermit as an SSH subsytem. Documentation:
  https://kermitproject.org/skermit.html
* SSH Keyboard Interactive authentication is now supported
* New SSH-related command: set ssh v2 key-exchange-methods
* "set tcp nodelay" should affect SSH sessions now too
* Implemented these SSH-related commands:
  * set ssh v2 ciphers
  * set ssh v2 hostkey-algorithms. New options: ecdsa-sha2-nistp256,
    ecdsa-sha2-nistp384, ecdsa-sha2-nistp521, rsa-sha2-256, rsa-sha2-512,
    ssh-ed25519
  * set ssh v2 macs. New options: hmac-sha1-etm@openssh.com, hmac-sha2-256,
    hmac-sha2-256-etm@openssh.com, hmac-sha2-512,
    hmac-sha2-512-etm@openssh.com, none
  * set ssh heartbeat-interval
  * ssh key create
  * ssh key display
  * ssh key change-passphrase

### Fixed Bugs:
* Fixed bug where some applications (eg, nano, htop) wouldn't come back properly
  after being suspended with Ctrl+Z and restored with `fg` when using the linux
  terminal type.
* Fixed terminal being cleared the first time you move the K95G window and
  possibly the other random occurrences of this happening
* Fixed terminal scrolling bug in OpenWatcom! Builds done with OpenWatcom are
  now functionally equivalent to Visual C++ 6 in platform support and features
  and have no known issues unique to that compiler.
* Fixed auto-download "ask" setting not working on Windows NT 3.51
* Receiving large files (>4GB) no longer fails with "Refused, size"
* Fixed the "space" command never reporting more than 4GB of available free
  space
* Fixed incorrect (too narrow) window size on first run

### Minor Enhancements and other changes:
* Improved error message when no authentication methods supported by the SSH
  server are enabled
* Upgraded to libssh 0.10.3
* Adjusted how the cursor is drawn so it blinks more nicely in the GUI version
  of CKW
* Removed these SSH commands as they are obsolete and will never be supported by
  libssh, the SSH backend used by CKW:
  * set ssh v1
  * set ssh version 1
  * set ssh v2 authentication {external-keyex, hostbased, srp-gex-sha1}
  * set ssh v2 ciphers {arcfour, blowfish-cbc, cast128-cbc, rijndael128-cbc,
    rijndael192-cbc, rijndael256-cbc}
  * set ssh v2 macs {hmac-md5, hmac-md5-96, hmac-ripemd160, hmac-sha1-96}
  * set ssh {kerberos4, kerberos5, krb4, kerb5, k4, k5}
  * ssh key v1
  * ssh key display /format:ietf
  * ssh v2 rekey

### Source Changes:
* Fixed compatibility with the OpenWatcom 2.0 fork
* Added support for building with Visual C++ 2.0
* Added support for targeting Windows NT 3.50 with either OpenWatcom 1.9 or
  Visual C++ 2.0
* Now builds on OS/2 with OpenWatcom 1.9. Only minimal testing has been done.
  Networking does not work and the builds are done without optimisations.
  Further work is required, likely by someone with OS/2 development knowledge,
  to get it back to the Kermit-95 level of functionality.
* OpenSSL 0.9.8 - 3.0.5 (the latest version) now works
* Added support for TLS 1.1, 1.2 and 1.3 when built with sufficiently new
  versions of OpenSSL


## C-Kermit for Windows 10.0b4 beta 2 - 17 August 2022
This release focused on returning some level of SSH support. Initial efforts 
were based on using an external SSH implementation which resulted in fixes to 
the DLL, PTY and COMMAND network types. External SSH didn't work well enough in 
the end so focus shifted to built-in SSH using libssh.

Support for some older Visual C++ releases (4.0 and 5.0) was added to enable 
RISC NT builds in the future (Visual C++ 4.0 was the last release to support 
MIPS and PowerPC), and OpenWatcom 1.9 support was added to enable future OS/2 
work.

 * Fixed builds with Visual C++ 14.x (2015-2022)
 * Fixed file transfer crash on builds done with Visual C++ 2008 and newer
 * Fixed builds with free versions of Visual C++ that don't include MFC
 * PTY support on Windows 10 v1809 and newer
 * Added OpenWatcom 1.9 support (win32 target only)
 * Fixed building with Visual C++ 97 (5.0)
 * Fixed building with Visual C++ 4.0
 * Fixed building with the free Visual C++ 2003 toolkit & Platform SDK
 * Fixed 64bit file seeking
 * Fixed detection of current windows releases
 * Fixed network DLL support (set network type dll)
 * Fixed file transfers when built with Visual C++ 5.0 and older
 * Removed ctl3dins.exe from the distribution (Windows defender thinks its 
   malware)
 * Now uses modern windows UI widgets on XP and newer
 * Built-in SSH
 * Added support for resizing DLL and PTY terminals when the CKW terminal is 
   resized

## C-Kermit for Windows 10.0b4 beta 1 - 17 July 2022
This release focused on tidying up the open-source Kermit 95 release of 
July 2011, getting it into a buildable state, and rebranding it as C-Kermit for 
Windows. No effort was made to replace features missing from the original open 
source release except for the GUI code (which was recovered and open-sourced in 
late 2013).

It was based on C-Kermit 10.0 beta.04 and is best built with Visual C++ 6 though
7.0-8.0 (2002, 2003, 2005) do work too.

 * Upgraded from C-Kermit 8.0.207 to C-Kermit 10.0 beta.04. See the 
   [C-Kermit 8.0.208 to 10.0b4 Change Log](https://www.kermitproject.org/ckupdates.html#ck100beta04)
   for more information on all that's changed there.
 * SRP support disabled
 * DECnet support disabled
 * LAT support disabled
 * Kerberos support disabled
 * SSH support removed
 * SSL support disabled
 * Other encryption features disabled
 * X/Y/Z modem support removed
 * OS/2 support disabled
 * Dialer removed
 * zlib support disabled
 * Fixed builds with Visual C++ 6
 * Fixed builds with Visual C++ 7
 * Removed 64bit file seeking - code was broken.
 * Removed licensing, registration and demo mode functionality
 * Reworked the About dialog
 * Fixed builds with Visual C++ 8 (2005)
 * Fixed builds with Visual C++ 7.1 (2003)
 * Fixed builds with Visual C++ 2010 and 2012
 * Updated version number, copyright dates, icons
 * Changed application name from Kermit 95 to C-Kermit for Windows
 * Disabled DNS SRV support
 * Removed border from GUI dialog buttons
 * Unused KUI code deleted
 * Fixed the pipe command
 * Fixed URLs in the help menu
 * Dropped separate version number for C-Kermit on Windows
 * Updated Windows version check
 * Replaced "K95" with "CKW" in the status line and prompt

## Kermit 95 v2.2 - never publicly released
Kermit 95 v2.2 was never publicly released, but 
[this file](../kermit/k95/CHANGES.TXT) documents what's new since Kermit 95 
v2.1.3.

Not every change for K95 v2.2 has made it in to C-Kermit for Windows due to the 
removal of some components that could not be open-sourced..

## Previous Kermit 95 releases
 * [1.1.21 to 2.1.3 Change Log](http://www.columbia.edu/kermit/k95news.html)
 * [1.1.17 to 1.1.20 Change Log](https://web.archive.org/web/20010405154138/http://www.columbia.edu/kermit/k95news.html)
 * [1.1.16 Changes](https://groups.google.com/g/comp.protocols.kermit.announce/c/8jaYcOv0cvo/m/Er5rCyp_xG8J)
 * [1.1.15 Changes](https://groups.google.com/g/comp.os.ms-windows.announce/c/IDbj1Dl16aU/m/WmJlmGtSY5cJ)
 * [1.1.14 Changes](https://groups.google.com/g/comp.protocols.kermit.announce/c/KWT_5sYXeC8/m/AGvXUCtXSh4J)
 * [1.1.2 to 1.1.13 Change Log](https://web.archive.org/web/19970815161519/http://www.columbia.edu/kermit/k95news.html)
 * 1.1.1 Changes - ?