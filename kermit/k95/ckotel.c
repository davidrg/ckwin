/* C K O T E L  --  Kermit stub to fake IBM Telnet and IBM TelnetPM */

/*
  Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
            Columbia University Academic Information Systems, New York City.
          Jeffrey E Altman <jaltman@secure-endpoints.com>
            Secure Endpoints Inc., New York City

  Copyright (C) 1996,2004 Trustees of Columbia University in the City of New
  York.
*/

/*
telnet.exe -?
Telnet - 08:32:06 on Sep 27 1994.

Usage: telnet [-c codepage] [-d filename] [-e envlist]
              [-f config]   [-h height]   [-k keyboard]
              [-l filename] [-o printer]  [-p port]
              [-t termtype] [-u color]    [-w width]    [hostname]

Option:
  -?            display this help message
  -c codepage   specify code page translation table
  -d filename   specify debugging filename
  -e envlist    specify environment variables
  -f config     specify configuration filename in <ETC> directory
  -h height     specify screen height
  -k keyboard   specify keyboard type
  -l filename   specify logging filename
  -o printer    specify local printer port name
  -p port       specify remote port number
  -t termtype   specify terminal emulator type
  -u color      specify color for underline
  -w width      specify screen width

The goal of this program is to simulate this interface and construct a
proper command line for C-Kermit.  Then call C-Kermit in the place of
telnet.exe or telnetpm.exe and perform the same functionality.

According to <sad@utl.edu> IBM changed -c to -cp and added -N to ignore 8th bit
(in other words, apply parity).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* codepage = NULL,
    * debuglog = NULL,
    * envlist  = NULL,
    * config   = NULL,
    * height   = NULL,
    * keyboard = NULL,
    * sessionlog = NULL,
    * printerfile = NULL,
    * tn_port = NULL,
    * termtype = NULL,
    * underline = NULL,
    * width = NULL,
    * hostname = NULL ;
char ckermitcmd[4096] ;

void
printhelp( char * progname )
{
printf("%s -? - Version 1.0a\n",progname);
printf("OS/2 C-Kermit Telnet Stub - allows calls to IBM Telnet or IBM TelnetPM\n");
printf("                            to be redirected to OS/2 C-Kermit.\n");
printf("Copyright (C) 1996, Trustees of Columbia University\n\n");
printf("Usage: telnet [-c codepage] [-d filename] [-e envlist]\n");
printf("              [-f config]   [-h height]   [-k keyboard]\n");
printf("              [-l filename] [-n]          [-o printer] [-p port]\n");
printf("              [-t termtype] [-u color]    [-w width]   [hostname]\n");
printf("Option:\n");
printf("  -?            display this help message\n");
printf("  -c codepage   specify code page translation table\n");
printf("  -d filename   specify debugging filename\n");
printf("  -e envlist    specify environment variables\n");
printf("  -f config     specify configuration filename in <ETC> directory\n");
printf("  -h height     specify screen height\n");
printf("  -k keyboard   specify keyboard type\n");
printf("  -l filename   specify logging filename\n");
printf("  -n            ignore 8th bit\n");
printf("  -o printer    specify local printer port name\n");
printf("  -p port       specify remote port number\n");
printf("  -t termtype   specify terminal emulator type\n");
printf("  -u color      specify color for underline\n");
printf("  -w width      specify screen width\n");
}

void
constructcmdline( char * exename )
{
    char * s = ckermitcmd ;
    *s = '\0' ;
    strcat( s, exename ) ;
    strcat( s, " -C \"set terminal code-page 850" );

    if ( debuglog ) {
        if (*s!='\0')
            strcat( s, ", " );
        strcat( s, "log debug " );
        strcat( s, debuglog );
        }

    if ( sessionlog ) {
        if (*s!='\0')
            strcat( s, ", " );
        strcat( s, "log session " );
        strcat( s, sessionlog );
        }

    if ( codepage ) {
        if (*s!='\0')
            strcat( s, ", " );
        strcat( s, "set terminal character-set " );
        if ( !strcmp( strlwr(codepage), "tcpdecmu" ) )
            strcat( s, "dec-multinational" ) ;
        else if ( !strcmp( codepage, "tcp8859" ) )
            strcat( s, "latin-1" ) ;
        else
            strcat( s, "cp850" ) ;
        }

    if ( height ) {
        if (*s!='\0')
            strcat( s, ", " );
        strcat( s, "set terminal height " );
        strcat( s, height );
        }

    if ( width ) {
        if (*s!='\0')
            strcat( s, ", " );
        strcat( s, "set terminal width " );
        strcat( s, width );
        }

    if ( printerfile ) {
        if (*s!='\0')
            strcat( s, ", " );
        strcat( s, "set printer " );
        strcat( s, printerfile );
        }

    if ( termtype ) {
        if (*s!='\0')
            strcat( s, ", " );
        strcat( s, "set terminal type " );
        strcat( s, termtype );
        }

    if (*s!='\0')
        strcat( s, ", " );
    strcat( s, "set exit on-disconnect on" ) ;

    if ( hostname ) {
        if (*s!='\0')
            strcat( s, ", " );
        strcat( s, "telnet " );
        strcat( s, hostname );
        if ( tn_port ) {
            strcat( s, " " ) ;
            strcat( s, tn_port ) ;
        }
        strcat( s, ", if failure exit" );
    }

    strcat( s, "\"" );
};

int
parsecmdline( int argc, char ** argv )
{
    int n = 0 ;
    while ( ++n < argc ) {
        if (argv[n][0] != '-' && argv[n][0] != '/') {
            /* then we have found the host name */
            hostname = argv[n] ;
            continue ;
            }

        switch ( argv[n][1] ) {
            case 'S':
            case 's':
                /* don't know what 's' is, but it is generated
                    by the template, it doesn't have a parameter */
                break;

            case 'C':
            case 'c':
                codepage = argv[++n] ;
                break;

            case 'D':
            case 'd':
                debuglog = argv[++n] ;
                break;

            case 'E':
            case 'e':
                envlist = argv[++n] ;
                break;

            case 'F':
            case 'f':
                config = argv[++n] ;
                break;

            case 'H':
            case 'h':
                height = argv[++n] ;
                break;

            case 'K':
            case 'k':
                keyboard = argv[++n] ;
                break;

            case 'L':
            case 'l':
                sessionlog = argv[++n] ;
                break;

            case 'O':
            case 'o':
                printerfile = argv[++n] ;
                break;

            case 'P':
            case 'p':
                tn_port = argv[++n] ;
                break;

            case 'T':
            case 't':
                termtype = argv[++n] ;
                break;

            case 'U':
            case 'u':
                underline = argv[++n] ;
                break;

            case 'W':
            case 'w':
                width = argv[++n] ;
                break;

            case 'N':
            case 'n':
                /* ignore */
                break;

            case '?':
                printhelp( argv[0] ) ;
                return 1 ;
                break;

            default:
                printf("ERROR: Illegal flag \"-%c\"\n", argv[n][1] );
                {
                int m = 0 ;
                for ( m = 0 ; m < argc ; m++ )
                    printf( "%s ", argv[m] ) ;
                printf("\n");
                }
                printhelp( argv[0] ) ;
#ifndef NT
                DosSleep( 10000 ) ;
#endif
                return 1 ;
            }
        }
    return 0;
};

int
main( int argc, char**argv )
{
    int rc = 0;
    if ( parsecmdline( argc, argv ) )
        return 1 ;

    constructcmdline("k2.exe") ;
    rc = system( ckermitcmd ) ;
    if ( rc == -1 ) {
        constructcmdline("ckermit.exe");
        rc = system( ckermitcmd ) ;
    }
    return rc;
} ;
