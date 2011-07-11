# makefile / Makefile / ckuker.mak / CKUKER.MAK
#
# Sun Aug 21 10:33:08 2011
BUILDID=20110821
CKVER= "9.0.302"
#
# -- Makefile to build C-Kermit for UNIX and UNIX-like platforms --
#
# Copyright (C) 1985, 2011,
#   Trustees of Columbia University in the City of New York.
#   All rights reserved.  See the C-Kermit COPYING.TXT file or the
#   copyright text in the ckcmai.c module for disclaimer and permissions.
#   In case you can't find the COPYING.TXT file, it contains the 
#   Simplified 3-Clause BSD License, which is an Open Source license.
#
# Author: Frank da Cruz, Columbia University
# 612 West 115th Street, New York NY 10025-7799, USA
# Email: fdc@columbia.edu
# Web:   http://kermit.columbia.edu/
# FTP:   ftp://kermit.columbia.edu/kermit/
#
# Note: Author is no longer at Columbia University or at the 115th Street
# address effective 1 July 2011.  The email address should still work,
# as well as the website and FTP addresses, for the foreseeable future.
# For new developments, also check:
#
#    http://www.columbia.edu/~fdc/kermit/
#
# Contributions from many others.  Special thanks to Jeff Altman for the
# secure-build targets, Peter Eichhorn, assyst GmbH, for the consolidated
# HP-UX targets and the "uninstall" target, to Robert Lipe for the updated
# and consolidated SCO UNIX / ODT / OSR5 targets, to Ric Anderson for the
# IRIX 6.x targets, to Seth Theriault for major improvements to the
# Mac OS X targets, and to Alexey Dokuchaev for FreeBSD 9.0.
#
# C-Kermit is written and produced by hand without any automated procedures
# such as autoconf / automake / configure, although some of the targets below
# (especially the linux target) inspect the environment and make some
# decisions in the most portable way possible. The automated tools are not
# used because (a) C-Kermit predates them, and (b) they are not portable to
# all the platforms where C-Kermit must be (or once was) built, and (c) to
# keep C-Kermit as independent as possible from external tools over which
# we have no control.
#
# Most entries use the "xermit" target, which uses the select()-based CONNECT
# module, ckucns.c.  The "wermit" target uses the original fork()-based CONNECT
# module, ckucon.c, which has some drawbacks but was portable to every Unix
# variant whether it had TCP/IP or not (select() is part of the TCP/IP
# library, which was not standard on older Unixes).  If your target still uses
# the "wermit" target, please try substituting the "xermit" one and if it
# works, let us know (mailto:kermit-support@columbia.edu).  When changing a
# target over from wermit to xermit, also remove -DNOLOEARN.
#
# CAREFUL: Don't put the lowercase word "if", "define", or "end" as the first
# word after the "#" comment introducer in the makefile, even if it is
# separated by whitespace.  Some versions of "make" understand these as
# directives.  Uppercase letters remove the danger, e.g. "# If you have..."
# 
# WARNING: This is a huge makefile.  Although this is less likely since the
# turn of the century, some "make" programs might run out of memory.  If this
# happens to you, edit away the parts that do not apply to your platform and
# try again.
#
# WARNING 2: In many cases this file invokes itself recursively, sometimes
# several levels deep (as in the Linux targets); i.e. some targets are used
# as 'subroutines' of other targets, with parameters passed by setting
# environment variables.  For that reason, don't use 'make -e'.
#
# Certain UNIX variations have their own separate makefiles:
#  . For 2.10 or 2.11 BSD on the DEC PDP-11, use ckubs2.mak.
#  . For Plan 9, use ckpker.mk.
#
# Separate build procedures are provided non-UNIX platforms: VMS, VOS,
# AOS/VS, etc.  See the ckaaaa.txt file or the Kermit website for details.
#
#
# DIRECTIONS FOR UNIX
#
# Rename this file to "makefile" or "Makefile" if necessary.  Pick out the
# entry most appropriate for your UNIX version from the list below and then
# give the appropriate "make" command, for example "make aix", "make macosx",
# "make linux".  If you experience any difficulties with the build procedure,
# then please also read any comments that accompany the make entry itself
# (search for the make entry name on the left margin).
#
# Other targets:
#  'make install' is an installation script (read accompanying comments!).
#  'make uninstall' undoes 'make install' (read accompanying comments!).
#  'make clean' removes intermediate and object files.
#  'make show' tells the default include and lib paths for secure builds.
#
# IMPORTANT:
#   For more detailed installation instructions, read the files ckuins.txt
#   and ckccfg.txt, also available at the Kermit website in HTML form:
#   http://www.columbia.edu/kermit/ckuins.html
#   http://www.columbia.edu/kermit/ckccfg.html
#
#  For descriptions of known problems and limitations,
#   read the files ckcbwr.txt and ckubwr.txt (the "beware files") or:
#   http://www.columbia.edu/kermit/ckcbwr.html
#   http://www.columbia.edu/kermit/ckubwr.html
#
# Most targets build C-Kermit with its symbol table included.  To reduce the
# size of the executable program, add "LNKFLAGS=-s" to the end of your 'make'
# command or to the makefile entry, or 'strip' the executable after
# building.  To further reduce the size after building, use 'mcs -d' if your
# Unix version has such a command.  For further details on size reduction, read
# ckccfg.txt to find out how to remove features that you don't need.
#
# TCP/IP networking support: If your C-Kermit version does not include TCP/IP
# networking, but your UNIX system does, try adding -DTCPSOCKET to the CFLAGS
# of your makefile entry.  If that doesn't work, look at some of the other
# targets that include this flag for ideas about what libraries might need to
# be included (typically -lsocket and/or -lBSD and/or -lnsl and/or -linet).
# NOTE: In some cases (old versions of SCO or HP-UX), you might need not only
# a C compiler, but also a "TCP/IP developers kit" for the required object
# libraries and header files.
#
# Please report modifications, failures (preferably with fixes) or successes
# to the author, fdc@columbia.edu.
#
# TARGETS FOR DIFFERENT UNIX PLATFORMS AND VERSIONS:
#
# + Marks those that have been built successfully for C-Kermit 9.0 or later.
# - Those that once built OK but no longer do (e.g. too big).
# ? Those that worked in a previous version but have not been tested recently.
# --------------------------
# Some commonly used targets:
#
# + "make linux" should work for any version of Linux on any hardware.
# + "make linux+ssl" ditto, with OpenSSL security added.
# + "make linux+krb5" ditto, with Kerberos 5 security added.
# + "make linux+krb5+ssl" Linux with OpenSSL and Kerberos 5.
# + "make netbsd", NetBSD, any version.
# + "make netbsd+ssl", NetBSD with OpenSSL 0.9.7 or later.
# + "make netbsd+krb5", NetBSD with Kerberos 5.
# + "make netbsd+krb5+ssl", NetBSD with Kerberos 5 and OpenSSL 0.9.7 or later.
# ? "make freebsd1" for FreeBSD 1.x
# ? "make freebsd2" for FreeBSD 2.x
# + "make freebsd3" for FreeBSD 3.x
# ? "make freebsd4" for FreeBSD 4.0
# + "make freebsd", FreeBSD 4.1 or later.
# + "make freebsd+ssl", FreeBSD 5.0 or later with OpenSSL 0.9.7 or later.
# + "make openbsd", OpenBSD 2.3 or later.
# + "make openbsd+ssl", OpenBSD 2.3 or later with OpenSSL 0.9.7 or later.
# + "make mirbsd", MirBSD.
# + "make mirbsd+ssl", MirBSD with OpenSSL 0.9.7 or later.
# + "make macosx" should work for any Mac OS X version 10.3.9 or later.
# + "make macosx+krb5+openssl" Mac OS X 10.3.9 or later + Kerberos V + OpenSSL.
# + "make aix" should work for any version of AIX 4.2 or later.
# + "make aixg" should work for any version of AIX 4.2 or later, using gcc.
# + "make aix+ssl" ditto, with OpenSSL (specifying SSLLIB and SSLINC)
# + "make aix+ibmssl" ditto, with IBM OpenSSL
# + "make solaris9", "make solaris10" for Solaris 9 or 10 with Sun cc.
# + "make solaris9g", "make solaris10g" for Solaris 9 or 10 with gcc.
# + "make solaris11" for Solaris 11 with Sun CC
# + "make solaris11g" for Solaris 11 with gcc
# + "make sco_osr600" for SCO OpenServer 6.0.0.
#
# For other current OSs such as Solaris, HP-UX, and SCO there are separate
# targets for different combinations of OS version and compiler; see the
# complete list.  For older OS's see the complete list.  If an old target
# doesn't work in this release of C-Kermit you can get a previous release from
# the Kermit FTP site: ftp://kermit.columbia.edu/kermit/
#
# SECURE TARGETS (versions that support authentication and encryption)
#  are described after the following list.  Search for ******* below.
#
# --------------------------
# Complete list (alphabetical):
# ? for 386BSD (Jolix) 0.0, 0.1, "make 386bsd" (see comments in entry),
#     or (preferably, if it works) "make bsd44" or "make bsd44c".
# ? for Acorn RISCiX, "make riscix" or "make riscix-gcc"
# ? for Alliant FX/8 with Concentrix 4.1 or later, "make bsdlck"
# ? for Altos 486, 586, 986 with Xenix 3.0, "make altos"
# ? for Altos ACS68000, 8Mhz 68000, UNIX System 3 Rel 2, 512K, "make altos3"
# ? for Amdahl UTS 2.4 on IBM 370 series & compatible mainframes, "make uts24"
# ? for Amdahl UTSV IBM 370 series & compatible mainframes, "make utsv"
# ? for Amdahl UTSV IBM 370 series mainframes with TCP/IP, "make utsvtcp"
# ? for Amdahl mainframes with UNIX System V R 5.2.6b 580, "make sys3"
# ? for Apollo Aegis 9.x, DOMAIN/IX 9.x, "make aegis"
#    (Last tested in C-Kermit 5A(189))
# ? for Apollo DOMAIN/IX, if the above fails, try "make apollobsd"
# ? for Apollo with SR10.0 or later, BSD environment, "make sr10-bsd"
# ? for Apollo with SR10.0 or later, System V environment, "make sr10-s5r3"
# ? for Apple Macintosh II with A/UX pre-3.0, "make aux", "auxgcc" or "auxufs"
# ? for Apple Macintosh with A/UX 3.0 and gcc, "make aux3gcc" or aux3gccc
# ? for Apple PowerMac with MkLinux, "make mklinux" (read Linux entry first)
# ? for Apple PowerMac with LinuxPPC, "make linuxppc"
# ? for Apple Macintosh with Minix 1.5.10, "make minix68k" or "make minixc68"
# ? for Apple Macintosh with Mac OS X 1.0 (Rhapsody), "make macosx10"
#     (no curses), "make macosx10c" (curses), or "make macosx10nc" (ncurses).
#     Or "make macosx10ncx" (ncurses but "make macosx10nc" doesn't work).
# ? for Apple Macintosh with Mac OS X 10.2, "make macosx102nc" (ncurses).
# ? for Apple Macintosh with Mac OS X 10.3, "make macosx103"
# ? for Apple Macintosh with Mac OS X 10.3.9 or later, "make macosx"
# ? for Arix System 90 with AT&T SVR3, "make sys5r3na"
# - for AT&T 6300 with IN/ix, "make sys5"
# - for AT&T 6300 PLUS, "make att6300" or (with no debugging) "make att6300nd"
# ? for AT&T 6386 WGS UNIX PC, "make sys5r3"
# ? for AT&T 3B2, 3B20 systems, "make att3b2".
#   for AT&T 3B1, 7300 UNIX PC (see notes with the entries):
#     In C-Kermit 7.0, only the gcc entries work:
# ?   "make sys3upcg", "make sys3upcgc", "make att351gm"
#    The others fail with "too many defines" (usually in ckuusr.h):
# -   "make sys3upc", "make sys3upcold", "make sys3upcc", "make sys3upcx",
#       "make sys3upcm", "make att351m"
# ? for AT&T System III/System V R2 or earlier, "make sys3" or "make sys3nid"
# ? for AT&T System III/System V with Honey DanBer UUCP, "make sys3hdb"
# ? for AT&T System V on DEC VAX, "make sys3" or "make sys5r3"
# ? for AT&T System V R3, use "make sys5r3" or "make sys5r3c"
# ? for AT&T System V/386 R3.2 built on Interactive 4.1.1, "make sys5r32is".
# ? for AT&T System V/386 R320.0 Versyss Systems, use "make sys5r3"
#     or "make sys5r3c".
# ? for AT&T System V R4, "make sys5r4", "make sys5r4sx", or "make sys5r4nx",
#     or if the ANSI C function prototyping makes trouble, add -DNOANSI,
#     as in "sys5r4sxna" entry
# ? for AT&T (USL) System V R4.2 use the sys5r4* entries.
# ? for Atari Falcon with MiNT, "make posix"
# ? for Atari ST with Minix ST 1.5.10.3, "make minix68k" or "make minixc68"
# ? for BBN C/70 with IOS 2.0, "make c70"
# ? for BeBox with Be OS 1.x DR7, "make beboxdr7"
#     Compiles OK but doesn't link with default linker which is limited to 64K.
#     Links OK with "Code Warrior Gold".  Many hacks in the source code need
#     to be removed when DR8 and later come out.
#     (Last tested in C-Kermit 6.0)
# - for BeBox with Be OS 1.x DR8, "make bebox"
#     (Needed functions missing from operating system and/or not working.)
# - for Bell Labs UNIX Version 6 (6th Edition), there is no makefile entry.
# ? for Bell Labs UNIX Version 7 (7th Edition), "make v7" (but see notes below)
#    (last built successfully in C-Kermit 5A188)
# ? for Bell Labs Research UNIX Version 10, "make bellv10"
#    (last built successfully in C-Kermit 6.0)
# ? for Bell Labs / Lucent Plan 9, use separate makefile ckpker.mk:
#    can be built for Intel, MIPS, 680x0, and PowerPC (last built C-Kermit 7.0)
# ? for BSDI BSD/386 1.x, "make bsdi"
# ? for BSDI BSD/OS 2.x, "make bsdi2"
# ? for BSDI BSD/OS 3.0 or 3.1, "make bsdi3"
# ? for BSDI BSD/OS 4.x, "make bsdi4"
# ? for BSDI BSD/OS 4.x, to build a binary that also works on FreeBSD,
#     "make bsdix".
# ? for Berkeley Unix 2.4, "make v7" (but read v7 material below)
# ? for Berkeley Unix 2.9 (DEC PDP-11 or Pro-3xx), "make bsd29"
# - for Berkeley Unix 2.10, use ckubs2.mak (a separate makefile)
# - for Berkeley Unix 2.11, use ckubs2.mak (a separate makefile)
#     This makefile is too big.  Read the instructions in ckubs2.mak.
#     "make -f ckubs2.mak bsd210" or "make -f ckubs2.mak bsd211".
#     (last built successfully in C-Kermit 6.0 - later versions too big)
# ? for Berkeley Unix 2.11 "make -f ckubs2.mak bsd210noicp" (no command parser)
# ? for Berkeley Unix 4.1, "make bsd41"
# ? for Berkeley Unix 4.2 on VAX, "make bsd42" or "make bsd42c"
# ? for Berkeley Unix 4.2 or 4.3 with HoneyDanBer UUCP, "make bsdhdb"
# ? for Berkeley Unix 4.3 on VAX, "make bsd43", "make bsd43nc".
# ? for Berkeley Unix 4.3 on VAX, no networking "make bsd43nonet.
# ? for Berkeley Unix 4.3 without acucntrl program, "make bsd42" or "bsd42c"
#     NOTE: all the C-Kermit 7.0 full builds for old BSDs fail with
#     "too many defines" in CPP, even on big architectures like VAX.  This
#     can be worked around with a clever ruse.  See comments at target.
# ? for Berkeley Unix 4.3, command-line only, "make bsdm".
# ? for Berkeley Unix 4.3-Tahoe, same as 4.3 BSD
# ? for Berkeley Unix 4.3-Reno, "make bsd43" or "make bsd44" or "make bsd44c"
# ? for Berkeley Unix 4.3-Carson City, "make bsd44" or "make bsd44c"
# ? for Berkeley Unix 4.4-Networking/2 or -Alpha, "make bsd44" or "make bsd44c"
# ? for Berkeley Unix 4.4, "make bsd44" or "make bsd44c"
# ? for Berkeley Unix 4.4-Lite, "make bsd44" or "make bsd44c"
# ? for Bull DPX/2 with BOS/X, "make bulldpx2"
# ? for Cadmus, "make sys3"
#   for Caldera, see SCO, Linux.
# ? for Callan Unistar, "make sys3"
# ? for CDC VX/VE 5.2.1 System V emulation, "make vxve"
# ? for Charles River Data Systems Universe 680x0 with UNOS 9.2, maybe
#     also other UNOS versions, "make crds"
# ? for CIE Systems 680/20 with Regulus, "make cie"
# ? for Commodore Amiga 3000UX Sys V R4, "make sys5r4sx"
# ? for Commodore Amiga 3000UX Sys V R4 and TCP/IP, "make svr4amiganet"
# ? for Commodore Amiga with Minix 1.5.10, "make minix68k" of "make minixc68"
# ? for Concurrent/Masscomp with RTU 4.0 or later, BSD environment, "make
#     rtubsd", "make rtubsd2", "make rtubsd3" (depending on where ndir.h
#     is stored, see entries below).
# ? for Concurrent/Masscomp with RTU 4.0 or later, System V R2, "make rtus5"
# ? for Concurrent (Perkin-Elmer) 3200 series, "make sys5".
# ? for Concurrent (Perkin-Elmer) 3200 series with <dirent.h>, "make ccop1"
# ? for Concurrent PowerMAX OS SVR4, "make powermax"
# ? for Consensys UNIX SV/386 R4V3, "make sys5r4sxtcpc" or "make sys5r4sx"
# ? for Convergent with CTIX Sys V R2, "make sys5"
# ? for Convergent with CTIX 6.4.1, "make ctix"
# ? for Convex C1, "make convex"
# ? for Convex C210 with Convex/OS 8, "make convex8"
# ? for Convex C2 with Convex/OS 9.1, "make convex9"
# ? for Convex C2 with Convex/OS 10.1 and gcc 2.x, "make convex10gcc"
# ? for Cray Research X/MP or YMP or C90 with UNICOS 6.x (System V R3),
#	"make cray"
# ? for Cray Research X/MP or YMP or C90 with UNICOS 7.x (System V R4),
#	"make cray"
# ? for Cray Research X/MP or YMP or C90 with UNICOS 8.0 Alpha, "make cray8"
# ? for Cray Research X/MP or Y-MP or C90 with UNICOS 9.0, "make cray9"
# ? for Cray Computer Cray-2 or Cray3 with CSOS, "make craycsos"
# ? for Cyber 910 (Silicon-Graphics Iris) with Irix 3.3, "irix33"
# ? for Data General AViiON with DG/UX 5.4 before R3.00, "make dgux540"
#     or "make dgux540c" (compile ckwart separately if necessary)
# ? for DG/UX 5.4 on AViiON Intel models, "make dgux540i" or dgux540ic.
# ? for DG/UX 5.4R4.11 on AViiON, all models, "make dgux54411"
# ? for DG/UX 5.4R4.20 on AViiON, all models, "make dgux54420"
# ? for Data General AViiON with DG/UX 4.3x using Sys V-isms, "make dgux430"
# ? for Data General AViiON with DG/UX 4.3x using BSD-isms, "make dgux430bsd"
# ? for Data General AViiON, earlier UNIX versions,
#     "make sys5r3" (maybe compile ckwart separately, or "touch ckcpro.c")
# ? for Data General MV systems with DG/UX, ???
# ? for Data General MV systems with MV/UX, use AOS/VS C-Kermit (CKDKER.MAK)
# ? for Data General MV systems with AOS/VS, use CKDKER.MAK (last = C-K 7.0)
# ? for DEC PDP-11 with Berkeley UNIX 2.x, see Berkeley UNIX 2.x.
# ? for DEC PDP-11 with Mini-UNIX (Bell 6th Edition for PDP-11 with no MMU),
#     probably no way to fit C-Kermit without I&D space.
# ? for DEC PDP-11 with Ultrix-11 3.x, ??? (probably needs overlays)
# ? for DEC VAX with Ultrix 1.x "make bsd"
# ? for DEC VAX with Ultrix 2.x "make ultrix2x"
# ? for DEC VAX or DECstation with Ultrix 3.0, 3.1, "make ultrix3x"
# ? for DECstation or VAX with Ultrix 4.0 or 4.1, "make ultrix40"
# ? for DECstation or VAX with Ultrix 4.2, "make ultrix42" or "make ultrix42c"
# ? for DECstation or VAX with Ultrix 4.x, POSIX world, "make posix"
# ? for DECstation or VAX with Ultrix 4.3, "make ultrix43".
# ? for DECstation or VAX with Ultrix 4.4, "make ultrix44".
# ? for DECstation 5000/50, /150 or /260 (R4x00 MIPS CPU), Ultrix 4.3A or later
#     "make ultrix43-mips3" or "make ultrix43c-mips3"
# ? for DECstation (MIPS) with Berkeley Sprite, "make bsd44"?
# ? for DECstation (MIPS) with OSF/1 V1.0 to 1.3, "make dec-osf"
# ? for DEC Alpha with OSF/1 1.0 to 1.3, "make dec-osf"
# ? for DEC PC 486 with OSF/1, "make dec-osf"
# ? for DEC Alpha with OSF/1 2.x, "make dec-osf20"
# ? for DEC Alpha with OSF/1 3.0, "make dec-osf30"
# ? for DEC Alpha with Digital UNIX 3.2, "make du32"
# ? for DEC Alpha with Digital UNIX 4.0-4.0D, "make du40" or "make du40gcc"
# ? for DEC Alpha with Digital UNIX 4.0E or higher, see Tru64.
# + for DEC Alpha with any version of DU or OSF/1, "make dec-osf1"
# - for DEC Pro-350 with Pro/Venix V1.x, "make provx1" (version 5A is too big)
# ? for DEC Pro-380 with Pro/Venix V2.0 (Sys V), "make sys3" or "make sys3nid"
# ? for DEC Pro-380 with 2.9, 2.10, or 2.11 BSD, "make bsd29" or "make bsd210"
#   for DEC PDP-11 with 2.xBSD (use separate makefile ckubs2.mak)
# ? for Dell UNIX Issue 2.x (= USL Sys V/386 R4.x + fixes), "make dellsys5r4"
#     or "make dellsys5r4c" (last tested in C-Kermit 5A).
# ? for DIAB DS90 with DNIX (any version) create an empty <sys/file.h> if
#     this file does not already exist (or add -DNOFILEH to the make entry).
# ? for DIAB DS90 with DNIX 5.2 (Sys V.2) or earlier, "make dnix",
#     "make dnixnd", or (to add curses and TCP/IP) "make dnixnetc",
# ? for DIAB DS90 with DNIX 5.3 (Sys V.3), "make dnix5r3"
# ? for DIAB DS90 with DNIX 5.3 (Sys V.3) and TCP/IP, "make dnix5r3net"
# ? for DIAB DS90 with DNIX 5.3 2.2 (Sys V.3), ANSI C, "make dnix5r3ansi"
#     or, to include TCP/IP, "make dnix5r3ansinet",
#     but you have to fix a bug in /usr/include/stdlib.h first:
#     change "extern void free(char *str);" to "extern void free(void *str);"
# ? for Dolphin Server Technology Triton 88/17 with SV/88 R3.2, "make sv88r32"
# ? for Encore Multimax 310, 510 with Umax 4.2, "make umax42"
# ? for Encore Multimax 310, 510 with Umax 4.3, "make umax43"
# ? for Encore Multimax 310, 510 with Umax V 2.2, use Berkeley cc, "make bsd"
# ? for Encore 88K with Umax V 5.2, "make encore88k"
# ? for ESIX System V R4.0.3 or 4.04 with TCP/IP support, "make esixr4"
#     NOTE: You can also build on Unixware 2.x with "make esixr4", and run
#     on ESIX, but there you must first:
#       ln /usr/lib/libsocket.so /usr/lib/libsocket.so.1
#       ln /usr/lib/libnsl.so /usr/lib/libnsl.so.1
#     (This worked for C-Kermit 6.0 but does not work for 7.0)
#     (But you can probably still build a non-networking version this way)
# ? for Everex STEP 386/25 Rev G with ESIX Sys V R3.2D, "make sys5r3"
# ? for Fortune 32:16, For:Pro 1.8, "make ft18"
# ? for Fortune 32:16, For:Pro 2.1, "make ft21"
# ? for FPS 500 with FPX 4.1, "made bsd"
# ? for FreeBSD 1.0, "make freebsd1"
# ? for FreeBSD 2.x, "make freebsd2" (ncurses) or "make freebsd2c" (curses)
# ? for FreeBSD 3.x, "make freebsd3" (ncurses) or "make freebsd3c" (curses)
# ? for FreeBSD 4.0, "make freebsd40"
# ? for FreeBSD 4.1 or later, "make freebsd"
# + NOTE: Just use "make freebsd" for any reasonably recent FreeBSD version.
# ? for Harris HCX-2900, "make sys5r3"
# ? for Harris Night Hawk 88K or 68K with CX/UX pre-6.1, "make sys5r3"
# ? for Harris Night Hawk 88K or 68K with CX/UX 6.1 or later, "make cx_ux"
# ? for Heurikon, "make sys3"
# ? for HP-3000, MPE/ix, "make posix"?
# ? for HP-9000 Series 300 with 4.4BSD, "make bsd44"

# NOTE: Most of the HP-UX targets were tested successfully in 2010.
# Verification needed for C-Kermit 9.0 Beta.01...

# ? for HP-9000 Series 500, HP-UX 5.21 and no networking "make hpux0500"
# ? for HP-9000 Series 500, HP-UX 5.21 with WIN/TCP 1.2 "make hpux0500wintcp"
# ? for HP-9000 Series, HP-UX 6.5, without long filenames,
#     "make hpux0650", "make hpux0650c" or "make hpux0650tcpc"
# ? for HP-9000 Series, HP-UX 7.0 or later no long filenames, "make hpux0700sf"
#     or (to include tcp/ip, curses, etc) "make hpux0700sftcpc"
# ? for HP-9000 Series with HP-UX 7.0, TCP/IP,long filenames,"make hpux0700lfn"
# ? for HP-9000 300/400 Series (680x0) with HP-UX 8.0, TCP/IP, "make hpux0800"
#      or "make hpux0800c"
# ? for HP-9000 700/800 Series (PA-RISC), HP-UX 8.0, TCP/IP, "make hpux0800pa"
#      or "make hpux0800pac"
# ? for HP-9000 Series with HP-UX 8.0, no TCP/IP, long filenames,
#      "make hpux0800notcp"
# ? for HP-9000 Series, HP-UX 9.0 - 9.10, TCP/IP, curses, restricted compiler
#     (no optimization, no ANSI), all models, "make hpux0900".  Read the
#     hpux0900 entry below for more info.
# ? for HP-9000 700 and 800 Series, HP-UX 9.x, TCP/IP, curses,
#     HP optimizing ANSI C compiler, "make hpux0900o700".
# ? for HP-9000 with Motorola CPUs, HP-UX 9.x, TCP/IP, curses,
#     HP optimizing ANSI C compiler, "make hpux0900mot".
# ? for HP-9000 on other CPUs, HP-UX 9.x, TCP/IP, curses,
#     HP optimizing ANSI C compiler, "make hpux0900o".
# ? for HP-9000 series, HP-UX 9.x, TCP/IP, curses, gcc, all models,
#     "make hpux0900gcc"
# ? for HP-9000 700/800 Series, HP-UX 10.00,10.01,10.10,10.20,10.30, TCP/IP,
#     curses, restricted compiler (no optimization, no ANSI) "make hpux1000".
# ? for HP-9000 700/800 Series, HP-UX 10.00,10.01,10.10,10.20,10.30, TCP/IP,
#     curses, HP ANSI/optimizing compiler "make hpux1000o" or "make hpux1000o+"
# ? for HP-9000 HP-UX 10.00 or later with gcc, "make hpux1000gcc"
# ? for Trusted HP-UX 10.xx "make hpux1000t", "make hpux1000to",
#     or make hpux1000to+"
# ? for HP-9000 700/800 Series, HP-UX 11.00,TCP/IP,curses, restricted compiler
#     (no optimization, no ANSI) "make hpux1100".
# ? for HP-9000 700/800 Series, HP-UX 11.00,TCP/IP,curses, restricted compiler
#     HP ANSI/optimizing compiler "make hpux1100o" or "make hpux1100o+"
# ? for Trusted HP-UX 11.xx "make hpux1100t", "make hpux1100to",
#     make hpux1100to+"
# ? for HP-9000 PA-RISC models with NeXTSTEP 3.3, "make nextquadfat".
# ? for HP-9000 PA-RISC models with OPENSTEP/Mach 4.1, "make nextquadfat".
# ? for IBM 370 Series with IX/370, "make ix370"
# ? for IBM 370 Series with AIX/370 1.2, "make aix370"
# ? for IBM 370 Series with AIX/370 3.0, "make aix370"
# ? for IBM 370 Series with AIX/ESA 2.1, "make aixesa"
# - for IBM PC/AT 286 & compatibles with Mark Williams Coherent OS,
#     command-line-only version, "make coherent" (version 5A & later too big)
# ? for IBM PC 386 & compatibles with Mark Williams Coherent OS,
#     minimum interactive version, "make coherentmi"
# ? for IBM PC 386 & compatibles with Mark Williams Coherent OS,
#     full interactive version, prior to v4.2, "make coherentmax"
# ? for IBM PC 386 & compatibles with Mark Williams Coherent OS 4.2,
#     "make coherent42"
# ? for IBM PC 386 & compatibles with LynxOS 2.0 or 2.1, "make lynx21"
# ? for IBM PC 386 & compatibles with LynxOS 2.2, "make lynx"
# - for IBM PC/AT & compatibles with original MINIX, "make minix" (too big)
# ? for IBM PC family, 386-based, with MINIX/386 1.5, "make minix386"
#     or if you have GNU CC, "make minix386gcc"
# ? for IBM PC family, 386-based, with MINIX 2.0, "make minix20"
# ? for IBM PC family, 386-based, with MINIX 3.0, "make minix3"
# + for IBM PC family, 386-based, with MINIX 3.0, "make minix315"
# ? for IBM PS/2 with PS/2 AIX 1.0, 1.1, or 1.2, "make ps2aix" or ps2aixnetc.
# ? for IBM PS/2 with PS/2 AIX 1.3, "make ps2aix3"
# ? for IBM RISC System/6000 with AIX 3.0, "make aix30"
# ? for IBM RISC System/6000 with AIX 3.1.x, "make aix31"
# ? for IBM RISC System/6000 with AIX 3.2.0 thru 3.2.5, "make aix32"
# ? for IBM RS/6000 or Power Series with AIX 4.1.x, "make aix41"
# ? for IBM RS/6000 or Power Series with AIX 4.1.x with gcc, "make aix41g"
# ? for IBM RS/6000 or Power Series with AIX 4.1 with X.25, "make aix41x25"
# ? for IBM RS/6000 or Power Series with AIX 4.2 or later: "make aix"
#  (the following "make aixnn" targets are no longer necessary except for gcc)
# ? for IBM RS/6000 or Power Series with AIX 4.2, "make aix42"
# ? for IBM RS/6000 or Power Series with AIX 4.3, "make aix43" (or aix43gcc)
# ? for IBM RS/6000 or Power Series with AIX 4.4, "make aix44" (or aix44gcc)
# ? for IBM RS/6000 or Power Series with AIX 4.5, "make aix45" (or aix45gcc)
# ? for IBM RS/6000 or Power Series with AIX 5.0, "make aix50" (or aix50gcc)
# ? for IBM RS/6000 or Power Series with AIX 5.1, "make aix51" (or aix51gcc)
# ? for IBM RS/6000 or Power Series with AIX 5.2, "make aix52" (or aix52gcc)
# ? for IBM RS/6000 or Power Series with AIX 5.3, "make aix53" (or aix53gcc)
# ? for IBM RS/6000 or Power Series with AIX 6.1, "make aix61" (or aix53gcc)
# ? for IBM RT PC with AIX 2.1, "make sys3"
# ? for IBM RT PC with AIX 2.2.1, "make rtaix" or "make rtaixc"
# ? for IBM RT PC with ACIS 4.2, "make bsd"
# ? for IBM RT PC with ACIS 4.3, "make rtacis" or "make bsd KFLAGS=-DNOANSI"
# ? for IBM RT PC with 4.3BSD/Reno, "make bsd44" or "make bsd44c"
# ? for ICL DRS400 or 400E, "make iclsys5r3"
# ? for ICL DRS3000 (80486) with DRS/NX, "make iclsys5r4_486"
# ? for ICL DRS6000 (SPARC) with DRS/NX, "make iclsys5r4"
# ? for ICL DRS6000 (SPARC) with DRS/NX 4.2MP 7MPlus, "make iclsys5r4m+"
# ?     Ditto but with IKSD support included, "make iclsys5r4m+iksd"
# ? for Integrated Solutions Inc V8S VME 68020, "make isi"
# ? for Intel 302 with Bell Tech Sys V/386 R3.2, "make sys5r3"
# ? for Intel Xenix/286, "make sco286"
# ? for Interactive System III (PC/IX), "make pcix" or "make is3"
# ? for Interactive System III (PC/IX) with gcc, "make is3gcc"
# ? for Interactive 386/ix 1.0.6 with TCP/IP networking, "make is5r3net2"
# ? for Interactive 386/ix 2.0.x, "make is5r3" or (POSIX) "make is5r3p"
# ? for Interactive 386/ix 2.0.x with TCP/IP networking, "make is5r3net"
#     or "make is5r3net2"
# ? for Interactive 386/ix 2.2.1, job control, curses, no net, gcc,
#     "make is5r3gcc"
# ? for Interactive UNIX Sys V R3.2 V2.2 - 4.0 without TCP/IP, "make is5r3jc"
# ? for Interactive UNIX Sys V R3.2 V2.2 - 4.0 with TCP/IP, "make is5r3netjc"
# ? for Intergraph Clipper, "make clix" or "make clixnet"
# ? for Jolix (see 386BSD)
# + for Linux 1.2 and later, "make linux".  Uses ncurses.  This version
#     handles serial speeds up to 460800 bps, Linux FSSTD 1.2, TCP/IP, and
#     should work on both libc and glibc systems.  For static linking, use
#     "make linux LNKFLAGS=-static".  Please read the comments that accompany
#     the linux entry.  As of 8.0.212 Dev.10, this also includes Large File
#     Support (LFS).
# + for Linux builds that fail with "sys/select.h: No such file or directory",
#     "make linuxns"
# + for Linux 1.2 and later but with curses.h and libcurses (rather than
#     ncurses.h and libncurses), use "make linuxc".
# + for Linux 1.2 and later with no curses support at all, "make linuxnc".
# + for Linux with no TCP/IP, "make linuxnotcp"
# (The following Linux targets are historic and might not work...)
# ? for Red Hat Linux 7.1 through RH9, fully configured (krb5, SSL, etc):
#     "make redhat71", "make redhat72", "make redhat73", "make redhat80"
#     "make redhat9"
#     NOTE: You must use this target for Red Hat 7.1 since it
#     also includes a workaround for its broken curses library.
#     WARNING: These targets create binaries that include code for
#     strong encryption and are therefore not exportable. DO NOT PUT
#     THESE BINARIES ON US OR CANADIAN WEB OR FTP SITES.
# ? for Linux on PowerMac (Mklinux DR3), "make mklinux".
# ? for Linux 1.2 and later, to build with egcs, "make linuxegcs".
# ? for Linux with lcc compiler, no TCP/IP, "make linuxnotcp-lcc"
# ? for Linux 1.0 or earlier, "make linux10".
# (End old linux targets)
# ? for Mach 2.6 on (anything, e.g. DECstation), "make bsd42" or "make bsd43".
# ? for MachTen (Tenon) 2.1.1.D on (e.g.) Apple Powerbook, "make machten".
# ? for Masscomp RTU AT&T System III, "make rtu"
#   for other Masscomp, see Concurrent.
# ? for Microport SV/AT (System V R2), "make mpsysv" (last edit tested: 144)
# ? for Microport SVR4 2.2, 3.1, or 4.1 "make sys5r4sx"
# ? for Microsoft,IBM Xenix (/286, PC/AT, etc), "make xenix" or "make sco286"
# ? for MIPS System with RISC/os (UMIPS) 4.52 = AT&T SVR3, "make mips"
#     or "make mipstcpc"
# ? for MkLinux on Power Macintosh, "make mklinux"
# ? for Modcomp 9730, Real/IX, "make sys5r3" (or modify to use gcc = GLS cc)
# ? for Modcomp Realstar 1000 with REAL/IX D.1, "make sv88r32"
# ? for Motorola Four Phase, "make sys3" or "make sys3nid"
# ? for Motorola Delta System V/68 R3, "make sv68r3"
# ? for Motorola Delta System V/68 R3V5, "make sv68r3v5"
# ? for Motorola Delta System V/68 R3V5.1, "make sv68r3v51"
# ? for Motorola Delta System V/68 R3V6 with NSE TCP/IP, "make sv68r3v6"
# ? for Motorola Delta System V/88 R32, "make sv88r32"
# ? for Motorola Delta System V/88 R40, "make sv88r40"
# ? for Mt Xinu Mach386 on 386/486-based PCs, "make bsd43"
# ? for NCR Tower 1632, OS 1.02, "make tower1"
# ? for NCR Tower 1632 or Minitower with System V R2, "make sys3"
#     or "make sys3nv"
# ? for NCR Tower 32, OS Release 1.xx.xx, "make tower32-1"
# ? for NCR Tower 32, OS Release 2.xx.xx, "make tower32-2"
# ? for NCR Tower 32, OS Releases based on Sys V R3, "make tower32"
# ? for NCR Tower 32, OS Releases based on Sys V R3 with gcc "make tower32g"
# ? for NCR System 3000, AT&T UNIX System V R4 2.0, "make sys5r4sxna"
# ? for NCR System 3000, AT&T UNIX System V R4 2.0 with Wollongong TCP/IP,
#     "make sys5r4net2" or "make sys5r4net2c".
#      Some header files might be misplaced; try this:
#       ln /usr/include/netinet/in.h /usr/include/sys/in.h
#       ln /usr/include/arpa/inet.h /usr/include/sys/inet.h
#       ln /usr/include/sys/termiox.h /usr/include/termiox.h
# ? for NCR System 3000, NCR UNIX 02.02.01, same as above.
# ? for NCR MP-RAS System V R4 V2.03 or 3.02, "make mpras" or "make mprastcpc"
# + for NetBSD any version on any architecture, "make netbsd"
# + for NetBSD with OpenSSL, "make netbsd+ssl"
# ? for NetBSD with ncurses specified instead of curses, "make netbsdn"
# ? for NetBSD with all curses support omitted, "make netbsdnc"
# ? for NeXT with NeXTSTEP 1.0 through 3.2, "make next" (on a NeXT)
# ? for NeXT with NeXTSTEP 3.3, "make next33"
# ? for NeXT with OPENSTEP/Mach 4.1, "make nextquadfat".
# ? for NeXT with OPENSTEP/Mach 4.2, "make openstep42".
# ? for NeXTSTEP/486, "make next" (on a PC)
# ? for NeXTSTEP portable binary (runs on Intel or Motorola), "make nextfat"
# ? for NeXTSTEP portable binary (Intel, Motorola, HP PA-RISC, or SPARC),
#     "make nextquadfat"
# ? for Nixdorf Targon/31, "make t31tos40x"
# ? for Norsk Data Uniline 88/17 with SV/88 R3.2, "make sv88r32"
#   for Novell UnixWare - see UnixWare
# ? for OSF/1 (vanilla, from OS/F), "make posix"
# ? for OkiStation 7300 Series, "make sys5r4sxtcp"
# ? for Olivetti LSX-3020 with X/OS R.2.3, "make xos23" or "make xos23c"
# + for OpenBSD, "make openbsd" (also see secure targets listed below).
# ? for OPENSTEP/Mach 4.1, "make nextquadfat" (NeXT, Intel, PA-RISC, SPARC)
# ? for OPENSTEP/Mach 4.2, "make openstep42" (tested on NeXT)
# ? for Perkin-Elmer (Concurrent) 3200 series, "make sys5".
# ? for Perkin-Elmer (Concurrent) 3200 series with <dirent.h>, "make ccop1"
# ? for Perkin-Elmer/Concurrent 3200 with Xelos R02, "make ccop1"
# ? for PFU Compact A Series SX/A TISP V10/E50 (Japan), "make sxae50"
# ? for Plexus, "make sys3"
# ? for Pyramid 9XXX (e.g. 9845) or MIServer T series, OSx 4.4b thru 5.1,
#     "ucb make pyramid" or for HDB UUCP, "ucb make pyramid-hdb" or:
# ? for Pyramid MIServer S or ES Series, DataCenter/OSx, "make pyrdcosx"
# ? for Pyramid MIS-S MIPS R3000, DataCenter OSx System V R4, "make pyrdcosx"
# ? for POSIX on anything, "make posix" (but adjustments might be necessary).
#      NOTE: this target is not very useful - many features are missing.
# ? for Prime 8000 MIPS, SVR3, "make mips" or "make mipstcpc"
# - for QNX 2.x (sorry we don't have a version of C-Kermit for QNX 2.x)
# ? for QNX 4.0 or 4.1, 16-bit, on 286 PC, Watcom C 8.5, "make qnx16_41"
# ? for QNX 4.21 - 4.22A (286+), and 4.23 (386+), or higher, 16-bit,
#     Watcom C 9.5x or higher, "make qnx16"
# + for QNX 4.21-4.25, 32-bit, 386 or above, Watcom C 10.6, "make qnx32"
#     NOTE: ("make qnx" == "make qnx32")
# ? for QNX Neutrino 2+, "make qnx_nto2+" (crosscompiled on QNX4 with Watcom C)
# ? for QNX 6 = Neutrino 2.xx, "make qnx6"
# ? for Ridge 32 (ROS3.2), "make ridge32"
# ? for Samsung MagicStation, "make sys5r4"
# ? for SCO Xenix 2.2.1 with development system 2.2 on 8086/8 "make sco86"
# ? for SCO Xenix/286 2.2.1 with development system 2.2 on 80286, "make sco286"
#     NOTE: reportedly this makefile is too long for SCO Xenix/286 make, but it
#     works with "makeL", or if some of the other make entries are edited out.
# ? for SCO Xenix/386 2.2.2, "make sco386"
# ? for SCO Xenix/386 2.3.x, "make sco3r2"
# ? for SCO Xenix/386 SCO 2.3.3 or 2.3.4 with gcc 1.37 or later,
#     "make sco386gcc" or (to add curses) "make sco386gccc".
# ? for SCO Xenix/386 or UNIX/386 with Excelan TCP/IP, "make sco3r2net"
#     or (to add curses support) "make sco3r2netc" or "sco386netc"
# + for SCO XENIX 2.3.4, "make sco234" or "make sco234c" to add curses.
# ? for SCO XENIX 2.3.4 with SCO TCP/IP & curses, "make sco234netc".
# ? for SCO Xenix 2.3.x with Racal-InterLan TCP/IP, "make sco3r2netri"
#   for other UNIX varieties with Racal Interlan TCP/IP, read sco3r2netri entry
# ? for SCO Xenix 2.3.x with SCO (Lachman) TCP/IP, "make sco3r2lai"
#     or (to add curses) "make sco3r2laic"
#   for SCO UNIX...  ALSO READ COMMENTS in the SCO UNIX entries for more info!
# ? for SCO UNIX/386 3.2.0 or 3.2.1, "make sco3r2" or "make sco3r2x"
# ? for SCO UNIX/386 3.2.2, "make sco3r22" or "make sco3r22gcc"
#     or "make sco3r22c"
# ? for SCO UNIX/386 3.2.2 with SCO TCP/IP, "make sco3r22net"
#     or "make sco3r22netc" (curses)
# ? for SCO ODT 1.1, "make sco3r22net" or "make sco3r22netc" (curses)
# ? for SCO UNIX/386 3.2 V4.x, no network support, "make sco32v4"
# ?   or "make sco32v4ns" (this one uses no select() or sockets library)
# ? for SCO UNIX/386 3.2 V4.x with TCP/IP, "make sco32v4net"
#     (also sco32v4gcc, sco32v4netgcc)
# ? for SCO UNIX/386 3.2 V5.0 - see SCO OpenServer.
# ? for SCO UNIX 3.2v4.x with TCP/IP, <dirent.h> for Extended Acer File
#     System (EAFS), curses, ANSI C compilation, "make sco32v4net"
# ?   or (to use select()-based CONNECT module) "make sco32v4netx".
# ? for SCO UNIX 3.2v4.2, "make sco-odt30" (includes TCP/IP).
# ? for SCO MPX 3.0 - The SCO UNIX binary runs on the corresponding MPX system.
#
# NOTE: Also see below for other entries that are variations on these.
# Also be sure to read the comments accompanying each SCO entry.
# Also see Unixware section.
#
# ? for SCO ODT 2.0, "make sco32v4net"
# ? for SCO ODT 3.0, "make sco-odt30"
# ? for SCO OpenServer 5.0 (OSR5), "make sco32v500"
# ? for SCO OpenServer 5.0 (OSR5) with networking, "make sco32v500net"
# ? for SCO OpenServer 5.0 (OSR5), gcc, "make sco32v500gcc"
# ? for SCO OpenServer 5.0 (OSR5), gcc, with networking, "make sco32v500netgcc"
# ? for SCO OpenServer 5.0 (OSR5), as above, ELF, "make sco32v500netgccelf"
# ? for SCO OpenServer 5.0.2, use "make sco32v502xxx" entries as above.
# ? for SCO OpenServer 5.0.4, use "make sco32v504xxx" entries as above.
# ? for SCO OpenServer 5.0.5, use "make sco32v505xxx" entries as above.
#     Use the sco32v505udkxxx entries if you have the UDK rather than /bin/cc.
# ? for SCO OpenServer 5.0.6, use "make sco32v506xxx" entries as above.
# ? for SCO OpenServer 5.0.6a,use "make sco32v506axxx" entries as above.
# ? for SCO OpenServer 5.0.7, use "make sco32v507", "make sco32v507net"
# ? for SCO (Univel) UnixWare 1.x, "make unixware" or "make unixwarenetc".
#     If there are problems with this in C-K 7+ see notes at unixware entry.
# + for SCO OpenServer 6.0.0, "make sco_osr600"
# ? for SCO UnixWare 2.0.x, "make uw20"
# ? for SCO UnixWare 2.1.0, "make uw21"
# ? for SCO UnixWare 2.1.3, "make uw213"
# + for SCO UnixWare 7, "make uw7" (includes large file support)
# ? for SCO UnixWare 7 with IKSD support, "make uw7iksd" or "make uw7iksdudk"
# ? for SCO UnixWare 7 with OpenSSL, "make uw7ssl"
# ? for SCO (Caldera) Open UNIX 8, "make ou8"
# ? for Sharp Zaurus SL5500 PDA, "make zsl5500".
# ? for Sequent with DYNIX/ptx 1.2.1, "make dynixptx12"
# ? for Sequent with DYNIX/ptx 1.3 or 1.4 with TCP/IP, "make dynixptx13"
# ? for Sequent with DYNIX/ptx 2.0 or 2.1 with TCP/IP, "make dynixptx20"
#     or "dynixptx20c"
# ? for Sequent with DYNIX/ptx 2.1.6 on i486, "dynixptx216c"
# ? for Sequent with DYNIX/ptx V4.1.3 with TCP/IP, "make dynixptx41c"
# ? for Sequent with DYNIX/ptx V4.4.2 with TCP/IP, "make dynixptx44"
# ? for Sequent Balance 8000 or B8 with DYNIX 3.0.xx, "make dynix3"
#    or "make dynix3noacu"
# ? for Sequent Symmetry S81 with DYNIX 3.0.xx, "make dynix3"
# ? for Sequent DYNIX 3.1.xx, "make dynix31" or "make dynix31c"
# ? for Siemens/Nixdorf SINIX-L Intel V5.41, "make sinix541i"
# + for Siemens/Nixdorf SINIX-N MIPS V5.42, "make sinix542"
# ? for Siemens/Nixdorf SINIX-P MIPS V5.42 with gcc, "make sinix542g"
# ? for Siemens/Nixdorf SINIX-Z Intel V5.42, "make sinix542i"
# ? for Siemens/Nixdorf Reliant UNIX V5.43, "make sni543"
# ? for Siemens/Nixdorf Reliant UNIX V5.44, "make sni544"
# ? for Silicon Graphics Iris System V IRIX 3.2 or earlier, "make iris"
# ? for Silicon Graphics Sys V R3 with IRIX 3.3 or later, "make sys5r3"
# ? for Silicon Graphics Iris Indigo with IRIX 4.0 or 5.0, "make irix40" or
#     (to include Yellow Pages and Curses) "make irix40ypc"
# ? for Silicon Graphics Iris Indigo or Elan with IRIX 4.0.x with microcode
#     optimization and -O4, "make irix40u" or "irix40uc" (and read notes
#     accompanying these entries).
# ? for Silicon Graphics IRIX 5.1, "make irix51" or "irix51x" (no optimize)
# ? for Silicon Graphics IRIX 5.2, "make irix52"
# ? for Silicon Graphics IRIX 5.3, "make irix53" or "irix53x" (no optimize)
# ? for Silicon Graphics IRIX 6.0, "make irix60".
# ? for Silicon Graphics IRIX 6.2, "make irix62".
# ? for Silicon Graphics IRIX 6.3, "make irix63".
# ? for Silicon Graphics IRIX 6.4, "make irix64" or "make irix64gcc".
# + for Silicon Graphics (SGI) IRIX 6.5, "make irix65"
# +   or "make irix65mips2" to force MIPS2, or "make irix65gcc" for GCC.
# + for Silicon Graphics (SGI) IRIX 6.5, "make irix65" or "make irix65mips2"
# ? for SGI IRIX 6.5 with SSL/TLS, SRP, and ZLIB "make irix65+ssl+srp+zlib"
# ? for Solaris 2.0-2.3 on SPARC or Intel, SunPro CC, "make solaris2x",
# ?   or to add SunLink X.25 8.0x support, "make solaris2x25".
# ? for Solaris 2.4 built with gcc, "make solaris24g".
# ? for Solaris 2.0-2.3 on SPARC or Intel, GNU CC, "make solaris2xg".
# ? for Solaris 2.4 with X.25, "make solaris24x25".
# ? for Solaris 2.5 on SPARC or Intel, SunPro CC, "make solaris25".
# ?   or to add SunLink X.25 8.0x support, "make solaris25x25".
# ? for Solaris 2.5 on SPARC or Intel, GNU CC, "make solaris25g".
# ? for Solaris 2.6 on SPARC or Intel, "make solaris26".
# ? for Solaris 7 on SPARC or Intel, SunPro CC, "make solaris7".
# ? for Solaris 7 on SPARC or Intel, GNU CC, "make solaris7g".
# ? for Solaris 8 on SPARC or Intel, SunPro CC, "make solaris8".
# ? for Solaris 8 on SPARC or Intel, GNU CC, "make solaris8g".
# + for Solaris 9 on SPARC (or Intel?), 32-bit, SunPro CC, "make solaris9".
# + for Solaris 9 on SPARC (or Intel?), 32-bit, GNU CC, "make solaris9g".
# ? for Solaris 9 on SPARC (or Intel?), 64-bit, GNU CC, "make solaris9g64".
# + for Solaris 10 on SPARC (or Intel?), 32-bit, SunPro CC, "make solaris10".
# + for Solaris 10 on SPARC 64-bit, SunPro CC, "make solaris10_64".
# + for Solaris 10 on SPARC (or Intel?), 32-bit, GNU CC, "make solaris10g".
# ? for Solaris 10 on SPARC (or Intel?), 64-bit, GNU CC, "make solaris10g64".
# ? for Solbourne 4/500 with OS/MP 4 "make sunos4"
# ? for Solbourne 4/500 with OS/MP 4.1 "make sunos41" or "make sunos41c"
# ? for SONY NEWS with NEWS-OS 4.0.1C, "make sonynews"
# ? for SONY NEWS with NEWS-OS 4.1.2C, "make sonynews"
# ? for Sperry/UNISYS 5000/20, UTS V 5.2 3R1, "make sys5"
# ? for Sperry/UNISYS 5000/30/35/50/55, UTS V 5.2 2.01, "make unisys5r2"
# ? for Sperry/UNISYS 5000/80 with System V R3, "make sys5r3"
# ? for Sperry/UNISYS 5000/95 with System V R3, "make sys5r3"
#     For UNISYS SVR3 it might be necessary to "make sys5r3 KFLAGS=-UDYNAMIC"
# ? for Stardent 1520, "make sys5r3"
# ? for Stratus FTX 2.x, try "make ftx" or else "make sys5r4" or "sys5r4sx"
# ? for Stratus FTX 3.x, PA-RISC 1.0 or 2.0, "make ftx" or "make ftxtcp"
# ? for Sun with Sun UNIX 3.5 and gcc, "make sunos3gcc"
# ? for Sun with pre-4.0 SunOS versions, "make bsd" (or appropriate variant)
# ? for Sun with SunOS 4.0, BSD environment, "make sunos4"
# ? for Sun with SunOS 4.0, BSD, with SunLink X.25, make sunos4x25
# ? for Sun with SunOS 4.1 or 4.1.1, BSD environment, "make sunos41"
#     or "make sunos41c" (curses) or "make sunos41gcc" (compile with gcc)
# ? for Sun with SunOS 4.1.x, BSD, with SunLink X.25 7.00 or earlier,
#     "make sunos41x25" or "make sunos41x25c" (curses)
# ? for Sun with SunOS 4.1, 4.1.1, AT&T Sys V R3 environment, "make sunos41s5"
# ? for Sun with SunOS 4.1.2, "make sunos41" or any of its variations.
#     NOTE:  All SunOS 4.x systems -- Shared libraries are used by default.
#       If this causes problems, add -Bstatic to CFLAGS.
#     NOTE2: When building C-Kermit under SunOS for the BSD universe,
#       but /usr/5bin/cc is ahead of /usr/ucb/cc in your PATH, add
#       "CC=/usr/ucb/cc CC2=/usr/ucb/cc" to the make entry.
#     NOTE3: If an executable built on one type of Sun hardware does not work
#       on another type, rebuild the program from source on the target machine.
#   for Sun with Solaris 1.x use SunOS 4.1 entries.
#   for Sun with Solaris 2.0 and higher use Solaris entries.
# + for Sun SPARC with Linux, "make linux"
# ? for Sun SPARC with OPENSTEP/Mach 4.1, "make nextquadfat"
# ? for Sun SPARC with OPENSTEP/Mach 4.2, "make openstep42"
# - for Tandy 16/6000 with Xenix 3.0, "make trs16" (C-Kermit 7.0 is too big)
# ? for Tektronix 6130/4132/43xx (e.g.4301) with UTek OS, "make utek"
#     or (for models without hardware flow control), "make uteknohwfc"
# ? for Tektronix XD88 series with UTekV OS, "make utekvr3"
# ? for Tri Star Flash Cache with Esix SVR3.2, "make sys5r3"
# NOTE: The Tru64 builds have been failing since 2010, but "make dec-osf" is OK
# ? for Tru64 UNIX 4.0E, "make tru64-40e"
# ? for Tru64 UNIX 4.0F, "make tru64-40f"
# ? for Tru64 UNIX 4.0G, "make tru64-40g"
# ? for Tru64 UNIX 5.0A, "make tru64-50a"
# ? for Tru64 UNIX 5.1A, "make tru64-51a"
# ? for Tru64 UNIX 5.1B, "make tru64-51b"
# ? for Unistar, "make sys5"
# ? for Unisys S/4040 68040 CTIX SVR3.2 6.4.1, "make ctix" or "make sys5r3"
# ? for Unisys U5000 UNIX SVR3 6.x, "make sys5r3" or "make sys5r3c"
# ? for Unisys U6000 UNIX SVR4 1.x, "make sys5r4nx" or "make sys5r4nxnetc"
#   for Unisys ... (also see Sperry)
#   for Univel - see UnixWare
#   for Unixware - see SCO
# ? for Valid Scaldstar, "make valid"
# ? for Whitechapel MG01 Genix 1.3, "make white"
# ? for Zilog ZEUS 3.21, "make zilog"
#
# The result should be a runnable program called "wermit" in the current
# directory.  After satisfactory testing, you can rename wermit to "kermit"
# and put it in some directory that's in everybody's PATH, such as
# /usr/local or /opt/local.
#
# To remove intermediate and object files, "make clean".
# If your C compiler produces files with an extension other than "o",
# then "make clean EXT=u", "make clean EXT=s", or whatever.
#
# To run lint on the source files, "make lintsun", "make lintbsd",
# "make lints5", as appropriate.
#
# ******************************
# SECURE TARGETS
#
# Beginning with C-Kermit 7.0, secure targets are included, as are the
# source modules (ckuat*.[ch], ck_*.[ch]) needed to build them.  Secure
# target names are like the regular names, but with security features
# indicated by plus (+) signs.  The features are:
#
# krb4     MIT Kerberos IV
# krb5     MIT Kerberos V
# openssl  OpenSSL (SSL/TLS)
# zlib     ZLIB compression for SSL/TLS
# srp      Stanford Secure Remote Password
# pam      PAM (pluggable authentication module)
# shadow   Shadow Password File
#
# You can build these targets if you have the Kermit source files and the
# required libraries (Kerberos, OpenSSL, SRP, etc) and header files.  See:
#   http://www.columbia.edu/kermit/security.html
# for specific details regarding supported versions.
#
# NOTE: OpenSSL 0.9.6 and earlier are not compatible with 0.9.7 and later.
# C-Kermit code was originally designed for 0.9.6.  To build with 0.9.7 you
# must add -DOPENSSL_097 to avoid missing symbols in the DES library and to
# use the entry points that were renamed to avoid conflict with Kerberos 4.
# If you have OpenSSL 0.9.8, add -DOPENSSL_098, which is a synonym for
# -DOPENSSL_097.  If you have 1.0.0 or later, add -DOPENSSL_100, which is
# another synonym.

# In OpenSSL builds add -ldl if you get unresolved references for
# dlopen, dlclose, dlsym, and/or dlerror.
#
# In order to build a secure version of Kermit, you need to know the location
# of the header (include) files and libraries for the desired form of
# security.  Unless you specify a location, this makefile looks in /usr/local
# and if the required files are not found, the build fails.
#
# If the secure headers and libraries are not on your computer, you have
# to download and install them, for example from http://www.openssl.org .
#
# The following symbols are used to specify library and header file locations:
# 
prefix  = /usr/local
srproot = $(prefix)
sslroot = $(prefix)
manroot = $(prefix)

K4LIB=-L/usr/kerberos/lib
K4INC=-I/usr/kerberos/include
K5LIB=-L/usr/kerberos/lib
K5INC=-I/usr/kerberos/include
SRPLIB=-L$(srproot)/lib
SRPINC=-I$(srproot)/include
SSLLIB=-L$(sslroot)/ssl/lib
SSLINC=-I$(sslroot)/ssl/include

# To override these assignments; for example, if your OpenSSL files are
# not in /usr/local/ssl, invoke the desired target like this:
# 
#  make solaris9+openssl "SSLINC=-I/opt/openssl-0.9.8k/include" \
#   "SSLLIB=-L/opt/openssl-0.9.8k/lib"
#
# (don't set the variables and then do "make -e" because that breaks 
# chaining of makefile targets.)
#
# Here are some up-to-date secure targets as of Sep 2009: 
#
# aix+openssl:                        IBM AIX 4.2 or later with OpenSSL
# freebsd44+srp+openssl               FreeBSD 4.4 with SRP and OpenSSL
# freebsd50+openssl                   FreeBSD 5.0 with OpenSSL
# hpux1100o+openssl:                  HP-UX 11.xx with OpenSSL
# hpux1000gcc+openssl:                HP-UX 10.xx with OpenSSL (build with gcc)
# hpux1100gcc+openssl:                HP-UX 11.xx with OpenSSL (build with gcc)
# irix6x+krb5:                        IRIX 6.x with Kerberos V
# irix65+krb5:                        etc etc...
# solaris9+openssl                    Solaris 9,10, or 11 with Openssl (Sun cc)
# solaris9g+openssl                   Solaris 9,10, or 11 with Openssl (gcc)
# linux+ssl                           OpenSSL only
# linux+krb5+ssl                      Linux with Kerberos 5 and OpenSSL
# linux+krb5:                         Kerberos 5 only
# 
# The following secure Linux targets have not been updated or tested recently.
# linux+krb5+krb4:
# linux+srp:
# linux+srp+pam:
# linux+srp+gmp:
# linux+srp+gmp+no-des:
# linux+srp+gmp-export:
# linux+srp+gmp+pam:
# linux+shadow+pam:
# linux+openssl:
# linux+openssl+shadow:
# linux+openssl+zlib+shadow+pam:
# linux+srp+openssl:
# linux+krb5+krb4+srp:
# linux+krb5+krb4+srp+openssl:
# linux+krb5+krb4+openssl:
# linux+krb5+krb4+openssl+shadow:
# linux+krb5+krb4+openssl+zlib+shadow:
# linux+krb5+krb4+srp-export:
# linux+krb5+krb4+srp+pam:
# linux+krb5+krb4+srp+openssl+pam-debug:
# linux+krb5+krb4+srp+openssl+pam:
# linux+krb5+krb4+srp+openssl+zlib+pam:
# linux+krb5+krb4+openssl+shadow+pam:
# linux+krb5+openssl+zlib+shadow+pam:
# 
# The following have not been tested recently either and might
# need adjustment.
#
# macosx+krb5+ssl:     Mac OS X 10.3.9 or later + OpenSSL and Kerberos 5
# macosx103+secure:    This one is probably redundant
# netbsd+openssl:      NetBSD with OpenSSL
# openbsd30+ssl:       OpenBSD 3.0 with OpenSSL
# redhat71,redhat72,redhat73,redhat80,redhat9 (Krb5,OpenSSL,Showdow,PAM,Zlib)
# sco32v500net+ssl:
# sco32v505net+ssl:
# solaris2x+krb4:
# solaris2xg+krb4:
# solaris2xg+openssl+pam+shadow:
# solaris2xg+openssl+zlib+pam+shadow:
# solaris2xg+krb5+krb4+openssl+shadow:
# solaris25+krb4:
# solaris25g+krb4:
# solaris26g+openssl:
# solaris8g+openssl+zlib+pam+shadow:
# solaris8g+krb4:
# solaris9g+openssl+zlib+pam+shadow:
# solaris9g+openssl+shadow+pam+zlib
# sunos41gcc+krb4:                    SunOS 4.1 built with gcc with Kerberos IV
# sunos41gcc+openssl:                 SunOS 4.1 built with gcc with OpenSSL
# sunos41gcc+krb4+openssl:            ...with Kerberos IV and OpenSSL
# sunos41gcc+krb4+openssl+zlib:       ditto, plus ZLIB compression
# sunos41gcc+krb4+srp+openssl+zlib:   ditto, plus SRP
# sunos41gcc+srp+openssl+zlib:
# tru64-51b-openssl:                  Tru64 (Digital) Unix 5.1B with OpenSSL
# uw7ssl                              Unixware 7 with SSL
#
###########################################################################
#
#  Compile and Link variables:
#
#  EXT is the extension (file type) for object files, normally o.
#  See MINIX entry for what to do if another filetype must be used.
#
EXT=o
#LNKFLAGS=
SHAREDLIB=
CC= cc
CC2= cc
MAKE= make
SHELL=/bin/sh

###########################################################################
# (Ancient) UNIX V7-specific variables.
# These are set up for Perkin-Elmer 3230 V7 Unix:
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
# SAMPLE INSTALLATION SCRIPT
#
# Modify to suit your own computer's file organization and permissions.  If
# you don't have write access to the destination directories, "make install"
# fails.  In most cases, a real installation also requires you to chown /
# chgrp the Kermit binary for the UUCP lockfile and/or tty devices, and
# perhaps also to chmod +s the corresponding permission fields.
#
# Default binary, man, and doc directories are supplied below.  You can
# override them in your 'make' command.  Examples:
#
#   make install                                   # Accept defaults.
#   make "INFODIR=/usr/share/lib/kermit" install   # Override INFODIR default.
#
# You can also build and install in one step, e.g.:
#
#   make linux install
#
# If you use the 'install' target to install C-Kermit, it creates an
# UNINSTALL script that can be used to uninstall it.
#
WERMIT = makewhat
BINARY = wermit
DESTDIR =
BINDIR = $(prefix)/bin
MANDIR = $(manroot)/man/man1
MANEXT = 1
SRCDIR =
INFODIR =
CERTDIR =

TEXTFILES = COPYING.TXT ckcbwr.txt ckubwr.txt ckuins.txt ckccfg.txt \
		ckcplm.txt ckermit.ini ckermod.ini ckermit70.txt ckermit80.txt

ALL = $(WERMIT)

all: $(ALL)

.c.o:
	$(CC) $(CFLAGS) -DKTARGET=\"$(KTARGET)\" -c $<

#Clean up intermediate and object files
clean:
	@echo 'Removing object files...'
	-rm -f ckcmai.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckcpro.$(EXT) ckcfns.$(EXT) \
ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) ckucon.$(EXT) ckutio.$(EXT) \
ckufio.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) ckwart.$(EXT) ckuusx.$(EXT) \
ckuusy.$(EXT) ckcnet.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) ckusig.$(EXT) \
ckucns.$(EXT) ckcmdb.$(EXT) ckuath.$(EXT) ckctel.$(EXT) ckclib.$(EXT) \
ckcuni.$(EXT) ck_crp.$(EXT) ck_ssl.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) \
ckcpro.c wart

show:
	@echo prefix=$(prefix)
	@echo srproot=$(srproot)
	@echo sslroot=$(sslroot)
	@echo manroot=$(manroot)
	@echo K4LIB=$(K4LIB)
	@echo K4INC=$(K4INC)
	@echo K5LIB=$(K5LIB)
	@echo K5INC=$(K5INC)
	@echo SRPLIB=$(SRPLIB)
	@echo SRPINC=$(SRPINC)
	@echo SSLLIB=$(SSLLIB)
	@echo SSLINC=$(SSLINC)
	@exit

# Install C-Kermit after building -- IMPORTANT: Read the instructions above
# (SAMPLE INSTALLATION SCRIPT).  For SSL/TLS versions, ca_certs.pem file
# should be installed in the appropriate place for your OpenSSL library, e.g.:
#
#   cp ca_certs.pem /usr/local/ssl/
#   cp ca_certs.pem /usr/share/ssl/
#
# To make sure 'man' notices the new source file and doesn't keep
# showing the old formatted version, remove the old formatted version,
# something like this:
#		rm -f $(MANDIR)/../cat$(MANEXT)/kermit.$(MANEXT)
# or this (which requires CATDIR to be defined):
#		rm -f $(CATDIR)/kermit.$(MANEXT)
#
# As of C-Kermit 8.0.205 this target also builds an UNINSTALL script, and
# so it might be too long for some old Bourne shells, in which case you can
# use a different shell:
#
#   make SHELL=ksh install
#   make SHELL=/bin/posix/sh install
#
install:
	@echo Installing C-Kermit version $(CKVER)...;\
	rm -f UNINSTALL;\
	exec 3>./UNINSTALL;\
	echo "# C-Kermit UNINSTALL script" >&3;\
	echo "# `date`\n" >&3;\
	echo "CKVER=$(CKVER)" >&3;\
	echo "PrN Uninstalling C-Kermit version $(CKVER)..." >&3;\
	echo DESTDIR=$(DESTDIR);\
	if test -n "$(DESTDIR)"; then\
		if test -d $(DESTDIR); then\
			echo  "$(DESTDIR) exists...\n";\
		else\
			echo "Creating $(DESTDIR)...";\
			DESTDIR=`echo $(DESTDIR) | sed 's!/*$$!!'`;\
			mkdir $$DESTDIR  || exit 1;\
		fi;\
		chmod 755 $(DESTDIR) || exit 1;\
	fi;\
	echo BINARY=$(BINARY);\
	if test -f $(BINARY); then\
		ls -l $(BINARY);\
	else\
		echo "?$(BINARY) not found";\
		exit 1;\
	fi;\
	if test -z "$(DESTDIR)$(BINDIR)"; then\
		echo "Binary directory not specified";\
		exit 1;\
	fi;\
	if test -d $(DESTDIR)$(BINDIR); then\
		echo  "$(DESTDIR)$(BINDIR) exists...";\
	else\
		echo "Creating $(DESTDIR)$(BINDIR)/...";\
		mkdir     $(DESTDIR)$(BINDIR) || exit 1;\
		chmod 755 $(DESTDIR)$(BINDIR);\
	fi;\
	rm -f $(DESTDIR)$(BINDIR)/kermit;\
	cp $(BINARY) $(DESTDIR)$(BINDIR)/kermit || exit 1;\
	chmod 755    $(DESTDIR)$(BINDIR)/kermit || exit 1;\
	rm -f        $(DESTDIR)$(BINDIR)/kermit-sshsub;\
	ln -s        $(DESTDIR)$(BINDIR)/kermit\
		     $(DESTDIR)$(BINDIR)/kermit-sshsub || exit 1;\
	echo 'set flag=f\nPrC Removing binaries' >&3;\
	echo "RmF $(DESTDIR)$(BINDIR)/kermit-sshsub" >&3;\
	echo "RmF $(DESTDIR)$(BINDIR)/kermit" >&3;\
	if test -f ckermit.ini; then\
		echo "#!$(BINDIR)/kermit" >\
			$(DESTDIR)$(BINDIR)/_tmp.ini;\
		cat ckermit.ini >> $(DESTDIR)$(BINDIR)/_tmp.ini;\
		mv $(DESTDIR)$(BINDIR)/_tmp.ini\
		   $(DESTDIR)$(BINDIR)/ckermit.ini;\
		chmod 755 $(DESTDIR)$(BINDIR)/ckermit.ini;\
		echo "RmF $(DESTDIR)$(BINDIR)/ckermit.ini" >&3;\
	fi;\
	echo;\
	echo 'EfM' >&3;\
	echo "Kermit binary installed:";\
	ls -l $(DESTDIR)$(BINDIR)/kermit\
	      $(DESTDIR)$(BINDIR)/kermit-sshsub\
	      $(DESTDIR)$(BINDIR)/ckermit.ini;\
	echo;\
	echo " WARNING: If C-Kermit is to be used for dialing out,";\
	echo " you must change its owner and group and permissions";\
	echo " to match the 'cu' program.  See the ckuins.txt file";\
	echo " for details.";\
	echo;\
	echo MANDIR=$(MANDIR);\
	if test -n "$(DESTDIR)$(MANDIR)"; then\
		if test -d $(DESTDIR)$(MANDIR); then\
			echo  "$(DESTDIR)$(MANDIR) exists...";\
		else\
			echo "Creating $(MANDIR)...";\
			mkdir $(MANDIR) || exit 1;\
			chmod 755 $(MANDIR) || exit 1;\
		fi;\
		rm -f $(DESTDIR)$(MANDIR)/kermit.$(MANEXT);\
		cp ckuker.nr $(DESTDIR)$(MANDIR)/kermit.$(MANEXT) || exit 1;\
		chmod 644 $(DESTDIR)$(MANDIR)/kermit.$(MANEXT) || exit 1;\
		echo 'set flag=f\nPrC Removing man pages' >&3;\
		echo "RmF $(DESTDIR)$(MANDIR)/kermit.$(MANEXT)" >&3;\
		echo 'EfM' >&3;\
		echo;\
	else\
		echo "Not installing man page!\n";\
	fi;\
	echo CERTDIR=$(CERTDIR);\
	if test -n "$(CERTDIR)"; then\
		if test -f ca_certs.pem; then\
			if test -d $(CERTDIR); then\
				echo  "$(CERTDIR) exists...";\
			else\
				echo "Creating $(CERTDIR)...";\
				mkdir $(CERTDIR) || exit 1;\
				chmod 755 $(CERTDIR) || exit 1;\
			fi;\
			echo "Installing certificates file...";\
			cp ca_certs.pem $(CERTDIR) || exit 1;\
			echo 'set flag=f' >&3;\
			echo 'PrC Removing certificates file' >&3;\
			echo "RmF $(CERTDIR)/ca_certs.pem" >&3;\
			echo 'EfM' >&3;\
			echo;\
		fi;\
	else\
		echo "Not installing certificates file!\n";\
	fi;\
	echo SRCDIR=$(DESTDIR)$(SRCDIR);\
	if test -n "$(SRCDIR)"; then\
		echo "Installing source files...";\
		if test -d $(DESTDIR)$(SRCDIR); then\
			echo  "$(DESTDIR)$(SRCDIR) exists...";\
		else\
			echo "Creating $(DESTDIR)$(SRCDIR)/...";\
			mkdir     $(DESTDIR)$(SRCDIR) || exit 1;\
			chmod 755 $(DESTDIR)$(SRCDIR);\
		fi;\
		echo "Copying source files to $(DESTDIR)$(SRCDIR)...";\
		echo 'set flag=f\nPrC Removing source files' >&3;\
		for TextFile in COPYING.TXT ck[cuw_]*.[cwh] makefile; do\
			cp $$TextFile $(DESTDIR)$(SRCDIR)/ && echo ".\c";\
			echo "RmF $(DESTDIR)$(SRCDIR)/$$TextFile" >&3;\
		done; echo;\
		echo 'EfM' >&3;\
		( cd $(DESTDIR)$(SRCDIR)/ &&\
		ls -l COPYING.TXT ck[cuw_]*.[cwh] makefile );echo;\
	else\
		echo "Not installing source code!\n";\
	fi;\
	echo INFODIR=$(DESTDIR)$(INFODIR);\
	if test -n "$(INFODIR)"; then\
		echo "Installing info files...";\
		if test -d $(DESTDIR)$(INFODIR); then\
			echo  "$(DESTDIR)$(INFODIR) exists...";\
		else\
			echo "Creating $(DESTDIR)$(INFODIR)/...";\
			mkdir     $(DESTDIR)$(INFODIR) || exit 1;\
			chmod 755 $(DESTDIR)$(INFODIR);\
		fi;\
		echo "Copying text files to $(DESTDIR)$(INFODIR)...";\
		echo 'set flag=f\nPrC Removing text files' >&3;\
		FileCopyList='';\
		for TextFile in $(TEXTFILES); do\
			test -f $$TextFile || continue;\
			cp $$TextFile $(DESTDIR)$(INFODIR) && echo ".\c" &&\
			FileCopyList="$$FileCopyList $$TextFile";\
			echo "RmF $(DESTDIR)$(INFODIR)/$$TextFile" >&3;\
		done; echo;\
		echo 'EfM' >&3;\
		( cd $(DESTDIR)$(INFODIR)/ && chmod  644   $$FileCopyList );\
		( cd $(DESTDIR)$(INFODIR)/ && pwd && ls -l $$FileCopyList );\
	else\
		echo "Not installing text files!\n";\
	fi;\
	echo "set flag=d\nPrN Removing empty dirs..." >&3;\
	echo "RmD $(DESTDIR)$(BINDIR)" >&3;\
	echo "RmD $(DESTDIR)$(SRCDIR)" >&3;\
	echo "RmD $(DESTDIR)$(INFODIR)" >&3;\
	echo "RmD $(CERTDIR)" >&3;\
	echo "RmD $(MANDIR)" >&3;\
	echo "RmD $(DESTDIR)" >&3;\
	echo "EfM" >&3;\
	echo "PrN C-Kermit version $(CKVER) is uninstalled!" >&3;\
	echo C-Kermit version $(CKVER) installed!

# UN-Install C-Kermit after building
# Please to not remove the extra blanks before and after '{}' within the
# functions. You would get syntax errors for some older Bourne shells! Best is 
# you don't change or remove anything.
#
uninstall:
	@if test ! -f UNINSTALL; then\
		echo "?C-Kermit UNINSTALL data file not found!";\
		exit 1;\
	fi; \
	X=`grep '^CKVER='$(CKVER)'$$' ./UNINSTALL || :`;\
	if test -z "$$X"; then\
		echo "?UNINSTALL file is not for C-Kermit version $(CKVER)";\
		exit 2;\
	fi;\
	PrN () { echo "$$*"; };\
	PrC () { echo "$$* \c"; };\
	RmF () { test -f "$$1" && rm -f "$$1" && echo ".\c" && flag=F ; };\
	RmD () { \
	dir=$$1;\
	while test -d "$$dir"; do\
		rmdir "$$dir" 2>&- || return && echo "$$dir" && flag=D;\
		dir=`echo "$$dir" | sed 's!/[^/]*/*$$!!'`;\
	done; \
	};\
	EfM () { \
	case "$$flag" in\
		f) echo "- Nothing to remove!";;\
		d) echo "Nothing to remove!";;\
		F) echo " done";;\
		D) echo "done";;\
	esac; \
	};\
	while read Act Args; do\
		case $$Act in\
			EfM) EfM;;\
			RmD) RmD $$Args;;\
			RmF) RmF $$Args;;\
			PrN) PrN $$Args;;\
			PrC) PrC $$Args;;\
			set) eval $$Args;;\
		esac;\
	done < ./UNINSTALL

makewhat:
	@echo 'make what?  You must tell which platform to make C-Kermit for.'
	@echo Examples: make linux, make aix, make solaris10, make hpux1100.
	@echo Please read the comments at the beginning of the makefile.

###########################################################################
#
# Dependencies Section:

wermit:	ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucon.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT)
	$(CC2) $(LNKFLAGS) -o wermit \
		ckcmai.$(EXT) ckclib.$(EXT) ckutio.$(EXT) ckufio.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckcpro.$(EXT) ckucmd.$(EXT) ckuus2.$(EXT) ckuus3.$(EXT) \
		ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) \
		ckuusx.$(EXT) ckuusy.$(EXT) ckuusr.$(EXT) ckucon.$(EXT) \
		ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) \
		ckusig.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) \
		$(LIBS)

# Preferred configuration with select()-based CONNECT

xermit:	ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucns.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) ckuath.$(EXT) \
		ck_crp.$(EXT) ck_ssl.$(EXT)
	$(CC2) $(LNKFLAGS) -o wermit \
		ckcmai.$(EXT) ckclib.$(EXT) ckutio.$(EXT) ckufio.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckcpro.$(EXT) ckucmd.$(EXT) ckuus2.$(EXT) ckuus3.$(EXT) \
		ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) \
		ckuusx.$(EXT) ckuusy.$(EXT) ckuusr.$(EXT) ckucns.$(EXT) \
		ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) ckusig.$(EXT) \
		ckctel.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) \
		ckuath.$(EXT) ck_crp.$(EXT) ck_ssl.$(EXT) $(LIBS)

# Malloc Debugging version

mermit:	ckcmdb.$(EXT) ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckcpro.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) \
		ckuxla.$(EXT) ckucon.$(EXT) ckutio.$(EXT) ckufio.$(EXT) \
		ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) \
		ckusig.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT)
	$(CC2) $(LNKFLAGS) -o mermit ckcmdb.$(EXT) ckclib.$(EXT) ckcmai.$(EXT)\
		ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) \
		ckcfn3.$(EXT) ckuxla.$(EXT) ckcpro.$(EXT) ckucmd.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckuusr.$(EXT) ckucon.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) \
		ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) ckcuni.$(EXT) \
		ckupty.$(EXT) ckcftp.$(EXT) $(LIBS)

# Kerberized Version - Subject to USA export restrictions.

# NOTE: We don't use this any more -- As of 15 Feb 2003, the "xermit"
# target is used for both secure and regular version.

krbmit:	ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucns.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckuath.$(EXT) ck_crp.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) \
		ckcftp.$(EXT) ck_ssl.$(EXT)
	$(CC2) $(LNKFLAGS) -o krbmit ckcmai.$(EXT) ckclib.$(EXT) \
		ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) \
		ckcfn3.$(EXT) ckuxla.$(EXT) ckcpro.$(EXT) ckucmd.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckuusr.$(EXT) ckucns.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) \
		ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) ckuath.$(EXT) \
		ck_crp.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) \
		ck_ssl.$(EXT) $(LIBS)

krbmit-debug:	ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckcpro.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) \
		ckuxla.$(EXT) ckucns.$(EXT) ckutio.$(EXT) ckufio.$(EXT) \
		ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) \
		ckusig.$(EXT) ckuath.$(EXT) ck_crp.$(EXT) ckcuni.$(EXT) \
		ckupty.$(EXT) ck_ssl.$(EXT) ckcmdb.$(EXT) ckcftp.$(EXT)
	$(CC2) $(LNKFLAGS) -o krbmit ckcmdb.$(EXT) ckcmai.$(EXT) \
		ckclib.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) \
		ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) ckcpro.$(EXT) \
		ckucmd.$(EXT) ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) \
		ckuus5.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) \
		ckuusy.$(EXT) ckuusr.$(EXT) ckucns.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckuath.$(EXT) ck_crp.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) \
		ckcftp.$(EXT) ck_ssl.$(EXT) $(LIBS)

# SRP(TM) Version - Subject to USA export restrictions.

srpmit:	ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucns.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckuath.$(EXT) ck_crp.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) \
		ckcftp.$(EXT) ck_ssl.$(EXT)
	$(CC2) $(LNKFLAGS) -o srpmit ckcmai.$(EXT) ckclib.$(EXT) \
		ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) \
		ckcfn3.$(EXT) ckuxla.$(EXT) ckcpro.$(EXT) ckucmd.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckuusr.$(EXT) ckucns.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) \
		ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) ckuath.$(EXT) \
		ck_crp.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ck_ssl.$(EXT) \
		ckcftp.$(EXT) $(LIBS)

# Kerberized Version - Not subject to USA export restrictions.

krbmit-export:	ckcmai.$(EXT) \
		ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucns.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckuath.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT)
	$(CC2) $(LNKFLAGS) -o krbmit-export ckcmai.$(EXT) ckclib.$(EXT) \
		ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) \
		ckcfn3.$(EXT) ckuxla.$(EXT) ckcpro.$(EXT) ckucmd.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckuusr.$(EXT) ckucns.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) \
		ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) ckuath.$(EXT) \
		ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) $(LIBS)

# SRP(TM) Version - Not subject to USA export restrictions.

srpmit-export:	ckcmai.$(EXT) \
		ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucns.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckuath.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT)
	$(CC2) $(LNKFLAGS) -o srpmit-export ckcmai.$(EXT) ckclib.$(EXT) \
		ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) \
		ckcfn3.$(EXT) ckuxla.$(EXT) ckcpro.$(EXT) ckucmd.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckuusr.$(EXT) ckucns.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) \
		ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) ckuath.$(EXT) \
		ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) $(LIBS)

###########################################################################
# man page...
#
ckuker.nr:
	@echo This target is obsolete.
	@echo The ckuker.nr file no longer needs any preprocessing.

###########################################################################
# Dependencies for each module...
#
ckcmai.$(EXT): ckcmai.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcnet.h ckcsig.h \
		ckuusr.h ckctel.h ckclib.h

ckclib.$(EXT): ckclib.c ckclib.h ckcdeb.h ckcasc.h ckcsym.h

ckcpro.$(EXT): ckcpro.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckclib.h

ckcpro.c: ckcpro.w wart ckcdeb.h ckcsym.h ckcasc.h ckcker.h ckcnet.h ckctel.h \
	 ckclib.h
	./wart ckcpro.w ckcpro.c

ckcfns.$(EXT): ckcfns.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h ckcuni.h \
		ckuxla.h ckclib.h ckcnet.h

ckcfn2.$(EXT): ckcfn2.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h \
		ckuxla.h ckctel.h ckclib.h ckcnet.h ckcuni.h

ckcfn3.$(EXT): ckcfn3.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h \
		ckuxla.h ckclib.h ckcuni.h

ckuxla.$(EXT): ckuxla.c ckcker.h ckcsym.h ckcdeb.h ckcxla.h ckuxla.h ckclib.h \
		 ckcuni.h

ckcuni.$(EXT): ckcuni.c ckcdeb.h ckcker.h ckucmd.h ckcuni.h ckcxla.h ckuxla.h

ckuusr.$(EXT): ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcsym.h ckcdeb.h ckcxla.h \
		ckuxla.h ckcasc.h ckcnet.h ckctel.h ckclib.h ckcuni.h

ckuus2.$(EXT): ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h ckctel.h ckclib.h ckcuni.h

ckuus3.$(EXT): ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h ckctel.h ckclib.h ckcuni.h

ckuus4.$(EXT): ckuus4.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckuver.h ckcsym.h ckctel.h ckclib.h ckcuni.h

ckuus5.$(EXT): ckuus5.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcnet.h \
		 ckcsym.h ckctel.h ckclib.h ckcxla.h ckuxla.h ckcuni.h

ckuus6.$(EXT): ckuus6.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcnet.h \
		 ckcsym.h ckctel.h ckclib.h

ckuus7.$(EXT): ckuus7.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h ckctel.h ckclib.h ckcuni.h

ckuusx.$(EXT): ckuusx.c ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcsym.h \
		ckcsig.h ckcnet.h ckctel.h ckclib.h ckcxla.h ckuxla.h ckcuni.h

ckuusy.$(EXT): ckuusy.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsym.h ckctel.h \
		 ckclib.h

ckucmd.$(EXT): ckucmd.c ckcasc.h ckucmd.h ckcdeb.h ckcsym.h ckctel.h ckclib.h

ckufio.$(EXT): ckufio.c ckcdeb.h ckuver.h ckcsym.h ckclib.h \
		ckcxla.h ckuxla.h ckcuni.h

ckutio.$(EXT): ckutio.c ckcdeb.h ckcnet.h ckuver.h ckcsym.h ckctel.h ckclib.h

ckucon.$(EXT): ckucon.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsym.h ckctel.h \
		 ckclib.h

ckucns.$(EXT): ckucns.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsym.h ckctel.h \
		 ckclib.h ckcxla.h ckuxla.h ckcuni.h

ckcnet.$(EXT): ckcnet.c ckcdeb.h ckcker.h ckcnet.h ckcsym.h ckcsig.h ckctel.h \
		 ckclib.h

ckctel.$(EXT): ckcsym.h ckcdeb.h ckcker.h ckcnet.h ckctel.h ckclib.h

# ck_off_t: ck_off_t.$(EXT)
#	$(CC) -o ck_off_t ck_off_t.$(EXT)

wart: ckwart.$(EXT)
	$(CC) $(LNKFLAGS) -o wart ckwart.$(EXT) $(LIBS)

ckcmdb.$(EXT): ckcmdb.c ckcdeb.h ckcsym.h ckclib.h

ckwart.$(EXT): ckwart.c

ckudia.$(EXT): ckudia.c ckcker.h ckcdeb.h ckucmd.h ckcasc.h ckcsym.h ckcsig.h \
		ckcnet.h ckctel.h ckclib.h

ckuscr.$(EXT): ckuscr.c ckcker.h ckcdeb.h ckcasc.h ckcsym.h ckcsig.h \
		ckcnet.h ckctel.h ckclib.h

ckusig.$(EXT): ckusig.c ckcasc.h ckcdeb.h ckcker.h ckcnet.h ckuusr.h \
		ckcsig.h ckctel.h ckclib.h

ckcftp.$(EXT): ckcftp.c ckcdeb.h ckcasc.h ckcker.h ckucmd.h ckuusr.h \
		ckcnet.h ckctel.h ckcxla.h ckuxla.h ckcuni.h

ckupty.$(EXT): ckupty.c ckupty.h ckcdeb.h

ckuath.$(EXT): ckuath.c ckcdeb.h ckucmd.h ckuath.h ckuat2.h ckctel.h \
		 ckclib.h ckcnet.h

ck_crp.$(EXT): ck_crp.c ckcdeb.h ckcnet.h ckuath.h ckclib.h

ck_ssl.$(EXT): ck_ssl.c ckcdeb.h ckucmd.h ckuath.h ckuat2.h ckctel.h \
		 ckclib.h ck_ssl.h

###########################################################################
#
# Entries to make C-Kermit for specific systems.
#
# Put the ones that need short makefiles first.

#Apollo Aegis 9.x.  Includes TCP/IP support.
#You can also add processor-dependent optimization switches like -M570.
aegis:
	@echo Making C-Kermit $(CKVER) for Apollo Aegis 9.x...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DNOCSETS -DCK_CURSES -O $(KFLAGS)" \
	"LIBS = -lcurses -ltermcap"

#Apple Mac II, A/UX pre-3.0
#Warning, if "send *" doesn't work, try the auxufs makefile entry below.
aux:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DAUX -DTCPSOCKET $(KFLAGS) -i -O" "LNKFLAGS = -i"

#Apple Mac II, A/UX pre-3.0, compiled with gcc
auxgcc:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DAUX -DTCPSOCKET -traditional $(KFLAGS) -i -O" \
	"LNKFLAGS = " "CC = gcc" "CC2 = gcc"

#Apple Mac II, A/UX, pre-3.0, but with ufs file volumes, uses <dirent.h>.
auxufs:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DAUX -DTCPSOCKET -DDIRENT $(KFLAGS) -i -O" "LNKFLAGS = -i"

#Apple Mac II, A/UX 3.0, compiled with gcc
aux3gcc:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX 3.0...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DAUX -DHDBUUCP -DLFDEVNO -DTCPSOCKET -DDIRENT $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = $(LIBS)" \
	"CC=gcc -pipe -traditional" "CC2=gcc -pipe -traditional"

#Apple Mac II, A/UX 3.0, compiled with gcc, uses curses
aux3cgcc:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX 3.0...
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) aux3gcc \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DCK_CURSES" "LIBS = -lcurses $(LIBS)"

# Tenon MachTen, tested on Apple Powerbook with MachTen 2.1.1.D.
# NOTE: This doesn't do anything about UUCP.  It only works if /usr/spool/uucp
# has permission of 777, and dialout device is world read/writeable.
machten:
	@echo Making C-Kermit $(CKVER) for MachTen...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD43 -DTCPSOCKET -DSIG_V -DNDGPWNAM -DCK_CURSES -O \
	$(KFLAGS)"  "LIBS=-lcurses -ltermcap"

#Bell Labs Research UNIX V10
#Can't add TCP/IP because there is no sockets library.  It would have to
#be done using streams, but there is no code in C-Kermit for that.
#Remove -DNOJC if desired (if your system has csh, ksh, or bash).
bellv10:
	@echo Making C-Kermit $(CKVER) for Bell Labs Research UNIX V10...
	$(MAKE) wermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBELLV10 -DBSD4 -DNDIR -DNOJC -DNOSYSIOCTLH -DNOSETREU \
	-DNOCSETS -MINIDIAL $(KFLAGS)"

# WARNING: The early BSD entries do not build in version 7.0 with the stock
# BSD compiler: "Too many defines".  Unless you can rebuild cpp to have more
# space for defines, these builds must be accomplished by:
# copying the /usr/include tree to someplace else, preprocessing there with cc
# -E -I./include or whatever (plus all the same -D's, adding any necessary
# -U/-D to override the architecture)), renaming the the resulting files back
# to their original names, bringing them back to the original BSD system, and
# running the make target there.  This technique was used for 4.2 and 4.3 BSD
# on a VAX in C-Kermit 7.0 (later, cpp on that machine was rebuilt to allow
# more symbols, so the C-Kermit 8.0 build proceeds normally).

#Berkeley Unix 4.1
bsd41:
	@echo Making C-Kermit $(CKVER) for 4.1BSD...
	$(MAKE) wermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD41" "LIBS = -ljobs"

#Berkeley 4.2, 4.3, also Ultrix-32 1.x, 2.x, 3.x, many others
# Add -O, -s, etc, if they work.
# If you have a version of BSD but signal() is void rather than int,
# "make bsd KFLAGS=-DSIG_V".
bsd42:
	@echo Making C-Kermit $(CKVER) for 4.2BSD...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DNOREALPATH -DNOTIMEH -DNOIKSD \
	-DCK_CURSES -DSYSTIMEBH -DNOPUTENV -DNOANSI -DBIGBUFOK -DBSD42HACK \
	$(KFLAGS)" "LIBS=-lcurses -ltermcap $(LIBS)"

bsd:
	$(MAKE) CC=$(CC) CC2=$(CC2) bsd42 KTARGET=$${KTARGET-$(@)}

#Berkeley Unix 4.2 or 4.3 with HoneyDanBer UUCP
bsdhdb:
	@echo Making C-Kermit $(CKVER) for 4.2BSD with HDB UUCP...
	$(MAKE) CC=$(CC) CC2=$(CC2) bsd KTARGET=$${KTARGET-$(@)} \
	"KFLAGS= -DHDBUUCP $(KFLAGS)"

#Berkeley Unix 4.3 with acucntrl program, curses, TCP/IP included.
bsd43:
	@echo Making C-Kermit $(CKVER) for 4.3BSD...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DBSD43 -DTCPSOCKET -DNOREALPATH -DNOTIMEH -DNOIKSD \
	-DCK_CURSES -DACUCNTRL -DSYSTIMEBH -DNOPUTENV -DNOANSI -DBIGBUFOK \
	-DBSD42HACK $(KFLAGS)" "LIBS=-lcurses -ltermcap $(LIBS)"

#4.3BSD, curses excluded
bsd43nc:
	@echo Making C-Kermit $(CKVER) for 4.3BSD...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DBSD43 -DTCPSOCKET -DNOREALPATH -DNOTIMEH \
	-DACUCNTRL -DSYSTIMEBH -DNOIKSD -DNOPUTENV -DNOANSI -DBIGBUFOK \
	-DBSD42HACK $(KFLAGS)" "LIBS=$(LIBS)"

#4.3BSD, TCP/IP excluded.
bsd43nonet:
	@echo Making C-Kermit $(CKVER) for 4.3BSD + curses...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DBSD43 -DTCPSOCKET -DNOREALPATH -DNOTIMEH -DNOIKSD \
	-DCK_CURSES -DACUCNTRL -DSYSTIMEBH -DNOPUTENV -DNOANSI -DBIGBUFOK \
	-DBSD42HACK -DNONET $(KFLAGS)" "LIBS=-lcurses -ltermcap $(LIBS)"

#Berkeley Unix 4.2 or 4.3 with lock directory /usr/spool/uucp/LCK/LCK..ttyxx,
#but without acucntrl program
bsdlck:
	@echo Making C-Kermit $(CKVER) for 4.2BSD, /usr/spool/uucp/LCK/...
	$(MAKE) CC=$(CC) CC2=$(CC2) bsd KTARGET=$${KTARGET-$(@)} \
	"KFLAGS= -DLCKDIR $(KFLAGS)"

#Berkeley UNIX 4.4-Lite, 4.4-Encumbered, Net/2, etc (Post-Reno),
#with TCP/IP networking.  This was the basis for FreeBSD, NetBSD, OpenBSD,
#BSDI, BSD/OS, and Mac OS X (each of which has its own set of targets that
#are newer than this one).
#
#NOTE: This is not a pure POSIX configuration.  Using -DPOSIX instead of
# -DBSD44 prevents any kind of directory-reading (for wildcard expansion),
#and disallows use of ENOTCONN symbol for detecting broken network
#connections, and disallows RTS/CTS flow control, and would also require
#definition of the appropriate UUCP lockfile convention.
#Do not add -DCK_POSIX_SIG without reading <signal.h> first!  For example,
#sigsetjmp(), etc, tend to be defined but not implemented.
#
#NOTE: originally crypt was in libc - later it was unbundled.
#Remove the LIBS clause to build on an early 4.4BSD platform.
#
bsd44:
	@echo Making C-Kermit $(CKVER) for 4.4BSD...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DTCPSOCKET $(KFLAGS) -O" "LIBS=-lcrypt"

#Berkeley UNIX 4.4, as above, but with curses for fullscreen display
#Please read notes for bsd44 entry just above.
# NOTE: This one dumped core on the real 4.4BSD development system at
# UC Berkeley (an HP-9000/300), so the no-curses version was used
# for that one, which was unplugged years ago.
bsd44c:
	@echo Making C-Kermit $(CKVER) for 4.4BSD with curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DTCPSOCKET $(KFLAGS) -O" \
	"LIBS= -lcurses -ltermcap -lcrypt $(LIBS)"

#For FreeBSD 1.x.
freebsd1:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD...'
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DTCPSOCKET -DNOCOTFMC -funsigned-char \
	-DFNFLOAT -DNOHTERMCAP -DNOREALPATH -DNOSYSCONF $(KFLAGS) -O -pipe" \
	"LIBS= -lcurses -ltermcap -lm $(LIBS)"

#FreeBSD 2.x with ncurses
freebsd2:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 2.x with ncurses...'
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_NCURSES -DTCPSOCKET -DNOCOTFMC -DUSE_STRERROR \
	-DTPUTSARGTYPE=int -DTPUTSARG1CONST -DFREEBSD2 -funsigned-char \
	-DFNFLOAT $(KFLAGS) -O -pipe" \
	"LIBS= -lncurses -ltermlib -lcrypt -lm $(LIBS)"

#For FreeBSD 2.x -- Uses curses rather than ncurses
freebsd2c:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 2.x with curses...'
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DTCPSOCKET -DNOCOTFMC -DUSE_STRERROR \
	-DTPUTSARGTYPE=int -DTPUTSARG1CONST -DFREEBSD2 -DFNFLOAT \
	-funsigned-char $(KFLAGS) -O -pipe" \
	"LIBS= -lcurses -ltermlib -lcrypt -lm $(LIBS)"

#FreeBSD 3.x with ncurses and uu_lock()
#(Note: uu_lock() goes back to 2.2.2, but not necessarily 2.0)
#OK 2011/08/20 FreeBSD 3.3
freebsd3:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 3.x with ncurses...'
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_NCURSES -DTCPSOCKET -DNOCOTFMC -funsigned-char \
	-DTPUTSARGTYPE=int -DUSE_STRERROR -DFREEBSD3 -DUSE_UU_LOCK -DFNFLOAT \
	$(KFLAGS) -O -pipe" \
	"LIBS= -lncurses -lcrypt -lutil -lm $(LIBS)"

#As above but with curses rather than ncurses.
freebsd3c:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 3.x with curses...'
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DTCPSOCKET -DNOCOTFMC -DUSE_UU_LOCK \
	-DTPUTSARGTYPE=int -DUSE_STRERROR -DFREEBSD3 $(KFLAGS) -DFNFLOAT \
	-funsigned-char -pipe -O" \
	"LIBS= -lcurses -lcrypt -lutil -lm $(LIBS)"

#FreeBSD 4.0 with ncurses and uu_lock().  Note - there is no curses in 4.0.
#ncurses 5.0 is broken requiring us to work around with setbuf().
freebsd40:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 4.x with ncurses...'
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_NCURSES -DTCPSOCKET -DNOCOTFMC -DFNFLOAT \
	-funsigned-char -DTPUTSARGTYPE=int -DUSE_STRERROR -DFREEBSD4 \
	-DNONOSETBUF -DUSE_UU_LOCK $(KFLAGS) -O -pipe" \
	"LIBS= -lncurses -lcrypt -lutil -lm $(LIBS)"

#FreeBSD 4.1 and above
#Like FreeBSD 4.0 but without the NONOSETBUF hack and with CK_NEWTERM.
#NOTE: This target definitely does not work for FreeBSD 3.3 in 9.0.302.
#and it has not been tested on 4 or 5.
#OK 2011/06/xx FreeBSD 3.3, 4,4, 4.7, and 8.2
#OK 2011/08/21 FreeBSD 3.3, 4.4, 6.4, 9.0
freebsd freebsd41 freebsd72 freebsd5 freebsd6 freebsd7 freebsd8 freebsd9:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 4.1 or later...'
	@if test `uname -r | cut -d . -f 1` -ge 8; then \
	   HAVE_FBSD8='-DFREEBSD8'; \
	else HAVE_FBSD8=''; fi; \
	if test `uname -r | cut -d . -f 1` -ge 9; then \
	   HAVE_FBSD9='-DFREEBSD9'; \
	else HAVE_FBSD9='';  fi; \
	if test -f /usr/include/utmpx.h ; \
	then HAVE_UTMPX='-DHAVEUTMPX' ; \
	else HAVE_UTMPX='' ; fi; \
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_NCURSES -DCK_NEWTERM -DTCPSOCKET -DNOCOTFMC \
	-DFREEBSD4 $$HAVE_FBSD8 $$HAVE_FBSD9 -DUSE_UU_LOCK -DFNFLOAT \
	$$HAVE_UTMPX -DHERALD=\"\\\" `uname -rs`\\\"\" \
	-funsigned-char -DTPUTSARGTYPE=int -DUSE_STRERROR $(KFLAGS) \
	-O2 -pipe"\
	"LIBS= -lncurses -lcrypt -lutil -lm $(LIBS)"

#FreeBSD 5.0 or later with OpenSSL.
#OK 2011/06/15 FreeBSD 4.7 and 8.2
#OK 2011/08/20 FreeBSD 9.0-CURRENT
freebsd+ssl freebsd+openssl freebsd50+openssl:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD with Kerberos 5...'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls /usr/lib/libdes* > /dev/null 2> /dev/null || \
	   ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
              echo "HAVE DES"; \
           else echo "NO DES"; \
	fi; \
	$(MAKE) freebsd KTARGET=$${KTARGET:-$(@)} "CC = $(CC)" "CC2 = $(CC2)" \
	KFLAGS="-DCK_AUTHENTICATION -DCK_SSL $(SSLINC) -DZLIB $$OPENSSLOPTION \
	$$HAVE_DES $(KFLAGS)"  "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS= -lncurses -lcrypt -lssl -lcrypto -lutil -lm \
	$(SSLLIB) $$DES_LIB $(LIBS)"; \
	if [ ! -f ./wermit ] || [ ./ckcmai.o -nt ./wermit ] ; then \
		echo ""; \
		echo "If build failed try:"; \
		echo ""; \
		echo "  make clean ; make $${KTARGET:-$(@)} KFLAGS=-UCK_DES"; \
		echo ""; \
	fi

#NetBSD 1.4.1 or later with vanity banner automated with uname
#and automatic inclusion of large file support if it is available.
#This target tested successfully on NetBSD 1.4.1, 1.5.2, and 2.0.3 (Jan 2006).
#Fails on NetBSD 2.0 on Sun/3 mc68030 with gcc 3.3.3 unless optimization is
#disabled on ckcfn2.c ("KFLAGS=-O0") (Letter O Digit Zero).
#(This could be automated by testing `uname -m` for "sun3".)
#OK: 2011/06/15 on NetBSD 1.5.2 and 5.1.
#OK: 2011/08/21 on 5.1.
netbsd netbsd2 netbsd15 netbsd16 old-netbsd:
	@echo Making C-Kermit $(CKVER) for NetBSD with curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS=`grep fseeko /usr/include/stdio.h > /dev/null && \
	echo '-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64'` \
	-DBSD44 -DCK_CURSES -DTCPSOCKET -DUSE_STRERROR -DHAVE_OPENPTY \
	-funsigned-char -DHERALD=\"\\\" NetBSD `uname -r`\\\"\" \
	-DCK_DTRCD -DCK_DTRCTS -DTPUTSARGTYPE=int -DFNFLOAT $(KFLAGS) -O" \
	"LIBS= -lcurses -lcrypt -lm -lutil $(LIBS)"

#NetBSD 1.4.1 or later with OpenSSL
#OK: 2011/06/15 on NetBSD 5.1 (but not 1.5.2 with OpenSSL 0.9.5a)
#OK: 2011/08/21 on 5.1.
netbsd+ssl netbsd+openssl:
	@echo 'Making C-Kermit $(CKVER) for NetBSD+OpenSSL SSLLIB=$(SSLLIB)'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls /usr/lib/libdes* > /dev/null 2> /dev/null || \
	   ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
              echo "HAVE DES"; \
           else echo "NO DES"; \
	fi; \
	$(MAKE) netbsd KTARGET=$${KTARGET:-$(@)} "CC = $(CC)" "CC2 = $(CC2)" \
	"KFLAGS= -DCK_AUTHENTICATION -DCK_ENCRYPTION -DCK_CAST $$HAVE_DES \
	-DCK_SSL -DCK_PAM -DZLIB -DNO_DCL_INET_ATON $$OPENSSLOPTION \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS= -L/usr/pkg/lib -R/usr/pkg/lib -lssl $$DES_LIB -lcurses \
	-lcrypto -lcrypt -lz -lm -lpam -lutil $(LIBS)"

#NetBSD with MIT Kerberos 5:
# OK 2011/06/15 (once K5INC and K5LIB were set right).
# NOT OK for Heimdal - Heimdal Kerberos support in C-Kermit needs work.
# OK: 2011/08/21 on 5.1.
netbsd+krb5:
	@echo 'Making C-Kermit $(CKVER) for NetBSD with Kerberos 5...'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls /usr/lib/libdes* > /dev/null 2> /dev/null || \
	   ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
              echo "HAVE DES"; \
           else echo "NO DES"; \
	fi; \
	$(MAKE) netbsd KTARGET=$${KTARGET:-$(@)} "CC = $(CC)" "CC2 = $(CC2)" \
	"KFLAGS= -DCK_AUTHENTICATION -DCK_ENCRYPTION -DCK_KERBEROS -DKRB5 \
	-DCK_CAST $$HAVE_DES -DNOFTP_GSSAPI $(K5INC) $(K5INC)/krb5 \
	$(KFLAGS)" \
	"LIBS= $(K5LIB) -L/usr/pkg/lib -R/usr/pkg/lib -lcurses $$DES_LIB \
	-lcrypto -lgssapi -lkrb5 -lm -lutil $(LIBS)"

# NetBSD - With Kerberos 5 and SSL and Zlib.
# OK: 2011/08/21 on 5.1 with MIT Kerberos.
netbsd+krb5+ssl netbsd+krb5+openssl+zlib:
	@echo 'Making C-Kermit $(CKVER) for NetBSD+OpenSSL+Kerberos5...'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls /usr/lib/libdes* > /dev/null 2> /dev/null || \
	   ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
              echo "HAVE DES"; \
           else echo "NO DES"; \
	fi; \
	$(MAKE) netbsd KTARGET=$${KTARGET:-$(@)} "CC = $(CC)" "CC2 = $(CC2)" \
	"KFLAGS= -DCK_AUTHENTICATION -DCK_ENCRYPTION -DCK_CAST $$HAVE_DES \
	-DCK_KERBEROS -DKRB5 -DNOFTP_GSSAPI $(K5INC) $(K5INC)/krb5 \
	-DCK_SSL -DCK_PAM -DZLIB -DNO_DCL_INET_ATON $$OPENSSLOPTION \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS= $(K5LIB) -L/usr/pkg/lib -R/usr/pkg/lib -lssl $$DES_LIB \
	-lcrypto -lcrypt -lgssapi -lkrb5 -lz -lm -lpam -lutil -lcurses $(LIBS)"

#Special Security Enhanced NetBSD target with SRP, SSL, and zlib support.
#To build this, you need to BUILD the pkgsrc srp_client package.  After
#you build it, you must go into work/srp-x.y.z/libkrypto and "bmake install"
#then go to work/srp-x.y.z/libsrp and "bmake install".  As of 2005Q3, the
#pkgsrc install only installed the statically linked client applications.  You
#need to manually install the libraries to build your own applications.
#NOT TESTED RECENTLY - probably needs work.
netbsd+ssl+srp+zlib:
	@echo Making C-Kermit $(CKVER) for NetBSD with curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DTCPSOCKET -DUSE_STRERROR -DNETBSD15 \
	-DCK_DTRCD -DCK_DTRCTS -DTPUTSARGTYPE=int -DHAVE_OPENPTY \
	-I/usr/include/openssl -I/usr/pkg/include \
	-DCK_AUTHENTICATION -DCK_SRP -DPRE_SRP_1_4_5 -DCK_ENCRYPTION \
	-DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DZLIB -DFNFLOAT $(KFLAGS) -O" \
	"LIBS= -L/usr/pkg/lib -R/usr/pkg/lib -lcurses -lsrp -lgmp -ldes \
	-lssl -lkrypto -lcrypto -lcrypt -lz -lm -lutil $(LIBS)"

#NetBSD with curses left out (e.g. for use as IKSD).
netbsdnc:
	@echo Making C-Kermit $(CKVER) for NetBSD with no curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) netbsd KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOCURSES"

#NetBSD with ncurses requested explicitly rather than curses-which-is-ncurses
netbsdn:
	@echo Making C-Kermit $(CKVER) for NetBSD with curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS=`grep fseeko /usr/include/stdio.h > /dev/null && \
	echo '-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64'` \
	-DBSD44 -DCK_CURSES -DTCPSOCKET -DUSE_STRERROR -DHAVE_OPENPTY \
	-DHERALD=\"\\\" NetBSD `uname -r`\\\"\" \
	-DCK_DTRCD -DCK_DTRCTS -DTPUTSARGTYPE=int -DFNFLOAT $(KFLAGS) -O" \
	"LIBS= -L/usr/pkg/lib -lncurses -lcrypt -lm -lutil $(LIBS)"

#OpenBSD before 2.3.
#Uses ncurses as its curses so use -ltermlib, not -ltermcap
#But it doesn't use uu_lock() which was introduced in OpenBSD 2.3.
#For that use the next entry.
#Add -DMAINTYPE=int if you get complaints about main: return type is not int.
openbsdold:
	@echo Making C-Kermit $(CKVER) for OpenBSD...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET -DOPENBSD \
	-DFNFLOAT -DNDSYSERRLIST $(KFLAGS) -O" "LIBS= -lcurses -ltermlib -lm"

#OpenBSD 2.3 or later
#Add -DMAINTYPE=int if you get complaints about main: return type is not int.
#For C-Kermit 8.0 (Christian Weisgerber):
# -ltermlib removed (presumably because -lcurses==ncurses already includes it)
# -DUSE_UU_LOCK and -lutil added for uu_lock()
# -DNDSYSERRLIST changed to -DUSE_STRERROR
#If this gives you trouble use the previous entry.
#NOTE: The openbsd and openbsd+ssl should be reworked to be like the
#corresponding FreeBSD and NetBSD targets.  The mirbsd targets should
openbsd:
	@echo Making C-Kermit $(CKVER) for OpenBSD 2.3 or later...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET -DOPENBSD \
	-DHERALD=\"\\\" OpenBSD `uname -r`\\\"\" \
	-DUSE_UU_LOCK -DFNFLOAT -DUSE_STRERROR $(KFLAGS) -O" \
	"LIBS= -lcurses -lutil -lm"

#Better to chain to the openbsd target but...
mirbsd:
	@echo Making C-Kermit $(CKVER) for OpenBSD 2.3 or later...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET -DOPENBSD \
	-DHERALD=\"\\\" MirBSD `uname -r`\\\"\" \
	-DUSE_UU_LOCK -DFNFLOAT -DUSE_STRERROR $(KFLAGS) -O" \
	"LIBS= -lcurses -lutil -lm"

#OpenBSD 3.0 or later includes OpenSSL
#Add -DMAINTYPE=int if you get complaints about main: return type is not int.
#For C-Kermit 8.0 (Christian Weisgerber):
# -ltermlib removed (presumably because -lcurses==ncurses already includes it)
# -DUSE_UU_LOCK and -lutil added for uu_lock()
# -DNDSYSERRLIST changed to -DUSE_STRERROR
#If this gives you trouble use the previous entry.
openbsd+ssl:
	@echo Making C-Kermit $(CKVER) for OpenBSD 3.0 or later...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET -DOPENBSD \
	-DHERALD=\"\\\" OpenBSD `uname -r`\\\"\" \
	-DUSE_UU_LOCK -DFNFLOAT -DUSE_STRERROR -DCK_AUTHENTICATION \
	-DCK_SSL $(KFLAGS) -O" \
	"LIBS= -lcurses -lutil -lm -lssl -lcrypto"

mirbsd+ssl:
	@echo Making C-Kermit $(CKVER) for OpenBSD 3.0 or later...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET -DOPENBSD \
	-DHERALD=\"\\\" MirBSD `uname -r`\\\"\" \
	-DUSE_UU_LOCK -DFNFLOAT -DUSE_STRERROR -DCK_AUTHENTICATION \
	-DCK_SSL -DNO_DCL_INET_ATON $(KFLAGS) -O" \
	"LIBS= -lcurses -lutil -lm -lssl -lcrypto"

# make 386bsd 0.0new, posix
# for  386bsd 0.1.24, change /usr/include/termios.h to #define NCCS if
#  _POSIX_SOURCE is #defined. (source: lewine, posix prgmrs guide, o`reilly)
#NOTE: Lock directory is /var/spool/lock.  Formerly, it was /var/spool/uucp,
#but reportedly <wjones@halcyon.com> that was due to a typo in 'man tip'.
386bsd:
	@echo 'Making C-Kermit $(CKVER) for jolix 386BSD 0.0new and 0.1.24...'
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DSETREUID -DPIDSTRING -DUSLEEP \
	-D_386BSD -DCK_CURSES -DTCPSOCKET \
	-DLOCK_DIR=\\\"/var/spool/lock\\\" \
	$(KFLAGS) -O" "LNKFLAGS = -s" "LIBS = -lcurses -ltermcap"

# Mac OS X 10 early versions.
# For 10.3.9 and later, use the macosx target below.

#Mac OS X 1.0 (Rhapsody, Darwin) -- TCP/IP but no curses.
oldmacosx10:
	@echo Making C-Kermit $(CKVER) for `uname -s`...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 -DTCPSOCKET -DUSE_STRERROR -O $(KFLAGS)"

#Mac OS X 1.0 (Rhapsody, Darwin) -- TCP/IP and curses.
#Note: curses must be obtained separately.  See next entry for ncurses.
#Add "LIBS = -lcurses -ltermcap" if necessary (but reportedly it is not).
oldmacosx10c:
	@echo Making C-Kermit $(CKVER) for `uname -s` + curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 -DCK_CURSES -DTPUTSFNTYPE=void -DTPUTSISVOID \
	-DTCPSOCKET -DUSE_STRERROR -O $(KFLAGS)"

#Mac OS X 1.0 (Rhapsody, Darwin) -- TCP/IP and ncurses.
#Note: ncurses must be obtained separately.
#In the event of trouble with this one try the next one.
oldmacosx10nc:
	@echo Making C-Kermit $(CKVER) for `uname -s` + ncurses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 -DCK_NCURSES -DTCPSOCKET -DUSE_STRERROR -O \
	$(KFLAGS)" "LIBS= -lncurses $(LIBS)"

#Mac OS X 10.2 (Jaguar) ncurses.
oldmacosx102nc:
	@echo Making C-Kermit $(CKVER) for `uname -s` + ncurses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 -DCK_NCURSES -DTCPSOCKET -DUSE_STRERROR -O \
	$(KFLAGS) " "LIBS= -lncurses $(LIBS)"

#The problem here is that if curses.h also exists, it conflicts with
#ncurses.h and and we have fatal errors.  If this happens to you, then
#try this target.
oldmacosx10ncx:
	@echo Making C-Kermit $(CKVER) for `uname -s` + ncurses...
	@rm -f ./curses.h; touch ./curses.h
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 -DCK_NCURSES -DTCPSOCKET -DUSE_STRERROR \
	-I. -O $(KFLAGS) " \
	"LIBS= -lncurses $(LIBS)"
	@rm -f ./curses.h

#Mac OS X 10.3 (Panther) - Assumes ncurses is installed.
oldmacosx103: 
	@echo Making C-Kermit $(CKVER) for `uname -s` + ncurses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 -DMACOSX103 -DCK_NCURSES -DTCPSOCKET -DCKHTTP \
	-DUSE_STRERROR -DUSE_NAMESER_COMPAT -O \
	$(KFLAGS) " "LIBS= -lncurses -lresolv $(LIBS)"

#Mac OS X 10.3 (Panther) with Kerberos 5 and SSL, assumes ncurses is installed.
oldmacosx103+secure:
	@echo Making Secure C-Kermit $(CKVER) for `uname -s` + ncurses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 -DMACOSX103 -DCK_NCURSES -DTCPSOCKET \
	-DUSE_STRERROR -DUSE_NAMESER_COMPAT -O -DCK_PAM \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 -DZLIB \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL \
	$(KFLAGS) " "LIBS= -lssl -lcrypto -lkrb5 -lcom_err \
	-lk5crypto -lgssapi_krb5 -lpam -lncurses -lresolv $(LIBS)"

# THIS IS THE MAIN MAC OS X TARGET (the next one is for Kerberos/SSL builds).
# Use this target for 10.3.9 (or maybe earlier) through 10.6 (maybe later)
# on both Power and Intel architectures.  This one uses utmp.h on 10.4 and
# earlier and utmpx.h on 10.5 onwards.
# Note: Mac OS X 10.5 and earlier are 32-bit; 10.6 and later 64-bit.
# Note 2: As of C-Kermit 9.0 -NOUUCP is included by default because
# Mac OS X doesn't support UUCP.  To undo this, use KFLAGS=-UNOUUCP.
#OK: 2011/06/14 (for 10.4.11, 10.5.8, 10.6.7)
macosx macosx10 macosx10.3.9 macosx10.4 macosx10.5 macosx10.6:
	@MACOSNAME=`/usr/bin/sw_vers -productName`; \
	MACOSV=`/usr/bin/sw_vers -productVersion`; \
	echo Making C-Kermit $(CKVER) for $$MACOSNAME $$MACOSV... ; \
	MACCPU=$$HOSTTYPE; \
	if test `uname -r | cut -d . -f 1` -gt 8; \
	then if test -f /usr/include/utmpx.h ; \
	then HAVE_UTMPX='-DHAVEUTMPX -D_UTMPX_COMPAT' ; \
	else HAVE_UTMPX='' ; fi ; fi; \
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 -DMACOSX103 -DCK_NCURSES -DTCPSOCKET -DCKHTTP \
	-DUSE_STRERROR -DUSE_NAMESER_COMPAT -DNOCHECKOVERFLOW -DFNFLOAT \
	-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 $$HAVE_UTMPX \
	-funsigned-char -DNODCLINITGROUPS \
	-DNOUUCP -O -DHERALD=\"\\\" $${MACOSNAME} $${MACOSV}\\\"\" \
	-DCKCPU=\"\\\"$${MACCPU}\\\"\" \
	$(KFLAGS)" "LIBS= -lncurses -lresolv $(LIBS)"

# Mac OS X 10.3.9 or later with Kerberos 5 and OpenSSL...
# NOTE: Apple has removed all support for DES in OpenSSL and Kerberos
#   in Mac OS X 10.6 and later.  The DES flags are included or left out
#   automatically based on the Mac OS X version number.
# See note about UUCP in previous target.
#OK: 2009/11/16 (for 10.3.9, 10.4.11, 10.5.8, 10.6.1)
#OK: 2011/06/14 (for 10.4.11, 10.5.8, 10.6.7)
macosx+krb5+ssl macosx10.5+krb5+ssl macosx10.6+krb5+ssl \
macosx+krb5+openssl macosx10.5+krb5+openssl macosx10.6+krb5+openssl:
	@MACOSNAME=`/usr/bin/sw_vers -productName`; \
	MACOSV=`/usr/bin/sw_vers -productVersion`; \
	echo Making C-Kermit $(CKVER) for $$MACOSNAME $$MACOSV... ; \
	MACCPU=$$HOSTTYPE; \
	if test `uname -r | cut -d . -f 1` -gt 8; \
	then if test -f /usr/include/utmpx.h ; \
	then HAVE_UTMPX='-DHAVEUTMPX -D_UTMPX_COMPAT' ; \
	else HAVE_UTMPX='' ; fi ; fi; \
	if test `uname -r | cut -d . -f 1` -eq 7; \
	then IS_MACOSX103='-DMACOSX103' ; \
	else IS_MACOSX103='' ; fi; \
	case $$MACOSV in \
	  10.[012345].*) HAVE_DES='-DCK_DES -DLIBDES' ;; \
	  *.*) HAVE_DES='' ;; \
	esac ; \
	if test -x /usr/bin/krb5-config ; \
	then HAVE_KRB5CONFIG=`/usr/bin/krb5-config --libs krb5 gssapi` ; \
	else HAVE_KRB5CONFIG='-lgssapi_krb5 -lkrb5 -lk5crypto \
	-lcom_err -lresolv' ; fi; \
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMACOSX10 $$IS_MACOSX103 -DCK_NCURSES -DTCPSOCKET \
	-DUSE_STRERROR -DUSE_NAMESER_COMPAT -DNOCHECKOVERFLOW -DFNFLOAT \
	-DCKHTTP -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 $$HAVE_UTMPX \
	-DNODCLINITGROUPS -DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 -DZLIB \
	-DCK_ENCRYPTION -DCK_CAST -DCK_SSL -DOPENSSL_098 $$HAVE_DES \
	-DNOUUCP -DHERALD=\"\\\" $${MACOSNAME} $${MACOSV}\\\"\" \
	-DCKCPU=\"\\\"$${MACCPU}\\\"\" \
	-funsigned-char -O $(KFLAGS)" \
	"LIBS= $$HAVE_KRB5CONFIG -lssl -lcrypto -lpam -lncurses $(LIBS)"

# End of Mac OS X Section

#Acorn RISCiX, based on ...
#Berkeley Unix 4.2 or 4.3 with lock directory /usr/spool/uucp/LCK/LCK..ttyxx,
#but without acucntrl program
riscix:
	@echo Making C-Kermit $(CKVER) for RISCiX, /usr/spool/uucp/LCK..ttyxx
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
		"CFLAGS= -DBSD42 -DBSD4 -DRISCIX -DNOCSETS \
		-DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DDIRENT -DCK_CURSES \
		-DMAXSP=9024 -DMAXRD=9024 -DSBSIZ=9050 -DRBSIZ=9050 \
		-DDFTTY=\\\"/dev/serial\\\" -DNOCSETS -DNOCYRIL \
		-DNOANSI -w -O2 -fomit-frame-pointer" \
		"LIBS= -lcurses -ltermcap " \
		"CC= /usr/ucb/cc" \
		"CC2= /usr/ucb/cc"

#Acorn RISCiX, as above, but using gcc
riscix-gcc:
	@echo Making C-Kermit $(CKVER) for RISCiX, /usr/spool/uucp/LCK..ttyxx
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
		"CFLAGS= -DBSD42 -DBSD4 -DRISCIX -DNOCSETS \
		-DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DDIRENT -DCK_CURSES \
		-DMAXSP=9024 -DMAXRD=9024 -DSBSIZ=9050 -DRBSIZ=9050 \
		-DDFTTY=\\\"/dev/serial\\\" -DNOCSETS -DNOCYRIL \
		-DNOANSI -w -O2 -fomit-frame-pointer" \
		"LIBS= -lcurses -ltermcap " \
		"CC= gcc -mbsd" \
		"CC2= gcc -mbsd"

#Convergent CTIX 6.4.1
ctix:
	@echo 'Making C-Kermit $(CKVER) for Convergent CTIX 6.4.1'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSVR3 -DDIRENT -DTCPSOCKET -DHDBUUCP -DCK_CURSES \
	-DNONAWS -DNOLEARN -DNOLONGLONG $(KFLAGS) -XO" \
	"LNKFLAGS=-s" "LIBS=-lsocket -lcurses -lc_s"
	mcs -d wermit

# The following makefile entry should work for any Harris Night Hawk system
# (either 88k or 68k based) running release 6.1 or later of the CX/UX
# operating system. This is a POSIX and ANSI-C compliant system which also
# supports BSD networking. (Earlier CX/UX releases will probably work with
# sys5r3, but this has not been verified).
#
cx_ux:
	@echo Making C-Kermit $(CKVER) for Harris Night Hawk CX/UX 6.1 or later
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS=-DPOSIX -DTCPSOCKET -DHDBUUCP -DPID_T=pid_t -DWAIT_T=int \
	-Dd_ino=d_fileno -DUID_T=uid_t -DGID_T=gid_t -DNOLONGLONG \
	$(KFLAGS) -Xa -O3 -g" "LNKFLAGS=-O3"

#Intergraph Clipper, CLIX, job control, HDB UUCP.
clix:
	@echo 'Making C-Kermit $(CKVER) for Intergraph CLIX...'
	$(MAKE) wermit "CC=acc" "CC2=acc" KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -w -DSVR3 -DCLIX -DDIRENT -DHDBUUCP -DNOSYSLOG -DUSE_MEMCPY \
	-DNOGETUSERSHELL -DNOREALPATH -DNOLEARN $(KFLAGS) -O" \
	"LNKFLAGS=" "LIBS= -lbsd"

#As above + TCP/IP...
clixnet:
	@echo 'Making networked C-Kermit $(CKVER) for Intergraph CLIX...'
	$(MAKE) wermit "CC=acc" "CC2=acc" KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -w -DSVR3 -DCLIX -DDIRENT -DHDBUUCP -DNOSYSLOG -DUSE_MEMCPY \
	-DTCPSOCKET -DNOGETUSERSHELL -DNOLEARN -DNOREALPATH $(KFLAGS) -O" \
	"LNKFLAGS=" "LIBS= -lbsd"

#Mark Williams Coherent 286 or 386 on IBM PC family.
#There is a 64K limit on program size, so this is a command-line only version.
coherent:
	$(MAKE) "CFLAGS = -O -DCOHERENT -DNOANSI -DNOICP -DNOSETKEY -DNOLEARN \
	-DNOCSETS -DNOHELP -DNODIAL -DNOSCRIPT -DNODEBUG -DNOTLOG -DNOXMIT \
	-DNOMSEND -DNOFRILLS -DNOSYSIOCTLH -DSELECT_H $(KFLAGS) -VSUVAR" \
	-DNOFLOAT KTARGET=$${KTARGET:-$(@)} wermit

#Mark Williams Coherent 386 on IBM PC family.
#This will make a "minimum interactive" version - no scripts,
#no character sets, no help, no dial, no debug/transaction logging, no
#transmit, msend, mail, type, etc.
coherentmi:
	$(MAKE) "CFLAGS = -O -DCOHERENT -DNOANSI -DNOSETKEY -DNOLEARN \
	-DNOSHOW -DNOCSETS -DNOHELP -DNODIAL -DNOSCRIPT -DNODEBUG -DNOTLOG \
	-DNOXMIT -DNOMSEND -DNOFRILLS -DNOSYSIOCTLH -DNOSERVER -DNOUUCP \
	-DNOSPL -DNOPUSH -DNOMDMHUP -DNOJC -DNOFDZERO -DNOESCSEQ -DNOFLOAT \
	-DNOCMDL $(KFLAGS) -VSUVAR -DSELECT_H" KTARGET=$${KTARGET:-$(@)} \
	wermit

#Mark Williams Coherent 386 on IBM PC/AT family.
coherentmax:
	$(MAKE) "CFLAGS = -O -DCOHERENT -DNOANSI -DSELECT_H -DNOLEARN \
	-DNOFLOAT -DNOSYSIOCTLH $(KFLAGS) -VSUVAR" "LNKFLAGS = -O -s" \
	KTARGET=$${KTARGET:-$(@)} wermit

#Mark Williams Coherent 386 4.2.  Includes curses but not TCP/IP.
#Requires updates to the 4.2.10 compiler; the regular compiler fails to
#to handle "complex expressions".  NOFLOAT is so it can work on old PCs
#without floating-point hardware.
coherent42:
	$(MAKE) "CFLAGS = -T500000 -DNOFLOAT -DCOHERENT -DNOANSI -DSELECT \
	-DNOSYSLOG -DDIRENT -DCK_CURSES -DCK_NEWTERM -DCK_WREFRESH -VSUVAR \
	-DDCLGETCWD -DNOSYSIOCTLH -DNOINITGROUPS -DNOSYMLINK -DSELECT_H \
	-DDCLGETCWD -O $(KFLAGS)" \
	"LNKFLAGS = -O -s" KTARGET=$${KTARGET:-$(@)} \
	"LIBS  = -lsocket -lcurses" wermit

#DEC Ultrix 2.x
# Add -O, -DDYNAMIC, -s, etc, if they work.
ultrix2x:
	@echo Making C-Kermit $(CKVER) for Ultrix 2.x ...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DDU2 -DNOGETUSERSHELL $(KFLAGS)"

du2:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET-$(@)} ultrix2x

#DEC Ultrix 3.0 and 3.1
ultrix30:
	@echo Making C-Kermit $(CKVER) for Ultrix 3.0...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DDIRENT -DSIG_V -DNOGETUSERSHELL \
	-DULTRIX3 -DCK_CURSES $(KFLAGS) -O" "LIBS= -lcurses -ltermcap"

du3:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET-$(@)} ultrix30

ultrix3x:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET-$(@)} ultrix30

#DEC Ultrix 4.0 or 4.1 on DECstation, VAXstation, VAX, etc.
ultrix40:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.0 or 4.1...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DDU4 -DNOGETUSERSHELL \
	$(KFLAGS) -Olimit 1450" "LNKFLAGS = -s"

#DEC Ultrix 4.2-4.5 on DECstation, DECsystem, VAXstation, VAX, etc.
#Like ultrix40, except now C compiler supports -O2 optimization.
ultrix42:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.2 or later...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DNOGETUSERSHELL $(KFLAGS) \
	-O2 -Olimit 1750" "LNKFLAGS = -s"

du42:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET-$(@)} ultrix42

#DEC Ultrix 4.2-4.5 on DECstation, DECsystem, VAXstation, VAX, etc.
#Like du42, but with curses support added and a couple features.
ultrix42c:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.2 or later...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DNOGETUSERSHELL \
	-DCK_CURSES -DNOIKSD $(KFLAGS)-G6 -O2 -Olimit 3000 " \
	"LNKFLAGS = -s" "LIBS= -lcurses -ltermcap"

ultrix43:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) \
	"KFLAGS=-DULTRIX43 $(KFLAGS)" KTARGET=$${KTARGET-$(@)} ultrix42c

ultrix43notcp:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) \
	"KFLAGS=-DULTRIX43 -DNONET $(KFLAGS)" \
	KTARGET=$${KTARGET-$(@)} ultrix42c

# NOTE: need -DNODEBUG on MIPS to avoid relocation errors at link time.
# Actually now (8.0) that we have discovered the -G option maybe debugging
# can be put back.
ultrix44:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) \
	"KFLAGS=-DULTRIX44 -G7 -DNODEBUG -DNETPTY -DNO_DEVTTY $(KFLAGS)" \
	KTARGET=$${KTARGET-$(@)} ultrix42c

ultrix45:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) \
	"KFLAGS=-DULTRIX45 $(KFLAGS)-DNETPTY -DNO_DEVTTY $(KFLAGS)" \
	KTARGET=$${KTARGET-$(@)} ultrix42c

du42c:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) \
	KTARGET=$${KTARGET-$(@)} ultrix42c

#DEC Ultrix 4.3A or later on DECsystem and DECstation 5000/50, /150 or /260
#with MIPS R4x00 processor.  The "-mips3" switch generates R4000-specific
#code, which is faster and more compact, but *won't* run on earlier
#DECsystems and DECstations.
ultrix43-mips3:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.3A or later, R4000 cpu...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DNOGETUSERSHELL \
	$(KFLAGS) -O2 -Olimit 1750 -mips3" "LNKFLAGS = -s -mips3"

du43-mips3:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) ultrix43-mips3

#DEC Ultrix 4.3A or later on MIPS R4x000 based systems.
#Like ultrix43-mips3 but with curses support added
ultrix43c-mips3:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.3A or later, R4000 cpu...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DNOGETUSERSHELL -DCK_CURSES \
	$(KFLAGS) -O2 -Olimit 3000 -mips3" "LNKFLAGS = -s -mips3" \
	"LIBS= -lcurses -ltermcap"

du43c-mips3:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) \
	KTARGET=$${KTARGET-$(@)} ultrix43c-mips3

#DEC Ultrix 4.4 on DECstation 5000/50 or /150 with R4000 MIPS processor,
#or 5000/260 with R4400.  The "-mips3" switch generates R4000-specific code,
#which is faster and more compact but *won't* run on earlier DECstations.
ultrix44-mips3:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.4, R4000 cpu ...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DNOGETUSERSHELL \
	$(KFLAGS) -O2 -Olimit 1450 -mips3" "LNKFLAGS = -s -mips3"

du44-mips3:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) \
	KTARGET=$${KTARGET-$(@)} ultrix44c-mips3

#DEC Ultrix 4.2 on DECstation, VAXstation, VAX, etc, System V R4 environment
ultrix42s5r4:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4 on Ultrix...'
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS = -O2 -Olimit 1500 -DSVR4 -DDIRENT -DHDBUUCP -DNOGETUSERSHELL \
	-DTCPSOCKET $(KFLAGS)" "LNKFLAGS = -s"

#OSF/1
osf osf1:
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD4 -DOSF -D_BSD -DTCPSOCKET -DCK_ANSIC -DSIG_V \
	-DCK_CURSES -DCK_RTSCTS -DFNFLOAT $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = $(LIBS) -lbsd -lcurses -ltermcap -lm"

#DEC OSF/1 V1.0-1.3 on DECstation, VAX, Alpha, or PC.
dec-osf dec-osf1:
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD4 -DOSF -DOSF13 -D_BSD -DTCPSOCKET -DCK_ANSIC -DSIG_V \
	-DNOREALPATH -DNOIKSD -DCK_CURSES -DCK_RTSCTS -DFNFLOAT -DNODEBUG \
	-DNOUNICODE $(KFLAGS)" \
	"LNKFLAGS = -non_shared" "LIBS = -lbsd -lcurses -ltermcap -lm"

# This one causes "relocation out-of-range" errors in the linker.
old-dec-osf:
	@echo Making C-Kermit $(CKVER) for DEC OSF/1 V1.x...
	@echo If you are building for DEC OSF/1 2.0, please use dec-osf20.
	@echo Remove or adjust -O2 and/or -Olimit if they cause trouble.
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -O2 -Olimit 2400 $(KFLAGS)"

#DEC OSF/1 2.0 on Alpha and probably nowhere else.
#The only difference from OSF/1 is that optimization is omitted.
#The optimized version gets strange runtime errors, like the PAUSE command
#not working.  Add "-unsigned" to make all chars unsigned.
dec-osf20:
	@echo Making C-Kermit $(CKVER) for DEC OSF/1 V2.0...
	@echo Optimization omitted because it causes runtime errors.
	@echo See comments in makefile.
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DOSF20 $(KFLAGS)"

dec-osf30:
	@echo Making C-Kermit $(CKVER) for DEC OSF/1 V3.0...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DOSF30 -O2 -Olimit 2400 $(KFLAGS)"

#Digital UNIX 3.2
# Must compile ckuus[6x].c separately without optimization otherwise
# the optimizer dumps core - keep CFLAGS here in sync with those from osf.
du32:
	@echo Making C-Kermit $(CKVER) for Digital UNIX 3.2...
	$(MAKE) CC=$(CC) CC2=$(CC2) ckuus6.$(EXT) \
	"CFLAGS= -DBSD4 -DOSF -D_BSD -DTCPSOCKET -DCK_ANSIC -DSIG_V \
	-DCK_CURSES -DCK_RTSCTS -DFNFLOAT -DOSF32 -DHDBUUCP $(KFLAGS)"
	$(MAKE) CC=$(CC) CC2=$(CC2) ckuusx.$(EXT) \
	"CFLAGS= -DBSD4 -DOSF -D_BSD -DTCPSOCKET -DCK_ANSIC -DSIG_V \
	-DCK_CURSES -DCK_RTSCTS -DFNFLOAT -DOSF32 -DHDBUUCP $(KFLAGS)"
	$(MAKE) CC=$(CC) CC2=$(CC2) osf \
	"KFLAGS= -DOSF32 -DHDBUUCP -O2 -Olimit 3200 $(KFLAGS)"

dec-osf32:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) du32 \
	KTARGET=$${KTARGET:-$(@)}

#Digital UNIX 4.0 through 4.0D (use tru64 targets for 4.0E and above)...
du40:
	@echo Making C-Kermit $(CKVER) for Digital UNIX 4.0...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DOSF40 -DHDBUUCP -DFNFLOAT \
	-unsigned -std1 -O3 -Olimit 2400 $(KFLAGS)" "LIBS=-lm"

du40gcc:
	@echo Making C-Kermit $(CKVER) for Digital UNIX 4.0 with gcc ...
	$(MAKE) osf CC=gcc CC2=gcc KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DOSF40 -DHDBUUCP $(KFLAGS)"

#Tru64 Unix 4.0E
tru64-40e:
	@echo Making C-Kermit $(CKVER) for Tru64 UNIX 4.0E...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DOSF40 -DOSF40E -DTRU64 -DHDBUUCP -DFNFLOAT -DNOCOTFMC \
	-unsigned -std1 -O3 -Olimit 2400 $(KFLAGS)" "LIBS=-lm"

tru64-40f:
	@echo Making C-Kermit $(CKVER) for Tru64 UNIX 4.0F...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DOSF40 -DOSF40F -DTRU64 -DHDBUUCP -DFNFLOAT -DNOCOTFMC \
	-unsigned -std1 -O3 -Olimit 2400 $(KFLAGS)" "LIBS=-lm"

tru64-40g:
	@echo Making C-Kermit $(CKVER) for Tru64 UNIX 4.0G...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DOSF40 -DOSF40G -DTRU64 -DHDBUUCP -DFNFLOAT -DNOCOTFMC \
	-unsigned -std1 -O3 -Olimit 2400 $(KFLAGS)" "LIBS=-lm"

tru64-50a:
	@echo Making C-Kermit $(CKVER) for Tru64 UNIX 5.0A...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DTRU64 -DOSF50 -DHDBUUCP \
	-unsigned -std1 -O3 -Olimit 2400 $(KFLAGS)"

tru64-51a:
	@echo Making C-Kermit $(CKVER) for Tru64 UNIX 5.1A...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DTRU64 -DOSF50 -DOSF51A -DHDBUUCP \
	-unsigned -std1 -O3 -Olimit 2400 $(KFLAGS)"

tru64-51b:
	@echo Making C-Kermit $(CKVER) for Tru64 UNIX 5.1B...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DTRU64 -DOSF50 -DOSF51A -DOSF51B -DHDBUUCP \
	-unsigned -std1 -O3 -Olimit 2400 $(KFLAGS)"

# Added 5.1b version with OpenSSL - CDW 6-13-2005...
tru64-51b+openssl:
	@echo Making C-Kermit $(CKVER) for Tru64 UNIX 5.1b
	@echo  including OpenSSL...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DTRU64 -DOSF50 -DOSF51A -DOSF51B -DHDBUUCP \
	-unsigned -std1 -O3 -Olimit 2400 \
	-DCK_AUTHENTICATION -DCK_SSL $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -rpath $(sslroot)/ssl/lib -lssl -lcrypto"

du50:
	$(MAKE) CC=$(CC) CC2=$(CC2) tru64-50a KTARGET=$${KTARGET:-$(@)}

du40-ridiculous-checking:
	@echo Making C-Kermit $(CKVER) for Digital UNIX 4.0.
	@echo Checking everything - assumes DECC...
	$(MAKE) CC=$(CC) CC2=$(CC2) osf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= -DOSF40 -DHDBUUCP -w0 -warnprotos -check -portable \
	-unsigned -std1 -O3 -Olimit 1760 $(KFLAGS)"

#Sequent DYNIX/ptx 1.2.1
dynixptx12:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX/ptx 1.2.1...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DPTX -DNOGETUSERSHELL -DNOLEARN \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#Sequent DYNIX/ptx 1.3 or 1.4
dynixptx13:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX/ptx 1.3 TCP/IP...
	$(MAKE) xermit "CFLAGS= -O KTARGET=$${KTARGET:-$(@)} \
	-DSVR3 -DDIRENT -DHDBUUCP -DPTX -DCK_POLL -DNOGETUSERSHELL \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DTCPSOCKET $(KFLAGS) -i" \
	"LNKFLAGS = -i" "LIBS = -lsocket -linet -lnsl"

#Sequent DYNIX/ptx 2.0, ANSI C compilation
#Should work on any hardware platform when DYNIX/ptx runs, including
#386, 486, Pentium.
dynixptx20:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 2.0...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DHDBUUCP -DTCPSOCKET \
	-DWAIT_T=int -DPTX -DNOGETUSERSHELL $(KFLAGS) -O" \
	"LIBS = -lsocket -linet -lnsl"

#Sequent DYNIX/ptx 2.0, ANSI C compilation, with curses
dynixptx20c:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 2.0...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DHDBUUCP -DTCPSOCKET -DWAIT_T=int -DPTX -DCK_CURSES \
	-DCK_NEWTERM -DNOGETUSERSHELL $(KFLAGS) -O" \
	"LIBS = -lsocket -linet -lnsl -lcurses -ltermcap"

#Sequent DYNIX/ptx 2.1.6, 80486, ANSI C compilation, with curses:
# -Xa -- use ANSI compiler.
# -Wc,-pw -- suppress portability warnings.
# -Wc,-i386 -- 80386 cpu.
# -Wc,-i486 -- 80486 cpu.
# -Wc,-P5 -- Pentium (default).
# -Wc,-O3 -- highest optimization.
# -Wa,-N17061 -- increase symbol table from default of 15013 for ckcuni.c.
# Early versions of DYNIX/ptx 2.1.x may need -DCK_POLL instead of -DSELECT.
# Add "$&" after the colon in the "xermit" target for parallel makes.
dynixptx216c:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 2.1.6'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DHDBUUCP -DDYNAMIC -DTCPSOCKET \
	-DSELECT -DCK_REDIR -DCK_NAWS -DCK_WREFRESH -DSW_ACC_ID \
	-DTCP_NODELAY=1 -DTRMBUFL=2048 -DBIGBUFOK -DHADDRLIST \
	-DPTX  -DCK_CURSES -DCK_NEWTERM -DNOGETUSERSHELL -DNOREALPATH \
	$(KFLAGS) -Xa -Wc,-pw -Wc,-i486 -Wc,-O3 -Wa,-N17061" \
	"LIBS = -lXbsd -lseq -lsocket -linet -lnsl -lmalloc -lm -lcurses" \
	"LNKFLAGS = -s"

#Sequent DYNIX/ptx 2.1.6, gcc 2.7.2.2, with curses:
dynixptx216cgcc:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 2.1.6 gcc'
	$(MAKE) xermit "CC = gcc" "CC2 = gcc" KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DHDBUUCP -DDYNAMIC -DTCPSOCKET \
	-DSELECT -DCK_REDIR -DCK_NAWS -DCK_WREFRESH -DSW_ACC_ID \
	-DTCP_NODELAY=1 -DTRMBUFL=2048 -DBIGBUFOK -DHADDRLIST \
	-DPTX  -DCK_CURSES -DCK_NEWTERM -DNOGETUSERSHELL -DNOREALPATH \
	$(KFLAGS) -O3 -pipe -funsigned-char" \
	"LIBS = -lXbsd -lseq -lsocket -linet -lnsl -lmalloc -lm -lcurses" \
	"LNKFLAGS = -s"

#Sequent DYNIX/ptx 4.0, ANSI C compilation, with curses
dynixptx41c:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 4.0...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DHDBUUCP -DTCPSOCKET \
	-DWAIT_T=int -DPTX -DPTX4 -DCK_CURSES -DCK_NEWTERM \
	-DNOGETUSERSHELL $(KFLAGS) -O" \
	"LIBS = -lsocket -lnsl -lcurses -ltermcap"

#Sequent DYNIX/ptx 4.4, ANSI C compilation, with curses
dynixptx44:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 4.4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPTX -DPTX4 -DPOSIX -DHDBUUCP -DTCPSOCKET -DWAIT_T=int \
	-DCK_CURSES -DCK_NEWTERM -DBIGBUFOK -DSELECT -DNOGETUSERSHELL \
	$(KFLAGS) -O" "LIBS = -lsocket -lnsl -lcurses -ltermcap"

#Sequent DYNIX 3.0.x
dynix3:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX 3.0.x...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD43 -DACUCNTRL -DTCPSOCKET -O \
	-DPWUID_T=int -DGID_T=int $(KFLAGS)"

#Sequent DYNIX 3.0.x, no ACUCNTRL
dynix3noacu:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX 3.0.x...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD43 -DLCKDIR -DTCPSOCKET -O \
	-DUID_T=int -DGID_T=int $(KFLAGS)"

#Sequent DYNIX 3.1.x
dynix31:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX 3.1.x...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DDCLPOPEN -DLCKDIR -DBSD4 -DTCPSOCKET $(KFLAGS)"

#Sequent DYNIX 3.1.2, as above but with curses, to be compiled by gcc 2.3.3.
dynix31c:
	@echo 'Making C-Kermit $(CKVER) for Sequent DYNIX 3.1.2, curses...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O2 -DDCLPOPEN -DACUCNTRL \
	-DBSD43 -DTCPSOCKET -DCK_CURSES -DUID_T=int \
	$(KFLAGS)" "LIBS= -lcurses -ltermcap"

#Convex C1 with Berkeley Unix
convex:
	@echo Making C-Kermit $(CKVER) for Convex C1 / BSD...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD4 -DNOLEARN $(KFLAGS) -Dmsleep=mnap"

#Convex C210 with Convex/OS 8
convex8:
	@echo Making C-Kermit $(CKVER) for Convex C210 with OS 8
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DNODEBUG -DDIRENT -DNOFILEH \
	$(KFLAGS) -DSIG_V -Dmsleep=mnap"

#Convex C2 with Convex OS 9.1 (should also work with 8.1 or later)
#with ANSI C compiler, uses BSD 4.3 uucp lockfile convention.
convex9:
	@echo Making C-Kermit $(CKVER) for Convex C210 with OS 9.1
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DCONVEX9 -DNOIEXTEN -DDIRENT -DNOFILEH -DTCPSOCKET \
	-D__STDC__ -DLCKDIR -Dmsleep=mnap -O -ext -tm c1 $(KFLAGS)" \
	"LNKFLAGS = -ext"

#Convex C2 with Convex OS 10.1 or later
#with gcc 2.x C compiler
convex10gcc:
	@echo Making C-Kermit $(CKVER) for Convex C2 with OS 10.1 using gcc
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DCONVEX9 -DNOIEXTEN -DDIRENT -DNOFILEH -DTCPSOCKET \
	-D__STDC__  -Dmsleep=mnap -O2 $(KFLAGS)" CC=gcc CC2=gcc

#Cray X-MP or Y-MP UNICOS 6.x or 7.x.
#NOTE: NPROC tells how many parallel makes to run.  If your Cray has multiple
#processors, you can set NPROC up to the number of CPUs, e.g. NPROC=16.
cray:
	@echo 'Making C-Kermit $(CKVER) for Cray X/Y-MP UNICOS 6.x or 7.0...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} NPROC=1 \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DTCPSOCKET $(KFLAGS) -O1"

#Cray X-MP or Y-MP UNICOS 8.0 Alpha.
cray8:
	@echo 'Making C-Kermit $(CKVER) for Cray X/Y-MP UNICOS 8.0 Alpha...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} NPROC=1 \
	"CFLAGS= -DSVR4 -DDIRENT -DHDBUUCP -DTCPSOCKET $(KFLAGS) -O1"

#Cray X-MP or Y-MP UNICOS 9.0.
#This one was executed successfully for C-Kermit 8.0.209.
#Earlier versions of Unicos will probably need the same flags.
cray9:
	@echo 'Making C-Kermit $(CKVER) for Cray X/Y-MP UNICOS 9.0...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} NPROC=1 \
	"CFLAGS= -DSVR4 -DDIRENT -DHDBUUCP -DNOLFDEVNO \
	-DTCPSOCKET $(KFLAGS) -O1"

#Cray-2 or Cray 3-CSOS
#NOTE: NPROC tells how many parallel makes to run.  If your Cray has multiple
#processors, you can set NPROC up to the number of CPUs, e.g. NPROC=16.
craycsos:
	@echo 'Making C-Kermit $(CKVER) for Cray-2/3 CSOS
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} NPROC=1 \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DTCPSOCKET \
	$(KFLAGS) -DCK_ANSIC -DCK_CURSES" "LIBS=-lnet"

#NeXTSTEP 1.0 through 3.2.
#Includes fullscreen file transfer display (curses) and TCP/IP support.
#Uses shared library to make executable program about 80K smaller.
#Remove "LIBS = -lsys_s" if this causes trouble.
next:
	@echo Making C-Kermit $(CKVER) for NeXTSTEP...
	@echo 'If you get errors in ckutio.c about w_S, w_T, etc,'
	@echo 'add KFGLAGS=-DNOREDIRECT to your make command.'
	$(MAKE) xermit CC=$(CC) CC2=$(CC2) KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DNEXT -DTCPSOCKET -DLCKDIR -DNOPUTENV -DFNFLOAT \
	-pipe -DCK_CURSES $(KFLAGS) -O -w" "LIBS = -lsys_s -lcurses -ltermcap"

nextc:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) next \
	KTARGET=$${KTARGET:-$(@)}

nextg:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) next \
	KFLAGS=-Wall KTARGET=$${KTARGET:-$(@)}

nextgc:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) next \
	KFLAGS=-Wall KTARGET=$${KTARGET:-$(@)}

#NeXTSTEP 3.3.
#Includes fullscreen file transfer display and TCP/IP.
# You might have to add 1 line to 1 NeXT header file <ip.h>
# to declare n_long as u_long by adding #include <bsd/netinet/in_systm.h>

next33:
	@echo Making C-Kermit $(CKVER) for NeXTSTEP 3.3...
	$(MAKE) xermit CC=$(CC) CC2=$(CC2) KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DNEXT33 -DTCPSOCKET -DLCKDIR -DNOPUTENV -DFNFLOAT \
	-pipe -DCK_CURSES $(KFLAGS) -O -w" "LIBS = -lsys_s -lcurses -ltermcap"

#OPENSTEP 4.2 for Sparc, m680x0, HP PA-RISC, and Intel.
#Includes fullscreen file transfer display and TCP/IP.
#ckcpro.c compiled without optimization because it crashes the compiler.
openstep42:
	@echo Making C-Kermit $(CKVER) for OPENSTEP 4.2...
	$(MAKE) ckcpro.$(EXT) \
	"CFLAGS= -DOPENSTEP42 -DNEXT33 -DTCPSOCKET -DLCKDIR -DNOPUTENV \
	-DFNFLOAT -pipe -DCK_CURSES $(KFLAGS) -w"
	$(MAKE) xermit CC=$(CC) CC2=$(CC2) KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DOPENSTEP42 -DNEXT33 -DTCPSOCKET -DLCKDIR -DNOPUTENV \
	-DFNFLOAT -pipe -DCK_CURSES $(KFLAGS) -O -w" \
	"LIBS = -lsys_s -lcurses -ltermcap"

#NeXT with malloc debugger
nextmd:
	@echo Making C-Kermit $(CKVER) for NeXT with malloc debugging...
	$(MAKE) mermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DNEXT -DTCPSOCKET -DLCKDIR -DNOPUTENV -DFNFLOAT \
	-DCK_CURSES $(KFLAGS) -O -w -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG" \
	"LIBS = -lsys_s -lcurses -ltermcap"

#Build for NeXTSTEP with "fat" binaries (MABs) that run on both Motorola
#and Intel platforms.
nextfat:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) \
	next KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-Wall -arch m68k -arch i386" "LNKFLAGS = -arch m68k -arch i386"

#NeXTSTEP on Intel Platforms.
next486:
	@echo Making C-Kermit $(CKVER) for NeXTSTEP on Intel Platforms...
	@echo 'If you get errors in ckutio.c about w_S, w_T, etc,'
	@echo 'add KFGLAGS=D-DNOREDIRECT to your make command.'
	$(MAKE) xermit CC=$(CC) CC2=$(CC2) KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DNEXT -DTCPSOCKET -DLCKDIR -DNOPUTENV -DFNFLOAT \
	-DNODEBUG -O3 -fno-omit-frame-pointer -fschedule-insns2 -pipe \
	-DCK_CURSES $(KFLAGS) -w" "LIBS = -lsys_s -lcurses -ltermcap"

#Single binary that runs on NeXT 68030 and 68040, Intel, HP, and Sparc,
#as well as on OpenStep/Mach.
nextquadfat:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) next \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-Wall -arch m68k -arch i386 -arch hppa -arch sparc" \
	"LNKFLAGS = -arch m68k -arch i386 -arch hppa -arch sparc"

#BeBox
beboxdr7:
	@echo 'Making C-Kermit $(CKVER) for the BeBox...'
	@echo 'Link step will fail with default Metroworks linker 64K limit.'
	@echo 'Code Warrior Gold required to link big programs.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CC=/boot/develop/tools/mwcc" "CC2=/boot/develop/tools/mwld" \
	"CFLAGS= -DBEBOX -DBE_DR_7 -DPOSIX -DNOUUCP -DNOLEARN $(KFLAGS) -O"

#BeBox BeOS DR7 only
bebox:
	@echo 'Making C-Kermit $(CKVER) for BeBox...'
	@echo 'Link step will fail with default Metroworks linker 64K limit.'
	@echo 'Code Warrior Pro 3.0 for BeBox required to link big programs.'
	$(MAKE) wermit "CC=mwcc" "CC2=mwld" KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBEBOX -DPOSIX -DNOLEARN -DNOUUCP $(KFLAGS) -O"

#BeOS 4.5
#We have to use the wermit target because 'fd_set' is unknown.
beos45:
	$(MAKE) wermit "CC=$(CC)" "CC2=$(CC2)" KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBEOS -DBEOS45 -DPOSIX -DNOIKSD -DNOREALPATH -DSYSTIMEH \
	-DNOCOTFMC -DNOUUCP -DNOLEARN $(KFLAGS) -O" \
	"LIBS = $(LIBS)"

#BeOS 4.5
beos45net:
	$(MAKE) CC=$(CC) CC2=$(CC2) beos45 \
	"KFLAGS=-DTCPSOCKET -DNO_DNS_SRV $(KFLAGS)" "LIBS=-lnet -lnetapi"

#Plan 9 from Bell Labs
plan9:
	@echo 'C-Kermit for Plan 9 from Bell Labs - calling ckpker.mk...'
	make -f ckpker.mk

#POSIX
posix:
	@echo 'Making C-Kermit $(CKVER) for pure POSIX...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DNOUUCP -DNOLEARN $(KFLAGS) -O"

# PowerMAX OS (SVR4) from Concurrent (tested on PowerMAX 5.1)
powermax:
	@echo 'Making C-Kermit $(CKVER) for Concurrent PowerMAX OS...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DPOWERMAX \
	-DNETPTY -DHAVE_STREAMS -DHAVE_GRANTPT -DHAVE_PTSNAME -DPUSH_PTEM \
	-DPUSH_LDTERM -DPUSH_TTCOMPAT \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lresolv -lcurses -lgen -lc -lucbc"

#Berkeley Software Design Inc. BSDI
# Substitute "LIBS= -lnewcurses -ltermcap" if desired.
bsdi:
	@echo 'Making C-Kermit $(CKVER) for BSDI ...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DSETREUID -DSW_ACC_ID -DBIGBUFOK -DFIXCRTSCTS \
	-DTCPSOCKET -DCK_CURSES -DFNFLOAT $(KFLAGS) -O" \
	"LIBS= -lcurses -ltermcap -lm"

#Berkeley Software Design Inc. BSDI - has higher serial speeds than 1.x.
bsdi2:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) bsdi \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS=-DBSDI2 $(KFLAGS)"

bsdi3:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) bsdi \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS=-DBSDI2 -DBSDI3 $(KFLAGS)"

bsdi4:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) bsdi \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DBSDI2 -DBSDI3 -DBSDI4 -DTPUTSFNTYPE=void -DTPUTSISVOID \
	-DCKHTTP -m486 $(KFLAGS)"

# (old name for the above)
bsdiposix:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) bsdi


#Build a BSDI 4.x binary that also runs under FreeBSD (Terry Kennedy).
#But watch out for details like serial-port locking.
bsdix:
	$(MAKE) "MAKE=$(MAKE)" CC=$(CC) CC2=$(CC2) bsdi \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DBSDI2 -DBSDI3 -DBSDI4 -DTPUTSFNTYPE=void -DTPUTSISVOID \
	-m486 $(KFLAGS)" "LNKFLAGS=-static -Wl,-m,i386bsdi -Wl,-e,_start"

#Pyramid 9XXX (e.g. 9845) or MIServer T series, OSx 4.4b thru 5.1
pyramid:
	@echo Making C-Kermit $(CKVER) for Pyramid Dual Port OSx
	ucb $(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD43 -DTCPSOCKET -DPYRAMID -O $(KFLAGS)" "LNKFLAGS = -s"

#Pyramid Dual Port OSx using HoneyDanBer UUCP, curses and TCP
pyramid-hdb:
	@echo Making C-Kermit $(CKVER) for Pyramid Dual Port OSx
	ucb $(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD43 -DTCPSOCKET -DHBDUUCP -DCK_CURSES -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lcurses -ltermcap"

#Pyramid DC/OSx (UNIX System V R4).
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C.
#NOTE: Remove -O and Olimit:2500 from CFLAGS if TELNET connections do not work.
pyrdcosx:
	@echo 'Making C-Kermit $(CKVER) for Pyramid DC/OSx...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -Xa -O -DSVR4 -DDIRENT -DHDBUUCP -DSELECT -DNOGETUSERSHELL \
	-DCK_CURSES -DSTERMIOX -DTCPSOCKET -DPYRAMID -K Olimit:3100 \
	-DNO_DNS_SRV $(KFLAGS)" "LIBS= -lcurses -lsocket -lnsl" "LNKFLAGS = -s"

#IBM's AIX 3.0 on IBM 370 mainframe, tested on AIX F44 thru F50.
aix370:
	@echo Making C-Kermit $(CKVER) for IBM System/370 AIX 3.0...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIX370 -DTCPSOCKET -DLCKDIR -DDIRENT $(KFLAGS)" \
	"LIBS = -lbsd"

#IBM's AIX/ESA 2.1 (OSF/1) on IBM mainframe
aixesa:
	@echo Making C-Kermit $(CKVER) for IBM AIX/ESA...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXESA -DTCPSOCKET $(KFLAGS) -O"

#IBM PS/2 with AIX 1.0 thru 1.3.
#  Reports indicate that -O switch must be omitted.
#  It is also possible that "make bsd" will work (reports welcome).
#  One report said "make LIBS=-lbsd bsd" did the trick.
#  NOTLOG is to get around a 'tlog' symbol defined in one of the headers.
ps2aix:
	@echo 'Making C-Kermit $(CKVER) for IBM AIX 1.x PS/2...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DNOREALPATH -DPS2AIX10 -DSIG_V \
	-DNOUNICODE -DNOTLOG -DNOLEARN $(KFLAGS) -i" \
	"LNKFLAGS = -i"

ps2aixnetc:
	@echo 'Making C-Kermit $(CKVER) for IBM AIX 1.x PS/2...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DNOREALPATH -DPS2AIX10 -DTCPSOCKET -DCK_CURSES \
	-DSIG_V -DNOUNICODE -DNOTLOG -DNOLEARN $(KFLAGS) -i" \
	"LIBS = -lcurses" "LNKFLAGS = -i"

ps2aix3:
	$(MAKE) ps2aix KTARGET=$${KTARGET:-$(@)}

#IBM RT PC with AIX 2.2.1, valid as of C-Kermit 8.0.
#NOTLOG because of a conflict in <sys/termio.h>.
#This one has unique and strange lockfiles.
#  -O removed on purpose (8.0).
#  In case of "compiler error: symbol table full", increase the -Nn number.
#  In case of "compiler error: Constant pool too big", boost the -Np number.
#  Add -DNOPUTENV if putenv() causes trouble.
#  Put -DNOIKSD back if IKSD-related problems occur.
rtaix:
	@echo 'Making C-Kermit $(CKVER) for IBM RT PC, AIX 2.2.1...'
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS = -DATTSV -DRTAIX -DHDBUUCP -DDIRENT -DNOTLOG -DTCPSOCKET \
	-DNOGETUSERSHELL -DCLSOPN -DNOREALPATH -DNOUNICODE -DBSD_INCLUDES \
	-DUSE_LSTAT -DFNFLOAT -Nn2500 -Np1000 -Wq,-SJ2 -a -w $(KFLAGS)" \
	"LIBS = -lm $(LIBS)" "LNKFLAGS = -s"

#### IBM RT PC - these targets were last verified in C-Kermit 8.0.211.

#IBM RT PC with AIX 2.2.1 + curses
rtaixc:
	$(MAKE) rtaix CC=$(CC) CC2=$(CC2) "KFLAGS=-DCK_CURSES" "LIBS=-lcurses"

#IBM RT PC with AIX (ACIS) 2.2.1 (BSD 4.3)
# Add -O, -DDYNAMIC, -s, etc, if they work.
rtacis:
	@echo Making C-Kermit $(CKVER) for RT PC with ACIS 2.2.1 = BSD 4.3...
	$(MAKE) xermit KTARGET=$${KTARGET-$(@)} \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DNOREALPATH -DNOIKSD -DNOPUTENV \
	$(KFLAGS) -U__STDC__" "LNKFLAGS = -s"

#### IBM AIX.  The first two targets should work for any version of AIX
#### from 4.2 onwards.  The ones after that are for older versions or
#### specific configurations, and/or with gcc.

# This one should work for any AIX 4.2 or later: "make aix".
# Other tags are for compatibility with old makefile targets.
#OK: 2011/06/11
aix aix42 aix43 aix44 aix45 aix50 aix51 aix52 aix53 aix54 aix61:
	@echo Making C-Kermit $(CKVER) for IBM AIX...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DAIX41 -DAIX42 -DSVR4 -DSTERMIOX -DTCPSOCKET \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DFNFLOAT \
	-DSELECT -DSELECT_H -DNOGETUSERSHELL -qmaxmem=16000 -O \
	-DCKCPU=\\\"`uname -p`\\\" \
	-DHERALD=\"\\\" IBM AIX `uname -v`.`uname -r`\\\"\" \
	-D_LARGE_FILES $(KFLAGS)" "LNKFLAGS = -s" "LIBS=-lcurses -lm"

# Same but using gcc instead of cc
# This works but we get "gcc: unrecognized option '-qmaxmem-..'" each module.
aixg:
	@echo "Using gcc..."
	$(MAKE) aix KTARGET=$${KTARGET:-$(@)} \
	CC=gcc CC2=gcc "KFLAGS=-pipe -funsigned-char"

# AIX 4.2 or later with OpenSSL 0.9.7 or later: "make aix+ssl"
# For earlier OpenSSL remove -DOPENSSL_097 or add "KFLAGS=-UOPENSSL_097".
# Synonym target names added to cover old redundant targets that were removed.
# If SSL is not installed in the /usr/local tree (see SSLINC and SSLLIB
# definitions near the top), you can specify the locations in your make
# command as in this example:
#
#  SSLINC=-I/opt/ssl/include SSLLIB=-L/opt/ssl/lib make -e aix+ssl
#
# To build with gcc use "make aix CC=gcc CC2=gcc", or "make aixg"
#
#OK: 2011/06/15
aix+ssl aix51+openssl aix52+openssl aix53+openssl:
	@echo "Making C-Kermit $(CKVER) for IBM AIX with OpenSSL..."
	@echo "SSLINC=$(SSLINC) SSLLIB=$(SSLLIB)"
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	CC=$(CC) CC2=$(CC2) \
	"CFLAGS=-DAIXRS -DAIX41 -DAIX42 -DSVR4 -DSTERMIOX -DTCPSOCKET \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DFNFLOAT \
	-D_LARGE_FILES -DSELECT -DSELECT_H -DNOGETUSERSHELL \
	-DCKCPU=\\\"`uname -p`\\\" \
	-DHERALD=\"\\\" IBM AIX `uname -v`.`uname -r`\\\"\" \
	-DCK_AUTHENTICATION -DCK_SSL -DOPENSSL_097 $(SSLINC) $(KFLAGS)" \
	"LNKFLAGS=-s" "LIBS=$(SSLLIB) -lssl -lcrypto -lcurses -lm -lcrypt"

# AIX 5.3 or 6.1 or later with IBM OpenSSL, which is always in the directories
# shown below so you don't have to set SSLINC and SSLLIB.  If for some reason
# the SSL include files and libraries are not in the places assumed, then use
# "make aix+ssl" (just above) and set SSLINC and SSLLIB to indicate where the
# SSL files are.  To build with gcc use "make aix+ibmssl CC=gcc CC2=gcc".
aix+ibmssl:
	@echo "Making C-Kermit $(CKVER) for IBM AIX 6.1 with OpenSSL..."
	@echo "If this fails use 'make aix+ss' and specify SSLINC and SSLLIB"
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	CC=$(CC) CC2=$(CC2) \
	"CFLAGS=-DAIXRS -DAIX41 -DAIX42 -DSVR4 -DSTERMIOX -DTCPSOCKET \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DFNFLOAT \
	-D_LARGE_FILES -DSELECT -DSELECT_H -DNOGETUSERSHELL \
	-DCKCPU=\\\"`uname -p`\\\" \
	-DHERALD=\"\\\" IBM AIX `uname -v`.`uname -r`\\\"\" \
	-DCK_AUTHENTICATION -DCK_SSL -DOPENSSL_098 \
	-I/usr/include/openssl $(KFLAGS)" \
	"LNKFLAGS=-s" \
	"LIBS=-L/usr/lib/openssl -lssl -lcrypto -lcurses -lm -lcrypt"

# Old AIX versions...

#IBM AIX 3.0, 3.1, or 3.2 for RISC System/6000.
rs6000:
	@echo Making C-Kermit $(CKVER) for IBM AIX 3.0 or 3.1, RS/6000...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DTCPSOCKET -DSVR3 -DDIRENT -DCK_ANSIC \
	-DCK_POLL -DCLSOPN -DSELECT_H -DNOTTYLOCK -O $(KFLAGS)" \
	"LNKFLAGS = -s"

#IBM AIX 3.0, 3.1, or 3.2 for RISC System/6000, with curses.
rs6000c:
	@echo Making C-Kermit $(CKVER) for IBM AIX 3.0 or 3.1, RS/6000...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DTCPSOCKET -DSVR3 -DDIRENT -DCK_ANSIC \
	-DCK_POLL -DCLSOPN -DCK_CURSES -DSELECT_H -DNOTTYLOCK -DNOREALPATH \
	-O $(KFLAGS)" "LIBS= -lcurses -ltermcap" "LNKFLAGS = -s"

aix30:
	$(MAKE) rs6000 CC=$(CC) CC2=$(CC2) KTARGET=$${KTARGET:-$(@)}

aix31:
	$(MAKE) rs6000 CC=$(CC) CC2=$(CC2) KTARGET=$${KTARGET:-$(@)}

#IBM AIX 3.2 for RISC System/6000.
#In case of "subprogram too complex" warnings, add "-qmaxmem=16000" to CFLAGS.
rs6aix32:
	@echo Making C-Kermit $(CKVER) for IBM AIX 3.2, RS/6000...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DTCPSOCKET -DSVR4 -DDIRENT -DCK_ANSIC -DNOREALPATH \
	-DSELECT_H -DCLSOPN -DNOTTYLOCK -O $(KFLAGS)" "LNKFLAGS = -s"

#IBM AIX 3.2 for RISC System/6000.
rs6aix32c:
	@echo Making C-Kermit $(CKVER) for IBM AIX 3.2, RS/6000, TCP+curses...
	@echo In case of Subprogram Too Complex warnings,
	@echo add -qmaxmem=16000 to CFLAGS.
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DTCPSOCKET -DSVR4 -DDIRENT -DCK_ANSIC -DNOREALPATH \
	-DCLSOPN -DCK_CURSES -DSELECT_H -DNOTTYLOCK -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS=-lcurses"

aix32:
	$(MAKE) rs6aix32c KTARGET=$${KTARGET:-$(@)}

#IBM AIX 4.1, 4.1.x on RISC System/6000 or Power Series.
#Generates common binary for all platforms if using xlc (IBM C compiler).
#When using gcc, add -mcpu=common to generate common binary.
#Note that this one needs CK_NEWTERM.
# Add -bbigtoc in case ld fails with TOC overflow.
aix41:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.1.1 RS/6000 or PowerPC...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DAIX41 -DSVR4 -DSTERMIOX -DTCPSOCKET -DDIRENT \
	-DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DNOGETUSERSHELL -qmaxmem=16000 -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS=-lcurses"

#Ditto but with gcc.
#Remove "CC=gcc CC2=gcc" if you have gcc installed as cc.
aix41g:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.1.1 RS/6000 or PowerPC...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC=gcc" "CC2=gcc" \
	"CFLAGS= -DAIXRS -DAIX41 -DSVR4 -DSTERMIOX -DTCPSOCKET -DDIRENT \
	-DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DNOGETUSERSHELL -O $(KFLAGS)" \
	"LNKFLAGS = -s -Xlinker -bbigtoc" "LIBS=-lcurses"

# Add -bbigtoc in case ld fails with TOC overflow.
aix41+krb5+krb4:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.1.1 RS/6000 or PowerPC...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DAIX41 -DSVR4 -DSTERMIOX -DTCPSOCKET -DDIRENT \
	-DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_DES $(K5INC) $(K5INC)/krb5  \
	-DNOGETUSERSHELL -qmaxmem=16000 -O $(KFLAGS)" \
	"LNKFLAGS = -s" \
	"LIBS = $(K5LIB) -lcurses -lkrb4 -ldes425 -lkrb5 \
	-lcom_err -lk5crypto -lgssapi_krb5"

#Old name for "aix41".
rs6aix41c:
	$(MAKE) aix41 KTARGET=$${KTARGET:-$(@)}

#IBM AIX 4.1, 4.1.x, or 4.2 on RISC System/6000 or Power Series,
# with X.25 support
#Generates common binary for all platforms if using xlc (IBM C compiler).
#When using gcc, add -mcpu=common to generate common binary.
# Add -bbigtoc in case ld fails with TOC overflow.
aix41x25:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.1.1 RS/6000 or PowerPC...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DAIX41 -DSVR4 -DSTERMIOX -DTCPSOCKET -DDIRENT \
	-DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DIBMX25 -DDEBUG -DNOGETUSERSHELL -qmaxmem=16000 -g $(KFLAGS)" \
	"LNKFLAGS = -g -bI:/lib/pse.exp" "LIBS=-lcurses -lodm -lcfg"
	-@echo "]0;kermit done\c"

#As above but without -g in LNKFLAGS.
# Add -bbigtoc in case ld fails with TOC overflow.
aix41x25o:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.1.1 RS/6000 or PowerPC...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DAIX41 -DSVR4 -DSTERMIOX -DTCPSOCKET -DDIRENT \
	-DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DIBMX25 -DNODEBUG -DNOGETUSERSHELL -qmaxmem=16000 $(KFLAGS)" \
	"LNKFLAGS = -bI:/lib/pse.exp" "LIBS=-lcurses -lodm -lcfg"
	-@echo "]0;kermit done\c"

#AIX 4.2 -- Use this target if the regular "make aix" doesn't work.
# Must have CK_NEWTERM or echoing is lost after curses.
# Add -bbigtoc in case ld fails with TOC overflow.  As of C-Kermit 8.0.212,
# all AIX builds 4.2 and later include large file support.
oldaix42:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.2 or higher...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DAIX41 -DAIX42 -DSVR4 -DSTERMIOX -DTCPSOCKET \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DCK_NEWTERM -DFNFLOAT \
	-DSELECT -DSELECT_H -DNOGETUSERSHELL -qmaxmem=16000 -O \
	-DCKCPU=\\\"`uname -p`\\\" \
	-DHERALD=\\\"\ IBM\ AIX\ `uname -v`.`uname -r`\\\" \
	-D_LARGE_FILES $(KFLAGS)" "LNKFLAGS = -s" "LIBS=-lcurses -lm"

#AIX 4.3 - Use this target if the regular "make aix" doesn't work.
# Must NOT have CK_NEWTERM or else C-Kermit hangs after curses.
# -bbigtoc needed on some systems but not others to avoid TOC overflow.
# "man ld" says -bbigtoc makes program run slower.
oldaix43:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.3 or higher...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DAIX41 -DAIX43 -DSVR4 -DSTERMIOX -DTCPSOCKET \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DFNFLOAT -DNOGETUSERSHELL -qmaxmem=16000 -bbigtoc -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS=-lcurses -lm"

#AIX 4.3 with IBM X.25.
aix43x25:
	@echo "Making C-Kermit $(CKVER) for IBM AIX 4.3 with X.25..."
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DAIXRS -DAIX41 -DAIX43 -DSVR4 -DSTERMIOX -DTCPSOCKET \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DFNFLOAT -DNOGETUSERSHELL -DIBMX25 \
	-qmaxmem=16000 -bbigtoc -O $(KFLAGS)" \
	"LNKFLAGS = -bI:/lib/pse.exp" "LIBS=-lcurses -lodm -lcfg -lm"

#AIX 4.3 -- Must NOT have CK_NEWTERM or else C-Kermit hangs after curses.
# -mminimal-toc needed on some systems but not others to avoid TOC overflow.
# "man ld" says -bbigtoc makes program run slower.
aix43g:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.3 gcc...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS= -mminimal-toc -g -O -DAIXRS -DAIX41 -DAIX43 -DSVR4 \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DSTERMIOX -DTCPSOCKET -DFNFLOAT -DNOGETUSERSHELL $(KFLAGS)" \
	"LIBS=-lcurses -lm"

aix43gcc:
	$(MAKE) aix43g

# None of the following aix43gcc attempts work on a gcc-only AIX 4.3.3 box.
# It just plain can't find the math routines (fmod, pow, exp, sqrt, log10,...)
# Which is odd because nm /usr/lib/libC.a finds them...

#in case aix43gcc can't find its math library...
aix43gccx:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.3 gcc...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS= -mminimal-toc -g -O -DAIXRS -DAIX41 -DAIX43 -DSVR4 \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DSTERMIOX -DTCPSOCKET -DFNFLOAT -DNOGETUSERSHELL $(KFLAGS)" \
	"LIBS= -L/usr/local/lib/gcc-lib/powerpc-ibm-aix4.3.1.0/2.95.2 \
	-lcurses -bloadmap -bnoquiet"

#in case aix43gccx can't find its math library...
aix43gccy:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.3 gcc...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS= -mminimal-toc -g -O -DAIXRS -DAIX41 -DAIX43 -DSVR4 \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DSTERMIOX -DTCPSOCKET -DFNFLOAT -DNOGETUSERSHELL $(KFLAGS)" \
	"LIBS= -lcurses -bloadmap -bnoquiet"

#in case aix43gccx can't find its math library...
aix43gccz:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.3 gcc...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS= -mminimal-toc -g -O -DAIXRS -DAIX41 -DAIX43 -DSVR4 \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DSTERMIOX -DTCPSOCKET -DFNFLOAT -DNOGETUSERSHELL $(KFLAGS)" \
	"LIBS= -L. -lcurses -bloadmap -bnoquiet"

#AIX 4.3 with MIT Kerberos 5 and Kerberos 4 compatibility mode
# Must NOT have CK_NEWTERM or else C-Kermit hangs after curses.
# -mminimal-toc needed on some systems but not others to avoid TOC overflow.
# "man ld" says -bbigtoc makes program run slower.
aix43gcc+krb5+krb4:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.3 or higher w/Kerberos...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS= -mminimal-toc -g -O -DAIXRS -DAIX41 -DAIX43 -DSVR4 \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DSTERMIOX -DTCPSOCKET -DFNFLOAT -DNOGETUSERSHELL \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_DES -funsigned-char $(K5INC) $(K5INC)/krb5 \
	$(KFLAGS)" \
	"LIBS=$(K5LIB) -lcurses -lm -lkrb4 -ldes425 -lkrb5 \
	-lcom_err -lk5crypto -lcrypt -lgssapi_krb5"

#AIX 4.3 with MIT Kerberos 5, Kerberos 4 compatibility mode and OpenSSL
# Must NOT have CK_NEWTERM or else C-Kermit hangs after curses.
# -mminimal-toc needed on some systems but not others to avoid TOC overflow.
# "man ld" says -bbigtoc makes program run slower.
aix43gcc+krb5+krb4+openssl:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.3 or higher w/Kerberos...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS= -mminimal-toc -g -O -DAIXRS -DAIX41 -DAIX43 -DSVR4 \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DSTERMIOX -DTCPSOCKET -DFNFLOAT -DNOGETUSERSHELL \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_DES -DCK_CAST -DLIBDES -DCK_SSL \
	-funsigned-char $(K5INC) $(K5INC)/krb5 $(SSLINC) $(KFLAGS)" \
	"LIBS=$(K5LIB) $(SSLLIB) -lssl -lcrypto \
	-lcurses -lm -lkrb4 -ldes425 -lkrb5 -lcom_err -lk5crypto -lcrypt \
	-lgssapi_krb5"

aix43gcc+openssl:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.3 or higher w/OpenSSL...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS= -mminimal-toc -g -O -DAIXRS -DAIX41 -DAIX43 -DSVR4 \
	-DDIRENT -DCK_ANSIC -DCLSOPN -DCK_CURSES -DSELECT -DSELECT_H \
	-DSTERMIOX -DTCPSOCKET -DFNFLOAT -DNOGETUSERSHELL \
	-DCK_AUTHENTICATION -DCK_SSL -funsigned-char $(SSLINC) $(KFLAGS)" \
	"LIBS=$(SSLLIB) -lssl -lcrypto -lcurses -lm -lcrypt"

aix44gcc:
	$(MAKE) aix43g "KFLAGS=-DAIX44 $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)}

aix45gcc:
	$(MAKE) aix43g "KFLAGS=-DAIX45 $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)}

aix50gcc:
	$(MAKE) aix43g "KFLAGS=-DAIX50 $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)}

aix51gcc:
	$(MAKE) aix43g "KFLAGS=-DAIX51 $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)}

aix52gcc:
	$(MAKE) aix43g "KFLAGS=-DAIX52 $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)}

aix53gcc:
	$(MAKE) aix43g "KFLAGS=-DAIX53 $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)}

#Bull DPX/2 with BOS/X, like AIX/RS6000
bulldpx2:
	@echo Making C-Kermit $(CKVER) for Bull DPX/2 with BOS/X...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSVR3 -DDIRENT -DCK_ANSIC -DCKTYP_H=<sys/types.h> \
	-DCK_POLL -DNOGETUSERSHELL -DCLSOPN -DNOLEARN -O $(KFLAGS)" \
	"LNKFLAGS = -s"

#Sun UNIX 3.5 with gcc 2.3.3.
sunos3gcc:
	@echo Making C-Kermit $(CKVER) for Sun UNIX 3.5 and gcc...
	$(MAKE) xermit CC=gcc CC2=gcc KTARGET=$${KTARGET:-$(@)} \
	CFLAGS="-g -O -DBSD4 -DTCPSOCKET $(KFLAGS)"

#SunOS version 4.0, BSD environment, has saved original euid feature.
# Add "CC=/usr/ucb/cc CC2=/usr/ucb/cc" if necessary.
# Note: Including Unicode crashes the assembler in ckcuni.c.
sunos4:
	@echo Making C-Kermit $(CKVER) for SunOS 4.0, BSD environment...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DSUNOS4 -DFNFLOAT -DNOUNICODE $(KFLAGS)" \
	"LIBS=-lm"

#As above, but with SunLink X.25 support
sunos4x25:
	@echo SunLink X.25 support
	$(MAKE) "MAKE=$(MAKE)" sunos4 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DFNFLOAT -DSUNX25" \
	"LIBS=-lm"

#SUN OS version 4.1 - 4.1.3, BSD environment, has saved original euid feature.
#Uses Honey DanBer UUCP.  Requires presence of /usr/spool/locks directory.
# /var/spool/ should be a symbolic link to  /usr/spool/.
# ... or 'make xermit "CC= /usr/ucb/cc " \'
# Note: "xermit" means use the select() version of the CONNECT module.
# Note for C-Kermit 9.0: Reportedly 'you need to modify the sys/ioctl.h
# include file, i.e. comment out the "struct winsize" and "struct
# ttysize". Otherwise there will be a conflict with sys/ttycom.h (included by
# termios.h) which also declares these structs. But you need both includes.'
sunos41:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 / BSD...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNOUNICODE $(KFLAGS)" \
	"LIBS= $(LIBS) -lresolv -lm"

#As above, but compiled with gcc.  Gives 24-32K size reduction
#with gcc 2.1 or 2.2.2.  CAUTION: make sure "fixincludes" has been run on
#the include files, so gcc's are in sync with the regular Sun ones.
#This includes the curses library for fullscreen file transfer display.
#NDGPWNAM needed for GCC 2.5.6, not needed for 2.4.0, but it's uncertain
#whether it will do any harm for 2.4.0 compilation -- if so, remove it.
sunos41gcc:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 with gcc and curses...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	-funsigned-char $(KFLAGS)" "LIBS= -lcurses -ltermcap -lresolv -lm"

# As above, but without -funsigned-char so I can see the warnings that
# everybody else will get when they use ANSI compilers that don't have this
# option (gsc = gcc signed char).
sunos41gsc:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 with gcc and curses...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	$(KFLAGS)" "LIBS= -lcurses -ltermcap -lresolv -lm"

#As above but with ckucon.c rather than ckucns.c (for testing only)
sunos41gccfork:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 with gcc and curses...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	-DNOLEARN -funsigned-char $(KFLAGS)" \
	"LIBS= -lcurses -ltermcap -lresolv -lm"

#as above but configured for Kerberos IV
sunos41gcc+krb4:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1, gcc, curses, krb4...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	-DTCPSOCKET -DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB4 \
	-DCK_ENCRYPTION -DCK_DES -DCK_CAST -DBIGBUFOK -funsigned-char \
	$(K4INC) $(KFLAGS)" \
	"LIBS= $(K4LIB) -lcurses -ltermcap -lresolv -lm -lkrb -ldes"

#as above but configured for SSL/TLS
sunos41gcc+openssl:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1, gcc, curses, ssl...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	-DCK_AUTHENTICATION -funsigned-char \
	-DCK_SSL -DTCPSOCKET -DBIGBUFOK $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -lcurses -ltermcap -lresolv -lm -lssl -lcrypto"

#as above but configured for Kerberos IV and SSL/TLS
sunos41gcc+krb4+openssl:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1, gcc, curses, krb4...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB4 -DCK_ENCRYPTION -DCK_DES \
	-DCK_CAST -DCK_SSL -DLIBDES -DTCPSOCKET -DBIGBUFOK -funsigned-char \
	$(K4INC) $(SSLINC) $(KFLAGS)" \
	"LIBS= $(K4LIB) $(SSLLIB) \
	-lcurses -ltermcap -lresolv -lm -lkrb -lssl -lcrypto"

#as above but configured for Kerberos IV and ZLIB enabled SSL/TLS
sunos41gcc+krb4+openssl+zlib:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1, gcc, curses, krb4...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB4 -DCK_ENCRYPTION -DCK_DES \
	-DCK_CAST -DCK_SSL -DLIBDES -DTCPSOCKET -DBIGBUFOK -funsigned-char \
	-DZLIB $(K4INC) $(SSLINC) \
	$(KFLAGS)" \
	"LIBS= $(K4LIB) $(SSLLIB) \
	-lcurses -ltermcap -lresolv -lm -lkrb -lssl -lcrypto -lz"

#as above but configured for Kerberos IV and SRP and ZLIB enabled SSL/TLS
sunos41gcc+krb4+srp+openssl+zlib:
	@echo "C-Kermit $(CKVER) SunOS 4.1: gcc,curses,krb4,srp,ssl,zlib..."
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB4 -DCK_ENCRYPTION -DCK_DES \
	-DCK_CAST -DCK_SSL -DLIBDES -DTCPSOCKET -DBIGBUFOK -funsigned-char \
	-DZLIB -DCK_SRP $(K4INC) $(SRPINC) $(SSLINC) $(KFLAGS)" \
	"LIBS= $(K4LIB) $(SRPLIB) $(SSLLIB) \
	-lcurses -ltermcap -lresolv -lm -lkrb -lkrypto \
	-lsrp -lssl -lcrypto -lz"

#as above but configured for Kerberos IV and SRP and ZLIB enabled SSL/TLS
sunos41gcc+srp+openssl+zlib:
	@echo "C-Kermit $(CKVER) SunOS 4.1: gcc,curses,srp,ssl,zlib..."
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNDGPWNAM -DCK_CURSES -DFNFLOAT \
	-DCK_AUTHENTICATION -DCK_ENCRYPTION -DCK_DES \
	-DCK_CAST -DCK_SSL -DLIBDES -DTCPSOCKET -DBIGBUFOK -funsigned-char \
	-DZLIB -DCK_SRP $(SRPINC) $(SSLINC) \
	$(KFLAGS)" \
	"LIBS= $(SRPLIB) $(SSLLIB) \
	-lcurses -ltermcap -lresolv -lm -lkrypto -lsrp -lssl -lcrypto -lz "

#SUNOS 4.1 as sunos41 above, but also with curses support
sunos41c:
	@echo Curses support
	$(MAKE) "MAKE=$(MAKE)" sunos41 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DCK_CURSES -DFNFLOAT " \
	"LIBS= -lcurses -ltermcap"

#As SunOS 4.1.x, gcc, configured as Internet Kermit Server.
# . NOLOCAL removes capability to make connections
# . TNCODE allows server-side Telnet negotiation.
# . used to include -lpwent, why?
# . used to include -L/usr/local/lib -lm, why?
sunos41giks:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 with gcc for IKS...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc" "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DNDGPWNAM -DFNFLOAT \
	-DNOLOCAL -DTCPSOCKET -DTNCODE -DNOPUSH $(KFLAGS)" \
	"LIBS= -lm -lresolv"

#SUNOS 4.1 with SunLink X.25 support
sunos41x25:
	@echo SunLink X.25 support
	$(MAKE) "MAKE=$(MAKE)" wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNOUNICODE -DFNFLOAT -DSUNX25 \
	-DNOLEARN $(KFLAGS)" "LIBS= $(LIBS) -lresolv -lm"

#SUNOS 4.1 with SunLink X.25 support and curses
sunos41x25c:
	@echo SunLink X.25 support + curses
	$(MAKE) "MAKE=$(MAKE)" wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DNOUNICODE -DFNFLOAT -DSUNX25 \
	-DCK_CURSES -DNOLEARN $(KFLAGS)" \
	"LIBS= $(LIBS) -lcurses -ltermcap -lresolv -lm"

#SUN with Solaris 2.0 = SunOS 5.0.
#Mostly the same as System V R4.  Don't use this with later Solaris versions.
solaris20:
	@echo 'Making C-Kermit $(CKVER) for Sun with Solaris 2.0 and curses...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DSOLARIS -DDIRENT -DHDBUUCP -DSTERMIOX \
	-DTCPSOCKET -DCK_CURSES -DFNFLOAT -DCK_POLL $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermlib -lm" "LNKFLAGS = -s"

#SUN with Solaris 2.0.
#As above, but built with the gcc compiler from the Cygnus CD-ROM.
solaris20g:
	@echo 'Making C-Kermit $(CKVER) for Sun Solaris 2.0, gcc, and curses..'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DSOLARIS -DDIRENT -DHDBUUCP -DSTERMIOX \
	-DTCPSOCKET -DCK_CURSES -DCK_POLL -DFNFLOAT $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermlib -lm" "LNKFLAGS = -s" \
	CC=/opt/cygnus-sol2-1.1/bin/gcc CC2=/opt/cygnus-sol2-1.1/bin/gcc

#SunOS 5.1 = Solaris 2.1.
#NOTE: A C compiler is no longer bundled with SunOS 5.1, so to compile C
#programs, you might have to change your PATH to include the directory
#/usr/ccs/bin AFTER the directory containing the compiler.  SunPRO C is
#installed by default in /opt/SUNWspro/bin.  So a sample PATH might be:
#
# /usr/local/bin:/usr/bin:/opt/SUNWspro/bin:/usr/ccs/bin:\
# /usr/ucb:/usr/sbin:/sbin:.
#
# or:
#
# /usr/openwin/bin:/export/home/SUNWspro/bin:/usr/ccs/bin:/usr/sbin:/usr/bin.
#
#NOTE 2: Compilation with the Apogee C compiler (apcc) might not work,
#because it refuses to allow "-Usun".  Reportedly, newer releases of apcc
#(such as 1.2.17) work OK, use: "make -e sunos51 CC=apcc CC2=apcc".
solaris21:
	@echo 'Making C-Kermit $(CKVER) for SunOS 5.x....'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -Usun -DSVR4 -DSOLARIS -DDIRENT -DHDBUUCP -DFNFLOAT \
	-DSELECT -DNODEBUG -DSTERMIOX $(KFLAGS)" "LIBS = -lm" "LNKFLAGS = -s"

#Solaris 2.0 - 2.4, SunPro compiler, includes curses and TCP/IP.
#When using SUNWspro CC 2.0.1 under Solaris 2.3, be sure all cc patches
#are applied, otherwise corrupt or truncated object files can result.
#To build, set your PATH as follows:
#  /usr/local/bin:/usr/bin:/opt/SUNWspro/bin:/usr/ccs/bin:\
#  /usr/ucb:/usr/sbin:/sbin:.
# or (depending on where the compiler has been installed):
#  /usr/openwin/bin:/export/home/SUNWspro/bin:/usr/ccs/bin:/usr/sbin:/usr/bin.
#For additional optimization try using "-fast -xO4 -xdepend".
solaris2x:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with SunPro cc...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -Usun -i -DSVR4 -DDIRENT -DSOLARIS -DHDBUUCP -DFNFLOAT \
	-DSELECT -DCK_CURSES -DCK_NEWTERM -DSTERMIOX -DTCPSOCKET $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS= -ltermlib -lsocket -lnsl -lm -lresolv"

#as above but configured for Kerberos IV
solaris2x+krb4:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x, SunPro cc, krb4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -Usun -i -DSVR4 -DDIRENT -DSOLARIS -DHDBUUCP -DFNFLOAT \
	-DSELECT -DCK_CURSES -DCK_NEWTERM -DSTERMIOX -DTCPSOCKET  \
	-DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB4 \
	-DCK_ENCRYPTION -DCK_DES -DCK_CAST $(K4INC) $(KFLAGS)" \
	"LNKFLAGS = -s" \
	"LIBS= $(K4LIB) -ltermlib -lsocket -lnsl -lm -lresolv -lkrb -ldes"

#C-Kermit for Solaris 2.0-2.4 compiled with gcc, includes curses and TCP/IP.
#Change -O2 to -O if -O2 gives trouble.
#Remove -Usun if it causes trouble.
#Your PATH should start with something like:
#  /usr/local/gnu/bin:/usr/ccs/bin:
#Produces a huge executable -- strip with /usr/ccs/bin/strip (not Gnu strip).
#Also don't add "LNKFLAGS = -s" -- strip manually instead.
#Also note: this can NOT be linked statically - Sun makes it impossible.
#And for Solaris 2.4, you might have to replace:
# /usr/local/lib/gcc-lib/i486-sun-solaris2/2.4.5/include/sys/stat.h
#with /usr/include/sys/stat.h.
solaris2xg:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with GNU cc...'
	@echo 'Please read the comments that accompany the solaris2xg target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -ltermlib -lsocket -lnsl -lm -lresolv $(LIBS)"

#ditto but no curses.
solaris2xgnc:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with GNU cc...'
	@echo 'Please read the comments that accompany the solaris2xg target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DFNFLOAT \
	-DDIRENT -DHDBUUCP -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lm -lresolv $(LIBS)"

#and with Kerberos IV
solaris2xg+krb4:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with GNU cc, krb4...'
	@echo 'Please read the comments that accompany the solaris2xg target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB4 -DCK_ENCRYPTION \
	-DCK_DES -DCK_CAST -DBIGBUFOK $(K4INC) $(KFLAGS)" \
	"LIBS= $(K4LIB) -ltermlib -lsocket -lnsl -lm -lresolv -lkrb -ldes \
	$(LIBS)"

#and with OpenSSL,ZLIB,PAM,SHADOW
solaris2xg+openssl+zlib+pam+shadow:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with gcc, OpenSSL...'
	@echo 'Please read the comments that accompany the solaris2xg target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-DCK_AUTHENTICATION -DCK_SSL -DCK_PAM -DCK_SHADOW  -DZLIB \
	-DBIGBUFOK $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -ltermlib \
	-lsocket -lnsl -lm -lresolv -lssl -lcrypto -lpam -lz"

#Ditto but with GCC 3.1 in which you have to specify 32-bit with -m32.
#In Solaris 9 (and maybe 8) you'll also need specifiy the Library path.
#Reportedly this can be done here, but only with:
# crle -l /usr/lib:/usr/local/ssl/lib
#prior to building.  Note: 64-bit not tested with SSL.
#For no-crypto 64-bit builds see the solaris9g64 target.
solaris2xg32+openssl+zlib+pam+shadow:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with gcc, OpenSSL...'
	@echo 'Please read the comments that accompany the solaris2xg target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC="gcc -m32" CC2="gcc -m32" \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-DCK_AUTHENTICATION -DCK_SSL -DCK_PAM -DCK_SHADOW  -DZLIB \
	-DBIGBUFOK $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -ltermlib \
	-lsocket -lnsl -lm -lresolv -lssl -lcrypto -lpam -lz"

#and with Krb5,Krb4,OpenSSL,SHADOW
solaris2xg+krb5+krb4+openssl+shadow:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with gcc,k5,k4,ssl...'
	@echo 'Please read the comments that accompany the solaris2xg target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_SSL -DCK_DES -DCK_CAST -DBIGBUFOK \
	$(K5INC) $(K5INC)/krb5 $(SSLINC) $(KFLAGS)" \
	"LIBS= $(K5LIB) $(SSLLIB) -ltermlib -lsocket -lnsl -lm -lresolv \
	-lkrb4 -lssl -lcrypto -lgssapi_krb5 -lkrb5 -lcom_err -lk5crypto \
	-ldes $(LIBS)"

#and with OpenSSL
solaris2xg+openssl+pam+shadow:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with gcc, OpenSSL...'
	@echo 'Please read the comments that accompany the solaris2xg target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-DCK_AUTHENTICATION -DCK_SSL -DCK_PAM -DCK_SHADOW \
	-DBIGBUFOK $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -ltermlib \
	-lsocket -lnsl -lm -lresolv -lssl -lcrypto -lpam"

solaris2xg+openssl+zlib+srp+pam+shadow:	
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with gcc, OpenSSL...'
	@echo 'Please read the comments that accompany the solaris2xg target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET -DBIGBUFOK \
	-DCK_AUTHENTICATION -DCK_ENCRYPTION -DCK_DES -DLIBDES -DCK_CAST \
	-DCK_SSL -DCK_PAM -DCK_SHADOW -DZLIB -DCK_SRP $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -ltermlib -lsocket -lnsl -lm -lresolv -lsrp -lssl \
	-ldes -lkrypto -lcrypto -lpam -lz"

solaris22g:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=-DPOSIX_CRTSCTS $(KFLAGS)" solaris2xg \
	KTARGET=$${KTARGET:-$(@)}

solaris23g:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=-DPOSIX_CRTSCTS $(KFLAGS)" solaris2xg \
	KTARGET=$${KTARGET:-$(@)}

#Solaris 2.4 built with gcc
solaris24g:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET:-$(@)} \
	solaris2xg "KFLAGS=-DSOLARIS24 -DPOSIX_CRTSCTS $(KFLAGS)"

#Solaris 2.0-2.3, SunPro compiler, with SunLink X.25 support.
#This will only run if user has /opt/SUNWconn/lib/libsockx25.so.1
#exists and can be dynamically linked.
#NOTE: Do not change target to xermit -- it doesn't support X.25.
solaris2x25:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x+X.25 with SunPro cc...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -i -Usun -DSVR4 -DSOLARIS -DDIRENT \
	-DSUNX25 -DTCPSOCKET -DHDBUUCP -DFNFLOAT -DNOLEARN \
	-DSELECT -DCK_CURSES -DCK_NEWTERM -DSTERMIOX $(KFLAGS)" \
	"LNKFLAGS = -s" \
	"LIBS= -ltermlib -L/opt/SUNWconn/lib -R/opt/SUNWconn/lib \
	-lsockx25 -lsocket -lnsl -lm -lresolv"

#Solaris 2.0-2.4, gcc, SunLink X.25 added.
#NOTE: Can't use xermit target with X.25.
solaris2xgx25:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x + X.25 with GNU cc...'
	@echo 'Please read the comments that accompany the solaris2xg entry.'
	$(MAKE) wermit CC=gcc CC2=gcc KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DSUNX25 \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET -DFNFLOAT \
	-DNOLEARN $(KFLAGS)" \
	"LIBS= -ltermlib -lm -L/opt/SUNWconn/lib -R/opt/SUNWconn/lib \
	-lsockx25 -lsocket -lnsl"

#Solaris 2.4, SunPro compiler, with SunLink X.25 support.
#This will only run if user has /opt/SUNWconn/lib/libsockx25.so.1
#exists and can be dynamically linked.
solaris24x25:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.4+X.25 with SunPro cc...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -i -Usun -DSVR4 -DSOLARIS -DSOLARIS24 -DDIRENT -DNOLEARN \
	-DSUNX25 -DTCPSOCKET -DHDBUUCP -DFNFLOAT -DPOSIX_CRTSCTS \
	-DSELECT -DCK_CURSES -DCK_NEWTERM -DSTERMIOX $(KFLAGS)" \
	"LNKFLAGS = -s" \
	"LIBS= -ltermlib -L/opt/SUNWconn/lib -R/opt/SUNWconn/lib \
	-lsockx25 -lsocket -lnsl -lm -lresolv"

#Solaris 2.5, SunPro compiler, with SunLink X.25 support.
solaris25x25:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.5+X.25 with SunPro cc...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -i -Usun -DSVR4 -DSOLARIS25 -DDIRENT -DSUNX25 \
	-DTCPSOCKET -DHDBUUCP -DSELECT -DCK_CURSES \
	-DCK_NEWTERM -DSTERMIOX -DFNFLOAT -DPOSIX_CRTSCTS -DNOLEARN \
	-I/opt/SUNWconn/include $(KFLAGS)" \
	"LIBS= -ltermlib -L/opt/SUNWconn/lib -R/opt/SUNWconn/lib \
	-lsockx25 -lsocket -lnsl -lm -lresolv"

solaris23:
	$(MAKE) "MAKE=$(MAKE)" solaris2x KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS)"

solaris24:
	$(MAKE) "MAKE=$(MAKE)" solaris2x KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS24 -DPOSIX_CRTSCTS $(KFLAGS)"

# template for Solaris 2.5 and above.
solaris25x:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with SunPro cc...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DFNFLOAT -O -Usun -i $(KFLAGS)" \
	"LNKFLAGS = -s" \
	"LIBS= -ltermlib -lsocket -lnsl -lm -lresolv $(LIBS)"

#Solaris 2.5, SunPro compiler, curses, TCP/IP
solaris25:
	$(MAKE) "MAKE=$(MAKE)" solaris25x KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS25 $(KFLAGS)"

#Solaris 2.5, SunPro compiler, curses, TCP/IP, Kerberos IV
solaris25+krb4:
	$(MAKE) "MAKE=$(MAKE)" solaris25x+krb4 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS25 $(KFLAGS)"

#Solaris 2.5 built with gcc
solaris25g:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-funsigned-char -DSOLARIS25 $(KFLAGS)"

#Solaris 2.5 built with gcc and Kerberos IV
solaris25g+krb4:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+krb4 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-funsigned-char -DSOLARIS25 $(KFLAGS)"

#Solaris 2.5 built with gcc and Kerberos V/IV, SSL, ...
solaris25g+krb5+krb4+openssl+shadow:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+krb5+krb4+openssl+shadow \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-funsigned-char -DSOLARIS25 $(KFLAGS)"

#Solaris 2.5, gcc, SunLink X.25 added.
solaris25gx25:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET:-$(@)} solaris2xgx25 \
	"KFLAGS=-DSOLARIS25 $(KFLAGS)"

#Solaris 2.6, gcc, SunLink X.25 added.
solaris26gx25:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET:-$(@)} solaris2xgx25 \
	"KFLAGS=-DSOLARIS26 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

#Solaris 2.6, SunPro compiler, curses, TCP/IP
solaris26:
	$(MAKE) "MAKE=$(MAKE)" solaris25x KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS26 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

#Solaris 2.6 with gcc
solaris26g:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET:-$(@)} solaris2xg \
	"KFLAGS= -DSOLARIS26 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS = -lpam"

#Solaris 2.6 with gcc and SSL
solaris26g+openssl:
	$(MAKE) "MAKE=$(MAKE)"  solaris2xg+openssl+pam+shadow \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS= -DSOLARIS26 $(KFLAGS)"

#Solaris 2.6 with gcc, no curses (e.g. because libtermlib is missing).
solaris26gnc:
	$(MAKE) "MAKE=$(MAKE)" KTARGET=$${KTARGET:-$(@)} solaris2xgnc \
	"KFLAGS= -DSOLARIS26 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

#Solaris 2.6, SunPro compiler, with SunLink X.25 support.
solaris26x25:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.6+X.25 with SunPro cc...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -i -Usun -DSVR4 -DSOLARIS26 -DDIRENT -DSUNX25 \
	-DTCPSOCKET -DHDBUUCP -DSELECT -DCK_CURSES -DCK_PAM -DCK_SHADOW \
	-DCK_NEWTERM -DSTERMIOX -DFNFLOAT -DPOSIX_CRTSCTS -DNOLEARN \
	-I/opt/SUNWconn/include $(KFLAGS)" \
	"LIBS= -ltermlib -L/opt/SUNWconn/lib -R/opt/SUNWconn/lib \
	-lsockx25 -lsocket -lnsl -lm -lresolv -lpam"

#Solaris 7 (2.7) with Sun CC
solaris7:
	$(MAKE) "MAKE=$(MAKE)" solaris25x KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS7 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

#Solaris 7 with gcc (32-bit)
solaris7g:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS7 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

#Solaris 7 with gcc + Kerberos IV (32-bit)
solaris7g+krb4:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+krb4 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS7 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

solaris7g+openssl+zlib+pam+shadow:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+openssl+zlib+pam+shadow \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS7 -DCK_PAM -DCK_SHADOW $(KFLAGS)"

#Solaris 7 with gcc + OpenSSL (32-bit)
solaris7g+openssl+zlib+srp+pam+shadow:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+openssl+zlib+srp+pam+shadow \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS7 -DCK_PAM -DCK_SHADOW $(KFLAGS)"

#Solaris 8
solaris8:
	$(MAKE) "MAKE=$(MAKE)" solaris25x KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS8 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

#Solaris 8 with gcc (32-bit)
solaris8g:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS8 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

# In OpenSSL builds add -ldl if you get unresolved references for
# dlclose, dlsym, dlopen, dlerror.

#Solaris 8 with gcc + OpenSSL (32-bit)
solaris8g+openssl+zlib+pam+shadow:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+openssl+zlib+pam+shadow \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS=-DSOLARIS8 $(KFLAGS)"

#Solaris 8 with gcc + Kerberos IV (32-bit)
solaris8g+krb4:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+krb4 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS8 -DCK_PAM -DCK_SHADOW $(KFLAGS)" \
	"LIBS= -lpam"

solaris9nolfs:
	$(MAKE) "MAKE=$(MAKE)" solaris25x KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS9 -DCK_PAM -DCK_SHADOW -DUSE_STRERROR $(KFLAGS)" \
	"LIBS= -lpam"

#Solaris 9 with malloc debugging
solaris9md:
	$(MAKE) mermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DFNFLOAT -O -Usun -i -D_FILE_OFFSET_BITS=64 \
	-DSOLARIS9 -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG \
	-DCK_PAM -DCK_SHADOW -DUSE_STRERROR $(KFLAGS)" \
	"LIBS= -lpam -ltermlib -lsocket -lnsl -lm -lresolv"

#Solaris 9 with gcc + OpenSSL + Shadow (32-bit)
#Add -DOPENSSL_097 for OpenSSL 0.9.7 or later.
solaris9g+openssl+shadow+pam+zlib:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+openssl+zlib+pam+shadow \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS9 -DHDBUUCP -DDIRENT -D_FILE_OFFSET_BITS=64 \
	-DNO_DCL_INET_ATON -DZLIB -DCK_PAM -DCK_SHADOW -DLIBDES $(KFLAGS)" \
	"LIBS= -lpam -ldes425 -lz $(LIBS)"

#Solaris 9 with gcc + OpenSSL + Kerberos 5 + Krb4 + Shadow (32-bit)
#Add -DOPENSSL_097 for OpenSSL 0.9.7 or later.
solaris9g+krb5+krb4+openssl+shadow+pam+zlib:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg+krb5+krb4+openssl+shadow \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS9 -DHDBUUCP -DDIRENT -D_FILE_OFFSET_BITS=64 \
	-DNO_DCL_INET_ATON -DZLIB -DCK_PAM -DCK_SHADOW -DLIBDES $(KFLAGS)" \
	"LIBS= -lpam -ldes -lz $(LIBS)"

#Solaris 9 with gcc + Kerberos 4 and 5:
solaris9g+krb5+krb4:
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -O -Usun -DSVR4 -DSOLARIS9 -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 -DKRB4 -DKRB524 \
	-D_FILE_OFFSET_BITS=64 \
	-DCK_ENCRYPTION -DCK_DES -DCK_CAST -DBIGBUFOK \
	$(K5INC) $(K5INC)/krb5 $(KFLAGS)" \
	"LIBS= $(K5LIB) -ltermlib -lsocket -lnsl -lm -lresolv \
	-lkrb4 -lcrypto -lgssapi_krb5 -lkrb5 -lcom_err -lk5crypto \
	-ldes $(LIBS)"

#Solaris 9, 10, or 11 with gcc...  
#Uses streams PTYs rather than BSD ptys as in C-Kermit 8.0 and earlier.
#This target is chained to be the secure solaris9g+xxx targets below.
solaris9g solaris10g solaris11g:
	@echo 'Making C-Kermit $(CKVER) for Solaris 9 or later with gcc'
	@case `uname -r` in \
	  5.9) SOLARISVERSION="-DSOLARIS9" ;; \
	  5.10) SOLARISVERSION="-DSOLARIS10" ;; \
	  5.11) SOLARISVERSION="-DSOLARIS11" ;; \
	  *) SOLARISVERSION="-DSOLARIS" ;; \
	esac ; \
	$(MAKE) "MAKE=$(MAKE)" CC="gcc -m32" CC2="gcc -m32" xermit \
	KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -g -O -Usun -DSVR4 $$SOLARISVERSION -DUSE_STRERROR \
	-DSTERMIOX -DSELECT -DFNFLOAT -DCK_PAM -DCK_SHADOW -funsigned-char \
	-DHAVE_STREAMS -DHAVE_GRANTPT -DHAVE_PTSNAME -DPUSH_PTEM \
	-DPUSH_LDTERM -DPUSH_TTCOMPAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-D_FILE_OFFSET_BITS=64 $(KFLAGS)" \
	"LIBS= -ltermlib -lsocket -lnsl -lm -lresolv -lpam $(LIBS)"

#Solaris 9, 10, or 11 with gcc + Kerberos 5 + OpenSSL.
#OK C-Kermit 9.0.301.
solaris9g+krb5+ssl solaris10g+krb5+ssl solaris11g+krb5+ssl:
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac ; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes425'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
              echo "HAVE DES"; \
           else echo "NO DES"; \
	fi; \
	GSSAPILIB=''; \
	K5DIR=`echo $(K5LIB) | sed 's|-L||'`; \
	echo K5DIR=$$K5DIR; \
	if ls $$K5DIR/libgssapi_krb5* > /dev/null 2> /dev/null; then \
              GSSAPILIB='-lgssapi_krb5'; \
          else GSSAPILIB='-lgssapi'; \
        fi; \
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -O -Usun -DSVR4 -DSOLARIS9 -DSTERMIOX -DSELECT -DFNFLOAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET  -DBIGBUFOK \
	-DCK_AUTHENTICATION -DCK_SSL -DZLIB -DCK_KERBEROS -DKRB5 \
	-DCK_ENCRYPTION -DCK_CAST $$OPENSSLOPTION \
	$$HAVE_DES $(SSLINC) $(K5INC) $(K5INC)/krb5 $(KFLAGS)" \
	"LIBS= $(SSLLIB) $(K5LIB) -lz -lssl -ltermlib -lsocket -lnsl -lm \
	-lresolv -lcrypto \
	$$GSSAPILIB -lkrb5 -lcom_err -lk5crypto $$DES_LIB $(LIBS)"

#Solaris 9, 10, or 11 with gcc, 64 bit build.
#Peeking inside FILE struct not allowed in 64-bit world.
#DON'T USE THIS ONE ON PC ARCHITECTURE - It compiles and links but won't run.
#OK: 2009/09/25 (but not tested on Solaris 11)
solaris9g64 solaris10g64 solaris11g64:
	@echo 'Making C-Kermit $(CKVER) for Solaris 9++ with gcc 64-bit'
	@case `uname -r` in \
	  5.9) SOLARISVERSION="-DSOLARIS9" ;; \
	  5.10) SOLARISVERSION="-DSOLARIS10" ;; \
	  5.11) SOLARISVERSION="-DSOLARIS11" ;; \
	  *) SOLARISVERSION="-DSOLARIS" ;; \
	esac ; \
	$(MAKE) "MAKE=$(MAKE)" CC="gcc -m64" CC2="gcc -m64" xermit \
	KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -g -O -Usun -funsigned-char \
	-DSVR4 $$SOLARISVERSION -DNOARROWKEYS \
	-DSTERMIOX -DSELECT -DFNFLOAT -DUSE_STRERROR -DCK_PAM -DCK_SHADOW \
	-DHAVE_STREAMS -DHAVE_GRANTPT -DHAVE_PTSNAME -DPUSH_PTEM \
	-DPUSH_LDTERM -DPUSH_TTCOMPAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -ltermlib -lsocket -lnsl -lm -lresolv -lpam $(LIBS)"

#Solaris 9, 10, or 11 with SunPro CC
#Uses streams PTYs rather than BSD ptys as in C-Kermit 8.0 and earlier.
#This target is chained to by the secure targets below.
#OK C-Kermit 9.0
solaris9 solaris10 solaris11:
	@echo 'Making C-Kermit $(CKVER) for Solaris 9 or later with Sun CC'
	@case `uname -r` in \
	  5.9) SOLARISVERSION="-DSOLARIS9" ;; \
	  5.10) SOLARISVERSION="-DSOLARIS10" ;; \
	  5.11) SOLARISVERSION="-DSOLARIS11" ;; \
	  *) SOLARISVERSION="-DSOLARIS" ;; \
	esac ; \
	$(MAKE) "MAKE=$(MAKE)" xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -Usun -DSVR4 $$SOLARISVERSION -DUSE_STRERROR \
	-DSTERMIOX -DSELECT -DFNFLOAT -DCK_PAM -DCK_SHADOW \
	-DHAVE_STREAMS -DHAVE_GRANTPT -DHAVE_PTSNAME -DPUSH_PTEM \
	-DPUSH_LDTERM -DPUSH_TTCOMPAT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-D_FILE_OFFSET_BITS=64 $(KFLAGS)" \
	"LIBS= $(LIBS) -ltermlib -lsocket -lnsl -lm -lresolv -lpam"

# Solaris 9, 10, or 11 with OpenSSL built with Sun CC.
# Here's an example of how to invoke this target in case your OpenSSL
# headers and libraries are not in /usr/local:
#
# make solaris9+openssl "SSLINC=" "SSLLIB=" \
#  "KFLAGS= -I/opt/openssl-0.9.8k/include -L/opt/openssl-0.9.8k/lib"
#
# Don't use 'make -e' because that inhibits passing of KFLAGS to
# the base (solaris9) target.
#
#OK C-Kermit 9.0
solaris9+ssl solaris10+ssl solaris11+ssl \
solaris9+openssl solaris10+openssl solaris11+openssl:
	@echo 'Making C-Kermit $(CKVER) for Solaris 9/10/11 with OpenSSL: cc'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac ; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes425'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
              echo "HAVE DES"; \
           else echo "NO DES"; \
	fi; \
	$(MAKE) "MAKE=$(MAKE)" solaris9 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DCK_AUTHENTICATION -DCK_SSL -DZLIB $$HAVE_DES \
	-DNO_DCL_INET_ATON $$OPENSSLOPTION $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -lz -lssl $$DES_LIB -lcrypto  $(LIBS)"

# Solaris 9 or later with OpenSSL, built with gcc.
# Remove -DNO_DCL_INET_ATON if inet_aton comes up missing.  This target nicely
# chains to the solaris{9,10,11}g target but for some reason it doesn't work if
# you add the -DFORWARD_X option, thus the solaris9g+openssl+forward_x target.
#
#OK: 2011/06/14
solaris9g+ssl solaris10g+ssl solaris11g+ssl \
solaris9g+openssl solaris10g+openssl solaris11g+openssl:
	@echo 'Making C-Kermit $(CKVER) for Solaris 9/10/11 with OpenSSL: gcc'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac ; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes425'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
              echo "HAVE DES"; \
           else echo "NO DES"; \
	fi; \
	$(MAKE) "MAKE=$(MAKE)" solaris9g KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DCK_AUTHENTICATION -DCK_SSL -DZLIB $$HAVE_DES \
	-DNO_DCL_INET_ATON $$OPENSSLOPTION $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -lz -lssl $$DES_LIB -lcrypto  $(LIBS)"

# Solaris 9 or later with gcc + OpenSSL + Shadow (32-bit).
# Remove -DNO_DCL_INET_ATON if inet_aton comes up missing.
# Includes long file support - not sure if this was available before Solaris 9.
# Detects Solaris version automatically.
#
solaris9g+openssl+forward_x:
	@echo 'Making C-Kermit $(CKVER) for Solaris 9 or later with OpenSSL...'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac ; case `uname -r` in \
	  5.9) SOLARISVERSION="-DSOLARIS9" ;; \
	  5.10) SOLARISVERSION="-DSOLARIS10" ;; \
	  5.11) SOLARISVERSION="-DSOLARIS11" ;; \
	  *) SOLARISVERSION="-DSOLARIS" ;; \
	esac ; \
	$(MAKE) xermit 	KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 $$SOLARISVERSION \
	-DHAVE_STREAMS -DHAVE_GRANTPT -DHAVE_PTSNAME -DPUSH_PTEM \
	-DPUSH_LDTERM -DPUSH_TTCOMPAT \
	-DSTERMIOX -DSELECT -DFNFLOAT -DBIGBUFOK -D_FILE_OFFSET_BITS=64 \
	-DCK_AUTHENTICATION -DCK_SSL -DCK_PAM -DCK_SHADOW -DZLIB -DLIBDES \
	-DFORWARD_X $$OPENSSLOPTION $(SSLINC) $(KFLAGS)" \
	"LIBS= $(SSLLIB) -lpam -ldes425 -lz -ltermlib \
	-lsocket -lnsl -lm -lresolv -lssl -lcrypto -lpam -lz $(LIBS)"

# These two should be folded in with the ones just above.

#Solaris 9 with gcc 3.1 + OpenSSL (32-bit)
solaris9g+openssl+zlib+pam+shadow:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg32+openssl+zlib+pam+shadow \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS9 -DUSE_STRERROR $(KFLAGS)"

#Solaris 10 with gcc 3.1 + OpenSSL (32-bit)
solaris10g+openssl+zlib+pam+shadow:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg32+openssl+zlib+pam+shadow \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSOLARIS10 -DUSE_STRERROR $(KFLAGS)"

#The following (old, old) sunosxxx entries are for debugging and testing only.

sunos41x:
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DSUNOS41 -DDIRENT -DNOTLOG -DNOMSEND \
	-DNOUUCP -DNOSIGWINCH -DNOREDIRECT -DNOPUSH -DNOCCTRAP \
	-DNOICP -DNOLOCAL $(KFLAGS)"

#SunOS 4.1.x, debugging with Pure Software, Inc., Purify 2 (commercial runtime
#error-detection software for catching wild array references, etc).
#Before running the resulting wermit, you'll also need to define and export
#the following environment variables (as in this example):
#PURIFYHOME=/usr/local/purify ; export PURIFYHOME
#PURIFYCACHEDIR=/tmp ; export PURIFYCACHEDIR
sunos41cp:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 / BSD / Curses / Purify...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CC2= purify -cache_dir=/usr/tmp cc" \
	"CFLAGS= -g -DSUNOS41 -DHDBUUCP -DDIRENT -DTCPSOCKET \
	-DSAVEDUID -DCK_CURSES $(KFLAGS)" \
	"LIBS= -lcurses -ltermcap"

#SunOS 4.1 with malloc debugger
sunos41md:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 malloc debug...
	$(MAKE) mermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DDIRENT -DTCPSOCKET \
	-DSAVEDUID $(KFLAGS) -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG"

sunos41gmd:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 with gcc and curses...
	$(MAKE) mermit KTARGET=$${KTARGET:-$(@)} "CC= gcc " "CC2= gcc" \
	"CFLAGS= -g -DSUNOS41 -DHDBUUCP -DDIRENT -DTCPSOCKET \
	-DNDGPWNAM -DSAVEDUID -DCK_CURSES -DRLOGCODE \
	$(KFLAGS) -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG" \
	"LIBS= -lcurses -ltermcap"

#SunOS version 4.1, gcc, profiling with gprof, no debugging.
#To get profile, "make sunos41p" (on Sun), then "./wermit".  After running
#wermit, "gprof ./wermit | lpr" (or whatever) to get execution profile.
sunos41p:
	@echo Making C-Kermit $(CKVER) for SunOS 4.x with profiling...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC= gcc " "CC2= gcc" \
	"CFLAGS= -DSUNOS41 -DNODEBUG -DSAVEDUID -DDIRENT -DTCPSOCKET \
	-DNDGPWNAM $(KFLAGS) -pg" "LNKFLAGS = -pg"

#SunOS version 4.1 or later, BSD environment, minimum features.
sunos41min:
	@echo Minimum interactive
	$(MAKE) "MAKE=$(MAKE)" sunos41 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOSPL -DNOXMIT -DNOMSEND -DNOFRILLS -DNORETRY \
	-DNODIAL -DNOHELP -DNODEBUG -DNOTLOG -DNOSCRIPT -DNOCSETS \
	-DNOSHOW -DNOSETKEY -DNOUUCP -DNORECALL -DNOREDIRECT \
	-DNOPUSH -DNOMDMHUP -DNOJC -DNOFDZERO -DNOESCSEQ \
	-DNONET -DCK_SMALL -DNOCKSPEED -DNOCKTIMERS -DNOLOGIN \
	-DNOCKXYZ -DNOKERBEROS -DNOMKDIR -DNOPATTERNS -DNOPERMS -DNOPIPESEND \
	-DNORECURSIVE -DNORENAME -DNORESEND -DNOSETKEY \
	-DNOTRIGGER -DNOTUNING $(KFLAGS)" "LNKFLAGS = -s"

#SunOS version 4.1, BSD environment, min size, command-line only...
sunos41m:
	@echo Minimum size
	$(MAKE) "MAKE=$(MAKE)" sunos41min KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOICP $(KFLAGS)"

#SunOS version 4.1, BSD environment, min size, cmd-line only, remote only...
#
sunos41mr:
	@echo Minimum size
	$(MAKE) "MAKE=$(MAKE)" sunos41min KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOICP -DNOLOCAL $(KFLAGS)"

#SunOS version 4.1, BSD environment, min size, interactive...
sunos41mi:
	@echo Minimum size
	$(MAKE) "MAKE=$(MAKE)" sunos41min KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOCMDL $(KFLAGS)"

#SunOS version 4.1, BSD environment, min size, interactive, remote only...
sunos41mir:
	@echo Minimum size
	$(MAKE) "MAKE=$(MAKE)" sunos41min KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOCMDL -DNOLOCAL $(KFLAGS)"

#SunOS 4.1, System V R3 environment (-i option omitted).
sunos41s5:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 System V R3...
	@echo For testing purposes only - NOT for production use.
	@echo For a useable version, make sunos41 instead.
	$(MAKE) wermit "CC= /usr/5bin/cc " "CC2=/usr/5bin/cc " \
	KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSUN4S5 -DDIRENT -DHDBUUCP -DNOLEARN -DCK_POLL $(KFLAGS) -O"

#As above, but with curses support
sunos41s5c:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 System V R3...
	@echo Curses included.
	@echo For testing purposes only - NOT for production use.
	@echo For a useable version, make sunos41 instead.
	$(MAKE) wermit "CC= /usr/5bin/cc " "CC2=/usr/5bin/cc " \
	KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSUN4S5 -DDIRENT -DHDBUUCP -DNOLEARN \
	-DCK_POLL -DCK_CURSES -DCK_NEWTERM $(KFLAGS) -O" "LIBS= -lcurses"

#As above, but with curses support AND net support
sunos41s5tcpc:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 System V R3...
	@echo TCP/IP and curses included.  No debug log.
	@echo For testing purposes only - NOT for production use.
	@echo For a useable version, make sunos41 instead.
	$(MAKE) xermit "CC= /usr/5bin/cc " "CC2=/usr/5bin/cc " \
	KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSUN4S5 -DDIRENT -DHDBUUCP -DCK_POLL \
	-DNODEBUG -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET $(KFLAGS) -O" \
	"LIBS= -lcurses -lresolv"

# (End of SunOS test entries...)

#Apollo with Domain SR10.0 or later, BSD environment
#Reportedly, it might also help to add '-A,systype=bsd4.3' to CFLAGS.
#Reportedly, there is also a problem with getc & putc macros that can
#be handled by using '#ifdef apollo' somewhere to redefine them???
#On the other hand, other reports indicate that it works fine as-is.
#NOTE: This entry was previously like this:
#	$(MAKE) wermit "CFLAGS= -DNOFILEH -DBSD4 $(KFLAGS) -Uaegis \
#	-DTCPSOCKET -U__STDC__"
#Reports (Dec 91) indicate SR10 has an ANSI-compliant C compiler,
#in addition to an older one that claimed to be ANSI-compliant but wasn't.
#The following make entry (plus checks that are made in ckcdeb.h) detect
#which compiler is used and define the CK_ANSIC or NOANSI flags accordingly.
sr10-bsd:
	@echo Making C-Kermit $(CKVER) for Apollo SR10.0 / BSD ...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DAPOLLOSR10 -DBSD43 -DTCPSOCKET -DCK_CURSES -DNOLEARN \
	-Uaegis $(KFLAGS)" "LIBS= -lcurses -ltermcap"

#Apollo with Domain SR10.0 or later, System V R3 environment.
#Don't use the optimizer (-O), it causes problems at runtime.
sr10-s5r3:
	@echo Making C-Kermit $(CKVER) for Apollo SR10.0 / Sys V R3 ...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DNOFILEH -DSVR3 -DAPOLLOSR10 -DNOLEARN $(KFLAGS) \
	-Uaegis -U__STDC__"

#Apollo Domain/IX (untested, try this if sr10-bsd doesn't work)
# -DTCPSOCKET can probably be added here.
apollobsd:
	@echo Making C-Kermit $(CKVER) for Apollo Domain/IX...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CC= /bin/cc " "CC2= /bin/cc " \
	"CFLAGS= -DNOFILEH -DBSD4 -DAPOLLOBSD -DNOLEARN $(KFLAGS) -Uaegis"

#Version 7 Unix (see comments near top of makefile)
v7:
	@echo Making C-Kermit $(CKVER) for UNIX Version 7.
	@echo Read the makefile if you have trouble with this...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS=-DV7 -DPROCNAME=\\\"$(PROC)\\\" \
	-DBOOTNAME=\\\"$(BOOTFILE)\\\" -DNPROCNAME=\\\"$(NPROC)\\\" \
	-DNPTYPE=$(NPTYPE) $(DIRECT) -DO_RDWR=2 -DO_NDELAY=0 -DO_SCCS_ID \
	-DNOLEARN $(KFLAGS)"

#AT&T UNIX System V R3, signal() is void rather than int.
#Uses dirent.h and Honey DanBer UUCP.
#Add the -i link option if necessary.
#If you get errors like "ws_row undefined" in ckutio.c, add -DNONAWS.
sys5r3:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DNOLEARN $(KFLAGS) -O" \
	"LNKFLAGS="

#As above, plus curses.
sys5r3c:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3 + curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DCK_CURSES -DNONAWS -DNOLEARN \
	$(KFLAGS) -O" "LNKFLAGS=" "LIBS = -ltermlib"

#System V R3.2 for PCs built on Interactive UNIX SV/386 R4.x
#but with all calls to dup2() disabled because generic SVR3 does not have dup2.
# (The -linet library might not need to be in this one.)
sys5r32is:
	@echo 'Making C-Kermit $(CKVER) for System V/386 R32
	$(MAKE) wermit CC="$(CC)" CC2="$(CC2)" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -O -DNOCSETS -DNOREALPATH \
	-DUID_T=ushort -DGID_T=ushort -DI386IX -DSVR3JC -DCK_CURSES -DNONAWS \
	-DPOSIX_JC -DCK_REDIR -DCK_POLL -DDCLGETCWD -DNOFDZERO -DNOREDIRECT \
	-DNOZEXEC -DNOLEARN $(KFLAGS)" "LIBS=-lcurses -lc_s -linet"

#System V R3.2 for PCs built on Interactive UNIX SV/386 R4.x
#but with all calls to dup2() disabled because generic SVR3 does not have dup2.
#With TCP/IP added.
sys5r32isnet:
	@echo 'Making C-Kermit $(CKVER) for System V/386 R32 + TCP/IP
	$(MAKE) wermit CC="$(CC)" CC2="$(CC2)" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -O -DNOCSETS -DNOREALPATH \
	-DUID_T=ushort -DGID_T=ushort -DI386IX -DSVR3JC -DCK_CURSES -DNONAWS \
	-DPOSIX_JC -DCK_REDIR -DCK_POLL -DDCLGETCWD -DNOFDZERO -DNOREDIRECT \
	-DNOLEARN -DNOZEXEC -DTCPSOCKET $(KFLAGS)" "LIBS=-lcurses -lc_s -linet"

iclsys5r3:
	make sys5r3 KTARGET=$${KTARGET:-$(@)} KFLAGS=-DICLSVR3

#AT&T UNIX System V R3.  As above, but no ANSI prototyping.
sys5r3na:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DNOANSI -DNOLEARN $(KFLAGS) -O" \
	"LNKFLAGS="

#AT&T UNIX System V R3, for 3B computers with Wollongong TCP/IP.
sys5r3net3b:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX SVR3/3B/Wollongong...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DWOLLONGONG -DNOLEARN $(KFLAGS) \
	-O" "LIBS= -lnet -lnsl_s" "LNKFLAGS ="

#AT&T UNIX System V R3, signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp, has <termiox.h>.
#Has <termiox.h> for RTS/CTS flow control.
sys5r3tx:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DTERMIOX -DNOLEARN \
	$(KFLAGS) -i -O" "LNKFLAGS ="

#AT&T UNIX System V R3, signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp, has <termiox.h>.
#Has <sys/termiox.h> for RTS/CTS flow control.
sys5r3sx:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DSTERMIOX -DNOLEARN \
	$(KFLAGS) -i -O" "LNKFLAGS ="

#AT&T UNIX System V R4.
#Has <termiox.h>.
sys5r4:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DTERMIOX -DNOLEARN $(KFLAGS)" \
	"LNKFLAGS = -s"

#AT&T UNIX System V R4 with Wollongong TCP/IP.
#Has <termiox.h>.
sys5r4net:
	@echo 'Making C-Kermit $(CKVER) for System V R4 + Wollongong TCP/IP...'
	@echo ' If sockets-library routines are missing at link time, then'
	@echo ' try the sys5r4net2 entry.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DNOLEARN \
	-DTERMIOX -DWOLLONGONG $(KFLAGS)" "LNKFLAGS = -s"

#As above, but needs libs included.
sys5r4net2:
	@echo ' PLEASE READ ckuins.txt IF YOU GET MISSING HEADER FILES.'
	@echo ' (Search for WOLLONGONG...)'
	$(MAKE) sys5r4net KTARGET=$${KTARGET:-$(@)} "LIBS= -lsocket -lnsl"

#As above plus curses.
sys5r4net2c:
	echo 'Making C-Kermit $(CKVER) for System V R4 + Wollongong TCP/IP...'
	@echo ' PLEASE READ ckuins.txt IF YOU GET MISSING HEADER FILES.'
	@echo ' (Search for WOLLONGONG...)'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DNOLEARN \
	-DTERMIOX -DWOLLONGONG -DCK_CURSES $(KFLAGS)" "LNKFLAGS = -s" \
	"LIBS= -lsocket -lnsl -lcurses"

#DELL UNIX System V R4.
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C constructs, advisory file locking on devices, etc.
#Warning: -DSTERMIOX enables hardware flow control (RTS/CTS), but reportedly
#this does not work with the normal drivers.  However, it might still work
#on non-Dell systems, or even Dell systems with different drivers installed.
dellsys5r4:
	@echo 'Making C-Kermit $(CKVER) for DELL UNIX System V R4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDELL_SVR4 -DDIRENT -DHDBUUCP \
	-DTCPSOCKET -DSTERMIOX -DCK_POLL $(KFLAGS)" \
	"LIBS= -lsocket -lnsl" "LNKFLAGS = -s"

#As above, curses support added...
dellsys5r4c:
	@echo 'Making C-Kermit $(CKVER) for DELL UNIX System V R4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDELL_SVR4 -DDIRENT -DHDBUUCP \
	-DTCPSOCKET -DSTERMIOX -DCK_CURSES -DCK_POLL \
	$(KFLAGS)" "LIBS= -lsocket -lnsl -lcurses -ltermcap" "LNKFLAGS = -s"

#Minimum interactive: As above, but with every conceivable option removed.
dellsys5r4mi:
	@echo 'Making C-Kermit $(CKVER) for DELL UNIX System V R4...'
	@echo 'Minimum-size interactive'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDELL_SVR4 -DDIRENT \
	-UTCPSOCKET -DNOCMDL -DNOSPL -DNOXMIT -DCK_POLL \
	-DNOMSEND -DNOFRILLS -DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOSHOW -DNOSETKEY -DNOSERVER -DNOUUCP \
	-DNOPUSH -DNOMDMHUP -DNOJC -DNOFDZERO -DNOESCSEQ  \
	$(KFLAGS)" "LNKFLAGS = -s"

#Command-line only version.
dellsys5r4m:
	@echo 'Making C-Kermit $(CKVER) for DELL UNIX System V R4...'
	@echo 'Command-line only'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDELL_SVR4 -DDIRENT \
	-UTCPSOCKET -DNOICP -DNOFRILLS -DNODIAL -DNODEBUG -DNOTLOG -DNOCSETS \
	-DNOSETKEY -DNOESCSEQ -DNOJC -DNOFDZERO -DCK_POLL \
	$(KFLAGS)" "LNKFLAGS = -s"

#AT&T UNIX System V R4.
#Has <sys/termiox.h>.
sys5r4sx:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DSTERMIOX -DNOLEARN \
	$(KFLAGS)" "LNKFLAGS = -s" "LIBS=$(LIBS)"

#AT&T UNIX System V R4.
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C constructs, <sys/termiox.h>, etc etc.
sys5r4sxtcp:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP \
	-DSTERMIOX -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl $(LIBS)" "LNKFLAGS= -s"

#AT&T UNIX System V R4.
#As above + curses.
sys5r4sxtcpc:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP \
	-DSTERMIOX  -DCK_CURSES -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap $(LIBS)" "LNKFLAGS = -s"

#AT&T UNIX System V R4.  CONSENSYS SVR4.2-1.
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C constructs, <sys/termiox.h>, etc.
# Fullscreen -DCK_CURSES added (with curses & termcap libs)
# Submission by Robert Weiner/Programming Plus, rweiner@watsun.cc.columbia.edu
sys5r4sxtcpf:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -L/usr/ccs/lib -lcurses -ltermcap" \
	"LIBS=$(LIBS)" "LNKFLAGS = -s"

#Smallest possible version for System V R4
s5r4m:
	@echo Minimum size
	$(MAKE) "MAKE=$(MAKE)" sys5r4sx KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOICP -DNOMSEND -UTCPSOCKET" "LNKFLAGS = -s"

#Smallest possible interactive version of above
s5r4mi:
	@echo Minimum interactive
	$(MAKE) "MAKE=$(MAKE)" sys5r4sx \
	"KFLAGS=-DNOSPL -DNOXMIT -DNOMSEND -DNOFRILLS -DNOSHOW \
	-DNODIAL -DNOHELP -DNODEBUG -DNOTLOG -DNOSCRIPT -DNOCSETS -DNOSETKEY \
	-UTCPSOCKET $(KFLAGS)" "LNKFLAGS = -s"

#AT&T UNIX System V R4, has <sys/termiox.h>
#ANSI C function prototyping disabled.
sys5r4sxna:
	@echo No ANSI C prototyping...
	$(MAKE) "MAKE=$(MAKE)" sys5r4sx KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DNOANSI"

#Stratus FTX.
ftx:
	@echo 'Making C-Kermit $(CKVER) for Stratus FTX 3.x...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DFTX -DDIRENT -DHDBUUCP -DSTERMIOX \
	-DNOGETUSERSHELL -DNOLEARN +DA1.1 $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS=$(LIBS)"

#Stratus FTX + TCP/IP.
ftxtcp:
	@echo 'Making C-Kermit $(CKVER) for Stratus FTX 3.x...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DFTX -DDIRENT -DHDBUUCP -DNOGETUSERSHELL \
	-DSTERMIOX -DTCPSOCKET -DNO_DNS_SRV +DA1.1 $(KFLAGS)" \
	"LIBS= -lsocket -lnsl $(LIBS)" "LNKFLAGS= -s"

#NCR MP-RAS 2.03 or 3.02
mpras:
	@echo 'Making C-Kermit $(CKVER) for NCR MP-RAS...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DNCRMPRAS -DDIRENT -DHDBUUCP -DSTERMIOX \
	-DNOGETUSERSHELL -DUSE_FILE__CNT -DNOLEARN -DNO_DNS_SRV $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS=$(LIBS)"

#NCR MP-RAS 2.03 or 3.02 with TCP/IP and curses
mprastcpc:
	@echo 'Making C-Kermit $(CKVER) for NCR MP-RAS + TCP/IP + curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CFLAGS=-DTCPSOCKET \
	-DCK_CURSES -DSVR4 -DNCRMPRAS -DDIRENT -DHDBUUCP -DSTERMIOX -DNOLEARN \
	-DNOGETUSERSHELL -DNO_DNS_SRV DUSE_FILE__CNT -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS= -lsocket -lnsl -lcurses -ltermcap $(LIBS)"

#SINIX-L V5.41 - includes curses, tcp/ip - Use this one for i386.
#This version of SINIX doesn't like fdopen() or popen().
sinix541:
	@echo 'Making C-Kermit $(CKVER) for Siemens/Nixdorf SINIX V5.41/i386'
	$(MAKE) ckcpro.$(EXT) "CFLAGS = -DSINIX -DSVR4 -DDIRENT -DHDBUUCP \
	-DSTERMIOX -DCK_CURSES -DTCPSOCKET -DSELECT -DCK_ANSIC -DNO_DNS_SRV \
	-DSNI541 -DNOGETUSERSHELL -DNONETCMD -DNOPOPEN -kansi -W0 $(KFLAGS)"
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSINIX -DSVR4 -DDIRENT -DHDBUUCP -DNO_DNS_SRV -DNOPOPEN \
	-DFNFLOAT -DSTERMIOX -DCK_CURSES -DTCPSOCKET -DSELECT -DCK_ANSIC \
	-DSNI541 -DNOGETUSERSHELL -DNONETCMD -kansi -W0 -O $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap -lm" "LNKFLAGS = -s"

sinix541i:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=$(KFLAGS)" sinix541

#SINIX V5.42 - includes curses, tcp/ip, everything - Use this one for MIPS.
# As of C-Kermit 7.1, optimization removed -- takes (literally) forever.
sinix542:
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSINIX -DSVR4 -DDIRENT -DHDBUUCP -DNO_DNS_SRV \
	-DFNFLOAT -DSTERMIOX -DCK_CURSES -DTCPSOCKET -DSELECT -DCK_ANSIC \
	-DSNI542 -DNOGETUSERSHELL -kansi -W0 $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap -lm" "LNKFLAGS = -s"

#SINIX V5.42 gcc - includes curses, tcp/ip, everything.
#This one was used to build the Pyramid-architecture RM600 version
#on SINIX-P 5.42 A10 with gcc but should work for SINIX 5.42 on any other
#architecture with gcc.
sinix542g:
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC=gcc" "CC2=gcc" \
	"CFLAGS = -DSINIX -DSVR4 -DDIRENT -DHDBUUCP -DNO_DNS_SRV \
	-DFNFLOAT -DSTERMIOX -DCK_CURSES -DTCPSOCKET -DSELECT -DCK_ANSIC \
	-DSNI542 -DNOGETUSERSHELL $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap -lm" \
	"LNKFLAGS = -s"

#SINIX V5.42 - includes curses, tcp/ip, everything - Use this one for Intel.
# (Note: SNI discontinued Intel support after 5.42.)
sinix542i:
	@echo 'Making C-Kermit $(CKVER) for Siemens/Nixdorf SINIX-Z V5.42...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSINIX -DSVR4 -DDIRENT -DHDBUUCP -DFNFLOAT -DSTERMIOX \
	-DCK_CURSES -DTCPSOCKET -DSELECT -DCK_ANSIC -DNO_DNS_SRV -kansi \
	-DSNI542 $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap -lm" \
	"LNKFLAGS = -s"

#Siemens Nixdorf Reliant UNIX V5.43 - includes curses, tcp/ip, everything:
# . gettimeofday() suddenly has only one arg instead of two (GTODONEARG).
# . The syntax of the Olimit specifier changed.
# . The name was changed from SINIX to Reliant UNIX in version 5.43C.
sni543:
	@echo 'Making C-Kermit $(CKVER) for Siemens/Nixdorf Reliant UNIX V5.43'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSINIX -DSNI543 -DSVR4 -DDIRENT -DHDBUUCP \
	-DSTERMIOX -DCK_CURSES -DTCPSOCKET -DSELECT -DCK_ANSIC -DGTODONEARG \
	-DNO_DNS_SRV -kansi -W0 -O -F Olimit,3100 $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap" "LNKFLAGS = -s"

#Siemens Nixdorf Reliant UNIX V5.44 - Like 5.43 but with different banner.
sni544:
	@echo 'Making C-Kermit $(CKVER) for Siemens/Nixdorf Reliant UNIX V5.44'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSINIX -DSNI544 -DSVR4 -DDIRENT -DHDBUUCP \
	-DSTERMIOX -DCK_CURSES -DTCPSOCKET -DSELECT -DCK_ANSIC -DGTODONEARG \
	-DNO_DNS_SRV -kansi -W0 -O -K Olimit,3100 $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap" "LNKFLAGS = -s"

#Commodore Amiga with AT&T UNIX System V R4 and TCP/IP support.
#Has <sys/termiox.h>.
svr4amiganet:
	@echo 'Making C-Kermit $(CKVER) for Amiga SVR4 + TCP/IP...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC=gcc" "CC2=gcc" \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DSTERMIOX \
	-DTCPSOCKET -DCK_CURSES $(KFLAGS)" "LNKFLAGS = -s" \
	"LIBS = -lsocket -lnsl -ltermlib"

#SCO (Novell (Univel)) UnixWare 1.x or 2.0, no TCP/IP.
#This assumes the Novell SDK 1.0, which has <sys/termiox.h>.
#UnixWare users with the "Prime Time Freeware" CD-ROM SDK will probably have
#to use the sys5r4 entry (no termiox.h file, so no hardware flow control).
#Change -DSELECT to -DCK_POLL if -DSELECT causes problems.
# NOTE: Unixware 1.x builds have not been tried in C-Kermit 7.0.
unixware:
	$(MAKE) "MAKE=$(MAKE)" sys5r4sx KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DOLD_UNIXWARE -DCK_NEWTERM -DSELECT -DNOGETUSERSHELL \
	-DNOSYSLOG $(KFLAGS)" "LIBS=-lcrypt"

#UnixWare 1.x or 2.0 with TCP/IP and curses.
#fork()-based CONNECT - no high serial speeds.
unixwarenetc:
	$(MAKE) "MAKE=$(MAKE)" sys5r4sxtcpc KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DOLD_UNIXWARE -DCK_NEWTERM -DSELECT -DNOGETUSERSHELL \
	-DNOSYSLOG $(KFLAGS)" "LIBS=-lcrypt -lresolv"

uw10:
	$(MAKE) unixwarenetc KTARGET=$${KTARGET:-$(@)} "KFLAGS=$(KFLAGS)"

#This is for Unixware 2.0.x only - use unixware21 for UW 2.1.x.
#Has special library search and enables special kludge around library
#foulup regarding vfork() (which Kermit doesn't use).  Forces POSIX-style
#hangup.
unixware20:
	@echo 'Making C-Kermit $(CKVER) for UnixWare 2.0.x...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DOLD_UNIXWARE -DUNIXWARE2 -DSELECT -DSVR4 -DDIRENT \
	-DHDBUUCP -DBIGBUFOK -DNOGETUSERSHELL -DSTERMIOX  -DCK_CURSES \
	-DTCPSOCKET -DUW200 -DFNFLOAT -DCK_NEWTERM -DNOSYSLOG $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap -lcrypt -lgen -lm -lresolv" \
	"LNKFLAGS = -s"

uw20:
	$(MAKE) unixware20 KTARGET=$${KTARGET:-$(@)} "KFLAGS=$(KFLAGS)"

#Adds big buffers ("large memory model") - otherwise the same as UnixWare 1.x.
unixware21:
	@echo 'Making C-Kermit $(CKVER) for UnixWare 2.1.x...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DUNIXWARE -DSELECT -DSVR4 -DDIRENT -DHDBUUCP -DBIGBUFOK \
	-DNOSYSLOG -DSTERMIOX  -DCK_CURSES -DTCPSOCKET \
	-DCK_NEWTERM -DFNFLOAT -DUNIXWARE2 $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap -lcrypt -lm -lresolv \
	$(LIBS)" "LNKFLAGS = -s"

#Unixware 2.1.0
uw21:
	$(MAKE) unixware21 KTARGET=$${KTARGET:-$(@)} "KFLAGS=$(KFLAGS)"

#Unixware 2.1.3
uw213:
	$(MAKE) unixware21 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DUSE_FILE__CNT $(KFLAGS)"

#Unixware 2.1 with IKSD support
uw21iksd:
	$(MAKE) unixware21 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DCK_SHADOW $(KFLAGS)" "LIBS= -lgen"

#UnixWare 7 with tc[gs]etspeed() high serial speeds & select()-based CONNECT
#and as of C-Kermit 8.0.212, large file support (LFS).
#NOTE: This is the one we use.
unixware7t:
	@echo 'Making C-Kermit $(CKVER) for UnixWare 7 with POSIX i/o...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DUNIXWARE -DSELECT -DSVR4 -DDIRENT -DHDBUUCP -DBIGBUFOK \
	-DFNFLOAT -DNOGETUSERSHELL -DSTERMIOX -DCK_CURSES -DTCPSOCKET -DPOSIX \
	-DUW7 -DUSETCSETSPEED -DCK_NEWTERM -DNOLSTAT -DDCLTIMEVAL \
	-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DNEEDMDMDEFS $(KFLAGS)" \
	"LIBS=-lsocket -lnsl -lcurses -ltermcap -lcrypt -lm -lresolv $(LIBS)" \
	"LNKFLAGS = -s"

#UnixWare 7 - select()-based CONNECT - no POSIX i/o - no high serial speeds.
#In other words, just like the UnixWare 1 and 2 builds.
unixware7x:
	@echo 'Making C-Kermit $(CKVER) for UnixWare 7...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DUNIXWARE -DSELECT -DSVR4 -DDIRENT -DHDBUUCP -DBIGBUFOK \
	-DUW7 -DNOGETUSERSHELL -DSTERMIOX -DCK_CURSES -DTCPSOCKET -DNOLSTAT \
	-DFNFLOAT -DCK_NEWTERM $(KFLAGS)" \
	"LIBS=-lsocket -lnsl -lcurses -ltermcap -lcrypt -lm -lresolv $(LIBS)" \
	"LNKFLAGS = -s"

#UnixWare 7 with POSIX cfset[oi]speed() to allow high serial speeds.
#(but the high speeds don't work)
unixware7p:
	@echo 'Making C-Kermit $(CKVER) for UnixWare 7 with POSIX i/o...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DUNIXWARE -DSELECT -DSVR4 -DDIRENT -DHDBUUCP -DBIGBUFOK \
	-DUW7 -DNOGETUSERSHELL -DSTERMIOX -DCK_CURSES -DTCPSOCKET -DPOSIX \
	-DFNFLOAT -DCK_NEWTERM -DNOLSTAT $(KFLAGS)" \
	"LIBS=-lsocket -lnsl -lcurses -ltermcap -lcrypt -lm -lresolv $(LIBS)" \
	"LNKFLAGS = -s"

# UnixWare 7 built with gcc - This does not work at all...
# Reportedly gcc 2.8.1 is broken on Unixware 7.  Try egcs?
unixware7g:
	@echo 'Making C-Kermit $(CKVER) for UnixWare 7 with gcc...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CC = gcc" "CC2 = gcc" "LNKFLAGS = -s -shlib"
	"CFLAGS = -O -DUNIXWARE -DSELECT -DSVR4 -DDIRENT -DHDBUUCP -DBIGBUFOK \
	-DUW7 -DNOGETUSERSHELL -DSTERMIOX  -DCK_CURSES -DTCPSOCKET -DNOLSTAT \
	-DFNFLOAT -DCK_NEWTERM $(KFLAGS)" \
	"LIBS=-lsocket -lnsl -lcurses -ltermcap -lcrypt -lm -lresolv $(LIBS)" \
	"LNKFLAGS = -s"

unixware7:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=$(KFLAGS)" unixware7t \
	KTARGET=$${KTARGET:-$(@)}

uw7:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=$(KFLAGS)" unixware7t \
	KTARGET=$${KTARGET:-$(@)}

#SCO OpenUNIX 8.0
ou8:
	@echo 'Making C-Kermit $(CKVER) for Open UNIX 8...'
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=-DOU8 $(KFLAGS)" unixware7t \
	KTARGET=$${KTARGET:-$(@)}

#UnixWare 7 with OpenSSL
uw7ssl uw7+ssl:
	@echo 'Making C-Kermit $(CKVER) for UnixWare 7 and OpenSSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DCK_AUTHENTICATION -DCK_SSL -DCK_SHADOW \
	-DUNIXWARE -DSELECT -DSVR4 -DDIRENT -DHDBUUCP -DBIGBUFOK \
	-DFNFLOAT -DNOGETUSERSHELL -DSTERMIOX -DCK_CURSES -DTCPSOCKET -DPOSIX \
	-DUW7 -DUSETCSETSPEED -DCK_NEWTERM -DNOLSTAT -DDCLTIMEVAL \
	$(SSLINC) $(KFLAGS)" \
	"LIBS=-lsocket -lnsl -lcurses -ltermcap -lcrypt -lm -lresolv \
	-lgen -lcudk70 $(SSLLIB) -lssl -lcrypto $(LIBS)" \
	"LNKFLAGS = -s"

#As above but includes Shadow password support needed for IKSD.
uw7iksd:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=-DCK_SHADOW $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)} "LIBS= -lgen" unixware7t

#As above but links with static API for realpath() so a binary built
#with this target on UW7.1 will also work on 7.0.  Requires SCO UDK
#rather than the stock compiler.
uw7iksdudk:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=-DCK_SHADOW $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)} "LIBS= -lgen -lcudk70" unixware7t

#ESIX SVR4.0.3 or 4.04 with TCP/IP support.
#Has <sys/termiox.h>, ANSI C function prototyping disabled.
#Add -m486 to CFLAGS if desired.
esixr4:
	@echo 'Making C-Kermit $(CKVER) for ESIX SVR4 + TCP/IP...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DNOANSI \
	-DSTERMIOX -DTCPSOCKET $(KFLAGS)" "LNKFLAGS = -s" \
	"LIBS = -lsocket -lnsl"

#AT&T UNIX System V R4.
#Has <sys/termiox.h>, Wollongong WIN/TCP TCP/IP.
sys5r4sxnet:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP \
	-DSTERMIOX -DWOLLONGONG $(KFLAGS)" "LNKFLAGS = -s"

#AT&T UNIX System V R4, no <termio.x> or <sys/termio.x>.
sys5r4nx:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DNOLEARN $(KFLAGS)" \
	"LNKFLAGS = -s"

#AT&T UNIX System V R4, no <termio.x> or <sys/termio.x>, curses, TCP/IP.
sys5r4nxnetc:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP \
	-DCK_CURSES -DTCPSOCKET $(KFLAGS)" \
	"LIBS = -lcurses -lsocket -lnsl -ltcpip" \
	"LNKFLAGS = -s"

#AT&T UNIX System V R4, no <termio.x> or <sys/termio.x>, Wollongong TCP/IP.
sys5r4nxtwg:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DWOLLONGONG $(KFLAGS)"
	"LNKFLAGS = -s"

#ICL UNIX System V R4.(DRS N/X) version :-
#UNIX System V Release 4.0 ICL DRS 6000 (SPARC)
#DRS/NX 6000 SVR4 Version 5  Level 1  Increment 4
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C constructs, advisory file locking on devices, etc.
#Remove -lnsl if it causes trouble.
iclsys5r4:
	@echo 'Making C-Kermit $(CKVER) for ICL UNIX System V R4 (DRS N/X)'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DICL_SVR4 -DDIRENT -DHDBUUCP -DNOGETUSERSHELL \
	-DSTERMIOX -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lresolv " "LNKFLAGS = -s"

#As above but for DRS/NX 4.2MP 7MPlus.
iclsys5r4m+:
	@echo 'Making C-Kermit $(CKVER) for ICL UNIX System V R4 DRS/NX 4.2MP+'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DICL_SVR4 -DDIRENT -DHDBUUCP -DNOIKSD \
	-DSTERMIOX -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lm -lc -g -lgen " "LNKFLAGS = -s"

#As above but for DRS/NX 4.2MP 7MPlus with IKSD support.
iclsys5r4m+iksd:
	@echo 'Making C-Kermit $(CKVER) for ICL UNIX System V R4 DRS/NX 4.2MP+'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DICL_SVR4 -DDIRENT -DHDBUUCP -DNOGETUSERSHELL \
	-DSTERMIOX -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lm -lc -g -lgen -lresolv " "LNKFLAGS = -s"

iclsys5r4_486:
	$(MAKE) "MAKE=$(MAKE)" iclsys5r4 KTARGET=$${KTARGET:-$(@)}

#Data General DG/UX 4.30 (System V R3) for DG AViiON, with TCP/IP support.
dgux430:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 4.30...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX430 -DSVR3 -DDIRENT -DTCPSOCKET \
	-DNOINADDRX -DNOGETUSERSHELL $(KFLAGS)"

#Data General DG/UX 4.30 for DG AViiON, with TCP/IP support with BSDisms.
dgux430bsd:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 4.30...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX430 -D_BSD_SOURCE -DBSD4 \
	-DNOINADDRX -DTCPSOCKET -DNOGETUSERSHELL $(KFLAGS)"

#Data General DG/UX 5.4 (System V R4) for DG AViiON, with TCP/IP support.
#Add -lsocket -lnsl if inet_addr comes up missing...
#Hmmm - I really think CK_POLL can be removed from this one in which case
#there is no difference between dgux540 and dgux540i.
dgux540:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.40...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX540 -DDIRENT -DHDBUUCP -DNOINADDRX \
	-DSTERMIOX -DTCPSOCKET -DCK_POLL -DNOGETUSERSHELL $(KFLAGS)"

#Data General DG/UX 5.40 (System V R4) for Intel AViiON, with TCP/IP support.
dgux540i:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.40...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX540 -DDIRENT -DHDBUUCP -DNOINADDRX \
	-DSTERMIOX -DTCPSOCKET -DNOGETUSERSHELL $(KFLAGS)" \
	"LIBS = -lsocket -lnsl"

dgux54:
	make dgux540 KTARGET=$${KTARGET:-$(@)}

#Data General DG/UX 5.4 (= System V R4) for DG AViiON, with TCP/IP support.
# And curses.
dgux540c:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.4...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX540 -DDIRENT -DHDBUUCP -DNOINADDRX \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES -DCK_NEWTERM -DNOGETUSERSHELL \
	$(KFLAGS)" "LIBS= -lcurses8 -ltermcap" "LNKFLAGS = -s"

#As above but for Intel - only difference is name library names.
dgux540ic:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.40...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX540 -DDIRENT -DHDBUUCP -DNOINADDRX \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES -DCK_NEWTERM -DNOGETUSERSHELL \
	$(KFLAGS)" "LIBS = -lsocket -lnsl -lcurses -ltermcap"

dgux54c:
	make dgux540c KTARGET=$${KTARGET:-$(@)}

#DG/UX 5.4R3.10
dgux54310:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.4R3...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DDGUX540 -DDGUX54310 -DDIRENT -DHDBUUCP -DSELECT \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES -DCK_NEWTERM	-DNOGETUSERSHELL \
	-DNOINADDRX $(KFLAGS)" "LIBS= -lcurses8 -ltermcap" "LNKFLAGS = -s"

#DG/UX 5.4R4.10 - Includes everything.
dgux54410:
	@echo 'Making C-Kermit $(CKVER) for DG/UX 5.4R4.10...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX540 -DDGUX54410 -DDIRENT -DHDBUUCP -DSELECT \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES -DCK_NEWTERM -DNOGETUSERSHELL \
	-DNOINADDRX $(KFLAGS)" "LIBS = -lsocket -lnsl -lcurses -ltermcap"

#DG/UX 5.4R4.11 - Includes everything.
dgux54411:
	@echo 'Making C-Kermit $(CKVER) for DG/UX 5.4R4.11...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX540 -DDGUX54411 -DDIRENT -DHDBUUCP -DSELECT \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES -DCK_NEWTERM -DNOGETUSERSHELL \
	-DNOINADDRX $(KFLAGS)" "LIBS = -lsocket -lnsl -lcurses -ltermcap"

#DG/UX 5.4R4.20 - Includes everything.
dgux54420:
	@echo 'Making C-Kermit $(CKVER) for DG/UX 5.4R4.20...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DDGUX540 -DDGUX54420 -DDIRENT -DHDBUUCP -DSELECT \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES -DCK_NEWTERM -DNOGETUSERSHELL \
	-DNOINADDRX $(KFLAGS)" \
	"LIBS = -lsocket -lresolv -lnsl -lcurses -ltermcap"

#Silicon Graphics System V R3 with BSD file system (IRIS)
iris:
	@echo Making C-Kermit $(CKVER) for Silicon Graphics IRIX pre-3.3...
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR3 -DLONGFN -DNOLEARN $(KFLAGS) -I/usr/include/bsd" \
	"LIBS = -lbsd"

#Silicon Graphics IRIS System V R3
irix33:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 3.3...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DNOLEARN $(KFLAGS) -O" \
	"LNKFLAGS = -s"

#Silicon Graphics Iris Indigo with IRIX 4.0.0 or 5.0...
#Strict ANSI C compilation, TCP/IP support included
irix40:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 4.0...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DIRIX40 -DSVR3 -DDIRENT -DHDBUUCP -DPWID_T=uid_t \
	-DCK_ANSIC -DTCPSOCKET $(KFLAGS) -O -Olimit 1600 -I/usr/include/bsd" \
	"LNKFLAGS = -s"

#As above, but with fullscreen display (curses) and Sun Yellow Pages support.
#NOTE: IRIX versions prior to 5 run COFF binaries.
irix40ypc:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 4.0.'
	@echo 'Includes fullscreen file display and Sun Yellow Pages...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DIRIX40 -DSVR3 -DDIRENT -DHDBUUCP -DCK_CURSES \
	-DPWID_T=uid_t -DCK_ANSIC -DTCPSOCKET $(KFLAGS) \
	-O -Olimit 1600 -I/usr/include/bsd" \
	"LIBS = -lcurses -lsun" "LNKFLAGS = -s"

# Silicon Graphics Iris Series 4D/*, IRIX 4.0.x, -O4 ucode optimized.
# Huge temporary file space needed for ucode optimizer.  If you get an error
# like "ugen: internal error writing to /tmp/ctmca08777: Error 0", define the
# the TMPDIR environment variable to point to a file system that has more
# space available, e.g. "setenv TMPDIR /usr/tmp".
irix40u:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 4.0...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DIRIX40 -DSVR3 -DDIRENT -DHDBUUCP -DPWID_T=uid_t \
	-DCK_ANSIC -DTCPSOCKET $(KFLAGS) -O4 -Olimit 1600" \
	"LNKFLAGS=-O4 -Olimit 1600 -s" "EXT=u"

# As above, with Curses Support added
irix40uc:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 4.0...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DIRIX40 -DSVR3 -DDIRENT -DHDBUUCP -DPWID_T=uid_t \
	-DCK_ANSIC -DCK_CURSES -DTCPSOCKET $(KFLAGS) -O4 -Olimit 1600" \
	"LNKFLAGS=-O4 -Olimit 1600 -s" "EXT=u" "LIBS= -lcurses -ltermcap"

#Silicon Graphics IRIX 5.x.
#Yellow Pages and Curses support included.
#IRIX version 5.x can run COFF or ELF binaries.
irix51:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 5.x'
	@echo 'Includes fullscreen file display and Yellow Pages...'
	@echo 'Add -mips<n> to CFLAGS specify a particular hardware target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DIRIX51 -DSVR4 -DDIRENT -DHDBUUCP -DCK_CURSES -DCK_NEWTERM \
	-DPWID_T=uid_t -DCK_ANSIC -DTCPSOCKET -DSELECT -DNOGETUSERSHELL \
	-DSYSTIMEH -DDCLPOPEN -DDCLFDOPEN $(KFLAGS) -ansi -O -Olimit 3000" \
	"LIBS = -lcurses" "LNKFLAGS = -s"

#Use this one if irix51 blows up due to lack of swap space or whatever.
irix51x:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 5.x'
	@echo 'Includes fullscreen file display and Yellow Pages...'
	@echo 'Add -mips<n> to CFLAGS specify a particular hardware target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DIRIX51 -DSVR4 -DDIRENT -DHDBUUCP -DCK_CURSES -DCK_NEWTERM \
	-DPWID_T=uid_t -DCK_ANSIC -DTCPSOCKET -DSELECT -DNOGETUSERSHELL \
	-DSYSTIMEH -DDCLPOPEN -DDCLFDOPEN $(KFLAGS)" \
	"LIBS = -lcurses" "LNKFLAGS = -s"

irix51ypc:
	$(MAKE) "MAKE=$(MAKE)" irix51 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS= $(KFLAGS)"

#IRIX 5.2 adds RTS/CTS
irix52:
	$(MAKE) "MAKE=$(MAKE)" irix51 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DIRIX52 -DCK_RTSCTS $(KFLAGS)"

irix53:
	$(MAKE) "MAKE=$(MAKE)" irix51 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DIRIX52 -DIRIX53 -DCK_RTSCTS $(KFLAGS)"

irix53x:
	$(MAKE) "MAKE=$(MAKE)" irix51x KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DIRIX52 -DIRIX53 -DCK_RTSCTS $(KFLAGS)"

#Silicon Graphics IRIX 6.[024] common stuff.
#Yellow Pages and Curses support included.
#IRIX version 6.0 and later runs only ELF binaries.
#Depends on code changes in ckcdeb.h that make -DIRIX6x define all
#lower IRIX6x values and IRIX51.
irix6x:
	@echo 'Includes fullscreen file display and Yellow Pages...'
	@echo 'Add -mips<n> to specify a particular hardware target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR4 -DDIRENT -DHDBUUCP -DNOGETUSERSHELL \
	-DCK_CURSES -DCK_NEWTERM -DPWID_T=uid_t -DCK_ANSIC -DTCPSOCKET \
	-DSELECT -DCK_RTSCTS -O $(KFLAGS)" \
	"LIBS = -lcurses" "LNKFLAGS = -s $(LNKFLAGS)"

#Silicon Graphics IRIX 6.0.
irix60:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 6.0'
	@$(MAKE) "MAKE=$(MAKE)" \
	"KFLAGS=-DIRIX60 -Olimit 2138 $(KFLAGS)" \
	irix6x KTARGET=$${KTARGET:-$(@)}

#Silicon Graphics IRIX 6.2.
#Serial speeds > 38400 are available in IRIX 6.2 on O-class machines only.
#Note: Olimit must be a number > 0.
irix62:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 6.2'
	@$(MAKE) "MAKE=$(MAKE)" \
	LNKFLAGS="-Wl,-woff,84" \
	"KFLAGS=-DIRIX62 -Olimit 4700 $(KFLAGS)" \
	irix6x KTARGET=$${KTARGET:-$(@)}

#Silicon Graphics IRIX 6.3.
irix63:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 6.3'
	@$(MAKE) "MAKE=$(MAKE)" irix62 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DIRIX63"

#Silicon Graphics IRIX 6.4.
# -woff,84 to linker stops complaints about no symbols loaded from
# curses, and -woff 1110 stops complaints about unreachable "break;"
# statements in ckcpro.c among others.
# tested on SGI Octane, running IRIX 6.4 up to 115200 bps.
# -Olimit 0 means infinite.
irix64:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 6.4'
	@$(MAKE) "MAKE=$(MAKE)" \
	LNKFLAGS="-Wl,-woff,84" \
	"KFLAGS=-DIRIX64 -DCK_RTSCTS -Olimit 3000 -woff 1110 $(KFLAGS)" \
	irix6x KTARGET=$${KTARGET:-$(@)}

irix64gcc:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 6.4 gcc'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -DSVR4 -DIRIX64 -DDIRENT -DHDBUUCP -DNOGETUSERSHELL \
	-DSELECT -DPWID_T=uid_t -DTCPSOCKET -DNOCOTFMC \
	-DCK_ANSIC -DCK_RTSCTS -DCK_NEWTERM -DCK_CURSES \
	$(KFLAGS) -O" "LIBS= -lcurses -ltermcap -lcrypt"

#Note the new Optimization option syntax for MIPSpro CC 7.2.1.2m.
#See note on irix65gcc target about Large File Support (LFS).
irix65:
	@echo 'Making C-Kermit $(CKVER) for SGI IRIX 6.5'
	@$(MAKE) "MAKE=$(MAKE)" LNKFLAGS="-Wl,-woff,84" \
	"KFLAGS=-DIRIX65 -D_LARGEFILE_SOURCE -DCK_RTSCTS -OPT:Olimit=0 \
	-woff 1110,1552,1174 $(KFLAGS)" \
	irix6x KTARGET=$${KTARGET:-$(@)}

#Build for those that have GCC instead of MIPSpro.
#
# Large File Support note: use the define _LARGEFILE_SOURCE to enable support
# for files larger than 2GB.  This may work on releases of Irix prior to
# 6.5.xx.  To verify, check the man page for fstat and verify that off_t is a
# 64 bit value for an -n32 build.  Also check the manpage for fseek and ftell
# to verify that the fseek64 and ftell64 functions are provided.  If so, then
# LFS support should work and you can try adding -D_LARGEFILE_SOURCE to CFLAGS
# for your selected Irix target.
#
irix65gcc:
	@echo 'Making C-Kermit $(CKVER) for SGI IRIX 6.5 with gcc'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CC = gcc" "CC2 = gcc" \
	"CFLAGS= -DSVR4 -DIRIX65 -D_LARGEFILE_SOURCE -DDIRENT -DHDBUUCP \
	-DNOGETUSERSHELL -DSELECT -DPWID_T=uid_t -DTCPSOCKET -DNOCOTFMC \
	-DCK_ANSIC -DCK_RTSCTS -DCK_NEWTERM -DCK_CURSES \
	$(KFLAGS) -O" "LIBS= -lcurses"

# The 64-bit IRIX target works but presumably is no longer needed given the
# large file support in the more portable and compact 32-bit version.
irix65_64:
	@echo 'Making C-Kermit $(CKVER) 64-bit for SGI IRIX 6.5'
	@$(MAKE) "MAKE=$(MAKE)" LNKFLAGS="-Wl,-woff,84" \
	"KFLAGS=-DIRIX65 -64 -DCK_RTSCTS -OPT:Olimit=0 -woff 1110,1552,1174 \
	-DCK_64BIT $(KFLAGS)" \
	irix6x KTARGET=$${KTARGET:-$(@)}

#Dumb down to MIPS-2 if building on R5000 or higher...
irix65mips2:
	@echo 'Making C-Kermit $(CKVER) for SGI IRIX 6.5 MIPS-2'
	@$(MAKE) "MAKE=$(MAKE)" LNKFLAGS="-o32 -mips2 -Wl,-woff,84" \
	"KFLAGS=-DIRIX65 -DCK_RTSCTS -OPT:Olimit=0 -o32 -mips2 \
	-woff 1110,1552,1174 $(KFLAGS)" \
	irix6x KTARGET=$${KTARGET:-$(@)}

#Special target that adds srp, ssl, and zlib support.  This requires
#that you have pkgsrc installed instead of Irix Freeware.  See
#NetBSD.org for pkgsrc for Irix.  You will need to BUILD the srp_client
#package yourself.  Install it manually using the directions found
#in the netbsds+ssl+srp+zlib target comments.
irix65+ssl+srp+zlib:
	@echo 'Making C-Kermit $(CKVER) for IRIX 6.5 with gcc and SSL SRP ZLIB'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CC = gcc" "CC2 = gcc" \
	"CFLAGS= -DIRIX65 -DSVR4 -DDIRENT -DHDBUUCP -DNOGETUSERSHELL -DSELECT \
	-DTCPSOCKET -DNOCOTFMC -DCK_NEWTERM -DPWID_T=uid_t -DCK_ANSIC \
	-I/usr/pkg/include -DCK_AUTHENTICATION -DCK_SRP -DPRE_SRP_1_4_5 \
	-DCK_RTSCTS -DCK_NCURSES -DCK_ENCRYPTION -DCK_CAST -DCK_DES -DCK_SSL \
	-DLIBDES -DZLIB -DFNFLOAT -I/usr/pkg/include/openssl $(KFLAGS) -O" \
	"LIBS= -L/usr/pkg/lib -rpath /usr/pkg/lib -lncurses -lsrp -lgmp -ldes \
	-lssl -lkrypto -lcrypto -lcrypt -lz -lm"

irix6x+krb5:
	@echo 'Includes fullscreen file display and Yellow Pages...'
	@echo 'Add -mips<n> to specify a particular hardware target.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR4 -DDIRENT -DHDBUUCP -DNOGETUSERSHELL \
	-DCK_CURSES -DCK_NEWTERM -DPWID_T=uid_t -DCK_ANSIC -DTCPSOCKET\
	-DSELECT -DCK_RTSCTS -O \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 -DCK_ENCRYPTION -DCK_DES \
	$(K5INC) $(K5INC)/krb5 $(KFLAGS)" \
	"LIBS = -lcurses $(K5LIB) -ldes425 -lkrb5 \
	-lcom_err -lcrypto -lcrypt -lgssapi_krb5" \
	"LNKFLAGS = -s $(LNKFLAGS)"

irix65+krb5:
	@echo 'Making C-Kermit $(CKVER) for SGI IRIX 6.5'
	@$(MAKE) "MAKE=$(MAKE)" \
	LNKFLAGS="-Wl,-woff,84" \
	"KFLAGS=-DIRIX65 -DCK_RTSCTS -OPT:Olimit=0 -woff 1110,1552,1174 \
	$(KFLAGS)" \
	irix6x+krb5 KTARGET=$${KTARGET:-$(@)}

#In case they type "make sys5"...
sys5:
	$(MAKE) "MAKE=$(MAKE)" sys3 KTARGET=$${KTARGET:-$(@)}

#Generic ATT System III or System V (with I&D space)
sys3:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System III'
	@echo 'or System V R2 or earlier...'
	@echo 'add -DNOMKDIR if mkdir is an undefined symbol.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DNOUNICODE -DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL \
	-DNOINITGROUPS -DNOFTRUNCATE -DNOREALPATH -DNOLEARN $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#Generic ATT System III or System V (no I&D space)
sys3nid:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System III'
	@echo 'or System V R2 or earlier, no I&D space...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DNOREALPATH -DNOUNICODE -DNOSYSLOG -DNOSYMLINK \
	-DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE -DNOLEARN $(KFLAGS) -O" \
	"LNKFLAGS ="

#Generic ATT System III or System V R2 or earlier, "no void":
#special entry to remove "Illegal pointer combination" warnings.
sys3nv:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System III'
	@echo 'or System V R2 or earlier...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DATTSV -DNOREALPATH -DNOUNICODE -DNOSYSLOG -DNOGETUSERSHELL \
	-DNOSYMLINK -DNOFTRUNCATE -DNOINITGROUPS -DNOLEARN \
	-Dvoid=int $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

# AT&T 7300 UNIX PC.  As of C-Kermit 6.1, many of these entries don't work
# any more due to "Out of memory" or "Too many defines" errors during
# compilation, at least not on systems without lots of memory.  The sys3upcgc
# entry works (using gcc) with optimization removed, and might also work
# with optimization enabled on machines with larger memories.

#AT&T 7300/UNIX PC (3B1) systems, sys3 but special handling for internal modem.
#Link with the shared library -- the conflict with openi in shared library
#is solved with -Dopeni=xopeni.  Note that the xermit target can't be used
#for the Unix PC; there is no select().
sys3upc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, shared lib...'
	@echo 'If shared lib causes trouble, use make sys3upcold.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DATT7300 -DNOMKDIR -DUSE_MEMCPY -DNOREALPATH -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOREDIRECT -DNOGFTIMER -DNOUNICODE $(KFLAGS) -Dopeni=xopeni" \
	"CC2 = ld /lib/crt0s.o /lib/shlib.ifile" "LNKFLAGS = -s"

#AT&T 7300/Unix PC systems, minimum kermit for those with smaller amounts
#of memory.
sys3upcm:
	@echo Minimum interactive
	$(MAKE) "MAKE=$(MAKE)" sys3upc KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOSPL -DNOFRILLS -DNOHELP -DNODEBUG -DNOTLOG -DNOCSETS \
	-DNOSYSLOG -DNOSETKEY -DNOREALPATH"

#AT&T 7300/UNIX PC (3B1) systems, with curses support.
#Curses and the shared library don't get along, so we don't use the
#shared library.  We need to include CK_NEWTERM to avoid a conflict
#with curses and buffering on stdout.  Merged with submission by
#Robert Weiner/Programming Plus, rweiner@watsun.cc.columbia.edu.
#We don't need -Dopeni=xopeni since we're not using the shared library,
#but we keep it to be consistent with the other entries.
sys3upcc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DATT7300 -DNOREALPATH \
	-DCK_CURSES -DCK_NEWTERM -DNOMKDIR -DNOREDIRECT -DNOGFTIMER -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DUSE_MEMCPY -DNOUNICODE $(KFLAGS) -Dopeni=xopeni" \
	"LIBS = -lcurses" "LNKFLAGS = -s"

#Like sys3upcc but for AT&T UNIX 3.51m (released as a patch on Fix Disk 2),
#adds hardware flow control.
att351m:
	$(MAKE) "MAKE=$(MAKE)" sys3upcc KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DCK_RTSCTS -DUNIX351M"

#As above but with gcc.
att351gm:
	$(MAKE) "MAKE=$(MAKE)" sys3upcgc KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DCK_RTSCTS -DUNIX351M"

#AT&T 7300 UNIX PC (3B1), as above, but no newterm().
sys3upcx:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DATT7300 -DNOREALPATH -DNOUNICODE -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DCK_CURSES -DNOMKDIR -DNOREDIRECT -DNOGFTIMER -DUSE_MEMCPY $(KFLAGS) \
	-Dopeni=xopeni" "LIBS = -lcurses -ltermcap" "LNKFLAGS = -s"

#AT&T 7300/UNIX PC (3B1) systems, with curses and shared library support.
sys3upcshcc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, shared lib...'
	@echo 'With curses.  Requires shcc.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DATT7300 -DNOMKDIR -DNOREALPATH -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DCK_NEWTERM -DCK_CURSES  -DNOREDIRECT -DNOGFTIMER \
	-DUSE_MEMCPY -DNOUNICODE $(KFLAGS) -Dopeni=xopeni" \
	"LNKFLAGS = -i -s" "CC = shcc" "CC2 = shcc" "LIBS = -lcurses"

#AT&T 7300/UNIX PC (3B1) systems, as above, no curses, but use gcc.
sys3upcg:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATT7300 -DNOREDIRECT -DUSE_MEMCPY -DNOUNICODE -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOGFTIMER -DNOMKDIR -DNOREALPATH $(KFLAGS) -Dopeni=xopeni" \
	"CC = gcc" "CC2 = gcc" "LNKFLAGS = -s -shlib"

#AT&T 7300/UNIX PC (3B1) systems, curses and gcc.
#Optimization omitted -- add it back in if your machine has lots of memory.
sys3upcgc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATT7300 -DNOREDIRECT -DUSE_MEMCPY -DNOGFTIMER -DNOUNICODE \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DCK_CURSES -DCK_NEWTERM -DNOMKDIR -DNOREALPATH -DNOLEARN $(KFLAGS)" \
	"CC = gcc" "CC2 = gcc" "LIBS = -lcurses" "LNKFLAGS = -s"

#AT&T 7300/UNIX PC (3B1) systems, special handling for internal modem.
#No FULLSCREEN file transfer display (curses).
sys3upcold:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATT7300 -DNOMKDIR -DUSE_MEMCPY -DNOUNICODE -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOGFTIMER -DNOREDIRECT -DNOREALPATH $(KFLAGS) -O" "LNKFLAGS = -i"

#As above, but with gcc. mininum features - fits on a 400K UNIX PC floppy
#after compression with room to spare; add -DNOSHOW or other -DNOxxxx items
#to reduce size even further.
sys3upcgm:
	@echo Minimum interactive
	$(MAKE) "MAKE=$(MAKE)" sys3upcg KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOSPL -DNOFRILLS -DNOHELP -DNODEBUG -DNOTLOG -DNOCSETS \
	-DNOSETKEY $(KFLAGS)"

#This target is designed to create a version with the most features possible
#that, after compression, still fits on a 400K UNIX PC floppy.
sys3upcgfd:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC floppy...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATT7300 -DNOREDIRECT -DUSE_MEMCPY -DNOSPL -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOGFTIMER -DNOREALPATH -Dopeni=xopeni \
	-DNOHELP -DNODEBUG -DNOTLOG -DNOCSETS -DNOSETKEY -DNOMKDIR $(KFLAGS)" \
	"CC = gcc" "CC2 = gcc" "LNKFLAGS = -s"

#AT&T 6300 PLUS (warning, -O might make it run out of space).
#NOTE: Remove -DHDBUUCP if not using Honey DanBer UUCP.
att6300:
	@echo 'Making C-Kermit $(CKVER) for AT&T 6300 PLUS...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATT6300 -DHDBUUCP -DNOFILEH -DNOREALPATH -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOUNICODE $(KFLAGS) -O -Ml -i" "LNKFLAGS = -i -Ml"

#As above, but with curses support.  Debugging disabled to prevent thrashing.
att6300c:
	@echo 'Making C-Kermit $(CKVER) for AT&T 6300 PLUS...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATT6300 -DHDBUUCP -DNOFILEH -DNOCSETS -DNOREALPATH \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DCK_CURSES -DNODEBUG -DNOUNICODE -DNOLEARN $(KFLAGS) -O -Ml -i" \
	"LNKFLAGS = -i -Ml" "LIBS = -lcurses"

#AT&T 6300 PLUS with no curses, no debugging (about 34K smaller)
# -Optimization saves about 20K too.
att6300nd:
	@echo 'Making C-Kermit $(CKVER) for AT&T 6300 PLUS, no debugging...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATT6300 -DHDBUUCP -DNODEBUG -DNOFILEH -DNOREALPATH \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOUNICODE -DNOLEARN $(KFLAGS) -O -i -Ml" "LNKFLAGS = -i -Ml"

#AT&T 3B2 and maybe 3B20-series computers running AT&T UNIX System V R3.
#This one was actually used to build C-Kermit 7.0 successfully on a 3B2/300.
att3b2:
	@echo 'Making C-Kermit $(CKVER) for AT&T 3B2'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DNOREDIRECT -DUSE_MEMCPY \
	-DNOTIMEVAL -DNOTIMEZONE -DMINIDIAL -DNOCHANNELIO -DNOBIGBUF \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOGFTIMER -DNOREALPATH -Dopeni=xopeni -DNOFRILLS -DNOLEARN \
	-DNOHELP -DNODEBUG -DNOTLOG -DNOCSETS -DNOSETKEY -DNOMKDIR $(KFLAGS)" \
	"CC = gcc" "CC2 = gcc" "LNKFLAGS = -s"

# The next two are likely not to work as-is.

#AT&T 3B2, 3B20-series computers running AT&T UNIX System V.
#This is just generic System V with Honey DanBer UUCP, so refer to sys3hdb.
#Remove -DNONAWS if you can get away with it.
att3bx:
	$(MAKE) "MAKE=$(MAKE)" sys3hdb KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DNONAWS -DNOTIMEVAL"

# 3Bx with charsets (except Unicode) but no curses.
att3bx1:
	@echo 'Making C-Kermit $(CKVER) for AT&T 3B2 or 3B20'
	@echo 'with Honey DanBer UUCP  no curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DHDBUUCP $(KFLAGS) -DNOREDIRECT \
	-DNOTIMEVAL -DNOTIMEZONE -DMINIDIAL -DNOCHANNELIO -DNOBIGBUF \
	-DNOHELP -DNODEBUG -DNOGFTIMER -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOREALPATH -DNOUNICODE -i" \
	"CC = gcc" "CC2 = gcc" "LNKFLAGS = -i -s"

#AT&T 3B2, 3B20-series computers running AT&T UNIX System V,
#with fullscreen file transfer display.
att3bxc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 3B2 or 3B20'
	@echo 'with Honey DanBer UUCP and curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DHDBUUCP -DNONAWS -DNOTIMEVAL $(KFLAGS) \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOREALPATH -DCK_CURSES -DCK_NEWTERM -DNOUNICODE -DNOLEARN -i -O" \
	"LNKFLAGS = -i" "LIBS=-lcurses"

#3bx with curses but no charsets
att3bxc3:
	@echo 'Making C-Kermit $(CKVER) for AT&T 3B2 or 3B20'
	@echo 'with Honey DanBer UUCP with curses...  no CSETS'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DHDBUUCP $(KFLAGS) -DNOREDIRECT \
	-DNOTIMEVAL -DNOTIMEZONE -DMINIDIAL -DNOCHANNELIO -DNOBIGBUF \
	-DNOHELP -DNODEBUG -DNOGFTIMER -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOREALPATH -DNOCSETS -DCK_CURSES -DCK_NEWTERM -i" \
	"CC = gcc" "CC2 = gcc" "LNKFLAGS = -i -s" "LIBS = -lcurses"

#Any System V R2 or earlier with Honey DanBer UUCP (same as above)
sys3hdb:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System III'
	@echo 'or System V R2 or earlier with Honey DanBer UUCP...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DHDBUUCP -DNOREALPATH -DNOUNICODE -DNOLEARN \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	$(KFLAGS) -i -O" "LNKFLAGS = -i"

#Sperry/UNISYS 5000 UTS V 5.2 (System V R2), Honey DanBer UUCP
unisys5r2:
	@echo 'Making C-Kermit $(CKVER) for Sperry/UNISYS 5000 UTS V 5.2...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DUNISYS52 -DHDBUUCP -DNOREALPATH -DNOUNICODE \
	-DNOSYSLOG -DNOSYMLINK -DNOGETUSERSHELL -DNOINITGROUPS -DNOFTRUNCATE \
	-DNOLEARN $(KFLAGS) -i -O" "LNKFLAGS = -i"

#In case they say "make sys5hdb" instead of "make sys3hdb"...
sys5hdb:
	$(MAKE) "MAKE=$(MAKE)" sys3hdb

#Create the common header line for all hpux[5-11]* entries and above. This 
#extra entry is here because our header message length may differ for each 
#C-Kermit version. Don't use 'fold -s' for HP-UX 5.x - 7.x! This option is 
#available only for HP-UX 8.0 and above!
hpux-header:
	@HPUX=`uname -r | sed -e 's/^[^1-9]*//' -e 's/\.00$$/.0/'` ; \
	[ "$(MESSAGE0)" ] && MESSAGE1="$(MESSAGE0)" ; \
	Message0='Making C-Kermit $(CKVER) for HP9000 HP-UX' ; \
	Message1=$${MESSAGE1:='without any extra compiler optimization'} ; \
	MessageH="$$Message0 $$HPUX" ; \
	case $$HPUX in \
	  [567].*) echo "$$MessageH\n$$Message1" ;; \
	      *.*) echo "$$MessageH $${Message1}$(MESSAGE1A)" | fold -s ;; \
	esac | sed -e 's/^ //' -e 's/ *$$//'

# Peter E's updated HP-UX 5.xx entries Oct 2001.

#HP-9000 500 HP-UX 5.xx, no TCP/IP.
# Last known successful build: C-Kermit 8.0.206 2002/20/27.
hpux0500:
	@MESSAGE0="no TCP/IP and no compiler optimization";\
	MESSAGE0=$${MESSAGE1:-$$MESSAGE0} \
	$(MAKE) hpux-header
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DHPUX -DHPUX5 -DHPUXPRE65 -DNOREDIRECT -DDCLGETCWD \
	-DNOGETUSERSHELL -DNOGFTIMER -DNOSYSLOG -DNOTOMACROS -DNOLSTAT \
	-DNOSYMLINK -DNOINITGROUPS -DNOUNICODE -DNOLEARN -DNOLONGLONG \
	-DVOID=int -DCKVOID=int $(KFLAGS)" "LIBS = $(LIBS)" "LNKFLAGS = "

#HP-9000 500 HP-UX 5.21 with Wollongong WIN/TCP 1.2 TCP/IP.
#Requires /usr/wins/usr/include and /usr/lib/libnet.a from Wollongong.
#Optimization skipped - takes forever.	Really.
# WARNING: this doesn't work if a file called "hpux0500" is on the disk.
# Last known successful build: C-Kermit 8.0.206 2002/20/27.
hpux0500wintcp:
	@MESSAGE1="with WIN/TCP but without any extra compiler optimization" \
	$(MAKE) hpux0500 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = -DTCPSOCKET -DHPUX5WINTCP -DINADDRX -DNO_DNS_SRV \
	-DNOMHHOST -DVOID=int -DCKVOID=int -DNOHADDRLIST -DNOLONGLONG \
	-I/usr/wins/usr/include $(KFLAGS)" "LIBS = /usr/lib/libnet.a"

#HP-UX 6.5, short filenames, no network and no curses support.
#ckcpro, ckuusr, ckuus3 and others are broken out because they make the
#optimizer run away.  Note that the XERMIT target does not work with HP-UX 6.5!
#
#If you get compiler warnings like:
#'Switch table overflow. Try the -Wc,-Nw option.' (for ckcuni.c, or
#other files) increase the '...' value in '-Wc,-Nw...'! The default maximum
#switch table stack (-Nw) is 250 table entries. ckcuni.c from Oct 16 2009
#needs 257 table entries (C-Kermit Version "9.0.299").
#OK: 2010/03/26
hpux0650:
	@$(MAKE) hpux-header
	@MESSAGE2=$${MESSAGE2:-'and NO network'}; \
	echo "supporting: NO long filenames $$MESSAGE2."

	$(MAKE) KTARGET=$${KTARGET:-$(@)} \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuusr.$(EXT) ckuxla.$(EXT) ckcftp.$(EXT) ckcpro.$(EXT) \
	"CFLAGS = -DHPUX -DHPUX6 -DSIG_V -DNOSYSLOG -DNOSELECT -DFNFLOAT \
	-DDCLGETCWD -DNOGETUSERSHELL -DNO_DNS_SRV -DNOLEARN -DNOLONGLONG \
	$(KFLAGS)"

	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DHPUX -DHPUX6 -DSIG_V -DNOSYSLOG -DNOSELECT -DFNFLOAT \
	-DDCLGETCWD -DNOGETUSERSHELL -DNO_DNS_SRV -DNOLEARN -DNOLONGLONG \
	$(KFLAGS) -Wc,-Nw260 $(OFLAGS)" "LNKFLAGS = -s" "LIBS = -lm $(LIBS)"

#Exactly as above, plus curses:
#OK: 2009/10/06
hpux0650c:
	@MESSAGE2="and NO network but with curses" \
	$(MAKE) hpux0650 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = -DCK_CURSES $(KFLAGS)" \
	"LIBS = -lcurses"

#Exactly as above, plus curses + network:
#OK: 2009/10/02
hpux0650tcpc:
	@MESSAGE2="but with curses and with TCP/IP" \
	$(MAKE) hpux0650 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DCK_CURSES -DTCPSOCKET -DNOHADDRLIST \
	-DINTSELECT -DNOCKGETFQHOST $(KFLAGS)" \
	"LIBS=-lcurses"

#Exactly as hpux0650 but with compiler optimization:
#OK: 2009/10/06
hpux0650o:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0650 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#Exactly as hpux0650c but with compiler optimization:
#OK: 2009/10/06
hpux0650oc:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0650c KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#Exactly as hpux0650tcpc but with compiler optimization:
#OK: 2009/10/06
hpux0650otcpc:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0650tcpc KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#Take this as startup entry for all 'non-optimized' files under HP-UX 7.x!
#Make sure we don't call it with the '-O' option because this will blow up
#the compiler!
#OK: 2009/09/30
hpux0700noopt:
	@case "$(CFLAGS)" in \
	*-O*) echo "Don't use CFLAGS= -O here!" ;; \
	   *) $(MAKE) KTARGET=$${KTARGET:-$(@)} \
	      ckuusr.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
	      ckuus6.$(EXT) ckuus7.$(EXT) ckuxla.$(EXT) \
	      ckcuni.$(EXT) ckcftp.$(EXT) ckcpro.$(EXT) \
	      ;; \
	esac

#HP-UX 7.0, no long filenames, no network support, no curses.
#If you get compiler warnings like:
#'Switch table overflow. Try the -Wc,-Nw option.' (for ckcuni.c, or
#other files) increase the '...' value in '-Wc,-Nw...'! The default maximum
#switch table stack (-Nw) is 250 table entries. ckcuni.c from Oct 16 2009
#needs 257 table entries (C-Kermit Version "9.0.299").
#OK: 2010/10/26
hpux0700sf:
	@$(MAKE) hpux-header
	@echo 'supporting: NO long filenames, NO network, NO curses.'
	$(MAKE) hpux0700noopt KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DHPUX -DHPUX7 -DSIG_V -DNOGETUSERSHELL -DFNFLOAT \
	-DNO_DNS_SRV $(KFLAGS) -Wc,-Nw260"

	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DHPUX -DHPUX7 -DSIG_V -DNOGETUSERSHELL -DFNFLOAT \
	-DNO_DNS_SRV $(KFLAGS) -Wc,-Nw260 $(OFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lm $(LIBS)"

#Exactly as hpux0700sf but with compiler optimization:
#OK: 2009/09/30
hpux0700osf:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0700sf KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#HP-UX 7.0, short filenames, but with tcp/ip and curses.
#To use this, you must have bought the ARPA Services Product from HP, and you
#must have /usr/lib/libBSD.a.
#
#If you get compiler warnings like:
#'Symbol table overflow. Try the -Wc,-Ns option.' (as for ckuus4.c or
#other files) increase the '...' value in '-Wc,-Ns...'! The default maximum
#symbol table size (-Ns) is 2000 table entries. ckuus4.c from Mar 12 2010
#needs 2031 table entries (C-Kermit Version "9.0.299").
#OK: 2010/03/24
hpux0700sftcpc:
	@$(MAKE) hpux-header
	@echo 'supporting: NO long filenames, \c'
	@echo 'but with networking, curses, HDB uucp...'
	$(MAKE) hpux0700noopt KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS =  -DHPUXDEBUG -DHPUX -DHPUX7 -DTCPSOCKET -DSIG_V \
	-DCK_REDIR -DCK_RTSCTS -DCK_CURSES -DNOGETUSERSHELL -DFNFLOAT \
	-DNO_DNS_SRV -DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" \
	-DNOLONGLONG $(KFLAGS) -Wc,-Nw260,-Ns2040"

	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS =  -DHPUXDEBUG -DHPUX -DHPUX7 -DTCPSOCKET -DSIG_V \
	-DCK_REDIR -DCK_RTSCTS -DCK_CURSES -DNOGETUSERSHELL -DFNFLOAT \
	-DNO_DNS_SRV -DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" \
	-DNOLONGLONG $(KFLAGS) -Wc,-Nw260,-Ns2040 $(OFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lm -lBSD -lcurses"

#Exactly as above but with compiler optimization:
#OK: 2009/09/30
hpux0700osftcpc:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0700sftcpc KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#HP 9000 series 300/800 HP-UX 7.0, long filenames, network support, HDB uucp,
#but NO curses. See comments in hpux0700sftcpc about TCP/IP support.
#
#If you get compiler warnings like:
#'Symbol table overflow. Try the -Wc,-Ns option.' (as for ckuus4.c or
#other files) increase the '...' value in '-Wc,-Ns...'! The default maximum
#symbol table size (-Ns) is 2000 table entries. ckuus4.c from Mar 12 2010
#needs 2031 table entries (C-Kermit Version "9.0.299").
#OK: 2010/03/24
hpux0700lfn:
	@$(MAKE) hpux-header
	@echo 'supporting: long filenames, networking, HDB uucp$(MESSAGE2)...'
	$(MAKE) hpux0700noopt KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS =  -DHPUXDEBUG -DHPUX -DHPUX7 -DTCPSOCKET -DSIG_V -DFNFLOAT \
	-DNOGETUSERSHELL -DNOSETBUF -DCK_REDIR -DCK_RTSCTS -DLONGFN \
	-DNO_DNS_SRV -DDIRENT -DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" \
	-DNOLONGLONG $(KFLAGS) -Wc,-Nw260,-Ns2040"

	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS =  -DHPUXDEBUG -DHPUX -DHPUX7 -DTCPSOCKET -DSIG_V -DFNFLOAT \
	-DNOGETUSERSHELL -DNOSETBUF -DCK_REDIR -DCK_RTSCTS -DLONGFN \
	-DNO_DNS_SRV -DDIRENT -DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" \
	-DNOLONGLONG $(KFLAGS) -Wc,-Nw260,-Ns2040 $(OFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lm -lBSD $(LIBS)"

#Exactly as above + curses.
#OK: 2009/09/30
hpux0700lfnc:
	@MESSAGE2=', curses' \
	$(MAKE) hpux0700lfn KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = -DCK_CURSES $(KFLAGS)" \
	"LIBS = -lcurses"

#Exactly as above hpux0700lfn but with compiler optimization:
#OK: 2009/09/30
hpux0700olfn:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0700lfn KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#Exactly as above hpux0700lfnc but with compiler optimization:
#OK: 2009/09/30
hpux0700olfnc:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0700lfnc KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#HP 9000 Series 300 or 400, HP-UX 8.0, long filenames and TCP/IP support.
#This one should also work on 700/800, but without PA-specific optimization.
#In case -DCK_RTSCTS and -DCK_REDIR make trouble, remove them.
#NOTE: ckcpro.c, ckuusr.c and ckuus3.c blow up the optimizer, so don't optimize
#them.
#For HP-UX 8.0 on Motorola CPUs, you might have to reinstall your kernel with
#maxdsiz >= 0x03000000.  But if physical memory is small, that still will not
#help much.
#OK: 2009/10/01
hpux0800:
	@$(MAKE) hpux-header
	@MESSAGE3=$${MESSAGE3:='TCP/IP'}; \
	echo "supporting: long filenames, $$MESSAGE3, HDB UUCP$(MESSAGE2)..."
	$(MAKE) -B "CC=$(CC)" "CC2=$(CC2)" KTARGET=$${KTARGET:-$(@)} \
	ckcpro.$(EXT) ckuusr.$(EXT) ckuus3.$(EXT) \
	"CFLAGS =  -DCK_REDIR -DHPUXDEBUG -DHPUX -DHPUX8 -DRENAME -DSIG_V \
	-DNOSETBUF -DDIRENT -DCK_RTSCTS -DSTERMIOX -DLONGFN -DTCPSOCKET \
	-DHDBUUCP  -DNO_DNS_SRV -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DFNFLOAT \
	-DNOLONGLONG $(KFLAGS)"

	$(MAKE) -B "CC=$(CC)" "CC2=$(CC2)" xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS =  -DCK_REDIR -DHPUXDEBUG -DHPUX -DHPUX8 -DRENAME -DSIG_V \
	-DNOSETBUF -DDIRENT -DCK_RTSCTS -DSTERMIOX -DLONGFN -DTCPSOCKET \
	-DHDBUUCP  -DNO_DNS_SRV -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DFNFLOAT \
	-DNOLONGLONG -DNODCLENDUSERSHELL $(KFLAGS) $(OFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lm -lBSD $(LIBS)"

#Exactly as above hpux0800 + curses.
#OK: 2009/10/01
hpux0800c:
	@MESSAGE2=', curses' \
	$(MAKE) hpux0800  KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS) -DCK_CURSES" "LIBS = -lcurses"

#HP 9000 HP-UX 8.0, no TCP/IP because /usr/lib/libBSD.a can't be found,
#or TCP/IP header files missing.
#OK: 2009/10/01
hpux0800notcp:
	@MESSAGE3='NO network, NO curses' \
	$(MAKE) "MAKE=$(MAKE)" hpux0800 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS) -UTCPSOCKET"

#Now the same as above hpux0800 but with compiler optimization
#OK: 2009/10/01
hpux0800o:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0800 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#Exactly as above hpux0800 + curses and with compiler optimization.
#OK: 2009/10/01
hpux0800oc:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) hpux0800c KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O" "LIBS = -lcurses"

#Exactly as above hpux0800notcp but with compiler optimization
#OK: 2009/10/01
hpux0800onotcp:
	@MESSAGE1="with compiler optimization" \
	$(MAKE) "MAKE=$(MAKE)" hpux0800notcp KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -O"

#HP 9000 Series 700 or 800, HP-UX 8.0, long filenames and TCP/IP support.
# Like the previous entries, but with PA-RISC-specific optimization.
#OK: 2009/10/01
hpux0800pa:
	@MESSAGE1="with PA-RISC-specific optimization" \
	$(MAKE) hpux0800 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS) +Obb1100"

#As above, but with curses.
#OK: 2009/10/01
hpux0800pac:
	@MESSAGE1="with PA-RISC-specific optimization" \
	$(MAKE) hpux0800c KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS) +Obb1100"

#As above, but compiled with GCC 2.3.3.
#OK: 2009/10/01
hpux0800pagcc:
	@MESSAGE1='using the gcc compiler' \
	$(MAKE) hpux0800 KTARGET=$${KTARGET:-$(@)} \
	"CC=gcc" "CC2=gcc" "KFLAGS = -funsigned-char $(KFLAGS)"

#HP-UX 9.0, 9.01, 9.03, 9.04, 9.05, 9.07, 9.10 ..., + TCP/IP + curses, fully
#configured.  Use this entry with the restricted compiler: no optimization, no
#ANSI support.  If you get unresolved sockets library references at link time,
#then try adding -lBSD to LIBS, or else remove -DTCPSOCKET to build a version
#without TCP/IP support.
#
#Please note that we have to add the compiler option +DA1.0/+DA1.1 to avoid
#core-dumps for large arguments in IF MATCH. The man page says these options
#are default but C-Kermit dumps core without them! Therefore keep them
#untouched. If you want to overwrite or disable the +DA1.0/+DA1.1 option use
#'make hpux0900 OFLAGS=...'. An other possibility would be to create a new
#kernel with maxssiz >= 0x01185000 (default maxssiz=0x00800000).
#OK: 2009/09/24
hpux0900:
	@MESSAGE1A='. Read hpux0900 entry comments if you have trouble.' \
	$(MAKE) hpux-header
	@case `uname -m` in \
	  */[34]*) KFLAGS='-DNOLONGLONG $(KFLAGS)' ;; \
	  */7*)    AFLAGS='+DA1.1' ;; \
	  */8*)    AFLAGS='+DA1.0' ;; \
	esac ; \
	OFLAGS=$${OFLAGS:-$$AFLAGS} ; \
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DHPUXDEBUG -DHPUX9 -DSTERMIOX -DDIRENT -DUTIMEH \
	-DNOSETBUF -DCK_CURSES -DTCPSOCKET -DRENAME -DCK_REDIR -DLONGFN \
	-DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DFNFLOAT \
	-DNODCLENDUSERSHELL $$KFLAGS $$OFLAGS" \
	"LNKFLAGS = -s" "LIBS = -lm -lcurses" "CC=$(CC)" "CC2=$(CC2)"

#Like hpux0900, but for the "value-added" compiler on all HP 9000 models.
#Adds optimization and ANSI compilation:
# +O2 makes smaller executable (= -O = Level-1 and global optimization)
# +O3 adds interprocedural global optimization, makes bigger executable.
# Please note: To support long-long we would need compiler switch '-Ae' but
# this one works only on Risc systems. But the equivalant compiler flags
# '-Aa -D_HPUX_SOURCE +e' works for Motorola and Risc.
# If optimization fails on some modules, you can add:
#  +Obb<n>, +Olimit <n>, or +Onolimit, depending on your cc version,
# where <n> is a number, e.g. +Obb1200.  In other words, if you get optimizer
# warnings, add (for example) +Obb1200; if you still get optimizer warnings,
# increase the number.  Repeat until warnings go away.  If your compiler
# permits it, use +Onolimit. If optimizer blows up on ckcpro.c, see next entry.
# Reportedly, on some configurations, such as HP9000/425e or /340, perhaps
# depending on the amount of main memory, this entry might fail no matter what
# you do ("Out of Memory", "cc: Fatal error in /lib/c.c1", etc).  In that case
# use "make hpux0900" (no "o").
#OK: 2009/09/24
hpux0900o:
	@MESSAGE1=$${MESSAGE1:-"with compiler optimization"} \
	$(MAKE) hpux0900 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS) -Aa -DCK_ANSIC -D_HPUX_SOURCE +O2 +e"

# For HP-UX 9.0 on Motorola CPUs, optimization of ckcpro.c tends to blow up
# the compiler.  You might have to reinstall your kernel with maxdsiz >=
# 0x03000000.  But if physical memory is small, that still will not help much.
# In that case, use this entry to skip optimization of ckcpro.c.  But for
# C-Kermit 8.0.208 you need a kernel with maxdsiz >= 0x02000000 to compile an
# optimized ckcftp.c.
# Please note: To support long-long we would need compiler switch '-Ae' but
# this one works only on Risc systems. But the equivalant compiler flags
# '-Aa -D_HPUX_SOURCE +e' works for Motorola and Risc.
hpux0900m68ko:
	@MESSAGE1='without compiler optimization for ckcpro.$(EXT) ...' \
	$(MAKE) hpux-header
	$(MAKE) ckuusr.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) \
	ckcftp.$(EXT) ckcpro.$(EXT) \
	"CFLAGS = -DHPUXDEBUG -DHPUX9 -DSTERMIOX -DDIRENT \
	-DNOSETBUF -DCK_CURSES -DTCPSOCKET -DRENAME  -DCK_REDIR -DLONGFN \
	-DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DFNFLOAT $(KFLAGS)"
	@echo
	@MESSAGE1="with compiler optimization for the rest" \
	$(MAKE) hpux0900 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS) -Aa -DCK_ANSIC -D_HPUX_SOURCE +O2 +e"

# Old name for hpux0900m68ko.
hpux0900mot:
	$(MAKE) hpux0900m68ko KTARGET=$${KTARGET:-$(@)} "KFLAGS = $(KFLAGS)"

#Like hpux0900o but with additional model-700/800-specific optimizations.
# +ESlit = consolidate strings in read-only memory.
# +ESfsc = inline millicode calls when comparing pointers.
hpux0900o700:
	@echo 'If you get optimizer warnings \c'
	@echo 'try "make hpux0900o700 KFLAGS=+Obb1200"'
	@MESSAGE1="with PA-RISC-specific optimizations" \
	$(MAKE) hpux0900o KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS) +ESlit +ESsfc"

#HP-UX 9.0, 9.01, 9.03, 9.04, 9.05, 9.07, 9.10 ..., + TCP/IP + curses, fully
#configured, built with gcc, all models except 800 series.
#You might need to add the include path for gcc headers, for example:
# 'KFLAGS=-I/usr/gnu/lib/gcc-lib/hppa1.1-hp-hpux/2.4.5/include/'
hpux0900gcc:
	@MESSAGE1='using the gcc compiler' \
	$(MAKE) hpux0900 KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"KFLAGS = -DCK_ANSIC $(KFLAGS)" \
	"OFLAGS = -funsigned-char -O2 $(OFLAGS)"

#HP-9000 HP-UX 10.0 + TCP/IP + curses, fully configured.
#Use with restricted (bundled) compiler: no optimization, no ANSI support.
#libcurses needed for fullscreen file xfer display in HP-UX 10.00 and 10.01.
#libHcurses (NOT libcurses!) for fullscreen display, to work around fatal bugs
#in HP-UX 10.10 and 10.20 curses. Maybe we could use lcurses for 10.30, since
#the 10.10 curses problem is supposedly fixed in 10.30.
# +DA1.0 = Generate PA-RISC 1.0 code that runs on both 700 and 800 models.
# +DA1.1 = Generate PA-RISC 1.1 code that runs on both 700 and 800 models.
# Note that HP-UX 10.20 and upwards do not support PA-RISC 1.0 systems.
# And that as of Dec 2001, 11.00 and 11.11 are PA-only and 11.20 is IA64-only.
# Later 11.2x releases are expected to be for both.  Architecture can be
# determined with the model command, at least in 10.20 and later...
#For future releases, we need to include +DA1.1 for PA builds, so that a
#binary built on PA 2.0 will still work on PA 1.1 machines, whereas +DA1.1
#must NOT be included for IA64 builds.
#4 Jan 2006 - Added Large File Support (LFS).  Large files (>2GB) are
#possible in HP-UX 10.20 and later.  The only change is to add:
# -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
#to KFLAGS.  These should be harmless in 10.00 and 10.10, if any of examples
#of those still exist, but I have no way to test this hypothesis.
#OK: 2009/11/16
hpux1000:
	@$(MAKE) hpux-header
	@LIBS='-lHcurses' ; \
	AFLAGS='+DA1.1' ; \
	case `uname -r` in \
	   [AB].10.0*)  KFLAGS='-DHPUX1000 $(KFLAGS)' ; \
	                AFLAGS='+DA1.0' ; LIBS='-lcurses'  ;; \
	   [AB].10.1*)  KFLAGS='-DHPUX1010 -D__HP_CURSES $(KFLAGS)' ; \
	                ;; \
	   [AB].10.2*)  KFLAGS='-DHPUX1020 -D__HP_CURSES $(KFLAGS)' ; \
	                ;; \
	   [AB].10.3*)  KFLAGS='-DHPUX1030 -D__HP_CURSES $(KFLAGS)' ; \
	                ;; \
	   [AB].10.?*)  KFLAGS='-DHPUX10XX -D__HP_CURSES $(KFLAGS)' ; \
	                ;; \
	   [AB].11.0*)  KFLAGS='-DHPUX1100 -D__HP_CURSES $(KFLAGS)' ; \
	                ;; \
	   [AB].11.1*)  KFLAGS='-DHPUX1100 -D__HP_CURSES $(KFLAGS)' ; \
	                ;; \
	   [AB].11.?*)  KFLAGS='-DHPUX1100 -D__HP_CURSES $(KFLAGS)' ; \
	                AFLAGS='' ; LIBS='-lcurses' ;; \
	esac ; \
	OFLAGS=$${OFLAGS:-$$AFLAGS} ; \
	$(MAKE) "SHELL=/usr/bin/sh" xermit KTARGET=$${KTARGET:-$(@)} \
	"CC=$(CC)" "CC2=$(CC2)" \
	"CFLAGS = -DHPUX10 -DDIRENT -DSTERMIOX -DCK_DSYSINI -DHDBUUCP \
	-DCK_CURSES -DCK_WREFRESH -DTCPSOCKET -DCK_REDIR -DRENAME -DFNFLOAT \
	-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
	$$KFLAGS $$OFLAGS" \
	"LNKFLAGS=-s $(LNKFLAGS)" "LIBS = -lm $$LIBS $(KLIBS)"

# This is a kludge, copying hpux0900gcc and adapting hpux1000
# (add CC and CC2, drop the A1.[0||1])
# Builds w/ no compiler warnings but minimally tested.
#
#OK: 2009/09/21
hpux1000gcc:
	@MESSAGE1="using the gcc compiler $(MESSAGE1)" \
	$(MAKE) hpux1000 KTARGET=$${KTARGET:-$(@)} CC=gcc CC2=gcc \
	"KFLAGS = $(KFLAGS)" "OFLAGS = -DCK_ANSIC -funsigned-char -O2"

# Trusted HP-UX 10
# echo KFLAGS=$(KFLAGS) YTARGET YTARGET=$(YTARGET) $(XTARGET) ;
hpux1000t:
	@case "$(KTARGET)" in \
	   *+openssl | *+ssl) \
		KENTRY=hpux1000o+openssl ;; \
	   *gcc) \
		KENTRY=hpux1000gcc ;; \
	   *o+) KENTRY=hpux1000o+ ;; \
	   *o)	KENTRY=hpux1000o ;; \
	   *)	KENTRY=hpux1000 ;; \
	esac ; \
	MESSAGE1="and support for 'Trusted HP-UX'" \
	$(MAKE) $$KENTRY KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS = $(KFLAGS) -DHPUX10_TRUSTED" "KLIBS=-lsec"

hpux1000to:
	$(MAKE) hpux1000t KTARGET=$${KTARGET:-$(@)}

hpux1000to+:
	$(MAKE) hpux1000t KTARGET=$${KTARGET:-$(@)}

hpux1000tgcc:
	$(MAKE) hpux1000t KTARGET=$${KTARGET:-$(@)}

hpux1000to+ssl hpux1000to+openssl:
	$(MAKE) hpux1000t KTARGET=$${KTARGET:-$(@)}

hpux1000tgcc+ssl hpux1000tgcc+openssl:
	$(MAKE) hpux1000t KTARGET=$${KTARGET:-$(@)}

#HP-9000 HP-UX 10.00 and higher with ANSI prototyping and optimization.
#PA-RISC only, no Motorola or other hardware is support in HP-UX 10.00++.
#The unbundled optional compiler is required.
#Your path should start with /opt/ansic/bin.
# -Wl,-Fw = Remove stack unwind table (info used by debuggers).
# +O2 makes a smaller executable (= -O = Level-1 and global optimization).
# +O3 adds interprocedural global optimization, makes a bigger executable.
# +Onolimit allows all modules to be optimized, no matter how complex.  But:
#  (a) +Onolimit does not seem to always be there in HP-UX 10.00, and:
#  (b) some modules might take hours on low-memory and/or slow systems.
# The following are PA-RISC-specific optimizations:
# +ESlit = Consolidate strings in read-only memory.
# +ESfsc = Inline millicode calls when comparing pointers.
# You might need to configure your kernel for a maxdsiz of 0x0B000000 (176MB)
# or greater to prevent the optimizer from running out of space.
# December 2001: +ESlit +ESsfc removed because not supported on IA64.
# Somebody who cares can use 'model' to see whether it's PA-RISC or IA64
# and include the architecture-specific optimization flags.  Also note:
# +DA1.1 is PA-only.  If this is included in in HP-UX 11.00 or later,
# then +DS2.0 should be included too (but don't use +DS2.0 without +DA1.1,
# or else the binary won't run on older PA hardware).
#OK: 2009/09/21
hpux1000o:
	@case `uname -m` in \
	  ia64) ;; \
	  *) MFLAGS='+ESlit +ESsfc' ;; \
	esac ; \
	MESSAGE1="with PA-RISC-specific optimizations $(MESSAGE1)" \
	$(MAKE) "SHELL=/usr/bin/sh" "PATH=/opt/ansic/bin:$$PATH" hpux1000 \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = $(KFLAGS) \
	-Ae -D_HPUX_SOURCE -DCK_ANSIC -DUTIMEH \
	+O2 -Wl,-Fw $$MFLAGS"

#Like hpux1000o but with "+Onolimit".
#On 700 series set kernel parameter maxdsiz >= 0x0D000000 (=208MB).
#Takes a long time.
hpux1000o+:
	@MESSAGE1="and +Onolimit $(MESSAGE1)" KTARGET=$${KTARGET:-$(@)} \
	$(MAKE) hpux1000o \
	"KFLAGS = $(KFLAGS) +Onolimit"

#HP-UX 10.xx + 11.xx with optimizing ANSI compiler and OpenSSL.
#Define SSLLIB and SSLINC appropriately for your OpenSSL installation.
#To overwrite the default SSLLIB and SSLINC settings you can also use the
#command-line variable KSSLLIB and KSSLINC like:
#make hpux1000o+openssl KSSLLIB=-L/opt/openssl/lib KSSLINC=-I/...
#Ditto for the Zlib location.
#This entry works for C-Kermit 8.0.206 on HP-UX 10.20 + 11.11
#with OpenSSL 0.9.6 + 0.9.7
#NOTE: an ANSI C compiler is required for the SSL interface.  If you don't
#have the HP Optimizing ANSI compiler, see the hpux1000gcc+openssl target
#below.
hpux1000o+ssl hpux1000o+openssl:
	@case "$(KTARGET)" in \
	   *gcc+*) \
	        KENTRY=hpux1000gcc ;; \
	   *)   KENTRY=hpux1000o ;; \
	esac ; \
	case "$(KTARGET)" in \
	   *-zlib*) \
	        DZLIB= LZLIB= ;; \
	   *)   DZLIB=-DZLIB LZLIB='-L/opt/zlib/lib -lz' ;; \
	esac ; \
	SSLINC=$${KSSLINC:-$(SSLINC)}; \
	SSLLIB=$${KSSLLIB:-$(SSLLIB)}; \
	MESSAGE1="and with OpenSSL $(MESSAGE1)" \
	$(MAKE) $$KENTRY KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DCK_AUTHENTICATION -DCK_SSL -DOPENSSL_097 $$DZLIB \
	$$SSLINC $(KFLAGS)" \
	KLIBS="$(KLIBS) \
	$$SSLLIB -lssl -lcrypto \
	$$LZLIB \
	"

# Ditto but without Zlib:
hpux1000o+ssl-zlib hpux1000o+openssl-zlib:
	@MESSAGE1="but without Zlib $(MESSAGE1)" \
	$(MAKE) hpux1000o+ssl KTARGET=$${KTARGET:-$(@)}

#HP-UX 10.00 or higher with OpenSSL 0.9.7.  Compiled with gcc.
#From Chris Chaney, NEC America Inc.  His instructions:
# (1) Install gcc version 3.2.3 & binutils version 2.13.2
#     (used binary depot from http://hpux.cs.utah.edu/)
# (2) Install gcc make version 3.80 from http://hpux.cs.utah.edu/
#
# or: gcc 2.9.2000-12-1 from "Linux to hp-ux 11.0/11i porting kit version 1.0
#     (2CD)" free from:  http://www.software.hp.com
#
# (3) Install openSSL version 0.9.7b from http://www.software.hp.com
# (4) Install flex version 2.5.4 from http://hpux.cs.utah.edu/
# (5) Install gmp version 3.1.1 from http://hpux.cs.utah.edu/
#
#Note from Peter Eichhorn, assyst Munich. It works also without gcc make!
hpux1000gcc+ssl hpux1000gcc+openssl:
	$(MAKE) hpux1000o+openssl KTARGET=$${KTARGET:-$(@)}

# Ditto but without Zlib:
hpux1000gcc+ssl-zlib hpux1000gcc+openssl-zlib:
	$(MAKE) hpux1000o+openssl-zlib KTARGET=$${KTARGET:-$(@)}

# Same for HP-UX 11
hpux1100o+ssl hpux1100o+openssl:
	$(MAKE) hpux1000o+openssl KTARGET=$${KTARGET:-$(@)}

#OK: 2009/09/26
hpux1100gcc+ssl hpux1100gcc+openssl:
	$(MAKE) hpux1000gcc+openssl KTARGET=$${KTARGET:-$(@)}

hpux1100o+ssl-zlib hpux1100o+openssl-zlib:
	$(MAKE) hpux1000o+openssl-zlib KTARGET=$${KTARGET:-$(@)}

hpux1100gcc+ssl-zlib hpux1100gcc+openssl-zlib:
	$(MAKE) hpux1000gcc+openssl-zlib KTARGET=$${KTARGET:-$(@)}

# HP-UX 11
# Note: these are 32-bit builds even on IA64.
# Adding +DD64 to CFLAGS produces 64-bit object files,
# but the linker fails to find the needed 64-bit libs.
#OK: 2009/09/26
hpux1100:
	$(MAKE) hpux1000 KTARGET=$${KTARGET:-$(@)}

#OK: 2009/09/26
hpux1100o:
	$(MAKE) hpux1000o KTARGET=$${KTARGET:-$(@)}

hpux1100o+:
	$(MAKE) hpux1000o+ KTARGET=$${KTARGET:-$(@)}

#OK: 2009/09/26
hpux1100gcc:
	$(MAKE) hpux1000gcc KTARGET=$${KTARGET:-$(@)}

# Trusted HP-UX 11
hpux1100t:
	$(MAKE) hpux1000t KTARGET=$${KTARGET:-$(@)}

hpux1100to:
	$(MAKE) hpux1000to KTARGET=$${KTARGET:-$(@)}

hpux1100to+:
	$(MAKE) hpux1000to+ KTARGET=$${KTARGET:-$(@)}

hpux1100tgcc:
	$(MAKE) hpux1000tgcc KTARGET=$${KTARGET:-$(@)}

hpux1100to+ssl hpux1100to+openssl:
	$(MAKE) hpux1000to+openssl KTARGET=$${KTARGET:-$(@)}

hpux1100tgcc+ssl hpux1100tgcc+openssl:
	$(MAKE) hpux1000tgcc+openssl KTARGET=$${KTARGET:-$(@)}

#Regulus on CIE Systems 680/20
cie:
	@echo 'Making C-Kermit $(CKVER) for CIE Systems 680/20 Regulus...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DATTSV -DNOFILEH -DCIE -DNOLEARN $(KFLAGS) -O" "LNKFLAGS ="

# Linux 1.2 or later with gcc, dynamic libraries, ncurses, TCP/IP.
#
# If your Linux system has curses rather than ncurses, use the linuxc
# entry, or if that doesn't work, linuxnc.
#
# The Kermit "large memory model" is used by default to configure big packet
# and script buffers, etc.  For small-memory or limited-resource systems,
# "make linux KFLAGS=-DNOBIGBUF".
#
# -DLINUXFSSTND (Linux File System Standard 1.2) gives UUCP lockfile /var/lock
# with string pid.  Remove this to get /usr/spool/uucp with int pid, used in
# very early Linux versions.  FSSTND 1.2 also says that the PID string in the
# UUCP lock file has leading spaces.  This is a change from FSSTND 1.0, which
# used leading zeros.  Add -DFSSTND10 to support FSSTND 1.0 instead of 1.2.
# I hope subsequent editions of the file-system standard did not change these
# again.
#
# Add -DOLINUXHISPEED (Old Linux High Speed support) to turn on an ugly kludge
# in Linux 1.0 and earlier to support speeds of 57600 and 115200.  Extremely
# old Linux systems (pre-0.99pl15) will not support this.  If OLINUXHISPEED is
# not defined, then only the standard POSIX termios methods of setting the port
# speed will be used, and in this case speeds can be as high as 460800 in most
# modern Linux versions.
#
# -DCK_POSIX_SIG (POSIX signal handling) is good for Linux releases back to at
# least 0.99.14; if it causes trouble for you, remove it from the CFLAGS.
#
# -pipe removes the need for temp files - remove it if it causes trouble.
#
# -funsigned-char makes all characters unsigned, as they should have been
#  in the first place.
#
# Add -DCK_DSYSINI if you want a shared system-wide init file.
#
# See http://www.columbia.edu/kermit/ckubwr.html about -DNOCOTFMC.
# Better still, should read the entire Linux section of that document.
#
# The "linuxa" entry can be referenced directly on LIBC systems, but not
# GLIBC, where -lcrypt is required.  The "make linux" entry should normally
# be used for all builds on all Linux distributions unless you have special
# requirements, in which case keep reading.  CK_NEWTERM added after 7.0b04
# due to new complaints about ncurses changing buffering of tty.

linuxa:
	@echo 'Making C-Kermit $(CKVER) for Linux 1.2 or later...'
	@echo 'IMPORTANT: Read the comments in the linux section of the'
	@echo 'makefile if you have trouble.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -DLINUX -pipe -funsigned-char -DFNFLOAT -DCK_POSIX_SIG \
	-DCK_NEWTERM -DTCPSOCKET -DLINUXFSSTND -DNOCOTFMC -DPOSIX \
	-DUSE_STRERROR $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(LIBS) -lm"

# As above but with profiling
linuxp:
	$(MAKE) linuxa KTARGET=$${KTARGET:-$(@)} "KFLAGS=$(KFLAGS) -pg" \
	"LIBS=-pg -lcrypt -lresolv"

#Linux.  This entry should work for any Linux distribution on any platform,
#32-bit or 64-bit, except for extremely ancient ones.  Automatically detects:
# . curses, ncurses, or no curses
# . Old versus new pty handling (new == glibc 2.1++)
# . Presence or absence of libcrypt.a and <crypt.h>
# . Presence or absence of libresolv.a
# . Transitional Long File API for 32-bit platforms (SUS V2 UNIX 98 LFS).
#Note: The HAVE_PTMX test was previously "if test -c /dev/ptmx" but this was
#not sufficient for Debian 2.1, because although it had /dev/ptmx, it did not
#have grantpt(), unlockpt(), or ptsname(), so has been changed to look for a
#grantpt() prototype in the header files.  Modified in 8.0.206 to allow for
#libraries that contain .so's but no .a's, e.g. Mandrake 9.0.
#HAVE_BAUDBOY added in 8.0.210 for Red Hat -- it's like AIX ttylock().
#Modified 17 Aug 2005 to use openpty() if available because the other stuff
#dumps core in 64-bit ia64 and x86_64 builds.
#Long file support for 32-bit builds added in 8.0.212 - if features.h contains
#__USE_LARGEFILE64 then we set the flags that must be set before reading any
#header files; on 32-bit platforms such as i386, this produces a 32-bit build
#capable of accessing, sending, receiving, and managing long (> 2GB) files.
#On 64-bit platforms, it does no harm.
#As of 3 March 2009 we detect automatically if we have curses, ncurses,
#or no curses at all.
#Added HAVE_LOCKDEV as openSuSE >= 11.3 uses ttylock directly instead of
#baudboy 2010/08/23
#OK: 2011/06/18
linux:
	@if test \
	`grep grantpt /usr/include/*.h /usr/include/sys/*.h | wc -l` -gt 0; \
	then if test -c /dev/ptmx; then HAVE_PTMX='-DHAVE_PTMX'; \
	else HAVE_PTMX=''; fi; fi ; \
	if test `grep openpty /usr/include/pty.h | wc -l` -gt 0; \
	then HAVE_OPENPTY='-DHAVE_OPENPTY'; \
	else HAVE_OPENPTY=''; fi ; \
	HAVE_LIBCURSES=''; \
	if test -f /usr/lib64/libncurses.so || \
	   test -f /usr/lib/libncurses.a  || \
	   test -f /usr/lib/libncurses.so; then \
	  HAVE_LIBCURSES='-lncurses'; \
	else if test -f /usr/lib64/libcurses.so || \
	   test -f /usr/lib/libcurses.a || \
	   test -f /usr/lib/libcurses.so; then \
	     HAVE_LIBCURSES='-lcurses'; fi; fi; \
	HAVE_CURSES=''; \
	if test -n '$$HAVE_LIBCURSES'; then \
	  if test -f /usr/include/ncurses.h; then \
	    HAVE_CURSES='-DCK_NCURSES  -I/usr/include/ncurses'; \
	  else if test -f /usr/include/curses.h; then \
	    HAVE_CURSES='-DCK_CURSES'; \
	fi; fi; fi; \
	if test -f /usr/include/baudboy.h || test -f /usr/include/ttylock.h; \
	then HAVE_LOCKDEV='-DHAVE_LOCKDEV' ; \
	else HAVE_LOCKDEV='' ; fi ; \
	if test -f /usr/include/baudboy.h ; \
	then HAVE_BAUDBOY='-DHAVE_BAUDBOY' ; \
	else HAVE_BAUDBOY='' ; fi ; \
	$(MAKE) KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$$HAVE_CURSES $$HAVE_PTMX $$HAVE_LOCKDEV \
	$$HAVE_BAUDBOY $$HAVE_OPENPTY \
	`grep __USE_LARGEFILE64 /usr/include/features.h > /dev/null && \
	echo '-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64'` \
	`if test -f /usr/include/crypt.h; then echo -DHAVE_CRYPT_H; fi` \
	$(KFLAGS)" \
	"LIBS=$(LIBS) $$HAVE_LIBCURSES \
	`if test -n '$$HAVE_OPENPTY'; then echo -lutil; fi` \
	`if test -f /usr/lib64/libresolv.a || test -f /usr/lib64/libresolv.so \
	|| test -f /usr/lib/libresolv.a || test -f /usr/lib/libresolv.so \
	|| test -f /usr/lib/i386-linux-gnu/libresolv.a \
	|| test -f /usr/lib/i386-linux-gnu/libresolv.so \
	|| ls /lib/x86_64-linux-gnu/libresolv.* > /dev/null 2> /dev/null; \
	then echo -lresolv; fi` \
	`if test -f /usr/lib64/libcrypt.a || test -f /usr/lib64/libcrypt.so \
	|| test -f /usr/lib/libcrypt.a || test -f /usr/lib/libcrypt.so \
	|| ls /lib/x86_64-linux-gnu/libcrypt.* > /dev/null 2> /dev/null; \
	then echo -lcrypt; fi` \
	`if test -f /usr/lib64/liblockdev.a || \
	test -f /usr/lib64/liblockdev.so || \
	test -f /usr/lib/liblockdev.a || \
	test -f /usr/lib/liblockdev.so; \
	then echo -llockdev; fi`" \
	linuxa

# Linux + Shadow passwords + PAM
# OK 2011/06/18
linux+shadow+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux+Shadow+PAM...'
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DCK_SHADOW -DCK_PAM $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = -lpam"

# Linux systems that have no <sys/select.h>.
# (not tested in recent years, perhaps no longer needed)
linuxns:
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} KFLAGS=-DNO_SYS_SELECT_H

# Linux-script-only:
# A minimum-size version for Linux that does only scripting and
# serial communication -- no networks, no file transfer, no security.
# OK 2011/06/18
linuxso:
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -DLINUX -pipe -funsigned-char -DPOSIX -DCK_POSIX_SIG \
	-DLINUXFSSTND -DNOCOTFMC -DNOXFER -DNODEBUG -DNOCSETS -DNOHELP \
	-DNONET -DMINIDIAL -DNOSCRIPT -DNOIKSD -DNOPUSH $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" "LIBS = "

# Secure targets for Linux.  These work on RHAS4, RHEL4, and RHEL5,
# unlike some of the older targets that follow.  They hook into the main Linux
# target so we pick up all the other new stuff - large files, baudboy.h, the
# appropriate pty interface, etc.

# Linux with Kerberos 5.
# Use "make linux+krb5 KFLAGS=-DNO_KRB5_INIT_ETS" if necessary.
#OK 2011/06/16 on Fedora 14 with:
# make linux+krb5 "LIBS=$LIBS /lib/libk5crypto.so.3 /lib/libcom_err.so.2"
# On RHEL5: make linux+krb5 -UCK_DES
linux+krb5:
	@echo 'Making C-Kermit $(CKVER) for Linux with Kerberos 5...'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls /usr/lib/libdes* > /dev/null 2> /dev/null || \
	   ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes425'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
              echo "HAVE DES"; \
           else echo "NO DES"; \
	fi; \
	K5CRYPTO=''; \
        if ls /lib/libk5crypto* > /dev/null 2> /dev/null; then \
                K5CRYPTO='-lk5crypto'; \
	else if ls /usr/lib/libk5crypto* > /dev/null 2> /dev/null; then \
		K5CRYPTO='-lk5crypto'; \
        else if ls /usr/lib64/libk5crypto* > /dev/null 2> /dev/null; then \
                K5CRYPTO='-lk5crypto'; \
        fi; fi; fi; \
	COM_ERR=''; \
	if ls /lib/libcom_err* > /dev/null 2> /dev/null; then \
		COM_ERR='-lcom_err'; \
	fi; \
	GSSAPILIB='-lgssapi'; \
	if ls /lib/libgssapi_krb5* > /dev/null 2> /dev/null; then \
		GSSAPILIB='-lgssapi_krb5'; \
	else if ls /usr/lib/libgssapi_krb5* > /dev/null 2> /dev/null; then \
		GSSAPILIB='-lgssapi_krb5'; \
	else K5DIR=`echo $(K5LIB) | sed 's|-L||'`; \
		if ls $$K5DIR/libgssapi_krb5* > /dev/null 2> /dev/null; then \
			GSSAPILIB='-lgssapi_krb5'; \
	fi; fi; fi; \
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"KFLAGS= -DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 $$OPENSSLOPTION \
	-DCK_ENCRYPTION $$HAVE_DES $(K5INC) $(K5INC)/krb5 \
	-I/usr/include/et $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $$DES_LIB -lcrypto $$GSSAPILIB -lkrb5 \
	$$K5CRYPTO $$COM_ERR $(LIBS)" ; \
	if [ ! -f ./wermit ] || [ ./ckcmai.o -nt ./wermit ] ; then \
		echo ""; \
		echo "If build failed try:"; \
		echo ""; \
		echo "  make clean ; make $${KTARGET:-$(@)} KFLAGS=-UCK_DES"; \
		echo ""; \
	fi

# Linux with Kerberos 5 and Kerberos 4.
# Use "make linux+krb5 KFLAGS=-DNO_KRB5_INIT_ETS" if necessary.
# Add "KFLAGS=-UCK_DES" if failure messages look DES-related.
# UNTESTED (because I can't find a box with Krb4 and Krb5 installed)
linux+krb5+krb4:
	@echo 'Making C-Kermit for Linux with Kerberos 4 and Kerberos 5'
	$(MAKE) linux+krb5 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DKRB4 -DKRB524 $(KFLAGS)" "LIBS=$(LIBS) -lkrb4"

# Linux with OpenSSL 
# In Linux, SSL libs are often in /lib or /usr/lib and so found by default.
# This targets takes into account the DES library might or might not
# exist.  If it does exist, however, the target will require some editing
# if its basename is not libdes425.  - fdc Tue Sep 21 14:28:00 2010
# IMPORTANT: Some Linux platforms have DES libraries but they are missing
# functions used by Kermit.  In that case you will get fatal errors at
# link time involving routines such as des_ecb3_encrypt, des_random_seed,
# and des_set_odd_parity.  In that case, "make linux KFLAGS=-UCK_DES"
# There's a new warning at the end that should come out if this happens,
# and that should not come out if it didn't.
#
linux+ssl linux+openssl linux+openssl+zlib+shadow+pam linux+openssl+shadow:
	@echo 'Making C-Kermit $(CKVER) for Linux+OpenSSL SSLLIB=$(SSLLIB)'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls /usr/lib/libdes* > /dev/null 2> /dev/null || \
	   ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes425'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
	      echo "HAVE DES"; \
	   else echo "NO DES"; \
	fi; \
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"KFLAGS= -DCK_AUTHENTICATION -DCK_ENCRYPTION -DCK_CAST $$HAVE_DES \
	-DCK_SSL -DCK_PAM -DZLIB -DCK_SHADOW $$OPENSSLOPTION $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SSLLIB) -lssl $$DES_LIB -lcrypto -lpam -ldl -lz $(LIBS)" ; \
	if [ ! -f ./wermit ] || [ ./ckcmai.o -nt ./wermit ] ; then \
		echo ""; \
		echo "If build failed try:"; \
		echo ""; \
		echo "  make clean ; make $${KTARGET:-$(@)} KFLAGS=-UCK_DES"; \
		echo ""; \
	fi

# Linux with Kerberos 5 and OpenSSL
# OK 2011/05/16
# Add -UCK_DES if functions like des_ecb3_encrypt, es_random_seed,
# come up missing at link time.
linux+krb5+ssl linux+krb5+openssl:
	@echo 'Making C-Kermit $(CKVER) for Linux with Krb5 and OpenSSL...'
	@case `openssl version` in \
	  *0.9.7*) OPENSSLOPTION="-DOPENSSL_097" ;; \
	  *0.9.8*) OPENSSLOPTION="-DOPENSSL_098" ;; \
	  *1.[0-9].[0-9]*) OPENSSLOPTION="-DOPENSSL_100" ;; \
	  *) OPENSSLOPTION="" ;; \
	esac; \
	HAVE_DES=''; \
	DES_LIB=''; \
	if ls /usr/lib/libdes* > /dev/null 2> /dev/null || \
	   ls $(SSLLIB)/libdes* > /dev/null 2> /dev/null; then \
	      DES_LIB='-ldes425'; \
	      HAVE_DES='-DCK_DES -DLIBDES'; \
	      echo "HAVE DES"; \
	   else echo "NO DES"; \
	fi; \
	K5CRYPTO=''; \
        if ls /lib/libk5crypto* > /dev/null 2> /dev/null; then \
                K5CRYPTO='-lk5crypto'; \
	else if ls /usr/lib/libk5crypto* > /dev/null 2> /dev/null; then \
		K5CRYPTO='-lk5crypto'; \
        else if ls /usr/lib64/libk5crypto* > /dev/null 2> /dev/null; then \
                K5CRYPTO='-lk5crypto'; \
	fi; fi; fi; \
	COM_ERR=''; \
	if ls /lib/libcom_err* > /dev/null 2> /dev/null; then \
		COM_ERR='-lcom_err'; \
	fi; \
	GSSAPILIB='-lgssapi'; \
	if ls /lib/libgssapi_krb5* > /dev/null 2> /dev/null; then \
		GSSAPILIB='-lgssapi_krb5'; \
	else if ls /usr/lib/libgssapi_krb5* > /dev/null 2> /dev/null; then \
		GSSAPILIB='-lgssapi_krb5'; \
	else K5DIR=`echo $(K5LIB) | sed 's|-L||'`; \
		if ls $$K5DIR/libgssapi_krb5* > /dev/null 2> /dev/null; then \
			GSSAPILIB='-lgssapi_krb5'; \
	fi; fi; fi; \
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"KFLAGS= -DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 \
	-DCK_SSL -DCK_PAM -DZLIB -DCK_SHADOW $$OPENSSLOPTION $(SSLINC) \
	-DCK_ENCRYPTION $$HAVE_DES $(K5INC) $(K5INC)/krb5 \
	-I/usr/include/et $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SSLLIB) -lssl $$DES_LIB -lpam -lz \
	-lcrypto $$GSSAPILIB -lkrb5 $$K5CRYPTO $$COM_ERR $(LIBS)" ; \
	if [ ! -f ./wermit ] || [ ./ckcmai.o -nt ./wermit ] ; then \
		echo ""; \
		echo "If build failed try:"; \
		echo ""; \
		echo "  make clean ; make $${KTARGET:-$(@)} KFLAGS=-UCK_DES"; \
		echo ""; \
	fi

# ::BEGIN_OLD_LINUX_TARGETS::

# The remaining Linux entries are for special or customized builds.  They have
# not been generalized ("subroutinized") like the ones above.  Ideally, we
# should allow for every combination of libc vs glibc, gcc vs egcs, curses vs
# ncurses, Kerberos IV vs Kerberos V vs SRP (in any combination), and so on.
# The best way to do this is to set KFLAGS and LIBS values and then chain to
# the main "linux" target, as in the examples just above.  To skip past all of
# these old targets (and there are many) search for ::END_OLD_LINUX_TARGETS::
# (after this line).

#Sharp Zaurus SL-5500 - Linux based
zsl5500:
	@echo 'Making C-Kermit $(CKVER) for Sharp Zaurus SL-5500...'
	@touch ckcpro.c
	@touch wart
	$(MAKE) linuxnc KTARGET=$${KTARGET:-$(@)} "KFLAGS=-DZSL5500" \
	"CC = gcc" "CC2 = gcc"

#Mklinux DR3 has horrible bug in <utmpbits.h> - see ckufio.c.
mklinux:
	$(MAKE) KTARGET=$${KTARGET:-$(@)} "KFLAGS=-DUTMPBUG" \
	"LIBS=-lcrypt -lresolv" linuxa

#LinuxPPC 1999
linuxppc:
	@echo 'Making C-Kermit $(CKVER) for LinuxPPC 1999...'
	@if test -f /usr/lib/libcrypt.a; then \
	    if test -f /usr/lib/libresolv.a; then \
	        $(MAKE) KTARGET=$${KTARGET:-$(@)} \
		"KFLAGS=$(NCURSES_CPP) -DHAVE_CRYPT_H \
		-DLOCK_DIR=\\\\\\"\"/var/lock/modem\\\\\\"\" $(KFLAGS)" \
	        "LIBS=-lncurses -lresolv -lcrypt" linuxa ; \
	    else \
	        $(MAKE) KTARGET=$${KTARGET:-$(@)} \
		"KFLAGS=$(NCURSES_CPP) -DHAVE_CRYPT_H \
		-DLOCK_DIR=\\\\\\"\"/var/lock/modem\\\\\\"\" $(KFLAGS)" \
	        "LIBS=-lncurses -lcrypt" linuxa ; \
	    fi \
	else \
	    if test -f /usr/lib/libresolv.a; then \
	        $(MAKE) KTARGET=$${KTARGET:-$(@)} \
		"KFLAGS=$(NCURSES_CPP) \
		-DLOCK_DIR=\\\\\\"\"/var/lock/modem\\\\\\"\" $(KFLAGS)" \
	        "LIBS=-lncurses -lresolv" linuxa ; \
	    else \
	        $(MAKE) KTARGET=$${KTARGET:-$(@)} \
		"KFLAGS=$(NCURSES_CPP) \
		-DLOCK_DIR=\\\\\\"\"/var/lock/modem\\\\\\"\" $(KFLAGS)" \
	        "LIBS=-lncurses" linuxa ; \
	    fi \
	fi


# Like "make linux" but built with egcs rather than gcc.
# If you get "Internal compiler error xxx, output pipe has been closed",
# try removing -pipe.
linuxegcs:
	@echo 'Making C-Kermit $(CKVER) for Linux 1.2 or later with egcs...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = egcs" "CC2 = egcs" \
	"CFLAGS = -O -DLINUX -pipe -funsigned-char \
	-DPOSIX -DCK_POSIX_SIG -DCK_NCURSES -DNOCOTFMC \
	-DTCPSOCKET -DLINUXFSSTND $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" "LIBS = -lncurses -lcrypt -lresolv"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.1 (no K4 compatibility).
linux+krb5-old:
	@echo 'Making C-Kermit $(CKVER) for Linux on Intel with Kerberos...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_KERBEROS -DKRB5 \
	-DCK_ENCRYPTION -DCK_DES -DCK_CURSES -DCK_POSIX_SIG \
	-DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H $(K5INC) $(K5INC)/krb5 \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) -lncurses -ltermcap -ldes425 -lkrb5 \
	-lcom_err -lk5crypto -lgssapi_krb5 -lcrypt -lresolv"

# Linux on Intel PC with SRP 1.7.4 using GNU MP, Krypto, and Eric Young's
# DES library.  Remove the -DCK_DES, -DLIBDES and -ldes if you do not have
# Eric Young's# libdes.a installed.
#
linux+srp+gmp:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with SRP...'
	$(MAKE) srpmit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(SRPINC) $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) \
	-lncurses -ltermcap -lsrp -lgmp -ldes -lkrypto -lcrypt -lresolv"

linux+srp+gmp+no-des:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with SRP ...'
	$(MAKE) srpmit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP \
	-DCK_ENCRYPTION -DCK_CAST \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(SRPINC) $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) \
	-lncurses -ltermcap -lsrp -lgmp -lkrypto -lcrypt -lresolv"

linux+srp+gmp-export:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with SRP...'
	$(MAKE) srpmit-export KTARGET=$${KTARGET:-$(@)} \
	"CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DFNFLOAT \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(SRPINC) $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) \
	-lncurses -ltermcap -lsrp -lgmp -lkrypto -lcrypt -lm -lresolv"

linux+srp+gmp+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with SRP...'
	$(MAKE) srpmit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	-DCK_PAM -DFNFLOAT $(SRPINC) $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) -lncurses -ltermcap -lsrp -lgmp -ldes -lkrypto \
	-lcrypt -lpam -ldl -lm -lresolv"

#Linux on Intel PC with SRP 1.7.4 built with OpenSSL for Big Number Math
#and Cryptographic functionality.
#
linux+srp:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with SRP...'
	$(MAKE) srpmit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(SRPINC) $(SSLINC) $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(SSLLIB) \
	-lncurses -ltermcap -lsrp -lkrypto -lcrypto -lcrypt -lresolv"

linux+srp+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with SRP...'
	$(MAKE) srpmit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	-DCK_PAM -DFNFLOAT $(SRPINC) $(SSLINC) $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(SSLLIB) -lncurses -ltermcap -lsrp -lkrypto \
	-lcrypto -lcrypt -lpam -ldl -lm -lresolv"

#Linux on Intel PC with SRP and SSL/TLS.
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 or higher to be compiled with KRB4 compatibility.
#Remove -ltermcap if it causes trouble e.g. in Debian 2.2.
#If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+srp+openssl:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SRP,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(SRPINC) $(SSLINC) $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(SSLLIB) \
	-lncurses -ltermcap -lsrp -lssl -lkrypto -lcrypto \
	-lcrypt -lresolv"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2 and SRP.
#
# libsrp.a should be build with GNU MP (libgmp.a)
# instead of AT&T CryptoLib (libcrypt.a) due to naming conflicts with
# standard distribution Linux libraries.
# Requires the Kerberos 1.2.2 or higher to be compiled with KRB4 compatibility.
linux+krb5+krb4+srp:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB54+SRP...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SRPINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SRPLIB) \
	-lncurses -ltermcap -lsrp -lgmp -lgssapi_krb5 -lkrypto \
	-ldes -lkrb4 -ldes425 -lkrb5 -lcom_err -lk5crypto -lcrypt -lresolv"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2, SRP and SSL/TLS.
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 or higher to be compiled with KRB4 compatibility.
# Requires OpenSSL 0.9.6a or higher
#If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+srp+openssl:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SRP,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SRPINC) $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SRPLIB) $(SSLLIB) \
	-lncurses -ltermcap -lsrp \
	-lkrb4 -lssl -lkrypto -lcrypto \
	-lkrb5 -lcom_err -lk5crypto -lgssapi_krb5 -lcrypt -lresolv"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2, SSL/TLS.
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
#If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+openssl:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SSLLIB) \
	-lncurses -ltermcap \
	-lkrb4 -lssl -lcrypto -lkrb5 -lcom_err \
	-lk5crypto -lgssapi_krb5 -lcrypt -lresolv"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.1, SSL/TLS.
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
# If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+openssl+shadow:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_SHADOW \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SSLLIB)  \
	-lncurses -ltermcap \
	-lkrb4 -lssl -lcrypto -lkrb5 -lcom_err \
	-lk5crypto -lgssapi_krb5 -lcrypt -lresolv"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2, SSL/TLS.
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
# If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+openssl+zlib+shadow:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 -DZLIB \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_SHADOW \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SSLLIB) \
	-lncurses -ltermcap \
	-lkrb4 -lssl -lcrypto -lkrb5 -lcom_err \
	-lk5crypto -lgssapi_krb5 -lcrypt -lresolv -lz"

linux+krb5+krb4+srp-export:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with SRP...'
	$(MAKE) xermit-export KTARGET=$${KTARGET:-$(@)} \
	"CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SRPINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(K5LIB) \
	-lncurses -ltermcap -lsrp -lgmp -lkrb4 -ldes425 -lkrb5 -lgssapi_krb5 \
	-lcom_err -lk5crypto -lkrypto -lcrypt -lresolv"

linux+krb5+krb4+srp+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with SRP...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	-DCK_PAM $(K5INC) $(K5INC)/krb5 $(SRPINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(K5LIB) \
	-lncurses -ltermcap -lsrp -lgmp -ldes -lkrb4 -ldes425 -lkrb5 \
	-lcom_err -lk5crypto -lgssapi_krb5 -lkrypto -lcrypt -lpam -ldl \
	-lresolv"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2, SRP and SSL/TLS.
# and PAM.
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
# If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+srp+openssl+pam-debug:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SRP,SSL...'
	$(MAKE) xermit-debug KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -g -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_PAM \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	-w -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG $(K5INC) $(K5INC)/krb5 \
	$(SRPINC) $(SSLINC) $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(K5LIB) $(SSLLIB) \
	-lncurses -ltermcap -lsrp -lkrb4 -lssl -lkrypto -lcrypto \
	-lkrb5 -lcom_err -lk5crypto -lgssapi_krb5 -lcrypt -lresolv -lpam -ldl"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.1, SRP and SSL/TLS.
# and PAM.
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
# If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+srp+openssl+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SRP,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -g -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_PAM \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SRPINC) $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(K5LIB) $(SSLLIB)  \
	-lm -lncurses -ltermcap -lsrp \
	-lkrb4 -lssl -lkrypto  -lcrypto -lgssapi_krb5 \
	-lkrb5 -lcom_err -lk5crypto -lcrypt -lresolv -lpam -ldl"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2, SRP, OpenSSL
# with ZLIB and PAM
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
# If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+srp+openssl+zlib+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SRP,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -g -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_PAM -DZLIB \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SRPINC) $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(K5LIB) $(SSLLIB) \
	-lm -lncurses -ltermcap -lsrp \
	-lkrb4 -lssl -lkrypto  -lcrypto -lgssapi_krb5 \
	-lkrb5 -lcom_err -lk5crypto -lcrypt -lresolv -lpam -ldl -lz"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2, SRP, OpenSSL
# with ZLIB, Shadow Passwords, and PAM
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
# If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+srp+openssl+zlib+shadow+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SRP,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -g -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_SRP -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_PAM -DZLIB \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	-DCK_SHADOW $(K5INC) $(K5INC)/krb5 $(SRPINC) $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(SRPLIB) $(K5LIB) $(SSLLIB) \
	-lm -lncurses -ltermcap -lsrp -lkrypto \
	-lkrb4 -lssl -lcrypto -lgssapi_krb5 \
	-lkrb5 -lcom_err -lk5crypto -lcrypt -lresolv -lpam -ldl -lz"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2, OpenSSL
# with Shadow Passwords, PAM
#
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
linux+krb5+krb4+openssl+shadow+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SSL,...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -g -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_PAM \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	-DCK_SHADOW $(K5INC) $(K5INC)/krb5 $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SSLLIB) \
	-lm -lncurses -ltermcap \
	-lkrb4 -lssl -lcrypto -lgssapi_krb5 \
	-lkrb5 -lcom_err -lk5crypto -lcrypt -lresolv -lpam -ldl"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2, OpenSSL
# with ZLIB, Shadow Passwords, PAM
#
# libsrp.a should be build with OpenSSL
# Requires the Kerberos 1.2.2 be compiled with KRB4 compatibility.
# If you have OpenSSL 0.9.7 or later, add -DOPENSSL_097 to KFLAGS.
linux+krb5+krb4+openssl+zlib+shadow+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB,SRP,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -g -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB5 -DKRB4 -DKRB524 \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_PAM -DZLIB \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	-DCK_SHADOW $(K5INC) $(K5INC)/krb5 $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SSLLIB) \
	-lm -lncurses -ltermcap \
	-lkrb4 -lssl -lcrypto -lgssapi_krb5 \
	-lkrb5 -lcom_err -lk5crypto -lcrypt -lresolv -lpam -ldl -lz"

#Red Hat 9 - full install includes Kerberos 5 (4 compat), PAM, SSL.
#Also works around bug in curses in which terminal goes dead after
#returning from file-transfer display.  Assumes OpenSSL 0.9.7 or later.
redhat9:
	@echo "Building SECURE Kermit for Red Hat 9.0..."
	$(MAKE) linux+krb5+krb4+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH90 -DOPENSSL_097 $(KFLAGS)"

#Ditto plus SRP (which is not normally included with RH Linux).
redhat9+srp:
	@echo "Building SECURE Kermit for Red Hat 9.0..."
	$(MAKE) linux+krb5+krb4+srp+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH90 -DOPENSSL_097 $(KFLAGS)"

#For Red Hat AS 2.1 with OpenSSL
redhat21+ssl:
	@echo "Building SECURE Kermit for Red Hat 2.1..."
	$(MAKE) linux+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS =  $(KFLAGS)"

#Red Hat Linux 8.0 - full install includes Kerberos 5 (4 compat), PAM, SSL.
#Also works around bug in curses in which terminal goes dead after
#returning from file-transfer display.
redhat80:
	@echo "Building SECURE Kermit for Red Hat 8.0..."
	$(MAKE) linux+krb5+krb4+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH80 $(KFLAGS)"

redhat80+srp:
	@echo "Building SECURE Kermit for Red Hat 8.0..."
	$(MAKE) linux+krb5+krb4+srp+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH80 $(KFLAGS)"

#Red Hat Linux 7.3 - full install includes Kerberos 5 (4 compat), PAM, SSL.
#Also works around bug in curses in which terminal goes dead after
#returning from file-transfer display.
redhat73:
	@echo "Building SECURE Kermit for Red Hat 7.3..."
	$(MAKE) linux+krb5+krb4+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH73 $(KFLAGS)"

redhat73+srp:
	@echo "Building SECURE Kermit for Red Hat 7.3..."
	$(MAKE) linux+krb5+krb4+srp+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH73 $(KFLAGS)"

#Red Hat Linux 7.2 - full install includes Kerberos 5 (4 compat), PAM, SSL.
#Also works around bug in curses in which terminal goes dead after
#returning from file-transfer display.
redhat72:
	@echo "Building SECURE Kermit for Red Hat 7.2..."
	$(MAKE) linux+krb5+krb4+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH72 $(KFLAGS)"

redhat72+srp:
	@echo "Building SECURE Kermit for Red Hat 7.2..."
	$(MAKE) linux+krb5+krb4+srp+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH72 $(KFLAGS)"

#Red Hat Linux 7.1 - full install includes Kerberos 5 (4 compat), PAM, SSL.
#Also works around bug in curses in which terminal goes dead after
#returning from file-transfer display.
redhat71:
	@echo "Building SECURE Kermit for Red Hat 7.1..."
	$(MAKE) linux+krb5+krb4+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH71 $(KFLAGS)"

redhat71+srp:
	@echo "Building SECURE Kermit for Red Hat 7.1..."
	$(MAKE) linux+krb5+krb4+srp+openssl+zlib+shadow+pam \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DRH71 $(KFLAGS)"

#Linux on Intel PC with Cygnus or MIT Kerberos 5 1.2.2, OpenSSL
# with ZLIB and PAM and Shadow passwords
linux+krb5+openssl+zlib+shadow+pam:
	@echo 'Making C-Kermit $(CKVER) for Linux on i386 with KRB5,SSL...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -g -O -funsigned-char -pipe -DPOSIX -DLINUX -DNOCOTFMC \
	-DCK_AUTHENTICATION -DCK_KERBEROS  -DKRB5 -DCK_SHADOW -DHAVE_PTMX \
	-DCK_ENCRYPTION -DCK_CAST -DCK_DES -DLIBDES -DCK_SSL -DCK_PAM -DZLIB \
	-DCK_CURSES -DCK_POSIX_SIG -DTCPSOCKET -DLINUXFSSTND -DHAVE_CRYPT_H \
	$(K5INC) $(K5INC)/krb5 $(SSLINC) \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(K5LIB) $(SSLLIB) \
	-lm -lncurses -ltermcap -lssl -lcrypto -lgssapi_krb5 \
	-lkrb5 -lcom_err -lk5crypto -lcrypt -lresolv -lpam -ldl -lz"

linuxnotcp:
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} "KFLAGS = -DNONET $(KFLAGS)"

# "make linuxnotcp" with lcc (see http://www.cs.princeton.edu/software/lcc)
# lcc does not understand various gcc extensions:
#  "__inline__" -- can be eliminated by adding "-D__inline__="
#  "__asm__ and "long long" -- in header files, should be surrounded by
#                              "#ifndef(__STRICT_ANSI__)"/"#endif"
#  however, TCP requires some __asm__ functions, so cannot be compiled
linuxnotcp-lcc:
	@echo 'Making C-Kermit $(CKVER) for Linux with lcc ...'
	@echo 'Read comments in makefile for additional information.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CC = lcc" "CC2 = lcc" \
	"CFLAGS = -DLINUX -DPOSIX -DCK_CURSES -DCK_POSIX_SIG \
	-UTCPSOCKET -DLINUXFSSTND -DNOLEARN $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" "LIBS = -lcurses -ltermcap"

# Linux 0.99.14 thru 1.0 with gcc, dynamic libraries, curses, TCP/IP.
# For Linux 1.2 or later, use "make linux" (above).
#
# -DLINUXFSSTND (Linux File System Standard) gives UUCP lockfile /var/lock with
# string pid.  Remove this and get /usr/spool/uucp with int pid, which was used
# in early Linux versions.
#
# If you get compiler errors regarding <linux/serial.h>, add -DNOHISPEED.
#
# -DCK_POSIX_SIG (POSIX signal handling) is good for Linux releases back to at
# least 0.99.14; if it causes trouble for you, just remove it.
#
# -DCK_CURSES: Here we link with the regular curses library.  But you should
# be using ncurses.  Internally, the ckuusx.c module includes <curses.h>, but
# this really should be <ncurses.h>.  Thus if you have the new curses
# material, you should either install it with the standard names, or else
# create symbolic links from the standard names to the new ones.  If you get
# compile-time errors complaining about data definitions in termcap.h, it
# means you have new kernel material mixed with older libc header files.  To
# fix, add "#include <termios.h>" to the <termcap.h> file.  Or if all this is
# too confusing, create a new makefile entry based on this one, but with
# -DCK_CURSES removed from CFLAGS and the entire LIBS= clause removed.
#
# But wait, there's more.  On most Linux systems, -ltermcap must be included
# in LIBS.  But on others, the linker complains that libtermcap can't be
# found.  In that case, try removing -ltermcap from LIBS=.
#
# But wait, there's more.  The format of the PID string in the UUCP lockfile
# changed between Linux FSSTND 1.0 and 1.2.  In the earlier standard, it had
# leading zeros; in the second, it has leading spaces.  By default this entry
# uses the newer standard.  To force the older one, add -DFSSTND10.
#
# "The nice thing about the Linux standard is there are so many to choose from"
#
# NOTE: Remove -DBIGBUFOK for small-memory or limited-resource systems.
linux10:
	@echo 'Making C-Kermit $(CKVER) for Linux 1.0 or earlier...'
	@echo 'IMPORTANT: Read the comments in the linux section of the'
	@echo 'makefile if you get compilation or link errors.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -DPOSIX -DCK_CURSES -DCK_POSIX_SIG -DLINUX \
	-DTCPSOCKET -DLINUXFSSTND -DOLINUXHISPEED -DNOLEARN $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" "LIBS = -lcurses -ltermcap"

#This version was used for Linux prior to C-Kermit 6.0.192.
#Now the "Linux File System Standard" is considered standard, ditto TCP/IP.
linuxold:
	@echo 'Making C-Kermit $(CKVER) for Linux...'
	@echo 'For FSSTND-recommended UUCP lockfiles, use:'
	@echo '  make linux "KFLAGS=-DLINUXFSSTND".'
	@echo 'Read comments in makefile for additional options.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -DLINUX -DPOSIX -DCK_CURSES -DCK_POSIX_SIG -DNOLEARN \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" "LIBS = -lcurses -ltermcap"

# ::END_OLD_LINUX_TARGETS::

# LynxOS 2.2 with GCC compiler, TCP/IP and fullscreen display.
# Probably also works with Lynx 2.1, and maybe even Lynx 2.0.
# -X means use termios serial drivers rather than BSD4.3-style sgtty drivers.
# If you have trouble with this, try "make bsd KFLAGS=-DNOFDZERO".
lynx:
	@echo 'Making C-Kermit $(CKVER) for LynxOS 2.2 with TCP/IP'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -O -DPOSIX -DDIRENT -DSETREUID -DCK_CURSES -DTCPSOCKET \
	-DCK_ANSIC -DLYNXOS -DNOLEARN" "LNKFLAGS = -X" "LIBS = -lcurses -lbsd"

lynx22:
	$(MAKE) lynx KTARGET=$${KTARGET:-$(@)} "KFLAGS=$(KFLAGS)"

# LynxOS 2.1 with GCC compiler 1.40 and TCP/IP.
lynx21:
	@echo 'Making C-Kermit $(CKVER) for LynxOS 2.1 with TCP/IP'
	$(MAKE) kermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -O -DSETREUID -DTCPSOCKET -DCK_ANSIC -DBSD4 -DLYNXOS" \
	"LIBS = -lbsd"

#SCO Xenix 2.2.1 for IBM PC, XT, PS2/30, or other 8088 or 8086 machine
#Should this not work, try some of the tricks from sco286.
#NOTE: -DRENAME is omitted for early SCO Xenix releases because it didn't
#exist, or its semantics were different from the later POSIX-compliant
#version of rename().
sco86:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/86...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DXENIX -DNOFILEH -DNOIKSD -DNOUNICODE -DNOLEARN \
	$(KFLAGS) -Dunix -F 3000 -i -M0me" \
	"LNKFLAGS = -F 3000 -i -s -M0me" "LIBS = -lx"

#SCO Xenix/286 2.2.1, e.g. for IBM PC/AT, PS/2 Model 50, etc.
#Reportedly, this "make" can fail simply because of the size of this
#makefile.  If that happens, use "makeL", or edit out some of the
#other entries.  No debugging or character-set translation.
sco286:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/286...'
	@echo 'If make fails, try using makeL.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -xenix -s -O -LARGE -DXENIX -DNOFILEH -Dunix -DRDCHK -DNAP \
	-DNOIKSD -DNODEBUG -DNOTLOG -DNOCSETS -DNOLEARN \
	$(KFLAGS) -F 3000 -i -M2let16" \
	"LIBS = -lx" "LNKFLAGS = -xenix -s -O -LARGE -F 3000 -i -M2let16"

#SCO Xenix/286 2.2.1, e.g. for IBM PC/AT, PS/2 Model 50, etc.
#As above, but with HDBUUCP (This one might need fixing -- see sco286).
sco286hdb:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/286 with HDB UUCP...'
	@echo 'If make fails, try using makeL.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -s -O -LARGE -DXENIX -DNOFILEH -Dunix -DRDCHK -DNAP \
	-DHDBUUCP -DNOIKSD -DNOUNICODE -DNOLEARN \
	$(KFLAGS) -F 3000 -i -M2let32" \
	"LIBS = -lx" "LNKFLAGS = -s -O -LARGE -F 3000 -i -M2let32"

#SCO Xenix/386 2.2.2 and 2.2.3
sco386:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/386 2.2.2...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DXENIX -DNOFILEH -DNOIKSD -DNOREDIRECT -DNOLEARN \
	-Dunix -DRDCHK -DNAP -DNOUNICODE $(KFLAGS) -Otcl -M3e" \
	"LNKFLAGS = -s" "LIBS = -lx"

#SCO XENIX/386 2.2.3 with Excelan TCP/IP + curses.
# NOTE: This one might need some work in C-Kermit 6.0.
# You might need to include /usr/include/sys/types.h
# containing "typedef char *caddr_t;".  Then at least it compiles.
sco386netc:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/386 2.2.3 + Excelan TCP'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -I/usr/include/exos -DXENIX -DCK_CURSES -DNOUNICODE \
	-Dunix -DRDCHK -DNAP -DTCPSOCKET -DEXCELAN -DNOJC -DNOMKDIR -DNOFILEH \
	-DNOLEARN -DNOREDIRECT -DNOIKSD -DNO_DNS_SRV $(KFLAGS) -Otcl -M3e" \
	"LNKFLAGS = -s" "LIBS = -lc -lx -lsocket -lcurses -ltermcap"

#SCO XENIX/386 2.3.3 with gcc 1.37 or later...
sco386gcc:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/386 2.3.3, gcc...'
	@echo 'Add -D_NO_PROTOTYPE if you have trouble with Xenix header files'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -O -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DNOJC -DNODEBUG -DNOUNICODE -DNOLEARN $(KFLAGS) \
	-traditional -fpcc-struct-return -fstrength-reduce \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_WORDSWAP -DM_XENIX -DNOIKSD -DNOREDIRECT \
	-DPWID_T=int " "LNKFLAGS = -s" "LIBS = -lx"

#As above, but with curses...
sco386gccc:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/386 2.3.3, gcc...'
	@echo 'Add -D_NO_PROTOTYPE if you have trouble with Xenix header files'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -O -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DNOJC -DNODEBUG -DCK_CURSES -DNOUNICODE -DNOLEARN $(KFLAGS) \
	-traditional -fpcc-struct-return -fstrength-reduce \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM -DNOREDIRECT \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_WORDSWAP -DM_XENIX -DNOIKSD \
	-DPWID_T=int " "LNKFLAGS = -s" "LIBS = -lx -lcurses -ltermlib"

#SCO UNIX (and ODT) entries...
#
#NOTE: All SCO UNIX entry LIBS should have "-lc_s -lc -lx" IN THAT ORDER (if
#shared C library is desired), or else "-lc -lx" IN THAT ORDER.  Use shared C
#libraries to save memory, but then don't expect to run the resulting binary
#on a different machine.  When using -lc_s, you must also use -lc, because the
#shared C library does not contain all of libc.a.  And in all cases, -lc must
#ALWAYS precede -lx.
#
#ANOTHER NOTE: -DRENAME is included in all SCO UNIX entries.  Remove it if it
#causes trouble.  No harm is done by removing it (see ckuins.txt).
#
#AND ANOTHER: In theory, it should be possible to run SCO UNIX binaries on
#SCO Xenix 2.3 and later.  In practice, this might not work because of the
#libraries, etc.  Also, don't add the -link -z switch (which is supposed to
#root out references to null pointers) because it makes UNIX binaries core
#dump when they are run under Xenix.

#NOTE: -Otcl removed and replaced by -O, since -Otcl produced incorrect code.
#SCO UNIX/386 3.2.0, 3.2.1, and SCO Xenix 2.3.x
sco3r2:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2.0 or 3.2.1 ...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following three continued lines into one line.'
	@echo 'Also, remove -DRENAME if _rename unresolved at link time.'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP -DNOLEARN \
	-DRENAME -DNOIKSD -DNOJC $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lc -lx"

#SCO UNIX/386 3.2.0 and SCO Xenix 2.3.x with Excelan TCP/IP support.
#In case of compilation or runtime problems, try adding
#"-DUID_T=int -DGID_T=int" to the CFLAGS.  If that doesn't work, try
#"-DUID_T=uid_t -DGID_T=gid_t".
sco3r2net:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 / Excelan...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -I/usr/include/exos -DXENIX -DSVR3 -DNOFILEH -DNOLEARN \
	-DHDBUUCP -DRDCHK -DNAP -DRENAME -DTCPSOCKET -DEXCELAN -DNOJC \
	-DNOIKSD -DNOREDIRECT $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lc -lx -lsocket"

#SCO UNIX/386 3.2.0 and SCO Xenix 2.3.x with Excelan TCP/IP support.
#As above, with curses added.
sco3r2netc:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 / Excelan / curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -I/usr/include/exos -DXENIX -DSVR3 -DNOFILEH -DNOLEARN \
	-DHDBUUCP -DRDCHK -DNAP -DTCPSOCKET -DEXCELAN -DNOJC $(KFLAGS) \
	-DRENAME -DCK_CURSES -DNOREDIRECT -DNOIKSD -O" "LNKFLAGS = -s" \
	"LIBS = -lc -lx -lsocket -lcurses -ltermcap"

#SCO UNIX 3.2.x or SCO Xenix 2.3.x with Racal InterLan TCP/IP support
# Extra compile flags for other version of Racal InterLan TCP/IP:
# Xenix286/NP621-286, use -Ml -DPARAMH -DINTERLAN -Di286 -DSYSV
# Xenix386/NP621-386, use -DPARAMH -DINTERLAN -Di386 -DSYSV
# ISC386ix/NP622I, use -DSYSV -Di386
# SCO Unix3.2/NP622S, use -DSYSV -Di386 -DSCO_UNIX
# AT&T SVR3.2/NP622A, use -DSYSV -Di386 -DATT
sco3r2netri:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 / Racal InterLan...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -I/usr/include/interlan -DXENIX -DNOFILEH -DHDBUUCP \
	-DSVR3 -DRDCHK -DNAP -DTCPSOCKET -DPARAMH -DINTERLAN -Di386 -DSYSV \
	-DRENAME -DNOREDIRECT -DNOIKSD -DNOJC -DNOLEARN $(KFLAGS) -Otcl -M3e" \
	"LNKFLAGS = -s" "LIBS = -lc -lx -ltcp"

# SCO XENIX/386 2.3.3 SysV with SCO TCP/IP
# System V STREAMS TCP developed by Lachman Associates Inc and
# Convergent Technologies.
# -DRENAME removed since some reports indicate it is not supported
# (whereas others say it is.)
sco3r2lai:
	@echo 'Making C-Kermit $(CKVER) for SCO XENIX/386 2.3.3 + TCP/IP...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DLAI_TCP -Di386 -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK \
	-DNAP -DTCPSOCKET -DPWID_T=int -DNOREDIRECT -DNOIKSD -DNOLEARN \
	$(KFLAGS) -Otcl -i -M3e" \
	"LNKFLAGS = -i -s" "LIBS = -lc -lx -lsocket"

sco3r2laic:
	@echo 'Making C-Kermit $(CKVER) for SCO XENIX/386 2.3.3 + TCP/IP...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DLAI_TCP -Di386 -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK \
	-DNAP -DTCPSOCKET -DCK_ANSIC -DCK_CURSES -DM_TERMINFO -DNOLEARN \
	-DPWID_T=int -DNOREDIRECT -DNOIKSD $(KFLAGS) -Otcl -i -M3e" \
	"LNKFLAGS = -i -s" "LIBS = -ltinfo -lc -lx -lsocket"

#SCO UNIX/386 3.2v2 (POSIX job control), shared libraries.
sco3r22:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v2 ...'
	make wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNOLEARN \
	-DNAP -DRENAME -DPID_T=pid_t -DPWID_T=int -DDIRENT -DNOIKSD \
	-DNOREDIRECT $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lc_s -lc -lx"

#SCO UNIX/386 3.2v2, POSIX job control, fullscreen file transfer display,
#dynamic memory allocation, shared C library
sco3r22c:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v2 ...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following four continued lines into one line.'
	make wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP -DNOLEARN \
	-DCK_CURSES -DDIRENT -DRENAME -DNOREDIRECT -DNOIKSD \
	-DPID_T=pid_t -DPWID_T=int $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lcurses -lc_s -lc -lx"

#SCO UNIX/386 3.2v2 with gcc 1.40 or later (POSIX job control)
sco3r22gcc:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v2, gcc'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following seven continued lines into one line.'
	make wermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" \
	"CFLAGS= -O -DPOSIX -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DNOLEARN -DRENAME -traditional -fpcc-struct-return -fstrength-reduce \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix \
	-DPID_T=pid_t -DPWID_T=int -DNOREDIRECT -DNOIKSD $(KFLAGS) " \
	"LNKFLAGS = -s" "LIBS = -lc_s -lc -lx"

#SCO UNIX/386 3.2v2 (ODT 1.1) (POSIX job control) with SCO TCP/IP, shared libs
#Requires SCO TCP/IP or ODT development system for telnet.h, etc.
sco3r22net:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2.2 + TCP/IP...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following three continued lines into one line.'
	make xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP -DTCPSOCKET \
	-DRENAME -DPID_T=pid_t -DPWID_T=int -DDIRENT -DNOREDIRECT -DNOIKSD \
	$(KFLAGS) -O" "LNKFLAGS = -s" "LIBS = -lsocket -lc_s -lc -lx"

#As above, but with curses for fullscreen file transfer display.
#Requires SCO TCP/IP or ODT development system for telnet.h, etc.
sco3r22netc:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v2 + TCP/IP...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following three continued lines into one line.'
	make xermit KTARGET=$${KTARGET:-$(@)} "CFLAGS= \
	-DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP -DTCPSOCKET -DRENAME \
	-DCK_CURSES -DDIRENT -DNOIKSD -DNOREDIRECT \
	-DPID_T=pid_t -DPWID_T=int -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lcurses -lsocket -lc_s -lc -lx"

#SCO XENIX 2.3.4, no curses, no TCP/IP, no IKSD.
#This one built and tested in C-Kermit 7.0.
#lcfp is C library floating-point support.
#Use -M3 to generate 32-bit i386 code instead of 16-bit segmented i286 code.
#Use -Me to enable MS nonstandard keywords in system headers.
#Use -W2 or W3 to increase the warning level.
#OK: 2011/06/15
sco234:
	@echo 'Making C-Kermit $(CKVER) for SCO XENIX 2.3.4...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSCO32 -DXENIX -DNOFILEH -DHDBUUCP -DRDCHK -DNOLEARN \
	-DNAP -DNOJC -DNOCOTFMC -DNOIKSD -DNOREDIRECT -DNOTNCODE -DNOGFTIMER \
	-DNOTIMEVAL -DNOTIMEZONE -DNOSYMLINK -DSCO234 -DDCLGETCWD $(KFLAGS) \
	-Otcl" "LNKFLAGS = -s" "LIBS = -lcfp -lc -lx"

#SCO XENIX 2.3.4, no TCP/IP, no IKSD, but with curses.
# Built and tested in C-Kermit 7.0.
# Note: XENIX 2.3.4 does not have newterm() so no point in adding -DCK_NEWTERM.
sco234c:
	@echo 'Making C-Kermit $(CKVER) for SCO XENIX 2.3.4 + curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSCO32 -DXENIX -DNOFILEH -DHDBUUCP -DRDCHK -DNOLEARN \
	-DNAP -DNOJC -DNOCOTFMC -DNOIKSD -DNOREDIRECT -DNOTNCODE -DNOGFTIMER \
	-DNOTIMEVAL -DNOTIMEZONE -DNOSYMLINK -DCK_CURSES -DSCO234 \
	-DDCLGETCWD $(KFLAGS) -Otcl" \
	"LNKFLAGS = -s" "LIBS = -lcfp -lc -ltinfo -lx"

#SCO XENIX 2.3.4 with SCO TCP/IP and curses, no IKSD.
# Built and tested in C-Kermit 7.0.  TCP/IP works and curses works.
# Previous versions of this target included -lmalloc, but this caused "error:
# " _calloc : symbol defined more than once" at link time so I removed it.
# Results are likely to vary depending on exactly which version of the SDK
# and TCP/IP SDK you have.
sco234netc:
	@echo 'Making C-Kermit $(CKVER) for SCO XENIX 2.3.4 + TCP + curses...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DSCO32 -DXENIX -DNOFILEH -DHDBUUCP -DRDCHK -DNOLEARN \
	-DNAP -DNOJC -DNOCOTFMC -DNOIKSD -DNOREDIRECT -DNOTNCODE -DNOGFTIMER \
	-DNOTIMEVAL -DNOTIMEZONE -DNOSYMLINK -DCK_CURSES -DSCO234 \
	-DDCLGETCWD -DTCPSOCKET -DNO_DNS_SRV $(KFLAGS) -Otcl" \
	"LNKFLAGS = -s" "LIBS = -ltinfo -lsocket -lcfp -lc -lx"

# SCO 3.2v4.x targets...

#  NOTE: Add -DDCLPOPEN and/or -DDCLFDOPEN to anySCO 3.2v4.x non-gcc entries
#  that complain about fdopen() or popen() at compile time.  They compile OK
#  without these flags as of July 1999.  However, the gcc entries seem to
#  need them, at least for gcc 2.7.2.2.

#  NOTE 2: To enable IKSD support, add:
#  -DCK_LOGIN -DNOGETUSERSHELL -DNOINITGROUPS
#  to CFLAGS (not tested).

#SCO UNIX/386 3.2v4 (POSIX job control), curses, ANSI C compilation,
#<dirent.h> (EAFS) file system.  Remove -lmalloc if it causes trouble.  It was
#put there to avoid core dumps caused by regular libc.a malloc.  Add -J to make
#all chars unsigned.  This version uses select() for CONNECT and also has
#high-precision timers and so might not work on non-TCP systems, in which case
#sco32v4ns should be used instead.
# If you get _ftime redefinition_ complaint, try adding -DODT30 to CFLAGS.
sco32v4:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v4...'
	make xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DCK_SCO32V4 -DNOFILEH -DHDBUUCP -DCK_CURSES -DM_TERMINFO \
	-DNOANSI -DSELECT -DNOIKSD -DDCLGETCWD -NOLSTAT \
	-DNOLINKBITS -DDCLGETCWD $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lcurses -lmalloc -lsocket -lc_s -lc -lx"

# As above, but with no dependence on sockets library or select().
sco32v4ns:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v4...'
	@echo 'No select() and no sockets library.'
	make wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DCK_SCO32V4 -DNOFILEH -DHDBUUCP -DCK_CURSES -DM_TERMINFO \
	-DNOANSI -DNOIKSD -DNOGFTIMER -DCK_POLL -DNAP -DDCLGETCWD -DNOLSTAT \
	-DNOLINKBITS -DDCLGETCWD -DNOLEARN -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lcurses -lmalloc -lc_s -lc -lx"

#SCO UNIX/386 3.2v4 (POSIX job control), TCP/IP, curses, ANSI C compilation,
#<dirent.h> (EAFS) file system.  With DIRENT, -lc must come before -lx.
#Reportedly it's OK to add -DCK_REDIR and -DCK_WREFRESH, and to remove -lc_s.
#Requires SCO TCP/IP development system or ODT for telnet.h, etc.
#See sco32v4 above for additional comments.
#NOTE: No more room for -Dxxx -- 25 seems to be the limit.  Move some to
#ckcdeb.h or somewhere...
sco32v4net:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v4...'
	@echo 'If you get _ftime redefinition_ complaint,'
	@echo 'use make sco-odt30.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DNOFILEH -DHDBUUCP -DTCPSOCKET -DCK_ANSIC -DCK_CURSES \
	-DNAP -DCK_WREFRESH -DNOLINKBITS -D_IBCS2 -DSELECT -DNOLSTAT \
	-DDCLGETCWD -DCK_SCO32V4 -DNOIKSD -O \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS) -s" \
	"LIBS = $(LIBS) -lcurses -lsocket -lmalloc -lsocket -lc_s -lc -lx"

#SCO UNIX/386 3.2v4 with gcc 1.40 or later, POSIX job control.
#Also see comments in sco32r4 entry.
sco32v4gcc:
	make xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" \
	"CFLAGS= -O -DNOFILEH -DHDBUUCP -DNOANSI -DCK_CURSES -DM_TERMINFO \
	-traditional -fpcc-struct-return -fstrength-reduce -funsigned-char \
	-D_KR -D_NO_PROTOTYPE -D_SVID -DNOIKSD -DCK_SCO32V4 -DNOLINKBITS \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM -DSELECT -DNOLSTAT \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 -DDCLGETCWD \
	-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix \
	-DDCLPOPEN -DDCLFDOPEN $(KFLAGS) " \
	"LNKFLAGS = -s" "LIBS = -lcurses -lsocket -lc_s -lc -lx"

#SCO UNIX/386 3.2v4 (POSIX job control), TCP/IP, curses, ANSI C compilation,
#Requires SCO TCP/IP or ODT development system for telnet.h, etc.
#<dirent.h> (EAFS) file system.  With DIRENT, -lc must come before -lx.
#gcc 1.40 or later.  Also see comments in sco32r4 entry.
sco32v4netgcc:
	make xermit KTARGET=$${KTARGET:-$(@)} "CC = gcc" "CC2=gcc" \
	"CFLAGS= -O2 -DNOFILEH -DHDBUUCP -DSELECT -DNOLSTAT \
	-DNOANSI -DTCPSOCKET -DCK_CURSES -DM_TERMINFO \
	-D_KR -D_NO_PROTOTYPE -D_SVID -DNOIKSD -DCK_SCO32V4 -DNOLINKBITS \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM -DDCLGETCWD \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix \
	-DDCLPOPEN -DDCLFDOPEN $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lcurses -lsocket -lc_s -lc -lx"

#As above but with bgcc BOUNDS CHECKING (for developers only).  -lcheck has
#bounds-checking replacements for malloc, memcpy, bcopy, etc, so must come
#before -lsocket and -lc.
sco32v4netbgcc:
	make xermit KTARGET=$${KTARGET:-$(@)} \
	"CC = bgcc -pipe -m386" "CC2=bgcc -pipe -m386" \
	"CFLAGS= -O1 -g -DNOFILEH -DHDBUUCP -DSELECT \
	-DNOANSI -DTCPSOCKET -DCK_CURSES -DM_TERMINFO \
	-D_KR -D_NO_PROTOTYPE -D_SVID -DNOIKSD -DCK_SCO32V4 -DNOLSTAT \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM -DNOLINKBITS \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 -DDCLGETCWD \
	-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix \
	-DDCLPOPEN -DDCLFDOPEN $(KFLAGS) " \
	"LNKFLAGS = -g" "LIBS = -lcurses -lcheck -lsocket -lx"

sco32v4netnd:
	@echo sco32v4net with no debug
	$(MAKE) "MAKE=$(MAKE)" sco32v4net KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DNODEBUG -DNOTLOG" "LIBS=$(LIBS)"

sco3r2netnd:
	@echo sco32v4netnd built for SCO XENIX 2.3 under SCO UNIX...
	@echo   requires copying /lib/386/Slibc.a to /lib/386/Slibc_s.a and
	@echo   getting /lib/386/Slibsocket.a from a XENIX devkit.
	@echo   WARNING: poll/CK_POLL supported only on XENIX 2.3.4
	echo    For earlier XENIX systems, replace CK_POLL with RDCHK.
	$(MAKE) "MAKE=$(MAKE)" sco32v4netnd KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -x2.3 -DNORENAME -DNOSYMLINK" \
	"LNKFLAGS = $(LNKFLAGS) -x2.3" \
	"LIBS=-ldir -lcfp $(LIBS)"

#SCO UNIX/386 3.2v4 (POSIX job control), TCP/IP, curses, ANSI C compilation,
#<dirent.h> (EAFS) file system.  With DIRENT, -lc must come before -lx.
#Reportedly it's OK to add -DCK_REDIR and -DCK_WREFRESH, and to remove -lc_s.
#Requires SCO TCP/IP development system or ODT for telnet.h, etc.
#See sco32v4 above for additional comments.
# Note: "xermit" means use the select() version of the CONNECT module.
sco32v4netx:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v4...'
	@echo 'If you get _ftime redefinition_ complaint,'
	@echo 'use make sco-odt30.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DNOFILEH -DHDBUUCP -DTCPSOCKET -DCK_ANSIC -DCK_CURSES -DNAP \
	-DCK_WREFRESH -DNOLINKBITS -D_IBCS2 -DSELECT -DDCLGETCWD \
	-DCK_SCO32V4 -DNOIKSD -DNOLSTAT -O $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS) -s" \
	"LIBS = $(LIBS) -lcurses -lsocket -lmalloc -lsocket -lc_s -lc -lx"

sco32v4netndx:
	@echo sco32v4netx with no debug
	$(MAKE) "MAKE=$(MAKE)" sco32v4netx KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DNODEBUG -DNOTLOG" "LIBS=$(LIBS)"

sco3r2netndx:
	@echo sco32v4netndx built for SCO XENIX 2.3 under SCO UNIX...
	@echo   requires copying /lib/386/Slibc.a to /lib/386/Slibc_s.a and
	@echo   getting /lib/386/Slibsocket.a from a XENIX devkit.
	@echo   WARNING: poll/CK_POLL supported only on XENIX 2.3.4
	echo    For earlier XENIX systems, replace CK_POLL with RDCHK.
	$(MAKE) "MAKE=$(MAKE)" sco32v4netndx KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -x2.3 -DNORENAME -DNOSYMLINK" \
	"LNKFLAGS = $(LNKFLAGS) -x2.3" \
	"LIBS=-ldir -lcfp $(LIBS)"

sco-odt30:
	@echo SCO ODT 3.0
	$(MAKE) "MAKE=$(MAKE)" sco32v4net KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$(KFLAGS) -DODT30"

#SCO OpenServer 5.0 (SCO UNIX 3.2v5.0) with SCO development tools, no TCP/IP.
#SCO OSR5 is much more like standard System V than previous SCO releases.
#The SCO development tools include TCP/IP, so this target is only for creating
#artificially limited versions of kermit required by site policy rather than
#the operating system.  NOSYSLOG is included because syslog() requires the
#sockets library.
sco32v500:
	@echo Making C-Kermit $(CKVER) for SCO OpenServer Release 5...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DDIRENT -DHDBUUCP -DSVR4 -DCK_SCOV5 -DCK_RTSCTS \
	-DCK_CURSES -DCK_WREFRESH -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DNOGETUSERSHELL -DNOLSTAT -DNOLINKBITS -DNOSYSLOG \
	$(KFLAGS)" \
	"LIBS=-lcurses $(LIBS)" "LNKFLAGS=$(LNKFLAGS)"

sco32v5:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=$(KFLAGS)" sco32v500


#SCO OpenServer 5.0 with networking, SCO development tools.
#Networking libraries are now provided with the OS.
sco32v500net:
	@echo Making C-Kermit $(CKVER) for SCO OpenServer Release 5...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DDIRENT -DHDBUUCP -DSVR4 -DCK_SCOV5 -DCK_RTSCTS \
	-DCK_CURSES -DCK_WREFRESH -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DNOGETUSERSHELL -DNOLSTAT -DNOLINKBITS -DTCPSOCKET \
	-DNO_DNS_SRV $(KFLAGS)" \
	"LIBS=-lcurses -lsocket $(LIBS)" "LNKFLAGS=$(LNKFLAGS)"

sco32v5net:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=$(KFLAGS)" sco32v500net

#SCO OpenServer 5.0 with networking and OpenSSL, SCO development tools.
#Networking libraries are now provided with the OS.
sco32v500net+ssl:
	@echo Making C-Kermit $(CKVER) for SCO OSR5 with OpenSSL...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DDIRENT -DHDBUUCP -DSVR4 -DCK_SCOV5 -DCK_RTSCTS \
	-DCK_CURSES -DCK_WREFRESH -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DNOGETUSERSHELL -DNOLSTAT -DNOLINKBITS -DTCPSOCKET \
	-DNO_DNS_SRV -DCK_AUTHENTICATION -DCK_SSL -DCK_TRIGGER \
	$(SSLINC) $(SSLLIB) $(KFLAGS)" \
	"LIBS=$(SSLLIB) -lcurses -lsocket -lssl -lcrypto $(LIBS)" \
	"LNKFLAGS=$(LNKFLAGS)"

#SCO OpenServer 5.0 with gcc, no networking.
#Note: NOSYSLOG required for non-net entries because it requires <socket.h>
sco32v500gcc:
	@echo Using gcc...
	$(MAKE) "MAKE=$(MAKE)" sco32v500CC=gcc CC2=gcc \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS= $(KFLAGS)"

#SCO OpenServer 5.0 with networking, gcc.
sco32v500netgcc:
	@echo TCP/IP networking added - using gcc...
	$(MAKE) "MAKE=$(MAKE)" sco32v500net CC=gcc CC2=gcc \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS=$(KFLAGS)"

#SCO OpenServer 5.0 with networking, gcc, elf.
sco32v500netgccelf:
	@echo TCP/IP networking added - using gcc, dynamic elf library
	$(MAKE) "MAKE=$(MAKE)" sco32v500net "CC=gcc" "CC2=gcc" \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS=-O3 -belf" "LNKFLAGS=-belf"

sco32v502:
	$(MAKE) "MAKE=$(MAKE)" sco32v500 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR502 $(KFLAGS)"

#SCO OpenServer 5.0.2 with networking, SCO development tools.
sco32v502net:
	@echo TCP/IP networking added...
	$(MAKE) "MAKE=$(MAKE)" sco32v500net KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-b elf -DSCO_OSR502 $(KFLAGS)"

#SCO OpenServer 5.0.4 (SCO UNIX 3.2v5.0.4) with SCO development tools.
#Like 5.0, but adds high serial speeds.  First POSIX-based SCO version.
#Note: the -O flag is deliberately omitted for /bin/cc (= /usr/ccs/bin/cc).
sco32v504:
	@echo Making C-Kermit $(CKVER) for SCO OpenServer Release 5.0.4...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DDIRENT -DHDBUUCP -DSVR4 -DCK_SCOV5 -DCK_RTSCTS \
	-DSCO_OSR504 -b elf -DPOSIX \
	-DCK_CURSES -DCK_WREFRESH -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DNOGETUSERSHELL -DNOLSTAT -DNOLINKBITS -DNOSYSLOG $(KFLAGS)" \
	"LIBS=-lcurses $(LIBS)" "LNKFLAGS=$(LNKFLAGS)"

#SCO OpenServer 5.0.4 with gcc, no networking.
sco32v504gcc:
	@echo Using gcc...
	$(MAKE) "MAKE=$(MAKE)" sco32v504 "CC=gcc" "CC2=gcc" \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS= $(KFLAGS)"

#SCO OpenServer 5.0.4 with networking.
#SCO development tools (/bin/cc = /usr/ccs/bin/cc).
#Optimization deliberately suppressed.
sco32v504net:
	@echo Making C-Kermit $(CKVER) for SCO OpenServer Release 5.0.4...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DDIRENT -DHDBUUCP -DSVR4 -DCK_SCOV5 -DCK_RTSCTS \
	-DCK_CURSES -DCK_WREFRESH -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DNOGETUSERSHELL -DNOLSTAT -DNOLINKBITS -DTCPSOCKET \
	-b elf -DSCO_OSR504 -DPOSIX -DNO_DNS_SRV $(KFLAGS)" \
	"LIBS=-lcurses -lsocket $(LIBS)" "LNKFLAGS=$(LNKFLAGS)"

#SCO OpenServer 5.0.4 with networking, gcc.
sco32v504netgcc:
	@echo TCP/IP networking added - using gcc...
	@echo If gcc crashes on ckwart.c then build it by hand:
	@echo " gcc -o wart -DCK_SCOV5 ckwart.c"
	$(MAKE) "MAKE=$(MAKE)" sco32v500net "CC=gcc" "CC2=gcc" \
	KTARGET=$${KTARGET:-$(@)} "KFLAGS=-DSCO_OSR504 -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.4 with networking, gcc, elf.
sco32v504netgccelf:
	@echo TCP/IP networking added - using gcc, dynamic elf library
	$(MAKE) "MAKE=$(MAKE)" sco32v500net "CC=gcc" "CC2=gcc"
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR504 -DPOSIX -O3 -belf $(KFLAGS)" \
	LNKFLAGS="-belf"

#SCO OpenServer 5.0.5 (SCO UNIX 3.2v5.0.5) with SCO /bin/cc.
#Like 5.0, but adds high serial speeds.  First POSIX-based SCO version.
#You might have to add "LIBS=-ltinfo" (some do, some don't).
sco32v505:
	$(MAKE) "MAKE=$(MAKE)" sco32v500 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DNOSHADOW -b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.5 (SCO UNIX 3.2v5.0.5) with SCO UDK.
#This one can't see the high serial speeds and anything to do with modem
#signals doesn't work because UKD cc has its own alternative universe of
#header files.
sco32v505udk:
	$(MAKE) "MAKE=$(MAKE)" sco32v500 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DDCLTIMEVAL -DNOSHADOW -b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.5 with networking, SCO /bin/cc.
#See comments with sco32v505 targets.
sco32v505net:
	@echo TCP/IP networking added...
	$(MAKE) "MAKE=$(MAKE)" sco32v500net KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DNOSHADOW -b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.5 with networking and OpenSSL, SCO /bin/cc.
#See comments with sco32v505 targets.
sco32v505net+ssl:
	@echo TCP/IP networking and OpenSSL added...
	$(MAKE) "MAKE=$(MAKE)" sco32v500net+ssl KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DNOSHADOW -b elf -DPOSIX $(KFLAGS) " \
	"LIBS=$(SSLLIB) -lcurses -lsocket -lssl -lcrypto $(LIBS)" \
	"LNKFLAGS=$(LNKFLAGS)"

#SCO OpenServer 5.0.5 with networking, SCO UDK.
#See comments with above sco32v505 targets.
sco32v505udknet:
	@echo TCP/IP networking added...
	$(MAKE) "MAKE=$(MAKE)" sco32v500net KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DDCLTIMEVAL -DNOSHADOW -b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.5 with gcc, no networking.
sco32v505gcc:
	@echo Using gcc...
	$(MAKE) "MAKE=$(MAKE)" sco32v500 "CC=gcc" "CC2=gcc" \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DPOSIX -funsigned-char $(KFLAGS)"

#SCO OpenServer 5.0.5 with gcc, no networking, no shadow passwords.
sco32v505xgcc:
	@echo Using gcc...
	$(MAKE) "MAKE=$(MAKE)" sco32v500 "CC=gcc" "CC2=gcc" \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DNOSHADOW -DPOSIX -funsigned-char $(KFLAGS)"

#SCO OpenServer 5.0.5 with networking, gcc.
sco32v505netgcc:
	@echo TCP/IP networking added - using gcc...
	@echo If gcc crashes on ckwart.c then build it by hand:
	@echo " gcc -o wart -DCK_SCOV5 ckwart.c"
	$(MAKE) "MAKE=$(MAKE)" sco32v500net "CC=gcc" "CC2=gcc" \
	KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DNOSHADOW -DPOSIX -funsigned-char $(KFLAGS)"

#egcs is just like gcc but generates ELF by default.
#Or you can include -melf (not -belf) to force it.
sco32v505netegcs:
	$(MAKE) "MAKE=$(MAKE)" "KFLAGS=$(KFLAGS)" sco32v505netgcc \
	KTARGET=$${KTARGET:-$(@)}

#SCO OpenServer 5.0.5 with networking, gcc, elf.
sco32v505netgccelf:
	@echo TCP/IP networking added - using gcc, dynamic elf library
	$(MAKE) "MAKE=$(MAKE)" sco32v500net "CC=gcc" "CC2=gcc" \
	"KFLAGS=-DSCO_OSR505 -DPOSIX -funsigned-char -O3 -belf $(KFLAGS)" \
	KTARGET=$${KTARGET:-$(@)} LNKFLAGS="-belf"

#SCO OpenServer 5.0.6 with SCO /bin/cc.
# Add -DDCLTIMEVAL when building with UDK.
#Like 5.0.5.   IMPORTANT: Use sco32v506a target for 5.0.6a.
sco32v506:
	$(MAKE) "MAKE=$(MAKE)" sco32v500 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DSCO_OSR506 -b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.6 with networking, SCO /bin/cc.
# Add -DDCLTIMEVAL when building with UDK.
# IMPORTANT: Use sco32v506a target for 5.0.6a.
sco32v506net:
	@echo TCP/IP networking added...
	$(MAKE) "MAKE=$(MAKE)" sco32v500net KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DSCO_OSR506 -b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.6a, no networking, SCO development tools.
#This one has patched sio drivers that, for the first time,
#actually handle modem signals correctly.
# Add -DDCLTIMEVAL when building with UDK.
sco32v506a:
	$(MAKE) "MAKE=$(MAKE)" sco32v500 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DSCO_OSR506 -DSCO_OSR506A -DNEEDMDMDEFS \
	-b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.6a with networking, SCO development tools.
# Add -DDCLTIMEVAL when building with UDK.
sco32v506anet:
	@echo TCP/IP networking added...
	$(MAKE) "MAKE=$(MAKE)" sco32v500net KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DSCO_OSR506 -DSCO_OSR506A -DNEEDMDMDEFS \
	-b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.7, no networking, SCO development tools.
#Adds flags to make PTY and SSH commands work.  These have been tested
#only in 5.0.7 but probably they can also be added to earlier OSR5 targets.
sco32v507:
	$(MAKE) "MAKE=$(MAKE)" sco32v500 KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DSCO_OSR506 -DSCO_OSR507 -DNEEDMDMDEFS \
	-DHAVE_PTSNAME -DHAVE_PTMX -DHAVE_GRANTPT \
	-b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 5.0.7 as above but with networking.
sco32v507net:
	@echo TCP/IP networking added...
	$(MAKE) "MAKE=$(MAKE)" sco32v500net KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DSCO_OSR505 -DSCO_OSR506 -DSCO_OSR507 -DNEEDMDMDEFS \
	-DHAVE_PTSNAME -DHAVE_PTMX -DHAVE_GRANTPT \
	-b elf -DPOSIX $(KFLAGS)"

#SCO OpenServer 6 (new target 30 Jan 2006)
sco_osr600 sco600:
	@echo Making C-Kermit $(CKVER) for SCO OpenServer 6.0.0...
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -O -DDIRENT -DHDBUUCP -DSVR4 -DCK_SCOV5 -DCK_RTSCTS \
	-DCK_CURSES -DCK_WREFRESH -DCK_NEWTERM -DSELECT -DSELECT_H \
	-DNOGETUSERSHELL -DNOLSTAT -DNOLINKBITS -DTCPSOCKET \
	-DNO_DNS_SRV -DSCO_OSR505 -DSCO_OSR506 -DSCO_OSR507 -DNEEDMDMDEFS \
	-DHAVE_PTSNAME -DHAVE_PTMX -DHAVE_GRANTPT -DDCLTIMEVAL \
	-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
	-DSOCKOPT_T=socklen_t -DGSOCKNAME_T=size_t -DGPEERNAME_T=size_t \
	-DHERALD=\"\\\" SCO OpenServer `uname -v`\\\"\" \
	-b elf -DPOSIX $(KFLAGS)" \
	"LIBS=-lcurses -lsocket $(LIBS)" "LNKFLAGS=$(LNKFLAGS)"

#Tandy 16/6000 with Xenix 3.0 (16 bits)
#C-Kermit 7.0 (and later) do not build here; "too many defines".
#Add more -DNOxxx options to remove features if program won't load.
#Successful operation is a function of program size, physical memory,
#available swap space, etc.  The following stripped-down configuration
#seems to work on most Tandy 6000s.  NOTE: "-+" means allow long variable
#names, needed for C-Kermit 6.0 because some identifiers are not unique
#within the first six characters.
trs16:
	@echo 'Making C-Kermit $(CKVER) for Tandy 16/6000, Xenix 3.0...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -+ -DATTSV -DTRS16 -DNOMKDIR -DDCLPOPEN -DCK_CURSES \
	-DNODEBUG -DNOTLOG -DNOHELP -DNOSCRIPT -DNOCSETS -DNOIKSD \
	-DNOREDIRECT -DNOSYSLOG -DNOPUTENV -DNOREALPATH -DNOLEARN \
	$(KFLAGS) -O" "LIBS= -lcurses -ltermcap" "LNKFLAGS = -+ -n -s"

# QNX 4.21 and above, 32-bit version, Watcom C32 10.6, fully configured,
# except no job control because QNX 4.x does not support it.  New NCURSES
# library used instead of CURSES.
#
# -Oatx optimizes to favor speed over size: loop optimization, inline fn's.
# -Os favors size over speed.  Saves 30-40K out of about 1.75M.
# -3r = generate 386 code with register-based arg passing.
# -3s = generate 386 code with stack-based arg passing.
# -ms = separate code & data 4GB segments (32-bit builds only).
# -mf = flat memory model code+data in one 4GB segment (ditto).
# -zc = place literal strings in code segment.
# -N4M = Big stack (increase the digit upon SIGSEGVs at runtime).
# chars are unsigned by default (-j makes them signed by default).
# -NOUUCP is included because QNX doesn't use it.
# Add these to the end if you like but they dump core on my QNX 4.25 system:
#
#	@wermit -h >use.qnx
#	@usemsg wermit use.qnx
#	@rm use.qnx
#
# If you get warnings about HEADER or C_IN add -DNO_DNS_SRV.
# OK 2011/06/14
qnx32:
	@echo 'Making C-Kermit $(CKVER) for QNX 4.2x, 32-bit...'
	$(MAKE) xermit \
	"LNKFLAGS = -N4M -3r" \
	"CFLAGS = -ms -3r -DQNX -DTCPSOCKET -DCK_CURSES -DNOGETUSERSHELL \
	-DCK_WREFRESH -DCK_REDIR -DSELECT -DSELECT_H -DCK_RTSCTS -DNOJC \
	-DNOINITGROUPS -DNOUUCP -DCK_ANSIC -DPID_T=pid_t -Oatx -zc $(KFLAGS)" \
	"LIBS= -lsocket -lncurses -ltermcap"

# As above but no networking since some QNX systems do not have TCP/IP
# installed, or the TCP/IP developers kit, which includes all the needed
# header files.  This entry has not been tested on a QNX system that, in
# fact, does not have TCP/IP installed; some adjustments might be necessary,
# in particular regarding the use of select(): is -lsocket needed, can we
# get the needed definitions from non-TCP/IP header files (FD_SET, etc)?
qnx32nonet:
	@echo 'Making C-Kermit $(CKVER) for QNX 4.2x, 32-bit, no net...'
	$(MAKE) xermit \
	"LNKFLAGS = -N4M -3r" \
	"CFLAGS = -3r -ms -DQNX -DNONET -DNOIKSD -DCK_CURSES \
	-DCK_WREFRESH -DCK_REDIR -DSELECT -DSELECT_H -DCK_RTSCTS -DNOJC \
	-DNOUUCP -DCK_ANSIC -DPID_T=pid_t -Oatx -zc $(KFLAGS)" \
	"LIBS= -lsocket -lncurses -ltermcap"
	@wermit -h >use.qnx
	@usemsg wermit use.qnx
	@rm use.qnx

# Synonym for qnx32.
qnx:
	$(MAKE) qnx32 "KFLAGS=$(KFLAGS)"

# QNX 4.21 and above, 16-bit version, Watcom C 8.5 - and higher on i286 PCs
# and above.
#
#	IMPORTANT: Do not use Watcom C 10.6!!!
#	If you have it installed, add "-v9.52 to CFLAGS"
#
# NOTE: QNX 4.23 onward does not work on 286's anyway.
# Stacksize 26000, objects larger than 100 bytes in their own segments,
# string constants to the codesegment, etc.  Fully configured except job ctrl.
# This entry works for building a 16-bit executable on a 32-bit system, but
# has not been tested on a 16-bit system.  Uses large memory model, links
# explicitly with large-model sockets library.  Correct-model curses library
# is chosen automatically.  See comment in qnx32 entry about -DNOUUCP.
#
# WARNING:
#
# Watcom C prior to 10.6 never had released curses library. To link against it,
# you must obtain ported free curses source from ftp://ftp.qnx.com/usr/free,
# then compile and build library (cursesl.lib) and place it in /usr/lib.  You
# must also copy curses.h to /usr/include.  Be aware that if you have Watcom
# 10.6 installed, you should already have curses.h, which is the new ncurses
# library. You must back it up and use free curses.h instead, since ncurses is
# only for 32-bit applications and some definitions in these files are
# different (e.g., clearok()).  For safety, curses is not defined in build.
#
# In 7.0 -DNOHELP added to keep ckuus2.c from blowing up; NOCSETS and NOSPL
# added because ckuus4 was blowing up, and NOFLOAT just because it seemed
# dangerous (remove -DNOFLOAT if you want to try it), The result works OK
# except for some mysterious beeps upon termination of the top-level keyword.
#
# Things to try next time we get in trouble:
#  . Change -zt100 to something smaller like -zt25
#  . Change -Oatx to -Omilerat (enable stack checking)
#  . Maybe get rid of -v9.52 -- it's only there because we were warned.
#
qnx16:
	@echo 'Making C-Kermit $(CKVER) for QNX 4.21, 16-bit...'
	$(MAKE) xermit \
	"LNKFLAGS = -2 -ml -N 26000" \
	"CFLAGS = -2 -Oatx -zc -zt100 -ml -DQNX -DQNX16 -DNOUUCP -DNOHELP \
	-DCK_REDIR -DSELECT -DSELECT_H -DNOJC -DNOGETUSERSHELL -DNOCSETS \
	-v9.52 -DTCPSOCKET -DCK_RTSCTS -DCK_ANSIC -DNOINITGROUPS -DNOKVERBS \
	-DNORANDOM -DNOCSETS -DNOSPL -DNOFLOAT -DPID_T=pid_t $(KFLAGS)"

# QNX 4.1, 16-bit version, with Watcom C 8.5 on i286 PCs and above.
# stacksize 26000, objects larger than 100 bytes in their own segments,
# string constants to the codesegment, etc.  Add -DNOUUCP if desired.
qnx16_41:
	@echo 'Making C-Kermit $(CKVER) for QNX 4.1, 16-bit...'
	$(MAKE) xermit \
	"LNKFLAGS = -mh -N 26000" "CFLAGS = -Wc,-fpc -Wc,-j -DNOGETUSERSHELL \
	-Wc,-Ols -Wc,-zdf -Wc,-zc -Wc,-zt100 -mh -DPOSIX -DQNX -DDIRENT \
	-DNOCYRIL -DNODEBUG -DNOMSEND -DMINIDIAL -DNOXMIT -DNOSCRIPT -DNOSPL \
	-DNOSETKEY -DNOINITGROUPS -DQNX16 -DPID_T=pid_t $(KFLAGS)"

# QNX Neutrino 2 (pwaechtler@qnx.de) crosscompiled on QNX 4.25.
# Gets lots of compiler warnings.
qnx_nto2+:
	@echo 'Making C-Kermit $(CKVER) for QNX Neutrino 2+ '
	cc -o wart ckwart.c
	$(MAKE) xermit \
	"CC = qcc -Vgcc_ntox86" \
	"CC2 = qcc -Vgcc_ntox86" \
	"LNKFLAGS = " \
	"CFLAGS = -DNEUTRINO -DTCPSOCKET -DCK_CURSES -DNOGETUSERSHELL \
	-DNOUUCP -DCK_WREFRESH -DCK_REDIR -DSELECT -DSELECT_H -DCK_RTSCTS \
	-DNOJC -DNOINITGROUPS -DCK_ANSIC -DPID_T=pid_t -DUNIX -DDIRENT \
	-DMYREAD -DBSD44ORPOSIX -DSVORPOSIX -DNDGPWNAM $(KFLAGS)" \
	"LIBS= -lsocket -lncurses "

# QNX 6 (= Neutrino 2.xx) native build (kirussel@cisco.com).
qnx6:
	@echo 'Making C-Kermit $(CKVER) for QNX6'
	$(MAKE) xermit KTARGET=QNX6 \
	"CFLAGS = -DPOSIX -DCK_POSIX_SIG -DNETPTY -DNOARROWKEYS \
	-DUSE_TIOCSDTR -DBIGBUFOK -DCKMAXOPEN=100 -DRLOGCODE -DNOREALPATH \
	-DMAXNAMLEN=48 -DQNX6 -DUSE_TERMIO -DINIT_SPTY \
	-DCK_CURSES -DCK_WREFRESH -DCK_NEWTERM -DDYNAMIC \
	-DTCPSOCKET -DNOGETUSERSHELL -DCK_REDIR -DSELECT -DSELECT_H \
	-DCK_RTSCTS -DNOJC -DSVORPOSIX -DBSD44ORPOSIX -DNOUUCP -DCK_ANSIC \
	$(KFLAGS) -O" \
	"LIBS= -lsocket  -lncurses"

#MINIX/2.0 32 Bit version for intel 386+ running the POSIX-compliant MINIX
# version 2.0 (The definition of fatal avoids a conflict with a symbol by
# the same name in the curses library.) It is impossible to compile with
# network support since Minix does not support Berkeley sockets.
# Note: use chmem liberally on the compiler passes, make, and the final
# kermit executable. (3 megabytes of memory for each is sufficient.)
# From Terry McConnell, Syracuse U, and Will Rose.  Will says:
# The stacks for make and some compiler passes needed to be increased
# with chmem as follows:
#   make 1MB
#   /usr/lib/em_cemcom.ansi 3MB
#   /usr/lib/em_opt 1MB
#   /usr/lib/i386/cg 1MB
#   /usr/lib/i386/as 1MB
# The compiler temporary directory was set to /usr/tmp via the TMPDIR
# environment variable; more than 1MB of temporary space was needed.
# Kermit itself needs at least 1MB of stack.
minix20:
	@echo 'Making C-Kermit $(CKVER) for MINIX 2.0/386...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} EXT=o \
	"CFLAGS=  -wo -DV7 -DMINIX2 -DMINIX -DSIG_V -D_POSIX_SOURCE \
	-DCKCPU=\\\"i-386\\\" -DNOIKSD -Dfatal=myfatal -DCK_CURSES -DNOLEARN \
	-DNOSYSLOG -DUSE_MEMCPY -DNOREALPATH $(KFLAGS)" "LIBS= -lcurses"

#MINIX/386 (PC Minix modified by Bruce Evans in Australia for 386 addressing)
# For MINIX 1.5+ (but < 2.0)
minix386:
	@echo 'Making C-Kermit $(CKVER) for MINIX/386...'
	@echo 'TOTALLY UNTESTED!'
	$(MAKE) wermit EXT=s KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DV7 -DMINIX -D_POSIX_SOURCE -DNOLEARN $(KFLAGS)"

#MINIX/386 Minix modified by Bruce Evans in Australia to use 386 addressing
minix386gcc:
	@echo 'Making C-Kermit $(CKVER) for MINIX/386 with gcc...'
	@echo 'TOTALLY UNTESTED!'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} "CC=gcc -g -O" "CC2=gcc -g" \
	"CFLAGS= -DV7 -DMINIX -D_POSIX_SOURCE -DNOLEARN $(KFLAGS)"

#MINIX - 68k version with ACK compiler.
# If you have trouble compiling or running wart, "touch wart".
# If it still doesn't work, "touch ckcpro.c".
# The version configured below has many features removed, including
# the TRANSMIT, MSEND, HELP, and SCRIPT commands, international
# character set support, and the entire script programming language.
# But it does have an interactive command parser.
# Make sure make(1) has (at least) 100000 chmemory!
# If you are using the Amsterdam C compiler, you might have to add "-D__ACK__".
minix68k:
	@echo 'Making C-Kermit $(CKVER) for MINIX 68k with ACK...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DV7 -DMINIX -D_MINIX -D_POSIX_SOURCE -DNOLEARN \
	-DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOSPL $(KFLAGS) \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V"

#MINIX - 68k version with c68 compiler.
# If you have trouble compiling or running wart, "touch wart" or
# "touch ckcpro.c". Compiling ckudia.c (no -DNODIAL!) might fail. :-(
# Give c68 250000 bytes of stack+heap; make sure make(1) has at least
# 100000 chmemory.  On a 1MB Atari ST this means that the recursive
# call of make fails due to memory shortage.  Try "make -n minixc68 >makeit",
# followed by ". makeit".  Otherwise, as above.
minixc68:
	@echo 'Making C-Kermit $(CKVER) for MINIX 68k with c68...'
	$(MAKE) wermit "CC= cc -c68" KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DV7 -DMINIX -D_MINIX -D_POSIX_SOURCE -DNOLEARN \
	-DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOSPL $(KFLAGS) \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V"

#MINIX - 68k version with c68 compiler.
#A variation on the above that was recently (Sep 95) reported to work.
minixc68a:
	@echo 'Making C-Kermit $(CKVER) for MINIX 68k with c68...'
	$(MAKE) wermit "CC= cc -c68" KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DV7 -DMINIX -D_MINIX -D_POSIX_SOURCE \
	-DCK_ANSIC -DNODEBUG -DNOTLOG -DMINIDIAL -DEXTEN -DMYCURSES \
	-DNOSCRIPT -DNOCSETS -DNOSPL -DNOJC -DDIRENT -DNOLEARN \
	-DNOSETKEY -DNOESCSEQ $(KFLAGS) \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V"

#MIPS Computer Systems with UMIPS RISC/OS 4.52 = AT&T UNIX System V R3.0.
#Remove -DNOJC if job control can be safely used.
mips:
	@echo 'Making C-Kermit $(CKVER) for MIPS RISC/OS...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DMIPS -DDIRENT -DCK_POLL -DNOJC -DNOLEARN -DPID_T=int \
	-DGID_T=gid_t -DUID_T=uid_t -i -O1500 $(KFLAGS)"

#As above, but with TCP/IP and fullscreen support.
mipstcpc:
	@echo 'Making C-Kermit $(CKVER) for MIPS RISC/OS...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DMIPS -DDIRENT -DCK_POLL -DNOJC \
	-DTCPSOCKET -DCK_CURSES -I/usr/include/bsd \
	-DPID_T=int -DGID_T=gid_t -DUID_T=uid_t -i -O1500 $(KFLAGS)" \
	"LIBS = -lcurses -lbsd"

#Motorola Delta System V/68 R3, signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp.  Supports TCP/IP.
#After building, use "mcs -d" to reduce size of the executable program.
sv68r3:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/68 R3...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR3 -DSV68 -DDIRENT -DHDBUUCP -DNO_DNS_SRV -DTCPSOCKET \
	-DNOUNICODE -DNOLEARN -DUSE_MEMCPY $(KFLAGS) -O" "LNKFLAGS ="

#Motorola Delta System V/68 R3V5, signal() is void rather than int.
#Uses dirent.h and Honey DanBer UUCP.  Supports TCP/IP.
#After building, use "mcs -d" to reduce size of the executable program.
sv68r3v5:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/68 R3V5'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR3 -DSV68 -DDIRENT -DHDBUUCP -DNO_DNS_SRV -DUSE_MEMCPY \
	-DTCPSOCKET -DINADDRX -DNOUNICODE -DFNFLOAT -DNOLEARN $(KFLAGS) -O" \
	"LNKFLAGS =" "LIBS = -linet -lm"

#Motorola MVME147 System V/68 R3 V5.1. Requires gcc 2.1 to compile.
#After building, use "mcs -d" to reduce size of the executable program.
sv68r3v51:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/68 R3V5.1'
	$(MAKE) wermit "CC=gcc-delta" "CC2=gcc-delta" \
	KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DNODEBUG -DNO_DNS_SRV -DNOLEARN \
	-DNOUNICODE -DFNFLOAT -DSV68 -DUSE_MEMCPY $(KFLAGS) \
	-O2 -v -ftraditional" \
	"LNKFLAGS = -s -v" "LIBS = -lm881 -lm"

#Motorola MVME147 System V/68 R3V6. derived from Motorola Delta System R3V5.
#Checked on larger Motorola System V/68 R3V6 (with NSE Network Services Ext.)
#After building, use "strip" to reduce size of the executable program.
# "LIBS = -lnsl" removed in C-Kermit 6.1 - put back if needed.
# "LIBS = lm" added in 7.1/8.0 for floating-point math.
# ckuusr.c clobbers the optimizer.
sv68r3v6:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/68 R3V6'
	$(MAKE) ckuusr.$(EXT) KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSV68R3V6 -DDIRENT -DHDBUUCP -DNOLOGIN -DNOINITGROUPS \
	-DNOSYMLINK -DNOREDIRECT -DNOGFTIMER -DTCPSOCKET -DDCLGETCWD -DSV68 \
	-DNO_DNS_SRV -DNOUNICODE -DFNFLOAT -DSELECT -DUSE_MEMCPY $(KFLAGS)"
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSV68R3V6 -DDIRENT -DHDBUUCP -DNOLOGIN -DNOINITGROUPS \
	-DNOSYMLINK -DNOREDIRECT -DNOGFTIMER -DTCPSOCKET -DDCLGETCWD -DSV68 \
	-DNO_DNS_SRV -DNOUNICODE -DFNFLOAT -DSELECT -DUSE_MEMCPY $(KFLAGS)" \
	"LNKFLAGS =" "LIBS = -lm"

#Motorola Delta System V/88 R32, signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp.  Needs <sys/utime.h> for setting
#file dates.  Supports TCP/IP.
#After building, use "mcs -d" to reduce size of the executable program.
sv88r32:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/88 R32...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -DSV88R32 -DDIRENT -DHDBUUCP -DTCPSOCKET \
	-DSYSUTIMEH -DCK_CURSES -DNOGETUSERSHELL -DGTODONEARG $(KFLAGS) -O" \
	"LIBS= -lcurses -lresolv" "LNKFLAGS = -s"

#Motorola Delta System V/88 R40.  Has <sys/termiox.h>, regular Berkeley
#sockets library, i.e. in.h and inet.h are not misplaced in sys (rather than
#netinet and arpa, respectively).  Uses ANSI C constructs, advisory file
#locking on devices, etc.  curses support added.  Reportedly, the
#/usr/include/sys/vnode.h file has a bug which must be fixed before this
#makefile entry can work correctly.  The "if DEBUG" directive at about line
#320 must be changed to "ifdef DEBUG" (Reportedly, this was fixed in
#in System V/88 R4.3).
#After building, use "mcs -d" to reduce size of the executable program.
sv88r40:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/88 R40...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DMOTSV88R4 -DDIRENT -DHDBUUCP -DSTERMIOX \
	-DTCPSOCKET -DCK_CURSES -DNOGETUSERSHELL -DGTODONEARG -DFNFLOAT \
	$(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -lresolv -lm" "LNKFLAGS = -s"

#As above but without the floating-point math library.
sv88r40nm:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/88 R40...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS = -O -DSVR4 -DMOTSV88R4 -DDIRENT -DHDBUUCP -DSTERMIOX \
	-DTCPSOCKET -DCK_CURSES -DNOGETUSERSHELL -DGTODONEARG $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -lresolv" "LNKFLAGS = -s"

#As above but with floating-point math library support \ffp...() functions
#and S-Expressions.

#Olivetti X/OS R2.3, 3.x.
#NOTES:
# . If you build the executable on 2.x X/OS, it will also run on 3.x.
# . If you build it on 3.x X/OS, it will NOT run on 2.x.
# . Kermit can run with no privileges unless the uucp lines are protected,
#   in which case kermit must be owned by uucp with suid bit set:
#   chown uucp kermit ; chmod 4111 kermit.
xos23:
	@echo 'Making C-Kermit $(CKVER) for Olivetti X/OS...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	'CFLAGS=-OLM -DOXOS -DTCPSOCKET -DHDBUUCP $(KFLAGS)' \
	"LIBS=" "LNKFLAGS="

#As above, but with curses.
xos23c:
	@echo 'Making C-Kermit $(CKVER) for Olivetti X/OS with curses...'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} \
	'CFLAGS=-OLM -DOXOS -DTCPSOCKET -DHDBUUCP -DCK_CURSES $(KFLAGS)' \
	"LIBS=-lcurses" "LNKFLAGS="

ckuuid:
	@echo 'building C-Kermit $(CKVER) set-UID/set-GID test programs'
	$(CC) -DANYBSD -DSAVEDUID -o ckuuid1 ckuuid.c
	$(CC) -DANYBSD -o ckuuid2 ckuuid.c
	$(CC) -DANYBSD -DNOSETREU -o ckuuid3 ckuuid.c
	$(CC) -DANYBSD -DSETEUID -DNOSETREU -o ckuuid4 ckuuid.c
	$(CC) -o ckuuid5 ckuuid.c
	@echo 'Read the top of ckuuid.c for directions...for testing'
	@echo 'you must make these programs setuid and setgid'

############################################################################
# A N T I Q U I T I E S
#
# The following are antique targets from C-Kermit 5A or earlier.  They have
# not been updated or tested in years.  Most of them will need recent features
# disabled, usually with some combination of -DNOUNICODE, -DNOIKSD, -DNOANSI,
# -DNOCKGHNLHOST, -DNO_DNS_SRV, -DNOREDIRECT, -DNOREALPATH, -DNOCURSES, etc.
# They are also missing the KTARGET=$${KTARGET:-$(@)} business.
# For details see ckuins.txt and ckccfg.txt.
#
############################################################################

#Berkeley Unix 2.8, 2.9 for PDP-11s with I&D space, maybe also Ultrix-11???
#C-Kermit(5A) is simply too large (even turning off almost every feature
#available) to run without both I&D space plus overlays.  The old comment
#suggested running 'pcc' but that won't help.  Changing 'cc' to 'ckustr.sed'
#will cause a string extraction to be done, saving D space by moving strings
#to a file.
bsd29:
	@echo Making C-Kermit $(CKVER) for 2.8 or 2.9BSD.
	@echo Read the makefile if you have trouble with this...
	$(MAKE) ovwermit \
	"CFLAGS= -DBSD29 -DNODEBUG -DNOTLOG -DNOCSETS -DNOHELP \
	-DNOSCRIPT -DNOSPL -DNOXMIT -DNODIAL $(KFLAGS)" \
	"LNKFLAGS= -i -lndir" "CC= cc " "CC2= cc"

bsd210:
	@echo Please use ckubs2.mak to build C-Kermit $(CKVER) for 2.10BSD.

bsd211:
	@echo Please use ckubs2.mak to build C-Kermit $(CKVER) for 2.11BSD.

#Charles River Data Systems Universe with UNOS Version 9.2
crds:
	@echo 'Making C-Kermit $(CKVER) for Charles River Data Systems...'
	make xermit \
	"CFLAGS = -DATTSV -DNOANSI -DDIRENT -DLONGFN -DTCPSOCKET \
	-DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DNOSETREU \
	-Dsuspend=ksuspend $(KFLAGS) -O" "LNKFLAGS ="

#Microport SV/AT for IBM PC/AT 286 and clones, System V R2.
#The -O flag may fail on some modules (like ckuus2.c), in which case you
#should compile them by hand, omitting the -O.  If you get "hash table
#overflow", try adding -DNODEBUG.
#Also, reportedly this compiles better with gcc than with cc.
mpsysv:
	@echo 'Making C-Kermit $(CKVER) for Microport SV/AT 286...'
	$(MAKE) wermit \
	"CFLAGS= -DATTSV -DNOLEARN $(KFLAGS) -O -Ml" "LNKFLAGS = -Ml"

#Microsoft "Xenix/286" e.g. for IBM PC/AT
xenix:
	@echo 'Making C-Kermit $(CKVER) for Xenix/286'
	$(MAKE) wermit \
	"CFLAGS= -DXENIX -DNOFILEH -DNOLEARN $(KFLAGS) -Dunix -F 3000 -i" \
	"LNKFLAGS = -F 3000 -i"

#PC/IX, Interactive Corp System III for IBM PC/XT
pcix:
	@echo 'Making C-Kermit $(CKVER) for PC/IX...'
	$(MAKE) wermit \
	"CFLAGS= -DPCIX -DISIII -DNOLEARN $(KFLAGS) \
	-Dsdata=sdatax -O -i" "LNKFLAGS = -i"

#Integrated Solutions Inc V8S VME 68020
isi:
	@echo Making C-Kermit $(CKVER) for 4.2BSD on ISI...
	$(MAKE) wermit "CC = cc" \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DINADDRX -DDCLPOPEN -DDEBUG -DNOSETREU \
	-DCK_CURSES -DNOLEARN $(KFLAGS)" "LIBS = -lcurses -ltermcap"

#Interactive Corp version of AT&T System III
#is3: (very old, probably not sufficient for 5A or later)
#	@echo 'Making C-Kermit $(CKVER) for Interactive System III...'
#	make wermit "CFLAGS = -DISIII -Ddata=datax -O -i" "LNKFLAGS = -i"
#The following should work, use it if you don't have gcc.
#Use is3gcc if you have gcc.
is3:
	@echo 'Making C-Kermit $(CKVER) for Interactive System III...'
	$(MAKE) wermit \
	"CFLAGS= -DISIII $(KFLAGS) -Ddata=datax -DNAP -DHDBUUCP
	-DLOCK_DIR=\"/usr/spool/uucp\" -DSIGTYP=void -O -i" "LNKFLAGS = -i"

#Interactive UNIX System V R3, no network support.  Uses <dirent.h> and Honey
#DanBer UUCP.  If this entry does not compile correctly, try any or all of the
#following.  These suggestions also apply more or less to the other is5r3xxx
#entries that follow this one.
# . Remove the UID_T and GID_T definitions, or change them as required.
# . Change -DDIRENT to -DSDIRENT.
# . Add -DSIGTYP=void.
# . Remove -g from LNKFLAGS.
# . Add -DNOANSI to remove compiler complaints about ANSI C constructions
# . Add other -DNOxxx's to save space (e.g. -DNOCSETS)
# See the next few makefile entries for related examples.
# Also see sys5r32is for making a portable i386 SVR3 binary.
is5r3:
	@echo 'Making C-Kermit $(CKVER) for Interactive 386/ix or later...'
	@echo 'If this does not work please read the makefile entry.'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -g -DNOCSETS -DNOREALPATH \
	-DUID_T=ushort -DGID_T=ushort -DI386IX $(KFLAGS)" \
	"LNKFLAGS = -g"

#Interactive Corp System System V R3 with gcc
is3gcc:
	@echo 'Making C-Kermit $(CKVER) for Interactive System V R3 / gcc...'
	$(MAKE) wermit CC=gcc CC2=gcc \
	'CFLAGS = -D_SYSV3 -DISIII -Ddata=datax -DNAP -DHDBUUCP -DNOREALPATH \
	-DLOCK_DIR=\"/usr/spool/uucp\" -DSIGTYP=void -O' "LNKFLAGS ="

#Interactive UNIX System V R3, POSIX variant.  Untested.
#Uses dirent.h and Honey DanBer uucp.  Read comments in is5r3 entry.
is5r3p:
	@echo 'Making C-Kermit $(CKVER) for Interactive 386/ix or later...'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -g -DNOCSETS -DNOREALPATH \
	-DI386IX -DPOSIX $(KFLAGS)" "LNKFLAGS=" "LIBS=-lcposix"

#Interactive UNIX SVR3 2.2.1, job control, curses, no net, gcc.
is5r3gcc:
	$(MAKE) wermit CC=gcc CC2=gcc \
	"CFLAGS=-g -posix -DSVR3 -DDIRENT -DNOREALPATH \
	-DHDBUUCP -O -DNOCSETS -DI386IX -DSVR3JC -DCK_CURSES \
	$(KFLAGS)" LNKFLAGS="-posix" LIBS="-lcurses -lc_s"

#Interactive UNIX System V R3 with TCP/IP network support.
#Needs -linet for net functions.  signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp. Read comments in is5r3 entry.
#Also see is5r3net2 if you have trouble with this entry.
is5r3net:
	@echo 'Making C-Kermit $(CKVER) for Interactive 386/ix...'
	@echo 'If this does not work please read the makefile entry.'
	$(MAKE) wermit CC="$(CC)" CC2="$(CC2)" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DTCPSOCKET -DNOREALPATH \
	-DI386IX $(KFLAGS) -O" "LIBS = -linet"

is5r3netgcc:
	$(MAKE) is5r3net CC=gcc CC2=gcc

#Interactive UNIX System V R3, no job control, signal() void rather than int.
#Uses dirent.h and Honey DanBer uucp.  Needs -linet for net functions.
#Read comments in is5r3 entry.  Use this entry if is5r3net fails.
#Saves some space by stripping (-s) and using shared library (-lc_s).
is5r3net2:
	@echo 'Making C-Kermit $(CKVER) for Interactive 386/ix...'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DTCPSOCKET -DNOJC -DNOREALPATH \
	-DSIGTYP=void -DNOANSI -DI386IX $(KFLAGS) -O" \
	"LNKFLAGS= -s" "LIBS = -linet -lc_s"

#Interactive UNIX System V R3 (version 2.2 or later) with job control & curses.
#Uses dirent.h and Honey DanBer UUCP.
is5r3jc:
	@echo 'Making C-Kermit $(CKVER) for Interactive Unix 2.2 or later...'
	$(MAKE) wermit CC="$(CC)" CC2="$(CC2)" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -O -DNOCSETS -DNOREALPATH \
	-DUID_T=ushort -DGID_T=ushort -DI386IX -DSVR3JC -DCK_CURSES \
	-DPOSIX_JC -DCK_REDIR -DCK_POLL -DDCLGETCWD \
	$(KFLAGS)" "LIBS=-lcurses -lc_s -linet"

is5r3jcgcc:
	$(MAKE) is5r3jc CC="gcc -DCK_ANSILIBS -DDCGPWNAM -O4" CC2=gcc \
	KFLAGS="$(KFLAGS)" LNKFLAGS="$(LNKFLAGS)"

#Sunsoft/Interactive UNIX System V R3 (version 2.2 or later)
#with job control, curses, and TCP/IP networking.
#Uses dirent.h and Honey DanBer UUCP.
is5r3netjc:
	@echo 'Making C-Kermit $(CKVER) for Interactive Unix 2.2 or later...'
	$(MAKE) wermit CC="$(CC)" CC2="$(CC2)" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -O -DNOCSETS -DNOREALPATH \
	-DUID_T=ushort -DGID_T=ushort -DI386IX -DSVR3JC -DCK_CURSES \
	-DPOSIX_JC -DCK_REDIR -DTCPSOCKET -DSELECT \
	$(KFLAGS)" "LIBS=-linet -lcurses -lc_s"

is5r3netjcgcc:
	$(MAKE) is5r3netjc CC="gcc -DCK_ANSILIBS -DDCGPWNAM -O4" CC2=gcc \
	KFLAGS="$(KFLAGS)" LNKFLAGS="$(LNKFLAGS)"

#Masscomp System III
rtu:
	@echo 'Making C-Kermit $(CKVER) for Masscomp RTU System III...'
	$(MAKE) wermit \
	"CFLAGS= -UFIONREAD -DATTSV $(KFLAGS) -O" "LNKFLAGS =" "LIBS= -ljobs"

#Masscomp/Concurrent RTU 4.0 or later, Berkeley environment.
#Includes <ndir.h> = /usr/include/ndir.h
#Note "LIBS = -lndir" might not be necessary because of "ucb make".
rtubsd:
	@echo 'Making C-Kermit $(CKVER) for Masscomp RTU 4.1A...'
	ucb make wermit \
	"CFLAGS= -DBSD4 -DRTU -DNDIR -DHDBUUCP -DTCPSOCKET $(KFLAGS)" \
	"LIBS = -lndir"

#Masscomp/Concurrent RTU 4.0 or later, same as above,
#Includes "usr/lib/ndir.h"
#Note "LIBS = -lndir" might not be necessary because of "ucb make".
rtubsd2:
	@echo 'Making C-Kermit $(CKVER) for Masscomp RTU 4.1A...'
	ucb make wermit \
	"CFLAGS= -DBSD4 -DRTU -DXNDIR -DHDBUUCP $(KFLAGS)" \
	"LIBS = -lndir"

#Masscomp/Concurrent RTU 4.0 or later, same as above,
#Includes <sys/ndir.h>
#Note "LIBS = -lndir" might not be necessary because of "ucb make".
rtubsd3:
	@echo 'Making C-Kermit $(CKVER) for Masscomp RTU 4.x BSD...'
	ucb make wermit "CFLAGS= -DBSD4 -DRTU -DHDBUUCP $(KFLAGS)" \
	"LIBS = -lndir"

#Masscomp/Concurrent RTU 4.0 or later, System V R2, using <dirent.h>.
#In case of problems, add back the -DRTU switch.
#In case -DTCPSOCKET gives trouble, remove it.
rtus5:
	@echo 'Making C-Kermit $(CKVER) for Masscomp RTU 4.x...'
	$(MAKE) wermit \
	"CFLAGS= -DATTSV -DHDBUUCP -DDIRENT -DTCPSOCKET $(KFLAGS)"

#Masscomp/Concurrent RTU 4.x, System V R3, using <dirent.h>.
#Use this one if rtus5 gives warnings about pointer type mismatches.
#In case of problems, add back the -DRTU switch.
rtus5r3:
	@echo 'Making C-Kermit $(CKVER) for Masscomp RTU Sys V R3...'
	$(MAKE) wermit "CFLAGS= -DSVR3 -DHDBUUCP -DDIRENT $(KFLAGS)"

#DEC Pro-3xx with Pro/Venix V1.0 or V1.1
# Requires code-mapping on non-I&D-space 11/23 processor, plus some
# fiddling to get interrupt targets into resident code section.
# This almost certainly doesn't work any more.
provx1:
	@echo 'Making C-Kermit $(CKVER) for DEC Pro-3xx, Pro/Venix 1.x...'
	$(MAKE) wart "CFLAGS= -DPROVX1 $(KFLAGS)" "LNKFLAGS= "
	$(MAKE) wermit "CFLAGS = -DPROVX1 -DNOFILEH -md780" \
		"LNKFLAGS= -u _sleep -lc -md780"

#Nixdorf Targon/31.
#AT&T UNIX System V R3, signal() is void rather than int.
#Uses dirent.h without Honey DanBer uucp.
t31tos40x:
	@echo 'Making C-Kermit $(CKVER) for Targon/31 with TOS 4.0.xx...'
		$(MAKE) wermit \
		"CFLAGS= -DSVR3 -DDIRENT $(KFLAGS) -O" \
		"LNKFLAGS="

#NCR Tower 1632, OS 1.02
tower1:
	@echo 'Making C-Kermit $(CKVER) for NCR Tower 1632, OS 1.02...'
	$(MAKE) wermit "CFLAGS= -DTOWER1 $(KFLAGS)"

#NCR Tower 32, OS Release 1.xx.xx
tower32-1:
	@echo 'Making C-Kermit $(CKVER) for NCR Tower 32 Rel 1 System V R2...'
	@echo 'Add KFLAGS=-DISDIRBUG if you get errors about S_ISREG/S_ISDIR.'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV $(KFLAGS) -O" "LNKFLAGS = -n"

#NCR Tower 32, OS Release 2.xx.xx
tower32-2:
	@echo 'Making C-Kermit $(CKVER) for NCR Tower 32 Rel 2 System V R2...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DHDBUUCP $(KFLAGS) -O2" \
	"LNKFLAGS = -n"

#NCR Tower 32, OS Releases based on System V R3
#Don't add -DNAP (doesn't work right) or -DRDCHK (not available in libc).
tower32:
	@echo 'Making C-Kermit $(CKVER) for NCR Tower 32 System V R3...'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DNOSYSIOCTLH $(KFLAGS) \
	-DUID_T=ushort -DGID_T=ushort -O1"

#NCR Tower 32, OS Releases based on System V R3
tower32g:
	@echo 'Making C-Kermit $(CKVER) for NCR Tower 32 System V R3, gcc...'
	$(MAKE) wermit "CC = gcc" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DNOSYSIOCTLH $(KFLAGS) \
	DUID_T=ushort -DGID_T=ushort -O -fstrength-reduce -fomit-frame-pointer"

#Fortune 32:16, For:Pro 1.8 (mostly like 4.1bsd)
ft18:
	@echo 'Making C-Kermit $(CKVER) for Fortune 32:16 For:Pro 1.8...'
	$(MAKE) wermit \
	"CFLAGS= -DNODEBUG -DBSD4 -DFT18 -DNOFILEH $(KFLAGS) \
	-DPID_T=short"

#Fortune 32:16, For:Pro 2.1 (mostly like 4.1bsd).
#The modules that break the optimizer are compiled separately.
ft21:
	@echo 'Making C-Kermit $(CKVER) for Fortune 32:16 For:Pro 2.1...'
	$(MAKE) ckuusx.$(EXT) "CFLAGS= -DNODEBUG -DBSD4 -DFT21 -DNOFILEH \
	-SYM 800  -DCK_CURSES $(KFLAGS) -DPID_T=short" \
	"LNKFLAGS= -n -s" "LIBS= -lcurses -ltermcap -lv -lnet"
	$(MAKE) ckuxla.$(EXT) "CFLAGS= -DNODEBUG -DBSD4 -DFT21 -DNOFILEH \
	-SYM 800  -DCK_CURSES $(KFLAGS) -DPID_T=short" \
	"LNKFLAGS= -n -s" "LIBS= -lcurses -ltermcap -lv -lnet"
	$(MAKE) ckudia.$(EXT) "CFLAGS= -DNODEBUG -DBSD4 -DFT21 -DNOFILEH \
	-SYM 800  -DCK_CURSES $(KFLAGS) -DPID_T=short" \
	"LNKFLAGS= -n -s" "LIBS= -lcurses -ltermcap -lv -lnet"
	$(MAKE) wermit \
	"CFLAGS= -O -DNODEBUG -DBSD4 -DFT21 -DNOFILEH -SYM 800 \
	-DCK_CURSES $(KFLAGS) -DPID_T=short" \
	"LNKFLAGS= -n -s" "LIBS= -lcurses -ltermcap -lv -lnet"

#Valid Scaldstar
#Berkeleyish, but need to change some variable names.
valid:
	@echo 'Making C-Kermit $(CKVER) for Valid Scaldstar...'
	$(MAKE) wermit \
	"CFLAGS= -DBSD4 -DNODEBUG -DNOTLOG -Dcc=ccx -DFREAD=1 $(KFLAGS)"

#IBM IX/370 on IBM 370 Series mainframes
#Mostly like sys3, but should buffer packets.
ix370:
	@echo 'Making C-Kermit $(CKVER) for IBM IX/370...'
	$(MAKE) wermit "CFLAGS = -DIX370 -DATTSV $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#Amdahl UTS 2.4 on IBM 370 series compatible mainframes.
#Mostly like V7, but can't do initrawq() buffer peeking.
uts24:
	@echo 'Making C-Kermit $(CKVER) for Amdahl UTS 2.4...'
	$(MAKE) wermit "CFLAGS=-DV7 -DPROCNAME=\\\"$(PROC)\\\" \
	-DUTS24 -DBOOTNAME=\\\"$(BOOTFILE)\\\" -DNPROCNAME=\\\"$(NPROC)\\\" \
	-DNPTYPE=$(NPTYPE) $(DIRECT) $(KFLAGS)"

#Amdahl UTSV UNIX System V = System V R2 or earlier.
utsv:
	@echo 'Making C-Kermit $(CKVER) for Amdahl UTSV...'
	$(MAKE) wermit \
	"CFLAGS = -DUTSV $(KFLAGS) -i -O" "LNKFLAGS = -i"

#Amdahl UTSV UNIX System V = System V R2 or earlier, with TCP sockets library.
utsvtcp:
	@echo 'Making C-Kermit $(CKVER) for Amdahl UTSV w/tcp...'
	$(MAKE) wermit "CFLAGS = \
	-DTCPSOCKET -DUTSV $(KFLAGS) -i -O" "LNKFLAGS = -i" \
	"LIBS = -lsocket"

#BBN C/70 with IOS 2.0
#Mostly Berkeley-like, but with some ATTisms
c70:
	@echo 'Making C-Kermit $(CKVER) for BBN C/70 IOS 2.0...'
	$(MAKE) wermit "CFLAGS= -DBSD4 -DC70 $(KFLAGS)"

#Zilog ZEUS 3.21
zilog:
	@echo 'Making C-Kermit $(CKVER) for Zilog Zeus 3.21...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DZILOG -DNODEBUG $(KFLAGS) -i -O" \
	"LNKFLAGS = -i -lpw"

#Whitechapel MG-1 Genix 1.3
white:
	@echo 'Making C-Kermit $(CKVER) for Whitechapel MG-1 Genix 1.3...'
	@touch ckcpro.c
	$(MAKE) wermit "CFLAGS= -DBSD4 -Dzkself()=0  $(KFLAGS)"

#Pixel 1000
pixel:
	@echo 'Making C-Kermit $(CKVER) for Pixel 1000...'
	$(MAKE) wermit "CFLAGS= -DBSD4 -Dzkself()=0 $(KFLAGS)"

ptx:
	$(MAKE) "MAKE=$(MAKE)" dynixptx12

#CDC VX/VE 5.2.1
vxve:
	@echo 'Making C-Kermit $(CKVER) for CDC VX/VE 5.2.1...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DVXVE -DNODEBUG -DNOTLOG $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#DIAB DS90 or LUXOR ABC-9000 with pre-5.2 DNIX.  Sys V with nap() and rdchk().
# nd = no opendir(), readdir(), closedir(), etc.
# Some of the modules fail to compile with -O.
dnixnd:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with very old DNIX 5.2.'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNAP -DRDCHK -DDCLPOPEN \
	-U__STDC__ $(KFLAGS)"

#DIAB DS90 with DNIX 5.2.  Sys V with nap() and rdchk().
# This one has opendir(), readdir(), closedir(), etc.
# Some of the modules fail to compile with -O.
dnix:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with old DNIX 5.2...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNAP -DRDCHK -DDIRENT  \
	-U__STDC__ $(KFLAGS)"

#DIAB DS90 with DNIX 5.2.  Sys V with nap() and rdchk().
# As above, but with curses and TCP/IP.
# You might get complaints about redefinition of O_RDONLY, etc, because
# of bugs in the DNIX header files, which can be fixed by adding #ifndef...
# around the offending definitions in the header files.
dnixnetc:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with old DNIX 5.2...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNAP -DRDCHK -DDIRENT  \
	-DTCPSOCKET -DCK_CURSES -I/usr/include/bsd -U__STDC__ $(KFLAGS)" \
	"LIBS = -ln -lcurses"

#DIAB DS90 with DNIX 5.3 or later, with HDB UUCP, nap() and rdchk().
dnix5r3:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with DNIX 5.3...'
	@echo 'with Honey DanBer UUCP'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DHDBUUCP -DNAP -DRDCHK -DDIRENT \
	-DCK_CURSES -DRENAME $(KFLAGS) -O" "LIBS= -lcurses"

#DIAB DS90 with DNIX 5.3 or later, with HDB UUCP, nap() and rdchk() + TCP/IP
dnix5r3net:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with DNIX 5.3...'
	@echo 'with Honey DanBer UUCP and TCP/IP'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DHDBUUCP -DNAP -DRDCHK -DDIRENT \
	-DTCPSOCKET -DCK_CURSES -DRENAME $(KFLAGS) -O \
	-I/usr/include/bsd" "LIBS = -ln -lcurses"

#DIAB DS90 with DNIX 5.3 2.2 or later, with HDB UUCP, nap() and rdchk(),
#ANSI C compilation and libraries.
#Note that for DNIX 5.3 2.2 you have to correct a bug in /usr/include/stdlib.h:
#change "extern	void free(char *str);"
#to     "extern void free(void *str);"
#NOTE: This bug is reportedly fixed in DNIX 5.3 2.2.1.
#Should you get fatal errors caused by harmless pointer-type mismatches,
#like between signed and unsigned char, just remove -X7.
dnix5r3ansi:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with DNIX 5.3...'
	@echo 'with ANSI C Honey DanBer UUCP'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIAB -DHDBUUCP -DNAP -DRDCHK -DDIRENT \
	-DCK_ANSILIBS -DCK_CURSES -DRENAME -O -X7 -X9 $(KFLAGS)" \
	"LIBS= -lcurses"

#DIAB DS90 with DNIX 5.3 2.2 or later, with HDB UUCP, nap() and rdchk(),
# + TCP/IP, ANSI C compilation and libraries.
#Should you get fatal errors caused by harmless pointer-type mismatches,
#like between signed and unsigned char, just remove -X7.
dnix5r3ansinet:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with DNIX 5.3...'
	@echo 'with ANSI C Honey DanBer UUCP'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIAB -DHDBUUCP -DNAP -DRDCHK -DDIRENT \
	-DTCPSOCKET -DCK_ANSILIBS -DCK_CURSES -DRENAME -O -X7 -X9 $(KFLAGS) \
	-I/usr/include/bsd" "LIBS= -ln -lcurses"

#Ridge 32 with ROS 3.2
ridge32:
	@echo 'Making C-Kermit $(CKVER) Ridge 32 ROS 3.2'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNOFILEH -DNODEBUG -DNOTLOG $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#Altos 486, 586, or 986 with Xenix 3.0
altos:
	@echo 'Making C-Kermit $(CKVER) for Altos x86 with Xenix 3.0...'
	$(MAKE) wermit \
	"CFLAGS= -DATTSV -DA986 -DNODEBUG -DNOTLOG $(KFLAGS) -i -O" \
	"LNKFLAGS= -i"

#Altos 986 with Xenix 3.0, as above, but command-line only, minimal size.
#For systems with small memories.  It might also be necessary to chop certain
#modules up into smaller pieces, e.g. ckuus3-6, because of symbol table
#overflow.   If this makefile is too big or complex for the Altos, compile
#and link by hand or write shell scripts.
altosc:
	@echo 'Making C-Kermit $(CKVER) for Altos x86 Xenix 3.0, remote...'
	$(MAKE) wermit \
	"CFLAGS= -DATTSV -DA986 -DNODEBUG -DNOTLOG -DNOSCRIPT -DNODIAL \
	-DNOCSETS -DNOANSI -DNOMSEND -DNOSPL -DNOICP $(KFLAGS) -Mm -O" \
	"LNKFLAGS= -Mm -s"

#Altos 986 with Xenix 3.0, as above, but interactive only, minimal size.
altosi:
	@echo 'Making C-Kermit $(CKVER) for Altos x86 Xenix 3.0, local...'
	$(MAKE) wermit \
	"CFLAGS= -DATTSV -DA986 -DNODEBUG -DNOTLOG -DNOSCRIPT -DNODIAL \
	-DNOCSETS -DNOANSI -DNOMSEND -DNOSPL -DNOCMDL -DNOFRILLS -DNOHELP \
	-DNOSETKEY $(KFLAGS) -Mm -O" "LNKFLAGS= -Mm -s"

# Altos ACS68000 68000 System, UNIX System 3 Release 2, 512k memory.
# also needs getcwd() external function; see ckuins.txt file.
# also, sys/types.h needed modifying:
#   #ifdef __SYS_TYPES_H__, #define ..., #endif
# also, ckuus2.c MUST be compiled NOOPT else symbol table is destroyed!
# Submission by Robert Weiner/Programming Plus, rweiner@progplus.com.
#
altos3:
	@echo 'Making C-Kermit $(CKVER) for Altos ACS68k UNIX System III'
	$(MAKE) ckuus2.$(EXT) "CFLAGS = -DATTSV -DNOCSETS -DNOSETKEY -DNOJC \
	-DNODIAL -DDCLPOPEN -DNOSCRIPT -DNOHELP $(KFLAGS) -i"
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNOCSETS -DNOSETKEY -DNOJC \
	-DNODIAL -DDCLPOPEN -DNOSCRIPT -DNOHELP $(KFLAGS) -i -O" \
	"LNKFLAGS = -i" "LIBS = getcwd.$(EXT)"

#MINIX - Original PC version with 64K+64K limit.
# Reportedly, the linker (asld) can run out of space while linking.  The only
# way around this is to make a copy of libc.a from which all modules that are
# not used by Kermit are removed.  If you have trouble compiling or running
# wart, "touch wart".  If that doesn't help, "touch ckcpro.c".
# The version configured below has no interactive command parser.
# If you can build this version successfully, maybe there will be room for
# a minimal interactive command parser too; try replacing -DNOICP with
# -DNOSPL, plus every other -DNOxxx flag there is, except for -DNOICP
# (see ckccfg.txt).
minix:
	@echo 'Making C-Kermit $(CKVER) for MINIX, no command parser...'
	@echo 'TOTALLY UNTESTED!'
	$(MAKE) wermit EXT=s \
	"CFLAGS= -DV7 -DMINIX -i -D_MINIX -D_POSIX_SOURCE \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V \
	-DNOXMIT -DNOMSEND -DNOFRILLS -DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOICP -DNOSETKEY $(KFLAGS)" \
	"LNKFLAGS= -i -T"

#MINIX - PC version with 64K+64K limit, new (as yet unreleased) ACK 2.0 beta C
#compiler, which outputs .o object files, rather than .s.  But 'make' still
#expects .s files, so must be patched to use .o.  Tested on Minix 1.5.10.
minix15:
	@echo 'Making C-Kermit $(CKVER) for MINIX (new ACK 2.0 compiler),'
	@echo 'no command parser...  TOTALLY UNTESTED!'
	$(MAKE) wermit \
	"CFLAGS= -DV7 -DMINIX -i -D_MINIX -D_POSIX_SOURCE \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V -DNODIAL \
	-DNOHELP -DNODEBUG -DNOTLOG -DNOSCRIPT -DNOCSETS -DNOICP $(KFLAGS)" \
	"LNKFLAGS= -i -T"

#MINIX3 - MINIX 3.0 (no VM) - May-Aug 2005 (not sure if this ever worked...)
minix3:
	@echo 'Making C-Kermit $(CKVER) for MINIX3...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DPOSIX -DNOUUCP -DNOLEARN $(KFLAGS) -DMINIX2 \
	-DMINIX3 -DNO_PARAM_H -DNOSYSLOG -DNOGETUSERSHELL \
	-DNOINITGROUPS -DNOFTRUNCATE -DNOARROWKEYS -DDNOREALPATH \
	-DTCPSOCKET -DNOTIMEZONE -DNOFTP -DNO_DNS_SRV -O"

#MINIX315 - MINIX 3 1.5 - January 2010
minix315:
	@echo 'Making C-Kermit $(CKVER) for Minix 3 1.5...'
	$(MAKE) wermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DMINIX315 -DPOSIX -DNOUUCP -DNOJC -DNOLEARN $(KFLAGS) \
	-DHAVE_OPENPTY -DNO_PARAM_H -DNOSYSLOG -DNOGETUSERSHELL \
	-DSYSTIMEH -DNOINITGROUPS -DNOFTRUNCATE -DNOARROWKEYS -DNOREALPATH \
	-DTCPSOCKET -DNOTIMEZONE -DNO_DNS_SRV -DNOFTP -O"

#PFU Compact A Series UNIX System V R3, SX/A TISP V10/L50 (Japan)
#Maybe the -i link option should be removed?
sxae50:
	@echo 'Making C-Kermit $(CKVER) for PFU SX/A V10/L50...'
	$(MAKE) xermit \
	"CFLAGS= -DSVR3 -DDIRENT -DsxaE50 -DTCPSOCKET $(KFLAGS) -i -O" \
	"LNKFLAGS= "

#Tektronix 6130, 4319, 4301, etc, with UTek OS, /usr/spool/uucp/LCK./...
#The models that support hardware flow control.
utek:
	@echo 'Making C-Kermit $(CKVER) for 4.2BSD/UTek, hardware flow control'
	$(MAKE) wermit \
	"CFLAGS= -O -DLCKDIR -DBSD4 -DTCPSOCKET \
	-DUTEK -DDCLPOPEN -DLOCK_DIR=\\\"/usr/spool/uucp/LCK.\\\" \
	-DTRMBUFL=2048 -DCK_DTRCTS $(KFLAGS)"

#Tektronix 4315, 4316, 4317 with UTek OS, /usr/spool/uucp/LCK./...
#The models that do not fully support hardware flow control.
uteknohwfc:
	@echo 'Making C-Kermit $(CKVER) for 4.2BSD/UTek, no h/w flow control'
	$(MAKE) wermit \
	"CFLAGS= -O -DLCKDIR -DBSD4 -DTCPSOCKET \
	-DUTEK -DDCLPOPEN -DLOCK_DIR=\\\"/usr/spool/uucp/LCK.\\\" \
	-DTRMBUFL=2048 $(KFLAGS)"

#Tektronix XD88 with  UTekV OS
utekvr3:
	@echo 'Making C-Kermit $(CKVER) for Tektronix XD88 UTekV R3...'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP \
	-DTCPSOCKET -DSYSUTIMEH -DCK_CURSES $(KFLAGS) -O" \
	"LIBS= -lcurses" "LNKFLAGS= -s"

#Perkin-Elmer 3200 Xelos R02 or earlier
ccop1:
	@echo 'Making C-Kermit $(CKVER) for Xelos & Public Domain Dirent calls'
	@echo 'or System V R2 or earlier...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -Dvoid=int -DDIRENT -DCK_CURSES \
	$(KFLAGS) -O" "LNKFLAGS =" "LIBS= -lcurses -ltermlib"

#Encore, UMAX 4.3 (BSD) but without acucntrl program.
encore:
	$(MAKE) "MAKE=$(MAKE)" umax43 "KFLAGS=$(KFLAGS)"

#Encore, as above, but with curses file transfer display included.
encorec:
	$(MAKE) "MAKE=$(MAKE)" umax43 "KFLAGS=-DCK_CURSES $(KFLAGS)" \
	"LIBS= -lcurses -ltermcap"

#Encore, UMAX 4.3 (BSD) but without acucntrl program.
umax43:
	@echo Making C-Kermit $(CKVER) for Encore UMAX 4.3...
	$(MAKE) "MAKE=$(MAKE)" PARALLEL=4 xermit \
	"CFLAGS= -DBSD43 -DENCORE -DTCPSOCKET $(KFLAGS) -O"

#Encore, UMAX 4.2 (BSD)
umax42:
	@echo Making C-Kermit $(CKVER) for Encore UMAX 4.2...
	$(MAKE) "MAKE=$(MAKE)" PARALLEL=4 xermit \
	"CFLAGS= -DBSD4 -DENCORE -DTCPSOCKET $(KFLAGS) -O"

#Encore 88K UMAX 5.3 with TCP/IP support
encore88k:
	@echo 'Making C-Kermit $(CKVER) for Encore 88K UMAX V, TCP/IP...'
	$(MAKE) xermit \
	"CFLAGS = -q ext=pcc -DSVR3 -DTCPSOCKET -DDIRENT \
	-DNOGETID_PROTOS -DHDBUUCP $(KFLAGS) -O" "LNKFLAGS ="

#Encore 88K UMAX 5.3 with TCP/IP support
encore88kgcc:
	@echo 'Making C-Kermit $(CKVER) for Encore 88K UMAX V, TCP/IP, gcc...'
	$(MAKE) xermit CC=gcc CC2=gcc \
	"CFLAGS = -DSVR3 -DTCPSOCKET -DDIRENT \
	-DNOGETID_PROTOS -DHDBUUCP $(KFLAGS) -O" "LNKFLAGS ="

#SONY NEWS, NEWS-OS 4.01C
sonynews:
	@echo Making C-Kermit $(CKVER) for SONY NEWS-OS 4.01C...
	$(MAKE) xermit "CFLAGS= -DBSD43 -DACUCNTRL -DTCPSOCKET -O"

#Run Lint on this mess for selected versions.
#These are pretty much obsolete since ANSI C / gcc.
lintsun:
	@echo 'Running Lint on C-Kermit $(CKVER) sources for SunOS version...'
	lint -x -DSUNOS4 -DDIRENT -DTCPSOCKET -DSAVEDUID \
	ck[cu]*.c > ckuker.lint.sun

lintbsd:
	@echo 'Running Lint on C-Kermit $(CKVER) sources for BSD 4.2 version..'
	lint -x -DBSD4 -DTCPSOCKET ck[cu]*.c > ckuker.lint.bsd42

lints5:
	@echo 'Running Lint on C-Kermit $(CKVER) sources for Sys V version...'
	lint -x -DATTSV ck[cu]*.c > ckuker.lint.s5

#Who remembers TECO?
love:
	@echo 'Not war?'
