# Building C-Kermit for Windows from Source

Building C-Kermit for Windows is easy! All you need is 
[a compiler for Windows](compilers.md) (Visual C++ 2019 Community Edition 
recommended). The compiler you choose will determine what features are 
available and the minimum version of windows your build of CKW will support. 
You can use the linked table to help choose the compiler that best meets your
needs if you're looking to run on versions of Windows older than XP.

## Build Process

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
the location where you installed vcpkg so the C-Kermit for Windows build process
can find libssh:
```
REM If you've installed these dependencies using vcpkg, set the following
REM to the vcpkg installed directory (where the include, lib and bin
REM subdirectories are) and ignore the rest.
set vcpkg_installed=C:\vcpkg\installed\x86-windows
```

If you'd rather build it all yourself, see 
[Optional Dependencies](optional-dependencies.md) for instructions. You don't
need to edit `setenv.bat` any further for this.

### Build C-Kermit for Windows

1. Open a console and change directory to where the source code is located
2. Setup the Visual C++ or OpenWatcom build environment by running the 
    appropriate batch file for your compiler. Some examples are: 
```
REM Visual C++ 2008:
C:\Program Files\Microsoft Visual Studio 8\VC\bin\vcvars32.bat

REM Visual C++ 2019:
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64_x86 -vcvars_ver=14.2

REM OpenWatcom:
C:\watcom\owsetenv.bat
```
3. Setup the C-Kermit build environment by running `setenv.bat`
4. CD into `kermit\k95`
5. run `mknt.bat` to build the console version and some related bits
6. run `mkg.bat` to build the GUI version (k95g.exe)
7. run `mkdist.bat` to collect everything you built plus any redistributable
    dependencies into `kermit\k95\dist`

Done! Depending on the Visual C++ version you're using you may need to also
distribute the Visual C++ runtime with CKW. If you built with OpenWatcom the 
runtime libraries should have already been copied there for you but if they 
weren't you'll need to grab `clbr19.dll mt719.dll plbr19.dll` from the watcom 
`binnt` subdirectory.
