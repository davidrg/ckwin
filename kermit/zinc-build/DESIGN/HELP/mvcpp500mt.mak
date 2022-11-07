# ----- General Definitions -------------------------------------------------
VERSION=mvcpp500mt

.SUFFIXES : .c .cpp .o32 .obn .ob9

# ----- DOS TNT Extender options --------------------------------------------
# for debug:    add -Zi to CPP_OPTS
#               and -CV to LINK_OPTS
D32_CPP=cl
D32_LINK=link
D32_LIBRARIAN=lib
D32_CPP_OPTS=-c -DDOS386 -nologo
D32_LINK_OPTS=
D32_LIB_OPTS=
D32_OBJS=

# --- Use the next line for UI_GRAPHICS_DISPLAY ---
D32_LIBS=dosx32.lib dos32.lib d32_zil.lib d32_gfx.lib ms_32gfx.lib oldnames.lib
# --- Use the next line for UI_MSC_DISPLAY ---
#D32_LIBS=dosx32.lib dos32.lib tntapi.lib d32_zil.lib d32_msc.lib oldnames.lib

.cpp.o32:
	$(D32_CPP) $(D32_CPP_OPTS) -Fo$*.o32 $<

# ----- Windows NT compiler options -----------------------------------------
# for debug:    add /Zi to CPP_OPTS
#               add /DEBUG:MAPPED,FULL /DEBUGTYPE:CV to LINK_OPTS
WNT_CPP=cl
WNT_LINK=link
WNT_LIBRARIAN=lib

WNT_CPP_OPTS= -c -W3 -D_X86_=1 -DWIN32 -MT
WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
WNT_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup
WNT_LIB_OPTS=/machine:i386 /subsystem:WINDOWS

WNT_OBJS=
WNT_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib wnt_zil.lib oldnames.lib ctl3d32.lib
WNT_CON_LIBS=libcmt.lib kernel32.lib wnt_zil.lib oldnames.lib

.cpp.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<

# ----- Windows 32 bit compiler options -------------------------------------
# for debug:    add /Zi to CPP_OPTS
#               add /DEBUG:MAPPED,FULL /DEBUGTYPE:CV to LINK_OPTS
W32_CPP=cl
W32_LINK=link
W32_LIBRARIAN=lib

W32_CPP_OPTS= -c -W3 -D_X86_=1 -DWIN32 -DZIL_WIN32 -MT
W32_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
W32_LIB_OPTS=/machine:i386 /subsystem:WINDOWS

W32_OBJS=
W32_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib winspool.lib w32_zil.lib oldnames.lib ctl3d32.lib

.cpp.ob9:
	$(W32_CPP) $(W32_CPP_OPTS) -Fo$*.ob9 $<

# ----- Usage --------------------------------------------------------------
usage:
	@echo ...........
	@echo ...........
	@echo To generate the library modules for specific environments type:
	@echo nmake -f $(VERSION).mak dos32
	@echo nmake -f $(VERSION).mak winnt
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean.bat

# ----- 32 bit DOS Extender Libraries and Programs --------------------------
dos32: hlpedt32.exe
	copy hlpedt32.exe ..\..\bin

hlpedt32.exe: main.o32 help32.lib
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:hlpedtt32.exe $(D32_OBJS) main.o32 help32.lib storag32.lib direct32.lib stredi32.lib servic32.lib $(D32_LIBS)

help32.lib : help.o32 help1.o32 import.o32 export.o32
	-@del help32.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
help.o32 help1.o32 import.o32 export.o32
<<
	copy help.hpp ..\..\include
	copy help32.lib ..\..\lib\$(VERSION)
	copy p_help.dat ..\..\bin\p_help.znc

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: nhlpedit.exe
	copy nhlpedit.exe ..\..\bin

nhlpedit.exe: main.obn nhelp.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nhlpedit.exe $(WNT_OBJS) main.obn nhelp.lib nstredit.lib nstorage.lib ndirect.lib nservice.lib $(WNT_LIBS)

nhelp.lib : help.obn help1.obn import.obn export.obn
	-@del whelp.lib
	$(WNT_LIBRARIAN) /out:nhelp.lib $(WNT_LIB_OPTS) @<<zil.rsp
help.obn
help1.obn
import.obn
export.obn
<<
	copy help.hpp ..\..\include
	copy nhelp.lib ..\..\lib\$(VERSION)
	copy p_help.dat ..\..\bin\p_help.znc

# ----- Windows 32 bit Libraries and Programs -------------------------------
win32: 9hlpedit.exe
	copy 9hlpedit.exe ..\..\bin

9hlpedit.exe: main.ob9 9help.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9hlpedit.exe $(W32_OBJS) main.ob9 9help.lib 9stredit.lib 9storage.lib 9direct.lib 9service.lib $(W32_LIBS)

9help.lib : help.ob9 help1.ob9 import.ob9 export.ob9
	-@del whelp.lib
	$(W32_LIBRARIAN) /out:9help.lib $(W32_LIB_OPTS) @<<zil.rsp
help.ob9
help1.ob9
import.ob9
export.ob9
<<
	copy help.hpp ..\..\include
	copy 9help.lib ..\..\lib\$(VERSION)
	copy p_help.dat ..\..\bin\p_help.znc
