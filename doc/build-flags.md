# K95 Build Flags

The K95 build system will automatically try to turn on all features the current
compiler and available 3rd party libraries will support. If you'd like to turn
off some features that would otherwise be on automatically, or otherwise
customise the build you can set a few environment variables before running
a build.

## Flags controlling features

These turn individual features on and off. Set to `yes` to enable or `no` to
disable before running a build (eg, `set CKF_SSH=no`). 

Features relying on libraries are turned on automatically when the required
library is found by `setenv.bat`. Other features are turned off automatically
when the compiler in use is too old to support that feature.

| Flag               | Default                                         | Description                                                                                            |
|--------------------|-------------------------------------------------|--------------------------------------------------------------------------------------------------------|
| CKF_BETATEST       | Yes                                             | Produce a beta test build (affects some strings and logging)                                           |
| CKF_CONPTY         | Yes (Visual C++ 2019+)                          | Enable PTY support on Windows 10 1809 or newer                                                         |
| CKF_CRYPTDLL       | Yes (if libdes is found)                        | Enables building of k95crypt.dll for Telnet DES and CAST encryption                                    |
| CKF_DEBUG          | Yes                                             | Enables debug logging.                                                                                 |
| CKF_DECNET         | Yes (if x86 or Alpha, Visual C++ 2019 or older  | DECnet LAT/CTERM support (pathworks32)                                                                 |
| CKF_INTERNAL_CRYPT | No                                              | Enables built-in DES/CAST encryption (statically linked k95crypt.dll). CKF_CRYPTDLL must be set to no. |
| CKF_K4W            | Yes (if Kerberos for Windows is found)          | Enables Kerberos support                                                                               |
| CKF_LOGIN          | Yes (Visual C++ 6.0+)                           |                                                                                                        |
| CKF_MOUSEWHEEL     | Yes (Visual C++ 6.0+)                           | Mouse wheel support in the terminal                                                                    |
| CKF_NETBIOS        | Yes (if OS/2 and not Watcom)                    | NetBIOS support. OS/2 only. Currently disabled on OS/2 due to bugs                                     |
| CKF_NO_CRYPTO      | No                                              | Disables SSH, SSL, Telnet encryption, SRP and Kerberos                                                 |
| CKF_NT_UNICODE     | No                                              | Target Windows NT Unicode APIs. Does not work (yet)                                                    |
| CKF_NTLM           | Yes (Visual C++ 6.0+)                           | NTLM Authentication support                                                                            |
| CKF_RICHEDIT       | Yes (if Visual C++ 4.0+)                        | Enables the rich edit control (requires NT 3.51/95 or newer)                                           |
| CKF_SRP            | Yes (if libsrp is found, unless Watcom on OS/2) | SRP authentication support                                                                             |
| CKF_SSH            | Yes (if Visual C++ and libssh is found)         | Enable built-in SSH support (via libssh)                                                               |
| CKF_SSL            | Yes (if openssl is found)                       | Enables SSL/TLS support for telnet, http and ftp                                                       |
| CKF_SUPERLAT       | Yes (if SuperLAT headers are found)             | Enables support for Meridian SuperLAT                                                                  |
| CKF_TAPI           | Yes (if Visual C++ 4.0+ and not MIPS)           | TAPI support on Windows 95/NT4+. Doesn't build on MIPS.                                                |
| CKF_TOOLBAR        | Yes (if Visual C++ 4.0+)                        | Enables the toolbar in K95G (requires NT 3.51/95)                                                      |
| CKF_XYZ            | Yes (unless Watcom on OS/2)                     | X/Y/Z MODEM support (via p95.dll). Disabled on OS/2 with Watcom due to build errors                    |
| CKF_ZLIB           | Yes (if zlib is found)                          | Enables zlib support                                                                                   |
| CKF_DYNAMIC_SSH    | No (unless Watcom)                              | Builds with the SSH subsystem in a DLL loadable at runtime                                             |
| CKF_SSH_BACKEND    | No (if Watcom)                                  | If set to No and CKF_DYNAMIC_SSH is set to Yes then the SSH backend (k95ssh.dll) won't be built        |

### Crypt DLL

Telnet DES/CAST encryption, when built, is usually supproted via k95crypt.dll.
If you'd rather get rid of the DLL and instead statically link this functionality
into k95g.exe, k95.exe and iksdnt.exe, set:

```
set CKF_CRYPTDLL=no
set CKF_INTERNAL_CRYPT=yes
```

## Target Platform

These are set automatically based on detected compiler. These all require either
Visual C++ 1.0 32bit, Visual C++ 2.0 or OpenWatcom.

| Flag     | Default                          | Description                                                              |
|----------|----------------------------------|--------------------------------------------------------------------------|
| CKT_NT35 | Yes (if Visual C++ 2.0)          | Don't use APIs newer than supported by NT 3.50. Requires Visual C++ 2.x. |
| CKT_NT31 | Yes (if Visual C++ 2.0 or older) | Don't use APIs newer than supported by NT 3.1                            |

When both CKT_NT35 and CKT_NT31 are set, code is generated for both platforms.
At runtime the appropriate API selections will be made automatically allowing
the generated binary to work on either NT 3.1 or 3.50.

Setting either CKT_NT35 or CKT_NT31 disables the following features:
* CKF_TAPI
* CKF_RICHEDIT
* CKF_TOOLBAR
* CKF_LOGIN
* CKF_NTLM

OpenWatcom by default will target NT 3.51 and newer but you can
`set CKT_NT35=yes` to target NT 3.50 instead. Builds with OpenWatcom don't
currently work on NT 3.1 regardless of whether CKT_NT31 is set or not.
