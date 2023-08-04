# DISPLAY tutorial makefile

#	nmake -fmvcpp600.mak dos32     (makes the DOS TNT extender display program)

# Be sure to set the LIB and INCLUDE environment variables for Zinc, e.g.:
#    set INCLUDE=.;C:\ZINC\INCLUDE;C:\MSVC\INCLUDE
#    set LIB=.;C:\ZINC\LIB\mvcpp600;C:\MSVC\LIB

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

# ----- Usage --------------------------------------------------------------
usage:
	@echo ...........
	@echo ...........
	@echo To make this Zinc tutorial for Visual C++ type:
	@echo nmake -fmvcpp600.mak dos32
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean

# ----- DOS 32 bit TNT Extender ---------------------------------------------
dos32: disp32.exe

disp32.exe: display.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:disp32.exe $(D32_OBJS) display.o32 $(D32_LIBS)


