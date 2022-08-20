# Building Optional Dependencies

C-Kermit for Windows has the following optional dependencies. You don't *need* 
these but if you don't have them some features (like built-in SSH) will be 
unavailable. These are:

* [zlib](https://zlib.net/) - required for compression, ssl (https) and ssh
* [OpenSSL](https://www.openssl.org/) - required for ssl and ssh
* [libssh](https://www.libssh.org/) - required for ssh

This doesn't necessarily document the *best* way to build these dependencies.
Ideally you should read the readme file and other documentation for each of 
these if that's your goal. But that takes a while and if you're just looking to
do a full build of C-Kermit for Windows with all features turned on these
instructions should do the job.

This has been tested against the following versions:
* zlib 1.2.12
* OpenSSL 1.1.1q
* libssh 0.9.6

And to build it all the following tools should work:
* Visual C++ (2022 community edition works)
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
   - libssh\
     - 0.9.6\
       - files & directories from libssh 0.9.6
```

Once you've built the dependencies you want you can uncomment the associated
lines in setenv.bat which will result in any features requiring the dependencies
you've built to be automatically enabled.

## 1. Building zlib

zlib is easy. Download zlib-1.1.12.tar.gz and extract it as `zlib\1.1.12`. Then:
```
cd zlib\1.1.12
cmake .
nmake -f win32\Makefile.msc
cd ..\..\
```

## 2. Building OpenSSL

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

## 3. Building libssh

For libssh you need to the following specifying the correct OPENSSL_ROOT_DIR and ZLIB_ROOT:
```
cd libssh\0.9.6
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DOPENSSL_ROOT_DIR=C:\path\to\ckwin\openssl\1.1.1q\ -DZLIB_ROOT:PATH=C:\path\to\ckwin\zlib\1.2.12\
nmake
cd ..\..\..\
```

Note that this does not build libssh with GSSAPI support.