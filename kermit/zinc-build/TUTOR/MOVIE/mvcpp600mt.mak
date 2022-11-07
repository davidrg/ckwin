# MOVIE tutorial makefile

#    nmake -f mvcpp600.mak dos32       (makes all the DOS TNT extender movie tutorials)
#    nmake -f mvcpp600.mak winnt       (makes all the Windows movie tutorials)
#    nmake -f mvcpp600.mak win32       (makes all the Windows movie tutorials)

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

# ----- Windows 32 bit compiler options -------------------------------------
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
dos32: movie32.exe movie132.exe movie232.exe movie332.exe movie432.exe movie532.exe movie632.exe movie732.exe

movie32.exe: movie.o32 p_movie.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:movie32.exe $(D32_OBJS) movie.o32 p_movie.o32 $(D32_LIBS)

movie132.exe: movie1.o32 p_movie1.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:movie132.exe $(D32_OBJS) movie1.o32 p_movie1.o32 $(D32_LIBS)

movie232.exe: movie2.o32 p_movie2.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:movie232.exe $(D32_OBJS) movie2.o32 p_movie2.o32 $(D32_LIBS)

movie332.exe: movie3.o32 p_movie3.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:movie332.exe $(D32_OBJS) movie3.o32 p_movie3.o32 $(D32_LIBS)

movie432.exe: movie4.o32 p_movie4.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:movie432.exe $(D32_OBJS) movie4.o32 p_movie4.o32 $(D32_LIBS)

movie532.exe: movie5.o32 p_movie5.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:movie532.exe $(D32_OBJS) movie5.o32 p_movie5.o32 $(D32_LIBS)

movie632.exe: movie6.o32 p_movie6.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:movie632.exe $(D32_OBJS) movie6.o32 p_movie6.o32 $(D32_LIBS)

movie732.exe: movie7.o32 p_movie7.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:movie732.exe $(D32_OBJS) movie7.o32 p_movie7.o32 $(D32_LIBS)

# ----- Windows NT ----------------------------------------------------------
winnt: nmovie.exe nmovie1.exe nmovie2.exe nmovie3.exe nmovie4.exe nmovie5.exe nmovie6.exe nmovie7.exe

nmovie.exe: movie.obn p_movie.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmovie.exe $(WNT_OBJS) movie.obn p_movie.obn $(WNT_LIBS)

nmovie1.exe: movie1.obn p_movie1.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmovie1.exe $(WNT_OBJS) movie1.obn p_movie1.obn $(WNT_LIBS)

nmovie2.exe: movie2.obn p_movie2.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmovie2.exe $(WNT_OBJS) movie2.obn p_movie2.obn $(WNT_LIBS)

nmovie3.exe: movie3.obn p_movie3.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmovie3.exe $(WNT_OBJS) movie3.obn p_movie3.obn $(WNT_LIBS)

nmovie4.exe: movie4.obn p_movie4.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmovie4.exe $(WNT_OBJS) movie4.obn p_movie4.obn $(WNT_LIBS)

nmovie5.exe: movie5.obn p_movie5.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmovie5.exe $(WNT_OBJS) movie5.obn p_movie5.obn $(WNT_LIBS)

nmovie6.exe: movie6.obn p_movie6.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmovie6.exe $(WNT_OBJS) movie6.obn p_movie6.obn $(WNT_LIBS)

nmovie7.exe: movie7.obn p_movie7.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmovie7.exe $(WNT_OBJS) movie7.obn p_movie7.obn $(WNT_LIBS)

# ----- 32 bit Windows ------------------------------------------------------
win32: 9movie.exe 9movie1.exe 9movie2.exe 9movie3.exe 9movie4.exe 9movie5.exe 9movie6.exe 9movie7.exe

9movie.exe: movie.ob9 p_movie.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9movie.exe $(W32_OBJS) movie.ob9 p_movie.ob9 $(W32_LIBS)

9movie1.exe: movie1.ob9 p_movie1.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9movie1.exe $(W32_OBJS) movie1.ob9 p_movie1.ob9 $(W32_LIBS)

9movie2.exe: movie2.ob9 p_movie2.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9movie2.exe $(W32_OBJS) movie2.ob9 p_movie2.ob9 $(W32_LIBS)

9movie3.exe: movie3.ob9 p_movie3.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9movie3.exe $(W32_OBJS) movie3.ob9 p_movie3.ob9 $(W32_LIBS)

9movie4.exe: movie4.ob9 p_movie4.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9movie4.exe $(W32_OBJS) movie4.ob9 p_movie4.ob9 $(W32_LIBS)

9movie5.exe: movie5.ob9 p_movie5.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9movie5.exe $(W32_OBJS) movie5.ob9 p_movie5.ob9 $(W32_LIBS)

9movie6.exe: movie6.ob9 p_movie6.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9movie6.exe $(W32_OBJS) movie6.ob9 p_movie6.ob9 $(W32_LIBS)

9movie7.exe: movie7.ob9 p_movie7.ob9
	$(W32_LINK) $(W32_LINK_OPTS) -out:9movie7.exe $(W32_OBJS) movie7.ob9 p_movie7.ob9 $(W32_LIBS)

