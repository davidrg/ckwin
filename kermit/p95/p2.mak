# Copyright 1995 Jyrki Salmi, Online Solutions Oy (www.online.fi)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
###############################################################################
#                                                                             #
#			      Makefile for P.DLL.                                         #
#									                                          #
###############################################################################

!message Attempting to detect compiler...
!include ../k95/compiler_detect.mak

!message
!message
!message ===============================================================================
!message P95 Build Configuration
!message ===============================================================================
!message  Architecture:             $(TARGET_CPU)
!message  Compiler:                 $(COMPILER)
!message  Compiler Version:         $(COMPILER_VERSION)
!message  Compiler Target Platform: $(TARGET_PLATFORM)
!message ===============================================================================
!message
!message

# COMPILER = ICC

P_SRCS = \
pdll_common.c \
pdll_crc.c \
pdll_dev.c \
pdll_global.c \
pdll_main.c \
pdll_omalloc.c \
pdll_error.c \
pdll_r.c \
pdll_ryx.c \
pdll_rz.c \
pdll_s.c \
pdll_syx.c \
pdll_sz.c \
pdll_tcpipapi.c \
pdll_x_global.c \
pdll_z.c \
pdll_z_global.c \
pdll_exeio.c

P_OBJS = \
pdll_common.obj \
pdll_crc.obj \
pdll_global.obj \
pdll_main.obj \
pdll_omalloc.obj \
pdll_error.obj \
pdll_r.obj \
pdll_ryx.obj \
pdll_rz.obj \
pdll_s.obj \
pdll_syx.obj \
pdll_sz.obj \
pdll_tcpipapi.obj \
pdll_x_global.obj \
pdll_z.obj \
pdll_z_global.obj \
pdll_dev.obj \
pdll_exeio.obj 

# put back cko32rt.lib

SRCS = $(P_SRCS)
OBJS = $(P_OBJS)
#LIBS = tcp32dll.lib so32dll.lib os2386.lib
!if "$(CMP)" != "OWWCL"
DEFS = p2.def
!endif

.c.obj:
	$(CC) $(CFLAGS) /Fo$@ $<

!if "$(CMP)" == "OWWCL"
# OpenWatcom WCL

# CC parameters not ported: -q
CC = wcl386

# CFLAGS not ported:
#  -Gt- -Ge- -Ti+ -Tx+ -Tm+  -Sp1 -Sm -Gm -G4 -Gt -Gd
#
#    ICC:       WCL386
#     -Gd        ?          /Gd+: Use the version of the runtime library that is dynamically linked.
#     -Gt        ?          Store variables so that they do not cross 64K boundaries. Default: /Gt-
#     -G4        ?          /G4: Generate code optimized for use on a Pentium processor.
#     -Gm        ? -bm      Link with multithread runtime libraries. Default: /Gm-
#     -Sm        ?          Ignore migration keywords. Default: /Sm-
#     -Sp1       -zp=1      /Sp<[1]|2|4|8|16> : Pack aggregate members on specified alignment. Default: /Sp4
#     -Tm+       ?          Enable debug memory management support
#     -Tx+       ?          Generate full exception register dump
#     -Ti+       ?          Generate debugging information
#     -Ge-       ?          Use the version of the runtime library that assumes a DLL is being built
CFLAGS = -D__DEBUG -DOS2 -DXYZ_DLL -bd -bt=os2 -i=.. -c

LD = wcl386

#LDFLAGS not ported:
#  /nologo /noi /exepack:1 /align:16 /base:0x10000
LDFLAGS = -l=os2v2_dll -fm -"export p_transfer"
OUT=-Fe=
!else

# IBM C/C++
CC = icc -q
CFLAGS =  -Gt- -Ge- -Ti+ -Tx+ -Tm+ -D__DEBUG -Sp1 -Sm -Gm -G4 -Gt -Gd -J -c -DOS2 -DXYZ_DLL -I..
LD = ilink
LDFLAGS = /nologo /noi /exepack:1 /align:16 /base:0x10000 #/debug /dbgpack
OUT=/OUT:
!endif

p2.dll: $(OBJS) $(DEFS)
	$(LD) $(LDFLAGS) $(DEFS) $(OUT)$@ $(OBJS) $(LIBS)

# For IBM ICC:
# p2.dll: $(OBJS) $(DEFS)
# 	$(LD) @<<
#            $(LDFLAGS) $(DEFS) $(OUT)$@ $(OBJS) $(LIBS)
# <<
#         dllrname $@ CPPRMI36=CKO32RTL

pdll_common.obj: ../k95/p_type.h pdll_common.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h \
            ../k95/p.h pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h \
            pdll_global.h pdll_main.h

pdll_crc.obj: ../k95/p_type.h

pdll_dev.obj: ../k95/p_type.h pdll_dev.h pdll_os2incl.h pdll_error.h ../k95/p.h pdll_common.h \
         pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h

pdll_global.obj: ../k95/p_type.h ../k95/p.h

pdll_main.obj: ../k95/p_type.h pdll_common.h pdll_dev.h pdll_os2incl.h pdll_error.h ../k95/p.h \
          pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_defs.h \
          pdll_global.h pdll_ryx.h \
          pdll_rz.h pdll_syx.h pdll_sz.h pdll_x_global.h pdll_z.h pdll_z_global.h

pdll_omalloc.obj: ../k95/p_type.h pdll_error.h ../k95/p.h

pdll_error.obj: ../k95/p_type.h pdll_global.h ../k95/p.h pdll_main.h pdll_common.h

pdll_r.obj: ../k95/p_type.h pdll_common.h pdll_defs.h pdll_global.h ../k95/p.h pdll_omalloc.h pdll_x_global.h \
       pdll_z_global.h pdll_modules.h

pdll_ryx.obj: ../k95/p_type.h pdll_common.h pdll_crc.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h \
         ../k95/p.h pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_global.h \
         pdll_omalloc.h pdll_r.h pdll_x_global.h

pdll_rz.obj: ../k95/p_type.h pdll_common.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h ../k95/p.h \
         pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_global.h \
       pdll_omalloc.h pdll_r.h \
         pdll_z.h pdll_z_global.h

pdll_s.obj: ../k95/p_type.h pdll_global.h ../k95/p.h

pdll_syx.obj: ../k95/p_type.h pdll_common.h pdll_crc.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h \
          ../k95/p.h pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_omalloc.h \
          pdll_global.h pdll_s.h pdll_syx.h pdll_x_global.h

pdll_sz.obj: ../k95/p_type.h pdll_common.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h ../k95/p.h \
         pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_global.h pdll_omalloc.h pdll_s.h \
         pdll_z.h pdll_z_global.h

pdll_tcpipapi.obj: ../k95/p_type.h pdll_error.h ../k95/p.h pdll_modules.h

pdll_exeio.obj: ../k95/p_type.h pdll_common.h ../k95/p.h pdll_global.h pdll_modules.h

pdll_x_global.obj: ../k95/p_type.h

pdll_z.obj: ../k95/p_type.h pdll_common.h pdll_crc.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h \
       ../k95/p.h pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_global.h \
       pdll_z.h pdll_z_global.h

pdll_z_global.obj: ../k95/p_type.h

