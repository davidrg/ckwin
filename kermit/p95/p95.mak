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
!include ..\k95\compiler_detect.mak

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

COMPILER = MSVC

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

SRCS = $(P_SRCS)
OBJS = $(P_OBJS)
LIBS = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
        advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
        msvcrt.lib rpcrt4.lib rpcns4.lib wsock32.lib \
        winmm.lib vdmdbg.lib
DEFS = p95.def

# Visual C++ 2015 refactored the C runtime .lib files - from 2015 onwards we
# must link against ucrt.lib and vcruntime.lib
!if ($(MSC_VER) >= 190)
LIBS = $(LIBS) ucrt.lib vcruntime.lib
!endif

.c.obj:
	$(CC) $(CFLAGS) $(CFLAGSO) /Fo$@ $<

CC = cl
CFLAGS = /nologo /LD /J /c /MD -DOS2 -DNT -DCK_ANSIC -I.. -DXYZ_DLL -DWIN32=1 /Zi
CFLAGSO = /Ot /Oi
CFLAGSD = /Zi
#CFLAGS = /J /c /MT -DOS2 -DNT -DCK_ANSIC -I.. /Zi
LD = link
LDFLAGS = /nologo /dll /nod /map /debug:full
# /align:0x1000 - removed from LDFLAGS as the linker warns about it since
#                 Visual C++ 5.0 SP3 and its almost just a leftover of the
#                 default Visual C++ 4.0 makefile settings

!if ($(MSC_VER) < 140)
# These flags and options are deprecated or unsupported
# from Visual C++ 2005 (v8.0) and up.

# Optimise for Pentium
CFLAGSO = $(CFLAGSO) /G5

# Global Optimizations: This been deprecated since at least Visual C++ 2005.
# Unsure about its status in Visual C++ 2003, but its fine to use in 2002.
CFLAGSO = $(CFLAGSO) /Og

!else

# Docs suggest using /O1 (Minimize Size) or /O2 (Maximise Speed) instead of
# /Og (General Optimizations)
CFLAGSO = $(CFLAGSO) /O2
!endif

p95.dll: $(OBJS) $(DEFS)
	$(LD) @<<
   $(LDFLAGS) /DEF:$(DEFS) /OUT:$@ $(OBJS) $(LIBS)
<<

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

