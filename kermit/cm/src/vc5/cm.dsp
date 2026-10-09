# Microsoft Developer Studio Project File - Name="cm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=cm - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
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

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
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
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /machine:I386

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
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Unicode Debug"
# PROP BASE Intermediate_Dir "Unicode Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Unicode Debug"
# PROP Intermediate_Dir "Unicode Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "cm - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unicode Release"
# PROP BASE Intermediate_Dir "Unicode Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode Release"
# PROP Intermediate_Dir "Unicode Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "cm - Win32 Release"
# Name "cm - Win32 Debug"
# Name "cm - Win32 Unicode Debug"
# Name "cm - Win32 Unicode Release"
# Begin Group "Connection Properties"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\conn_props.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props.h
# End Source File
# Begin Source File

SOURCE=..\conn_props\connection.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\connection_serial.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\general.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\gui.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\gui_colors.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\keyboard.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\logging.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\login.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\printer.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\term_colors.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\terminal.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_props\transfer.cpp
# End Source File
# End Group
# Begin Group "cJSON"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\cJSON\cJSON.c
# End Source File
# Begin Source File

SOURCE=..\cJSON\cJSON.h
# End Source File
# Begin Source File

SOURCE=..\cJSON\cJSON_Utils.c
# End Source File
# Begin Source File

SOURCE=..\cJSON\cJSON_Utils.h
# End Source File
# End Group
# Begin Group "storage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\color_theme.h
# End Source File
# Begin Source File

SOURCE=..\config_file.cpp
# End Source File
# Begin Source File

SOURCE=..\config_file.h
# End Source File
# Begin Source File

SOURCE=..\conn_profile.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_profile.h
# End Source File
# Begin Source File

SOURCE=..\json_color_theme.cpp
# End Source File
# Begin Source File

SOURCE=..\json_color_theme.h
# End Source File
# Begin Source File

SOURCE=..\json_config.cpp
# End Source File
# Begin Source File

SOURCE=..\json_config.h
# End Source File
# Begin Source File

SOURCE=..\json_profile.cpp
# End Source File
# Begin Source File

SOURCE=..\json_profile.h
# End Source File
# Begin Source File

SOURCE=..\json_template.h
# End Source File
# Begin Source File

SOURCE=..\json_util.cpp
# End Source File
# Begin Source File

SOURCE=..\json_util.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\charset.cpp
# End Source File
# Begin Source File

SOURCE=..\charset.h
# End Source File
# Begin Source File

SOURCE=..\cm.ico
# End Source File
# Begin Source File

SOURCE=..\cm.rc
# End Source File
# Begin Source File

SOURCE=..\cmstring.cpp
# End Source File
# Begin Source File

SOURCE=..\cmstring.h
# End Source File
# Begin Source File

SOURCE=..\conn_list.cpp
# End Source File
# Begin Source File

SOURCE=..\conn_list.h
# End Source File
# Begin Source File

SOURCE=..\ipc_messages.h
# End Source File
# Begin Source File

SOURCE=..\jumplist.cpp
# End Source File
# Begin Source File

SOURCE=..\jumplist.h
# End Source File
# Begin Source File

SOURCE=..\kerm_track.cpp
# End Source File
# Begin Source File

SOURCE=..\kerm_track.h
# End Source File
# Begin Source File

SOURCE=..\logo.ico
# End Source File
# Begin Source File

SOURCE=..\main.cpp
# End Source File
# Begin Source File

SOURCE=..\new_conn.cpp
# End Source File
# Begin Source File

SOURCE=..\new_conn.h
# End Source File
# Begin Source File

SOURCE=..\resource.h
# End Source File
# Begin Source File

SOURCE=..\statusbar.cpp
# End Source File
# Begin Source File

SOURCE=..\statusbar.h
# End Source File
# Begin Source File

SOURCE=..\term_info.cpp
# End Source File
# Begin Source File

SOURCE=..\term_info.h
# End Source File
# Begin Source File

SOURCE=..\terminal.ico
# End Source File
# Begin Source File

SOURCE=..\terminal_conn.ico
# End Source File
# Begin Source File

SOURCE=..\toolbar.cpp
# End Source File
# Begin Source File

SOURCE=..\toolbar.h
# End Source File
# Begin Source File

SOURCE=..\util.cpp
# End Source File
# Begin Source File

SOURCE=..\util.h
# End Source File
# End Target
# End Project
