/*  xktock.c - Copy experimental C-Kermit files xk*.* to ck*.*  */
/*  F. da Cruz, CUCCA, Aug 87  */

#include <stdio.h>

char name[500], cmd[500], msg[100];	/* Name, command, & message buffers */

main(argc,argv) int argc; char **argv; {
    int i;

    if (argc < 2) usage();		/* Need files on command line. */

/* For each file, if name starts with "xk" change first letter to "c". */
/* If file of that name already exists, exit with a warning message.   */

    for (i = 1; i < argc; i++) {	/* Loop for each file. */
	strcpy(name,argv[i]);
	if ((name[0] == 'x') && (name[1] == 'k')) {
	    *name = 'c';
	    if (!access(name,0)) {
		fprintf(stderr,"Error: %s already exists\n",name);
		printf(stderr,
		  "Please dispose of your ck*.* files and run this again.\n");
		usage();
	    }
	    movefile(argv[i],name);	/* All OK, rename the file. */
	}
    }
    if (!access("ckuker.mak",0)) {	/* Handle the Makefile specially. */
	movefile("ckuker.mak","Makefile");
    }  else {
	fprintf(stderr,"\nError: Makefile (ckuker.mak) missing!\n");
	exit(1);
    }
    printf("\nDone.  Now you can make C-Kermit for your system.\n");
    printf("See the Makefile for instructions.\n");
    exit(0);
}
  
movefile(n1,n2) char *n1, *n2; {	/* Function to rename a file.  */
    int a;
    sprintf(cmd,"mv %s %s",n1,n2);	/* Construct command */
    printf(cmd);			/* Echo it. */
    a = system(cmd);			/* Run it.  */
    if ((a == 127) || (a < 0)) {	/* Handle errors. */
	sprintf(msg,"\n%s failed (%d)",a,cmd);
	perror(msg);
	exit(1);
    } else printf(" (ok)\n");
}

usage() {				/* Usage message & exit */
    fprintf(stderr,"\nUsage: xktock xk*\n\n");
    fprintf(stderr,"This programs renames the experimental C-Kermit files\n");
    fprintf(stderr,"from xk*.* to ck*.* so that you can build them using\n");
    fprintf(stderr,"the Makefile.  Before running this program, put all\n");
    fprintf(stderr,"xk*.* files in a directory by themselves and cd to\n");
    fprintf(stderr,"that directory.\n\n");
    exit(1);
}
