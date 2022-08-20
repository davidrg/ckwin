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
6. run `mkdist-os2.bat` to collect everything into the dist-os2 subdirectory

This should leave you with a collection of 32-bit OS/2 executables in the
dist-os2 subdirectory:

| Executable   | Description                                      |
|--------------|--------------------------------------------------|
| k2.exe       | C-Kermit for OS/2 executable                     |
| cko32i20.dll | TCP/IP Library                                   |
| k2clip.exe   | PM Clipboard server                              |
| textps.exe   | Convert plain text to PostScript                 |
| telnet.exe   | Telnet stub (launches k2.exe as a telnet client) |
| rlogin.exe   | RLogin stub (launches k2.exe as a rlogin client) |

 

