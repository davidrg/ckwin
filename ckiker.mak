# This is a makefile for C Kermit using Amiga Aztec C V3.6.  The lint
# line is for Gimpel Lint V3.03 assuming some kind of standard setup.

CFLAGS = -DAMIGA -DTLOG +l +x5 -E640

LFLAGS =

OBJS = ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckcpro.o ckcfns.o \
	ckcfn2.o ckicon.o ckitio.o ckifio.o ckistu.o ckiutl.o

SRCS = ckcmai.c ckucmd.c ckuusr.c ckuus2.c ckuus3.c ckcpro.c ckcfns.c \
	ckcfn2.c ckicon.c ckitio.c ckifio.c ckistu.c ckiutl.c

kermit:    $(OBJS)
# Building in RAM saves a lot of time, especially on floppies
#	ln -o ram:kermit $(OBJS) -lc32
#	copy ram:kermit\#? ""
#	delete ram:kermit\#?
	ln $(LFLAGS) -o kermit $(OBJS) -lc32

ckcpro.o: ckcpro.w wart
	wart ckcpro.w ckcpro.c
	cc $(CFLAGS) ckcpro.c

wart: ckwart.o
	ln -o wart ckwart.o -lc32

ckuus2.o: ckuus2.c
	cc $(CFLAGS) -Z4000 ckuus2.c

lint: $(SRCS)
	lint -DAMIGA std.lnt $(SRCS)
