/* R L O G I N . C  --  Rlogin stub for Kermit 95 */
/*
  Authors: Frank da Cruz (fdc@columbia.edu)
           Jeffrey Altman (jaltman@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1995, 1997, Trustees of Columbia University in the City of
  New York.  All rights reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * tn_port = "login",
     * hostname = NULL,
     * username = NULL;

char ckermitcmd[4096];

void
constructcmdline( char * rlogin ) {
    char * s = ckermitcmd ;
    int n = 0 ;

    ckermitcmd[0] = '"';
    strcpy(&ckermitcmd[1],rlogin);
    s = ckermitcmd + strlen(rlogin) + 1;
    while (s != (ckermitcmd+1) && *s != '\\' && *s != '/')
        s--;
    if (s != (ckermitcmd+1))
        s++;
#ifdef NT
    strcpy( s, "k95.exe\"" );
#else /* NT */
    strcpy( s, "k2.exe\"" ) ;
#endif /* NT */

    if ( username ) {
        strcat( s," -M " ) ;
        strcat( s, username ) ;
    }

    if (hostname) {
        strcat(s," -J ");
        strcat(s,hostname);
        n = strlen(ckermitcmd) - 1;
        while (n >= 0 && ckermitcmd[n] == '/')
            ckermitcmd[n--] = (char) 0;

        strcat(s," ");
        strcat(s,"login");
    }
}

void
usage(void)
{
    fprintf( stderr, "rlogin [-l username] hostname\n\n" ) ;
}

int
parsecmdline( int argc, char ** argv ) {
    int i=1;
    if ( argc == 1 )
    {
        usage() ;
        return -1;
    }

    hostname = argv[argc-1] ;
    argc-- ;

    for ( i=1 ; i<argc ; i++ )
    {
        if ( argv[i][0] == '-' )
        {

            switch ( argv[i][1] ) {
            case 'l':
                i++ ;
                if ( i < argc )
                  username = argv[i] ;
                else
                {
                    usage();
                    return -1;
                }
                break;
            case '8':
                break;
            default:
                usage();
                return -1;
            }
        }
    }
    return 0;
}

int
main( int argc, char **argv ) {
    if ( parsecmdline( argc, argv ) )
        return 1 ;
    constructcmdline(argv[0]) ;
    return system( ckermitcmd ) ;
}
