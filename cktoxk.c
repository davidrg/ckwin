/*  cktoxk.c - Copy C-Kermit files ck*.* to xk*.*  */
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
	if ((name[0] == 'c') && (name[1] == 'k')) {
	    *name = 'x';
	    copyfile(argv[i],name);	/* All OK, copy the file. */
	}
    }
    if (!access("Makefile",0)) {	/* Handle the Makefile specially. */
	copyfile("Makefile","xkuker.mak");
    }  else {
	fprintf(stderr,"\nError: Makefile missing!\n");
	exit(1);
    }
    printf("\nDone.  Now you have xk*.* ready for distribution.\n");
    exit(0);
}
  
copyfile(n1,n2) char *n1, *n2; {	/* Function to copy a file.  */
    int a;
    sprintf(cmd,"cp %s %s",n1,n2);	/* Construct command. */
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
