/* ckermit.h -- Symbol and macro definitions for C-Kermit */

#include <stdio.h>
#include <ctype.h>
#include "ckdebu.h"

/* Mnemonics for ASCII characters */

#define SOH	   001	    	/* ASCII Start of header */
#define BEL        007		/* ASCII Bell (Beep) */
#define BS         010		/* ASCII Backspace */
#define CR         015		/* ASCII Carriage Return */
#define XON	   021	    	/* ASCII XON */
#define SP	   040		/* ASCII Space */
#define DEL	   0177		/* ASCII Delete (Rubout) */

/* Kermit parameters and defaults */

#define MAXPACK	   94		/* Maximum packet size */
#define RBUFL	   200 	    	/* Receive buffer length */
#define CTLQ	   '#'		/* Control char prefix I will use */
#define MYEBQ	   '&'		/* 8th-Bit prefix char I will use */
#define MYRPTQ	   '~'		/* Repeat count prefix I will use */

#define MAXTRY	    10	  	/* Times to retry a packet */
#define MYPADN	    0	  	/* How many padding chars I need */
#define MYPADC	    '\0'  	/* Which padding character I need */

#define DMYTIM	    7	  	/* Default timeout interval to use. */
#define URTIME	    10	  	/* Timeout interval to be used on me. */

#define DEFTRN	    0           /* Default line turnaround handshake */
#define DEFPAR	    0           /* Default parity */
#define MYEOL	    CR          /* End-Of-Line character I need on packets. */

#define DRPSIZ	    90	        /* Default incoming packet size. */
#define DSPSIZ	    90	        /* Default outbound packet size. */

#define DDELAY      5		/* Default delay. */
#define DSPEED	    9600 	/* Default line speed. */

/* Files */

#define ZCTERM      0	    	/* Console terminal */
#define ZSTDIO      1		/* Standard input/output */
#define ZIFILE	    2		/* Current input file */
#define ZOFILE      3	    	/* Current output file */
#define ZDFILE      4	    	/* Current debugging log file */
#define ZTFILE      5	    	/* Current transaction log file */
#define ZPFILE      6	    	/* Current packet log file */
#define ZSFILE      7		/* Current session log file */
#define ZNFILS      8	    	/* How many defined file numbers */

/* Macros */

#define tochar(ch)  ((ch) + SP )	/* Number to character */
#define unchar(ch)  ((ch) - SP )	/* Character to number */
#define ctl(ch)     ((ch) ^ 64 )	/* Controllify/Uncontrollify */
#define unpar(ch)   ((ch) & 127)	/* Clear parity bit */
