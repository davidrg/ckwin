# CM.MAK - Kermit 95 Connection Manager Makefile
#
# Authors:
#  David Goodwin <david@zx.net.nz>
#
# Build with:
#     Release, ANSI: nmake -f cm.mak
#       Debug, ANSI: nmake -f cm.mak DEBUG=yes
#  Release, Unicode: nmake -f cm.mak UNICODE=yes
#    Debug, Unicode: nmake -f cm.mak DEBUG=yes UNICODE=yes
#

# --------------------------------------------------------------
# Try to detect the compiler version being used so we can adjust
# compiler flags accordingly.
!message Attempting to detect compiler...
!include ..\..\k95\compiler_detect.mak

# --------------------------------------------------------------
# Set Enabled Features

!if ($(MSC_VER) > 150)
# Visual C++ 2010 and newer have all the modern shell stuff. Visual C++ 2008
# should support it too provided the Windows 7 SDK is installed. Turn on support
# for JumpLists
FEAT_JUMPLISTS=yes
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DCKMODERNSHELL
ENABLED_FEATURES = JumpLists
!else
DISABLED_FEATURES = JumpLists
!endif

!if "$(UNICODE)" == "yes"
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DUNICODE -D_UNICODE
ENABLED_FEATURES = $(ENABLED_FEATURES) Unicode
!endif

# --------------------------------------------------------------
# Output Results

!message
!message
!message ===============================================================================
!message K95 Connection Manager - Build Configuration
!message ===============================================================================
!message  Platform:                 $(PLATFORM)
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

!if "$(TARGET_PLATFORM)" != "Windows"
!error Invalid target platform
!endif

# --------------------------------------------------------------
# Set compiler flags

# CFLG_EHSC - Synchronous C++ Exception Handling, extern "C" defaults to nothrow
# /GX is equivalent to /EHsc, and was deprecated in Visual C++ 2019 or earlier,
# removed in Visual C++ 2022.
!if $(MSC_VER) >= 110
# 110 = Visual C++ 5.0
CFLG_EHSC=/EHsc
!else
# This is supported through to Visual C++
CFLG_EHSC=/GX
!endif

# CFLG_ML - Link with Single-threaded runtime (LIBC.LIB) if possible
# The single-threaded runtime was removed in Visual C++ 2005. Applications are
# to use the multithreaded runtime (/MT) instead.
!if $(MSC_VER) >= 140
# 140 = Visual C++ 2005 (8.0)
CFLG_ML=/MT
CFLG_MLD=/MTd
!else
CFLG_ML=/ML
CFLG_MLD=/MLd
!endif

# CFLG_YX - Limit precompilation to header files
# /YX removed in Visual C++ 2005
!if $(MSC_VER) >= 140
# 140 = Visual C++ 2005 (8.0)
CFLG_YX=
!else
CFLG_YX=/YX
!endif


# --------------------------------------------------------------
# Connection Manager Build


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
LINK32=link.exe

# CFLAGS=
# LDFLAGS=
# RCFLAGS=

# !if "$(UNICODE)" == "yes"
# OUTDIR=".\Unicode "
# CFLAGS=$(CFLAGS)  /D "UNICODE" /D "_UNICODE"
# !else
# OUTDIR=".\"
# !endif


!if  "$(DEBUG)" == "yes"
OUTDIR=$(OUTDIR)Debug
CFLAGS=$(CFLAGS) $(CFLG_MLD) /Gm /Zi /Od /D "_DEBUG" $(ENABLED_FEATURE_DEFS)
LDFLAGS=$(LDFLAGS) /incremental:yes /debug /pdbtype:sept
RCFLAGS=$(RCFLAGS) /d "_DEBUG"
!else
OUTDIR=.\Release
CFLAGS=$(CFLAGS) $(CFLG_ML) /O2 /D "NDEBUG" $(ENABLED_FEATURE_DEFS)
LDFLAGS=$(LDFLAGS) /incremental:no
RCFLAGS=$(RCFLAGS) /d "NDEBUG"
!endif

LIBS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
      advapi32.lib shell32.lib comctl32.lib version.lib ole32.lib

CPP_PROJ=/nologo /W3 $(CFLG_EHSC) $(CFLAGS) /D "WIN32" /D "_WINDOWS" /D _CRT_SECURE_NO_WARNINGS \
    /Fp"$(OUTDIR)\cm.pch" $(CFLG_YX) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /FD /c
LINK32_FLAGS=$(LIBS) $(LDFLAGS) /nologo /subsystem:windows \
  /pdb:"$(OUTDIR)\cm.pdb" /machine:I386 /out:"$(OUTDIR)\cm.exe"

LINK32_OBJS= \
	"$(OUTDIR)\charset.obj" \
	"$(OUTDIR)\cJSON.obj" \
	"$(OUTDIR)\cJSON_Utils.obj" \
	"$(OUTDIR)\cm.res" \
	"$(OUTDIR)\cmstring.obj" \
	"$(OUTDIR)\config_file.obj" \
	"$(OUTDIR)\conn_list.obj" \
	"$(OUTDIR)\conn_profile.obj" \
	"$(OUTDIR)\conn_props.obj" \
	"$(OUTDIR)\connection.obj" \
	"$(OUTDIR)\connection_serial.obj" \
	"$(OUTDIR)\general.obj" \
	"$(OUTDIR)\gui.obj" \
	"$(OUTDIR)\gui_colors.obj" \
	"$(OUTDIR)\json_color_theme.obj" \
	"$(OUTDIR)\json_config.obj" \
	"$(OUTDIR)\json_profile.obj" \
	"$(OUTDIR)\json_util.obj" \
	"$(OUTDIR)\jumplist.obj" \
	"$(OUTDIR)\kerm_track.obj" \
	"$(OUTDIR)\keyboard.obj" \
	"$(OUTDIR)\logging.obj" \
	"$(OUTDIR)\login.obj" \
	"$(OUTDIR)\main.obj" \
	"$(OUTDIR)\new_conn.obj" \
	"$(OUTDIR)\printer.obj" \
	"$(OUTDIR)\statusbar.obj" \
	"$(OUTDIR)\term_colors.obj" \
	"$(OUTDIR)\term_info.obj" \
	"$(OUTDIR)\terminal.obj" \
	"$(OUTDIR)\toolbar.obj" \
	"$(OUTDIR)\transfer.obj" \
	"$(OUTDIR)\util.obj"

ALL : "$(OUTDIR)\cm.exe"

CLEAN :
	-@erase "$(OUTDIR)\charset.obj"
	-@erase "$(OUTDIR)\cJSON.obj"
	-@erase "$(OUTDIR)\cJSON_Utils.obj"
	-@erase "$(OUTDIR)\cm.res"
	-@erase "$(OUTDIR)\cmstring.obj"
	-@erase "$(OUTDIR)\config_file.obj"
	-@erase "$(OUTDIR)\conn_list.obj"
	-@erase "$(OUTDIR)\conn_profile.obj"
	-@erase "$(OUTDIR)\conn_props.obj"
	-@erase "$(OUTDIR)\connection.obj"
	-@erase "$(OUTDIR)\connection_serial.obj"
	-@erase "$(OUTDIR)\general.obj"
	-@erase "$(OUTDIR)\gui.obj"
	-@erase "$(OUTDIR)\gui_colors.obj"
	-@erase "$(OUTDIR)\json_color_theme.obj"
	-@erase "$(OUTDIR)\json_config.obj"
	-@erase "$(OUTDIR)\json_profile.obj"
	-@erase "$(OUTDIR)\json_util.obj"
	-@erase "$(OUTDIR)\jumplist.obj"
	-@erase "$(OUTDIR)\kerm_track.obj"
	-@erase "$(OUTDIR)\keyboard.obj"
	-@erase "$(OUTDIR)\logging.obj"
	-@erase "$(OUTDIR)\login.obj"
	-@erase "$(OUTDIR)\main.obj"
	-@erase "$(OUTDIR)\new_conn.obj"
	-@erase "$(OUTDIR)\printer.obj"
	-@erase "$(OUTDIR)\statusbar.obj"
	-@erase "$(OUTDIR)\term_colors.obj"
	-@erase "$(OUTDIR)\term_info.obj"
	-@erase "$(OUTDIR)\terminal.obj"
	-@erase "$(OUTDIR)\toolbar.obj"
	-@erase "$(OUTDIR)\transfer.obj"
	-@erase "$(OUTDIR)\util.obj"
	-@erase "$(OUTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\cm.exe"
	-@erase "$(OUTDIR)\cm.exp"
	-@erase "$(OUTDIR)\cm.lib"
	-@erase "$(OUTDIR)\cm.pch"
	-@erase "$(OUTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\vc140.idb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(OUTDIR)\cm.exe" : "$(OUTDIR)" $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

.cpp{$(OUTDIR)}.obj:
   $(CPP) @<< $(CPP_PROJ) $<
<<
{conn_props}.cpp{$(OUTDIR)}.obj:
   $(CPP) @<< $(CPP_PROJ) $<
<<
{cJSON}.c{$(OUTDIR)}.obj:
   $(CPP) @<< $(CPP_PROJ) $<
<<

"$(OUTDIR)\conn_props.obj" : "$(OUTDIR)" conn_props.cpp charset.h cmstring.h \
                             conn_profile.h conn_props.h term_info.h
"$(OUTDIR)\connection.obj" : "$(OUTDIR)" conn_props\connection.cpp charset.h \
                             cmstring.h conn_profile.h conn_props.h term_info.h
"$(OUTDIR)\connection_serial.obj" : "$(OUTDIR)" conn_props\connection_serial.cpp \
                            charset.h cmstring.h conn_profile.h conn_props.h \
                            term_info.h util.h
"$(OUTDIR)\general.obj"    : "$(OUTDIR)" conn_props\general.cpp charset.h \
                             cmstring.h conn_profile.h conn_props.h term_info.h
"$(OUTDIR)\gui.obj"        : "$(OUTDIR)" conn_props\gui.cpp charset.h \
                             cmstring.h conn_profile.h conn_props.h term_info.h
"$(OUTDIR)\gui_colors.obj" : "$(OUTDIR)" conn_props\gui_colors.cpp charset.h \
                             cmstring.h color_theme.h config_file.h conn_profile.h \
                             conn_props.h term_info.h
"$(OUTDIR)\keyboard.obj"   : "$(OUTDIR)" conn_props\keyboard.cpp cmstring.h \
                             conn_profile.h conn_props.h term_info.h
"$(OUTDIR)\logging.obj"          : "$(OUTDIR)" conn_props\logging.cpp charset.h \
                                   cmstring.h conn_profile.h conn_props.h \
                                   term_info.h
"$(OUTDIR)\login.obj" : "$(OUTDIR)" conn_props\login.cpp charset.h cmstring.h \
                        conn_profile.h conn_props.h term_info.h
"$(OUTDIR)\logging.obj"          : "$(OUTDIR)" conn_props\logging.cpp charset.h \
                                   cmstring.h conn_profile.h conn_props.h \
                                   term_info.h
"$(OUTDIR)\printer.obj"          : "$(OUTDIR)" conn_props\printer.cpp charset.h \
                                   cmstring.h conn_profile.h conn_props.h \
                                   term_info.h
"$(OUTDIR)\term_colors.obj" : "$(OUTDIR)" conn_props\term_colors.cpp charset.h \
                             cmstring.h conn_profile.h conn_props.h term_info.h
"$(OUTDIR)\terminal.obj" : "$(OUTDIR)" conn_props\terminal.cpp charset.h cmstring.h \
                            conn_profile.h conn_props.h term_info.h
"$(OUTDIR)\transfer.obj"         : "$(OUTDIR)" conn_props\transfer.cpp charset.h \
                                   cmstring.h conn_profile.h conn_props.h \
                                   term_info.h
"$(OUTDIR)\cJSON.obj" : "$(OUTDIR)" cJSON\cJSON.c  cJSON\cJSON.h
"$(OUTDIR)\cJSON_Utils.obj" : cJSON\cJSON_Utils.c "$(OUTDIR)" cJSON\cJSON.h cJSON\cJSON_Utils.h
"$(OUTDIR)\config_file.obj"  : "$(OUTDIR)" config_file.cpp charset.h cmstring.h \
                               color_theme.h config_file.h conn_profile.h term_info.h
"$(OUTDIR)\conn_profile.obj" : "$(OUTDIR)" conn_profile.cpp charset.h cmstring.h \
                               conn_profile.h ipc_messages.h kerm_track.h \
                               term_info.h util.h
"$(OUTDIR)\json_color_theme.obj" : "$(OUTDIR)" json_color_theme.cpp charset.h \
                                   cJSON\cJSON.h cmstring.h color_theme.h \
                                   config_file.h conn_profile.h \
                                   json_color_theme.h json_config.h json_util.h \
                                   term_info.h

"$(OUTDIR)\json_config.obj" : json_config.cpp "$(OUTDIR)" charset.h cJSON\cJSON.h \
                              cmstring.h  config_file.h conn_profile.h json_config.h \
                              json_profile.h term_info.h
"$(OUTDIR)\json_profile.obj" : json_profile.cpp "$(OUTDIR)" charset.h cJSON\cJSON.h \
                               cmstring.h config_file.h  conn_profile.h json_config.h \
                               json_profile.h term_info.h
"$(OUTDIR)\json_util.obj"        : "$(OUTDIR)" json_util.cpp charset.h \
                                   cJSON\cJSON.h cmstring.h color_theme.h \
                                   config_file.h conn_profile.h \
                                   json_config.h json_util.h term_info.h
"$(OUTDIR)\jumplist.obj" : jumplist.cpp "$(OUTDIR)" jumplist.h config_file.h
"$(OUTDIR)\charset.obj" : charset.cpp "$(OUTDIR)" charset.h
"$(OUTDIR)\cmstring.obj" : cmstring.cpp "$(OUTDIR)" cmstring.h util.h
"$(OUTDIR)\conn_list.obj" : "$(OUTDIR)" conn_list.cpp charset.h cmstring.h \
                            config_file.h conn_list.h conn_profile.h jumplist.h \
                            kerm_track.h statusbar.h term_info.h toolbar.h util.h
"$(OUTDIR)\kerm_track.obj" : kerm_track.cpp "$(OUTDIR)" charset.h cmstring.h conn_profile.h \
                             ipc_messages.h kerm_track.h term_info.h
"$(OUTDIR)\main.obj" : main.cpp "$(OUTDIR)" charset.h cmstring.h color_theme.h \
                       config_file.h conn_list.h conn_profile.h conn_props.h \
                       ipc_messages.h json_config.h kerm_track.h new_conn.h \
                       statusbar.h term_info.h toolbar.h util.h jumplist.h
"$(OUTDIR)\new_conn.obj" : new_conn.cpp "$(OUTDIR)" charset.h cmstring.h config_file.h \
                           conn_list.h conn_profile.h conn_props.h term_info.h util.h
"$(OUTDIR)\statusbar.obj" : statusbar.cpp "$(OUTDIR)" charset.h cmstring.h conn_profile.h \
                            kerm_track.h statusbar.h term_info.h
"$(OUTDIR)\term_info.obj" : term_info.cpp "$(OUTDIR)" charset.h term_info.h
"$(OUTDIR)\toolbar.obj" : toolbar.cpp "$(OUTDIR)" toolbar.h
"$(OUTDIR)\util.obj" : util.cpp "$(OUTDIR)" util.h

"$(OUTDIR)\cm.res" : cm.rc "$(OUTDIR)" logo.ico toolbar1.bmp
	$(RSC) /l 0x409 /fo"$(OUTDIR)\cm.res" /i "." $(RCFLAGS) cm.rc
