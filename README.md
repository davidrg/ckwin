C-Kermit for Windows
====================

This is C-Kermit for Windows. It is a free and open source version of the
program formerly known as Kermit-95, a commercial product of Columbia
University from 1994 to 2011. For more information on Kermit, visit the
Kermit Project website: http://www.kermitproject.org.

This software is currently based on C-Kermit version 10.0 Beta.04 of
3-JUN-2022 and is available under the 3-clause BSD license.

This code is based on what was going to be Kermit 95 v2.2. Compared to the
final K95 release (2.1.3) a number of bugs have been fixed and a few new
features have been added. A full list of these is available here:

http://www.kermitproject.org/k95-fixes-since-213.txt

New Features
------------

SSH Support is back in a limited way. Interactive sessions and file transfers
work but forwarding and some of the `set ssh` commands have not yet been
implemented. For more details, see the [SSH Readme](doc/ssh-readme.md).

PTYs are also supported on Windows 10+ now via the `pty` command. For example,
`pty cmd.exe` will open the Windows shell inside C-Kermit and from there you can
run any windows console tool. Note that Kermit file transfers are not supported
via this mechanism as Windows PTYs are not transparent. There may also be some
minor terminal emulation glitches as windows slots a terminal emulator in 
between the subprocess and CKW (Windows PTYs are not transparent).

The `pipe` command has also been fixed and kermit file transfers *are* supported
via this mechanism. Applications relying on the special Windows terminal APIs
won't work properly but anything that just outputs standard ANSI escape
sequences should work interactively. PuTTYs plink tool works as long as the
remote host is already trusted, and you use public key authentication.

Network DLLs are also fixed. You can now add support for additional protocols
via custom DLLs which are loaded with the `set network type dll` command.

Features Expected to Return Soon
--------------------------------

* The Dialer - it builds fine with OpenZinc so it just needs tidying up for
  open-sourcing (removing the registration code, etc).
* X/Y/Z Modem support - the author of the 3rd party library Kermit 95 used has
  OKd its open-sourcing so once that happens it will return.

Missing Kermit 95 Features
--------------------------
When Kermit 95 was open-sourced a number of features were disabled due to
obsolete 3rd party libraries, cryptography export regulations, or unavailability
of the required tools. Some of these may return someday, others likely will not.
This includes:

* OS/2 support (the code is still there but requires the IBM VisualAge compiler 
  to build at the moment)
* DECnet (formerly provided by DEC PATHWORKS) 
* LAT (formerly provided by either SuperLAT or PATHWORKS)
* Kerberos (provided by a very old version of MIT Kerberos for Windows)
* SRP (provided by the Stanford SRP distribution, unmaintained for over a decade now)
* SSL (OpenSSL 0.9.7)

Other features may be missing as a result of the above features being disabled.
For a full list of features available, type the following at the Kermit prompt:

        SHOW FEATURES

Note that this may lie about some features being available due to them not being
disabled cleanly.

Compiling
---------

To build C-Kermit for Windows, see the [Build Instructions](doc/building.md).
You'll need at least Visual C++ 6.0 SP6 or newer, or OpenWatcom 1.9. To build
with ConPTY and SSH support you'll need to use Visual C++ 2019 or newer.

Making Changes
---------------
Any files matching the pattern ck[cu]*.[cwh] are shared by implementations of 
C-Kermit for other platforms (UNIX, Linux, VMS, and others) and are not 
specific to the Windows and OS/2 port in this repository. Any changes to these 
files should be sent to [The Kermit Project](https://www.kermitproject.org/)
to be included in future C-Kermit releases for other platforms. If your changes
are not intended to affect other platforms, make sure they're ifdef'd for either
OS2 (OS/2+Windows) or NT (Windows only).

Files matching starting with ck[on]*.* can be safely modified as they are only
used by the OS/2 and Windows targets.
