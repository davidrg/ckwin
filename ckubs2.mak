# CKUBS2.MAK, Sat Nov  7 22:10:40 1992
#
# Abbreviated version for 2.10 / 2.11 BSD, which chokes on full-size makefile
# because "Make: out of memory".
#
# Instructions:
#   1. Make sure there are no other files called "makefile" or "Makefile"
#      in the same directory.
#   2. Change the name of this file to "makefile".
#   3. Read below about the strings file.
#   4. "make bsd211"
#   5. If you are not on a system with /usr/lib/ctimed (2.10BSD for example),
#      type "make bsd210" (which will compile cku2tm.c into 'ctimed')
#      and then install 'ctimed' in the right place (default is /usr/lib).
#
# Author: Frank da Cruz, Columbia University Center for Computing Activities
# 612 West 115th Street, New York, NY 10025, USA.  Phone (212) 854-5126.
# e-mail: fdc@watsun.cc.columbia.edu, fdc@columbia.edu, or FDCCU@CUVMA.BITNET.
# BSD 2.10/2.11 specifics by Steven M Schultz, sms@wlv.iipo.gtegsc.com.
#
# Modified 4 July 1992 to reshuffle overlays (because the first one got too
#   big) and to improve the method of defining the string file.  fdc.
# And again 23 Aug 1992.  fdc.
# And again 06 Sep 1992 to work around ckudia.c blowing up optimizers.  sms.
# And again 09 Sep 1992 to incorporate cku2tm.c and new ckustr.c.  sms.
# & again 19 Sep 92 to add -DMINIDIAL to reduce size of DIAL module.  fdc.
# & again 7 Nov 92 because two of the segments got too big.  fdc.
#
###########################################################################
#
# 2.10BSD and 2.11BSD (the latter to a larger extent) are the same as 4.3BSD
# but without a large address space.
#
# A string extraction method is used to put approx. 10kb of strings into
# a file. The module ckustr.c needs to find this file when C-Kermit runs.  
# The pathname where this file will reside is defined below (change it if
# necessary).  After make is finished, the file kermit5.sr must be moved
# to where ckustr.c has been told to look for it.
#
# For testing purposes, you can define an environment variable KSTR to
# override the built-in pathname, for example:
#
#  setenv KSTR `pwd`/kermit5.sr
#
# If the resulting wermit program sprews garbage all over your screen, it's
# because it is reading the wrong strings file.
#
# If the resulting wermit program doesn't run at all because UNIX says it
# is out of memory, it's most likely because one of the overlays is too big.
# The maximum size is 48K (49152 bytes) for the base segment and 15K (16384
# bytes) for each overlay.  For example:
#
#  % size wermit
#  48000 + 23702 + 31064
#  16512,15936,15488,11072
#
# This shows that the first overlay is too large.  The cure is move the
# smallest module from the first overlay into the last (smallest) overlay.
# Try hard not to mess with the selection of modules in the root segment.
#
###########################################################################
#
# Compile and Link variables:
#
# EXT is the extension (file type) for object files, normally o.
# DFLAGS is the set of options and flags used for modules that can be
#  processed by the optimizer.
# EFLAGS is the same as DFLAGS except that the -O is removed.
#  Use it for modules that blow up the optimizer.
#
EXT=o
DFLAGS="-O -DBSD43 -DLCKDIR -DNODEBUG -DNOTLOG -DMINIDIAL \
	-DNOCSETS -DNOHELP -DNOSCRIPT -DNOSPL -DNOXMIT -DNOSETBUF $(KFLAGS) \
	-DSTR_FILE=\\\"/usr/local/lib/kermit5.sr\\\" -Dgethostname=gethnam \
	-DSTR_CTIMED=\\\"/usr/lib/ctimed\\\""
EFLAGS=-DBSD43 -DLCKDIR -DNODEBUG -DNOTLOG -DMINIDIAL \
	-DNOCSETS -DNOHELP -DNOSCRIPT -DNOSPL -DNOXMIT -DNOSETBUF $(KFLAGS) \
	-DSTR_FILE=\\\"/usr/local/lib/kermit5.sr\\\" -Dgethostname=gethnam \
	-DSTR_CTIMED=\\\"/usr/lib/ctimed\\\"
LNKFLAGS= -i
CC=./ckustr.sed
CC2=cc
#
###########################################################################
#
# Dependencies Section, including wermit overlay structure.

wermit: ckcmai.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
	ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckcpro.$(EXT) \
	ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
	ckucon.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
	ckuscr.$(EXT) ckcnet.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) \
	ckuusy.$(EXT) ckustr.o strings.o
	ar x /lib/libc.a getpwent.o ndbm.o
	$(CC2) $(LNKFLAGS) -o wermit \
		ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) \
		ckcfn3.$(EXT) \
		 -Z ckcmai.$(EXT) ckuusr.$(EXT) ckucmd.$(EXT) \
		 -Z ckuus4.$(EXT) ckuus5.$(EXT) ckcpro.$(EXT) ckuus6.$(EXT) \
		 -Z ckuus7.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) \
		    ckuusy.$(EXT) \
		 -Z ckuus2.$(EXT) ckuusx.$(EXT) ckucon.$(EXT) \
		    ckuus3.$(EXT) getpwent.o ndbm.o \
		 -Y ckustr.o strings.o $(LIBS)

strings.o: strings
	xstr
	cc -c xs.c
	mv -f xs.o strings.o
	rm -f xs.c

###########################################################################
# Dependencies for each module...
#
ckcmai.$(EXT): ckcmai.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcnet.h

ckcpro.$(EXT): ckcpro.c ckcker.h ckcdeb.h ckcasc.h

ckcpro.c: ckcpro.w wart ckcdeb.h ckcasc.h ckcker.h
	./wart ckcpro.w ckcpro.c

ckcfns.$(EXT): ckcfns.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h \
		ckuxla.h

ckcfn2.$(EXT): ckcfn2.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h ckuxla.h

ckcfn3.$(EXT): ckcfn3.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h \
		ckuxla.h

ckuxla.$(EXT): ckuxla.c ckcker.h ckcdeb.h ckcxla.h ckuxla.h

ckuusr.$(EXT): ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h

ckuus2.$(EXT): ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h

ckuus3.$(EXT): ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h

ckuus4.$(EXT): ckuus4.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h

ckuus5.$(EXT): ckuus5.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcasc.h

ckuus6.$(EXT): ckuus6.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcasc.h

ckuus7.$(EXT): ckuus7.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h

ckuusx.$(EXT): ckuusx.c  ckcker.h ckuusr.h ckcdeb.h ckcasc.h

ckuusy.$(EXT): ckuusy.c  ckcker.h ckcdeb.h ckcasc.h

ckucmd.$(EXT): ckucmd.c ckcasc.h ckucmd.h ckcdeb.h

ckufio.$(EXT): ckufio.c ckcdeb.h ckuver.h

ckutio.$(EXT): ckutio.c ckcdeb.h ckcnet.h ckuver.h

ckucon.$(EXT): ckucon.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h

ckcnet.$(EXT): ckcnet.c ckcdeb.h ckcker.h ckcnet.h

wart: ckwart.$(EXT)
	$(CC) $(LNKFLAGS) -o wart ckwart.$(EXT) $(LIBS)

ckcmdb.$(EXT): ckcmdb.c ckcdeb.h

ckwart.$(EXT): ckwart.c

ckudia.$(EXT): ckudia.c ckcker.h ckcdeb.h ckucmd.h ckcasc.h
	$(CC) CFLAGS=${EFLAGS} -c ckudia.c

ckuscr.$(EXT): ckuscr.c ckcker.h ckcdeb.h ckcasc.h

#2.11BSD
#
bsd211:
	@echo "Making C-Kermit 5A for 2.10/2.11BSD with overlays..."
	@echo -n "Be sure to install kermit5.sr with the same pathname"
	@echo " specified in ckustr.c!"
	chmod +x ckustr.sed
	make wermit CFLAGS=${DFLAGS}

#2.10BSD
#
bsd210:
	@echo -n "Be sure to install ctimed with the same pathname"
	@echo " specified in ckustr.c for STR_CTIMED!"
	make bsd211 ctimed

ctimed:
	$(CC2) $(DFLAGS) $(LNKFLAGS) -o ctimed cku2tm.c

#Clean up intermediate and object files
clean:
	@echo 'Removing intermediate files...'
	-rm -f *.$(EXT) ckcpro.c wart strings kermit5.sr ctimed
