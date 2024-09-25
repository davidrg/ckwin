# Visual C++ 14.x required with a >2018 Platform SDK.

# This builds a NET_DLL that provides PTY support on sufficiently
# new (>2018) versions of Windows 10 and greater.

OUT_LIB	    = conpty.lib
OUT_DLL	    = conpty.dll

SRC         = conptydll.c cknpty.c ckclib.c
DEF         = conptydll.def

DEFINES	    = -D__WIN32__ -DWIN32 -DNT -D__STDC__ -DNETCMD \
              -D_CRT_SECURE_NO_DEPRECATE -DUSE_STRERROR -DCK_CONPTY

LIBS        = kernel32.lib

CFLAGS      = $(DEFINES)



release:    $(OUT_LIB) $(OUT_DLL)
all:	    $(OUT_LIB) $(OUT_DLL)

$(OUT_LIB): $(OUT_DLL)

$(OUT_DLL): $(OBJS) $(DEF)
	cl /nologo $(CFLAGS) /LD  $(SRC) $(LIBS) /link /DEF:$(DEF) /out:$(OUT_DLL)

clean:
	-@del $(OUT_DLL)
	-@del $(OUT_LIB)
	-@del *.obj
	-@del *.exp
	-@del *.pdb

