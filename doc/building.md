# Building Kermit 95 from Source

Building Kermit 95 is easy! All you need is 
[a compiler for Windows](compilers.md) (Visual C++ 2019 Community Edition 
recommended). The compiler you choose will determine what features are 
available and the minimum version of windows your build of K95 will support. 
You can use the linked table to help choose the compiler that best meets your
needs if you're looking to run on versions of Windows older than XP or CPU
architectures other than x86 and ARM.

If you wish to also build the Dialer (entirely optional), you'll need ideally a 
copy of Visual C++ 6.0 SP6. Open Watcom 1.9 is also supported but due to a lack 
of support for CTRL3D32 in that compiler the resulting application will look 
like something from Windows 3.1.

If you're using MinGW, see [Building with MinGW](mingw-building.md) - the
content below only applies to Visual C++ and Open Watcom.

## Build Environment

Firstly put the source code somewhere convenient. Where ever you put it, you 
need to edit `setenv.bat` in the root of the folder to point to your source 
code location. Uncomment the line `REM set root=C:\src` near the top of the 
file and set the value to where your source code lives, for example 
`set root=C:\dev\ckwin`, then save the file.

### Prepare optional dependencies

If you want to build with SSH support you must first build libssh and all of its
dependencies.

The *easiest* and quickest way to do this is to use vcpkg:
```
vcpkg install libssh
```
You'll then need to edit `setenv.bat` again and, under the Optional Dependencies
heading, you'll need to uncomment the `vcpkg_installed` variable and set it to
the location where you installed vcpkg so the Kermit 95 build process
can find libssh:
```
REM If you've installed these dependencies using vcpkg, set the following
REM to the vcpkg installed directory (where the include, lib and bin
REM subdirectories are) and ignore the rest.
set vcpkg_installed=C:\vcpkg\installed\x86-windows
```

If you'd rather build it all yourself, see 
[Optional Dependencies](optional-dependencies.md) for instructions. You don't
need to edit `setenv.bat` any further for this if you're using the same versions
of the optional dependencies.

### Prepare OpenZinc (optional)
The Dialer is an entirely optional component of Kermit 95. To build
it you'll need to grab a copy of OpenZinc 1.0 from
[the OpenZinc Website](http://openzinc.com/Downloads.html). The full package
is the one you need - "OpenZinc Engine, DOS, all Windows, OS/2, Unicode (zip)".
This includes source and binaries for a selection of compilers.

Unzip the OpenZinc distribution to the `zinc` subdirectory directly under the
source root so that the following files exist in the following locations
relative to each other:
```
\setenv.bat
\mkzinc.bat
\kermit\k95\ckoker.mak
\zinc\copymak.bat
```

If you're building with Open Watcom, this is all that should be required - when
you run setenv.bat it will detect the presence of OpenZinc and add it to the
build environment so that you can build the Dialer. You can skip ahead to the
build step

#### Building OpenZinc for Visual C++
If you're using Visual C++ you've got to rebuild OpenZinc as the default build
configuration included in the OpenZinc distribution is unsuitable (linked
against the wrong C runtime). If you've got a supported version of Visual C++
this process should be pretty painless.

To start with, open `\zinc\SOURCE\UI_ENV.HPP` in your preferred editor and
uncomment line 40:
```C
#define ZIL_MSWINDOWS_CTL3D   // Use CTL3DV2.DLL (3D look). For Windows only. 
```
This will enable 3D Windows 95-style widgets (or the native widgets on XP and
newer). This is only necessary for Visual C++ older than 2010.

Next run `\setenv.bat`. This should detect your compiler, the presence of
OpenZinc and the fact you don't have a suitable build of it for your compiler
available. You'll get a message along the lines of:
```
OpenZinc is required for building the dialer. You can build it by extracting
the OpenZinc distribution to C:\dev\ckw\zinc and running
C:\dev\ckw\mkzinc.bat
```

As instructed, just run mkzinc.bat to build OpenZinc. On a modern PC it
doesn't take very long.

## Build Kermit 95

1. Open a console and change directory to where the source code is located
2. Setup the Visual C++ or Open Watcom build environment by running the 
    appropriate batch file for your compiler. Some examples are: 
```
REM Visual C++ 2008:
C:\Program Files\Microsoft Visual Studio 8\VC\bin\vcvars32.bat

REM Visual C++ 2019:
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64_x86 -vcvars_ver=14.2

REM Visual C++ 2022, 64bit:
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=14.3

REM Open Watcom:
C:\watcom\owsetenv.bat
```
3. Setup the C-Kermit build environment by running `setenv.bat`
4. CD into `kermit\p95`
5. run `mknt.bat` to build the X/Y/Z Modem library
6. CD into `..\k95`
7. run `mknt.bat` to build the console version and some related bits
8. run `mkg.bat` to build the GUI version (k95g.exe)
9. run `mkdist.bat` to collect everything you built plus any redistributable
    dependencies into `kermit\k95\dist`

Done! Depending on the Visual C++ version you're using you may need to also
distribute the Visual C++ runtime with K95. If you built with Open Watcom the 
runtime libraries should have already been copied there for you but if they 
weren't you'll need to grab `clbr19.dll mt719.dll plbr19.dll` from the watcom 
`binnt` subdirectory.

## Build the Dialer (optional)

To build the dialer, run `setenv.bat` as you normally would for building
C-Kermit, then:
```
cd kermit\dialer
mk.bat
mkdist.bat
```
This should leave you with the directory `kermit\dialer\dist` containing the
following files:

| File                 | Description                                                    |
|----------------------|----------------------------------------------------------------|
| COPYING.LESSOR.txt   | LGPL License text                                              |
| dialer.dat           | Zinc data file containing all the dialer UI bits               |
| k95dial.exe          | The Kermit 95 Dialer                                |
| k95dial.exe.manifest | Makes the dialer fit in very slightly better on modern windows |
| p_direct.znc         | ?                                                              |
| p_servic.znc         | ?                                                              |

All files are required for the dialer to work normally.

### ctl3d32.dll Installation Procedure
When built with Visual C++ 6.0 or older the dialer relies on ctl3d32 to get the Windows 95
look and feel (rather than the "16bit app on Windows 95" look and feel). Ctl3d32 is not
available for Open Watcom or newer versions of Visual C++ so when built with these the dialer
will have the 2D "16bit app on Windows 95" look.

In order to be used ctl3d32.dll *must* be installed correctly on the users machine - it can't
simply be distributed in the K95 folder like other DLLs. To further complicate matters, there
are two different versions of ctl3d32 and the correct one for the users operating system must
be installed. The ANSI version is for Windows 95/98/ME and the Unicode version is for 
Windows NT and its descendants (including Windows Xp/7/8/10/11).

You can get both versions of Ctl3d32 from your Visual C++ CD-ROM:

| Visual C++ Version | ANSI (for 95/98/ME)                     | Unicode (for NT/2000/XP/Vista/7/8/10/11) |
|--------------------|-----------------------------------------|------------------------------------------|
| Visual C++ 6.0     | `\VC98\REDIST\ANSI\CTL3D32.DLL`         | `\VC98\REDIST\CTL3D32.DLL`               |
| Visual C++ 5.0     | `\DEVSTUDIO\VC\REDIST\ANSI\CTL3D32.DLL` | `\DEVSTUDIO\VC\REDIST\CTL3D32.DLL`       |
| Visual C++ 4.x     | `\MSDEV\REDIST\ANSI\CTL3D32.DLL`        | `\MSDEV\REDIST\CTL3D32.DLL`              |
| Visual C++ 2.x     | `\MSVC20\REDIST\ANSI\CTL3D32.DLL`       | `\MSVC20\REDIST\CTL3D32.DLL`             |

The appropriate version of ctl3d32.dll can be installed on the users machine using `ctl3dins.exe`
which is produced as part of the standard K95 build procedure. This relies on having both versions of
ctl3d32.dll present in the K95 folder alongside ctl3dins.exe. To use it, the ANSI version of ctl3d32.dll 
must be renamed to `ctl3d95.dll` and the Unicode version to `ctl3dnt.dll`, then distribute them 
alongside ctl3dins.exe with the rest of Kermit 95.

When the user runs ctl3dins.exe if ctl3d32.dll isn't installed it will rename the appropriate
version for the users operating system (ctl3dnt.dll or ctl3d95.dll) back to ctl3d32.dll and
install it. If it was built with Visual C++ 2.0 it will also install msvcrt20.dll if necessary.

## Build k95cinit.exe (*very* optional)

Historically Kermit 95 was, since v1.1.10, distributed with a utility called
k95cinit.exe. This tool is only required to correct an issue relating to COM
port devices on Windows 95 versions prior to OSR2 
[described here](https://kermitproject.org/k95bugs.html#b79).

It's pretty unlikely anyone will ever run into the issue this tool solves in the
2020s so building it is highly optional - Kermit 95 will work just
fine on modern systems without it.

If you'd like to build this utility anyway, Visual C++ 1.5 is required along 
with OpenZinc 1.0.

To build it, open a command prompt and run something like the following from the
root of your C-Kermit source directory (eg, C:\src):
```
C:\src> C:\msvc\bin\msvcvars.bat
C:\src> setenv.bat
OpenZinc found!

Your compiler is: Visual C++ 1.0 (16-bit)
This compiler is only supported for building the k95cinit.exe utility.
You can build that now by running mk.bat
C:\src\kermit\dialer\init> mk.bat
```
The K95 setenv.bat script will detect your environment is set up for the 16bit
Visual C++ 1.x compiler (due to running `msvcvars.bat`) and will configure the
environment specially for building k95cinit.exe. All you have to do from there
is run mk.bat to do the build, and optionally mkdist.bat to copy all the
required files to `\kermit\dialer\dist`

If you don't have OpenZinc installed you may see a different message instructing
you to build OpenZinc first - this will leave the current directory unchanged.

# Improving build times

Kermit 95 uses nmake makefiles to build. nmake doesn't know how to
schedule builds on more than one CPU so if you're building with Visual C++ on
a computer with multiple processors you can reduce build times significantly
by using a compatible build tool that *is* aware of multiple processors.

[JOM](https://wiki.qt.io/Jom) is regularly tested with K95 and works well with
all versions of Visual C++ (if you're building on a modern version of Windows).
Just [Download](http://download.qt.io/official_releases/jom/jom.zip) and unzip it
somewhere on your path, then before running `setenv.bat` run `set MAKE=jom`:

```
set PATH=%PATH%;C:\path\to\jom
set MAKE=jom
setenv.bat
```

You can also build recent versions of OpenSSL with JOM. Just add `-FS` to the end 
of the Configure command and then run `jom` instead of `nmake`. For example:

```
perl Configure VC-WIN32 zlib-dynamic --with-zlib-include=C:\path\to\ckwin\zlib\1.2.13 -FS
jom
```

Note that at the time of writing OpenSSL doesn't officially support JOM (see
[this ticket](https://github.com/openssl/openssl/issues/9931)) so while it works
fine with current released versions there is always the possibility of it breaking
in the future.