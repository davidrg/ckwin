# -- Makefile to build C-Kermit for Motorola SYSTEM V/68 R1 --
#
###########################################################################
#
#  Compile and Link variables:
#
LNKFLAGS=
CC2= cc
#
###########################################################################
#
#  Targets
#
all: help
help:
	@echo "You must specify a target: sysv68, sys3, sys3alt, or clean"
	@false
#
#SYSTEM V/68, A&TT System III, or System V (with I&D space)
sysv68: sys3
sys3:
	make kermit "CFLAGS = -DUXIII -DDEBUG -DTLOG -DFP_FILE -i -O" \
		"LNKFLAGS = -i -lc"
#
#System III or System V-like, non-pcc, without TLOG, only buffered IO 
sys3alt:
	make kermit "CFLAGS = -DUXIII -DDEBUG -O" \
		"LNKFLAGS = -lc"
#Cleanup
clean:
	rm -f *.[osL] kermit wart ckcpro.c a.out core
#
###########################################################################
#
#
# Dependencies Section:
#
kermit: ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckcpro.o ckcfns.o \
                 ckcfn2.o ckucon.o ckutio.o ckufio.o ckudia.o ckuscr.o
	$(CC2) -o kermit ckcmai.o ckutio.o ckufio.o ckcfns.o \
                 ckcfn2.o ckcpro.o ckucmd.o ckuus2.o ckuus3.o ckuusr.o \
                 ckucon.o ckudia.o ckuscr.o $(LNKFLAGS)
#
ckcmai.o: ckcmai.c ckcker.h ckcdeb.h
#
ckuusr.o: ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
#
ckuus2.o: ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
#
ckuus3.o: ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
#
ckucmd.o: ckucmd.c ckucmd.h ckcdeb.h
#
ckcpro.o: ckcpro.c ckcker.h ckcdeb.h
#
ckcpro.c: ckcpro.w wart
	./wart ckcpro.w ckcpro.c
#
ckcfns.o: ckcfns.c ckcker.h ckcdeb.h
#
ckcfn2.o: ckcfn2.c ckcker.h ckcdeb.h
#
ckufio.o: ckufio.c ckcker.h ckcdeb.h
#
ckutio.o: ckutio.c ckcdeb.h
#
ckucon.o: ckucon.c ckcker.h ckcdeb.h
#
wart: ckwart.o
	$(CC) -o wart ckwart.o $(LNKFLAGS)
#
ckwart.o: ckwart.c
#
ckudia.o: ckudia.c ckcker.h ckcdeb.h
#
ckuscr.o: ckuscr.c ckcker.h ckcdeb.h
