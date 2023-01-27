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
IBM12DIR  = C:\TCPIP
IBM12LIBS = $(IBM12DIR)\lib\tcpipdll.lib
IBM12INC  = $(IBM12DIR)\include

# for IBM TCP/IP 2.0
IBM20DIR  = C:\TCPIP
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

# Base flags for all versions of Visual C++ (and OpenWatcom
# pretending to be Visual C++)
!if "$(CKB_STATIC_CRT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MT
!else
COMMON_CFLAGS = /MD
!endif

# These options are used for all Windows .exe targets
COMMON_OPTS = /Ox
# These are:
# /GA     Optimise for Windows Application (ignored by OpenWatcom)
# /Ox     Maximum Opts (= /Ogityb2 /Gs in VC6/7.0)

# If Visual C++ <= 2003 or OpenWatcom:
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
CFLAG_GF=/GF

# On windows we'll try to detect the Visual C++ version being used and adjust
# compiler flags accordingly.
!if "$(PLATFORM)" == "NT"
!message Attempting to detect compiler...

!include compiler_detect.mak
!message
!else
# On OS/2 we'll just assume OpenWatcom for now. I don't have access to the
# IBM compiler to find a way to tell it apart from watcom like we do for
# Visual C++.
CMP = OWCL386
COMPILER = OpenWatcom WCL386
COMPILER_VERSION = OpenWatcom

# wcl386 doesn't pretend to be Visual C++ and doesn't take the same
# command line arguments.
MSC_VER = 0

# Nothing supports PowerPC OS/2.
TARGET_CPU = x86
TARGET_PLATFORM = OS/2

# Override CL so we don't end up running the Visual C++ clone cl.
CL = wcl386
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

# Standard windows headers from MinGW that don't come with OpenWatcom:
INCLUDE = $(INCLUDE);ow\;

!endif

!if ($(MSC_VER) < 60)
!error Unsupported compiler version. Visual C++ 6.0 SP6 or newer required.
!endif

# TODO: Much of this compiler flag work should be applied to the KUI Makefile
#       too

# Check to see if we're using Visual C++ and targeting 64bit x86. If so
# then tell the linker we're targeting x86-64
!if "$(TARGET_CPU)" == "x86-64"
LDFLAGS = $(LDFLAGS) /MACHINE:X64
!endif

!if ($(MSC_VER) >= 170) && ($(MSC_VER) <= 192)
# Starting with Visual C++ 2012, the default subsystem version is set to 6.0
# which makes the generated binaries invalid on anything older than Windows
# Vista (you get the "is not a valid win32 application" error). Visual C++ 2012
# through to 2019 are capable of targeting Windows XP so we set the subsystem
# version to 5.1 so the generated binaries are compatible.
SUBSYSTEM_CONSOLE=console,5.1
SUBSYSTEM_WIN32=windows,5.1
!endif

!if ($(MSC_VER) > 90)
COMMON_OPTS = $(COMMON_OPTS) /GA
!endif

!if ($(MSC_VER) < 140)
# These flags and options are deprecated or unsupported
# from Visual C++ 2005 (v8.0) and up.

# /GX- is new in Visual C++ 2.0
!if ($(MSC_VER) > 80)
COMMON_CFLAGS = $(COMMON_CFLAGS) /GX-
!endif

!if ($(MSC_VER) < 100)
# Visual C++ 2.0 and 1.0 32-bit edition don't support these flags, so don't
# use them.
CFLAG_GF=
!endif

COMMON_CFLAGS = $(COMMON_CFLAGS) /Ze /YX
# These are:    /Ze     Enable extensions (default)
#               /GX-    Enable C++ Exception handling (same as /EHs /EHc)
#               /YX     Automatic .PCH

# Optimise for Pentium
COMMON_OPTS = $(COMMON_OPTS) /G5

!else
COMMON_CFLAGS = $(COMMON_CFLAGS) /EHs-c-
# These are:    /EHs-c-     Enable C++ Exception handling (replaces /GX-)
!endif

!endif

RCDEFINES=$(RC_FEATURE_DEFS) /dCOMPILER_$(CMP)

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
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /OPT:REF" DEF="cknker.def"

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
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_CONSOLE) /MAP /DEBUG:full /debugtype:both /WARN:3 /FIXED:NO /PROFILE /OPT:REF" \
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
    LINKFLAGS="/nologo /MAP /DEBUG:full /SUBSYSTEM:$(SUBSYSTEM_WIN32)" \
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
    LINKFLAGS="/nologo /SUBSYSTEM:$(SUBSYSTEM_WIN32)" \
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
#         /Ge-    ? -br         Use the version of the runtime library that assumes a DLL is being
#                               built. Default: /Ge+
# CFLAGS: -Sp1    -zp=1         /Sp<[1]|2|4|8|16> : Pack aggregate members on specified alignment. Default: /Sp4
#         -Sm     ?             Ignore migration keywords. Default: /Sm-
#         -Gm     ? -bm         Link with multithread runtime libraries. Default: /Gm-
#         -G5     ?             /G5: Generate code optimized for use on a Pentium processor.
#         -Gt     ?             Store variables so that they do not cross 64K boundaries. Default: /Gt-
#         -Gd     ?             /Gd+: Use the version of the runtime library that is dynamically linked.
#         -J      N/A (uchar is default)          /J+: Make default char type unsigned. Default: /J+
#                 -bt=os2v2     Compile for target OS
# NOLINK: -c        /C+: Perform compile only, no link.
# LINKFLAGS: /nologo
#            /noi
#            /align:16
#            /base:0x10000
#                           -l=os2v2
#                           -x

# Watcom C targeting OS/2
# TODO: Fix buiding with OPT="-ox " (currently this causes it to crash on
# startup with trap 001 )
wcos2:
	$(MAKE) -f ckoker.mak os232 \
	    CMP="OWCL386" \
	    CC="wcl386" \
        CC2="-Fh" \
        OUT="-Fe=" O=".obj" \
	    OPT=" " \
        DEBUG="-DNDEBUG" \
        DLL="-br" \
	    CFLAGS="-q -zp=1 -bm -bt=os2 -aa" \
        LDFLAGS="" \
        PLATFORM="OS2" \
        NOLINK="-c" \
!ifdef WARP
        WARP="YES" \
!endif
        LINKFLAGS="-l=os2v2 -x" \
	    DEF=""  # ckoker32.def

wcos2d:
	$(MAKE) -f ckoker.mak os232 \
	    CMP="OWCL386" \
	    CC="wcl386" \
        CC2="-Fh -d3" \
        OUT="-Fe=" O=".obj" \
	    OPT=" " \
        DEBUG="-DNDEBUG" \
        DLL="-br" \
	    CFLAGS="-q -zp=1 -bm -bt=os2 -aa" \
        LDFLAGS="" \
        PLATFORM="OS2" \
        NOLINK="-c" \
!ifdef WARP
        WARP="YES" \
!endif
        LINKFLAGS="-l=os2v2 -x" \
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
#   -br             Build with dll runtime library - maybe equivalent to /Ge- on the
#                   IBM compiler.
#   -q              Operate quietly
#   -bt=os2         Compile for OS/2 (rather than DOS/NetWare/Windows/QNX/whatever)
#   -c              Compile only, don't link
#   -l=os2v2        Link for 32bit OS/2
#   -x              Make names case sensitive

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
!endif /* PLATFORM */

# To build with NETWORK support, uncomment the following three 
# lines and comment out the previous set:
!ifdef PLATFORM
!if "$(PLATFORM)" == "OS2"
DEFINES = -DOS2 -DDYNAMIC -DKANJI -DTCPSOCKET \
          -DNPIPE -DOS2MOUSE -DHADDRLIST -DPCFONTS \
          -DRLOGCODE -DNETFILE -DONETERMUPD \
          $(ENABLED_FEATURE_DEFS) $(DISABLED_FEATURE_DEFS) \
!if "$(CMP)" == "OWCL386"
          -D__32BIT__
!endif
# OpenWatcom doesn't define __32BIT__ by default which upsets a lot of OS/2
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
          $(ENABLED_FEATURE_DEFS) $(DISABLED_FEATURE_DEFS) \
!if "$(CMP)" != "OWCL"
          -D__STDC__ \
!endif
!endif
!endif  /* PLATFORM */
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
LIBS = os2386.lib rexx.lib \
!if "$(CMP)" != "OWCL"
       bigmath.lib
!endif
# OpenWatcom doesn't have bigmath.lib
# SRP support: libsrp.lib
!else if "$(PLATFORM)" == "NT"
!if "$(K95BUILD)" == "UIUC"
LIBS = kernel32.lib user32.lib gdi32.lib wsock32.lib \
       winmm.lib mpr.lib advapi32.lib winspool.lib 
       # Kerberos: wshload.lib
!else
KUILIBS = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
        advapi32.lib shell32.lib rpcrt4.lib rpcns4.lib wsock32.lib \
        winmm.lib vdmdbg.lib comctl32.lib mpr.lib $(COMMODE_OBJ) \
!if "$(CKF_SSH)" == "yes"
       ssh.lib ws2_32.lib \
!endif
!if "$(CKF_SSL)" == "yes"
       $(SSL_LIBS) \
!endif

!if ($(MSC_VER) > 80)
# I doubt these are actually ever required. But if they ever are, they're only
# required when building with Visual C++ 2.0 or newer (1.0 32-bit doesn't have
# them)
KUILIBS = $(KUILIBS) ole32.lib oleaut32.lib uuid.lib
!endif

        #msvcrt.lib
        #Kerberos: wshload.lib
		# SRP support: srpstatic.lib
        #libsrp.lib bigmath.lib
LIBS = kernel32.lib user32.lib gdi32.lib wsock32.lib shell32.lib\
       winmm.lib mpr.lib advapi32.lib winspool.lib $(COMMODE_OBJ) \
!if "$(CKF_SSH)" == "yes"
       ssh.lib ws2_32.lib \
!endif
!if "$(CKF_SSL)" == "yes"
       $(SSL_LIBS) \
!endif
       #msvcrt.lib  
       # Kerberos: wshload.lib
	   # SRP support: srpstatic.lib
       # libsrp.lib bigmath.lib
!endif
!endif /* PLATFORM */
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
!endif /* PLATFORM */
        ckuath$(O) ckoath$(O) ck_ssl$(O) ckossl$(O) ckosslc$(O) \
        ckosftp$(O) ckozli$(O) \
!if 0
        ck_crp$(O) ck_des$(O) \
!endif
!if ("$(CKF_SSH)" == "yes")
        ckossh$(O) ckorbf$(O) ckoshs$(O) \
!endif
        ckocon$(O) ckoco2$(O) ckoco3$(O) ckoco4$(O) ckoco5$(O) \
        ckoetc$(O) ckoetc2$(O) ckokey$(O) ckomou$(O) ckoreg$(O) \
        ckonet$(O) \
        ckoslp$(O) ckosyn$(O) ckothr$(O) ckotek$(O) ckotio$(O) ckowys$(O) \
        ckodg$(O)  ckoava$(O) ckoi31$(O) ckotvi$(O) ckovc$(O) \
        ckoadm$(O) ckohzl$(O) ckohp$(O) ckoqnx$(O)\
!if "$(PLATFORM)" == "NT"
        cknnbi$(O) \
!else
        ckonbi$(O) \
!endif /* PLATFORM */
!if ("$(CKF_XYZ)" == "yes")
        ckop$(O) p_callbk$(O) p_global$(O) p_omalloc$(O) p_error$(O) \
        p_common$(O) p_tl$(O) p_dir$(O)
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

PDLLDIR = pdll
PDLLOBJS = \
$(PDLLDIR)\pdll_common.obj \
$(PDLLDIR)\pdll_crc.obj \
$(PDLLDIR)\pdll_dev.obj \
$(PDLLDIR)\pdll_error.obj \
$(PDLLDIR)\pdll_exeio.obj \
$(PDLLDIR)\pdll_global.obj \
$(PDLLDIR)\pdll_main.obj \
$(PDLLDIR)\pdll_omalloc.obj \
$(PDLLDIR)\pdll_r.obj \
$(PDLLDIR)\pdll_ryx.obj \
$(PDLLDIR)\pdll_rz.obj \
$(PDLLDIR)\pdll_s.obj \
$(PDLLDIR)\pdll_syx.obj \
$(PDLLDIR)\pdll_sz.obj \
$(PDLLDIR)\pdll_tcpipapi.obj \
$(PDLLDIR)\pdll_x_global.obj \
$(PDLLDIR)\pdll_z.obj \
$(PDLLDIR)\pdll_z_global.obj \

os232: ckoker32.exe tcp32 otelnet.exe ckoclip.exe orlogin.exe osetup.exe otextps.exe \
!if "$(CMP)" != "OWCL386"
       cko32rtl.dll     # IBM compiler only.
!endif
# SRP support: srp-tconf.exe srp-passwd.exe
# Crypto stuff: k2crypt.dll 

# docs pcfonts.dll cksnval.dll 



win32: cknker.exe wtelnet wrlogin k95d textps ctl3dins.exe iksdsvc.exe iksd.exe \
    se.exe \
!if "$(CKF_CRYPTDLL)" == "yes"
    k95crypt.dll
!endif
# SRP support: srp-tconf.exe srp-passwd.exe

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

tcp32: cko32i20.dll
# cko32i12.dll cko32f13.dll
# cko32n30.dll

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

telnet.exe: telnet.obj $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ telnet.obj $(LIBS) 
<<

rlogin.exe: rlogin.obj $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ rlogin.obj $(LIBS) 
<<

se.exe: se.obj $(DEF) ckoker.mak
       link.exe @<<
       $(LINKFLAGS) /OUT:$@ se.obj $(LIBS)
<<

orlogin.exe: rlogin.obj $(DEF) ckoker.mak
!if "$(CMP)" == "OWCL386"
        $(CC) $(CC2) $(LINKFLAGS) rlogin.obj $(OUT)$@ $(LDFLAGS) $(LIBS)
!else
      	$(CC) $(CC2) /B"$(LINKFLAGS)" rlogin.obj $(OUT) $@ $(LDFLAGS) $(LIBS)
!endif

otextps.exe: textps.obj $(DEF) ckoker.mak
!if "$(CMP)" == "OWCL386"
        $(CC) $(CC2) $(LINKFLAGS) textps.obj $(OUT)$@ $(LDFLAGS) $(LIBS)
!else
      	$(CC) $(CC2) /B"$(LINKFLAGS)" textps.obj $(OUT) $@ $(LDFLAGS) $(LIBS)
!endif

k95d.exe: k95d.obj $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ k95d.obj $(LIBS) 
<<

ctl3dins.exe: ctl3dins.obj $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ ctl3dins.obj $(LIBS) VERSION.LIB
<<

textps.exe: textps.obj $(DEF) ckoker.mak
       link.exe @<< 
       $(LINKFLAGS) /OUT:$@ textps.obj $(LIBS) 
<<

#       ckoker.msb  -- no idea what this is
ckoker32.exe: $(OBJS) $(DEF) ckoker.res ckoker.mak
!if "$(CMP)" == "OWCL386"
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
cko32i20.dll: ckoi20.obj ckoker.mak
!if "$(CMP)" == "OWCL386"
    $(CC) $(CC2) $(DEBUG) $(DLL) ckoi20.obj $(OUT)$@ \
	 $(LINKFLAGS) tcpip32.lib $(LIBS)
!else
	$(CC) $(CC2) $(DEBUG) $(DLL) ckoi20.obj cko32i20.def $(OUT) $@ \
	/B"/noe /noi" $(IBM20LIBS) $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL
!endif

cko32i12.dll: ckoi12.obj cko32i12.def ckoker.mak
	$(CC) $(CC2) $(DEBUG) $(DLL) ckoi12.obj cko32i12.def $(OUT) $@ \
	/B"/noe /noi" $(IBM12LIBS) $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL       

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

cksnval.dll: cksnval.obj cksnval.def ckoker.mak
	$(CC) $(CC2) $(DEBUG) $(DLL) cksnval.obj \
        cksnval.def $(OUT) $@ $(LIBS)

k95crypt.dll: ck_crp.obj ck_des.obj ckclib.obj ck_crp.def ckoker.mak
	link /dll /debug /def:ck_crp.def /out:$@ ck_crp.obj ckclib.obj ck_des.obj libdes.lib

k2crypt.dll: ck_crp.obj ck_des.obj ckclib.obj k2crypt.def ckoker.mak
	ilink /nologo /noi /exepack:1 /align:16 /base:0x10000 k2crypt.def \
            /out:$@ ck_crp.obj ck_des.obj ckclib.obj libdes.lib
        dllrname $@ CPPRMI36=CKO32RTL


docs:   ckermit.inf

# ckotel.def
otelnet.exe: ckotel.obj ckoker.mak
!if "$(CMP)" == "OWCL386"
        $(CC) $(CC2) $(DEBUG) ckotel.obj $(LINKFLAGS) $(OUT)$@ $(LIBS)
!else
        $(CC) $(CC2) $(DEBUG) ckotel.obj ckotel.def $(OUT) $@ $(LIBS)
        dllrname $@ CPPRMI36=CKO32RTL
!endif

osetup.exe: setup.obj osetup.def ckoker.mak
!if "$(CMP)" == "OWCL386"
        $(CC) $(DEBUG) setup.obj $(LINKFLAGS) $(OUT)$@
!else
        $(CC) $(DEBUG) setup.obj osetup.def $(OUT) $@
!endif

# ckoclip.def
ckoclip.exe: ckoclip.obj ckoker.mak ckoclip.res
!if "$(CMP)" == "OWCL386"
        $(CC) $(CC2) $(LINKFLAGS) $(DEBUG) ckoclip.obj $(OUT)$@ $(LIBS)
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
#srp-tconf.exe: srp-tconf.obj getopt.obj ssh\ckosslc.obj ckoker.mak
#!if "$(PLATFORM)" == "OS2"
#        $(CC) $(CC2) $(DEBUG) srp-tconf.obj getopt.obj ssh\ckosslc.obj ckotel.def $(OUT) $@ $(LIBS)
#        dllrname $@ CPPRMI36=CKO32RTL       
#!else if "$(PLATFORM)" == "NT"
#	link /debug /out:$@ srp-tconf.obj getopt.obj ssh\ckosslc.obj $(LIBS)
#!endif
#        
#srp-passwd.exe: srp-passwd.obj getopt.obj ssh\ckosslc.obj ckoker.mak
#!if "$(PLATFORM)" == "OS2"
#        $(CC) $(CC2) $(DEBUG) srp-passwd.obj getopt.obj ssh\ckosslc.obj ckotel.def $(OUT) $@ $(LIBS)
#        dllrname $@ CPPRMI36=CKO32RTL       
#!else if "$(PLATFORM)" == "NT"
#	link /debug /out:$@ srp-passwd.obj getopt.obj ssh\ckosslc.obj $(LIBS)
#!endif
        
iksdsvc.exe: iksdsvc.obj ckoker.mak
!if "$(PLATFORM)" == "OS2"
!else if "$(PLATFORM)" == "NT"
	link /debug /out:$@ iksdsvc.obj $(LIBS)
!endif
        
iksd.exe: iksd.obj ckoker.mak
!if "$(PLATFORM)" == "OS2"
!else if "$(PLATFORM)" == "NT"
	link /debug /out:$@ iksd.obj $(LIBS)
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
                ckuusr.h ckonet.h ckcsig.h ckocon.h ckntap.h ckocon.h ck_ssl.h ckossl.h
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
		  ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckocon.h cknwin.h \
	          ckowin.h ckntap.h kui\ikui.h
ckuus2$(O):	ckuus2.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		  ckcxla.h ckuxla.h ckokvb.h ckocon.h ckokey.h ckcnet.h ckctel.h
ckuus3$(O):	ckuus3.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		  ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckonbi.h ckntap.h \
                  ckocon.h ckokey.h ckokvb.h ckcuni.h ck_ssl.h ckossl.h ckuath.h kui\ikui.h
ckuus4$(O):	ckuus4.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		  ckcxla.h ckuxla.h ckuver.h ckcnet.h ckctel.h ckonet.h ckocon.h \
	          ckoetc.h ckntap.h ckuath.h ck_ssl.h
ckuus5$(O):	ckuus5.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
                ckocon.h ckokey.h ckokvb.h ckcuni.h ckcnet.h ckctel.h ck_ssl.h ckossl.h kui\ikui.h
ckuus6$(O):	ckuus6.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h ckntap.h \
                ckcnet.h ckctel.h
!if "$(PLATFORM)" == "OS2"
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(DEFINES) $(NOLINK) ckuus6.c

!endif
ckuus7$(O):	ckuus7.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h \
		  ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckocon.h ckodir.h \
                  ckokey.h ckokvb.h cknwin.h ckowin.h ckntap.h ckcuni.h \
                  ckntap.h ckuath.h ck_ssl.h kui\ikui.h
ckuusx$(O):	ckuusx.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckonbi.h \
                ckocon.h cknwin.h ckowin.h ckntap.h ckcnet.h ckctel.h kui\ikui.h
ckuusy$(O):	ckuusy.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckuusr.h ckucmd.h ckcnet.h ckctel.h \
	        ck_ssl.h kui\ikui.h
ckofio$(O):	ckofio.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckuver.h ckodir.h ckoker.h \
                ckuusr.h ckcxla.h ck_ssl.h
ckoava$(O):     ckoava.c ckoava.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h
ckocon$(O):	ckocon.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckoker.h ckocon.h ckcnet.h ckctel.h \
                ckonbi.h ckokey.h ckokvb.h ckuusr.h cknwin.h ckowin.h ckcuni.h kui\ikui.h
ckoco2$(O):     ckoco2.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckoker.h ckocon.h \
                ckonbi.h ckopcf.h ckuusr.h ckokey.h ckokvb.h ckcuni.h kui\ikui.h
ckoco3$(O):     ckoco3.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckoker.h ckocon.h \
                ckokey.h ckokvb.h ckuusr.h ckowys.h ckodg.h  ckoava.h ckoi31.h \
                ckohp.h  ckoadm.h ckohzl.h ckoqnx.h ckotvi.h ckovc.h  ckcuni.h \
                ckcnet.h ckctel.h kui\ikui.h
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
ckoi31$(O):     ckoi31.c ckoi31.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h
ckokey$(O):     ckokey.c ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcker.h ckuusr.h ckctel.h \
                ckocon.h ckokey.h ckokvb.h ckcxla.h ckuxla.h ckcuni.h kui\ikui.h
ckoqnx$(O):     ckoqnx.c ckoqnx.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h
ckotek$(O): ckotek.c ckotek.h ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h ckoker.h ckocon.h \
                ckokey.h ckokvb.h ckuusr.h ckcnet.h ckctel.h
ckotio$(O):	ckotio.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckuver.h ckodir.h ckoker.h \
                ckocon.h ckokey.h ckokvb.h ckuusr.h ckoslp.h ckcsig.h ckop.h \
                ckcuni.h ckowin.h ckcnet.h ckctel.h \
!if "$(PLATFORM)" == "NT"
                ckntap.h cknwin.h  kui\ikui.h
!else

!endif
ckotvi$(O):     ckotvi.c ckotvi.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h \
                ckctel.h ckokvb.h
ckovc$(O):      ckovc.c  ckovc.h  ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h
ckowys$(O):     ckowys.c ckowys.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcasc.h ckocon.h ckuusr.h \
                ckcuni.h ckokey.h ckokvb.h ckctel.h
ckcnet$(O):	ckcnet.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h ckonet.h ckotcp.h \
                ckuusr.h ckcsig.h ckocon.h ckuath.h ck_ssl.h ckossl.h ckosslc.h
ckcftp$(O):     ckcftp.c ckcdeb.h ckoker.h ckcasc.h ckcker.h ckucmd.h ckuusr.h ckcnet.h ckctel.h \
                ckcxla.h ckuath.h ck_ssl.h ckoath.h
ckctel$(O):	ckctel.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckctel.h ckcnet.h ckocon.h ck_ssl.h \
                ckossl.h ckosslc.h
ckonet$(O):	ckonet.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckoker.h ckcnet.h ckctel.h ckonet.h \
                ckotcp.h ckonbi.h ckuusr.h ckcsig.h cknwin.h ckowin.h ckuath.h \
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
ckomou$(O):     ckomou.c ckocon.h ckcdeb.h ckoker.h ckclib.h ckokey.h ckokvb.h ckuusr.h
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
ckothr$(O): ckothr.c ckocon.h ckcsym.h ckcasc.h ckcdeb.h ckoker.h ckclib.h ckcker.h ckcsig.h
ckntap$(O): ckntap.c ckcdeb.h ckoker.h ckclib.h ckcker.h ckntap.h cknwin.h ckowin.h ckuusr.h ckucmd.h ckowin.h
ckoreg$(O): ckoreg.c ckcdeb.h ckoker.h ckclib.h ckcker.h
cknalm$(O): cknalm.c cknalm.h
cknwin$(O): cknwin.c cknwin.h ckowin.h ckcdeb.h ckoker.h ckclib.h ckntap.h ckocon.h
cknprt$(O): cknprt.c ckcdeb.h ckoker.h ckcker.h ckucmd.h

ckuath$(O):     ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h ckuath.h ckuat2.h ck_ssl.h ckossl.h \
                ckosslc.h ckuath.c ckoath.h
ckoath$(O):     ckoath.c ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h ckuath.h ckuat2.h ckoath.h
ck_ssl$(O):     ck_ssl.c ckcdeb.h ckoker.h ckclib.h ckctel.h ck_ssl.h ckosslc.h ckossl.h
ckossl$(O):     ckossl.c ckcdeb.h ckoker.h ck_ssl.h ckossl.h
ckosslc$(O):    ckosslc.c ckcdeb.h ckoker.h ck_ssl.h ckosslc.h
ckozli$(O):     ckozli.c ckcdeb.h ckoker.h ckozli.h

ckossh$(O):     ckoshs.h ckoshs.h ckorbf.h ckcdeb.h ckoker.h ckclib.h ckosslc.h ckossh.c ckossh.h
ckoshs(O):      ckoshs.c ckoshs.h ckorbf.h ckcdeb.h ckcker.h ckocon.h
ckorbf(O):      ckorbf.c ckorbf.h ckcdeb.h


ckosftp$(O):    ckcdeb.h ckoker.h ckclib.h ckosftp.h ckosftp.c
	$(CC) $(CC2) $(CFLAGS) $(DLL) $(DEBUG) $(DEFINES) $(NOLINK) ckosftp.c

ck_crp$(O):     ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h ckuath.h ckuat2.h ck_crp.c
!if "$(PLATFORM)" == "OS2"
	$(CC) $(CC2) $(CFLAGS) $(DLL) $(DEBUG) $(DEFINES) $(NOLINK) ck_crp.c
!endif

ck_des$(O):     ck_des.c
!if "$(PLATFORM)" == "OS2"
	$(CC) $(CC2) $(CFLAGS) $(DLL) $(DEBUG) $(DEFINES) $(NOLINK) ck_des.c

!endif

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
#getopt$(O):     getopt.c getopt.h
#srp-tconf$(O):  srp-tconf.c getopt.h 
#srp-passwd$(O): srp-passwd.c getopt.h

iksdsvc$(O):    iksdsvc.c 

iksd$(O):    iksd.c 

ckof13.obj: ckoftp.c ckotcp.h
        @echo > ckof13.obj
        del ckof13.obj
	$(CC) $(CC2) $(CFLAGS) -DTCPERRNO -I$(FTP13INC) \
           $(DEBUG) $(OPT) $(DEFINES) $(DLL) -c ckoftp.c
        ren ckoftp.obj ckof13.obj

ckoi20.obj: ckoibm.c ckotcp.h
        @echo > ckoi20.obj
        del ckoi20.obj
!if "$(CMP)" == "OWCL386"
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) $(DEFINES) -D__SOCKET_32H $(DLL) -c ckoibm.c
	# Watcom lacks the headers to support -DSOCKS_ENABLED
!else
	$(CC) $(CC2) $(CFLAGS) -I$(IBM20INC) \
           $(DEBUG) $(OPT) $(DEFINES) -DSOCKS_ENABLED $(DLL) -c ckoibm.c
!endif
        ren ckoibm.obj ckoi20.obj

ckoi12.obj: ckoibm.c ckotcp.h
        @echo > ckoi12.obj
        del ckoi12.obj
	$(CC) $(CC2) $(CFLAGS) -I$(IBM12INC) \
           $(DEBUG) $(OPT) $(DEFINES) $(DLL) -c ckoibm.c
        ren ckoibm.obj ckoi12.obj

ckon30.obj: ckonov.c ckotcp.h
        @echo > ckon30.obj
        del ckon30.obj
	$(CC) $(CC2) $(CFLAGS) -DTCPERRNO -I$(LWP30INC) \
           $(DEBUG) $(OPT) $(DLL) -c ckonov.c
        ren ckonov.obj ckon30.obj

cksnval$(O):  ckoetc.c
    @echo > cksnval.obj
    del cksnval.obj
    ren ckoetc.obj ckoetc.o
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) -DREXXDLL /Gn- -c ckoetc.c 
    ren ckoetc.obj cksnval.obj
    ren ckoetc.o ckoetc.obj

ckoker.res: ckoker.rc
!if "$(CMP)" == "OWCL386"
        wrc -r -bt=os2 ckoker.rc
!else
        rc -r ckoker.rc
!endif

cknker.res: cknker.rc cknker.ico
        rc $(RCDEFINES) /fo cknker.res cknker.rc

ckopcf.res: ckopcf.rc ckopcf.h
        rc -r ckopcf.rc

ckoclip.res: ckoclip.rc ckoclip.h ckoclip.ico
!if "$(CMP)" == "OWCL386"
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
