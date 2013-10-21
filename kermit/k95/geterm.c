#ifdef VCSTR
 static char GETERMC[]="@(#) geterm.c 1.6 91/02/15 12:56:55";  /*sccsid*/
#endif

#ifdef DOCUMENTATION
 ******************************* DOCZ Header *********************************
.MODULE                         geterm
.LIBRARY                        toolz
.TYPE                           program
.SYSTEM                         unix
.AUTHOR                         Todd Merriman
.LANGUAGE                       C
.APPLICATION            terminal
.DESCRIPTION            
        Determine the terminal attached
.ARGUMENTS                      
        geterm
.NARRATIVE                      
        The geterm utility queries the terminal on standard input by commanding
        the answerback sequence.  The following may be passed to standard output:

        vt320
        vt220
        vt100
        vt102
        z29
        vt52
        wy50
        h19     
        avt-4p-s
        avt-8p-s
        la120   
        cit101e 
        xt100+  
        vt125   
        vt200-sb
        f220    
        tvi9220 

        An empty line will be output if the terminal cannt be identified.
.RETURNS                        
        0 if the answerback is valid, 1 if not.
.CAUTIONS
        The answerback delay is currently only 3 seconds.
.REVISIONS              9/20/90
        Add wy50
.REVISION               9/24/90
        Added more terminals from similar Usenet program
.REVISION               2/15/91
        Strip 8th bit on characters returned from terminal
.EXAMPLE
        if [ "`tty`" != "/dev/console" ] && [ "`tty|cut -c1-7`" != "/dev/vt" ]
        then
                TERM=`/u/geterm`
                if [ "$TERM" = "vt102" ]
                then
                        TERM=vt220
                fi

                if [ "x$TERM" = "x" ]
                then
                        TERM=nansipc
                else
                        if [ "$TERM" = "vt220" ]
                        then
                                stty erase \^?
                        fi
                fi
        fi
        export TERM
        echo $LOGNAME has a `tput longname`
.ENDOC                          END DOCUMENTATION
 *****************************************************************************
#endif  /* DOCUMENTATION */

#include <stdio.h>
#include <ctype.h>
#include <termio.h>
#include <setjmp.h>
#include <signal.h>
#ifndef unix
#include <stdlib.h>
#endif
#ifndef VMS
#include <fcntl.h>
#endif

#define SEC_WAIT        3               /* no. seconds to wait for response */

static char *escseq[] =
{
        "\033i0",
        "\033 ",                                /* Wyse 50 */
        "\033[c",
        "\033Z",
        NULL
};

static struct
{
    char    ansseq [32],            /* answerback response */
    termname [32];  /* terminal name in terminfo */
} termtbl [] =
{
    {"\033[?65;","vt520"},
    {"\033[?64;","vt420"},
    {"\033[?63;","vt320"},
    {"\033[?62;","vt220"},
    {"\033[?1;","vt100"},
    {"\033[?6","vt102"},
    {"\033iB0","z29"},
    {"\033K","vt52"},
    {"50","wy50"},
    {"60","wy60"},
    {"30","wy30"},
    {"/K","h19"},                                                                           /* Zenith z19 */
    {"\033[?1;0c"},{"vt100"},                                               /* Base vt100 */
    {"\033[?1;1c"},{"vt100"},                                               /* vt100 with STP */
    {"\033[?1;2c"},{"vt100"},                                               /* ANSI/VT100 Clone */
    {"\033[?1;3c"},{"vt100"},                                               /* vt100 with AVO and STP */
    {"\033[?1;4c"},{"vt100"},                                               /* vt100 with GPO */
    {"\033[?1;5c"},{"vt100"},                                               /* vt100 with GPO and STP */
    {"\033[?1;6c"},{"vt100"},                                               /* vt100 with GPO and AVO */
    {"\033[?1;7c"},{"vt100"},                                               /* vt100 with GPO, STP, and AVO */
    {"\033[?6c"},{"vt102"},                                                 /* vt102 or MS-Kermit */
    {"\033[?8c"},{"vt100"},                                                 /* TeleVideo 970 */
    {"\033[0n"},{"vt100"},                                                  /* AT&T Unix PC 7300 */
    {"\033[?l;0c"},{"vt100"},                                               /* AT&T Unix PC 7300 */
    {"\033[?12c"},{"vt100"},                                                /* Concept from Pro 350/UNIX */
    {"\033[?;c"},{"vt100"},                                                 /* Concept From Pro 350/UNIX */
    {"\033[=1;1c"},{"avt-4p-s"},                                    /* Concept with 4 pages memory */
    {"\033[=1;2c"},{"avt-8p-s"},                                    /* Concept with 8 pages memory */
    {"\033/Z"},{"vt52"},                                                            /* Generic vt52 */
    {"\033[?10c"},{"la120"},                                                /* DEC Writer III */
    {"\033[?1;11c"},{"cit101e"},                                    /* CIE CIT-101 Enhanced w/Graphics */
    {"\033[?12;7;0;102c"},{"vt125"},                                /* DEC Pro 350 in vt125 mode */
    {"\033[?62;1;2;6;7;8;9c"},{"vt220"},            /* DEC VT220 */
    {"\033[?62;1;4;6;7;8;9;15c"},{"vt200-sb"},/* Microvax II VMS */
    {"\033[62;1;2;6;8c"},{"f220"},                          /* Freedom 220 DEC clone */
    {"\033[?63;1;2;6;7;8c"},{"tvi9220"},            /* TeleVideo 9220 */
    {NULL,NULL}
};	

int     Odev;
static void con__tim();
void strascii();
void termsetr();

/*****************************************************************************
        Main entry
*****************************************************************************/
main(argc,argv)
    int argc;
    char *argv[];
{
    char    buff [256];
    int     u,
    ix = 0,
    iy;

    if ((Odev = open("/dev/tty",O_RDWR | O_NDELAY)) != EOF)
    {
	while (escseq[ix])
	{
	    if (write(Odev,escseq[ix],strlen(escseq[ix])) == -1)
		exit(1);

	    coninstr(buff,SEC_WAIT,0);
	    strascii(buff);

	    if (*buff)
	    {
#ifdef TESTING
		u = 0;
		while (buff[u])
		    printf("%02X ",buff[u++]);
		line(1);

#endif
		if (*buff)
		{
		    iy = 0;
		    while (termtbl[iy].ansseq)
		    {
			if (strncmp(termtbl[iy].ansseq,buff,
				     strlen(termtbl[iy].ansseq)) == 0)
			{
			    puts(termtbl[iy].termname);
			    exit(0);
			}
			++iy;
		    }
		}
	    }
	    ++ix;
	}
	close(Odev);
    }
#ifdef TESTING
    puts("not found");
#endif

    puts("");
    exit(1);
}   	    /* end of main */


/*****************************************************************************
        coninstr
*****************************************************************************/
coninstr(str,timo,echoflg)
    char    *str;   /* (w) the input string */
    int     timo,   /* (r) timeout value in seconds or 0 */
    echoflg; /* (r) TRUE to echo input */
{
    int n = 0;
    register ix;

    termsetr(0);
    signal(SIGALRM,con__tim);                                 /* Timed read, so set up timer */
    alarm(timo);
    ix = 0;

    while ((n = read(0, &str[ix], 1)) > 0)
    {
	if (echoflg)
	    write (1,&str[ix],1);
	if (str[ix] == '\r')
	    break;
	++ix;
    }

    alarm(0);                                                                       /* Stop timing, we got our character */
    signal(SIGALRM,SIG_DFL);
    if (echoflg)
	putchar('\n');
    termsetr(1);
    str[ix] = '\0';

    if (n < 0)
	return(-1);
    return 0;
}       /* end coninstr */

/*****************************************************************************
        com__tim
*****************************************************************************/
static void con__tim()
{
    return;
} /* end com__tim */


/*****************************************************************************
        Strascii
*****************************************************************************/
void strascii(p)
    register unsigned char *p;              /* (r/w) the string to convert */
{

    while (*p)
    {
	*p &= 0x7F;
	++p;
    }
                
    return;
}       /* end strascii */


/*****************************************************************************
        termsetr
*****************************************************************************/
void termsetr(func)
    int     func;   /* (r) 0=raw, 1=cooked */
{
    static struct termio
	oldmode;
    struct termio
	newmode;

    if (func)
	ioctl(0,TCSETA,&oldmode);                 /* reset original mode */
    else
    {
	ioctl(0,TCGETA,&oldmode);                       /* save mode */
	memcpy(&newmode,&oldmode,sizeof(struct termio));
	newmode.c_iflag |= (BRKINT|IGNPAR);
	newmode.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC|IXON);
	newmode.c_lflag &= ~(ISIG|ICANON|ECHO);
	newmode.c_oflag &= ~(ONLCR|OCRNL|ONLRET);
	newmode.c_cc[4] = 1;
	newmode.c_cc[5] = 1;
	ioctl(0,TCSETA,&newmode);                 /* set raw mode */
    }

    return;

}       /* end termsetr */

/*****************************************************************************
        End geterm.c
*****************************************************************************/

