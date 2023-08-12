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

WNT_CPP_OPTS= -c -W3 -D_X86_=1 -DWIN32 -MT
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

W32_CPP_OPTS= -c -W3 -D_X86_=1 -DWIN32 -DZIL_WIN32 -MT
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
dos32: i18edt32.exe
	copy i18edt32.exe ..\..\bin

i18edt32.exe: main.o32 i18n32.lib
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:i18edt32.exe $(D32_OBJS) main.o32 i18n32.lib storag32.lib direct32.lib stredi32.lib servic32.lib $(D32_LIBS)

i18n32.lib : i18n.o32 language.o32 locale.o32 \
	export.o32 import.o32 \
	z_bnum.o32 z_curr.o32 z_date.o32 z_date1.o32 z_day.o32 z_error.o32 \
	z_help.o32 z_int.o32 z_month.o32 z_msg.o32 z_num.o32 z_real.o32 \
	z_sys.o32 z_time.o32 z_time1.o32 z_time2.o32 z_win.o32
	-@del i18n.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
i18n.o32 language.o32 locale.o32
export.o32 import.o32
z_bnum.o32 z_curr.o32 z_date.o32 z_date1.o32 z_day.o32 z_error.o32
z_help.o32 z_int.o32 z_month.o32 z_msg.o32 z_num.o32 z_real.o32
z_sys.o32 z_time.o32 z_time1.o32 z_time2.o32 z_win.o32
<<
	copy i18n.hpp ..\..\include
	copy i18n32.lib ..\..\lib\$(VERSION)
	copy p_i18n.dat ..\..\bin\p_i18n.znc

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: ni18edit.exe
	copy ni18edit.exe ..\..\bin

ni18edit.exe: main.obn ni18n.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:ni18edit.exe $(WNT_OBJS) main.obn ni18n.lib nstorage.lib ndirect.lib nstredit.lib nservice.lib $(WNT_LIBS)

ni18n.lib : i18n.obn language.obn locale.obn \
	export.obn import.obn \
	z_bnum.obn z_curr.obn z_date.obn z_date1.obn z_day.obn z_error.obn \
	z_help.obn z_int.obn z_month.obn z_msg.obn z_num.obn z_real.obn \
	z_sys.obn z_time.obn z_time1.obn z_time2.obn z_win.obn
	-@del ni18n.lib
	$(WNT_LIBRARIAN) /out:ni18n.lib $(WNT_LIB_OPTS) @<<zil.rsp
i18n.obn
language.obn
locale.obn
export.obn
import.obn
z_bnum.obn
z_curr.obn
z_date.obn
z_date1.obn
z_day.obn
z_error.obn
z_help.obn
z_int.obn
z_month.obn
z_msg.obn
z_num.obn
z_real.obn
z_sys.obn
z_time.obn
z_time1.obn
z_time2.obn
z_win.obn
<<
	copy i18n.hpp ..\..\include
	copy ni18n.lib ..\..\lib\$(VERSION)
	copy p_i18n.dat ..\..\bin\p_i18n.znc

# ----- Windows 32 bit Libraries and Programs -------------------------------
win32: 9i18edit.exe
	copy 9i18edit.exe ..\..\bin

9i18edit.exe: main.ob9 9i18n.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9i18edit.exe $(W32_OBJS) main.ob9 9i18n.lib 9storage.lib 9direct.lib 9stredit.lib 9service.lib $(W32_LIBS)

9i18n.lib : i18n.ob9 language.ob9 locale.ob9 \
	export.ob9 import.ob9 \
	z_bnum.ob9 z_curr.ob9 z_date.ob9 z_date1.ob9 z_day.ob9 z_error.ob9 \
	z_help.ob9 z_int.ob9 z_month.ob9 z_msg.ob9 z_num.ob9 z_real.ob9 \
	z_sys.ob9 z_time.ob9 z_time1.ob9 z_time2.ob9 z_win.ob9
	-@del 9i18n.lib
	$(W32_LIBRARIAN) /out:9i18n.lib $(W32_LIB_OPTS) @<<zil.rsp
i18n.ob9
language.ob9
locale.ob9
export.ob9
import.ob9
z_bnum.ob9
z_curr.ob9
z_date.ob9
z_date1.ob9
z_day.ob9
z_error.ob9
z_help.ob9
z_int.ob9
z_month.ob9
z_msg.ob9
z_num.ob9
z_real.ob9
z_sys.ob9
z_time.ob9
z_time1.ob9
z_time2.ob9
z_win.ob9
<<
	copy i18n.hpp ..\..\include
	copy 9i18n.lib ..\..\lib\$(VERSION)
	copy p_i18n.dat ..\..\bin\p_i18n.znc
