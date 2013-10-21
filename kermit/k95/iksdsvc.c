/* Windows Internet Kermit Service Daemon (WIKSD):
 * Copyright (C) 1985, 2003, Trustees of Columbia University in the City of New
 * York.  All rights reserved.
 *  
 * PERMISSIONS:
 *  
 *   The WIKSD software may be obtained directly, in binary form only, from
 *   the Kermit Project at Columbia University by any individual for his or
 *   her OWN USE, and by any company or other organization for its own
 *   INTERNAL DISTRIBUTION and use, including installation on servers that
 *   are accessed by customers or clients, WITHOUT EXPLICIT LICENSE.  All
 *   other forms of redistribution must be licensed from the Kermit Project
 *   at Columbia University.  These permissions apply only to the nonsecure
 *   version of WIKSD.
 *  
 * DISCLAIMER:
 *  
 *   THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
 *   TRUSTEES OF COLUMBIA UNIVERSITY IN THE CITY OF NEW YORK AS TO ITS
 *   FITNESS FOR ANY PURPOSE, AND WITHOUT WARRANTY BY THE TRUSTEES OF
 *   COLUMBIA UNIVERSITY IN THE CITY OF NEW YORK OF ANY KIND, EITHER
 *   EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *   THE TRUSTEES OF COLUMBIA UNIVERSITY IN THE CITY OF NEW YORK SHALL NOT
 *   BE LIABLE FOR ANY DAMAGES, INCLUDING SPECIAL, INDIRECT, INCIDENTAL,
 *   OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM ARISING OUT OR IN
 *   CONNECTION WITH THE USE OF THE SOFTWARE, EVEN IF IT HAS BEEN OR IS
 *   HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  YOU SHALL
 *   INDEMNIFY AND HOLD HARMLESS THE TRUSTEES OF COLUMBIA UNIVERSITY IN
 *   THE CITY OF NEW YORK, ITS EMPLOYEES AND AGENTS FROM AND AGAINST ANY
 *   AND ALL CLAIMS, DEMANDS, LOSS, DAMAGE OR EXPENSE (INCLUDING
 *   ATTORNEYS' FEES) ARISING OUT OF YOUR USE OF THIS SOFTWARE.
 *  
 * The above copyright notice, permissions notice, and disclaimer may not be
 * removed, altered, or obscured and shall be included in all copies of the
 * WIKSD software.  The Trustees of Columbia University in the City of
 * New York reserve the right to revoke this permission if any of the terms
 * of use set forth above are breached.
 *  
 * For further information, contact the Kermit Project, Columbia University,
 * 612 West 115th Street, New York NY 10025-7799, USA; Phone +1 (212) 854 3703,
 * Fax +1 (212) 662 6442, kermit@columbia.edu, http://www.columbia.edu/kermit/
 */

/* On Windows compile with the command line:
 *  
 *  cl /MT -DNT iksdsvc.c wsock32.lib advapi32.lib
 */

/* 
 * Data for the Windows Service IKSDSVC could be stored in:
 * 
 *   HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\IKSD\
 */

#include <stdio.h>

#ifdef NT
#include <windows.h>
#include <winsock.h>
#include <winsvc.h>
#include <process.h>
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

#define MAXCHILDREN 4096
struct CHILDREN
{
    HANDLE hProcess;
    int    socket;
} children[MAXCHILDREN];
int childcount = 0;

SERVICE_STATUS          IKSDStatus;
SERVICE_STATUS_HANDLE   IKSDStatusHandle;
int iksd_started = 0;
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

void  WINAPI IKSDStart (DWORD argc, LPTSTR *argv);
void  WINAPI IKSDCtrlHandler (DWORD opcode);
DWORD IKSDInitialization (DWORD argc, LPTSTR *argv, DWORD *specificError);
HANDLE StartKermit( int socket, char * cmdline, int ShowCmd, int * );

void
SvcDebugOut(LPSTR String, DWORD Status)
{
#ifdef DEBUG
    CHAR  Buffer[1024];
    if (strlen(String) < 1024)
    {
        sprintf(Buffer, String, Status);
        OutputDebugStringA(Buffer);
    }
#endif /* DEBUG */
}

static SERVICE_TABLE_ENTRY   DispatchTable[] =
{
    { "IKSD", (LPSERVICE_MAIN_FUNCTION) IKSDStart },
    { NULL,   NULL      }
};

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
kill_children(void)
{
    int i;

    for (i=0; i<MAXCHILDREN && childcount; i++) {
        if ( children[i].hProcess != INVALID_HANDLE_VALUE ) {
            SvcDebugOut("TerminateProcess  %d\n",(int)children[i].hProcess);
            TerminateProcess(children[i].hProcess, 1111);
            closesocket(children[i].socket);
            CloseHandle(children[i].hProcess);
            children[i].socket = (int)INVALID_HANDLE_VALUE;
            children[i].hProcess = INVALID_HANDLE_VALUE;
            childcount--;       
        }
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
                SvcDebugOut("GetExitCodeProcess ExitCode = %d\n",dwExit);
                if ( dwExit != STILL_ACTIVE ) {
                    SvcDebugOut("Closing socket and process handle\n",0);
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

    while (childcount == MAXCHILDREN) {
        SvcDebugOut("Max Number of Children: Going to Sleep\n",0);
        Sleep(1000);
        check_children();
    }
    
    for ( i=0;i<MAXCHILDREN;i++ ) {
        if ( children[i].hProcess == INVALID_HANDLE_VALUE ) {
            SvcDebugOut("Child Added at %d\n",i);
            children[i].hProcess = hProcess;
            children[i].socket = socket;
            childcount++;
            return;
        }
    }
}

int
main(int argc, char **argv)
{
    init_children();

    if ( ParseStandardArgs(argc, argv) )
        return(0);

    if (!StartServiceCtrlDispatcher(DispatchTable)) {
            SvcDebugOut(" [IKSD] StartServiceCtrlDispatcher error = %d\n",
                         GetLastError());
            return(1);
    }
    return(0);
}

void
iksd_stopped()
{
    DWORD status;

    iksd_started = 0;
    // Initialization complete - report running status.
    IKSDStatus.dwCurrentState       = SERVICE_STOPPED;
    IKSDStatus.dwCheckPoint         = 0;
    IKSDStatus.dwWaitHint           = 0;

    if (!SetServiceStatus (IKSDStatusHandle, &IKSDStatus))
    {
        status = GetLastError();
        SvcDebugOut(" [IKSD] SetServiceStatus error %ld\n",status);
    }

    exit(1);
}

int
open_sockets(DWORD *specificError)
{
    int i;
    int on = 1;

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
            *specificError = 3;
            return 1;
        }
        errno = 0;

        /* Specify the Port may be reused */
        setsockopt(ports[i].lsocket, SOL_SOCKET, SO_REUSEADDR,(char *) &on, sizeof on);

        /* Now bind to the socket */
#ifdef DEBUG
        SvcDebugOut("Binding socket to port %d ...\n",
                ntohs((unsigned short)ports[i].id)) ;
#endif
        if (bind(ports[i].lsocket, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
        {
            i = errno;                  /* save error code */
            closesocket(ports[i].lsocket) ;
            ports[i].lsocket = -1 ;
            ports[i].asocket = -1;
            errno = i;                  /* and report this error */
            SvcDebugOut("tcpsrv_open bind errno %d\n",errno);
            *specificError = 4;
            return 1;
        }
#ifdef DEBUG
        SvcDebugOut("Listening ...\n",0);
#endif
        if (listen(ports[i].lsocket, 15) < 0)
        {
            i = errno;                  /* save error code */
            closesocket(ports[i].lsocket) ;
            ports[i].lsocket = -1 ;
            ports[i].asocket = -1;
            errno = i;                  /* and report this error */
            SvcDebugOut("tcpsrv_open listen errno %d\n",errno);
            *specificError = 5;
            return 1;
        }
    }
    return(0);
}

void
listen_thread( void * dummy )
{
    int i, j;
    int on = 1;
    HANDLE hProcess;
    int sockdup;

    SvcDebugOut("Servicing ports:\n",0);
    for ( i=0;i<portcount ;i++ )
    {
        SvcDebugOut("  %d\n", ntohs((unsigned short)ports[i].id) ) ;
    }

    saddrlen = sizeof(saddr) ;

#ifdef BSDSELECT
    tv.tv_sec  = 1L;
    tv.tv_usec = 0L;
#endif /* BSDSELECT */

    while ( iksd_started )
    {
        while ( !ready_to_accept && iksd_started )
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
???          /* in order to make this work, we need to create an array of socket values */
             ready_to_accept = (( select(&tcpsrv_fd, 1, 0, 0,1 )) == 1) ;
#endif /* IBMSELECT */
#endif /* BSDSELECT */
        }

        if ( ready_to_accept )
        {
            if ((ports[i].asocket = accept(ports[i].lsocket, (struct sockaddr *)&saddr,&saddrlen)) < 0)
            {
                i = errno;                      /* save error code */
                closesocket(ports[i].lsocket) ;
                ports[i].asocket = -1;
                ports[i].lsocket = -1 ;
                errno = i;                      /* and report this error */
                SvcDebugOut("tcpsrv_open accept errno %d\n",errno);
                iksd_stopped();
                return;
            }

            setsockopt(ports[i].asocket, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);

            if ((host = gethostbyaddr((char *)&saddr.sin_addr,4,PF_INET)) != NULL)
            {
                SvcDebugOut(host->h_name,0);
                SvcDebugOut(" connected on port %d\n",ntohs(ports[i].id)) ;
            }

            /* Now start subprocess */
            if ( ports[i].asocket && ports[i].k95cmd ) {
                SvcDebugOut("Starting IKSD with socket %d and command\n  ",ports[i].asocket);
                SvcDebugOut(ports[i].k95cmd,0);
                SvcDebugOut("\n",0);
            } else
                SvcDebugOut("Starting IKSD with socket %d\n",ports[i].asocket);
            hProcess = StartKermit(ports[i].asocket,ports[i].k95cmd, ports[i].showcmd, &sockdup);
            if ( hProcess != INVALID_HANDLE_VALUE ) 
                add_child(hProcess, sockdup);
            else /* we used CLOSE_SOURCE when duplicating */
                closesocket(ports[i].asocket);      
            ready_to_accept = 0;
            continue;                           /* Go get the next one */
        }
        else
        {
            j = errno;                  /* save error code */

            if ( !iksd_started ) {
                /* Close all ports */
                for ( i=0;i<portcount;i++ ) {
                    closesocket(ports[i].lsocket) ;
                    ports[i].lsocket = -1;
                    ports[i].asocket = -1;
                }
                iksd_stopped();
            } else {
                closesocket(ports[i].lsocket) ;
                ports[i].lsocket = -1;
                ports[i].asocket = -1;
            }
            errno = j;                  /* and report this error */
            SvcDebugOut("tcpsrv_open accept errno %d\n",errno);
            return;
        }
    }
}

void WINAPI
IKSDStart (DWORD argc, LPTSTR *argv)
{
    DWORD status;
    DWORD specificError;

    IKSDStatus.dwServiceType        = SERVICE_WIN32;
    IKSDStatus.dwCurrentState       = SERVICE_START_PENDING;
    IKSDStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
    IKSDStatus.dwWin32ExitCode      = 0;
    IKSDStatus.dwServiceSpecificExitCode = 0;
    IKSDStatus.dwCheckPoint         = 0;
    IKSDStatus.dwWaitHint           = 0;

    IKSDStatusHandle = RegisterServiceCtrlHandler( "IKSD", IKSDCtrlHandler);

    if (IKSDStatusHandle == (SERVICE_STATUS_HANDLE)0)
    {
        SvcDebugOut(" [IKSD] RegisterServiceCtrlHandler failed %d\n",
                     GetLastError());
        return;
    }

    // Initialization code goes here.
    status = IKSDInitialization(argc,argv, &specificError);

    // Handle error condition
    if (status != NO_ERROR)
    {
        IKSDStatus.dwCurrentState       = SERVICE_STOPPED;
        IKSDStatus.dwCheckPoint         = 0;
        IKSDStatus.dwWaitHint           = 0;
        IKSDStatus.dwWin32ExitCode      = status;
        IKSDStatus.dwServiceSpecificExitCode = specificError;

        SetServiceStatus (IKSDStatusHandle, &IKSDStatus);
        return;
    }

    iksd_started = 1;
    _beginthread(listen_thread,65536, 0);

    // Initialization complete - report running status.
    IKSDStatus.dwCurrentState       = SERVICE_RUNNING;
    IKSDStatus.dwCheckPoint         = 0;
    IKSDStatus.dwWaitHint           = 0;

    if (!SetServiceStatus (IKSDStatusHandle, &IKSDStatus))
    {
        status = GetLastError();
        SvcDebugOut(" [IKSD] SetServiceStatus error %ld\n",status);
    }

    // This is where the service does its work.
    SvcDebugOut(" [IKSD] Returning the Main Thread \n",0);

    return;
}

VOID WINAPI
IKSDCtrlHandler (DWORD Opcode)
{
    DWORD status;

    switch(Opcode)
    {
        case SERVICE_CONTROL_PAUSE:
        // Do whatever it takes to pause here.
            IKSDStatus.dwCurrentState = SERVICE_PAUSED;
            break;

        case SERVICE_CONTROL_CONTINUE:
        // Do whatever it takes to continue here.
            IKSDStatus.dwCurrentState = SERVICE_RUNNING;
            break;

        case SERVICE_CONTROL_STOP:
        // Do whatever it takes to stop here.
            IKSDStatus.dwWin32ExitCode = 0;
            IKSDStatus.dwCurrentState  = SERVICE_STOP_PENDING;
            IKSDStatus.dwCheckPoint    = 0;
            IKSDStatus.dwWaitHint      = 0;

            if (!SetServiceStatus (IKSDStatusHandle, &IKSDStatus))
            {
                status = GetLastError();
                SvcDebugOut(" [IKSD] SetServiceStatus error %ld\n",
                             status);
            }

            iksd_started = 0;
            kill_children();
            SvcDebugOut(" [IKSD] Leaving IKSD \n",0);
            return;

        case SERVICE_CONTROL_INTERROGATE:
        // Fall through to send current status.
            break;

        default:
            SvcDebugOut(" [IKSD] Unrecognized opcode %ld\n",
                Opcode);
    }

    // Send current status.
    if (!SetServiceStatus (IKSDStatusHandle,  &IKSDStatus))
    {
        status = GetLastError();
        SvcDebugOut(" [IKSD] SetServiceStatus error %ld\n",status);
    }
    return;
}


// Stub initialization function.
int
ParseCmdLine( int argc, char * argv[], DWORD * specificError )
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
        SvcDebugOut("memory allocation error\n",0) ;
        *specificError = 1;
        return 0;
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
                if (isdigit(*p)) {                      /* Use socket number without lookup */
                    service = &servrec;
                    service->s_port = htons((unsigned short)atoi(p));
                } else {                                /* Otherwise lookup the service name */
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

    *specificError = 0;
    portcount++ ;
    return portcount ;
}

HANDLE
StartKermit( int socket, char * cmdline, int ShowCmd, int *psockdup )
{
#ifdef NT
   PROCESS_INFORMATION StartKermitProcessInfo ;
   OSVERSIONINFO osverinfo ;
   STARTUPINFO si ;
   HANDLE sockdup = INVALID_HANDLE_VALUE ;
   static HANDLE hCurrent = INVALID_HANDLE_VALUE;
   static char buf[512] ;

   osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
   GetVersionEx( &osverinfo ) ;

   memset( &si, 0, sizeof(STARTUPINFO) ) ;
   si.cb = sizeof(STARTUPINFO);
   si.dwFlags = STARTF_USESHOWWINDOW;
   si.wShowWindow = ShowCmd;
    
    if ( hCurrent == INVALID_HANDLE_VALUE )
        hCurrent = GetCurrentProcess();

    *psockdup = (int)INVALID_HANDLE_VALUE;

   if (!DuplicateHandle( hCurrent, (HANDLE) socket,
                    hCurrent, &sockdup,
                    DUPLICATE_SAME_ACCESS, TRUE,
                    DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ))
   {
       SvcDebugOut( "\nINTERNET KERMIT SERVICE DAEMON FATAL ERROR:\n",0 );
       SvcDebugOut(" You are using a WinSOCK which does not allow socket handles\n",0);
       SvcDebugOut(" to be duplicated or shared with child processes.\n\n",0);
       SvcDebugOut(" If you are attempting to start Kermit 95 Host Mode,\n",0);
       SvcDebugOut(" please change SESSIONS to 1 in your configuration.\n",0);
       SvcDebugOut("\n Press ENTER to exit...\n",0);
       fscanf( stdin, "" ) ;
       return(INVALID_HANDLE_VALUE) ;
   }

    /* First try IKSDNT.EXE */
#ifdef MSDEV
    strcpy( buf, "msdev.exe iksdnt.exe -# 132 -A " ) ;
#else
    strcpy( buf, "iksdnt.exe -# 132 -A " ) ;
#endif /* DEBUG */
    ltoa( (LONG) sockdup, buf+strlen(buf), 10 ) ;
    strcat( buf, " " );
    strcat( buf, cmdline ) ;

    SvcDebugOut("Executing: ",0);
    SvcDebugOut(buf,0) ;
    SvcDebugOut("\n",0);

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
        CloseHandle(StartKermitProcessInfo.hThread);
        *psockdup = (int)sockdup;
        return (StartKermitProcessInfo.hProcess);
    }

    /* If that fails then try K95.EXE */
#ifdef MSDEV
    strcpy( buf, "msdev.exe k95.exe -# 132 -A " ) ;
#else
    strcpy( buf, "k95.exe -# 132 -A " ) ;
#endif /* DEBUG */
    ltoa( (LONG) sockdup, buf+strlen(buf), 10 ) ;
    strcat( buf, " " );
    strcat( buf, cmdline ) ;

    SvcDebugOut("Executing: \n",0);
    SvcDebugOut(buf,0) ;
    SvcDebugOut("\n",0);
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
        CloseHandle(StartKermitProcessInfo.hThread);
        *psockdup = (int)sockdup;
        return (StartKermitProcessInfo.hProcess);
    }

    SvcDebugOut("CreateProcess() failed gle=%ul\n",GetLastError());
    CloseHandle(sockdup) ;
    return(INVALID_HANDLE_VALUE);
#else /* NT */
    Not built for OS/2 yet.
#endif /* NT */
}


// Stub initialization function.
DWORD
IKSDInitialization(DWORD   argc, LPTSTR  *argv,
    DWORD *specificError)
{
    char *p=NULL, * dbdir=NULL, dbfile[256];
    int i, x;
    int on = 1, rc = 0;
#ifdef NT
    WSADATA data ;
    OSVERSIONINFO osverinfo ;

    SvcDebugOut("Internet Kermit Service Daemon\n",0);
    rc = WSAStartup( MAKEWORD( 2, 0 ), &data ) ;
    if ( rc == WSAVERNOTSUPPORTED )
    {
      WSACleanup() ;
      rc = WSAStartup( MAKEWORD( 1, 1 ), &data ) ;
   }
#else /* NT */

#endif /* NT */

    if (ParseCmdLine( argc, argv, specificError ) == 0)
        return 1;

    if ( !portcount )
    {
#ifdef MSDEV
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

    if ( open_sockets(specificError) )
        return(1);
    return 0;
}


BOOL
IsInstalled()
{
    BOOL bResult = FALSE;
    SC_HANDLE hSCM;
    SC_HANDLE hService;

    // Open the Service Control Manager
    hSCM = OpenSCManager( NULL, // local machine
                          NULL, // ServicesActive database
                          SC_MANAGER_ALL_ACCESS); // full access
    if (hSCM) {

        // Try to open the service
        hService = OpenService( hSCM,
                                "IKSD",
                                SERVICE_QUERY_CONFIG);
        if (hService) {
            bResult = TRUE;
            CloseServiceHandle(hService);
        }

        CloseServiceHandle(hSCM);
    }

    return bResult;
}

BOOL
Install()
{
    char szFilePath[_MAX_PATH];
    SC_HANDLE hSCM;
    SC_HANDLE hService;
    char szKey[256];
    HKEY hKey = NULL;
    DWORD dwData;

    // Open the Service Control Manager
    hSCM = OpenSCManager( NULL, // local machine
                          NULL, // ServicesActive database
                          SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM)
        return FALSE;

    // Get the executable file path
    GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

    // Create the service
    hService = CreateService( hSCM,
                              "IKSD",
                              "IKSD",
                              SERVICE_ALL_ACCESS,
                              SERVICE_WIN32_OWN_PROCESS,
                              SERVICE_AUTO_START,        // start condition
                              SERVICE_ERROR_NORMAL,
                              szFilePath,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return FALSE;
    }

    // make registry entries to support logging messages
    // Add the source name as a subkey under the Application
    // key in the EventLog service portion of the registry.
    strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\IKSD");
    if (RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return FALSE;
    }

    // Add the Event ID message-file name to the 'EventMessageFile' subkey.
    RegSetValueEx( hKey,
                   "EventMessageFile",
                    0,
                    REG_EXPAND_SZ,
                    (CONST BYTE*)szFilePath,
                    strlen(szFilePath) + 1);

    // Set the supported types flags.
    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    RegSetValueEx( hKey,
                   "TypesSupported",
                    0,
                    REG_DWORD,
                    (CONST BYTE*)&dwData,
                     sizeof(DWORD));
    RegCloseKey(hKey);

    // LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_INSTALLED, "IKSD");

    // tidy up
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return TRUE;
}

BOOL
Uninstall()
{
    BOOL bResult = FALSE;
    SC_HANDLE hService;
    SC_HANDLE hSCM;

    // Open the Service Control Manager
    hSCM = OpenSCManager( NULL, // local machine
                          NULL, // ServicesActive database
                          SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM)
        return FALSE;

    hService = OpenService( hSCM,
                            "IKSD",
                            DELETE);
    if (hService) {
        if (DeleteService(hService)) {
            // LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_REMOVED, "IKSD");
            bResult = TRUE;
        } else {
            // LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_NOTREMOVED, "IKSD");
        }
        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCM);
    return bResult;
}

// Returns TRUE if it found an arg it recognised, FALSE if not
// Note: processing some arguments causes output to stdout to be generated.
BOOL
ParseStandardArgs(int argc, char* argv[])
{
    char szFilePath[_MAX_PATH];

    // See if we have any command line args we recognize
    if (argc <= 1)
        return FALSE;

    if ( _stricmp(argv[1], "-h") == 0 ||
         _stricmp(argv[1], "-?") == 0 ||
         _stricmp(argv[1], "/h") == 0 ||
         _stricmp(argv[1], "/?") == 0) {

        //
        GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
        printf("usage: %s [-v | -i | -u | -h]\r\n",szFilePath);
        return TRUE;
    } else if (_stricmp(argv[1], "-v") == 0 ||
               _stricmp(argv[1], "/v") == 0 ) {

        // Spit out version info
        printf("IKSD Version 0.1\n",0);
        printf("The service is %s installed\n",
               IsInstalled() ? "currently" : "not");
        return TRUE; // say we processed the argument

    } else if (_stricmp(argv[1], "-i") == 0 ||
               _stricmp(argv[1], "/i") == 0) {

        // Request to install.
        if (IsInstalled()) {
            printf("%s is already installed\n", "IKSD");
        } else {
            // Try and install the copy that's running
            if (Install()) {
                printf("%s installed\n", "IKSD");
            } else {
                printf("%s failed to install. Error %d\n", "IKSD", GetLastError());
            }
        }
        return TRUE; // say we processed the argument

    } else if (_stricmp(argv[1], "-u") == 0 ||
               _stricmp(argv[1], "/u") == 0) {

        // Request to uninstall.
        if (!IsInstalled()) {
            printf("%s is not installed\n", "IKSD");
        } else {
            // Try and remove the copy that's installed
            if (Uninstall()) {
                // Get the executable file path
                GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
                printf("%s removed. (You must delete the file (%s) yourself.)\n",
                       "IKSD", szFilePath);
            } else {
                printf("Could not remove %s. Error %d\n", "IKSD", GetLastError());
            }
        }
        return TRUE; // say we processed the argument

    }

    // Don't recognise the args
    return FALSE;
}
