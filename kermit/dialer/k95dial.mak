# K95 Dialer makefile

#    nmake -f k95dial.mak winnt       
#    nmake -f k95dial.mak os2      

# Be sure to set the LIB and INCLUDE environment variables for Zinc, e.g.:
#    set INCLUDE=.;C:\ZINC\INCLUDE;C:\MSVC\INCLUDE
#    set LIB=.;C:\ZINC\LIB\MVCPP400;C:\MSVC\LIB

# TODO: We should only do this on Windows.
!message Attempting to detect compiler...
!include ..\k95\compiler_detect.mak

!message
!message
!message ===============================================================================
!message C-Kermit Dialer Build Configuration
!message ===============================================================================
!message  Architecture:             $(TARGET_CPU)
!message  Compiler:                 $(COMPILER)
!message  Compiler Version:         $(COMPILER_VERSION)
!message  Compiler Target Platform: $(TARGET_PLATFORM)
!message ===============================================================================
!message
!message

# ----- Windows NT compiler options -----------------------------------------
# for debug:    add /Zi to CPP_OPTS
#               add /DEBUG:MAPPED,FULL /DEBUGTYPE:CV to LINK_OPTS
WNT_CPP=cl
WNT_LINK=link
WNT_LIBRARIAN=lib

!if "$(CMP)" == "VCXX"

#WNT_CPP_OPTS= -c -W3 -MT -DWIN32 -DOS2 -DNT -I..\k95 /Zi -noBool
#WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup /MAP /NODEFAULTLIB:libc /Debug:full /Debugtype:cv 
WNT_CPP_OPTS= -c -W3 -MT -DWIN32 -DOS2 -DNT -DCKODIALER -I..\k95 -noBool
!if $(MSC_VER) < 100
# Visual C++ 2.0 or older
WNT_CPP_OPTS=$(WNT_CPP_OPTS) -DNODIAL -DCKT_NT31
!endif
WNT_LINK_OPTS=-subsystem:windows -entry:WinMainCRTStartup /MAP /NODEFAULTLIB:libc
WNT_CON_LINK_OPTS=-subsystem:console -entry:mainCRTStartup
WNT_LIBS=wnt_zil.lib ndirect.lib nservice.lib nstorage.lib libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib shell32.lib ole32.lib uuid.lib advapi32.lib oldnames.lib # compmgr.lib
!if $(MSC_VER) < 130
!message Using ctl3d32
# CTL3D32 is only available on Visual C++ 6.0 and earlier.
WNT_LIBS=$(WNT_LIBS) ctl3d32.lib
!endif

WNT_CON_LIBS=w32_zil.lib ndirect.lib nservice.lib nstorage.lib libc.lib kernel32.lib oldnames.lib

!else

WNT_CPP_OPTS= -c -W3 -MT -DWIN32 -DOS2 -DNT -DCKODIALER -I..\k95
WNT_LINK_OPTS=-subsystem:windows -entry:WinMainCRTStartup /MAP
WNT_LIBS=wnt_zil.lib ndirect.lib nservice.lib nstorage.lib
WNT_CON_LINK_OPTS=-subsystem:console -entry:mainCRTStartup /MAP
WNT_CON_LIBS=w32_zil.lib ndirect.lib nservice.lib nstorage.lib

!endif

WNT_LIB_OPTS=/machine:i386 /subsystem:WINDOWS
WNT_OBJS=

.cpp.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<

.c.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<

# ----- OS/2 compiler options -----------------------------------------------
!if "$(CMP)" == "OWWCL"
OS2_CPP=wpp386
OS2_LINK=wlink
OS2_LIBRARIAN=wlib
OS2_RC=wrc
!else
OS2_CPP=icc
OS2_LINK=ilink
OS2_LIBRARIAN=ilib
OS2_RC=rc
!endif

# ----- Compile, Link, and Lib options --------------------------------------
#OS2_CPP_OPTS=/c /D__OS2__ /DOS2 /Gx+ /Sp1 /Ti+ /Tm+ /Tx+ -D_DEBUG
#OS2_LINK_OPTS=/BASE:0x10000 /PM:PM /NOI /NOE /debug 
# ----- Next line for pre-compiled headers ----------------------------------
#OS2_CPP_OPTS=/c /D__OS2__ /DOS2 /Gx+ /Sp1 /FiZIL.SYM /SiZIL.SYM
#OS2_LINK_OPTS=/BASE:0x10000 /PM:PM /NOI /NOE 
# ----- Next line for pre-compiled headers and optimization -----------------
!if "$(CMP)" == "OWWCL"
# ICC   WCC386
# /Gx+  -xs         Enable generation of C++ Exception Handling Code (watcom: -xs = balanced exception handling)
# /Sp1  -zp=1       Pack aggregate members on specified alignment
# -Sm   ?           Ignore migration keywords
# -G5   ?           Generate code optimized for use on a Pentium processor
# -Gt   ?           Store variables so that they do not cross 64K boundaries. Default: /Gt-
# -Gd-  ?           /Gd+: Use the version of the runtime library that is statically linked.
# -Gn+              Do not generate default libraries in object
# -J    N/A         /J+: Make default char type unsigned. Default: /J+ (uchar is default in watcom)
# -Fi+  ?           Generate Precompiled Headers
# -Si+  -Fh=<file>  Use precompiled headers where available
# -Gi+  ?           Generate fast integer code
# -Gl+  ?           Enable smart linking
# -O    ?           Optimize generated code
# -Oi25 -Oe=<num>   Set the threshold for auto-inlining to <value> intermediate code instructions
# -Gm   ? -bm       Link with multithread runtime libraries. Default: /Gm-
#       -bt=os2     Compile for target OS
#
# Link Flags - ICC  wlink
# /BASE:0x10000
# /PM:PM            ? -bw ?
# /NOI
# /NOE
#                   -l=os2v2_pm Link for OS/2 v2 Presentation Manager

#
# -c -xs
OS2_CPP_OPTS=-DOS2 -DCKODIALER -zp=1 -bm -Fh -bt=os2
OS2_LINK_OPTS=SYSTEM os2v2_pm OP ST=96000
OS2_LIB_OPTS=
OS2_RC_OPTS=
OS2_OBJS=
OS2_LIBS=os2_zil.lib,odirect.lib,oservice.lib,ostorage.lib
!else
OS2_CPP_OPTS=/c /D__OS2__ /DOS2 /DCKODIALER /Gx+ /Sp1 -Sm -G5 -Gt -Gd- -Gn+ -J -Fi+ -Si+ -Gi+ -Gl+ -O -Oi25 -Gm
OS2_LINK_OPTS=/BASE:0x10000 /PM:PM /NOI /NOE
OS2_LINK_OUT=
OS2_LIB_OPTS=
OS2_RC_OPTS=
OS2_OBJS=
OS2_LIBS=os2_zil.lib odirect.lib oservice.lib ostorage.lib
!endif

.SUFFIXES : .cpp .c

.c.obo:
	$(OS2_CPP) $(OS2_CPP_OPTS) -Fo$*.obo $<

.cpp.obo:
	$(OS2_CPP) $(OS2_CPP_OPTS) /Fo$*.obo $<

# ----- Usage --------------------------------------------------------------
usage:
	@echo ...........
	@echo ...........
	@echo To make the K95 Dialer type:
	@echo nmake -f k95dial.mak winnt
	@echo nmake -f k95dial.mak win32
	@echo nmake -f k95dial.mak os2
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean

# ----- Windows NT ----------------------------------------------------------
winnt: nckdial.exe 

nckdial.exe: main.obn dialer.obn lstitm.obn kconnect.obn \
            kdialopt.obn kquick.obn kdconfig.obn kcolor.obn nk95dial.res dialetc.obn \
            kdirnet.obn kdirdial.obn kabout.obn kdemo.obn kstatus.obn kwinmgr.obn ktapi.obn \
            klocation.obn kmodem.obn kmdmdlg.obn kappl.obn ksetgeneral.obn \
            ksetterminal.obn ksetxfer.obn ksetserial.obn ksettelnet.obn \
            ksetkerberos.obn ksettls.obn ksetkeyboard.obn ksetlogin.obn \
            ksetprinter.obn ksetlogs.obn ksetssh.obn ksetftp.obn ksetgui.obn \
            ksetdlg.obn ksettcp.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nk95dial.exe $(WNT_OBJS) \
    main.obn dialer.obn lstitm.obn kconnect.obn kdialopt.obn kquick.obn kdconfig.obn\
    kcolor.obn nk95dial.res dialetc.obn kdirnet.obn kdirdial.obn kdemo.obn kabout.obn kstatus.obn\
    kwinmgr.obn klocation.obn ktapi.obn kmodem.obn kmdmdlg.obn kappl.obn ksetgeneral.obn \
    ksetterminal.obn ksetxfer.obn ksetserial.obn  ksettelnet.obn ksetkerberos.obn \
    ksettls.obn ksetkeyboard.obn ksetlogin.obn ksetprinter.obn ksetlogs.obn \
    ksetssh.obn ksetgui.obn ksetftp.obn ksetdlg.obn ksettcp.obn \
    $(WNT_LIBS)

nk95dial.res: k95dial.rc k95f.ico
    rc -v -dWINVER=0x0400 -fo nk95dial.res k95dial.rc

main.obn: main.cpp dialer.hpp kconnect.hpp kwinmgr.hpp kdemo.hpp

dialer.obn: dialer.cpp dialer.hpp

lstitm.obn: lstitm.cpp lstitm.hpp usermsg.hpp kstatus.hpp kmdminf.h

klocation.obn: klocation.cpp klocation.hpp usermsg.hpp kstatus.hpp \

kmodem.obn: kmodem.cpp kmodem.hpp usermsg.hpp kstatus.hpp

kconnect.obn: kconnect.cpp kconnect.hpp dialer.hpp lstitm.hpp \
              kdialopt.hpp kquick.hpp usermsg.hpp kdconfig.hpp kdirnet.hpp kdirdial.hpp\
              kabout.hpp kstatus.hpp klocation.hpp kmdmdlg.hpp kmodem.hpp kmdminf.h \
              kappl.hpp ksetgeneral.hpp ksetterminal.hpp ksetxfer.hpp ksetserial.hpp \
              ksettelnet.hpp ksetkerberos.hpp ksettls.hpp ksetkeyboard.hpp ksetlogin.hpp \
              ksetprinter.hpp ksetlogs.hpp ksetssh.hpp ksetftp.hpp ksetgui.hpp ksettcp.hpp

kwinmgr.obn:  kwinmgr.cpp kwinmgr.hpp 

ksetgeneral.obn: ksetgeneral.cpp ksetgeneral.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetterminal.obn: ksetterminal.cpp ksetterminal.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp kcolor.hpp

ksetxfer.obn: ksetxfer.cpp ksetxfer.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetserial.obn: ksetserial.cpp ksetserial.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksettelnet.obn: ksettelnet.cpp ksettelnet.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksettcp.obn: ksettcp.cpp ksettcp.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksettls.obn: ksettls.cpp ksettls.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetlogs.obn: ksetlogs.cpp ksetlogs.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetgui.obn: ksetgui.cpp ksetgui.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetdlg.obn: ksetdlg.cpp ksetdlg.hpp

ksetssh.obn: ksetssh.cpp ksetssh.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetftp.obn: ksetftp.cpp ksetftp.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetprinter.obn: ksetprinter.cpp ksetprinter.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetlogin.obn: ksetlogin.cpp ksetlogin.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetkeyboard.obn: ksetkeyboard.cpp ksetkeyboard.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetkerberos.obn: ksetkerberos.cpp ksetkerberos.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

kdialopt.obn: kdialopt.cpp kdialopt.hpp dialer.hpp usermsg.hpp klocation.hpp

kmdmdlg.obn: kmdmdlg.cpp kmdmdlg.hpp dialer.hpp usermsg.hpp kmodem.hpp

kquick.obn: kquick.cpp kquick.hpp dialer.hpp usermsg.hpp kstatus.hpp

kdconfig.obn: kdconfig.cpp kdconfig.hpp usermsg.hpp

kcolor.obn: kcolor.cpp kcolor.hpp

dialetc.obn: dialetc.c ..\k95\ckoetc.h

ktapi.obn: ktapi.c ktapi.h ..\k95\ckcdeb.h ..\k95\ckcker.h ..\k95\ckucmd.h ..\k95\ckuusr.h \
	   ..\k95\ckowin.h ..\k95\cknwin.h kconnect.hpp

kappl.obn:  kappl.cpp kappl.hpp kdconfig.hpp dialer.hpp

kdirdial.obn:  kdirdial.cpp kdirdial.hpp dialer.hpp

kdirnet.obn: kdirnet.cpp kdirnet.hpp dialer.hpp

kabout.obn: kabout.cpp kabout.hpp dialer.hpp

kdemo.obn: kdemo.cpp kdemo.hpp dialer.hpp

kstatus.obn: kstatus.cpp kstatus.hpp

# ----- OS/2 ----------------------------------------------------------
os2: k2dial.exe

#$(CC) $(CC2) $(LINKFLAGS) $(DEBUG) $(OBJS) $(DEF) $(OUT)$@ $(LIBS) $(LDFLAGS)
#        wrc -q -bt=os2 ckoker.res $@
# LINKFLAGS="-l=os2v2" \

# os2.def was previously included below but does not exist in the K95 2.1.3
# build tree. I can only assume either this file was supplied by the IBM
# compiler, or the IBM compiler did not care it was missing. Watcom cares.
# os2.def was also removed from the link command just before $(OS2_LIBS)
k2dial.exe: main.obo dialer.obo lstitm.obo kconnect.obo \
            kdialopt.obo kquick.obo kdconfig.obo kcolor.obo dialetc.obo \
            kdirnet.obo kdirdial.obo kdemo.obo kstatus.obo kwinmgr.obo \
            klocation.obo kmodem.obo kmdmdlg.obo kappl.obo ksetgeneral.obo \
            ksetterminal.obo ksetxfer.obo ksetserial.obo ksettelnet.obo \
            ksetkerberos.obo ksettls.obo ksetkeyboard.obo ksetlogin.obo \
            ksetprinter.obo ksetlogs.obo ksetssh.obo ksetftp.obo ksetgui.obo \
            ksetdlg.obo kabout.obo ksettcp.obo k2dial.rc
!if "$(CMP)" == "OWWCL"
	$(OS2_LINK) $(OS2_LINK_OPTS) N k2dial.exe \
    F main.obo,dialer.obo,lstitm.obo,kconnect.obo,kdialopt.obo,kquick.obo,kdconfig.obo,\
    kcolor.obo,dialetc.obo,kdirnet.obo,kdirdial.obo,kdemo.obo,kstatus.obo,kwinmgr.obo,\
    klocation.obo,kmodem.obo,kmdmdlg.obo,kappl.obo,ksetgeneral.obo,\
    ksetterminal.obo,ksetxfer.obo,ksetserial.obo,ksettelnet.obo,ksetkerberos.obo,\
    ksettls.obo,ksetkeyboard.obo,ksetlogin.obo,ksetprinter.obo,ksetlogs.obo,\
    ksetssh.obo,ksetgui.obo,ksetftp.obo,ksetdlg.obo,kabout.obo,ksettcp.obo \
    L $(OS2_LIBS)
    # k2dial.rc is empty so no need to do anything with the resource compiler.
    #rc k2dial.rc k2dial.exe
!else
	$(OS2_LINK) $(OS2_LINK_OPTS) -out:k2dial.exe \
    main.obo dialer.obo lstitm.obo kconnect.obo kdialopt.obo kquick.obo kdconfig.obo\
    kcolor.obo dialetc.obo kdirnet.obo kdirdial.obo kdemo.obo kstatus.obo kwinmgr.obo \
    klocation.obo kmodem.obo kmdmdlg.obo kappl.obo ksetgeneral.obo \
    ksetterminal.obo ksetxfer.obo ksetserial.obo  ksettelnet.obo ksetkerberos.obo \
    ksettls.obo ksetkeyboard.obo ksetlogin.obo ksetprinter.obo ksetlogs.obo \
    ksetssh.obo ksetgui.obo ksetftp.obo ksetdlg.obo kabout.obo ksettcp.obo os2.def $(OS2_LIBS)
    rc k2dial.rc k2dial.exe
!endif

main.obo: main.cpp dialer.hpp kconnect.hpp kwinmgr.hpp

dialer.obo: dialer.cpp dialer.hpp

lstitm.obo: lstitm.cpp lstitm.hpp usermsg.hpp kstatus.hpp kmdminf.h

kconnect.obo: kconnect.cpp kconnect.hpp dialer.hpp lstitm.hpp \
              kdialopt.hpp kquick.hpp usermsg.hpp kdconfig.hpp kdirnet.hpp kdirdial.hpp\
              kdemo.hpp kstatus.hpp kmdminf.h kappl.hpp ksetgeneral.hpp

kappl.obo:  kappl.cpp kappl.hpp dialer.hpp

kwinmgr.obo:  kwinmgr.cpp kwinmgr.hpp 

ksetgeneral.obo: ksetgeneral.cpp ksetgeneral.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetterminal.obo: ksetterminal.cpp ksetterminal.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp kcolor.hpp

ksetxfer.obo: ksetxfer.cpp ksetxfer.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetserial.obo: ksetserial.cpp ksetserial.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksettelnet.obo: ksettelnet.cpp ksettelnet.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksettls.obo: ksettls.cpp ksettls.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetlogs.obo: ksetlogs.cpp ksetlogs.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetgui.obo: ksetgui.cpp ksetgui.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetdlg.obo: ksetdlg.cpp ksetdlg.hpp

ksetssh.obo: ksetssh.cpp ksetssh.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetftp.obo: ksetftp.cpp ksetftp.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetprinter.obo: ksetprinter.cpp ksetprinter.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetlogin.obo: ksetlogin.cpp ksetlogin.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetkeyboard.obo: ksetkeyboard.cpp ksetkeyboard.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

ksetkerberos.obo: ksetkerberos.cpp ksetkerberos.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 


kdialopt.obo: kdialopt.cpp kdialopt.hpp dialer.hpp usermsg.hpp klocation.hpp

kquick.obo: kquick.cpp kquick.hpp dialer.hpp usermsg.hpp kstatus.hpp

kdconfig.obo: kdconfig.cpp kdconfig.hpp usermsg.hpp

kcolor.obo: kcolor.cpp kcolor.hpp

dialetc.obo: dialetc.c ..\k95\ckoetc.h

kdirdial.obo:  kdirdial.cpp kdirdial.hpp dialer.hpp

kdirnet.obo: kdirnet.cpp kdirnet.hpp dialer.hpp

kdemo.obo: kdemo.cpp kdemo.hpp dialer.hpp

kstatus.obo: kstatus.cpp kstatus.hpp

klocation.obo: klocation.cpp klocation.hpp usermsg.hpp kstatus.hpp

kmodem.obo: kmodem.cpp kmodem.hpp usermsg.hpp kstatus.hpp

kmdmdlg.obo: kmdmdlg.cpp kmdmdlg.hpp dialer.hpp usermsg.hpp kmodem.hpp

kabout.obo: kabout.cpp kabout.hpp dialer.hpp

ksettcp.obo: ksettcp.cpp ksettcp.hpp dialer.hpp lstitm.hpp kconnect.hpp \
        usermsg.hpp 

