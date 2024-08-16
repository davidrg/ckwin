# Building Optional Dependencies

C-Kermit for Windows has the following optional dependencies. You don't *need* 
these but if you don't have them some features (like built-in SSH) will be 
unavailable. These are:

* [zlib](https://zlib.net/)
* [OpenSSL](https://www.openssl.org/) 0.9.8 - 3.1.x
* [libssh](https://www.libssh.org/) 0.9.x, 0.10.x
* libdes 4.01

For SSH support the first three are required and the openssl version should 
be >=1.1.1 (though apparently 1.0.2 and 1.1.0 also work but have known security 
issues). 

You may also want a CA certificates bundle. A convenient source is here:
https://curl.se/docs/caextract.html. Save the file as `ca_certs.pem` in the CKW 
directory and it should be picked up automatically.

## Building with SSH and SSL/TLS Support

This doesn't necessarily document the *best* way to build these dependencies.
Ideally you should read the readme file and other documentation for each of 
these if that's your goal. But that takes a while and if you're just looking to
do a full build of C-Kermit for Windows with all features turned on these
instructions should do the job.

This has been tested against the following versions:
* zlib 1.2.13
* OpenSSL 1.1.1v, 3.0.10, 3.1.1
* libssh 0.9.6, 0.10.1, 0.10.3, 0.10.5, 0.10.6

And to build it all the following tools should work:
* Visual C++ (2022 community edition works, or for Windows XP compatibility use 2019)
* cmake - required for zlib and libssh (version 3.22 tested)
* [NASM](https://www.nasm.us/) - required for OpenSSL (2.15.05 tested)
* Perl ([StarwBerry Perl](https://strawberryperl.com/) 5.32.1.1 tested) - required for OpenSSL

You'll need a Visual C++ command prompt to do all of this in with Perl and NASM 
in your path. Something like this should do the job:
```
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64_x86 -vcvars_ver=14.3
set path=C:\Program Files\NASM;%PATH%
set path=C:\strawberry\perl\bin;%PATH%
```

Normally everything is arranged into directories as follows:
```
 ckwin\
   - setenv.bat
   - kerberos\
     - kfw-2.6-final\src
       - Kerberos for Windows source code (athena, doc, scripts)
     - build26.bat
   - kermit\
     - k95\
       - C-Kermit for windows source code
   - zlib\
     - 1.2.13\
       - files & directories from zlib 1.1.13
   - openssl\
     - 3.0.10\
       - files & directories from openssl 3.0.10
     - build.bat
     - README.md
   - libdes\
     - des\
       - files & directories from the libdes distribution
   - libssh\
     - 0.10.6\
       - files & directories from libssh 0.10.6
   - superlat\
     - include\
       - SuperLAT header files
```

Once you've built the dependencies you want you can alter the associated lines
in setenv.bat to point to where you've built the dependencies which will result
in any features requiring them to be automatically enabled.

### 1. Building zlib

zlib is easy. Download zlib-1.1.13.tar.gz and extract it as `zlib\1.1.13`. Then:
```
cd zlib\1.1.13
cmake .
nmake -f win32\Makefile.msc
cd ..\..\
```

If using an older compiler like Visual C++ 6 and don't have cmake handy you can
just skip that step. Running `nmake -f win32\Makefile.msc` seems to work fine
without it.

### 2. Building OpenSSL

Before building OpenSSL for the first time you need to install `Text::Template`
from CPAN with: `cpan -i Text::Template`

Then you can build OpenSSL with the following (adjusting the zlib include path
as necessary):
```
cd openssl\3.0.10
perl Configure VC-WIN32 zlib-dynamic --with-zlib-include=C:\path\to\ckwin\zlib\1.2.13
nmake
cd ..\..\
```

If you're building for something other than 32bit x86, you will need to replace VC-WIN32
with one of the following:

| Architecture | Target         |
|--------------|----------------|
| x86 32bit    | `VC-WIN32`     |
| x86 64bit    | `VC-WIN64A`    |
| Itanium      | `VC-WIN64I`    |
| ARM 32bit    | `VC-WIN32-ARM` |
| ARM 64bit    | `VC-WIN64-ARM` |

If you're cross-compiling (your target architecture is not the same as the machine you're
building on) you *may* get a link error in some versions of OpenSSL. This has been observed
primarily cross-compiling from x86 to Itanium and x86-64 with Visual C++ 2010. If this
occurs you need to open `makefile` in a text editor, find the line beginning with
`LDFLAGS=/nologo` and add either `/machine:ia64` (Itanium) or `/machine:x64` (x86-64) to the
end of it. Save the file and run the build again.

If you want OpenSSL to work on versions of windows older than Vista, add the
`-D"_WIN32_WINNT=0x502"` parameter to the Configure step.

To help automate this a little you can try using `openssl\build.bat` which is
[documented here](../openssl/README.md). This script uses the C-Kermit build environment
to figure out the appropriate target and zlib path then runs the configure and make
step.

### 3. Building libssh

For libssh you need to the following specifying the correct OPENSSL_ROOT_DIR and ZLIB_ROOT:
```
cd libssh\0.10.5
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DOPENSSL_ROOT_DIR=C:\path\to\ckwin\openssl\3.0.10\ -DZLIB_ROOT:PATH=C:\path\to\ckwin\zlib\1.2.13\
nmake
cd ..\..\..\
```

Note that this does not build libssh with GSSAPI support. If you're building
libssh 0.10.x and want DSA support (ssh-dss), add `-DWITH_DSA=ON` to the end
of the cmake command.

#### Building with GSSAPI (Kerberos) support

Libssh does not currently support GSSAPI on windows, though only a very
minor change is required to make it work: A single `#define` needs to be added
to the top of `gssapi.c` to ensure the correct version of the winsock header is
included, and the `FindGSSAPI.cmake` CMake module needs to be adjusted so it can
find the MIT Kerberos for Windows SDK.

These changes are available in the form of a convenient patch: 
`libssh/win32-gssapi.patch`

To build with GSSAPI support:
```
cd libssh\0.10.5
patch -p1 < ..\win32-gssapi.patch
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DOPENSSL_ROOT_DIR=C:\path\to\ckwin\openssl\3.0.10\ -DZLIB_ROOT:PATH=C:\path\to\ckwin\zlib\1.2.13\ -DGSSAPI_ROOT_DIR="C:\Program Files\MIT\Kerberos"
nmake
cd ..\..\..\
```

Remember to add `-DWITH_DSA=ON` if you still need DSA (ssh-dss) support
(requires libssh 0.10.x or older).

*Note:* The resulting ssh.dll will depend on gssapi64.dll (or gssapi32.dll for a
32bit build) meaning it won't work on any systems that don't have MIT Kerberos
for Windows installed. If you're distributing your build to end-users who may or
may not need Kerberos support it may be best to produce two builds of libssh, one
with GSSAPI support and one without.

#### Building the easy way

To make things easier, a batch file is provided (`libssh\build.bat`) which is
[documented here](../libssh/README.md). This batch fly.ile supports applying
patches to enable GSSAPI and (for 0.10.6) Windows XP support

#### Building with Dynamic SSH support

C-Kermit for Windows can optionally be built to load its SSH backend from a DLL
on startup. This means that:

* If LibSSH can't be found or can't be loaded for some reason, C-Kermit can
  still start up with SSH features disabled
* Multiple SSH backends can be provided (eg, with and without GSSAPI/Kerberos 
  support) and C-Kermit will use the first one that loads successfully. This
  saves the user having to swap around Kerberos and non-Kerberos (or Windows XP
  and non-Windows XP) versions of LibSSH manual
* Alternative SSH backends possibly not based on LibSSH could be provided by the
  user 
* C-Kermit run without SSH support by starting it with the `-#2` command line
  argument (disable loading of optional network DLLs) and SSH can then be loaded
  later when needed with the `ssh load` command.

To build with Dynamic SSH support, run `set CKF_DYNAMIC_SSH=yes` after running
`setenv.bat`. If you'd also like to disable building of the LibSSH backend DLLs,
run `set CKF_SSH_BACKEND=no`. If you'd like to build your own SSH backend using
something other than libssh, see `ckonssh.c` for a starting point (this is a
"null" backend that implements all the required APIs but otherwise does 
nothing).

Normally the C-Kermit build process will build a single SSH backend, 
`k95ssh.dll`, linked against ssh.dll. If you'd like to build multiple backends
against different variants of LibSSH (such as one with GSSAPI support and one 
without) then LibSSH needs to be built with different library names, and you
need to do the same with C-Kermits SSH backend.

To Build LibSSH in this way, do something like the following:
```
build /C /R /M out /W libssh-0.10.6
build /C /R /M out /W /N g /G /K C:\dev\ckwin\kerberos\kfw41 libssh-0.10.6
build /C /R /M out /X /N x libssh-0.10.6
build /C /R /M out /X /N gx /G /K C:\dev\ckwin\kerberos\kfw41 libssh-0.10.6
```

Where:

* The `/M out` parameter moves the build artifacts (`ssh.dll` and `ssh.lib`) into
  the `out` subdirectory
* The `/N` parameter (eg, `/N gx`) adds a suffix (eg, `gx`) to the built
  artifacts resulting in, for example, `sshgx.dll` and `sshgx.lib`
* The `/X` parameter applies the Windows XP patch, while the `/W` parameter
  removes it if it was previously applied
* The `/G` parameter applies the GSSAPI patch if needed
* The `/K` parameter specifies the path to the Kerberos for Windows SDK

The result of running the four libssh builds above is:

| DLL name  | LIB name  | GSSAPI | XP  | Description                                                        |
|-----------|-----------|--------|-----|--------------------------------------------------------------------|
| ssh.dll   | ssh.lib   | no     | no  | For Vista and newer, no GSSAPI, no GSSAPI support                  |
| sshg.dll  | sshg.lib  | yes    | no  | For Vista and newer, requires Kerberos for Windows to be installed |
| sshx.dll  | sshx.lib  | no     | yes | For Windows XP, no GSSAPI support.                                 |
| sshgx.dll | sshgx.lib | yes    | yes | For Windows XP, requires Kerberos for Windows to be installed      |

These are the four variants of libssh that C-Kermit ships with. As a result,
`setenv.bat` will detect them automatically as long as they're placed in the
`out` directory (`/M out`), and the script for building the distribution will
pick them up automatically.

Only the standard backend using `ssh.dll` (`k95ssh.dll`) is built by default. To
build the other three, you need to do something like the following (or run 
`mksshdll-all.bat`):

```
del k95ssh*.dll
del k95ssh.res

REM Windows Vista and newer, GSSAPI-enabled (sshg.dll, k95sshg.dll)
set SSH_LIB=sshg.lib
set CKF_SSH_DLL_VARIANT=g
call mksshdll.bat
ren k95ssh.dll k95sshg.dll
del k95ssh.res

REM Windows XP only, not GSSAPI-enabled (sshx.dll, k95sshx.dll)
set SSH_LIB=sshx.lib
set CKF_SSH_DLL_VARIANT=x
call mksshdll.bat
ren k95ssh.dll k95sshx.dll
del k95ssh.res

REM Windows XP only, GSSAPI-enabled (sshgx.dll, k95sshgx.dll)
set SSH_LIB=sshgx.lib
set CKF_SSH_DLL_VARIANT=gx
call mksshdll.bat
ren k95ssh.dll k95sshgx.dll
del k95ssh.res

REM Windows Vista and newer, not GSSAPI-enabled (ssh.dll, k95ssh.dll)
set SSH_LIB=ssh.lib
set CKF_SSH_DLL_VARIANT=
REM built as part of the regular build process
```

Here we're specifying which variant of libssh to build against (the `SSH_LIB`
environment variable), while the `CKF_SSH_DLL_VARIANT` environment variable
adds a string to the resulting DLLs description.

## Building with Telnet Encryption Option (DES and CAST) Support
In addition to SSL/TLS secured telnet, C-Kermit for Windows also optionally
supports the [Telnet Encryption Option](https://www.rfc-editor.org/rfc/rfc2946.html)
using the long obsolete DES and CAST encryption algorithms. This relies on
libdes, a very old crypto library. Needless to say the security here will not be
great, but perhaps it's better than entirely unencrypted telnet (assuming you 
can find a telnet _server_ that supports the Encryption Option).

This feature currently requires at least Visual C++ 5.0 - OpenWatcom and earlier
versions of Visual C++ are not supported due to makefile incompatibilities.

To enable the Encryption Option, simply build libdes. To do this:
1. Grab libdes from somewhere. [This version](http://www.mirrorservice.org/sites/ftp.wiretapped.net/pub/security/cryptography/libraries/libdes/libdes.tar.gz)
   from early 1998 calling itself 4.01 works.
2. extract the libdes distribution  to `/libdes/des` such that the file `des.h` 
   exists at `/libdes/des/des.h`.
3. run `mknt.bat` inside of `/libdes`

When you build CKW, libdes should be detected and the support library 
`k95crypt.dll` will be built.

## Building with Meridian SuperLAT support

On Windows NT, C-Kermit for Windows can be built with support for making LAT 
connections through [SuperLAT](https://web.archive.org/web/20000619044544/http://www.meridian.com/superlat.html)
by Meridian Technology Corporation. Standard builds for vintage Windows provided
by The Kermit Project do not include SuperLAT due to the SDK not being freely
available under an open-source license.

But you can build it yourself easily enough if you need it! If the SuperLAT 
headers are found in `\superlat\include` and you're building with Visual C++
2005 or older SuperLAT support will be automatically enabled when you run the
`setenv.bat` script.

The specific header files required to be in this directory are: `LATIOC.H`, 
`NETTYPES.H`, `NTDDTDI.H`, `PACKOFF.H`, `PACKON.H`, `TDI.H`, `TIHDR.H`.

These can be obtained from the SuperLAT SDK. If you don't have a copy of that,
they can also be found in the `TESTSVC` directory inside the 
[slatfio.zip](https://web.archive.org/web/20000929005919/http://www.meridian.com/slatfio.zip)
archive originally distributed by Meridian on their website ("SuperLAT File I/O" 
on [this page](https://web.archive.org/web/20010830141239/http://www.meridian.com/DownloadSL.html)).

To actually use SuperLAT support, you'll need a licensed copy of the SuperLAT
product. This has not been commercially available or supported since 
31 December 2000 so is probably extremely difficult to find today.

## Building with Kerberos Support

C-Kermit for Windows is known to build with Kerberos for Windows version 2.6.0
(the last version to support Windows 95) and newer. If you need Kerberos IV
support, use KFW 2.x or 3.x - KFW 4.x is Kerberos V only.

Kermit 95 was built with Kerberos for Windows version 2.2-beta2 so there is a
good chance building with versions as old as that will work too, though this has
not been tested in over 20 years.

### Kerberos for Windows 3.x, 4.x

The Kerberos for Windows SDK is sufficient. This won't give you DNS-SRV support
as that currently relies on a library that isn't included in the SDK.

All you've got to do is download and install/unzip the KFW SDK then point
`setenv.bat` at the KFW SDK directory (where the lib and include/inc 
subdirectories are).

You can do this by either editing `setenv.bat` and updating the `k4w_root`
variable, or you can set the `k4w_root_override` environment variable before
invoking setenv.bat. For example:

```batch
C:\dev\ckwin> set k4w_root_override=C:\Program Files\MIT\Kerberos
C:\dev\ckwin> setenv.bat
```

### Kerberos for Windows 2.6.0 - from source (provides DNS-SRV support)

And likely the rest of the 2.6.x series.

#### Requirements
* Visual C++ 6.0 or newer, with the August 2001 or newer Platform SDK. 
  Visual C++ 2002 and 2003 seem to satisfy this requirement fine
* Microsoft HTML Help compiler is required. This should be available on the
  Visual C++ CD, or you can [download it from Microsoft](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/htmlhelp/microsoft-html-help-downloads)
* A selection of UNIX tools in your PATH: perl, sed, gawk, cat and rm. 
  Strawberry perl seems fine, and the rest can come from GnuWin32.

#### Build
The build scripts need to be patched as the distributed versions rely on some
headers and tools that don't appear to be available outside of MIT. To automate
this (and the rest of the build), a script is p[r]

To build:
1. Download the source code for desired Kerberos for Windows release, eg
   [Version 2.6.0](https://web.mit.edu/kerberos/dist/historic.html#KFW2.6)
   (kfw-2.6-src-final.zip)
2. Extract the zip file inside the kerberos directory, creating
   \kerberos\kfw-2.6-final\src
3. Add the following to your PATH: perl, sed, gawk, cat, rm and the HTML Help
   compiler
4. CD into the src directory and run ..\..\build26.bat

Example:
```
cd kerberos
wget https://web.mit.edu/kerberos/dist/kfw/2.6/kfw-2.6/kfw-2.6-src-final.zip
unzip kfw-2.6-src-final.zip
cd kfw-2.6-final\src
..\..\build26.bat
```

## Building with Older OpenSSL Versions
If you want to build with older **_INSECURE_** versions of OpenSSL for some
reason, C-Kermit for Windows still supports the following:

* 0.9.8zf of 2016-01-29 (**_INSECURE_**)
* 1.0.0s of 2016-01-29 (**_INSECURE_**)
* 1.0.1u of 2016-09-22 (**_INSECURE_**)
* 1.0.2u of 2019-12-20 (**_INSECURE_** unless you pay for premium OpenSSL support)
* 1.1.0l of 2019-09-10 (**_INSECURE_**)
* 1.1.1v of 2023-08-01 (End of life from 11 September 2023 unless you pay for premium OpenSSL support)

This is of course not recommended. But perhaps bad encryption is better than
none at all in some situations. Or maybe you're not doing anything where
security matters all that much.

Firstly, build zlib as described above. You'll also want NASM on your PATH. For 
OpenSSL 0.9.8, NASM 0.98 should work. For newer versions, NASM 2.15.05 seems to 
work fine.

### OpenSSL 0.9.8zf
Extract to `\openssl\0.9.8`, update `openssl_root` in your `setenv.bat`, then
do the following:

```
cd openssl\0.9.8
perl configure VC-WIN32
ms\do_ms
nmake -f ms\ntdll.mak
```

To build an optimised version, use `ms\do_nasm` instead of `ms\do_ms`.

### OpenSSL 1.0.0s
Extract to `\openssl\1.0.0`, update `openssl_root` in your `setenv.bat`, then
do the following

```
cd openssl\1.0.0
perl configure VC-WIN32 enable-static-engine -DOPENSSL_USE_IPV6=0
ms\do_nasm
nmake -f ms\ntdll.mak
```

For some older versions of Visual C++, including Visual C++ 6, the additional
`-DOPENSSL_USE_IPV6=0` parameter is required as IPv6 isn't supported properly.
For newer compilers you should omit it.

### OpenSSL 1.0.1u
Extract to `\openssl\1.0.1`, update `openssl_root` in your `setenv.bat`, then
do the following

```
cd openssl\1.0.1
perl configure VC-WIN32 enable-static-engine zlib-dynamic --with-zlib-include=C:\path\to\ckwin\zlib\1.2.12 -DOPENSSL_USE_IPV6=0
ms\do_ms
nmake -f ms\ntdll.mak
```

For some older versions of Visual C++, including Visual C++ 6, the additional
`-DOPENSSL_USE_IPV6=0` parameter is required as IPv6 isn't supported properly.
For newer compilers you should omit it.

### OpenSSL 1.0.2u

If you're paying for OpenSSL Premium Support (US$50k/year), you should have
access to newer versions of OpenSSL 1.0.2 aren't full of known security
vulnerabilities. C-Kermit for windows hasn't been tested against anything newer
than the final public release (1.0.2u) but it should work with later patch
levels.

Extract to `\openssl\1.0.2`, update `openssl_root` in your `setenv.bat`, then
do the following

```
cd openssl\1.0.2
perl configure VC-WIN32 enable-static-engine zlib-dynamic --with-zlib-include=C:\path\to\ckwin\zlib\1.2.12 -DOPENSSL_USE_IPV6=0
ms\do_ms
nmake -f ms\ntdll.mak
```

For some older versions of Visual C++, including Visual C++ 6, the additional
`-DOPENSSL_USE_IPV6=0` parameter is required as IPv6 isn't supported properly.
For newer compilers you should omit it.

To build with Visual C++ 6, you may have to edit `Configure` and remove `-WX`
from line 596:
```
"VC-WIN32","cl:-W3 -WX -Gs0 -GF -Gy -nologo -DOPENSSL_SYSNAME_WIN32 -DWIN32_LEAN_AND_MEAN -DL_ENDIAN -D_CRT_SECURE_NO_DEPRECATE -D_WINSOCK_DEPRECATED_NO_WARNINGS:::WIN32::BN_LLONG RC4_INDEX EXPORT_VAR_AS_FN ${x86_gcc_opts}:${x86_asm}:win32n:win32",
```

### OpenSSL 1.1.0l
Extract to `\openssl\1.1.0`, update `openssl_root` in your `setenv.bat`, then
do the following

```
cd openssl\1.1.0
perl Configure VC-WIN32 zlib-dynamic --with-zlib-include=C:\path\to\ckwin\zlib\1.2.12
nmake
cd ..\..\
```

This version does not build with Visual C++ 6.

### OpenSSL 1.1.1v

The process for this is the same as 3.0.x. Extract to `\openssl\1.1.1v`, 
update `openssl_root` in your `setenv.bat`, then do the following

```
cd openssl\1.1.1v
perl Configure VC-WIN32 zlib-dynamic --with-zlib-include=C:\path\to\ckwin\zlib\1.2.13
nmake
cd ..\..\
```

This version does not build with Visual C++ 6.