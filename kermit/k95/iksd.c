#include <stdio.h>

#ifdef NT
#include <windows.h>
#include <winsock.h>
#define strdup _strdup
#define ltoa   _ltoa
#endif
#define CONFIG_FILE "iksd.cfg"

#define bzero(x,y) memset(x,0,y)
#define BSDSELECT

#define MAXPORTS 32
struct PORT 
{
    short id ;
    int   lsocket ;
    int   asocket ;
    char  * k95cmd ;
    int   showcmd;
} ports[MAXPORTS] ;
int portcount = 0 ;

int
ParseCmdLine( int argc, char * argv[] )
{
    int len = 0, i ;
    char * p = NULL ;
    static struct servent *service, servrec;
    int arg = 1;

    /* Set up some defaults */
    ports[portcount].showcmd = SW_SHOWNORMAL;
    service = getservbyname("kermit", "tcp");

    /* Allocate the max needed memory */
    for ( i = 1 ; i < argc ; i++ )
        len += strlen( argv[i] ) + 1 ;
   
    ports[portcount].k95cmd = (char *) malloc( len+1 ) ;
    if ( !ports[portcount].k95cmd )
    {
        fprintf( stderr, "memory allocation error\n" ) ;
        exit(1) ;
    }
    ports[portcount].k95cmd[0] = '\0' ;

    /* Process each command line option */
    for ( i=1 ; i<argc ; i++ )
    {
        if ( argv[arg][0] == '-' && argv[arg][1] == '-' ) {
            if ( !_stricmp("--maximized",argv[arg]) ) {
                ports[portcount].showcmd = SW_SHOWMAXIMIZED;
                continue;
            }
            else if ( !_stricmp("--minimized",argv[arg]) ) {
                ports[portcount].showcmd = SW_SHOWMINIMIZED;
                continue;
            }
            else if ( !_stricmp("--normal",argv[arg]) ) {
                ports[portcount].showcmd = SW_NORMAL;
                continue;
            }
            else if ( !_stricmp("--minnoactive",argv[arg]) ) {
                ports[portcount].showcmd = SW_SHOWMINNOACTIVE;
                continue;
            }
            else if ( !_strnicmp("--port:",argv[arg],7) ) {
                p = &argv[arg][7];
                if (isdigit(*p)) {			/* Use socket number without lookup */
                    service = &servrec;
                    service->s_port = htons((unsigned short)atoi(p));
                } else {				/* Otherwise lookup the service name */
                    service = getservbyname(p, "tcp");
                }
                continue;
            }
        }

        if ( ports[portcount].k95cmd[0] )
            strcat( ports[portcount].k95cmd, " ");
        strcat( ports[portcount].k95cmd, argv[i] );
    }

    if ( service )
        ports[portcount].id = service->s_port;
    else
        ports[portcount].id = htons( 1649 );

    portcount++ ;
    return portcount ;
}

HANDLE
StartKermit( int socket, char * cmdline, int ShowCmd ) 
{
#ifdef NT
   PROCESS_INFORMATION StartKermitProcessInfo ;
   OSVERSIONINFO osverinfo ;
   STARTUPINFO si ;
   HANDLE sockdup = INVALID_HANDLE_VALUE ;
   static char buf[512] ;

   osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
   GetVersionEx( &osverinfo ) ;
    
   memset( &si, 0, sizeof(STARTUPINFO) ) ;
   si.cb = sizeof(STARTUPINFO);
   si.dwFlags = STARTF_USESHOWWINDOW;
   si.wShowWindow = ShowCmd;

   if (!DuplicateHandle( GetCurrentProcess(), (HANDLE) socket, 
                    GetCurrentProcess(), &sockdup,
                    DUPLICATE_SAME_ACCESS, TRUE, 
                    DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ))
   {
      closesocket( (int) socket ) ;
      fprintf( stderr, "\nINTERNET KERMIT SERVICE DAEMON FATAL ERROR:\n" );
      fprintf( stderr, " You are using a WinSOCK which does not allow socket handles\n");
      fprintf( stderr, " to be duplicated or shared with child processes.\n\n");
      fprintf( stderr, " If you are attempting to start Kermit 95 Host Mode,\n");
      fprintf( stderr, " please change SESSIONS to 1 in your configuration.\n");
      fprintf( stderr, "\n Press ENTER to exit...\n");
      fscanf( stdin, "" ) ; 
      exit( 2 ) ;
   }


#ifdef DEBUG
    strcpy( buf, "msdev.exe cknker.exe -# 132 -A " ) ;
    ltoa( (LONG) sockdup, buf+strlen(buf), 10 ) ;
    strcat( buf, " " );
    strcat( buf, cmdline ) ;

   printf("Executing: %s\n",buf ) ;
   if (CreateProcess(
                       (LPSTR)NULL,          /* start K-95  */
                       (LPSTR)buf, /* give it the file */
                       (LPSECURITY_ATTRIBUTES)NULL, 
                       (LPSECURITY_ATTRIBUTES)NULL, 
                        TRUE,                      /* fix if necessary */
                       (DWORD) CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,    
                       (LPVOID)NULL,                /* fix if necessary */
                       (LPSTR)NULL,               /* Current directory */
                       &si,                     /* Startup info, fix */
                       &StartKermitProcessInfo  /* Process info */
                      )) 
    {
        closesocket( (int) sockdup ) ;
        return (StartKermitProcessInfo.hProcess);
    } 
#endif /* DEBUG */

#ifdef DEBUG
    strcpy( buf, "msdev.exe iksdnt.exe -# 132 -A " ) ;
#else
    strcpy( buf, "iksdnt.exe -# 132 -A " ) ;
#endif /* DEBUG */
    ltoa( (LONG) sockdup, buf+strlen(buf), 10 ) ;
    strcat( buf, " " );
    strcat( buf, cmdline ) ;

   printf("Executing: %s\n",buf ) ;
   if (CreateProcess(
                       (LPSTR)NULL,          /* start K-95  */
                       (LPSTR)buf, /* give it the file */
                       (LPSECURITY_ATTRIBUTES)NULL, 
                       (LPSECURITY_ATTRIBUTES)NULL, 
                        TRUE,                      /* fix if necessary */
                       (DWORD) CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,    
                       (LPVOID)NULL,                /* fix if necessary */
                       (LPSTR)NULL,               /* Current directory */
                       &si,                     /* Startup info, fix */
                       &StartKermitProcessInfo  /* Process info */
                      )) 
    {
        closesocket( (int) sockdup ) ;
        return (StartKermitProcessInfo.hProcess);
    } 

#ifdef DEBUG
    strcpy( buf, "msdev.exe k95.exe -# 132 -A " ) ;
#else
    strcpy( buf, "k95.exe -# 132 -A " ) ;
#endif /* DEBUG */
    ltoa( (LONG) sockdup, buf+strlen(buf), 10 ) ;
    strcat( buf, " " );
    strcat( buf, cmdline ) ;

   printf("Executing: %s\n",buf ) ;
   if (CreateProcess(
                       (LPSTR)NULL,          /* start K-95  */
                       (LPSTR)buf, /* give it the file */
                       (LPSECURITY_ATTRIBUTES)NULL, 
                       (LPSECURITY_ATTRIBUTES)NULL, 
                        TRUE,                      /* fix if necessary */
                       (DWORD) CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,    
                       (LPVOID)NULL,                /* fix if necessary */
                       (LPSTR)NULL,               /* Current directory */
                       &si,                     /* Startup info, fix */
                       &StartKermitProcessInfo  /* Process info */
                      )) 
    {
        closesocket( (int) sockdup ) ;
        return (StartKermitProcessInfo.hProcess);
    } 

    printf("CreateProcess() failed gle=%ul\n",GetLastError());
    closesocket( (int) sockdup ) ;
    return(FALSE);
#else /* NT */
    Not built for OS/2 yet.
#endif /* NT */
}

int
main( int argc, char * argv[] ) {
    char *p=NULL, * dbdir=NULL, dbfile[256];
    int i, x;
    int on = 1, rc = 0;
    int ready_to_accept = 0 ;
    static struct servent *service, servrec;
    static struct hostent *host;
    static struct sockaddr_in saddr;
    static int saddrlen ;
#ifdef BSDSELECT
    fd_set rfds;
    struct timeval tv;
#endif /* BSDSELECT */
    int tcpsrv_fd = -1, ttyfd = -1 ;
#ifdef NT
    WSADATA data ;
    OSVERSIONINFO osverinfo ;
    HANDLE hProcess;

    printf("Internet Kermit Service Daemon\n");
    rc = WSAStartup( MAKEWORD( 2, 0 ), &data ) ;
    if ( rc == WSAVERNOTSUPPORTED )
    {
      WSACleanup() ;
      rc = WSAStartup( MAKEWORD( 1, 1 ), &data ) ;
   }
#else /* NT */

#endif /* NT */

    ParseCmdLine( argc, argv );
    if ( !portcount )
    {
#ifdef DEBUG
        ports[portcount].showcmd = SW_NORMAL;
#else
        ports[portcount].showcmd = SW_SHOWMINNOACTIVE;
#endif
        ports[portcount].k95cmd = (char *) malloc( 1 ) ;
        ports[portcount].k95cmd[0] = '\0' ;
        ports[portcount].id = htons( 1649 ) ;
        portcount++;
    }

#ifdef NT
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;

    if (osverinfo.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
        dbdir = getenv("SystemRoot");
    } else {
        dbdir = getenv("winbootdir");
        if (!dbdir)  
            dbdir = getenv("windir");
    }
    if (!dbdir) 
        dbdir = "C:/";
#else /* NT */
    dbdir = "C:/";
#endif /* NT */
    sprintf(dbfile,"%s\\iksd.lck",dbdir);
    unlink(dbfile);
    sprintf(dbfile,"%s\\iksd.db",dbdir);
    unlink(dbfile);

   for ( i=0; i<portcount; i++)
   {
      /* Set up socket structure and get host address */

      bzero((char *)&saddr, sizeof(saddr));
      saddr.sin_family = AF_INET ;
      saddr.sin_addr.s_addr = INADDR_ANY ;

      /* Get a file descriptor for the connection. */

      saddr.sin_port = ports[i].id;

      if ((ports[i].lsocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
         perror("TCP socket error");
         exit (3);
      }
      errno = 0;

      /* Specify the Port may be reused */
      setsockopt(ports[i].lsocket, SOL_SOCKET, SO_REUSEADDR,(char *) &on, sizeof on);

      /* Now bind to the socket */
#ifdef DEBUG
      printf("Binding socket to port %d ... ",
              ntohs((unsigned short)ports[i].id)) ;
#endif
      if (bind(ports[i].lsocket, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) 
      {
         i = errno;			/* save error code */
         closesocket(ports[i].lsocket) ;
         ports[i].lsocket = -1 ;
         ports[i].asocket = -1;
         errno = i;			/* and report this error */
         fprintf(stderr,"tcpsrv_open bind errno","",errno);
         exit(4);
      }
#ifdef DEBUG
      printf("Listening ...\n");
#endif
      if (listen(ports[i].lsocket, 15) < 0)
      {
         i = errno;			/* save error code */
         closesocket(ports[i].lsocket) ;
         ports[i].lsocket = -1 ;
         ports[i].asocket = -1;
         errno = i;			/* and report this error */
         fprintf(stderr,"tcpsrv_open listen errno","",errno);
         exit(5);
      }
   }
              
   printf("Servicing ports:\n");
   for ( i=0;i<portcount ;i++ )
   {
      printf("  %d\n", ntohs((unsigned short)ports[i].id) ) ;
   }

   saddrlen = sizeof(saddr) ;

#ifdef BSDSELECT
   tv.tv_sec  = 1L;
   tv.tv_usec = 0L;
#endif /* BSDSELECT */

   for ( ;; )
   {
      while ( !ready_to_accept )
      {
#ifdef BSDSELECT
         FD_ZERO(&rfds);
         for ( i=0; i<portcount;i++ )
            FD_SET(ports[i].lsocket, &rfds);
         if (select(FD_SETSIZE, &rfds, NULL, NULL, &tv ) > 0)
         {
            for ( i=0; i<portcount ; i++ )
            {
               if ( ready_to_accept = FD_ISSET(ports[i].lsocket, &rfds) )
                  break;
            }
         }
#else /* BSDSELECT */
#ifdef IBMSELECT
???      /* in order to make this work, we need to create an array of socket values */
         ready_to_accept = (( select(&tcpsrv_fd, 1, 0, 0,1 )) == 1) ;
#endif /* IBMSELECT */
#endif /* BSDSELECT */
      }
     
      if ( ready_to_accept )
      {
         if ((ports[i].asocket = accept(ports[i].lsocket, (struct sockaddr *)&saddr,&saddrlen)) < 0)
         {
            i = errno;			/* save error code */
            closesocket(ports[i].lsocket) ;
            ports[i].asocket = -1;
            ports[i].lsocket = -1 ;
            errno = i;			/* and report this error */
            fprintf(stderr,"tcpsrv_open accept errno","",errno);
            exit(6);
         }

         setsockopt(ports[i].asocket, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);

         if ((host = gethostbyaddr((char *)&saddr.sin_addr,4,PF_INET)) != NULL) 
         {
            printf("%s connected on port %d\n",host->h_name,ntohs(ports[i].id)) ;
         }

         /* Now start subprocess */
          if ( ports[i].asocket,ports[i].k95cmd )
              printf("Starting IKSD with socket %d and command %s\n",ports[i].asocket,ports[i].k95cmd);
          else 
              printf("Starting IKSD with socket %d\n");
         hProcess = StartKermit(ports[i].asocket,ports[i].k95cmd, ports[i].showcmd) ;
	 if ( hProcess != INVALID_HANDLE_VALUE )
	     CloseHandle(hProcess);
         closesocket(ports[i].asocket);
         continue;				/* Go get the next one */
      }
      else
      {
         i = errno;			/* save error code */
         closesocket(ports[i].lsocket) ;
         ports[i].lsocket = -1;
         ports[i].asocket = -1;
         errno = i;			/* and report this error */
         fprintf(stderr,"tcpsrv_open accept errno","",errno);
         exit(7);
      }
   }

#ifdef NT
   WSACleanup() ;
#else

#endif /* NT */
}
