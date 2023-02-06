PROJ = kui

OUTDIR = .\win95

!include ..\compiler_detect.mak
!include ..\feature_flags.mak

!if "$(CKB_STATIC_CRT)"=="yes"
RTFLAG = /MT
!else
RTFLAG = /MD
!endif

all: $(OUTDIR)\$(PROJ).exe 

KUIOBJS = \
    $(OUTDIR)\kregedit.obj $(OUTDIR)\ksysmets.obj     \
    $(OUTDIR)\ikui.obj     $(OUTDIR)\kprogres.obj \
    $(OUTDIR)\ikterm.obj   $(OUTDIR)\ikcmd.obj    $(OUTDIR)\kuidef.obj   \
    $(OUTDIR)\karray.obj   $(OUTDIR)\khwndset.obj $(OUTDIR)\kwin.obj     \
    $(OUTDIR)\kszpopup.obj $(OUTDIR)\kflstat.obj  $(OUTDIR)\kcustdlg.obj \
    $(OUTDIR)\kmenu.obj    $(OUTDIR)\kstatus.obj  $(OUTDIR)\ktoolbar.obj \
    $(OUTDIR)\kscroll.obj  \
    $(OUTDIR)\kfont.obj    \
    $(OUTDIR)\kfontdlg.obj $(OUTDIR)\kabout.obj   \
    $(OUTDIR)\kdwnload.obj \
    $(OUTDIR)\kuikey.obj   $(OUTDIR)\kclient.obj  \
    $(OUTDIR)\kappwin.obj  \
    $(OUTDIR)\ktermin.obj  $(OUTDIR)\kui.obj

RC_DEP = icon1.ico resource.h kui.rc

#--------------------------------------------------------------------------
# Dependencies

$(OUTDIR)\kabout.obj: kabout.cxx kabout.hxx kwin.hxx kuidef.h resource.h
$(OUTDIR)\KUIKEY.obj: KUIKEY.CXX kuidef.h resource.h
$(OUTDIR)\kdwnload.obj: kdwnload.cxx kdwnload.hxx kwin.hxx kuidef.h resource.h
$(OUTDIR)\kflstat.obj: kflstat.cxx kflstat.hxx kwin.hxx kuidef.h resource.h kflstat.h karray.hxx kprogres.hxx
$(OUTDIR)\kmenu.obj: kmenu.cxx kmenu.hxx kuidef.h resource.h kwin.hxx ksysmets.hxx
$(OUTDIR)\kscroll.obj: kscroll.cxx kscroll.hxx kwin.hxx kuidef.h resource.h ikextern.h
$(OUTDIR)\kstatus.obj: kstatus.cxx kstatus.hxx kwin.hxx kuidef.h resource.h khwndset.hxx ksysmets.hxx kcustdlg.hxx karray.hxx
$(OUTDIR)\KWIN.obj: KWIN.CXX kwin.hxx kuidef.h resource.h khwndset.hxx
$(OUTDIR)\kuidef.obj: kuidef.cxx kuidef.h resource.h
$(OUTDIR)\ktermin.obj: ktermin.cxx ktermin.hxx kappwin.hxx kwin.hxx kuidef.h resource.h kmenu.hxx ktoolbar.hxx kstatus.hxx kclient.hxx kdwnload.hxx kfont.hxx ksysmets.hxx ikcmd.h kdefs.h ikterm.h ikui.h
$(OUTDIR)\khwndset.obj: khwndset.cxx khwndset.hxx kuidef.h resource.h karray.hxx
$(OUTDIR)\KFONTDLG.obj: KFONTDLG.CXX kfontdlg.hxx kwin.hxx kuidef.h resource.h khwndset.hxx
$(OUTDIR)\kregedit.obj: kregedit.cxx kregedit.hxx kuidef.h resource.h
$(OUTDIR)\karray.obj: karray.cxx karray.hxx kuidef.h resource.h
$(OUTDIR)\Kui.obj: Kui.cxx kui.hxx kuidef.h resource.h ktermin.hxx kappwin.hxx kwin.hxx kflstat.hxx khwndset.hxx kflstat.h ikui.h ikcmd.h kdefs.h
$(OUTDIR)\kfont.obj: kfont.cxx kfont.hxx kuidef.h resource.h karray.hxx
$(OUTDIR)\Ksysmets.obj: Ksysmets.cxx ksysmets.hxx kuidef.h resource.h
$(OUTDIR)\ktoolbar.obj: ktoolbar.cxx ktoolbar.hxx kwin.hxx kuidef.h resource.h khwndset.hxx ikcmd.h kdefs.h
$(OUTDIR)\Kclient.obj: Kclient.cxx kclient.hxx kwin.hxx kuidef.h resource.h kscroll.hxx khwndset.hxx ksysmets.hxx kfont.hxx karray.hxx ikterm.h ikcmd.h kdefs.h
$(OUTDIR)\Kcustdlg.obj: Kcustdlg.cxx kcustdlg.hxx kwin.hxx kuidef.h resource.h kstatus.hxx khwndset.hxx kui.hxx ksysmets.hxx
$(OUTDIR)\KPROGRES.obj: KPROGRES.CXX kprogres.hxx kuidef.h resource.h ksysmets.hxx
$(OUTDIR)\kszpopup.obj: kszpopup.CXX kszpopup.hxx kwin.hxx kuidef.h resource.h kfont.hxx
$(OUTDIR)\Ikui.obj: Ikui.cxx ikui.h kuidef.h resource.h kui.hxx kregedit.hxx ksysmets.hxx khwndset.hxx
$(OUTDIR)\Kappwin.obj: Kappwin.cxx kappwin.hxx kwin.hxx kuidef.h resource.h kmenu.hxx ktoolbar.hxx kstatus.hxx kclient.hxx kfontdlg.hxx kabout.hxx kfont.hxx ksysmets.hxx kszpopup.hxx ikterm.h ikcmd.h kdefs.h

$(OUTDIR)\ikterm.obj: ikterm.cxx ikterm.h ikui.h \
    $(KERMITDIR)\ckcdeb.h $(KERMITDIR)\ckcker.h $(KERMITDIR)\ckcasc.h \
    $(KERMITDIR)\ckcxla.h $(KERMITDIR)\ckcnet.h $(KERMITDIR)\ckuusr.h \
    $(KERMITDIR)\ckopcf.h $(KERMITDIR)\ckocon.h $(KERMITDIR)\ckokey.h \
    $(KERMITDIR)\ckcuni.h

$(OUTDIR)\ikcmd.obj: ikcmd.c ikcmd.h kdefs.h \
    $(KERMITDIR)\ckcdeb.h $(KERMITDIR)\ckcker.h $(KERMITDIR)\ckcasc.h \
    $(KERMITDIR)\ckcxla.h $(KERMITDIR)\ckcnet.h $(KERMITDIR)\ckuusr.h \
    $(KERMITDIR)\ckopcf.h $(KERMITDIR)\ckocon.h $(KERMITDIR)\ckokey.h \
    $(KERMITDIR)\ckcxxx.h $(KERMITDIR)\ckcuni.h

!ifdef NODEBUG
# Non-debugging options

!MESSAGE 32-bit Non-debugging build

CFLAGS    = /c /W3 $(RTFLAG) /Ox /J /nologo /Fo$(OUTDIR)\ /I$(KERMITDIR)
CDEFINES  = /D_MT /DSTRICT /D_X86_ /DNDEBUG /D_WINDOWS /DWIN32 /DWIN32_LEAN_AND_MEAN \
                -DNT -DOS2 -DDYNAMIC -DKANJI -DNETCONN -DTCPSOCKET \
                -DHADDRLIST -DSCRIPTTERM -DOS2MOUSE -DOS2TEST -DKUI \
                -D_WIN32_WINNT=$(WIN32_VERSION) \
                $(ENABLED_FEATURE_DEFS) $(DISABLED_FEATURE_DEFS)
LFLAGS    = /NOD /NOLOGO /SUBSYSTEM:windows
RCFLAGS   = /r
RCDEFINES = /dNDEBUG /dWIN32 $(RC_FEATURE_DEFS)
MAPFILE   =

# /G5 is only valid on 7.1 and earlier
!if ($(MSC_VER) < 140)
CFLAGS = $(CFLAGS) /G5
!endif

!if ($(MSC_VER) > 90)
CFLAGS = $(CFLAGS) /GA
!endif

!else
# Debugging options

!MESSAGE 32-bit Debugging build

CFLAGS    = /c /W3 /MD /Od /Zi /J /Fd$(OUTDIR)\$(PROJ).pdb /nologo \
                /Fo$(OUTDIR)\ /I$(KERMITDIR)
CDEFINES  = /D_MT /D_X86_ /DSTRICT /D_DEBUG /D_WINDOWS /DWIN32 /DWIN32_LEAN_AND_MEAN \
                -DNT -DOS2 -DDYNAMIC -DKANJI -DNETCONN -DTCPSOCKET -DSUPERLAT\
                -DHADDRLIST -DSCRIPTTERM -DOS2MOUSE -DRLOGCODE \
                -DNETFILE -DONETERMUPD -DKUI \
                -D_WIN32_WINNT=$(WIN32_VERSION) \
                $(ENABLED_FEATURE_DEFS) $(DISABLED_FEATURE_DEFS)

LFLAGS    = /NOD /NOLOGO /DEBUG:full /DEBUGTYPE:cv /SUBSYSTEM:windows
RCFLAGS   = /r
RCDEFINES = /d_DEBUG /dWIN32 /dCOMPILER_$(CMP)
!if "$(CKT_NT31)" == "yes"
RCDEFINES = $(RCDEFINES) /dCKT_NT31
!endif
MAPFILE   = /map:$(OUTDIR)\$(PROJ).map

!endif

!if "$(CKF_TOOLBAR)" == "no"
RCDEFINES = $(RCDEFINES) /dNOTOOLBAR
!endif

# Customise for the compiler version we're using
!IF ($(MSC_VER) < 131)
# Visual C++ 7.0 (2002) and earlier

!if ($(MSC_VER) > 100)
# This switch is new to Visual C++ 5.0 (along with bool support)
CFLAGS = $(CFLAGS) /noBool
!endif

# Visual C++ 7.1 - nothing required
!ELSEIF ($(MSC_VER) > 131)
# Visual C++ 8.0 or newer

CFLAGS = $(CFLAGS) /Zc:wchar_t-
CDEFINES = $(CDEFINES) -D_CRT_SECURE_NO_DEPRECATE

!ENDIF

#--------------------------------------------------------------------------
# 32-bit Build Rules

$(OUTDIR):
    if not exist $(OUTDIR)\nul mkdir $(OUTDIR)

$(OUTDIR)\$(PROJ).EXE: $(OUTDIR) $(KUIOBJS) $(OUTDIR)\$(PROJ).res
!ifdef VCSID
    cd $(KERMITDIR)
    set PLATFORM=NT
    nmake /nologo /e /f ckoker.mak kuid
!endif

$(OUTDIR)\$(PROJ).res: $(PROJ).rc $(RC_DEP)
    rc $(RCFLAGS) $(RCDEFINES) /fo$(OUTDIR)\$(PROJ).res $(PROJ).rc

.c{$(OUTDIR)}.obj: 
    cl $(CFLAGS) $(CDEFINES) $<

.cxx{$(OUTDIR)}.obj: 
    cl $(CFLAGS) $(CDEFINES) $<
