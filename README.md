Kermit 95 (C-Kermit for Windows and OS/2)
=========================================

Kermit 95 is a free scriptable terminal emulator and file transfer utility for
[nearly all](https://davidrg.github.io/ckwin/screenshots.html) 32-bit and 64-bit
Microsoft Windows, as well as 32-bit IBM OS/2.

Kermit 95 is the Windows and OS/2 port of C-Kermit from the Kermit Project and
was formerly a commercial product of Columbia University from 1994 until 2011 
(versions up to 2.1.3). Since 2013 it has been developed under the 3-clause 
BSD license. For more information on Kermit, visit the Kermit Project website: 
http://www.kermitproject.org, or see the [Kermit 95 3.0 Beta website](https://www.kermitproject.org/ckw10beta.html).

From 2013 until August 2024 this project was developed under the name
*C-Kermit for Windows* but the decision has been made to switch back to the
original name, Kermit 95, starting with beta 7 (January 2025) to reduce confusion
(plus it's a shorter name and works better on OS/2). While the name may have
changed, it's still the same program.

![Screenshot](doc/web/images/stack.png)

Highlights
----------
* [Dozens of terminal emulation options](https://davidrg.github.io/ckwin/current/termtype.html)
* [Full keyboard remapping](https://davidrg.github.io/ckwin/current/#keymap)
* Connect via [SSH](https://davidrg.github.io/ckwin/current/sshclien.html)†, 
  Serial, Modem ([TAPI](https://davidrg.github.io/ckwin/current/tapi.html) or Direct), 
  [Named Pipe](https://davidrg.github.io/ckwin/current/namepipe.html), 
  [Telnet](https://davidrg.github.io/ckwin/current/telnet.html), 
  Secure Telnet (TLS, Kerberos)†, rlogin, and PTY. Additionally, NetBIOS is 
  supported on OS/2
* [LAT and CTERM supported with DEC/Compaq/HP Pathworks32](https://davidrg.github.io/ckwin/current/dec.html)
* [RFC2217 Serial-over-telnet support](https://kermitproject.org/ckermit80.html#x14)
* Supports Windows _natively_ on x86, x86-64, ARM32, ARM64, Alpha, MIPS, 
  PowerPC, and Itanium CPUs
* Runs as a _native_ multithreaded 32-bit OS/2 application
* Full-featured Kermit protocol implementation including server mode and IKSD (based on C-Kermit)
* [Built-in X/Y/Z-MODEM support](https://davidrg.github.io/ckwin/current/#c8proto)
* Scriptable [FTP(s)](https://kermitproject.org/ckermit80.html#ftp)† and [HTTP(s)](https://kermitproject.org/ckermit80.html#x2.2)† clients
* Scriptable via its own scripting language (shared with 
  [C-Kermit](https://www.kermitproject.org/ckermit.html)) and 
  [REXX](https://davidrg.github.io/ckwin/current/rexx.html)‡
* Can listen for incoming telnet or modem connections for file transfer and 
  scripting (eg, the included [Host Mode](https://www.kermitproject.org/k95host.html) scripts)
* Optional menubar, toolbar and statusbar

† Secure communication methods (SSH, Telnet-ssl, ftps, https, Kerberos) require 
  Windows XP SP3 or newer.

‡ REXX scripting currently requires Windows XP SP3 or newer and an x86 or x86-64 
  CPU, or IBM OS/2. The next release will include REXX support on more Windows 
  releases and CPU architectures.

A [Feature Comparison](https://github.com/davidrg/ckwin/wiki/Feature-Comparison)
with some other terminal emulators/SSH clients is available on the Wiki.


Documentation
-------------
Kermit 95 comes with a full users guide - just type `manual` at the K-95 prompt,
or choose the *Manual* option from the Help menu.

For convenience, three versions of the users guide are also available online:

 * [Kermit 95 v3.0 - current release](https://davidrg.github.io/ckwin/current/) (v3.0.0 beta 7 currently)
 * [latest development build](https://davidrg.github.io/ckwin/dev/) - may discuss features and details not yet available in the current version
 * [Kermit 95 v2.1.3](https://kermitproject.org/k95manual/) - the last commercial release from January 2003

The Kermit 95 users guide primarily covers details unique to Kermit 95 such as
its terminal emulator and SSH client. For information on its command interface,
telnet client and other aspects it shares with C-Kermit, see 
[The C-Kermit Documentation](https://www.kermitproject.org/uckindex.html)

For information on the control sequences supported by Kermit 95, see the draft 
[Kermit 95 Control Sequences](https://davidrg.github.io/ckwin/dev/ctlseqs.html) 
document. Note that this document covers the next release of K95, not the 
current one! If you need to know what control sequences are supported in beta 7,
you can find a copy of this file in your DOCS folder.

If you're upgrading from Kermit 95 2.1.3 or earlier and use the SSH client, you
may want to check the [_SSH Client Reference_](https://davidrg.github.io/ckwin/current/sshclien.html)
section of the users guide for details on what's changed, or consult the
[SSH Readme](doc/ssh-readme.md) for a quick summary.

Getting Help
------------

If you run into any trouble with Kermit 95 or need help with something, you can 
ask a question [on GitHub Discussions](https://github.com/davidrg/ckwin/discussions). 
If you don't have a GitHub account, or would rather not ask a question in public
space, you can also email [ckw@kermitproject.org](mailto:ckw@kermitproject.org).

There is also a [Kermit 95 How-To](https://www.kermitproject.org/ckwhowto.html)
which may be useful for new users. The [Kermit 95 FAQ](https://www.kermitproject.org/k95faq.html), while not updated 
for v3.0, still contains some relevant information too.

If you think you may have found a bug, you can check the 
[K95 Bugs List](https://davidrg.github.io/ckwin/current/k95bugs.html) or the 
[Issue Tracker on GitHub](https://github.com/davidrg/ckwin/issues) to see if 
your bug is described anywhere. If it isn't, feel free to log it on the issue 
tracker or if you're not sure get in touch via one of the above methods. Bugs 
that aren't reported aren't likely to get fixed anytime soon!

Nature of the Current Release
-----------------------------
While recent Kermit 95 releases have carried the "beta" label, they have in 
practice been stable feature releases. They continue to carry the beta label 
only because:

* C-Kermit v10.0, on which Kermit 95 is based, is still in beta (though the 
  last few beta releases have only been addressing Unix/Linux/OpenVMS portability
  concerns).
* Kermit 95 v3.0 is not yet at feature parity with 2.1.3, though it isn't far 
  off now and the remaining missing features are becoming increasingly esoteric.
* There are a few terminal emulation features that have become common over the 
  last 20 years which Kermit 95 v3.0 should probably support (24bit color, 
  alternate screen buffer, and left/right margins to name a few).

Because the last "stable" release was in 2003 and isn't freely available (or 
particularly usable in 2025 due to changing encryption algorithms), Kermit 95 
is currently in a kind of [perpetual beta](https://en.wikipedia.org/wiki/Perpetual_beta) 
development stage. If the latest release meets you needs it isn't worth waiting 
for the beta tag to disappear - the latest beta releases are already more stable 
last stable commercial release, and the beta tag may still be around for another 
year or three depending on how much free time the projects single developer has.

The Dialer
----------
![Dialer Screenshot](doc/manual/dialer.gif)

The "Dialer" as included in Kermit 95 2.1.3 and earlier is still included with 
Kermit 95 v3.0 where possible, but it should now be considered a deprecated 
feature on Windows. It's not going to go away, but its also not going to get 
much in the way of enhancements or new features. It's there if you need it, 
but you're better off writing scripts or macros to save connection details (see 
the [K95 How-To](https://www.kermitproject.org/ckwhowto.html) for an example 
macro).

The dialer is trapped in the 90s by the frameworks it was built with, so it is 
not possible to build it for x86-64, Itanium, ARM32 or ARM64. As a result it's 
not included with the Itanium, ARM32 or ARM64 versions of Kermit 95.

A new replacement will be developed eventually (it is in fact already more than 
half built), but this likely won't appear until some release after K95 v3.0. 
When it does appear, the dialer will likely receive one final upgrade to support 
exporting its connections to a format its replacement can understand.

The new dialer replacement will be Windows only, so the existing Dialer will 
stick around for OS/2 (it works fine there, or would if it could be compiled 
with Open Watcom) and the Windows version will continue to be made available for
anyone who wants/needs it.

Kermit 95 for Windows Specifics
-------------------------------

There are many different varieties of Kermit 95 for Windows, and they aren't all
the same! Different compilers support targeting different versions of Windows, 
and all the 3rd party libraries Kermit 95 depends on to provide some of its
features require a minimum compiler version higher than what Kermit 95 itself
requires.

So as the Windows releases get older, Kermit 95 has to lose a few features in
order to be able to run at all.

| Feature        | Minimum Windows Version                     | Description / Notes                                                                                            |
|----------------|---------------------------------------------|----------------------------------------------------------------------------------------------------------------|
| PTY            | Windows 10 version 1809                     | Windows cmd/powershell/WSL using the K95 terminal emulator                                                     |
| SSH Client     | Windows XP SP 3                             | Earlier Windows releases will require writing an alternative SSH module based on some other SSH implementation |
| SSL/TLS        | Windows XP SP 3                             | This includes https, ftps and secure telnet support                                                            |
| REXX Scripting | Windows XP SP 3 on x86 or x86-64            | It is hoped REXX support will be extended to builds for other Windows versions eventually.                     |
| TAPI           | Windows 95 or NT 4.0                        | Modem dialing via `set modem` should still work on earlier versions of windows                                 |
| Toolbar        | Windows NT 3.51                             | Some GUI dialogs also require at least NT 3.51                                                                 |
| Dialer         | Windows NT 3.51                             | Not available on NT 3.50 currently                                                                             |
| GUI            | Windows NT 3.50 (x86), NT 3.51 (alpha/mips) | The GUI version (k95g.exe) is not currently available on the Alpha and MIPS versions of NT 3.50.               |

Kermit 95 for OS/2 (aka Kermit/2) Specifics
-------------------------------------------

![Screenshot on OS/2 Warp 3](doc/screenshot-os2.png)

Kermit 95 v3.0 beta 7 sees the return of 32bit OS/2 support! This replaces the
free OS/2 C-Kermit 5A(191) release of April 1995 and mostly replaces the 
commercial Kermit 95 2.1.2 release of November 2002 as well.

It should work on IBM OS/2 2.0 or newer with IBM TCP/IP 2.0 or newer, or IBM 
Internet Connection (dialup). If a compatible TCP/IP stack can't be found, the
Telnet, FTP and HTTP clients become unavailable but everything else is still
available for, eg, making serial connections.

As with past Kermit 95 releases, the OS/2 version does not include an SSH client
or a GUI/Presentation Manager variant (like K95G.EXE on Windows). It only exists
as an OS/2 Console (VIO) application. Now that its open source, if there is
sufficient demand it may be possible to add these someday.

Current releases don't currently include the PM connection manager (dialer),
SSL/TLS support, legacy telnet crypto (DES/CAST), or support for FTP Software 
PC/TCP 1.2 and IBM TCP/IP 1.2.1. Where possible these features may return in
future releases. For more information, see 
[issue #8](https://github.com/davidrg/ckwin/issues/8).

OS/2 is supported on a best-efforts basis due to a lack of OS/2 development
experience. If you use Kermit 95 on OS/2 (successfully or otherwise) 
[please let us know!](https://github.com/davidrg/ckwin/discussions) Bug reports
are important, as is knowing people are actually using it and what its being
used for.

The return of OS/2 support in Kermit 95 was made possible thanks to the help
of [Michal Necasek](https://www.os2museum.com/) who found fixes for all the
issues that popped up in the switch from IBM C/C++ 3.6 to Open Watcom 1.9.


Supported Terminal Emulations
-----------------------------

Kermit 95s terminal emulator supports the ADDS Regent 25; ADM-3A and ADM-5; ANSI-BBS; 
Avatar/0+; AT386; BeBox ANSI; Data General DASHER D200, D210;
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

You can pick a terminal emulation with either the `set term type` command, or
the drop-down list in the toolbar.

While Kermit 95 supports a reasonable subset of what other terminal emulators
such as xterm or PuTTY can do, Kermit 95 does not provide an "xterm" emulation 
option (and you [could argue](https://invisible-island.net/xterm/xterm.faq.html#other_versions) 
that many other terminal emulators don't really either). Instead, it is hoped
that eventually a proper terminfo entry can be written for it to properly expose
the features it supports without also exposing features it doesn't (help would be 
appreciated here!) 

In the meantime, for connecting to modern linux hosts you'll generally get the
best feature set  (colour, mouse reporting, etc) by using the linux console 
emulation with the utf-8 character set, though with some limitations imposed by
what the linux console itself supports:
```
set term type linux
set term remote utf8
```

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
never released publicly. The code was
[partially open-sourced in 2011](https://www.kermitproject.org/k95sourcecode-orig.html)
with [further bits open-sourced or rewritten in the decade since](doc/k95-open.md).

For full details on what's changed since the last public release of Kermit 95,
v2.1.3 of 21 January 2003, see the [Change Log](doc/changes.md).
