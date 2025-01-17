/* This is an example of how to create your K95 Connection DLL    */
/* The DLL must be safe for multithreading because K95 will call  */
/* it from multiple threads.                                      */

/* This example implements a simple ECHO or LOOPBACK service.     */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

/* Local variables for the Local Echo structures that do most of the work. */

#define LOCAL_ECHO_BUFSIZE 512
static USHORT LocalEchoBuf[LOCAL_ECHO_BUFSIZE] ;
static int LocalEchoStart=0, LocalEchoEnd=0, LocalEchoData=0 ;
static HANDLE hmtxLocalEcho = (HANDLE) 0 ;
static HANDLE hevLocalEchoAvail = (HANDLE) 0 ;
static open=0;

static DWORD
CreateLocalEchoMutex( BOOL owned )
{
    if ( hmtxLocalEcho )
        CloseHandle( hmtxLocalEcho ) ;
    hmtxLocalEcho = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxLocalEcho == NULL)
        return GetLastError();
    return 0;
}

static DWORD
RequestLocalEchoMutex( ULONG timo )
{
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxLocalEcho, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
}

static DWORD
ReleaseLocalEchoMutex( void )
{
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxLocalEcho ) ;
    return rc == TRUE ? 0 : GetLastError() ;
}

static DWORD
CloseLocalEchoMutex( void )
{
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxLocalEcho ) ;
    hmtxLocalEcho = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
}


static DWORD
CreateLocalEchoAvailSem( BOOL posted )
{
    if ( hevLocalEchoAvail )
        CloseHandle( hevLocalEchoAvail ) ;
    hevLocalEchoAvail = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevLocalEchoAvail == NULL ? GetLastError() : 0 ;
}

static DWORD
PostLocalEchoAvailSem( void )
{
    BOOL rc = 0 ;

    rc = SetEvent( hevLocalEchoAvail ) ;
    return rc == TRUE ? 0 : GetLastError() ;
}

static DWORD
WaitLocalEchoAvailSem( ULONG timo )
{
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevLocalEchoAvail, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
}

static DWORD
WaitAndResetLocalEchoAvailSem( ULONG timo )
{
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevLocalEchoAvail, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevLocalEchoAvail ) ;
    return rc == WAIT_OBJECT_0 ;
}

static DWORD
ResetLocalEchoAvailSem( void )
{
    BOOL rc = 0 ;

    rc = ResetEvent( hevLocalEchoAvail ) ;
    return rc ;
}

static DWORD
CloseLocalEchoAvailSem( void )
{
    BOOL rc = 0 ;
    rc = CloseHandle( hevLocalEchoAvail ) ;
    hevLocalEchoAvail = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
}


static void
LocalEchoInit( void ) {
    CreateLocalEchoAvailSem( FALSE );
    CreateLocalEchoMutex( TRUE ) ;
    memset(LocalEchoBuf,0,LOCAL_ECHO_BUFSIZE*sizeof(USHORT)) ;
    LocalEchoStart = 0 ;
    LocalEchoEnd = 0 ;
    LocalEchoData = 0;
    ReleaseLocalEchoMutex() ;
}

static void
LocalEchoCleanup( void ) {
    CloseLocalEchoMutex() ;
    CloseLocalEchoAvailSem() ;
}

static int
LocalEchoInBuf() {
    int rc = 0 ;

    if (RequestLocalEchoMutex(INFINITE))
        return(-1);
    if ( LocalEchoStart != LocalEchoEnd )
    {
	rc = (LocalEchoEnd - LocalEchoStart + LOCAL_ECHO_BUFSIZE)%LOCAL_ECHO_BUFSIZE;
    }  
    ReleaseLocalEchoMutex() ;
    return rc ;
}

static int
LocalEchoPutChar( char ch ) {
    int rc = 0 ;

    RequestLocalEchoMutex( INFINITE ) ;
    while ( (LocalEchoStart - LocalEchoEnd == 1) || 
            ( LocalEchoStart == 0 && LocalEchoEnd == LOCAL_ECHO_BUFSIZE - 1 ) )
        /* Buffer is full */
    {
        ReleaseLocalEchoMutex() ;
        Sleep(250);
        RequestLocalEchoMutex( INFINITE ) ;
    }	

    LocalEchoBuf[LocalEchoEnd++] = ch ;
    if ( LocalEchoEnd == LOCAL_ECHO_BUFSIZE )
        LocalEchoEnd = 0 ;
    LocalEchoData = TRUE;
    PostLocalEchoAvailSem()  ;
    ReleaseLocalEchoMutex() ;
    return rc ;
}

static int
LocalEchoPutChars( char * s, int n )
{
    int rc = 0 ;
    int i = 0;

    RequestLocalEchoMutex( INFINITE ) ;
    for ( i=0 ; i<n ; i++ ) 
      rc = LocalEchoPutChar( s[i] ) ;
    ReleaseLocalEchoMutex() ;
    return rc ;
}

static int
LocalEchoPutStr( char * s )
{
    char * p ;
    int rc = 0 ;
    RequestLocalEchoMutex( INFINITE ) ;
    for ( p=s; *p && !rc ; p++ )
      rc = LocalEchoPutChar( *p ) ;
    ReleaseLocalEchoMutex() ;
    return rc ;
}

static int 
LocalEchoGetChar( char * pch )
{
    int rc = 0 ;

    RequestLocalEchoMutex( INFINITE ) ;
    if ( LocalEchoStart != LocalEchoEnd ) {
        *pch = LocalEchoBuf[LocalEchoStart] ;
        LocalEchoBuf[LocalEchoStart]=0;
        LocalEchoStart++ ;

        if ( LocalEchoStart == LOCAL_ECHO_BUFSIZE )
          LocalEchoStart = 0 ;

        if ( LocalEchoStart == LocalEchoEnd ) {
            LocalEchoData = FALSE;
            ResetLocalEchoAvailSem() ;
        }
        rc++ ;
    }
    else 
    {
        *pch = 0 ;
    }
    ReleaseLocalEchoMutex() ;
    return rc ;
}



int
netopen(char * command_line, char * termtype, int height, int width,
         int (* readpass)(char *,char *,int))
{
    char passwd[64];

    LocalEchoInit();

    readpass("Password:",passwd,64);
    if ( strcmp(passwd,"echo") )
        return(-3);

    LocalEchoPutChars(command_line,strlen(command_line));
    LocalEchoPutChars("\r\n",2);
    LocalEchoPutChars(termtype,strlen(termtype));
    LocalEchoPutChars("\r\n",2);
    open = 1;
    return(0);
}

int
netclos() {
    open = 0;
    LocalEchoCleanup();
    return(0);
}
int
nettchk(void) {
    if ( !open ) return(-1);
    return LocalEchoInBuf();
}

int
netflui(void) {
    char c;
    if ( !open ) return(-1);

    while ( LocalEchoInBuf() > 0 )
        LocalEchoGetChar(&c);
    return(0);
}
    
int
netbreak(void) {
    if ( !open ) return(-1);
    return(0);
}

int
netinc(int timeout)  {
    char c;

    if ( !open ) return(-1);


    if ( timeout == 0 ) {
        if (WaitLocalEchoAvailSem( INFINITE ))
            return(-3);
    }
    else {
        if ( timeout > 0 )
            timeout *= 1000;
        else
            timeout *= -1;

        if ( WaitLocalEchoAvailSem(timeout) )
            return(-1);

        if ( LocalEchoInBuf() > 0 ) {
            LocalEchoGetChar(&c);
            return(c);
        }
        else 
            return(-3);
    }
}
    

int
netxin(int count, char * buffer) {
    int i=0;
    
    if ( !open ) return(-1);
    
    while ( i < count ) {
        if ( LocalEchoInBuf() > 0 )
            LocalEchoGetChar(&buffer[i++]);
        else 
            return(-1);
    }
    return(i);
}
    

int
nettoc(int c) {
    if ( !open ) return(-1);

    LocalEchoPutChar(c);
    return(0);
}

int
nettol(char * buffer, int count) {
    if ( !open ) return(-1);

    LocalEchoPutChars(buffer,count);
    return(count);
}

void
terminfo(char * terminal, int height, int width) {
    LocalEchoPutStr(terminal);
}

const char * 
version(void) {
    return("Example K95 Connection DLL");
}

const char *
errorstr(int error) {
    switch ( error ) {
    case 0:
        return("success");
    case -1:
        return("timeout");
    case -2:
        return("connection closed");
    case -3:
        return("incorrect password.  Try 'echo'");
    default:
        return("unknown");
    }
}

int
ttvt(void)
{
    if ( !open ) return(-1);
    return(0);
}

int
ttpkt(void)
{
    if ( !open ) return(-1);
    return(0);
}

int
ttres(void)
{
    if ( !open ) return(-1);
    return(0);
}
