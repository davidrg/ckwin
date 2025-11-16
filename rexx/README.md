REXX Support
============

Kermit 95 can optionally be built with REXX support on Windows (REXX support is
automatic on OS/2). To do this, place the Regina REXX distribution built for
your platform in the `regina` subdirectory.

If you want/need to build it from source, something like the following should
work:
```
set REGINA_SRCDIR=%root%\rexx\regina
REM In Regina 3.9.6, platfrom can be one of: x86, x86-64 or arm64: 
set PLATFORM=x86
cd %REGINA_SRCDIR%
nmake -f makefile.win.vc
mkdir include
copy rexxsaa.h include\
mkdir lib
copy regina.lib lib\
```

Starting with Regina REXX 3.9.7, building with older compilers such as Visual
C++ 6.0 and 4.0 is possible allowing it to run on Windows NT 3.51 or newer
(including RISC platforms) as well as Windows 9x. When building with these older
compilers, you must append `HAVE_MT=no` to the nmake line, for example: 
`nmake -f makefile.win.vc HAVE_MT=no`

Building for 32bit ARM
----------------------
Regina REXX doesn't support ARM32 (just like Microsoft!), but Kermit 95 does.
The process for building it is a bit annoying:

1. Build Regina from source for x86 (or arm64 if that's the platform you're on)
   using the instructions above
2. Grab a copy of `trexx.exe` that is produced during the build process and put
   it somewhere safe
3. Do a clean: `nmake -f makefile.win.vc clean`
4. Put `trexx.exe` back where you found it
5. Patch the makefile: `patch < ..\arm32.patch` - this will add an arm32 target,
   and disable building of `trexx.exe`
6. Switch to an ARM32 cross-compiler and build Regina as above setting `PLATFORM=arm32`

RexxRE
-------
RexxRE is a handy regular expressions library for REXX available from
http://home.interlog.com/~ptjm/. The archive includes binaries for x86-32
Windows, but the latest release is over 20 years old so you'll probably want to
rebuild it from source.

The RexxRE source has issues with modern versions of Visual C++ so a minor patch
file is provided.

To build RexxRE, just place it in the `rexxre` subdirectory and then:
```
cd rexxre
patch -p1 ..\rexxre.patch
nmake -f Makefile.NT
```

If you build RexxRE in this location, setenv.bat should pick it up automatically
and add it to the distribution.