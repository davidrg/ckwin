# Change Log
This document covers whats changed in each release of C-Kermit for Windows (formerly known as Kermit 95).
For a more in-depth look at whats changed, check the git commit log.

## C-Kermit for Windows 10.0 beta 2 - 17 August 2022
This release focused on returning some level of SSH support. Initial efforts were based on using an external
SSH implementation which resulted in fixes to the DLL, PTY and COMMAND network types. External SSH didn't
work well enough in the end so focus shifted to built-in SSH using libssh.

Support for some older Visual C++ releases (4.0 and 5.0) was added to enable RISC NT builds in the future
(Visual C++ 4.0 was the last release to support MIPS and PowerPC), and OpenWatcom 1.9 support was added to
enable future OS/2 work.

 * Fixed builds with Visual C++ 14.x (2015-2022)
 * Fixed file transfer crash on builds done with Visual C++ 2008 and newer
 * Fixed builds with free versions of Visual C++ that don't include MFC
 * PTY support on Windows 10 v1809 and newer
 * Added OpenWatcom 1.9 support (win32 target only)
 * Fixed building with Visual C++ 97 (5.0)
 * Fixed building with Visual C++ 4.0
 * Fixed building with the free VIsual C++ 2003 toolkit & Platform SDK
 * Fixed 64bit file seeking
 * Fixed detection of current windows releases
 * Fixed network DLL support (set network type dll)
 * Fixed file transfers when built with Visual C++ 5.0 and older
 * Removed ctl3dins.exe from the distribution (Windows defender thinks its malware)
 * Now uses modern windows UI widgets on XP and newer
 * Built-in SSH
 * Added support for resizing DLL and PTY terminals when the CKW terminal is resized

## C-Kermit for Windows 10.0 beta 1 - 17 July 2022
This release focused on tidying up the open-source Kermit 95 release of July 2011, getting it into a 
buildable state, and rebranding it as C-Kermit for Windows. No effort was made to replace features
missing from the original open source release except for the GUI code (which was recovered and
open-sourced in late 2013).

It was based on C-Kermit 10.0 beta.04 and is best built with Visual C++ 6 though 7.0-8.0 (2002, 2003,
2005) do work too.

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
 * Reworked the about dialog
 * Fixed builds with Visual C++ 8 (2005)
 * Fixed builds with Visual C++ 7.1 (2003)
 * Fixed builds with Visual C++ 2010 and 2012
 * Updated version number, copyright dates, icons
 * Changed application name from Kermit 95 to C-Kermit for Windows
 * Disabled DNS SRV support
 * Removed border from GUI dialog buttons
 * Unused KUI code deleted
 * Upgraded to C-Kermit 10.0 beta.04
 * Fixed the pipe command
 * Fixed URLs in the help menu
 * Dropped separate version number for C-Kermit on Windows
 * Updated windows version check
 * Replaced "K95" with "CKW" in the status line and prompt

## Kermit 95 v2.2 - never publicly released
Kermit 95 v2.2 was never publicly released, but [this file](../kermit/k95/CHANGES.TXT) documents 
whats new since Kermit 95 v2.1.3.

Not every change for K95 v2.2 has made it in to C-Kermit for Windows due to the removal of some
components that could not be open-sourced.

## Previous Kermit 95 releases
 * [1.1.21 to 2.1.3 Change Log](http://www.columbia.edu/kermit/k95news.html)
 * [1.1.17 to 1.1.20 Change Log](https://web.archive.org/web/20010405154138/http://www.columbia.edu/kermit/k95news.html)
 * 1.1.14 to 1.1.16 Change Log
 * [1.1.2 to 1.1.13 Change Log](https://web.archive.org/web/19970815161519/http://www.columbia.edu/kermit/k95news.html)
