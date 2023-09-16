# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=cm - Win32 Unicode Debug
!MESSAGE No configuration specified.  Defaulting to cm - Win32 Unicode Debug.
!ENDIF 

!IF "$(CFG)" != "cm - Win32 Release" && "$(CFG)" != "cm - Win32 Debug" &&\
 "$(CFG)" != "cm - Win32 Unicode Debug" && "$(CFG)" !=\
 "cm - Win32 Unicode Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "cm.mak" CFG="cm - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cm - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "cm - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "cm - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "cm - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "cm - Win32 Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\cm.exe"

CLEAN : 
	-@erase ".\Release\cm.exe"
	-@erase ".\Release\term_info.obj"
	-@erase ".\Release\terminal.obj"
	-@erase ".\Release\statusbar.obj"
	-@erase ".\Release\conn_profile.obj"
	-@erase ".\Release\main.obj"
	-@erase ".\Release\cmstring.obj"
	-@erase ".\Release\cJSON.obj"
	-@erase ".\Release\new_conn.obj"
	-@erase ".\Release\conn_props.obj"
	-@erase ".\Release\json_profile.obj"
	-@erase ".\Release\general.obj"
	-@erase ".\Release\charset.obj"
	-@erase ".\Release\util.obj"
	-@erase ".\Release\toolbar.obj"
	-@erase ".\Release\json_config.obj"
	-@erase ".\Release\conn_list.obj"
	-@erase ".\Release\kerm_track.obj"
	-@erase ".\Release\term_colors.obj"
	-@erase ".\Release\cm.res"
	-@erase ".\Release\login.obj"
	-@erase ".\Release\connection.obj"
	-@erase ".\Release\connection_serial.obj"
	-@erase ".\Release\keyboard.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/cm.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/cm.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/cm.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib Version.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib comctl32.lib Version.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/cm.pdb" /machine:I386 /out:"$(OUTDIR)/cm.exe" 
LINK32_OBJS= \
	".\Release\term_info.obj" \
	".\Release\terminal.obj" \
	".\Release\statusbar.obj" \
	".\Release\conn_profile.obj" \
	".\Release\main.obj" \
	".\Release\cmstring.obj" \
	".\Release\cJSON.obj" \
	".\Release\new_conn.obj" \
	".\Release\conn_props.obj" \
	".\Release\json_profile.obj" \
	".\Release\general.obj" \
	".\Release\charset.obj" \
	".\Release\util.obj" \
	".\Release\toolbar.obj" \
	".\Release\json_config.obj" \
	".\Release\conn_list.obj" \
	".\Release\kerm_track.obj" \
	".\Release\term_colors.obj" \
	".\Release\login.obj" \
	".\Release\connection.obj" \
	".\Release\connection_serial.obj" \
	".\Release\keyboard.obj" \
	".\Release\cm.res"

"$(OUTDIR)\cm.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\cm.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\cm.exe"
	-@erase ".\Debug\main.obj"
	-@erase ".\Debug\charset.obj"
	-@erase ".\Debug\kerm_track.obj"
	-@erase ".\Debug\terminal.obj"
	-@erase ".\Debug\cJSON.obj"
	-@erase ".\Debug\conn_list.obj"
	-@erase ".\Debug\term_info.obj"
	-@erase ".\Debug\json_config.obj"
	-@erase ".\Debug\statusbar.obj"
	-@erase ".\Debug\util.obj"
	-@erase ".\Debug\term_colors.obj"
	-@erase ".\Debug\general.obj"
	-@erase ".\Debug\json_profile.obj"
	-@erase ".\Debug\conn_profile.obj"
	-@erase ".\Debug\cmstring.obj"
	-@erase ".\Debug\toolbar.obj"
	-@erase ".\Debug\conn_props.obj"
	-@erase ".\Debug\new_conn.obj"
	-@erase ".\Debug\cm.res"
	-@erase ".\Debug\login.obj"
	-@erase ".\Debug\connection.obj"
	-@erase ".\Debug\connection_serial.obj"
	-@erase ".\Debug\keyboard.obj"
	-@erase ".\Debug\cm.ilk"
	-@erase ".\Debug\cm.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/cm.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/cm.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/cm.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib Version.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib comctl32.lib Version.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/cm.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/cm.exe" 
LINK32_OBJS= \
	".\Debug\main.obj" \
	".\Debug\charset.obj" \
	".\Debug\kerm_track.obj" \
	".\Debug\terminal.obj" \
	".\Debug\cJSON.obj" \
	".\Debug\conn_list.obj" \
	".\Debug\term_info.obj" \
	".\Debug\json_config.obj" \
	".\Debug\statusbar.obj" \
	".\Debug\util.obj" \
	".\Debug\term_colors.obj" \
	".\Debug\general.obj" \
	".\Debug\json_profile.obj" \
	".\Debug\conn_profile.obj" \
	".\Debug\cmstring.obj" \
	".\Debug\toolbar.obj" \
	".\Debug\conn_props.obj" \
	".\Debug\new_conn.obj" \
	".\Debug\login.obj" \
	".\Debug\connection.obj" \
	".\Debug\connection_serial.obj" \
	".\Debug\keyboard.obj" \
	".\Debug\cm.res"

"$(OUTDIR)\cm.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cm___Win"
# PROP BASE Intermediate_Dir "cm___Win"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "cm___Win"
# PROP Intermediate_Dir "cm___Win"
# PROP Target_Dir ""
OUTDIR=.\cm___Win
INTDIR=.\cm___Win

ALL : "$(OUTDIR)\cm.exe"

CLEAN : 
	-@erase ".\cm___Win\vc40.pdb"
	-@erase ".\cm___Win\vc40.idb"
	-@erase ".\cm___Win\cm.exe"
	-@erase ".\cm___Win\charset.obj"
	-@erase ".\cm___Win\terminal.obj"
	-@erase ".\cm___Win\main.obj"
	-@erase ".\cm___Win\json_profile.obj"
	-@erase ".\cm___Win\conn_profile.obj"
	-@erase ".\cm___Win\term_info.obj"
	-@erase ".\cm___Win\cJSON.obj"
	-@erase ".\cm___Win\kerm_track.obj"
	-@erase ".\cm___Win\cmstring.obj"
	-@erase ".\cm___Win\general.obj"
	-@erase ".\cm___Win\conn_list.obj"
	-@erase ".\cm___Win\conn_props.obj"
	-@erase ".\cm___Win\toolbar.obj"
	-@erase ".\cm___Win\util.obj"
	-@erase ".\cm___Win\new_conn.obj"
	-@erase ".\cm___Win\json_config.obj"
	-@erase ".\cm___Win\statusbar.obj"
	-@erase ".\cm___Win\term_colors.obj"
	-@erase ".\cm___Win\cm.res"
	-@erase ".\cm___Win\login.obj"
	-@erase ".\cm___Win\connection.obj"
	-@erase ".\cm___Win\connection_serial.obj"
	-@erase ".\cm___Win\keyboard.obj"
	-@erase ".\cm___Win\cm.ilk"
	-@erase ".\cm___Win\cm.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)/cm.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\cm___Win/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/cm.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/cm.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib Version.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib comctl32.lib Version.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/cm.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/cm.exe" 
LINK32_OBJS= \
	".\cm___Win\charset.obj" \
	".\cm___Win\terminal.obj" \
	".\cm___Win\main.obj" \
	".\cm___Win\json_profile.obj" \
	".\cm___Win\conn_profile.obj" \
	".\cm___Win\term_info.obj" \
	".\cm___Win\cJSON.obj" \
	".\cm___Win\kerm_track.obj" \
	".\cm___Win\cmstring.obj" \
	".\cm___Win\general.obj" \
	".\cm___Win\conn_list.obj" \
	".\cm___Win\conn_props.obj" \
	".\cm___Win\toolbar.obj" \
	".\cm___Win\util.obj" \
	".\cm___Win\new_conn.obj" \
	".\cm___Win\json_config.obj" \
	".\cm___Win\statusbar.obj" \
	".\cm___Win\term_colors.obj" \
	".\cm___Win\login.obj" \
	".\cm___Win\connection.obj" \
	".\cm___Win\connection_serial.obj" \
	".\cm___Win\keyboard.obj" \
	".\cm___Win\cm.res"

"$(OUTDIR)\cm.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cm___Wi0"
# PROP BASE Intermediate_Dir "cm___Wi0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "cm___Wi0"
# PROP Intermediate_Dir "cm___Wi0"
# PROP Target_Dir ""
OUTDIR=.\cm___Wi0
INTDIR=.\cm___Wi0

ALL : "$(OUTDIR)\cm.exe"

CLEAN : 
	-@erase ".\cm___Wi0\cm.exe"
	-@erase ".\cm___Wi0\util.obj"
	-@erase ".\cm___Wi0\terminal.obj"
	-@erase ".\cm___Wi0\json_config.obj"
	-@erase ".\cm___Wi0\term_colors.obj"
	-@erase ".\cm___Wi0\conn_list.obj"
	-@erase ".\cm___Wi0\json_profile.obj"
	-@erase ".\cm___Wi0\conn_profile.obj"
	-@erase ".\cm___Wi0\cmstring.obj"
	-@erase ".\cm___Wi0\term_info.obj"
	-@erase ".\cm___Wi0\main.obj"
	-@erase ".\cm___Wi0\cJSON.obj"
	-@erase ".\cm___Wi0\statusbar.obj"
	-@erase ".\cm___Wi0\conn_props.obj"
	-@erase ".\cm___Wi0\new_conn.obj"
	-@erase ".\cm___Wi0\general.obj"
	-@erase ".\cm___Wi0\charset.obj"
	-@erase ".\cm___Wi0\toolbar.obj"
	-@erase ".\cm___Wi0\kerm_track.obj"
	-@erase ".\cm___Wi0\cm.res"
	-@erase ".\cm___Wi0\login.obj"
	-@erase ".\cm___Wi0\connection.obj"
	-@erase ".\cm___Wi0\connection_serial.obj"
	-@erase ".\cm___Wi0\keyboard.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)/cm.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\cm___Wi0/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/cm.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/cm.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib Version.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib comctl32.lib Version.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/cm.pdb" /machine:I386 /out:"$(OUTDIR)/cm.exe" 
LINK32_OBJS= \
	".\cm___Wi0\util.obj" \
	".\cm___Wi0\terminal.obj" \
	".\cm___Wi0\json_config.obj" \
	".\cm___Wi0\term_colors.obj" \
	".\cm___Wi0\conn_list.obj" \
	".\cm___Wi0\json_profile.obj" \
	".\cm___Wi0\conn_profile.obj" \
	".\cm___Wi0\cmstring.obj" \
	".\cm___Wi0\term_info.obj" \
	".\cm___Wi0\main.obj" \
	".\cm___Wi0\cJSON.obj" \
	".\cm___Wi0\statusbar.obj" \
	".\cm___Wi0\conn_props.obj" \
	".\cm___Wi0\new_conn.obj" \
	".\cm___Wi0\general.obj" \
	".\cm___Wi0\charset.obj" \
	".\cm___Wi0\toolbar.obj" \
	".\cm___Wi0\kerm_track.obj" \
	".\cm___Wi0\login.obj" \
	".\cm___Wi0\connection.obj" \
	".\cm___Wi0\connection_serial.obj" \
	".\cm___Wi0\keyboard.obj" \
	".\cm___Wi0\cm.res"

"$(OUTDIR)\cm.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "cm - Win32 Release"
# Name "cm - Win32 Debug"
# Name "cm - Win32 Unicode Debug"
# Name "cm - Win32 Unicode Release"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\util.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\util.cpp"
DEP_CPP_UTIL_=\
	".\..\util.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\toolbar.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\toolbar.cpp"
DEP_CPP_TOOLB=\
	".\..\toolbar.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\toolbar.obj" : $(SOURCE) $(DEP_CPP_TOOLB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\toolbar.obj" : $(SOURCE) $(DEP_CPP_TOOLB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\toolbar.obj" : $(SOURCE) $(DEP_CPP_TOOLB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\toolbar.obj" : $(SOURCE) $(DEP_CPP_TOOLB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\statusbar.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\statusbar.cpp"
DEP_CPP_STATU=\
	".\..\statusbar.h"\
	".\..\kerm_track.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\statusbar.obj" : $(SOURCE) $(DEP_CPP_STATU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\statusbar.obj" : $(SOURCE) $(DEP_CPP_STATU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\statusbar.obj" : $(SOURCE) $(DEP_CPP_STATU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\statusbar.obj" : $(SOURCE) $(DEP_CPP_STATU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\resource.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\new_conn.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\new_conn.cpp"
DEP_CPP_NEW_C=\
	".\..\config_file.h"\
	".\..\conn_list.h"\
	".\..\conn_props.h"\
	".\..\util.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\new_conn.obj" : $(SOURCE) $(DEP_CPP_NEW_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\new_conn.obj" : $(SOURCE) $(DEP_CPP_NEW_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\new_conn.obj" : $(SOURCE) $(DEP_CPP_NEW_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\new_conn.obj" : $(SOURCE) $(DEP_CPP_NEW_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\main.cpp"
DEP_CPP_MAIN_=\
	".\..\toolbar.h"\
	".\..\statusbar.h"\
	".\..\conn_list.h"\
	".\..\conn_props.h"\
	".\..\new_conn.h"\
	".\..\util.h"\
	".\..\ipc_messages.h"\
	".\..\kerm_track.h"\
	".\..\json_config.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	".\..\config_file.h"\
	
NODEP_CPP_MAIN_=\
	".\..\dummy_config.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\kerm_track.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\kerm_track.cpp"
DEP_CPP_KERM_=\
	".\..\kerm_track.h"\
	".\..\ipc_messages.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\kerm_track.obj" : $(SOURCE) $(DEP_CPP_KERM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\kerm_track.obj" : $(SOURCE) $(DEP_CPP_KERM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\kerm_track.obj" : $(SOURCE) $(DEP_CPP_KERM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\kerm_track.obj" : $(SOURCE) $(DEP_CPP_KERM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\json_template.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\json_profile.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\json_profile.cpp"
DEP_CPP_JSON_=\
	".\..\json_profile.h"\
	".\..\cJSON\cJSON.h"\
	".\..\json_config.h"\
	".\..\config_file.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\json_profile.obj" : $(SOURCE) $(DEP_CPP_JSON_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\json_profile.obj" : $(SOURCE) $(DEP_CPP_JSON_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\json_profile.obj" : $(SOURCE) $(DEP_CPP_JSON_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\json_profile.obj" : $(SOURCE) $(DEP_CPP_JSON_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\json_config.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\json_config.cpp"
DEP_CPP_JSON_C=\
	".\..\json_config.h"\
	".\..\cJSON\cJSON.h"\
	".\..\json_profile.h"\
	".\..\config_file.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\json_config.obj" : $(SOURCE) $(DEP_CPP_JSON_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\json_config.obj" : $(SOURCE) $(DEP_CPP_JSON_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\json_config.obj" : $(SOURCE) $(DEP_CPP_JSON_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\json_config.obj" : $(SOURCE) $(DEP_CPP_JSON_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\ipc_messages.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props.cpp"
DEP_CPP_CONN_=\
	".\..\conn_props.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\conn_props.obj" : $(SOURCE) $(DEP_CPP_CONN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\conn_props.obj" : $(SOURCE) $(DEP_CPP_CONN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\conn_props.obj" : $(SOURCE) $(DEP_CPP_CONN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\conn_props.obj" : $(SOURCE) $(DEP_CPP_CONN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_profile.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_profile.cpp"
DEP_CPP_CONN_P=\
	".\..\conn_profile.h"\
	".\..\util.h"\
	".\..\ipc_messages.h"\
	".\..\kerm_track.h"\
	".\..\term_info.h"\
	".\..\charset.h"\
	".\..\cmstring.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\conn_profile.obj" : $(SOURCE) $(DEP_CPP_CONN_P) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\conn_profile.obj" : $(SOURCE) $(DEP_CPP_CONN_P) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\conn_profile.obj" : $(SOURCE) $(DEP_CPP_CONN_P) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\conn_profile.obj" : $(SOURCE) $(DEP_CPP_CONN_P) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_list.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_list.cpp"
DEP_CPP_CONN_L=\
	".\..\conn_list.h"\
	".\..\statusbar.h"\
	".\..\toolbar.h"\
	".\..\kerm_track.h"\
	".\..\util.h"\
	".\..\config_file.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\conn_list.obj" : $(SOURCE) $(DEP_CPP_CONN_L) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\conn_list.obj" : $(SOURCE) $(DEP_CPP_CONN_L) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\conn_list.obj" : $(SOURCE) $(DEP_CPP_CONN_L) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\conn_list.obj" : $(SOURCE) $(DEP_CPP_CONN_L) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\config_file.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\cmstring.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\cmstring.cpp"
DEP_CPP_CMSTR=\
	".\..\cmstring.h"\
	".\..\util.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\cmstring.obj" : $(SOURCE) $(DEP_CPP_CMSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\cmstring.obj" : $(SOURCE) $(DEP_CPP_CMSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\cmstring.obj" : $(SOURCE) $(DEP_CPP_CMSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\cmstring.obj" : $(SOURCE) $(DEP_CPP_CMSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\cm.rc"
DEP_RSC_CM_RC=\
	".\..\toolbar1.bmp"\
	".\..\logo.ico"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\cm.res" : $(SOURCE) $(DEP_RSC_CM_RC) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/cm.res" /i "\ckwin-dial32\kermit\cm\src" /d\
 "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\cm.res" : $(SOURCE) $(DEP_RSC_CM_RC) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/cm.res" /i "\ckwin-dial32\kermit\cm\src" /d\
 "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\cm.res" : $(SOURCE) $(DEP_RSC_CM_RC) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/cm.res" /i "\ckwin-dial32\kermit\cm\src" /d\
 "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\cm.res" : $(SOURCE) $(DEP_RSC_CM_RC) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/cm.res" /i "\ckwin-dial32\kermit\cm\src" /d\
 "NDEBUG" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\cJSON\cJSON.c"
DEP_CPP_CJSON=\
	".\..\cJSON\cJSON.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\cJSON.obj" : $(SOURCE) $(DEP_CPP_CJSON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\cJSON.obj" : $(SOURCE) $(DEP_CPP_CJSON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\cJSON.obj" : $(SOURCE) $(DEP_CPP_CJSON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\cJSON.obj" : $(SOURCE) $(DEP_CPP_CJSON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\cJSON\cJSON.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props\general.cpp"
DEP_CPP_GENER=\
	".\..\conn_props.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\general.obj" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\general.obj" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\general.obj" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\general.obj" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\term_info.cpp"
DEP_CPP_TERM_=\
	".\..\term_info.h"\
	".\..\charset.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\term_info.obj" : $(SOURCE) $(DEP_CPP_TERM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\term_info.obj" : $(SOURCE) $(DEP_CPP_TERM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\term_info.obj" : $(SOURCE) $(DEP_CPP_TERM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\term_info.obj" : $(SOURCE) $(DEP_CPP_TERM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\charset.cpp"
DEP_CPP_CHARS=\
	".\..\charset.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\charset.obj" : $(SOURCE) $(DEP_CPP_CHARS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\charset.obj" : $(SOURCE) $(DEP_CPP_CHARS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\charset.obj" : $(SOURCE) $(DEP_CPP_CHARS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\charset.obj" : $(SOURCE) $(DEP_CPP_CHARS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props\terminal.cpp"
DEP_CPP_TERMI=\
	".\..\conn_props.h"\
	".\..\conn_profile.h"\
	".\..\term_info.h"\
	".\..\charset.h"\
	".\..\cmstring.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\terminal.obj" : $(SOURCE) $(DEP_CPP_TERMI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\terminal.obj" : $(SOURCE) $(DEP_CPP_TERMI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\terminal.obj" : $(SOURCE) $(DEP_CPP_TERMI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\terminal.obj" : $(SOURCE) $(DEP_CPP_TERMI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\term_info.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\charset.h"

!IF  "$(CFG)" == "cm - Win32 Release"

!ELSEIF  "$(CFG)" == "cm - Win32 Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props\term_colors.cpp"
DEP_CPP_TERM_C=\
	".\..\conn_props.h"\
	".\..\conn_profile.h"\
	".\..\term_info.h"\
	".\..\charset.h"\
	".\..\cmstring.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\term_colors.obj" : $(SOURCE) $(DEP_CPP_TERM_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\term_colors.obj" : $(SOURCE) $(DEP_CPP_TERM_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\term_colors.obj" : $(SOURCE) $(DEP_CPP_TERM_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\term_colors.obj" : $(SOURCE) $(DEP_CPP_TERM_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props\login.cpp"
DEP_CPP_LOGIN=\
	".\..\conn_props.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\login.obj" : $(SOURCE) $(DEP_CPP_LOGIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\login.obj" : $(SOURCE) $(DEP_CPP_LOGIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\login.obj" : $(SOURCE) $(DEP_CPP_LOGIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\login.obj" : $(SOURCE) $(DEP_CPP_LOGIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props\connection.cpp"
DEP_CPP_CONNE=\
	".\..\conn_props.h"\
	".\..\conn_profile.h"\
	".\..\term_info.h"\
	".\..\charset.h"\
	".\..\cmstring.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\connection.obj" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\connection.obj" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\connection.obj" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\connection.obj" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props\connection_serial.cpp"
DEP_CPP_CONNEC=\
	".\..\conn_props.h"\
	".\..\conn_profile.h"\
	".\..\util.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\connection_serial.obj" : $(SOURCE) $(DEP_CPP_CONNEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\connection_serial.obj" : $(SOURCE) $(DEP_CPP_CONNEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\connection_serial.obj" : $(SOURCE) $(DEP_CPP_CONNEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\connection_serial.obj" : $(SOURCE) $(DEP_CPP_CONNEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE="\ckwin-dial32\kermit\cm\src\conn_props\keyboard.cpp"
DEP_CPP_KEYBO=\
	".\..\conn_props.h"\
	".\..\conn_profile.h"\
	".\..\cmstring.h"\
	".\..\charset.h"\
	".\..\term_info.h"\
	

!IF  "$(CFG)" == "cm - Win32 Release"


"$(INTDIR)\keyboard.obj" : $(SOURCE) $(DEP_CPP_KEYBO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Debug"


"$(INTDIR)\keyboard.obj" : $(SOURCE) $(DEP_CPP_KEYBO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"


"$(INTDIR)\keyboard.obj" : $(SOURCE) $(DEP_CPP_KEYBO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"


"$(INTDIR)\keyboard.obj" : $(SOURCE) $(DEP_CPP_KEYBO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
