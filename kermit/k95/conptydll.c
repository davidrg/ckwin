/*
 * Windows ConPTY DLL for Kermit 95 / C-Kermit for Windows
 * Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New York.
 * Copyright (C) 2022, David Goodwin <david@zx.net.nz>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  + Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  + Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  + Neither the name of Columbia University nor the names of any
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is basically a copy of the NET_CMD code from ckonet.c and
 * ckotio.c that has been reworked to fit the K95DLL API and altered
 * to make use of Windows PTYs.
 */

// This DLL supports NT only so it always supports NT.
#ifndef NT
#define NT
#endif

//#define LOGGING

#include "ckcdeb.h"

#ifdef fprintf
#undef fprintf
#endif

#include "cknpty.h"
#include "ckclib.h"
#include "ckosyn.h"
#include "ckcker.h"

#include <windows.h>
#include <stdio.h>



#define ERR_NO_PTY_SUPPORT -1
#define ERR_CREATE_PIPE_FAILED -2
#define ERR_OPEN_PTY_FAILED -3
#define ERR_CREATE_PROCESS_FAILED -4
#define ERR_STARTUP_INFO -5

#define CMDLINE_MAX 256

static HANDLE outputReader = NULL, inputWriter = NULL;
static PROCESS_INFORMATION procinfo ;
static STARTUPINFOEX       startinfo ;

/* N E T I N C - Input Buffer */
static unsigned long size = 0, pos = 0;
static unsigned char inbuf[MAXRP+1] ;

/* Stolen from ckonet.c */
#define NET_CMD_BUFSIZE 512
static USHORT NetCmdBuf[NET_CMD_BUFSIZE] ;
static int NetCmdStart=0, NetCmdEnd=0, NetCmdData=0 ;

HANDLE hmtxNetCmd = (HANDLE) 0 ;
HANDLE hevNetCmdAvail = (HANDLE) 0 ;

int ck_sleepint = CK_SLEEPINT * 2;
int fp_digits = 0;
int deblog = 0;

/* Logging */
#ifdef LOGGING
static FILE *logFile = NULL;
static FILE *conOutLog = NULL;
static FILE *conInLog = NULL;

#define log(...) fprintf(logFile, __VA_ARGS__)
#define inchar(c) fprintf(conInLog, "%c", c)
#define outchar(c) fprintf(conOutLog, "%c", c)

void open_log() {
    logFile = fopen("conpty.log", "w");
	conOutLog = fopen("conout.log", "w");
	conInLog = fopen("conin.log", "w");
}

void close_log() {
    log("================ LOG CLOSED =============\n");
    fclose(logFile);
	fclose(conOutLog);
	fclose(conInLog);
    logFile = NULL;
}
#else
#define log(...)
#define inchar(c)
#define outchar(c)
#define open_log()
#define close_log()
#endif

int
dodebug(int flag, char * s1, char * s2, CK_OFF_T n)
{
    return(-1);
}

int
msleep(int m) {
    ULONG start_t, now_t, ms;
    int tt, tr, ti, i;

    start_t = GetTickCount();        /* msecs since Windows was started */

    if ( m <= 500 ) {
        Sleep((long) m);
        return(0);
    }

    tt = m / ck_sleepint;
    tr = m % ck_sleepint;
    ti = ck_sleepint;

    for (i = 0; i < tt; i++) {
        Sleep((long) ti);

        now_t = GetTickCount();

        if ( now_t < start_t ) {
            /* we wrapped */
            ms = (MAXDWORD - start_t + now_t);
        }
        else {
            ms = (now_t - start_t);
        }
        if ( ms >= m )
            return(0);
    }
    if ( tr ) {
        Sleep((long) tr);
    }
    return (0);
}

/* from ckosslc.c */
static void
fatal(char *msg) {
  //  if (!msg) msg = "";

   // printf(msg);
    exit(1);        /* Exit indicating failure */
}


void *
kmalloc(size_t size)
{
    void *ptr;

    if (size == 0) {
        fatal("kmalloc: zero size");
    }
    ptr = malloc(size);
    if (ptr == NULL) {
        fatal("kmalloc: out of memory");
    }
    return ptr;
}

void
kfree(void *ptr)
{
    if (ptr == NULL) {
       // printf("kfree: NULL pointer given as argument");
        return;
    }
    free(ptr);
}

/** End from ckossc.c */

APIRET
CreateNetCmdMutex( BOOL owned )
{
    if ( hmtxNetCmd )
        CloseHandle( hmtxNetCmd ) ;
    hmtxNetCmd = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxNetCmd == NULL)
        return GetLastError();
    return 0;
}

APIRET
RequestNetCmdMutex( ULONG timo )
{
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxNetCmd, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
}

APIRET
ReleaseNetCmdMutex( void )
{
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxNetCmd ) ;
    return rc == TRUE ? 0 : GetLastError() ;
}

APIRET
CloseNetCmdMutex( void )
{
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxNetCmd ) ;
    hmtxNetCmd = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
}


APIRET
CreateNetCmdAvailSem( BOOL posted )
{
    if ( hevNetCmdAvail )
        CloseHandle( hevNetCmdAvail ) ;
    hevNetCmdAvail = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevNetCmdAvail == NULL ? GetLastError() : 0 ;
}

APIRET
PostNetCmdAvailSem( void )
{
    BOOL rc = 0 ;

    rc = SetEvent( hevNetCmdAvail ) ;
    return rc == TRUE ? 0 : GetLastError() ;
}

APIRET
WaitNetCmdAvailSem( ULONG timo )
{
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevNetCmdAvail, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
}

APIRET
WaitAndResetNetCmdAvailSem( ULONG timo )
{
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevNetCmdAvail, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevNetCmdAvail ) ;
    return rc == WAIT_OBJECT_0 ;
}

APIRET
ResetNetCmdAvailSem( void )
{
    BOOL rc = 0 ;

    rc = ResetEvent( hevNetCmdAvail ) ;
    return rc ;
}

APIRET
CloseNetCmdAvailSem( void )
{
    BOOL rc = 0 ;
    rc = CloseHandle( hevNetCmdAvail ) ;
    hevNetCmdAvail = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
}

int NetCmdPutChar( char ch );

void NetCmdInit( void ) {
    int i;

    log("-->NetCmdInit()\n");

    CreateNetCmdAvailSem( FALSE );
    CreateNetCmdMutex( TRUE ) ;
    for ( i = 0 ; i < NET_CMD_BUFSIZE ; i++ )
        memset(NetCmdBuf,0,NET_CMD_BUFSIZE*sizeof(USHORT)) ;
    NetCmdStart = 0 ;
    NetCmdEnd = 0 ;
    NetCmdData = 0;
    ReleaseNetCmdMutex() ;
}

void NetCmdCleanup( void ) {
    log("--> NetCmdCleanup()\n");
    CloseNetCmdMutex() ;
    CloseNetCmdAvailSem() ;
}

int NetCmdInBuf( void ) {
    int rc = 0 ;

    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    if ( NetCmdStart != NetCmdEnd )
    {
        rc = (NetCmdEnd - NetCmdStart + NET_CMD_BUFSIZE)%NET_CMD_BUFSIZE;
    }
    ReleaseNetCmdMutex() ;
    return rc ;
}

int NetCmdPutStr( char * s )
{
    char * p ;
    int rc = 0 ;
    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    for ( p=s; *p && !rc ; p++ )
        rc = NetCmdPutChar( *p ) ;
    ReleaseNetCmdMutex() ;
    return rc ;
}

int NetCmdPutChars( char * s, int n )
{
    int rc = 0 ;
    int i = 0;

//    ckhexdump("NetCmdPutChars",s,n);
    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    for ( i=0 ; i<n ; i++ )
        rc = NetCmdPutChar( s[i] ) ;
    ReleaseNetCmdMutex() ;
//    debug(F101,"NetCmdPutChars","",rc);
    return rc ;
}

int NetCmdPutChar( char ch ) {
    int rc = 0 ;

    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    while ( (NetCmdStart - NetCmdEnd == 1) ||
            ( NetCmdStart == 0 && NetCmdEnd == NET_CMD_BUFSIZE - 1 ) )
        /* Buffer is full */
    {
//        debug(F111,"NetCmdPutChar","Buffer is Full",ch);
        ReleaseNetCmdMutex() ;
        msleep(250);
        RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    }

    NetCmdBuf[NetCmdEnd++] = ch ;
    if ( NetCmdEnd == NET_CMD_BUFSIZE )
        NetCmdEnd = 0 ;
    NetCmdData = TRUE;
    PostNetCmdAvailSem()  ;
    ReleaseNetCmdMutex() ;
    return rc ;
}

int NetCmdGetChar( char * pch )
{
    int rc = 0 ;

    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    if ( NetCmdStart != NetCmdEnd ) {
        *pch = NetCmdBuf[NetCmdStart] ;
        NetCmdBuf[NetCmdStart]=0;
        NetCmdStart++ ;

        if ( NetCmdStart == NET_CMD_BUFSIZE )
            NetCmdStart = 0 ;

        if ( NetCmdStart == NetCmdEnd ) {
            NetCmdData = FALSE;
            ResetNetCmdAvailSem() ;
        }
        rc++ ;
    }
    else
    {
        *pch = 0 ;
    }
    ReleaseNetCmdMutex() ;
    return rc ;
}

void NetCmdReadThread( HANDLE pipe )
{
    int success = 1;
    CHAR c;
    DWORD io;

    log("-->NetCmdReadThread()\n");

    while ( success ) {
        if ( success = ReadFile(pipe, &c, 1, &io, NULL ) )
        {
            NetCmdPutChar(c);
        }
    }

    log("Read failed - read pipe closed?\n");

    // Pipe closed, subprocess terminated. Time to clean up.
    NetCmdCleanup();
}
/* End Stolen from ckonet.c */


int netopen(char * command_line, char * termtype, int height, int width,
            int (* readpass)(char * prompt,char * buffer, int length)) {

    /* termtype is a string representing the currently selected
     * terminal type. Might be nice to pass it to the subprocess
     * in an environment variable. Otherwise there isn't really much
     * we can do with it sadly.
     *
     * readpass is a function for getting a password from the user.
     * We don't need that as we're just running the windows shell.
     *
     * if command_line is non-empty then we'll tell the window shell
     * to launch that program.
     */

    open_log();
    log("--> netopen(\"%s\", \"%s\", %i, %i, readpass)\n", command_line, termtype, height, width);

    if (!pseudo_console_available()) {
        log("Error: No PTY support\n");
        close_log();
        return ERR_NO_PTY_SUPPORT; /* Pseudo consoles are not available in this version of windows */
    }

    // TODO: Check any previous instance has been cleaned up

    NetCmdInit();

    // - Close these after CreateProcess of child application with pseudoconsole object.
    HANDLE inputReadSide = NULL, outputWriteSide = NULL;

//    SECURITY_ATTRIBUTES saAttr;
//    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
//    saAttr.bInheritHandle = TRUE;
//    saAttr.lpSecurityDescriptor = NULL;


    // outputReadSide -> outputReader
    // inputWriter - > inputWriteSide;

    if (!CreatePipe(&inputReadSide, &inputWriter, NULL, 0))
    {
        // Create pipe failed!
        log("FAILED: CreatePipe(inputReadSide, inputWriter)\n");
        CloseHandle(inputReadSide); inputReadSide = NULL;
        CloseHandle(inputWriter); inputWriter = NULL;
        NetCmdCleanup();
        close_log();
        return ERR_CREATE_PIPE_FAILED;
    }

    if (!CreatePipe(&outputReader, &outputWriteSide, NULL, 0))
    {
        // Create pipe failed!
        log("FAILED: CreatePipe(outpuReader, outputWriteSide)\n");
        CloseHandle(inputReadSide); inputReadSide = NULL;
        CloseHandle(inputWriter); inputWriter = NULL;
        CloseHandle(outputReader); outputReader = NULL;
        CloseHandle(outputWriteSide); outputWriteSide = NULL;
        NetCmdCleanup();
        close_log();
        return ERR_CREATE_PIPE_FAILED;
    }

    COORD size;
    size.X = width;
    size.Y = height;
    HRESULT result = open_pseudo_console(size, inputReadSide, outputWriteSide);

    // These are no longer required
    CloseHandle(inputReadSide); inputReadSide = NULL;
    CloseHandle(outputWriteSide); outputWriteSide = NULL;

    if (FAILED(result)) {
        log("FAILED: open_pseudo_console(...)\n");
        CloseHandle(inputWriter); inputWriter = NULL;
        CloseHandle(outputReader); outputReader = NULL;
        NetCmdCleanup();
        close_log();
        return ERR_OPEN_PTY_FAILED;
    }

    // Ok. We should now have a PTY. Now we need some startup info.
    if (FAILED(prepare_startup_info(&startinfo))) {
        log("FAILED: prepare_startup_info(...)\n");
        CloseHandle(inputWriter); inputWriter = NULL;
        CloseHandle(outputReader); outputReader = NULL;
        NetCmdCleanup();
        close_log();
        return ERR_STARTUP_INFO;
    }

    // Now we need to figure out what we're running.
    // This was all stolen from ckonet.c where NET_CMD is handled.
    char cmd_line[CMDLINE_MAX], *cmd_exe, *args, *p;
    int argslen;
    cmd_line[0] = '\0' ;

    cmd_exe = getenv("SHELL");
    if ( !cmd_exe )
        cmd_exe = getenv("COMSPEC");
    if ( !cmd_exe )
        cmd_exe = "cmd.exe";
    ckstrncpy(cmd_line, cmd_exe,CMDLINE_MAX);
    args = cmd_line + strlen(cmd_line); /* don't skip zero */
    argslen = CMDLINE_MAX-strlen(cmd_line);

    /* Look for MKS Shell, if found use -c instead of /c */
    _strlwr_s(cmd_exe, CMDLINE_MAX);

    p = strstr(cmd_exe,"sh.exe");
    if ( !p )
        p = strstr(cmd_exe,"bash.exe");
    if ( !p )
        p = strstr(cmd_exe,"ash.exe");
    if ( p && (p == cmd_exe || *(p-1) == '\\' || *(p-1) == '/')) {
        sprintf(args, " -c \"%s\"", command_line);
    }
    else {
        ckstrncpy(args, " /c ",argslen);
        ckstrncat(args, command_line, argslen);
    }

    log("Running subprocess: %s\n", cmd_line);

    // Now start the subprocess
    ZeroMemory(&procinfo, sizeof(procinfo));

    BOOL success = CreateProcess(
                     NULL,                  /* application name */
                     cmd_line,              /* command line */
                     NULL,                  /* process security attributes */
                     NULL,                  /* primary thread security attrs */
                     FALSE,                 /* inherit handles */
                     EXTENDED_STARTUPINFO_PRESENT, /* creation flags */
                     NULL,                  /* use parent's environment */
                     NULL,                  /* use parent's current directory */
                     &startinfo.StartupInfo,            /* startup info */
                     &procinfo ) ;           /* process info */
// (STARTUPINFO*)
    if (!success) {
        // Failure.
        DWORD errorMessageID = GetLastError();


        LPSTR messageBuffer = NULL;

        //Ask Win32 to give us the string version of that message ID.
        //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        log("FAILED: CreateProcess(), error code: %i - %s\n", GetLastError(), messageBuffer);

        //Free the Win32's string's buffer.
        LocalFree(messageBuffer);

        CloseHandle(procinfo.hProcess);
        CloseHandle(procinfo.hThread);

        CloseHandle(inputWriter); inputWriter = NULL;
        CloseHandle(outputReader); outputReader = NULL;
        NetCmdCleanup();
        close_log();
        return ERR_CREATE_PROCESS_FAILED;
    }

    log("Process created!\n");

    /* Start reading from pipe */
    _beginthread(NetCmdReadThread, 65536, outputReader);

    // Don't need these any further.
//    CloseHandle(procinfo.hProcess);
//    CloseHandle(procinfo.hThread);

    return 0; // success
}


int netclos() {
    log("--> netclos()\n");

    close_pseudo_console();

    close_log();
    return 0; // Success
}

int nettchk(void) {
    if (WaitForSingleObject(procinfo.hProcess, 0L) == WAIT_OBJECT_0) {
        log("--> nettchk()\nWaitForSingleObject failed - closing connection\n");
        // Fatal error - close the connection.
        return(-1);
    } else {
        return (NetCmdInBuf());
    }
}

int netflui(void) {
    return(0);
}

int netbreak(void) {
    return(0); // TODO: Can we just write it to the subprocess?
}

int netinc(int timeout) {
    CHAR  c ;

    if ( !WaitNetCmdAvailSem(timeout<0?-timeout:timeout*1000) ) {
        if (NetCmdGetChar(&c)) {
            log("--> netinc(%i) returns %c\n", timeout, c);
			outchar(c);
            return (c);
        } else {
            return (-1);
        }
    }
    else {
        return (-1);
    }
}

int netxin(int count, char * buffer) {
    int len = nettchk();
    int rc = 0;
    int copysize = count;
    if (copysize > len) {
        copysize = len;
    }
    for (int i = 0; i < copysize; i++) {
        char c = 0;
        int x = NetCmdGetChar(&c);
        if (x > 0) {
            rc += x;
            buffer[i] = c;
        } else {
            break; /* Run out of characters to read. */
        }
    }
    return rc;
}

int nettoc(int c) {
    ULONG bytesWritten=0;
	inchar(c);
    if ( !WriteFile(inputWriter, &c, 1, &bytesWritten, NULL) ) {
        return(-1);
    } else if (bytesWritten == 1) {
        return (0);
    } else {
        return (-1);
    }
}

int nettol(char * buffer, int count) {
    ULONG byteswritten=0;
    if ( !WriteFile( inputWriter, buffer, count, &byteswritten, NULL ) )
    {
//        debug(F101,"nettol unable to write to child process","",GetLastError());
        return(-1);
    }
    else
        return(byteswritten);
}

int ttvt() {
    // K95 is about to send (probably printable) connect/dial output.
    // Most likely a bunch of escape sequences. Nothing we can do anything
    // with. Just acknowledge.
    return 0; // Nothing to do.
}

int ttpkt() {
    // K95 is about to start a file transfer. Nothing we can really do
    // about that. Just acknowledge.
    return 0; // Nothing to do.
}

int ttres() {
    // K95 wants to restore the terminal to "normal" mode (file transfer
    // finished?). Nothing much we can do with this - just acknowledge.
    return 0;
}

void terminfo(char * termtype, int height, int width) {
    // Terminal type and/or size has changed.

    // We can't do anything much about the terminal type (termtype)
    // but the size needs to be forwarded on to the PTY so conhost
    // can resize its buffer.
    COORD size;
    size.X = width;
    size.Y = height;
    resize_pseudo_console(size);
}

const char * version(void) {
    return "Windows ConPTY v0.1";
}

const char * errorstr(int error)  {
    switch(error) {
        case ERR_NO_PTY_SUPPORT:
            return "PTY support unavailable on this version of Windows";
        case ERR_CREATE_PIPE_FAILED:
            return "CreatePipe failed";
        case ERR_OPEN_PTY_FAILED:
            return "CreatePseudoConsole failed";
        case ERR_CREATE_PROCESS_FAILED:
            return "CreateProcess failed";
        case ERR_STARTUP_INFO:
            return "Failed to prepare Startup Info";
        default:
            return "Unknown error";
    }
}

