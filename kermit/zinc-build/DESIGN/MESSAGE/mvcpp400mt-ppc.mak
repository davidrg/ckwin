# ----- General Definitions -------------------------------------------------
VERSION=mvcpp400mt-ppc

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
WNT_LIB_OPTS=/machine:PPC /subsystem:WINDOWS

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
W32_LIB_OPTS=/machine:PPC /subsystem:WINDOWS

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
dos32: msgedt32.exe
	copy msgedt32.exe ..\..\bin

msgedt32.exe: main.o32 messag32.lib
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:i18edt32.exe $(D32_OBJS) main.o32 messag32.lib storag32.lib direct32.lib stredi32.lib servic32.lib $(D32_LIBS)

messag32.lib : message.o32 message1.o32 prefer.o32 \
	import.o32 export.o32
	-@del messag32.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
message.o32 message1.o32 prefer.o32
import.o32 export.o32
<<
	copy message.hpp ..\..\include
	copy messag32.lib ..\..\lib\$(VERSION)
	copy p_messag.dat ..\..\bin\p_messag.znc

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: nmsgedit.exe
	copy nmsgedit.exe ..\..\bin

nmsgedit.exe: main.obn nmessage.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nmsgedit.exe $(WNT_OBJS) main.obn nmessage.lib nstredit.lib nstorage.lib ndirect.lib nservice.lib $(WNT_LIBS)

nmessage.lib : message.obn message1.obn prefer.obn \
	import.obn export.obn
	-@del nmessage.lib
	$(WNT_LIBRARIAN) /out:nmessage.lib $(WNT_LIB_OPTS) @<<zil.rsp
message.obn
message1.obn
prefer.obn
import.obn
export.obn
<<
	copy message.hpp ..\..\include
	copy nmessage.lib ..\..\lib\$(VERSION)
	copy p_messag.dat ..\..\bin\p_messag.znc

# ----- Windows 32 bit Libraries and Programs -------------------------------
win32: 9msgedit.exe
	copy 9msgedit.exe ..\..\bin

9msgedit.exe: main.ob9 9message.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9msgedit.exe $(W32_OBJS) main.ob9 9message.lib 9stredit.lib 9storage.lib 9direct.lib 9service.lib $(W32_LIBS)

9message.lib : message.ob9 message1.ob9 prefer.ob9 \
	import.ob9 export.ob9
	-@del 9message.lib
	$(W32_LIBRARIAN) /out:9message.lib $(W32_LIB_OPTS) @<<zil.rsp
message.ob9
message1.ob9
prefer.ob9
import.ob9
export.ob9
<<
	copy message.hpp ..\..\include
	copy 9message.lib ..\..\lib\$(VERSION)
	copy p_messag.dat ..\..\bin\p_messag.znc
