C-Kermit for Windows
====================

This is C-Kermit for Windows and OS/2. It is a free and open source version of the
program formerly known as Kermit-95, a commercial product of Columbia
University from 1994 to 2011. For more information on Kermit, visit the
Kermit Project website: http://www.kermitproject.org.

Major features include:
 * More than [40 terminal emulations](#supported-terminal-emulations)
 * Virtual Terminal connections over SSH, Telnet (including TLS support), 
   rlogin, modem, serial port and pty
 * Scriptable file transfer via Kermit, X/Y/Z Modem, HTTP, HTTPS, FTP and FTPS protocols
 * International character set translation
 * Pathworks supported for LAT and CTERM Virtual Terminal connections. 
   Can optionally be built with SuperLAT support as well (not enabled by
   default)

To get the latest most stable release as well as other news and information, 
visit the [C-Kermit for Windows Beta website](https://www.kermitproject.org/ckw10beta.html).
This software is currently based on C-Kermit version 10.0 Beta.07 of
7-DEC-2022 and is available under the 3-clause BSD license.

![Screenshot](doc/screenshot-w10.png)

Requirements
------------

C-Kermit for Windows should work on every 32bit or better x86 version of Windows
_except_ Windows NT 3.1. Win32s (Windows 3.x) is also not supported.

Certain features within C-Kermit for Windows have a minimum Windows version
requirement:

| Feature | Minimum Windows Version | Description                                                                    |
|---------|-------------------------|--------------------------------------------------------------------------------|
| PTY     | Windows 10 version 1809 | Windows cmd/powershell/WSL using the CKW terminal emulator                     |
| SSH     | Windows XP SP 3         |                                                                                |
| SSL/TLS | Windows XP SP 3         | This includes https, ftps and secure telnet support                            |
| TAPI    | Windows 95 or NT 4.0    | Modem dialing via `set modem` should still work on earlier versions of windows |
| Toolbar | Windows NT 3.51         | Some GUI dialogs also require at least NT 3.51                                 |

Supported Terminal Emulations
-----------------------------

ANSI-BBS; Avatar/0+; AT386; BeBox ANSI; Data General DASHER D200, D210; 
Data General DASHER D217 in native and Unix modes; DEC VT52; DEC VT100, VT102, 
VT220, VT320 with color extensions; Hazeltine 1500; Heath/Zenith 19; 
Hewlett Packard 2621A; HPTERM; IBM HFT and AIXTERM; IBM 3151; Linux console; 
Microsoft VTNT; QNX ANSI and QNX Console; SCOANSI; 
Siemens Nixdorf BA80 and 97801-5xx; Sun Console; 
Televideo TVI910+, TVI925, TVI950; Volker Craig VC404; 
Wyse 30, 50, 60, 160, and 370

For more details on what features the various emulations support, see 
[The Manual](https://www.kermitproject.org/k95manual/termtype.html).

Mouse reporting via the X10, X11/normal, URXVT and SGR protocols is also
supported and can be configured via the `set mouse reporting` command.

Documentation
-------------

The [Kermit 95 manual](https://www.kermitproject.org/k95manual/) still covers 
C-Kermit for Windows quite well and is the first place to look for details on
features and capabilities specific to Windows and OS/2. The SSH Client reference
is now a little out of date, see the [SSH Readme](doc/ssh-readme.md) for more
details on what has changed in the C-Kermit for Windows SSH implementation. 
There is also a [C-Kermit for Windows How-To](https://www.kermitproject.org/ckwhowto.html)
which may be useful for new users.

For everything else, consult the [C-Kermit Documentation](https://www.kermitproject.org/ckbindex.html).

If you previously used Kermit 95 and would like to know whats changed, see the
[C-Kermit for Windows Change Log](doc/changes.md) as well as 

New Features
------------
Since the [original open-source release](https://www.kermitproject.org/k95sourcecode-orig.html)
way back in July 2011, a lot of work has gone on to restore missing features and
bits that couldn't be open sourced at the time:

* The missing source code required to build the GUI version has been recovered
* The Dialer has been made available now that the framework it was built with,
  Zinc, is available under an open license
* X/Y/Z MODEM support has been restored thanks to Jyrki Salmi of 
  Online Solutions Oy (www.online.fi) providing his "P" X/Y/Z MODEM library 
  under the same license as C-Kermit for Windows
* An entirely new SSH subsystem using [libssh](https://libssh.org/) has been
  built. Not all SSH features are supported yet though - see the 
  [SSH Readme](doc/ssh-readme.md)
* The SSL/TLS code has been upgraded to support the latest version of OpenSSL
  allowing support for https, ftps and secure telnet to return
* Initial support for OS/2 is back, now built with OpenWatcom. Much
  more work is required to get this into a usable state however and assistance
  from anyone familiar with OS/2 development would be much appreciated.
* DECnet support has been re-enabled but you need a licensed copy of Pathworks32
  installed to use it.
* SuperLAT support has been restored as a custom build option. The SDK, while made
  publicly available by Meridian, has an unclear license so SuperLAT support
  isn't included by default at this time.
* The Telnet Encryption Option (DES/CAST) is supported again, not that anyone
  should be using it today if they care about security.

Additionally, a number of new features have been added:
* Compiler support has been extended down to Visual C++ 2.0 and all the way up
  to Visual C++ 2022 fixing a number of issues along the way. This enables
  proper support for the latest versions of windows, as well as the second
  oldest 32bit version - Windows NT 3.50
* PTYs are also supported on Windows 10+ now via the `pty` command. For example,
  `pty cmd.exe` will open the Windows shell inside C-Kermit and from there you can
  run any windows console tool. Note that Kermit file transfers are not supported
  via this mechanism as Windows PTYs are not transparent. There may also be some
  minor terminal emulation glitches as windows slots a terminal emulator in 
  between the subprocess and CKW (Windows PTYs are not transparent).
* The `pipe` command has also been fixed and kermit file transfers *are* supported
  via this mechanism. Applications relying on the special Windows terminal APIs
  won't work properly but anything that just outputs standard ANSI escape
  sequences should work interactively. PuTTYs plink tool works as long as the
  remote host is already trusted, and you use public key authentication.
* Network DLLs are also fixed. You can now add support for additional protocols
  via custom DLLs which are loaded with the `set network type dll` command.
* Mouse wheel support and terminal mouse reporting have been implemented
* A selection of other bugs fixed

A full [Change Log](doc/changes.md) is available and updated for major releases.

### Still Missing Features
The following features supported by Kermit 95 v2.1.3 remain unavailable in
C-Kermit for Windows at this time:

* SSH port forwarding, X11 forwarding and a few other features: Not reimplemented yet
* Kerberos: Formerly provided by Kerberos for Windows, last updated around six
  years ago. Work needs to be done to support Heimdal Kerberos which seems to
  be more up-to-date.
* SRP: Formerly provided by the Stanford SRP distribution, now unmaintained for
  over a decade. OpenSSL includes SRP support, but it would probably be a lot of
  work to switch

Code to support Kerberos for Windows and Stanford SRP still exists and should
still work if there is some need for these features despite any security issues
that may exist.

Compiling
---------

To build C-Kermit for Windows, see the [Build Instructions](doc/building.md).
You'll need at least Visual C++ 2.0 or newer, or OpenWatcom 1.9+. To build
with ConPTY and SSH support you'll need to use Visual C++ 2019 or newer.

To build C-Kermit for OS/2 using OpenWatcom, see the
[OS/2 Build Instructions](doc/os2-building.md).

### Making Changes
Any files matching the pattern `ck[cu]*.[cwh]` are shared by implementations of 
C-Kermit for other platforms (UNIX, Linux, VMS, and others) and are not 
specific to the Windows and OS/2 port in this repository. Any changes to these 
files should be sent to [The Kermit Project](https://www.kermitproject.org/)
to be included in future C-Kermit releases for other platforms. If your changes
are not intended to affect other platforms, make sure they're ifdef'd for either
OS2 (OS/2+Windows) or NT (Windows only).

Files matching `ck[on]*.*` can be safely modified as they are only used by the 
OS/2 and Windows targets

The dialer data files (dialer.dat, registry.dat) are binary files edited by the
zinc designer. It's not possible to merge these files, so it's probably best to
get in touch before making changes to these files.

History
-------
C-Kermit for Windows is based on what was going to be Kermit 95 v2.2. Compared 
to the final K95 release (2.1.3) a number of bugs have been fixed and a few new
features were added for 2.2. A full list of these is available here:

http://www.kermitproject.org/k95-fixes-since-213.txt

Due to the way Kermit 95 was open-sourced, not all changes documented above for
Kermit 95 2.2 are included in C-Kermit for Windows. In particular, the version
of the Dialer included here comes from the Kermit 95 v2.1.3 codebase and so 
doesn't include any of the 2.2 enhancements. The original SSH subsystem was not
open-sourced so of course any listed SSH enhancements to that don't apply.