# C-Kermit for Windows Makefile for GCC
#
# Requires:
#	- nothing
#
# This makefile is provided to enable CKW developers to work from and test on
# a Linux/UNIX host using GCC and WINE. While this toolchain is regularly
# exercised via CI, the builds themselves are rarely tested.
#
# At the time of writing, Release builds are done with:
#	NT 3.10/3.50		Visual C++ 2.0
#   NT 3.51+/4/2000,9X	Visual C++ 6.0
#   IA64 Windows		Visual C++ 2010
#   Windows XP SP3+		Visual C++ 2019
#	ARM Windows			Visual C++ 2022
#   IBM OS/2 32bit		OpenWatcom 1.9
#
# The following optional dependencies are *NOT* supported when building with
# GCC due to extensive use of cast-as-lvalue:
#	Kerberos
#	SSLDLL (dynamically loading an OpenSSL 0.9.8 DLL)
#	SRP
# GCC has not supported cast-as-lvalue since 4.0 so making these optional
# dependencies in a GCC build will require writing a few hundred typedefs
# for function pointers and moving the cast from the lvalue to the rvalue
# when GetProcAddress is called.

# TODO: dependencies for shared .c files
# TODO: KUI defines & build flags
# TODO: Resource script builds
# TODO: Fix GCC build errors

DEFINES=-DNO_SSL -DNO_SRP -DNDEBUG -DWIN32 -D_WIN32 -D_CONSOLE -D__32BIT__ \
       	-D_WIN32_WINNT=0x0400 -DK95BUILD=K95 -DNT -DWINVER=0x0400 -DOS2 \
       	-DDYNAMIC -DKANJI -DNETCONN -DHADDRLIST -DNPIPE -DOS2MOUSE -DTCPSOCKET \
       	-DRLOGCODE -DZLIB -DNETFILE -DONETERMUPD -DLIBDES -DCRYPT_DLL -DNEWFTP \
       	-DNO_DNS_SRV -DNO_KERBEROS -DNO_SRP -DOPENSSL_097 -DNO_SSL_KRB5 \
       	-DNOCKXYZ -DNOSSH -DCK_HAVE_INTPTR_T

KUIDEFINES=

LDDEBUG=
LDRELEASE=-s
LDFLAGS=$(LDRELEASE) # can flip to LDDEBUG

#-march=i686
CCDEBUG=-g3 -ggdb
CCRELEASE= -O2 -Wno-implicit-function-declaration -w
CFLAGS=-funsigned-char $(CCRELEASE) # can flip to CCDEBUG

LIBS=-lws2_32 -lspoolss -lwinspool -lgdi32 -lwinmm -lversion

O=.obj

.SUFFIXES:

$(OUTDIR)/%$(O): %.c
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) $(DEFINES) -c -o $@ $*.c

$(OUTDIR)/%$(O): %.cxx
	$(CC) $(CC2) $(CFLAGS) $(DEBUG) $(OPT) $(DEFINES) -c -o $@ $*.c

# These are specific to the K95G and KUI targets
KUIOBJS = \
	$(OUTDIR)/kregedit$(O) 	$(OUTDIR)/ksysmets$(O)	$(OUTDIR)/ikui$(O)		\
	$(OUTDIR)/kprogres$(O)	$(OUTDIR)/ikterm$(O)	$(OUTDIR)/ikcmd$(O)		\
	$(OUTDIR)/kuidef$(O)    $(OUTDIR)/karray$(O)	$(OUTDIR)/khwndset$(O)	\
	$(OUTDIR)/kwin$(O)      $(OUTDIR)/kszpopup$(O)	$(OUTDIR)/kflstat$(O)	\
	$(OUTDIR)/kcustdlg$(O)  $(OUTDIR)/kmenu$(O)		$(OUTDIR)/kstatus$(O)	\
	$(OUTDIR)/ktoolbar$(O)  $(OUTDIR)/kscroll$(O)	$(OUTDIR)/kfont$(O)		\
	$(OUTDIR)/kfontdlg$(O)	$(OUTDIR)/kabout$(O)    $(OUTDIR)/kdwnload$(O)	\
	$(OUTDIR)/kuikey$(O)	$(OUTDIR)/kclient$(O)	$(OUTDIR)/kappwin$(O)	\
	$(OUTDIR)/ktermin$(O)	$(OUTDIR)/kui$(O)
    
# These are shared by all targets capable of making connections and doing
# file transfer (K95G, KUI, CKNKER, IKSD, etc)
OBJS =  $(OUTDIR)/ckcmai$(O)  $(OUTDIR)/ckcfns$(O)  $(OUTDIR)/ckcfn2$(O) \
		$(OUTDIR)/ckcfn3$(O)  $(OUTDIR)/ckcnet$(O)  $(OUTDIR)/ckcpro$(O) \
		$(OUTDIR)/ckucmd$(O)  $(OUTDIR)/ckudia$(O)  $(OUTDIR)/ckofio$(O) \
		$(OUTDIR)/ckuscr$(O)  $(OUTDIR)/ckuusr$(O)  $(OUTDIR)/ckuus2$(O) \
		$(OUTDIR)/ckuus3$(O)  $(OUTDIR)/ckuus4$(O)  $(OUTDIR)/ckuus5$(O) \
		$(OUTDIR)/ckuus6$(O)  $(OUTDIR)/ckuus7$(O)  $(OUTDIR)/ckuusx$(O) \
		$(OUTDIR)/ckuusy$(O)  $(OUTDIR)/ckuxla$(O)  $(OUTDIR)/ckclib$(O) \
		$(OUTDIR)/ckctel$(O)  $(OUTDIR)/ckcuni$(O)  $(OUTDIR)/ckcftp$(O) \
		$(OUTDIR)/cknsig$(O)  $(OUTDIR)/cknalm$(O)  $(OUTDIR)/ckntap$(O) \
		$(OUTDIR)/cknwin$(O)  $(OUTDIR)/cknprt$(O)  $(OUTDIR)/cknpty$(O) \
		$(OUTDIR)/ckuath$(O)  $(OUTDIR)/ckoath$(O)  $(OUTDIR)/ck_ssl$(O) \
		$(OUTDIR)/ckossl$(O)  $(OUTDIR)/ckosslc$(O) $(OUTDIR)/ckosftp$(O) \
		$(OUTDIR)/ckozli$(O)  $(OUTDIR)/ckocon$(O)  $(OUTDIR)/ckoco2$(O) \
		$(OUTDIR)/ckoco3$(O)  $(OUTDIR)/ckoco4$(O)  $(OUTDIR)/ckoco5$(O) \
		$(OUTDIR)/ckoetc$(O)  $(OUTDIR)/ckoetc2$(O) $(OUTDIR)/ckokey$(O) \
		$(OUTDIR)/ckomou$(O)  $(OUTDIR)/ckoreg$(O)  $(OUTDIR)/ckonet$(O) \
		$(OUTDIR)/ckoslp$(O)  $(OUTDIR)/ckosyn$(O)  $(OUTDIR)/ckothr$(O) \
		$(OUTDIR)/ckotek$(O)  $(OUTDIR)/ckotio$(O)  $(OUTDIR)/ckowys$(O) \
		$(OUTDIR)/ckodg$(O)   $(OUTDIR)/ckoava$(O)  $(OUTDIR)/ckoi31$(O) \
		$(OUTDIR)/ckotvi$(O)  $(OUTDIR)/ckovc$(O)   $(OUTDIR)/ckoadm$(O) \
		$(OUTDIR)/ckohzl$(O)  $(OUTDIR)/ckohp$(O)   $(OUTDIR)/ckoqnx$(O) \
		$(OUTDIR)/cknnbi$(O)  $(OUTDIR)/ckop$(O)    $(OUTDIR)/p_callbk$(O) \
		$(OUTDIR)/p_global$(O) $(OUTDIR)/p_omalloc$(O) $(OUTDIR)/p_error$(O) \
		$(OUTDIR)/p_common$(O) $(OUTDIR)/p_tl$(O)      $(OUTDIR)/p_dir$(O)

# For SSH-enabled builds
#OBJS = $(OBJS)  ckossh$(O) ckorbf$(O) ckoshs$(O) 

all: console iksdnt
#k95g

clean:
	rm win95-gcc/*
	rm nt-gcc/*
	rm iksdnt-gcc/*

#mkdir -p win95
k95g:
	echo "========== Build GUI =========="
	$(MAKE) w32k95g OUTDIR=win95

#mkdir -p nt
console:
	echo "========== Build Console =========="

	$(MAKE) w32console OUTDIR=nt

#mkdir -p iksdnt
iksdnt:
	echo "========== Build IKSDNT =========="
	$(MAKE) w32iksdnt OUTDIR=iksdnt

w32k95g: $(OUTDIR)/k95g.exe

# k95crypt.dll
w32console: $(OUTDIR)/cknker.exe  $(OUTDIR)/telnet.exe $(OUTDIR)/rlogin.exe \
			$(OUTDIR)/k95d.exe    $(OUTDIR)/textps.exe $(OUTDIR)/ctl3dins.exe \
	        $(OUTDIR)/iksdsvc.exe $(OUTDIR)/iksd.exe   $(OUTDIR)/se.exe

w32iksdnt: $(OUTDIR)/iksdnt.exe

$(OUTDIR)/cknker.exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OUTDIR)/iksdnt.exe: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OUTDIR)/k95g.exe: $(OBJS) $(KUIOBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OUTDIR)/telnet.exe: $(OUTDIR)/telnet$(O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
 
$(OUTDIR)/rlogin.exe: $(OUTDIR)/rlogin$(O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
 
$(OUTDIR)/k95d.exe: $(OUTDIR)/k95d$(O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
 
$(OUTDIR)/textps.exe: $(OUTDIR)/textps$(O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
 
$(OUTDIR)/ctl3dins.exe: $(OUTDIR)/ctl3dins$(O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OUTDIR)/iksdsvc.exe: $(OUTDIR)/iksdsvc$(O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
 
$(OUTDIR)/iksd.exe: $(OUTDIR)/iksd$(O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
 
$(OUTDIR)/se.exe: $(OUTDIR)/se$(O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

# KUI - k95g.exe, cknkui.exe
$(OUTDIR)/kregedit$(O): kui/kregedit.cxx kui/kregedit.hxx
$(OUTDIR)/ksysmets$(O): kui/ksysmets.cxx kui/ksysmets.hxx
$(OUTDIR)/ikui$(O):	 	kui/ikui.cxx kui/kui.hxx kui/ktermin.hxx \
	kui/kflstat.hxx kui/khwndset.hxx kui/kmenu.hxx kui/kflstat.h \
	kui/ikui.h kui/ikcmd.h kui/ikextern.h
$(OUTDIR)/kprogres$(O): kui/kprogres.cxx kui/kprogres.hxx kui/ksysmets.hxx
$(OUTDIR)/ikterm$(O): 	kui/ikterm.cxx kui/ikterm.h kui/ikui.h
$(OUTDIR)/ikcmd$(O): 	kui/ikcmd.c kui/ikextern.h kui/ikcmd.h ckcdeb.h \
	ckcker.h ckcasc.h ckcxla.h ckcuni.h ckcnet.h ckuusr.h ckopcf.h ckocon.h \
	 ckokey.h ckcxxx.h kui/ikextern.h kui/ikcmd.h
$(OUTDIR)/kuidef$(O): 	kui/kuidef.cxx kui/kuidef.h
$(OUTDIR)/karray$(O): 	kui/karray.cxx kui/karray.hxx kui/kuidef.h
$(OUTDIR)/khwndset$(O): kui/khwndset.cxx kui/khwndset.hxx kui/karray.hxx
$(OUTDIR)/kwin$(O): 	kui/kwin.cxx kui/kwin.hxx kui/khwndset.hxx
$(OUTDIR)/kszpopup$(O): kui/kszpopup.cxx kui/kszpopup.hxx kui/kfont.hxx
$(OUTDIR)/kflstat$(O): 	kui/kflstat.cxx kui/kflstat.hxx kui/kflstat.h \
	kui/karray.hxx kui/kprogres.hxx
$(OUTDIR)/kcustdlg$(O): kui/kcustdlg.cxx kui/kcustdlg.hxx kui/kstatus.hxx \
	kui/khwndset.hxx kui/kui.hxx kui/ksysmets.hxx
$(OUTDIR)/kmenu$(O): 	kui/kmenu.cxx kui/kmenu.hxx kui/kwin.hxx \
	kui/ksysmets.hxx
$(OUTDIR)/kstatus$(O): 	kui/kstatus.cxx kui/kstatus.hxx kui/khwndset.hxx \
	kui/ksysmets.hxx kui/kcustdlg.hxx kui/karray.hxx
$(OUTDIR)/ktoolbar$(O): kui/ktoolbar.cxx kui/kui.hxx kui/ktermin.hxx \
	kui/kfont.hxx kui/ktoolbar.hxx kui/khwndset.hxx kui/ikcmd.h kui/ikextern.h
$(OUTDIR)/kscroll$(O): 	kui/kscroll.cxx kui/kscroll.hxx kui/ikextern.h
$(OUTDIR)/kfont$(O):	kui/kfont.cxx kui/kfont.hxx kui/karray.hxx \
	kui/ikextern.h
$(OUTDIR)/kfontdlg$(O): kui/kfontdlg.cxx kui/kfontdlg.hxx kui/khwndset.hxx \
	kui/ikextern.h
$(OUTDIR)/kabout$(O): 	kui/kabout.cxx kui/kabout.hxx kui/ikextern.h
$(OUTDIR)/kdwnload$(O): kui/kdwnload.cxx kui/kdwnload.hxx kui/ikextern.h
$(OUTDIR)/kuikey$(O): 	kui/kuikey.cxx kui/kuidef.h
$(OUTDIR)/kclient$(O): 	kui/kclient.cxx kui/kclient.hxx kui/kappwin.hxx \
	kui/kscroll.hxx kui/khwndset.hxx kui/ksysmets.hxx kui/kfont.hxx \
	kui/karray.hxx kui/ikterm.h kui/ikcmd.h kui/ikextern.h
$(OUTDIR)/kappwin$(O): 	kui/kappwin.cxx kui/kappwin.hxx kui/kmenu.hxx \
	kui/ktoolbar.hxx kui/kstatus.hxx kui/kclient.hxx kui/kfontdlg.hxx \
	kui/kabout.hxx kui/kfont.hxx kui/ksysmets.hxx kui/kszpopup.hxx \
	kui/ikterm.h kui/ikcmd.h
$(OUTDIR)/ktermin$(O): 	kui/ktermin.cxx kui/ktermin.hxx kui/kmenu.hxx \
	kui/ktoolbar.hxx kui/kstatus.hxx kui/kclient.hxx kui/kdwnload.hxx \
	kui/kfont.hxx kui/ksysmets.hxx kui/ikcmd.h kui/ikterm.h kui/ikui.h \
	kui/ikextern.h
$(OUTDIR)/kui$(O): 		kui/kui.cxx kui/ikextern.h

# Main - cknker.exe, iksdnt.exe, k95g.exe, cknkui.exe and others
# TODO: The list of dependencies here and in ckoker.mak haven't been updated
# 		since C-Kermit 8.0 (~2002) unless otherwise stated
$(OUTDIR)/ckcmai$(O): ckcmai.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckcsym.h ckcnet.h ckctel.h  ckuusr.h ckonet.h ckcsig.h ckocon.h ckntap.h \
	ckocon.h ck_ssl.h ckossl.h
# CKCFNS.C DEPS UPDATED: 2022-04-29
$(OUTDIR)/ckcfns$(O): ckcsym.h ckcasc.h ckcdeb.h ckcker.h ckcxla.h ckcnet.h \
	ckocon.h
$(OUTDIR)/ckcfn2$(O): ckcfn2.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckcsym.h ckcxla.h ckuxla.h ckcnet.h ckctel.h
$(OUTDIR)/ckcfn3$(O): ckcfn3.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckcsym.h ckcxla.h ckuxla.h
$(OUTDIR)/ckcnet$(O): ckcnet.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcnet.h \
	ckctel.h ckonet.h ckotcp.h  ckuusr.h ckcsig.h ckocon.h ckuath.h ck_ssl.h \
	ckossl.h ckosslc.h
# CKCPRO.C DEPS UPDATED: 2023-04-29
$(OUTDIR)/ckcpro$(O): ckcpro.c ckcsym.h ckcdeb.h ckcasc.h ckcker.h ckocon.h \
	ckuath.h ckoker.h ckclib.h ckothr.h ckosyn.h
$(OUTDIR)/ckucmd$(O): ckucmd.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckucmd.h ckuusr.h ckcnet.h ckctel.h
$(OUTDIR)/ckudia$(O): ckudia.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckucmd.h ckuusr.h ckcsig.h ckocon.h cknwin.h ckowin.h ckntap.h ckcnet.h \
	ckctel.h
$(OUTDIR)/ckofio$(O): ckofio.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckuver.h \
	ckodir.h ckoker.h ckuusr.h ckcxla.h ck_ssl.h ckoreg.h
$(OUTDIR)/ckuscr$(O): ckuscr.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckcsig.h ckcnet.h ckctel.h
$(OUTDIR)/ckuusr$(O): ckuusr.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckucmd.h ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckocon.h \
	cknwin.h ckowin.h ckntap.h kui\ikui.h
$(OUTDIR)/ckuus2$(O): ckuus2.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckucmd.h ckcxla.h ckuxla.h ckokvb.h ckocon.h ckokey.h ckcnet.h \
	ckctel.h
$(OUTDIR)/ckuus3$(O): ckuus3.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckucmd.h ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckonbi.h \
	ckntap.h ckoreg.h ckocon.h ckokey.h ckokvb.h ckcuni.h ck_ssl.h ckossl.h \
	ckuath.h kui\ikui.h
$(OUTDIR)/ckuus4$(O): ckuus4.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckucmd.h ckcxla.h ckuxla.h ckuver.h ckcnet.h ckctel.h ckonet.h \
	ckocon.h ckoetc.h ckntap.h ckuath.h ck_ssl.h ckoreg.h ckoetc.h
$(OUTDIR)/ckuus5$(O): ckuus5.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckucmd.h ckocon.h ckokey.h ckokvb.h ckcuni.h ckcnet.h ckctel.h \
	ck_ssl.h ckossl.h kui\ikui.h
$(OUTDIR)/ckuus6$(O): ckuus6.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckucmd.h ckntap.h ckcnet.h ckctel.h ckcsym.h ckcxla.h ckocon.h \
	ckodir.h
$(OUTDIR)/ckuus7$(O): ckuus7.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckucmd.h ckcxla.h ckuxla.h ckcnet.h ckctel.h ckonet.h ckocon.h \
	ckodir.h ckokey.h ckokvb.h cknwin.h ckowin.h ckntap.h ckcuni.h ckntap.h \
	ckuath.h ck_ssl.h kui\ikui.h
$(OUTDIR)/ckuusx$(O): ckuusx.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckonbi.h ckocon.h cknwin.h ckowin.h ckntap.h ckcnet.h ckctel.h \
	kui\ikui.h
$(OUTDIR)/ckuusy$(O): ckuusy.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckuusr.h ckucmd.h ckcnet.h ckctel.h ck_ssl.h kui\ikui.h
$(OUTDIR)/ckuxla$(O): ckuxla.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcxla.h \
	ckuxla.h
$(OUTDIR)/ckclib$(O): ckclib.c ckcsym.h ckcdeb.h ckoker.h ckclib.h ckcasc.h
$(OUTDIR)/ckctel$(O): ckctel.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckctel.h \
	ckcnet.h ckocon.h ck_ssl.h ckossl.h ckosslc.h
$(OUTDIR)/ckcuni$(O): ckcuni.c ckcsym.h ckcdeb.h ckoker.h ckcker.h ckucmd.h \
	ckcxla.h ckuxla.h
#CKCFTP.C DEPS UPDATED: 2023-04-29
$(OUTDIR)/ckcftp$(O): ckcftp.c ckcsym.h ckcdeb.h ckowin.h ckocon.h ckcsig.h \
	ckcasc.h ckcker.h ckucmd.h ckuusr.h ckcnet.h ckctel.h ckcxla.h ckuath.h \
	ckoreg.h ck_ssl.h ckoath.h
$(OUTDIR)/cknsig$(O): cknsig.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckcsym.h ckcnet.h ckctel.h ckonet.h ckuusr.h ckonet.h ckcsig.h ckocon.h
$(OUTDIR)/cknalm$(O): cknalm.c cknalm.h
$(OUTDIR)/ckntap$(O): ckntap.c ckcdeb.h ckoker.h ckclib.h ckcker.h ckntap.h \
	cknwin.h ckowin.h ckuusr.h ckucmd.h ckowin.h ckntapi.h
$(OUTDIR)/cknwin$(O): cknwin.c cknwin.h ckowin.h ckcdeb.h ckoker.h ckclib.h \
	ckntap.h ckocon.h
$(OUTDIR)/cknprt$(O): cknprt.c ckcdeb.h ckoker.h ckcker.h ckucmd.h
$(OUTDIR)/cknpty$(O): cknpty.c cknpty.h
$(OUTDIR)/ckuath$(O): ckuath.c ckoker.h ckclib.h ckcnet.h ckctel.h ckuath.h \
	ckuat2.h ck_ssl.h ckossl.h ckosslc.h ckuath.c ckoath.h
$(OUTDIR)/ckoath$(O): ckoath.c ckcdeb.h ckoker.h ckclib.h ckcnet.h ckctel.h \
	ckuath.h ckuat2.h ckoath.h
$(OUTDIR)/ck_ssl$(O): ck_ssl.c ckcdeb.h ckoker.h ckclib.h ckctel.h ck_ssl.h \
	ckosslc.h ckossl.h
$(OUTDIR)/ckossl$(O): ckossl.c ckcdeb.h ckoker.h ck_ssl.h ckossl.h
$(OUTDIR)/ckosslc$(O): ckosslc.c ckcdeb.h ckoker.h ck_ssl.h ckosslc.h
$(OUTDIR)/ckosftp$(O): ckosftp.c ckoker.h ckclib.h ckosftp.h ckosftp.c
$(OUTDIR)/ckozli$(O): ckozli.c ckcdeb.h ckoker.h ckozli.h
$(OUTDIR)/ckocon$(O): ckocon.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckoker.h ckocon.h ckcnet.h ckctel.h ckonbi.h ckokey.h ckokvb.h ckuusr.h \
	cknwin.h ckowin.h ckcuni.h kui\ikui.h
$(OUTDIR)/ckoco2$(O): ckoco2.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckoker.h ckocon.h ckonbi.h ckopcf.h ckuusr.h ckokey.h ckokvb.h ckcuni.h \
	kui\ikui.h
$(OUTDIR)/ckoco3$(O): ckoco3.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckcasc.h \
	ckoker.h ckocon.h ckokey.h ckokvb.h ckuusr.h ckowys.h ckodg.h  ckoava.h \
	ckoi31.h ckohp.h  ckoadm.h ckohzl.h ckoqnx.h ckotvi.h ckovc.h  ckcuni.h \
    ckcnet.h ckctel.h kui\ikui.h ckossh.h
$(OUTDIR)/ckoco4$(O): ckoco4.c ckcdeb.h ckoker.h ckclib.h ckocon.h ckokey.h \
	ckokvb.h ckuusr.h ckcasc.h ckokey.h ckokvb.h
$(OUTDIR)/ckoco5$(O): ckoco5.c ckcdeb.h ckoker.h ckclib.h ckocon.h
$(OUTDIR)/ckoetc$(O): ckoetc.c ckcdeb.h ckoker.h ckclib.h ckoetc.h
$(OUTDIR)/ckoetc2$(O): ckoetc2.c ckcdeb.h ckoker.h ckclib.h ckoetc.h
$(OUTDIR)/ckokey$(O): ckokey.c ckcdeb.h ckoker.h ckclib.h ckcasc.h ckcker.h \
	ckuusr.h ckctel.h ckocon.h ckokey.h ckokvb.h ckcxla.h ckuxla.h ckcuni.h \
	kui\ikui.h
$(OUTDIR)/ckomou$(O): ckomou.c ckocon.h ckcdeb.h ckoker.h ckclib.h ckokey.h \
	ckokvb.h ckuusr.h
$(OUTDIR)/ckoreg$(O): ckoreg.c ckcdeb.h ckoker.h ckclib.h ckcker.h
$(OUTDIR)/ckonet$(O): ckonet.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckoker.h \
	ckcnet.h ckctel.h ckonet.h ckotcp.h ckonbi.h ckuusr.h ckcsig.h cknwin.h \
	ckowin.h ckuath.h ck_ssl.h ckossl.h ckosslc.h
$(OUTDIR)/ckoslp$(O): ckoslp.c ckoslp.h ckcdeb.h ckoker.h ckclib.h
$(OUTDIR)/ckosyn$(O): ckosyn.c ckcdeb.h ckoker.h ckclib.h ckcker.h ckocon.h \
	ckuusr.h ckntap.h
$(OUTDIR)/ckothr$(O): ckothr.c ckocon.h ckcsym.h ckcasc.h ckcdeb.h ckoker.h \
	ckclib.h ckcker.h ckcsig.h
$(OUTDIR)/ckotek$(O): ckotek.c ckotek.h ckcker.h ckcdeb.h ckoker.h ckclib.h \
	ckcasc.h ckoker.h ckocon.h ckokey.h ckokvb.h ckuusr.h ckcnet.h ckctel.h
$(OUTDIR)/ckotio$(O): ckotio.c ckcker.h ckcdeb.h ckoker.h ckclib.h ckuver.h \
	ckodir.h ckoker.h ckocon.h ckokey.h ckokvb.h ckuusr.h ckoslp.h ckcsig.h \
	ckop.h   ckcuni.h ckowin.h ckcnet.h ckctel.h ckoreg.h ckntap.h cknwin.h \
	kui\ikui.h
$(OUTDIR)/ckowys$(O): ckowys.c ckowys.h ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h ckcuni.h ckokey.h ckokvb.h ckctel.h
$(OUTDIR)/ckodg$(O):  ckodg.c  ckodg.h  ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h ckcnet.h ckctel.h
$(OUTDIR)/ckoava$(O): ckoava.c ckoava.h ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h
$(OUTDIR)/ckoi31$(O): ckoi31.c ckoi31.h ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h
$(OUTDIR)/ckotvi$(O): ckotvi.c ckotvi.h ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h ckctel.h ckokvb.h
$(OUTDIR)/ckovc$(O):  ckovc.c  ckovc.h  ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h
$(OUTDIR)/ckoadm$(O): ckoadm.c ckoadm.h ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h ckcnet.h ckctel.h ckcuni.h
$(OUTDIR)/ckohzl$(O): ckohzl.c ckohzl.h ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h ckcnet.h ckctel.h ckcuni.h
$(OUTDIR)/ckohp$(O):  ckohp.c  ckohp.h  ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h ckokey.h ckokvb.h
$(OUTDIR)/ckoqnx$(O): ckoqnx.c ckoqnx.h ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckcasc.h ckocon.h ckuusr.h
$(OUTDIR)/cknnbi$(O): cknnbi.c ckonbi.h ckcdeb.h ckoker.h ckclib.h
$(OUTDIR)/ckop$(O):   ckop.c   ckop.h   ckcdeb.h ckoker.h ckclib.h ckcker.h \
	ckuusr.h ckcnet.h ckctel.h ckonet.h ckocon.h p_global.h p_callbk.h
$(OUTDIR)/p_callbk$(O): p_callbk.c ckcdeb.h ckoker.h ckclib.h ckocon.h \
	p_type.h p.h p_callbk.h p_common.h p_brw.h p_error.h  p_global.h \
	p_module.h p_omalloc.h
$(OUTDIR)/p_global$(O): p_global.c ckcdeb.h ckoker.h ckclib.h ckocon.h \
 	p_type.h p_tl.h p_brw.h p.h
$(OUTDIR)/p_omalloc$(O): p_omalloc.c ckcdeb.h ckoker.h ckclib.h p_type.h \
	p_error.h p.h
$(OUTDIR)/p_error$(O): p_error.c ckcdeb.h ckoker.h ckclib.h ckocon.h p_type.h \
	p_errmsg.h ckcnet.h ckctel.h ckonet.h
$(OUTDIR)/p_common$(O): p_common.c ckcdeb.h ckoker.h ckclib.h ckocon.h \
	p_type.h p_common.h p_error.h p_module.h p_global.h
$(OUTDIR)/p_tl$(O):  p_tl.c ckcdeb.h ckoker.h ckclib.h ckocon.h p_type.h \
	p_tl.h p_brw.h p.h
$(OUTDIR)/p_dir$(O): p_dir.c ckcdeb.h ckoker.h ckclib.h ckocon.h p_type.h \
	p_dir.h

# Minor utilities
$(OUTDIR)/telnet$(O): telnet.c
$(OUTDIR)/rlogin$(O): rlogin.c
$(OUTDIR)/k95d$(O): k95d.c
$(OUTDIR)/textps$(O): textps.c
$(OUTDIR)/ctl3dins$(O): ctl3dins.c
$(OUTDIR)/iksdsvc$(O): iksdsvc.c
$(OUTDIR)/iksd$(O): iksd.c
$(OUTDIR)/se$(O): se.c

# Headers
ckcasc.h:
ckcsym.h:
ckcdeb.h: ckoker.h ckclib.h
ckcker.h:
ckcxla.h: ckuxla.h ckcuni.h
ckcnet.h:
ckcsig.h:
ckctel.h:

ckuath.h:
ckucmd.h:
ckuusr.h: ckucmd.h

ckocon.h: ckothr.h ckosyn.h
ckothr.h:
ckosyn.h:
ckodir.h:
ckowin.h:
ckoreg.h:
ckosslc.h:
ckossl.h:
ckoath.h:

ckntap.h:

ck_ssl.h: ckosslc.h ckossl.h
