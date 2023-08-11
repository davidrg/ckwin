# INTERPOL tutorial makefile

#    nmake -f mvcpp600.mak dos32       (makes all the DOS TNT extender intrpol tutorials)
#    nmake -f mvcpp600.mak winnt       (makes all the Windows intrpol tutorials)
#    nmake -f mvcpp600.mak win32       (makes all the 32 bit Windows intrpol tutorials)

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
WNT_LIB_OPTS=/machine:MIPS /subsystem:WINDOWS

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

W32_CPP_OPTS= -MT -c -W3 -D_X86_=1 -DWIN32 -DZIL_WIN32
W32_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
W32_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup
W32_LIB_OPTS=/machine:MIPS /subsystem:WINDOWS

W32_OBJS=
W32_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib w32_zil.lib oldnames.lib ctl3d32.lib
W32_CON_LIBS=libcmt.lib kernel32.lib w32_zil.lib oldnames.lib

.cpp.ob9:
	$(W32_CPP) $(W32_CPP_OPTS) -Fo$*.ob9 $<

# ----- Usage --------------------------------------------------------------
usage:
	@echo ...........
	@echo ...........
	@echo To make this Zinc tutorial for Visual C++ type:
	@echo nmake -f mvcpp600.mak dos32
	@echo nmake -f mvcpp600.mak winnt
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean

# ----- DOS 32 bit TNT Extender ---------------------------------------------
dos32: intrp132.exe intrp232.exe

intrp132.exe: intrpol1.o32 ipolwin1.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:intrp132.exe $(D32_OBJS) intrpol1.o32 ipolwin1.o32 $(D32_LIBS)

intrp232.exe: intrpol2.o32 ipolwin2.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:intrp232.exe $(D32_OBJS) intrpol2.o32 ipolwin2.o32 $(D32_LIBS)

# ----- Windows NT ----------------------------------------------------------
winnt: nintrpo1.exe nintrpo2.exe

nintrpo1.exe: intrpol1.obn ipolwin1.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nintrpo1.exe $(WNT_OBJS) intrpol1.obn ipolwin1.obn $(WNT_LIBS)

nintrpo2.exe: intrpol2.obn ipolwin2.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nintrpo2.exe $(WNT_OBJS) intrpol2.obn ipolwin2.obn $(WNT_LIBS)

# ----- 32 bit Windows ------------------------------------------------------
win32: 9intrpo1.exe 9intrpo2.exe

9intrpo1.exe: intrpol1.ob9 ipolwin1.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9intrpo1.exe $(W32_OBJS) intrpol1.ob9 ipolwin1.ob9 $(W32_LIBS)

9intrpo2.exe: intrpol2.ob9 ipolwin2.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9intrpo2.exe $(W32_OBJS) intrpol2.ob9 ipolwin2.ob9 $(W32_LIBS)


