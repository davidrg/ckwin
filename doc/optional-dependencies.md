# Building Optional Dependencies

C-Kermit for Windows has the following optional dependencies. You don't *need* 
these but if you don't have them some features (like built-in SSH) will be 
unavailable. These are:

* [zlib](https://zlib.net/)
* [OpenSSL](https://www.openssl.org/) 0.9.8 - 3.0.x
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
* zlib 1.2.12
* OpenSSL 1.1.1q
* libssh 0.9.6, 0.10.1, 0.10.3

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
   - kermit\
     - k95\
       - C-Kermit for windows source code
   - zlib\
     - 1.2.12\
       - files & directories from zlib 1.1.12
   - openssl\
     - 1.1.1q\
       - files & directories from openssl 1.1.1q
   - libdes\
     - des\
       - files & directories from the libdes distribution
   - libssh\
     - 0.9.6\
       - files & directories from libssh 0.9.6
   - superlat\
     - include\
       - SuperLAT header files
```

Once you've built the dependencies you want you can alter the associated lines
in setenv.bat to point to where you've built the dependencies which will result
in any features requiring them to be automatically enabled.

### 1. Building zlib

zlib is easy. Download zlib-1.1.12.tar.gz and extract it as `zlib\1.1.12`. Then:
```
cd zlib\1.1.12
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
cd openssl\1.1.1q
perl Configure VC-WIN32 zlib-dynamic --with-zlib-include=C:\path\to\ckwin\zlib\1.2.12
nmake
cd ..\..\
```

If you want OpenSSL to work on versions of windows older than Vista, add the
`-D"_WIN32_WINNT=0x502"` parameter to the Configure step.

### 3. Building libssh

For libssh you need to the following specifying the correct OPENSSL_ROOT_DIR and ZLIB_ROOT:
```
cd libssh\0.9.6
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DOPENSSL_ROOT_DIR=C:\path\to\ckwin\openssl\1.1.1q\ -DZLIB_ROOT:PATH=C:\path\to\ckwin\zlib\1.2.12\
nmake
cd ..\..\..\
```

Note that this does not build libssh with GSSAPI support. If you're building
libssh 0.10.x and want DSA support (ssh-dss), add `-DWITH_DSA=ON` to the end
of the cmake command.

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

## Building with Older OpenSSL Versions
If you want to build with older **_INSECURE_** versions of OpenSSL for some
reason, C-Kermit for Windows still supports the following:

* 0.9.8zf of 2016-01-29 (**_INSECURE_**)
* 1.0.0s of 2016-01-29 (**_INSECURE_**)
* 1.0.1u of 2016-09-22 (**_INSECURE_**)
* 1.0.2u of 2019-12-20 (**_INSECURE_** unless you pay for premium OpenSSL support)
* 1.1.0l of 2019-09-10 (**_INSECURE_**)

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
