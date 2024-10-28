# K95 Dialer makefile

#    nmake -f k95dial.mak winnt       
#    nmake -f k95dial.mak os2      

# Be sure to set the LIB and INCLUDE environment variables for Zinc, e.g.:
#    set INCLUDE=.;C:\ZINC\INCLUDE;C:\MSVC\INCLUDE
#    set LIB=.;C:\ZINC\LIB\MVCPP400;C:\MSVC\LIB

# ----- Windows NT compiler options -----------------------------------------
# for debug:    add /Zi to CPP_OPTS
#               add /DEBUG:MAPPED,FULL /DEBUGTYPE:CV to LINK_OPTS
WNT_CPP=cl /nologo
WNT_LINK=link /nologo
WNT_LIBRARIAN=lib /nologo

!if "$(CMP)" == "VCXX"

#WNT_CPP_OPTS= -c -W3 -MT -DWIN32 -DOS2 -DNT -I.\.. -noBool
#WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup /MAP /NODEFAULTLIB:libc
WNT_CPP_OPTS= -c -W3 -MT -DWIN32 -DOS2 -DNT -I.\.. /Zi -noBool
WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup /MAP /Debug:full /NODEFAULTLIB:libc /Debugtype:cv
WNT_LIBS=wnt_zil.lib ndirect.lib nservice.lib nstorage.lib libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib ctl3d32.lib shell32.lib ole32.lib uuid.lib advapi32.lib oldnames.lib # compmgr.lib
WNT_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup 
WNT_CON_LIBS=w32_zil.lib ndirect.lib nservice.lib nstorage.lib libc.lib kernel32.lib oldnames.lib

!else

WNT_CPP_OPTS= -c -W3 -MT -DWIN32 -DOS2 -DNT -I.\.. -Z7
WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup /MAP /Debug:full
WNT_LIBS=wnt_zil.lib ndirect.lib nservice.lib nstorage.lib # compmgr.lib
WNT_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup 
WNT_CON_LIBS=w32_zil.lib ndirect.lib nservice.lib nstorage.lib

!endif

WNT_LIB_OPTS=/machine:i386 /subsystem:WINDOWS
WNT_OBJS=

.cpp.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<

.c.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<


# ----- OS/2 compiler options -----------------------------------------------
OS2_CPP=icc
OS2_LINK=ilink
OS2_LIBRARIAN=ilib
OS2_RC=rc

# ----- Compile, Link, and Lib options --------------------------------------
#OS2_CPP_OPTS=/c /D__OS2__ /DOS2 /Gx+ /Sp1 /Ti+ /Tm+ /Tx+ -D_DEBUG
#OS2_LINK_OPTS=/BASE:0x10000 /PM:PM /NOI /NOE /debug 
# ----- Next line for pre-compiled headers ----------------------------------
#OS2_CPP_OPTS=/c /D__OS2__ /DOS2 /Gx+ /Sp1 /FiZIL.SYM /SiZIL.SYM
#OS2_LINK_OPTS=/BASE:0x10000 /PM:PM /NOI /NOE 
# ----- Next line for pre-compiled headers and optimization -----------------
OS2_CPP_OPTS=/c /D__OS2__ /DOS2 /Gx+ /Sp1 -Sm -G5 -Gt -Gd- -Gn+ -J -Fi+ -Si+ -Gi+ -Gl+ -O -Oi25
OS2_LINK_OPTS=/BASE:0x10000 /PM:PM /NOI /NOE 
OS2_LIB_OPTS=
OS2_RC_OPTS=

OS2_OBJS=
OS2_LIBS=os2_zil.lib odirect.lib oservice.lib ostorage.lib

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
winnt: nsetup.exe

nsetup.exe: setup.obn main.obn ktapi.obn ksetup.obn ksetdlg.obn kwelcome.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nsetup.exe $(WNT_OBJS) \
    main.obn setup.obn ktapi.obn ksetup.obn ksetdlg.obn kwelcome.obn $(WNT_LIBS)

nsetup.res: nsetup.rc k95f.ico
    rc -v -fo nsetup.res nsetup.rc

main.obn: main.cpp setup.hpp ksetup.hpp

setup.obn: setup.cpp setup.hpp

ksetup.obn: ksetup.cpp ksetup.hpp setup.hpp

ksetdlg.obn: ksetdlg.cpp ksetdlg.hpp

kwelcome.obn: kwelcome.cpp kwelcome.hpp

ktapi.obn: ktapi.c ktapi.h ..\..\ckcdeb.h ..\..\ckcker.h ..\..\ckucmd.h ..\..\ckuusr.h \
           ..\..\ckowin.h ..\..\cknwin.h 

# ----- OS/2 ----------------------------------------------------------
os2: k2dial.exe k2reg.exe k2site.exe

k2dial.exe: main.obo dialer.obo lstitm.obo kconnect.obo ksetnote.obo \
            kdialopt.obo kquick.obo kdconfig.obo kcolor.obo ckoetc.obo \
            kdirnet.obo kdirdial.obo kabout.obo kstatus.obo kwinmgr.obo \
            klocation.obo kmodem.obo kmdmdlg.obo os2.def k2dial.rc
	$(OS2_LINK) $(OS2_LINK_OPTS) -out:k2dial.exe \
    main.obo dialer.obo lstitm.obo kconnect.obo ksetnote.obo kdialopt.obo kquick.obo kdconfig.obo\
    kcolor.obo ckoetc.obo kdirnet.obo kdirdial.obo kabout.obo kstatus.obo kwinmgr.obo \
    klocation.obo kmodem.obo kmdmdlg.obo \
    os2.def $(OS2_LIBS)
    rc k2dial.rc k2dial.exe 

k2reg.exe: ckreg.obo dialer.obo ckoetc.obo kreg.obo k2reg.rc
	$(OS2_LINK) $(OS2_LINK_OPTS) -out:k2reg.exe $(OS2_OBJS) \
    ckreg.obo dialer.obo ckoetc.obo kreg.obo\
    $(OS2_LIBS)
    rc k2reg.rc k2reg.exe 

k2site.exe: ckreg.obo dialer.obo ckoetc.obo kreg2.obo k2reg.rc
	$(OS2_LINK) $(OS2_LINK_OPTS) -out:k2site.exe $(OS2_OBJS) \
    ckreg.obo dialer.obo ckoetc.obo kreg2.obo\
    $(OS2_LIBS)
    rc k2reg.rc k2site.exe 

main.obo: main.cpp dialer.hpp kconnect.hpp kwinmgr.hpp

dialer.obo: dialer.cpp dialer.hpp

lstitm.obo: lstitm.cpp lstitm.hpp usermsg.hpp kstatus.hpp kmdminf.h

kconnect.obo: kconnect.cpp kconnect.hpp dialer.hpp lstitm.hpp ksetnote.hpp\
              kdialopt.hpp kquick.hpp usermsg.hpp kdconfig.hpp kdirnet.hpp kdirdial.hpp\
              kabout.hpp kstatus.hpp kmdminf.h

kwinmgr.obo:  kwinmgr.cpp kwinmgr.hpp 

ksetnote.obo: ksetnote.cpp ksetnote.hpp dialer.hpp lstitm.hpp kconnect.hpp usermsg.hpp\
	      kmodem.hpp

kdialopt.obo: kdialopt.cpp kdialopt.hpp dialer.hpp usermsg.hpp klocation.hpp

kquick.obo: kquick.cpp kquick.hpp dialer.hpp usermsg.hpp kstatus.hpp

kdconfig.obo: kdconfig.cpp kdconfig.hpp usermsg.hpp

kcolor.obo: kcolor.cpp kcolor.hpp

ckoetc.obo: ckoetc.c ..\ckoetc.h

kdirdial.obo:  kdirdial.cpp kdirdial.hpp dialer.hpp

kdirnet.obo: kdirnet.cpp kdirnet.hpp dialer.hpp

kabout.obo: kabout.cpp kabout.hpp dialer.hpp

ckoreg.obo: ckoreg.cpp dialer.hpp ..\ckoetc.h

kreg.obo: kreg.cpp kreg.hpp dialer.hpp ..\ckoetc.h

kreg2.obo: kreg.cpp kreg.hpp dialer.hpp ..\ckoetc.h
    ren kreg.cpp kreg2.cpp
	$(OS2_CPP) $(OS2_CPP_OPTS) -DSITE_LICENSE -Fokreg2.obo kreg2.cpp
    ren kreg2.cpp kreg.cpp
    

kstatus.obo: kstatus.cpp kstatus.hpp

klocation.obo: klocation.cpp klocation.hpp usermsg.hpp kstatus.hpp

kmodem.obo: kmodem.cpp kmodem.hpp usermsg.hpp kstatus.hpp

kmdmdlg.obo: kmdmdlg.cpp kmdmdlg.hpp dialer.hpp usermsg.hpp kmodem.hpp

