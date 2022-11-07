# ----- General Definitions -------------------------------------------------
VERSION=mvcpp700mt

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

WNT_CPP_OPTS= -MT -c -W3 -D_X86_=1 -DWIN32
WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
WNT_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup
WNT_LIB_OPTS=/machine:i386 /subsystem:WINDOWS

WNT_OBJS=
WNT_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib wnt_zil.lib oldnames.lib 
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
W32_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib winspool.lib w32_zil.lib oldnames.lib 

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
dos32: winedt32.exe
	copy winedt32.exe ..\..\bin

winedt32.exe: main.o32 window32.lib
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:winedt32.exe $(D32_OBJS) main.o32 window32.lib storag32.lib direct32.lib stredi32.lib servic32.lib $(D32_LIBS)

window32.lib : advanced.o32 derive.o32 geometry.o32 group.o32 object.o32 prefer.o32 \
   window.o32 window1.o32 \
	export.o32 import.o32 position.o32 \
	z_bnum.o32 z_border.o32 z_button.o32 z_combo.o32 \
	z_date.o32 z_fmtstr.o32 z_gmgr.o32 z_group.o32 \
	z_hlist.o32 z_icon.o32 z_int.o32 z_max.o32 z_min.o32 z_notebk.o32 z_obj.o32 \
	z_plldn.o32 z_plldn1.o32 z_popup.o32 z_popup1.o32 z_prompt.o32 \
	z_real.o32 z_scroll.o32 z_sbar.o32 z_spin.o32 z_string.o32 z_sys.o32 \
	z_table.o32 z_tbar.o32 z_text.o32 z_time.o32 z_title.o32 z_vlist.o32 z_win.o32
	-@del window32.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
advanced.o32 derive.o32 geometry.o32 group.o32 object.o32 prefer.o32 window.o32 window1.o32
	export.o32 import.o32 position.o32 
	z_bnum.o32 z_border.o32 z_button.o32 z_combo.o32 
	z_date.o32 z_fmtstr.o32 z_gmgr.o32 z_group.o32 
	z_hlist.o32 z_icon.o32 z_int.o32 z_max.o32 z_min.o32 z_notebk.o32 z_obj.o32 
	z_plldn.o32 z_plldn1.o32 z_popup.o32 z_popup1.o32 z_prompt.o32 
	z_real.o32 z_scroll.o32 z_sbar.o32 z_spin.o32 z_string.o32 z_sys.o32 
	z_table.o32 z_tbar.o32 z_text.o32 z_time.o32 z_title.o32 z_vlist.o32 z_win.o32
<<
	copy window.hpp ..\..\include
	copy window32.lib ..\..\lib\$(VERSION)
	copy p_window.dat ..\..\bin\p_window.znc

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: nwinedit.exe
	copy nwinedit.exe ..\..\bin

nwinedit.exe: main.obn nwindow.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nwinedit.exe $(WNT_OBJS) main.obn nwindow.lib nstorage.lib ndirect.lib nstredit.lib nservice.lib $(WNT_LIBS)

nwindow.lib : advanced.obn derive.obn export.obn \
	geometry.obn group.obn import.obn object.obn \
	position.obn prefer.obn window.obn window1.obn \
	z_bnum.obn z_border.obn z_button.obn z_combo.obn \
	z_date.obn z_fmtstr.obn z_gmgr.obn z_group.obn \
	z_hlist.obn z_icon.obn z_int.obn z_max.obn z_min.obn z_notebk.obn z_obj.obn \
	z_plldn.obn z_plldn1.obn z_popup.obn z_popup1.obn z_prompt.obn \
	z_real.obn z_sbar.obn z_scroll.obn z_spin.obn z_string.obn z_sys.obn \
	z_table.obn z_tbar.obn z_text.obn z_time.obn z_title.obn z_vlist.obn z_win.obn
	-@del nwindow.lib
	$(WNT_LIBRARIAN) /out:nwindow.lib $(WNT_LIB_OPTS) @<<zil.rsp
advanced.obn
derive.obn
export.obn
geometry.obn
group.obn
import.obn
object.obn
position.obn
prefer.obn
window.obn
window1.obn
z_bnum.obn
z_border.obn
z_button.obn
z_combo.obn
z_date.obn
z_fmtstr.obn
z_gmgr.obn
z_group.obn
z_hlist.obn
z_icon.obn
z_int.obn
z_max.obn
z_min.obn
z_notebk.obn
z_obj.obn
z_plldn.obn
z_plldn1.obn
z_popup.obn
z_popup1.obn
z_prompt.obn
z_real.obn
z_sbar.obn
z_scroll.obn
z_spin.obn
z_string.obn
z_sys.obn
z_table.obn
z_tbar.obn
z_text.obn
z_time.obn
z_title.obn
z_vlist.obn
z_win.obn
<<
	copy window.hpp ..\..\include
	copy nwindow.lib ..\..\lib\$(VERSION)
	copy p_window.dat ..\..\bin\p_window.znc

# ----- Windows 32 Libraries and Programs -----------------------------------
win32: 9winedit.exe
	copy 9winedit.exe ..\..\bin

9winedit.exe: main.ob9 9window.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9winedit.exe $(W32_OBJS) main.ob9 9window.lib 9storage.lib 9direct.lib 9stredit.lib 9service.lib $(W32_LIBS)

9window.lib : advanced.ob9 derive.ob9 export.ob9 \
	geometry.ob9 group.ob9 import.ob9 object.ob9 \
	position.ob9 prefer.ob9 window.ob9 window1.ob9 \
	z_bnum.ob9 z_border.ob9 z_button.ob9 z_combo.ob9 \
	z_date.ob9 z_fmtstr.ob9 z_gmgr.ob9 z_group.ob9 \
	z_hlist.ob9 z_icon.ob9 z_int.ob9 z_max.ob9 z_min.ob9 z_notebk.ob9 z_obj.ob9 \
	z_plldn.ob9 z_plldn1.ob9 z_popup.ob9 z_popup1.ob9 z_prompt.ob9 \
	z_real.ob9 z_sbar.ob9 z_scroll.ob9 z_spin.ob9 z_string.ob9 z_sys.ob9 \
	z_table.ob9 z_tbar.ob9 z_text.ob9 z_time.ob9 z_title.ob9 z_vlist.ob9 z_win.ob9
	-@del 9window.lib
	$(W32_LIBRARIAN) /out:9window.lib $(W32_LIB_OPTS) @<<zil.rsp
advanced.ob9
derive.ob9
export.ob9
geometry.ob9
group.ob9
import.ob9
object.ob9
position.ob9
prefer.ob9
window.ob9
window1.ob9
z_bnum.ob9
z_border.ob9
z_button.ob9
z_combo.ob9
z_date.ob9
z_fmtstr.ob9
z_gmgr.ob9
z_group.ob9
z_hlist.ob9
z_icon.ob9
z_int.ob9
z_max.ob9
z_min.ob9
z_notebk.ob9
z_obj.ob9
z_plldn.ob9
z_plldn1.ob9
z_popup.ob9
z_popup1.ob9
z_prompt.ob9
z_real.ob9
z_sbar.ob9
z_scroll.ob9
z_spin.ob9
z_string.ob9
z_sys.ob9
z_table.ob9
z_tbar.ob9
z_text.ob9
z_time.ob9
z_title.ob9
z_vlist.ob9
z_win.ob9
<<
	copy window.hpp ..\..\include
	copy 9window.lib ..\..\lib\$(VERSION)
	copy p_window.dat ..\..\bin\p_window.znc
