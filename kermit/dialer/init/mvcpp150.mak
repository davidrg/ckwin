# ----- General Definitions -------------------------------------------------
VERSION=mvcpp150

.SUFFIXES : .c .cpp .obj .obw

# ----- Windows compiler options --------------------------------------------
# for debug:    add -Zi to CPP_OPTS
#               and /CO to LINK_OPTS
WIN_CPP=cl
WIN_LINK=link
WIN_LIBRARIAN=lib
WIN_RC=rc

WIN_CPP_OPTS=-c -AL -BATCH -Gsw -Os -nologo
WIN_LINK_OPTS=/NOE /NOD /BATCH /SEGMENTS:256 /ON:N
#WIN_CPP_OPTS=-c -AL -BATCH -Gsw -Os -nologo -Zi
#WIN_LINK_OPTS=/NOE /NOD /BATCH /SEGMENTS:256 /ON:N /CO
WIN_RC_OPTS=-k
WIN_LIB_OPTS=/NOI
WIN_OBJS=
WIN_LIBS=win_zil libw llibcew oldnames 

.cpp.obw:
	$(WIN_CPP) $(WIN_CPP_OPTS) -Fo$*.obw $<

# ----- Usage --------------------------------------------------------------
usage:
	@echo ...........
	@echo ...........
	@echo To generate the library modules for specific environments type:
	@echo nmake -fmvcpp150.mak windows
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean.bat

# ----- Copy files ----------------------------------------------------------
copy:
	copy ui_*.hpp ..\include
	copy z_clean.* ..\bin

gfx_copy:
	copy gfx\source\gfx.h ..\include
	copy gfx\source\gfx_pro.h ..\include


# ----- Windows Libraries and Programs --------------------------------------
windows: k95cinit.exe

k95cinit.exe: main.obw cinit.obw k95cinit.obw
	$(WIN_LINK) $(WIN_LINK_OPTS) @<<zil.rsp
$(WIN_OBJS)+main.obw+cinit.obw+k95cinit.obw
$*,NUL,$(WIN_LIBS),wtest.def
<<
	$(WIN_RC) $(WIN_RC_OPTS) wtest.rc $*.exe


