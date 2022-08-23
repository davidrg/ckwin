# K95 Registry and Desktop Tool makefile

#    nmake -f k95dial.mak winnt       

# Be sure to set the LIB and INCLUDE environment variables for Zinc, e.g.:
#    set INCLUDE=.;C:\ZINC\INCLUDE;C:\MSVC\INCLUDE
#    set LIB=.;C:\ZINC\LIB\MVCPP400;C:\MSVC\LIB

# ----- Windows NT compiler options -----------------------------------------
# for debug:    add /Zi to CPP_OPTS
#               add /DEBUG:MAPPED,FULL /DEBUGTYPE:CV to LINK_OPTS
WNT_CPP=cl
WNT_LINK=link
WNT_LIBRARIAN=lib

WNT_CPP_OPTS= -c -MT -W3 -DWIN32 -DOS2 -DNT -I.\.. -J -noBool
WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup /MAP
#WNT_CPP_OPTS= -c -MT -W3 -D_X86_=1 -DWIN32 -DOS2 -DNT -I.\.. /Zi -J -noBool
#WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup /MAP /Debug:full /Debugtype:cv 
WNT_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup 
WNT_LIB_OPTS=/machine:i386 /subsystem:WINDOWS

WNT_OBJS=
WNT_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib wnt_zil.lib ndirect.lib nservice.lib nstorage.lib oldnames.lib ctl3d32.lib shell32.lib ole32.lib uuid.lib advapi32.lib # compmgr.lib

.cpp.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<

.c.obn:
	$(WNT_CPP) $(WNT_CPP_OPTS) -Fo$*.obn $<


# ----- Usage --------------------------------------------------------------
usage:
	@echo ...........
	@echo ...........
	@echo To make the K95 Dialer type:
	@echo nmake -f k95dial.mak winnt
	@echo ...........
	@echo ...........

# ----- Clean ---------------------------------------------------------------
clean:
	z_clean

# ----- Windows NT ----------------------------------------------------------
winnt: k95regtl.exe

k95regtl.exe: main.obn kregistry.obn registry.obn
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:k95regtl.exe $(WNT_OBJS) \
    main.obn kregistry.obn registry.obn $(WNT_LIBS)

k95regtl.res: k95regtl.rc k95f.ico
    rc -v -fo k95regtl.res k95regtl.rc

main.obn: main.cpp kregistry.hpp registry.hpp

registry.obn: registry.cpp registry.hpp

kregistry.obn:  kregistry.cpp kregistry.hpp

