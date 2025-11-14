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
#		       !! NOTE: GNU MAKE IS REQUIRED !!                               #
#                                                                             #
###############################################################################

# Uncomment for EMX/GCC:
# COMPILER = GCC
# Uncomment for IBM's C/SET++:
# COMPILER = ICC
# Uncomment for MS VC 2.0:
COMPILER = MSVC

P_SRCS = \
common.c \
crc.c \
dev.c \
global.c \
main.c \
omalloc.c \
p_error.c \
r.c \
ryx.c \
rz.c \
s.c \
syx.c \
sz.c \
tcpipapi.c \
x_global.c \
z.c \
z_global.c

P_OBJS = $(P_SRCS:.c=.obj)

SRCS = $(P_SRCS)
OBJS = $(P_OBJS)
LIBS =
DEFS = p.def

ifeq ($(COMPILER), GCC)
%.obj : %.c
	$(CC) $(CFLAGS) $< -o $@

CC = gcc
CFLAGS = -Zomf -Wall -O2 -c
LD = gcc
LDFLAGS = -Zdll -Zomf -Zsys -Zso -s -Zlinker /packc:262144 -Zlinker /packd:262144 -o ../p.dll $(OBJS) $(DEFS)
endif

ifeq ($(COMPILER), ICC)
%.obj : %.c
	$(CC) $(CFLAGS) /Fo$@ $<

CC = icc
CFLAGS = /Q /C /Ge-
LD = link386
LDFLAGS = /nol /noi $(OBJS), p.dll,, $(LIBS), $(DEFS)
endif

ifeq ($(COMPILER), MSVC)
%.obj : %.c
	$(CC) $(CFLAGS) /Fo$@ $<

CC = cl
CFLAGS = /LD /J
LD = link
LDFLAGS = /nol /noi $(OBJS), p.dll,, $(LIBS), $(DEFS)
endif

p.dll: $(OBJS) $(DEFS)
	$(LD) $(LDFLAGS)

# makedepend is a program distributed with X11. It creates automatic
# dependency information. 

depend:
	makedepend -o.obj -- $(CFLAGS) -- $(SRCS)

$(OBJS): makefile

# DO NOT DELETE THIS LINE -- make depend depends on it.

common.obj: typedefs.h common.h defs.h dev.h os2incl.h p_error.h
common.obj: ../exesrc/p.h tcpipapi.h modules.h async.h pipe.h socket.h
common.obj: global.h main.h
crc.obj: typedefs.h
dev.obj: typedefs.h dev.h os2incl.h p_error.h ../exesrc/p.h common.h
dev.obj: tcpipapi.h modules.h async.h pipe.h socket.h
global.obj: typedefs.h ../exesrc/p.h
main.obj: typedefs.h common.h dev.h os2incl.h p_error.h ../exesrc/p.h
main.obj: tcpipapi.h modules.h async.h pipe.h socket.h defs.h global.h ryx.h
main.obj: rz.h syx.h sz.h x_global.h z.h z_global.h
omalloc.obj: typedefs.h p_error.h ../exesrc/p.h
p_error.obj: typedefs.h global.h ../exesrc/p.h main.h common.h
r.obj: typedefs.h common.h defs.h global.h ../exesrc/p.h omalloc.h x_global.h
r.obj: z_global.h modules.h
ryx.obj: typedefs.h common.h crc.h defs.h dev.h os2incl.h p_error.h
ryx.obj: ../exesrc/p.h tcpipapi.h modules.h async.h pipe.h socket.h global.h
ryx.obj: omalloc.h r.h x_global.h
rz.obj: typedefs.h common.h defs.h dev.h os2incl.h p_error.h ../exesrc/p.h
rz.obj: tcpipapi.h modules.h async.h pipe.h socket.h global.h omalloc.h r.h
rz.obj: z.h z_global.h
s.obj: typedefs.h global.h ../exesrc/p.h
syx.obj: typedefs.h common.h crc.h defs.h dev.h os2incl.h p_error.h
syx.obj: ../exesrc/p.h tcpipapi.h modules.h async.h pipe.h socket.h omalloc.h
syx.obj: global.h s.h syx.h x_global.h
sz.obj: typedefs.h common.h defs.h dev.h os2incl.h p_error.h ../exesrc/p.h
sz.obj: tcpipapi.h modules.h async.h pipe.h socket.h global.h omalloc.h s.h
sz.obj: z.h z_global.h
tcpipapi.obj: typedefs.h p_error.h ../exesrc/p.h modules.h
x_global.obj: typedefs.h
z.obj: typedefs.h common.h crc.h defs.h dev.h os2incl.h p_error.h
z.obj: ../exesrc/p.h tcpipapi.h modules.h async.h pipe.h socket.h global.h
z.obj: z.h z_global.h
z_global.obj: typedefs.h
