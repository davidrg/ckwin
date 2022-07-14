
# This makefile tries to figure out what compiler we're using,
# its version, the target CPU Architecture and platform.
#
# It should support all 32-bit versions of Visual C++ up to some
# recent release and it may also pickup Watcom and the 16bit
# versions of Visual C++.

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
COMPILER_VERSION = Visual C++ 7.0 compatible

!ELSEIF ([cl 2>&1 | findstr /C:"Version 19.3" > nul] == 0)
# Visual C++ 14.3 (Visual Studio 2022)
MSC_VER = 193
COMPILER_VERSION = 14.3 (Visual Studio 2022)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 19.2" > nul] == 0)
# Visual C++ 14.2 (Visual Studio 2019)
MSC_VER = 192
COMPILER_VERSION = 14.2 (Visual Studio 2019)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 19.1" > nul] == 0)
# Visual C++ 14.1 (Visual Studio 2017)
MSC_VER = 191
COMPILER_VERSION = 14.1 (Visual Studio 2017)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 19.0" > nul] == 0)
# Visual C++ 14.0 (Visual Studio 2015)
MSC_VER = 190
COMPILER_VERSION = 14.0 (Visual Studio 2015)

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

!ELSEIF ([cl 2>&1 | findstr /R /C:"32-bit.*Version 8\.0" > nul] == 0)
# This could also pick up the Win32 SDK, the final release of which calls itself:
#   Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 8.00.3190a
# While Visual C++ 1.00 calls itself:
#   Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 8.00
# Confusingly, while the box is labeled "Visual C++/NT 1.00", the
# About dialog in the IDE calls itself "Visual C++ 1.10"
MSC_VER = 80
COMPILER_VERSION = 1.00 (32-bit edition)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 8.00" > nul] == 0)
# Compiler banner doesn't mention 32-bit so it must be a 16bit compiler.
# Visual C++ 1.52c calls itself:
#   Microsoft (R) C/C++ Optimizing Compiler Version 8.00c
# Not sure what Visual C++ 1.52b/1.51/1.5 call themselves (don't have a copy to check)
# Even less sure what Visual C++ 1.0 calls itself given it replaced Microsoft C/C++ 7.0.
# Perhaps Visual C++ 1.x == Version 8.00 ??

MSC_VER = 80
COMPILER_VERSION = 1.x 16bit

!ELSE
# Just assume its something modern that we've not
# seen before
MSC_VER = 999
COMPILER_VERSION = Unknown
!ENDIF

#
# Try to work out which CPU we're targeting too.
#

# Assume we're building for Windows
TARGET_PLATFORM = Windows

!IFDEF __VERSION__
# OpenWatcom again. Assume x86.
TARGET_CPU = x86

TARGET_PLATFORM = Windows
# TODO: What if we're targeting OS/2? Watcom supports OS/2...

!ELSEIF ([cl 2>&1 | findstr /C:"for x64" > nul] == 0)
# We're using the 64bit x86 compiler
TARGET_CPU = x86-64

!ELSEIF ([cl 2>&1 | findstr /C:"for Itanium" > nul] == 0)
# Intel Itanium
TARGET_CPU = IA64

!ELSEIF ([cl 2>&1 | findstr /C:"for ARM" > nul] == 0)
# The ARM compiler (for targeting the ARM edition of Windows 10+ most likely)
# could also be Windows RT but thats obscure now.
TARGET_CPU = ARM

!ELSEIF ([cl 2>&1 | findstr /C:"for Power Mac" > nul] == 0)
# Visual C++ Cross-Development Edition - Power Macintosh
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 10.00.5271 for Power Mac
TARGET_CPU = PPC
TARGET_PLATFORM = Macintosh

!ELSEIF ([cl 2>&1 | findstr /C:"for 680x0" > nul] == 0)
# Visual C++ Cross-Development Edition - 68k
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 10.00.5270 for 680x0
TARGET_CPU = 68k
TARGET_PLATFORM = Macintosh

!ELSE

# We'll assume its just x86 (although really it could be Alpha, 
# MIPS, or PPC too but I don't have access to those compilers
# to see how they report themselves)
TARGET_CPU = x86

!ENDIF