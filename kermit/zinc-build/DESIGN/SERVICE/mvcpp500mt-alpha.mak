# ----- General Definitions -------------------------------------------------
VERSION=mvcpp500mt-alpha

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

W32_CPP_OPTS= -MT -c -W3 -DWIN32 -DZIL_WIN32
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
	@echo nmake -f $(VERSION).mak win32
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean.bat

# ----- 32 bit DOS Extender Libraries and Programs --------------------------
dos32: test32.exe

test32.exe: main.o32 servic32.lib
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:test32.exe $(D32_OBJS) main.o32 servic32.lib $(D32_LIBS)

servic32.lib : service.o32
	-@del servic32.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
   service.o32
<<
	copy service.hpp ..\..\include
	copy servic32.lib ..\..\lib\$(VERSION)
	copy p_servic.dat ..\..\bin\p_servic.znc

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: ntest.exe

ntest.exe: main.obn nservice.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:ntest.exe $(WNT_OBJS) main.obn nservice.lib $(WNT_LIBS)

nservice.lib : service.obn
	-@del nservice.lib
	$(WNT_LIBRARIAN) /out:nservice.lib $(WNT_LIB_OPTS) @<<zil.rsp
service.obn
<<
	copy service.hpp ..\..\include
	copy nservice.lib ..\..\lib\$(VERSION)
	copy p_servic.dat ..\..\bin\p_servic.znc

# ----- Windows 32 bit Libraries and Programs -------------------------------
win32: 9test.exe

9test.exe: main.ob9 9service.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9test.exe $(W32_OBJS) main.ob9 9service.lib $(W32_LIBS)

9service.lib : service.ob9
	-@del 9service.lib
	$(W32_LIBRARIAN) /out:9service.lib $(W32_LIB_OPTS) @<<zil.rsp
service.ob9
<<
	copy service.hpp ..\..\include
	copy 9service.lib ..\..\lib\$(VERSION)
	copy p_servic.dat ..\..\bin\p_servic.znc
