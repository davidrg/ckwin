# CKOKER.MAK, Version 1.00, 30 June 1988
#
# -- Makefile to build C-Kermit for OS/2 --
#
# Before proceeding, read the instructions below, and also read the file
# ckoker.bwr (the "beware file") if it exists.  Then run MAKE.  Note that
# the MARKEXE program may return an error code -- this can be ignored.
#
# This make file assumes the use of the Microsoft C compiler version 5.1.
# The sources work with the Microsoft OS/2 Software Developer's Kit version
# 1.06 (or later).  You'll need the file SETARGV.OBJ as well as the libraries.
#
# The result is a runnable program called "ckoker.exe" in the current directory.
# After satisfactory testing, you can rename ckoker to "kermit.exe" and put it
# in your utilities directory (or wherever).
#
#

#---------- Macros:

# If a debug log is (is not) wanted, comment out the first (second) line below.
DEBUG=
#DEBUG=/D DEBUG

# If a transaction log is (is not) wanted, comment out the first (second) line below.
#TLOG=
TLOG=/D TLOG

# If Codeview support is (is not) wanted, comment out the first (second) line below.
CVIEW=/O
#CVIEW=/Zi /Od

CMPLFLAGS=/Alfu /FPc /c /Zp $(CVIEW) /D OS2 /D US_CHAR /U MSDOS $(DEBUG) $(TLOG)

#---------- Inference rules:

.c.obj:
	cl $(CMPLFLAGS) $*.c

#---------- Dependencies:
 
ckcmai.obj: ckcmai.c ckcker.h ckcdeb.h ckcsym.h
 
ckuusr.obj: ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
 
ckuus2.obj: ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
 
ckuus3.obj: ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h
 
ckucmd.obj: ckucmd.c ckucmd.h ckcdeb.h
 
ckcpro.obj: ckcpro.c ckcker.h ckcdeb.h
 
ckcfns.obj: ckcfns.c ckcker.h ckcdeb.h ckcsym.h
 
ckcfn2.obj: ckcfn2.c ckcker.h ckcdeb.h ckcsym.h
 
ckofio.obj: ckofio.c ckcker.h ckcdeb.h
 
ckotio.obj: ckotio.c ckcdeb.h
 
ckocon.obj: ckocon.c ckcker.h ckcdeb.h
 
ckudia.obj: ckudia.c ckcker.h ckcdeb.h ckucmd.h
 
ckuscr.obj: ckuscr.c ckcker.h ckcdeb.h

ckoker.exe:	ckcmai.obj ckucmd.obj ckuusr.obj ckuus2.obj ckuus3.obj \
		ckcpro.obj ckcfns.obj ckcfn2.obj ckocon.obj ckotio.obj \
		ckofio.obj ckudia.obj ckuscr.obj
	LINK @ckoker.lnk
	MARKEXE windowcompat ckoker.exe
 

