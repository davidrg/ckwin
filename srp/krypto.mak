# Microsoft Developer Studio Generated NMAKE File, Based on krypto.dsp
!IF "$(CFG)" == ""
CFG=Debug
!MESSAGE No configuration specified. Defaulting to Debug.
!ENDIF

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "krypto.mak" CFG="Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

!message Attempting to detect compiler...
!include ..\kermit\k95\compiler_detect.mak

!message
!message
!message ===============================================================================
!message libkrypto Build Configuration
!message ===============================================================================
!message  Architecture:             $(TARGET_CPU)
!message  Compiler:                 $(COMPILER)
!message  Compiler Version:         $(COMPILER_VERSION)
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
OUTDIR=.\win32\libkrypto_openssl\Release
INTDIR=.\win32\libkrypto_openssl\Release
OutDir=.\win32\libkrypto_openssl\Release
!ELSEIF  "$(CFG)" == "Debug"
OUTDIR=.\win32\libkrypto_openssl\Debug
INTDIR=.\win32\libkrypto_openssl\Debug
OutDir=.\win32\libkrypto_openssl\Debug
!ENDIF

ALL : "$(OUTDIR)\krypto.dll"


CLEAN :
	-@erase "$(INTDIR)\cipher.obj"
	-@erase "$(INTDIR)\cipher_crypt_cbc.obj"
	-@erase "$(INTDIR)\cipher_crypt_cfb.obj"
	-@erase "$(INTDIR)\cipher_crypt_ecb.obj"
	-@erase "$(INTDIR)\cipher_crypt_ofb.obj"
	-@erase "$(INTDIR)\cipher_imp_blowfish.obj"
	-@erase "$(INTDIR)\cipher_imp_cast.obj"
	-@erase "$(INTDIR)\cipher_imp_des.obj"
	-@erase "$(INTDIR)\cipher_imp_none.obj"
	-@erase "$(INTDIR)\hash.obj"
	-@erase "$(INTDIR)\hash_imp_md5.obj"
	-@erase "$(INTDIR)\hash_imp_sha.obj"
	-@erase "$(INTDIR)\krypto.obj"
	-@erase "$(INTDIR)\krypto_rand_conf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\krypto.dll"
	-@erase "$(OUTDIR)\krypto.exp"
	-@erase "$(OUTDIR)\krypto.lib"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\krypto.ilk"
	-@erase "$(OUTDIR)\krypto.pdb"
	-@erase "$(OUTDIR)\krypto.pch"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CFLAGS=  /nologo /W3 /I ".\win32\libkrypto_openssl" /I "../include" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KRYPTO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\krypto.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

!if ($(MSC_VER) < 140)
CFLAGS=$(CFLAGS) /GX /YX
!else
CFLAGS=$(CFLAGS) /EHsc
!endif

!IF  "$(CFG)" == "Release"
!if "$(CKB_STATIC_CRT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MT
!else
COMMON_CFLAGS = /MD
!endif
!ELSEIF  "$(CFG)" == "Debug"
!if "$(CKB_STATIC_CRT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MTd
!else
COMMON_CFLAGS = /MDd
!endif
!ENDIF

CPP=cl.exe

!IF  "$(CFG)" == "Release"
CPP_PROJ=$(CFLAGS) $(COMMON_CFLAGS) /O2  /D "NDEBUG"
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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\krypto.bsc"
BSC32_SBRS= \

LINK32=link.exe

LIBS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
LIBS=$(LIBS) shell32.lib ole32.lib oleaut32.lib uuid.lib
LIBS=$(LIBS) $(CKF_SSL_LIBS)
# odbc32.lib odbccp32.lib

LINK32_FLAGS=/nologo /dll /pdb:"$(OUTDIR)\krypto.pdb" /machine:I386 /out:"$(OUTDIR)\krypto.dll" /implib:"$(OUTDIR)\krypto.lib" /libpath:"../lib"

!IF  "$(CFG)" == "Release"
LINK32_FLAGS=$(LINK32_FLAGS) /incremental:no
!ELSEIF  "$(CFG)" == "Debug"
LINK32_FLAGS=$(LINK32_FLAGS) /incremental:yes /debug /pdbtype:sept
!ENDIF

LINK32_FLAGS=$(LIBS) $(LINK32_FLAGS)


LINK32_OBJS= \
	"$(INTDIR)\cipher.obj" \
	"$(INTDIR)\cipher_crypt_cbc.obj" \
	"$(INTDIR)\cipher_crypt_cfb.obj" \
	"$(INTDIR)\cipher_crypt_ecb.obj" \
	"$(INTDIR)\cipher_crypt_ofb.obj" \
	"$(INTDIR)\cipher_imp_blowfish.obj" \
	"$(INTDIR)\cipher_imp_cast.obj" \
	"$(INTDIR)\cipher_imp_des.obj" \
	"$(INTDIR)\cipher_imp_none.obj" \
	"$(INTDIR)\hash.obj" \
	"$(INTDIR)\hash_imp_md5.obj" \
	"$(INTDIR)\hash_imp_sha.obj" \
	"$(INTDIR)\krypto.obj" \
	"$(INTDIR)\krypto_rand_conf.obj"

"$(OUTDIR)\krypto.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<



libkrypto\cipher.c : \
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\cipher_crypt_cbc.c : \
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\cipher_crypt_cfb.c : \
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\cipher_crypt_ecb.c : \
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\cipher_crypt_ofb.c : \
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\cipher_imp_blowfish.c : \
	"libkrypto\cipher_imp_blowfish.h"\
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\cipher_imp_cast.c : \
	"cast\cast.c"\
	"cast\cast.h"\
	"libkrypto\cipher_imp_cast.h"\
	"libkrypto\krypto.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\cipher_imp_des.c : \
	"libkrypto\cipher_imp_des.h"\
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\cipher_imp_none.c : \
	"libkrypto\krypto.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\hash.c : \
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\hash_imp_md5.c : \
	"libkrypto\hash_imp_md5.h"\
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\hash_imp_sha.c : \
	"libkrypto\hash_imp_sha.h"\
	"libkrypto\krypto.h"\
	"libsrp\t_sha.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\krypto.c : \
	"libkrypto\krypto.h"\
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\


libkrypto\krypto_rand_conf.c : \
	"libkrypto\krypto_locl.h"\
	".\win32\libkrypto_openssl\config.h"\



SOURCE=libkrypto\cipher.c

"$(INTDIR)\cipher.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\cipher_crypt_cbc.c

"$(INTDIR)\cipher_crypt_cbc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\cipher_crypt_cfb.c

"$(INTDIR)\cipher_crypt_cfb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\cipher_crypt_ecb.c

"$(INTDIR)\cipher_crypt_ecb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\cipher_crypt_ofb.c

"$(INTDIR)\cipher_crypt_ofb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\cipher_imp_blowfish.c

"$(INTDIR)\cipher_imp_blowfish.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\cipher_imp_cast.c

"$(INTDIR)\cipher_imp_cast.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\cipher_imp_des.c

"$(INTDIR)\cipher_imp_des.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\cipher_imp_none.c

"$(INTDIR)\cipher_imp_none.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\hash.c

"$(INTDIR)\hash.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\hash_imp_md5.c

"$(INTDIR)\hash_imp_md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\hash_imp_sha.c

"$(INTDIR)\hash_imp_sha.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\krypto.c

"$(INTDIR)\krypto.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=libkrypto\krypto_rand_conf.c

"$(INTDIR)\krypto_rand_conf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



