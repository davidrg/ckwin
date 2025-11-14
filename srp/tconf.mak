# Microsoft Developer Studio Generated NMAKE File, Based on tconf.dsp
!IF "$(CFG)" == ""
CFG=Debug
!MESSAGE No configuration specified. Defaulting to Debug.
!ENDIF

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "tconf.mak" CFG="Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Debug" (based on "Win32 (x86) Console Application")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

# For some reason this isn't running at all
# !message Attempting to detect compiler...
# !include ..\kermit\k95\compiler_detect.mak

# So instead we just rely on the values from setenv.bat, which is faster anyway
TARGET_CPU=$(CKB_TARGET_ARCH)
COMPILER=$(CK_COMPILER_NAME)
MSC_VER=$(CKB_MSC_VER)
TARGET_PLATFORM=Windows

!message
!message
!message ===============================================================================
!message libsrp Build Configuration
!message ===============================================================================
!message  Architecture:             $(TARGET_CPU)
!message  Compiler:                 $(COMPILER)
#!message  Compiler Version:         $(COMPILER_VERSION)
!message  Compiler Target Platform: $(TARGET_PLATFORM)
!message  Configuration:            $(CFG)
!message ===============================================================================
!message
!message

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

!IF  "$(CFG)" == "Release"
OUTDIR=.\win32\libsrp_openssl\Release
INTDIR=.\win32\libsrp_openssl\tconf___Win32_Release
OutDir=.\win32\libsrp_openssl\Release
!ELSEIF  "$(CFG)" == "Debug"
OUTDIR=.\win32\libsrp_openssl\Debug
INTDIR=.\win32\libsrp_openssl\tconf___Win32_Debug
OutDir=.\win32\libsrp_openssl\Debug
!ENDIF

ALL : "$(OUTDIR)\tconf.exe"

CLEAN :
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\tconf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\tconf.exe"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\tconf.pch"
	-@erase "$(OUTDIR)\tconf.ilk"
    -@erase "$(OUTDIR)\tconf.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe

CFLAGS=/nologo /W3 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\tconf.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

!if ($(MSC_VER) < 140)
CFLAGS=$(CFLAGS) /GX /YX
!else
CFLAGS=$(CFLAGS) /EHsc
!endif

!IF  "$(CFG)" == "Release"
!if "$(CKB_STATIC_CRT_NT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MT
!else
COMMON_CFLAGS = /MD
!endif
!ELSEIF  "$(CFG)" == "Debug"
!if "$(CKB_STATIC_CRT_NT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MTd
!else
COMMON_CFLAGS = /MDd
!endif
!ENDIF

!IF  "$(CFG)" == "Release"
CPP_PROJ=$(CFLAGS) $(COMMON_CFLAGS)  /O2 /D "NDEBUG"
!ELSEIF  "$(CFG)" == "Debug"
CPP_PROJ=$(CFLAGS) $(COMMON_CFLAGS) /Gm /ZI /Od /D "_DEBUG" /GZ
!ENDIF



.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tconf.bsc"
BSC32_SBRS= \

LIBS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
LIBS=$(LIBS) shell32.lib ole32.lib oleaut32.lib uuid.lib
# odbc32.lib odbccp32.lib

LINK32=link.exe

LINK32_FLAGS=/nologo /subsystem:console /pdb:"$(OUTDIR)\tconf.pdb" /machine:I386 /out:"$(OUTDIR)\tconf.exe"

!IF  "$(CFG)" == "Release"
LINK32_FLAGS= $(LINK32_FLAGS) /incremental:no
!ELSEIF  "$(CFG)" == "Debug"
LINK32_FLAGS= $(LINK32_FLAGS) /incremental:yes /debug /pdbtype:sept
!ENDIF

LINK32_FLAGS=$(LIBS) $(LINK32_FLAGS)

LINK32_OBJS= \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\tconf.obj" \
	"$(OUTDIR)\srp.lib"

"$(OUTDIR)\tconf.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<



win32\src\getopt.c : \
	"win32\src\getopt.h"\


libsrp\tconf.c : \
	"libsrp\cstr.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_pwd.h"\




SOURCE=win32\src\getopt.c

"$(INTDIR)\getopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\tconf.c

"$(INTDIR)\tconf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



