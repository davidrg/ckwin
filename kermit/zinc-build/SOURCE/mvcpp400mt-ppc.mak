# ----- General Definitions -------------------------------------------------
VERSION=mvcpp400mt-ppc

D32HARDWAREDEP=i_mscat.o32
D32HARDWARELIB=i_mscat.o32

.SUFFIXES : .c .cpp .o32 .obn

# ----- 32 bit Dos Extender compiler options --------------------------------
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
WNT_LIB_OPTS=/machine:PPC /subsystem:WINDOWS

WNT_OBJS=
WNT_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib wnt_zil.lib oldnames.lib ctl3d32.lib
WNT_CON_LIBS=libcmt.lib kernel32.lib wnt_zil.lib oldnames.lib

.cpp.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<

# ----- 32 bit Windows compiler options -------------------------------------
# for debug:    add /Zi to CPP_OPTS
#               add /DEBUG:MAPPED,FULL /DEBUGTYPE:CV to LINK_OPTS
W32_CPP=cl
W32_LINK=link
W32_LIBRARIAN=lib

W32_CPP_OPTS= -MT -c -W3 -D_X86_=1 -DWIN32 -DZIL_WIN32
W32_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
W32_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup
W32_LIB_OPTS=/machine:PPC /subsystem:WINDOWS

W32_OBJS=
W32_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib w32_zil.lib oldnames.lib ctl3d32.lib
W32_CON_LIBS=libcmt.lib kernel32.lib w32_zil.lib oldnames.lib

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

# ----- Copy files ----------------------------------------------------------
copy:
	copy ui_*.hpp ..\include
	copy z_clean.* ..\bin

commCopy:
	copy z_comctl.h ..\include

gfx_copy:
	copy gfx\source\gfx.h ..\include
	copy gfx\source\gfx_pro.h ..\include

# ----- 32 bit DOS Extender Libraries and Programs --------------------------
dos32: copy gfx_copy d32_msc.lib d32_gfx.lib ms_32gfx.lib d32_zil.lib test32.exe

test32.exe: test.o32
	$(D32_LINK) @msvc32.lnk $(D32_LINK_OPTS) -out:test32.exe $(D32_OBJS) test.o32 $(D32_LIBS)

d32_zil.lib : \
	d_bnum.o32 \
	d_border.o32 \
	d_button.o32 \
	d_combo.o32 \
	d_cursor.o32 \
	d_date.o32 \
	d_error.o32 \
	d_error1.o32 \
	d_event.o32 \
	d_fmtstr.o32 \
	d_group.o32 \
	d_hlist.o32 \
	d_icon.o32 \
	d_image.o32 \
	d_int.o32 \
	d_intl.o32 \
	d_keybrd.o32 \
	d_max.o32 \
	d_min.o32 \
	d_mouse.o32 \
	d_notebk.o32 \
	d_plldn.o32 \
	d_plldn1.o32 \
	d_popup.o32 \
	d_popup1.o32 \
	d_prompt.o32 \
	d_real.o32 \
	d_sbar.o32 \
	d_scroll.o32 \
	d_spin.o32 \
	d_string.o32 \
	d_sys.o32 \
	d_table.o32 \
	d_table1.o32 \
	d_table2.o32 \
	d_tbar.o32 \
	d_tdsp.o32 \
	d_text.o32 \
	d_time.o32 \
	d_title.o32 \
	d_vlist.o32 \
	d_win.o32 \
	d_win1.o32 \
	d_win2.o32 \
	z_bnum.o32 \
	z_bnum1.o32 \
	z_bnum2.o32 \
	z_border.o32 \
	z_button.o32 \
	z_combo.o32 \
	z_cursor.o32 \
	z_date.o32 \
	z_date1.o32 \
	z_decor.o32 \
	z_device.o32 \
	z_dialog.o32 \
	z_dsp.o32 \
	z_error.o32 \
	z_error1.o32 \
	z_event.o32 \
	z_file.o32 \
	z_fmtstr.o32 \
	z_gmgr.o32 \
	z_gmgr1.o32 \
	z_gmgr2.o32 \
	z_gmgr3.o32 \
	z_group.o32 \
	z_help.o32 \
	z_help1.o32 \
	z_hlist.o32 \
	z_icon.o32 \
	z_image.o32 \
	z_i18n.o32 \
	z_int.o32 \
	z_intl.o32 \
	z_lang.o32 \
	z_list.o32 \
	z_list1.o32 \
	z_locale.o32 \
	z_map1.o32 \
	z_map2.o32 \
	z_max.o32 \
	z_min.o32 \
	z_msgwin.o32 \
	z_notebk.o32 \
	z_path.o32 \
	z_plldn.o32 \
	z_plldn1.o32 \
	z_popup.o32 \
	z_popup1.o32 \
	z_printf.o32 \
	z_prompt.o32 \
	z_real.o32 \
	z_region.o32 \
	z_sbar.o32 \
	z_scanf.o32 \
	z_scroll.o32 \
	z_spin.o32 \
	z_stdarg.o32 \
	z_stored.o32 \
	z_storer.o32 \
	z_storew.o32 \
	z_string.o32 \
	z_sys.o32 \
	z_table.o32 \
	z_table1.o32 \
	z_table2.o32 \
	z_tbar.o32 \
	z_text.o32 \
	z_time.o32 \
	z_time1.o32 \
	z_timer.o32 \
	z_title.o32 \
	z_utils.o32 \
	z_utime.o32 \
	z_utime1.o32 \
	z_vlist.o32 \
	z_win.o32 \
	z_win1.o32 \
	z_win2.o32 \
	z_win3.o32 \
	z_win4.o32 \
	g_dsp.o32 \
	g_event.o32 \
	g_evt.o32 \
	g_gen.o32 \
	g_i18n.o32 \
	g_jump.o32 \
	g_lang.o32 \
	g_lang1.o32 \
	g_loc.o32 \
	g_loc1.o32 \
	g_mach.o32 \
	g_pnorm.o32 \
	g_win.o32 \
	i_file.o32 \
	i_map.o32 \
	i_str1.o32 \
	i_str2.o32 \
	i_str3.o32 \
	i_str4.o32 \
	i_str5.o32 \
	i_type.o32 \
	$(D32HARDWAREDEP)
	-@del d32_zil.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
	d_bnum.o32
	d_border.o32
	d_button.o32
	d_combo.o32
	d_cursor.o32
	d_date.o32
	d_error.o32
	d_error1.o32
	d_event.o32
	d_fmtstr.o32
	d_group.o32
	d_hlist.o32
	d_icon.o32
	d_image.o32
	d_int.o32
	d_intl.o32
	d_keybrd.o32
	d_max.o32
	d_min.o32
	d_mouse.o32
	d_notebk.o32
	d_plldn.o32
	d_plldn1.o32
	d_popup.o32
	d_popup1.o32
	d_prompt.o32
	d_real.o32
	d_sbar.o32
	d_scroll.o32
	d_spin.o32
	d_string.o32
	d_sys.o32
	d_table.o32
	d_table1.o32
	d_table2.o32
	d_tbar.o32
	d_tdsp.o32
	d_text.o32
	d_time.o32
	d_title.o32
	d_vlist.o32
	d_win.o32
	d_win1.o32
	d_win2.o32
	z_bnum.o32
	z_bnum1.o32
	z_bnum2.o32
	z_border.o32
	z_button.o32
	z_combo.o32
	z_cursor.o32
	z_date.o32
	z_date1.o32
	z_decor.o32
	z_device.o32
	z_dialog.o32
	z_dsp.o32
	z_error.o32
	z_error1.o32
	z_event.o32
	z_file.o32
	z_fmtstr.o32
	z_gmgr.o32
	z_gmgr1.o32
	z_gmgr2.o32
	z_gmgr3.o32
	z_group.o32
	z_help.o32
	z_help1.o32
	z_hlist.o32
	z_icon.o32
	z_image.o32
	z_i18n.o32
	z_int.o32
	z_intl.o32
	z_lang.o32
	z_list.o32
	z_list1.o32
	z_locale.o32
	z_map1.o32
	z_map2.o32
	z_max.o32
	z_min.o32
	z_msgwin.o32
	z_notebk.o32
	z_path.o32
	z_plldn.o32
	z_plldn1.o32
	z_popup.o32
	z_popup1.o32
	z_printf.o32
	z_prompt.o32
	z_real.o32
	z_region.o32
	z_sbar.o32
	z_scanf.o32
	z_scroll.o32
	z_spin.o32
	z_stdarg.o32
	z_stored.o32
	z_storer.o32
	z_storew.o32
	z_string.o32
	z_sys.o32
	z_table.o32
	z_table1.o32
	z_table2.o32
	z_tbar.o32
	z_text.o32
	z_time.o32
	z_time1.o32
	z_timer.o32
	z_title.o32
	z_utils.o32
	z_utime.o32
	z_utime1.o32
	z_vlist.o32
	z_win.o32
	z_win1.o32
	z_win2.o32
	z_win3.o32
	z_win4.o32
	g_dsp.o32
	g_event.o32
	g_evt.o32
	g_gen.o32
	g_i18n.o32
	g_jump.o32
	g_lang.o32
	g_lang1.o32
	g_loc.o32
	g_loc1.o32
	g_mach.o32
	g_pnorm.o32
	g_win.o32
	i_file.o32
	i_map.o32
	i_str1.o32
	i_str2.o32
	i_str3.o32
	i_str4.o32
	i_str5.o32
	i_type.o32
	$(D32HARDWARELIB)
<<
	-@md ..\lib\$(VERSION)
	copy d32_zil.lib ..\lib\$(VERSION)

d32_msc.lib : d_mscdsp.o32 d_mscprn.o32 z_appmsc.o32
	-@del d32_msc.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib d_mscdsp.o32 d_mscprn.o32 z_appmsc.o32
<<
	-@md ..\lib\$(VERSION)
	copy d32_msc.lib ..\lib\$(VERSION)

d32_gfx.lib : d_gfxdsp.o32 d_gfxprn.o32 z_appgfx.o32 \
	OEM_smal.o32 OEM_dial.o32 OEM_syst.o32 \
	ISO_smal.o32 ISO_dial.o32 ISO_syst.o32
       -@del d32_gfx.lib
       $(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib d_gfxdsp.o32 d_gfxprn.o32 z_appgfx.o32
OEM_smal.o32 OEM_dial.o32 OEM_syst.o32
ISO_smal.o32 ISO_dial.o32 ISO_syst.o32
<<
       -@md ..\lib\$(VERSION)
       copy d32_gfx.lib ..\lib\$(VERSION)

ms_32gfx.lib:
       @cd gfx\source
       nmake -fmvcpp200.mak dos32
       @cd ..\..

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: copy ntest.exe

ntest.exe: test.obn wnt_zil.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:ntest.exe $(WNT_OBJS) test.obn $(WNT_LIBS)

wnt_zil.lib : \
	3_bnum.obn \
	3_border.obn \
	3_button.obn \
	3_combo.obn \
	3_cursor.obn \
	3_dsp.obn \
	3_date.obn \
	3_error.obn \
	3_error1.obn \
	3_event.obn \
	3_fmtstr.obn \
	3_group.obn \
	3_hlist.obn \
	3_icon.obn \
	3_image.obn \
	3_int.obn \
	3_intl.obn \
	3_keybrd.obn \
	3_max.obn \
	3_min.obn \
	3_mouse.obn \
	3_notebk.obn \
	3_plldn.obn \
	3_plldn1.obn \
	3_popup.obn \
	3_popup1.obn \
	3_print.obn \
	3_prompt.obn \
	3_real.obn \
	3_sbar.obn \
	3_scroll.obn \
	3_spin.obn \
	3_string.obn \
	3_sys.obn \
	3_table.obn \
	3_table1.obn \
	3_table2.obn \
	3_tbar.obn \
	3_text.obn \
	3_time.obn \
	3_title.obn \
	3_vlist.obn \
	3_win.obn \
	3_win1.obn \
	3_win2.obn \
	z_app.obn \
	z_bnum.obn \
	z_bnum1.obn \
	z_bnum2.obn \
	z_border.obn \
	z_button.obn \
	z_combo.obn \
	z_cursor.obn \
	z_date.obn \
	z_date1.obn \
	z_decor.obn \
	z_device.obn \
	z_dialog.obn \
	z_dsp.obn \
	z_error.obn \
	z_error1.obn \
	z_event.obn \
	z_file.obn \
	z_fmtstr.obn \
	z_gmgr.obn \
	z_gmgr1.obn \
	z_gmgr2.obn \
	z_gmgr3.obn \
	z_group.obn \
	z_help.obn \
	z_help1.obn \
	z_hlist.obn \
	z_icon.obn \
	z_image.obn \
	z_i18n.obn \
	z_int.obn \
	z_intl.obn \
	z_lang.obn \
	z_list.obn \
	z_list1.obn \
	z_locale.obn \
	z_map1.obn \
	z_map2.obn \
	z_max.obn \
	z_min.obn \
	z_msgwin.obn \
	z_notebk.obn \
	z_path.obn \
	z_plldn.obn \
	z_plldn1.obn \
	z_popup.obn \
	z_popup1.obn \
	z_printf.obn \
	z_prompt.obn \
	z_real.obn \
	z_region.obn \
	z_sbar.obn \
	z_scanf.obn \
	z_scroll.obn \
	z_spin.obn \
	z_stdarg.obn \
	z_stored.obn \
	z_storer.obn \
	z_storew.obn \
	z_string.obn \
	z_sys.obn \
	z_table.obn \
	z_table1.obn \
	z_table2.obn \
	z_tbar.obn \
	z_text.obn \
	z_time.obn \
	z_time1.obn \
	z_timer.obn \
	z_title.obn \
	z_utils.obn \
	z_utime.obn \
	z_utime1.obn \
	z_vlist.obn \
	z_win.obn \
	z_win1.obn \
	z_win2.obn \
	z_win3.obn \
	z_win4.obn \
	g_dsp.obn \
	g_event.obn \
	g_evt.obn \
	g_gen.obn \
	g_i18n.obn \
	g_jump.obn \
	g_lang.obn \
	g_lang1.obn \
	g_loc.obn \
	g_loc1.obn \
	g_mach.obn \
	g_pnorm.obn \
	g_win.obn \
	i_file.obn \
	i_map.obn \
	i_str1.obn \
	i_str2.obn \
	i_str3.obn \
	i_str4.obn \
	i_str5.obn \
	i_type.obn
	-@del wnt_zil.lib
	$(WNT_LIBRARIAN) /out:wnt_zil.lib $(WNT_LIB_OPTS) @<<zil.rsp
3_bnum.obn
3_border.obn
3_button.obn
3_combo.obn
3_cursor.obn
3_dsp.obn
3_date.obn
3_error.obn
3_error1.obn
3_event.obn
3_fmtstr.obn
3_group.obn
3_hlist.obn
3_icon.obn
3_image.obn
3_int.obn
3_intl.obn
3_keybrd.obn
3_max.obn
3_min.obn
3_mouse.obn
3_notebk.obn
3_plldn.obn
3_plldn1.obn
3_popup.obn
3_popup1.obn
3_print.obn
3_prompt.obn
3_real.obn
3_sbar.obn
3_scroll.obn
3_spin.obn
3_string.obn
3_sys.obn
3_table.obn
3_table1.obn
3_table2.obn
3_tbar.obn
3_text.obn
3_time.obn
3_title.obn
3_vlist.obn
3_win.obn
3_win1.obn
3_win2.obn
z_app.obn
z_bnum.obn
z_bnum1.obn
z_bnum2.obn
z_border.obn
z_button.obn
z_combo.obn
z_cursor.obn
z_date.obn
z_date1.obn
z_decor.obn
z_device.obn
z_dialog.obn
z_dsp.obn
z_error.obn
z_error1.obn
z_event.obn
z_file.obn
z_fmtstr.obn
z_gmgr.obn
z_gmgr1.obn
z_gmgr2.obn
z_gmgr3.obn
z_group.obn
z_help.obn
z_help1.obn
z_hlist.obn
z_icon.obn
z_image.obn
z_i18n.obn
z_int.obn
z_intl.obn
z_lang.obn
z_list.obn
z_list1.obn
z_locale.obn
z_map1.obn
z_map2.obn
z_max.obn
z_min.obn
z_msgwin.obn
z_notebk.obn
z_path.obn
z_plldn.obn
z_plldn1.obn
z_popup.obn
z_popup1.obn
z_printf.obn
z_prompt.obn
z_real.obn
z_region.obn
z_sbar.obn
z_scanf.obn
z_scroll.obn
z_spin.obn
z_stdarg.obn
z_stored.obn
z_storer.obn
z_storew.obn
z_string.obn
z_sys.obn
z_table.obn
z_table1.obn
z_table2.obn
z_tbar.obn
z_text.obn
z_time.obn
z_time1.obn
z_timer.obn
z_title.obn
z_utils.obn
z_utime.obn
z_utime1.obn
z_vlist.obn
z_win.obn
z_win1.obn
z_win2.obn
z_win3.obn
z_win4.obn
g_dsp.obn
g_event.obn
g_evt.obn
g_gen.obn
g_i18n.obn
g_jump.obn
g_lang.obn
g_lang1.obn
g_loc.obn
g_loc1.obn
g_mach.obn
g_pnorm.obn
g_win.obn
i_file.obn
i_map.obn
i_str1.obn
i_str2.obn
i_str3.obn
i_str4.obn
i_str5.obn
i_type.obn
<<
	-@md ..\lib\$(VERSION)
	copy wnt_zil.lib ..\lib\$(VERSION)

# ----- 32 bit Windows Libraries and Programs -------------------------------
win32: copy commCopy 9test.exe

9test.exe: test.ob9 w32_zil.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9test.exe $(W32_OBJS) test.ob9 $(W32_LIBS)

w32_zil.lib : \
	9_bnum.ob9 \
	9_border.ob9 \
	9_button.ob9 \
	9_combo.ob9 \
	9_cursor.ob9 \
	9_dsp.ob9 \
	9_date.ob9 \
	9_error.ob9 \
	9_error1.ob9 \
	9_event.ob9 \
	9_fmtstr.ob9 \
	9_group.ob9 \
	9_hlist.ob9 \
	9_icon.ob9 \
	9_image.ob9 \
	9_int.ob9 \
	9_intl.ob9 \
	9_keybrd.ob9 \
	9_max.ob9 \
	9_min.ob9 \
	9_mouse.ob9 \
	9_notebk.ob9 \
	9_plldn.ob9 \
	9_plldn1.ob9 \
	9_popup.ob9 \
	9_popup1.ob9 \
	9_print.ob9 \
	9_prompt.ob9 \
	9_real.ob9 \
	9_sbar.ob9 \
	9_scroll.ob9 \
	9_spin.ob9 \
	9_string.ob9 \
	9_sys.ob9 \
	9_table.ob9 \
	9_table1.ob9 \
	9_table2.ob9 \
	9_tbar.ob9 \
	9_text.ob9 \
	9_time.ob9 \
	9_title.ob9 \
	9_vlist.ob9 \
	9_win.ob9 \
	9_win1.ob9 \
	9_win2.ob9 \
	z_app.ob9 \
	z_bnum.ob9 \
	z_bnum1.ob9 \
	z_bnum2.ob9 \
	z_border.ob9 \
	z_button.ob9 \
	z_combo.ob9 \
	z_cursor.ob9 \
	z_date.ob9 \
	z_date1.ob9 \
	z_decor.ob9 \
	z_device.ob9 \
	z_dialog.ob9 \
	z_dsp.ob9 \
	z_error.ob9 \
	z_error1.ob9 \
	z_event.ob9 \
	z_file.ob9 \
	z_fmtstr.ob9 \
	z_gmgr.ob9 \
	z_gmgr1.ob9 \
	z_gmgr2.ob9 \
	z_gmgr3.ob9 \
	z_group.ob9 \
	z_help.ob9 \
	z_help1.ob9 \
	z_hlist.ob9 \
	z_icon.ob9 \
	z_image.ob9 \
	z_i18n.ob9 \
	z_int.ob9 \
	z_intl.ob9 \
	z_lang.ob9 \
	z_list.ob9 \
	z_list1.ob9 \
	z_locale.ob9 \
	z_map1.ob9 \
	z_map2.ob9 \
	z_max.ob9 \
	z_min.ob9 \
	z_msgwin.ob9 \
	z_notebk.ob9 \
	z_path.ob9 \
	z_plldn.ob9 \
	z_plldn1.ob9 \
	z_popup.ob9 \
	z_popup1.ob9 \
	z_printf.ob9 \
	z_prompt.ob9 \
	z_real.ob9 \
	z_region.ob9 \
	z_sbar.ob9 \
	z_scanf.ob9 \
	z_scroll.ob9 \
	z_spin.ob9 \
	z_stdarg.ob9 \
	z_stored.ob9 \
	z_storer.ob9 \
	z_storew.ob9 \
	z_string.ob9 \
	z_sys.ob9 \
	z_table.ob9 \
	z_table1.ob9 \
	z_table2.ob9 \
	z_tbar.ob9 \
	z_text.ob9 \
	z_time.ob9 \
	z_time1.ob9 \
	z_timer.ob9 \
	z_title.ob9 \
	z_utils.ob9 \
	z_utime.ob9 \
	z_utime1.ob9 \
	z_vlist.ob9 \
	z_win.ob9 \
	z_win1.ob9 \
	z_win2.ob9 \
	z_win3.ob9 \
	z_win4.ob9 \
	g_dsp.ob9 \
	g_event.ob9 \
	g_evt.ob9 \
	g_gen.ob9 \
	g_i18n.ob9 \
	g_jump.ob9 \
	g_lang.ob9 \
	g_lang1.ob9 \
	g_loc.ob9 \
	g_loc1.ob9 \
	g_mach.ob9 \
	g_pnorm.ob9 \
	g_win.ob9 \
	i_file.ob9 \
	i_map.ob9 \
	i_str1.ob9 \
	i_str2.ob9 \
	i_str3.ob9 \
	i_str4.ob9 \
	i_str5.ob9 \
	i_type.ob9
	-@del w32_zil.lib
	$(W32_LIBRARIAN) /out:w32_zil.lib $(W32_LIB_OPTS) @<<zil.rsp
9_bnum.ob9
9_border.ob9
9_button.ob9
9_combo.ob9
9_cursor.ob9
9_dsp.ob9
9_date.ob9
9_error.ob9
9_error1.ob9
9_event.ob9
9_fmtstr.ob9
9_group.ob9
9_hlist.ob9
9_icon.ob9
9_image.ob9
9_int.ob9
9_intl.ob9
9_keybrd.ob9
9_max.ob9
9_min.ob9
9_mouse.ob9
9_notebk.ob9
9_plldn.ob9
9_plldn1.ob9
9_popup.ob9
9_popup1.ob9
9_print.ob9
9_prompt.ob9
9_real.ob9
9_sbar.ob9
9_scroll.ob9
9_spin.ob9
9_string.ob9
9_sys.ob9
9_table.ob9
9_table1.ob9
9_table2.ob9
9_tbar.ob9
9_text.ob9
9_time.ob9
9_title.ob9
9_vlist.ob9
9_win.ob9
9_win1.ob9
9_win2.ob9
z_app.ob9
z_bnum.ob9
z_bnum1.ob9
z_bnum2.ob9
z_border.ob9
z_button.ob9
z_combo.ob9
z_cursor.ob9
z_date.ob9
z_date1.ob9
z_decor.ob9
z_device.ob9
z_dialog.ob9
z_dsp.ob9
z_error.ob9
z_error1.ob9
z_event.ob9
z_file.ob9
z_fmtstr.ob9
z_gmgr.ob9
z_gmgr1.ob9
z_gmgr2.ob9
z_gmgr3.ob9
z_group.ob9
z_help.ob9
z_help1.ob9
z_hlist.ob9
z_icon.ob9
z_image.ob9
z_i18n.ob9
z_int.ob9
z_intl.ob9
z_lang.ob9
z_list.ob9
z_list1.ob9
z_locale.ob9
z_map1.ob9
z_map2.ob9
z_max.ob9
z_min.ob9
z_msgwin.ob9
z_notebk.ob9
z_path.ob9
z_plldn.ob9
z_plldn1.ob9
z_popup.ob9
z_popup1.ob9
z_printf.ob9
z_prompt.ob9
z_real.ob9
z_region.ob9
z_sbar.ob9
z_scanf.ob9
z_scroll.ob9
z_spin.ob9
z_stdarg.ob9
z_stored.ob9
z_storer.ob9
z_storew.ob9
z_string.ob9
z_sys.ob9
z_table.ob9
z_table1.ob9
z_table2.ob9
z_tbar.ob9
z_text.ob9
z_time.ob9
z_time1.ob9
z_timer.ob9
z_title.ob9
z_utils.ob9
z_utime.ob9
z_utime1.ob9
z_vlist.ob9
z_win.ob9
z_win1.ob9
z_win2.ob9
z_win3.ob9
z_win4.ob9
g_dsp.ob9
g_event.ob9
g_evt.ob9
g_gen.ob9
g_i18n.ob9
g_jump.ob9
g_lang.ob9
g_lang1.ob9
g_loc.ob9
g_loc1.ob9
g_mach.ob9
g_pnorm.ob9
g_win.ob9
i_file.ob9
i_map.ob9
i_str1.ob9
i_str2.ob9
i_str3.ob9
i_str4.ob9
i_str5.ob9
i_type.ob9
<<
	-@md ..\lib\$(VERSION)
	copy w32_zil.lib ..\lib\$(VERSION)

# --- Done with libraries ---------------------------------------------------

z_appmsc.o32: z_app.cpp
        $(D32_CPP) -DMSC $(D32_CPP_OPTS) -Fo$@ $?

z_appgfx.o32: z_app.cpp
        $(D32_CPP) -DGFX $(D32_CPP_OPTS) -Fo$@ $?

z_apptxt.o32: z_app.cpp
        $(D32_CPP) -DTEXT $(D32_CPP_OPTS) -Fo$@ $?

d_mscprn.o32: d_print.cpp
        $(D32_CPP) -DMSC $(D32_CPP_OPTS) -Fo$@ $?

d_gfxprn.o32: d_print.cpp
        $(D32_CPP) -DGFX $(D32_CPP_OPTS) -Fo$@ $?

OEM_dial.cpp: gfx\OEM_dial.cpp
	copy gfx\OEM_dial.cpp

OEM_smal.cpp: gfx\OEM_smal.cpp
	copy gfx\OEM_smal.cpp

OEM_syst.cpp: gfx\OEM_syst.cpp
	copy gfx\OEM_syst.cpp

ISO_dial.cpp: gfx\ISO_dial.cpp
	copy gfx\ISO_dial.cpp

ISO_smal.cpp: gfx\ISO_smal.cpp
	copy gfx\ISO_smal.cpp

ISO_syst.cpp: gfx\ISO_syst.cpp
	copy gfx\ISO_syst.cpp


