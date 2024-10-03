!if "$(PLATFORM)" == "NT"

# This makefile tries to figure out what compiler we're using,
# its version, the target CPU Architecture and platform.
#
# It should support all 32-bit versions of Visual C++ up to some
# recent release and it may also pickup Watcom and the 16bit
# versions of Visual C++.

# TODO: Flip this around so the oldest compilers are at the top to
#       improve performance on older machines that are more likely
#       to be running older compilers.

# Check if nmake is really jom (if so we'll want to avoid some compiler flags on older
# versions of Visual C++)
ISJOM=no
!if [echo $(MAKE) | findstr /C:"jom" > nul] == 0
!Message Make appears to be jom
ISJOM=yes
!endif

!if "$(CK_DETECT_COMPILER)" != "no"

# We'll start off assuming Visual C++ and overwrite this later if
# its really watcom
CMP = VCXX
COMPILER = Visual C++

!IF ([wcc386 . <nul 2>&1 > nul] == 0)
# Open Watcom
CMP = OWCL
COMPILER = OpenWatcom C/C++ CL clone
MSC_VER = 130
COMPILER_VERSION = Visual C++ 7.0 compatible

!ELSEIF ([cl 2>&1 | findstr /C:"Digital Mars" > nul] == 0)
MSC_VER = 120
COMPILER_VERSION = Digital Mars C/C++

!ELSEIF ([cl 2>&1 | findstr /C:"Version 19.4" > nul] == 0)
# Visual C++ 14.3 (Visual Studio 2022)
MSC_VER = 194
COMPILER_VERSION = 14.4 (Visual Studio 2022)

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
# Microsoft (R) C/C++ Optimizing Compiler Version 14.00.40310.39 for IA-64
# Microsoft (R) C/C++ Optimizing Compiler Version 14.00.40310.41 for AMD64
MSC_VER = 140
COMPILER_VERSION = 8.0 (Visual Studio 2005)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 13.1" > nul] == 0)
# Visual C++ 7.1 (Visual Studio .net 2003)
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 13.10.3077 for 80x86
MSC_VER = 131
COMPILER_VERSION = 7.10 (Visual Studio .net 2003)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 13.0" > nul] == 0)
# Visual C++ 7.0 (Visual Studio .net 2002)
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 13.00.9466 for 80x86
MSC_VER = 130
COMPILER_VERSION = 7.0 (Visual Studio .net 2002)

!ELSEIF ([cl 2>&1 | findstr /R /C:"Digital.*Alpha.*Version 13.0" > nul] == 0)
# DEC Alpha compiler for 64bit Windows 2000
# Microsoft (R) & Digital (TM) Alpha C/C++ Optimizing Compiler Version 13.00.8499

# Version number suggests its Visual C++ 7.0 but its from 1999, two years before the
# final release of Visual C++ 7.0. So its probably closer to Visual C++ 6 in features,
# though its close enough to Visual C++ 7.0 for our purposes.

MSC_VER = 130
COMPILER_VERSION = 64bit Windows 2000 for DEC Alpha SDK

!ELSEIF ([cl 2>&1 | findstr /C:"Version 12.0" > nul] == 0)
# Visual C++ 6.0 (Visual Studio 6.0)
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 12.00.8804 for 80x86
MSC_VER = 120
COMPILER_VERSION = 6.0 (Visual Studio 6.0)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 11.0" > nul] == 0)
# Visual C++ 5.0 (Visual Studio 97)
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 11.00.7022 for 80x86
MSC_VER = 110
COMPILER_VERSION = 5.0 (Visual Studio 97)

!ELSEIF ([cl 2>&1 | findstr /C:"Version 10.20" > nul] == 0)
# Visual C++ 4.2. The Enterprise version from MSDN calls itself:
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 10.20.6166 for 80x86
MSC_VER = 102
COMPILER_VERSION = 4.2

!ELSEIF ([cl 2>&1 | findstr /C:"Version 10.10" > nul] == 0)
# GUESS: Visual C++ 4.1
MSC_VER = 101
COMPILER_VERSION = 4.1

!ELSEIF ([cl 2>&1 | findstr /C:"Version 10.0" > nul] == 0)
# Visual C++ 4.0 (there was no 3.0). The retail version and cross-dev for Mac call
# themselves:
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 10.00.5270 for 80x86
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 10.00.5270 for 680x0
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 10.00.5271 for Power Mac
MSC_VER = 100
COMPILER_VERSION = 4.0

!ELSEIF ([cl 2>&1 | findstr /C:"Version 9.10" > nul] == 0)
# Visual C++ 2.2 (and perhaps 2.1?) subscription update:
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 9.10 for 80x86
MSC_VER = 90
COMPILER_VERSION = 2.2

# TODO: How does Visual C++ 2.1 report itself?

!ELSEIF ([cl 2>&1 | findstr /C:"Version 9.0" > nul] == 0)
# Visual C++ 2.0 (very old 32bit compiler). The retail box version calls itself:
# Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 9.00 for 80x86
MSC_VER = 90
COMPILER_VERSION = 2.0

!ELSEIF ([cl 2>&1 | findstr /R /C:"AXP.*Version 8\.03" > nul] == 0)
# The Win32 SDK for Windows NT 3.50 includes an Alpha compiler which calls itself:
# Microsoft (R) & Digital (TM) AXP C/C++ Optimizing Compiler Version 8.03.JFa
# (This is on the Microsoft Solutions Development Kit)

MSC_VER = 80
COMPILER_VERSION = 1.00 (NT 3.50 SDK, AXP)

!ELSEIF ([cl 2>&1 | findstr /R /C:"Centaur.*Version 8\.00" > nul] == 0)
# The Win32 SDK Final Release (NT 3.1) MIPS compiler calls itself:
# Microsoft (R) C Centaur Optimizing Compiler Version 8.00.081

MSC_VER = 80
COMPILER_VERSION = 1.00 (NT 3.1 SDK, MIPS)

# This compiler behaves weirdly. Flag it so we can deal with it later.
MIPS_CENTAUR = yes

!ELSEIF ([cl 2>&1 | findstr /R /C:"32-bit.*Version 8\.0" > nul] == 0)
# This could also pick up the Win32 SDK, the final release of which calls itself:
#   Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 8.00.3190a
# While Visual C++ 1.00 calls itself:
#   Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 8.00
# Confusingly, while the box is labeled "Visual C++/NT 1.00", the
# About dialog in the IDE calls itself "Visual C++ 1.10"
#
# The Win32 SDK ships with an older version of nmake which isn't compatible with
# these makefiles, and also doesn't include link.exe.
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
!message Unrecognised compiler! Please update compiler_detect.mak.
!ENDIF

#
# Try to work out which CPU we're targeting too.
#

# Assume we're building for Windows
TARGET_PLATFORM = Windows

!if "$(CMP)" == "OWCL"
# Open Watcom

TARGET_CPU = x86

!ELSEIF ([cl 2>&1 | findstr /C:"for MIPS R-Series" > nul] == 0)
# We're targeting (and running on) Windows NT MIPS
TARGET_CPU = MIPS

!ELSEIF ([cl 2>&1 | findstr /R /C:"Centaur.*Version 8\.00" > nul] == 0)
# The Win32 SDK Final Release (NT 3.1) MIPS compiler calls itself:
# Microsoft (R) C Centaur Optimizing Compiler Version 8.00.081

TARGET_CPU = MIPS

!ELSEIF ([cl 2>&1 | findstr /C:"for PowerPC" > nul] == 0)
# We're targeting (and running on) Windows NT PowerPC
TARGET_CPU = PPC

!ELSEIF ([cl 2>&1 | findstr /C:"AXP" > nul] == 0)
# Assuming Alpha - the Alpha compiler in the NT 3.50 SDK calls itself:
# Microsoft (R) & Digital (TM) AXP C/C++ Optimizing Compiler Version 8.03.JFa

TARGET_CPU=AXP

!ELSEIF ([cl 2>&1 | findstr /R /C:"Digital.*Alpha.*Version 13.0" > nul] == 0)
# Assuming 64bit NT on Alpha - the Alpha compiler in the 64bit Windows 2000 SDK calls itself:
# Microsoft (R) & Digital (TM) Alpha C/C++ Optimizing Compiler Version 13.00.8499

# Yes, this is a thing that exists. And no, there isn't any way to actually run
# binaries produced with it. 64bit Windows for the Alpha was never seen outside
# Microsoft. So there is really no point in building something for this target.

TARGET_CPU=AXP64

!ELSEIF ([cl 2>&1 | findstr /C:"for x64" > nul] == 0)
# We're using the 64bit x86 compiler
TARGET_CPU = x86-64

!ELSEIF ([cl 2>&1 | findstr /C:"for AMD64" > nul] == 0)
# We're using the 64bit x86 compiler
# Microsoft (R) C/C++ Optimizing Compiler Version 14.00.40310.41 for AMD64
TARGET_CPU = x86-64

!ELSEIF ([cl 2>&1 | findstr /C:"for Itanium" > nul] == 0)
# Intel Itanium
TARGET_CPU = IA64

!ELSEIF ([cl 2>&1 | findstr /C:"for IA-64" > nul] == 0)
# Intel Itanium
# Microsoft (R) C/C++ Optimizing Compiler Version 14.00.40310.39 for IA-64
TARGET_CPU = IA64

!ELSEIF ([cl 2>&1 | findstr /C:"for ARM64" > nul] == 0)
# The 64-bit ARM compiler (for targeting the ARM edition of Windows 10+)
TARGET_CPU = ARM64

!ELSEIF ([cl 2>&1 | findstr /C:"for ARM" > nul] == 0)
# The 32bit ARM compiler (for targeting Windows RT most likely)
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

# Figure out the host CPU Architecture
HOST_CPU = $(PROCESSOR_ARCHITECTURE)
!if "$(HOST_CPU)" == "AMD64"
HOST_CPU = x86-64
!endif

# And if we're cross-compiling from a CPU architecture
# other than the target.
CROSS_BUILD = no
!if "$(HOST_CPU)" != "$(TARGET_CPU)"
CROSS_BUILD = yes
!endif

# And if we're cross-compiling, can we
CROSS_BUILD_COMPATIBLE = yes
!if "$(CROSS_BUILD)" == "yes"
# We're cross-compiling.

!if "$(HOST_CPU)" == "x86-64" && "$(TARGET_CPU)" == "x86"
CROSS_BUILD_COMPATIBLE = yes

!elseif "$(HOST_CPU)" == "ARM64" && "$(TARGET_CPU)" == "ARM"
# TODO: Is this actually true? Do the compilers even run on these architectures?
CROSS_BUILD_COMPATIBLE = yes

!else
# x86 can't run x86-64 code for example.
CROSS_BUILD_COMPATIBLE = no

!endif

!endif

!endif

!ELSE IF "$(PLATFORM)" == "OS2"

# On OS/2 we'll just assume OpenWatcom for now. I don't have access to the
# IBM compiler to find a way to tell it apart from watcom like we do for
# Visual C++.
CMP = OWWCL
COMPILER = Open Watcom WCL
COMPILER_VERSION = Open Watcom

# wcl386 doesn't pretend to be Visual C++ and doesn't take the same
# command line arguments.
MSC_VER = 0

# Nothing supports PowerPC OS/2.
TARGET_CPU = x86
TARGET_PLATFORM = OS/2

# Override CL so we don't end up running the Visual C++ clone cl.
CL = wcl386

!ENDIF
