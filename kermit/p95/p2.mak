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

COMPILER = ICC

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
LIBS = tcp32dll.lib so32dll.lib os2386.lib
DEFS = p2.def

.c.obj:
	$(CC) $(CFLAGS) /Fo$@ $<

CC = icc -q
CFLAGS =  -Gt- -Ge- -Ti+ -Tx+ -Tm+ -D__DEBUG -Sp1 -Sm -Gm -G4 -Gt -Gd -J -c -DOS2 -DXYZ_DLL -I..
LD = ilink
LDFLAGS = /nologo /noi /exepack:1 /align:16 /base:0x10000 #/debug /dbgpack

p2.dll: $(OBJS) $(DEFS)
	$(LD) @<<
           $(LDFLAGS) $(DEFS) /OUT:$@ $(OBJS) $(LIBS)
<<
        dllrname $@ CPPRMI36=CKO32RTL           

pdll_common.obj: ../p_type.h pdll_common.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h \
            ../p.h pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h \
            pdll_global.h pdll_main.h

pdll_crc.obj: ../p_type.h

pdll_dev.obj: ../p_type.h pdll_dev.h pdll_os2incl.h pdll_error.h ../p.h pdll_common.h \
         pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h

pdll_global.obj: ../p_type.h ../p.h

pdll_main.obj: ../p_type.h pdll_common.h pdll_dev.h pdll_os2incl.h pdll_error.h ../p.h \
          pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_defs.h \
          pdll_global.h pdll_ryx.h \
          pdll_rz.h pdll_syx.h pdll_sz.h pdll_x_global.h pdll_z.h pdll_z_global.h

pdll_omalloc.obj: ../p_type.h pdll_error.h ../p.h

pdll_error.obj: ../p_type.h pdll_global.h ../p.h pdll_main.h pdll_common.h

pdll_r.obj: ../p_type.h pdll_common.h pdll_defs.h pdll_global.h ../p.h pdll_omalloc.h pdll_x_global.h \
       pdll_z_global.h pdll_modules.h

pdll_ryx.obj: ../p_type.h pdll_common.h pdll_crc.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h \
         ../p.h pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_global.h \
         pdll_omalloc.h pdll_r.h pdll_x_global.h

pdll_rz.obj: ../p_type.h pdll_common.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h ../p.h \
         pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_global.h \
       pdll_omalloc.h pdll_r.h \
         pdll_z.h pdll_z_global.h

pdll_s.obj: ../p_type.h pdll_global.h ../p.h

pdll_syx.obj: ../p_type.h pdll_common.h pdll_crc.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h \
          ../p.h pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_omalloc.h \
          pdll_global.h pdll_s.h pdll_syx.h pdll_x_global.h

pdll_sz.obj: ../p_type.h pdll_common.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h ../p.h \
         pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_global.h pdll_omalloc.h pdll_s.h \
         pdll_z.h pdll_z_global.h

pdll_tcpipapi.obj: ../p_type.h pdll_error.h ../p.h pdll_modules.h

pdll_exeio.obj: ../p_type.h pdll_common.h ../p.h pdll_global.h pdll_modules.h

pdll_x_global.obj: ../p_type.h

pdll_z.obj: ../p_type.h pdll_common.h pdll_crc.h pdll_defs.h pdll_dev.h pdll_os2incl.h pdll_error.h \
       ../p.h pdll_tcpipapi.h pdll_modules.h pdll_async.h pdll_pipe.h pdll_socket.h pdll_global.h \
       pdll_z.h pdll_z_global.h

pdll_z_global.obj: ../p_type.h

