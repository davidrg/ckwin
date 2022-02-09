#include <stdio.h>
#include fab
#include nam
#include descrip
#include iodef
#include ssdef
#include ttdef
#include tt2def

extern int confd, debug, turn, ttnam;

ttopen(tn)
char *tn;
{
    int fd;

    if (tn == 0)			/* Line name supplied? */
    {
	fd = open("SYS$OUTPUT",2);	/* Open the console */
	confd = fd;			/* Save file descriptor for ttbin() */
    }
    else
    {
	fd = open(tn,2);		/* Open the specified line */
	ttnam = tn;			/* Save file name for ttbin() */
    }
    if (fd < 0)
    {
	printmsg("Cannot open %s",tn);
	exit(1);
    }
    return(fd);
}  


ttbin(fd,old)
int fd, *old;
{
    int stat, new[3], *des;
    $DESCRIPTOR(sys_con,"SYS$OUTPUT");	/* Descriptor for the console */
    $DESCRIPTOR(sys_tt,"TTA0");		/* Line descriptor to fill in later */

    if (fd == confd) 			/* Is this the console? */
	stat = SYS$ASSIGN(&sys_con,&old[3],0,0);
    else
    {
	sys_tt.dsc$a_pointer = ttnam;	/* Set up this descriptor with the */
	sys_tt.dsc$w_length = strlen(ttnam); /* right tty name */
	stat = SYS$ASSIGN(&sys_tt,&old[3],0,0);
    }

    if (stat != SS$_NORMAL)
    {
	if (debug) printf("SYS$ASSIGN: Abnormal return: %d\n",stat);
	exit(1);
    }
    if (debug) printf("SYS$OUTPUT open on channel %d\n",old[3]);

    SYS$QIOW(0,old[3],IO$_SENSEMODE,0,0,0,old,12,0,0,0,0);
    if (debug) printf("Old TTY characteristics: %o %o %o\n",old[0], old[1],
	old[2]);

    stat = SYS$QIOW(0,old[3],IO$_SENSEMODE,0,0,0,new,12,0,0,0,0);
    if (stat != SS$_NORMAL)
    {
	if (debug) printf("SYS$QIOW: Abnormal return: %d\n",stat);
	exit(1);
    }

    new[1] |= TT$M_EIGHTBIT | TT$M_NOBRDCST | TT$M_NOECHO | TT$M_PASSALL;
    new[1] &= ~ (TT$M_CRFILL | TT$M_ESCAPE | TT$M_HALFDUP | TT$M_HOLDSCREEN |
		TT$M_LFFILL | TT$M_NOTYPEAHD | TT$M_READSYNC | TT$M_WRAP);
    new[2] &= ~ TT2$M_LOCALECHO;
    SYS$QIOW(0,old[3],IO$_SETMODE,0,0,0,new,12,0,0,0,0);
    if (debug)
	printf("New TTY characteristics: %o %o %o\n",new[0],new[1],new[2]);
}

ttres(fd,old)
int fd, *old;
{
    SYS$QIOW(0,old[3],IO$_SETMODE,0,0,0,old,12,0,0,0,0);
}

connect()
{
    printf("Connect command not implemented in this version\n");
}
