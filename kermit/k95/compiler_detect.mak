
# This makefile tries to figure out what compiler we're using,
# its version and the target CPU Architecture.

# We'll start off assuming Visual C++ and overwrite this later if
# its really watcom
CMP = VCXX
COMPILER = Visual C++

!IFDEF __VERSION__
# This is Watcom wmake pretending to be nmake. We'll assume the compiler is
# really Watcom C pretending to be Visual C++ 2002.
#
# This check has to come first because OpenWatcom 1.9 just stops waiting for
# input and never exits
CMP = OWCL
COMPILER = OpenWatcom C/C++ CL clone
MSC_VER = 130
COMPILER_VERSION = Visual C++ 7.0 compatbile

!ELSEIF ([cl 2>&1 | findstr /C:"Version 18.0" > nul] == 0)
# Visual C++ 12.0 (Visual Studio 2013)
MSC_VER = 180
COMPILER_VERSION = 12.0 (Visual Studio 2013)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 17.0" > nul] == 0)
# Visual C++ 11.0 (Visual Studio 2012)
MSC_VER = 170
COMPILER_VERSION = 11.0 (Visual Studio 2012)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 16.0" > nul] == 0)
# Visual C++ 10.0 (Visual Studio 2010)
MSC_VER = 160
COMPILER_VERSION = 10.0 (Visual Studio 2010)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 15.0" > nul] == 0)
# Visual C++ 9.0 (Visual Studio 2008)
MSC_VER = 150
COMPILER_VERSION = 9.0 (Visual Studio 2008)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 14.0" > nul] == 0)
# Visual C++ 8.0 (Visual Studio 2005)
MSC_VER = 140
COMPILER_VERSION = 8.0 (Visual Studio 2005)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 13.1" > nul] == 0)
# Visual C++ 7.1 (Visual Studio .net 2003)
MSC_VER = 131
COMPILER_VERSION = 7.10 (Visual Studio .net 2003)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 13.0" > nul] == 0)
# Visual C++ 7.0 (Visual Studio .net 2002)
MSC_VER = 130
COMPILER_VERSION = 7.0 (Visual Studio .net 2002)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 12.0" > nul] == 0)
# Visual C++ 6.0 (Visual Studio 6.0)
MSC_VER = 120
COMPILER_VERSION = 6.0 (Visual Studio 6.0)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 11.0" > nul] == 0)
# Visual C++ 5.0 (Visual Studio 97)
MSC_VER = 110
COMPILER_VERSION = 5.0 (Visual Studio 97)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 10.0" > nul] == 0)
# Visual C++ 4.0 (there was no 3.0)
MSC_VER = 100
COMPILER_VERSION = 4.0

!ELSEIF ([cl 2>&1 | findstr /C:"Version 9.0" > nul] == 0)
# Visual C++ 2.0 (very old 32bit compiler)
MSC_VER = 90
COMPILER_VERSION = 2.0

!ELSEIF ([cl 2>&1 | findstr /C:"Version 8.0" > nul] == 0)
# Probably the ancient 16bit compiler (1.5) but its
# possible Visual C++ 1.0 32bit edition might also
# report itself as 8.0
MSC_VER = 80
COMPILER_VERSION = 1.5 16bit (assumed)

!ELSE
# Just assume its something modern that we've not
# seen before
MSC_VER = 999
COMPILER_VERSION = Unknown
!ENDIF

#
# Try to work out which CPU we're targeting too.
#

!IFDEF __VERSION__
# OpenWatcom again. Assume x86.
TARGET_CPU = x86

!ELSEIF ([cl 2>&1 | findstr /C:"for x64" > nul] == 0)
# We're using the 64bit x86 compiler
TARGET_CPU = x86-64

!ELSEIF ([cl 2>&1 | findstr /C:"for Itanium" > nul] == 0)
# Intel Itanium
TARGET_CPU = IA64

!ELSEIF ([cl 2>&1 | findstr /C:"for ARM" > nul] == 0)
# The ARM compiler (for targeting Windows RT most likely)
TARGET_CPU = ARM
!ELSE

# We'll assume its just x86 (although really it could be Alpha, 
# MIPS, PPC or 68k too but I don't have access to those compilers 
# to see how they report themselves)
TARGET_CPU = x86

!ENDIF