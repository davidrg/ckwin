#This is a sample makefile for Amiga Kermit.
#Since this will not be useful to most people compiling on an Amiga,
#an EXECUTE script for compiling Kermit is in CKIKER.BLD.

#The following define is used to enable the debugging logging.
#KERFLAGS = -DDEBUG -DTLOG
KERFLAGS = 

#The following flags are used with Lattice 3.03.
#LC1FLAGS = -csw -iinclude:lattice/
#LC2FLAGS = -v

#The following flags are used with Lattice 3.10
LC1FLAGS = -csw -DLAT310 -b
LC2FLAGS = -v -r

kermit: ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckipro.o ckcfns.o\
        ckcfn2.o ckicon.o ckitio.o ckifio.o ckistu.o ckiutl.o
 lc:BLink with ckiker.lnk smallcode smalldata verbose nodebug

ckiutl.o: ckiutl.c
 lc:lc1 $(KERFLAGS) -iinclude: $(LC1FLAGS) -oram: ckiutl
 lc:lc2 $(LC2FLAGS) -ockiutl.o ram:ckiutl

ckitio.o: ckitio.c
 lc:lc1 $(KERFLAGS) -iinclude: $(LC1FLAGS) -oram: ckitio
 lc:lc2 $(LC2FLAGS) -ockitio.o ram:ckitio

ckistu.o: ckistu.c ckcker.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckistu
 lc:lc2 $(LC2FLAGS) -ockistu.o ram:ckistu

ckicon.o: ckicon.c ckcker.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckicon
 lc:lc2 $(LC2FLAGS) -ockicon.o ram:ckicon

ckifio.o: ckifio.c ckcker.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckifio
 lc:lc2 $(LC2FLAGS) -ockifio.o ram:ckifio

ckcmai.o: ckcmai.c ckcker.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckcmai
 lc:lc2 $(LC2FLAGS) -ockcmai.o ram:ckcmai

ckcpro.o: ckcpro.c ckcker.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckcpro
 lc:lc2 $(LC2FLAGS) -ockcpro.o ram:ckcpro

ckipro.o: ckipro.c ckcker.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckipro
 lc:lc2 $(LC2FLAGS) -ockipro.o ram:ckipro

ckcfns.o: ckcfns.c ckcker.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckcfns
 lc:lc2 $(LC2FLAGS) -ockcfns.o ram:ckcfns

ckcfn2.o: ckcfn2.c ckcker.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckcfn2
 lc:lc2 $(LC2FLAGS) -ockcfn2.o ram:ckcfn2

ckuusr.o: ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckuusr
 lc:lc2 $(LC2FLAGS) -ockuusr.o ram:ckuusr

ckuus2.o: ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckuus2
 lc:lc2 $(LC2FLAGS) -ockuus2.o ram:ckuus2

ckuus3.o: ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckuus3
 lc:lc2 $(LC2FLAGS) -ockuus3.o ram:ckuus3

ckucmd.o: ckucmd.c ckucmd.h ckcdeb.h
 lc:lc1 $(KERFLAGS) $(LC1FLAGS) -oram: ckucmd
 lc:lc2 $(LC2FLAGS) -ockucmd.o ram:ckucmd
