# CKUKER.MAK, Version 2.21, 28 Jul 89
#
# -- Makefile to build C-Kermit for Unix and Unix-like systems --
#
# Before proceeding, read the instructions below, and also read the file
# ckuker.bwr (the "beware file"), and then rename this file to "makefile"
# or "Makefile" if necessary, and then:
#
# for Alliant FX/8 with Concentrix 4.1, "make bsdlck"
# for Amdahl UTS 2.4 on IBM 370 series & compatible mainframes, "make uts24"
# for Amdahl UTSV IBM 370 series & compatible mainframes, "make sys3"
# for Apollo DOMAIN/IX, "make bsd" or "make sys3", for desired environment
# for Apollo with SR10.0 BSD or later, "make sr10-bsd"
# for Apple Macintosh II with A/UX, "make aux"
# for AT&T 6300 PLUS, "make att6300"
# for AT&T 3B2, 3B20 systems, "make att3bx"
# for AT&T 3B1, 7300 Unix PC, "make sys3upc"
# for AT&T generic System III/System V, "make sys3" or "make sys3nid"
# for AT&T System V R3, use "make sys5r3".  This is different from the above.
# for BBN C/70 with IOS 2.0, "make c70"
# for Bell Unix Version 7 (aka 7th Edition), "make v7" (but see below)
# for Berkeley Unix 4.1, "make bsd41"
# for Berkeley Unix 4.2, "make bsd" (tested with 4.2 and 4.3)
# for Berkeley Unix 4.3, "make bsd43" (uses acucntrl program)
# for Berkeley Unix 4.3 without acucntrl program, "make bsdlck" or "make bsd"
# for Berkeley Unix 4.2 or 4.3 with HoneyDanBer UUCP, "make bsdhdb"
# for Berkeley Unix 2.9 (DEC PDP-11 or Pro-3xx), "make bsd29"
# for Berkeley Unix 2.10, "make bsd210"
# for CDC VX/VE 5.2.1 Sys V emulation, "make vxve"
# for Charles River Data Systems 680x0 systems with Unos, "make sys3nid"
# for CIE Systems 680/20 with Regulus, "make cie"
# for Convex C1, "make convex"
# for DEC Ultrix on VAX or DECstation, "make bsd"
# for DEC Pro-350 with Pro/Venix V1.x, "make provx1"
# for DEC Pro-350 with Pro/Venix V2.0 (Sys V), "make sys3nid" 
# for DEC Pro-380 with Pro/Venix V2.0 (Sys V), "make sys3" or "make sys3nid"
# for Encore Multimax 310, 510 with UMAX 4.2, "make bsd"
# for Encore Multimax 310, 510 with UMAX V 2.2, use Berkeley cc, "make bsd"
# for Fortune 32:16, For:Pro 1.8, "make ft18"
# for HP-9000 Series with HP-UX, "make hpux"
# for IBM 370 Series with IX/370, "make ix370"
# for IBM PS/2 with PS/2 AIX, "make ps2aix"
# for IBM RT PC with AIX 2.1, "make sys3"
# for IBM RT PC with AIX 2.2, "make rtaix" (is one of these extraneous?)
# for IBM RT PC with ACIS, "make bsd"
# for Intel Xenix, "make sco286"
# for Interactive System III (PC/IX) on PC/XT, "make pcix"
# for Interactive Sys III on other systems, "make is3"
# for Masscomp variation on Sys III, "make rtu"
# for Masscomp/Concurrent with RTU 4.0 or later Berkeley, "make rtubsd"
# for Microport Sys V, "make mpsysv"
# for Microsoft,IBM Xenix (/286, PC/AT, etc), "make xenix" or "make sco286"
# for NCR Tower 1632, OS 1.02, "make tower1"
# for NCR Tower 1632 with System V, "make sys3"
# for NeXT, "make next"
# for SCO Xenix 2.2.1 with development system 2.2 on 8086/8 "make sco86"
# for SCO Xenix/286 2.2.1 with development system 2.2 on 80286, "make sco286"
# for SCO Xenix/386 2.2.2, "make sco386"
# for Sequent Balance 8000 or B8 with DYNIX 3.0.14, "make bsdlck"
# for Sequent Symmetry S81 running DYNIX 3.0.12, "make bsdlck"
# for SUN with SUNOS 4.0 or later, "make sunos4" (uses BSD environment)
# for SUN with SUNOS 4.0 or later, "make sys5r3" (uses AT&T environment)
# for Tandy 16/6000 with Xenix 3.0, "make trs16"
# for Valid Scaldstar, "make valid"
# for Zilog ZEUS 3.21, "make zilog"
#
# The result should be a runnable program called "wermit" in the current 
# directory.  After satisfactory testing, you can rename wermit to "kermit" 
# and put it where users can find it.
#
# To remove intermediate and object files, "make clean".
# To run lint on the source files, "make lint".
#
##############################################################################
#
# Notes:
#
#  In many cases, the -O (optimize) compiler switch is omitted.  Feel free
#  to add it if you trust your optimizer.  The ckuus2.c module, in particular,
#  tends to make optimizers blow up.
#
#  "make bsd" should produce a working C-Kermit for 4.1, 4.2, and 4.3bsd on
#  VAX, SUN-3, SUN-4, Pyramid, and other 4.x systems, and also VAX/Ultrix.
#
#  Either "make sys3" or "make sys3nid" tends to produce a working version on
#  any ATT System III or System V R2 or earlier system, including Motorola Four
#  Phase, Callan, Unistar, Cadmus, NCR Tower, HP9836 Series 200, Plexus,
#  Heurikon, etc etc (for exceptions, see below; some AT&T 3Bx systems have 
#  their own entry).  As far as C-Kermit goes, there is no functional 
#  difference between ATT System III and System V R2, so there is no need for 
#  a separate "make sys5" entry (but there is one anyway; it merely invokes
#  "make sys3"). But for ATT System V R3, use "make sys5r3".  This is 
#  different from the above because of the redefinition of signal().
#
#  "make sys3nid" is equivalent to "make sys3" but leaves out the -i option,
#  which is used indicate that separate instruction and data (text) spaces are
#  to be used, as on a PDP-11.  Some systems don't support this option, others
#  may require it.  If one of these options doesn't work on your System III
#  or System V system, try the other.
#
#  For Xenix...  What's Xenix?  There are so many different products & versions
#  sold under this name, the name "xenix" is almost meaningless.  IBM, SCO,
#  Microsoft, etc, IBM Xenix 1.0 =(?) Microsoft Xenix 3.0 = ???  Nevertheless,
#  try "make xenix" for IBM or Microsoft, or "make sco286" or "make sco86" for
#  for SCO Xenix.  If these don't work, try the following modifications to
#  this Makefile:
#    Change "CC= cc" to "CC = cc -LARGE"
#    In the "xenix:" make entry, add "-M2m -UM_I86" to the compiler switches
#    (for the IBM PC family), and "-Mm -lx" to the link switches.
#
#  For Unix Version 7, several variables must be defined to the values
#  associated with your system.  BOOTNAME=/edition7 is the kernel image on
#  okstate's Perkin-Elmer 3230.  Others will probably be /unix.  PROCNAME=proc
#  is the name of the structure assigned to each process on okstate's system.
#  This may be "_proc" or some other variation.  See <sys/proc.h> for more info
#  on your systems name conventions.  NPROCNAME=nproc is the name of a
#  Kernal variable that tells how many "proc" structures there are.  Again
#  this may be different on your system, but nproc will probably be somewhere.
#  The variable NPTYPE is the type of the nproc variable -- int, short, etc.
#  which can probably be gleaned from <sys/param.h>.
#  The definition of DIRECT is a little more complicated.  If nlist() returns,
#  for "proc" only, the address of the array, then you should define DIRECT
#  as it is below.  If however, nlist() returns the address of a pointer to
#  the array, then you should give DIRECT a null definition (DIRECT= ).  The
#  extern declaration in <sys/proc.h> should clarify this for you.  If it
#  is "extern struct proc *proc", then you should NOT define DIRECT.  If it
#  is "extern struct proc proc[]", then you should probably define DIRECT as
#  it is below.  See ckuv7.hlp for further information.
#
#  For 2.9bsd, the makefile may use pcc rather than cc for compiles;
#  that's what the CC and CC2 definitions are for (the current version
#  of the makefile uses cc for both; this was tested and seems to work on
#  the DEC Pro 380).  2.9 support basically follows the 4.1 path.
#  Some 2.9 systems use "dir.h" for the directory header file, others
#  will need to change this to "ndir.h".
#
#  The v7 and 2.9bsd versions assume I&D space on a PDP-11.  When building
#  C-Kermit for v7 on a PDP-11, you should probably add the -i option to
#  the link flags.  Without I&D space, overlays would probably have to be
#  used (or code mapping a`la Pro/Venix if that's available).
#
#  Other systems require some special treatment:
#
#  For Ridge32 (ROS3.2), use "make sys3", but
#  1. Use "CFLAGS = -DUXIII -i -O" "LNKFLAGS = -i"
#  2. Don't #include <sys/file.h> in cku[tf]io.c.
#
#  For Whitechapel MG-1 Genix 1.3, use "make bsd", but
#  1. In ckufio.c, have zkself() return 0 or call getpid, rather than getppid,
#     or simply do "return(kill(0,9))" rather than "return(kill getppid(),1)".
#  2. Wart reportedly can't process ckcpro.w; just work directly from ckcpro.c.
#
#  For Pixel 1000, use "make bsd", do step 1 for Whitechapel MG-1, above.
#
#  For Altos 986 with Xenix 3.0, use "make sys3", but
#  1. Get rid of any "(void)"'s (they're only there for Lint anyway)
#  2. In ckcdeb.h, define CHAR to be "char" rather than "unsigned char".
#
#  For Tandy 6000 running Sys III based Xenix (Xenix 3.xxx), use "make sys3"
#  but insert "#include <sys/types.h>" in any file that that uses the "void"
#  data type.
#
#  Other systems that are close to, but not quite, like Sys III or V, or
#  4.x BSD or V7 -- look at some of the tricks used below and see if you
#  can find a combination that works for you.
#
##############################################################################
#
#  V7-specific variables.
#  These are set up for Perkin-Elmer 3230 V7 Unix:
# 
PROC=proc
DIRECT=
NPROC=nproc
NPTYPE=int
BOOTFILE=/edition7
#
# ( For old Tandy TRS-80 Model 16A or 6000 V7-based Xenix, use PROC=_proc,
#   DIRECT=-DDIRECT, NPROC=_Nproc, NPTYPE=short, BOOTFILE=/xenix )
#
###########################################################################
#
#  Compile and Link variables:
#
LNKFLAGS=
SHAREDLIB=
CC= cc
CC2= cc
#
###########################################################################
#
# Dependencies Section:
#
make:
	@echo 'Make what?  You must tell which system to make C-Kermit for.'

wermit: ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckcpro.o ckcfns.o \
		 ckcfn2.o ckucon.o ckutio.o ckufio.o ckudia.o ckuscr.o
	$(CC2) $(LNKFLAGS) -o wermit ckcmai.o ckutio.o ckufio.o ckcfns.o \
		 ckcfn2.o ckcpro.o ckucmd.o ckuus2.o ckuus3.o ckuusr.o \
		 ckucon.o ckudia.o ckuscr.o $(LIBS)

ckcmai.o: ckcmai.c ckcker.h ckcdeb.h ckcsym.h

ckuusr.o: ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckuus2.o: ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckuus3.o: ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckucmd.o: ckucmd.c ckucmd.h ckcdeb.h

ckcpro.o: ckcpro.c ckcker.h ckcdeb.h

ckcpro.c: ckcpro.w wart
	./wart ckcpro.w ckcpro.c

ckcfns.o: ckcfns.c ckcker.h ckcdeb.h ckcsym.h

ckcfn2.o: ckcfn2.c ckcker.h ckcdeb.h ckcsym.h

ckufio.o: ckufio.c ckcker.h ckcdeb.h

ckutio.o: ckutio.c ckcdeb.h

ckucon.o: ckucon.c ckcker.h ckcdeb.h

wart: ckwart.o
	$(CC) $(LNKFLAGS) -o wart ckwart.o

ckwart.o: ckwart.c

ckudia.o: ckudia.c ckcker.h ckcdeb.h ckucmd.h

ckuscr.o: ckuscr.c ckcker.h ckcdeb.h
#
###########################################################################
#
# Make commands for specific systems:
#
#
#Apple Mac II, A/UX
aux:
	make wermit "CFLAGS = -DAUX -DUXIII -DDEBUG -DTLOG -i -O" \
		"LNKFLAGS = -i"

#Berkeley Unix 4.1
bsd41:
	make wermit "CFLAGS= -DBSD4 -DBSD41 -DDEBUG -DTLOG" "LIBS = -ljobs"

#Berkeley 4.2, 4.3, also Ultrix-32 1.x, 2.0, maybe 3.0, many others
bsd:
	make wermit "CFLAGS= -DBSD4 -DDEBUG -DTLOG"

#Berkeley Unix 4.2 or 4.3 with lock directory /usr/spool/uucp/LCK/LCK..tty??,
#but without acucntrl program
bsdlck:
	make wermit "CFLAGS= -DLCKDIR -DBSD4 -DDEBUG -DTLOG"

#Berkeley Unix with HoneyDanBer UUCP
bsdhdb:
	make wermit "CFLAGS= -DHDBUUCP -DBSD4 -DDEBUG -DTLOG"

#Berkeley Unix 4.3 with acucntrl program
bsd43:
	make wermit "CFLAGS= -DBSD43 -DNEWUUCP -DBSD4 -DDEBUG -DTLOG -O"

#Berkeley Unix 2.8, 2.9 for PDP-11s with I&D space, maybe also Ultrix-11???
#If you have trouble with this, try removing "-l ndir".  If you still have
#trouble, remove "-DDEBUG -DTLOG".  Or try defining CC and/or CC2 as "pcc"
#instead of "cc".
bsd29:
	make wermit "CFLAGS= -DBSD29 -DDEBUG -DTLOG" \
		"LNKFLAGS= -i -lndir" "CC= cc " "CC2= cc"

#Berkeley Unix 2.10 (Stan Barber, sob@bcm.tmc.edu)
bsd210:
	make wermit "CFLAGS= -DBSD29 -DDEBUG -DTLOG" -DLCKDIR \
		"LNKFLAGS= -i " "CC= cc " "CC2= cc"

#Convex C1 with Berkeley Unix
convex:
	make wermit "CFLAGS= -DBSD4 -Dmsleep=mnap -DDEBUG -DTLOG"

#NeXT
next:
	make wermit "CFLAGS= -fwritable-strings -DLCKDIR -DBSD4 -DDEBUG -DTLOG"

#SUN OS version 4.0 or later (like Berkeley, but different signal() type)
sunos4:
	make wermit "CFLAGS= -DBSD4 -DSUNOS4 -DDEBUG -DTLOG"

#Apollo with SR10.0 or later
sr10-bsd:
	make wermit "CFLAGS= -DBSD4 -DDEBUG -DTLOG -Uaegis"

#Version 7 Unix (see comments above)
v7:
	make wermit "CFLAGS=-DV7 -DDEBUG -DTLOG -DPROCNAME=\\\"$(PROC)\\\" \
	-DBOOTNAME=\\\"$(BOOTFILE)\\\" -DNPROCNAME=\\\"$(NPROC)\\\" \
	-DNPTYPE=$(NPTYPE) $(DIRECT)"

#System V R3, some things changed since Sys V R2...
sys5r3:
	make wermit "CFLAGS = -DSVR3 -DUXIII -DDEBUG -DTLOG -i -O" \
		"LNKFLAGS = -i"

#In case they type "make sys5"...
sys5:
	make sys3

#Generic ATT System III or System V (with I&D space)
sys3:
	make wermit "CFLAGS = -DUXIII -DDEBUG -DTLOG -i -O" "LNKFLAGS = -i"

#Generic ATT System III or System V (no I&D space)
sys3nid:
	make wermit "CFLAGS = -DUXIII -DDEBUG -DTLOG -O" "LNKFLAGS ="

#AT&T 6300 PLUS
att6300:
	make wermit "CFLAGS = -DUXIII -DDEBUG -DTLOG -DATT6300 \
		-i -O -Ml" \
		"LNKFLAGS = -i -Ml"

#AT&T 7300/Unix PC systems, sys3 but define symbol ATT7300
sys3upc:
	make wermit "CFLAGS = -DUXIII -DATT7300 -DDEBUG -DTLOG -i -O" \
		"LNKFLAGS = -i"

#AT&T 3B2, 3B20-series computers running System V
#  Only difference from sys3 is lock file stuff...
att3bx:
	make wermit "CFLAGS = -DUXIII -DATT3BX -DDEBUG -DTLOG -i -O" \
		"LNKFLAGS = -i"

#IBM PS/2 with AIX 1.0 (currently in field test as F10A)
#  Reports indicate that -O switch must be omitted.
#  It is also possible that "made bsd" will work (reports welcome).
ps2aix:
	make wermit "CFLAGS = -DUXIII -DDEBUG -DTLOG -i" \
		"LNKFLAGS = -i"

#HP 9000 series 300, 500, 800.
hpux:
	make wermit "CFLAGS = -DUXIII -DHPUX -DDEBUG -DTLOG -O" "LNKFLAGS ="

#Regulus on CIE Systems 680/20
cie:
	make wermit "CFLAGS = -DUXIII -DCIE -DDEBUG -DTLOG -O" "LNKFLAGS ="

#Microport Sys V for IBM PC/AT and clones
mpsysv:
	make wermit "CFLAGS= -O -DXENIX -Dunix -DUXIII -DTLOG -Ml -i" \
		"LNKFLAGS = -Ml -i"

#Microsoft "Xenix/286" e.g. for IBM PC/AT
xenix:
	make wermit "CFLAGS= -DXENIX -Dunix -DUXIII -DDEBUG -DTLOG \
	-F 3000 -i" \
		"LNKFLAGS = -F 3000 -i"

#SCO Xenix/286 2.2.1, e.g. for IBM PC/AT, PS/2 Model 50, etc.
sco286:
	make wermit "CFLAGS= -DXENIX  -Dunix -DUXIII -DDEBUG -DTLOG \
	-F 3000 -i -M2le" \
		"LNKFLAGS = -F 3000 -i -M2le"

#SCO Xenix 2.2.1 for IBM PC, XT, PS2/30, or other 8088 or 8086 machine
sco86:
	make wermit "CFLAGS= -DXENIX  -Dunix -DUXIII -DDEBUG -DTLOG \
	-F 3000 -i -M0me" \
		"LNKFLAGS = -F 3000 -i -M0me"

#SCO Xenix/386 2.2.2
sco386:
	make wermit "CFLAGS= -DXENIX -Dunix -DUXIII -DDEBUG -DTLOG \
	-Otcl  -i -M3e" \
		"LNKFLAGS = -i"

#PC/IX, Interactive Corp System III for IBM PC/XT
pcix:
	make wermit \
	"CFLAGS= -DPCIX -DUXIII -DISIII -DDEBUG -DTLOG -Dsdata=sdatax -O -i" \
		"LNKFLAGS = -i"

#Interactive Corp System III port in general --
is3:
	make wermit \
		"CFLAGS = -DISIII -DUXIII -DDEBUG -DTLOG -Ddata=datax -O -i" \
		"LNKFLAGS = -i"


#Masscomp System III
rtu:
	make wermit "CFLAGS= -UFIONREAD -DUXIII -DDEBUG -DTLOG -O" \
		"LNKFLAGS =" "LIBS= -ljobs"

#Masscomp/Concurrent RTU 4.0 or later, Berkeley environment:
rtubsd:
	ucb make wermit "CFLAGS= -DBSD4 -DRTU -DDEBUG -DTLOG"

#DEC Pro-3xx with Pro/Venix V1.0 or V1.1
# Requires code-mapping on non-I&D-space 11/23 processor, plus some
# fiddling to get interrupt targets into resident code section.
provx1:
	make wart "CFLAGS= -DPROVX1" "LNKFLAGS= "
	make wermit "CFLAGS = -DPROVX1 -DDEBUG -DTLOG -md780" \
		"LNKFLAGS= -u _sleep -lc -md780"

#NCR Tower 1632, OS 1.02
tower1:
	make wermit "CFLAGS= -DDEBUG -DTLOG -DTOWER1"

#Fortune 32:16, For:Pro 1.8 (mostly like 4.1bsd)
ft18:
	make wermit "CFLAGS= -DTLOG -DBSD4 -DFT18"

#Valid Scaldstar
#Berkeleyish, but need to change some variable names.
valid:
	make wermit "CFLAGS= -DBSD4 -Dcc=ccx -DFREAD=1"

#IBM IX/370 on IBM 370 Series mainframes
#Mostly like sys3, but should buffer packets.
ix370:
	make wermit "CFLAGS = -DIX370 -DUXIII -DDEBUG -DTLOG -i -O" \
	"LNKFLAGS = -i"

#Amdahl UTS 2.4 on IBM 370 series compatible mainframes.
#Mostly like V7, but can't do initrawq() buffer peeking.
uts24:
	make wermit "CFLAGS=-DV7 -DDEBUG -DTLOG -DPROCNAME=\\\"$(PROC)\\\" \
	-DUTS24 -DBOOTNAME=\\\"$(BOOTFILE)\\\" -DNPROCNAME=\\\"$(NPROC)\\\" \
	-DNPTYPE=$(NPTYPE) $(DIRECT)"

#BBN C/70 with IOS 2.0
#Mostly Berkeley-like, but with some ATTisms
c70:
	make wermit "CFLAGS= -DBSD4 -DC70 -DDEBUG -DTLOG"

#Zilog ZEUS 3.21
zilog:
	make wermit "CFLAGS = -DUXIII -DZILOG -DTLOG -i -O" \
	"LNKFLAGS = -i -lpw"

#CDC VX/VE 5.2.1
vxve:
	make wermit "CFLAGS = -DUXIII -DVXVE -i -O" "LNKFLAGS = -i"
 
#IBM PC-RT with AIX
rtaix:
	make wermit "CFLAGS = -DUXIII -DRTAIX -DDEBUG -DTLOG -O -w" \
	"LNKFLAGS = -i"
   
#Tandy 16/6000 with Xenix 3.0
trs16:
	make wermit "CFLAGS= -DTRS16 -DXENIX -Dunix -DUXIII -DDEBUG -DTLOG \
	-DM_VOID -Dvoid=int -F 3000 -n" \
		"LNKFLAGS = -F 3000 -n"

#Clean up intermediate and object files
clean:
	-rm -f ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckcpro.o \
	ckcfns.o ckcfn2.o ckucon.o ckutio.o ckufio.o ckudia.o ckuscr.o \
	ckwart.o ckcpro.c

#Run Lint on this mess for the BSD version.
lint:
	-lint -x -DBSD4 -DDEBUG -DTLOG ck[cu]*.[hc] > ck.lint.bsd4

