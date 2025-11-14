# ----- General Definitions -------------------------------------------------
VERSION=mvcpp400mt-alpha

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

WNT_CPP_OPTS= -MT -c -W3 -DWIN32
WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
WNT_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup
WNT_LIB_OPTS=/machine:ALPHA /subsystem:WINDOWS

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

W32_CPP_OPTS= -MT -c -W3 -DWIN32
W32_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
W32_LIB_OPTS=/machine:ALPHA /subsystem:WINDOWS

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
dos32: stredt32.exe

stredt32.exe: main.o32 stredi32.lib
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:stredt32.exe $(D32_OBJS) main.o32 stredi32.lib servic32.lib $(D32_LIBS)

stredi32.lib : stredit.o32
	-@del stredi32.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
stredit.o32
<<
	copy stredit.hpp ..\..\include
	copy stredi32.lib ..\..\lib\$(VERSION)
	copy p_string.dat ..\..\bin\p_string.znc

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: nstredit.exe

nstredit.exe: main.obn nstredit.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nstredit.exe $(WNT_OBJS) main.obn nstredit.lib nservice.lib $(WNT_LIBS)

nstredit.lib : stredit.obn
	-@del nstredit.lib
	$(WNT_LIBRARIAN) /out:nstredit.lib $(WNT_LIB_OPTS) @<<zil.rsp
stredit.obn
<<
	copy stredit.hpp ..\..\include
	copy nstredit.lib ..\..\lib\$(VERSION)
	copy p_string.dat ..\..\bin\p_string.znc

# ----- Windows 32 bit Libraries and Programs -------------------------------
win32: 9stredit.exe

9stredit.exe: main.ob9 9stredit.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9stredit.exe $(W32_OBJS) main.ob9 9stredit.lib 9service.lib $(W32_LIBS)

9stredit.lib : stredit.ob9
	-@del 9stredit.lib
	$(W32_LIBRARIAN) /out:9stredit.lib $(W32_LIB_OPTS) @<<zil.rsp
stredit.ob9
<<
	copy stredit.hpp ..\..\include
	copy 9stredit.lib ..\..\lib\$(VERSION)
	copy p_string.dat ..\..\bin\p_string.znc
