!IF "$(CFG)" == ""
CFG=Release
!MESSAGE No configuration specified. Defaulting to Release.
!ENDIF 

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libdes.mak" CFG="libdes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Release" - Release build
!MESSAGE "Debug"   - Debug build
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

# This is needed by compiler_detect.mak
PLATFORM=NT

!message Attempting to detect compiler...
!include ..\kermit\k95\compiler_detect.mak

!message
!message
!message ===============================================================================
!message libdes Build Configuration
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

CPP=cl.exe

SRC=des

!IF  "$(CFG)" == "Release"
OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros
!ELSE
OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros
!ENDIF

ALL : "$(OUTDIR)\libdes.lib"

CLEAN :
	-@erase "$(INTDIR)\cbc3_enc.obj"
	-@erase "$(INTDIR)\cbc_cksm.obj"
	-@erase "$(INTDIR)\cbc_enc.obj"
	-@erase "$(INTDIR)\cfb64ede.obj"
	-@erase "$(INTDIR)\cfb64enc.obj"
	-@erase "$(INTDIR)\cfb_enc.obj"
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\des3s.obj"
	-@erase "$(INTDIR)\des_enc.obj"
	-@erase "$(INTDIR)\des_opts.obj"
	-@erase "$(INTDIR)\dess.obj"
	-@erase "$(INTDIR)\ecb3_enc.obj"
	-@erase "$(INTDIR)\ecb_enc.obj"
	-@erase "$(INTDIR)\enc_read.obj"
	-@erase "$(INTDIR)\enc_writ.obj"
	-@erase "$(INTDIR)\fcrypt.obj"
	-@erase "$(INTDIR)\fcrypt_b.obj"
	-@erase "$(INTDIR)\ofb64ede.obj"
	-@erase "$(INTDIR)\ofb64enc.obj"
	-@erase "$(INTDIR)\ofb_enc.obj"
	-@erase "$(INTDIR)\pcbc_enc.obj"
	-@erase "$(INTDIR)\qud_cksm.obj"
	-@erase "$(INTDIR)\rand_key.obj"
	-@erase "$(INTDIR)\read2pwd.obj"
	-@erase "$(INTDIR)\read_pwd.obj"
	-@erase "$(INTDIR)\rpc_enc.obj"
	-@erase "$(INTDIR)\rpw.obj"
	-@erase "$(INTDIR)\set_key.obj"
	-@erase "$(INTDIR)\speed.obj"
	-@erase "$(INTDIR)\str2key.obj"
	-@erase "$(INTDIR)\supp.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\xcbc_enc.obj"
	-@erase "$(OUTDIR)\libdes.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CFLAGS=/nologo /W3 /D "WIN32" /D "_WINDOWS" /D MSDOS=1 \
	/Fp"$(INTDIR)\libdes.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /c

#

# /ML	- Link with LIBC.LIB (/MLd = LIBCD.LIB)
# /YX   - Automatic PCH
# /GX   - Enable C++ Exception Handling (same as /EHsc)

!if ($(MSC_VER) < 140)
CFLAGS=$(CFLAGS) /GX

# Jom runs multiple instances of cl in parallel which causes issues with PCH locking.
!if "$(ISJOM)" == "no"
CFLAGS=$(CFLAGS) /YX
!endif

!else
CFLAGS=$(CFLAGS) /EHsc
!endif

!if "$(CKB_STATIC_CRT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MT
!else
COMMON_CFLAGS = /ML
!endif

!if "$(TARGET_CPU)" == "AXP64"
# This compiler is capable of targeting AXP64, so add the build flag to do that.
COMMON_CFLAGS = $(COMMON_CFLAGS) /Ap64 -DCK_HAVE_INTPTR_T
!endif

!IF  "$(CFG)" == "Release"
CFLAGS=$(CFLAGS) /O2 /D "NDEBUG"


!if "$(CKB_STATIC_CRT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MT
!else
COMMON_CFLAGS = /MD
!endif


CPP_OBJS=.\Release/
!ELSE
CFLAGS=$(CFLAGS) /Z7 /Od /D "_DEBUG"

!if "$(CKB_STATIC_CRT)"=="yes"
!message Building with statically linked native CRT as requested.
COMMON_CFLAGS = /MTd
!else
COMMON_CFLAGS = /MDd
!endif

CPP_OBJS=.\Debug/
!ENDIF

CFLAGS=$(CFLAGS) $(COMMON_CFLAGS)

CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libdes.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libdes.lib" 
LIB32_OBJS= \
	"$(INTDIR)\cbc3_enc.obj" \
	"$(INTDIR)\cbc_cksm.obj" \
	"$(INTDIR)\cbc_enc.obj" \
	"$(INTDIR)\cfb64ede.obj" \
	"$(INTDIR)\cfb64enc.obj" \
	"$(INTDIR)\cfb_enc.obj" \
	"$(INTDIR)\des_enc.obj" \
	"$(INTDIR)\ecb3_enc.obj" \
	"$(INTDIR)\ecb_enc.obj" \
	"$(INTDIR)\enc_read.obj" \
	"$(INTDIR)\enc_writ.obj" \
	"$(INTDIR)\fcrypt.obj" \
	"$(INTDIR)\fcrypt_b.obj" \
	"$(INTDIR)\ofb64ede.obj" \
	"$(INTDIR)\ofb64enc.obj" \
	"$(INTDIR)\ofb_enc.obj" \
	"$(INTDIR)\pcbc_enc.obj" \
	"$(INTDIR)\qud_cksm.obj" \
	"$(INTDIR)\rand_key.obj" \
	"$(INTDIR)\read2pwd.obj" \
	"$(INTDIR)\read_pwd.obj" \
	"$(INTDIR)\rpc_enc.obj" \
	"$(INTDIR)\set_key.obj" \
	"$(INTDIR)\str2key.obj" \
	"$(INTDIR)\supp.obj" \
	"$(INTDIR)\xcbc_enc.obj"

"$(OUTDIR)\libdes.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CFLAGS) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CFLAGS) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CFLAGS) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CFLAGS) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CFLAGS) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CFLAGS) $< 
<<


SOURCE=$(SRC)\cbc3_enc.c
DEP_CPP_CBC3_=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
"$(INTDIR)\cbc3_enc.obj" : $(SOURCE) $(DEP_CPP_CBC3_) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\cbc_cksm.c
DEP_CPP_CBC_C=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
"$(INTDIR)\cbc_cksm.obj" : $(SOURCE) $(DEP_CPP_CBC_C) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\cbc_enc.c
DEP_CPP_CBC_E=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
"$(INTDIR)\cbc_enc.obj" : $(SOURCE) $(DEP_CPP_CBC_E) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\cfb64ede.c
DEP_CPP_CFB64=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
"$(INTDIR)\cfb64ede.obj" : $(SOURCE) $(DEP_CPP_CFB64) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\cfb64enc.c
DEP_CPP_CFB64E=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
"$(INTDIR)\cfb64enc.obj" : $(SOURCE) $(DEP_CPP_CFB64E) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\cfb_enc.c
DEP_CPP_CFB_E=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
"$(INTDIR)\cfb_enc.obj" : $(SOURCE) $(DEP_CPP_CFB_E) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\des.c
DEP_CPP_DES_C=\
	"$(SRC)\des.h"\
	"$(SRC)\des_ver.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
"$(INTDIR)\des.obj" : $(SOURCE) $(DEP_CPP_DES_C) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\des3s.cpp
DEP_CPP_DES3S=\
	"$(SRC)\des.h"\
	
"$(INTDIR)\des3s.obj" : $(SOURCE) $(DEP_CPP_DES3S) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\des_enc.c
DEP_CPP_DES_E=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
"$(INTDIR)\des_enc.obj" : $(SOURCE) $(DEP_CPP_DES_E) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\des_opts.c
DEP_CPP_DES_O=\
	"$(SRC)\des.h"\
	"$(SRC)\des_enc.c"\
	"$(SRC)\des_locl.h"\
	"$(SRC)\spr.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	
"$(INTDIR)\des_opts.obj" : $(SOURCE) $(DEP_CPP_DES_O) "$(INTDIR)"\
 "$(SRC)\des_enc.c"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\dess.cpp
DEP_CPP_DESS_=\
	"$(SRC)\des.h"\
	
	
"$(INTDIR)\dess.obj" : $(SOURCE) $(DEP_CPP_DESS_) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\ecb3_enc.c
DEP_CPP_ECB3_=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
	
"$(INTDIR)\ecb3_enc.obj" : $(SOURCE) $(DEP_CPP_ECB3_) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\ecb_enc.c
DEP_CPP_ECB_E=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	"$(SRC)\spr.h"\
	
	
"$(INTDIR)\ecb_enc.obj" : $(SOURCE) $(DEP_CPP_ECB_E) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\enc_read.c
DEP_CPP_ENC_R=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
	
"$(INTDIR)\enc_read.obj" : $(SOURCE) $(DEP_CPP_ENC_R) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\enc_writ.c
DEP_CPP_ENC_W=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
	
"$(INTDIR)\enc_writ.obj" : $(SOURCE) $(DEP_CPP_ENC_W) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\fcrypt.c
DEP_CPP_FCRYP=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
	
"$(INTDIR)\fcrypt.obj" : $(SOURCE) $(DEP_CPP_FCRYP) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\fcrypt_b.c
DEP_CPP_FCRYPT=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
"$(INTDIR)\fcrypt_b.obj" : $(SOURCE) $(DEP_CPP_FCRYPT) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\ofb64ede.c
DEP_CPP_OFB64=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
	
"$(INTDIR)\ofb64ede.obj" : $(SOURCE) $(DEP_CPP_OFB64) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\ofb64enc.c
DEP_CPP_OFB64E=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
	
"$(INTDIR)\ofb64enc.obj" : $(SOURCE) $(DEP_CPP_OFB64E) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\ofb_enc.c
DEP_CPP_OFB_E=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
	
"$(INTDIR)\ofb_enc.obj" : $(SOURCE) $(DEP_CPP_OFB_E) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\pcbc_enc.c
DEP_CPP_PCBC_=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	

"$(INTDIR)\pcbc_enc.obj" : $(SOURCE) $(DEP_CPP_PCBC_) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\qud_cksm.c
DEP_CPP_QUD_C=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	

"$(INTDIR)\qud_cksm.obj" : $(SOURCE) $(DEP_CPP_QUD_C) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\rand_key.c
DEP_CPP_RAND_=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	

"$(INTDIR)\rand_key.obj" : $(SOURCE) $(DEP_CPP_RAND_) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\read2pwd.c
DEP_CPP_READ2=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	

"$(INTDIR)\read2pwd.obj" : $(SOURCE) $(DEP_CPP_READ2) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\read_pwd.c
DEP_CPP_READ_=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	

"$(INTDIR)\read_pwd.obj" : $(SOURCE) $(DEP_CPP_READ_) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\rpc_enc.c

DEP_CPP_RPC_E=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	"$(SRC)\des_ver.h"\
	"$(SRC)\rpc_des.h"\
	

"$(INTDIR)\rpc_enc.obj" : $(SOURCE) $(DEP_CPP_RPC_E) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\rpw.c
DEP_CPP_RPW_C=\
	"$(SRC)\des.h"\
	

"$(INTDIR)\rpw.obj" : $(SOURCE) $(DEP_CPP_RPW_C) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\set_key.c
DEP_CPP_SET_K=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	"$(SRC)\podd.h"\
	"$(SRC)\sk.h"\
	

"$(INTDIR)\set_key.obj" : $(SOURCE) $(DEP_CPP_SET_K) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\speed.c
DEP_CPP_SPEED=\
	"$(SRC)\des.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\speed.obj" : $(SOURCE) $(DEP_CPP_SPEED) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\str2key.c
DEP_CPP_STR2K=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	

"$(INTDIR)\str2key.obj" : $(SOURCE) $(DEP_CPP_STR2K) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\supp.c
DEP_CPP_SUPP_=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	

"$(INTDIR)\supp.obj" : $(SOURCE) $(DEP_CPP_SUPP_) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)


SOURCE=$(SRC)\xcbc_enc.c
DEP_CPP_XCBC_=\
	"$(SRC)\des.h"\
	"$(SRC)\des_locl.h"\
	
	
"$(INTDIR)\xcbc_enc.obj" : $(SOURCE) $(DEP_CPP_XCBC_) "$(INTDIR)"
	$(CPP) $(CFLAGS) $(SOURCE)
