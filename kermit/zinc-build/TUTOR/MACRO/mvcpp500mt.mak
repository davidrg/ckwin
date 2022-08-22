# MACRO tutorial makefile

#    nmake -f mvcpp600.mak dos32       (makes the DOS TNT extender macro tutorial)
#    nmake -f mvcpp600.mak winnt       (makes the Windows NT macro tutorial)
#    nmake -f mvcpp600.mak win32       (makes the Windows NT macro tutorial)

# Be sure to set the LIB and INCLUDE environment variables for Zinc, e.g.:
#    set INCLUDE=.;C:\ZINC\INCLUDE;C:\MSVC\INCLUDE
#    set LIB=.;C:\ZINC\LIB\MVCPP100;C:\MSVC\LIB

# ----- DOS TNT Extender options --------------------------------------------
# for debug:    add -Zi to CPP_OPTS
#               and -CV to LINK_OPTS
D32_CPP=cl
D32_LINK=link
D32_CPP_OPTS=-c -DDOS386 -nologo
D32_LINK_OPTS=
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

WNT_CPP_OPTS= -MT -c -W3 -D_X86_=1 -DWIN32
WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
WNT_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup
WNT_LIB_OPTS=/machine:i386 /subsystem:WINDOWS

WNT_OBJS=
WNT_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib wnt_zil.lib oldnames.lib ctl3d32.lib
WNT_CON_LIBS=libcmt.lib kernel32.lib wnt_zil.lib oldnames.lib

.cpp.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<

# ----- Windows 32 compiler options -----------------------------------------
# for debug:    add /Zi to CPP_OPTS
#               add /DEBUG:MAPPED,FULL /DEBUGTYPE:CV to LINK_OPTS
W32_CPP=cl
W32_LINK=link
W32_LIBRARIAN=lib

W32_CPP_OPTS= -MT -c -W3 -D_X86_=1 -DWIN32 -DZIL_WIN32
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
	@echo To make this Zinc tutorial for Visual C++ type:
	@echo nmake -f mvcpp600.mak dos32
	@echo nmake -f mvcpp600.mak winnt
	@echo nmake -f mvcpp600.mak win32
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean

# ----- DOS 32 bit TNT Extender ---------------------------------------------
dos32: macro32.exe

macro32.exe: macro.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:macro32.exe $(D32_OBJS) macro.o32 $(D32_LIBS)

# ----- Windows NT ----------------------------------------------------------
winnt: nmacro.exe

nmacro.exe: macro.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmacro.exe $(WNT_OBJS) macro.obn $(WNT_LIBS)

# ----- 32 bit Windows ------------------------------------------------------
win32: 9macro.exe

9macro.exe: macro.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9macro.exe $(W32_OBJS) macro.ob9 $(W32_LIBS)


