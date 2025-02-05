Kermit 95 (C-Kermit for Windows and OS/2)
=========================================

This is Kermit 95, the Windows and OS/2 port of C-Kermit from the Kermit Project.
Kermit 95 was formerly a commercial product of Columbia University from 1994 
to 2011 (versions up to 2.1.3) and since 2013 has been developed under the 
3-clause BSD license. For more information on Kermit, visit the Kermit Project 
website: http://www.kermitproject.org.

Major features include:
 * More than [40 terminal emulations](#supported-terminal-emulations)
 * Virtual Terminal connections over SSH, Telnet (including TLS support), 
   rlogin, modem, serial port, named pipe and pty. Additionally, LAT and CTERM
   are supported if you've got Pathworks installed, and NetBIOS is supported on OS/2.    
 * Scriptable file transfer via Kermit, X/Y/Z Modem, HTTP, HTTPS, FTP and FTPS protocols
 * International character set translation

From 2013 until August 2024 this project was developed under the name
*C-Kermit for Windows* but the decision has been made to switch back to the
original name, Kermit 95, starting with beta 7 to reduce confusion
(plus it's a shorter name and works better on OS/2). While the name may have
changed, it's still the same program

For more news and information, visit the 
[Kermit 95 Beta website](https://www.kermitproject.org/ckw10beta.html).
This software is currently based on C-Kermit version 10.0 Beta.11 of
29-JUN-2024 and is available under the 3-clause BSD license.

![Screenshot](doc/screenshot-w11.png)

Requirements (Windows)
----------------------

Kermit 95 supports virtually all 32bit and 64bit releases of Microsoft Windows
on all CPU architectures. The only remaining exception is
[Windows NT 3.10](https://github.com/davidrg/ckwin/issues/164). However,
due to lack of hardware Itanium and ARM64 are not actively tested
so bug reports from users on these platforms are essential.

Unfortunately despite best efforts certain features in K95 do have a minimum version
requirement:

| Feature | Minimum Windows Version | Description / Notes                                                                                                  |
|---------|-------------------------|----------------------------------------------------------------------------------------------------------------------|
| PTY     | Windows 10 version 1809 | Windows cmd/powershell/WSL using the K95 terminal emulator                                                           |
| SSH     | Windows XP SP 3         | Earlier Windows releases will require writing an alternative SSH module based on some other SSH implementation       |
| SSL/TLS | Windows XP SP 3         | This includes https, ftps and secure telnet support                                                                  |
| TAPI    | Windows 95 or NT 4.0    | Modem dialing via `set modem` should still work on earlier versions of windows                                       |
| Toolbar | Windows NT 3.51         | Some GUI dialogs also require at least NT 3.51                                                                       |
| GUI     | Windows NT 3.50 (x86), NT 3.51 (alpha/mips) | The GUI version (k95g.exe) is not currently available on the Alpha and MIPS versions of NT 3.50. |

OS/2 Support
------------

Kermit 95 3.0 beta 7 should work on IBM OS/2 2.0 or newer. Optional TCP/IP
support should work with IBM TCP/IP 2.0 or newer. 

There are still a number of outstanding issues caused by the switch from IBM
C/C++ 3.6 (for which the project has no license) to Open Watcom 1.9 (see issue 
[#8](https://github.com/davidrg/ckwin/issues/8)), but none of these affect
making basic telnet, serial or NetBIOS connections.

Supported Terminal Emulations
-----------------------------

ADM-3A and ADM-5; ANSI-BBS; Avatar/0+; AT386; BeBox ANSI; Data General DASHER D200, D210; 
Data General DASHER D217 in native and Unix modes; DEC VT52; DEC VT100, VT102, 
VT220, VT320 with color extensions; Hazeltine 1500; Heath/Zenith 19; 
Hewlett Packard 2621A; HPTERM; IBM HFT and AIXTERM; IBM 3151; Linux console; 
Microsoft VTNT; QNX ANSI and QNX Console; SCOANSI; 
Siemens Nixdorf BA80 and 97801-5xx; Sun Console; 
Televideo TVI910+, TVI925, TVI950; Volker Craig VC404; 
Wyse 30, 50, 60, 160, and 370

For more details on what features the various emulations support, see 
[The Manual](https://davidrg.github.io/ckwin/current/termtype.html).

Mouse reporting via the X10, X11/normal, URXVT and SGR protocols is also
supported and can be configured via the `set mouse reporting` command.

Documentation
-------------

The Kermit 95 manual has mostly been revised for K95 v3.0 and is now
[available on the web](https://davidrg.github.io/ckwin/current/) in addition to
being included as part of the standard K95 distribution as of beta 7. To open it
from Kermit 95, choose _Manual_ from the _Help_ menu, or type the `manual` 
command at the K-95 prompt. There is also a [Kermit 95 How-To](https://www.kermitproject.org/ckwhowto.html) which may be 
useful for new users.

The full documentation for the built-in SSH client is now included in the
[_SSH Client Reference_](https://davidrg.github.io/ckwin/current/sshclien.html) 
section of the users guide, but for a quick summary you can consult the 
[SSH Readme](doc/ssh-readme.md). 

If you previously used Kermit 95 v2.1 or earlier and would like to know what's 
changed, see the [Kermit 95 Change Log](doc/changes.md) as well as the C-Kermit 9.0
and 10.0 changelogs

New Features
------------

Since the [original open-source release](https://www.kermitproject.org/k95sourcecode-orig.html)
way back in July 2011, a lot of work has gone on to [restore missing features and
bits that couldn't be open sourced at the time](doc/k95-open.md). In addition to this work getting
Kermit 95 back to where it was in 2003, a number of new features have been added to bring it
forward:

* An entirely new SSH subsystem using [libssh](https://libssh.org/) has been
  built. Not all SSH features are supported yet though - see the [SSH Readme](doc/ssh-readme.md)
  for more information.
* Significantly expanded compiler support
  *  Added support for Visual C++ 5.0 down to 1.0 32-bit
  *  Added support for Visual C++ 2003 up to 2022
  *  Added support for Open Watcom (targeting both Win32 and OS/2)
  *  Added limited support for GCC (MinGW)
* Significant work on supported CPU architectures:
  * New port to 64bit Windows on x86-64, ARM64, Itanium and Alpha64
  * New port to 32bit ARM (Windows RT) and MIPS
  * Reinstated ports to Alpha and PowerPC. Alpha support was previously discontinued
    after Kermit 95 v1.1.20 (May 2000) and PowerPC was discontinued after Kermit 95
    v1.1.16 (8 April 1998).
* Improved support for modern Windows versions, and added support for Windows NT 3.50
* PTYs are now supported on Windows 10 v1809 or newer via the `pty` command. For example,
  `pty cmd.exe` will open the Windows shell inside Kermit 95 and from there you can
  run any windows console tool. Note that Kermit file transfers are not supported
  via this mechanism as Windows PTYs are not transparent. There may also be some
  minor terminal emulation glitches as windows slots a terminal emulator in 
  between the subprocess and K95 (Windows PTYs are not transparent).
* Mouse wheel support and terminal mouse reporting have been implemented
* REXX is now supported on Windows as well as OS/2
* A selection of other bugs fixed and other minor features added

A full [Change Log](doc/changes.md) is available and updated for major releases.

### Missing/Removed Features
The following features supported by Kermit 95 v2.1.3 remain unavailable in
Kermit 95 at this time:

* few misc SSH features have not been implemented yet - ticket
  [#44](https://github.com/davidrg/ckwin/issues/44) is tracking these.
* DNS-SRV ([#205](https://github.com/davidrg/ckwin/issues/205)) - support for this
  previously came from wshelper, a part of MIT Kerberos for Windows that was removed
  in 2018.

And the following features will not be returning outside of custom builds (with the
exception of SSHv1 the code to support them still exists give the right obsolete
dependencies):
  
* SSHv1: Not supported by LibSSH anymore
* SSH/SSL/TLS on Windows versions prior to Windows XP SP3 will likely not return
  as current versions of OpenSSL no longer supports these older versions of
  Windows or the compilers required to target them.
* SRP: Formerly provided by the Stanford SRP distribution, now unmaintained for
  over a decade it is not compatible with any supported version of OpenSSL. If
  SRP support is ever to return, it will likely be via someone adding support for
  OpenSSLs implementation of it ([#32](https://github.com/davidrg/ckwin/issues/205))
* Kerberos IV support is not included in standard builds as current versions of
  MIT Kerberos for Windows no longer support it. 

Compiling
---------

To build Kermit 95, see the [Build Instructions](doc/building.md).
Visual C++ 2019 or newer is strongly recommended, but any released 32bit or 64bit
version *should* work (with certain features automatically excluded on older compilers).

There is also limited support for compiling with Open Watcom 1.9+ and MinGW. For
Open Watcom, just follow the normal build instructions above. For cross-compiling
from Linux, see the [MinGW Build Instructions](doc/mingw-building.md).

To build Kermit 95 for OS/2 (Kermit/2) using Open Watcom, see the
[OS/2 Build Instructions](doc/os2-building.md).

For a list of supported compilers and which features they do or don't get,
see [Compiler Support](doc/compilers.md)


History
-------
Kermit 95 v3.0 is based on what was going to be Kermit 95 v2.2 which was
never released publicly. The code was partially open-sourced in 2011 with
[further bits open-sourced in the decade since](doc/k95-open.md).

For full details on what's changed since the last public release of Kermit 95,
v2.1.3 of 21 January 2003, see the [Change Log](doc/changes.md).
