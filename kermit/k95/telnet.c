/* T E L N E T . C  --  Telnet stub for Kermit 95 */
/*
  Authors: Frank da Cruz (fdc@columbia.edu)
           Jeffrey Altman (jaltman@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1995-1996, Trustees of Columbia University in the City of
  New York.  All rights reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

char * tn_port = NULL,
     * hostname = NULL;
char ckermitcmd[1024]="";
char ckermitarg[1024]="";

void
constructcmdline( char * telnet ) {
    char * s = ckermitcmd ;
    int n = 0 ;

    strcpy(s,telnet);
    s = ckermitcmd + strlen(telnet) + 1;
    while (s != (ckermitcmd) && *s != '\\' && *s != '/')
        s--;
    if (s != (ckermitcmd))
        s++;
    strcpy( s, "k95.exe" );

    if (hostname) {
        s = ckermitarg;
        strcpy(s,"\"telnet ");
        strcat(s,hostname);
        n = strlen(ckermitarg) - 1;
        while (n >= 0 && ckermitarg[n] == '/')
            ckermitarg[n--] = (char) 0;

        if (tn_port) {
            strcat(s," ");
            strcat(s,tn_port);
            n = strlen(ckermitarg) - 1;
            while (n >= 0 && ckermitarg[n] == '/')
              ckermitarg[n--] = (char) 0;
        }
        strcat(s,"\"");
    }
}

int
parsecmdline( int argc, char ** argv ) {
    if (argc > 1)
      hostname = argv[1];
    if (argc > 2)
      tn_port = argv[2] ;
    return 0;
}

int
main( int argc, char **argv ) {
    if ( parsecmdline( argc, argv ) )
        return 1 ;
    constructcmdline(argv[0]) ;
    if ( ckermitarg[0] )
        return spawnl( _P_WAIT, ckermitcmd, "k95.exe", "-C",ckermitarg, NULL ) ;
    else
        return spawnl( _P_WAIT, ckermitcmd, "k95.exe", NULL);
}
