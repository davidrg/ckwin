# CK9KER.MAK, version 1, 04/30/87 Robert Larson
# based on ckuker.mak
#
# -- Makefile to build C-Kermit for os9/68k
#
# Before proceeding, read the instructions below, and also read the file
# ckuker.bwr (the "beware file"), and then rename this file to "makefile"
# or "Makefile", and then:
# 
# for os9/68k: make
#
##############################################################################
#
# Notes:
#
#	You should remove the -t=/r0 option if you don't have a 
#	ramdisk named /r0.
#
#	Change objdir to where your cmds directory is.
#
###########################################################################
#
#  Compile and Link variables:
#
LNKFLAGS= -i -bg
CC= cc
CC2= cc
CFLAGS= -t=/r0 -dDEBUG -dTLOG
OBJDIR= /dd/cmds
PROGRAM= kermit
#
###########################################################################
#
# Dependencies Section:
#
BIN = ckcmai.r ckucmd.r ckuusr.r ckuus2.r \
   ckuus3.r ckcpro.r ckcfns.r \
   ckcfn2.r ck9con.r ck9tio.r ck9fio.r ckudia.r ckuscr.r

$(OBJDIR)/$(PROGRAM): $(BIN)
 $(CC2) $(LNKFLAGS) -f=$(OBJDIR)/$(PROGRAM) $(BIN)

ckcmai.r: ckcmai.c ckcker.h ckcdeb.h

ckuusr.r: ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckuus2.r: ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckuus3.r: ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckucmd.r: ckucmd.c ckucmd.h ckcdeb.h

ckcpro.r: ckcpro.c ckcker.h ckcdeb.h

ckcpro.c: ckcpro.w $(OBJDIR)/wart
 $(OBJDIR)/wart ckcpro.w ckcpro.c

ckcfns.r: ckcfns.c ckcker.h ckcdeb.h

ckcfn2.r: ckcfn2.c ckcker.h ckcdeb.h

ck9fio.r: ck9fio.c ckcker.h ckcdeb.h

ck9tio.r: ck9tio.c ckcdeb.h

ck9con.r: ck9con.c ckcker.h ckcdeb.h

$(OBJDIR)/wart: ckwart.r
 $(CC) $(LNKFLAGS) -f=$(OBJDIR)/wart ckwart.r

ckwart.r: ckwart.c

ckudia.r: ckudia.c ckcker.h ckcdeb.h

ckuscr.r: ckuscr.c ckcker.h ckcdeb.h

