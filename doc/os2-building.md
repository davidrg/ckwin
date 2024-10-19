# Building Kermit 95 for OS/2 (Kermit/2) from Source

Kermit 95 for OS/2 was previously built with IBM C/C++ 3.6 (included with
IBM VisualAge C/C++ 4.0), and _may_ still be buildable with this or, with
some makefile adjustments, earlier versions down to C/C++ 2.0.

The Kermit Project no longer has access to any of these compilers and
compatibility has not been tested since 2002 so the build process is unlikely
to be smooth. If you have access to IBM C/C++ 2.0+ it may be worth trying 
([let us know how you get on](https://github.com/davidrg/ckwin/discussions/new?category=general) 
if you do), but using Open Watcom 1.9 will be easiest. The Open Watcom 2.0
fork may also work, but it has not been tested.

## Cross-compiling from Windows with Open Watcom
This is the recommended method for building Kermit 95 for OS/2.

### Build Process

Firstly put the source code somewhere convenient. Where ever you put it, you
need to edit `setenv.bat` in the root of the folder to point to your source
code location. Uncomment the line `REM set root=C:\src` near the top of the
file and set the value to where your source code lives, for example
`set root=C:\dev\ckwin`, then save the file.

There are no further dependencies you need to build as OpenSSL (and therefore 
libssh) does not support Open Watcom.

### Build Kermit 95 for OS/2

1. Open a console and change directory to where the source code is located
2. Open Watcom build environment by running `owsetenv.bat` (in `C:\WATCOM` by default)
3. Setup the Kermit 95 build environment by running `setenv.bat`
4. CD into `kermit\k95`
5. run `mkos2.bat` for a release build or `mkos2d.bat` for a debug build
6. run `mkdist-os2.bat` to collect everything into the dist-os2 subdirectory

This should leave you with a collection of 32-bit OS/2 executables in the
dist-os2 subdirectory:

| Executable   | Description                                      |
|--------------|--------------------------------------------------|
| k2.exe       | Kermit 95 for OS/2 executable                     |
| cko32i20.dll | TCP/IP Library                                   |
| k2clip.exe   | PM Clipboard server                              |
| textps.exe   | Convert plain text to PostScript                 |
| telnet.exe   | Telnet stub (launches k2.exe as a telnet client) |
| rlogin.exe   | RLogin stub (launches k2.exe as a rlogin client) |


## Building on OS/2 with Open Watcom

The recommended method is to cross-compile from Windows as this is the only
method that is tested regularly. Doing builds *on* OS/2 is really only supported
for debugging purposes at this time.

The build process on OS/2 is much the same as on Windows, the only real
difference is there is no mkdist-os2 script; you've got to collect and rename 
the build outputs yourself.

Firstly, edit `setenv.cmd`, uncomment the line `REM set root=C:\src` near the
top of the file and set the value to where your source code lives then save the
file. For example, `set root=C:\dev\ckwin`.

Next, run `C:\watcom\owsetenv.cmd` followed by the Kermit 95 `setenv.cmd` script.
You should end up in `\kermit\k95`. To build, run either `mkos2.cmd` for a
release build or `mkos2d.cmd` for a debug build.
