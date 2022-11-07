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

WNT_CPP_OPTS= -c -W3 -D_X86_=1 -DWIN32 -MT
WNT_LINK_OPTS=-align:0x1000 -subsystem:windows -entry:WinMainCRTStartup
WNT_CON_LINK_OPTS=-align:0x1000 -subsystem:console -entry:mainCRTStartup
WNT_LIB_OPTS=/machine:i386 /subsystem:WINDOWS

WNT_OBJS=
WNT_LIBS=libcmt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib wnt_zil.lib oldnames.lib 
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
dos32: file32.exe
	copy file32.exe ..\..\bin

file32.exe: main.o32 file32.lib
	$(D32_LINK) $(D32_LINK_OPTS) -out:file32.exe $(D32_OBJS) main.o32 storag32.lib stredi32.lib file32.lib direct32.lib servic32.lib $(D32_LIBS)

file32.lib : file.o32
	-@del file32.lib
	$(D32_LIBRARIAN) $(D32_LIB_OPTS) @<<zil.rsp
/out:$*.lib
file.o32
<<
	copy file.hpp ..\..\include
	copy file32.lib ..\..\lib\$(VERSION)
	copy p_file.dat ..\..\bin\p_file.znc

# ----- Windows NT Libraries and Programs -----------------------------------
winnt: nfile.exe
	copy nfile.exe ..\..\bin

nfile.exe: main.obn nfile.lib
	$(WNT_LINK) $(WNT_LINK_OPTS) -out:nfile.exe $(WNT_OBJS) main.obn nstorage.lib nstredit.lib nfile.lib ndirect.lib nservice.lib $(WNT_LIBS)


nfile.lib : file.obn
	-@del nfile.lib
	$(WNT_LIBRARIAN) /out:nfile.lib $(WNT_LIB_OPTS) @<<zil.rsp
file.obn
<<
	copy file.hpp ..\..\include
	copy nfile.lib ..\..\lib\$(VERSION)
	copy p_file.dat ..\..\bin\p_file.znc

# ----- Windows 32 bit Libraries and Programs -------------------------------
win32: 9file.exe
	copy 9file.exe ..\..\bin

9file.exe: main.ob9 9file.lib
	$(W32_LINK) $(W32_LINK_OPTS) -out:9file.exe $(W32_OBJS) main.ob9 9storage.lib 9stredit.lib 9file.lib 9direct.lib 9service.lib $(W32_LIBS)


9file.lib : file.ob9
	-@del 9file.lib
	$(W32_LIBRARIAN) /out:9file.lib $(W32_LIB_OPTS) @<<zil.rsp
file.ob9
<<
	copy file.hpp ..\..\include
	copy 9file.lib ..\..\lib\$(VERSION)
	copy p_file.dat ..\..\bin\p_file.znc
