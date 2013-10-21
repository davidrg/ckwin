#include <stdio.h>

#ifdef NT
#include <windows.h>
#include <winsock.h>
#define strdup _strdup
#define ltoa   _ltoa
#define CONFIG_FILE "k95d.cfg"
#else

#define CONFIG_FILE "k2d.cfg"
#endif


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
    int   rdns;
} ports[MAXPORTS] ;
int portcount = 0 ;

#define MAXCHILDREN 256
struct CHILDREN
{
    HANDLE hProcess;
    int    socket;
} children[MAXCHILDREN];
int childcount = 0;

void
init_children(void)
{       
    int i;
    for ( i=0 ;i<MAXCHILDREN;i++ ) {
        children[i].hProcess = INVALID_HANDLE_VALUE;
        children[i].socket = (int)INVALID_HANDLE_VALUE;
    }   
}

void
check_children(void)
{
    int i,found;
    DWORD dwExit;

    for (i=0, found=0; i<MAXCHILDREN && found<childcount; i++) {
        if ( children[i].hProcess != INVALID_HANDLE_VALUE ) {
            if (GetExitCodeProcess(children[i].hProcess, &dwExit)) {
                if ( dwExit != STILL_ACTIVE ) {
                    closesocket(children[i].socket);
                    CloseHandle(children[i].hProcess);
                    children[i].socket = (int)INVALID_HANDLE_VALUE;
                    children[i].hProcess = INVALID_HANDLE_VALUE;
                    childcount--;       
                    /* Do not increase found if we reduce childcount */
                    continue;
                }
            }
            found++;
        }
    }
}

void
add_child(HANDLE hProcess, int socket)
{
    int i;
    for ( i=0;i<MAXCHILDREN;i++ ) {
        if ( children[i].hProcess == INVALID_HANDLE_VALUE ) {
            children[i].hProcess = hProcess;
            children[i].socket = socket;
            childcount++;
            return;
        }
    }
}

int
ParseCmdLine( int argc, char * argv[] )
{
    int len = 0, i ;
    char * p = NULL ;
    static struct servent *service, servrec;
    int arg = 1;

   if ( argc < 3 )
      return 0;

    ports[portcount].showcmd = SW_SHOWNORMAL;
    ports[portcount].rdns = 1;

    for ( arg = 1; argv[arg][0] == '-' && argv[arg][1] == '-' ; arg++ ) {
        if ( !_stricmp("--maximized",argv[arg]) )
             ports[portcount].showcmd = SW_SHOWMAXIMIZED;
        else if ( !_stricmp("--minimized",argv[arg]) )
             ports[portcount].showcmd = SW_SHOWMINIMIZED;
        else if ( !_stricmp("--normal",argv[arg]) )
             ports[portcount].showcmd = SW_NORMAL;
        else if ( !_stricmp("--minnoactive",argv[arg]) )
             ports[portcount].showcmd = SW_SHOWMINNOACTIVE;
        else if ( !_stricmp("--nodns",argv[arg]) )
             ports[portcount].rdns = 0;
    }

    p = argv[arg];                      /* Was a service requested? */

    if (isdigit(*p)) {                  /* Use socket number without lookup */
      service = &servrec;
      service->s_port = htons((unsigned short)atoi(p));
   } else {                             /* Otherwise lookup the service name */
      service = getservbyname(p, "tcp");
   }

   if ( service )
   {
      ports[portcount].id = service->s_port ;
   }
   else if ( !strcmp( "kermit", p ) )
   {  /* use now assigned Kermit Service Port */
      service = &servrec ;
      service->s_port = htons( 1649 ) ;
   }
   else /* !service */
   {
      fprintf(stderr, "Cannot find port for service %s\n", p);
      exit(2);
   }

    arg++;

   for ( i = arg ; i < argc ; i++ )
   {
      len += strlen( argv[i] ) + 1 ;
   }

   ports[portcount].k95cmd = (char *) malloc( len ) ;
   if ( !ports[portcount].k95cmd )
   {
      fprintf( stderr, "memory allocation error\n" ) ;
      exit(1) ;
   }

   ports[portcount].k95cmd[0] = '\0' ;
   for ( i=arg ; i<argc ; i++ )
   {
      strcat( ports[portcount].k95cmd, argv[i] ) ;
   }
   portcount++ ;
   return portcount ;
}

int
ParseCfgFile( void )
{
   FILE * cfgfd = NULL ;
   static struct servent *service, servrec;
   char cmdbuf[256], *p ;
   int i ;

   p = cmdbuf;

   /* try and open file in local directory */
    cfgfd = fopen( CONFIG_FILE, "r" ) ;
   if ( !cfgfd )
      return 0;

   while ( !feof(cfgfd) && portcount < MAXPORTS )
   {
       ports[portcount].showcmd = SW_SHOWNORMAL;
       ports[portcount].rdns = 1;
       service = NULL ;

      /* read and parse the first white space delimited string */
      switch ( fscanf( cfgfd, "%s ", &cmdbuf ) )
      {
      case EOF:
         fclose( cfgfd ) ;
         return portcount ;

      case 0:
         /* lets assume we should just skip this line */
         fscanf( cfgfd, "%[^\n]\n", cmdbuf ) ;
         break;

      default:
         {
           flag:
             /* look for an optional show flag */
             if ( p[0] == '-' && p[1] == '-' ) {
                 if ( !_stricmp("--maximized",p) )
                     ports[portcount].showcmd = SW_SHOWMAXIMIZED;
                 else if ( !_stricmp("--minimized",p) )
                     ports[portcount].showcmd = SW_SHOWMINIMIZED;
                 else if ( !_stricmp("--normal",p) )
                     ports[portcount].showcmd = SW_NORMAL;
                 else if ( !_stricmp("--minnoactive",p) )
                     ports[portcount].showcmd = SW_SHOWMINNOACTIVE;
                 else if ( !_stricmp("--nodns",p) )
                     ports[portcount].rdns = 0;

                 p = cmdbuf;
                 *p = '\0';

                 /* now read the service */
                 switch ( fscanf( cfgfd, "%s ", &cmdbuf ) )
                 {
                 case EOF:
                     fclose( cfgfd ) ;
                     return portcount ;

                 case 0:
                     /* lets assume we should just skip this line */
                     fscanf( cfgfd, "%[^\n]\n", cmdbuf ) ;
                     break;

                 default:
                     if ( p[0] == '-' && p[1] == '-' )
                         goto flag;
                     break;
                 }

             }

             if (isdigit(*p)) {                 /* Use socket number without lookup */
               service = &servrec;
               service->s_port = htons((unsigned short)atoi(p));
            } else {                            /* Otherwise lookup the service name */
               service = getservbyname(p, "tcp");
            }

            if ( service )
            {
               ports[portcount].id = service->s_port ;
            }
            else if ( !strcmp( "kermit", p ) )
            {  /* use now assigned Kermit Service Port */
               service = &servrec ;
               ports[portcount].id = service->s_port = htons( 1649 ) ;
            }
            else /* ( !service ) */
            {
               if ( cmdbuf[0] != ';' )
                  fprintf( stderr, "WARNING: invalid service or port \"%s\"\n",
                           cmdbuf ) ;
               fscanf( cfgfd, "%[^\n]\n", cmdbuf ) ;
               break;
            }

            for ( i=0 ; i<portcount ; i++ )
            {
               if ( ports[i].id == ports[portcount].id )
               {
                  fprintf( stderr, "WARNING: ignoring duplicate entry for port \"%s\"\n",
                           cmdbuf ) ;
                  fscanf( cfgfd, "%[^\n]\n", cmdbuf ) ;
                  break;
               }
            }

            /* Parse the rest of the line or up to a comment */
            switch ( fscanf( cfgfd, "%[^;\n]", cmdbuf ) )
            {
            case EOF:
            case 0:
               fclose( cfgfd ) ;
               return portcount;

            default:
               ports[portcount].k95cmd = strdup(cmdbuf) ;
               ports[portcount].lsocket = -1 ;
               ports[portcount].asocket = -1 ;
               portcount++ ;
            }
         }
      }
   }
   fclose( cfgfd ) ;
   return portcount ;
}

HANDLE
StartKermit( int socket, char * scriptfile, int ShowCmd, int * psockdup )
{
#ifdef NT
   PROCESS_INFORMATION StartKermitProcessInfo ;
   OSVERSIONINFO osverinfo ;
   STARTUPINFO si ;
   HANDLE sockdup = INVALID_HANDLE_VALUE ;
   static char buf[512] ;

    *psockdup = (int)INVALID_HANDLE_VALUE;

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
      fprintf( stderr, "\nKERMIT DAEMON FATAL ERROR:\n" );
      fprintf( stderr, " You are using a WinSOCK which does not allow socket handles\n");
      fprintf( stderr, " to be duplicated or shared with child processes.\n\n");
      fprintf( stderr, " If you are attempting to start Kermit 95 Host Mode,\n");
      fprintf( stderr, " please change SESSIONS to 1 in your configuration.\n");
      fprintf( stderr, "\n Press ENTER to exit...\n");
      fscanf( stdin, "" ) ;
      exit( 2 ) ;
   }
   strcpy( buf, "k95.exe -j $" ) ;
   ltoa( (LONG) sockdup, buf+strlen(buf), 10 ) ;
   strcat( buf, " -C \"" ) ;
   strcat( buf, scriptfile ) ;
   strcat( buf, "\"" ) ;

   printf("Executing: %s\n",buf ) ;
   if (!CreateProcess(
                       (LPSTR)NULL,          /* start K-95  */
                       (LPSTR)buf, /* give it the file */
                       (LPSECURITY_ATTRIBUTES)NULL, /* fix if necessary */
                       (LPSECURITY_ATTRIBUTES)NULL, /* fix if necessary */
                        TRUE,                      /* fix if necessary */
                       (DWORD) CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
                       (LPVOID)NULL,                /* fix if necessary */
                       (LPSTR)NULL,               /* Current directory */
                       &si,                     /* Startup info, fix */
                       &StartKermitProcessInfo  /* Process info */
                      ))
    {
        CloseHandle(StartKermitProcessInfo.hThread);
        *psockdup = (int)sockdup;
        return (StartKermitProcessInfo.hProcess);
    }
    else
    {
        closesocket( (int) sockdup ) ;
        CloseHandle(StartKermitProcessInfo.hProcess);
        CloseHandle(StartKermitProcessInfo.hThread);
        return(INVALID_HANDLE_VALUE);
    }
#else /* NT */

#endif /* NT */
}

int
main( int argc, char * argv[] ) {
   char *p;
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
    HANDLE hProcess;
    int sockdup = (int)INVALID_HANDLE_VALUE;

   printf("Kermit-95 Daemon\n");
   rc = WSAStartup( MAKEWORD( 2, 0 ), &data ) ;
   if ( rc == WSAVERNOTSUPPORTED )
   {

      WSACleanup() ;
      rc = WSAStartup( MAKEWORD( 1, 1 ), &data ) ;
   }
#else /* NT */

#endif /* NT */

    init_children();

   ParseCmdLine( argc, argv ) ;

   if ( !portcount ) /* Only use k95.cfg if there was no command line */
      ParseCfgFile() ;

   if ( !portcount )
   {
      fprintf( stderr, "Nothing to do." ) ;
      exit(8) ;
   }


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
         i = errno;                     /* save error code */
         closesocket(ports[i].lsocket) ;
         ports[i].lsocket = -1 ;
         ports[i].asocket = -1;
         errno = i;                     /* and report this error */
         fprintf(stderr,"tcpsrv_open bind errno","",errno);
         exit(4);
      }
#ifdef DEBUG
      printf("Listening ...\n");
#endif
      if (listen(ports[i].lsocket, 15) < 0)
      {
         i = errno;                     /* save error code */
         closesocket(ports[i].lsocket) ;
         ports[i].lsocket = -1 ;
         ports[i].asocket = -1;
         errno = i;                     /* and report this error */
         fprintf(stderr,"tcpsrv_open listen errno","",errno);
         exit(5);
      }
   }

   printf("Servicing ports:\n");
   for ( i=0;i<portcount ;i++ )
   {
      printf("  %d\n", ntohs((unsigned short)ports[i].id) ) ;
   }
    printf("Press Ctrl-C to cancel...\n");

   saddrlen = sizeof(saddr) ;

#ifdef BSDSELECT
   tv.tv_sec  = 1L;
   tv.tv_usec = 0L;
#endif /* BSDSELECT */

   for ( ;; )
   {
      while ( !ready_to_accept )
      {
          check_children();

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
            i = errno;                  /* save error code */
            closesocket(ports[i].lsocket) ;
            ports[i].asocket = -1;
            ports[i].lsocket = -1 ;
            errno = i;                  /* and report this error */
            fprintf(stderr,"tcpsrv_open accept errno","",errno);
            exit(6);
         }

         setsockopt(ports[i].asocket, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);

         if (ports[i].rdns &&
             (host = gethostbyaddr((char *)&saddr.sin_addr,4,PF_INET)) != NULL)
         {
            printf("%s [%s] connected on port %d\n",host->h_name,
                    (char *)inet_ntoa(saddr.sin_addr),ntohs(ports[i].id));
         } else {
            printf("%s connected on port %d\n",(char *)inet_ntoa(saddr.sin_addr),
                   ntohs(ports[i].id));
         }

         /* Now start subprocess */
         printf("Starting Kermit with socket %d and command %s\n",ports[i].asocket,ports[i].k95cmd);
          hProcess = StartKermit(ports[i].asocket,ports[i].k95cmd, ports[i].showcmd, &sockdup);
          if ( hProcess != INVALID_HANDLE_VALUE ) 
              add_child(hProcess, sockdup);
          else
              closesocket(ports[i].asocket);      /* we use CLOSE_SOURCE when duplicating */
         continue;                              /* Go get the next one */
      }
      else
      {
         i = errno;                     /* save error code */
         closesocket(ports[i].lsocket) ;
         ports[i].lsocket = -1;
         ports[i].asocket = -1;
         errno = i;                     /* and report this error */
         fprintf(stderr,"tcpsrv_open accept errno","",errno);
         exit(7);
      }
   }

#ifdef NT
   WSACleanup() ;
#else

#endif /* NT */
}
