</$objtype/mkfile
CC=pcc -c -B
LD=pcc
CO=`{sed -n 's/^O=//p' /$cputype/mkfile}

OFILES=\
	ckcmai.$O \
	ckutio.$O \
	ckufio.$O \
	ckcfns.$O \
	ckcfn2.$O \
	ckcfn3.$O \
	ckuxla.$O \
	ckcpro.$O \
	ckucmd.$O \
	ckuus2.$O \
	ckuus3.$O \
	ckuus4.$O \
	ckuus5.$O \
	ckuus6.$O \
	ckuus7.$O \
	ckuusx.$O \
	ckuusy.$O \
	ckuusr.$O \
	ckucon.$O \
	ckudia.$O \
	ckuscr.$O \
	ckcnet.$O \
	ckusig.$O

HFILES=\
	ckcasc.h \
	ckcdeb.h \
	ckcker.h \
	ckcnet.h \
	ckcsig.h \
	ckcsym.h \
	ckcxla.h \
	ckucmd.h \
	ckuusr.h \
	ckuver.h \
	ckuxla.h

BIN=/$objtype/bin
TARG=kermit

</sys/src/cmd/mkone

# source gets too many warnings to include -w, even with -B
CFLAGS= -D_POSIX_SOURCE -DPOSIX -DNOUUCP -D_BSD_EXTENSION -DCK_RTSCTS -DBIGBUFOK -DCK_SYSINI="/sys/lib/kermit/ckermit.ini" -DCK_INI_B -DNETCONN -DPlan9

ckcpro.c:	$CO.wart ckcpro.w
	./$CO.wart ckcpro.w ckcpro.c

$CO.wart:	ckwart.c
	objtype=$cputype
	mk -f ckpker.mk wart.$cputype

wart.$cputype:V:	ckwart.$O
	$LD -o $O.wart ckwart.$O

clean:V:
	rm -f *.[$OS] [$OS].out [$OS].wart ckcpro.c $TARG

nuke:V:
	rm -f *.[$OS] [$OS].out [$OS].wart ckcpro.c $TARG
