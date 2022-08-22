# ----- General Definitions -------------------------------------------------
VERSION=mvcpp400mt

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
	@echo nmake -f $(VERSION).mak win32
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean.bat

# ----- 32 bit DOS Extender Libraries and Programs --------------------------
dos32: imgedt32.exe
	copy imgedt32.exe ..\..\bin

imgedt32.exe: main.o32 image32.lib
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:imgedt32.exe $(D32_OBJS) main.o32 image32.lib stredi32.lib direct32.lib storag32.lib servic32.lib $(D32_LIBS)

image32.lib : odib.o32 odib1.o32 odib2.o32 \
	wdib.o32 wdib1.o32 wdib2.o32 \
	wico.o32 wico1.o32 wico2.o32 \
	xpm.o32 xpm1.o32 xpm2.o32 \
	znc1.o32 znc2.o32 \
	image.o32 image1.o32 image2.o32 color.o32 draw.o32 \
	export.o32 import.o32 \
	d_image.o32
	-@del image32.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
odib.o32 odib1.o32 odib2.o32
wdib.o32 wdib1.o32 wdib2.o32
wico.o32 wico1.o32 wico2.o32
xpm.o32 xpm1.o32 xpm2.o32
znc1.o32 znc2.o32
image.o32 image1.o32 image2.o32 color.o32 draw.o32
export.o32 import.o32
d_image.o32
<<
	copy image.hpp ..\..\include
	copy image32.lib ..\..\lib\$(VERSION)
	copy p_image.dat ..\..\bin\p_image.znc

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: nimgedit.exe
	copy nimgedit.exe ..\..\bin

nimgedit.exe: main.obn nimage.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nimgedit.exe $(WNT_OBJS) main.obn nimage.lib nstredit.lib nstorage.lib ndirect.lib nservice.lib $(WNT_LIBS)

nimage.lib : odib.obn odib1.obn odib2.obn \
	wdib.obn wdib1.obn wdib2.obn \
	wico.obn wico1.obn wico2.obn \
	xpm.obn xpm1.obn xpm2.obn \
	znc1.obn znc2.obn \
	image.obn image1.obn image2.obn color.obn draw.obn \
	export.obn import.obn \
	w_image.obn
	-@del nimage.lib
	$(WNT_LIBRARIAN) /out:nimage.lib $(WNT_LIB_OPTS) @<<zil.rsp
odib.obn
odib1.obn
odib2.obn
wdib.obn
wdib1.obn
wdib2.obn
wico.obn
wico1.obn
wico2.obn
xpm.obn
xpm1.obn
xpm2.obn
znc1.obn
znc2.obn
image.obn
image1.obn
image2.obn
color.obn
draw.obn
w_image.obn
export.obn
import.obn
<<
	copy image.hpp ..\..\include
	copy nimage.lib ..\..\lib\$(VERSION)
	copy p_image.dat ..\..\bin\p_image.znc

# ----- Windows 32 bit Libraries and Programs -------------------------------
win32: 9imgedit.exe
	copy 9imgedit.exe ..\..\bin

9imgedit.exe: main.ob9 9image.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9imgedit.exe $(W32_OBJS) main.ob9 9image.lib 9stredit.lib 9storage.lib 9direct.lib 9service.lib $(W32_LIBS)

9image.lib : odib.ob9 odib1.ob9 odib2.ob9 \
	wdib.ob9 wdib1.ob9 wdib2.ob9 \
	wico.ob9 wico1.ob9 wico2.ob9 \
	xpm.ob9 xpm1.ob9 xpm2.ob9 \
	znc1.ob9 znc2.ob9 \
	image.ob9 image1.ob9 image2.ob9 color.ob9 draw.ob9 \
	export.ob9 import.ob9 \
	w_image.ob9
	-@del 9image.lib
	$(W32_LIBRARIAN) /out:9image.lib $(W32_LIB_OPTS) @<<zil.rsp
odib.ob9
odib1.ob9
odib2.ob9
wdib.ob9
wdib1.ob9
wdib2.ob9
wico.ob9
wico1.ob9
wico2.ob9
xpm.ob9
xpm1.ob9
xpm2.ob9
znc1.ob9
znc2.ob9
image.ob9
image1.ob9
image2.ob9
color.ob9
draw.ob9
w_image.ob9
export.ob9
import.ob9
<<
	copy image.hpp ..\..\include
	copy 9image.lib ..\..\lib\$(VERSION)
	copy p_image.dat ..\..\bin\p_image.znc
