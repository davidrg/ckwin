# Building Kermit 95 for Windows with MinGW

There is limited support for building K95 using the MinGW toolchain. This support is
provided primarily to allow development under Linux without having to run a compiler
under a Virtual Machine. As a result there aren't any automatic build configuration 
or distribution scripts provided, just the minimum required to build the core 
application.

The instructions here only cover cross-compiling on Linux. It should be possible
to do this on Windows too using msys2 but on Windows you're really much better off
just using the free Visual Studio Community Edition as that's what's used for 
producing release builds and the build process is much more automated.

On Debian/Ubuntu you'll need to install the MinGW cross-compiler:
```shell
sudo apt install g++-mingw-w64-x86-64-posix gcc-mingw-w64-x86-64-posix binutils-mingw-w64-x86-64
```

## Minimal Build
To build the application without SSL/TLS or SSH support:
```shell
export CXX=/usr/bin/x86_64-w64-mingw32-c++
export CC=/usr/bin/x86_64-w64-mingw32-gcc

git clone https://github.com/davidrg/ckwin.git
cd ckwin/kermit/k95
make RC=/usr/bin/x86_64-w64-mingw32-windres
```


## Full Build

To build the application with SSL/TLS support, do something like the following:
```shell
export CXX=/usr/bin/x86_64-w64-mingw32-c++
export CC=/usr/bin/x86_64-w64-mingw32-gcc
export RC=/usr/bin/x86_64-w64-mingw32-windres

git clone https://github.com/davidrg/ckwin.git
mkdir -p ckwin/openssl
cd ckwin/openssl

# Build OpenSSL
wget https://www.openssl.org/source/openssl-3.0.8.tar.gz
tar zxf openssl-3.0.8.tar.gz
cd openssl-3.0.8
perl Configure mingw64
make depend
make

# Build K95
cd ../kermit/k95
make RC=/usr/bin/x86_64-w64-mingw32-windres OPENSSL=../../openssl/openssl-3.0.8/
```

It *should* be possible to build with libssh too, but this hasn't been tested.

## Build result
Assuming everything builds OK, you should end up with the following directories:

| Directory           | Contents                                                  |
|---------------------|-----------------------------------------------------------|
| `/kermit/k95/nt`    | Console utilities (cknker.exe, telnet.exe, k95d.exe, etc) |
| `/kermit/k95/win95` | The GUI version of K95 (k95g.exe)                         |
| `kermit/k95/iksdnt` | IKSD Service                                              |

The X/Y/Z-MODEM library, p95.dll, will be copied into both 
the win95 and nt directories.

To build a minimal distribution, you'll need to do something like the following:
```shell
mkdir -p dist/doc/manual
cd dist
cp ../kermit/k95/nt/*.exe ./
cp ../kermit/k95/nt/*.dll ./
cp ../kermit/k95/iksdnt/*.exe ./
cp ../kermit/k95/win95/*.exe ./
cp ../kermit/k95/*.manifest ./
cp ../kermit/k95/k95d.cfg ./
cp ../kermit/k95/k95*.ini ./
cp ../openssl/openssl-*/*.dll ./
cp ../doc/manual/ckwin.htm docs/manual/ckwin.htm
mv cknker.exe k95.exe
mv cknker.exe.manifest k95.exe.manifest
```

To see what's involved in building a full-blown distribution, see windows batch
file `/kermit/k95/mkdist.bat` which is used to produce the release distributions.
The GitHub Actions job (in `/.github/workflows/build.yml` ) produces similar
distribution archives.