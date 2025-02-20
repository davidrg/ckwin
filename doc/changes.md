# Change Log
This document covers what has changed in each release of Kermit 95 since its
first open-source release in 2022.  For a more in-depth look at what has 
changed, check the git commit log.

Because it's been over 20 years since the last stable release of K95, changes
for the Kermit 95 v3.0 beta releases (which are already more usable than the
last stable release) are temporarily included here, with the full v3.0 change 
log living in the [Whats New in 3.0](whats-new.md) document. When the final 
release of v3.0 eventually happens, the details about the various v3.0 
eventually be moved elsewhere, with the full v3.0 change log taking their place.

## Kermit 95 v3.0 beta 7 - 27 January 2025

As of Beta 7, C-Kermit for Windows has been renamed back to Kermit 95, the name
it carried from 1995 through to 2013.

### Things to be aware of when upgrading
* K95G no longer opens COM1 by default. If you previously depended on this, 
  you'll need to add `set port com1` to your k95custom.ini
* The mouse wheel now scrolls half a screen at time, rather than one line at
  a time. If you'd prefer to retain the old default, put the following in your
  k95custom.ini:
  ```
  set mouse wheel up none \Kupone
  set mouse wheel down none \Kdnone
  ```

#### The default SSH directory has changed!
The default SSH directory in beta 7 has changed from `\v(home).ssh` back to
`\v(appdata)ssh`, the location used by Kermit 95 2.1.3 and earlier.

This means Kermit 95 may not find your known hosts file, or your identity 
(public key authentication) files after upgrading to beta 7.

If you'd prefer to keep these files in `\v(home).ssh`, the same location used
by OpenSSH on modern versions of windows, add the command 
`set ssh directory \v(home).ssh` to your k95custom.ini

> [!TIP]
> To find out where `\v(appdata)`, `\v(home)` and other such directories are
> on your disk, you can use the `orient` command.


### New features
* SSH Port forwarding (tunneling) is now supported again in both
  the Direct/Local and Reverse/Remote forms. You can add forwards before
  establishing a connection with `SSH ADD { local, remote }` and remove all
  forwards of a given type with `SSH CLEAR { local, remote }`. As in past releases
  of Kermit 95, these commands don't have any effect on an already established SSH
  connection - this may be changed in the future.
* New command to allow removing individual port forwards (`SSH REMOVE 
  { local, remote }`) - previously Kermit 95 only had commands to remove *all*
  forwarded ports of a given type.
* X11 forwarding is back. Turn on with `SET SSH X11 ON`, and set your display
  with `SET TELNET ENV DISPLAY`
* The SSH backend has been moved into a DLL. On startup, K95 attempts to
  load the backend DLL provided the `-#2` command line argument has not been
  supplied. If no SSH backend gets loaded, you can load one manually with the new
  `ssh load` command. This allows K95 to load the appropriate backend automatically
  based on operating system (Windows XP or not) and presence of MIT Kerberos for
  Windows. This removes the need to manually shuffle around ssh.dll variants, and
  also means that alternative SSH backends not based on libssh can now be supported
  should anyone want to build one, opening the door to SSH on vintage windows or
  OS/2 systems.
* SSH is now supported on 32bit ARM devices (Windows RT) for the first time
* Initial very limited SSH agent support has been added. Libssh is currently the
  limiting factor here and SSH agent support in K95 likely won't get much better
  without significant improvements to libssh in this area. See the SSH Readme
  for more details.
* REXX support has been extended to x86 systems running Windows XP or newer. This
  was previously an OS/2-only feature. You can now run REXX scripts and commands
  from the K95 command mode with the `rexx` command, and REXX scripts run from 
  K95 can execute any Kermit 95 command by enclosing the K95 command in single
  quotes (eg, `'set parity none'`). For full details, see the REXX section of
  the K95 manual: https://kermitproject.org/k95manual/os2k95.html#os2rexx. The
  REXX implementation is the current Regina REXX release. regutil.dll is included
  but note that the Console I/O Routines it provides are not currently compatible 
  with K95. The rexxre.dll external function package is also included providing
  support for POSIX regular expressions.
* New command to turn the menubar on or off: `set gui menubar visible { on, off }`
  When the menubar is turned off in this way (rather than with the command line 
  parameter), important menu items are moved on to the system/control/window menu
  (right-click on the title bar for the actions menu and a few other things):
  ![k95-sysmenu](https://github.com/user-attachments/assets/4a016ca5-f339-43c5-83e8-7b899b28d117)
* New command to turn the toolbar on or off: `set gui toolbar visible { on, off }`
* New command to turn the statusbar on or off: `set gui statusbar { on, off }`
* New screen scroll kverbs:
  * `\Kuphscn` - Scroll up half a screen
  * `\Kdnhscn` - Scroll down half a screen
* IBM OS/2 support is back! It should work on IBM OS/2 2.0 or newer with 
  optional TCP/IP support provided by IBM TCP/IP 2.0 or newer. No SSH or 
  Presentation Manager GUI as in past Kermit/2 releases. Additionally:
  * No PC/TCP 1.2 or IBM TCP/IP 1.2.1 support (no SDK license; the DLLs from 
    K95 2.1.2 should work if you need it)
  * No dialer yet (crashes when built with Open Watcom)
  * No SSL/TLS support (can't be built with Open Watcom)
  * No legacy telnet encryption (no longer useful, but may return in a future
    release anyway)
* Reimplemented the three checkboxes in the Dialers GUI settings page for the
  menu bar, toolbar and status bar. These options were new in Kermit 95 2.1.3
  but were not present in previous open source releases of the dialer as it's
  based on code from K95 2.1.2. Any dialer entries edited with the dialer from 
  C-Kermit for Windows betas 4-6, or Kermit 95 2.1.2 or older, will have these 
  checkboxes default to ON. Entries last edited with the K95 2.1.3 dialer will 
  have these three settings preserved. Turning off the GUI bars in this way does
  so via command line arguments rather than the new `set gui` commands so they
  can't be turned back on with the new `set gui` commands.
* Added support xterms Bracketed Paste feature
* Most of the users guide has been revised for this release and is now included
  as part of the release. The _Kermit Security Reference_ and sections dealing
  with installing and uninstalling kermit 95 are still waiting to be overhauled.

### Minor Enhancements and other changes
* All executables (*.exe, *.dll) now have proper versioninfo resources
* Upgraded to OpenSSL 3.4 which fixes a number of bugs and security issues and
  will receive security fixes until October 2026. 
* K95G no longer opens COM1 by default. If you still want this behaviour, add
  `set port com1` to your k95custom.ini
* The command `set gui toolbar off` has been renamed to
  `set gui toolbar disabled` to better describe what it actually does and to
  make room for new commands to turn it on and off. The previous syntax
  (`set gui toolbar { off, on }`) is still accepted for compatibility with 
  existing scripts. `set gui toolbar on` still does nothing as it always has
  (disabling the toolbar is a session lockdown feature)
* The command `set gui menubar off` has been renamed to
  `set gui menubar disabled` to better describe what it actually does and to
  make room for new commands to turn it on and off. The previous syntax
  (`set gui menubar { off, on }`) is still accepted for compatibility with
  existing scripts. `set gui menubar on` still does nothing as it always has
  (disabling the menubar is a session lockdown feature)
* Implemented the `set ssh identity-file` command
* Added new command `set ssh directory` which allows you to set the default
  location where K95 looks for user known hosts and identity files.
* The default SSH directory has changed from `\v(home).ssh` back to
  `\v(appdata)ssh`
* The `ssh key` commands will now default to opening or saving keys in the
  SSH directory.
* The `skermit` command now has help text
* The default mouse wheel configuration has changed:
  * Wheel up/down now scrolls up/down half a screen (like PuTTY) rather than a
    single line. This provides better much speed than a line at a time with
    better usability than a screen at a time.
  * Shift+Wheel up/down now scrolls up/down one line
* Improve exit time when the console version (k95.exe) is just being run to show
  usage information (`k95.exe -h` or `k95.exe --help`). Previously K95 would 
  pause for about 5 seconds after printing usage information before returning 
  you to the shell.
* Fixed the `telnet.exe` and `rlogin.exe` stubs - these now behave as in K95  
  2.1.3
* Added the `ssh.exe`, `ftp.exe` and `http.exe` stubs that were included in
  K95 2.1.3 
* `iksdnt.exe` is now included.

### Fixed bugs
* Fix `fopen` causing a crash. This issue seems to have come in some recent 
  version of the Microsoft C Runtime.
* Fix hitting backspace in an empty search-in-buffer crashing
* Fix `pty dir` (or trying to run anything else that isn't a valid program)
  causing a crash
* Fixed POTS modem support not being available on NT 3.50
* Fixed OpenSSL libraries not being included in the ARM32 distribution
* Fixed \Kexit (Alt+x by default) not updating the state of the associated
  toolbar button
* Fix the SSH global known hosts file not being set to something sensible
  on windows. It's now set to the value used by past Kermit 95 releases by
  default: `\v(common)ssh\known_hosts2`
* Fixed generation of 4096 RSA SSH keys
* Fixed stdout parameter not working correctly
* Fixed a pair of issues in the OS/2 NetBIOS implementation which has likely
  been totally broken since Kermit 95 v1.1.17:
  * `SET HOST` doing nothing for NetBIOS connections
  * NetBIOS name not being correctly padded when making a connection to
    a NetBIOS Server
* Fixed emacs turning off mouse reporting when started
* Fixed K95G hanging when closing the window or using File->Exit with when
  a connection is active and GUI dialogs are turned off
* Fixed K95 bug 797: Dialer generated connection scripts will no longer include
  `SET LOGIN PROMPT` or `SET LOGIN PASSWORD` commands if those fields do not
  have a value as this broke the use of the standard login.ksc script.
* Fixed K95 bug 770: When editing an FTP entry in the dialer the general settings
  page doesn't load the port number causing it to be cleared on save.
* Fixed `TYPE` command error "The /HEIGHT switch is not supported without /GUI"
  when the `/HEIGHT` switch has not been supplied.

### Other Source Changes
* Fixed a selection of build warnings, and improved compatibility with the 
  Open Watcom compiler.


## C-Kermit for Windows 10.0b11 beta 6 - 11 August 2024

This is a minor release focused on upgrading from OpenSSL 1.1.1 (which is
now out of support) to OpenSSL 3.0.x, and libssh 0.10.5 to 0.10.6. Also 
included are a selection of bug fixes and other minor enhancements. 

Also in this release: official support for Windows NT on Alpha and PowerPC
has returned after being discontinued in March 2000 and April 1998 
respectively. And for the first time ever, C-Kermit is now supported on
Windows NT for MIPS computers, though without TAPI support.

This is also the last release carrying the "C-Kermit for Windows" name. The
next release will be Kermit 95 3.0 beta 7.

### Things to be aware of when upgrading
Windows XP users: current versions of libssh are no longer compatible with 
Windows XP. See the included SSH Readme for a workaround for SSH support on
Windows XP.

### Fixed Bugs
* Fixed directory listings not reporting a size or modified time for files
  requiring more than 32 bits to store the file size on Windows NT 3.51 and
  newer. This issue will remain on NT 3.1/3.50.
* C-Kermit will now find the dialer correctly if C-kermit isn't started from
  the dialer. This behaviour was broken by rebranding the Kermit 95 dialer -
  C-Kermit looks for it by its window title.
* The default k95custom.ini no longer produces an error when it's run in the
  console version of C-Kermit or in a build of C-Kermit that lacks SSH support.
* Fixed the SSH, SSL and Kerberos options in the dialer not being available or
  not working correctly when they should be. These options are now permanently
  enabled whether the bundled version of C-Kermit supports these features.
  Previously the dialer tried to load the Kerberos, OpenSSL and SSH DLLs to
  see if they were there. This no longer works due to changing DLL names (due
  in part to changing library versions) and the fact these DLLs are now often
  built for 64bit while the dialer remains 32bit. 
* The SSH client will now check which authentication methods are allowed by
  the server after each authentication method is attempted. This allows the
  client to handle servers which change their list of allowed authentication
  methods during login.
* Fixed `show network` command showing "SSH V1 and V2 protocols - not available"
  in builds that *do* have SSH support
* Fixed `show network` command showing "SSH V1 and V2 protocols" when SSH V1 is
  no longer supported in C-Kermit for Windows.
* Fixed not being able to resize the terminal area to greater than the primary
  display in K95G. For example, if the window was moved on to a display that
  was taller than the primary display and maximised the bottom of the terminal
  screen would not be correctly rendered. This fix only applies to modern
  versions of Windows.
* Fixed included openssl.exe not working on Windows XP
* Fixed paging for the "help options all" command where argument help contains
  line breaks

### Minor Enhancements and other changes

* Upgraded to OpenSSL 3.0.14, the current long term support release 
  (supported until 7 September 2026)
* Updated to libssh 0.10.6
* Updated to zlib 1.3.1
* Help text for "set gui window position" updated: this command *is* supported
  and it does work.
* The default k95custom.ini now displays a message the console-mode version
  (k95.exe) is run pointing new users to the GUI version (k95g.exe)
* The SSH client will now attempt authentication methods in the order specified
  by `set ssh v2 authentication` (the list is now ordered rather than being
  simply a list of what is allowed). The default order is: none, gssapi, 
  public key, keyboard-interactive, password.
* Binaries are now provided for Windows NT running on Alpha, MIPS and PowerPC
  systems.
* Upgraded to C-Kermit 10.0 Beta.11
* About window (Help -> About) now includes the beta number
* Added help text for `set terminal autopage` and `set terminal autoscroll`
* Increased the maximum number of terminal columns from 256 to 512 in K95G.
  This should be enough to fill a 4K display at with a 10pt font or larger.
  As this change increases memory requirements by around 1MB whether the extra
  columns are used or not, it has only been increased in builds targeting
  modern PCs. Vintage PCs will still be limited to 256 columns.
* CKW no longer rejects updated OpenSSL DLLs provided the major and minor
  versions match.

### New features

* Implemented the DECRQCRA VT420 escape sequence which is required by esctest2.
  This is disabled by default due to its security risks, but can be enabled 
  with `set terminal send-data on`. As C-Kermit doesn't currently have a VT420
  terminal option yet, select VT320 to make use of VT420 features.

### Source Changes

* Some minor changes to support a future replacement for the dialer.
* Fixed over 1,000 build warnings. Some of the things warned about could lead
  to application crashes on 64bit windows.

## C-Kermit for Windows 10.0b10 beta 5 - 3 July 2023
This release mostly focused on porting the codebase to 64bit Windows - x86-64,
ARM64, Itanium and even the never publicly released 64bit Windows 2000 for Alpha.
Also included are a selection of minor enhancements, features and bugfixes.

### Things to be aware of when upgrading

This release corrects some issues with the DEFAULT template which the Dialer
creates automatically on first run. The auto-created template has bad colour
values for colours 1 through 9 resulting in blue appearing green, etc (terminal
sessions that should have a blue background will have a green one). The correct
values are in the table below - these should be applied to the DEFAULT template 
as well as any other connections or templates you've created via the 
*GUI Settings* page:

<table>
<tr><th>Colour       </th><th>Red</th><th>Green</th><th>Blue</th></tr>
<tr><td>1 - blue     </td><td>0  </td><td>0    </td><td>127 </td></tr>
<tr><td>2 - green    </td><td>0  </td><td>127  </td><td>0   </td></tr>
<tr><td>3 - cyan     </td><td>0  </td><td>127  </td><td>127 </td></tr>
<tr><td>4 - red      </td><td>127</td><td>0    </td><td>0   </td></tr>
<tr><td>5 - magenta  </td><td>127</td><td>0    </td><td>127 </td></tr>
<tr><td>6 - brown    </td><td>127</td><td>127  </td><td>0   </td></tr>
<tr><td>7 - lightgray</td><td>192</td><td>192  </td><td>192 </td></tr>
<tr><td>8 - darkgray </td><td>127</td><td>127  </td><td>127 </td></tr>
<tr><td>9 - lightblue</td><td>0  </td><td>0    </td><td>255 </td></tr>
</table>

The default logfile names in the *Log File Settings* page should also start with
the `\` character - by default in prior versions they started with an invalid 
non-printable character. In Kermit 95 the defaults did not start with 
\v(appdata) so you may wish to just remove this prefix entirely instead of
correcting the first character.

If you have Kermit 95 installed these bugs likely doesn't affect you - while the
Kermit 95 dialer has them too, Kermit 95 shipped with 
`C:\ProgramData\Kermit 95\dialinf.dat` which contains a DEFAULT template
containing the correct values.

### New Features
* Builds for 64bit Windows
    * x86-64
    * ARM64
    * Itanium
* Build for ARM32
* Added TLS SNI support for the http and telnet clients
* The old hostmode and other scripts are now included in the distribution with
  no substantial changes since Kermit 95.
* Kerberos V support has returned (Kerberos IV support can be built from source)
* GSSAPI authentication in the SSH client (requires MIT Kerberos for Windows,
  see ssh-readme for more information)
* Updated to C-Kermit 10 Beta.10
* Updated to libssh 0.10.5
* Updated to OpenSSL 1.1.1u

### Fixed Bugs
* Fixed crash on startup under Crossover on MacOS and Wine on Linux
* Fixed `show terminal` command showing autodownload as being "on" when it was
  actually set to "ask"
* Fixed receiving files with an as-name that includes a path using UNIX 
  separators failing when autodownload is set to "ask" in the GUI version of CKW
  (receiving a file sent as `send foo.txt C:/temp/foo.txt` now works)
* Fixed CKW for modern Windows (XP SP3+) not setting modified time on received
  files
* Fixed entering plane-1 unicode values via Alt+n crashing the application (this
  bug was inherited from Kermit 95)
* Fixed SSH bugs:
    * anonymous SSH (userauth none) doesn't work
    * buffer not flushed on logout
    * fixed some (but not all) occurrences of a server disconnect not being
      detected
* `set syslog` works again - this was accidentally removed in beta 4.
* The colours in the dialer *GUI Settings* for the DEFAULT template
  automatically created when first run were completely wrong. This bug affected
  Kermit 95 as well but most users wouldn't have seen it as the shipped 
  dialinf.dat included a DEFAULT with the correct settings (along with a
  selection of other templates).
* The default log file names in the *Log File Settings* page for the DEFAULT
  template automatically created when first run begin with an invalid
  non-printable chracter instad of '\'. This bug affected Kermit 95 as well.

### Minor Enhancements and other changes
* The sample k95custom.ini file now sets:
    * autodownload to "on" rather than "ask"
    * resize mode to change terminal dimensions
    * SSH heartbeat on
* New escape sequences in the terminal emulator
    * `CSI 1 8 ; Ps ; Ps t` - get text area size in characters (xterm)
    * `CSI > Pm t` - this is now parsed and ignored, previously this xterm-specific
      escape sequence would incorrectly minimise the window
* A sample IKSD initialisation script is now included in the distribution
  Setup instructions are here: https://kermitproject.org/k95manual/wiksdadm.html
* The "show features" command now includes the availability (or lack of) DECnet,
  SuperLAT and PTY support. Some of these are optional features for custom
  builds only and others may or may not be present depending on the compiler
  used.
* "show versions" now shows OS version more correctly
* Updated the dates and version numbers for windows-specific entries in the 
  "show versions" command
* k95custom.ini is now distributed as k95custom.sample to prevent customised
  user files from being accidentally overwritten when upgrading. The default
  k95.ini will now rename k95custom.sample to k95custom.ini if it can't find
  an existing k95custom.ini in any of the usual locations.
* Updated the About dialog: xyzmodem and pathworks support are no longer guaranteed
  to be missing.
* Improvements to the terminal status line:
    * Increasing the window width now gives more space to the hostname and protocol
      fields
    * If the hostname and protocol text are too long to fit, the exit text 
      ("Command: Alt-X" by default) is hidden to make room
    * Maximum length for exit text increased from 20 to 30 characters (where there 
      is room to display it)
    * When there is sufficient room available an extra two columns of padding is
      inserted between the exit text and the hostname to match the other fields

### Source Changes
* Fixed build failure with Visual C++ 2017
* Fixed build failure with Visual C++ 4.1 and 4.2
* Fixed some reported build errors with Visual C++ 4.0 RISC Edition. MIPS should
  now build fine with TAPI disabled, PowerPC may also need debug logging turned
  off (`set CKF_DEBUG=no`). No one has tried building the dialer yet.
* Fixed build issues with the DEC Alpha compiler in the NT 3.50 SDK. The linker
  supplied in the SDK is not compatible, however.
* Ported to 64bit Windows
* Added support for building for 64bit Windows on the DEC Alpha (AXP64 target)
* Ported the core of the application to MinGW (GCC)
* Added support for Visual C++ 1.0/1.1 32-bit edition
* Ported to Windows NT 3.10 but there are currently unresolved bugs (networking
  is broken for one)
* SRP support is now available as a custom build option. You'll need quite an
  old and insecure version of OpenSSL (1.0.1u works) and an old compiler too
  (Visual C++ 2003 works)
* DES/CAST crypto can now be statically linked into k95.exe/k95g.exe/iksdnt.exe
  instead of being delivered as a shared dll (k95crypto.dll)
* Building with jom (or any other sufficiently compatible nmake clone) instead
  of nmake is now supported for Visual C++ builds. Just `set make=jom.exe` to
  make better use of multicore systems.

## C-Kermit for Windows 10.0b8 beta 4 - 15 December 2022
This release is mostly a collection of minor improvements and bug fixes as I
have been unexpectedly busy since the prior release resulting in the schedule
slipping somewhat.

### Things to be aware of when upgrading
* The third mouse button is now supported in k95g which may affect any scripts
  you have that map mouse buttons. On a three button mouse:
    * Previously: Left was button 1, Middle was unsupported, Right was button 2
    * Now: Left is button 1, Middle is button 2, Right is button 3
* Updates to the Dialer to support new SSH settings have resulted in the version
  number for the dialer data file being bumped. It is advised you take a backup
  of your existing data file before running the new version of the dialer for
  the first time.
    * On first start, your data file(s) will be upgraded to the new format
    * SSH connection scripts generated by the new dialer will not be compatible
      with Kermit 95 due to a difference in supported SSH options.

### New Features:
* Upgraded from C-Kermit 10.0 beta.04 to beta.07, plus beta.08 changes from
  the 11th and 12th of December 2022.
    * [New in C-Kermit 10.0 Beta.05](https://kermitproject.org/ckupdates.html#ck100beta05) 
    * [New in C-Kermit 10.0 Beta.06](https://kermitproject.org/ckupdates.html#ck100beta06)
    * [New in C-Kermit 10.0 Beta.07](https://kermitproject.org/ckupdates.html#ck10beta07)
    * [New in C-Kermit 10.0 Beta.08](https://kermitproject.org/ckupdates.html#ck10beta08)
      (Only up to and including 12 December 2022)
* The default `k95custom.ini` now outputs a message on startup directing new 
  users to have a look at and optionally customise the file
* X/Y/Z MODEM support is back thanks to Jyrki Salmi of Online Solutions Oy 
  (www.online.fi) providing his "P" X/Y/Z MODEM library under the same license 
  as C-Kermit for Windows
* Terminal mouse reporting
    * X10 protocol (send button + coordinates on mouse down) supported for Linux 
      and ANSI terminal types
    * X11/Normal, URXVT and SGR protocols (send button + modifiers + coordinates 
      on mouse down and mouse up) supported for all terminal types
    * Mouse wheel supported for all but the X10 protocol: You can scroll the 
      panels in midnight commander with the mouse wheel!
    * New command: `set mouse reporting x` where x is one of:
        * `disabled` - mouse events will not be reported
        * `enabled` - Applications can turn mouse reporting on. Mouse reports 
          will be sent only if a mouse event is mapped to `\Kignore` (eg, if you 
          map right-click to `\Kpaste` then right-click will never be sent)
        * `override` - Applications can turn mouse reporting on. All mouse  
          events will be sent to the remote host and any configured action in 
          CKW will be ignored when mouse reporting is on. For example, if you
          map right-click to `\Kpaste` this will only have an effect outside of
          applications that turn mouse reporting on.
    * The `show mouse` command shows the mouse reporting setting plus current
      state (if it's active or not, and the protocol in use)
* The old registry tool (`k95regtl.exe`) is back and fixed up for C-Kermit as an
  interim solution until a proper installer is created. This tool lets you 
  create desktop & start menu shortcuts and the .ksc file association.
* The dialer is now included by default with C-Kermit for Windows as changes to
  the SSH options render it incompatible with Kermit 95 (and the Kermit 95
  version incompatible with CKW)
* DECnet support has been re-enabled. You must install a licensed copy of
  Pathworks32 in order to make LAT or CTERM connections.
* SuperLAT support is now available as a custom build option - it is not enabled
  by default due to the unclear license on the publicly available Meridian
  SuperLAT SDK.
* The Telnet Encryption Option (DES/CAST) is supported again, not that you
  should use it if you care about security.

### Fixed Bugs
* Fixed a bug introduced in beta 3 that can prevent the cursor and other 
  elements on the screen with the blink attribute set from blinking reliably
* File transfers over SSH are now _much_ faster
* Fixed the `set mouse activate` command (aka `set terminal mouse`) in K95G -
  previously this command only worked in the old console version.
* Fixed support for the third mouse button in K95G - previously this only worked
  in the old console version. This change may affect your mouse button mapping!
* Fixed a mark being set (and not cleared on mouse button release) on drag when
  the drag event is mapped to `\Kignore`
* Fixed decoding of mouse scroll wheel event coordinates
* The context (right-click) menu in the dialer now works again
* The manual command now works
* The `show mouse` command no longer tries to output non-printable characters 
  bound to mouse events
* SSH will now prompt for a username if the default login userid has been 
  cleared (this is how Kermit 95 behaved)
* Fixed SSH help error: hmac-md5 is not supported, but hmac-sha1 is
* The GUI window now resizes correctly on Windows NT 3.5x

### Minor Enhancements and other changes:
* Upgraded to zlib 1.2.13 fixing [CVE-2022-37434](https://nvd.nist.gov/vuln/detail/CVE-2022-37434)
* Upgraded to openssl 1.1.1u
* The libssh version number is now included in the output of the `show ssh`
  command
* The default k95custom.ini now sets the default browser to nothing which should
  result in Edge being used instead of Internet Explorer. This is just a 
  temporary workaround for the default browser not being correctly picked up.
* Dialer: removed SSH v1 options as CKW now supports SSH 2.0 exclusively
* Dialer: Updated the lists of available ciphers, macs and host key algorithms 
  to match what CKW actually supports

### Source Changes:
* The Dialer now builds with Open Watcom 1.9 and Visual C++ 2.0
* dropped the /ALIGN linker flag which has produced a linker warning since 
  Visual C++ 5.0 SP3 (November 1997)

## C-Kermit for Windows 10.0b4 beta 3 - 14 September 2022
This release focused on improving SSH support, returning SSL support, minor
enhancements, porting to new platforms (NT 3.50, OS/2) and new compilers
(Visual C++ 2.0, Open Watcom 2.0, Open Watcom 1.9 for OS/2)

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
* Fixed terminal scrolling bug in Open Watcom! Builds done with Open Watcom are
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
* Fixed compatibility with the Open Watcom 2.0 fork
* Added support for building with Visual C++ 2.0
* Added support for targeting Windows NT 3.50 with either Open Watcom 1.9 or
  Visual C++ 2.0
* Now builds on OS/2 with Open Watcom 1.9. Only minimal testing has been done.
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
MIPS and PowerPC), and Open Watcom 1.9 support was added to enable future OS/2 
work.

 * Fixed builds with Visual C++ 14.x (2015-2022)
 * Fixed file transfer crash on builds done with Visual C++ 2008 and newer
 * Fixed builds with free versions of Visual C++ that don't include MFC
 * PTY support on Windows 10 v1809 and newer
 * Added Open Watcom 1.9 support (win32 target only)
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
[this file](https://www.kermitproject.org/k95-fixes-since-213.txt) documents 
what's new since Kermit 95 v2.1.3.

Not every change for K95 v2.2 has made it in to C-Kermit for Windows due to the 
removal of some components that could not be open-sourced. In particular,
changes for the Dialer in K95 v2.2 do not apply as the CKW dialer is based on
K95 v2.1.3, and changes for the SSH subsystem don't apply to CKW as CKW uses an
entirely new SSH implementation.

## Previous Kermit 95 releases
Change logs going back to the release of the first version in October 1995 (1.1)

 * [1.1.21 to 2.1.3 Change Log](http://www.columbia.edu/kermit/k95news.html) - 2 April 2002 to 21 January 2003
 * [1.1.20 Changes](https://groups.google.com/g/comp.protocols.kermit.announce/c/gpLy0vTV1Ug/m/hHFQqajRe98J) - 4 April 2000 ([k95news](https://web.archive.org/web/20010405154138/http://www.columbia.edu/kermit/k95news.html))
 * [1.1.19 Changes](https://groups.google.com/g/comp.protocols.kermit.announce/c/uN9G8fp84nY/m/53HTzJvYQdgJ) - 17 February 2000
 * 1.1.18 - Internal CU release
 * [1.1.17 Changes](https://groups.google.com/g/comp.protocols.kermit.announce/c/0mZIfP_LspA/m/cqLPWLsJiFYJ) - 21 June 1998
 * [1.1.16 Changes](https://groups.google.com/g/comp.protocols.kermit.announce/c/8jaYcOv0cvo/m/Er5rCyp_xG8J) - 8 April 1998
 * [1.1.15 Changes](https://groups.google.com/g/comp.os.ms-windows.announce/c/IDbj1Dl16aU/m/WmJlmGtSY5cJ) - 3 October 1997
 * [1.1.14 Changes](https://groups.google.com/g/comp.protocols.kermit.announce/c/KWT_5sYXeC8/m/AGvXUCtXSh4J) - 25 September 1997
 * [1.1.2 to 1.1.13 Change Log](https://web.archive.org/web/19970815161519/http://www.columbia.edu/kermit/k95news.html) - 24 July 1996 to 24 June 1997
   * v1.1.11 of February 1997 restored OS/2 support replacing OS/2 C-Kermit 5A(191)
 * [1.1.6 Announce](https://groups.google.com/g/comp.protocols.kermit.announce/c/Yb8oikR0uuQ/m/kVDydxVQKT4J) - 18 July 1996
 * [1.1.5 Announce](https://groups.google.com/g/comp.protocols.kermit.announce/c/L9wpCZEBw4Q/m/Y89bIR-wp3UJ) - 2 July 1996
 * [1.1.2 to 1.1.4 Change Log (down the bottom)](https://web.archive.org/web/19970815161519/http://www.columbia.edu/kermit/k95news.html) - 18 December 1995 to 7 March 1996
 * 1.1.1 - 3 November 1995:
   * Attempts to remove preloaded entries from Dialer caused a crash
   * Alphabetization of Dialer entries fixed not to be case-sensitive
   * Download directory specification in Dialer no longer ignored
   * Dial prefix no longer also treated as dial suffix by Dialer
   * Kermit BBS Dialer entry fixed to have Backspace key send Backspace
   * Range checking of various numbers by Dialer fixed
   * SET MODEM commands in K95CUSTOM.INI no longer ignored
   * Improved search technique for command files
   * Accuracy of Dialer status line online timer improved
   * ZMODEM downloads fixed to work when FILE COLLISION is BACKUP or RENAME
   * ZMODEM transfers fixed to work over various types of TELNET connections
   * Faster detection of lost connections during file transfer
 * 1.1 - First Release - 2 October 1995
 * [OS/2 C-Kermit 5A(191)](https://www.columbia.edu/kermit/cko191.html) - 23 April 1995
   * Last free release before it was renamed [Kermit 95 for OS/2](https://www.columbia.edu/kermit/os2.html), ported to Windows and sold commercially
 * [OS/2 C-Kermit 5A(190)](https://www.columbia.edu/kermit/os2new.html) - 4 October 1994 
   * Last release to include 16-bit OS/2 1.x support
   * Last release for which source code was publicly available until the Kermit 95 2.2 code was open-sourced in 2011
 * [OS/2 C-Kermit 5A(189)](https://groups.google.com/g/bit.listserv.os2-l/c/BSURfg2ufek/m/GjcIh14Jt_QJ) - 18 July 1993
 * [OS/2 C-Kermit 5A(188) (update)](https://groups.google.com/g/comp.os.os2.apps/c/DesD23imeHI/m/I6-udyEnNhAJ) - 3 February 1993
 * [OS/2 C-Kermit 5A(188)](https://groups.google.com/g/bit.listserv.i-amiga/c/DvS37Mfjj8s/m/sYYcpymJ3woJ) - 23 November 1992
 * [OS/2 C-Kermit 5A(179)](https://groups.google.com/g/fj.kermit/c/rG6d5lpJilI/m/b94gm9h4XgIJ) - 14 February 1992
 * OS/2 C-Kermit 4F(091) (OS/2 Kermit 1.00) - ??
   * Probably not an "official release" - by this point the OS/2 code was just part of the regular C-Kermit codebase
   * Files: [ckoker.doc](http://www.columbia.edu/kermit/ftp/old/ckermit4f/ckoker.doc),
            [ckoker.bwr](http://www.columbia.edu/kermit/ftp/old/ckermit4f/ckoker.bwr),
            [ckoker.boo](http://www.columbia.edu/kermit/ftp/old/ckermit4f/ckoker.boo) (encoded version of kermit.exe, see [ckboo.txt](https://www.columbia.edu/kermit/ftp/boo/ckboo.txt))
 * [OS/2 C-Kermit 4E(070) (OS/2 Kermit 1.00)](https://groups.google.com/g/comp.protocols.kermit/c/BXydCmAjmxo/m/jBWCa9BNkvwJ) - 10 May 1989
   * First release for which the OS/2 code was publicly available. 
 * [OS/2 C-Kermit 4E(070) (OS/2 beta test version 1.0p](https://groups.google.com/g/comp.protocols.kermit/c/M8vYD4F-nBc/m/N4WrA1DpwaIJ) - 15 March 1989
   * For OS/2 1.0+: 
     [ckoker.ann](https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu/pub/kermit/old/misc/ck4e/ckoker.ann),
     [ckoker.boo](https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu/pub/kermit/old/misc/ck4e/ckoker.boo) (encoded version of ckoker.exe, see [ckboo.txt](https://www.columbia.edu/kermit/ftp/boo/ckboo.txt)),
     [ckoker.bwr](https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu/pub/kermit/old/misc/ck4e/ckoker.bwr),
     [ckoker.doc](https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu/pub/kermit/old/misc/ck4e/ckoker.doc)
 * [OS/2 C-Kermit 1.0b](https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu/pub/kermit/old/misc/ck4e/ckoker.ann) - 5 August 1988
 * [OS/2 C-Kermit 1.0a](https://groups.google.com/g/comp.protocols.kermit/c/KZ0P49Za-JA/m/ZpzhtBJOyT4J) - 29 July 1988
