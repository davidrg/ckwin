# Makefile to build C-Kermit for Berkeley, Microsoft, Interactive, 
#		and ATT Unix 
# for Berkeley Unix 4.x, "make bsd"
# for Microsoft xenix (/286, pc/at, etc) "make xenix"
# for Interactive on pc/xt "make pcix"
# for Interactive on other host "make is3"
# for Bell generic III/V "make sys3"
# for Pro-3xx Venix 1.0 "make provx1"
# for NCR Tower 1632, "make tower1"

LNKFLAGS=

make: 
	@echo 'Make what?  You must tell which system to make C-Kermit for.'

wermit: ckmain.o ckcmd.o ckuser.o ckusr2.o ckusr3.o ckprot.o ckfns.o ckfns2.o \
		 ckconu.o ckxunx.o ckzunx.o ckdial.o cklogi.o makefile
	cc $(LNKFLAGS) -o wermit ckmain.o ckxunx.o ckzunx.o ckfns.o ckfns2.o \
		 ckprot.o ckcmd.o ckusr2.o ckusr3.o ckuser.o ckconu.o \
		 ckdial.o cklogi.o

ckmain.o: ckmain.c ckermi.h

ckuser.o: ckuser.c ckcmd.h ckermi.h ckuser.h

ckusr2.o: ckusr2.c ckcmd.h ckermi.h ckuser.h

ckusr3.o: ckusr3.c ckcmd.h ckermi.h ckuser.h

ckcmd.o: ckcmd.c ckcmd.h ckdebu.h

ckprot.o: ckprot.w wart ckermi.h
	wart ckprot.w ckprot.c ; cc $(CFLAGS) -c ckprot.c

ckfns.o: ckfns.c ckermi.h ckdebu.h

ckfns2.o: ckfns.c ckermi.h ckdebu.h

ckzunx.o: ckzunx.c ckermi.h ckdebu.h

ckxunx.o: ckxunx.c ckdebu.h

ckconu.o: ckconu.c ckermi.h

wart: ckwart.o
	cc $(LNKFLAGS) -o wart ckwart.o

ckwart.o: ckwart.c

ckdial.o: ckdial.c

cklogi.o: cklogi.c

#Berkeley Unix
bsd:
	make wermit "CFLAGS= -DBSD4"

#Microsoft "Xenix/286" e.g., as sold by IBM for PC/AT
xenix:
	make wermit "CFLAGS= -DXENIX -DUXIII -F3000 -i" "LNKFLAGS = -F3000 -i"

#PC/IX, Interactive Corp System III port for IBM PC/XT as sold by IBM
pcix:
	make wermit "CFLAGS= -DPCIX -DUXIII -DISIII -Dsdata=sdatax -O -i" \
		"LNKFLAGS = -i"

#interactive corp system III port --
is3:
	make wermit "CFLAGS = -DISIII -DUXIII -Ddata=datax -O -i" \
		"LNKFLAGS = -i"

#plain old Bell System III or System V without strange things
sys3:
	make wermit "CFLAGS = -DUXIII -i -O" "LNKFLAGS = -i"

#DEC Pro-3xx with Venix 1.0
provx1:
	make wart "CFLAGS= " "LNKFLAGS= "
	make wermit "CFLAGS = -DPROVX1 -md780" \
		"LNKFLAGS= -u _sleep -lc -md780"

#NCR Tower 1632, OS 1.02
tower1:
	make wermit "CFLAGS = -DTOWER1"

