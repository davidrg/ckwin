# CKUKER.MAK / ckuker.mak / makefile / Makefile
# Fri Sep  6 23:23:25 1996
#
CKVER= "6.0.192"
#
# -- Makefile to build C-Kermit for UNIX and UNIX-like systems --
#
# Author: Frank da Cruz, Columbia University
# 612 West 115th Street, New York NY 10025-7799, USA.
# E-mail: fdc@columbia.edu
# Fax:    +1 212 662-6442.
# Web:    http://www.columbia.edu/kermit/
#
# Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New
# York.  The C-Kermit software may not be, in whole or in part, licensed or
# sold for profit as a software product itself, nor may it be included in or
# distributed with commercial products or otherwise distributed by commercial
# concerns to their clients or customers without written permission of the
# Office of Kermit Development and Distribution, Columbia University.  This
# copyright notice must not be removed, altered, or obscured.
#
# CAREFUL: Don't put the lowercase word "if", "define", or "end" as the first
# word after the "#" comment introducer in the makefile, even if it is
# separated by whitespace.  Some versions of "make" understand these as
# directives.  Uppercase letters remove the danger, e.g. "# If you have..."
#
# WARNING: This is a huge makefile, and it contains nested makes.
# Some "make" programs run out of memory.  If this happens to you, edit
# away the parts that do not apply to your system and try again.
#
# For 2.10 or 2.11 BSD on DEC PDP-11s, use the separate makefile, ckubs2.mak.
# For Plan 9, use ckpker.mk.
#
#   C-Kermit can also be built for other systems not supported by this
#   makefile, including VAX/VMS and OpenVMS, Data General AOS/VS, OS/2, the
#   Apple Macintosh, Stratus VOS, Apollo Aegis, the Commodore Amiga, OS-9, and
#   the Atari ST.  They have their own separate build procedures.  See the
#   file CKAAAA.HLP for information.
#
# INSTALLATION NOTES:
#
# Rename this file to "makefile" or "Makefile" if necessary.  Pick out the
# entry most appropriate for your UNIX system from the list below and
# then give the appropriate "make" command, for example "make bsd", "make
# sys5r4", "make posix".  The make targets, hundreds of them, are listed
# below.  If you experience any difficulties with the build procedure, then
# please also read any comments that accompany the make entry itself (search
# for the make entry name on the left margin).
#
# For more detailed installation instructions, read the files ckuins.doc and
# ckccfg.doc.  For descriptions of known problems and limitations, read the
# files ckcker.bwr and ckuker.bwr (the "beware files").
# 
# Most entries build C-Kermit with its symbol table included.  To reduce the
# size of the executable program, add "LNKFLAGS=-s" to the end of your 'make'
# command or to the makefile entry, or 'strip' the executable after
# building.  To further reduce the size after building, use 'mcs -d' if your
# system has such a command.  For further details on size reduction, read
# ckccfg.doc to find out how to remove unneeded features.
#
# TCP/IP networking support: If your C-Kermit version does not include TCP/IP
# networking, but your UNIX system does, try adding -DTCPSOCKET to the CFLAGS
# of your makefile entry.  If that doesn't work, look at some of the other
# entries that include this flag for ideas about what libraries might need to
# be included, etc.  NOTE: In some cases (certain versions of SCO or HP-UX),
# you might need not only a C compiler, but also a "TCP/IP developers kit"
# for the required object libraries and header files.
#
# Fullscreen file transfer display support: If you are going to use C-Kermit
# for establishing connections (dialed, network, etc), you can configure it 
# to produce a formatted file transfer display by including the curses library
# and adding -DCK_CURSES to the CFLAGS for your option, and linking with the
# appropriate libraries.  There are many examples below, usually ending in
# "c", like rs6000c, du42c, sunos41c, etc.  Also add -DCK_WREFRESH if your
# curses library includes clearok() and wrefresh() functions (or remove
# -DNOWREFRESH if the linker complains that it can't find these functions).
# After building, you still have to SET FILE DISPLAY FULLSCREEN to get the
# formatted screen display.
#
# Please report modifications, successes, failures (preferably with fixes) or
# successes to the author.
#
# MAKE COMMANDS FOR DIFFERENT UNIX PLATFORMS AND VERSIONS:
#
# + Marks those that have been tested successfully with C-Kermit 5A or later.
# - Marks those that are known not to work in version 5A or later.
# ? Marks those as yet untested in version 5A or later.  Some of these have 
#     never been tried, others are known to have worked in C-Kermit version 4
#     (1985-1989), which was considerably smaller, but have not been tested
#     in version 5A.
#
# + for 386BSD (Jolix) 0.0, 0.1, "make 386bsd" (see comments in entry),
#     or (preferably, if it works) "make bsd44" or "make bsd44c".
# + for Acorn RISCiX, "make riscix" or "make riscixgcc"
# + for Alliant FX/8 with Concentrix 4.1 or later, "make bsdlck"
# + for Altos 486, 586, 986 with Xenix 3.0, "make altos"
# + for Altos ACS68000, 8Mhz 68000, UNIX System 3 Rel 2, 512K, "make altos3"
# ? for Amdahl UTS 2.4 on IBM 370 series & compatible mainframes, "make uts24"
# + for Amdahl UTSV IBM 370 series & compatible mainframes, "make utsv"
# + for Amdahl UTSV IBM 370 series mainframes with TCP/IP, "make utsvtcp"
# + for Amdahl mainframes with UNIX System V R 5.2.6b 580, "make sys3"
# + for Apollo Aegis 9.x, DOMAIN/IX 9.x, "make aegis"
# ? for Apollo DOMAIN/IX, if the above fails, try "make apollobsd"
# + for Apollo with SR10.0 or later, BSD environment, "make sr10-bsd"
# + for Apollo with SR10.0 or later, System V environment, "make sr10-s5r3"
# + for Apple Macintosh II with A/UX pre-3.0, "make aux", "auxgcc" or "auxufs"
# + for Apple Macintosh with A/UX 3.0 and gcc, "make aux3gcc" or aux3gccc
# ? for Apple Macintosh with Minix 1.5.10, "make minix68k" or "make minixc68"
# ? for Arix System 90 with AT&T SVR3, "make sys5r3na"
# ? for AT&T 6300 with IN/ix, "make sys5"
# + for AT&T 6300 PLUS, "make att6300" or (with no debugging) "make att6300nd"
# + for AT&T 6386 WGS UNIX PC, "make sys5r3"
# + for AT&T 3B2, 3B20 systems, "make att3bx" or "make att3bxc"
# + for AT&T 3B1, 7300 UNIX PC, "make sys3upc", "sys3upcg" (gcc), "sys3upcold",
#     or (for fullscreen curses display) "make sys3upcc", "sys3upcgc" (gcc)
#        or "make sys3upcx" (see entries for more explanation),
#     or (for fullscreen curses display and shared library) "make sys3upcshcc"
#     or for minimum-size interactive version for systems with small memories
#       "make sys3upcm" or (with gcc) "make sys3upcgm".
# + for AT&T System III/System V R2 or earlier, "make sys3" or "make sys3nid"
# + for AT&T System III/System V with Honey DanBer UUCP, "make sys3hdb"
# + for AT&T System V on DEC VAX, "make sys3" or "make sys5r3"
# + for AT&T System V R3, use "make sys5r3" or "make sys5r3c"
# + for AT&T System V/386 R320.0 Versyss Systems, use "make sys5r3"
#     or "make sys5r3c" with KFLAGS=-DNONAWS.
# + for AT&T System V R4, "make sys5r4", "make sys5r4sx", or "make sys5r4nx",
#     or if the ANSI C function prototyping makes trouble, add -DNOANSI,
#     as in "sys5r4sxna" entry
# + for AT&T System V R4 with bundled TCP/IP, "make sys5r4netc", ...
# + for AT&T System V R4 with Wollongong TCP/IP, "make sys5r4twg", ...
# + for AT&T (USL) System V R4.2 use the sys5r4* entries.
# + for Atari Falcon with MiNT, "make posix"
# + for Atari ST with Minix ST 1.5.10.3, "make minix68k" or "make minixc68"
# ? for BBN C/70 with IOS 2.0, "make c70"
# + for BeBox with Be OS 1.x DR7, "make beboxdr7"
#     Compiles OK but doesn't link with default linker which is limited to 64K.
#     Links OK with "Code Warrior Gold".  Many hacks in the source code need
#     to be removed when DR8 and later come out.
# + for BeBox with Be OS 1.x DR8, "make bebox"
# ? for Bell Labs UNIX Version 6 (6th Edition), there is no makefile entry.
# + for Bell Labs UNIX Version 7 (7th Edition), "make v7" (but see notes below)
# ? for Bell Labs Research UNIX Version 8,  "make bellv10"
# ? for Bell Labs Research UNIX Version 9,  "make bellv10"
# + for Bell Labs Research UNIX Version 10, "make bellv10"
# + for Bell Labs / Lucent Plan 9, use separate makefile ckpker.mk:
#     can be built for Intel, MIPS, 680x0, and PowerPC.
# + for BSDI/386 1.x, "make bsdi"
# + for BSDI/386 2.x, "make bsdi"
# + for Berkeley Unix 2.4, "make v7" (but read v7 material below)
# ? for Berkeley Unix 2.9 (DEC PDP-11 or Pro-3xx), "make bsd29"
# + for Berkeley Unix 2.10, use ckubs2.mak (a separate makefile)
# + for Berkeley Unix 2.11, use ckubs2.mak (a separate makefile)
#     This makefile is too big.  Read the instructions in ckubs2.mak.
#     Rename ckubs2.mak to makefile, then "make bsd210" or "make bsd211".
# + for Berkeley Unix 4.1, "make bsd41"
# + for Berkeley Unix 4.2, "make bsd" (tested with 4.2 and 4.3)
# + for Berkeley Unix 4.2 or 4.3 with HoneyDanBer UUCP, "make bsdhdb"
# + for Berkeley Unix 4.3, "make bsd43" (uses acucntrl program for locks)
# + for Berkeley Unix 4.3 without acucntrl program, "make bsdlck" or "make bsd"
# ? for Berkeley Unix 4.3-Tahoe, same as 4.3 BSD
# + for Berkeley Unix 4.3-Reno, "make bsd43" or "make bsd44" or "make bsd44c"
# + for Berkeley Unix 4.3-Carson City, "make bsd44" or "make bsd44c"
# + for Berkeley Unix 4.4-Networking/2 or -Alpha, "make bsd44" or "make bsd44c"
# + for Berkeley Unix 4.4, "make bsd44" or "make bsd44c"
# + for Berkeley Unix 4.4-Lite, "make bsd44" or "make bsd44c"
# + for Bull DPX/2 with BOS/X, "make bulldpx2"
# ? for Cadmus, "make sys3"
# ? for Callan Unistar, "make sys3"
# ? for CDC VX/VE 5.2.1 System V emulation, "make vxve"
# + for Charles River Data Systems Universe 680x0 with UNOS 9.2, maybe
#     also other UNOS versions, "make crds"
# ? for CIE Systems 680/20 with Regulus, "make cie"
# + for Commodore Amiga 3000UX Sys V R4, "make sys5r4sx"
# + for Commodore Amiga 3000UX Sys V R4 and TCP/IP, "make svr4amiganet"
# ? for Commodore Amiga with Minix 1.5.10, "make minix68k" of "make minixc68"
# + for Concurrent/Masscomp with RTU 4.0 or later, BSD environment, "make
#     rtubsd", "make rtubsd2", "make rtubsd3" (depending on where ndir.h
#     is stored, see entries below).
# ? for Concurrent/Masscomp with RTU 4.0 or later, System V R2, "make rtus5"
# + for Concurrent/Masscomp with RTU 5.0 or later, System V R3, "make rtusvr3"
# + for Concurrent (Perkin-Elmer) 3200 series, "make sys5".
# + for Concurrent (Perkin-Elmer) 3200 series with <dirent.h>, "make ccop1"
# + for Consensys UNIX SV/386 R4V3, "make sys5r4sxtcpc" or "make sys5r4sx"
# ? for Convergent with CTIX Sys V R2, "make sys5"
# + for Convergent with CTIX 6.4.1, "make ctix"
# + for Convex C1, "make convex"
# + for Convex C210 with Convex/OS 8, "make convex8"
# + for Convex C2 with Convex/OS 9.1, "make convex9"
# + for Convex C2 with Convex/OS 10.1 and gcc 2.x, "make convex10gcc"
# + for Cray Research X/MP or YMP or C90 with UNICOS 6.x (System V R3),
#	"make cray"
# + for Cray Research X/MP or YMP or C90 with UNICOS 7.x (System V R4),
#	"make cray"
# + for Cray Research X/MP or YMP or C90 with UNICOS 8.0 Alpha, "make cray8"
# + for Cray Computer Cray-2 or Cray3 with CSOS, "make craycsos"
# + for Cyber 910 (Silicon-Graphics Iris) with Irix 3.3, "irix33"
# + for Data General AViiON with DG/UX 5.4, "make dgux540"
#     or "make dgux540c" (compile ckwart separately if necessary)
# + for Data General AViiON with DG/UX 5.4R3.00, "make dgux543c"
# + for DG/UX 5.4 R4.11 on AViiON Intel models, "make dgux544i" or dgux544ic.
# + for Data General AViiON with DG/UX 4.3x using Sys V-isms, "make dgux430"
# ? for Data General AViiON with DG/UX 4.3x using BSD-isms, "make dgux430bsd"
# ? for Data General AViiON, earlier UNIX versions,
#     "make sys5r3" (maybe compile ckwart separately, or "touch ckcpro.c")
# ? for Data General MV systems with DG/UX, ???
# + for Data General MV systems with MV/UX, use AOS/VS C-Kermit (CKDKER.MAK)
# + for Data General MV systems with AOS/VS, use CKDKER.MAK.
# + for DEC PDP-11 with Berkeley UNIX 2.x, see Berkeley UNIX 2.x.
# ? for DEC PDP-11 with Mini-UNIX (Bell 6th Edition for PDP-11 with no MMU),
#     probably no way to fit C-Kermit without I&D space.
# ? for DEC PDP-11 with Ultrix-11 3.x, ??? (probably needs overlays)
# + for DEC VAX with Ultrix 1.x "make bsd"
# + for DEC VAX with Ultrix 2.x "make ultrix2s"
# + for DEC VAX or DECstation with Ultrix 3.0, 3.1, "make ultrix3x"
# + for DECstation or VAX with Ultrix 4.0 or 4.1, "make ultrix40"
# + for DECstation or VAX with Ultrix 4.2 or later, "make ultrix42" or
#     "make ultrix42c"
# + for DECstation 5000/50, /150 or /260 (R4x00 MIPS CPU), Ultrix 4.3A or later
#     "make ultrix43-mips3" or "make ultrix43c-mips3"
# + for DECstation or VAX with Ultrix 4.2, Sys V R4 world, "make du42s5r4"
# + for DECstation or VAX with Ultrix 4.x, POSIX world, "make posix"
# + for DECstation with Ultrix 4.3, "make ultrix42"
# + for DECstation 5000/50 or /150 or /260 (R4x00 MIPS CPU), Ultrix 4.3A/4.4
#     "make ultrix43c-mips3" or "make ultrix44-mips3"
# ? for DECstation (MIPS) with Berkeley Sprite, "make bsd44"?
# + for DECstation (MIPS) with OSF/1 V1.0 to 1.3, "make dec-osf"
# + for DEC Alpha with OSF/1 1.0 to 1.3, "make dec-osf"
# + for DEC PC 486 with OSF/1, "make dec-osf"
# + for DEC Alpha with OSF/1 2.x, "make dec-osf20"
# + for DEC Alpha with OSF/1 3.0, "make dec-osf30"
# + for DEC Alpha with Digital UNIX 3.2, "make du32"
# + for DEC Alpha with Digital UNIX 4.0, "make du40"
# - for DEC Pro-350 with Pro/Venix V1.x, "make provx1" (version 5A is too big)
# ? for DEC Pro-350 with Pro/Venix V2.0 (Sys V), "make sys3nid"
# ? for DEC Pro-380 with Pro/Venix V2.0 (Sys V), "make sys3" or "make sys3nid"
# ? for DEC Pro-380 with 2.9, 2.10, or 2.11 BSD, "make bsd29" or "make bsd210"
# + for Dell UNIX Issue 2.x (= USL Sys V/386 R4.x + fixes), "make dellsys5r4"
#     or "make dellsys5r4c"
# + for DIAB DS90 with DNIX (any version) create an empty <sys/file.h> if
#     this file does not already exist (or add -DNOFILEH to the make entry).
# + for DIAB DS90 or LUXOR ABC-9000 with pre-5.2 DNIX, add "getcwd" to libc.a
#     (see ckuins.doc), then "make dnixold".
# + for DIAB DS90 with DNIX 5.2 (Sys V.2) or earlier, "make dnix",
#     "make dnixnd", or (to add curses and TCP/IP) "make dnixnetc",
# + for DIAB DS90 with DNIX 5.3 (Sys V.3), "make dnix5r3"
# + for DIAB DS90 with DNIX 5.3 (Sys V.3) and TCP/IP, "make dnix5r3net"
# + for DIAB DS90 with DNIX 5.3 2.2 (Sys V.3), ANSI C, "make dnix5r3ansi"
#     or, to include TCP/IP, "make dnix5r3ansinet",
#     but you have to fix a bug in /usr/include/stdlib.h first:
#     change "extern void free(char *str);" to "extern void free(void *str);"
# + for Dolphin Server Technology Triton 88/17 with SV/88 R3.2, "make sv88r32"
# + for Encore Multimax 310, 510 with Umax 4.2, "make umax42"
# + for Encore Multimax 310, 510 with Umax 4.3, "make umax43"
# + for Encore Multimax 310, 510 with Umax V 2.2, use Berkeley cc, "make bsd"
# + for Encore 88K with Umax V 5.2, "make encore88k"
# + for ESIX System V R4.0.3 or 4.04 with TCP/IP support, "make esixr4"
# + for Everex STEP 386/25 Rev G with ESIX Sys V R3.2D, "make sys5r3"
# ? for Fortune 32:16, For:Pro 1.8, "make ft18"
# + for Fortune 32:16, For:Pro 2.1, "make ft21"
# + for FPS 500 with FPX 4.1, "made bsd"
# + for FreeBSD 1.0, "make freebsd"
# + for FreeBSD 2.0, "make freebsd2"
# + for Harris HCX-2900, "make sys5r3"
# ? for Harris Night Hawk 88K or 68K with CX/UX pre-6.1, "make sys5r3"
# + for Harris Night Hawk 88K or 68K with CX/UX 6.1 or later, "make cx_ux"
# ? for Heurikon, "make sys3"
# ? for HP-3000, MPE/ix, "make posix"?
# + for HP-9000 Series 500, HP-UX 5.21 with WIN/TCP 1.2 "make hpux500wintcp"
# + for HP-9000 Series, HP-UX < 6.5, without long filenames, no job control,
#     "make hpuxpre65"
# + for HP-9000 Series, HP-UX pre-7.0, without long filenames, "make hpux"
# + for HP-9000 Series, HP-UX 6.5, without long filenames,
#     "make hpux65" or "make hpux65c"
# + for HP-9000 Series, HP-UX 7.0 or later, no long filenames, "make hpux7sf"
#     or (to include tcp/ip, curses, etc) "make hpux7sftcpc"
# + for HP-9000 Series with HP-UX Sys V R2, BSD long names, "make hpuxlf"
# + for HP-9000 Series with HP-UX Sys V R2, dirent long names, "make hpuxde"
#     or (to include TCP/IP support) "make hpuxdetcp"
# + for HP-9000 Series with HP-UX Sys V R3, "make hpuxs5r3"
# + for HP-9000 Series with HP-UX 7.0, TCP/IP, long filenames, "make hpux70lfn"
# + for HP-9000 300/400 Series (680x0) with HP-UX 8.0, TCP/IP, "make hpux80"
#      or "make hpux80c"
# + for HP-9000 700/800 Series (PA-RISC), HP-UX 8.0, TCP/IP, "make hpux80pa"
#      or "make hpux80pac"
# + for HP-9000 Series with HP-UX 8.0, no TCP/IP, long filenames,
#      "make hpux80notcp" or "make hpuxde"
# + for HP-9000 Series, HP-UX 9.0 - 9.10, TCP/IP, curses, restricted compiler
#     (no optimization, no ANSI), all models, "make hpux90".  Read the hpux90
#     entry below for more info.
# + for HP-9000 700 and 800 Series, HP-UX 9.x, TCP/IP, curses,
#     HP optimizing ANSI C compiler, "make hpux90o700".
# + for HP-9000 with Motorola CPUs, HP-UX 9.x, TCP/IP, curses,
#     HP optimizing ANSI C compiler, "make hpux90mot".
# + for HP-9000 on other CPUs, HP-UX 9.x, TCP/IP, curses,
#     HP optimizing ANSI C compiler, "make hpux90o".
# + for HP-9000 series, HP-UX 9.x, TCP/IP, curses, gcc, all models,
#     "make hpux90gcc"
# + for HP-9000 700/800 Series, HP-UX 10.00,10.01,10.10,10.20,10.30, TCP/IP,
#     curses, restricted compiler (no optimization, no ANSI) "make hpux100".
# + for HP-9000 700/800 Series, HP-UX 10.00,10.01,10.10,10.20,10.30, TCP/IP,
#     curses, HP ANSI/optimizing compiler "make hpux100o" or "make hpux100o+"
# ? for IBM 370 Series with IX/370, "make ix370"
# + for IBM 370 Series with AIX/370 1.2, "make aix370"
# ? for IBM 370 Series with AIX/370 3.0, "make aix370"
# + for IBM 370 Series with AIX/ESA 2.1, "make aixesa"
# - for IBM PC/AT 286 & compatibles with Mark Williams Coherent OS,
#     command-line-only version, "make coherent" (version 5A is too big)
# + for IBM PC 386 & compatibles with Mark Williams Coherent OS,
#     minimum interactive version, "make coherentmi"
# + for IBM PC 386 & compatibles with Mark Williams Coherent OS,
#     full interactive version, prior to v4.2, "make coherentmax"
# + for IBM PC 386 & compatibles with Mark Williams Coherent OS 4.2,
#     "make coherent42"
# + for IBM PC 386 & compatibles with LynxOS 2.0 or 2.1, "make lynx21"
# + for IBM PC 386 & compatibles with LynxOS 2.2, "make lynx"
# - for IBM PC/AT & compatibles with original MINIX, "make minix" (too big)
# + for IBM PC/AT & compatibles with MINIX, new compiler, "make minixnew"
# + for IBM PC family, 386-based, with MINIX/386, "make minix386"
#     or if you have GNU CC, "make minix386gcc"
# + for IBM PS/2 with PS/2 AIX 1.0, 1.1, or 1.2, "make ps2aix" on make level
#      1009 with U401450
# + for IBM PS/2 with PS/2 AIX 1.3, "make ps2aix3"
# + for IBM RISC System/6000 with AIX 3.0 or 3.1 "make rs6000" or
#     "make rs6000c" on make level 2008, 3.1.8
# + for IBM RISC System/6000 with AIX 3.2.0 thru 3.2.5
#     "make rs6aix32" or "make rs6aix32c"
# + for IBM RISC System/6000 with AIX 4.1 or AIX 4.1.1, "make rs6aix41c"
# ? for IBM RT PC with AIX 2.1, "make sys3"
# + for IBM RT PC with AIX 2.2.1, "make rtaix" or "make rtaixc"
# ? for IBM RT PC with ACIS 4.2, "make bsd"
# + for IBM RT PC with ACIS 4.3, "make rtacis" or "make bsd KFLAGS=-DNOANSI"
# + for IBM RT PC with 4.3BSD/Reno, "make bsd44" or "make bsd44c"
# + for ICL DRS400 or 400E, "make iclsys5r3"
# + for ICL DRS3000 (80486) with DRS/NX, "make iclsys5r4_486"
# + for ICL DRS6000 (SPARC) with DRS/NX, "make iclsys5r4"
# + Integrated Solutions Inc V8S VME 68020, "make isi"
# + for Intel 302 with Bell Tech Sys V/386 R3.2, "make sys5r3"
# ? for Intel Xenix/286, "make sco286"
# ? for Interactive System III (PC/IX), "make pcix" or "make is3"
# + for Interactive System III (PC/IX) with gcc, "make is3gcc"
# + for Interactive 386/ix 1.0.6 with TCP/IP networking, "make is5r3net2"
# + for Interactive 386/ix 2.0.x, "make is5r3" or (POSIX) "make is5r3p"
# + for Interactive 386/ix 2.0.x with TCP/IP networking, "make is5r3net"
#     or "make is5r3net2"
# + for Interactive 386/ix 2.2.1, job control, curses, no net, gcc,
#     "make is5r3gcc"
# + for Interactive UNIX Sys V R3.2 V2.2 - 4.0 without TCP/IP, "make is5r3jc"
# + for Interactive UNIX Sys V R3.2 V2.2 - 4.0 with TCP/IP, "make is5r3netjc"
# + for Intergraph Clipper, "make clix" or "make clixnetc"
# + for Jolix (see 386BSD)
# + for Linux, "make linux" or (to remove TCP/IP) "make linuxnotcp".
#     For static linking, use "make linuxs".  IMPORTANT: Read the comments
#     that accompany the "linux:" entry.
# + for Linux with lcc compiler, "make linuxnotcp-lcc"
# + for Luxor ABC-9000 (DIAB DS-90) with pre-5.2 DNIX, add "getcwd" to libc.a
#     (see ckuins.doc), then "make dnixold".
# + for Mach 2.6 on (anything, e.g. DECstation), "make bsd42" or "make bsd43".
# + for MachTen (Tenon) 2.1.1.D on (e.g.) Apple Powerbook, "make machten".
# ? for Masscomp RTU AT&T System III, "make rtu"
#   for other Masscomp, see Concurrent.
# ? for Microport SV/AT (System V R2), "make mpsysv" (last edit tested: 144)
# + for Microport SVR4 2.2, 3.1, or 4.1 "make sys5r4sx"
# ? for Microsoft,IBM Xenix (/286, PC/AT, etc), "make xenix" or "make sco286"
# + for MIPS System with RISC/os (UMIPS) 4.52 = AT&T SVR3, "make mips"
#     or "make mipstcpc"
# ? for MkLinux on Power Macintosh, "make linux"
# + for Modcomp 9730, Real/IX, "make sys5r3" (or modify to use gcc = GLS cc)
# + for Modcomp Realstar 1000 with REAL/IX D.1, "make sv88r32"
# ? for Motorola Four Phase, "make sys3" or "make sys3nid"
# + for Motorola Delta System V/68 R3, "make sv68r3"
# + for Motorola Delta System V/68 R3V5, "make sv68r3v5"
# + for Motorola Delta System V/68 R3V5.1, "make sv68r3v51"
# + for Motorola Delta System V/68 R3V6 with NSE TCP/IP, "make sv68r3v6"
# + for Motorola Delta System V/88 R32, "make sv88r32"
# + for Motorola Delta System V/88 R40, "make sv88r40"
# + for Mt Xinu Mach386 on 386/486-based PCs, "make bsd43"
# ? for NCR Tower 1632, OS 1.02, "make tower1"
# + for NCR Tower 1632 or Minitower with System V R2, "make sys3"
#     or "make sys3nv"
# + for NCR Tower 32, OS Release 1.xx.xx, "make tower32-1"
# + for NCR Tower 32, OS Release 2.xx.xx, "make tower32-2"
# + for NCR Tower 32, OS Releases based on Sys V R3, "make tower32"
# + for NCR Tower 32, OS Releases based on Sys V R3 with gcc "make tower32g"
# + for NCR System 3000, AT&T UNIX System V R4 2.0, "make sys5r4sxna"
# + for NCR System 3000, AT&T UNIX System V R4 2.0 with Wollongong TCP/IP,
#     "make sys5r4net2" or "make sys5r4net2c".
#      Some header files might misplaced; try this:
#       ln /usr/include/netinet/in.h /usr/include/sys/in.h
#       ln /usr/include/arpa/inet.h /usr/include/sys/inet.h
#       ln /usr/include/sys/termiox.h /usr/include/termiox.h
# + for NCR System 3000, NCR UNIX 02.02.01, same as above.
# + for NCR MP-RAS System V R4 V2.03, same as above.
# + for NCR System 3000, NCR UNIX 02.03.x or 02.04.x, "make sys5r4net2c"
# + for NetBSD on PC 386/486,..., "make netbsd" or (ncurses) "make netbsdn"
# + for NeXT with NeXTSTEP 1.0 through 3.2, "make next" (on a NeXT)
# + for NeXT with NeXTSTEP 3.3, "make next33"
# + for NeXTSTEP/486, "make next" (on a PC)
# + for NeXTSTEP portable binary (runs on Intel or Motorola), "make nextfat"
# + for Nixdorf Targon/31, "make t31tos40x"
# + for Norsk Data Uniline 88/17 with SV/88 R3.2, "make sv88r32"
#   for Novell UnixWare - see UnixWare
# + for OSF/1 (vanilla, from OS/F), "make posix"
# + for OkiStation 7300 Series, "make sys5r4sxtcp"
# + for Olivetti LSX-3020 with X/OS R.2.3, "make xos23" or "make xos23c"
# ? for OpenBSD, "make netbsd"?
# + for Perkin-Elmer (Concurrent) 3200 series, "make sys5".
# + for Perkin-Elmer (Concurrent) 3200 series with <dirent.h>, "make ccop1"
# + for Perkin-Elmer/Concurrent 3200 with Xelos R02, "make ccop1"
# + for PFU Compact A Series SX/A TISP V10/E50 (Japan), "make sxae50"
# ? for Plexus, "make sys3"
# + for Pyramid 9XXX (e.g. 9845) or MIServer T series, OSx 4.4b thru 5.1,
#     "ucb make pyramid" or for HDB UUCP, "ucb make pyramid-hdb" or:
# + for Pyramid MIServer S or ES Series, DataCenter/OSx, "make pyrdcosx"
# + for Pyramid MIS-S MIPS R3000, DataCenter OSx System V R4, "make pyrdcosx"
# + for POSIX on anything, "make posix" (but adjustments might be necessary).
# + for POSIX on SunOS 4.1 or later, "make sunposix"
# + for Prime 8000 MIPS, SVR3, "make mips" or "make mipstcpc"
# + for QNX 4.0 or 4.1, 16-bit, on 286 PC, Watcom C 8.5, "make qnx16_41"
# + for QNX 4.21 - 4.22A (286+), and 4.23 (386+), 16-bit, Watcom C 9.5x,
#     "make qnx16"
# + for QNX 4.21 and above, 32-bit, 386 or above, Watcom C 10.6, "make qnx32"
#     NOTE: Default is qnx32 ("make qnx")
# ? for Ridge 32 (ROS3.2), "make ridge32"
# ? for Samsung MagicStation, "make sys5r4"
# ? for SCO Xenix 2.2.1 with development system 2.2 on 8086/8 "make sco86"
# + for SCO Xenix/286 2.2.1 with development system 2.2 on 80286, "make sco286"
#     NOTE: reportedly this makefile is too long for SCO Xenix/286 make, but it
#     works with "makeL", or if some of the other make entries are edited out.
# + for SCO Xenix/386 2.2.2, "make sco386"
# + for SCO Xenix/386 2.3.x, "make sco3r2" or "make sco3r2x"
# + for SCO Xenix/386 SCO 2.3.3 or 2.3.4 with gcc 1.37 or later,
#     "make sco386gcc" or (to add curses) "make sco386gccc".
# + for SCO Xenix/386 or UNIX/386 with Excelan TCP/IP, "make sco3r2net"
#     or (to add curses support) "make sco3r2netc" or "sco386netc"
# + for SCO Xenix 2.3.x with Racal-InterLan TCP/IP, "make sco3r2netri"
# + for other UNIX varieties with Racal Interlan TCP/IP, read sco3r2netri entry
# + for SCO Xenix 2.3.x with SCO (Lachman) TCP/IP, "make sco3r2lai"
#     or (to add curses) "make sco3r2laic"
#   for SCO UNIX...  ALSO READ COMMENTS in the SCO UNIX entries for more info!
# + for SCO UNIX/386 3.2.0 or 3.2.1, "make sco3r2" or "make sco3r2x"
# + for SCO UNIX/386 3.2.2, "make sco3r22" or "make sco3r22gcc"
#     or "make sco3r22c"
# + for SCO UNIX/386 3.2.2 with SCO TCP/IP, "make sco3r22net"
#     or "make sco3r22netc" (curses)
# + for SCO ODT 1.1, "make sco3r22net" or "make sco3r22netc" (curses)
# + for SCO UNIX/386 3.2 V4.x, no network support, "make sco32v4"
# + for SCO UNIX/386 3.2 V5.0 - see SCO OpenServer.
# + for SCO UNIX 3.2v4.x with TCP/IP, <dirent.h> for Extended Acer File
#     System (EAFS), curses, ANSI C compilation, "make sco32v4net"
# + for SCO UNIX 3.2v4.2, "make sco_odt30"
#
# NOTE: Also see below for other entries that are variations on these,
# e.g. to compile with gcc rather than the SCO Development System.
# Also be sure to read the comments accompanying each SCO entry.
#
# + for SCO ODT 2.0, "make sco32v4net"
# + for SCO ODT 3.0, "make sco_odt30"
# + for SCO OpenServer 5.0 (OSR5), "make sco32v5"
# + for SCO OpenServer 5.0 (OSR5) with networking, "make sco32v5net"
# + for SCO OpenServer 5.0 (OSR5), gcc, "make sco32v5gcc"
# + for SCO OpenServer 5.0 (OSR5), gcc, with networking, "make sco32v5netgcc"
# + for SCO OpenServer 5.0 (OSR5) as above, ELF binary, "make sco32v5netgccelf"
#   for SCO UnixWare - see UnixWare
# + for Sequent with DYNIX/ptx 1.2.1, "make dynixptx12"
# + for Sequent with DYNIX/ptx 1.3 or 1.4 with TCP/IP, "make dynixptx13"
# + for Sequent with DYNIX/ptx 2.0 or 2.1 with TCP/IP, "make dynixptx20"
#     or "dynixptx20c"
# + for Sequent with DYNIX/ptx 4.0 V4.1.3 with TCP/IP, "make dynixptx41c"
# + for Sequent Balance 8000 or B8 with DYNIX 3.0.xx, "make dynix3"
#    or "make dynix3noacu"
# + for Sequent Symmetry S81 with DYNIX 3.0.xx, "make dynix3"
# + for Sequent DYNIX 3.1.xx, "make dynix31" or "make dynix31c"
# + for Siemens/Nixdorf SINIX-N MIPS V5.42, "make sinix542"
# + for Silicon Graphics Iris System V IRIX 3.2 or earlier, "make iris"
# + for Silicon Graphics Sys V R3 with IRIX 3.3 or later, "make sys5r3"
# + for Silicon Graphics Iris Indigo with IRIX 4.0 or 5.0, "make irix40" or
#     (to include Yellow Pages and Curses) "make irix40ypc"
# + for Silicon Graphics Iris Indigo or Elan with IRIX 4.0.x with microcode
#     optimization and -O4, "make irix40u" or "irix40uc" (and read notes
#     accompanying these entries).
# + for Silicon Graphics IRIX 5.1/5.2/5.3, "make irix51".
# + for Silicon Graphics models with IRIX 6.0, "make irix60".
# + for Solaris 2.0-2.4 on SPARC or Intel, SunPro CC, "make solaris2x",
# +   or to add SunLink X.25 8.0x support, "make solaris2x25".
# + for Solaris 2.0-2.4 on SPARC or Intel, GNU CC, "make solaris2xg".
# + for Solbourne 4/500 with OS/MP 4 "make sunos4"
# + for Solbourne 4/500 with OS/MP 4.1 "make sunos41" or "make sunos41c"
# + for SONY NEWS with NEWS-OS 4.0.1C, "make sonynews"
# + for SONY NEWS with NEWS-OS 4.1.2C, "make sonynews"
# + for Sperry/UNISYS 5000/20, UTS V 5.2 3R1, "make sys5"
# + for Sperry/UNISYS 5000/30/35/50/55, UTS V 5.2 2.01, "make unisys5r2"
# + for Sperry/UNISYS 5000/80 with System V R3, "make sys5r3"
# + for Sperry/UNISYS 5000/95 with System V R3, "make sys5r3"
#     For UNISYS SVR3 it might be necessary to "make sys5r3 KFLAGS=-UDYNAMIC"
# + for Stardent 1520, "make sys5r3"
# + for Stratus, various models, FTX 2.1, probably also 2.2, "make sys5r4"
# + for Sun with Sun UNIX 3.5 and gcc, "make sunos3gcc"
# + for Sun with pre-4.0 SunOS versions, "make bsd" (or appropriate variant)
# + for Sun with SunOS 4.0, BSD environment, "make sunos4"
# + for Sun with SunOS 4.0, BSD, with SunLink X.25, make sunos4x25
# + for Sun with SunOS 4.0, AT&T Sys V R3 environment, "make sunos4s5"
# + for Sun with SunOS 4.1 or 4.1.1, BSD environment, "make sunos41"
#     or "make sunos41c" (curses) or "make sunos41gcc" (compile with gcc)
# + for Sun with SunOS 4.1.x, BSD, with SunLink X.25 7.00 or earlier,
#     "make sunos41x25" or "make sunos41x25c" (curses)
# + for Sun with SunOS 4.1, 4.1.1, AT&T Sys V R3 environment, "make sunos41s5"
# + for Sun with SunOS 4.1, 4.1.1, POSIX environment, "make sunposix"
# + for Sun with SunOS 4.1.2, "make sunos41" or any of its variations.
#     NOTE:  All SunOS 4.x systems -- Shared libraries are used by default.
#       If this causes problems, add -Bstatic to CFLAGS.
#     NOTE2: When building C-Kermit under SunOS for the BSD universe,
#       but /usr/5bin/cc is ahead of /usr/ucb/cc in your PATH, add
#       "CC=/usr/ucb/cc CC2=/usr/ucb/cc" to the make entry.
#     NOTE3: If an executable built on one type of Sun hardware does not work
#       on another type, rebuild the program from source on the target machine.
# + for Sun with Solaris 1.x use SunOS 4.1 entries.
# + for Sun with Solaris 2.0 through 2.5 and SunPro CC, "make solaris2x"
# + for Sun with Solaris 2.0 through 2.5 and GNU CC, "make solaris2xg"
# + for Tandy 16/6000 with Xenix 3.0, "make trs16"
# + for Tektronix 6130/4132/43xx (e.g.4301) with UTek OS, "make utek"
#     or (for models without hardware flow control), "make uteknohwfc"
# + for Tektronix XD88 series with UTekV OS, "make utekvr3"
# + for Tri Star Flash Cache with Esix SVR3.2, "make sys5r3"
# ? for Unistar, "make sys5"
# + for UNISYS S/4040 68040 CTIX SVR3.2 6.4.1, "make ctix" or "make sys5r3"
# + for UNISYS U5000 UNIX SVR3 6.x, "make sys5r3" or "make sys5r3c"
# + for UNISYS U6000 UNIX SVR4 1.x, "make sys5r4nx" or "make sys5r4nxnetc"
#   for UNISYS ... (also see Sperry)
#   for Univel - see UnixWare
# + for UnixWare, "make unixware" or "make unixwarenetc"
# ? for Valid Scaldstar, "make valid"
# ? for Whitechapel MG01 Genix 1.3, "make white"
# ? for Zilog ZEUS 3.21, "make zilog"
#
# The result should be a runnable program called "wermit" in the current
# directory.  After satisfactory testing, you can rename wermit to "kermit"
# and put it where users can find it.
#
# To remove intermediate and object files, "make clean".
# If your C compiler produces files with an extension other than "o",
# then "make clean EXT=u", "make clean EXT=s", or whatever.
#
# To run lint on the source files, "make lintsun", "make lintbsd",
# "make lints5", as appropriate.
#
##############################################################################
#
# NOTES FOR V7 AND 2.X BSD (BASED ON VERSION 4E OF C-KERMIT):
#
# For Unix Version 7, several variables must be defined to the values
# associated with your system.  BOOTNAME=/edition7 is the kernel image on
# okstate's Perkin-Elmer 3230.  Others will probably be /unix.  PROCNAME=proc
# is the name of the structure assigned to each process on okstate's system.
# This may be "_proc" or some other variation.  See <sys/proc.h> for more
# info on your systems name conventions.  NPROCNAME=nproc is the name of a
# kernel variable that tells how many "proc" structures there are.  Again
# this may be different on your system, but nproc will probably be somewhere.
# The variable NPTYPE is the type of the nproc variable -- int, short, etc.
# which can probably be gleaned from <sys/param.h>.  The definition of DIRECT
# is a little more complicated.  If nlist() returns, for "proc" only, the
# address of the array, then you should define DIRECT as it is below.  If
# however, nlist() returns the address of a pointer to the array, then you
# should give DIRECT a null definition (DIRECT= ).  The extern declaration in
# <sys/proc.h> should clarify this for you.  If it is "extern struct proc
# *proc", then you should NOT define DIRECT.  If it is "extern struct proc
# proc[]", then you should probably define DIRECT as it is below.  See
# ckuv7.hlp for further information.
#
# For 2.9 BSD, the makefile may use pcc rather than cc for compiles; that's
# what the CC and CC2 definitions are for (the current version of the
# makefile uses cc for both; this was tested in version 4E of C-Kermit and
# worked OK on the DEC Pro 380, but all bets are off for version 5A).  2.9
# support basically follows the 4.1 path.  Some 2.9 systems use "dir.h" for
# the directory header file, others will need to change this to "ndir.h".
#
# The v7 and 2.9bsd versions assume I&D space on a PDP-11.  When building
# C-Kermit for v7 on a PDP-11, you should probably add the -i option to the
# link flags.  Without I&D space, overlays will be necessary (if available),
# or code segment mapping (a`la Pro/Venix) if that's available.
#
# C-Kermit 5A (and 6.0?) can be built for 2.10 and 2.11BSD, using overlays,
# but a separate makefile is used because this one is too big.
#
##############################################################################
#
# V7-specific variables.
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
#
###########################################################################
# SAMPLE ONLY.
# Easy installation. Modify this to suit your own computer's file organization
# and permissions.  If you don't have write access to the destination
# directories, "make install" will fail.  In many cases, a real installation
# will also require you to chown / chgrp for the UUCP lockfile and/or tty
# devices, and perhaps also to chmod +s the appropriate permission fields.

WERMIT = makewhat
DESTDIR =
BINDIR = /usr/local/bin
MANDIR = /usr/man/manl
MANEXT = l
ALL = $(WERMIT)

all: $(ALL)

install: $(ALL)
	cp wermit $(DESTDIR)$(BINDIR)/kermit
# The following can fail if the program image was already stripped,
# for example by the link flags in the makefile entry.
#	strip $(DESTDIR)$(BINDIR)/kermit
	chmod 755 $(DESTDIR)$(BINDIR)/kermit
	cp ckuker.nr $(DESTDIR)$(MANDIR)/kermit.$(MANEXT)
# To make sure 'man' notices the new source file and doesn't keep
# showing the old formatted version, remove the old formatted version,
# something like this:
#	rm -f $(DESTDIR)$(MANDIR)/../cat$(MANEXT)/kermit.$(MANEXT)
# or this (which requires CATDIR to be defined):
#	rm -f $(DESTDIR)$(CATDIR)/kermit.$(MANEXT)
	chmod 644 $(DESTDIR)$(MANDIR)/kermit.$(MANEXT)

makewhat:
	@echo 'make what?  You must tell which system to make C-Kermit for.'
	@echo Examples:  make hpux90, make sys5r4, make solaris2x, etc.
	@echo Please read the comments at the beginning of the makefile.

###########################################################################
#
# Dependencies Section:

manpage: ckuker.nr

wermit:	ckcmai.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) ckuus3.$(EXT) \
		ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) \
		ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) ckcfns.$(EXT) \
		ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) ckucon.$(EXT) \
		ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) \
		ckcnet.$(EXT) ckusig.$(EXT)
	$(CC2) $(LNKFLAGS) -o wermit ckcmai.$(EXT) ckutio.$(EXT) \
		ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) \
		ckuxla.$(EXT) ckcpro.$(EXT) ckucmd.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckuusr.$(EXT) \
		ckucon.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) \
		ckusig.$(EXT) $(LIBS)

#Malloc Debugging version

mermit:	ckcmdb.$(EXT) ckcmai.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucon.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckusig.$(EXT)
	$(CC2) $(LNKFLAGS) -o mermit ckcmdb.$(EXT) ckcmai.$(EXT) \
		ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) \
		ckcfn3.$(EXT) ckuxla.$(EXT) ckcpro.$(EXT) ckucmd.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckuusr.$(EXT) ckucon.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) \
		ckcnet.$(EXT) ckusig.$(EXT) $(LIBS)

###########################################################################
# man page...
#
# WARNING: Using "cc -E" to preprocess the man page is not portable, but it
# works OK in SunOS 4.1.x, HP-UX, etc.  We use the preprocessor to produce
# custom man pages based on ifdef, else, and endif directives.  But the
# preprocessor replaces omitted lines by blank lines and comment lines, so we
# use grep to filter them out.  THIS MEANS THAT THE SOURCE FILE, ckuker.cpp,
# MUST NOT CONTAIN ANY BLANK LINES!
#
ckuker.nr: ckuker.cpp
	$(CC) $(CFLAGS) -E ckuker.cpp |grep -v "^$$" |grep -v "^\#" > ckuker.nr

###########################################################################
# Dependencies for each module...
#
ckcmai.$(EXT): ckcmai.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcnet.h ckcsig.h \
		ckuusr.h

ckcpro.$(EXT): ckcpro.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h

ckcpro.c: ckcpro.w wart ckcdeb.h ckcsym.h ckcasc.h ckcker.h
	./wart ckcpro.w ckcpro.c

ckcfns.$(EXT): ckcfns.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h \
		ckuxla.h

ckcfn2.$(EXT): ckcfn2.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h ckuxla.h

ckcfn3.$(EXT): ckcfn3.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h \
		ckuxla.h

ckuxla.$(EXT): ckuxla.c ckcker.h ckcsym.h ckcdeb.h ckcxla.h ckuxla.h

ckuusr.$(EXT): ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcsym.h ckcdeb.h ckcxla.h \
		ckuxla.h ckcasc.h ckcnet.h

ckuus2.$(EXT): ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h

ckuus3.$(EXT): ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h

ckuus4.$(EXT): ckuus4.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckuver.h ckcsym.h

ckuus5.$(EXT): ckuus5.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcnet.h \
		 ckcsym.h

ckuus6.$(EXT): ckuus6.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcnet.h \
		 ckcsym.h

ckuus7.$(EXT): ckuus7.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h

ckuusx.$(EXT): ckuusx.c  ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcsym.h ckcsig.h

ckuusy.$(EXT): ckuusy.c  ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsym.h

ckucmd.$(EXT): ckucmd.c ckcasc.h ckucmd.h ckcdeb.h ckcsym.h

ckufio.$(EXT): ckufio.c ckcdeb.h ckuver.h ckcsym.h

ckutio.$(EXT): ckutio.c ckcdeb.h ckcnet.h ckuver.h ckcsym.h

ckucon.$(EXT): ckucon.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsym.h

ckcnet.$(EXT): ckcnet.c ckcdeb.h ckcker.h ckcnet.h ckcsym.h ckcsig.h

wart: ckwart.$(EXT)
	$(CC) $(LNKFLAGS) -o wart ckwart.$(EXT) $(LIBS)

ckcmdb.$(EXT): ckcmdb.c ckcdeb.h ckcsym.h

ckwart.$(EXT): ckwart.c

ckudia.$(EXT): ckudia.c ckcker.h ckcdeb.h ckucmd.h ckcasc.h ckcsym.h ckcsig.h

ckuscr.$(EXT): ckuscr.c ckcker.h ckcdeb.h ckcasc.h ckcsym.h ckcsig.h

ckusig.$(EXT): ckusig.c ckcasc.h ckcdeb.h ckcker.h ckcnet.h ckuusr.h ckcsig.h

###########################################################################
#
# Entries to make C-Kermit for specific systems.
#
# Put the ones that need short makefiles first.

bsd210:
	@echo Please use ckubs2.mak to build C-Kermit $(CKVER) for 2.10BSD.

bsd211:
	@echo Please use ckubs2.mak to build C-Kermit $(CKVER) for 2.11BSD.

#Apollo Aegis 9.x.  Includes TCP/IP support.
#You can also add processor-dependent optimization switches like -M570.
aegis:
	@echo Making C-Kermit $(CKVER) for Apollo Aegis 9.x...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DDYNAMIC -DTCPSOCKET \
	-DCK_CURSES -O $(KFLAGS)" "LIBS = -lcurses -ltermcap"

#Apple Mac II, A/UX pre-3.0
#Warning, if "send *" doesn't work, try the auxufs makefile entry below.
aux:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX...
	$(MAKE) wermit "CFLAGS = -DAUX -DDYNAMIC -DTCPSOCKET \
	$(KFLAGS) -i -O" "LNKFLAGS = -i"

#Apple Mac II, A/UX pre-3.0, compiled with gcc
auxgcc:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX...
	$(MAKE) wermit "CFLAGS = -DAUX -DDYNAMIC -DTCPSOCKET \
	-traditional $(KFLAGS) -i -O" "LNKFLAGS = " "CC = gcc" "CC2 = gcc"

#Apple Mac II, A/UX, pre-3.0, but with ufs file volumes, uses <dirent.h>.
auxufs:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX...
	$(MAKE) wermit "CFLAGS = -DAUX -DDYNAMIC -DTCPSOCKET -DDIRENT \
	$(KFLAGS) -i -O" "LNKFLAGS = -i"

#Apple Mac II, A/UX 3.0, compiled with gcc
aux3gcc:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX 3.0...
	$(MAKE) wermit "CFLAGS = -DAUX -DHDBUUCP -DLFDEVNO -DDYNAMIC \
	-DTCPSOCKET -DDIRENT $(KFLAGS) -O2" "LNKFLAGS = -s" "LIBS = $(LIBS)" \
	"CC=gcc -pipe -traditional" "CC2=gcc -pipe -traditional"

#Apple Mac II, A/UX 3.0, compiled with gcc, uses curses
aux3cgcc:
	@echo Making C-Kermit $(CKVER) for Macintosh A/UX 3.0...
	$(MAKE) "MAKE=$(MAKE)" aux3gcc "KFLAGS=$(KFLAGS) -DCK_CURSES" \
	"LIBS = -lcurses $(LIBS)"

#Bell Labs Research UNIX V10
#Can't add TCP/IP because there is no sockets library.  It would have to
#be done using streams, but there is no code in C-Kermit for that.
#Remove -DNOJC if desired (if your system has csh, ksh, or bash).
bellv10:
	@echo Making C-Kermit $(CKVER) for Bell Labs Research UNIX V10...
	$(MAKE) wermit "CFLAGS= -DBELLV10 -DBSD4 -DNDIR -DDYNAMIC -DNOJC \
	-DNOSYSIOCTLH -DNOSETREU -DNOSETBUF -DNOCSETS -MINIDIAL $(KFLAGS)"

#Berkeley Unix 4.1
bsd41:
	@echo Making C-Kermit $(CKVER) for 4.1BSD...
	$(MAKE) wermit "CFLAGS= -DBSD41" "LIBS = -ljobs"

#Berkeley 4.2, 4.3, also Ultrix-32 1.x, 2.x, 3.x, many others
# Add -O, -DDYNAMIC, -s, etc, if they work.
# If you have a version of BSD but signal() is void rather than int,
# "make bsd KFLAGS=-DSIG_V".
bsd:
	@echo Making C-Kermit $(CKVER) for 4.2BSD...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET $(KFLAGS)"

#Berkeley 4.2, 4.3, minimum size
bsdm:
	@echo Making C-Kermit $(CKVER) for 4.2BSD...
	$(MAKE) wermit "CFLAGS= -O -DBSD4 -DDYNAMIC -DNODIAL -DNOHELP \
	-DNODEBUG -DNOTLOG -DNOSCRIPT -DNOCSETS -DNOICP $(KFLAGS)" \
	"LNKFLAGS = -s"

#Berkeley Unix with HoneyDanBer UUCP
bsdhdb:
	@echo Making C-Kermit $(CKVER) for 4.2BSD with HDB UUCP...
	$(MAKE) wermit "CFLAGS= -DHDBUUCP -DBSD4 -DTCPSOCKET $(KFLAGS)"

#Berkeley Unix 4.3 with acucntrl program
bsd43:
	@echo Making C-Kermit $(CKVER) for 4.3BSD with acucntrl...
	$(MAKE) wermit \
	"CFLAGS= -DBSD43 -DACUCNTRL -DTCPSOCKET $(KFLAGS) -O"

#Ditto, with curses support.
bsd43c:
	@echo Making C-Kermit $(CKVER) for 4.3BSD with acucntrl and curses...
	$(MAKE) wermit "CFLAGS= -DBSD43 -DACUCNTRL -DTCPSOCKET \
	 -DCK_CURSES $(KFLAGS) -O"  "LIBS=-lcurses -ltermcap"

#Berkeley Unix 4.2 or 4.3 with lock directory /usr/spool/uucp/LCK/LCK..ttyxx,
#but without acucntrl program
bsdlck:
	@echo Making C-Kermit $(CKVER) for 4.2BSD, /usr/spool/uucp/LCK/...
	$(MAKE) wermit "CFLAGS= -DLCKDIR -DBSD4 -DTCPSOCKET $(KFLAGS)"

#Berkeley UNIX 4.4-Lite, 4.4-Encumbered, Net/2, etc (Post-Reno),
#with TCP/IP networking.  This includes NetBSD, FreeBSD, etc.
#NOTE: This is not a pure POSIX configuration.  Using -DPOSIX instead of
# -DBSD44 prevents any kind of directory-reading (for wildcard expansion),
#and disallows use of ENOTCONN symbol for detecting broken network
#connections, and disallows RTS/CTS flow control, and would also require
#definition of the appropriate UUCP lockfile convention.
#Do not add -DCK_POSIX_SIG without reading <signal.h> first!  For example,
#sigsetjmp(), etc, tend to be defined but not implemented.
bsd44:
	@echo Making C-Kermit $(CKVER) for 4.4BSD...
	$(MAKE) wermit \
	"CFLAGS= -DBSD44 -DDYNAMIC -DTCPSOCKET $(KFLAGS) -O"

#Berkeley UNIX 4.4, as above, but with curses for fullscreen display
#Please read notes for bsd44 entry just above.
bsd44c:
	@echo Making C-Kermit $(CKVER) for 4.4BSD with curses...
	$(MAKE) wermit \
	"CFLAGS= -DBSD44 -DCK_CURSES -DDYNAMIC -DTCPSOCKET $(KFLAGS) -O" \
	"LIBS= -lcurses -ltermcap"

#NetBSD
netbsd:
	$(MAKE) bsd44c "KFLAGS=$(KFLAGS) -DNOCOTFMC"

#NetBSD with ncurses
netbsdn:
	@echo Making C-Kermit $(CKVER) for NetBSD with ncurses...
	$(MAKE) wermit \
	"CFLAGS= -DBSD44 -DCK_CURSES -DDYNAMIC -DTCPSOCKET -DNOCOTFMC \
	$(KFLAGS) -O" \
	"LIBS= -lncurses -ltermcap"

#Acorn RISCiX, based on ...
#Berkeley Unix 4.2 or 4.3 with lock directory /usr/spool/uucp/LCK/LCK..ttyxx,
#but without acucntrl program
riscix:
	@echo Making C-Kermit $(CKVER) for RISCiX, /usr/spool/uucp/LCK..ttyxx
	$(MAKE) wermit "CFLAGS= -DBSD42 -DBSD4 -DRISCIX -DNOCSETS \
		-DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DDIRENT -DCK_CURSES \
		-DMAXSP=9024 -DMAXRD=9024 -DSBSIZ=9050 -DRBSIZ=9050 \
		-DFTTY=\\\"/dev/serial\\\" -DNOCSETS -DNOCYRIL -DNOSETBUF \
		-DNOANSI -w -O2 -fomit-frame-pointer" \
		"LIBS= -lcurses -ltermcap " \
		"CC= /usr/ucb/cc" \
		"CC2= /usr/ucb/cc"

#Acorn RISCiX, as above, but using gcc
riscix-gcc:
	@echo Making C-Kermit $(CKVER) for RISCiX, /usr/spool/uucp/LCK..ttyxx
	$(MAKE) wermit "CFLAGS= -DBSD42 -DBSD4 -DRISCIX -DNOCSETS \
		-DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DDIRENT -DCK_CURSES \
		-DMAXSP=9024 -DMAXRD=9024 -DSBSIZ=9050 -DRBSIZ=9050 \
		-DFTTY=\\\"/dev/serial\\\" -DNOCSETS -DNOCYRIL -DNOSETBUF \
		-DNOANSI -w -O2 -fomit-frame-pointer" \
		"LIBS= -lcurses -ltermcap " \
		"CC= gcc -mbsd" \
		"CC2= gcc -mbsd"

#Tektronix 6130, 4319, 4301, etc, with UTek OS, /usr/spool/uucp/LCK./...
#The models that support hardware flow control.
utek:
	@echo 'Making C-Kermit $(CKVER) for 4.2BSD/UTek, hardware flow control'
	$(MAKE) wermit \
	"CFLAGS= -O -DLCKDIR -DBSD4 -DTCPSOCKET -DDYNAMIC \
	-DUTEK -DDCLPOPEN -DLOCK_DIR=\\\"/usr/spool/uucp/LCK.\\\" \
	-DTRMBUFL=2048 -DCK_DTRCTS $(KFLAGS)"

#Tektronix 4315, 4316, 4317 with UTek OS, /usr/spool/uucp/LCK./...
#The models that do not fully support hardware flow control.
uteknohwfc:
	@echo 'Making C-Kermit $(CKVER) for 4.2BSD/UTek, no h/w flow control'
	$(MAKE) wermit \
	"CFLAGS= -O -DLCKDIR -DBSD4 -DTCPSOCKET -DDYNAMIC \
	-DUTEK -DDCLPOPEN -DLOCK_DIR=\\\"/usr/spool/uucp/LCK.\\\" \
	-DTRMBUFL=2048 $(KFLAGS)"

#Tektronix XD88 with  UTekV OS
utekvr3:
	@echo 'Making C-Kermit $(CKVER) for Tektronix XD88 UTekV R3...'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DTCPSOCKET -DSYSUTIMEH -DCK_CURSES $(KFLAGS) -O" \
	"LIBS= -lcurses" "LNKFLAGS= -s"

#Convergent CTIX 6.4.1
ctix:
	@echo 'Making C-Kermit $(CKVER) for Convergent CTIX 6.4.1'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DTCPSOCKET -DHDBUUCP -DCK_CURSES -DDYNAMIC \
	-DNONAWS $(KFLAGS) -XO" "LNKFLAGS=-s" "LIBS=-lsocket -lcurses -lc_s"
	mcs -d wermit

# The following makefile entry should work for any Harris Night Hawk system
# (either 88k or 68k based) running release 6.1 or later of the CX/UX
# operating system. This is a POSIX and ANSI-C compliant system which also
# supports BSD networking. (Earlier CX/UX releases will probably work with
# sys5r3, but this has not been verified).
#
cx_ux:
	@echo Making C-Kermit $(CKVER) for Harris Night Hawk CX/UX 6.1 or later
	$(MAKE) wermit \
	"CFLAGS=-DPOSIX -DTCPSOCKET -DHDBUUCP -DPID_T=pid_t -DWAIT_T=int \
	-Dd_ino=d_fileno -DUID_T=uid_t -DGID_T=gid_t -DDYNAMIC $(KFLAGS) -Xa \
	-O3 -g" "LNKFLAGS=-O3"

#Perkin-Elmer 3200 Xelos R02 or earlier
ccop1:
	@echo 'Making C-Kermit $(CKVER) for Xelos & Public Domain Dirent calls'
	@echo 'or System V R2 or earlier...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -Dvoid=int -DDIRENT -DCK_CURSES \
	$(KFLAGS) -O" "LNKFLAGS =" "LIBS= -lcurses -ltermlib"

#Intergraph Clipper 2000, 3000, 4000, 5000, or 6000 with CLIX 3.1 = Sys V R3
#Note, cc has been phased out, acc is used instead.  gcc can be substituted.
clix:
	$(MAKE) "CC=acc" "CC2=acc" sys5r3

#Intergraph Clipper, as above plus curses, TCP/IP, job control, HDB UUCP.
#Probably, should be added to regular clix entry too.
clixnetc:
	$(MAKE) wermit "CC=acc" "CC2=acc" \
	"CFLAGS= -DSVR3JC -DSVR3 -DCK_CURSES -DDIRENT -DCK_NEWTERM \
	-DTCP_SOCKET -HDBUUCP -DDYNAMIC $(KFLAGS) -O" \
	"LNKFLAGS=" "LIBS= -lcurses -ltermlib -lbsd"

#Mark Williams Coherent 286 or 386 on IBM PC family.
#There is a 64K limit on program size, so this is a command-line only version.
coherent:
	$(MAKE) "CFLAGS = -O -DCOHERENT -DDYNAMIC -DNOANSI -DNOICP -DNOSETKEY \
	-DNOCSETS -DNOHELP -DNODIAL -DNOSCRIPT -DNODEBUG -DNOTLOG -DNOXMIT \
	-DNOMSEND -DNOFRILLS -DNOSYSIOCTLH -DSELECT_H $(KFLAGS) -VSUVAR" wermit

#Mark Williams Coherent 386 on IBM PC family.
#This will make a "minimum interactive" version - no scripts,
#no character sets, no help, no dial, no debug/transaction logging, no
#transmit, msend, mail, type, etc.
coherentmi:
	$(MAKE) "CFLAGS = -O -DCOHERENT -DDYNAMIC -DNOANSI -DNOSETKEY \
	-DNOSHOW -DNOCSETS -DNOHELP -DNODIAL -DNOSCRIPT -DNODEBUG -DNOTLOG \
	-DNOXMIT -DNOMSEND -DNOFRILLS -DNOSYSIOCTLH -DNOSERVER -DNOUUCP \
	-DNOSPL -DNOSETBUF -DNOPUSH -DNOMDMHUP -DNOJC -DNOFDZERO -DNOESCSEQ \
	-DNOCMDL $(KFLAGS) -VSUVAR -DSELECT_H" wermit

#Mark Williams Coherent 386 on IBM PC/AT family.
coherentmax:
	$(MAKE) "CFLAGS = -O -DCOHERENT -DDYNAMIC -DNOANSI -DSELECT_H \
	-DNOSYSIOCTLH $(KFLAGS) -VSUVAR" "LNKFLAGS = -O -s" wermit

#Mark Williams Coherent 386 4.2 on IBM PC/AT family
coherent42:
	$(MAKE) "CFLAGS = -T0 -O -DCOHERENT -DDYNAMIC -DNOANSI -DSELECT \
	-DDIRENT -DCK_CURSES -DCK_NEWTERM -DCK_WREFRESH -DNOSYSIOCTLH \
	-DSELECT_H $(KFLAGS) -VSUVAR" "LNKFLAGS = -O -s" \
	"LIBS  = -lsocket -lcurses" wermit

#DEC Ultrix 2.x
# Add -O, -DDYNAMIC, -s, etc, if they work.
ultrix2x:
	@echo Making C-Kermit $(CKVER) for Ultrix 2.x ...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET -DDU2 $(KFLAGS)"

du2:
	$(MAKE) "MAKE=$(MAKE)" ultrix2x

#DEC Ultrix 3.1 (and probably also 3.0)
ultrix3x:
	@echo Making C-Kermit $(CKVER) for Ultrix 3.x...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET -DDIRENT -DSIG_V \
	-DDYNAMIC $(KFLAGS) -O" "LNKFLAGS = -s"
du3:
	$(MAKE) "MAKE=$(MAKE)" ultrix3x


#DEC Ultrix 4.0 or 4.1 on DECstation, VAXstation, VAX, etc.
ultrix40:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.0 or 4.1...
	$(MAKE) wermit \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DDYNAMIC -DDU4 \
	$(KFLAGS) -Olimit 1450" "LNKFLAGS = -s"

du4:
	$(MAKE) "MAKE=$(MAKE)" ultrix4x

#DEC Ultrix 4.2-4.5 on DECstation, DECsystem, VAXstation, VAX, etc.
#Like du4, except now C compiler supports -O2 optimization.
ultrix42:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.2 or later...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DDYNAMIC \
	$(KFLAGS) -O2 -Olimit 1750" "LNKFLAGS = -s"

du42:
	$(MAKE) "MAKE=$(MAKE)" ultrix42

#DEC Ultrix 4.2-4.5 on DECstation, DECsystem, VAXstation, VAX, etc.
#Like du42, but with curses support added
ultrix42c:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.2 or later...
	@echo WARNING - If you have problems with the executable
	@echo then rebuild without -O2.
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DDYNAMIC \
	-DCK_CURSES $(KFLAGS) -O2 -Olimit 1750" "LNKFLAGS = -s" \
	"LIBS= -lcurses -ltermcap"

du42c:
	$(MAKE) "MAKE=$(MAKE)" ultrix42c

#DEC Ultrix 4.3A or later on DECsystem and DECstation 5000/50, /150 or /260
#with MIPS R4x00 processor.  The "-mips3" switch generates R4000-specific
#code, which is faster and more compact, but *won't* run on earlier
#DECsystems and DECstations.
ultrix43-mips3:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.3A or later, R4000 cpu...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DDYNAMIC \
	$(KFLAGS) -O2 -Olimit 1750 -mips3" "LNKFLAGS = -s -mips3"

du43-mips3:
	$(MAKE) "MAKE=$(MAKE)" ultrix43-mips3

#DEC Ultrix 4.3A or later on MIPS R4x000 based systems.
#Like ultrix43-mips3 but with curses support added
ultrix43c-mips3:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.3A or later, R4000 cpu...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DDYNAMIC \
	-DCK_CURSES $(KFLAGS) -O2 -Olimit 1750 -mips3" "LNKFLAGS = -s -mips3" \
	"LIBS= -lcurses -ltermcap"

du43c-mips3:
	$(MAKE) "MAKE=$(MAKE)" ultrix43c-mips3

#DEC Ultrix 4.4 on DECstation 5000/50 or /150 with R4000 MIPS processor,
#or 5000/260 with R4400.  The "-mips3" switch generates R4000-specific code,
#which is faster and more compact but *won't* run on earlier DECstations.
ultrix44-mips3:
	@echo Making C-Kermit $(CKVER) for Ultrix 4.4, R4000 cpu ...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET -DSIG_V -DDYNAMIC \
	$(KFLAGS) -O2 -Olimit 1450 -mips3" "LNKFLAGS = -s -mips3"

du44-mips3:
	$(MAKE) "MAKE=$(MAKE)" ultrix44c-mips3

#DEC Ultrix 4.2 on DECstation, VAXstation, VAX, etc, System V R4 environment
ultrix42s5r4:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4 on Ultrix...'
	$(MAKE) wermit \
	"CFLAGS = -O2 -Olimit 1500 -DSVR4 -DDIRENT -DHDBUUCP \
	-DDYNAMIC -DTCPSOCKET $(KFLAGS)" "LNKFLAGS = -s"

#OSF/1
osf:
	$(MAKE) wermit \
	"CFLAGS= -DBSD4 -DOSF -D_BSD -DTCPSOCKET -DCK_ANSIC -DSIG_V \
	-DDYNAMIC -DKANJI -DCK_CURSES -DCK_RTSCTS $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lbsd -lcurses -ltermcap"

#DEC OSF/1 V1.0-1.3 on DECstation, VAX, Alpha, or PC.
dec-osf:
	@echo Making C-Kermit $(CKVER) for DEC OSF/1 V1.x...
	@echo If you are building for DEC OSF/1 2.0, please use dec-osf20.
	@echo Remove or adjust -O2 and/or -Olimit if they cause trouble.
	$(MAKE) osf "KFLAGS= -O2 -Olimit 1460 $(KFLAGS)"

#DEC OSF/1 2.0 on Alpha and probably nowhere else.
#The only difference from OSF/1 is that optimization is omitted.
#The optimized version gets strange runtime errors, like the PAUSE command
#not working.  Add "-unsigned" to make all chars unsigned.
dec-osf20:
	@echo Making C-Kermit $(CKVER) for DEC OSF/1 V2.0...
	@echo Optimization omitted because it causes runtime errors.
	@echo See comments in makefile.
	$(MAKE) osf "KFLAGS= -DOSF20 $(KFLAGS)"

dec-osf30:
	@echo Making C-Kermit $(CKVER) for DEC OSF/1 V3.0...
	$(MAKE) osf "KFLAGS= -DOSF30 -O2 -Olimit 1760 $(KFLAGS)"

#Digital UNIX 3.2
du32:
	@echo Making C-Kermit $(CKVER) for Digital UNIX 3.2...
	$(MAKE) osf \
	"KFLAGS= -DOSF32 -DHDBUUCP -O2 -Olimit 1760 $(KFLAGS)"

dec-osf32:
	$(MAKE) "MAKE=$(MAKE)" du32

du40:
	@echo Making C-Kermit $(CKVER) for Digital UNIX 4.0...
	$(MAKE) osf \
	"KFLAGS= -DOSF40 -DHDBUUCP -unsigned -std1 -O3 -Olimit 1760 $(KFLAGS)"

dec-osf40:
	$(MAKE) "MAKE=$(MAKE)" du40

du40-ridiculous-checking:
	@echo Making C-Kermit $(CKVER) for Digital UNIX 4.0.
	@echo Checking everything - assumes DECC...
	$(MAKE) osf "KFLAGS= -DOSF40 -DHDBUUCP -w0 -warnprotos -check \
	-portable -unsigned -std1 -O3 -Olimit 1760 $(KFLAGS)"

#Sequent DYNIX/ptx 1.2.1
dynixptx12:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX/ptx 1.2.1...
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DPTX \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#Sequent DYNIX/ptx 1.3 or 1.4
dynixptx13:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX/ptx 1.3 TCP/IP...
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DPTX -DCK_POLL -O \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DTCPSOCKET $(KFLAGS) -i" \
	"LNKFLAGS = -i" "LIBS = -lsocket -linet -lnsl"

#Sequent DYNIX/ptx 2.0, ANSI C compilation
#Should work on any hardware platform when DYNIX/ptx runs, including
#386, 486, Pentium.
dynixptx20:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 2.0...'
	$(MAKE) wermit \
	"CFLAGS= -DPOSIX -DHDBUUCP -DDYNAMIC -DTCPSOCKET \
	-DWAIT_T=int -DNOSETBUF -DPTX  -O" \
	"LIBS = -lsocket -linet -lnsl"

#Sequent DYNIX/ptx 2.0, ANSI C compilation, with curses
dynixptx20c:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 2.0...'
	$(MAKE) wermit \
	"CFLAGS= -DPOSIX -DHDBUUCP -DDYNAMIC -DTCPSOCKET \
	-DWAIT_T=int -DNOSETBUF -DPTX  -DCK_CURSES -DCK_NEWTERM -O" \
	"LIBS = -lsocket -linet -lnsl -lcurses -ltermcap"

#Sequent DYNIX/ptx 4.0, ANSI C compilation, with curses
dynixptx41c:
	@echo 'Making C-Kermit $(CKVER) for POSIX, Sequent DYNIX/ptx 4.1...'
	$(MAKE) wermit \
	"CFLAGS= -DPOSIX -DHDBUUCP -DDYNAMIC -DTCPSOCKET \
	-DWAIT_T=int -DNOSETBUF -DPTX  -DCK_CURSES -DCK_NEWTERM -O" \
	"LIBS = -lsocket -lnsl -lcurses -ltermcap"

#Sequent DYNIX 3.0.x
dynix3:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX 3.0.x...
	$(MAKE) wermit \
	"CFLAGS= -DBSD43 -DACUCNTRL -DTCPSOCKET -O \
	-DPWUID_T=int -DGID_T=int $(KFLAGS)"

#Sequent DYNIX 3.0.x, no ACUCNTRL
dynix3noacu:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX 3.0.x...
	$(MAKE) wermit "CFLAGS= -DBSD43 -DLCKDIR -DTCPSOCKET -O \
	-DUID_T=int -DGID_T=int $(KFLAGS)"

#Sequent DYNIX 3.1.x
dynix31:
	@echo Making C-Kermit $(CKVER) for Sequent DYNIX 3.1.x...
	$(MAKE) wermit \
	"CFLAGS= -O -DDYNAMIC -DDCLPOPEN -DLCKDIR -DBSD4 -DTCPSOCKET $(KFLAGS)"

#Sequent DYNIX 3.1.2, as above but with curses, to be compiled by gcc 2.3.3.
dynix31c:
	@echo 'Making C-Kermit $(CKVER) for Sequent DYNIX 3.1.2, curses...'
	$(MAKE) wermit "CFLAGS= -O2 -DDYNAMIC -DDCLPOPEN -DACUCNTRL \
	-DBSD43 -DTCPSOCKET -DCK_CURSES -DNOSETBUF -DUID_T=int \
	$(KFLAGS)" "LIBS= -lcurses -ltermcap"

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
	$(MAKE) "MAKE=$(MAKE)" PARALLEL=4 wermit \
	"CFLAGS= -DBSD43 -DENCORE -DDYNAMIC -DKANJI -DTCPSOCKET $(KFLAGS) -O"

#Encore, UMAX 4.2 (BSD)
umax42:
	@echo Making C-Kermit $(CKVER) for Encore UMAX 4.2...
	$(MAKE) "MAKE=$(MAKE)" PARALLEL=4 wermit \
	"CFLAGS= -DBSD4 -DENCORE -DDYNAMIC -DTCPSOCKET $(KFLAGS) -O"

#Encore 88K UMAX 5.3 with TCP/IP support
encore88k:
	@echo 'Making C-Kermit $(CKVER) for Encore 88K UMAX V, TCP/IP...'
	$(MAKE) wermit \
	"CFLAGS = -q ext=pcc -DSVR3 -DTCPSOCKET -DDIRENT \
	-DNOGETID_PROTOS -DHDBUUCP -DDYNAMIC $(KFLAGS) -O" "LNKFLAGS ="

#Encore 88K UMAX 5.3 with TCP/IP support
encore88kgcc:
	@echo 'Making C-Kermit $(CKVER) for Encore 88K UMAX V, TCP/IP, gcc...'
	$(MAKE) wermit CC=gcc CC2=gcc \
	"CFLAGS = -DSVR3 -DTCPSOCKET -DDIRENT \
	-DNOGETID_PROTOS -DHDBUUCP -DDYNAMIC $(KFLAGS) -O" "LNKFLAGS ="

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

#Convex C1 with Berkeley Unix
convex:
	@echo Making C-Kermit $(CKVER) for Convex C1 / BSD...
	$(MAKE) wermit "CFLAGS= -DBSD4 $(KFLAGS) -Dmsleep=mnap"

#Convex C210 with Convex/OS 8
convex8:
	@echo Making C-Kermit $(CKVER) for Convex C210 with OS 8
	$(MAKE) wermit \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DNODEBUG -DDIRENT -DNOFILEH \
	-DDYNAMIC $(KFLAGS) -DSIG_V -Dmsleep=mnap"

#Convex C2 with Convex OS 9.1 (should also work with 8.1 or later)
#with ANSI C compiler, uses BSD 4.3 uucp lockfile convention.
convex9:
	@echo Making C-Kermit $(CKVER) for Convex C210 with OS 9.1
	$(MAKE) wermit \
	"CFLAGS= -DPOSIX -DCONVEX9 -DNOIEXTEN -DDIRENT -DNOFILEH -DTCPSOCKET \
	-DDYNAMIC -D__STDC__ -DLCKDIR -Dmsleep=mnap -O -ext -tm c1 $(KFLAGS)" \
	"LNKFLAGS = -ext"

#Convex C2 with Convex OS 10.1 or later
#with gcc 2.x C compiler
convex10gcc:
	@echo Making C-Kermit $(CKVER) for Convex C2 with OS 10.1 using gcc
	$(MAKE) wermit \
	"CFLAGS= -DPOSIX -DCONVEX9 -DNOIEXTEN -DDIRENT -DNOFILEH -DTCPSOCKET \
	-DDYNAMIC -D__STDC__  -Dmsleep=mnap -O2 $(KFLAGS)" CC=gcc CC2=gcc

#Cray X-MP or Y-MP UNICOS 6.x or 7.x.
#NOTE: NPROC tells how many parallel makes to run.  If your Cray has multiple
#processors, you can set NPROC up to the number of CPUs, e.g. NPROC=16.
cray:
	@echo 'Making C-Kermit $(CKVER) for Cray X/Y-MP UNICOS 6.x or 7.0...
	$(MAKE) wermit NPROC=1 \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DTCPSOCKET $(KFLAGS) -O1"

#Cray X-MP or Y-MP UNICOS 8.0 Alpha.
cray8:
	@echo 'Making C-Kermit $(CKVER) for Cray X/Y-MP UNICOS 8.0 Alpha...
	$(MAKE) wermit NPROC=1 \
	"CFLAGS= -DSVR4 -DDIRENT -DHDBUUCP -DTCPSOCKET $(KFLAGS) -O1"

#Cray-2 or Cray 3-CSOS
#NOTE: NPROC tells how many parallel makes to run.  If your Cray has multiple
#processors, you can set NPROC up to the number of CPUs, e.g. NPROC=16.
craycsos:
	@echo 'Making C-Kermit $(CKVER) for Cray-2/3 CSOS
	$(MAKE) wermit NPROC=1 \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DTCPSOCKET \
	$(KFLAGS) -DCK_ANSIC -DCK_CURSES" "LIBS=-lnet"

#Charles River Data Systems Universe with UNOS Version 9.2
crds:
	@echo 'Making C-Kermit $(CKVER) for Charles River Data Systems...'
	make wermit \
	"CFLAGS = -DATTSV -DNOANSI -DDIRENT -DDYNAMIC -DLONGFN -DTCPSOCKET \
	-DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DNOSETREU \
	-Dsuspend=ksuspend $(KFLAGS) -O" "LNKFLAGS ="

#NeXTSTEP 1.0 through 3.2.
#Includes fullscreen file transfer display (curses) and TCP/IP support.
#Uses shared library to make executable program about 80K smaller.
#Remove "LIBS = -lsys_s" if this causes trouble.
next:
	@echo Making C-Kermit $(CKVER) for NeXTSTEP...
	@echo 'If you get errors in ckutio.c about w_S, w_T, etc,'
	@echo 'add KFGLAGS=-DNOREDIRECT to your make command.'
	$(MAKE) wermit \
	"CFLAGS= -DNEXT -DNOSETBUF -DTCPSOCKET -DDYNAMIC -DLCKDIR -DKANJI \
	-DCK_CURSES $(KFLAGS) -O -w" "LIBS = -lsys_s -lcurses -ltermcap"

nextc:
	$(MAKE) "MAKE=$(MAKE)" next

nextg:
	$(MAKE) "MAKE=$(MAKE)" next KFLAGS=-Wall

nextgc:
	$(MAKE) "MAKE=$(MAKE)" next KFLAGS=-Wall

#NeXTSTEP 3.3.
#Includes fullscreen file transfer display and TCP/IP.
next33:
	@echo Making C-Kermit $(CKVER) for NeXTSTEP 3.3...
	$(MAKE) wermit \
	"CFLAGS= -DNEXT33 -DNOSETBUF -DTCPSOCKET -DDYNAMIC -DLCKDIR -DKANJI \
	-DCK_CURSES $(KFLAGS) -O -w" "LIBS = -lsys_s -lcurses -ltermcap"

#NeXT with malloc debugger
nextmd:
	@echo Making C-Kermit $(CKVER) for NeXT with malloc debugging...
	$(MAKE) mermit \
	"CFLAGS= -DNEXT -DNOSETBUF -DTCPSOCKET -DDYNAMIC -DLCKDIR -DKANJI \
	-DCK_CURSES $(KFLAGS) -O -w -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG" \
	"LIBS = -lsys_s -lcurses -ltermcap"

#Build for NeXTSTEP with "fat" binaries (MABs) that run on both Motorola
#and Intel platforms.
nextfat:
	$(MAKE) "MAKE=$(MAKE)" next "KFLAGS=-Wall -arch m68k -arch i386" \
	"LNKFLAGS = -arch m68k -arch i386"

#NeXTSTEP on Intel Platforms.
next486:
	@echo Making C-Kermit $(CKVER) for NeXTSTEP on Intel Platforms...
	@echo 'If you get errors in ckutio.c about w_S, w_T, etc,'
	@echo 'add KFGLAGS=D-DNOREDIRECT to your make command.'
	$(MAKE) wermit \
	"CFLAGS= -DNEXT -DNOSETBUF -DTCPSOCKET -DDYNAMIC -DLCKDIR -DKANJI \
	-DNODEBUG -O3 -fno-omit-frame-pointer -fschedule-insns2 -pipe \
	-DCK_CURSES $(KFLAGS) -w" "LIBS = -lsys_s -lcurses -ltermcap"

#BeBox
beboxdr7:
	@echo 'Making C-Kermit $(CKVER) for the BeBox...'
	@echo 'Link step will fail with default Metroworks linker 64K limit.'
	@echo 'Code Warrior Gold required to link big programs.'
	$(MAKE) wermit \
	"CC=/boot/develop/tools/mwcc" "CC2=/boot/develop/tools/mwld" \
	"CFLAGS= -DBEBOX -DBE_DR_7 -DPOSIX -DNOUUCP $(KFLAGS) -O"

#BeBox
bebox:
	@echo 'Making C-Kermit $(CKVER) for the BeBox...'
	@echo 'Link step will fail with default Metroworks linker 64K limit.'
	@echo 'Code Warrior Gold required to link big programs.'
	$(MAKE) wermit \
	"CC=/boot/develop/tools/mwcc" "CC2=/boot/develop/tools/mwld" \
	"CFLAGS= -DBEBOX -DPOSIX -DNOUUCP $(KFLAGS) -O"

#Plan 9 from Bell Labs
plan9:
	make -f ckpker.mk

#POSIX
posix:
	@echo 'Making C-Kermit $(CKVER) for POSIX, no UUCP lockfile support...'
	$(MAKE) wermit "CFLAGS= -DPOSIX -DNOUUCP $(KFLAGS) -O"

#Berkeley Software Design Inc. BSDI
# Substitute "LIBS= -lnewcurses -ltermcap" if desired.
bsdi:
	@echo 'Making C-Kermit $(CKVER) for BSD/386 ...'
	$(MAKE) wermit \
	"CFLAGS= -DBSD44 -DSETREUID -DSW_ACC_ID \
	-DTCPSOCKET -DCK_CURSES -DDYNAMIC -DBPS_57K $(KFLAGS) -O" \
	"LIBS= -lcurses -ltermcap"

# (old name for the above)
bsdiposix:
	$(MAKE) "MAKE=$(MAKE)" bsdi

# make 386bsd 0.0new, posix
# for  386bsd 0.1.24, change /usr/include/termios.h to #define NCCS if
#  _POSIX_SOURCE is #defined. (source: lewine, posix prgmrs guide, o`reilly)
#NOTE: Lock directory is /var/spool/lock.  Formerly, it was /var/spool/uucp,
#but reportedly <wjones@halcyon.com> that was due to a typo in 'man tip'.
386bsd:
	@echo 'Making C-Kermit $(CKVER) for jolix 386BSD 0.0new and 0.1.24...'
	$(MAKE) wermit \
	"CFLAGS= -DPOSIX -DSETREUID -DPIDSTRING -DUSLEEP \
	-D_386BSD -DCK_CURSES -DTCPSOCKET -DDYNAMIC -DNOSETBUF \
	-DLOCK_DIR=\\\"/var/spool/lock\\\" \
	$(KFLAGS) -O" "LNKFLAGS = -s" "LIBS = -lcurses -ltermcap"

freebsd:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 1.0 Release'
	$(MAKE) wermit \
	"CFLAGS= -DBSD44 -DCK_CURSES -DDYNAMIC -DTCPSOCKET -DNOCOTFMC \
	$(KFLAGS) -O" "LIBS= -lcurses -ltermcap"

freebsd2:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 2.0 Release'
	$(MAKE) freebsd "KFLAGS=-DNDSYSERRLIST"

#Pyramid 9XXX (e.g. 9845) or MIServer T series, OSx 4.4b thru 5.1
pyramid:
	@echo Making C-Kermit $(CKVER) for Pyramid Dual Port OSx
	ucb $(MAKE) wermit "CFLAGS= -DBSD43 -DTCPSOCKET -DDYNAMIC \
	-DPYRAMID -O $(KFLAGS)" "LNKFLAGS = -s"

#Pyramid Dual Port OSx using HonyDanBer UUCP, curses and TCP
pyramid-hdb:
	@echo Making C-Kermit $(CKVER) for Pyramid Dual Port OSx
	ucb $(MAKE) wermit "CFLAGS= -DBSD43 -DTCPSOCKET -DDYNAMIC \
	-DHBDUUCP -DCK_CURSES -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lcurses -ltermcap"

#Pyramid DC/OSx (UNIX System V R4).
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C constructs, <sys/termiox.h>, etc etc.
#NOTE: Remove -O and Olimit:1500 from CFLAGS if TELNET connections do not work.
pyrdcosx:
	@echo 'Making C-Kermit $(CKVER) for Pyramid DC/OSx...'
	$(MAKE) wermit \
	"CFLAGS = -Xa -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DCK_CURSES -DSTERMIOX -DTCPSOCKET -DPYRAMID -K Olimit:1500 \
	$(KFLAGS)" "LIBS= -lcurses -lsocket -lnsl" "LNKFLAGS = -s"

#SONY NEWS, NEWS-OS 4.01C
sonynews:
	@echo Making C-Kermit $(CKVER) for SONY NEWS-OS 4.01C...
	$(MAKE) wermit "CFLAGS= -DBSD43 -DKANJI -DACUCNTRL \
	-DTCPSOCKET -O"

#SUNPOSIX
#SunOS 4.1.x in the POSIX universe (but with HDB UUCP added)...
sunposix:
	@echo Making C-Kermit $(CKVER) for POSIX...
	$(MAKE) wermit "CC= /usr/5bin/cc " "CC2= /usr/5bin/cc " \
	"CFLAGS= -DPOSIX -DHDBUUCP -DVOID=void -O"

#IBM's AIX 3.0 on IBM 370 mainframe, tested on AIX F44 thru F50.
aix370:
	@echo Making C-Kermit $(CKVER) for IBM System/370 AIX 3.0...
	$(MAKE) wermit \
	"CFLAGS= -DAIX370 -DTCPSOCKET -DLCKDIR -DDIRENT -DDYNAMIC $(KFLAGS)" \
	"LIBS = -lbsd"

#IBM's AIX/ESA 2.1 (OSF/1) on IBM mainframe
aixesa:
	@echo Making C-Kermit $(CKVER) for IBM AIX/ESA...
	$(MAKE) wermit \
	"CFLAGS= -DAIXESA -DDYNAMIC -DTCPSOCKET $(KFLAGS) -O"

#IBM's AIX 1.3 on IBM PS/2, tested on AIX F44 thru F50.
#This is exactly the same as AIX370 except for the version herald.
ps2aix3:
	@echo Making C-Kermit $(CKVER) for IBM PS/2 AIX 3.0...
	$(MAKE) wermit \
	"CFLAGS= -DAIXPS2 -DTCPSOCKET -DLCKDIR -DDIRENT -DDYNAMIC $(KFLAGS)" \
	"LIBS = -lbsd"

#IBM AIX 3.0, 3.1, or 3.2 for RISC System/6000.
rs6000:
	@echo Making C-Kermit $(CKVER) for IBM AIX 3.0 or 3.1, RS/6000...
	$(MAKE) wermit \
	"CFLAGS= -DAIXRS -DTCPSOCKET -DSVR3 -DDIRENT -DDYNAMIC -DCK_ANSIC \
	-DCK_POLL -DNOSETBUF -DCLSOPN -DKANJI -DSELECT_H -O $(KFLAGS)" \
	"LNKFLAGS = -s"

#IBM AIX 3.0, 3.1, or 3.2 for RISC System/6000, with curses.
rs6000c:
	@echo Making C-Kermit $(CKVER) for IBM AIX 3.0 or 3.1, RS/6000...
	$(MAKE) wermit \
	"CFLAGS= -DAIXRS -DTCPSOCKET -DSVR3 -DDIRENT -DDYNAMIC -DCK_ANSIC \
	-DCK_POLL -DNOSETBUF -DCLSOPN -DCK_CURSES -DKANJI  -DSELECT_H \
	-O $(KFLAGS)" "LIBS= -lcurses -ltermcap" "LNKFLAGS = -s"

#IBM AIX 3.2 for RISC System/6000.
#In case of "subprogram too complex" warnings, add "-qmaxmem=4000" to CFLAGS.
rs6aix32:
	@echo Making C-Kermit $(CKVER) for IBM AIX 3.2, RS/6000...
	$(MAKE) wermit \
	"CFLAGS= -DAIXRS -DTCPSOCKET -DSVR4 -DDIRENT -DDYNAMIC -DCK_ANSIC \
	-DSELECT_H -DCLSOPN -DKANJI -O $(KFLAGS)" "LNKFLAGS = -s"

#IBM AIX 3.2 for RISC System/6000.
rs6aix32c:
	@echo Making C-Kermit $(CKVER) for IBM AIX 3.2, RS/6000, TCP+curses...
	@echo In case of Subprogram Too Complex warnings,
	@echo add -qmaxmem=4000 to CFLAGS.
	$(MAKE) wermit \
	"CFLAGS= -DAIXRS -DTCPSOCKET -DSVR4 -DDIRENT -DDYNAMIC -DCK_ANSIC \
	-DCLSOPN -DCK_CURSES -DKANJI -DSELECT_H -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS=-lcurses"

#IBM AIX 4.1 (Beta) or AIX 4.1.1 for RISC System/6000 or Power Series
rs6aix41c:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.1.1 RS/6000 or PowerPC...
	$(MAKE) wermit \
	"CFLAGS= -DAIXRS -DAIX41 -DSVR4 -DSTERMIOX -DTCPSOCKET -DDIRENT \
	-DDYNAMIC -DCK_ANSIC -DCLSOPN -DCK_CURSES -DKANJI -DSELECT -DSELECT_H \
	-qmaxmem=6000 -O $(KFLAGS)" "LNKFLAGS = -s" "LIBS=-lcurses"

#IBM AIX 4.1 (Beta) or AIX 4.1.1 for RISC System/6000 or Power Series
rs6aix41d:
	@echo Making C-Kermit $(CKVER) for IBM AIX 4.1.1 - debugging...
	$(MAKE) wermit \
	"CFLAGS= -g -DAIXRS -DAIX41 -DSVR4 -DSTERMIOX -DTCPSOCKET -DDIRENT \
	-DDYNAMIC -DCK_ANSIC -DCLSOPN -DCK_CURSES -DKANJI -DSELECT -DSELECT_H \
	-DRLOGCODE -qcheck -qextchk -qinfo -qproto -qflag=i:i -langlvl=ansi \
	-qtbtable=full -bnoquiet $(KFLAGS)" "LNKFLAGS = -s" "LIBS=-lcurses"

#Bull DPX/2 with BOS/X, somewhat like AIX/RS6000
bulldpx2:
	@echo Making C-Kermit $(CKVER) for Bull DPX/2 with BOS/X...
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DDYNAMIC -DCK_ANSIC -DCKTYP_H=<sys/types.h> \
	-DCK_POLL -DNOSETBUF -DCLSOPN -O $(KFLAGS)" "LNKFLAGS = -s"

#Sun UNIX 3.5 with gcc 2.3.3.
sunos3gcc:
	@echo Making C-Kermit $(CKVER) for Sun UNIX 3.5 and gcc...
	$(MAKE) wermit CC=gcc CC2=gcc \
	CFLAGS="-g -O -DBSD4 -DTCPSOCKET $(KFLAGS)"

#SunOS version 4.0, BSD environment, has saved original euid feature.
# Add "CC=/usr/ucb/cc CC2=/usr/ucb/cc" if necessary.
sunos4:
	@echo Making C-Kermit $(CKVER) for SunOS 4.0, BSD environment...
	$(MAKE) wermit \
	"CFLAGS= -O -DSUNOS4 -DDIRENT -DTCPSOCKET -DSAVEDUID \
	-DKANJI -DDYNAMIC $(KFLAGS)"

#As above, but with SunLink X.25 support
sunos4x25:
	@echo SunLink X.25 support
	$(MAKE) "MAKE=$(MAKE)" sunos4 "KFLAGS=$(KFLAGS) -DSUNX25"

#SUN OS version 4.1 - 4.1.3, BSD environment, has saved original euid feature.
#Uses Honey DanBer UUCP.  Requires presence of /usr/spool/locks directory.
# /var/spool/ should be a symbolic link to  /usr/spool/.
# ... or 'make wermit "CC= /usr/ucb/cc " \'
sunos41:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 / BSD...
	$(MAKE) wermit "CFLAGS= -O -DSUNOS41 -DHDBUUCP -DDIRENT -DTCPSOCKET \
	-DNOSETBUF -DKANJI -DSAVEDUID -DDYNAMIC -DHADDRLIST $(KFLAGS)"

#As above, but compile with gcc.  Gives 24-32K size reduction
#with gcc 2.1 or 2.2.2.  CAUTION: make sure "fixincludes" has been run on
#the include files, so gcc's are in sync with the regular Sun ones!
#This includes the curses library for fullscreen file transfer display.
#NDGPWNAM needed for GCC 2.5.6, not needed for 2.4.0, but it's uncertain
#whether it will do any harm for 2.4.0 compilation -- if so, remove it.
sunos41gcc:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1/BSD with gcc and curses...
	$(MAKE) wermit "CC= gcc " "CC2= gcc" \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DDIRENT -DTCPSOCKET -DNOSETBUF \
	-DNDGPWNAM -DSAVEDUID -DDYNAMIC -DKANJI -DCK_CURSES $(KFLAGS)" \
	"LIBS= -lcurses -ltermcap"

# Tenon MachTen, tested on Apple Powerbook with MachTen 2.1.1.D.
# NOTE: This doesn't do anything about UUCP.  It only works if /usr/spool/uucp
# has permission of 777, and dialout device is world read/writeable.
machten:
	@echo Making C-Kermit $(CKVER) for MachTen...
	$(MAKE) wermit "CFLAGS= -DBSD43 -DTCPSOCKET -DSIG_V -DNDGPWNAM \
	-DCK_CURSES -O $(KFLAGS)"  "LIBS=-lcurses -ltermcap"

#SUNOS 4.1 as sunos41 above, but also with curses support
sunos41c:
	@echo Curses support
	$(MAKE) "MAKE=$(MAKE)" sunos41 "KFLAGS=$(KFLAGS) -DCK_CURSES" \
	"LIBS= -lcurses -ltermcap"

#SUNOS 4.1 with SunLink X.25 support
sunos41x25:
	@echo SunLink X.25 support
	$(MAKE) "MAKE=$(MAKE)" sunos41 "KFLAGS=$(KFLAGS) -DSUNX25"

#SUNOS 4.1 with SunLink X.25 support and curses
sunos41x25c:
	@echo SunLink X.25 support, curses
	$(MAKE) "MAKE=$(MAKE)" sunos41c "KFLAGS=$(KFLAGS) -DSUNX25"

# NOTE: The sunsolxx and sunos5xx entries should no longer be necessary, but
# are retained just in case.  Instead, please use the solarisxx entries.

#SUN with Solaris 2.0 = SunOS 5.0.
#Mostly the same as System V R4.
sunsol20:
	@echo 'Making C-Kermit $(CKVER) for Sun with Solaris 2.0 and curses...'
	$(MAKE) wermit "CFLAGS = -O -DSVR4 -DSOLARIS \
	-DDIRENT -DHDBUUCP -DDYNAMIC -DSTERMIOX -DTCPSOCKET -DCK_CURSES \
	-DCK_POLL $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermlib" "LNKFLAGS = -s"

#SUN with Solaris 2.0.
#As above, but built with the gcc compiler from the Cygnus CD-ROM.
sunsol20gcc:
	@echo 'Making C-Kermit $(CKVER) for Sun Solaris 2.0, gcc, and curses..'
	$(MAKE) wermit "CFLAGS = -O -DSVR4 -DSOLARIS \
	-DDIRENT -DHDBUUCP -DDYNAMIC -DSTERMIOX -DTCPSOCKET -DCK_CURSES \
	-DCK_POLL $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermlib" "LNKFLAGS = -s" \
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
#(such as 1.057) work OK, use: "make -e sunos51 CC=apcc CC2=apcc".
sunos51:
	@echo 'Making C-Kermit $(CKVER) for SunOS 5.x....'
	$(MAKE) wermit \
	"CFLAGS = -O -Usun -DSVR4 -DSOLARIS -DDIRENT -DHDBUUCP \
	-DSELECT -DDYNAMIC -DNODEBUG -DSTERMIOX $(KFLAGS)" "LNKFLAGS = -s"

#C-Kermit for Solaris 2.x compiled with gcc, includes curses and TCP/IP.
#Change -O2 to -O if -O2 gives trouble.
#Remove -Usun if it causes trouble.
#Your PATH should start with something like:
#  /usr/local/gnu/bin:/usr/ccs/bin:
#Produces a huge executable -- strip with /usr/ccs/bin/strip (not Gnu strip).
#Also don't add "LNKFLAGS = -s" -- strip manually instead.
#Also note: this can NOT be linked statically - Sun makes it impossible.
#And for Solaris 2.4, you might have replace:
# /usr/local/lib/gcc-lib/i486-sun-solaris2/2.4.5/include/sys/stat.h
#with /usr/include/sys/stat.h.
solaris2xg:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x with GNU cc...'
	@echo 'Please read the comments that accompany this entry.'
	$(MAKE) wermit CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DDYNAMIC -DNOSETBUF \
	-DTCPSOCKET -DKANJI $(KFLAGS)" \
	"LIBS= -ltermlib -lsocket -lnsl"

#As above with SunLink X.25 added.
solaris2xg25:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x + X.25 with GNU cc...'
	@echo 'Please read the comments that accompany the solaris2xg entry.'
	$(MAKE) wermit CC=gcc CC2=gcc \
	"CFLAGS = -g -O -Usun -DSVR4 -DSOLARIS -DSTERMIOX -DSELECT -DSUNX25 \
	-DCK_CURSES -DCK_NEWTERM -DDIRENT -DHDBUUCP -DDYNAMIC -DNOSETBUF \
	-DTCPSOCKET -DKANJI $(KFLAGS)" \
	"LIBS= -ltermlib -L/opt/SUNWconn/lib -R/opt/SUNWconn/lib \
	-lsockx25 -lsocket -lnsl"

sunos51cgcc:
	$(MAKE) "MAKE=$(MAKE)" solaris2xg

#Solaris 2.x, SunPro compiler, includes curses and TCP/IP.
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
	$(MAKE) wermit \
	"CFLAGS = -O -Usun -i -DSVR4 -DDIRENT -DSOLARIS -DHDBUUCP \
	-DDYNAMIC -DSELECT -DCK_CURSES -DCK_NEWTERM -DSTERMIOX -DNOSETBUF \
	-DTCPSOCKET -DKANJI -DHADDRLIST $(KFLAGS)" "LNKFLAGS = -s" \
	"LIBS= -ltermlib -lsocket -lnsl"

#Solaris 2.x with SunLink X.25 support.
#This will only run if user has /opt/SUNWconn/lib/libsockx25.so.1
#exists and can be dynamically linked.
solaris2x25:
	@echo 'Making C-Kermit $(CKVER) for Solaris 2.x+X.25 with SunPro cc...'
	$(MAKE) wermit \
	"CFLAGS = -O -i -Usun -DSVR4 -DSOLARIS -DDIRENT \
	-DSUNX25 -DTCPSOCKET -DHADDRLIST -DHDBUUCP \
	-DDYNAMIC -DSELECT -DCK_CURSES -DCK_NEWTERM -DSTERMIOX -DNOSETBUF \
	-DKANJI $(KFLAGS)" "LNKFLAGS = -s" \
	"LIBS= -ltermlib -L/opt/SUNWconn/lib -R/opt/SUNWconn/lib \
	-lsockx25 -lsocket -lnsl"

sunos51tcp:
	$(MAKE) "MAKE=$(MAKE)" solaris2x

sunos51tcpc:
	$(MAKE) "MAKE=$(MAKE)" solaris2x

#The following sunosxxx entries are for debugging and testing only.

sunos41x:
	$(MAKE) wermit "CFLAGS= -O -DSUNOS41 -DDIRENT -DNOTLOG -DNOMSEND \
	-DNOUUCP -DNOSIGWINCH -DNOREDIRECT -DNOPUSH -DNOCCTRAP \
	-DNOSETBUF -DKANJI -DDYNAMIC -DNOICP -DNOLOCAL $(KFLAGS)"

#SunOS 4.1.x, debugging with Pure Software, Inc., Purify 2 (commercial runtime
#error-detection software for catching wild array references, etc).
#Before running the resulting wermit, you'll also need to define and export
#the following environment variables (as in this example):
#PURIFYHOME=/usr/local/purify ; export PURIFYHOME
#PURIFYCACHEDIR=/tmp ; export PURIFYCACHEDIR
sunos41cp:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 / BSD / Curses / Purify...
	$(MAKE) wermit \
	"CC2= purify -cache_dir=/usr/tmp cc" \
	"CFLAGS= -g -DSUNOS41 -DHDBUUCP -DDIRENT -DTCPSOCKET -DNOSETBUF \
	-DKANJI -DSAVEDUID -DDYNAMIC -DCK_CURSES $(KFLAGS)" \
	"LIBS= -lcurses -ltermcap"

#SunOS 4.1 with malloc debugger
sunos41md:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 malloc debug...
	$(MAKE) mermit \
	"CFLAGS= -O -DSUNOS41 -DHDBUUCP -DDIRENT -DTCPSOCKET \
	-DSAVEDUID -DDYNAMIC $(KFLAGS) -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG"

sunos41gmd:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1/BSD with gcc and curses...
	$(MAKE) mermit "CC= gcc " "CC2= gcc" \
	"CFLAGS= -g -DSUNOS41 -DHDBUUCP -DDIRENT -DTCPSOCKET -DNOSETBUF \
	-DNDGPWNAM -DSAVEDUID -DDYNAMIC -DKANJI -DCK_CURSES -DRLOGCODE \
	$(KFLAGS) -Dmalloc=dmalloc -Dfree=dfree -DMDEBUG" \
	"LIBS= -lcurses -ltermcap"

#SunOS version 4.1, gcc, profiling with gprof, no debugging.
#To get profile, "make sunos4p" (on Sun), then "./wermit".  After running
#wermit, "gprof ./wermit | lpr" (or whatever) to get execution profile.
sunos41p:
	@echo Making C-Kermit $(CKVER) for SunOS 4.x with profiling...
	$(MAKE) wermit "CC= gcc " "CC2= gcc" \
	"CFLAGS= -DSUNOS41 -DNODEBUG -DSAVEDUID -DDIRENT -DTCPSOCKET \
	-DNDGPWNAM -DDYNAMIC $(KFLAGS) -pg" "LNKFLAGS = -pg"

#SunOS version 4.1, BSD environment, minimum size...
sunos41m:
	@echo Minimum size
	$(MAKE) "MAKE=$(MAKE)" sunos41 \
	"KFLAGS=$(KFLAGS) -DNODIAL -DNOHELP -DNODEBUG -DNOTLOG -DNOCKSPEED \
	-DNOSCRIPT -DNOCSETS -DNOICP -DNOMSEND -UTCPSOCKET" \
	"LNKFLAGS = -s"

#SunOS version 4.1 or later, BSD environment, minimum size w/command parser.
sunos41mi:
	@echo Minimum interactive
	$(MAKE) "MAKE=$(MAKE)" sunos41 \
	"KFLAGS=-DNOSPL -DNOXMIT -DNOMSEND -DNOFRILLS -DNORETRY -DNOCKSPEED \
	-DNODIAL -DNOHELP -DNODEBUG -DNOTLOG -DNOSCRIPT -DNOCSETS \
	-DNOSHOW -DNOSETKEY -DNOSERVER -DNOUUCP -DNORECALL -DNOREDIRECT \
	-DNOSETBUF -DNOPUSH -DNOMDMHUP -DNOJC -DNOFDZERO -DNOESCSEQ \
	-DNOCMDL -UTCPSOCKET $(KFLAGS)" "LNKFLAGS = -s"

#SunOS 4.1, System V R3 environment (-i option omitted).
sunos41s5:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 System V R3...
	@echo For testing purposes only - NOT for production use.
	@echo For a useable version, make sunos41 instead.
	$(MAKE) wermit "CC= /usr/5bin/cc " "CC2=/usr/5bin/cc " \
	"CFLAGS = -DSUN4S5 -DDIRENT -DHDBUUCP -DDYNAMIC -DCK_POLL $(KFLAGS) -O"

#As above, but with curses support
sunos41s5c:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 System V R3...
	@echo Curses included.
	@echo For testing purposes only - NOT for production use.
	@echo For a useable version, make sunos41 instead.
	$(MAKE) wermit "CC= /usr/5bin/cc " "CC2=/usr/5bin/cc " \
	"CFLAGS = -DSUN4S5 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DCK_POLL -DCK_CURSES -DCK_NEWTERM $(KFLAGS) -O" "LIBS= -lcurses"

#As above, but with curses support AND net support
sunos41s5tcpc:
	@echo Making C-Kermit $(CKVER) for SunOS 4.1 System V R3...
	@echo TCP/IP and curses included.  No debug log.
	@echo For testing purposes only - NOT for production use.
	@echo For a useable version, make sunos41 instead.
	$(MAKE) wermit "CC= /usr/5bin/cc " "CC2=/usr/5bin/cc " \
	"CFLAGS = -DSUN4S5 -DDIRENT -DHDBUUCP -DDYNAMIC -DNOSETBUF -DCK_POLL \
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
	$(MAKE) wermit \
	"CFLAGS= -O -DDYNAMIC -DBSD43 -DTCPSOCKET -DCK_CURSES \
	-Uaegis $(KFLAGS)" "LIBS= -lcurses -ltermcap"

#Apollo with Domain SR10.0 or later, System V R3 environment.
#Don't use the optimizer (-O), it causes problems at runtime.
sr10-s5r3:
	@echo Making C-Kermit $(CKVER) for Apollo SR10.0 / Sys V R3 ...
	$(MAKE) wermit \
	"CFLAGS= -DNOFILEH -DSVR3 $(KFLAGS) -Uaegis -U__STDC__"

#Apollo Domain/IX (untested, try this if sr10-bsd doesn't work)
# -DTCPSOCKET -DYNAMIC can probably be added here.
apollobsd:
	@echo Making C-Kermit $(CKVER) for Apollo Domain/IX...
	$(MAKE) wermit "CC= /bin/cc " "CC2= /bin/cc " \
	"CFLAGS= -DNOFILEH -DBSD4 $(KFLAGS) -Uaegis"

#Version 7 Unix (see comments near top of makefile)
v7:
	@echo Making C-Kermit $(CKVER) for UNIX Version 7.
	@echo Read the makefile if you have trouble with this...
	$(MAKE) wermit "CFLAGS=-DV7 -DPROCNAME=\\\"$(PROC)\\\" \
	-DBOOTNAME=\\\"$(BOOTFILE)\\\" -DNPROCNAME=\\\"$(NPROC)\\\" \
	-DNPTYPE=$(NPTYPE) $(DIRECT) -DO_RDWR=2 -DO_NDELAY=0 -DO_SCCS_ID \
	$(KFLAGS)"

#AT&T UNIX System V R3, signal() is void rather than int.
#Uses dirent.h and Honey DanBer UUCP.
#Add the -i link option if necessary.
#If you get errors like "ws_row undefined" in ckutio.c, add -DNONAWS.
sys5r3:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3...'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC $(KFLAGS) -O" \
	"LNKFLAGS="

#As above, plus curses.
sys5r3c:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3 + curses...'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DCK_CURSES $(KFLAGS) -O" \
	"LNKFLAGS=" \
	"LIBS = -ltermlib"

iclsys5r3:
	make sys5r3 KFLAGS=-DICLSVR3

#AT&T UNIX System V R3.  As above, but no ANSI prototyping.
sys5r3na:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3...'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DNOANSI $(KFLAGS) -O" \
	"LNKFLAGS="

#AT&T UNIX System V R3, for 3B computers with Wollongong TCP/IP.
sys5r3net3b:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX SVR3/3B/Wollongong...'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DWOLLONGONG $(KFLAGS) \
	-O" "LIBS= -lnet -lnsl_s" "LNKFLAGS ="

#AT&T UNIX System V R3, signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp, has <termiox.h>.
#Has <termiox.h> for RTS/CTS flow control.
sys5r3tx:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3...'
	$(MAKE) wermit "CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DTERMIOX $(KFLAGS) -i -O" "LNKFLAGS ="

#AT&T UNIX System V R3, signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp, has <termiox.h>.
#Has <sys/termiox.h> for RTS/CTS flow control.
sys5r3sx:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R3...'
	$(MAKE) wermit "CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DSTERMIOX $(KFLAGS) -i -O" "LNKFLAGS ="

#AT&T UNIX System V R4.
#Has <termiox.h>.
sys5r4:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC -DTERMIOX $(KFLAGS)" \
	"LNKFLAGS = -s"

#AT&T UNIX System V R4 with Wollongong TCP/IP.
#Has <termiox.h>.
sys5r4net:
	@echo 'Making C-Kermit $(CKVER) for System V R4 + Wollongong TCP/IP...'
	@echo ' If sockets-library routines are missing at link time, then'
	@echo ' try the sys5r4net2 entry.'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DTERMIOX -DWOLLONGONG $(KFLAGS)" "LNKFLAGS = -s"

#As above, but needs libs included.
sys5r4net2:
	@echo ' PLEASE READ ckuins.doc IF YOU GET MISSING HEADER FILES.'
	@echo ' (Search for WOLLONGONG...)'
	$(MAKE) sys5r4net "LIBS= -lsocket -lnsl"

#As above plus curses.
sys5r4net2c:
	echo 'Making C-Kermit $(CKVER) for System V R4 + Wollongong TCP/IP...'
	@echo ' PLEASE READ ckuins.doc IF YOU GET MISSING HEADER FILES.'
	@echo ' (Search for WOLLONGONG...)'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
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
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDELL_SVR4 -DDIRENT -DHDBUUCP \
	-DDYNAMIC -DTCPSOCKET -DSTERMIOX -DCK_POLL -DKANJI $(KFLAGS)" \
	"LIBS= -lsocket -lnsl" "LNKFLAGS = -s"

#As above, curses support added...
dellsys5r4c:
	@echo 'Making C-Kermit $(CKVER) for DELL UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDELL_SVR4 -DDIRENT -DHDBUUCP \
	-DDYNAMIC -DTCPSOCKET -DSTERMIOX -DKANJI -DCK_CURSES -DCK_POLL \
	$(KFLAGS)" "LIBS= -lsocket -lnsl -lcurses -ltermcap" "LNKFLAGS = -s"

#Mininum interactive: As above, but with every conceivable option removed.
dellsys5r4mi:
	@echo 'Making C-Kermit $(CKVER) for DELL UNIX System V R4...'
	@echo 'Minimum-size interactive'
	$(MAKE) wermit "CFLAGS = -O -DSVR4 -DDELL_SVR4 -DDIRENT \
	-DDYNAMIC -UTCPSOCKET -DNOCMDL -DNOSPL -DNOXMIT -DCK_POLL \
	-DNOMSEND -DNOFRILLS -DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOSHOW -DNOSETKEY -DNOSERVER -DNOUUCP \
	-DNOSETBUF -DNOPUSH -DNOMDMHUP -DNOJC -DNOFDZERO -DNOESCSEQ  \
	$(KFLAGS)" "LNKFLAGS = -s"

#Command-line only version.
dellsys5r4m:
	@echo 'Making C-Kermit $(CKVER) for DELL UNIX System V R4...'
	@echo 'Command-line only'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDELL_SVR4 -DDIRENT -DDYNAMIC \
	-UTCPSOCKET -DNOICP -DNOFRILLS -DNODIAL -DNODEBUG -DNOTLOG -DNOCSETS \
	-DNOSETKEY -DNOESCSEQ -DNOSETBUF -DNOJC -DNOFDZERO -DCK_POLL \
	$(KFLAGS)" "LNKFLAGS = -s"

#AT&T UNIX System V R4.
#Has <sys/termiox.h>.
sys5r4sx:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DSTERMIOX $(KFLAGS)" "LNKFLAGS = -s"

#AT&T UNIX System V R4.
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C constructs, <sys/termiox.h>, etc etc.
sys5r4sxtcp:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DSTERMIOX -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl" "LNKFLAGS = -s"

#AT&T UNIX System V R4.
#As above + curses.
sys5r4sxtcpc:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DSTERMIOX  -DCK_CURSES -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap" "LNKFLAGS = -s"

#AT&T UNIX System V R4.  CONSENSYS SVR4.2-1.
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C constructs, <sys/termiox.h>, etc. 
# Fullscreen -DCK_CURSES added (with curses & termcap libs)
# Submission by Robert Weiner/Programming Plus, rweiner@watsun.cc.columbia.edu
sys5r4sxtcpf:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DSTERMIOX -DTCPSOCKET -DCK_CURSES $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -L/usr/ccs/lib -lcurses -ltermcap" \
	"LNKFLAGS = -s"

#Smallest possible version for System V R4
s5r4m:
	@echo Minimum size
	$(MAKE) "MAKE=$(MAKE)" sys5r4sx \
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
	$(MAKE) "MAKE=$(MAKE)" sys5r4sx "KFLAGS=$(KFLAGS) -DNOANSI"

#SINIX V5.42 - includes curses, tcp/ip, everything
sinix542:
	@echo 'Making C-Kermit $(CKVER) for Siemens/Nixdorf SINIX V5.42...'
	$(MAKE) wermit "CFLAGS = -DSINIX -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DSTERMIOX -DCK_CURSES -DTCPSOCKET -DSELECT -DKANJI CK_ANSIC \
	-kansi -W0 -O -KOlimit,1600 $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses -ltermcap" "LNKFLAGS = -s"

#Commodore Amiga with AT&T UNIX System V R4 and TCP/IP support.
#Has <sys/termiox.h>.
svr4amiganet:
	@echo 'Making C-Kermit $(CKVER) for Amiga SVR4 + TCP/IP...'
	$(MAKE) wermit "CC=gcc" "CC2=gcc" \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC -DSTERMIOX \
	-DTCPSOCKET -DCK_CURSES $(KFLAGS)" "LNKFLAGS = -s" \
	"LIBS = -lsocket -lnsl -ltermlib"

#SCO (Novell (Univel)) UnixWare 1.x or 2.x, no TCP/IP.
#This assumes the Novell SDK 1.0, which has <sys/termiox.h>.
#UnixWare users with the "Prime Time Freeware" CD-ROM SDK will probably have
#to use the sys5r4 entry (no termiox.h file, so no hardware flow control).
#Change -DSELECT to -DCK_POLL if -DSELECT causes problems.
unixware:
	$(MAKE) "MAKE=$(MAKE)" sys5r4sx \
	"KFLAGS=-DUNIXWARE -DSELECT $(KFLAGS)"

#UnixWare 1.x or 2.x with TCP/IP and curses.
unixwarenetc:
	$(MAKE) "MAKE=$(MAKE)" sys5r4sxtcpc \
	"KFLAGS=-DUNIXWARE -DSELECT $(KFLAGS)"

#No difference between 1.x and 2.x as far as Kermit is concerned.
unixware2:
	$(MAKE) "MAKE=$(MAKE)" sys5r4sxtcpc \
	"KFLAGS=-DUNIXWARE -DSELECT $(KFLAGS)"

#ESIX SVR4.0.3 or 4.04 with TCP/IP support.
#Has <sys/termiox.h>, ANSI C function prototyping disabled.
#Add -m486 to CFLAGS if desired.
esixr4:
	@echo 'Making C-Kermit $(CKVER) for ESIX SVR4 + TCP/IP...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC -DNOANSI \
	-DSTERMIOX -DTCPSOCKET $(KFLAGS)" "LNKFLAGS = -s" \
	"LIBS = -lsocket -lnsl"

#AT&T UNIX System V R4.
#Has <sys/termiox.h>, Wollongong WIN/TCP TCP/IP.
sys5r4sxnet:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DSTERMIOX -DWOLLONGONG $(KFLAGS)" "LNKFLAGS = -s"

#AT&T UNIX System V R4, no <termio.x> or <sys/termio.x>.
sys5r4nx:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC $(KFLAGS)" \
	"LNKFLAGS = -s"

#AT&T UNIX System V R4, no <termio.x> or <sys/termio.x>, curses, TCP/IP.
sys5r4nxnetc:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DCK_CURSES -DTCPSOCKET $(KFLAGS)" \
	"LIBS = -lcurses -lsocket -lnsl -ltcpip" \
	"LNKFLAGS = -s"

#AT&T UNIX System V R4, no <termio.x> or <sys/termio.x>, Wollongong TCP/IP.
sys5r4nxtwg:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System V R4...'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DWOLLONGONG $(KFLAGS)" "LNKFLAGS = -s"

#ICL UNIX System V R4.(DRS N/X) version :-
#UNIX System V Release 4.0 ICL DRS 6000 (SPARC)
#DRS/NX 6000 SVR4 Version 5  Level 1  Increment 4
#Has <sys/termiox.h>, regular Berkeley sockets library, i.e. in.h and inet.h
#are not misplaced in sys (rather than netinet and arpa, respectively).
#Uses ANSI C constructs, advisory file locking on devices, etc.
iclsys5r4:
	@echo 'Making C-Kermit $(CKVER) for ICL UNIX System V R4 (DRS N/X)'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DICL_SVR4 -DDIRENT -DHDBUUCP \
	-DDYNAMIC -DSTERMIOX -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket " "LNKFLAGS = -s"

#ICL UNIX System V R4.(DRS N/X) version :-
#UNIX System V Release 4.0 ICL DRS 3000 (486)
#DRS/NX 6000 SVR4 Version 5  Level 1  Increment 4
#As above, but also needs -lnsl.
iclsys5r4_486:
	@echo 'Making C-Kermit $(CKVER) for ICL UNIX System V R4 (DRS N/X)'
	$(MAKE) wermit \
	"CFLAGS = -O -DSVR4 -DICL_SVR4 -DDIRENT -DHDBUUCP \
	-DDYNAMIC -DSTERMIOX -DTCPSOCKET $(KFLAGS)" \
	"LIBS= -lsocket -lnsl " "LNKFLAGS = -s"

#Data General DG/UX 4.30 (System V R3) for DG AViiON, with TCP/IP support.
dgux430:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 4.30...'
	$(MAKE) wermit \
	"CFLAGS = -O -DDGUX430 -DSVR3 -DDIRENT -DDYNAMIC \
	-DTCPSOCKET $(KFLAGS)"

#Data General DG/UX 4.30 for DG AViiON, with TCP/IP support with BSDisms.
dgux430bsd:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 4.30...'
	$(MAKE) wermit "CFLAGS = -O -DDGUX430 -D_BSD_SOURCE -DBSD4 \
	-DDYNAMIC -DTCPSOCKET $(KFLAGS)"

#Data General DG/UX 5.4 (System V R4) for DG AViiON, with TCP/IP support.
dgux540:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.40...'
	$(MAKE) wermit "CFLAGS = -O -DDGUX540 -DDIRENT -DHDBUUCP \
	-DKANJI -DDYNAMIC -DSTERMIOX -DTCPSOCKET -DCK_POLL $(KFLAGS)"

dgux54:
	make dgux540

#DG/UX 5.4R3.00
dgux543c:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.4R3...'
	$(MAKE) wermit "CFLAGS = -O -DDGUX540 -DDGUX543 -DDIRENT -DHDBUUCP \
	-DDYNAMIC -DSTERMIOX -DTCPSOCKET -DCK_CURSES -DCK_POLL $(KFLAGS)" \
	"LIBS= -lcurses8 -ltermcap" "LNKFLAGS = -s"

#Data General DG/UX 5.4 (= System V R4) for DG AViiON, with TCP/IP support.
# And curses.
dgux540c:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.4...'
	$(MAKE) wermit "CFLAGS = -O -DDGUX540 -DDIRENT -DHDBUUCP \
	-DDYNAMIC -DSTERMIOX -DTCPSOCKET -DCK_CURSES -DCK_POLL $(KFLAGS)" \
	"LIBS= -lcurses8 -ltermcap" "LNKFLAGS = -s"

dgux54c:
	make dgux540c

#Data General DG/UX 5.40 (System V R4) for Intel AViiON, with TCP/IP support.
dgux540i:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.40...'
	$(MAKE) wermit "CFLAGS = -O -DDGUX540 -DDIRENT -DHDBUUCP -DNOINADDRX \
	-DKANJI -DDYNAMIC -DSTERMIOX -DTCPSOCKET -DCK_POLL $(KFLAGS)" \
	"LIBS = -lsocket -lnsl"

#As above + curses.
dgux540ic:
	@echo 'Making C-Kermit $(CKVER) for DG AViiON DG/UX 5.40...'
	$(MAKE) wermit "CFLAGS = -O -DDGUX540 -DDIRENT -DHDBUUCP -DNOINADDRX \
	-DKANJI -DDYNAMIC -DSTERMIOX -DTCPSOCKET -DCK_POLL -DCK_CURSES \
	$(KFLAGS)" "LIBS = -lsocket -lnsl -lcurses -ltermcap"

#Silicon Graphics System V R3 with BSD file system (IRIS)
iris:
	@echo Making C-Kermit $(CKVER) for Silicon Graphics IRIX pre-3.3...
	$(MAKE) wermit "CFLAGS = -O -DSVR3 -DLONGFN  $(KFLAGS) \
	-I/usr/include/bsd" "LIBS = -lbsd"

#Silicon Graphics IRIS System V R3
irix33:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 3.3...'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC $(KFLAGS) -O" \
	"LNKFLAGS = -s"

#Silicon Graphics Iris Indigo with IRIX 4.0.0 or 5.0...
#Strict ANSI C compilation, TCP/IP support included
irix40:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 4.0...'
	$(MAKE) wermit \
	"CFLAGS = -DIRIX40 -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DPWID_T=uid_t \
	-DCK_ANSIC -DTCPSOCKET $(KFLAGS) -O -Olimit 1500 -I/usr/include/bsd" \
	"LNKFLAGS = -s"

#As above, but with fullscreen display (curses) and Sun Yellow Pages support.
#NOTE: IRIX versions prior to 5 run COFF binaries.
irix40ypc:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 4.0.'
	@echo 'Includes fullscreen file display and Sun Yellow Pages...'
	$(MAKE) wermit \
	"CFLAGS = -DIRIX40 -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DCK_CURSES \
	-DPWID_T=uid_t -DCK_ANSIC -DTCPSOCKET $(KFLAGS) \
	-O -Olimit 1500 -I/usr/include/bsd" \
	"LIBS = -lcurses -lsun" "LNKFLAGS = -s"

# Silicon Graphics Iris Series 4D/*, IRIX 4.0.x, -O4 ucode optimized.
# Huge temporary file space needed for ucode optimizer.  If you get an error
# like "ugen: internal error writing to /tmp/ctmca08777: Error 0", define the
# the TMPDIR environment variable to point to a file system that has more 
# space available, e.g. "setenv TMPDIR /usr/tmp".
irix40u:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 4.0...'
	$(MAKE) wermit \
	"CFLAGS = -DIRIX40 -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DPWID_T=uid_t \
	-DCK_ANSIC -DTCPSOCKET $(KFLAGS) -O4 -Olimit 1500" \
	"LNKFLAGS=-O4 -Olimit 1500 -s" "EXT=u"

# As above, with Curses Support added
irix40uc:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 4.0...'
	$(MAKE) wermit \
	"CFLAGS = -DIRIX40 -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DPWID_T=uid_t \
	-DCK_ANSIC -DCK_CURSES -DTCPSOCKET $(KFLAGS) -O4 -Olimit 1500" \
	"LNKFLAGS=-O4 -Olimit 1500 -s" "EXT=u" "LIBS= -lcurses -ltermcap"

#Silicon Graphics IRIX 5.x.
#Yellow Pages and Curses support included.
#IRIX version 5.x can run COFF or ELF binaries.
irix51:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 5.x'
	@echo 'Includes fullscreen file display and Yellow Pages...'
	@echo 'Add -mips<n> to CFLAGS specify a particular hardware target.'
	$(MAKE) wermit \
	"CFLAGS = -DIRIX51 -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC -DCK_CURSES \
	-DPWID_T=uid_t -DCK_ANSIC -DTCPSOCKET -DSELECT -DKANJI \
	$(KFLAGS) -O -Olimit 1500" "LIBS = -lcurses" "LNKFLAGS = -s"

irix51ypc:
	$(MAKE) "MAKE=$(MAKE)" irix51

#Silicon Graphics IRIX 6.0 or later.
#Yellow Pages and Curses support included.
#IRIX version 6.0 and later runs only ELF binaries.
irix60:
	@echo 'Making C-Kermit $(CKVER) for Silicon Graphics IRIX 6.0'
	@echo 'Includes fullscreen file display and Yellow Pages...'
	@echo 'Add -mips<n> to specify a particular hardware target.'
	$(MAKE) wermit \
	"CFLAGS = -DIRIX51 -DIRIX60 -DSVR4 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DCK_CURSES -DCK_NEWTERM -DPWID_T=uid_t -DCK_ANSIC -DTCPSOCKET \
	-DSELECT -DKANJI -O $(KFLAGS)" "LIBS = -lcurses" "LNKFLAGS = -s"

#In case they type "make sys5"...
sys5:
	$(MAKE) "MAKE=$(MAKE)" sys3

#Generic ATT System III or System V (with I&D space)
sys3:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System III'
	@echo 'or System V R2 or earlier...'
	@echo 'add -DNOMKDIR if mkdir is an undefined symbol.'
	$(MAKE) wermit "CFLAGS = -DATTSV $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#Generic ATT System III or System V R2 or earlier, "no void":
#special entry to remove "Illegal pointer combination" warnings.
sys3nv:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System III'
	@echo 'or System V R2 or earlier...'
	$(MAKE) wermit "CFLAGS = -DATTSV -Dvoid=int  $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#Generic ATT System III or System V (no I&D space)
sys3nid:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System III'
	@echo 'or System V R2 or earlier, no I&D space...'
	$(MAKE) wermit "CFLAGS = -DATTSV $(KFLAGS) -O" "LNKFLAGS ="

#AT&T 7300/UNIX PC (3B1) systems, sys3 but special handling for internal modem.
#Link with the shared library -- the conflict with openi in shared library
#is solved with -Dopeni=xopeni
sys3upc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, shared lib...'
	@echo 'If shared lib causes trouble, use make sys3upcold.'
	$(MAKE) wermit \
	"CFLAGS = -O -DATT7300 -DDYNAMIC -DNOSETBUF -DNOMKDIR $(KFLAGS) \
	-Dopeni=xopeni" \
	"CC2 = ld /lib/crt0s.o /lib/shlib.ifile" "LNKFLAGS = -s"

#AT&T 7300/UNIX PC (3B1) systems, with curses support.
#Curses and the shared library don't get along, so we don't use the
#shared library.  We need to include CK_NEWTERM to avoid a conflict
#with curses and buffering on stdout.  Merged with submission by
#Robert Weiner/Programming Plus, rweiner@watsun.cc.columbia.edu.
#We don't need -Dopeni=xopeni since we're not using the shared library,
#but we keep it to be consistent with the other entries.
sys3upcc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, curses...'
	$(MAKE) wermit "CFLAGS = -O -DATT7300 -DDYNAMIC -DNOSETBUF \
	-DCK_CURSES -DCK_NEWTERM -DNOMKDIR $(KFLAGS) -Dopeni=xopeni" \
	"LIBS = -lcurses" "LNKFLAGS = -s"

#AT&T 7300 UNIX PC (3B1), as above, but no newterm().
sys3upcx:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, curses...'
	$(MAKE) wermit "CFLAGS = -O -DATT7300 -DDYNAMIC -DNOSETBUF \
	-DCK_CURSES -DNOMKDIR $(KFLAGS) -Dopeni=xopeni" \
	"LIBS = -lcurses -ltermcap" "LNKFLAGS = -s"

#AT&T 7300/UNIX PC (3B1) systems, with curses and shared library support.
sys3upcshcc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, shared lib...'
	@echo 'With curses.  Requires shcc.'
	$(MAKE) wermit "CFLAGS = -O -DATT7300 -DDYNAMIC -DNOMKDIR \
	-DNOSETBUF -DCK_NEWTERM -DCK_CURSES $(KFLAGS) -Dopeni=xopeni" \
	"LNKFLAGS = -i -s" "CC = shcc" "CC2 = shcc" "LIBS = -lcurses"

#AT&T 7300/UNIX PC (3B1) systems, as above, no curses, but use gcc.
sys3upcg:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC...'
	$(MAKE) wermit \
	"CFLAGS = -O -DATT7300 -DDYNAMIC -DNOSETBUF -DNOMKDIR $(KFLAGS) \
	-Dopeni=xopeni" \
	"CC = gcc" "CC2 = gcc" "LNKFLAGS = -s -shlib"

#AT&T 7300/UNIX PC (3B1) systems, curses and gcc.
sys3upcgc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC, curses...'
	$(MAKE) wermit "CFLAGS = -O -DATT7300 -DDYNAMIC -DNOSETBUF \
	-DCK_CURSES -DIFDEBUG -DCK_NEWTERM -DNOMKDIR $(KFLAGS)" \
	"CC = gcc" "CC2 = gcc" "LIBS = -lcurses" "LNKFLAGS = -s"

#AT&T 7300/UNIX PC (3B1) systems, special handling for internal modem.
#No FULLSCREEN file transfer display (curses).
sys3upcold:
	@echo 'Making C-Kermit $(CKVER) for AT&T 7300 UNIX PC...'
	$(MAKE) wermit "CFLAGS = -DATT7300 -DNOMKDIR $(KFLAGS) -O" \
	"LNKFLAGS = -i"

#AT&T 7300/Unix PC systems, minimum kermit for those with smaller amounts
#of memory.
sys3upcm:
	@echo Minimum interactive
	$(MAKE) "MAKE=$(MAKE)" sys3upc \
	"KFLAGS=-DNOSPL -DNOFRILLS -DNOHELP -DNODEBUG -DNOTLOG -DNOCSETS \
	-DNOSETKEY -DNOSETBUF"

#As above, but with gcc...
sys3upcgm:
	@echo Minimum interactive
	$(MAKE) "MAKE=$(MAKE)" sys3upcg \
	"KFLAGS=-DNOSPL -DNOFRILLS -DNOHELP -DNODEBUG -DNOTLOG -DNOCSETS \
	-DNOSETKEY -DNOSETBUF"

#AT&T 6300 PLUS (warning, -O might make it run out of space).
#NOTE: Remove -DHDBUUCP if not using Honey DanBer UUCP.
att6300:
	@echo 'Making C-Kermit $(CKVER) for AT&T 6300 PLUS...'
	$(MAKE) wermit "CFLAGS = -DATT6300 -DHDBUUCP -DNOFILEH \
	$(KFLAGS) -O -Ml -i" "LNKFLAGS = -i -Ml"

#As above, but with curses support.  Debugging disabled to prevent thrashing.
att6300c:
	@echo 'Making C-Kermit $(CKVER) for AT&T 6300 PLUS...'
	$(MAKE) wermit \
	"CFLAGS = -DATT6300 -DHDBUUCP -DNOFILEH -DNOCSETS \
	-DCK_CURSES -DNODEBUG $(KFLAGS) -O -Ml -i" "LNKFLAGS = -i -Ml" \
	"LIBS = -lcurses"

#AT&T 6300 PLUS with no curses, no debugging (about 34K smaller)
# -Optimization saves about 20K too.
att6300nd:
	@echo 'Making C-Kermit $(CKVER) for AT&T 6300 PLUS, no debugging...'
	$(MAKE) wermit \
	"CFLAGS = -DATT6300 -DHDBUUCP -DNODEBUG -DNOFILEH $(KFLAGS) \
	-O -i -Ml" "LNKFLAGS = -i -Ml"

#AT&T 3B2, 3B20-series computers running AT&T UNIX System V.
#This is just generic System V with Honey DanBer UUCP, so refer to sys3hdb.
#Might need addition of -DNONAWS if ckutio.c compilation fails in ttgwsiz().
att3bx:
	$(MAKE) "MAKE=$(MAKE)" sys3hdb

#AT&T 3B2, 3B20-series computers running AT&T UNIX System V,
#with fullscreen file transfer display
att3bxc:
	@echo 'Making C-Kermit $(CKVER) for AT&T 3B2 or 3B20'
	@echo 'with Honey DanBer UUCP and curses...'
	$(MAKE) wermit "CFLAGS = -DATTSV -DHDBUUCP $(KFLAGS) \
		-DCK_CURSES -DCK_NEWTERM -i -O" \
	"LNKFLAGS = -i" "LIBS=-lcurses"

#Any System V R2 or earlier with Honey DanBer UUCP (same as above)
sys3hdb:
	@echo 'Making C-Kermit $(CKVER) for AT&T UNIX System III'
	@echo 'or System V R2 or earlier with Honey DanBer UUCP...'
	$(MAKE) wermit "CFLAGS = -DATTSV -DHDBUUCP $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#Sperry/UNISYS 5000 UTS V 5.2 (System V R2), Honey DanBer UUCP
unisys5r2:
	@echo 'Making C-Kermit $(CKVER) for Sperry/UNISYS 5000 UTS V 5.2...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DUNISYS52 -DHDBUUCP $(KFLAGS) -i -O" \
	"LNKFLAGS = -i"

#In case they say "make sys5hdb" instead of "make sys3hdb"...
sys5hdb:
	$(MAKE) "MAKE=$(MAKE)" sys3hdb

#IBM PS/2 with AIX 1.0.
#  Reports indicate that -O switch must be omitted.
#  It is also possible that "make bsd" will work (reports welcome).
#  One report said "make LIBS=-lbsd bsd" did the trick.
ps2aix:
	@echo 'Making C-Kermit $(CKVER) for IBM AIX 1.0 PS/2...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DPS2AIX10 $(KFLAGS) -i" "LNKFLAGS = -i"

#IBM RT PC with AIX 2.2.1
#This one has unique and strange lockfiles.
rtaix:
	@echo 'Making C-Kermit $(CKVER) for IBM RT PC, AIX 2.2.1...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DRTAIX -DHDBUUCP -DTCPSOCKET -Wq,-SJ2 \
	-DCLSOPN $(KFLAGS) -O -w" "LNKFLAGS = -s"

#IBM RT PC with AIX 2.2.1
#This one has unique and strange lockfiles.
rtaixc:
	@echo 'Making C-Kermit $(CKVER) for IBM RT PC, AIX 2.2.1, curses...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DRTAIX -DHDBUUCP -DTCPSOCKET -DCK_CURSES \
	-DCLSOPN -Wq,-SJ2 $(KFLAGS) -O -w" "LIBS=-lcurses" "LNKFLAGS = -s"

#IBM RT PC with AIX (ACIS) 2.2.1 (BSD 4.3)
# Add -O, -DDYNAMIC, -s, etc, if they work.
rtacis:
	@echo Making C-Kermit $(CKVER) for RT PC with ACIS 2.2.1 = BSD 4.3...
	$(MAKE) wermit "CFLAGS= -DBSD4 -DTCPSOCKET $(KFLAGS) -U__STDC__" \
	"LNKFLAGS = -s"

hpux:
	@echo 'Please pick a more specific HP-UX entry.'

#HP 9000 series 300, 500, 800, no long filenames and no job control.
#This works for HP-UX versions earlier than 6.5.
#It definitely works with HP-UX 5.21.
hpuxpre65:
	@echo 'Making C-Kermit $(CKVER) for HP-9000 HP-UX, no long filenames.'
	$(MAKE) wermit \
	"CFLAGS = -DHPUX -DHPUXPRE65 $(KFLAGS) -O" "LNKFLAGS ="

#HP-9000 500 HP-UX 5.21 with Wollongong WIN/TCP 1.2 TCP/IP
#Requires /usr/wins/usr/include and /usr/lib/libnet.a from Wollongong
hpux500wintcp:
	@echo 'Making C-Kermit $(CKVER) for HP-9000 500 HP-UX 5.21 WIN/TCP'
	$(MAKE) wermit \
	"CFLAGS = -DHPUX -DHPUXPRE65 -DTCPSOCKET -DDYNAMIC \
	-I/usr/wins/usr/include $(KFLAGS) -O" \
	"LIBS = /usr/lib/libnet.a" "LNKFLAGS = "

#HP 9000 series 300, 800, long filenames (using BSD file system)
# (This one is probably necessary for the Series 300)
hpuxlf:
	@echo 'Making C-Kermit $(CKVER) for HP-9000 HP-UX, long filenames...'
	$(MAKE) wermit \
	"CFLAGS = -DHPUX -DNDIR -DLONGFN -DDYNAMIC $(KFLAGS) -O" \
	"LNKFLAGS ="

#HP 9000 series 300, 800, long filenames (using <dirent.h>)
hpuxde:
	@echo 'Making C-Kermit $(CKVER) for HP-9000 HP-UX, long filenames...'
	$(MAKE) wermit \
	"CFLAGS = -DHPUX -DDIRENT -DDYNAMIC $(KFLAGS) -O" \
	"LNKFLAGS ="

#HP 9000 series 300, 800, long filenames (using <dirent.h>) and TCP/IP
hpuxdetcp:
	@echo 'Making C-Kermit $(CKVER) for HP-9000 HP-UX, long filenames...'
	$(MAKE) wermit \
	"CFLAGS = -DHPUX -DDIRENT -DDYNAMIC -DTCPSOCKET $(KFLAGS) -O" \
	"LNKFLAGS ="

#HP 9000 series 300, 800, long filenames, System V R3 or later
# (Does anybody know what is the earliest release of HP-UX based on SVR3?)
hpuxs5r3:
	@echo 'Making C-Kermit $(CKVER) for HP-9000 HP-UX, long filenames...'
	$(MAKE) wermit \
	"CFLAGS = -DHPUX -DSVR3 -DDIRENT -DTCPSOCKET -DDYNAMIC \
	-DNOSETBUF $(KFLAGS) -O" "LNKFLAGS =" "LIBS=-lBSD"

#HP-UX 6.5, short filenames, no network support.
#ckcpro and ckuusr are broken out because they make the optimizer run away.
hpux65:
	@echo 'Making C-Kermit $(CKVER) for HP-UX 6.5, no long filenames.'
	$(MAKE) ckcpro.$(EXT) \
	"CFLAGS = -DHPUX -DDYNAMIC -DNOSETBUF -DSIG_V $(KFLAGS)"
	$(MAKE) ckuusr.$(EXT) \
	"CFLAGS = -DHPUX -DDYNAMIC -DNOSETBUF -DSIG_V $(KFLAGS)"
	$(MAKE) wermit \
	"CFLAGS = -DHPUX -DDYNAMIC -DNOSETBUF -DSIG_V $(KFLAGS) -O"

#HP-UX 6.5, as above, plus curses:
hpux65c:
	@echo 'Making C-Kermit $(CKVER) for HP-UX 6.5 + curses.'
	$(MAKE) ckcpro.$(EXT) \
	"CFLAGS = -DHPUX -DDYNAMIC -DNOSETBUF -DCK_CURSES -DSIG_V $(KFLAGS)"
	$(MAKE) ckuusr.$(EXT) \
	"CFLAGS = -DHPUX -DDYNAMIC -DNOSETBUF -DCK_CURSES -DSIG_V $(KFLAGS)"
	$(MAKE) wermit \
	"CFLAGS = -DHPUX -DDYNAMIC -DNOSETBUF -DCK_CURSES -DSIG_V $(KFLAGS) \
	-O" "LIBS= -lcurses -ltermcap" 

#HP-UX 7.0, no long filenames, no network support.
hpux7sf:
	@echo 'Making C-Kermit $(CKVER) for HP-9000 HP-UX, no long filenames.'
	$(MAKE) ckcpro.$(EXT) \
	"CFLAGS = -DHPUX $(KFLAGS) -DSIG_V"
	$(MAKE) wermit \
	"CFLAGS = -DHPUX $(KFLAGS) -DSIG_V -O" "LNKFLAGS ="

#HP-UX 7.0, short filenames, but with tcp/ip and curses.
#There must be <arpa/telnet.h> & <arpa/inet.h> present to support this
#configuration.  To use this, you must have bought the ARPA Services
#Product from HP, and you must get the files "telnet.h" and "inet.h"
#from the Berkeley Standard Distribution because (reportedly) they are not
#part of the HP product.  And you need /usr/lib/libBSD.a.
hpux70sftcpc:
	@echo 'Making C-Kermit $(CKVER) for HP9000/8xx HP-UX V. 7.0'
	@echo 'supporting: NO long filenames, networking, curses, HDB uucp...'
	$(MAKE) ckcpro.$(EXT) \
	"CFLAGS = -DHPUXDEBUG -DHPUX -DSVR3 -DCK_REDIR -DDYNAMIC -DNOSETBUF \
	-DTCPSOCKET -DCK_CURSES $(KFLAGS)"
	$(MAKE) wermit \
	"CFLAGS = -DHPUXDEBUG -DHPUX -DSVR3 -DCK_REDIR -DDYNAMIC -DNOSETBUF \
	-DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DCK_RTSCTS \
	-DTCPSOCKET -DCK_CURSES $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS= -lcurses -ltermcap -lBSD"

#HP 9000 series 800 HP-UX 7.0, long filenames, network support, HDB uucp.
#See comments in hpux70sftcpc about TCP/IP support.
hpux70lfn:
	@echo 'Making C-Kermit $(CKVER) for HP9000/8xx HP-UX V. 7.0'
	@echo 'supporting: long filenames, networking, HDB uucp...'
	$(MAKE) ckcpro.$(EXT) \
	"CFLAGS = -DHPUXDEBUG -DHPUX -DSVR3 -DDYNAMIC -DNOSETBUF \
	-DCK_REDIR -DCK_RTSCTS -DTCPSOCKET $(KFLAGS)"
	$(MAKE) wermit \
	"CFLAGS = -DHPUXDEBUG -DHPUX -DSVR3 -DDYNAMIC -DNOSETBUF \
	-DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DDIRENT -DLONGFN \
	-DCK_REDIR -DCK_RTSCTS -DTCPSOCKET  $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lBSD"

hpux70lfnc:
	@echo 'Making C-Kermit $(CKVER) for HP9000/8xx HP-UX V. 7.0'
	@echo 'supporting: curses, long filenames, networking, HDB UUCP...'
	$(MAKE) ckcpro.$(EXT) \
	"CFLAGS = -DHPUXDEBUG -DHPUX -DSVR3 -DDIRENT -DLONGFN \
	-DDYNAMIC -DNOSETBUF -DCK_REDIR -DTCPSOCKET -DCK_CURSES $(KFLAGS)"
	$(MAKE) wermit \
	"CFLAGS = -DHPUXDEBUG -DHPUX -DSVR3 -DDIRENT -DLONGFN \
	-DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DCK_RTSCTS \
	-DDYNAMIC -DNOSETBUF -DCK_REDIR -DTCPSOCKET -DCK_CURSES $(KFLAGS) -O" \
	"LIBS= -lcurses -ltermcap -lBSD" "LNKFLAGS = -s"

#HP 9000 Series 300 or 400, HP-UX 8.0, long filenames and TCP/IP support.
#This one should also work on 700/800, but without PA-specific optimization.
#In case -DCK_RTSCTS and -DCK_REDIR make trouble, remove them.
#NOTE: ckcpro.c blows up the optimizer, so don't optimize it. For HP-UX 8.0
#on Motorola CPUs, you might have to reinstall your kernel with maxdsiz >=
#0x02000000.  But if physical memory is small, that still will not help much.
hpux80:
	@echo 'Making C-Kermit $(CKVER) for HP9000 HP-UX V. 8.0'
	@echo 'supporting: long filenames, TCP/IP, HDB UUCP...'
	$(MAKE) -B "CC=$(CC)" "CC2=$(CC2)" ckcpro.$(EXT) \
	"CFLAGS = -DCK_REDIR -DHPUXDEBUG -DHPUX -DHPUX8 -DSVR3 -DRENAME \
	-DDYNAMIC -DTCPSOCKET $(KFLAGS)"
	$(MAKE) -B "CC=$(CC)" "CC2=$(CC2)" wermit \
	"CFLAGS = -DCK_REDIR -DHPUXDEBUG -DHPUX -DHPUX8 -DSVR3 -DRENAME \
	-DDIRENT -DLONGFN -DCK_RTSCTS \
	-DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DNOSETBUF \
	-DDYNAMIC -DTCPSOCKET $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lBSD"

#Exactly as above + curses.
hpux80c:
	@echo 'Making C-Kermit $(CKVER) for HP9000 HP-UX V. 8.0'
	@echo 'supporting: long filenames, TCP/IP, HDB UUCP, curses...'
	$(MAKE) -B "CC=$(CC)" "CC2=$(CC2)" ckcpro.$(EXT) \
	"CFLAGS = -DCK_REDIR -DHPUXDEBUG -DHPUX -DHPUX8 -DSVR3 -DRENAME \
	-DDYNAMIC -DTCPSOCKET $(KFLAGS)"
	$(MAKE) -B "CC=$(CC)" "CC2=$(CC2)" wermit \
	"CFLAGS = -DCK_REDIR -DHPUXDEBUG -DHPUX -DHPUX8 -DSVR3 -DRENAME \
	-DDIRENT -DLONGFN -DCK_RTSCTS -DCK_CURSES \
	-DHDBUUCP -DLOCK_DIR=\\\"/usr/spool/uucp\\\" -DNOSETBUF \
	-DDYNAMIC -DTCPSOCKET $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lBSD -lcurses"

#HP 9000 Series 700 or 800, HP-UX 8.0, long filenames and TCP/IP support.
# Like the previous entries, but with PA-specific optimization.
hpux80pa:
	$(MAKE) hpux80 "KFLAGS = $(KFLAGS) +Obb1100"

#As above, but with curses.
hpux80pac:
	$(MAKE) hpux80c "KFLAGS = $(KFLAGS) +Obb1100"

#As above, but compiled with GCC 2.3.3.
hpux80pagcc:
	@echo 'Making C-Kermit $(CKVER) for HP9000 HP-UX 8.0 with gcc,'
	@echo 'supporting: long filenames, networking, HDB uucp...'
	$(MAKE) "CC=gcc" "CC2=gcc" hpux80

#HP 9000 HP-UX 8.0, no TCP/IP because /usr/lib/libBSD.a can't be found,
#or TCP/IP header files missing.
hpux80notcp:
	$(MAKE) "MAKE=$(MAKE)" hpux80 "KFLAGS = $(KFLAGS) -UTCPSOCKET"

#HP-UX 9.0, 9.01, 9.03, 9.04, 9.05, 9.07, 9.10 ..., + TCP/IP + curses, fully
#configured.  Use this entry with the restricted compiler: no optimization, no
#ANSI support.  If you get unresolved sockets library references at link time,
#then try adding -lBSD to LIBS, or else remove -DTCPSOCKET to build a version
#without TCP/IP support.
hpux90:
	@echo 'Making C-Kermit $(CKVER) for HP9000 HP-UX V. 9.0...'
	@echo 'Read comments in the hpux90 entry if you have trouble.'
	$(MAKE) wermit "CFLAGS = -DHPUX9 -DDIRENT -DHPUXDEBUG -DLONGFN \
	-DTCPSOCKET -DCK_CURSES -DHDBUUCP -DDYNAMIC -DNOSETBUF -DKANJI \
	-DSTERMIOX -DCK_REDIR -DRENAME \
	-DLOCK_DIR=\\\"/usr/spool/uucp\\\" $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lcurses" "CC=$(CC)" "CC2=$(CC2)"

#Like hpux90, but for the "value-added" compiler on all HP 9000 models.
#Adds optimization and ANSI compilation:
# +O2 makes smaller executable (= -O = Level-1 and global optimization)
# +O3 adds interprocedural global optimization, makes bigger executable.
# If optimization fails on some modules, you can add:
#  +Obb<n>, +Olimit <n>, or +Onolimit, depending on your cc version,
# where <n> is a number, e.g. +Obb1200.  In other words, if you get optimizer
# warnings, add (for example) +Obb1200; if you still get optimizer warnings,
# increase the number.  Repeat until warnings go away.  If your compiler
# permits it, use +Onolimit. If optimizer blows up on ckcpro.c, see next entry.
# Reportedly, on some configurations, such as HP9000/425e or /340, perhaps
# depending on the amount of main memory, this entry might fail no matter what
# you do ("Out of Memory", "cc: Fatal error in /lib/c.c1", etc).  In that case
# use "make hpux90" (no "o").
hpux90o:
	$(MAKE) hpux90 \
	"KFLAGS = $(KFLAGS) -Aa -DCK_ANSIC -D_HPUX_SOURCE +O2"

# For HP-UX 9.0 on Motorola CPUs, optimization of ckcpro.c tends to blow up
# the compiler.  You might have to reinstall your kernel with maxdsiz >=
# 0x03000000.  But if physical memory is small, that still will not help much.
# In that case, use this entry to skip optimization of ckcpro.c.
hpux90mot:
	$(MAKE) ckcpro.$(EXT) "CFLAGS = -DHPUX9 -DHPUXDEBUG \
	-DTCPSOCKET -DDYNAMIC -DCK_REDIR -DRENAME $(KFLAGS)"
	$(MAKE) hpux90 \
	"KFLAGS = $(KFLAGS) -Aa -DCK_ANSIC -D_HPUX_SOURCE +O2"

#Like hpux90o but with additional model-700/800-specific optimizations.
# +ESlit = consolidate strings in read-only memory.
# +ESfsc = inline millicode calls when comparing pointers.
hpux90o700:
	@echo 'If you get optimizer warnings'
	@echo 'Try "make hpux90o700 KFLAGS=+Obb1200"'
	$(MAKE) hpux90o "KFLAGS = $(KFLAGS) +ESlit +ESsfc"

#HP-UX 9.0, 9.01, 9.03, 9.04, ..., + TCP/IP + curses, fully configured,
#built with gcc, all models except 800 series.
hpux90gcc:
	$(MAKE) hpux90 CC=gcc CC2=gcc \
	"KFLAGS = $(KFLAGS) -DCK_ANSIC -O2"

#HP-9000 HP-UX 10.0 + TCP/IP + curses, fully configured.
#Use with restricted (bundled) compiler: no optimization, no ANSI support.
#libcurses needed for fullscreen file xfer display in HP-UX 10.00 and 10.01.
#libHcurses (NOT libcurses!) for fullscreen display, to work around fatal bugs 
#in HP-UX 10.10 and 10.20 curses. Maybe we could use lcurses for 10.30, since 
#the 10.10 curses problem is supposedly fixed in 10.30.
hpux100:
	@echo 'Making C-Kermit $(CKVER) for HP9000 HP-UX V.10.xx...'
	@touch ckuker.cpp
	$(MAKE) "SHELL=/usr/bin/sh" "CFLAGS=-DHPUX10" manpage
	@case `uname -r` in \
	   [AB].10.0*)	KFLAGS='-DHPUX1000 $(KFLAGS)' ; \
			LIBS='	-lcurses'  ;; \
	   [AB].10.1*)	KFLAGS='-DHPUX1010 -D__HP_CURSES $(KFLAGS)' ; \
			LIBS='	-lHcurses' ;; \
	   [AB].10.2*)	KFLAGS='-DHPUX1020 -D__HP_CURSES $(KFLAGS)' ; \
			LIBS='	-lHcurses' ;; \
	   [AB].10.3*)	KFLAGS='-DHPUX1030 -D__HP_CURSES $(KFLAGS)' ; \
			LIBS='	-lHcurses' ;; \
	   [AB].10.?*)	KFLAGS='-DHPUX10XX -D__HP_CURSES $(KFLAGS)' ; \
			LIBS='	-lHcurses' ;; \
	esac ; \
	$(MAKE) "SHELL=/usr/bin/sh" wermit \
	"CFLAGS = -DHPUX10 -DDIRENT -DSTERMIOX -DCK_DSYSINI -DHDBUUCP \
	-DCK_CURSES -DCK_WREFRESH -DKANJI -DDYNAMIC -DNOSETBUF \
	-DTCPSOCKET -DCK_REDIR -DRENAME $$KFLAGS" \
	"LNKFLAGS=-s $(LNKFLAGS)" "LIBS = $$LIBS"

#HP-9000 HP-UX 10.00 and higher with ANSI prototyping and optimization.
#PA-RISC only, no Motorola or other hardware is support in HP-UX 10.00++.
#The unbundled optional compiler is required.
#Your path should start with /opt/ansic/bin.
# -Wl,-Fw = Remove stack unwind table (info used by debuggers).
# +O2 makes a smaller executable (= -O = Level-1 and global optimization).
# +O3 adds interprocedural global optimization, makes a bigger executable.
# +Onolimit allows all modules to be optimized, no matter how complex.	But:
#  (a) +Onolimit does not seem to always be there in HP-UX 10.00, and:
#  (b) some modules might take hours on low-memory and/or slow systems.
# The following are PA-RISC-specific optimizations:
# +ESlit = Consolidate strings in read-only memory.
# +ESfsc = Inline millicode calls when comparing pointers.
# +DA1.0 = Generate code that runs on both 700 and 800 models.
# You might need to configure your kernel for a maxdsiz of 0x06000000 (96MB) 
# or greater to prevent the optimizer from running out of space.
hpux100o:
	$(MAKE) "SHELL=/usr/bin/sh" "PATH=/opt/ansic/bin:$$PATH" hpux100 \
	"KFLAGS = $(KFLAGS) \
	-Aa -D_HPUX_SOURCE -DCK_ANSIC -DUTIMEH \
	+O2 -Wl,-Fw +ESlit +ESsfc +DA1.0" 

#The same like hpux100o but with "+Onolimit"
hpux100o+:
	$(MAKE) hpux100o \
	"KFLAGS = $(KFLAGS) +Onolimit" 

#Regulus on CIE Systems 680/20
cie:
	@echo 'Making C-Kermit $(CKVER) for CIE Systems 680/20 Regulus...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNOFILEH -DCIE $(KFLAGS) -O" "LNKFLAGS ="

# Linux 0.99.14 or later with gcc, dynamic libraries, curses, TCP/IP.
# 
# -DLINUXFSSTND (Linux File System Standard) gives UUCP lockfil /var/lock with
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
# uses the newer standard.  To force the second one, add -DFSSTND10.
#
# "The nice thing about the Linux standard is there are so many to choose from"
#
# NOTE: Remove -DBIGBUFOK for small-memory or limited-resource systems.
linux:
	@echo 'Making C-Kermit $(CKVER) for Linux...'
	@echo 'IMPORTANT: Read the comments in the linux section of the'
	@echo 'makefile if you get compilation or link errors.'
	$(MAKE) wermit "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -DPOSIX -DDYNAMIC -DCK_CURSES -DCK_POSIX_SIG \
	-DBIGBUFOK -DTCPSOCKET -DLINUXFSSTND $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" "LIBS = -lcurses -ltermcap"

#This version was used for Linux prior to C-Kermit 6.0.192.
#Now the "Linux File System Standard" is considered standard, ditto TCP/IP.
linuxold:
	@echo 'Making C-Kermit $(CKVER) for Linux...'
	@echo 'For FSSTND-recommended UUCP lockfiles, use:'
	@echo '  make linux "KFLAGS=-DLINUXFSSTND".'
	@echo 'Use "make linuxtcp" to add TCP/IP support.'
	@echo 'Read comments in makefile for additional options.'
	$(MAKE) wermit "CC = gcc" "CC2 = gcc" \
	"CFLAGS = -O -DPOSIX -DDYNAMIC -DCK_CURSES -DCK_POSIX_SIG \
	$(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" "LIBS = -lcurses -ltermcap"

# "make linux", but with static linking to avoid confusion with DLL versions.
# Use this to make a portable binary.  Adds about 100K to the executable.
linuxs:
	$(MAKE) linux "KFLAGS = $(KFLAGS)" "LNKFLAGS = -static"

linuxnotcp:
	$(MAKE) linux "KFLAGS = -DNONET $(KFLAGS)"

# "make linuxnotcp" with lcc (see http://www.cs.princeton.edu/software/lcc)
# lcc does not understand various gcc extensions:
#  "__inline__" -- can be eliminated by adding "-D__inline__="
#  "__asm__ and "long long" -- in header files, should be surrounded by
#                              "#ifndef(__STRICT_ANSI__)"/"#endif"
#  however, TCP requires some __asm__ functions, so cannot be compiled
linuxnotcp-lcc:
	@echo 'Making C-Kermit $(CKVER) for Linux with lcc ...'
	@echo 'Read comments in makefile for additional information.'
	$(MAKE) wermit "CC = lcc" "CC2 = lcc" \
	"CFLAGS = -DPOSIX -DDYNAMIC -DCK_CURSES -DCK_POSIX_SIG \
	-UTCPSOCKET -DLINUXFSSTND $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" "LIBS = -lcurses -ltermcap"

# LynxOS 2.2 with GCC compiler, TCP/IP and fullscreen display.
# Probably also works with Lynx 2.1, and maybe even Lynx 2.0.
# -X means use termios serial drivers rather than BSD4.3-style sgtty drivers.
# If you have trouble with this, try "make bsd KFLAGS=-DNOFDZERO".
lynx:
	@echo 'Making C-Kermit $(CKVER) for LynxOS 2.2 with TCP/IP'
	$(MAKE) wermit "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -O -DPOSIX -DDIRENT -DSETREUID -DCK_CURSES -DTCPSOCKET \
	-DCK_ANSIC -DLYNXOS" "LNKFLAGS = -X" "LIBS = -lcurses -lbsd"

lynx22:
	$(MAKE) lynx "KFLAGS=$(KFLAGS)"

# LynxOS 2.1 with GCC compiler 1.40 and TCP/IP.
lynx21:
	@echo 'Making C-Kermit $(CKVER) for LynxOS 2.1 with TCP/IP'
	$(MAKE) kermit "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -O -DSETREUID -DTCPSOCKET -DCK_ANSIC -DBSD4 -DLYNXOS" \
	"LIBS = -lbsd"

#Microport SV/AT for IBM PC/AT 286 and clones, System V R2.
#The -O flag may fail on some modules (like ckuus2.c), in which case you
#should compile them by hand, omitting the -O.  If you get "hash table
#overflow", try adding -DNODEBUG.
#Also, reportedly this compiles better with gcc than with cc.
mpsysv:
	@echo 'Making C-Kermit $(CKVER) for Microport SV/AT 286...'
	$(MAKE) wermit \
	"CFLAGS= -DATTSV $(KFLAGS) -O -Ml" "LNKFLAGS = -Ml"

#Microsoft "Xenix/286" e.g. for IBM PC/AT
xenix:
	@echo 'Making C-Kermit $(CKVER) for Xenix/286'
	$(MAKE) wermit \
	"CFLAGS= -DXENIX -DNOFILEH $(KFLAGS) -Dunix -F 3000 -i" \
	"LNKFLAGS = -F 3000 -i"

#SCO Xenix 2.2.1 for IBM PC, XT, PS2/30, or other 8088 or 8086 machine
#Should this not work, try some of the tricks from sco286.
#NOTE: -DRENAME is omitted for early SCO Xenix releases because it didn't
#exist, or its semantics were different from the later POSIX-compliant
#version of rename().
sco86:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/86...'
	$(MAKE) wermit \
	"CFLAGS= -DXENIX -DNOFILEH $(KFLAGS) -Dunix -F 3000 -i -M0me" \
	"LNKFLAGS = -F 3000 -i -s -M0me" "LIBS = -lx"

#SCO Xenix/286 2.2.1, e.g. for IBM PC/AT, PS/2 Model 50, etc.
#Reportedly, this "make" can fail simply because of the size of this
#makefile.  If that happens, use "makeL", or edit out some of the
#other entries.  No debugging or character-set translation.
sco286:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/286...'
	@echo 'If make fails, try using makeL.'
	$(MAKE) wermit \
	"CFLAGS= -s -O -LARGE -DXENIX -DNOFILEH -Dunix -DRDCHK -DNAP \
	-DNODEBUG -DNOTLOG -DNOCSETS \
	$(KFLAGS) -F 3000 -i -M2let32" \
	"LIBS = -lx" "LNKFLAGS = -s -O -LARGE -F 3000 -i -M2let32"

#SCO Xenix/286 2.2.1, e.g. for IBM PC/AT, PS/2 Model 50, etc.
#As above, but with HDBUUCP
sco286hdb:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/286 with HDB UUCP...'
	@echo 'If make fails, try using makeL.'
	$(MAKE) wermit \
	"CFLAGS= -s -O -LARGE -DXENIX -DNOFILEH -Dunix -DRDCHK -DNAP \
	-DHDBUUCP $(KFLAGS) -F 3000 -i -M2let32" \
	"LIBS = -lx" "LNKFLAGS = -s -O -LARGE -F 3000 -i -M2let32"

#SCO Xenix/386 2.2.2 and 2.2.3
sco386:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/386 2.2.2...'
	$(MAKE) wermit \
	"CFLAGS= -DXENIX -DNOFILEH -Dunix -DRDCHK -DNAP $(KFLAGS) -Otcl -M3e" \
	"LNKFLAGS = -s" "LIBS = -lx"

#SCO XENIX/386 2.2.3 with Excelan TCP/IP + curses.
# NOTE: This one might need some work in C-Kermit 6.0.
# You might need to include /usr/include/sys/types.h
# containing "typedef char *caddr_t;".  Then at least it compiles, but the
# stack 
sco386netc:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/386 2.2.3 + Excelan TCP'
	$(MAKE) wermit \
	"CFLAGS= -I/usr/include/exos -DXENIX -DNOFILEH -DCK_CURSES -DDYNAMIC \
	-Dunix -DRDCHK -DNAP -DTCPSOCKET -DEXCELAN -DNOJC -DNOMKDIR \
	$(KFLAGS) -Otcl -M3e" \
	"LNKFLAGS = -s" "LIBS = -lc -lx -lsocket -lcurses -ltermcap"

#SCO XENIX/386 2.3.3 with gcc 1.37 or later...
sco386gcc:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/386 2.3.3, gcc...'
	@echo 'Add -D_NO_PROTOTYPE if you have trouble with Xenix header files'
	$(MAKE) wermit "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -O -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DNOJC -DNODEBUG -DDYNAMIC $(KFLAGS) \
	-traditional -fpcc-struct-return -fstrength-reduce \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_WORDSWAP -DM_XENIX \
	-DPWID_T=int " "LNKFLAGS = -s" "LIBS = -lx"

#As above, but with curses...
sco386gccc:
	@echo 'Making C-Kermit $(CKVER) for SCO Xenix/386 2.3.3, gcc...'
	@echo 'Add -D_NO_PROTOTYPE if you have trouble with Xenix header files'
	$(MAKE) wermit "CC = gcc" "CC2 = gcc" \
	"CFLAGS= -O -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DNOJC -DNODEBUG -DDYNAMIC -DCK_CURSES $(KFLAGS) \
	-traditional -fpcc-struct-return -fstrength-reduce \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_WORDSWAP -DM_XENIX \
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
#causes trouble.  No harm is done by removing it (see ckuins.doc).
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
	$(MAKE) wermit \
	"CFLAGS= -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DRENAME -DNOJC $(KFLAGS) -O" "LNKFLAGS = -s" "LIBS = -lc -lx"

# Exactly the same as above, but enables some special SCO-specific code
# that allegedly clears up some problems with HANGUP and with uugetty.
# For satisfactory operation on bidirectional lines that are handled by
# uugetty, you must install the kermit program with owner=group=uucp
# and permission 06755.
sco3r2x:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following four continued lines into one line.'
	@echo 'Also, remove -DRENAME if _rename unresolved at link time.'
	$(MAKE) wermit \
	"CFLAGS= -DSCO32 -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DNOJC -DRENAME -DNOCOTFMC $(KFLAGS) -Otcl" \
	"LNKFLAGS = -s" "LIBS = -lc -lx"

#SCO UNIX/386 3.2.0 and SCO Xenix 2.3.x with Excelan TCP/IP support.
#In case of compilation or runtime problems, try adding
#"-DUID_T=int -DGID_T=int" to the CFLAGS.  If that doesn't work, try
#"-DUID_T=uid_t -DGID_T=gid_t".
sco3r2net:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 / Excelan...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following three continued lines into one line.'
	$(MAKE) wermit \
	"CFLAGS= -I/usr/include/exos -DXENIX -DSVR3 -DDYNAMIC -DNOFILEH \
	-DHDBUUCP -DRDCHK -DNAP -DRENAME -DTCPSOCKET -DEXCELAN -DNOJC \
	$(KFLAGS) -O" "LNKFLAGS = -s" "LIBS = -lc -lx -lsocket"

#SCO UNIX/386 3.2.0 and SCO Xenix 2.3.x with Excelan TCP/IP support.
#As above, with curses library added for FULLSCREEN file transfer display.
#See comments for sco3r2net.
sco3r2netc:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 / Excelan / curses...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following three continued lines into one line.'
	$(MAKE) wermit \
	"CFLAGS= -I/usr/include/exos -DXENIX -DSVR3 -DDYNAMIC -DNOFILEH \
	-DHDBUUCP -DRDCHK -DNAP -DTCPSOCKET -DEXCELAN -DNOJC $(KFLAGS) \
	-DRENAME -DCK_CURSES -O" "LNKFLAGS = -s" \
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
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following five continued lines into one line.'
	$(MAKE) wermit \
	"CFLAGS= -I/usr/include/interlan -DXENIX -DNOFILEH -DHDBUUCP \
	-DSVR3 -DRDCHK -DNAP -DTCPSOCKET -DPARAMH -DINTERLAN -Di386 -DSYSV \
	-DRENAME -DNOJC $(KFLAGS) -Otcl -M3e" "LNKFLAGS = -s" \
	"LIBS = -lc -lx -ltcp"

# SCO XENIX/386 2.3.3 SysV with SCO TCP/IP
# System V STREAMS TCP developed by Lachman Associates Inc and
# Convergent Technologies.
# -DRENAME removed since some reports indicate it is not supported
# (whereas others say it is.)
sco3r2lai:
	@echo 'Making C-Kermit $(CKVER) for SCO XENIX/386 2.3.3 + TCP/IP...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following four continued lines into one line.'
	$(MAKE) wermit \
	"CFLAGS= -DLAI_TCP -Di386 -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK \
	-DNAP -DTCPSOCKET -DPWID_T=int $(KFLAGS) -Otcl -i -M3e" \
	"LNKFLAGS = -i -s" "LIBS = -lc -lx -lsocket"

sco3r2laic:
	@echo 'Making C-Kermit $(CKVER) for SCO XENIX/386 2.3.3 + TCP/IP...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following five continued lines into one line.'
	$(MAKE) wermit \
	"CFLAGS= -DLAI_TCP -Di386 -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK \
	-DNAP -DTCPSOCKET -DDYNAMIC -DCK_ANSIC -DCK_CURSES -DM_TERMINFO \
	-DPWID_T=int $(KFLAGS) -Otcl -i -M3e" \
	"LNKFLAGS = -i -s" "LIBS = -ltinfo -lc -lx -lsocket"

#SCO UNIX/386 3.2v2 (POSIX job control), shared libraries.
sco3r22:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v2 ...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following three continued lines into one line.'
	make wermit \
	"CFLAGS= -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP -DRENAME \
	-DPID_T=pid_t -DPWID_T=int -DDIRENT -DDYNAMIC $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lc_s -lc -lx"

#SCO UNIX/386 3.2v2, POSIX job control, fullscreen file transfer display,
#dynamic memory allocation, shared C library
sco3r22c:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v2 ...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following four continued lines into one line.'
	make wermit \
	"CFLAGS= -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DCK_CURSES -DDYNAMIC -DDIRENT -DRENAME \
	-DPID_T=pid_t -DPWID_T=int $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lcurses -lc_s -lc -lx"

#SCO UNIX/386 3.2v2 with gcc 1.40 or later (POSIX job control)
sco3r22gcc:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v2, gcc'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following seven continued lines into one line.'
	make wermit "CC = gcc" \
	"CFLAGS= -O -DPOSIX -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP \
	-DRENAME -traditional -fpcc-struct-return -fstrength-reduce \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix \
	-DPID_T=pid_t -DPWID_T=int $(KFLAGS) " "LNKFLAGS = -s" \
	"LIBS = -lc_s -lc -lx"

#SCO UNIX/386 3.2v2 (ODT 1.1) (POSIX job control) with SCO TCP/IP, shared libs
#Requires SCO TCP/IP or ODT development system for telnet.h, etc.
sco3r22net:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2.2 + TCP/IP...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following three continued lines into one line.'
	make wermit \
	"CFLAGS= -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP -DTCPSOCKET \
	-DRENAME -DPID_T=pid_t -DPWID_T=int -DDIRENT -DDYNAMIC \
	$(KFLAGS) -O" "LNKFLAGS = -s" "LIBS = -lsocket -lc_s -lc -lx"

#As above, but with curses for fullscreen file transfer display.
#Requires SCO TCP/IP or ODT development system for telnet.h, etc.
sco3r22netc:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v2 + TCP/IP...'
	@echo 'Warning: If make blows up, edit the makefile to join'
	@echo 'the following three continued lines into one line.'
	make wermit "CFLAGS= \
	-DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DRDCHK -DNAP -DTCPSOCKET -DRENAME \
	-DCK_CURSES -DDIRENT -DDYNAMIC \
	-DPID_T=pid_t -DPWID_T=int -O $(KFLAGS)" \
	"LNKFLAGS = -s" "LIBS = -lcurses -lsocket -lc_s -lc -lx"

#SCO UNIX/386 3.2v4 (POSIX job control), curses, ANSI C compilation,
#<dirent.h> (EAFS) file system.  Remove -lmalloc if it causes trouble.
#It was put there to avoid core dumps caused by regular libc.a malloc.
#CK_POLL is used instead of SELECT because SELECT requires gettimeofday(),
#which is in libsocket.a, which is available only on systems with the
#TCP/IP development system.  Add -J to make all chars unsigned.
sco32v4:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v4...'
	@echo 'If you get _ftime redefinition_ complaint,'
	@echo 'Try adding -DODT30 to CFLAGS.'
	make wermit \
	"CFLAGS= -DXENIX -DSVR3 -DDIRENT -DNOFILEH -DHDBUUCP -DCK_POLL -DNAP \
	-DRENAME -DCK_CURSES -DM_TERMINFO -DNOANSI -DPID_T=pid_t -DPWID_T=int \
	-DNOSETBUF -DDYNAMIC -DSVR3JC -DCK_RTSCTS $(KFLAGS) -O" \
	"LNKFLAGS = -s" "LIBS = -lcurses -lmalloc -lc_s -lc -lx"

#SCO UNIX/386 3.2v4 with gcc 1.40 or later, POSIX job control
#gcc 1.40 or later.  Also see comments in sco32r4 entry.
sco32v4gcc:
	make wermit "CC = gcc" \
	"CFLAGS= -O -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DCK_POLL -DNAP \
	-DNOANSI -DRENAME -DDIRENT -DCK_CURSES -DM_TERMINFO -DNOSETBUF \
	-traditional -fpcc-struct-return -fstrength-reduce \
	-D_KR -D_NO_PROTOTYPE -D_SVID \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix \
	-DPID_T=pid_t -DPWID_T=int -DSVR3JC -DCK_RTSCTS $(KFLAGS) " \
	"LNKFLAGS = -s" "LIBS = -lcurses -lc_s -lc -lx"

#SCO UNIX/386 3.2v4 (POSIX job control), TCP/IP, curses, ANSI C compilation,
#Requires SCO TCP/IP or ODT development system for telnet.h, etc.
#<dirent.h> (EAFS) file system.  With DIRENT, -lc must come before -lx.
#gcc 1.40 or later.  Also see comments in sco32r4 entry.
sco32v4netgcc:
	make wermit "CC = gcc" "CC2=gcc" \
	"CFLAGS= -O2 -DXENIX -DSVR3 -DNOFILEH -DHDBUUCP -DSELECT -DNAP \
	-DNOANSI -DRENAME -DTCPSOCKET -DDIRENT -DCK_CURSES -DM_TERMINFO \
	-DNOSETBUF -DDYNAMIC -D_KR -D_NO_PROTOTYPE -D_SVID \
	-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \
	-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \
	-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix \
	-DPID_T=pid_t -DPWID_T=int -DSVR3JC -DCK_RTSCTS $(KFLAGS) " \
	"LNKFLAGS = -s" "LIBS = -lcurses -lsocket -lc_s -lc -lx"

#SCO UNIX/386 3.2v4 (POSIX job control), TCP/IP, curses, ANSI C compilation,
#<dirent.h> (EAFS) file system.  With DIRENT, -lc must come before -lx.
#Reportedly it's OK to add -DCK_REDIR and -DCK_WREFRESH, and to remove -lc_s.
#Requires SCO TCP/IP development system or ODT for telnet.h, etc.
#See sco32v4 above for additional comments.
sco32v4net:
	@echo 'Making C-Kermit $(CKVER) for SCO UNIX/386 3.2v4...'
	@echo 'If you get _ftime redefinition_ complaint,'
	@echo 'use make sco_odt30.'
	$(MAKE) wermit \
	"CFLAGS= -DXENIX -DSVR3 -DDIRENT -DNOFILEH -DHDBUUCP -DRENAME \
	-DDYNAMIC -DTCPSOCKET -DCK_ANSIC -DCK_CURSES -DNAP -DCK_WREFRESH \
	-D_IBCS2 -DSELECT -DNOSETBUF -DPID_T=pid_t -DPWID_T=int -DSVR3JC \
	-DCK_RTSCTS -O -DCK_SCOV4 $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS) -s" \
	"LIBS = $(LIBS) -lcurses -lsocket -lmalloc -lc_s -lc -lx"

sco32v4netnd:
	@echo sco32v4net with no debug
	$(MAKE) "MAKE=$(MAKE)" sco32v4net \
	"KFLAGS=$(KFLAGS) -DNODEBUG -DNOTLOG" "LIBS=$(LIBS)"

sco3r2netnd:
	@echo sco32v4netnd built for SCO XENIX 2.3 under SCO UNIX...
	@echo   requires copying /lib/386/Slibc.a to /lib/386/Slibc_s.a and
	@echo   getting /lib/386/Slibsocket.a from a XENIX devkit.
	@echo   WARNING: poll/CK_POLL supported only on XENIX 2.3.4
	echo    For earlier XENIX systems, replace CK_POLL with RDCHK.
	$(MAKE) "MAKE=$(MAKE)" sco32v4netnd \
	"KFLAGS=$(KFLAGS) -x2.3 -DNORENAME" "LNKFLAGS = $(LNKFLAGS) -x2.3" \
	"LIBS=-ldir $(LIBS)"

sco_odt30:
	@echo SCO ODT 3.0
	$(MAKE) "MAKE=$(MAKE)" sco32v4net "KFLAGS=$(KFLAGS) -DODT30"

#SCO OpenServer 5.0 (SCO UNIX 3.2v5.0) with SCO development tools, no TCP/IP.
#SCO OSR5 is much more like standard System V than previous SCO releases.
#Thanks to Robert Lipe for the simplified makefile entries.
sco32v5: 
	@echo Making C-Kermit $(CKVER) for SCO OpenServer Release 5...
	$(MAKE) wermit CC=$(CC) CC2=$(CC2) \
	"CFLAGS= -O -DBIGBUFOK -DDYNAMIC -DDIRENT -DHDBUUCP -DSVR4 -DCK_SCOV5 \
	-DCK_RTSCTS -DCK_CURSES -DCK_WREFRESH -DCK_NEWTERM -DSELECT \
	-DSELECT_H $(KFLAGS)" "LIBS=-lcurses $(LIBS)" "LNKFLAGS=-s $(LNKFLAGS)"

#SCO OpenServer 5.0 with gcc, no networking.
sco32v5gcc:
	@echo Using gcc...
	$(MAKE) "MAKE=$(MAKE)" sco32v5 CC=gcc CC2=gcc

#SCO OpenServer 5.0 with networking, SCO development tools.
#Networking libraries are now provided with the OS.
sco32v5net:
	@echo TCP/IP networking added...
	$(MAKE) "MAKE=$(MAKE)" sco32v5 KFLAGS="-DTCPSOCKET $(KFLAGS)" \
	LIBS="-lsocket"

#SCO OpenServer 5.0 with networking, gcc.
sco32v5netgcc:
	@echo TCP/IP networking added - using gcc...
	$(MAKE) "MAKE=$(MAKE)" sco32v5 CC=gcc CC2=gcc KFLAGS="-DTCPSOCKET" \
	LIBS="-lsocket"

#SCO OpenServer 5.0 with networking, gcc, elf.
sco32v5netgccelf:
	@echo TCP/IP networking added - using gcc, dynamic elf library
	$(MAKE) "MAKE=$(MAKE)" sco32v5 CC=gcc CC2=gcc \
	KFLAGS="-DTCPSOCKET -O3 -belf" \
	LNKFLAGS="-belf" LIBS="-lsocket"

#PC/IX, Interactive Corp System III for IBM PC/XT
pcix:
	@echo 'Making C-Kermit $(CKVER) for PC/IX...'
	$(MAKE) wermit \
	"CFLAGS= -DPCIX -DISIII $(KFLAGS) -Dsdata=sdatax -O -i" "LNKFLAGS = -i"

#Integrated Solutions Inc V8S VME 68020
isi:
	@echo Making C-Kermit $(CKVER) for 4.2BSD on ISI...
	$(MAKE) wermit "CC = cc" \
	"CFLAGS= -DBSD4 -DTCPSOCKET -DINADDRX -DDCLPOPEN -DDEBUG -DNOSETREU \
	-DCK_CURSES $(KFLAGS)" "LIBS = -lcurses -ltermcap"

#Interactive Corp System III port in general --
#is3: (very old, probably not sufficient for 5A)
#	@echo 'Making C-Kermit $(CKVER) for Interactive System III...'
#	make wermit "CFLAGS = -DISIII -Ddata=datax -O -i" "LNKFLAGS = -i"
#The following should work, use it if you don't have gcc.
#Use is3gcc if you have gcc.
is3:
	@echo 'Making C-Kermit $(CKVER) for Interactive System III...'
	$(MAKE) wermit \
	"CFLAGS= -DISIII $(KFLAGS) -Ddata=datax -DNAP -DHDBUUCP
	-DLOCK_DIR=\"/usr/spool/uucp\" -DSIGTYP=void -O -i' "LNKFLAGS = -i"

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
is5r3:
	@echo 'Making C-Kermit $(CKVER) for Interactive 386/ix or later...'
	@echo 'If this does not work please read the makefile entry.'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -g -DNOCSETS \
	-DUID_T=ushort -DGID_T=ushort -DDYNAMIC -DI386IX $(KFLAGS)" \
	"LNKFLAGS = -g"

#Interactive Corp System System V R3 with gcc
is3gcc:
	@echo 'Making C-Kermit $(CKVER) for Interactive System V R3 / gcc...'
	$(MAKE) wermit CC=gcc CC2=gcc \
	'CFLAGS = -D_SYSV3 -DISIII -Ddata=datax -DNAP -DHDBUUCP \
	-DLOCK_DIR=\"/usr/spool/uucp\" -DSIGTYP=void -O' "LNKFLAGS ="

#Interactive UNIX System V R3, POSIX variant.  Untested.
#Uses dirent.h and Honey DanBer uucp.  Read comments in is5r3 entry.
is5r3p:
	@echo 'Making C-Kermit $(CKVER) for Interactive 386/ix or later...'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DHDBUUCP -g -DNOCSETS \
	-DDYNAMIC -DI386IX -DPOSIX $(KFLAGS)" "LNKFLAGS=" "LIBS=-lcposix"

#Interactive UNIX SVR3 2.2.1, job control, curses, no net, gcc.
is5r3gcc:
	$(MAKE) wermit CC=gcc CC2=gcc \
	"CFLAGS=-g -posix -DSVR3 -DDIRENT \
	-DHDBUUCP -O -DNOCSETS -DDYNAMIC -DI386IX -DSVR3JC -DCK_CURSES \
	$(KFLAGS)" LNKFLAGS="-posix" LIBS="-lcurses -lc_s"

#Interactive UNIX System V R3 with TCP/IP network support.
#Needs -linet for net functions.  signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp. Read comments in is5r3 entry.
#Also see is5r3net2 if you have trouble with this entry.
is5r3net:
	@echo 'Making C-Kermit $(CKVER) for Interactive 386/ix...'
	@echo 'If this does not work please read the makefile entry.'
	$(MAKE) wermit CC="$(CC)" CC2="$(CC2)" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DTCPSOCKET \
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
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DTCPSOCKET -DNOJC \
	-DSIGTYP=void -DNOANSI -DI386IX $(KFLAGS) -O" \
	"LNKFLAGS= -s" "LIBS = -linet -lc_s"

#Interactive UNIX System V R3 (version 2.2 or later) with job control & curses.
#Uses dirent.h and Honey DanBer UUCP.
is5r3jc:
	@echo 'Making C-Kermit $(CKVER) for Interactive Unix 2.2 or later...'
	$(MAKE) wermit CC="$(CC)" CC2="$(CC2)" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -O -DNOCSETS \
	-DUID_T=ushort -DGID_T=ushort -DDYNAMIC -DI386IX -DSVR3JC -DCK_CURSES \
	-DPOSIX_JC -DCK_REDIR -DNOSETBUF \
	-DCK_POLL \
	$(KFLAGS)" "LIBS=-lcurses -lc_s"

is5r3jcgcc:
	$(MAKE) is5r3jc CC="gcc -DCK_ANSILIBS -DDCGPWNAM -O4" CC2=gcc \
	KFLAGS="$(KFLAGS)" LNKFLAGS="$(LNKFLAGS)"

#Sunsoft/Interactive UNIX System V R3 (version 2.2 or later)
#with job control, curses, and TCP/IP networking.
#Uses dirent.h and Honey DanBer UUCP.
is5r3netjc:
	@echo 'Making C-Kermit $(CKVER) for Interactive Unix 2.2 or later...'
	$(MAKE) wermit CC="$(CC)" CC2="$(CC2)" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -O -DNOCSETS \
	-DUID_T=ushort -DGID_T=ushort -DDYNAMIC -DI386IX -DSVR3JC -DCK_CURSES \
	-DPOSIX_JC -DCK_REDIR -DNOSETBUF \
	-DTCPSOCKET -DSELECT -DHADDRLIST \
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
	"CFLAGS= -DATTSV -DHDBUUCP -DDIRENT -DTCPSOCKET -DDYNAMIC \
	-DNOSETBUF $(KFLAGS)"

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
		"CFLAGS= -DSVR3 -DDIRENT -DDYNAMIC $(KFLAGS) -O" \
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
	"CFLAGS = -DATTSV -DDYNAMIC $(KFLAGS) -O" "LNKFLAGS = -n"

#NCR Tower 32, OS Release 2.xx.xx
tower32-2:
	@echo 'Making C-Kermit $(CKVER) for NCR Tower 32 Rel 2 System V R2...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DHDBUUCP -DDYNAMIC $(KFLAGS) -O2" \
	"LNKFLAGS = -n"

#NCR Tower 32, OS Releases based on System V R3
#Don't add -DNAP (doesn't work right) or -DRDCHK (not available in libc).
tower32:
	@echo 'Making C-Kermit $(CKVER) for NCR Tower 32 System V R3...'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DNOSYSIOCTLH $(KFLAGS) \
	-DUID_T=ushort -DGID_T=ushort -O1"

#NCR Tower 32, OS Releases based on System V R3
tower32g:
	@echo 'Making C-Kermit $(CKVER) for NCR Tower 32 System V R3, gcc...'
	$(MAKE) wermit "CC = gcc" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DNOSYSIOCTLH $(KFLAGS) \
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
	-SYM 800  -DDYNAMIC -DNOSETBUF -DCK_CURSES $(KFLAGS) -DPID_T=short" \
	"LNKFLAGS= -n -s" "LIBS= -lcurses -ltermcap -lv -lnet"
	$(MAKE) ckuxla.$(EXT) "CFLAGS= -DNODEBUG -DBSD4 -DFT21 -DNOFILEH \
	-SYM 800  -DDYNAMIC -DNOSETBUF -DCK_CURSES $(KFLAGS) -DPID_T=short" \
	"LNKFLAGS= -n -s" "LIBS= -lcurses -ltermcap -lv -lnet"
	$(MAKE) ckudia.$(EXT) "CFLAGS= -DNODEBUG -DBSD4 -DFT21 -DNOFILEH \
	-SYM 800  -DDYNAMIC -DNOSETBUF -DCK_CURSES $(KFLAGS) -DPID_T=short" \
	"LNKFLAGS= -n -s" "LIBS= -lcurses -ltermcap -lv -lnet"
	$(MAKE) wermit \
	"CFLAGS= -O -DNODEBUG -DBSD4 -DFT21 -DNOFILEH -SYM 800 \
	-DDYNAMIC -DNOSETBUF -DCK_CURSES $(KFLAGS) -DPID_T=short" \
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

#Tandy 16/6000 with Xenix 3.0
#Add more -DNOxxx options to remove features if program won't load.
#Successful operation is a function of program size, physical memory,
#available swap space, etc.  The following stripped-down configuration
#seems to work on most Tandy 6000s.  NOTE: "-+" means allow long variable
#names, needed for C-Kermit 6.0 because some identifiers are not unique
#within the first six characters.
trs16:
	@echo 'Making C-Kermit $(CKVER) for Tandy 16/6000, Xenix 3.0...'
	$(MAKE) wermit \
	"CFLAGS = -+ -DATTSV -DTRS16 -DNOMKDIR -DDCLPOPEN -DCK_CURSES \
	-DDYNAMIC -DNODEBUG -DNOTLOG -DNOHELP -DNOSCRIPT -DNOCSETS \
	$(KFLAGS) -O" "LIBS= -lcurses -ltermcap" "LNKFLAGS = -+ -n -s"

#DIAB DS90 or LUXOR ABC-9000 with pre-5.2 DNIX.  Sys V with nap() and rdchk().
# nd = no opendir(), readdir(), closedir(), etc.
# Some of the modules fail to compile with -O.
dnixnd:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with very old DNIX 5.2.'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNAP -DRDCHK -DDYNAMIC -DDCLPOPEN \
	-U__STDC__ $(KFLAGS)"

#DIAB DS90 with DNIX 5.2.  Sys V with nap() and rdchk().
# This one has opendir(), readdir(), closedir(), etc.
# Some of the modules fail to compile with -O.
dnix:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with old DNIX 5.2...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNAP -DRDCHK -DDIRENT -DDYNAMIC  \
	-U__STDC__ $(KFLAGS)"

#DIAB DS90 with DNIX 5.2.  Sys V with nap() and rdchk().
# As above, but with curses and TCP/IP.
# You might get complaints about redefinition of O_RDONLY, etc, because
# of bugs in the DNIX header files, which can be fixed by adding #ifndef...
# around the offending definitions in the header files.
dnixnetc:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with old DNIX 5.2...'
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNAP -DRDCHK -DDIRENT -DDYNAMIC  \
	-DTCPSOCKET -DCK_CURSES -I/usr/include/bsd -U__STDC__ $(KFLAGS)" \
	"LIBS = -ln -lcurses"

#DIAB DS90 with DNIX 5.3 or later, with HDB UUCP, nap() and rdchk().
dnix5r3:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with DNIX 5.3...'
	@echo 'with Honey DanBer UUCP'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DHDBUUCP -DNAP -DRDCHK -DDIRENT \
	-DDYNAMIC -DCK_CURSES -DRENAME $(KFLAGS) -O" "LIBS= -lcurses"

#DIAB DS90 with DNIX 5.3 or later, with HDB UUCP, nap() and rdchk() + TCP/IP
dnix5r3net:
	@echo 'Making C-Kermit $(CKVER) for DIAB DS90 with DNIX 5.3...'
	@echo 'with Honey DanBer UUCP and TCP/IP'
	$(MAKE) wermit \
	"CFLAGS = -DSVR3 -DHDBUUCP -DNAP -DRDCHK -DDIRENT \
	-DTCPSOCKET -DDYNAMIC -DCK_CURSES -DRENAME $(KFLAGS) -O \
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
	"CFLAGS = -DSVR3 -DDIAB -DHDBUUCP -DNAP -DRDCHK -DDIRENT -DDYNAMIC \
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
	"CFLAGS = -DSVR3 -DDIAB -DHDBUUCP -DNAP -DRDCHK -DDIRENT -DDYNAMIC \
	-DTCPSOCKET -DCK_ANSILIBS -DCK_CURSES -DRENAME -O -X7 -X9 $(KFLAGS) \
	-I/usr/include/bsd" "LIBS= -ln -lcurses"

# QNX 4.21 and above, 32-bit version, Watcom C32 10.6, register calling
# conventions, fully configured, except no job control because QNX 4.x
# does not support it.  New NCURSES library used instead of CURSES.
#
# -Oatx optimizes to favor speed over size: loop optimization, inline fn's.
# -Os favors size over speed.  The size difference is about 30-40K.
# -NOUUCP is included because QNX is shipped without UUCP and no native
# QNX software uses UUCP lockfiles.  Remove -DNOUUCP if you want to use
# UUCP lockfiles for exclusive access to dialout devices.
qnx32:
	@echo 'Making C-Kermit $(CKVER) for QNX 4.2x, 32-bit...'
	$(MAKE) wermit \
	"LNKFLAGS = -3r" \
	"CFLAGS = -3r -ms -DQNX -DDYNAMIC -DKANJI -DTCPSOCKET -DCK_CURSES \
	-DCK_WREFRESH -DCK_REDIR -DSELECT -DSELECT_H -DCK_RTSCTS -DNOJC \
	-DNOUUCP -DNOSETBUF -DCK_ANSIC -DPID_T=pid_t -Oatx -zc $(KFLAGS)" \
	"LIBS= -lsocket -lncurses -ltermcap"
	@wermit -h >use.qnx
	@usemsg wermit use.qnx
	@rm use.qnx

# Synonym for qnx32. 
qnx:
	$(MAKE) qnx32 "KFLAGS=$(KFLAGS)"

# QNX 4.21 and above, 16-bit version, Watcom C 8.5 - Watcom 9.52
# on i286 PCs and above.
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
# 10.6 installed, you should already have curses.h, which is for new ncurses
# library. You must back it up and use free curses.h instead, since ncurses is
# only for 32-bit applications and some definitions in these files are
# different (e.g., clearok()).
#
qnx16:
	@echo 'Making C-Kermit $(CKVER) for QNX 4.21, 16-bit...'
	$(MAKE) wermit \
	"LNKFLAGS = -2 -ml -N 26000" \
	"CFLAGS = -2 -Oatx -zc -zt100 -ml -DNOSETBUF -DNOUUCP \
	-DQNX -DDYNAMIC -DCK_REDIR -DSELECT -DSELECT_H -DNOJC -DTCPSOCKET \
	-DCK_RTSCTS -DCK_CURSES -DCK_WREFRESH -DCK_ANSIC -DKANJI \
	-DPID_T=pid_t $(KFLAGS)" \
	"LIBS=-lsocketl -lcurses -ltermcap -ltermlib"
	@wermit -h >use.qnx
	@usemsg wermit use.qnx
	@rm use.qnx

# QNX 4.1, 16-bit version, with Watcom C 8.5 on i286 PCs and above.
# stacksize 26000, objects larger than 100 bytes in their own segments,
# string constants to the codesegment, etc.  Add -DNOUUCP if desired.
qnx16_41:
	@echo 'Making C-Kermit $(CKVER) for QNX 4.1, 16-bit...'
	$(MAKE) wermit \
	"LNKFLAGS = -mh -N 26000" "CFLAGS = -Wc,-fpc -Wc,-j \
	-Wc,-Ols -Wc,-zdf -Wc,-zc -Wc,-zt100 -mh -DPOSIX -DQNX -DDIRENT \
	-DNOCYRIL -DNODEBUG -DNOMSEND -DMINIDIAL -DNOXMIT -DNOSCRIPT -DNOSPL \
	-DNOSETKEY -DDYNAMIC -DPID_T=pid_t $(KFLAGS)"
	@wermit -h >use.qnx
	@usemsg wermit use.qnx
	@rm use.qnx

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
# also needs getcwd() external function; see ckuins.doc file.
# also, sys/types.h needed modifying:
#   #ifdef __SYS_TYPES_H__, #define ..., #endif
# also, ckuus2.c MUST be compiled NOOPT else symbol table is destroyed!
# Submission by Robert Weiner/Programming Plus, rweiner@progplus.com.
#
altos3:
	@echo 'Making C-Kermit $(CKVER) for Altos ACS68k UNIX System III'
	$(MAKE) ckuus2.$(EXT) "CFLAGS = -DATTSV -DNOCSETS -DNOSETKEY -DNOJC \
	-DNODIAL -DDCLPOPEN -DNOSETBUF -DNOSCRIPT -DNOHELP $(KFLAGS) -i"
	$(MAKE) wermit \
	"CFLAGS = -DATTSV -DNOCSETS -DNOSETKEY -DNOJC \
	-DNODIAL -DDCLPOPEN -DNOSETBUF -DNOSCRIPT -DNOHELP $(KFLAGS) -i -O" \
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
# (see ckccfg.doc).
minix:
	@echo 'Making C-Kermit $(CKVER) for MINIX, no command parser...
	@echo 'TOTALLY UNTESTED!'
	$(MAKE) wermit EXT=s \
	"CFLAGS= -DV7 -DMINIX -i -D_MINIX -D_POSIX_SOURCE -DDYNAMIC \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V \
	-DNOXMIT -DNOMSEND -DNOFRILLS -DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOICP -DNOSETKEY $(KFLAGS)" \
	"LNKFLAGS= -i -T"

#MINIX - PC version with 64K+64K limit, new (as yet unreleased) ACK 2.0 beta C
#compiler, which outputs .o object files, rather than .s.  But 'make' still
#expects .s files, so must be patched to use .o.  Tested on Minix 1.5.10.
minixnew:
	@echo 'Making C-Kermit $(CKVER) for MINIX (new ACK 2.0 compiler),'
	@echo 'no command parser...  TOTALLY UNTESTED!'
	$(MAKE) wermit \
	"CFLAGS= -DV7 -DMINIX -i -D_MINIX -D_POSIX_SOURCE -DDYNAMIC \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V -DNODIAL \
	-DNOHELP -DNODEBUG -DNOTLOG -DNOSCRIPT -DNOCSETS -DNOICP $(KFLAGS)" \
	"LNKFLAGS= -i -T"

#MINIX/386 (PC Minix modified by Bruce Evans in Australia for 386 addressing)
minix386:
	@echo 'Making C-Kermit $(CKVER) for MINIX/386...'
	@echo 'TOTALLY UNTESTED!'
	$(MAKE) wermit EXT=s \
	"CFLAGS= -DV7 -DMINIX -D_POSIX_SOURCE -DDYNAMIC $(KFLAGS)"

#MINIX/386 (PC Minix modifed by Bruce Evans in Australia to use 386 addressing)
minix386gcc:
	@echo 'Making C-Kermit $(CKVER) for MINIX/386 with gcc...'
	@echo 'TOTALLY UNTESTED!'
	$(MAKE) wermit "CC=gcc -g -O" "CC2=gcc -g" \
	"CFLAGS= -DV7 -DMINIX -D_POSIX_SOURCE -DDYNAMIC $(KFLAGS)"

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
	@echo 'Making C-Kermit $(CKVER) for MINIX 68k with ACK...
	$(MAKE) wermit \
	"CFLAGS= -DV7 -DMINIX -D_MINIX -D_POSIX_SOURCE -DDYNAMIC \
	-DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOSPL $(KFLAGS) \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V"

#MINIX - 68k version with c68 compiler.
# If you have trouble compiling or running wart, "touch wart" or
# "touch ckcpro.c". Compiling ckudia.c (no -DNODIAL!) might fail. :-(
# Give c68 250000 bytes of stack+heap; make sure make(1) has at least
# 100000 chmemory.  On a 1Mb Atari ST this means that the recursive
# call of make fails due to memory shortage.  Try "make -n minixc68 >makeit",
# followed by ". makeit".  Otherwise, as above.
minixc68:
	@echo 'Making C-Kermit $(CKVER) for MINIX 68k with c68...
	$(MAKE) wermit "CC= cc -c68" \
	"CFLAGS= -DV7 -DMINIX -D_MINIX -D_POSIX_SOURCE -DDYNAMIC \
	-DNODIAL -DNOHELP -DNODEBUG -DNOTLOG \
	-DNOSCRIPT -DNOCSETS -DNOSPL $(KFLAGS) \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V"

#MINIX - 68k version with c68 compiler.
#A variation on the above that was recently (Sep 95) reported to work.
minixc68a:
	@echo 'Making C-Kermit $(CKVER) for MINIX 68k with c68...
	$(MAKE) wermit "CC= cc -c68" \
	"CFLAGS= -DV7 -DMINIX -D_MINIX -D_POSIX_SOURCE -DDYNAMIC \
	-DCK_ANSIC -DNODEBUG -DNOTLOG -DMINIDIAL -DEXTEN -DMYCURSES \
	-DNOSCRIPT -DNOCSETS -DNOSPL -DNOJC -DDIRENT -DNOSETBUF \
	-DNOSETKEY -DNOESCSEQ $(KFLAGS) \
	-DPID_T=pid_t -DUID_T=uid_t -DGID_T=gid_t -DSIG_V"

#MIPS Computer Systems with UMIPS RISC/OS 4.52 = AT&T UNIX System V R3.0.
#Remove -DNOJC if job control can be safely used.
mips:
	@echo 'Making C-Kermit $(CKVER) for MIPS RISC/OS...'
	$(MAKE) wermit \
	"CFLAGS = -DMIPS -DDIRENT -DDYNAMIC -DCK_POLL -DNOJC -DPID_T=int \
	-DGID_T=gid_t -DUID_T=uid_t -i -O1500 $(KFLAGS)"

#As above, but with TCP/IP and fullscreen support.
mipstcpc:
	@echo 'Making C-Kermit $(CKVER) for MIPS RISC/OS...'
	$(MAKE) wermit \
	"CFLAGS = -DMIPS -DDIRENT -DDYNAMIC -DCK_POLL -DNOJC \
	-DTCPSOCKET -DCK_CURSES -I/usr/include/bsd \
	-DPID_T=int -DGID_T=gid_t -DUID_T=uid_t -i -O1500 $(KFLAGS)" \
	"LIBS = -lcurses -lbsd"

#Motorola Delta System V/68 R3, signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp.  Supports TCP/IP.
#After building, use "mcs -d" to reduce size of the executable program.
sv68r3:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/68 R3...'
	$(MAKE) wermit "CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DTCPSOCKET $(KFLAGS) \
	-O" "LNKFLAGS ="

#Motorola Delta System V/68 R3V5, signal() is void rather than int.
#Uses dirent.h and Honey DanBer UUCP.  Supports TCP/IP.
#After building, use "mcs -d" to reduce size of the executable program.
sv68r3v5:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/68 R3V5'
	$(MAKE) wermit "CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DTCPSOCKET -DINADDRX $(KFLAGS) -O" "LNKFLAGS =" "LIBS = -linet"

#Motorola MVME147 System V/68 R3 V5.1. Requires gcc 2.1 to compile.
#After building, use "mcs -d" to reduce size of the executable program.
sv68r3v51:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/68 R3V5.1'
	$(MAKE) wermit "CC=gcc-delta" "CC2=gcc-delta" \
	"CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC -DNODEBUG -DNOSETBUF \
	$(KFLAGS) -O2 -v -ftraditional" "LNKFLAGS = -s -v" "LIBS = -lm881"

#Motorola MVME147 System V/68 R3V6. derived from Motorola Delta System R3V5.
#Checked on larger Motorola System V/68 R3V6 ( with NSE Network Services Ext.)
#After building, use "strip" to reduce size of the executable program.
#Try removing the LIBS clause if it causes trouble (libnsl might not actually
#be necessary, and reportedly it is missing on some systems).
sv68r3v6:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/68 R3V6'
	@echo 'Remove the LIBS clause if it causes trouble.'
	$(MAKE) wermit "CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DTCPSOCKET $(KFLAGS) -O" "LNKFLAGS =" "LIBS = -lnsl"

#Motorola Delta System V/88 R32, signal() is void rather than int.
#Uses dirent.h and Honey DanBer uucp.  Needs <sys/utime.h> for setting
#file dates.  Supports TCP/IP.
#After building, use "mcs -d" to reduce size of the executable program.
sv88r32:
	@echo 'Making C-Kermit $(CKVER) for Motorola UNIX System V/88 R32...'
	$(MAKE) wermit "CFLAGS = -DSVR3 -DDIRENT -DHDBUUCP -DDYNAMIC \
	-DTCPSOCKET -DSYSUTIMEH -DCK_CURSES $(KFLAGS) \
	-O" "LIBS= -lcurses" "LNKFLAGS = -s"

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
	$(MAKE) wermit "CFLAGS = -O -DSVR4 -DMOTSV88R4 -DDIRENT -DHDBUUCP \
	-DDYNAMIC -DSTERMIOX -DTCPSOCKET -DCK_CURSES $(KFLAGS)" \
	"LIBS= -lsocket -lnsl -lcurses" "LNKFLAGS = -s"

#PFU Compact A Series UNIX System V R3, SX/A TISP V10/L50 (Japan)
#Maybe the -i link option should be removed?
sxae50:
	@echo 'Making C-Kermit $(CKVER) for PFU SX/A V10/L50...'
	$(MAKE) wermit \
	"CFLAGS= -DSVR3 -DDIRENT -DsxaE50 -DTCPSOCKET $(KFLAGS) -i -O \
	-DKANJI" "LNKFLAGS= "

#Olivetti X/OS R2.3, 3.x.
#NOTES:
# . If you build the executable on 2.x X/OS, it will also run on 3.x.
# . If you build it on 3.x X/OS, it will NOT run on 2.x.
# . Kermit can run with no privileges unless the uucp lines are protected,
#   in which case kermit must be owned by uucp with suid bit set:
#   chown uucp kermit ; chmod 4111 kermit.
xos23:
	@echo 'Making C-Kermit $(CKVER) for Olivetti X/OS...'
	$(MAKE) wermit \
	'CFLAGS=-OLM -DOXOS -DTCPSOCKET -DHDBUUCP -DDYNAMIC $(KFLAGS)' \
	"LIBS=" "LNKFLAGS="

#As above, but with curses.
xos23c:
	@echo 'Making C-Kermit $(CKVER) for Olivetti X/OS with curses...'
	$(MAKE) wermit \
	'CFLAGS=-OLM -DOXOS -DTCPSOCKET -DHDBUUCP -DDYNAMIC -DCK_CURSES \
	$(KFLAGS)' "LIBS=-lcurses" "LNKFLAGS="

#Clean up intermediate and object files
clean:
	@echo 'Removing object files...'
	-rm -f ckcmai.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckcpro.$(EXT) ckcfns.$(EXT) \
ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) ckucon.$(EXT) ckutio.$(EXT) \
ckufio.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) ckwart.$(EXT) ckuusx.$(EXT) \
ckuusy.$(EXT) ckcnet.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) ckusig.$(EXT) \
ckcmdb.$(EXT) ckcpro.c wart

#Run Lint on this mess for the SUN/BSD version.
lintsun:
	@echo 'Running Lint on C-Kermit $(CKVER) sources for SunOS version...'
	lint -x -DSUNOS4 -DDIRENT -DTCPSOCKET -DSAVEDUID -DDYNAMIC \
	ck[cu]*.c > ckuker.lint.sun

lintbsd:
	@echo 'Running Lint on C-Kermit $(CKVER) sources for BSD 4.2 version..'
	lint -x -DBSD4 -DTCPSOCKET ck[cu]*.c > ckuker.lint.bsd42

lints5:
	@echo 'Running Lint on C-Kermit $(CKVER) sources for Sys V version...'
	lint -x -DATTSV ck[cu]*.c > ckuker.lint.s5

lintmips:
	@echo 'Running lint on C-Kermit $(CKVER) sources for MIPS version...'
	lint -DMIPS -DDIRENT -DPID_T=int -DGID_T=gid_t \
	-DUID_T=uid_t -DNOJC ck[cu]*.c > ckuker.lint.mips

ckuuid:
	@echo 'building C-Kermit $(CKVER) set-UID/set-GID test programs'
	$(CC) -DANYBSD -DSAVEDUID -o ckuuid1 ckuuid.c
	$(CC) -DANYBSD -o ckuuid2 ckuuid.c
	$(CC) -DANYBSD -DNOSETREU -o ckuuid3 ckuuid.c
	$(CC) -DANYBSD -DSETEUID -DNOSETREU -o ckuuid4 ckuuid.c
	$(CC) -o ckuuid5 ckuuid.c
	@echo 'Read the top of ckuuid.c for directions...for testing'
	@echo 'you must make these programs setuid and setgid'

#Remember TECO?
love:
	@echo 'Not war?'
