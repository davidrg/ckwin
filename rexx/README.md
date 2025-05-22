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