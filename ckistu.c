/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1986 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors.                                                    */
/* | o  | ||    Dave Baker     Ed Burnette  Stan Chow    Jay Denebeim        */
/* |  . |//     Gordon Keener  Jack Rouse   John Toebes  Doug Walker         */
/* ======          BBS:(919)-471-6436      VOICE:(919)-469-4210              */ 
/*                                                                           */
/* Contributed to Columbia University for inclusion in C-Kermit.             */
/* Permission is granted to any individual or institution to use, copy, or   */
/* redistribute this software so long as it is not sold for profit, provided */
/* this copyright notice is retained.                                        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*  C K I S T U  --  Stubs for functions not yet implemented on Amiga */
 
/*
 Author: Jack Rouse, The Software Distillery
*/

#include "ckcdeb.h"
#include <stdio.h>
#include <ctype.h>
#include "ckcker.h"
#include "ckucmd.h"

char *dialv = "Dial Command unimplemented";
struct keytab mdmtab[] = {
	"direct", 	0, 	0,	/* no modem control */
	"generic", 	1, 	0	/* use 7 wire modem control */
};

int nmdm = sizeof(mdmtab) / sizeof(struct keytab);
 
ckdial()
{
	printf("Sorry, DIAL command not implemented yet.\n");
	return(-2);
}

char *loginv = "Script Command unimplemented";
login()
{
	printf("Sorry, SCRIPT command not implemented yet\n");
	return(-2);
}
