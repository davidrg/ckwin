
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int printon = 0, lstclosed = 1 ;
FILE * lst = NULL ;
char * printfile = NULL ;

void
printeron() {
    if (printon)			/* It's already on. */
      return;
    if (lstclosed || !lst) {		/* Open printer device */
        if (!printfile)		/* If printer not redirected */
          lst = fopen("prn", "w");	/* open the PRN device */
        else /* otherwise */
          lst = fopen(printfile, "ab"); /* open the file in append mode. */
    }
    if (lst) {				/* Open OK? */
        lstclosed = 0;		/* So not closed */
        printon = 1;			/* and printer is on. */
    }
}

void
printeroff() {				/* Turn off printer */
    if (!printon)			/* It's already off. */
      return;
    if (lst && !lstclosed)
    {
        fclose(lst);
    }
    lstclosed = 1;
    printon = 0;
}


int main( int argc, char * argv[] ) 
{
    if ( argc > 2 )
    {
        printf( "Usage: %s [<printer_device>]\n",argv[0] ) ;
        return(1);
    }

    if ( argc == 2 )
      printfile = argv[1] ;

    printf( "Opening Printer %s\n", printfile ? printfile : "prn") ;
    printeron();
    if ( lst )
    {
        int i,j,k=1;
        printf("Printer Open - Testing\n");
        for ( i=1; i<50 ; i++ )
        {
            for ( j=1 ; j<80 ; j++ )
              fprintf(lst,"%d",k%10);
            fprintf(lst,"\n");
        }
        printeroff();
        printf("Printer Closed\n");
    }
    else
    {
        printf("Unable to Open Printer: %s\n", strerror(errno));
        return(2);
    }
    return(0);
}
