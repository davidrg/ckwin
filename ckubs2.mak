# CKUBS2.MAK, Thu Aug 22 18:32:45 1996
#
CKVER= "6.0.192 Beta.029"
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
#   6. 2.11BSD includes ctimed and the necessary stub routines.  The 'ctimed'
#      path is in <paths.h>.  The "libstubs.a" (obtained via a "-lstubs" at link
#      time) contains the stub routines.
#
# Authors: Frank da Cruz, Columbia University, fdc@columbia.edu,
# and Steven M Schultz, sms@wlv.iipo.gtegsc.com.
#
# Modified 4 July 1992 to reshuffle overlays (because the first one got too
#   big) and to improve the method of defining the string file.  fdc.
# And again 23 Aug 1992.  fdc.
# And again 06 Sep 1992 to work around ckudia.c blowing up optimizers.  sms.
# And again 09 Sep 1992 to incorporate cku2tm.c and new ckustr.c.  sms.
# & again 19 Sep 92 to add -DMINIDIAL to reduce size of DIAL module.  fdc.
# & again 7 Nov 92 because two of the segments got too big.  fdc.
# & again 15-18 Apr 94, ditto, fdc.
# + again 11-13 Jun 96, for version 6.0.192, fdc.
# 18 Jul 96 to incorporate new 'ctimed' and stubs, sms.
# 22 Aug 96 to reshuffle overlays, fdc.
###########################################################################
#
# 2.10BSD and 2.11BSD (the latter to a larger extent) are the same as 4.3BSD
# but without a large address space.
#
# A string extraction method is used to put approx. 16KB of strings into a
# file.  The module ckustr.c needs to find this file when C-Kermit runs.
# The pathname where this file will reside is defined below (change it if
# necessary).  After make is finished, the file cku192.sr must be moved to
# where ckustr.c has been told to look for it, or you can define an
# environment variable KSTR to override the built-in pathname, for example:
#
#  setenv KSTR `pwd`/cku192.sr
#
# If the resulting wermit program sprews garbage all over your screen, it's
# because it is reading the wrong strings file.
#
# If the resulting wermit program doesn't run at all because UNIX says it
# is too big, it's most likely because the data segment, the root segment,
# or one of the overlays is too big.  The sum of the data (mostly strings.o)
# and bss (mostly static buffers) sizes must be less than about 52K (56K is
# the maximum, but about 4K is needed for stdio buffers that are added in at
# runtime).  If the comibed data+bss size exceeds 52K, start chopping away
# at static buffers.  When it is borderline (> 52K but < 56K), performance
# will be terrible -- screen output will be very slow and jerky because
# stdio functions are doing a system call per character because they could
# not allocate any buffers.
#
# The maximum number of overlays is 15, but the fewer overlays, the better
# the peformance.  The smaller the root segment, the bigger the overlays can
# be:
# 
#   Root   Overlay
#   56KB     8KB
#   48KB    16KB
#   40KB    24KB  <--  This arrangement used in 6.0.192
#   32KB    32KB
#   24KB    40KB
#   16KB    48KB
#    8KB    56KB
#
# The hardest-hit modules should go into the root segment, so top priority
# goes to ckutio and ckufio, the low-level i/o modules.  It would also be
# good to put ckcpro and ckucmd in the root segment but they are too big.
#
# Here is the layout for 6.0.192:
#
# % size wermit
# text    data    bss     dec     hex
# 34368   25574   26414   86356   15154   total text: 126912
#         overlays: 23936,24512,23872,20224
#
# This shows root segment text is less than 40K,
# data+bss is less than 52K, and each overlay is less than 24K.
#
###########################################################################
#
# Compile and Link variables:
#
# EXT is the extension (file type) for object files, normally o.
# EFLAGS is the CFLAGS _without_ the optimize flag (that is added separately).
#  The optimizer can not handle a couple modules (ckcpro.c and ckudia.c).
#  Sometimes this happens silently -- it just dies.
#  In that case there might be a message like:
#    Fatal error in /lib/c2 (which is the optimizer)
#    mv: x.o: Cannot access: No such file or directory
# NOTE: You can't add any more -D's to these because there is already
#  the maximum number of them.  See ckcker.h and ckucmd.h for additional
#  PDP-11 feature disabling.
EXT=o
OPT=-O
EFLAGS=-DBSD43 -DLCKDIR -DNODEBUG -DNOTLOG -DNODIAL \
	-DNOCSETS -DNOHELP -DNOSCRIPT -DNOSPL -DNOXMIT -DNOSETBUF \
	-DNOMSEND -DNOFRILLS -DNOPARSEN -DNOAPC $(KFLAGS) \
	-DSTR_FILE=\\\"/usr/local/lib/cku192.sr\\\"
LNKFLAGS= -i
CC=./ckustr.sed
CC2=cc
#
###########################################################################
#
# Dependencies section and overlay structure.

wermit: ckcmai.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
	ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckcpro.$(EXT) \
	ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
	ckucon.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
	ckuscr.$(EXT) ckcnet.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) \
	ckuusx.$(EXT) ckuusy.$(EXT) ckusig.$(EXT) ckustr.o strings.o
	$(CC2) $(LNKFLAGS) -o wermit \
		ckufio.$(EXT) ckutio.$(EXT) ckcmai.$(EXT) ckusig.$(EXT)  \
		 -Z ckcfns.$(EXT) ckuus3.$(EXT) ckuusy.$(EXT) \
		 -Z ckcpro.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		    ckuus6.$(EXT) ckuus2.$(EXT) \
		 -Z ckucmd.$(EXT) ckuxla.$(EXT) ckuscr.$(EXT) \
		    ckuusr.$(EXT) ckuus7.$(EXT) ckudia.$(EXT) \
		 -Z ckcfn2.$(EXT) ckcfn3.$(EXT) ckucon.$(EXT) \
		    ckcnet.$(EXT) ckuusx.$(EXT) \
		 -Y ckustr.o strings.o $(LIBS)

strings.o: strings
	xstr
	cc -c xs.c
	mv -f xs.o strings.o
	rm -f xs.c

###########################################################################
# Dependencies for each module...
#
ckcmai.$(EXT): ckcmai.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcnet.h ckcsig.h

ckcpro.$(EXT): ckcpro.c ckcker.h ckcdeb.h ckcasc.h
	$(CC) CFLAGS=${EFLAGS} -c ckcpro.c

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

ckuusx.$(EXT): ckuusx.c  ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcsig.h

ckuusy.$(EXT): ckuusy.c  ckcker.h ckcdeb.h ckcasc.h

ckucmd.$(EXT): ckucmd.c ckcasc.h ckucmd.h ckcdeb.h

ckufio.$(EXT): ckufio.c ckcdeb.h ckuver.h

ckutio.$(EXT): ckutio.c ckcdeb.h ckcnet.h ckuver.h

ckucon.$(EXT): ckucon.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsig.h

ckcnet.$(EXT): ckcnet.c ckcdeb.h ckcker.h ckcnet.h ckcsig.h

wart: ckwart.$(EXT)
	$(CC) $(LNKFLAGS) -o wart ckwart.$(EXT)

ckcmdb.$(EXT): ckcmdb.c ckcdeb.h

ckwart.$(EXT): ckwart.c

ckudia.$(EXT): ckudia.c ckcker.h ckcdeb.h ckucmd.h ckcasc.h ckcsig.h
	$(CC) CFLAGS=${EFLAGS} -c ckudia.c

ckuscr.$(EXT): ckuscr.c ckcker.h ckcdeb.h ckcasc.h ckcsig.h

ckusig.$(EXT): ckusig.c ckcsig.h ckcasc.h ckcdeb.h ckcker.h ckcnet.h ckuusr.h

#2.11BSD
#
bsd211:
	@echo "Making C-Kermit $(CKVER) for 2.10/2.11BSD with overlays..."
	@echo -n "Be sure to install cku192.sr with the same pathname"
	@echo " specified in ckustr.c!"
	chmod +x ckustr.sed
	make wermit CFLAGS="${OPT} ${EFLAGS}" LIBS=-lstubs

#2.10BSD
#
bsd210:
	@echo -n "Be sure to install ctimed with the same pathname"
	@echo " specified in ckustr.c for STR_CTIMED!"
	@echo "Making C-Kermit $(CKVER) for 2.10/2.11BSD with overlays..."
	@echo -n "Be sure to install cku192.sr with the same pathname"
	@echo " specified in ckustr.c!"
	chmod +x ckustr.sed
	make wermit CFLAGS="${OPT} ${EFLAGS} \
			-DSTR_CTIMED=\\\"/usr/lib/ctimed\\\""

ctimed:
	$(CC2) $OPT $(EFLAGS) $(LNKFLAGS) -o ctimed cku2tm.c

#Clean up intermediate and object files
clean:
	@echo 'Removing intermediate files...'
	-rm -f *.$(EXT) ckcpro.c wart strings cku192.sr ctimed wermit xs.c
	-rm -f xxmk.c mk.c x.c
