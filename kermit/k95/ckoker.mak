# CKOKER.MAK, Version 10.0
# See CKOMAK.HLP for further information.
# Authors: 
#   Jeffrey Altman, Frank da Cruz, Columbia University, New York City, USA
#   David Goodwin <david@zx.net.nz>
#
# Last update: 
#
# -- Makefile to build C-Kermit 10.0 for OS/2 and Windows NT --
#
# The result is a runnable program called CKOKER32.EXE (OS/2) or CKNKER.EXE
# (NT), or K95G (NT, GUI) in the current directory.  Or if you "make winsetup",
# SETUP.EXE.
#
# To override the following definitions without having to edit this file,
# define them as environment variables and then run NMAKE with the /E switch.

# which operating system
PLATFORM = NT
K95BUILD = K95

# IBM VisualAge Libs
VISUALAGE = C:\IBMCXX0

# for IBM TCP/IP 1.2.1
# While tcpipdll.lib is on the Warp 4 CD-ROM, the headers are not. You likely
# need the IBM TCP/IP 1.2.1 Programmers Tool Kit and probably some updates like
# PROGCSD.EXE to build cko32i12.dll
IBM12DIR  = C:\TCPIP
IBM12LIBS = $(IBM12DIR)\lib\tcpipdll.lib
IBM12INC  = $(IBM12DIR)\include

# for IBM TCP/IP 2.0
# This should be the IBM TCP/IP 2.0 Programmers Tool Kit, but the SDK on the
# Warp 4 CD-ROM seems to work too. Set IBM20DIR in setenv.cmd (OS/2), or follow
# the instructions in \ibmtcp\README.md (Windows).
#IBM20DIR  = C:\TCPIP
IBM20LIBS = $(IBM20DIR)\lib\tcp32dll.lib $(IBM20DIR)\lib\so32dll.lib
IBM20INC  = $(IBM20DIR)\include

# for FTP PC/TCP 1.3
FTP13DIR  = C:\DEVKIT
FTP13LIBS32 = $(FTP13DIR)\lib\socket32.lib
FTP13LIBS16 = $(FTP13DIR)\lib\socket16.lib
FTP13INC  = $(FTP13DIR)\include

# for Novell LAN Workplace 3.0
LWP30DIR  = C:\LANWP\TOOLKIT
LWP30LIBS32 = $(LWP30DIR)\os2lib20\socklib.lib
LWP30INC    = $(LWP30DIR)\inc20

# Manually Specify TCP/IP support DLLs to build
#OS2TCPDLLS = cko32i41.dll cko32i20.dll
# Options are:
#   cko32i41.dll    - for IBM TCP/IP 4.1+ (MPTS 5.3+), the "new" 32-bit TCP/IP stack
#   cko32i20.dll    - for IBM TCP/IP 2.0-4.0
#   cko32i12.dll    - for IBM TCP/IP 1.2.1
#   cko32f13.dll    - for FTP PC/TCP 1.3
#   cko32n30.dll    - for Novell LAN Workplace 3.0 (cko32n30.dll was never released)
# If not specified, any DLLs you can build will be configured automatically
# based on compiler and available SDKs

# Base flags for all versions of Visual C++ (and Open Watcom
# pretending to be Visual C++)
#!if "$(DEBUG)" == "-DNDEBUG"
# Doing a release build
!if "$(CKB_STATIC_CRT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MT
!else
COMMON_CFLAGS = /MD
!endif

#!else
# Doing a Debug build, use the Debug CRT
#!if "$(CKB_STATIC_CRT)"=="yes"
#!message Building with statically linked native CRT as requested.
#COMMON_CFLAGS = /MTd
#!else
#COMMON_CFLAGS = /MDd
#!endif
#!endif

# These options are used for all Windows .exe targets
!if "$(CKF_DEV_CHECKS)" == "yes"
# Enable extra runtime checks. These only work with a debug build and
# Visual C++ 2002 and newer
COMMON_CFLAGS = $(COMMON_CFLAGS) /RTCsu
!else
COMMON_OPTS = /Ox
!endif

# These are:
# /GA     Optimise for Windows Application (ignored by Open Watcom)
# /Ox     Maximum Opts (= /Ogityb2 /Gs in VC6/7.0)

# If Visual C++ <= 2003 or Open Watcom:
#   /G5     Optimise for Pentium

# These may be good to add at some point
#   /GS         Buffer Security Check   (since VC 2022, maybe earlier versions)
#   /guard:cf   Control Flow Guard      (since VC 2017, maybe earlier versions)

# Formerly, the following options were used:
# Option    Targets     Description
# /G4       A           Optimise for 486
# /Ot       A           Favor code speed
# /Og       A           Enable global optimisations  (included in /Ox)
# /Oi       A, msvcp    Enable intrinsic functions   (included in /Ox)
# /Ox       B           Maximum Opts
# /G5       B, msvcp    Optimise for Pentium
# /GA       B, msvcp    Optimise for Windows Application
# /Ob1      msvcp       Inline expansion (default n=0)
# Where:
# A = winsetup, test, rlogin, telnet
# B = k95g, kui, msvc-iksd, msvc


!if "$(CK_DETECT_COMPILER)" != "no"
CMP = VCXX
COMPILER = unknown
COMPILER_VERSION = assuming Visual C++ 1.0
MSC_VER = 80
TARGET_CPU = x86
!endif

WIN32_VERSION=0x0400

# So that we can set the minimum subsystem version when needed
SUBSYSTEM_CONSOLE=console
SUBSYSTEM_WIN32=windows

# These are not supported by Visual C++ prior to 4.0
# /GF enables read-only string pooling
CFLAG_GF=/GF

# Try to detect the compiler version being used so we can adjust
# compiler flags accordingly.
!message Attempting to detect compiler...
!include compiler_detect.mak

!if "$(MIPS_CENTAUR)" == "yes"
!message
!message MIPS Centaur compiler - forcing build with statically linked CRT.
# /QmipsOb5000 increases the basic block threshold for optimisation
COMMON_CFLAGS = /D_MT /QmipsOb5000
CKB_STATIC_CRT = yes
!endif

# Figure out which OS/2 TCP/IP Support DLLs to build
!if "$(OS2TCPDLLS)" == ""

# Open Watcom can always build cko32i41.dll
!if "$(CMP)" == "OWWCL"
OS2TCPDLLS=$(OS2TCPDLLS) cko32i41.dll
!endif

# If setenv found the IBM TCP/IP 2.0-4.0 SDK, then build that too
!if "$(CKB_IBMTCP20)" == "yes"
OS2TCPDLLS=$(OS2TCPDLLS) cko32i20.dll
!endif

!endif

# This turns features on and off based on set feature flags (CKF_*), the
# platform being targeted, and the compiler currently in use.
!include feature_flags.mak

!message
!message
!message ===============================================================================
!message C-Kermit Build Configuration
!message ===============================================================================
!message  Platform:                 $(PLATFORM)
!message  Build:                    $(K95BUILD)
!message  Architecture:             $(TARGET_CPU)
!if "$(CROSS_BUILD)" == "yes"
!if "$(CROSS_BUILD_COMPATIBLE)" == "yes"
!message  Host Architecture:        $(HOST_CPU) (cross-compiling to compatible architecture)
!else
!message  Host Architecture:        $(HOST_CPU) (cross-compiling)
!endif
!else
!message  Host Architecture:        $(HOST_CPU)
!endif
!message  Compiler:                 $(COMPILER)
!message  Compiler Version:         $(COMPILER_VERSION)
!message  Compiler Target Platform: $(TARGET_PLATFORM)
!message  Enabled Features:         $(ENABLED_FEATURES)
!message  Disabled Features:        $(DISABLED_FEATURES)
!message ===============================================================================
!message
!message

!if "$(PLATFORM)" == "NT"
!if "$(CMP)" == "OWCL"

# Standard windows headers from MinGW that don't come with Open Watcom:
INCLUDE = $(INCLUDE);ow\;

!endif   # EndIf CMP == OWCL

!if ($(MSC_VER) < 80)
!error Unsupported compiler version. Visual C++ 1.0 32-bit edition or newer required.
!endif   # EndIf MSC_VER < 80

# TODO: Much of this compiler flag work should be applied to the KUI Makefile
#       too

# Check to see if we're using Visual C++ and targeting 64bit x86. If so
# then tell the linker we're targeting x86-64
!if "$(TARGET_CPU)" == "x86-64"
LDFLAGS = $(LDFLAGS) /MACHINE:X64
!endif  # EndIf TARGET_CPU == x86-64

!if "$(TARGET_CPU)" == "AXP64"
# This compiler is capable of targeting AXP64, so add the build flag to do that.
COMMON_CFLAGS = $(COMMON_CFLAGS) /Ap64
LINKFLAGS = $(LINKFLAGS) /MACHINE:ALPHA64
!endif  # EndIf TARGET_CPU == AXP64

!if ("$(DEBUG)" != "-DNDEBUG") && ($(MSC_VER) <= 130)
# This debug flag is only valid on Visual C++ 6.0 and older.
LINKFLAGS = $(LINKFLAGS) /debugtype:both
!endif  # EndIf DEBUG != -DNDEBUG and MSC_VER <= 130

!if ($(MSC_VER) >= 170) && ($(MSC_VER) <= 192)
# Starting with Visual C++ 2012, the default subsystem version is set to 6.0
# which makes the generated binaries invalid on anything older than Windows
# Vista (you get the "is not a valid win32 application" error). Visual C++ 2012
# through to 2019 are capable of targeting Windows XP so we set the subsystem
# version to 5.1 so the generated binaries are compatible.
SUBSYSTEM_CONSOLE=console,5.1
SUBSYSTEM_WIN32=windows,5.1
!endif  # EndIf MSC_VER >= 170 and MSC_VER <= 192

!if ($(MSC_VER) == 80) && ("$(TARGET_CPU)" == "AXP")
# The linker included with the NT 3.50 SDK for Alpha can't handle
# K95 (complains "LINK : error LNK1155: Special symbol 'end' already defined.")
# So to support using a newer linker that has less problems, we'll set
# the subsystem version so the result still works on NT 3.1/3.50
SUBSYSTEM_CONSOLE=console,3.1
SUBSYSTEM_WIN32=windows,3.1
!endif  # EndIf MSC_VER == 80 and TARGET_CPU == AXP

!if ($(MSC_VER) > 90)
!if "$(TARGET_CPU)" != "MIPS"
# This flag isn't valid on Visual C++ 4.0 MIPS (or, I assume, any other version)
COMMON_OPTS = $(COMMON_OPTS) /GA
!endif  # EndIf TARGET_CPU != MIPS
!endif  # EndIf MSC_VER > 90

# PDB Generation Stuff

!if ($(MSC_VER) < 180) && ("$(ISJOM)" == "yes") && ("$(CKB_MAKE_PDB)" != "yes")
!message Make is JOM and compiler is older than Visual C++ 2013. Can't reliably
!message synchronise writes to a PDB file with this compiler. Disabling PDB
!message generation. override with: set CKB_MAKE_PDB=yes but you may get build
!message errors.
CKB_MAKE_PDB=no
!endif  # EndIf MSC_VER < 180 and ISJOM == yes and CKB_MAKE_PDB != yes

!if "$(CKB_MAKE_PDB)" != "no"
# Lets see if we can make a PDB file! This requires Visual C++ 4.0 or newer.
!if ($(MSC_VER) > 90)
!message Enabling PDB generation

COMMON_CFLAGS = $(COMMON_CFLAGS) /Zi
LDDEBUG = $(LDDEBUG) /DEBUG /INCREMENTAL:NO /OPT:REF

# /OPT:ICF is new in Visual C++ 5.0
!if ($(MSC_VER) >= 110)
LDDEBUG = $(LDDEBUG) /OPT:ICF
!endif  # EndIf MSC_VER >= 110

# /FS is required to synchronise writes to a PDB when doing parallel builds with
# something like JOM. It was introduced in Visual C++ 2013.
!if ($(MSC_VER) >= 180)
COMMON_CFLAGS = $(COMMON_CFLAGS) /FS
!endif  # EndIf MSC_VER >= 180

!endif  # EndIf MSC_VER > 90
!endif  # EndIf CKB_MAKE_PDB != no

# End PDB Generation Stuff

!if ($(MSC_VER) < 140)
# These flags and options are deprecated or unsupported
# from Visual C++ 2005 (v8.0) and up.

# /GX- is new in Visual C++ 2.0
!if ($(MSC_VER) > 80)
COMMON_CFLAGS = $(COMMON_CFLAGS) /GX-
!endif  # EndIf MSC_VER > 80

!if ($(MSC_VER) < 100)
# Visual C++ 2.0 and 1.0 32-bit edition don't support these flags, so don't
# use them.
CFLAG_GF=
!endif  # EndIf MSC_VER < 100

COMMON_CFLAGS = $(COMMON_CFLAGS) /Ze
# These are:    /Ze     Enable extensions (default)
#               /GX-    Enable C++ Exception handling (same as /EHs /EHc)
#               /YX     Automatic .PCH

# Jom runs multiple instances of cl in parallel which causes problems with PCH locking.
# So only generate PCH files when nmake instead of jom.
!if "$(ISJOM)" == "no"
COMMON_CFLAGS = $(COMMON_CFLAGS) /YX
!endif  # EndIf ISJOM == no

!if "$(TARGET_CPU)" == "x86"
# Optimise for Pentium
COMMON_OPTS = $(COMMON_OPTS) /G5
!endif  # EndIf TARGET_CPU == x86

!else  # Else MSC_VER < 140
COMMON_CFLAGS = $(COMMON_CFLAGS) /EHs-c-
# These are:    /EHs-c-     Enable C++ Exception handling (replaces /GX-)
!endif  # EndIf MSC_VER < 140

!endif  # EndIf PLATFORM  == NT

RCDEFINES=$(RC_FEATURE_DEFS) /dCOMPILER_$(CMP)
!if "$(SSH_LIB)" == ""
SSH_LIB = ssh.lib
!endif

!if "$(REXX_LIB)" == ""
REXX_LIB = rexx.lib
!endif

#---------- Compiler targets:
#
# To build: "[dn]make <target>"

unknown:
	@echo Please specify target: "ibmc", "msvc" or "clean"

#    IMPORTANT: When building with TCP/IP support, edit the IBM-supplied
#    TCPIP\INCLUDE\NETLIB.H to remove the spurious #define for SIGALRM!
#    Similarly, the FTP Software PC/TCP devkit header files will need some
#    editing to correct far vs _far confusion.

# IBM C Set++ 2.x (32-bit) with static linking -- no DLL's required.
# Which is good, because otherwise users would need to have the IBM
# OS/2 development system C libraries on their PCs.
# - Current CSDs are CTC0011 and CTU0003
# - Current LINK386 is 2.01.016
#
# -G4 optimizes for the i486 pipeline.  It might make the program run a bit
# faster on 486 and above, but it also increases the size of .EXE by about
# 30K.  The result still runs OK on i386 processors.
# Add -G4 to the CC= line below if you want i486 optimization.
#
# msgbind does not use any environment variables to find DDE4.MSG.  Therefore,
# its path (on the system where you are building C-Kermit) must be edited into
# CKOKER.MSB, or the DDE4.MSG file must be copied into the current directory.
# It is normally found in IBMCPP\HELP.

################### WINDOWS TARGETS ###################
telnet:
	$(MAKE) -f ckoker.mak wtelnet \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) /J /DWIN32=1 /D_WIN32 /D_CONSOLE /D__32BIT__ /W2" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="-c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE)" \
	DEF="wtelnet.def"


rlogin:
	$(MAKE) -f ckoker.mak wrlogin \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) /J /DWIN32=1 /D_WIN32 /D_CONSOLE /D__32BIT__ /W2" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="-c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE)" \
	DEF="wrlogin.def"

# release version
test:
	$(MAKE) -f ckoker.mak wtest \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) /J /DWIN32=1 /D_CONSOLE /D__32BIT__ /W2" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="-c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE)" \
	DEF="wtest.def"

winsetup:
	$(MAKE) -f ckoker.mak wsetup \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) /J /D_WIN32 /DOS2 /DNT /D_CONSOLE /D__32BIT__ /W2 /D_WIN32_WINNT=$(WIN32_VERSION)" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="-c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /OPT:REF" \
	DEF="wsetup.def"

# release version
msvc:
	$(MAKE) -f ckoker.mak win32 \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) $(CFLAG_GF) /J /DWIN32=1 /D_WIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /Fm /F65536" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="/c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /OPT:REF $(LDDEBUG)" DEF="cknker.def"

!if "$(CKF_DYNAMIC_SSH)" == "yes"
msvc-sshdll:
	$(MAKE) -f ckoker.mak win32sshdll \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) $(CFLAG_GF) /J /DWIN32=1 /D_WIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /Fm /F65536" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="/c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /OPT:REF" DEF="cknker.def"
msvc-sshdlld:
	$(MAKE) -f ckoker.mak win32sshdll \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) $(CFLAG_GF) /J /DWIN32=1 /D_WIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /Fm /F65536" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="/c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /DEBUG:full /debugtype:both /OPT:REF" DEF="cknker.def"
!endif

# release version
msvc-iksd:
     $(MAKE) -f ckoker.mak iksdnt \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) $(CFLAG_GF) /J /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION)  /D_CONSOLE /D__32BIT__ /W2 /Fm /F65536" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="/c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /OPT:REF" DEF="cknker.def"

# debug version
msvcd:
        $(MAKE) -f ckoker.mak win32 \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
	OPT="" \
    DEBUG="/Zi /Odi /Ge " \
    DLL="" \
	CFLAGS=" $(COMMON_CFLAGS) $(CFLAG_GF)  /GZ /J /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /F65536" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="/c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /DEBUG:full /WARN:3 /FIXED:NO /PROFILE /OPT:REF" \
	DEF="cknker.def"

# debug version
msvcd-iksd:
        $(MAKE) -f ckoker.mak iksdnt \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
	OPT="" \
    DEBUG="/Zi /Odi /Ge " \
    DLL="" \
	CFLAGS=" $(COMMON_CFLAGS) $(CFLAG_GF)  /GZ /J /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /F65536" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="/c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /DEBUG:full /WARN:3 /FIXED:NO /PROFILE /OPT:REF" \
	DEF="cknker.def"

# memory debug version
msvcmd:
        $(MAKE) -f ckoker.mak win32md \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
	OPT="" \
    DEBUG="/Zi /Odi /Ge -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG" \
    DLL="" \
	CFLAGS=" $(COMMON_CFLAGS) /J /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /F65536" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="/c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /DEBUG:full /WARN:3 /FIXED:NO /PROFILE" \
	DEF="cknker.def"

# profile version
msvcp:
        $(MAKE) -f ckoker.mak win32 \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) /J /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /Fm /F65536" \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="/c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /FIXED:NO /PROFILE" \
	DEF="cknker.def"

# kui debug version
kuid:
	$(MAKE) -f ckoker.mak win32kui \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
	OPT="" \
    DEBUG="/Zi /Odi" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) $(CFLAG_GF) /J /DKUI /DCK_WIN /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 -I." \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="-c" \
    LINKFLAGS="/nologo /DEBUG:full /SUBSYSTEM:$(SUBSYSTEM_WIN32)" \
	DEF="cknker.def"

kui:
	$(MAKE) -f ckoker.mak win32kui \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
	CFLAGS=" $(COMMON_CFLAGS) /J /DKUI /DCK_WIN /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /I." \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="-c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_WIN32)" \
	DEF="cknker.def"

# k95g debug version
k95gd:
	$(MAKE) -f ckoker.mak win32k95g \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
	OPT="" \
    DEBUG="/Zi /Odi" \
    DLL="" \
    CFLAGS=" $(COMMON_CFLAGS) /J /DKUI /DK95G /DCK_WIN /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 -I." \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="-c" \
    LINKFLAGS="/nologo /MAP /DEBUG /SUBSYSTEM:$(SUBSYSTEM_WIN32)" \
	DEF="cknker.def"

k95g:
	$(MAKE) -f ckoker.mak win32k95g \
	CC="cl /nologo" \
    CC2="" \
    OUT="-Fe" O=".obj" \
    OPT="$(COMMON_OPTS)" \
    DEBUG="-DNDEBUG" \
    DLL="" \
	CFLAGS=" $(COMMON_CFLAGS) /J /DKUI /DK95G /DCK_WIN /DWIN32 /D_WIN32_WINNT=$(WIN32_VERSION) /D_CONSOLE /D__32BIT__ /W2 /I." \
    LDFLAGS="" \
    PLATFORM="NT" \
    NOLINK="-c" \
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_WIN32) $(LDDEBUG)" \
	DEF="cknker.def"

################### OS/2 TARGETS ###################

# Old IBMC parameters - http://www.edm2.com/index.php/Icc.exe
#         ICC     WCC386
# CC2:    -Fi+    ?             Generate Precompiled Headers
#         -Si+    -Fh=<file>    Use precompiled headers where available
#         -Gi+    ?             Generate fast integer code
# OUT:    -Fe     ?             Specify the name of the executable file (/Fe<name>)
# OPT:    -O      ?             Optimize generated code
#         -Oi25   -Oe=<num>     Set the threshold for auto-inlining to <value> intermediate code instructions
# DEBUG:  -Gs     ?             Suppress stack probes in function prologs
# DLL:    -Gt-    ?             Store variables so that they do not cross 64K boundaries. Default: /Gt-
#                 -bd           Compile for DLL
#         /Ge-    ? -br         Use DLL version of C/C++ runtime library - maybe equivalent to /Ge-
#                               on the IBM compiler.
# CFLAGS: -Sp1    -zp=1         /Sp<[1]|2|4|8|16> : Pack aggregate members on specified alignment. Default: /Sp4
#         -Sm     ?             Ignore migration keywords. Default: /Sm-
#         -Gm     ? -bm         Link with multithread runtime libraries. Default: /Gm-
#         -G5     ?             /G5: Generate code optimized for use on a Pentium processor.
#         -Gt     ?             Store variables so that they do not cross 64K boundaries. Default: /Gt-
#         -Gd     ?             /Gd+: Use the version of the runtime library that is dynamically linked.
#         -J      N/A (uchar is default)          /J+: Make default char type unsigned. Default: /J+
#                 -bt=os2       Compile for target OS
# NOLINK: -c        /C+: Perform compile only, no link.
# LINKFLAGS: /nologo
#            /noi
#            /align:16
#            /base:0x10000
#                           -l=os2v2

# Watcom C targeting OS/2
# TODO: Fix buiding with OPT="-ox " (currently this causes it to crash on
# startup with trap 001 )
wcos2:
	$(MAKE) -f ckoker.mak os232 \
	    CMP="OWWCL" \
	    CC="wcl386" \
        CC2="-Fh" \
        OUT="-Fe=" O=".obj" \
	    OPT=" " \
        DEBUG="-DNDEBUG" \
        DLL="-bd" \
	    CFLAGS="-zq -zp=1 -bm -bt=os2 -aa" \
        LDFLAGS="" \
        PLATFORM="OS2" \
        NOLINK="-c" \
!ifdef WARP
        WARP="YES" \
!endif
        LINKFLAGS="-l=os2v2 -k512K" \
        LINKFLAGS_WIN="-l=os2v2_pm" \
        LINKFLAGS_DLL="-l=os2v2_dll" \
	    DEF=""  # ckoker32.def

wcos2d:
	$(MAKE) -f ckoker.mak os232 \
	    CMP="OWWCL" \
	    CC="wcl386" \
        CC2="-Fh -d3" \
        OUT="-Fe=" O=".obj" \
	    OPT=" " \
        DEBUG="-DNDEBUG" \
        DLL="-bd" \
	    CFLAGS="-zq -zp=1 -bm -bt=os2 -aa" \
        LDFLAGS="" \
        PLATFORM="OS2" \
        NOLINK="-c" \
!ifdef WARP
        WARP="YES" \
!endif
        LINKFLAGS="-l=os2v2 -k512K" \
        LINKFLAGS_WIN="-l=os2v2_pm" \
        LINKFLAGS_DLL="-l=os2v2_dll" \
	    DEF=""  # ckoker32.def

# Flags are:
#   --aa            Allows non-const initializers for local aggregates or unions.
#                   Required to fix initialisation of viocell with geterasecolor()
#                   in a few places.
#   -Fh=<file>      Use precompiled headers where available. Equivalent to -Si+
#                   on the IBM compiler.
#   -zp=1           Struct packing align. Equivalent to -Sp1 on the IBM compiler.
#   -bm             Build multithreaded application. Should be the same as -Gm on
#                   the IBM compiler.
#   -Fe=<file>      Output executable filename
#   -ox             Maximum optimisation
#   -bd             Compile for DLL
#   -br             Use DLL version of C/C++ runtime library - maybe equivalent to /Ge-
#                   on theIBM compiler.
#   -zq             Operate quietly
#   -bt=os2         Compile for OS/2 (rather than DOS/NetWare/Windows/QNX/whatever)
#   -c              Compile only, don't link
#   -l=os2v2        Link for 32bit OS/2

# release version
#         CC2="-Fi+ -Si+ -Gi+ -Gl+" \
#         add /Gn+ back to hide the default library info after I figure out how to build the runtime library dll

# Targets for the IBM compiler
ibmc:
	$(MAKE) -f ckoker.mak os232 \
	CC="icc -q" \
        CC2="-Fi+ -Si+ -Gi+" \
        OUT="-Fe" O=".obj" \
	OPT="-O -Oi25" \
        DEBUG="-Gs -DNDEBUG" \
        DLL="-Gt- /Ge-" \
	CFLAGS="-Sp1 -Sm -Gm -G5 -Gt -Gd -J" \
        LDFLAGS="" \
        PLATFORM="OS2" \
        NOLINK="-c" \
!ifdef WARP
        WARP="YES" \
        LINKFLAGS="/nologo /noi /align:16 /base:0x10000" \
!else
        LINKFLAGS="/nologo /noi /align:16 /base:0x10000" \
!endif
	DEF="ckoker32.def"

# source browser
ibmsb:
	$(MAKE) -f ckoker.mak os232 \
	CC="sb" \
        CC2="" \
        OUT="-Fo" O="._sb" \
	OPT="" \
        DEBUG="" \
        DLL="" \
	CFLAGS="" \
        LDFLAGS="" \
        PLATFORM="OS2" \
        NOLINK="" \
!ifdef WARP
        WARP="YES" \
!endif
        LINKFLAGS="" \
	DEF=""

# profiling version
ibmcp:
	$(MAKE) -f ckoker.mak os232 \
	CC="icc -q" \
        CC2="-Fi+ -Si+ -Gi+ /Gl+"\
        OUT="-Fe" O=".obj" \
	OPT="-O -Oi25" \
        DEBUG="-Gh -Ti -DNDEBUG" \
        DLL="-Gt- /Ge-" \
	CFLAGS="-Sp1 -Sm -Gm -G5 -Gt -Gd -J" \
        LDFLAGS="dde4xtra.obj" \
        PLATFORM="OS2" \
        NOLINK="-c" \
        LINKFLAGS="/nologo /noi /align:16 /base:0x10000" \
	DEF="ckoker32.def"

# debugging version
ibmcd:
	$(MAKE) -f ckoker.mak os232 \
	CC="icc -q" \
        CC2=""\
        OUT="-Fe" O=".obj" \
	OPT="" \
        DEBUG="-Ti+ -Tx+ -Tm+ -D__DEBUG" \
        DEBUG2="/Wcmp /Wcnd /Wcns /Wdcl \
            /Weff /Wenu /Wext /Wgnr /Word /Wpar /Wppc /Wpro /Wrea \
            /Wret /Wtrd /Wund /Wuni /Wuse" \
        DLL="-Gt- /Ge-" \
	CFLAGS="-Sp1 -Sm -Gm -G5 -Gt -Gd -J" \
        PLATFORM="OS2" \
        LDFLAGS="" \
        NOLINK="-c" \
        LINKFLAGS="/nologo /noi /align:16 /base:0x10000 /debug /dbgpack" \
	DEF="ckoker32.def"

#---------- Macros:

# To build without NETWORK support, uncomment the following line and
# then comment out the next: (save 60K)
!if "$(PLATFORM)" == "OS2"
#DEFINES = -DOS2 -DDYNAMIC -DKANJI -DOS2MOUSE -DPCFONTS\
#          -DONETERMUPD 
!else if "$(PLATFORM)" == "NT"
#DEFINES = -DNT -DOS2 -DDYNAMIC -DKANJI -DOS2MOUSE \
#          -DONETERMUPD
!endif

# To build with NETWORK support, uncomment the following three 
# lines and comment out the previous set:
!ifdef PLATFORM
!if "$(PLATFORM)" == "OS2"
DEFINES = -DOS2 -DDYNAMIC -DKANJI -DTCPSOCKET \
          -DNPIPE -DOS2MOUSE -DHADDRLIST -DPCFONTS \
          -DRLOGCODE -DNETFILE -DONETERMUPD \
          $(ENABLED_FEATURE_DEFS) $(DISABLED_FEATURE_DEFS) \
!if "$(CMP)" == "OWWCL"
          -D__32BIT__
!endif
# Open Watcom doesn't define __32BIT__ by default which upsets a lot of OS/2
# code. Maybe there is some compiler flag thats missing though it seems to be
# producing 32bit OS/2 binaries fine as-is.

# zlib support:  -DZLIB
           
!else if "$(PLATFORM)" == "NT"
!ifndef K95BUILD
K95BUILD = K95
!endif
!if "$(K95BUILD)" == "TLSONLY"
DEFINES = -DNT -D__STDC__ -DWINVER=0x0400 -DOS2 -DNOSSH \
          -DDYNAMIC -DNETCONN -DHADDRLIST -DOS2MOUSE -DTCPSOCKET -DRLOGCODE -DUSE_STRERROR \
          -DNETFILE -DONETERMUPD -DNO_ENCRYPTION -DZLIB \
          -DNO_SRP -DNO_KERBEROS -DBETATEST -DNOCKXYZ
!else if "$(K95BUILD)" == "UIUC"
DEFINES = -DNT -D__STDC__ -DWINVER=0x0400 -DOS2 -DNOSSH \
          -DDYNAMIC -DNETCONN -DHADDRLIST -DOS2MOUSE -DTCPSOCKET -DRLOGCODE -DUSE_STRERROR \
          -DNETFILE -DONETERMUPD -DZLIB \
          -DNOXFER -DNODIAL -DNOHTTP -DNOFORWARDX -DNOBROWSER -DNOLOGIN \
          -DNOCYRIL -DNOKANJI -DNOHEBREW -DNOGREEK -DNOLOGIN -DNOIKSD -DNOHELP \
          -DNOSOCKS -DNONETCMD -DNO_SRP -DNO_SSL -DNOFTP -DBETATEST \
          -DNODEBUG -DCK_TAPI -DNOPUSH -DNO_COMPORT -DNOXMIT -DNOSCRIPT -DNO_KERBEROS -DNOCKXYZ
!else if "$(K95BUILD)" == "IKSD"
DEFINES = -DNT -D__STDC__ -DWINVER=0x0400 -DOS2 -DNOSSH -DONETERMUPD -DUSE_STRERROR \
          -DDYNAMIC -DKANJI -DNETCONN -DIKSDONLY -DZLIB \
          -DHADDRLIST -DCK_LOGIN \
          -DNO_SRP -DNO_KERBEROS -DNOCKXYZ
		  #-DBETATEST # -DPRE_SRP_1_7_3
!else
DEFINES = -DNT -DWINVER=0x0400 -DOS2 -D_CRT_SECURE_NO_DEPRECATE -DUSE_STRERROR\
          -DDYNAMIC -DKANJI \
          -DHADDRLIST -DNPIPE -DOS2MOUSE -DTCPSOCKET -DRLOGCODE \
          -DNETFILE -DONETERMUPD  \
          -DNEWFTP -DBETATEST -DNO_DNS_SRV \
          $(ENABLED_FEATURE_DEFS) $(DISABLED_FEATURE_DEFS)
!if "$(CMP)" != "OWCL"
DEFINES = $(DEFINES) -D__STDC__
!endif
!endif
!endif
!else
! ERROR Macro named PLATFORM undefined
!endif

!if "$(CMP)" == "OWCL"
# Watcom was the full path to commode.obj - its not enough for it to
# be on the library path.
COMMODE_OBJ = $(WATCOM)\lib386\nt\commode.obj
!else
COMMODE_OBJ = commode.obj
!endif

!ifdef PLATFORM
!if "$(PLATFORM)" == "OS2"
LIBS = os2386.lib rexx.lib

# Open Watcom doesn't have bigmath.lib
#  -> this likely comes from the SRP distribution (srp\srp-1.4\cryptolib_1.1\VISUALC\BIGMATH)
!if "$(CMP)" != "OWCL" && "$(CMP)" != "OWWCL"
LIBS = $(LIBS) bigmath.lib
!endif

!if "$(CKF_SRP)" == "yes"
LIBS = $(LIBS) libsrp.lib
!endif

!else if "$(PLATFORM)" == "NT"
!if "$(K95BUILD)" == "UIUC"
LIBS = kernel32.lib user32.lib gdi32.lib wsock32.lib \
       winmm.lib mpr.lib advapi32.lib winspool.lib

# wshelper (via wshload) is required for DNS-SRV support
!if "$(CKF_K4W_WSHELPER)" == "yes"
LIBS = $(LIBS) wshload.lib
!endif

!else
KUILIBS = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
        advapi32.lib shell32.lib rpcrt4.lib rpcns4.lib wsock32.lib \
        winmm.lib comctl32.lib mpr.lib $(COMMODE_OBJ)
# vdmdbg.lib
!if "$(CKF_SSH)" == "yes" && "$(CKF_DYNAMIC_SSH)" != "yes"
KUILIBS = $(KUILIBS) $(SSH_LIB) ws2_32.lib
!endif

!if "$(CKF_REXX)" == "yes"
KUILIBS = $(KUILIBS) $(REXX_LIB)
!endif

!if "$(CKF_SSL)" == "yes"
KUILIBS = $(KUILIBS) $(SSL_LIBS)
!endif

!if ($(MSC_VER) > 80)
# I doubt these are actually ever required. But if they ever are, they're only
# required when building with Visual C++ 2.0 or newer (1.0 32-bit doesn't have
# them)
KUILIBS = $(KUILIBS) ole32.lib oleaut32.lib uuid.lib
!endif

#msvcrt.lib

!if "$(CKF_SRP)" == "yes"
# K95 2.1.3 was built with srpstatic.lib
#KUILIBS = $(KUILIBS) srpstatic.lib
KUILIBS = $(KUILIBS) srp.lib
!endif

# wshelper (via wshload) is required for DNS-SRV support
!if "$(CKF_K4W_WSHELPER)" == "yes"
KUILIBS = $(KUILIBS) wshload.lib
!endif

!if "$(MIPS_CENTAUR)" == "yes"
KUILIBS = $(KUILIBS) libcmt.lib
!endif

# Commented out KUILIBS in K95 2.1.3: msvcrt.lib libsrp.lib bigmath.lib

LIBS = kernel32.lib user32.lib gdi32.lib wsock32.lib shell32.lib\
       winmm.lib mpr.lib advapi32.lib winspool.lib $(COMMODE_OBJ)

!if "$(CKF_SSH)" == "yes" && "$(CKF_DYNAMIC_SSH)" != "yes"
LIBS = $(LIBS) $(SSH_LIB) ws2_32.lib
!endif

!if "$(CKF_REXX)" == "yes"
LIBS = $(LIBS) $(REXX_LIB)
!endif

!if "$(MIPS_CENTAUR)" == "yes"
LIBS = $(LIBS) libcmt.lib
!endif

!if "$(CKF_SSL)" == "yes"
LIBS = $(LIBS) $(SSL_LIBS)
!endif

!if "$(CKF_SRP)" == "yes"
# K95 2.1.3 was built with srpstatic.lib
#LIBS = $(LIBS) srpstatic.lib
LIBS = $(LIBS) srp.lib
!endif

# wshelper (via wshload) is required for DNS-SRV support
!if "$(CKF_K4W_WSHELPER)" == "yes"
LIBS = $(LIBS) wshload.lib
!endif

# Visual C++ 2005 for IA64 in the Windows Server 2003 SP1 Platform SDK
# seems to want this extra library otherwise we get link errors like:
#   error LNK2001: unresolved external symbol .__security_check_cookie
!if "$(TARGET_CPU)" == "IA64" && $(MSC_VER) < 150
LIBS = $(LIBS) bufferoverflowu.lib
KUILIBS = $(KUILIBS) bufferoverflowu.lib
!endif

# Commented out LIBS in K95 2.1.3: msvcrt.lib libsrp.lib bigmath.lib

!endif
!endif
!endif

#---------- Inference rules:

.SUFFIXES: .w .c $(O)

.c$(O):
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) $(DEFINES) $(NOLINK) $*.c

#---------- Targets:

OBJS =  ckcmai$(O) ckcfns$(O) ckcfn2$(O) ckcfn3$(O) ckcnet$(O) ckcpro$(O) \
        ckucmd$(O) ckudia$(O) ckofio$(O) ckuscr$(O) ckuusr$(O) ckuus2$(O) \
        ckuus3$(O) ckuus4$(O) ckuus5$(O) ckuus6$(O) ckuus7$(O) ckuusx$(O) \
        ckuusy$(O) ckuxla$(O) ckclib$(O) ckctel$(O) ckcuni$(O) ckcftp$(O) \
!if "$(PLATFORM)" == "NT"
        cknsig$(O) cknalm$(O) ckntap$(O) cknwin$(O) cknprt$(O) cknpty$(O) \
!else
        ckusig$(O) \
!endif
        ckuath$(O) ckoath$(O) ck_ssl$(O) ckossl$(O) ckosslc$(O) \
        ckosftp$(O) ckozli$(O) \
!if 0
        ck_crp$(O) ck_des$(O) \
!endif
!if ("$(CKF_SSH)" == "yes")
        ckossh$(O) \
!if ("$(CKF_DYNAMIC_SSH)" != "yes")
        ckolssh$(O) ckorbf$(O) ckolsshs$(O) \
!endif
!endif
        ckocon$(O) ckoco2$(O) ckoco3$(O) ckoco4$(O) ckoco5$(O) \
        ckoetc$(O) ckoetc2$(O) ckokey$(O) ckomou$(O) ckoreg$(O) \
        ckonet$(O) \
        ckoslp$(O) ckosyn$(O) ckothr$(O) ckotek$(O) ckotio$(O) ckowys$(O) \
        ckodg$(O)  ckoava$(O) ckoi31$(O) ckotvi$(O) ckovc$(O) \
        ckoadm$(O) ckohzl$(O) ckohp$(O) ckoqnx$(O)
!if "$(PLATFORM)" == "NT"
OBJS = $(OBJS) cknnbi$(O)
!else
OBJS = $(OBJS) ckonbi$(O)
!endif
!if ("$(CKF_XYZ)" == "yes")
OBJS = $(OBJS) ckop$(O) p_callbk$(O) p_global$(O) p_omalloc$(O) p_error$(O) \
        p_common$(O) p_tl$(O) p_dir$(O)
!endif

# Internal cryptography (instead of k95crypt.dll)
!if "$(CKF_INTERNAL_CRYPT)" == "yes"
# TODO: If being built for internal, don't build ck_crp and ck_des with $(DLL)
OBJS = $(OBJS) ck_crp.obj ckclib.obj ck_des.obj
LIBS = $(LIBS) libdes.lib
KUILIBS = $(KUILIBS) libdes.lib
!endif

#OUTDIR = \kui\win95
KUIOBJS = \
    $(OUTDIR)\kregedit.obj $(OUTDIR)\ksysmets.obj     \
    $(OUTDIR)\ikui.obj     $(OUTDIR)\kprogres.obj \
    $(OUTDIR)\ikterm.obj   $(OUTDIR)\ikcmd.obj    $(OUTDIR)\kuidef.obj   \
    $(OUTDIR)\karray.obj   $(OUTDIR)\khwndset.obj $(OUTDIR)\kwin.obj     \
    $(OUTDIR)\kszpopup.obj $(OUTDIR)\kflstat.obj  $(OUTDIR)\kcustdlg.obj \
    $(OUTDIR)\kmenu.obj    $(OUTDIR)\kstatus.obj  $(OUTDIR)\ktoolbar.obj \
    $(OUTDIR)\kscroll.obj  \
    $(OUTDIR)\kfont.obj    \
    $(OUTDIR)\kfontdlg.obj $(OUTDIR)\kabout.obj   \
    $(OUTDIR)\kdwnload.obj \
    $(OUTDIR)\kuikey.obj   $(OUTDIR)\kclient.obj  \
    $(OUTDIR)\kappwin.obj  \
    $(OUTDIR)\ktermin.obj  $(OUTDIR)\kui.obj


os232: ckoker32.exe tcp32 otelnet.exe ckoclip.exe orlogin.exe osetup.exe otextps.exe k2dc.exe \
!if "$(CMP)" != "OWWCL"
       cko32rtl.dll \    # IBM compiler only.
!endif
!if "$(CKF_SRP)" == "yes"
#!if "$(CKF_SSL)" == "yes"
       srp-tconf.exe srp-passwd.exe \
#!endif
!endif
!if "$(CKF_CRYPTDLL)" == "yes"
# TODO: Figure out how to build this for OS/2 with Watcom: k2crypt.dll
!endif

# docs pcfonts.dll



win32: cknker.exe wtelnet wrlogin k95d textps ctl3dins.exe iksdsvc.exe iksd.exe \
    se.exe \
!if "$(CKF_CRYPTDLL)" == "yes"
    k95crypt.dll \
!endif
!if "$(CKF_DYNAMIC_SSH)" == "yes"
!if "$(CKF_SSH_BACKEND)" != "no"
    k95ssh.dll nullssh.dll \
!endif
!endif
# These likely require an old version of SRP (perhaps pre-1.7?) to build. They
# appear to just be versions of utilities that come with SRP likely modified to
# load the SSL DLL dynamically like K95 did - not really something we care much
# about now.
#!if "$(CKF_SRP)" == "yes"
#       srp-passwd.exe srp-tconf.exe
#!endif

!if "$(CKF_DYNAMIC_SSH)" == "yes"
win32sshdll: k95ssh.dll
!endif

win32md: mdnker.exe

win32kui: cknkui.exe

win32k95g: k95g.exe

iksdnt: iksdnt.exe

wsetup: setup.exe

wtest: test.exe

wtelnet: telnet.exe

wrlogin: rlogin.exe

k95d: k95d.exe

textps: textps.exe

# Remove the DLLs you don't have the Development Kits for:
# IBM TCP/IP 2.0          - cko32i20.dll
# IBM TCP/IP 1.2          - cko32i12.dll 
# FTP Software PC/TCP 1.3 - cko32i13.dll 
# Novell LWP OS/2 3.0     - cko32n30.dll 

tcp32: $(OS2TCPDLLS)
# cko32i41.dll    - for IBM TCP/IP 4.1+ (MPTS 5.3+), the "new" 32-bit TCP/IP stack
# cko32i20.dll    - for IBM TCP/IP 2.0-4.0
# cko32i12.dll    - for IBM TCP/IP 1.2.1
# cko32f13.dll    - for FTP PC/TCP 1.3
# cko32n30.dll    - for Novell LAN Workplace 3.0 (cko32n30.dll was never released)

cknker.exe: $(OBJS) cknker.res $(DEF) ckoker.mak 
#        $(CC) $(CC2) /link "$(LINKFLAGS)" $(DEBUG) $(OBJS) $(DEF) $(OUT) $@ $(LIBS) $(LDFLAGS)
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ $(OBJS) cknker.res $(LIBS) 
<<

iksdnt.exe: $(OBJS) cknker.res $(DEF) ckoker.mak 
#        $(CC) $(CC2) /link "$(LINKFLAGS)" $(DEBUG) $(OBJS) $(DEF) $(OUT) $@ $(LIBS) $(LDFLAGS)
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ $(OBJS) cknker.res $(LIBS) 
<<

mdnker.exe: $(OBJS) ckcmdb$(O) cknker.res $(DEF) ckoker.mak 
#        $(CC) $(CC2) /link "$(LINKFLAGS)" $(DEBUG) ckcmdb$(O) $(OBJS) $(DEF) $(OUT) $@ $(LIBS) $(LDFLAGS)
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ ckcmdb$(O) $(OBJS) cknker.res $(LIBS) 
<<

k95g.exe: $(OBJS) $(KUIOBJS) cknker.res $(DEF) ckoker.mak 
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ $(OBJS) $(KUIOBJS) $(OUTDIR)\kui.res $(KUILIBS) 
<<

cknkui.exe: $(OBJS) $(KUIOBJS) cknker.res $(DEF) ckoker.mak 
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ $(OBJS) $(KUIOBJS) $(OUTDIR)\kui.res $(KUILIBS) 
<<

setup.exe: setup.obj settapi.obj $(DEF) ckoker.mak 
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ setup.obj settapi.obj cknker.res $(LIBS) 
<<

test.exe: test.obj $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ test.obj cknker.res $(LIBS) 
<<

telnet.exe: telnet.obj telnet.res $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ telnet.obj telnet.res $(LIBS)
<<

rlogin.exe: rlogin.obj rlogin.res $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ rlogin.obj rlogin.res $(LIBS)
<<

se.exe: se.obj se.res $(DEF) ckoker.mak
       link.exe @<<
       $(LINKFLAGS) /OUT:$@ se.obj se.res $(LIBS)
<<

k2dc.exe: k2dc.obj $(DEF) ckoker.mak
!if "$(CMP)" == "OWWCL"
        $(CC) $(CC2) $(LINKFLAGS) k2dc.obj $(OUT)$@ $(LDFLAGS) $(LIBS)
!else
      	$(CC) $(CC2) /B"$(LINKFLAGS)" k2dc.obj $(OUT) $@ $(LDFLAGS) $(LIBS)
!endif

orlogin.exe: rlogin.obj $(DEF) ckoker.mak
!if "$(CMP)" == "OWWCL"
        $(CC) $(CC2) $(LINKFLAGS) rlogin.obj $(OUT)$@ $(LDFLAGS) $(LIBS)
!else
      	$(CC) $(CC2) /B"$(LINKFLAGS)" rlogin.obj $(OUT) $@ $(LDFLAGS) $(LIBS)
!endif

otextps.exe: textps.obj $(DEF) ckoker.mak
!if "$(CMP)" == "OWWCL"
        $(CC) $(CC2) $(LINKFLAGS) textps.obj $(OUT)$@ $(LDFLAGS) $(LIBS)
!else
      	$(CC) $(CC2) /B"$(LINKFLAGS)" textps.obj $(OUT) $@ $(LDFLAGS) $(LIBS)
!endif

k95d.exe: k95d.obj k95d.res $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ k95d.obj k95d.res $(LIBS)
<<

ctl3dins.exe: ctl3dins.obj ctl3dins.res $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ ctl3dins.obj ctl3dins.res $(LIBS) VERSION.LIB
<<

textps.exe: textps.obj textps.res $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ textps.obj textps.res $(LIBS)
<<

#       ckoker.msb  -- no idea what this is
ckoker32.exe: $(OBJS) $(DEF) ckoker.res ckoker.mak
!if "$(CMP)" == "OWWCL"
        $(CC) $(CC2) $(LINKFLAGS) $(DEBUG) $(OBJS) $(DEF) $(OUT)$@ $(LIBS) $(LDFLAGS)
        wrc -q -bt=os2 ckoker.res $@
!else
        $(CC) $(CC2) /B"$(LINKFLAGS)" $(DEBUG) $(OBJS) $(DEF) $(OUT) $@ $(LIBS) $(LDFLAGS)
!ifdef WARP
       rc -p -x2 ckoker.res $@
!else
       rc -p -x1 ckoker.res $@
!endif
       dllrname $@ CPPRMI36=CKO32RTL
!endif

cko32rtl.dll:
        copy $(VISUALAGE)\RUNTIME\CPPRMI36.DLL cko32rtl.dll
        dllrname $@ CPPRMI36=CKO32RTL

cko32rtl.lib: cko32rtl.dll cko32rt.def cko32rt.c
        ILIB /GI cko32rt.dll
        ILIB /NOBR /OUT:cko32rt.lib $(VISUALAGE)\LIB\CPPRNO36.LIB

# cko32i20.def
# TODO: What libs are required for the IBM compiler when targeting TCP-32?
cko32i41.dll: ckoi41.obj ckoker.mak
!if "$(CMP)" == "OWWCL"
    $(CC) $(CC2) $(DEBUG) $(DLL) ckoi41.obj $(OUT)$@ \
	 $(LINKFLAGS_DLL) tcpip32.lib $(LIBS)
!else
	$(CC) $(CC2) $(DEBUG) $(DLL) ckoi41.obj cko32i41.def $(OUT) $@ \
	/B"/noe /noi" $(IBM20LIBS) $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL
!endif

# cko32i20.def
# TODO: WATCOM: I'm really not sure about the "ALIAS __res=_res" bit. It makes
#       it link and it seems to work on Warp 4 at least, but surely having to
#       do this is a sign there is some other issue elsewhere.
cko32i20.dll: ckoi20.obj ckoker.mak
!if "$(CMP)" == "OWWCL"
    $(CC) $(CC2) $(DEBUG) $(DLL) ckoi20.obj $(OUT)$@ \
	 $(LINKFLAGS_DLL) $(IBM20LIBS) $(LIBS) -"ALIAS __res=_res"
!else
	$(CC) $(CC2) $(DEBUG) $(DLL) ckoi20.obj cko32i20.def $(OUT) $@ \
	/B"/noe /noi" $(IBM20LIBS) $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL
!endif

# cko32i12.def
cko32i12.dll: ckoi12.obj  ckoker.mak
!if "$(CMP)" == "OWCL386"
    $(CC) $(CC2) $(DEBUG) $(DLL) ckoi12.obj $(OUT)$@ \
	 -bd -l=os2v2_dll $(IBM12LIBS) $(LIBS)
!else
	$(CC) $(CC2) $(DEBUG) $(DLL) ckoi12.obj cko32i12.def $(OUT) $@ \
	/B"/noe /noi" $(IBM12LIBS) $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL       
!endif

cko32f13.dll: ckof13.obj cko32f13.def ckoker.mak
	$(CC) $(CC2) $(DEBUG) $(DLL) ckof13.obj cko32f13.def $(OUT) $@ \
	/B"/noe /noi" $(FTP13LIBS32) $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL       

cko32n30.dll: ckon30.obj cko32n30.def ckoker.mak
	$(CC) $(CC2) $(DEBUG) $(DLL) ckon30.obj cko32n30.def $(OUT) $@ \
	/B"/noe /noi" $(LWP30LIBS32) $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL       

pcfonts.dll: ckopcf.obj cko32pcf.def ckopcf.res ckoker.mak
	$(CC) $(CC2) $(DEBUG) $(DLL) ckopcf.obj \
        cko32pcf.def $(OUT) $@ $(LIBS)
!ifdef WARP
        rc -p -x2 ckopcf.res pcfonts.dll
!else
        rc -p -x1 ckopcf.res pcfonts.dll
!endif

k95crypt.dll: ck_crp.obj ck_des.obj ckclib.obj ck_crp.def ckoker.mak k95crypt.res
	link /dll /debug /def:ck_crp.def /out:$@ ck_crp.obj ckclib.obj ck_des.obj \
	    libdes.lib \
!if "$(PLATFORM)" != "OS2"
	    k95crypt.res \
!endif
!if "$(TARGET_CPU)" == "IA64" && $(MSC_VER) < 150
        bufferoverflowu.lib
!endif

nullssh.dll: ckonssh.obj ckoker.mak
	link /dll /debug /def:nullssh.def /out:$@ ckonssh.obj

k95ssh.dll: ckolssh.obj ckolsshs.obj ckorbf.obj k95ssh.res ckoker.mak
	link /dll /debug /def:k95ssh.def /out:$@ ckolssh.obj ckolsshs.obj \
	    ckorbf.obj k95ssh.res $(SSH_LIB) ws2_32.lib

k2crypt.dll: ck_crp.obj ck_des.obj ckclib.obj k2crypt.def ckoker.mak
	ilink /nologo /noi /exepack:1 /align:16 /base:0x10000 k2crypt.def \
            /out:$@ ck_crp.obj ck_des.obj ckclib.obj libdes.lib
        dllrname $@ CPPRMI36=CKO32RTL

docs:   ckermit.inf

# ckotel.def
otelnet.exe: ckotel.obj ckoker.mak
!if "$(CMP)" == "OWWCL"
        $(CC) $(CC2) $(DEBUG) ckotel.obj $(LINKFLAGS) $(OUT)$@ $(LIBS)
!else
        $(CC) $(CC2) $(DEBUG) ckotel.obj ckotel.def $(OUT) $@ $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL
!endif

osetup.exe: setup.obj osetup.def ckoker.mak
!if "$(CMP)" == "OWWCL"
        $(CC) $(DEBUG) setup.obj $(LINKFLAGS) $(OUT)$@
!else
        $(CC) $(DEBUG) setup.obj osetup.def $(OUT) $@
!endif

# ckoclip.def
ckoclip.exe: ckoclip.obj ckoker.mak ckoclip.res
!if "$(CMP)" == "OWWCL"
        $(CC) $(CC2) $(LINKFLAGS_WIN) $(DEBUG) ckoclip.obj $(OUT)$@ $(LIBS)
        wrc -q -bt=os2 ckoclip.res $@
!else
        $(CC) $(CC2) $(DEBUG) ckoclip.obj ckoclip.def $(OUT) $@ $(LIBS)
!ifdef WARP
       rc -p -x2 ckoclip.res $@
!else
       rc -p -x1 ckoclip.res $@
!endif
        dllrname $@ CPPRMI36=CKO32RTL
!endif

# SRP support
!if "$(CKF_SRP)" == "yes"
srp-tconf.exe: srp-tconf.obj getopt.obj ckosslc.obj ckoker.mak
!if "$(PLATFORM)" == "OS2"
        $(CC) $(CC2) $(DEBUG) srp-tconf.obj getopt.obj ckosslc.obj ckotel.def $(OUT) $@ $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL
!else if "$(PLATFORM)" == "NT"
	link /debug /out:$@ srp-tconf.obj getopt.obj ckosslc.obj $(LIBS)
!endif

srp-passwd.exe: srp-passwd.obj getopt.obj ckosslc.obj ckoker.mak
!if "$(PLATFORM)" == "OS2"
        $(CC) $(CC2) $(DEBUG) srp-passwd.obj getopt.obj ckosslc.obj ckotel.def $(OUT) $@ $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL
!else if "$(PLATFORM)" == "NT"
	link /debug /out:$@ srp-passwd.obj getopt.obj ckosslc.obj $(LIBS)
!endif
!endif
        
iksdsvc.exe: iksdsvc.obj iksdsvc.res ckoker.mak
!if "$(PLATFORM)" == "OS2"
!else if "$(PLATFORM)" == "NT"
	link /debug /out:$@ iksdsvc.obj iksdsvc.res $(LIBS)
!endif
        
iksd.exe: iksd.obj iksd.res ckoker.mak
!if "$(PLATFORM)" == "OS2"
!else if "$(PLATFORM)" == "NT"
	link /debug /out:$@ iksd.obj iksd.res $(LIBS)
!endif
        

#---------- Dependencies:

!if "$(PLATFORM)" == "OS2"
setup$(O):	setup.c
	$(CC) $(CC2) $(CFLAGS) /Gd- /Gn- $(DEBUG) $(DEFINES) $(NOLINK) setup.c
!else
setup$(O):	setup.c ckcdeb.h ckoker.h ckcker.h ckucmd.h ckuusr.h ckowin.h ckntap.h
	$(CC) $(CC2) $(CFLAGS) $(DEFINES) $(DEBUG) $(OPT) /Gn- -c setup.c

settapi$(O):  settapi.c 
	$(CC) $(CC2) $(CFLAGS) $(DEFINES) $(DEBUG) $(OPT) /Gn- -c settapi.c

!endif

test$(O):	test.c

telnet$(O):	telnet.c

rlogin$(O):	rlogin.c

textps$(O):     textps.c

ckcmai$(O):	ckcmai.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcsym.h ckcnet.h ckctel.h \
                ckuusr.h ckonet.h ckcsig.h ckocon.h ckntap.h ckocon.h ck_ssl.h ckossl.h ckover.h
ckcmdb$(O):     ckcmdb.c ckcsym.h ckcdeb.h ckoker.h
ckclib$(O):     ckclib.c ckcsym.h ckcdeb.h ckoker.h ckclib.h ckcasc.h
ckcfns$(O):	ckcfns.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcsym.h ckcxla.h ckuxla.h \
                ckcnet.h
ckcfn2$(O):	ckcfn2.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcsym.h ckcxla.h ckuxla.h \
                ckcnet.h ckctel.h
ckcfn3$(O):	ckcfn3.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcsym.h ckcxla.h ckuxla.h
ckcpro$(O):	ckcpro.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcnet.h ckctel.h
ckcuni$(O):     ckcuni.c ckcsym.h ckcdeb.h ckoker.h ckcker.h ckucmd.h ckcxla.h ckuxla.h 
ckuxla$(O):	ckuxla.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcxla.h ckuxla.h
ckucmd$(O):	ckucmd.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckucmd.h ckuusr.h ckcnet.h \
                ckctel.h
ckudia$(O):	ckudia.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckucmd.h ckuusr.h \
            ckcsig.h ckocon.h cknwin.h ckowin.h ckntap.h ckcnet.h ckctel.h
ckuscr$(O):	ckuscr.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckcsig.h ckcnet.h \
                ckctel.h
ckuusr$(O):	ckuusr.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		    ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckocon.h cknwin.h ckossh.h \
	        ckowin.h ckntap.h kui\ikui.h
ckuus2$(O):	ckuus2.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		  ckcxla.h ckuxla.h ckokvb.h ckocon.h ckokey.h ckcnet.h ckctel.h ckover.h
ckuus3$(O):	ckuus3.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		  ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckonbi.h ckntap.h ckoreg.h \
          ckocon.h ckokey.h ckokvb.h ckcuni.h ck_ssl.h ckossl.h ckuath.h kui\ikui.h \
          ckossh.h
ckuus4$(O):	ckuus4.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		  ckcxla.h ckuxla.h ckuver.h ckcnet.h ckctel.h ckonet.h ckocon.h ckossh.h \
	      ckoetc.h ckntap.h ckuath.h ck_ssl.h ckoreg.h ckoetc.h
ckuus5$(O):	ckuus5.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
            ckocon.h ckokey.h ckokvb.h ckcuni.h ckcnet.h ckctel.h ck_ssl.h ckossl.h \
            ckossh.h kui\ikui.h
ckuus6$(O):	ckuus6.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h ckntap.h \
                ckcnet.h ckctel.h
!if "$(PLATFORM)" == "OS2"
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(DEFINES) $(NOLINK) ckuus6.c

!endif
ckuus7$(O):	ckuus7.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		    ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckocon.h ckodir.h \
            ckokey.h ckokvb.h cknwin.h ckowin.h ckntap.h ckcuni.h ckossh.h \
            ckntap.h ckuath.h ck_ssl.h kui\ikui.h
ckuusx$(O):	ckuusx.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckonbi.h \
                ckocon.h cknwin.h ckowin.h ckntap.h ckcnet.h ckctel.h kui\ikui.h
ckuusy$(O):	ckuusy.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h ckcnet.h ckctel.h \
	        ck_ssl.h ckossh.h kui\ikui.h
ckofio$(O):	    ckofio.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckuver.h ckodir.h ckoker.h \
                ckuusr.h ckcxla.h ck_ssl.h ckoreg.h ckosyn.h ckuath.h
ckoava$(O):     ckoava.c ckoava.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h
ckocon$(O):	    ckocon.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckoker.h ckocon.h ckcnet.h \
                ckctel.h ckonbi.h ckokey.h ckokvb.h ckuusr.h cknwin.h ckowin.h ckcuni.h ckossh.h \
                kui\ikui.h
ckoco2$(O):     ckoco2.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckoker.h ckocon.h \
                ckonbi.h ckopcf.h ckuusr.h ckokey.h ckokvb.h ckcuni.h kui\ikui.h
ckoco3$(O):     ckoco3.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckoker.h ckocon.h \
                ckokey.h ckokvb.h ckuusr.h ckowys.h ckodg.h  ckoava.h ckoi31.h \
                ckohp.h  ckoadm.h ckohzl.h ckoqnx.h ckotvi.h ckovc.h  ckcuni.h \
                ckcnet.h ckctel.h kui\ikui.h ckossh.h
ckoco4$(O):     ckoco4.c ckcdeb.h ckoker.h ckclib.h ckocon.h ckokey.h ckokvb.h ckuusr.h ckcasc.h \
                ckokey.h ckokvb.h
ckoco5$(O):     ckoco5.c ckcdeb.h ckoker.h ckclib.h ckocon.h 
ckodg$(O):      ckodg.c  ckodg.h  ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h \
                ckcnet.h ckctel.h
ckoetc$(O):     ckoetc.c ckcdeb.h ckoker.h ckclib.h ckoetc.h
ckoetc2$(O):    ckoetc2.c ckcdeb.h ckoker.h ckclib.h ckoetc.h
ckohp$(O):      ckohp.c  ckohp.h  ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h \
                ckokey.h ckokvb.h
ckohzl$(O):     ckohzl.c ckohzl.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h \
                ckcnet.h ckctel.h ckcuni.h
ckoadm$(O):     ckoadm.c ckoadm.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h \
                ckcnet.h ckctel.h ckcuni.h
ckoi31$(O):     ckoi31.c ckoi31.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h ckcxla.h
ckokey$(O):     ckokey.c ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcker.h ckuusr.h ckctel.h \
                ckocon.h ckokey.h ckokvb.h ckcxla.h ckuxla.h ckcuni.h kui\ikui.h
ckoqnx$(O):     ckoqnx.c ckoqnx.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h ckcuni.h
ckotek$(O): ckotek.c ckotek.h ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckoker.h ckocon.h \
                ckokey.h ckokvb.h ckuusr.h ckcnet.h ckctel.h
ckotio$(O):	ckotio.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckuver.h ckodir.h ckoker.h \
                ckocon.h ckokey.h ckokvb.h ckuusr.h ckoslp.h ckcsig.h ckop.h \
                ckcuni.h ckowin.h ckcnet.h ckctel.h ckoreg.h \
!if "$(PLATFORM)" == "NT"
                ckntap.h cknwin.h  kui\ikui.h
!else

!endif
ckotvi$(O):     ckotvi.c ckotvi.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h \
                ckctel.h ckokvb.h
ckovc$(O):      ckovc.c  ckovc.h  ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h ckcuni.h
ckowys$(O):     ckowys.c ckowys.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h \
                ckcuni.h ckokey.h ckokvb.h ckctel.h
ckcnet$(O):	ckcnet.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h ckonet.h ckotcp.h \
                ckuusr.h ckcsig.h ckocon.h ckuath.h ck_ssl.h ckossl.h ckosslc.h
ckcftp$(O):     ckcftp.c ckcdeb.h ckoker.h ckcasc.h ckcker.h ckucmd.h ckuusr.h ckcnet.h ckctel.h \
                ckcxla.h ckuath.h ck_ssl.h ckoath.h ckoreg.h
ckctel$(O):	ckctel.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckctel.h ckcnet.h ckocon.h ck_ssl.h \
                ckossl.h ckosslc.h
ckonet$(O):	    ckonet.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckoker.h ckcnet.h ckctel.h ckonet.h \
                ckotcp.h ckonbi.h ckuusr.h ckcsig.h cknwin.h ckowin.h ckuath.h ckossh.h \
                ck_ssl.h ckossl.h ckosslc.h
!if "$(PLATFORM)" == "NT"
cknnbi$(O):     cknnbi.c ckonbi.h ckcdeb.h ckoker.h ckclib.h 
!else
ckonbi$(O):     ckonbi.c ckonbi.h ckcdeb.h ckoker.h ckclib.h 
!endif
!if "$(PLATFORM)" == "NT"
cknpty$(O):     cknpty.c cknpty.h
!endif
ckoslp$(O):     ckoslp.c ckoslp.h ckcdeb.h ckoker.h ckclib.h 
ckomou$(O):     ckomou.c ckocon.h ckcdeb.h ckoker.h ckclib.h ckokey.h ckokvb.h ckuusr.h ckoreg.h
!if "$(CKF_XYZ)" == "yes"
ckop$(O):       ckop.c ckop.h ckcdeb.h ckoker.h ckclib.h ckcker.h \
                ckuusr.h ckcnet.h ckctel.h ckonet.h ckocon.h \
				p_global.h p_callbk.h
!endif
cknsig$(O):	cknsig.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcsym.h ckcnet.h ckctel.h ckonet.h\
                ckuusr.h ckonet.h ckcsig.h ckocon.h
ckusig$(O):	ckusig.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcsym.h ckcnet.h ckctel.h ckonet.h\
                ckuusr.h ckonet.h ckcsig.h ckocon.h
ckosyn$(O):     ckosyn.c ckcdeb.h ckoker.h ckclib.h ckcker.h ckocon.h ckuusr.h ckntap.h
ckothr$(O): ckothr.c ckocon.h ckcsym.h ckcasc.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcsig.h ckosyn.h
ckntap$(O): ckntap.c ckcdeb.h ckoker.h ckclib.h ckcker.h ckntap.h cknwin.h ckowin.h ckuusr.h ckucmd.h\
            ckowin.h ckntapi.h ckosyn.h ckocon.h
ckoreg$(O): ckoreg.c ckcdeb.h ckoker.h ckclib.h ckcker.h
cknalm$(O): cknalm.c cknalm.h
cknwin$(O): cknwin.c cknwin.h ckowin.h ckcdeb.h ckoker.h ckclib.h ckntap.h ckocon.h
cknprt$(O): cknprt.c ckcdeb.h ckoker.h ckcker.h ckucmd.h

ckuath$(O):     ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h ckuath.h ckuat2.h ck_ssl.h ckossl.h \
                ckosslc.h ckuath.c ckoath.h
ckoath$(O):     ckoath.c ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h ckuath.h ckuat2.h ckoath.h ckoetc.h
ck_ssl$(O):     ck_ssl.c ckcdeb.h ckoker.h ckclib.h ckctel.h ck_ssl.h ckosslc.h ckossl.h ckossh.h
ckossl$(O):     ckossl.c ckcdeb.h ckoker.h ck_ssl.h ckossl.h
ckosslc$(O):    ckosslc.c ckcdeb.h ckoker.h ck_ssl.h ckosslc.h
ckozli$(O):     ckozli.c ckcdeb.h ckoker.h ckozli.h

ckolssh$(O):    ckolsshs.h ckolsshs.h ckorbf.h ckcdeb.h ckoker.h ckclib.h ckosslc.h ckolssh.c ckolssh.h ckossh.h
ckolsshs$(O):   ckolsshs.c ckolsshs.h ckorbf.h ckcdeb.h ckcker.h ckocon.h
ckorbf$(O):     ckorbf.c ckorbf.h ckcdeb.h

ckossh$(O):     ckossh.c ckossh.h ckcdeb.h ckuusr.h ckcker.h ckocon.h ckoreg.h

ckonssh$(O):    ckonssh.c ckossh.h ckcdeb.h

ckosftp$(O):    ckcdeb.h ckoker.h ckclib.h ckosftp.h ckosftp.c

ck_crp$(O):     ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h ckuath.h ckuat2.h ck_crp.c

ck_des$(O):     ck_des.c

# X/Y/Z Modem support (3rd-party library)
!if "$(CKF_XYZ)" == "yes"
p_brw$(O):     ckcdeb.h ckoker.h ckclib.h ckocon.h p_brw.c p_type.h p_brw.h
p_callbk$(O):  ckcdeb.h ckoker.h ckclib.h ckocon.h p_callbk.c p_type.h p.h p_callbk.h p_common.h p_brw.h \
               p_error.h  p_global.h p_module.h p_omalloc.h
p_common$(O):  ckcdeb.h ckoker.h ckclib.h ckocon.h p_common.c p_type.h p_common.h p_error.h p_module.h p_global.h
p_dir$(O):     ckcdeb.h ckoker.h ckclib.h ckocon.h p_dir.c    p_type.h p_dir.h
p_error$(O):   ckcdeb.h ckoker.h ckclib.h ckocon.h p_error.c  p_type.h p_errmsg.h ckcnet.h ckctel.h ckonet.h
p_global$(O):  ckcdeb.h ckoker.h ckclib.h ckocon.h p_global.c p_type.h p_tl.h p_brw.h p.h
p_tl$(O):      ckcdeb.h ckoker.h ckclib.h ckocon.h p_tl.c     p_type.h p_tl.h p_brw.h p.h
p_omalloc$(O): ckcdeb.h ckoker.h ckclib.h p_omalloc.c p_type.h p_error.h p.h
!endif

# We're not always able to build and use WART so only do it if we're told to.
!if "$(CKB_BUILD_WART)" == "yes"
ckcpro.c:	ckcpro.w ckwart.exe
#		$(MAKE) -f ckoker.mak ckwart.exe \
#		  CC="$(CC) $(CC2)" OUT="$(OUT)" O="$(O)" OPT="$(OPT)" \
#		  DEBUG="$(DEBUG)" CFLAGS="-DCK_ANSIC $(CFLAGS)" LDFLAGS="$(LDFLAGS)"
		ckwart ckcpro.w ckcpro.c

!if "$(MIPS_CENTAUR)" == "yes"
WART_DEFS = /D_MT /D_MIPS_=1 /DCKT_NT31
WART_LIBS = libcmt.lib kernel32.lib
!endif

ckwart$(O):     ckwart.c
	$(CC) $(WART_DEFS) -c ckwart.c


ckwart.exe: ckwart.obj $(DEF)
	$(CC) $(WART_LIBS) ckwart.obj

!elseif "$(CKB_USE_WART)" == "yes"

!if "$(WART)" == ""
WART=ckwart
!endif

ckcpro.c: ckcpro.w
		$(WART) ckcpro.w ckcpro.c

!endif

ckopcf$(O):     ckopcf.c ckopcf.h
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) $(DLL) -c ckopcf.c

ckotel$(O):     ckotel.c
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) /Gn- -c ckotel.c

ckoclip$(O):     ckoclip.c
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) /Gn- -c ckoclip.c


#cko32rt$(O):     cko32rt.c
#        /Gd+ /Ge- $(DLL) -c cko32rt.c

k95d$(O):  k95d.c

# SRP support
!if "$(CKF_SRP)" == "yes"
getopt$(O):     getopt.c getopt.h
srp-tconf$(O):  srp-tconf.c getopt.h
srp-passwd$(O): srp-passwd.c getopt.h
!endif

iksdsvc$(O):    iksdsvc.c 

iksd$(O):    iksd.c 

ckof13.obj: ckoftp.c ckotcp.h
        @echo > ckof13.obj
        del ckof13.obj
	$(CC) $(CC2) $(CFLAGS) -DTCPERRNO -I$(FTP13INC) \
           $(DEBUG) $(OPT) $(DEFINES) $(DLL) -c ckoftp.c
        ren ckoftp.obj ckof13.obj

# TODO: What headers and libs are needed for the IBM compiler when
#       targeting TCP-32?
ckoi41.obj: ckoibm.c ckotcp.h
        @echo > ckoi41.obj
        del ckoi41.obj
!if "$(CMP)" == "OWWCL"
        @echo > wcc386.pch
        del wcc386.pch
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) $(DEFINES) \
	    -D__SOCKET_32H $(DLL) -bd -c ckoibm.c
	# Watcom lacks the headers to support -DSOCKS_ENABLED
!else
	$(CC) $(CC2) $(CFLAGS) -I$(IBM20INC) \
           $(DEBUG) $(OPT) $(DEFINES) -DSOCKS_ENABLED $(DLL) -c ckoibm.c
!endif
        ren ckoibm.obj ckoi41.obj

ckoi20.obj: ckoibm.c ckotcp.h
        @echo > ckoi20.obj
        del ckoi20.obj
!if "$(CMP)" == "OWWCL"
        @echo > wcc386.pch
        del wcc386.pch
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) $(DEFINES) -DTCPV40HDRS \
	     -D_ERRNO_H_INCLUDED -DSOCKS_ENABLED $(DLL) -I$(IBM20INC) -bd -c ckoibm.c
!else
	$(CC) $(CC2) $(CFLAGS) -I$(IBM20INC) \
           $(DEBUG) $(OPT) $(DEFINES) -DSOCKS_ENABLED -DTCPV40HDRS $(DLL) -c ckoibm.c
!endif
        ren ckoibm.obj ckoi20.obj

ckoi12.obj: ckoibm.c ckotcp.h
        @echo > ckoi12.obj
        del ckoi12.obj
!if "$(CMP)" == "OWWCL"
        @echo > wcc386.pch
        del wcc386.pch
	$(CC) $(CC2) $(CFLAGS) -I$(IBM12INC) -D_ERRNO_H_INCLUDED \
           $(DEBUG) $(OPT) $(DEFINES) $(DLL) -bd -c ckoibm.c
!else
	$(CC) $(CC2) $(CFLAGS) -I$(IBM12INC) \
           $(DEBUG) $(OPT) $(DEFINES) $(DLL) -c ckoibm.c
!endif
        ren ckoibm.obj ckoi12.obj

ckon30.obj: ckonov.c ckotcp.h
        @echo > ckon30.obj
        del ckon30.obj
	$(CC) $(CC2) $(CFLAGS) -DTCPERRNO -I$(LWP30INC) \
           $(DEBUG) $(OPT) $(DLL) -c ckonov.c
        ren ckonov.obj ckon30.obj

ckoker.res: ckoker.rc k95f_os2.ico
!if "$(CMP)" == "OWWCL"
        wrc -r -bt=os2 ckoker.rc
!else
        rc -r ckoker.rc
!endif

cknker.res: cknker.rc cknker.ico
        rc $(RCDEFINES) /fo cknker.res cknker.rc

k95ssh.res: k95ssh.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo k95ssh.res k95ssh.rc

k95crypt.res: k95crypt.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo k95crypt.res k95crypt.rc

ctl3dins.res: ctl3dins.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo ctl3dins.res ctl3dins.rc

iksd.res: iksd.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo iksd.res iksd.rc

iksdsvc.res: iksdsvc.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo iksdsvc.res iksdsvc.rc

k95d.res: k95d.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo k95d.res k95d.rc

telnet.res: telnet.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo telnet.res telnet.rc

rlogin.res: rlogin.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo rlogin.res rlogin.rc

se.res: se.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo se.res se.rc

textps.res: textps.rc cknver.h
        rc $(RCDEFINES) $(RC_FEATURE_DEFS) /fo textps.res textps.rc

ckopcf.res: ckopcf.rc ckopcf.h
        rc -r ckopcf.rc

ckoclip.res: ckoclip.rc ckoclip.h ckoclip.ico
!if "$(CMP)" == "OWWCL"
        wrc -r -bt=os2 ckoclip.rc
!else
        rc -r ckoclip.rc
!endif

ckermit.inf:    ckermit.ipf cker01.ipf cker02.ipf cker03.ipf cker04.ipf \
                cker05.ipf cker06.ipf ckermit.bmp
                ipfc ckermit.ipf /inf

clean:
       -del *.obj
       -del *.res
