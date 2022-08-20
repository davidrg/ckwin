# Building C-Kermit for OS/2 from Source

Cross-compiling from Windows using OpenWatcom 1.9 is the only method supported
at this time. The process is largely the same as building for Windows with that 
compiler and quite easy.

## Build Process

Firstly put the source code somewhere convenient. Where ever you put it, you
need to edit `setenv.bat` in the root of the folder to point to your source
code location. Uncomment the line `REM set root=C:\src` near the top of the
file and set the value to where your source code lives, for example
`set root=C:\dev\ckwin`, then save the file.

There are no further dependencies you need to build as OpenSSL (and therefore 
libssh) does not support OpenWatcom.

### Build C-Kermit for OS/2

1. Open a console and change directory to where the source code is located
2. OpenWatcom build environment by running `owsetenv.bat` (in `C:\WATCOM` by default)
3. Setup the C-Kermit build environment by running `setenv.bat`
4. CD into `kermit\k95`
5. run `mkos2.bat` for a release build or `mkos2d.bat` for a debug build

This should leave you with a collection of 32-bit OS/2 executables in the
current directory:

| Executable   | Description                                     |
|--------------|-------------------------------------------------|
| ckoker32.exe | C-Kermit for OS/2 executable - rename to k2.exe |
| cko32i20.dll | TCP/IP Library                                  |
| ckoclip.exe  | PM Clipboard server?                            |
| otextps.exe  | Convert plain text to PostScript                |
| osetup.exe   | Setup program?                                  |
| otelnet.exe  | Telnet stub - rename to telnet.exe              |
| orlogin.exe  | RLogin stub - rename to rlogin.exe              |

 

