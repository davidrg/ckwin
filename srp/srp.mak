# Microsoft Developer Studio Generated NMAKE File, Based on srp.dsp
!IF "$(CFG)" == ""
CFG=Debug
!MESSAGE No configuration specified. Defaulting to Debug.
!ENDIF

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "srp.mak" CFG="Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
INTDIR=.\win32\libsrp_openssl\Release
OutDir=.\win32\libsrp_openssl\Release
!ELSEIF  "$(CFG)" == "Debug"
OUTDIR=.\win32\libsrp_openssl\Debug
INTDIR=.\win32\libsrp_openssl\Debug
OutDir=.\win32\libsrp_openssl\Debug
!ENDIF

INCLUDEDIR=.\include

ALL : "$(OUTDIR)\srp.dll"

CLEAN :
	-@erase "$(INTDIR)\cstr.obj"
	-@erase "$(INTDIR)\rfc2945_client.obj"
	-@erase "$(INTDIR)\rfc2945_server.obj"
	-@erase "$(INTDIR)\srp.obj"
	-@erase "$(INTDIR)\srp6_client.obj"
	-@erase "$(INTDIR)\srp6_server.obj"
	-@erase "$(INTDIR)\t_client.obj"
	-@erase "$(INTDIR)\t_conf.obj"
	-@erase "$(INTDIR)\t_conv.obj"
	-@erase "$(INTDIR)\t_getpass.obj"
	-@erase "$(INTDIR)\t_math.obj"
	-@erase "$(INTDIR)\t_misc.obj"
	-@erase "$(INTDIR)\t_pw.obj"
	-@erase "$(INTDIR)\t_read.obj"
	-@erase "$(INTDIR)\t_server.obj"
	-@erase "$(INTDIR)\t_sha.obj"
	-@erase "$(INTDIR)\t_truerand.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\srp.dll"
	-@erase "$(OUTDIR)\srp.exp"
	-@erase "$(OUTDIR)\srp.lib"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\srp.ilk"
	-@erase "$(OUTDIR)\srp.pdb"
	-@erase "$(OUTDIR)\srp.pch"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe

CFLAGS= /nologo /W3 /I ".\win32\libsrp_openssl" /D "WIN32" /D "_WINDOWS" \
    /D "_MBCS" /D "_USRDLL" /D "SRP_EXPORTS" /D "HAVE_CONFIG_H" \
    /Fp"$(INTDIR)\srp.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
# /I "../include"

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
CPP_PROJ=$(CFLAGS) $(COMMON_CFLAGS) /O2 /D "NDEBUG"
!ELSEIF  "$(CFG)" == "Debug"
CPP_PROJ=$(CFLAGS) $(COMMON_CFLAGS) /Gm  /ZI /Od  /D "_DEBUG" /GZ
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

LIBS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
LIBS=$(LIBS) shell32.lib ole32.lib oleaut32.lib uuid.lib
LIBS=$(LIBS) $(CKF_SSL_LIBS)
# odbc32.lib odbccp32.lib

LINK32=link.exe

LINK32_FLAGS = /nologo /dll /pdb:"$(OUTDIR)\srp.pdb" /machine:I386
LINK32_FLAGS = $(LINK32_FLAGS) /out:"$(OUTDIR)\srp.dll" /implib:"$(OUTDIR)\srp.lib"
#/libpath:"../lib"

!IF  "$(CFG)" == "Release"
LINK32_FLAGS= $(LINK32_FLAGS) /incremental:no
!ELSEIF  "$(CFG)" == "Debug"
LINK32_FLAGS= $(LINK32_FLAGS) /incremental:yes /debug /pdbtype:sept
!ENDIF

LINK32_FLAGS = $(LIBS) $(LINK32_FLAGS)

LINK32_OBJS= \
	"$(INTDIR)\cstr.obj" \
	"$(INTDIR)\rfc2945_client.obj" \
	"$(INTDIR)\rfc2945_server.obj" \
	"$(INTDIR)\srp.obj" \
	"$(INTDIR)\srp6_client.obj" \
	"$(INTDIR)\srp6_server.obj" \
	"$(INTDIR)\t_client.obj" \
	"$(INTDIR)\t_conf.obj" \
	"$(INTDIR)\t_conv.obj" \
	"$(INTDIR)\t_getpass.obj" \
	"$(INTDIR)\t_math.obj" \
	"$(INTDIR)\t_misc.obj" \
	"$(INTDIR)\t_pw.obj" \
	"$(INTDIR)\t_read.obj" \
	"$(INTDIR)\t_server.obj" \
	"$(INTDIR)\t_sha.obj" \
	"$(INTDIR)\t_truerand.obj"

"$(OUTDIR)\srp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

# Microsoft Developer Studio Generated Dependency File, included by srp.mak

libsrp\cstr.c : \
	"libsrp\cstr.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\rfc2945_client.c : \
	"libsrp\cstr.h"\
	"libsrp\srp.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_pwd.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\rfc2945_server.c : \
	"libsrp\cstr.h"\
	"libsrp\srp.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\srp.c : \
	"libsrp\cstr.h"\
	"libsrp\srp.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_pwd.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\srp6_client.c : \
	"libsrp\cstr.h"\
	"libsrp\srp.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_pwd.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\srp6_server.c : \
	"libsrp\cstr.h"\
	"libsrp\srp.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_client.c : \
	"libsrp\cstr.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_client.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_pwd.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_conf.c : \
	"libsrp\cstr.h"\
	"libsrp\nys_config.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_pwd.h"\
	"libsrp\t_read.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_conv.c : \
	"libsrp\cstr.h"\
	"libsrp\t_defines.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_getpass.c : \
	"libsrp\t_defines.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_math.c : \
	"libsrp\cstr.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_pwd.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_misc.c : \
	"libsrp\t_defines.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_pw.c : \
	"libsrp\cstr.h"\
	"libsrp\nys_config.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_pwd.h"\
	"libsrp\t_read.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_read.c : \
	"libsrp\cstr.h"\
	"libsrp\t_read.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_server.c : \
	"libsrp\cstr.h"\
	"libsrp\srp_aux.h"\
	"libsrp\t_defines.h"\
	"libsrp\t_pwd.h"\
	"libsrp\t_server.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_sha.c : \
	"libsrp\t_defines.h"\
	"libsrp\t_sha.h"\
	"win32\libsrp_openssl\config.h"\


libsrp\t_truerand.c : \
	"libsrp\t_defines.h"\
	"win32\libsrp_openssl\config.h"\




SOURCE=libsrp\cstr.c
"$(INTDIR)\cstr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\rfc2945_client.c
"$(INTDIR)\rfc2945_client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\rfc2945_server.c
"$(INTDIR)\rfc2945_server.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\srp.c
"$(INTDIR)\srp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\srp6_client.c
"$(INTDIR)\srp6_client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\srp6_server.c
"$(INTDIR)\srp6_server.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_client.c
"$(INTDIR)\t_client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_conf.c
"$(INTDIR)\t_conf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_conv.c
"$(INTDIR)\t_conv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_getpass.c
"$(INTDIR)\t_getpass.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_math.c
"$(INTDIR)\t_math.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_misc.c
"$(INTDIR)\t_misc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_pw.c
"$(INTDIR)\t_pw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_read.c
"$(INTDIR)\t_read.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_server.c
"$(INTDIR)\t_server.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_sha.c
"$(INTDIR)\t_sha.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libsrp\t_truerand.c
"$(INTDIR)\t_truerand.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)
