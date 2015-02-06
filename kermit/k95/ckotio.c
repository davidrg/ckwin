#ifdef NT
char *ckxv = "Win32 Communications I/O, 8.0.229, 29 Dec 2005";
#else
char *ckxv = "OS/2 Communications I/O, 8.0.229, 29 Dec 2005";
#endif /* NT */

/* Define the following symbol to allow setting the title bar... */
/* Not recommended in OS/2 because the call to do this is undocumented. */
#define CK_SETTITLE

/* C K O T I O  --  Kermit communications I/O support for OS/2 systems */

/*
  Also contains code to emulate the UNIX alarm() function under OS/2
  and a set of opendir/readdir/closedir, etc, functions.
*/

/*
  Authors: Jeffrey E Altman (jaltman@secure-endpoints.com), 
             Secure Endpoints Inc., New York City
           Frank da Cruz (fdc@columbia.edu),
             Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.  All rights reserved.

  Originally adapted to OS/2 by Chris Adie <C.Adie@uk.ac.edinburgh>,
  Edinburgh University Computing Service, 1988.

  Adapted to Kermit 5A by Kai Uwe Rommel <rommel@informatik.tu-muenchen.de>,
  1992-93.
*/

/* Includes */

#include "ckcdeb.h"                     /* Typedefs, debug formats, etc */
#include "ckcasc.h"                     /* ASCII character names */
#include "ckcker.h"                     /* Kermit definitions */
#include "ckcnet.h"                     /* Kermit definitions */
#include "ckcxla.h"
#include "ckuxla.h"                     /* Translation tables */
#include "ckuusr.h"         /* Command definitions - needed for ckokey.h */

#include <ctype.h>                      /* Character types */
#include <stdio.h>                      /* Standard i/o */
#include <io.h>                         /* File io function declarations */
#include <fcntl.h>
#include <process.h>            /* Process-control functions */
#include <string.h>                     /* String manipulation declarations */
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>                       /* Time functions */
#include <signal.h>
#ifdef NT
#include <locale.h>
#endif /* NT */
#ifdef OS2ONLY
#include <limits.h>
#endif /* OS2ONLY */

#include <assert.h>
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#include "ckodir.h"

/* macros we might need */
#ifndef _O_APPEND
#define _O_APPEND  O_APPEND
#endif
#ifndef _O_WRONLY
#define _O_WRONLY  O_WRONLY
#endif
#ifndef _O_CREAT
#define _O_CREAT   O_CREAT
#endif
#ifndef _O_TEXT
#define _O_TEXT    O_TEXT
#endif
#ifndef _SH_DENYNO
#define _SH_DENYNO SH_DENYNO
#endif
#ifndef _O_BINARY
#define _O_BINARY  O_BINARY
#endif
#ifndef _S_IFMT
#define _S_IFMT S_IFMT
#endif
#ifndef _S_IFDIR
#define _S_IFDIR S_IFDIR
#endif

#ifdef NT
#include <windows.h>
#include <commctrl.h>
#include <tapi.h>
#include <mcx.h>
#include "cknwin.h"
#include "ckntap.h"
#ifdef CK_TAPI
int TAPIAvail = 0 ;   /* is TAPI Installed */
extern int tttapi ;   /* is Line TAPI? */
extern int tapipass;  /* TAPI Passthrough */
#endif /* CK_TAPI */
#ifdef KUI
#include "ikui.h"
struct _kui_init kui_init = {0,0,0,0,NULL,0,0,0,0,0,0,1};
#endif /* KUI */
_PROTOTYP( void DisplayCommProperties, (HANDLE));
#define popen _popen
#define pclose _pclose
#endif /* NT */

/* Version herald(s) */

#include "ckuver.h"
#ifdef CK_AUTHENTICATION
#include "ckuath.h"
#endif /* CK_AUTHENTICATION */

char ckxsystem[256] = HERALD;
char *ckxsys = HERALD;
#ifdef CK_UTSNAME
char unm_nam[80]="";
char unm_rel[80]="";
char unm_mch[80]="";
char unm_ver[80]="";
char unm_mod[80]="";
#endif /* CK_UTSNAME */
static char *ckxrev = "32-bit";

/* OS/2 system header files & related stuff */

#ifndef NT
#define INCL_WIN
#define INCL_WINSWITCHLIST
#define INCL_ERRORS
#define INCL_KBD
#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */
#define INCL_VIO
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSQUEUES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSIGNALS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSNLS
#define INCL_DOSASYNCTIMER
#define INCL_DOSDATETIME
#define INCL_DOSNMPIPES
#include <os2.h>        /* This pulls in a whole load of stuff */
#ifdef CK_REXX
#define  INCL_REXXSAA
#include <rexxsaa.h>
#endif /* CK_REXX */
#undef COMMENT
#endif /* NT */

#include "ckowin.h"
#include "ckcuni.h"
#include "ckocon.h"
#include "ckcsig.h"
#include "ckokey.h"
#include "ckoslp.h"

#ifdef CK_XYZ
#include "p.h"
#include "ckop.h"
#ifdef XYZ_DLL
int p_avail = 0 ;      /* Must load the DLL */
#else
int p_avail = 1 ;      /* No DLL to load - built-in */
#endif /* XYZ_DLL */
#endif /* CK_XYZ */

HKBD KbdHandle = 0 ;
TID tidKbdHandler = (TID) 0,
    tidRdComWrtScr   = (TID) 0,
    tidConKbdHandler = (TID) 0,
    tidTermScrnUpd   = (TID) 0;
#ifdef NT
TID  tidCommand = (TID) 0 ;
DWORD CommandID = 0 ;
int ckconraw = 0 ;
HINSTANCE hInstance = NULL ;
extern HWND hwndConsole;
#endif /* NT */
BOOL KbdActive = 0 ;
int Shutdown = FALSE;
extern int tt_status[VNUM] ;
int k95stdin=0,k95stdout=0;
extern int inserver, local;

#ifdef CHAR
#undef CHAR
#endif /* CHAR */

/*
 Variables available to outside world:

   dftty  -- Pointer to default tty name string, like "/dev/tty".
   dfloc  -- 0 if dftty is console, 1 if external line.
   dfprty -- Default parity
   dfflow -- Default flow control
   ckxech -- Flag for who echoes console typein:
     1 - The program (system echo is turned off)
     0 - The system (or front end, or terminal).
   functions that want to do their own echoing should check this flag
   before doing so.

 Functions for assigned communication line (either external or console tty):

   sysinit()               -- System dependent program initialization
   syscleanup()            -- System dependent program Shutdown
   ttopen(ttname,local,mdmtyp) -- Open the named tty for exclusive access.
   ttclos()                -- Close & reset the tty, releasing any access lock.
   ttpkt(speed,flow,parity)-- Put the tty in packet mode
                                or in DIALING or CONNECT modem control state.
   ttvt(speed,flow)        -- Put the tty in virtual terminal mode.
   ttinl(dest,max,timo,...) -- Timed read packet from the tty.
   ttinc(timo)             -- Timed read character from tty.
   ttchk()                 -- See how many characters in tty input buffer.
   ttxin(n,buf)            -- Read n characters from tty (untimed).
   ttol(string,length)     -- Write a string to the tty.
   ttxout(string,length)   -- Write a string to the tty (with conversions).
   ttoc(c)                 -- Write a character to the tty.
   ttflui()                -- Flush tty input buffer.
   ttgspd()                -- Speed of tty line.

Functions for console terminal:

   conraw()  -- Set console into Raw mode
   concooked() -- Set console into Cooked mode
   conoc(c)  -- Unbuffered output, one character to console.
   conol(s)  -- Unbuffered output, null-terminated string to the console.
   conola(s) -- Unbuffered output, array of strings to the console.
   conxo(n,s) -- Unbuffered output, n characters to the console.
   conchk()  -- Check if characters available at console (bsd 4.2).
                Check if escape char (^\) typed at console (System III/V).
   coninc(timo)  -- Timed get a character from the console.
 Following routines are dummies:
   congm()   -- Get console terminal mode.
   concb()   -- Put console into single char mode with no echo.
   conres()  -- Restore console to congm mode.
   conint()  -- Enable console terminal interrupts.
   connoi()  -- No console interrupts.

Time functions

   sleep(t)  -- Like UNIX sleep
   msleep(m) -- Millisecond sleep
   ztime(&s) -- Return pointer to date/time string
   rtimer() --  Reset timer
   gtimer()  -- Get elapsed time since last call to rtimer()
*/


/* Defines */

#define HUPTIME 1000                    /* Milliseconds for hangup */

#ifndef DEVNAMLEN
#ifdef NETCONN          /* Allow for long network hostnames */
#define DEVNAMLEN 128
#else                   /* No networks, applies to OS/2 device names only. */
#define DEVNAMLEN 14
#endif /* NETCONN */
#endif /* DEVNAMLEN */

/* definitions hiding 32-bit / 16-bit differences */

#ifdef __32BIT__

#ifndef NT
USHORT DosDevIOCtl32(PVOID pData, USHORT cbData, PVOID pParms, USHORT cbParms,
                     USHORT usFunction, USHORT usCategory, HFILE hDevice)
{
  ULONG ulParmLengthInOut = cbParms, ulDataLengthInOut = cbData;
  return (USHORT) DosDevIOCtl(hDevice, usCategory, usFunction,
                              pParms, cbParms, &ulParmLengthInOut,
                              pData, cbData, &ulDataLengthInOut);
}
#endif /* NT */

typedef ULONG U_INT;
#define FILEFINDBUF FILEFINDBUF3
#define FSQBUFFER FSQBUFFER2

#define DosFindFirst(p1, p2, p3, p4, p5, p6) \
        DosFindFirst(p1, p2, p3, p4, p5, p6, 1)

#define DosDevIOCtl DosDevIOCtl32
#endif

/* Declarations */

/* dftty is the device name of the default device for file transfer */
/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

extern long speed;
extern int parity, fcharset, flow, ttcarr, isinterrupted;
extern MACRO *macrotab;
#ifdef OS2PM
extern int os2pm;
#endif /* OS2PM */
#ifndef K95G
/* This is to allow remote operation */
char *dftty = "0"; /* stdin */
int dfloc = 0;
#else
char *dftty = "com1"; /* COM1 */
int dfloc = 1;
#endif /* K95G */

int OSVer = 0;
int nt351 = 0;

#ifdef NTSIG
int TlsIndex = 0;
#endif /* NTSIG */

#ifdef OS2ONLY
bool ttslip = 0 ;  /* Equals 1 if being used as a replacement for SLIPTERM */
bool ttppp  = 0 ;  /* Equals 1 if being used as a replacement for SLATTACH */
#endif /* OS2ONLY */
bool ttshare = 0;                /* do not open devices in shared mode */
int ttyfd = -1;         /* TTY file descriptor (not open yet) */
int dfprty = 0;                 /* Default parity (0 = none) */
int ttprty = 0;                 /* Parity in use. */
int ttmdm = 0;                  /* Modem in use. */
int dfflow = FLO_AUTO;          /* Default flow is AUTO-DETECT */
int backgrd = 0;                /* Assume in foreground */
int ttcarr = CAR_AUT;           /* Carrier handling mode. */
int ckxech = 1; /* 0 if system normally echoes console characters, else 1 */
extern int tcsl ;               /* Local character set */
extern int tt_inpacing ;        /* Input pacing */
extern int exithangup;

#ifdef NETCONN
extern int network, nettype, ttnproto, tn_exit, xitsta ;
#endif
extern int wasclosed;
extern int exitonclose;
extern int quiet;

int ck_sleepint = CK_SLEEPINT ;

char startupdir[CKMAXPATH] = "./";
char exedir[CKMAXPATH] = "./";
char usertitle[64] = "";

/* Declarations of variables global within this module */

static struct rdchbuf_rec {             /* Buffer for serial characters */
        unsigned char buffer[32768];
        U_INT length, index;
} rdchbuf;

int ttpush=-1;                          /* So we can perform a peek */

static ULONG tcount;                    /* Elapsed time counter */
static int conmode, consaved;
static int ttpmsk = 0377;               /* Parity stripping mask. */
static char ttnmsv[DEVNAMLEN+1];
int islocal;
int ishandle=0;
int pid = 0;
#ifdef NT
static DCB ttydcb ;
static LPCOMMCONFIG ttycfg=NULL;
static DWORD cfgsize=0;
#else /* NT */
static DCBINFO ttydcb;
#endif /* NT */

int quitonbreak = FALSE ;               /* Should SIGBREAK result in Quit */
static int nOldCP;
static char szOldTitle[80];

#ifdef NT
HANDLE
#else
HVIO
#endif /* NT */
VioHandle = 0;

#ifndef NOLOCAL
#ifndef KUI
static CK_VIDEOMODEINFO StartupConsoleModeInfo ;
#endif /* KUI */
#endif /* NOLOCAL */
#ifndef NT
HAB hab = 0 ;         /* Handle to an Anchor Block */
#endif /* NT */

TID tidAlarm = (TID) 0 ;

#define THRDSTKSIZ      (2*131072)


#ifdef NT
/* Overlapped I/O structures */
#define MAXOW 7
int maxow_usr = MAXOW;
int maxow = MAXOW; /* only use 1; otherwise, serial ports are overdriven */
/* This was caused because the default TX buffer size is guess what?  */
/* ZERO.  Set the TX Buffer Size to be the largest packet size or     */
/* larger with SetupComm() and things work great!                     */
static OVERLAPPED overlapped_write[30] = {
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1}
};
static char * ow_ptr[30] = {
    NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL
};
static int    ow_size[30] = {
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0
};
static int    ow_inuse[30] = {
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0
};
int nActuallyWritten ;

/* Even though we allocate all of these read structs only the first one is */
/* ever used unless NEWRDCH is defined.                                    */
static OVERLAPPED overlapped_read[30] = {
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},
    {0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1},{0L,0L,0L,0L,(HANDLE)-1}
};
static char * or_ptr[30] = {
    NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL
};
static int    or_size[30] = {
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0
};
static int    or_inuse[30] = {
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0
};
static int    or_read[30] = {
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0
};
int nActuallyRead ;

/* Lets try owwait = FALSE and see if anybody notices */
int owwait = FALSE ;
#endif /* NT */

char OS2CmdFile[256] = "";              /* a copy of cmdfile if specified */

#define TT_MODE_NONE 0
#define TT_MODE_VT   1
#define TT_MODE_PKT  2
#define TT_MODE_NORM 3
static int tt_mode = TT_MODE_NONE;


/* Forward declarations */

_PROTOTYP( static int os2setdtr,  (int) );
_PROTOTYP( static int os2setcarr, (int) );
_PROTOTYP( static int ttsettings, (int, int) );
_PROTOTYP( int ttsetspd, (long) );
_PROTOTYP( int concooked, (void) );
_PROTOTYP( int os2rexxinit, (void) );
_PROTOTYP( int os2setcp, (int) );
_PROTOTYP( int os2getcplist, (int *, int) );
_PROTOTYP( int os2getcp, (void) );
_PROTOTYP( int os2settitle, (char *, int) );
_PROTOTYP( int os2gettitle, (char *, int) );
#ifndef NOLOCAL
_PROTOTYP( int StartConnectThreads, ( void ) );
_PROTOTYP( int StopConnectThreads, (int) );
#endif /* NOLOCAL */

#ifndef NT
_PROTOTYP( APIRET ConnectToPM, (void) );
_PROTOTYP( APIRET ReadFromPM, (char *, ULONG, ULONG *) );
_PROTOTYP( APIRET DisconnectFromPM, (void) );
#endif /* OS2PM */

#ifdef HWPARITY
/*
  Unfortunately we must do this with global variables rather than through the
  tt...() APIs to avoid changing the APIs and the many modules that use them.
  If hwparity != 0, this indicates 8 data bits + parity, rather than 7 data
  bits + parity or 8 data bits and no parity, and overrides the regular parity
  variable, which is communicated to this module thru ttpkt(), and represented
  locally by the ttprty variable.
*/
    extern int hwparity;                /* Hardware parity */
    extern int stopbits;                /* Stop bits */
#endif /* HWPARITY */

#ifdef CKNTSIG
/*
  Our own SIGNAL routine, to avoid calling C runtime library.
*/
static SIGTYP (*siginthandler)(int) = NULL; /* Pointer to SIGINT handler */

static BOOL
cknthandler(DWORD dummy) {
    debug(F100,"cknthandler","",0);
    (*siginthandler)(SIGINT);
    return(TRUE);
}

SIGTYP (*
ckntsignal(int sig, SIGTYP (*f)(int)))(int) {
    debug(F101,"ckntsignal sig","",sig);
    if (f == SIG_DFL) {
        siginthandler = NULL;
#ifndef KUI
        SetConsoleCtrlHandler(NULL, FALSE);
#endif /* KUI */
    } else if (f == SIG_IGN) {
        siginthandler = NULL;
#ifndef KUI
        SetConsoleCtrlHandler(NULL, FALSE);
#endif /* KUI */
    } else {
        siginthandler = f;
#ifndef KUI
        SetConsoleCtrlHandler((PHANDLER_ROUTINE) cknthandler, TRUE);
#endif /* KUI */
    }
}
#endif /* CKNTSIG */

/* Saving/restoring of hot handles */

static int savedtty = 0;
#ifdef NT
static DCB saveddcb ;
static LPCOMMCONFIG savedcfg=NULL;
static DWORD savedcfgsize=0;
#else /* NT */
static long savedspeed;
static LINECONTROL savedlc;
static DCBINFO saveddcb;
static BYTE savedstat;
#endif /* NT */


#ifdef NT
/* d e b u g C o m m -- generate a debug log entry for the */
/* current state of the DCB and TIMEOUT structures         */

void
debugComm( char * msg, DCB * lpDCB, COMMTIMEOUTS * timeouts )
{
    if ( !deblog )
        return;

    debug(F110,"debugComm",msg,0);
    if ( lpDCB ) {
    debug(F101,"  BaudRate                    ","",lpDCB->BaudRate);
    debug(F101,"  Binary Mode (no EOF check)  ","",lpDCB->fBinary);
    debug(F101,"  Enable Parity Checking      ","",lpDCB->fParity);
    debug(F101,"  CTS Handshaking on output   ","",lpDCB->fOutxCtsFlow);
    debug(F101,"  DSR Handshaking on output   ","",lpDCB->fOutxDsrFlow);
    debug(F101,"  DTR Flow Control            ","",lpDCB->fDtrControl);
    debug(F101,"  DSR Sensitivity             ","",lpDCB->fDsrSensitivity);
    debug(F101,"  Continue TX when Xoff sent  ","",lpDCB->fTXContinueOnXoff);
    debug(F101,"  Enable output X-ON/X-OFF    ","",lpDCB->fOutX);
    debug(F101,"  Enable input X-ON/X-OFF     ","",lpDCB->fInX);
    debug(F101,"  Enable Err Replacement      ","",lpDCB->fErrorChar);
    debug(F101,"  Enable Null stripping       ","",lpDCB->fNull);
    debug(F101,"  RTS flow control            ","",lpDCB->fRtsControl);
    debug(F101,"  Abort all I/O on Error      ","",lpDCB->fAbortOnError);
    debug(F101,"  Transmit X-ON threshold     ","",lpDCB->XonLim);
    debug(F101,"  Transmit X-OFF threshold    ","",lpDCB->XoffLim);
    debug(F101,"  Num of bits/byte, 4-8       ","",lpDCB->ByteSize);
    debug(F101,"  Parity (0-4=N,O,E,M,S)      ","",lpDCB->Parity);
    debug(F101,"  StopBits (0,1,2 = 1, 1.5, 2)","",lpDCB->StopBits);
    debug(F101,"  Tx and Rx X-ON character    ","",lpDCB->XonChar);
    debug(F101,"  Tx and Rx X-OFF character   ","",lpDCB->XoffChar);
    debug(F101,"  Error replacement char      ","",lpDCB->ErrorChar);
    debug(F101,"  End of Input character      ","",lpDCB->EofChar);
    debug(F101,"  Received Event character    ","",lpDCB->EvtChar);
    }
    if ( timeouts ) {
    debug(F101,"  ReadIntervalTimeout        ","",timeouts->ReadIntervalTimeout);
    debug(F101,"  ReadTotalTimeoutMultiplier ","",timeouts->ReadTotalTimeoutMultiplier);
    debug(F101,"  ReadTotalTimeoutConstant   ","",timeouts->ReadTotalTimeoutConstant);
    debug(F101,"  WriteTotalTimeoutMultiplier","",timeouts->WriteTotalTimeoutMultiplier);
    debug(F101,"  WriteTotalTimeoutConstant  ","",timeouts->WriteTotalTimeoutConstant);
    }
}
#endif /* NT */


int
savetty() {
    if (ttyfd != -1) {
#ifdef NT
        saveddcb.DCBlength = sizeof(DCB);
        GetCommState( (HANDLE) ttyfd, &saveddcb ) ;

        if ( deblog )
            debugComm( "savetty initial values", &ttydcb, NULL );

        savedcfg->dwSize = 1024;
        savedcfgsize = 1024;
        GetCommConfig( (HANDLE) ttyfd, savedcfg, &savedcfgsize );
#else /* NT */
        savedspeed = ttgspd();
        DosDevIOCtl(&savedlc,sizeof(savedlc),NULL,0,
                ASYNC_GETLINECTRL,IOCTL_ASYNC,ttyfd);
        DosDevIOCtl(&saveddcb,sizeof(saveddcb),NULL,0,
                ASYNC_GETDCBINFO,IOCTL_ASYNC,ttyfd);
        DosDevIOCtl(&savedstat,sizeof(savedstat),NULL,0,
                ASYNC_GETMODEMOUTPUT,IOCTL_ASYNC,ttyfd);
#endif /* NT */
        savedtty = 1;
    }
    return 0;
}

int
restoretty() {
#ifndef NT
    MODEMSTATUS ms;
    UINT cmd = 0, data = 0 ;
#endif /* NT */

  if (savedtty) {
#ifdef NT
#ifdef COMMENT
      SetCommState( (HANDLE) ttyfd, &saveddcb ) ;
#else
      SetCommConfig( (HANDLE) ttyfd, savedcfg, savedcfgsize );
#endif
#else /* NT */
    ttsetspd(savedspeed);
    DosDevIOCtl(&data,sizeof(data),&cmd,sizeof(cmd),
                DEV_FLUSHOUTPUT,IOCTL_GENERAL,ttyfd);
    DosDevIOCtl(NULL,0,&savedlc,sizeof(savedlc),
                ASYNC_SETLINECTRL,IOCTL_ASYNC,ttyfd);
    ms.fbModemOn = 0;
    ms.fbModemOff = 255;
    if (savedstat & DTR_ON) ms.fbModemOn |= DTR_ON;
    else ms.fbModemOff &= DTR_OFF;
    if (savedstat & RTS_ON) ms.fbModemOn |= RTS_ON;
    else ms.fbModemOff &= RTS_OFF;
    DosDevIOCtl(&data,sizeof(data),&ms,sizeof(ms),
                ASYNC_SETMODEMCTRL,IOCTL_ASYNC,ttyfd);
    DosDevIOCtl(NULL,0,&saveddcb,sizeof(saveddcb),
                ASYNC_SETDCBINFO,IOCTL_ASYNC,ttyfd);
#endif /* NT */
  }
  return 0;
}

/* Code Page functions */

int
os2getcp() {
#ifdef NT
#ifdef KUI
    return GetACP();
#else /* KUI */
    return GetConsoleCP() ;
#endif /* KUI */
#else /* NT */
    U_INT cbData, nCodePage[8];
    return (DosQueryCp(sizeof(nCodePage), nCodePage, &cbData) == 0)
           ? nCodePage[0] : 0;
#endif /* NT */
}


#ifdef COMMENT
int
os2checkcp(cp) int cp; {
#ifdef NT
    return FALSE ;
#else /* NT */
    U_INT cbData, nCodePage[8], nCnt;
    if (DosQueryCp(sizeof(nCodePage), nCodePage, &cbData))
      return FALSE;
    for (nCnt = 1; nCnt < cbData / sizeof(nCodePage[0]); nCnt++)
      if (nCodePage[nCnt] == cp)
        return TRUE;
    return FALSE;
#endif /* NT */
}
#endif /* COMMENT */

#ifdef NT
static int * CPList ;
static int CPListSize ;
static int CPListIndex ;

BOOL CALLBACK
enumproc( LPSTR cpstr )
{
   CPList[CPListIndex] = atoi(cpstr) ;
   if ( ++CPListIndex < CPListSize )
      return TRUE ;
   else return FALSE ;
}
#endif /* NT */

int
os2getcplist(cplist, size) int *cplist; int size; {
#ifdef NT
   int i ;

    CPList = cplist ;
    CPListSize = size / sizeof(int) ;
    CPListIndex = 1 ;

    for ( i=0 ; i<CPListSize ; i++ )
        CPList[i] = 0 ;

    CPList[0] = GetConsoleCP() ;

   if (size > 1)
       EnumSystemCodePages( enumproc, CP_INSTALLED ) ;

    return CPListIndex ;
#else /* NT */
    U_INT cbData;
    if (DosQueryCp(size, (U_INT *) cplist, &cbData))
      return 0;
    return cbData / sizeof(int);
#endif /* NT */
}

int                                     /* Change code page */
os2setcp(cp) int cp; {
#ifdef NT
#ifdef KUI
    return 1;                           /* Code Pages do not apply */
#else /* KUI */
    return SetConsoleOutputCP( cp )
        && SetConsoleCP( cp ) ;
#endif /* KUI */
#else /* NT */
    if ( k95stdout )                    /* Can't set Code Page for stdio    */
        return 0;                       /* This has the affect of blocking  */
                                        /* and never returning if we have a */
                                        /* getchar() in progress.           */

    return((VioSetCp(0, cp, VioHandle) == 0) &&
           (KbdSetCp(0, cp, KbdHandle) == 0) &&
           (DosSetProcessCp(cp) == 0));
#endif /* NT */
}

void
DisableHardErrors( void )
{
#ifdef NT
   SetErrorMode( SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS ) ;
#else /* NT */
    DosError(FERR_DISABLEHARDERR | FERR_DISABLEEXCEPTION);
#endif /* NT */
}

APIRET
os2getpid(void)
{
#ifdef NT
    return GetCurrentProcessId();
#else
    PTIB pptib;
    PPIB pppib;
    DosGetInfoBlocks(&pptib, &pppib);
    return pppib -> pib_ulpid;
#endif
}

#ifdef NT
int
setOSVer( void )
{
    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    OSVer = osverinfo.dwPlatformId ;
    if ( osverinfo.dwMajorVersion < 4 )
        nt351 = 1;
    return(OSVer);
}
#endif /* NT */

char *
getcpu( void )
{
   static char buffer[64] = "unknown" ;
#ifdef NT
   SYSTEM_INFO si ;
   char numstr[32] ;
   memset( &si, 0, sizeof(si) ) ;
   GetSystemInfo( &si ) ;
   if ( isWin95() && !si.wProcessorLevel )
   {
      switch ( si.dwProcessorType ) {
      case 386:
         strcpy( buffer, "intel-386" ) ;
         break;
      case 486:
         strcpy( buffer, "intel-486" ) ;
         break;
      case 586:
         strcpy( buffer, "intel-pentium" ) ;
         break;
      default:
         strcpy( buffer, "intel-" ) ;
         _itoa( si.dwProcessorType, numstr, 10 ) ;
         ckstrncat( buffer, numstr, 64 ) ;
      }
   }
   else
   {
      switch ( si.wProcessorArchitecture ) {
      case PROCESSOR_ARCHITECTURE_INTEL:
         ckstrncpy( buffer, "intel-", 64 ) ;
         switch ( si.wProcessorLevel ) {
         case 3:
            ckstrncat( buffer, "386", 64 ) ;
            break;
         case 4:
            ckstrncat( buffer, "486", 64 ) ;
            break;
         case 5:
            ckstrncat( buffer, "pentium", 64 ) ;
            break;
         default:
            _itoa( si.wProcessorLevel, numstr, 10 ) ;
            ckstrncat( buffer, numstr, 64 );
         }
         break;
      case PROCESSOR_ARCHITECTURE_PPC:
         ckstrncpy( buffer, "ppc-", 64 ) ;
         switch ( si.wProcessorLevel ) {
         case 1:
            ckstrncat( buffer, "601", 64 ) ;
            break;
         case 3:
            ckstrncat( buffer, "603", 64 ) ;
            break;
         case 4:
            ckstrncat( buffer, "604", 64 ) ;
            break;
         case 6:
            ckstrncat( buffer, "603+", 64 ) ;
            break;
         case 9:
            ckstrncat( buffer, "604+", 64 ) ;
            break;
         case 20:
            ckstrncat( buffer, "620", 64 ) ;
            break;
         default:
            _itoa( si.wProcessorLevel, numstr, 10 ) ;
            ckstrncat( buffer, numstr, 64 );
         }
         break;
      case PROCESSOR_ARCHITECTURE_MIPS:
         ckstrncpy( buffer, "mips-", 64 ) ;
         switch ( si.wProcessorLevel ) {
         case 4:
            ckstrncat( buffer, "r4000", 64 ) ;
            break;
         default:
            _itoa( si.wProcessorLevel, numstr, 10 ) ;
            ckstrncat( buffer, numstr, 64 );
         }
         break;
      case PROCESSOR_ARCHITECTURE_ALPHA:
         ckstrncpy( buffer, "alpha-", 64 ) ;
         switch ( si.wProcessorLevel ) {
         default:
            _itoa( si.wProcessorLevel, numstr, 10 ) ;
            ckstrncat( buffer, numstr, 64 );
         }
         break;
      case PROCESSOR_ARCHITECTURE_UNKNOWN:
         ckstrncpy( buffer, "unknown", 64 ) ;
         break;
      }
   }
#else /* NT */
   ckstrncpy( buffer, CKCPU, 64 ) ;
#endif
   return buffer ;
}

static void
term_signal(int sig) {
    debug(F101,"term_signal handler","",sig);
#ifndef NOLOCAL
    SetConnectMode(0,CSX_KILL_SIG);
#endif /* NOLOCAL */
    printf("\n!!! Received KILL signal !!!\n");
    bleep(BP_FAIL);
#ifndef NODIAL
    if (mdmhup() < 1)
#endif /* NODIAL */
        tthang();
    doexit(BAD_EXIT,-1);
    signal(SIGTERM, SIG_DFL);
}

#ifdef NT
BOOL
CloseHandler( DWORD dwCtrlType ) {      //  control signal type
    extern unsigned long startflags;

    debug(F111,"Win32 Close Handler","dwCtrlType",dwCtrlType);
    switch ( dwCtrlType ) {
    case CTRL_LOGOFF_EVENT:
        if ( startflags & 128 )
            return TRUE;
        doexit(BAD_EXIT,0);
        return TRUE;
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        doexit(BAD_EXIT,0);
        return TRUE;
    default:
        return FALSE;
    }
}

#ifdef COMMENT
/*
    If we were MS-DOS Kermit we could use these functions.
    But we are a Win32 app, so we can't.
*/
BOOL
Win95QueryClose( void ) {
    short rc ;
    __asm {
        mov ah, 16h ;
        mov al, 8Fh ;
        mov dh, 1 ;
        mov dl, 0 ;
        int 2Fh ;
        mov rc, ax ;
    }
    return !rc;
}

BOOL
Win95AcknowledgeClose( void ) {
    short rc ;
    __asm {
        mov ah, 16h ;
        mov al, 8Fh ;
        mov dh, 2 ;
        mov dl, 0 ;
        int 2Fh ;
        or ax, ax ;
        mov rc, ax ;
    }
    return rc ;
}

BOOL
Win95CancelClose( void ) {
    short rc ;
    __asm {
        mov ah, 16h ;
        mov al, 8Fh ;
        mov dh, 3 ;
        mov dl, 0 ;
        int 2Fh ;
        or ax, ax ;
        mov rc, ax ;
    }
    return rc;
}

BOOL
Win95EnableClose( void ) {
    short rc ;
    __asm {
        mov ah, 16h ;
        mov al, 8Fh ;
        mov dh, 0 ;
        mov dl, 01h ;
        int 2Fh ;
        or ax, ax ;
        mov rc, ax ;
    }
    return rc;

}

BOOL
Win95DisableClose( void ) {
    short rc ;
    __asm {
        mov ah, 16h ;
        mov al, 8Fh ;
        mov dh, 0 ;
        mov dl, 00h ;
        int 2Fh ;
        or ax, ax ;
        mov rc, ax ;
    }
    return rc;
}
#endif /* COMMENT */
#endif /* NT */


#ifdef NT
void
Win95AltGrInit( void )
{
    /*
    Performing a grep ALTGRUSED *.ASM in the ddk\keyb\samples\driver\layout
    directory show that at least of the samples only three layouts do not
    have an AltGr key.  So if we can read the layout, lets initialize the
    win95altgr variable to 1 if the layout is not one of those three.
    And 0 if we are unable to read the layout.

    Btw, 4NT.EXE just assumes that every layout has an AltGr key.
    */
    extern int win95altgr;
    CHAR    lpLayoutName[KL_NAMELENGTH]="";
    int rc ;
    rc = GetKeyboardLayoutName(lpLayoutName);
    debug(F111,"Win95 Keyboard Layout Name",lpLayoutName,rc);
    if ( lpLayoutName[0] ) {
        if ( !strcmp("00000409",lpLayoutName) )         /* US 101 */
            win95altgr = 0;
        else if ( !strcmp("00020409",lpLayoutName) )    /* US Dvorak */
            win95altgr = 0;
        else if ( !strcmp("00000411",lpLayoutName) )    /* Japan 101 */
            win95altgr = 0;
        else
            win95altgr = 1;
    }
    else
        win95altgr = 0;
    debug(F101,"Win95 AltGr Initialized","",win95altgr);
}

void
Win95DisplayLocale( void )
{
    LCID    Locale = LOCALE_SYSTEM_DEFAULT ;
    LCTYPE  LCType = 0;
    CHAR    lpLCDATA[1024]="";
    int     cchData=1024;
    int     rc=0;
    int     i=0;
    HKL     KBLayout=0;
    CHAR    lpLayoutName[KL_NAMELENGTH]="";

    KBLayout = GetKeyboardLayout(0);
    GetKeyboardLayoutName(lpLayoutName);
    printf("Keyboard Layout = %s [%u]\n",lpLayoutName,(unsigned short)KBLayout);


    printf("Locale Information:\n");
    for ( LCType=0 ; LCType<= 0x5A ; LCType++ ) {
        rc = GetLocaleInfo( LOCALE_SYSTEM_DEFAULT,LCType,lpLCDATA,cchData);
        printf("  LCType 0x%08x = \"%-25s\"",LCType,lpLCDATA);
        rc = GetLocaleInfo( LOCALE_USER_DEFAULT,LCType,lpLCDATA,cchData);
        printf("  \"%-25s\"\n",lpLCDATA);
    }
}
#endif /* NT */

char *
GetLocalUser()
{
    static char localuser[UIDBUFLEN]="";

    /* Initialize the Username buffer if possible */
    /* Allow environment variable to override the OS value */
    char * user;
    
    if (localuser[0])
        return(localuser);
    
    user = getenv( "USER" );
    if ( user ) {
        ckstrncpy( localuser, user, UIDBUFLEN );
    }
    else
        localuser[0] = '\0';

#ifdef NT
    if (!localuser[0]) {
        DWORD uidlen = UIDBUFLEN ;
        GetUserName( localuser, &uidlen );
    }
#endif /* NT */
    return(localuser);
}

#ifdef NT
VOID
firsttime(void)
{
    /* 
     *  What we will do is check to see if the Common and User App Data
     *  directories exist.  If they do not, we will create them.
     */

    char dir[CKMAXPATH+1], file[CKMAXPATH+1], *common, *appdata, *personal;

    common = (char *)GetAppData(1);
    if ( common ) {
        ckmakmsg(dir,CKMAXPATH+1,(char *)common, "Kermit 95/", NULL, NULL);
        if ( !isdir(dir) ) {
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)common, "Kermit 95/SSH/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)common, "Kermit 95/CERTS/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)common, "Kermit 95/CRLS/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)common, "Kermit 95/KEYMAPS/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)common, "Kermit 95/PHONES/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)common, "Kermit 95/SCRIPTS/", NULL, NULL);
            zmkdir(dir);
        }
    }
    appdata = (char *)GetAppData(0);
    if ( appdata ) {
        ckmakmsg(dir,CKMAXPATH+1,(char *)appdata, "Kermit 95/", NULL, NULL);
        if ( !isdir(dir) ) {
            zmkdir(dir);

            if ( common = (char *)GetAppData(1) ) {
                ckmakmsg(file,CKMAXPATH+1,(char *)common, "Kermit 95/K95CUSTOM.INI", NULL, NULL);
                zcopy(file,dir);
                appdata = (char *)GetAppData(0);
            }

            ckmakmsg(dir,CKMAXPATH+1,(char *)appdata, "Kermit 95/SSH/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)appdata, "Kermit 95/CERTS/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)appdata, "Kermit 95/CRLS/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)appdata, "Kermit 95/KEYMAPS/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)appdata, "Kermit 95/PHONES/", NULL, NULL);
            zmkdir(dir);

            ckmakmsg(dir,CKMAXPATH+1,(char *)appdata, "Kermit 95/SCRIPTS/", NULL, NULL);
            zmkdir(dir);
        }

        personal = (char *)GetPersonal();
        if ( personal ) {
            ckmakmsg(dir,CKMAXPATH+1,(char *)personal, "DOWNLOAD/", NULL, NULL);
            if ( !isdir(dir) )
                zmkdir(dir);
        }
    }

}
#endif /* NT */

/*  S Y S I N I T  --  System-dependent program initialization.  */

int SysInited = 0;

int
sysinit() {
    extern char uidbuf[] ;
    extern struct ck_p ptab[] ;
    extern int priority;
    extern int tt_mouse ;
    extern int tt_modechg;
    char *ptr;
    int n;
    extern unsigned long startflags;
    extern int DeleteStartupFile;
    extern int StartedFromDialer;
    extern int fdispla;
    extern int initvik;
    extern int tt_type;

    /* we raise the priority here because when we are not in
        connect mode, there is only thread (this one) which
        performs any serial or Network I/O.  So this has to
        be a server priority thread.
        Upon entering connect mode we will lower the priority
        of this thread, and then raise upon exiting.  This
        should provide snappy ckermit performance without too
        much damage to other processes.  That is because this
        thread remains blocked most of time waiting on our
        keyboard (event) buffer semaphore.  So even though the
        KbdHandler is Ready (at regular priority), this thread
        will be blocked unless we are waiting for serial or
        network I/O.
    */
#ifdef NT
    int    WinThreadInit=0;
    DWORD mode ;
#ifndef NOTERM
    extern int tt_attr_bug ;
#endif /* NOTERM */

    connoi() ;
    ck_sleepint = isWin95() ? CK_SLEEPINT : CK_SLEEPINT * 2;
    SetFileApisToOEM() ;  /* Otherwise, filenames are translated */

    /* Allocate memory for COMMCONFIG structure */
    savedcfg = (LPCOMMCONFIG) malloc( 1024 );
    if ( savedcfg ) {
        memset( savedcfg, 0, 1024 );
        savedcfg->dwSize = 1024;
    }
    ttycfg = (LPCOMMCONFIG) malloc( 1024 );
    if (ttycfg) {
        memset( ttycfg, 0, 1024 );
        ttycfg->dwSize = 1024;
    }

#ifndef NOLOCAL
#ifndef KUI
    Win32ConsoleInit() ;            /* Initialize the Console, Stdin, Stdout */
#endif /* KUI */
#endif /* NT */
#endif /* NOLOCAL */

    /* Construct the system ID string */
#ifdef NT
    {
        OSVERSIONINFO osverinfo ;
        osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
        GetVersionEx( &osverinfo ) ;

        if ( startflags & 1 )
            OSVer = VER_PLATFORM_WIN32_NT;
        else
            OSVer = osverinfo.dwPlatformId ;

        sprintf(ckxsystem, " %s %1d.%02d(%1d)%s%s",
                 ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ?
                   (osverinfo.dwMinorVersion == 0 ? "Windows 95" : "Windows 98")  :
                   osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT ?
                   (osverinfo.dwMajorVersion < 5 ? "Windows NT" : "Windows 2000/XP") :
                   "Windows Unknown" ),
                 osverinfo.dwMajorVersion,
                 osverinfo.dwMinorVersion,
                 LOWORD(osverinfo.dwBuildNumber),
                 osverinfo.szCSDVersion && osverinfo.szCSDVersion[0] ? " " : "",
                 osverinfo.szCSDVersion ? osverinfo.szCSDVersion : "");
#ifdef CK_UTSNAME
        sprintf(unm_nam,
                 ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ?
                   (osverinfo.dwMinorVersion == 0 ? "Windows 95" : "Windows 98")  :
                   osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT ?
                   (osverinfo.dwMajorVersion < 5 ? "Windows NT" : "Windows 2000/XP") :
                   "Windows Unknown" ));
        sprintf(unm_rel,"%1d.%02d",
                 osverinfo.dwMajorVersion,
                 osverinfo.dwMinorVersion);
        sprintf(unm_ver,"(%1d)%s%s",
                LOWORD(osverinfo.dwBuildNumber),
                osverinfo.szCSDVersion && osverinfo.szCSDVersion[0] ? " " : "",
                osverinfo.szCSDVersion ? osverinfo.szCSDVersion : "");
#endif /* CK_UTSNAME */
#ifdef KUI
        InitCommonControls();
#endif /* KUI */
    }
#else /* NT */
    {
        ULONG StartIndex = QSV_VERSION_MAJOR ; /* Major Version Number */
        ULONG EndIndex   = QSV_VERSION_REVISION ; /* Revision Letter      */
        ULONG DataBuf[3] ;
        ULONG DataBufLen = 3 * sizeof(ULONG) ;

        DosQuerySysInfo( StartIndex,
                             EndIndex,
                             DataBuf,
                             DataBufLen );
        sprintf(ckxsystem, " %s OS/2 %1d.%02d", ckxrev, _osmajor / 10, _osminor);
#ifdef CK_UTSNAME
        sprintf(unm_nam,"OS/2");
        sprintf(unm_rel,"%1d.%02d", _osmajor / 10, _osminor);
        sprintf(unm_ver,"%c",DataBuf[2]);
#endif /* CK_UTSNAME */
    }
#endif /* NT */
#ifdef CK_UTSNAME
    sprintf(unm_mch,getcpu());
#endif /* CK_UTSNAME */
    debug(F110,"Operating System",ckxsystem,0);

#ifdef NT
    setlocale(LC_ALL, "" );             /* Import locale from OS */
#endif /* NT */

    if ( StartedFromDialer )            /* We delete the cmdfile by default  */
        DeleteStartupFile = 1;          /* only when started from the dialer */

    k95stdin = (startflags & 32) ? 1 : 0;
    k95stdout = (startflags & 64) ? 1 : 0;

    if ( k95stdout ) {
        extern int interm;
        interm = 0;                     /* SET INPUT TERMINAL OFF */
    }

    if ( k95stdin ) {
#ifdef NT
        _setmode(_fileno(stdin),_O_BINARY);
#else
        setmode(fileno(stdin),_O_BINARY);
#endif /* NT */
    }
    if ( k95stdout ) {
#ifdef NT
        _setmode(_fileno(stdout),_O_TEXT);
#else
        setmode(fileno(stdout),_O_TEXT);
#endif /* NT */
        setbuf(stdout,NULL);
        fdispla = XYFD_S;
    }
#ifdef OS2ONLY
    if ( IsOS2FullScreen() )
    {
        extern int trueunderline;
        extern int trueblink;
        trueunderline = FALSE;
        trueblink   = FALSE;
    }
#endif /* OS2ONLY */

#ifdef NTSIG
    TlsIndex = TlsAlloc();
#endif /* NTSIG */

    CreateAlarmMutex( FALSE ) ;
#ifndef NOLOCAL
    CreateScreenMutex( FALSE ) ;
    CreateVscrnMutex( FALSE ) ;
    CreateVscrnDirtyMutex( FALSE );
    CreateConnectModeMutex( FALSE ) ;
#endif /* NOLOCAL */
    CreateThreadMgmtMutex( FALSE );
    CreateTCPIPMutex( FALSE );
    CreateDebugMutex( FALSE ) ;
    CreateTelnetMutex( FALSE ) ;
    CreateCommMutex( FALSE );
#ifdef CK_SSL
    CreateSSLMutex( FALSE );
#endif /* CK_SSL */
    CreateAlarmSem( FALSE ) ;
#ifndef NOLOCAL
    CreateCommandModeSem( TRUE );
    CreateTerminalModeSem( FALSE );
    CreateAlarmSem( FALSE ) ;
    CreateRdComWrtScrThreadSem( FALSE ) ;
    CreateTermScrnUpdThreadSem( FALSE ) ;
    CreateConKbdHandlerThreadSem( FALSE ) ;
    CreateRdComWrtScrThreadDownSem( FALSE ) ;
    CreateTermScrnUpdThreadDownSem( FALSE ) ;
    CreateConKbdHandlerThreadDownSem( FALSE ) ;
    CreateKeyMapInitSem( FALSE ) ;
    CreateVscrnDirtySem( TRUE );
#endif /* NOLOCAL */

#ifndef NOSETKEY
    keymapinit();                       /* Initialize key maps */
    keynaminit();                       /* Initialize key names */
#endif /* NOSETKEY */
#ifdef OS2MOUSE
    mousemapinit(-1,-1);                /* Initialize mouse maps */
#endif /* OS2MOUSE */
#ifndef NOKVERBS
    initvik = 1;                        /* Remember that we did */
#endif /* NOKVERBS */

#ifdef NT
#ifndef NOTERM
    tt_attr_bug = isWin95();
#endif /* NOTERM */
#ifndef KUI
    hInstance = GetModuleHandle(NULL) ;
    debug(F101,"hInstance","",hInstance);
    hwndConsole = GetConsoleHwnd() ;
    if ( isWin95() )
    {
        MENUITEMINFO info;
        HMENU hMenu;

        hMenu = GetSystemMenu(hwndConsole, FALSE);
        info.cbSize = sizeof(info);
        RemoveMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
        DrawMenuBar(hwndConsole);
        CloseHandle(hMenu);
    }
#endif /* KUI */
    WinThreadInit = WindowThreadInit( (void *) hInstance );
#endif /* NT */

#ifdef OS2ONLY
    hab = WinInitialize(0);
    debug ( F101, "WinInitialize hab","",hab) ;
    ConnectToPM();
#endif /* OS2ONLY */

    pid = os2getpid() ;
    DisableHardErrors() ;

    le_init();
    alarm(0) ;          /* We must start the alarm thread */

    keybufinit() ;      /* Must come before the KbdHandler */

#ifndef NOLOCAL
#ifndef KUI
#ifdef IKSD
    if ( !inserver )
#endif /* IKSD */
    KbdHandlerInit() ;
#endif /* KUI */
#endif /* NOLOCAL */

#ifdef NT
    CreateSerialMutex( FALSE ) ;
#ifndef KUI
#ifdef COMMENT
    VioHandle = GetStdHandle( STD_OUTPUT_HANDLE ) ;
#else
    if ( !k95stdout )
    VioHandle = CreateFile( "CONOUT$", GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       0) ;
#endif /* COMMENT */
#endif /* KUI */
    DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),
                    GetCurrentProcess(),&tidCommand,
                    0,FALSE, DUPLICATE_SAME_ACCESS );
    debug(F101,"Command Thread Handle","",tidCommand);
    CommandID = GetCurrentThreadId() ;
    debug(F101,"Command Thread ID","",CommandID) ;
#endif /* NT */
#ifndef NOLOCAL
#ifndef KUI
    if ( !k95stdout ) {
#ifdef NT
        CK_VIDEOMODEINFO mi;
#endif /* NT */
        GetMode( &StartupConsoleModeInfo );
#ifdef NT
        if ( StartupConsoleModeInfo.col != StartupConsoleModeInfo.sbcol ||
             StartupConsoleModeInfo.row != StartupConsoleModeInfo.sbrow ) {
            memcpy(&mi, &StartupConsoleModeInfo, sizeof(CK_VIDEOMODEINFO));
            mi.sbcol = mi.col;
            mi.sbrow = mi.row;
            SetMode( &mi );
        }
#endif /* NT */
    }
#endif /* KUI */
#ifndef NT
    OpenClipboardServer() ;
#endif /* NT */

    getcmdcolor();
    os2gettitle(szOldTitle, sizeof(szOldTitle));
    debug(F110,"sysinit szOldTitle",szOldTitle,0);
    os2settitle("", TRUE);
#endif /* NOLOCAL */

    nOldCP = os2getcp();

    switch (nOldCP) {
    case 437:
        tcsl = TX_CP437 ;
        fcharset = FC_CP437;
        break;
    case 850:
        tcsl = TX_CP850;
        fcharset = FC_CP850;
        break;
    case 858:
        tcsl = TX_CP858;
        fcharset = FC_CP850;
        break;
    case 855:
        tcsl = TX_CP855;
        break;
    case 856:
        tcsl = TX_CP856;
        break;
    case 852:
        tcsl = TX_CP852;
        fcharset = FC_CP852;
        break;
    case 862:
        tcsl = TX_CP862;
        fcharset = FC_CP862;
        break;
    case 866:
        tcsl = TX_CP866;
        fcharset = FC_CP866;
        break;
    case 1004:  /* Windows 3.1 - supposed to be Latin-1 */
        tcsl = TX_8859_1;
        fcharset = FC_1LATIN;
        break;
    case 1252:  /* Win32 - like Latin-1 */
        tcsl = TX_CP1252;
        fcharset = FC_1LATIN;
        break;
#ifdef KUI
    default:
        tcsl = TX_CP1252;
        fcharset = FC_1LATIN;
#else
    default:
        tcsl = TX_CP437 ;
        fcharset = FC_CP437;
#endif
    }


#ifdef __IBMC__
    setvbuf(stdout, NULL, _IONBF, 0);
    setmode(1, _O_TEXT);
#endif /* __IBMC__ */

#ifdef NT
    {
        char * p = GetLoadPath();
        DWORD len;

        len = GetShortPathName(p,exedir,CKMAXPATH);
        if ( len == 0 || len > CKMAXPATH )
            ckstrncpy(exedir, p, CKMAXPATH);
    }
#else
    ckstrncpy(exedir, GetLoadPath(), CKMAXPATH);
#endif /* NT */
    if ( (ptr = strrchr(exedir, '\\')) != NULL )
        *ptr = 0;
    for (ptr = exedir; *ptr; ptr++)     /* Convert backslashes to slashes */
        if (*ptr == '\\')
            *ptr = '/';
    n = (int)strlen(exedir);            /* Add slash to end if necessary */
    if (n > -1 && n < CKMAXPATH) {
        if (exedir[n-1] != '/') {
            exedir[n] = '/';
            exedir[n+1] = '\0';
        }
    }
#ifdef NT
    {
        char * p = zgtdir();
        DWORD len;

        len = GetShortPathName(p,startupdir,CKMAXPATH);
        if ( len == 0 || len > CKMAXPATH )
            ckstrncpy(startupdir, p, CKMAXPATH);
    }
#else
    ckstrncpy(startupdir, zgtdir(), CKMAXPATH);
#endif /* NT */
    for (ptr = startupdir; *ptr; ptr++) { /* Convert backslashes to slashes */
        if (*ptr == '\\')
            *ptr = '/';
    }
    n = (int)strlen(startupdir);        /* Add slash to end if necessary */
    if (n > -1 && n < CKMAXPATH) {
        if (startupdir[n-1] != '/') {
            startupdir[n] = '/';
            startupdir[n+1] = '\0';
        }
    }
    ckstrncpy(ttnmsv, dftty, DEVNAMLEN+1);
    islocal = is_a_tty(0) && !ttiscom(0);
    if (!islocal) {
       os2setdtr(1);
       ttsettings(dfprty,stopbits);
       ttsetflow(flow);
       os2settimo(0,0);
       os2setcarr(ttcarr == CAR_ON);
    }
    else
       concooked();                     /* Initialize keyboard */
#ifndef NOLOCAL
    if ( !k95stdout ) {
        extern int row_init, col_init;
        ttgcwsz() ;
        if ( !row_init && !col_init )
            ttgwsiz() ;
    }

    debug(F100,"about to VscrnInit()","",0);
    /* Setup the Virtual Screens */
    VscrnInit( VCMD ) ;
    VscrnInit( VTERM ) ;
    /* VscrnInit( VCS ) ; */
    VscrnInit( VSTATUS ) ;
    settermtype(tt_type,1);
#endif /* NOLOCAL */
#ifdef KUI
    /* This does more than just display the Splash Screen (which is good as it
     * doesn't do that anymore)*/
    KuiInit( (void *) hInstance, &kui_init );
    hwndConsole = KuiThreadInit( (void *) hInstance ) ;
#endif /* KUI */

#ifdef CK_REXX
    os2rexxinit() ;
#endif /* CK_REXX */

#ifdef CK_SECURITY
    ck_security_loaddll();
#endif /* CK_SECURITY */

#ifdef CK_TAPI
    debug(F100,"Initializing TAPI","",0);
    if ( WinThreadInit && !(startflags & 4)) {
        /* TAPI requires a Window Handle */
        TAPIAvail = cktapiinit() ;
        if ( TAPIAvail ) {
            cktapiopen();
#ifndef NODIAL 
            CopyTapiLocationInfoToKermitDialCmd();
#endif /* NODIAL */
        }
    }
#endif /* CK_TAPI */

#ifdef CK_XYZ
#ifdef XYZ_DLL
    if ( !(startflags & 16) ) {
        debug(F100,"Initializing P_DLL","",0);
        p_avail = !load_p_dll() ;
        if ( !p_avail )
        {
            bleep(BP_WARN);
#ifdef NT
            printf("Unable to load P95.DLL - X,Y,Zmodem is unavailable\n");
#else /* NT */
            printf("Unable to load P2.DLL - X,Y,Zmodem is unavailable\n");
#endif /* NT */
        }
    }
#endif /* XYZ_DLL */
#endif /* CK_XYZ */

#ifndef NOLOCAL
#ifndef KUI
#ifdef ONETERMUPD
    if ( !(inserver && k95stdout) )
        tidTermScrnUpd = (TID) ckThreadBegin( &TermScrnUpd,
                                          THRDSTKSIZ, 0, TRUE, 0 ) ;
#endif /* ONETERMUPD */
#endif /* KUI */
#endif /* NOLOCAL */
#ifndef NOTERM
    updanswerbk();
#endif /* NOTERM */

    SetThreadPrty(priority,0);

#ifdef NT
#ifndef KUI
    debug(F100,"About to SetConsoleCtrlHandler","",0);
    if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CloseHandler, TRUE ))
        debug(F100,"SetConsoleCtrlHandler succeeded","",0);
    else
        debug(F101,"SetConsoleCtrlHandler failed","",GetLastError());
#endif /* KUI */
#endif /* NT */

#ifndef KUI
#ifdef OS2MOUSE
    if (tt_mouse)
        os2_mouseon() ;
#endif /* OS2MOUSE */
#endif /* KUI */

    if ( quitonbreak )
        signal(SIGBREAK,term_signal);
    signal(SIGTERM,term_signal);

#ifdef IKSD
    if ( !inserver )
#endif /* IKSD */
#ifndef NOLOCAL
    StartConnectThreads();
#endif /* NOLOCAL */

#ifdef CK_ENVIRONMENT
    {
        extern char tn_env_acct[], tn_env_disp[], tn_env_job[],
        tn_env_prnt[], tn_env_sys[], uidbuf[];
        char *p;
        if (p = getenv("USER")) {
            ckstrncpy(uidbuf,p,UIDBUFLEN);
        }
        if (p = getenv("JOB")) {
            ckstrncpy(tn_env_job,p,64);
        }
        if (p = getenv("ACCT")) {
            ckstrncpy(tn_env_acct,p,64);
        }
        if (p = getenv("PRINTER")) {
            ckstrncpy(tn_env_prnt,p,64);
        }
        if (p = getenv("DISPLAY")) {
            ckstrncpy(tn_env_disp,p,64);
        }
#ifdef NT
        strcpy(tn_env_sys,"WIN32");
#else
        strcpy(tn_env_sys,"OS2");
#endif /* NT */
    }
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    {
        extern char * tn_loc;
        char *p;
        if (p = getenv("LOCATION"))
          makestr(&tn_loc,p);
    }
#endif /* CK_SNDLOC */
#ifndef NOLOCAL
    os2InitFromRegistry();
#endif /* NOLOCAL */
#ifdef NT
    if ( isWin95() ) {
#ifdef BETADEBUG
        Win95DisplayLocale();
#endif /* BETADEBUG */
        Win95AltGrInit();
    }
#endif /* NT */

#ifdef NETCONN
    netinit();
#endif /* NETCONN */

#ifdef IKSD
    if ( !inserver )
#endif /* ISKD */
        ckstrncpy( uidbuf, GetLocalUser(), UIDBUFLEN );

#ifdef NT
#ifdef BETADEBUG
    printf("Console Window Handle=0x%x\n",hwndConsole);
    printf("WM_GETFONT=0x%x\n",(ULONG)SendMessage(hwndConsole,WM_GETFONT,0,0));
#endif /* BETADEBUG */
#endif /* NT */

#ifdef OS2ONLY
    if (getenv ("TZ") == NULL)  /* Default New York time. */
    {
        printf("Warning: TZ environment variable not set.  Using EST5EDT.\n\n");
        bleep(BP_WARN);
        _timezone = 18000;
        _daylight = 1;
        _tzname[0] = "EST";
        _tzname[1] = "EDT";
    }
    else
#endif /* OS2ONLY */
        _tzset() ;

#ifdef NT
    if ( isWin95() ) {                  /* Check TMP,TEMP,K95TMP */
        char * p = NULL;
        int warning = 0;

        p = getenv("K95TMP");
        if ( p ) {
            if ( strlen(p) == 3 && p[1] == ':' && p[2] == '\\' ) {
                printf("\n  WARNING: K95TMP = %s\n",p);
                warning = 1;
            }
        }
        p = getenv("TEMP");
        if ( p ) {
            if ( strlen(p) == 3 && p[1] == ':' && p[2] == '\\' ) {
                if ( !warning )
                    printf("\n");
                printf("  WARNING: TEMP = %s\n",p);
                warning = 1;
            }
        }
        p = getenv("TMP");
        if ( p ) {
            if ( strlen(p) == 3 && p[1] == ':' && p[2] == '\\' ) {
                if ( !warning )
                    printf("\n");
                printf("  WARNING: TMP = %s\n",p);
                warning = 1;
            }
        }
        if ( warning ) {
            printf("  The directory for temporary files in Windows 95 must be a subdirectory\n");
            printf("  to avoid problems with Kerberos, Tapi, and other subsystems.\n\n");
            bleep(BP_WARN);
        }
    }

    /* Check to make sure we have the default directory heirarchy */
    if ( !inserver )
        firsttime();
#endif /* NT */

#ifndef NOLOCAL
    VscrnForceFullUpdate();             /* Just in case command screen did not write */
#endif /* NOLOCAL */
    SysInited = 1;
	debug(F100,"sysinit complete","",0);

    return(0);
}

/*  S Y S C L E A N U P  --  System-dependent program cleanup.  */

int
syscleanup() {
    extern int printtimo;

    if (!SysInited)
      return(0);

    SysInited = 0;

    printtimo = 0;                      /* Turn off delayed print closure */
    printeroff();                       /* Close printer if open */

#ifdef NT
#ifdef NEWRDCH
    OverlappedReadCleanup();
#endif /* NEWRDCH */
#endif /* NT */

#ifdef COMMENt
#ifndef NOLOCAL
    StopConnectThreads(0);
#endif /* NOLOCAL */
#endif /* COMMENT */
#ifdef NT
#ifdef CK_TAPI
    if ( TAPIAvail ) {
        debug(F100,"Close TAPI starting","",0);
        cktapiclose();
        cktapiunload() ;
        debug(F100,"Close TAPI done","",0);
    }
#endif /* CK_TAPI */
#ifndef KUI
    WindowThreadClose();
#endif /* KUI */
#endif /* NT */

#ifdef KUI
    KuiThreadCleanup( (void *) hInstance ) ;
#endif /* KUI */
#ifdef CK_SECURITY
    ck_security_unloaddll();
#endif /* CK_SECURITY */
#ifdef NETCONN
    netcleanup() ;
#endif /* NETCONN */
    os2settitle(szOldTitle,FALSE);
    os2setcp(nOldCP);

    signal(SIGINT, SIG_DFL);
    signal(SIGBREAK, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

#ifndef KUI
#ifndef NOLOCAL
    if ( !k95stdout ) {
#ifdef NT
        if ( StartupConsoleModeInfo.sbrow != VscrnGetHeight(VCMD) ||
             StartupConsoleModeInfo.sbcol != VscrnGetWidth(VCMD) ) {
            clearcmdscreen();
            SetMode( &StartupConsoleModeInfo );
        }
#else /* NT */
        if ( StartupConsoleModeInfo.row != VscrnGetHeight(VCMD) ||
             StartupConsoleModeInfo.col != VscrnGetWidth(VCMD) ) {
            clearcmdscreen();
            SetMode( &StartupConsoleModeInfo );
        }
#endif /* NT */
    }
    KbdHandlerCleanup() ;   /* must come after os2settitle */
    debug(F100,"KbdHandlerCleanup done","",0);
    if ( isWin95() )            /* Win95 needs time to let the kbdthread die */
        msleep(50);

    keybufcleanup() ;
    debug(F100,"keybufcleanup done","",0);
#endif /* NOLOCAL */

    le_clean();

    CloseAlarmMutex() ;
#ifndef NOLOCAL
    CloseScreenMutex() ;
    CloseVscrnMutex() ;
    CloseVscrnDirtyMutex() ;
    CloseConnectModeMutex() ;
    CloseCommandModeSem() ;
    CloseTerminalModeSem() ;
#endif /* NOLOCAL */
    CloseCommMutex();
    CloseThreadMgmtMutex();
    CloseDebugMutex() ;
    CloseTelnetMutex() ;
    CloseTCPIPMutex() ;
    CloseAlarmSem() ;
#ifdef CK_SSL
    CloseSSLMutex() ;
#endif /* CK_SSL */
#ifndef NOLOCAL
    CloseRdComWrtScrThreadSem() ;
    CloseConKbdHandlerThreadSem() ;
    CloseTermScrnUpdThreadSem() ;
    CloseRdComWrtScrThreadDownSem() ;
    CloseConKbdHandlerThreadDownSem() ;
    CloseTermScrnUpdThreadDownSem() ;
    CloseKeyMapInitSem() ;
    CloseVscrnDirtySem() ;
#endif /* NOLOCAL */
#endif /* KUI */
#ifdef NT
    CloseSerialMutex() ;
    if ( !stdout )
        CloseHandle(VioHandle);
    VioHandle = 0 ;
#endif /* NT */
    CloseThreadMgmtMutex() ;
    debug(F100,"Close Mutexes and Semaphores done","",0);

#ifndef NOLOCAL
#ifndef NT
    CloseClipboardServer() ;
    debug(F100,"Close Clipboard done","",0);
#endif /* NT */
#endif /* NOLOCAL */

#ifdef NTSIG
    TlsFree(TlsIndex);
    TlsIndex = 0;
    debug(F100,"Free Thread Local Memory done","",0);
#endif /* NTSIG */

#ifdef OS2ONLY
    WinTerminate(hab) ;
    DisconnectFromPM();
#endif /* OS2ONLY */

#ifdef CK_XYZ
#ifdef XYZ_DLL
   if ( p_avail )
      unload_p_dll() ;
    debug(F100,"Zmodem released","",0);
#endif /* XYZ_DLL */
#endif /* CK_XYZ */
    return(0);
}

/* Timeout handler for communication line input functions */

static ckjmpbuf kbbuf;                  /* Timeout longjmp targets */
static ckjmpbuf sjbuf;

#ifndef __EMX__
unsigned alarm(unsigned);               /* Prototype */
#endif /* __EMX__ */
#ifdef OS2
SIGTYP (* volatile saval)(int) = NULL;  /* For saving alarm() handler */
#else /* OS2 */
SIGTYP (*saval)(int) = NULL;            /* For saving alarm() handler */
#endif /* OS2 */

VOID
ttimoff() {                             /* Turn off any timer interrupts */
    int xx;
    xx = alarm(0);
    if (saval) {                        /* Restore any previous */
        signal(SIGALRM,saval);          /* alarm handler. */
        saval = NULL;
    } else {
        signal(SIGALRM,SIG_IGN);
    }
}


/* O S 2 S E T T I M O -- set read and write timeouts */

int
os2settimo(int spd, int modem)
{
#ifdef NT
    COMMTIMEOUTS timeouts ;
#endif /* NT */

    debug(F111,"os2settimo","spd",spd);
    debug(F111,"os2settimo","modem",modem);

#ifdef NT
    /* Calculate the max overlapeed writes for this connection */
    /* A rethinking of how this number is calculated.
     * The fastest modem connection is 53,000 bits per second (6625 bytes per second)
     * the largest kermit packet is 9024 bytes.  Therefore, we can never process more
     * than one write/second therefore it makes no sense for this value to be anything
     * but 2 for modems.  For non-modems, use bytes/sec/max-packet 
     */
    if ( modem > 0 )
        maxow = 2;
    else
        maxow = 1 + (spd / 8 / 9024) ;

    if ( maxow > maxow_usr )
        maxow = maxow_usr;

#ifdef CK_TAPI
    if ( tttapi && !tapipass ) {
        HANDLE hModem = NULL;
        LPDEVCFG        lpDevCfg = NULL;
        LPCOMMCONFIG    lpCommConfig = NULL;
        LPMODEMSETTINGS lpModemSettings = NULL;
        DCB *           lpDCB = NULL;

        hModem = GetModemHandleFromLine( (HLINE) 0 );
        if ( hModem == NULL )
            return(-1);

        if (!GetCommTimeouts( hModem, &timeouts ))
        {
            CloseHandle( hModem );
            return -1 ;
        }

        /* Timeouts for Overlapped I/O from MS TTY.C example */
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = MAXDWORD /* 0 */ ;
        timeouts.ReadTotalTimeoutConstant = 1 ;
        timeouts.WriteTotalTimeoutMultiplier = (modem > 0 ? 250 : (2500000 / spd));
        timeouts.WriteTotalTimeoutConstant =  (modem > 0 ? 90000 : 0);

        if (!SetCommTimeouts( hModem, &timeouts )) {
            CloseHandle( hModem );
            return -1 ;
        }
        CloseHandle(hModem);
        return(0);
    }
#endif /* CK_TAPI */
    if (!GetCommTimeouts( (HANDLE) ttyfd, &timeouts ))
        return -1 ;

    if ( deblog )
        debugComm( "os2settimo initial values", NULL, &timeouts );

    /* Timeouts for Overlapped I/O from MS TTY.C example */
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = MAXDWORD /* 0 */;
    timeouts.ReadTotalTimeoutConstant = 1  ;
    timeouts.WriteTotalTimeoutMultiplier = (modem > 0 ? 250 : (2500000 / spd));
    timeouts.WriteTotalTimeoutConstant =  (modem > 0 ? 90000 : 0);

    debug(F111,"os2settimo SetCommTimeouts","WriteTotalTimeoutMultiplier",
              timeouts.WriteTotalTimeoutMultiplier);
    debug(F111,"os2settimo SetCommTimeouts","WriteTotalTimeoutConstant",
              timeouts.WriteTotalTimeoutConstant);
    if (!SetCommTimeouts( (HANDLE) ttyfd, &timeouts ))
        return -1 ;
#else /* not NT */
    /* Get the current settings */
    if (DosDevIOCtl(&ttydcb,sizeof(ttydcb),NULL,0,
                     ASYNC_GETDCBINFO,IOCTL_ASYNC,ttyfd))
        return(-1);

    /* set write timeout */
    ttydcb.fbTimeout &= ~MODE_NO_WRITE_TIMEOUT;
#ifdef COMMENT
    ttydcb.usWriteTimeout = 15 * 100;   /* 15-second timeout */
#else
    ttydcb.usWriteTimeout = ((100 / spd) + 1) * MAXSP
        + (modem > 0 ? 60 * 100 : 0);
#endif /* COMMENT */

    /* Read "some" data from line mode */
    ttydcb.fbTimeout &= ~MODE_NOWAIT_READ_TIMEOUT;
    ttydcb.fbTimeout |= MODE_WAIT_READ_TIMEOUT;

    /* Set DCB */
    if (DosDevIOCtl(NULL,0,&ttydcb,sizeof(ttydcb),
                     ASYNC_SETDCBINFO,IOCTL_ASYNC,ttyfd))
        return(-1);
#endif /* NT */
    return(0);
}

/*  T T S E T F L O W -- set flow state of tty */

int
ttsetflow(int nflow) {
#ifdef NT
   DWORD mode ;
#endif /* NT */

    debug(F101,"setflow","",nflow) ;

#ifdef TN_COMPORT
    if (network)
        return(tnsetflow(nflow));
#endif /* TN_COMPORT */

#ifdef NT
#ifdef CK_TAPI
    if ( tttapi && !tapipass ) {
        HANDLE hModem = NULL;
        LPDEVCFG        lpDevCfg = NULL;
        LPCOMMCONFIG    lpCommConfig = NULL;
        LPMODEMSETTINGS lpModemSettings = NULL;
        DCB *           lpDCB = NULL;

        hModem = GetModemHandleFromLine( (HLINE) 0 );
        if ( hModem == NULL )
            return(-1);

        if (!cktapiGetModemSettings(&lpDevCfg,&lpModemSettings,
                                     &lpCommConfig,&lpDCB))
        {
            CloseHandle( hModem );
            return -1 ;
        }

        if ( deblog )
            debugComm( "ttsetflow initial values", lpDCB, NULL );

        lpDCB->fDsrSensitivity = FALSE ;
        lpDCB->fDtrControl = DTR_CONTROL_ENABLE ;

        switch(nflow) {
        case FLO_XONX:
            lpDCB->fOutX = TRUE ;
            lpDCB->fInX = TRUE ;
            lpDCB->fRtsControl = RTS_CONTROL_ENABLE ;
            lpDCB->fOutxCtsFlow = FALSE ;
            lpDCB->fTXContinueOnXoff = /* TRUE */ FALSE ;
            break;
        case FLO_RTSC:
            lpDCB->fOutX = FALSE ;
            lpDCB->fInX = FALSE ;
            lpDCB->fRtsControl = RTS_CONTROL_HANDSHAKE ;
            lpDCB->fOutxCtsFlow = TRUE ;
            lpDCB->fTXContinueOnXoff = TRUE ;
            break;
        case FLO_KEEP:
            /* leave things exactly as they are */
            break;
        case FLO_NONE:
            /* turn off all flow control completely */
            lpDCB->fOutX = FALSE ;
            lpDCB->fInX = FALSE ;
            lpDCB->fRtsControl = RTS_CONTROL_ENABLE ;
            lpDCB->fOutxCtsFlow = FALSE ;
            lpDCB->fTXContinueOnXoff = TRUE ;
            break;
        }

        lpDCB->XonChar = 0x11 ;
        lpDCB->XoffChar = 0x13;
        lpDCB->XonLim = 10 ;
        lpDCB->XoffLim = 10 ;

        if (!cktapiSetModemSettings(lpDevCfg,lpCommConfig)) {
            CloseHandle( hModem );
            return -1 ;
        }

        if ( lpDCB->fRtsControl != RTS_CONTROL_HANDSHAKE )
            EscapeCommFunction( hModem, SETRTS ) ;

        CloseHandle(hModem);
        return(0);
    }
#endif /* CK_TAPI */
    ttydcb.DCBlength = sizeof(DCB);

    if (!GetCommState( (HANDLE) ttyfd, &ttydcb ))
        return -1 ;

    if ( deblog )
        debugComm( "ttsetflow initial values", &ttydcb, NULL );

    ttydcb.fDsrSensitivity = FALSE ;
    ttydcb.fDtrControl = DTR_CONTROL_ENABLE ;

    switch(nflow) {
    case FLO_XONX:
        ttydcb.fOutX = TRUE ;
        ttydcb.fInX = TRUE ;
        ttydcb.fRtsControl = RTS_CONTROL_ENABLE ;
        ttydcb.fOutxCtsFlow = FALSE ;
        ttydcb.fTXContinueOnXoff = /* TRUE */ FALSE ;
        break;
    case FLO_RTSC:
        ttydcb.fOutX = FALSE ;
        ttydcb.fInX = FALSE ;
        ttydcb.fRtsControl = RTS_CONTROL_HANDSHAKE ;
        ttydcb.fOutxCtsFlow = TRUE ;
        ttydcb.fTXContinueOnXoff = TRUE ;
        break;
    case FLO_KEEP:
        /* leave things exactly as they are */
        break;
    case FLO_NONE:
        /* turn off all flow control completely */
        ttydcb.fOutX = FALSE ;
        ttydcb.fInX = FALSE ;
        ttydcb.fRtsControl = RTS_CONTROL_ENABLE ;
        ttydcb.fOutxCtsFlow = FALSE ;
        ttydcb.fTXContinueOnXoff = TRUE ;
        break;
    }

    ttydcb.XonChar = 0x11 ;
    ttydcb.XoffChar = 0x13;
    ttydcb.XonLim = 10 ;
    ttydcb.XoffLim = 10 ;

    if (!SetCommState( (HANDLE) ttyfd, &ttydcb ))
        return -1 ;

    if ( ttydcb.fRtsControl != RTS_CONTROL_HANDSHAKE )
        EscapeCommFunction( (HANDLE) ttyfd, SETRTS ) ;
#else /* not NT */
    /* Get the current settings */
    if (DosDevIOCtl(&ttydcb,sizeof(ttydcb),NULL,0,
                     ASYNC_GETDCBINFO,IOCTL_ASYNC,ttyfd))
        return(-1);

    ttydcb.fbCtlHndShake = MODE_DTR_CONTROL;
    ttydcb.fbFlowReplace &= ~(MODE_AUTO_RECEIVE | MODE_AUTO_TRANSMIT |
    /* clear only a few */    MODE_RTS_CONTROL  | MODE_RTS_HANDSHAKE);

    if (nflow == FLO_XONX) {
        ttydcb.fbFlowReplace |=
            (MODE_AUTO_RECEIVE | MODE_AUTO_TRANSMIT | MODE_RTS_CONTROL);
    }
    else if (nflow == FLO_RTSC) {
        ttydcb.fbCtlHndShake |= MODE_CTS_HANDSHAKE;
        ttydcb.fbFlowReplace |= MODE_RTS_HANDSHAKE;
    }
    else if ( nflow != FLO_KEEP ) {
        ttydcb.fbFlowReplace |= MODE_RTS_CONTROL;
    }

    /* Set DCB */
    if (DosDevIOCtl(NULL,0,&ttydcb,sizeof(ttydcb),
                     ASYNC_SETDCBINFO,IOCTL_ASYNC,ttyfd))
        return(-1);

    if (nflow != FLO_RTSC && nflow != FLO_KEEP) {/* keep RTS permanently on */
        MODEMSTATUS ms;
        UINT data;
        ms.fbModemOn = RTS_ON;
        ms.fbModemOff = 255;
        DosDevIOCtl(&data,sizeof(data),&ms,sizeof(ms),
                    ASYNC_SETMODEMCTRL,IOCTL_ASYNC,ttyfd);
    }
#endif /* NT */
    return(0);
}

static int
os2setcarr(int ncarr) {
    debug(F101,"setcarr","",ncarr) ;
#ifdef NT
#ifdef COMMENT
    ttydcb.DCBlength = sizeof(DCB);
    if (!GetCommState( (HANDLE) ttyfd, &ttydcb ))
        return -1 ;

    if ( deblog )
        debugComm( "os2setcarr initial values", &ttydcb, NULL );

   if (!SetCommState( (HANDLE) ttyfd, &ttydcb ))
        return -1 ;
#endif /* COMMENT */
#else /* not NT */
    /* Get the current settings */
    if (DosDevIOCtl(&ttydcb,sizeof(ttydcb),NULL,0,
                    ASYNC_GETDCBINFO,IOCTL_ASYNC,ttyfd))
        return(-1);

    if (ncarr)
      ttydcb.fbCtlHndShake |=  MODE_DCD_HANDSHAKE;
    else
      ttydcb.fbCtlHndShake &= ~MODE_DCD_HANDSHAKE;

    /* Set DCB */
    if (DosDevIOCtl(NULL,0,&ttydcb,sizeof(ttydcb),
                    ASYNC_SETDCBINFO,IOCTL_ASYNC,ttyfd))
        return(-1);
#endif
    return(0);
}


/*  O S 2 S E T D T R -- set state of DTR signal */

static int
os2setdtr(int on) {
#ifdef NT
   DWORD mode = 0;
#else /* NT */
    MODEMSTATUS ms;
    UINT data;
#endif /* NT */

    debug(F101,"setdtr","",on);

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return(0);

#ifdef TN_COMPORT
    if (network) {
        if (istncomport()) {
            return(tnc_set_dtr_state(on));
        } else
            return(-1);
    }
#endif /* TN_COMPORT */

#ifdef NT
#ifdef CK_TAPI
    if ( tttapi && !tapipass )
    {
        HANDLE hModem = NULL;

        hModem = GetModemHandleFromLine( (HLINE) 0 );
        if ( hModem == NULL )
            return(-1);

        mode = on ? SETDTR : CLRDTR ;
        if (!EscapeCommFunction( hModem, mode ))
        {
            int error = GetLastError() ;
            debug(F101,"os2setdtr EscapeCommFunction failed","",error) ;
            CloseHandle( hModem );
            return -1 ;
        }

        CloseHandle( hModem );
        return(0);
    }
#endif /* CK_TAPI */

   mode = on ? SETDTR : CLRDTR ;
   if (!EscapeCommFunction( (HANDLE) ttyfd, mode ))
   {
      int error = GetLastError() ;
      debug(F101,"os2setdtr EscapeCommFunction failed","",error) ;
      return -1 ;
   }
   return 0;
#else /* NT */
    ms.fbModemOn = on ? DTR_ON : 0;
    ms.fbModemOff = on ? 255 : DTR_OFF;
    return(DosDevIOCtl(&data,sizeof(data),&ms,sizeof(ms),
                       ASYNC_SETMODEMCTRL,IOCTL_ASYNC,ttyfd));
#endif
}


/*  T T S E T T I N G S  --  Set the device driver parity and stop bits */

static int
ttsettings(int par, int stop) {
#ifdef OS2ONLY
    LINECONTROL lc;
#endif /* OS2ONLY */

#ifdef TN_COMPORT
    if (network)
        return(tnsettings(par,stop));
#endif /* TN_COMPORT */

#ifdef NT
#ifdef CK_TAPI
    if ( tttapi && !tapipass )
    {
        HANDLE hModem = NULL;
        LPDEVCFG        lpDevCfg = NULL;
        LPCOMMCONFIG    lpCommConfig = NULL;
        LPMODEMSETTINGS lpModemSettings = NULL;
        DCB *           lpDCB = NULL;

        hModem = GetModemHandleFromLine( (HLINE) 0 );
        if ( hModem == NULL )
            return(-1);

        if (!cktapiGetModemSettings(&lpDevCfg,&lpModemSettings,
                                     &lpCommConfig,&lpDCB)) {
            CloseHandle( hModem );
            return -1 ;
        }

        lpDCB->fBinary = TRUE ;
        lpDCB->fErrorChar = FALSE ;
        lpDCB->fAbortOnError = FALSE ;
        lpDCB->ErrorChar = '?' ;

        if (hwparity) {
            switch (hwparity) {
            case 'e':
                lpDCB->Parity = EVENPARITY ;
                break;
            case 'o':
                lpDCB->Parity = ODDPARITY ;
                break;
            case 'm':
                lpDCB->Parity = MARKPARITY ;
                break;
            case 's':
                lpDCB->Parity = SPACEPARITY ;
                break;
            default:
                lpDCB->Parity = NOPARITY ;
            }
            lpDCB->fParity = TRUE;
            lpDCB->ByteSize = 8;
            ttprty = 0;
        } else {
            lpDCB->Parity = NOPARITY;
            lpDCB->fParity = FALSE;
            lpDCB->ByteSize = 8;
            ttprty = par;
        }

        switch(stop) {
        case 2:
            lpDCB->StopBits = TWOSTOPBITS ;
            break;
        case 1:
            lpDCB->StopBits = ONESTOPBIT ;
            break;
        default:
            ; /* no change */
        }

        if (!cktapiSetModemSettings(lpDevCfg,lpCommConfig))  {
            CloseHandle( hModem );
            return -1 ;
        }

        CloseHandle( hModem );
        return (0);
    }
#endif /* CK_TAPI */

    ttydcb.DCBlength = sizeof(DCB);
    if (!GetCommState( (HANDLE) ttyfd, &ttydcb ))
        return -1 ;


    if ( deblog )
        debugComm( "ttsettings initial values", &ttydcb, NULL );

    ttydcb.fBinary = TRUE ;
    ttydcb.fErrorChar = FALSE ;
    ttydcb.fAbortOnError = FALSE ;
    ttydcb.ErrorChar = '?' ;

    if (hwparity) {
        switch (hwparity) {
        case 'e':
            ttydcb.Parity = EVENPARITY ;
            break;
        case 'o':
            ttydcb.Parity = ODDPARITY ;
            break;
        case 'm':
            ttydcb.Parity = MARKPARITY ;
            break;
        case 's':
            ttydcb.Parity = SPACEPARITY ;
            break;
        default:
            ttydcb.Parity = NOPARITY ;
        }
        ttydcb.fParity = TRUE;
        ttydcb.ByteSize = 8;
        ttprty = 0;
    } else {
        ttydcb.Parity = NOPARITY;
        ttydcb.fParity = FALSE;
        ttydcb.ByteSize = 8;
        ttprty = par;
    }

    switch(stop) {
    case 2:
        ttydcb.StopBits = TWOSTOPBITS ;
        break;
    case 1:
        ttydcb.StopBits = ONESTOPBIT ;
        break;
    default:
        ; /* no change */
    }

    if (!SetCommState( (HANDLE) ttyfd, &ttydcb ))
        return -1 ;

#else /* NT */
    if (DosDevIOCtl(&lc,sizeof(lc),NULL,0,
                    ASYNC_GETLINECTRL,IOCTL_ASYNC,ttyfd))
      return(-1); /* Get line */

#ifdef HWPARITY
    if (hwparity) {
        switch (hwparity) {
        case 'o':
            lc.bDataBits = 8;   /* Data bits */
            lc.bParity   = 1;
            break;
        case 'e':
            lc.bDataBits = 8;   /* Data bits */
            lc.bParity   = 2;
            break;
        case 'm':
            lc.bDataBits = 8;   /* Data bits */
            lc.bParity   = 3;
            break;
        case 's':
            lc.bDataBits = 8;   /* Data bits */
            lc.bParity   = 4;
            break;
        default :
            lc.bDataBits = 8;   /* Data bits */
            lc.bParity   = 0;   /* No parity */
        }
        ttprty = 0;
    } else
#endif /* HWPARITY */
    {
        /* Always let Kermit handle parity itself */
        lc.bDataBits = 8;   /* Data bits */
        lc.bParity   = 0;   /* No parity */
        ttprty = par;
    }
    switch (stop) {
        case 2:
            lc.bStopBits = 2;   /* Two stop bits */
            break;
        case 1:
            lc.bStopBits = 0;   /* One stop bit */
            break;
        default:                /* No change */
            break;
    }
    if (DosDevIOCtl(NULL,0,&lc,sizeof(lc),
                    ASYNC_SETLINECTRL,IOCTL_ASYNC,ttyfd))
      return(-1); /* Set line */
#endif /* NT */
    return(0);
}

void
ttname2title(char * ttname)
{                                       /* Set session title */
    char * p, name[72];                 /* in window list. */
    p = name ;
    ckstrncpy(name, ttname, 72);
    while (*p) {                        /* Uppercase it for emphasis. */
        if (islower(*p))
          *p = toupper(*p);
        p++;
    }
    os2settitle((char *) name, 1);
}

/*  T T O P E N  --  Open a tty for exclusive access.  */

/*  Returns 0 on success, -1 on failure.  */
/*
  If called with lcl < 0, sets value of lcl as follows:
  0: the terminal named by ttname is the job's controlling terminal.
  1: the terminal named by ttname is not the job's controlling terminal.
  But watch out: if a line is already open, or if requested line can't
  be opened, then lcl remains (and is returned as) -1.
*/
ttopen(char *ttname, int *lcl, int modem, int spare) {
    char *x;
    extern char* ttyname();
    int rc=0 ;
    U_INT action, res;
#ifdef NT
    int i ;
    SECURITY_ATTRIBUTES security ;
    char portname[267];
#endif

    debug(F111,"ttopen DEVNAMLEN","",DEVNAMLEN);
    debug(F111,"ttopen entry modem",ttname,modem);
    debug(F101,"ttopen ttyfd","",ttyfd);
    debug(F111,"ttopen ttnmsv",ttnmsv,(int) strlen(ttnmsv));
    debug(F111,"ttopen ttname",ttname,(int) strlen(ttname));

    rdchbuf.length = rdchbuf.index = 0;

    if (ttyfd != -1
#ifdef COMMENT
#ifdef CK_TAPI
         && ttyfd != -2
#endif /* CK_TAPI */
#endif /* COMMENT */
         ) {                            /* if device already opened */
       if ((ttname[0] == '_' ||
             ttname[0] == '$' ||
             ttname[0] == '!')
            && ttyfd == atoi(&ttname[1]))
           return(0);

#ifdef NT
        /* Support for Pragma Systems Telnet/Terminal Servers */
        if (!strcmp(ttname,"0")) {
            char * p = getenv("PRAGMASYS_INETD_SOCK");
            if ( p && ttyfd == atoi(p))
                return(0);
        }
#endif /* NT */

       if ( strncmp(ttname,ttnmsv,DEVNAMLEN) ) { /* new & old names equal? */
           debug(F111,"ttopen closing",ttname,ttyfd);
           ttclos(0);                   /* no, close old ttname, open new */
       } else {                         /* else same, ignore this call, */
           debug(F111,"ttopen already open",ttname,ttyfd);
           ttname2title(ttname);

           /* This next line is a work around in case 'local' gets set   */
           /* somehow to remote mode which will result in file transfers */
           /* failing. */
           if (*lcl == -1) {
#ifdef IKSD
               if ( inserver )
                   *lcl = 0;
               else
#endif /* IKSD */
               *lcl = 1;                /* we can't open in remote mode */
           }
           return(0);                   /* and return. */
       }
    }


    wasclosed = 0;
    ishandle = 0;
    ttmdm = modem;                      /* Make this available to other fns */

    /* We need to catch the sequence of commands:
     *   set net type ...
     *   set host ...
     *   set modem type ...
     *
     * since that will reset the 'modem' variable to a positive
     * value.
     */
    if (network && ckstrcmp(ttname,ttnmsv,-1,0) || modem < 0) {
        network = 1;
    } else {
        network = 0;
    }
    tt_mode = TT_MODE_NONE;

#ifdef NT
    /* Support for Pragma Systems Telnet/Terminal Servers */
    if ( !strcmp(ttname,"0") )
    {
        char * p = getenv("PRAGMASYS_INETD_SOCK");
        if ( p ) {
            /* We are running under PragmaSys Telnetd/Inetd */
            extern int reliable;
            debug(F110,"PRAGMASYS_INETD_SOCK",p,0);
            ckstrncpy(ttnmsv, ttname, DEVNAMLEN); /* Keep copy of name locally. */
            ttyfd = atoi(p);
            if ( p = getenv("PRAGMASYS_COMPORT") ) {
                /* 
                 * The PragaSys sample code puts a DuplicateHandle here
                 */
                debug(F110,"PRAGMASYS_COMPORT",p,0);
                network = 0;
                reliable = 0;
            } else {
                network = 1;
                nettype = NET_TCPB;
                ttnproto = NP_TELNET;
                reliable = 1;
            }
            *lcl = 0;
            return(0);
        }
    }
#endif /* NT */

#ifndef NOTERM
    doreset(1);         /* Soft Reset the terminal - clrscreen && home cursor */
#endif /* NOTERM */

#ifdef NETCONN
#ifdef TCPSOCKET
    {
        extern int tcpsrv_fd;
        if ( tcpsrv_fd != -1 && ttname[0] != '*')
            tcpsrv_close();
    }
#endif /* TCPSOCKET */
    if (network) {
        debug(F100,"ttopen calling os2_netflui()","",0);
        os2_netflui();
        debug(F100,"ttopen calling os2_netopen()","",0);
        rc = os2_netopen(ttname, lcl, modem < 0 ? -modem : modem);
        debug(F111,"ttname changed to",ttname,(int) strlen(ttname));
        if (!rc) {
            ckstrncpy(ttnmsv, ttname, DEVNAMLEN);   /* Keep copy of name locally. */
            ttname2title(ttname);
        }
    }
    else
#endif /* NETCONN */
    {
#ifdef CK_TAPI
    if ( tttapi )  /* We are trying to open a TAPI Line Device */
    {
        rc = tapi_open(ttname) ;
        debug(F111,"tapi_open rc",ttname,rc);
        ckstrncpy(ttnmsv, ttname, DEVNAMLEN );
        if (!rc) {
            *lcl = 1;                   /* Assume it's local. */
#ifdef COMMENT
            /* ttmdm is now set in tapi_open() */
            if (!tapipass)
                ttmdm = 38;             /* ckudia.c */
#endif /* COMMENT */
        }
    }
    else
#endif /* CK_TAPI */
    {
        /*
        This code lets you give Kermit an open file descriptor for a serial
        communication device, rather than a device name.  Kermit assumes that the
        line is already open, conditioned with the right parameters, etc.
        */
        for (x = ttname; isdigit(*x); x++) ; /* Check for all digits */

        if (*x == '\0') {
            ttyfd = atoi(ttname);
            ishandle = 1;
            exithangup = 0;  /* Do not close on exit */
            *lcl = 1;                   /* Assume it's local. */
            if (ttiscom(ttyfd))
            {
                rc = savetty();
                if ( !rc )
                    ttname2title(ttname);
                return rc;
            }
            ttyfd = -1;
            wasclosed = 1;
            return(-4);
        }

#ifdef NT
        memset(&security, 0, sizeof(SECURITY_ATTRIBUTES));
        security.nLength = sizeof(SECURITY_ATTRIBUTES);
        security.lpSecurityDescriptor = NULL ;
        security.bInheritHandle = TRUE ;

        /*
         * If DOS device names terminate with backslash on Windows 95/98
         * K95 will crash.
         */
        if ( isWin95() &&
             (!ckstrcmp(ttname,"lpt",3,0) ||
              !ckstrcmp(ttname,"com",3,0) ||
              !ckstrcmp(ttname,"prn",3,0)) ) {
            while ( ttname[strlen(ttname)-1] == '\\' )
                ttname[strlen(ttname)-1] = '\0';
        }

        if ( ttname[0] == '\\' || ttname[strlen(ttname)-1] == '$' )
            ckstrncpy(portname,ttname,267);
        else {
            strcpy(portname,"\\\\.\\");
            ckstrncpy(&portname[4],ttname,263);
        }
        if ( (HANDLE)(ttyfd =
                       (int) CreateFile(portname,
                                         GENERIC_READ | GENERIC_WRITE,
                                         ttshare ? (FILE_SHARE_READ | FILE_SHARE_WRITE) : 0,
                                         &security,
                                         OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                         NULL)) == INVALID_HANDLE_VALUE) {
            debug(F111,"ttopen CreateFile","GetLastError",GetLastError());
            ttyfd = -1 ;
            wasclosed = 1;
            return -1 ;
        }
#else /* not NT */
        if (ttslip) {
            rc = SlipOpen( ttname ) ;
            if ( rc ) {
                PPPSlipClose() ;
                ttyfd = -1 ;
                wasclosed = 1;
                return -6 ;
            }
        }
        else if ( ttppp ) {
            rc = PPPOpen( ttname ) ;
            if ( rc ) {
                PPPSlipClose() ;
                ttyfd = -1 ;
                wasclosed = 1;
                return -6 ;
            }
        }

        if (res = DosOpen(ttname,(PHFILE)&ttyfd,&action,0L,0,FILE_OPEN,
                           OPEN_ACCESS_READWRITE |
                           ( (ttshare || ttppp || ttslip) ? OPEN_SHARE_DENYNONE : OPEN_SHARE_DENYREADWRITE) |
                           OPEN_FLAGS_FAIL_ON_ERROR ,0L)) {
            debug(F111,"ttopen DosOpen","res",res);
            if (ttslip)
                PPPSlipClose() ;
            ttyfd = -1;
            wasclosed = 1;
            return((res == ERROR_SHARING_VIOLATION) ? -5 : -1);
        }
#endif /* NT */
        ckstrncpy(ttnmsv, ttname, DEVNAMLEN );
    }
    debug(F111,"ttopen ok",ttname,*lcl);

    /* Caller wants us to figure out if line is controlling tty */
    if (*lcl == -1) {
        *lcl = 1;                       /* Can never transfer with console */
    }
    if (*lcl) {
        if (!ttiscom(ttyfd)) {          /* Not a serial port */
            ttclos(0);
            return(-4);
        }

#ifdef NT
#ifdef BETADEBUG
        DisplayCommProperties((HANDLE)ttyfd);
#endif /* BETADEBUG */
        SetCommMask( (HANDLE) ttyfd, EV_RXCHAR ) ;
        SetupComm( (HANDLE) ttyfd, 20000, 20000 ) ;
        PurgeComm( (HANDLE) ttyfd,
                   PURGE_RXABORT | PURGE_TXABORT |
                   PURGE_RXCLEAR | PURGE_TXCLEAR );
#endif /* NT */
        savetty();
        ttprty = dfprty;                        /* Make parity the default parity */
        if (ttsettings(ttprty,stopbits))
            return(-1);
        rc = ttflui();

#ifdef NT
#ifdef NEWRDCH
        /* Reset the Overlapped I/O structures */
        OverlappedWriteInit();
        OverlappedReadInit();
#endif
#endif
    }
    }

    if ( !rc )
        ttname2title(ttname);

    return rc;
}

/*  T T I S C O M  --  Is the given handle an open COM port? */

ttiscom(int f) {
#ifdef NT
    DCB testdcb;
#else /* NT */
    DCBINFO testdcb;
#endif /* NT */
#ifdef COMMENT
#ifdef TN_COMPORT
    /* TELNET REMOTE COM PORT option means we are a com port */
    if (istncomport())
        return(1);
#endif /* TN_COMPORT */
#endif /* COMMENT */
#ifdef NT
#ifdef CK_TAPI
    if ( tttapi && !tapipass )
    {
        HANDLE hModem = NULL;

        hModem = GetModemHandleFromLine( (HLINE) 0 );
        if ( hModem == NULL )
            return(0);

        memset( ttycfg, 0, 1024 );
        ttycfg->dwSize = 1024;
        cfgsize = 1024;

        if (!GetCommConfig( hModem, ttycfg, &cfgsize )) {
            CloseHandle(hModem);
            return 0 ;
        }
        CloseHandle( hModem );
        return (1);
    }
#endif /* CK_TAPI */

    testdcb.DCBlength = sizeof(DCB);
    if (!GetCommState( (HANDLE) ttyfd, &testdcb))
        return(0);
#else /* NT */
    /* Read DCB */
    if (DosDevIOCtl(&testdcb,sizeof(testdcb),NULL,0,
                    ASYNC_GETDCBINFO,IOCTL_ASYNC,f)) {
        return( 0 );                    /* Bad, not a serial port */
    }
#endif /* NT */
    return( 1 );                        /* Good */
}

/*  T T C L O S  --  Close the TTY.  */

ttclos(int spare) {
    int rc = 0;

    if (ttyfd == -1
#ifdef CK_TAPI
         || (!tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         ) return(0);           /* Wasn't open. */

#ifdef NT
    /* Support for Pragma Systems Telnet/Terminal Servers */
    {
        char * p = getenv("PRAGMASYS_INETD_SOCK");
        if ( p && ttyfd == atoi(p)) {
            /* We are running under PragmaSys Telnetd/Inetd */
            ttyfd = -1;
            wasclosed = 1;
            return(0);
        }
    }
#endif /* NT */

    os2settitle("",TRUE);

#ifdef NETCONN
    if (network) {
        rc = os2_netclos();
    }
    else
#endif /* NETCONN */
#ifdef CK_TAPI
    if ( tttapi ) /* Closing a TAPI Line Device */
    {
        rc = tapi_clos();
    }
    else
#endif /* CK_TAPI */
    {
        if (savedtty
#ifdef OS2ONLY
            && !ttslip && !ttppp        /* Restoring the line has a tendancy */
                                        /* to drop DTR, which hangs up the   */
                                        /* connection, not a good thing      */
#endif /* OS2ONLY */
            )
           restoretty();
        if (!ishandle) {
#ifdef NT
           CloseHandle( (HANDLE) ttyfd ) ;
#else /* NT */
           DosClose(ttyfd);
#endif /* NT */
        }
        ishandle = 0;
        ttyfd = -1;
        wasclosed = 1;
        savedtty = 0 ;

#ifndef NT
        PPPSlipClose() ;
#endif /* NT */
    }
    return(rc);
}

/*  T T G S P D  --  return speed of COM port, or of default line */

long
ttgspd() {
#ifndef NT
    long sp = 0;
    struct {
      long current_rate;
      char current_fract;
      long minimum_rate;
      char minimum_fract;
      long maximum_rate;
      char maximum_fract;
    } speed;
#endif /* NT */

    if (ttyfd == -1
#ifdef CK_TAPI
         || (!tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return -1 ;
#ifdef TN_COMPORT
    if ( network) {
        if (istncomport()) {
            return(tnc_get_baud());
        } else
            return(-1);
    }
#endif /* TN_COMPORT */
#ifdef NT
#ifdef CK_TAPI
    if ( tttapi && !tapipass )
    {
        HANDLE hModem = NULL;

        hModem = GetModemHandleFromLine( (HLINE) 0 );
        if ( hModem == NULL )
            return(-1);

        memset( ttycfg, 0, 1024 );
        ttycfg->dwSize = 1024;
        cfgsize = 1024;

        if (!GetCommConfig( hModem, ttycfg, &cfgsize )) {
            CloseHandle( hModem );
            return -1 ;
        }
        CloseHandle( hModem );
        return ttycfg->dcb.BaudRate;
    }
#endif /* CK_TAPI */

    ttydcb.DCBlength = sizeof(DCB);
    if (!GetCommState( (HANDLE) ttyfd, &ttydcb))
        return -1 ;
    else
        return ttydcb.BaudRate ;
#else /* NT */
    if (DosDevIOCtl(&speed,sizeof(speed),NULL,0,0x0063,IOCTL_ASYNC,ttyfd) == 0)
      return speed.current_rate;
    else
      if (DosDevIOCtl(&sp,sizeof(sp),NULL,0,
                      ASYNC_GETBAUDRATE,IOCTL_ASYNC,ttyfd) == 0)
      return sp;
    else
      return -1;
#endif /* NT */
}

ttsetspd(long sp) {
#ifdef NT
        int rc=0;
#else
    struct {
      long rate;
      char fract;
    } speed;

#endif /* NT */

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         ) return -1 ;
#ifdef TN_COMPORT
    if ( network) {
        if (istncomport()) {
            return(tnc_set_baud(sp));
        } else
            return(-1);
    }
#endif /* TN_COMPORT */
#ifdef NT
#ifdef CK_TAPI
    if ( tttapi && !tapipass )
    {
        HANDLE hModem = NULL;
        LPDEVCFG        lpDevCfg = NULL;
        LPCOMMCONFIG    lpCommConfig = NULL;
        LPMODEMSETTINGS lpModemSettings = NULL;
        DCB *           lpDCB = NULL;

        hModem = GetModemHandleFromLine( (HLINE) 0 );
        if ( hModem == NULL )
            return(-1);

        if (!cktapiGetModemSettings(&lpDevCfg,&lpModemSettings,&lpCommConfig,&lpDCB)) {
            CloseHandle( hModem );
            return -1 ;
        }

        lpDCB->BaudRate = sp ;

        if (!cktapiSetModemSettings(lpDevCfg,lpCommConfig)) {
            CloseHandle( hModem );
            return -1 ;
        }

        CloseHandle( hModem );
        return (0);
    }
#endif /* CK_TAPI */

    ttydcb.DCBlength = sizeof(DCB);
    if (!GetCommState( (HANDLE) ttyfd, &ttydcb))
        return -1 ;


    if ( deblog )
        debugComm( "ttsetspd initial values", &ttydcb, NULL );


    ttydcb.BaudRate = sp ;
    rc = SetCommState( (HANDLE) ttyfd, &ttydcb ) ;
        if (!rc) {
            printf("ttsetspd failed: %d\n", GetLastError());
            return -1;
        }
        else
            return 0;
#else
    if (sp > 65535L) {
      speed.rate = sp;
      speed.fract = 0;
      return DosDevIOCtl(NULL,0,&speed,sizeof(speed),0x0043,IOCTL_ASYNC,ttyfd);
    } else
      return DosDevIOCtl(NULL,0,&sp,sizeof(sp),
                         ASYNC_SETBAUDRATE,IOCTL_ASYNC,ttyfd);
#endif /* NT */
}



/*  T T H A N G -- Hangup phone line */

tthang() {
    extern int what;

    if ( ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return(0);              /* Nothing to do */
#ifdef NETCONN
    if (network 
#ifdef TN_COMPORT
	 && !istncomport()
#endif /* TN_COMPORT */
	 ) {
        int rc = os2_netclos();
        if (network && tn_exit && ttyfd == -1) {
            DialerSend(OPT_KERMIT_HANGUP, 0);
            doexit(GOOD_EXIT,xitsta);   /* Exit with good status */
        }
        return rc;
    }
#endif /* NETCONN */
    if ( exitonclose && !(what & W_DIALING) && (ttchk() < 0) ) {
        DialerSend(OPT_KERMIT_HANGUP, 0);
        doexit(GOOD_EXIT,xitsta);       /* Exit with good status */
    }
#ifdef CK_TAPI
    if ( tttapi && !tapipass )
        return cktapihangup();
#endif /* CK_TAPI */

 /*
  Perhaps better to either let user specify the hangup interval, or else
  do something with carrier -- e.g. if CD was on when we entered this routine,
  then cancel the sleep as soon as it goes down, so we don't sleep longer
  than we need to.
*/
    if (os2setdtr(0))
       return -1;
    msleep(HUPTIME);
    os2setdtr(1);
    return 1;
}

#ifdef PARSENSE
static int needpchk = 1;
#else /* PARSENSE */
static int needpchk = 0;
#endif /* PARSENSE */

#ifdef TCPSOCKET
static int nodelay_sav = -1;
#endif /* TCPSOCKET */
#ifndef NOTERM
static int tt_update_sav = -1;
extern int tt_update;
#endif /* NOTERM */

#ifdef NT
/* For support of Pragma Systems Telnet/Terminal Servers */
HANDLE  hPragmaSysInputMutex = NULL;
HANDLE  hPragmaSysOutputMutex = NULL;

#define SET_TELNETD_INPUT_MUTEX(V,P)    sprintf( V, "TelnetDInput%dMutex", P )
#define SET_TELNETD_OUTPUT_MUTEX(V,P)   sprintf( V, "TelnetDOutput%dMutex", P )
#endif /* NT */


/*  T T R E S  --  Restore terminal to "normal" mode.  */

ttres() {                               /* Restore the tty to normal. */
    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return(-1);     /* Not open */

#ifndef NOTERM
    if ( tt_update_sav > -1 ) {
        tt_update = tt_update_sav;
        tt_update_sav = -1;
    }
#endif /* NOTERM */

#ifdef COMMENT
    if (tt_mode == TT_MODE_NORM)
        return(0);
#endif /* COMMENT */

#ifdef NT
    /* Support for Pragma Systems Telnet/Terminal Servers */
    {
        char * p = getenv("PRAGMASYS_INETD_SOCK");
        if ( p && ttyfd == atoi(p)) {
            if ( hPragmaSysOutputMutex ) {
                ReleaseMutex(hPragmaSysOutputMutex);
                CloseHandle(hPragmaSysOutputMutex);
                hPragmaSysOutputMutex = NULL;
            }
            if ( hPragmaSysInputMutex ) {
                ReleaseMutex(hPragmaSysInputMutex);
                CloseHandle(hPragmaSysInputMutex);
                hPragmaSysInputMutex = NULL;
            }
            tt_mode = TT_MODE_NORM;
            return(0);
        }
    }
#endif /* NT */

    if (network) {
#ifdef TCPSOCKET
#ifdef TCP_NODELAY
        if ( network && nettype == NET_TCPB ) {
            extern int tcp_nodelay;
            if ( nodelay_sav > -1 ) {
                no_delay(ttyfd,nodelay_sav);
                nodelay_sav = -1;
            }
        }
#endif /* TCP_NODELAY */
#ifdef TN_COMPORT
        if (network && istncomport()) {
            int rc = -1;
            if ((rc = tnsetflow(flow)) < 0)
                return(rc);
            if (speed <= 0)
                speed = tnc_get_baud();
            else if ((rc = tnc_set_baud(speed)) < 0)
                return(rc);
            tnc_set_datasize(8);
            if (hwparity) {
                switch (hwparity) {
                case 'e':                       /* Even */
                    debug(F100,"ttres 8 bits + even parity","",0);
                    tnc_set_parity(3);
                    break;
                case 'o':                       /* Odd */
                    debug(F100,"ttres 8 bits + odd parity","",0);
                    tnc_set_parity(2);
                    break;
                case 'm':                       /* Mark */
                    debug(F100,"ttres 8 bits + invalid parity: mark","",0);
                    tnc_set_parity(4);
                    break;
                case 's':                       /* Space */
                    debug(F100,"ttres 8 bits + invalid parity: space","",0);
                    tnc_set_parity(5);
                    break;
                }
            } else {
                tnc_set_parity(1);              /* None */
            }
            tnc_set_stopsize(stopbits);
            tt_mode = TT_MODE_NORM;
            return(0);
        }
#endif /* TN_COMPORT */
#endif /* TCPSOCKET */
#ifdef NETDLL
        if ( network && nettype == NET_DLL ) {
            extern int (*net_dll_ttres)(void);
            extern char * (*net_dll_errorstr)(int);
            int rc = 0;
            if ( net_dll_ttres ) {
                rc = net_dll_ttres();
                debug(F111,"net_dll_ttres()",
                       (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
                if (rc < 0)
                    return(-1);
            }
            else
                return(-1);
        }
#endif /* NETDLL */
    }

    ResetThreadPrty();

    tt_mode = TT_MODE_NORM;
    return(0);
}

/*  T T P K T  --  Condition the communication line for packets. */
/*              or for modem dialing */

/*  If called with speed > -1, also set the speed.  */
/*  Returns 0 on success, -1 on failure.  */

ttpkt(long speed, int flow, int parity) {
    extern int priority;
    int s;
#ifdef NT
    char * p = NULL;
#endif /* NT */

    debug(F111,"ttpkt","speed",speed);
    debug(F111,"ttpkt","flow",flow);
    debug(F111,"ttpkt","parity",parity);
#ifdef HWPARITY
    debug(F111,"ttpkt","hwparity",hwparity);
    debug(F111,"ttpkt","stopbits",stopbits);
#endif /* HWPARITY */

#ifdef COMMENT
    if ( tt_mode == TT_MODE_PKT )
        return(0);
#endif /* COMMENT */
    if ( tt_mode != TT_MODE_NORM && tt_mode != TT_MODE_PKT )
        ttres();

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return(-1);             /* Not open. */

    ttprty = parity;
    ttpmsk = ttprty ? 0177 : 0377;      /* Parity stripping mask */
#ifdef PARSENSE
    needpchk = ttprty ? 0 : 1;          /* Parity check needed? */
#else
    needpchk = 0;
#endif /* PARSENSE */
    debug(F101,"ttpkt ttpmsk","",ttpmsk);

#ifdef NT
    SetThreadPrty(priority,isWin95() ? 9 : 13);
#else /* NT */
    SetThreadPrty(priority,3);
#endif /* NT */

#ifdef NT
    /* Support for Pragma Systems Telnet/Terminal Servers */
    p = getenv("PRAGMASYS_INETD_SOCK");
    if ( p && ttyfd == atoi(p)) {
        char PragmaSysInputMutexName[MAX_PATH];
        char PragmaSysOutputMutexName[MAX_PATH];

        p = getenv("PRAGMASYS_TELNETD_PID");
        if ( p )
        {
            debug(F111,"ttpkt","PRAGMASYS_TELNETD_PID",atoi(p));
            SET_TELNETD_INPUT_MUTEX(PragmaSysInputMutexName,(DWORD)atoi(p));
            SET_TELNETD_OUTPUT_MUTEX(PragmaSysOutputMutexName,(DWORD)atoi(p));
            debug(F110,"ttpkt",PragmaSysInputMutexName,0);
            debug(F110,"ttpkt",PragmaSysOutputMutexName,0);

            hPragmaSysOutputMutex = OpenMutex( SYNCHRONIZE, FALSE, PragmaSysOutputMutexName);
            if ( hPragmaSysOutputMutex == NULL )
            {
                debug(F111,"ttpkt","PragmaSys unable to OpenMutex(Output)",
                       GetLastError());
                return(-1);
            }

            if (WaitForSingleObject( hPragmaSysOutputMutex,  5000 ) != WAIT_OBJECT_0)
            {
                debug(F111,"ttpkt","PragmaSys unable to WaitForSingleObject(Output)",
                       GetLastError());
                CloseHandle(hPragmaSysOutputMutex);
                hPragmaSysOutputMutex = NULL;
                return(-1);
            }

            hPragmaSysInputMutex = OpenMutex( SYNCHRONIZE, FALSE, PragmaSysInputMutexName);
            if ( hPragmaSysInputMutex == NULL )
            {
                debug(F111,"ttpkt","PragmaSys unable to OpenMutex(Input)",
                       GetLastError());
                CloseHandle(hPragmaSysOutputMutex);
                hPragmaSysOutputMutex = NULL;
                return(-1);
            }

            if (WaitForSingleObject( hPragmaSysInputMutex,  5000 ) != WAIT_OBJECT_0)
            {
                debug(F111,"ttpkt","PragmaSys unable to WaitForSingleObject(Input)",
                       GetLastError());
                CloseHandle(hPragmaSysOutputMutex);
                hPragmaSysOutputMutex = NULL;
                CloseHandle(hPragmaSysInputMutex);
                hPragmaSysInputMutex = NULL;
                return(-1);
            }
            debug(F100,"ttpkt success","",0);
            tt_mode = TT_MODE_PKT;
            return(0);
        }
        debug(F110,"ttpkt","PragmaSys unable to getenv(PRAGMASYS_TELNETD_PID)",0);
        return(-1);
    }
#endif /* NT */

    if (network) {
#ifdef TCPSOCKET
#ifdef TCP_NODELAY
        if ( network && nettype == NET_TCPB ) {
            extern int tcp_nodelay;
            nodelay_sav = tcp_nodelay;
            no_delay(ttyfd,1);
        }
#endif /* TCP_NODELAY */
#ifdef TN_COMPORT
        if (network && istncomport()) {
            int rc = -1;
            if ((rc = tnsetflow(flow)) < 0)
                return(rc);
            if (speed <= 0)
                speed = tnc_get_baud();
            else if ((rc = tnc_set_baud(speed)) < 0)
                return(rc);
            tnc_set_datasize(8);
            if (hwparity) {
                switch (hwparity) {
                case 'e':                       /* Even */
                    debug(F100,"ttpkt 8 bits + even parity","",0);
                    tnc_set_parity(3);
                    break;
                case 'o':                       /* Odd */
                    debug(F100,"ttpkt 8 bits + odd parity","",0);
                    tnc_set_parity(2);
                    break;
                case 'm':                       /* Mark */
                    debug(F100,"ttpkt 8 bits + invalid parity: mark","",0);
                    tnc_set_parity(4);
                    break;
                case 's':                       /* Space */
                    debug(F100,"ttpkt 8 bits + invalid parity: space","",0);
                    tnc_set_parity(5);
                    break;
                }
            } else {
                tnc_set_parity(1);              /* None */
            }
            tnc_set_stopsize(stopbits);
            tt_mode = TT_MODE_PKT;
            return(0);
        }
#endif /* TN_COMPORT */
#endif /* TCPSOCKET */
#ifdef NETDLL
        if ( network && nettype == NET_DLL ) {
            extern int (*net_dll_ttpkt)(void);
            extern char * (*net_dll_errorstr)(int);
            int rc = 0;
            if ( net_dll_ttpkt ) {
                rc = net_dll_ttpkt();
                debug(F111,"net_dll_ttpkt()",
                       (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
                if (rc<0)
                    return(-1);
            }
            else
                return(-1);
        }
#endif /* NETDLL */
        debug(F100,"ttpkt success","",0);
        tt_mode = TT_MODE_PKT;
        return(0);
    }

    if (speed < 0)
        return(-1);

    os2setdtr(1);
    if (ttsetspd(speed))
        return(-1);
    if (ttsettings(ttprty,stopbits))
        return(-1);
    if (ttsetflow(flow))
        return(-1);
    if (os2settimo(speed,ttmdm))
        return(-1);
    if (os2setcarr(ttcarr == CAR_ON && flow != FLO_DIAL))
        return(-1);

    debug(F100,"ttpkt success","",0);
    tt_mode = TT_MODE_PKT;
    return(0);
}


/*  T T V T -- Condition communication line for use as virtual terminal  */

ttvt(long speed, int flow) {
    extern int priority;
#ifdef NT
    char * p = NULL;
#endif /* NT */

    debug(F111,"ttvt","speed",speed);
    debug(F111,"ttvt","flow",flow);
    debug(F111,"ttvt","parity",parity);

#ifdef COMMENT
    if (tt_mode == TT_MODE_VT)
        return(0);
#endif
    if ( tt_mode != TT_MODE_NORM && tt_mode != TT_MODE_NORM )
        ttres();

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         ) {
        if ( ttchk() > 0 )
            return(0);          /* Data waiting to be read */
        else
            return(-1);         /* Not open. */
    }
#ifndef NOTERM
    if ( tt_update_sav > -1 ) {
        tt_update = tt_update_sav;
        tt_update_sav = -1;
    }
#endif /* NOTERM */
    ResetThreadPrty();

#ifdef NT
    /* Support for Pragma Systems Telnet/Terminal Servers */
    p = getenv("PRAGMASYS_INETD_SOCK");
    if ( p && ttyfd == atoi(p)) {
        char PragmaSysInputMutexName[MAX_PATH];
        char PragmaSysOutputMutexName[MAX_PATH];

        p = getenv("PRAGMASYS_TELNETD_PID");
        if ( p )
        {
            if ( hPragmaSysOutputMutex && hPragmaSysInputMutex )
                return(0);

            debug(F111,"ttvt","PRAGMASYS_TELNETD_PID",atoi(p));
            SET_TELNETD_INPUT_MUTEX(PragmaSysInputMutexName,(DWORD)atoi(p));
            SET_TELNETD_OUTPUT_MUTEX(PragmaSysOutputMutexName,(DWORD)atoi(p));
            debug(F110,"ttvt",PragmaSysInputMutexName,0);
            debug(F110,"ttvt",PragmaSysOutputMutexName,0);

            hPragmaSysOutputMutex = OpenMutex( SYNCHRONIZE, FALSE, PragmaSysOutputMutexName);
            if ( hPragmaSysOutputMutex == NULL )
            {
                debug(F111,"ttvt","PragmaSys unable to OpenMutex(Output)",
                       GetLastError());
                return(-1);
            }

            if (WaitForSingleObject( hPragmaSysOutputMutex,  5000 ) != WAIT_OBJECT_0)
            {
                debug(F111,"ttvt","PragmaSys unable to WaitForSingleObject(Output)",
                       GetLastError());
                CloseHandle(hPragmaSysOutputMutex);
                hPragmaSysOutputMutex = NULL;
                return(-1);
            }

            hPragmaSysInputMutex = OpenMutex( SYNCHRONIZE, FALSE, PragmaSysInputMutexName);
            if ( hPragmaSysInputMutex == NULL )
            {
                debug(F111,"ttvt","PragmaSys unable to OpenMutex(Input)",
                       GetLastError());
                CloseHandle(hPragmaSysOutputMutex);
                hPragmaSysOutputMutex = NULL;
                return(-1);
            }

            if (WaitForSingleObject( hPragmaSysInputMutex,  5000 ) != WAIT_OBJECT_0)
            {
                debug(F111,"ttvt","PragmaSys unable to WaitForSingleObject(Input)",
                       GetLastError());
                CloseHandle(hPragmaSysOutputMutex);
                hPragmaSysOutputMutex = NULL;
                CloseHandle(hPragmaSysInputMutex);
                hPragmaSysInputMutex = NULL;
                return(-1);
            }
            debug(F100,"ttvt success","",0);
            tt_mode = TT_MODE_VT;
            return(0);
        }
        debug(F110,"ttvt","PragmaSys unable to getenv(PRAGMASYS_TELNETD_PID)",0);
        return(-1);
    }
#endif /* NT */

    if (network) {
#ifdef TCPSOCKET
#ifdef TCP_NODELAY
        if ( network && nettype == NET_TCPB ) {
            extern int tcp_nodelay;
            if ( nodelay_sav > -1 ) {
                no_delay(ttyfd,nodelay_sav);
                nodelay_sav = -1;
            }
        }
#endif /* TCP_NODELAY */
#ifdef TN_COMPORT
        if (network && istncomport()) {
            int rc = -1;
            if ((rc = tnsetflow(flow)) < 0)
                return(rc);
            if (speed <= 0)
                speed = tnc_get_baud();
            else if ((rc = tnc_set_baud(speed)) < 0)
                return(rc);
            tnc_set_datasize(8);
            if (hwparity) {
                switch (hwparity) {
                case 'e':                       /* Even */
                    debug(F100,"ttvt 8 bits + even parity","",0);
                    tnc_set_parity(3);
                    break;
                case 'o':                       /* Odd */
                    debug(F100,"ttvt 8 bits + odd parity","",0);
                    tnc_set_parity(2);
                    break;
                case 'm':                       /* Mark */
                    debug(F100,"ttvt 8 bits + invalid parity: mark","",0);
                    tnc_set_parity(4);
                    break;
                case 's':                       /* Space */
                    debug(F100,"ttvt 8 bits + invalid parity: space","",0);
                    tnc_set_parity(5);
                    break;
                }
            } else {
                tnc_set_parity(1);              /* None */
            }
            tnc_set_stopsize(stopbits);
            tt_mode = TT_MODE_VT;
            return(0);
        }
#endif /* TN_COMPORT */
#endif /* TCPSOCKET */
#ifdef NETDLL
        if ( network && nettype == NET_DLL ) {
            extern int (*net_dll_ttvt)(void);
            extern char * (*net_dll_errorstr)(int);
            int rc = 0;
            if ( net_dll_ttvt ) {
                rc = net_dll_ttvt();
                debug(F111,"net_dll_ttvt()",
                       (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
                if ( rc < 0 )
                    return(-1);
            }
            else
                return(-1);
        }
#endif /* NETDLL */
        debug(F100,"ttvt success","",0);
        tt_mode = TT_MODE_VT;
        return(0);
    }   /* end of network */

    if (speed < 0)
        return(-1);

    ttprty = parity;
    ttpmsk = 0xff;

    os2setdtr(1);
    if (ttsetspd(speed))
        return(-1);
    if (ttsettings(ttprty,stopbits))
        return(-1);
    if (ttsetflow(flow))
        return (-1);
    if (os2settimo(speed,ttmdm))
        return(-1);
    if (os2setcarr(ttcarr == CAR_ON || ttcarr == CAR_AUT))
        return(-1);

    debug(F100,"ttvt success","",0);
    tt_mode = TT_MODE_VT;
    return(0);
}


/*  T T S S P D  --  Return the speed if OK, otherwise -1 */

int
ttsspd(int speed) {
    long s;

    if (speed < 0) return(-1);

    s = (long) speed * 10L;
    ttsetspd(s);
    return(0);
}

/* The following functions will provide the interface for the local echo */
/* buffer to be used when 'duplex' is TRUE                               */

#ifdef OS2ONLY
#define LOCAL_ECHO_BUFSIZE (65536/sizeof(USHORT))
#else /* OS2ONLY */
#define LOCAL_ECHO_BUFSIZE 65536
#endif /* OS2ONLY */

static USHORT LocalEchoBuf[LOCAL_ECHO_BUFSIZE] ;
static int LocalEchoStart=0, LocalEchoEnd=0, LocalEchoData=0 ;

void
le_init( void ) {
    CreateLocalEchoAvailSem( FALSE );
    CreateLocalEchoMutex( TRUE ) ;
    memset(LocalEchoBuf,0,LOCAL_ECHO_BUFSIZE*sizeof(USHORT)) ;
    LocalEchoStart = 0 ;
    LocalEchoEnd = 0 ;
    LocalEchoData = 0;
    ReleaseLocalEchoMutex() ;
}

void
le_clean( void ) {
    CloseLocalEchoMutex() ;
    CloseLocalEchoAvailSem() ;
}

int
le_inbuf( void ) {
    int rc = 0 ;

    RequestLocalEchoMutex( SEM_INDEFINITE_WAIT ) ;
    if ( LocalEchoStart != LocalEchoEnd )
    {
        rc = (LocalEchoEnd - LocalEchoStart + LOCAL_ECHO_BUFSIZE)%LOCAL_ECHO_BUFSIZE;
    }
    ReleaseLocalEchoMutex() ;
    return rc ;
}

int
le_putstr( char * s )
{
    int rc = 0;
    if ( s && s[0] )
        rc = le_puts( s, strlen(s) ) ;
    return rc ;
}

int
le_puts( char * s, int n )
{
    int rc = 0 ;
    int i = 0;

    ckhexdump("LocalEchoPutChars",s,n);
    RequestLocalEchoMutex( SEM_INDEFINITE_WAIT ) ;
    for ( i=0 ; i<n ; i++ ) {
        while ( (LocalEchoStart - LocalEchoEnd == 1) ||
                ( LocalEchoStart == 0 && LocalEchoEnd == LOCAL_ECHO_BUFSIZE - 1 ) )
            /* Buffer is full */
        {
            debug(F111,"LocalEchoPutChar","Buffer is Full",s[i]);
            ReleaseLocalEchoMutex() ;
            msleep(250);
            RequestLocalEchoMutex( SEM_INDEFINITE_WAIT ) ;
        }

        LocalEchoBuf[LocalEchoEnd++] = s[i];
        if ( LocalEchoEnd == LOCAL_ECHO_BUFSIZE )
            LocalEchoEnd = 0 ;
        LocalEchoData = TRUE;
        PostLocalEchoAvailSem()  ;
    }
    ReleaseLocalEchoMutex() ;
    debug(F101,"LocalEchoPutChars","",rc);
    return rc ;
}

int
le_putchar( char ch ) {
    int rc = 0 ;

    RequestLocalEchoMutex( SEM_INDEFINITE_WAIT ) ;
    while ( (LocalEchoStart - LocalEchoEnd == 1) ||
            ( LocalEchoStart == 0 && LocalEchoEnd == LOCAL_ECHO_BUFSIZE - 1 ) )
        /* Buffer is full */
    {
        debug(F111,"LocalEchoPutChar","Buffer is Full",ch);
        ReleaseLocalEchoMutex() ;
        msleep(250);
        RequestLocalEchoMutex( SEM_INDEFINITE_WAIT ) ;
    }

    LocalEchoBuf[LocalEchoEnd++] = ch ;
    if ( LocalEchoEnd == LOCAL_ECHO_BUFSIZE )
        LocalEchoEnd = 0 ;
    LocalEchoData = TRUE;
    PostLocalEchoAvailSem()  ;
    ReleaseLocalEchoMutex() ;
    return rc ;
}

int
le_getchar( CHAR * pch )
{
    int rc = 0 ;

    RequestLocalEchoMutex( SEM_INDEFINITE_WAIT ) ;
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

/*  T T F L U I  --  Flush tty input buffer */

ttflui() {
    char parm=0;
    long int data;
    int i;

    ttpush = -1;                               /* Clear the peek-ahead char */

    while ( LocalEchoData && (le_inbuf() > 0) ) {
        char ch=0;
        if ( le_getchar(&ch) > 0 ) {
            debug(F111,"ttflui le_getchar","ch",ch);
        }
    }

#ifdef NETCONN
    if (network) {
#ifdef TN_COMPORT
#ifdef COMMENT
        if (istncomport())
            tnc_send_purge_data(TNC_PURGE_RECEIVE);
#endif /* TN_COMPORT */
#endif /* COMMENT */
        return os2_netflui() ;
    }
#endif /* NETCONN */
    rdchbuf.index = rdchbuf.length = 0;         /* Flush internal buffer */

#ifdef NT
    PurgeComm( (HANDLE) ttyfd, PURGE_RXCLEAR | PURGE_RXABORT ) ;
#else /* NT */
    DosDevIOCtl(&data,sizeof(data),&parm,sizeof(parm),
                DEV_FLUSHINPUT,IOCTL_GENERAL,ttyfd); /* Flush driver */
#endif /* NT */
    return(0);
}


/*  T T C H K  --  Tell how many characters are waiting in tty input buffer  */

ttchk() {
    int count=0;
#ifdef NT
    DWORD rc, lasterror;
    DWORD errors ;
    COMSTAT comstat ;
#else /* NT */
    USHORT data[2]={0,0};
#endif /* NT */

    if ( ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         ) {
        if ( ttpush >= 0 )
            count++;
        count += le_inbuf();
        return(count>0?count:-1);                       /* No connection */
    }

#ifdef NETCONN
    if (network) {
        count = os2_nettchk();
        if ( count >= 0 ) {
            if ( ttpush >= 0 )
                count++;
            count += le_inbuf();
#ifdef TN_COMPORT
            if (count == 0 && istncomport() && ttcarr != CAR_OFF) {
                /* Check carrier */
                extern int clsondisc;
                int mdmsig = tngmdm();
                if ( mdmsig >= 0 && (mdmsig & BM_DCD) == 0 ) {
                    debug(F111,"ttchk","Carrier not detected",ttcarr);
                    if (clsondisc) {        /* If "close-on-disconnect" */
                        debug(F100,"ttchk close-on-disconnect","",0);
                        ttclos(0);      /* close device */
                        return(-2);
                    }
                    return(-1);
                }
            }
#endif /* TN_COMPORT */
            debug(F101,"ttchk() network istncomport returns","",count);
            return count;
        }
        else {
            int count2 = 0;
            if ( ttpush >= 0 )
                count2++;
            count2 += le_inbuf();
            debug(F101,"ttchk() network !istncomport returns","",
                   count2?count2:count);
            return(count2?count2:count);
        }
    }
#endif /* NETCONN */

#ifdef NT
    RequestCommMutex(SEM_INDEFINITE_WAIT);
    rc = ClearCommError( (HANDLE) ttyfd, &errors, &comstat );
    ReleaseCommMutex();
    lasterror = GetLastError();
    if (!rc) {
        debug(F101,"ttchk() ClearCommError failed","",lasterror);
        count = -1;
    } else {
        if ( errors && deblog )
        {
            if ( errors & CE_RXOVER )
                debug(F110,"ttchk ClearCommError","Receive Queue overflow",0);
            if ( errors & CE_OVERRUN )
                debug(F110,"ttchk ClearCommError","Receive Overrun error",0);
            if ( errors & CE_RXPARITY )
                debug(F110,"ttchk ClearCommError","Receive Parity error",0);
            if ( errors & CE_FRAME )
                debug(F110,"ttchk ClearCommError","Receive Framing error",0);
            if ( errors & CE_BREAK )
                debug(F110,"ttchk ClearCommError","Break detected",0);
            if ( errors & CE_TXFULL )
                debug(F110,"ttchk ClearCommError","TX Queue is full",0);
            if ( errors & CE_PTO )
                debug(F110,"ttchk ClearCommError","LPTx Timeout",0);
            if ( errors & CE_IOE )
                debug(F110,"ttchk ClearCommError","LPTx I/O Error",0);
            if ( errors & CE_DNS )
                debug(F110,"ttchk ClearCommError","LPTx Device Not Selected",0);
            if ( errors & CE_OOP )
                debug(F110,"ttchk ClearCommError","LPTx Out Of Paper",0);
        }
        count = comstat.cbInQue+(rdchbuf.length-rdchbuf.index);
    }
#else /* NT */
    if ( DosDevIOCtl(data,sizeof(USHORT),NULL,0,
                      ASYNC_GETCOMMERROR,IOCTL_ASYNC,ttyfd)) {
        if ( data[0] & RX_QUE_OVERRUN ) {
            debug( F100,"ttchk RX_QUE_OVERRUN","",0);
        }
        if ( data[0] & RX_HARDWARE_OVERRUN ) {
            debug( F100,"ttchk RX_HARDWARE_OVERRUN", "", 0 );
        }
    }

    if(DosDevIOCtl(data,sizeof(data),NULL,0,
                    ASYNC_GETINQUECOUNT,IOCTL_ASYNC,ttyfd))
        count = 0;
    else
        count = rdchbuf.length-rdchbuf.index+data[0];
#endif /* NT */

    if ( count >= 0 ) {
        if ( ttpush >= 0 )
            count++;
        count += le_inbuf();

        if (count == 0 && ttcarr != CAR_OFF) {
            /* Check carrier */
            extern int clsondisc;
            int mdmsig = ttgmdm();
            if ( mdmsig >= 0 && (mdmsig & BM_DCD) == 0 ) {
                debug(F111,"ttchk","Carrier not detected",ttcarr);
                if (clsondisc) {        /* If "close-on-disconnect" */
                    debug(F100,"ttchk close-on-disconnect","",0);
                    ttclos(0);      /* close device */
                    return(-2);
                }
                return(-1);
            }
        }

#ifdef NT
#ifdef NEWRDCH
        /* Is there anything in the overlapped I/O buffers? */
        /* Network Overlapped I/O is handled in nettchk()   */
        count += OverlappedDataWaiting();
#endif /* NEWRDCH */
#endif /* NT */
    } else {
        int cnt = 0;
        if ( ttpush >= 0 )
            cnt++;
        cnt += le_inbuf();
        if ( cnt )
            count = cnt;
    }
    debug(F101,"ttchk() !network returns","",count);
    return(count);
}


/*  T T X I N  --  Get n characters from tty input buffer  */

/*  Returns number of characters actually gotten, or -1 on failure  */

/*  Intended for use only when it is known that n characters are actually */
/*  available in the input buffer because this function blocks.           */

int
ttxin(int n, CHAR *buf) {
    int i=0, j=0, k=0;
    CHAR m=0 ;

    m = (ttprty) ? 0177 : 0377;         /* Parity stripping mask. */

    buf[0] = '\0';              /* make it easy to read the buffer in a debugger */

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
)
        return(-1);             /* Not open. */

    debug(F101,"ttxin n","",n);
    if ( n <= 0 )
        return(0);

    if ( ttpush >= 0 ) {
        buf[0] = ttpush & m; /* set first char of buffer to pushed char */
        ttpush = -1;              /* clear the push buffer                   */
        if ( ttchk() > 0 )
            return(ttxin( n-1, &buf[1] ) + 1);      /* recurse the call */
        else
            return(1);
    }

    if ( LocalEchoData ) {
        while ( le_inbuf() > 0 ) {
            le_getchar( &buf[i] );
            buf[i] &= m;
            i++;
            n--;
        }
        if (ttchk() > 0)
            return(ttxin( n, &buf[i] ) + i);
        else
            return(i);
    }

#ifdef NETCONN
    if (network) {
        j = os2_netxin(n,buf);
        if ( ttprty ) {
            for ( i=0;i<j;i++ )
                buf[i] &= m;
        }
    }
    else {
#endif /* NETCONN */
        debug( F101,"ttxin rdchbuf.index","",rdchbuf.index);
        debug( F101,"ttxin rdchbuf.length","",rdchbuf.length);
        if ( rdchbuf.index == rdchbuf.length ) {
            if ( (j = rdch(0)) < 0 )
                return(j);
            rdchbuf.index-- ;
        }
        i = rdchbuf.length - rdchbuf.index ;
        debug(F101,"ttxin i","",i);
        if (i <= n) {
            debug( F100,"ttxin i <= n","",0);
            memcpy(buf, &rdchbuf.buffer[rdchbuf.index], i );
            rdchbuf.index = rdchbuf.length ;
            j = i;
        }
        else {
            debug( F100,"ttxin i > n","",0);
            memcpy(buf, &rdchbuf.buffer[rdchbuf.index], n ) ;
            rdchbuf.index += n ;
            j = n;
        }
        if ( ttprty ) {
            for ( i=0;i<j;i++ )
                buf[i] &= m;
        }
#ifdef NETCONN
    }
#endif /* NETCONN */
    return(j);
}


#ifdef NT
/* Overlapped I/O code */

int
getOverlappedIndex( int serial ) {
    int ow ;
    /* get an overlapped_write structure that is not in use */
    for ( ow=0 ; ow<maxow ; ow++ ) {
        if ( !ow_inuse[ow] )
            break;
    }

    debug(F111,"getOverlappedIndex","next available ow",ow);
    if ( ow == maxow ) {
        /* They are all in use.  So lets either find or wait */
        /* for one which is complete.                        */
        ow = -1;
        while(!GetOverlappedResult( (HANDLE) ttyfd,
                                    &overlapped_write[++ow],
                                    &nActuallyWritten, owwait ))
        {
            DWORD error = GetLastError() ;
            if ( error == ERROR_IO_INCOMPLETE ) {
                debug(F111,"getOverlappedIndex ERROR_IO_INCOMPLETE","ow",ow);
                debug(F111,"getOverlappedIndex waiting to complete",ow_ptr[ow],ow);
                if ( (ow+1) >= maxow ) {

                    if ( serial ) {
                    /* All buffers are still outstanding */
                    /* Lets see if we can figure out why */
                    DWORD errorflags ;
                    COMSTAT comstat ;
                    RequestCommMutex(SEM_INDEFINITE_WAIT);
                    ClearCommError( (HANDLE) ttyfd, &errorflags, &comstat ) ;
                    ReleaseCommMutex();
                    debug(F101,"getOverlappedIndex ClearCommError",
                           "",errorflags) ;
#ifdef BETADEBUG
                    printf("\nClearCommError: %d\n",errorflags);
                    if ( errorflags ) {
                        if ( errorflags & CE_RXOVER )
                            printf("  Receive Queue overflow\n");
                        if ( errorflags & CE_OVERRUN )
                            printf("  Receive Overrun error\n");
                        if ( errorflags & CE_RXPARITY )
                            printf("  Receive Parity error\n");
                        if ( errorflags & CE_FRAME )
                            printf("  Receive Framing error\n");
                        if ( errorflags & CE_BREAK )
                            printf("  Break detected\n");
                        if ( errorflags & CE_TXFULL )
                            printf("  TX Queue is full\n");
                        if ( errorflags & CE_PTO )
                            printf("  LPTx Timeout\n");
                        if ( errorflags & CE_IOE )
                            printf("  LPTx I/O Error\n");
                        if ( errorflags & CE_DNS )
                            printf("  LPTx Device Not Selected\n");
                        if ( errorflags & CE_OOP )
                            printf("  LPTx Out Of Paper\n");
                    }
                    printf("Port Status:\n");
                    printf("  Cts Hold:  %d\n",comstat.fCtsHold);
                    printf("  Dsr Hold:  %d\n",comstat.fDsrHold);
                    printf("  Rlsd Hold: %d\n",comstat.fRlsdHold);
                    printf("  Xoff Hold: %d\n",comstat.fXoffHold);
                    printf("  Xoff Sent: %d\n",comstat.fXoffSent);
                    printf("  Eof:       %d\n",comstat.fEof);
                    printf("  Tx Immed:  %d\n",comstat.fTxim);
                    printf("  In Que:    %d\n",comstat.cbInQue);
                    printf("  Out Que:   %d\n",comstat.cbOutQue);
#endif /* BETADEBUG */
                    if ( errorflags && deblog )
                    {
                        if ( errorflags & CE_RXOVER )
                            debug(F110,"ClearCommError","Receive Queue overflow",0);
                        if ( errorflags & CE_OVERRUN )
                            debug(F110,"ClearCommError","Receive Overrun error",0);
                        if ( errorflags & CE_RXPARITY )
                            debug(F110,"ClearCommError","Receive Parity error",0);
                        if ( errorflags & CE_FRAME )
                            debug(F110,"ClearCommError","Receive Framing error",0);
                        if ( errorflags & CE_BREAK )
                            debug(F110,"ClearCommError","Break detected",0);
                        if ( errorflags & CE_TXFULL )
                            debug(F110,"ClearCommError","TX Queue is full",0);
                        if ( errorflags & CE_PTO )
                            debug(F110,"ClearCommError","LPTx Timeout",0);
                        if ( errorflags & CE_IOE )
                            debug(F110,"ClearCommError","LPTx I/O Error",0);
                        if ( errorflags & CE_DNS )
                            debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                        if ( errorflags & CE_OOP )
                            debug(F110,"ClearCommError","LPTx Out Of Paper",0);
                    }
                    if ( deblog ) {
                        debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                        debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                        debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                        debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                        debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                        debug(F111,"ClearCommError","Eof",comstat.fEof);
                        debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                        debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                        debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
                    }
                    }
                    ow = -1;
                    Sleep(100); /* take a breather */
                }
                continue;
            }
            else if ( error == ERROR_OPERATION_ABORTED )
            {
                debug(F100,"getOverlappedIndex OPERATION ABORTED","",0);
                ResetEvent( overlapped_write[ow].hEvent ) ;
                ow_inuse[ow] = FALSE ;
                break;
            }
            else
            {
                LPVOID lpMsgBuf;
                int    freebuf = 1;
                if ( !FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                    FORMAT_MESSAGE_FROM_SYSTEM |
                                    FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL,
                                    error,
                                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     (LPTSTR) &lpMsgBuf,
                                     0,
                                     NULL)) {
                    lpMsgBuf = "";
                    freebuf = 0;
                }
                debug(F111,"getOverlappedIndex Overlapped I/O Error",
                       (char *) lpMsgBuf, error) ;
                if ( freebuf )
                    LocalFree(lpMsgBuf);

                if ( serial ) {
                    DWORD errorflags ;
                    COMSTAT comstat ;
                    ClearCommError( (HANDLE) ttyfd, &errorflags, &comstat ) ;
                    debug(F101,"getOverlappedIndex ClearCommError",
                           "",errorflags) ;
                    if ( errorflags && deblog )
                    {
                        if ( errorflags & CE_RXOVER )
                            debug(F110,"ClearCommError","Receive Queue overflow",0);
                        if ( errorflags & CE_OVERRUN )
                            debug(F110,"ClearCommError","Receive Overrun error",0);
                        if ( errorflags & CE_RXPARITY )
                            debug(F110,"ClearCommError","Receive Parity error",0);
                        if ( errorflags & CE_FRAME )
                            debug(F110,"ClearCommError","Receive Framing error",0);
                        if ( errorflags & CE_BREAK )
                            debug(F110,"ClearCommError","Break detected",0);
                        if ( errorflags & CE_TXFULL )
                            debug(F110,"ClearCommError","TX Queue is full",0);
                        if ( errorflags & CE_PTO )
                            debug(F110,"ClearCommError","LPTx Timeout",0);
                        if ( errorflags & CE_IOE )
                            debug(F110,"ClearCommError","LPTx I/O Error",0);
                        if ( errorflags & CE_DNS )
                            debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                        if ( errorflags & CE_OOP )
                            debug(F110,"ClearCommError","LPTx Out Of Paper",0);
                    }
                    if ( deblog ) {
                        debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                        debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                        debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                        debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                        debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                        debug(F111,"ClearCommError","Eof",comstat.fEof);
                        debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                        debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                        debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
                    }
                }
                ResetEvent( overlapped_write[ow].hEvent ) ;
                ow_inuse[ow] = FALSE ;
                break;
            }
        }
        debug(F111,"getOverlappedIndex COMPLETE","ow",ow);
        debug(F111,"getOverlappedIndex COMPLETE",ow_ptr[ow],ow);
        debug(F111,"getOverlappedIndex COMPLETE","nActuallyWritten",
               nActuallyWritten);
    }

    debug(F111,"GetOverlappedIndex","available ow",ow);
    return ow ;
}

int
freeOverlappedComplete( int serial ) {
    int ow, rc = -1 ;

    debug(F100,"freeOverlappedComplete","",0);

#ifdef COMMENT
    if ( owwait )       /* if owwait, then we will wait for completion */
        return ;        /* before attempting the next write            */
#endif /* COMMENT */

    /* free any which are complete */
    for ( ow=0 ; ow<maxow ; ow++ ) {
        if ( ow_inuse[ow] ) {
            if ( !owwait && !HasOverlappedIoCompleted(&overlapped_write[ow]) )
                continue;

            if ( GetOverlappedResult( (HANDLE) ttyfd,
                                      &overlapped_write[ow],
                                      &nActuallyWritten, owwait ) )
            {
                debug(F111,"freeOverlappedIndex COMPLETE","ow",ow);
                debug(F111,"freeOverlappedIndex COMPLETE",ow_ptr[ow],ow);
                debug(F111,"freeOverlappedIndex COMPLETE","nActuallyWritten",
                       nActuallyWritten);
                ow_inuse[ow] = FALSE ;
                ResetEvent( overlapped_write[ow].hEvent ) ;
                rc = nActuallyWritten ;
            }
            else
            {
                DWORD error = GetLastError() ;
                if ( error == ERROR_IO_INCOMPLETE ) {
                    debug(F111,"freeOverlappedIndex ERROR_IO_INCOMPLETE","ow",ow);
                    debug(F111,"freeOverlappedIndex waiting to complete",ow_ptr[ow],ow);
                    continue;
                }
                else if ( error == ERROR_OPERATION_ABORTED )
                {
                    debug(F111,"freeOverlappedIndex OPERATION ABORTED","ow",ow);
                    debug(F111,"freeOverlappedIndex OPERATION ABORTED",ow_ptr[ow],ow);
                    ow_inuse[ow] = FALSE ;
                    // ResetEvent( overlapped_write[ow].hEvent ) ;
                    rc = -1;
                }
                else
                {
                    debug(F101,"freeOverlappedComplete Overlapped I/O Error",
                           "",error ) ;
                    if ( serial ) {
                    DWORD errorflags ;
                    COMSTAT comstat ;
                    ClearCommError( (HANDLE) ttyfd, &errorflags, &comstat ) ;
                    debug(F101,"freeOverlappedComplete ClearCommError",
                           "",errorflags) ;
                    if ( errorflags && deblog )
                    {
                        if ( errorflags & CE_RXOVER )
                            debug(F110,"ClearCommError","Receive Queue overflow",0);
                        if ( errorflags & CE_OVERRUN )
                            debug(F110,"ClearCommError","Receive Overrun error",0);
                        if ( errorflags & CE_RXPARITY )
                            debug(F110,"ClearCommError","Receive Parity error",0);
                        if ( errorflags & CE_FRAME )
                            debug(F110,"ClearCommError","Receive Framing error",0);
                        if ( errorflags & CE_BREAK )
                            debug(F110,"ClearCommError","Break detected",0);
                        if ( errorflags & CE_TXFULL )
                            debug(F110,"ClearCommError","TX Queue is full",0);
                        if ( errorflags & CE_PTO )
                            debug(F110,"ClearCommError","LPTx Timeout",0);
                        if ( errorflags & CE_IOE )
                            debug(F110,"ClearCommError","LPTx I/O Error",0);
                        if ( errorflags & CE_DNS )
                            debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                        if ( errorflags & CE_OOP )
                            debug(F110,"ClearCommError","LPTx Out Of Paper",0);
                    }
                    }
                    ResetEvent( overlapped_write[ow].hEvent ) ;
                    ow_inuse[ow] = FALSE ;
                    rc = -1 ;
                }
            }
        }
    }
    return rc ;
}
#endif /* NT */

/*  T T X O U T -- Similar to ttxin, but for writing.  */
/*                 Performs conversions as necessary.  */
/*
  Outputs all n characters of s, or else fails with -2 if the
  connection is broken, or with -1 upon some other kind of error.
  Relies on ttol() or os2_netxout() to perform the real work.
*/

int
ttxout(char *s, int n) {
    int rc = 0, i=0 ;
#ifndef NOLOCAL
    extern int tt_pacing;               /* output pacing */

    if ( tt_pacing != 0 )
    {
        if ( ttprty ) {
            for ( ;i<n;i++ ) {
                ttoc(dopar(s[i]));
                msleep(tt_pacing);
            }
        }
    }
    else
#endif /* NOLOCAL */
    {
        if ( ttprty ) {
            for ( ;i<n;i++ ) {
                s[i] = dopar(s[i]);
            }
        }

        if (network)
            rc = os2_netxout(s,n);
        else
            rc = ttol(s,n);
    }
    return(rc);
}

#ifdef NT
int
OverlappedWrite( int serial, char * chars, int charsleft )
{
    DWORD i ;
    int ow = 0, iopending = 0 ;
    int rc = 0;

    debug(F111,"OverlappedWrite","charsleft",charsleft);
    ow = getOverlappedIndex(serial) ;

    if ( ow_size[ow] < charsleft ) {
        if ( ow_ptr[ow] ) {
            debug(F111,"OverlappedWrite","free(ow_ptr[ow])",ow);
            free( ow_ptr[ow] );
            ow_ptr[ow] = NULL;
            ow_size[ow] = 0;
        }

        ow_ptr[ow] = (char *) malloc( charsleft * 2 );
        if ( !ow_ptr[ow] ) {
            debug(F110,"OverlappedWrite","malloc() failed",0);
            return -1;
        }
        ow_size[ow] = charsleft * 2 ;
    }

    debug(F111,"OverlappedWrite chars to send",ow_ptr[ow],charsleft);
    memcpy( ow_ptr[ow], chars, charsleft );

    if ( overlapped_write[ow].hEvent == (HANDLE) -1 )
    {
        overlapped_write[ow].hEvent = CreateEvent( NULL,    // no security
                                                   TRUE,    // explicit reset req
                                                   FALSE,   // initial event reset
                                                   NULL ) ; // no name
        if ( overlapped_write[ow].hEvent == (HANDLE) -1 )
            debug(F101,"Overlapped Write CreateEvent error","",GetLastError() ) ;
    }
    overlapped_write[ow].Offset = overlapped_write[ow].OffsetHigh = 0 ;
    ResetEvent( overlapped_write[ow].hEvent ) ;
    nActuallyWritten = 0 ;

    if ( !WriteFile( (HANDLE) ttyfd, ow_ptr[ow], charsleft, &nActuallyWritten,
                     &overlapped_write[ow]) )
    {
        DWORD error = GetLastError() ;
        if ( error != ERROR_IO_PENDING )
        {
            debug(F101,"OverlappedWrite WriteFile - real error occurred","",error) ;
            ResetEvent( overlapped_write[ow].hEvent ) ;
            ow_inuse[ow] = FALSE ;
            return -1 ;
        }
        else {
            debug(F111,"OverlappedWrite WriteFile ERROR_IO_PENDING","ow",ow) ;
            ow_inuse[ow] = TRUE ;
            rc = charsleft;
            iopending = TRUE ;
        }
    }
    else {
        debug(F111,"OverlappedWrite WriteFile COMPLETE","nActuallyWritten",
               nActuallyWritten) ;
        rc = nActuallyWritten;
        ResetEvent( overlapped_write[ow].hEvent ) ;
        ow_inuse[ow] = FALSE ;
    }

    i = freeOverlappedComplete(serial) ;
    if ( owwait && iopending )
        rc = i ;

    return(rc);
}


int
OverlappedWriteInit( void )
{
    int i;
    for ( i=0;i<maxow;i++ )
        ow_inuse[i] = FALSE;
    return(0);
}

static int OldReadPending = FALSE ;
static int NextReadPending = 0;

#ifdef NEWRDCH
int
OverlappedReadInit( void )
{
    int i;

    debug(F111,"OverlappedReadInit","OldReadPending",OldReadPending);

    if ( OldReadPending )
        OverlappedReadCleanup();

    if ( owwait )
        return(0);

    for ( i=0; i < maxow; i++ ) {
        if ( overlapped_read[i].hEvent == (HANDLE) -1 ) {
            overlapped_read[i].hEvent = CreateEvent( NULL,   /* no security */
                                                     TRUE,   /* explicit reset req */
                                                     FALSE,  /* initial event set */
                                                     NULL    /* no name */
                                                     );
            }
            if (overlapped_read[i].hEvent == (HANDLE) -1)
                debug(F101,"OverlappedRead CreateEvent error","",GetLastError() ) ;
            overlapped_read[i].Offset = overlapped_read[i].OffsetHigh = 0 ;
            or_ptr[i] = malloc(32768);
            if ( or_ptr[i] == NULL ) {
                debug(F101,"OverlappedRead malloc error","",GetLastError() ) ;
                or_size[i] = 0;
            }
            else
                or_size[i] = 32768;
            ResetEvent( overlapped_read[i].hEvent );
            ReadFile((HANDLE) ttyfd, or_ptr[i], or_size[i], &or_read[i],
                      &overlapped_read[i]);
    }
    OldReadPending = TRUE;
    return(1);
}

int
OverlappedReadFlush( void )
{
}

int
OverlappedReadCleanup( void )
{
    int i;
    OverlappedReadFlush();

    for ( i=0; i < 5; i++ ) {
        if ( overlapped_read[i].hEvent != (HANDLE) -1 ) {
            CloseHandle(overlapped_read[i].hEvent);
            overlapped_read[i].hEvent = (HANDLE) -1;
        }
        overlapped_read[i].Offset = overlapped_read[i].OffsetHigh = 0 ;
        free(or_ptr[i]);
        or_ptr[i];
        or_size[i] = 0;
    }
    OldReadPending = FALSE;
    return(1);
}

int
OverlappedDataWaiting( void )
{
    int i;
    int count = 0;
    for ( i=0;i<maxow;i++ )
        count += or_read[i];
    return(count);
}

int
WaitForOverlappedReadData( int timo )
{
    int rc=0;
    if ( !OldReadPending )
        return TRUE;

    if ( OverlappedDataWaiting() )
        return TRUE;

    rc = WaitForSingleObjectEx( overlapped_read[NextReadPending].hEvent,
        timo < 0 ? -timo : timo == 0 ? -1 :timo*1000, TRUE );
    if ( rc != WAIT_OBJECT_0 )
        return FALSE;
    return TRUE;
}

int
OverlappedRead( int serial, char * buf, int bufsize, int timo )
{
    int len=0;


#ifdef BETADEBUG
    if ( bufsize < 32768 )
        printf("bufsize too small (%d)- potential data loss\n",
                bufsize);
#endif /* BETADEBUG */

    if ( !OldReadPending ) {
        debug(F110,"rdch","!OldReadPending",0);
        OverlappedReadInit();
    }

    debug(F111,"rdch","NextReadPending",NextReadPending);

    if (OldReadPending
#ifdef COMMENT
         ||         !ReadFile((HANDLE) ttyfd,
                    buf,
                    bufsize,
                    &nActuallyRead,
                    &overlapped_read)
#endif /* COMMENT */
         ) {
        DWORD error = GetLastError() ;
        nActuallyRead = 0;
        if ( OldReadPending || error == ERROR_IO_PENDING ) {
            int timedout=FALSE;
#ifndef COMMENT
            debug(F100,"rdch ReadFile ERROR_IO_PENDING","",0);
#endif /* COMMENT */
            while(!GetOverlappedResult( (HANDLE) ttyfd,
                                        &overlapped_read[NextReadPending],
                                        &nActuallyRead,
                                        FALSE )
                   ) {
                DWORD error = GetLastError() ;
                if ( error == ERROR_IO_INCOMPLETE ) {
                    int rc=0;
                    debug(F100,"rdch ReadFile ERROR_IO_INCOMPLETE","",0);
                    if ( timedout ) {
                        debug(F110,"rdch ReadFile","timedout",0);
                        OldReadPending = TRUE;
                        return(-1);
                    }
                    rc = WaitForSingleObjectEx( overlapped_read[NextReadPending].hEvent,
                                                timo, TRUE ) ;
                    debug(F111,"rdch","NextReadPending",NextReadPending);
                    debug(F111,"rdch WaitForSingleObjectEx","rc",rc);
                    if ( rc != WAIT_OBJECT_0 )
                        timedout = TRUE;
                    continue;
                } else if ( error == ERROR_OPERATION_ABORTED ) {
                    debug(F100,"rdch ReadFile OPERATION ABORTED","",0);
                    ResetEvent( overlapped_read[NextReadPending].hEvent ) ;
                    if ( nActuallyRead > 0 )
                        memcpy(buf,or_ptr[NextReadPending],
                                bufsize<nActuallyRead?bufsize:nActuallyRead);
                    ReadFile((HANDLE) ttyfd, or_ptr[NextReadPending],
                              or_size[NextReadPending],
                              &or_read[NextReadPending],
                              &overlapped_read[NextReadPending]);
                    or_read[NextReadPending] = 0;
                    NextReadPending++;
                    NextReadPending %= maxow;
                    if ( nActuallyRead > 0 )
                        return(nActuallyRead);
                    else
                        return (-1); /* fdc */
                } else {
                    if ( serial ) {
                    DWORD errorflags ;
                    COMSTAT comstat ;
                    debug(F101,"rdch ReadFile Overlapped I/O Error",
                           "",error);
                    ClearCommError( (HANDLE) ttyfd,
                                    &errorflags,
                                    &comstat );
                    debug(F101,"rdch ClearCommError","",errorflags);
                    if ( errorflags && deblog )
                    {
                        if ( errorflags & CE_RXOVER )
                            debug(F110,"ClearCommError","Receive Queue overflow",0);
                        if ( errorflags & CE_OVERRUN )
                            debug(F110,"ClearCommError","Receive Overrun error",0);
                        if ( errorflags & CE_RXPARITY )
                            debug(F110,"ClearCommError","Receive Parity error",0);
                        if ( errorflags & CE_FRAME )
                            debug(F110,"ClearCommError","Receive Framing error",0);
                        if ( errorflags & CE_BREAK )
                            debug(F110,"ClearCommError","Break detected",0);
                        if ( errorflags & CE_TXFULL )
                            debug(F110,"ClearCommError","TX Queue is full",0);
                        if ( errorflags & CE_PTO )
                            debug(F110,"ClearCommError","LPTx Timeout",0);
                        if ( errorflags & CE_IOE )
                            debug(F110,"ClearCommError","LPTx I/O Error",0);
                        if ( errorflags & CE_DNS )
                            debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                        if ( errorflags & CE_OOP )
                            debug(F110,"ClearCommError","LPTx Out Of Paper",0);
                    }
                    if ( deblog ) {
                        debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                        debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                        debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                        debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                        debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                        debug(F111,"ClearCommError","Eof",comstat.fEof);
                        debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                        debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                        debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
                    }
                    }
                    ResetEvent( overlapped_read[NextReadPending].hEvent ) ;
                    if ( nActuallyRead > 0 )
                        memcpy(buf,or_ptr[NextReadPending],
                                bufsize<nActuallyRead?bufsize:nActuallyRead);
                    ReadFile((HANDLE) ttyfd, or_ptr[NextReadPending],
                              or_size[NextReadPending],
                              &or_read[NextReadPending],
                              &overlapped_read[NextReadPending]);
                    or_read[NextReadPending] = 0;
                    NextReadPending++;
                    NextReadPending %= maxow;
                    if ( nActuallyRead > 0 ) {
                        return(nActuallyRead);
                    }
                    return -1;
                }
            }
        } else {
            if ( serial ) {
            DWORD errorflags ;
            COMSTAT comstat ;
            debug(F101,"rdch ReadFile - real error occurred","",error) ;
            ClearCommError( (HANDLE) ttyfd,
                            &errorflags,
                            &comstat );
            debug(F101,"rdch ClearCommError","",errorflags);
            if ( errorflags && deblog )
            {
                if ( errorflags & CE_RXOVER )
                    debug(F110,"ClearCommError","Receive Queue overflow",0);
                if ( errorflags & CE_OVERRUN )
                    debug(F110,"ClearCommError","Receive Overrun error",0);
                if ( errorflags & CE_RXPARITY )
                    debug(F110,"ClearCommError","Receive Parity error",0);
                if ( errorflags & CE_FRAME )
                    debug(F110,"ClearCommError","Receive Framing error",0);
                if ( errorflags & CE_BREAK )
                    debug(F110,"ClearCommError","Break detected",0);
                if ( errorflags & CE_TXFULL )
                    debug(F110,"ClearCommError","TX Queue is full",0);
                if ( errorflags & CE_PTO )
                    debug(F110,"ClearCommError","LPTx Timeout",0);
                if ( errorflags & CE_IOE )
                    debug(F110,"ClearCommError","LPTx I/O Error",0);
                if ( errorflags & CE_DNS )
                    debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                if ( errorflags & CE_OOP )
                    debug(F110,"ClearCommError","LPTx Out Of Paper",0);
            }
            if ( deblog ) {
                debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                debug(F111,"ClearCommError","Eof",comstat.fEof);
                debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
            }
            }
            ResetEvent( overlapped_read[NextReadPending].hEvent ) ;
            if ( nActuallyRead > 0 )
                memcpy(buf,or_ptr[NextReadPending],
                        bufsize<nActuallyRead?bufsize:nActuallyRead);
            ReadFile((HANDLE) ttyfd, or_ptr[NextReadPending],
                      or_size[NextReadPending],
                      &or_read[NextReadPending],
                      &overlapped_read[NextReadPending]);
            or_read[NextReadPending] = 0;
            NextReadPending++;
            NextReadPending %= maxow;
            if ( nActuallyRead > 0 ) {
                return(nActuallyRead);
            }
            return -2 ;
        }
    }
    ResetEvent( overlapped_read[NextReadPending].hEvent ) ;
#ifndef COMMENT
    debug(F101,"rdch nActuallyRead","",nActuallyRead ) ;
    debug(F101,"rdch overlapped_read.Offset","",
           overlapped_read[NextReadPending].Offset) ;
#endif /* COMMENT */
    if ( nActuallyRead > 0 )
        memcpy(buf,or_ptr[NextReadPending],
                bufsize<nActuallyRead?bufsize:nActuallyRead);
    ReadFile((HANDLE) ttyfd, or_ptr[NextReadPending],
              or_size[NextReadPending],
              &or_read[NextReadPending],
              &overlapped_read[NextReadPending]);
    or_read[NextReadPending] = 0;
    NextReadPending++;
    NextReadPending %= maxow;
    return(nActuallyRead);
}
#endif /* NEWRDCH */
#endif /* NT */


/*  T T O L  --  Similar to "ttinl", but for writing.  */
#ifdef CK_ENCRYPTION
static CHAR * xpacket = NULL;
static int nxpacket = 0;
#endif /* CK_ENCRYPTION */

int
ttol(CHAR *s, int n) {
    UINT i;
    int  rc = 0 ;
    int  charsleft;
    CHAR *chars;
#ifdef CKXXCHAR
    extern int dblflag;                 /* For SET SEND DOUBLE-CHAR */
    extern short dblt[];
    CHAR *p = NULL, *p2, *s2, c;
    int n2 = 0;

    /*  Double any characters that must be doubled.  */
    debug(F101,"ttol dblflag","",dblflag);
    if (dblflag) {
        p = (CHAR *) malloc(n + n + 1);
        if (p) {
            s2 = s;
            p2 = p;
            n2 = 0;
            while (*s2) {
                c = *s2++;
                *p2++ = c;
                n2++;
                if (dblt[(unsigned) c] & 2) {
                    *p2++ = c;
                    n2++;
                }
            }
            s = p;
            n = n2;
        }
    }
#endif /* CKXXCHAR */

#ifdef CK_ENCRYPTION
/*
  This is to avoid encrypting a packet that is already encrypted, e.g.
  when we resend a packet directly out of the packet buffer, and also to
  avoid encrypting a constant (literal) string, which can cause a memory
  fault.
*/
    if (TELOPT_ME(TELOPT_ENCRYPTION)) {
        int x;
        if (nxpacket < n) {
            if (xpacket) {
                free(xpacket);
                xpacket = NULL;
                nxpacket = 0;
            }
            x = n > 10240 ? n : 10240;
            xpacket = (CHAR *)malloc(x);
            if (!xpacket) {
                fprintf(stderr,"ttol malloc failure\n");
                return(-1);
            } else
              nxpacket = x;
        }
        memcpy((char *)xpacket,(char *)s,n);
        s = xpacket;
    }
#endif /* CK_ENCRYPTION */


    chars = s;
    charsleft = n;

    while ( rc >= 0 && charsleft > 0 ) {
#ifdef NETCONN
        if (network) {
            debug(F111,"ttol()","charsleft",charsleft);
            rc = os2_nettol(chars,charsleft);
            debug(F111,"ttol()","os2_nettol() returns",rc);
        } else {
#endif /* NETCONN */
#ifdef COMMENT
            if (ttchk() < 0)
                rc = -1 ;               /* Not open. */
            else
#endif /* COMMENT */
            {
#ifdef NT
                rc = OverlappedWrite( TRUE, chars, charsleft );
#else /* NT */
                if(DosWrite(ttyfd,chars,charsleft,(PVOID)&i))
                    rc = -1 ;
                else
                    rc = i ;
#endif /* NT */
            }
#ifdef NETCONN
        }
#endif /* NETCONN */

        if ( rc >= 0 ) {
            charsleft -= rc ;
            chars += rc ;
        }
    }
    debug( F101, "ttol returns","",rc >= 0 ? (n-charsleft) : rc ) ;

#ifdef CKXXCHAR
    if (p) free(p);
#endif /* CKXXCHAR */
    if ( rc < 0 ) {
        if ( ttchk() < 0 )
            return -2;          /* connection dropped */
        else
            return -1;          /* soft error - timeout ? */
    }
    else
        return( n - charsleft ) ;
}


/*  T T O C  --  Output a character to the communication line  */
int
ttoc(char c) {
    int rc = 0 ;
#ifdef NT
    DWORD i ;
    int ow = 0 ;
#else /* NT */
    UINT i;
#endif /* NT */
    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
) return(-1);           /* Not open. */
#ifdef NETCONN
    if (network)
        return os2_nettoc(c);
#endif /* NETCONN */
#ifdef NT
#ifndef COMMENT
     rc = OverlappedWrite( TRUE, &c, 1 );
#else /* COMMENT */
    ow = getOverlappedIndex() ;

    if ( ow_size[ow] < 1 ) {
        if ( ow_ptr[ow] ) {
            free( ow_ptr[ow] );
            ow_ptr[ow] = NULL;
            ow_size[ow] = 0;
        }

        ow_ptr[ow] = (char *) malloc( 128 );
        if ( !ow_ptr[ow] ) {
            return -1;
        }
        ow_size[ow] = 128 ;
    }
    ow_ptr[ow][0] = c;
    ow_ptr[ow][1] = NUL;
    debug(F111,"ttoc chars to send",ow_ptr[ow],1);

    if ( overlapped_write[ow].hEvent == (HANDLE) -1 )
    {
        overlapped_write[ow].hEvent = CreateEvent( NULL,    // no security
                                                   TRUE,    // explicit reset req
                                                   FALSE,   // initial event reset
                                                   NULL ) ; // no name
        if ( overlapped_write[ow].hEvent == (HANDLE) -1 )
        {
            debug(F101,"ttoc CreateEvent error","",GetLastError() ) ;
        }
    }
    overlapped_write[ow].Offset = overlapped_write[ow].OffsetHigh = 0 ;
    ResetEvent( overlapped_write[ow].hEvent ) ;

    nActuallyWritten = 0 ;
    if ( !WriteFile( (HANDLE) ttyfd, ow_ptr[ow], 1, &nActuallyWritten,
                     &overlapped_write[ow]) )
    {
        DWORD error = GetLastError() ;
        if ( error != ERROR_IO_PENDING )
        {
            debug(F101,"ttoc WriteFile - real error occurred","",error) ;
            ResetEvent( overlapped_write[ow].hEvent ) ;
            ow_inuse[ow] = FALSE ;
            return -1 ;
        }
        else
            ow_inuse[ow] = TRUE ;
    }
    else {
        ResetEvent( overlapped_write[ow].hEvent ) ;
        ow_inuse[ow] = FALSE ;
    }
#ifndef COMMENT
    debug(F101,"ttoc nActuallyWritten","",nActuallyWritten ) ;
#endif /* COMMENT */

    freeOverlappedComplete() ;
    rc = 1 ;
#endif /* COMMENT */
#else /* NT */
    if(DosWrite(ttyfd,&c,1,(PVOID)&i))
       rc = -1;
    else rc = i;
#endif /* NT */
    return rc ;
}

#ifndef NOTTOCI
#define NEWTTOCI
#endif /* NOTTOCI */

/*  T T O C I  --  Output a character to the communication line immediately */
#ifdef NEWTTOCI
int
ttoci(char c) {
    int x;
    BYTE i;
    ULONG Data = 0L ;
#ifdef NT
   DWORD errors ;
   COMSTAT comstat ;
#endif /* NT */

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
) return(-1);          /* Not open. */
#ifdef NETCONN
    if (network)
        return os2_nettoc(c);
#endif /* NETCONN */

#ifdef NT
    if ( ttoc(c) )                      /* write the character first */
        return(0);

    /* if an error occurs, then try to clear it and try again */
    ClearCommError( (HANDLE) ttyfd, &errors, &comstat ) ;
    if ( errors && deblog )
    {
        debug( F101, "ttoci: ClearCommError errors","",errors ) ;
        if ( errors & CE_RXOVER )
            debug(F110,"ClearCommError","Receive Queue overflow",0);
        if ( errors & CE_OVERRUN )
            debug(F110,"ClearCommError","Receive Overrun error",0);
        if ( errors & CE_RXPARITY )
            debug(F110,"ClearCommError","Receive Parity error",0);
        if ( errors & CE_FRAME )
            debug(F110,"ClearCommError","Receive Framing error",0);
        if ( errors & CE_BREAK )
            debug(F110,"ClearCommError","Break detected",0);
        if ( errors & CE_TXFULL )
            debug(F110,"ClearCommError","TX Queue is full",0);
        if ( errors & CE_PTO )
            debug(F110,"ClearCommError","LPTx Timeout",0);
        if ( errors & CE_IOE )
            debug(F110,"ClearCommError","LPTx I/O Error",0);
        if ( errors & CE_DNS )
            debug(F110,"ClearCommError","LPTx Device Not Selected",0);
        if ( errors & CE_OOP )
            debug(F110,"ClearCommError","LPTx Out Of Paper",0);
    }
    Data = comstat.fCtsHold |
           comstat.fDsrHold << 1 |
           comstat.fRlsdHold << 2 |
           comstat.fXoffHold << 3 |
           comstat.fXoffSent << 4 |
           comstat.fEof << 5 |
           comstat.fTxim << 6 ;
    if ( Data ) {
        debug( F101,"ttoci: COM Status","",Data) ;

        if ( (flow == FLO_XONX &&
              comstat.fXoffHold) ||
             (flow == FLO_RTSC &&
               comstat.fCtsHold) )
            return(Data) ;
    }
    return ttoc(c) == 1 ? 0 : -1;
#else /* NT */
    x =
     DosDevIOCtl(&Data,sizeof(Data),0,0,ASYNC_GETCOMMSTATUS,IOCTL_ASYNC,ttyfd);
    if ( Data ) {
        debug( F101,"ttoci: Query COM Status","",Data) ;
        return(Data) ;
    }
    x = DosWrite(ttyfd,&c,1,(PVOID)&i) ;
    if (x) {
        debug(F101,"ttoci failure status","",x);
        return(x);
    } else return(0);
#endif /* NT */
}

#else /* NEWTTOCI */

int
ttoci(char c) {
    int x;
    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return(-1);           /* Not open. */
#ifdef NETCONN
    if (network)
        return os2_nettoc(c);
#endif /* NETCONN */
#ifdef NT
    return TransmitCommChar( ttyfd, c ) ? 0 : -1 ;
#else /* NT */
    x =  DosDevIOCtl(NULL,0,&c,sizeof(c),ASYNC_TRANSMITIMM,IOCTL_ASYNC,ttyfd);
#endif /* NT */
    if (x) {
        debug(F101,"ttoci failure status","",x);
        return(x);
    } else return(0);
}
#endif /* NEWTTOCI */

static int inlret ;
static CHAR * inldest, inleol, inlstart ;
static int inlmax, inlturn ;

static int
ckcgetc(int dummy) {
    return ttinc(1);
}

#ifndef NOXFER
/*  T T I N L  --  Read a packet from the communication device.  */
/*
  blah blah
*/
ttinl(CHAR *dest, int max, int timo, CHAR eol, CHAR start, int turn) {
    extern int xfrcan, xfrchr, xfrnum;  /* Defined in ckcmai.c */
    extern int priority;
    int x=0, c=0, ccn=0;
    /* register */ int i=0, j=0, m=0, n=0;      /* Local variables */
    int timespent = 0 ;
    int flag = 0;
    unsigned char ch;
    int pktlen = -1;
    int lplen = 0;
    int havelen = 0;
    extern int server, srvidl, rpsiz ;
    int srvtimer=0 ;
    int sopmask = 0xff;
#ifdef CKXXCHAR
    extern short dblt[];                /* Ignore-character table */
    extern int ignflag;
#endif /* CKXXCHAR */
#ifdef IKS_OPTION
    extern int stchr;
#endif /* IKS_OPTION */
#ifdef STREAMING
    extern int streaming;
    extern int sndtyp;
#endif /* STREAMING */
    extern int tcp_incoming, idletmo;
    int canread=0, haveread=0;
#define INXBUF_SIZE 10240
    static char inxbuf[INXBUF_SIZE];
    time_t start_time = time(NULL);

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return(-3);          /* Not open. */

    inlret = -1 ;                       /* Assume we are going to fail */
    inldest = dest ;
    inlmax = max ;
    inleol = eol ;
    inlstart = start ;
    inlturn = turn ;

    debug(F101,"ttinl max","",max);
    debug(F101,"ttinl timo","",timo);
    debug(F101,"ttinl network","",network);
    debug(F101,"ttinl ttnproto","",ttnproto);

   *dest = '\0';                       /* Clear destination buffer */
   if (timo < 0) timo = 0;              /* Safety */

    debug(F101,"ttinl start","",inlstart);
    flag = 0;                           /* Start of packet flag */

    ttpmsk = m = (ttprty) ? 0177 : 0377; /* Set parity stripping mask. */
    sopmask = needpchk ? 0177 : ttpmsk;  /* And SOP mask */

/* Now read into destination, stripping parity and looking for the */
/* the packet terminator, and also for two Ctrl-C's typed in a row. */

    i = 0;                              /* Destination index */
    debug(F101,"ttinl eol","",inleol);

    do
    {
        canread = ttchk();
        if ( canread > 0 ) {
            int blah=pktlen-i+5;
            int pleaseread;

            if ( !havelen || blah < 0 ) {
#ifdef COMMENT
                /* causes us to read too much data when the peer as been issued */
                /* a SET SEND PACKET-SIZE which is smaller than our SET RECEIVE */
                /* PACKET-SIZE value.  */
                blah = rpsiz < LOCAL_ECHO_BUFSIZE ? rpsiz : LOCAL_ECHO_BUFSIZE;
#else
                /* Enough to get the size */
                blah = 40;
#endif
            }

            debug(F111,"ttinl","blah",blah);
            debug(F111,"ttinl","rpsiz",rpsiz);
            debug(F111,"ttinl","canread",canread);

            if ( blah > 0 && blah < canread && blah < INXBUF_SIZE )
                pleaseread = blah;
            else if ( canread < INXBUF_SIZE )
                pleaseread = canread;
            else
                pleaseread = INXBUF_SIZE;

            debug(F111,"ttinl","will attempt to read",pleaseread);

            haveread = ttxin( pleaseread,inxbuf);
#ifdef IKS_OPTION
            if (haveread >= 0) {
                inlstart = stchr;
                if (TELOPT_ME(TELOPT_KERMIT) &&
                     !TELOPT_SB(TELOPT_KERMIT).kermit.me_start &&
                     server)
                {   /* I'm not in Server mode */
                    debug(F100,"ttinl TELOPT_ME_KERMIT && !me_start && server","",0);
                    if ( haveread ) {
                        debug(F111,"ttinl putting back chars","haveread",
                               haveread);
                        le_puts(inxbuf, haveread);
                    }
                    return(-2);         /* End server mode */
                }
#ifdef IKSD
                if (TELOPT_U(TELOPT_KERMIT) &&
                     TELOPT_SB(TELOPT_KERMIT).kermit.u_start &&
                     !inserver && server && !tcp_incoming)
                {   /* Remote in Server mode */
                    if ( haveread ) {
                        debug(F111,"ttinl putting back chars","haveread",
                               haveread);
                        le_puts(inxbuf, haveread);
                    }
                    return(-2);         /* End server mode */
                }
#endif /* IKSD */
            }
#endif /* IKS_OPTION */
        }
        else if ( canread == 0 ) {
            debug(F100,"ttinl canread == 0","",0);
#ifdef STREAMING
            if (streaming && sndtyp == 'D')
                return(0);
#endif /* STREAMING */

            if (
#ifdef IKSD
                 !inserver &&
#endif /* IKSD */
                 conchk() > 0 ) {
                debug(F100,"ttinl conchk() > 0","",0);
                return(-1);         /* User pressed a key */
            }
#ifdef COMMENT
#ifdef STREAMING
            if (streaming && sndtyp == 'D')
                return(-1);
#endif /* STREAMING */
#endif /* COMMENT */
            if ((c=ttinc((timo||srvidl)?1:0))<0) {
                if ( c < -1 ) {
                    debug(F101,"ttinl ttinc() returned error","",c);
                    ttclos(0);
                    return(-3);
                }
                else {
                    debug(F101,"ttinl ttinc() returned timeout","",c);
                    timespent = time(NULL) - start_time;
                    if ( server && srvidl )
                    {
                        if ( timespent > srvidl )
                        {
#ifdef IKSD
                            if ( inserver ) {
                                printf("\r\nIKS IDLE TIMEOUT: %d sec\r\n", srvidl);
                                doexit(GOOD_EXIT,xitsta);
                            }
#endif /* IKSD */
                            idletmo = 1;
                            return(-2) ;
                        }
                    }
                    continue;
                }
            } else if (c == IAC && (network && IS_TELNET()
#ifdef IKSD
                                      || inserver
#endif /* IKSD */
                                     )) {
                int tx=0;
                extern int duplex;
                debug(F100,"ttinl got IAC","",0);
                if ((tx = tn_doop((CHAR)(c & 0xff),duplex,ckcgetc)) == 0) {
                    continue;
                }
                else if (tx == -1) {    /* I/O error */
                    return(-3);
                }
                else if (tx == -2 || tx == -3) { /* Connection failed. */
                    ttclos(0) ;
                    return(-3);
                }
                else if (tx == 1) {     /* ECHO change */
                    duplex = 1; /* Get next char */
                    continue;
                }
                else if (tx == 2) {     /* ECHO change */
                    duplex = 0; /* Get next char */
                    continue;
                }
                else if (tx == 3) {     /* Quoted IAC */
                    c = 255;    /* proceeed with it. */
                }
#ifdef IKS_OPTION
                else if (tx == 4) {
                    if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start &&
                         server
                         /* Remote in Server mode */
#ifdef IKSD
                        && !inserver
#endif /* IKSD */
                         ) {
                        debug(F100,"ttinl u_start && server && !inserver","",0);
                        return(-2);         /* End server mode */
                    }
                    if (!TELOPT_SB(TELOPT_KERMIT).kermit.me_start &&
                         server) {
                        debug(F100,"ttinl me_start && server","",0);
                        return(-2);
                    }
                }
                else if (tx == 5) {     /* IKS State Change */
                    extern int stchr;
                    inlstart = stchr;
                }
#endif /* IKS_OPTION */
                else if (tx == 6) {
                    /* Remote Logout */
                    ttclos(0);
#ifdef IKSD
                    if (inserver && !local)
                        doexit(GOOD_EXIT,0);
                    else
#endif /* IKSD */
                        return(-2);
                }
                else continue;  /* Unknown, get next char */
            }
            inxbuf[0] = (char) c;
            haveread = 1;
        }
        else /* canread < 0 == error */ {
            debug(F101,"ttinl ttchk() returned error","",n);
            ttclos(0);
            return (-3);         /* i/o error or no connection */
        }

        ckhexdump("ttinl() ttin?() returned",inxbuf,haveread);
        j = 0;
        while ( j < haveread )
        {
            n = inxbuf[j++];
            ch = n & 0xff;
#ifdef CKXXCHAR
            if (ignflag)
                if (dblt[(unsigned) ch] & 1) /* Character to ignore? */
                    continue;
#endif /* CKXXCHAR */

            /* Check cancellation */
            if (xfrcan && ((ch & ttpmsk) == xfrchr)) {
                if (++ccn >= xfrnum) {  /* If xfrnum in a row, bail out. */
                    if (xfrchr < 32)
                      printf("^%c...\r\n",(char)(xfrchr+64));
                    else
                      printf("Canceled...\r\n");
                    return(-2);
                }
            } else ccn = 0;             /* No cancellation, reset counter, */

            if (flag == 0) {            /* Find the Start of Packet */
                if ((n & sopmask) == inlstart) /* Got it! */
                    flag = 1;
                else {                  /* Keep looking */
                    debug(F101,"ttinl skipping","",n);
#ifndef NOLOCAL
#ifdef BETATEST_X
                    cwrite(n);
#endif /* BETATEST */
#endif /* NOLOCAL */
                    continue;
                }
            }
            inldest[i++] = n & ttpmsk;

            if (!havelen) {
                /* Try to compute the packet length */
                if (i == 2) {
                    pktlen = xunchar(inldest[1] & ttpmsk);
                    havelen = (pktlen > 1);
                    debug(F101,"ttinl length","",pktlen);
                } else if (i == 5 && pktlen == 0) {
                    lplen = xunchar(inldest[4] & ttpmsk);
                } else if (i == 6 && pktlen == 0) {
                    pktlen = lplen * 95 + xunchar(inldest[5] & ttpmsk) + 5;
                    havelen = 1;
                    debug(F101,"ttinl extended length","",pktlen);
                }

/*
  Suppose we looked at the sequence number here and found it was out of
  range?  This would mean either (a) incoming packets had SOP unprefixed
  and we are out of sync, or (b) the packet is damaged.  Since (a) is bad
  practice, let's ignore it.  So what should we do here if we know the
  packet is damaged?

   1. Nothing -- keep trying to read the packet till we find what we think
      is the end, or we time out, and let the upper layer decide what to
      do.  But since either the packet is corrupt or we are out of sync,
      our criterion for finding the end does not apply and we are likely
      to time out (or swallow a piece of the next packet) if our assumed
      length is too long.  (This was the behavior prior to version 7.0.)

   2. set flag = 0 and continue?  This would force us to wait for the
      next packet to come in, and therefore (in the nonwindowing case),
      would force a timeout in the other Kermit.

   3. set flag = 0 and continue, but only if the window size is > 1 and
      the window is not blocked?  Talk about cheating!

   4. Return a failure code and let the upper layer decide what to do.
      This should be equivalent to 3, but without the cheating.  So let's
      do it that way...
*/
                if (i == 3) {                   /* Peek at sequence number */
                    x = xunchar(dest[2]);       /* If it's not in range... */
                    if (x < 0 || x > 63) {
                        debug(F111,"ttinl xunchar(dest[2])",dest,x);
                        return(-1);             /* return a nonfatal error */
                    }
                }
            }
            else if ((i > pktlen+1) &&
                     (!inlturn || (inlturn && (n & ttpmsk) == inleol)) ) {
                /* We have the end of the packet */
                inldest[i] = '\0';              /* Terminate the string, */

                if (deblog) {
                    if ((n & ttpmsk) != inleol) {
                        debug(F101,"ttinl EOP length","",pktlen);
                        debug(F101,"ttinl i","",i);
                    } else debug(F101,"ttinl got eol","",inleol);

                    debug(F101,"ttinl needpchk","",needpchk);
                    debug(F101,"ttinl ttprty","",ttprty);
                    debug(F101,"ttinl ttpmsk","",ttpmsk);
                }

                /* Parity checked yet? */
                if ( needpchk ) {
                    if (ttprty == 0) {
                        if ((ttprty = parchk(inldest,inlstart,i)) > 0) { /* No */
                            int j;
                            debug(F101,"ttinl senses parity","",ttprty);
                            debug(F110,"ttinl packet before",inldest,0);
                            ttpmsk = 0x7f;
                            for (j = 0; j < i; j++)
                                inldest[j] &= ttpmsk;   /* Strip parity from packet */
                            debug(F110,"ttinl packet after ",inldest,0);
                        } else ttprty = 0;      /* restore if parchk error */
                    }
                    sopmask = ttprty;
                    needpchk = 0;
                }
                ckhexdump("ttinl got",inldest,i);
                inlret = i ;
                srvtimer = 0 ;

                /* if there is anything left in the buffer, we better save it */
                /* But first, throw out chars that are before the start char  */
                while (++j<haveread) {
                    if (inxbuf[j] == inlstart)
                        break;
                    debug(F101,"ttinl discarding","",inxbuf[j]);
                }
                if ( j < haveread ) {
                    debug(F111,"ttinl putting back chars","haveread - j",
                           haveread-j);
                    le_puts( &inxbuf[j], haveread-j );
                }
#ifdef STREAMING
                if (streaming && sndtyp == 'D') {
                    debug(F110,"ttinl quitting","streaming && sndtyp == 'D'",0);
                    return(-1);
                }
#endif /* STREAMING */

                return inlret;
            }
        }   /* end of while() */
    } while ((i < inlmax-1)  &&  (!timo || (timespent < timo)));
    /* end of while() */

    debug(F111,"ttinl timout","timespent",timespent); /* Get here on timout. */
    /* debug(F110," with",(char *) dest,0); */
    inlret = -1 ;
    debug(F101,"ttinl return inlret","",inlret);
    return inlret ;
}
#endif /* NOXFER */

/*  T T I N C --  Read a character from the communication line  */

/* The time should be in secs for consistency with the other modules in    */
/* kermit.  To retain the option of using times of less than 1s a negative */
/* parameter is interpreted as meaning multiples of 0.01s                  */
/* On failure, returns -1 or other negative myread error code,             */
/* or -2 if connection is broken or ttyfd < 0.                             */
/* or -3 if session limit has expired,                                     */
/* or -4 if something or other...                                          */

int rdch(int timo);
int
ttinc(int timo) {
    int m, i=0, j=0;
    char ch = 0;
#ifdef NT
    int tt, tr, interval;
#endif /* NT */
    int t ;

    m = (ttprty) ? 0177 : 0377;         /* Parity stripping mask. */

    if ( ttpush >= 0 )
    {
        debug(F111,"ttinc","ttpush",ttpush);
        ch = ttpush;
        ttpush = -1;
        return(ch & m);
    }

    if ( LocalEchoData ) {
        if ( le_getchar(&ch) > 0 ) {
            debug(F111,"ttinc le_inbuf","ch",ch);
            return(ch & m);
        }
    }

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         ) return(-2);          /* Not open. */

    if ( tt_inpacing )
        msleep(tt_inpacing);

#ifdef NETCONN
    if (network) {
        extern int duplex;
        if ( !duplex
#ifdef IKSD
             || inserver
#endif /* IKSD */
             ) {
            i = os2_netinc(timo);
            return(i >= 0 ? i & m : i);
        }
        else {
            if ( timo < 0 ) {
                j = -timo / 100;
            } else if ( timo > 0 )
                j = timo * 10;

            do {
                if ( LocalEchoData ) {
                    if ( le_getchar(&ch) > 0 ) {
                        debug(F111,"ttinc le_inbuf","ch",ch);
                        return(ch & m);
                    }
                }
                i = os2_netinc(-100);
            } while (timo ? (i==-1 && --j>0) : (i == -1));
            return(i >= 0 ? i & m : i);
        }
    }
#endif /* NETCONN */

    if (timo == 0) {                    /* Untimed read. */
        do {
            i = rdch(ck_sleepint);
            if ( i < 0 &&
#ifdef COMMENT
                 (ttcarr != CAR_OFF) && /* Carrier dropped */
                 ((ttgmdm() & BM_DCD) == 0)
#else /* COMMENT */
                 (ttchk() < 0)
#endif /* COMMENT */
                 )
                DialerSend( OPT_KERMIT_HANGUP, 0 ) ;
#ifdef NTSIG
            ck_ih();
#endif /* NTSIG */
            if ( LocalEchoData ) {
                if ( le_getchar(&ch) > 0 ) {
                    debug(F111,"ttinc le_inbuf","ch",ch);
                    return(ch & m);
                }
            }
        } while (i == -1);              /* Wait for a character. */
        return(i >= 0 ? i & m : i);
    }

/* Timed read */

    if (timo < 0)                       /* Convert to milliseconds */
    {
       timo= -timo;
       t = 0 ;
    }
    else
    {
       t = timo ;
       timo = timo * 1000;
    }

#ifdef NT
    tt = timo / ck_sleepint;            /* Break up into chunks */
    tr = timo % ck_sleepint;            /* Break up into chunks */
   if ( tt )
      interval = ck_sleepint + tr / tt ;
   else
   {
      tt = 1 ;
      interval = tr ;
   }
    debug(F111,"ttinc","rdch interval",interval);
    debug(F111,"ttinc","rdch tries",tt);

    for (j = 0; j < tt; j++) {          /* to avoid being stuck */
       i = rdch(interval);              /* in a system call...  */
#ifdef NTSIG
       ck_ih();
#endif /* NTSIG */
        if (i >= 0)
            break;
        else if (i < -1)
           break;
        if ( LocalEchoData ) {
            if ( le_getchar(&ch) > 0 ) {
                debug(F111,"ttinc le_inbuf","ch",ch);
                return(ch & m);
            }
        }
    }
#else /* NT */
    i = rdch(timo);
#endif /* NT */
    return( (i < 0) ? i : (i & m) );
}

/*  RDCH -- Read characters from the serial port, maintaining an internal
            buffer of characters for the sake of efficiency.
            Timeout is specified in milliseconds.
*/

/*
   The MustComplete section in this code is required because SIO.SYS at
   least versions up to 1.45 set the length to buffersize when a SIGINT
   occurs during the processing of this call.
*/
#ifdef NEWRDCH
static int
rdch(int timo /* ms */) {
    ULONG Nesting;
#ifdef NT
    COMMTIMEOUTS timeouts ;
    static int OldReadPending = FALSE ;
    int rc=0;
#endif /* NT */

    if ( ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return -2;                      /* No connection */

    if (rdchbuf.index == rdchbuf.length) {
        rdchbuf.index = rdchbuf.length = 0;

        rmstimer();

        debug(F111,"NEWRDCH rdch","timo",timo);
        if ( timo == 0 ) {
#ifdef NT
            memset(&timeouts, 0, sizeof(COMMTIMEOUTS)) ;
            GetCommTimeouts( (HANDLE) ttyfd, &timeouts ) ;
/*
  These settings will wait until at least one character
  is available to be read.  at least in Win95
*/
            timeouts.ReadIntervalTimeout = MAXDWORD ;
            timeouts.ReadTotalTimeoutMultiplier = MAXDWORD ;
            timeouts.ReadTotalTimeoutConstant = 10 ;
            debug(F111,"NEWRDCH rdch SetCommTimeouts","WriteTotalTimeoutMultiplier",
                   timeouts.WriteTotalTimeoutMultiplier);
            debug(F111,"NEWRDCH rdch SetCommTimeouts","WriteTotalTimeoutConstant",
                   timeouts.WriteTotalTimeoutConstant);
            SetCommTimeouts( (HANDLE) ttyfd, &timeouts ) ;
#else /* NT */
            if (ttydcb.usReadTimeout != 9) {
                ttydcb.usReadTimeout = 9;       /* Test every  0.1s per call */
                if (DosDevIOCtl(NULL,0,&ttydcb,sizeof(ttydcb),
                                ASYNC_SETDCBINFO,1,ttyfd))
                  return(-1);
            }
#endif /* NT */
        } else {
#ifdef NT
            memset(&timeouts, 0, sizeof(COMMTIMEOUTS) ) ;
            GetCommTimeouts( (HANDLE) ttyfd, &timeouts ) ;
            timeouts.ReadIntervalTimeout = MAXDWORD ;
            timeouts.ReadTotalTimeoutMultiplier = MAXDWORD ;
            timeouts.ReadTotalTimeoutConstant = timo;
            debug(F111,"NEWRDCH rdch SetCommTimeouts","WriteTotalTimeoutMultiplier",
                   timeouts.WriteTotalTimeoutMultiplier);
            debug(F111,"NEWRDCH rdch SetCommTimeouts","WriteTotalTimeoutConstant",
                   timeouts.WriteTotalTimeoutConstant);
            SetCommTimeouts( (HANDLE) ttyfd, &timeouts ) ;
#else /* NT */
            if (ttydcb.usReadTimeout != timo) { /* Set timeout value */
                ttydcb.usReadTimeout = (timo/10)-1; /* 0.01 sec units.  0 = 0.01 sec */
                if (DosDevIOCtl(NULL,0,&ttydcb,sizeof(ttydcb),
                                ASYNC_SETDCBINFO,IOCTL_ASYNC,ttyfd))
                  return(-1);
            }
#endif /* NT */
        }

#ifdef NT
#ifndef COMMENT
        rc = OverlappedRead( TRUE, rdchbuf.buffer, sizeof(rdchbuf.buffer), timo );
        if ( rc >= 0 )
            rdchbuf.length = rc;
        else
            return(rc);
#else /* COMMENT */
        if ( overlapped_read.hEvent == (HANDLE) -1 ) {
            overlapped_read.hEvent = CreateEvent(
                                            NULL,   /* no security */
                                            TRUE,   /* explicit reset req */
                                            FALSE,  /* initial event set */
                                            NULL    /* no name */
                                                );

            if (overlapped_read.hEvent == (HANDLE) -1) {
                debug(F101,"rdch CreateEvent error","",GetLastError() ) ;
            }
        }

        if ( !OldReadPending ) {
            overlapped_read.Offset = overlapped_read.OffsetHigh = 0 ;
            ResetEvent( overlapped_read.hEvent ) ;
        }

        if (OldReadPending ||
             !ReadFile((HANDLE) ttyfd,
                       rdchbuf.buffer,
                       sizeof(rdchbuf.buffer),
                       &nActuallyRead,
                         &overlapped_read)
            ) {
            DWORD error = GetLastError() ;
            if ( OldReadPending || error == ERROR_IO_PENDING ) {
                int timedout=FALSE;
#ifndef COMMENT
                debug(F100,"rdch ReadFile ERROR_IO_PENDING","",0);
#endif /* COMMENT */
                while(!GetOverlappedResult( (HANDLE) ttyfd,
                                            &overlapped_read,
                                            &nActuallyRead,
                                            FALSE )
                       ) {
                    DWORD error = GetLastError() ;
                    if ( error == ERROR_IO_INCOMPLETE ) {
                        int rc=0;
                        debug(F100,"rdch ReadFile ERROR_IO_INCOMPLETE","",0);
                        if ( timedout ) {
                            debug(F110,"rdch ReadFile","timedout",0);
                            OldReadPending = TRUE;
                            return(-1);
                        }
                        rc = WaitForSingleObjectEx( overlapped_read.hEvent, timo, TRUE ) ;
                        debug(F111,"rdch WaitForSingleObjectEx","rc",rc);
                        if ( rc != WAIT_OBJECT_0 )
                            timedout = TRUE;
                        continue;
                    } else if ( error == ERROR_OPERATION_ABORTED ) {
                        debug(F100,"rdch ReadFile OPERATION ABORTED","",0);
                        ResetEvent( overlapped_read.hEvent ) ;
                        if (nActuallyRead > 0 )
                            rdchbuf.length = nActuallyRead ;
                        OldReadPending = FALSE;
                        return (-1); /* fdc */
                    } else {
                        DWORD errorflags ;
                        COMSTAT comstat ;
                        debug(F101,"rdch ReadFile Overlapped I/O Error",
                               "",error);
                        ClearCommError( (HANDLE) ttyfd,
                                        &errorflags,
                                        &comstat );
                        debug(F101,"rdch ClearCommError","",errorflags);
                        if ( errorflags && deblog )
                        {
                            if ( errorflags & CE_RXOVER )
                                debug(F110,"ClearCommError","Receive Queue overflow",0);
                            if ( errorflags & CE_OVERRUN )
                                debug(F110,"ClearCommError","Receive Overrun error",0);
                            if ( errorflags & CE_RXPARITY )
                                debug(F110,"ClearCommError","Receive Parity error",0);
                            if ( errorflags & CE_FRAME )
                                debug(F110,"ClearCommError","Receive Framing error",0);
                            if ( errorflags & CE_BREAK )
                                debug(F110,"ClearCommError","Break detected",0);
                            if ( errorflags & CE_TXFULL )
                                debug(F110,"ClearCommError","TX Queue is full",0);
                            if ( errorflags & CE_PTO )
                                debug(F110,"ClearCommError","LPTx Timeout",0);
                            if ( errorflags & CE_IOE )
                                debug(F110,"ClearCommError","LPTx I/O Error",0);
                            if ( errorflags & CE_DNS )
                                debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                            if ( errorflags & CE_OOP )
                                debug(F110,"ClearCommError","LPTx Out Of Paper",0);
                        }
                        if ( deblog ) {
                            debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                            debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                            debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                            debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                            debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                            debug(F111,"ClearCommError","Eof",comstat.fEof);
                            debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                            debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                            debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
                        }
                        ResetEvent( overlapped_read.hEvent ) ;
                        if (nActuallyRead > 0 )
                            rdchbuf.length = nActuallyRead ;
                        OldReadPending = FALSE;
                        return -1;
                    }
                }
            } else {
                DWORD errorflags ;
                COMSTAT comstat ;
                debug(F101,"rdch ReadFile - real error occurred","",error) ;
                ClearCommError( (HANDLE) ttyfd,
                                &errorflags,
                                &comstat );
                debug(F101,"rdch ClearCommError","",errorflags);
                if ( errorflags && deblog )
                {
                    if ( errorflags & CE_RXOVER )
                        debug(F110,"ClearCommError","Receive Queue overflow",0);
                    if ( errorflags & CE_OVERRUN )
                        debug(F110,"ClearCommError","Receive Overrun error",0);
                    if ( errorflags & CE_RXPARITY )
                        debug(F110,"ClearCommError","Receive Parity error",0);
                    if ( errorflags & CE_FRAME )
                        debug(F110,"ClearCommError","Receive Framing error",0);
                    if ( errorflags & CE_BREAK )
                        debug(F110,"ClearCommError","Break detected",0);
                    if ( errorflags & CE_TXFULL )
                        debug(F110,"ClearCommError","TX Queue is full",0);
                    if ( errorflags & CE_PTO )
                        debug(F110,"ClearCommError","LPTx Timeout",0);
                    if ( errorflags & CE_IOE )
                        debug(F110,"ClearCommError","LPTx I/O Error",0);
                    if ( errorflags & CE_DNS )
                        debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                    if ( errorflags & CE_OOP )
                        debug(F110,"ClearCommError","LPTx Out Of Paper",0);
                }
                if ( deblog ) {
                    debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                    debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                    debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                    debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                    debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                    debug(F111,"ClearCommError","Eof",comstat.fEof);
                    debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                    debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                    debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
                }
                ResetEvent( overlapped_read.hEvent ) ;
                if (nActuallyRead > 0 )
                    rdchbuf.length = nActuallyRead ;
                OldReadPending = FALSE;
                return -2 ;
            }
        }
        ResetEvent( overlapped_read.hEvent ) ;
#ifndef COMMENT
        debug(F101,"rdch nActuallyRead","",nActuallyRead ) ;
        debug(F101,"rdch overlapped_read.Offset","",overlapped_read.Offset) ;
#endif /* COMMENT */
        rdchbuf.length = nActuallyRead ;
#endif /* COMMENT */
#else /* NT */
        DosEnterMustComplete( &Nesting ) ;
        if (DosRead(ttyfd,rdchbuf.buffer,sizeof(rdchbuf.buffer),
                    &rdchbuf.length)) {
            rdchbuf.length = 0;
            DosExitMustComplete( &Nesting );
            return(-1);
        }
        DosExitMustComplete( &Nesting );
#endif /* NT */
    }

#ifndef COMMENT
    debug(F101,"rdch() index","",rdchbuf.index);
    debug(F101,"rdch() length","",rdchbuf.length);
#endif /* COMMENT */

#ifdef NT
    OldReadPending = FALSE;
#endif /* NT */
    return((rdchbuf.index < rdchbuf.length) ?
            rdchbuf.buffer[rdchbuf.index++] :
            -1 );
}
#else /* NEWRDCH */
static int
rdch(int timo /* ms */) {
    ULONG Nesting;
#ifdef NT
    COMMTIMEOUTS timeouts ;
    static int OldReadPending = FALSE ;
#endif /* NT */

    if ( ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return -2;                      /* No connection */

    if (rdchbuf.index == rdchbuf.length) {
        rdchbuf.index = rdchbuf.length = 0;

        debug(F111,"OLD rdch","timo",timo);
        if ( timo == 0 ) {
#ifdef NT
            int change = 0;
            memset(&timeouts, 0, sizeof(COMMTIMEOUTS)) ;
            GetCommTimeouts( (HANDLE) ttyfd, &timeouts ) ;
/*
  These settings will wait until at least one character
  is available to be read.  at least in Win95
*/              
            if ( timeouts.ReadIntervalTimeout != MAXDWORD ) {
                timeouts.ReadIntervalTimeout = MAXDWORD ;
                change++;
            }
            if ( timeouts.ReadTotalTimeoutMultiplier != MAXDWORD ) {
                timeouts.ReadTotalTimeoutMultiplier = MAXDWORD ;
                change++;
            }
            if ( timeouts.ReadTotalTimeoutConstant != 10 ) {
                timeouts.ReadTotalTimeoutConstant = 10 ;
                change++;
            }
            if ( change ) {
                debug(F111,"OLD rdch SetCommTimeouts","WriteTotalTimeoutMultiplier",
                       timeouts.WriteTotalTimeoutMultiplier);
                debug(F111,"OLD rdch SetCommTimeouts","WriteTotalTimeoutConstant",
                       timeouts.WriteTotalTimeoutConstant);
                SetCommTimeouts( (HANDLE) ttyfd, &timeouts ) ;
            }
#else /* NT */
            if (ttydcb.usReadTimeout != 9) {
                ttydcb.usReadTimeout = 9;       /* Test every  0.1s per call */
                if (DosDevIOCtl(NULL,0,&ttydcb,sizeof(ttydcb),
                                ASYNC_SETDCBINFO,1,ttyfd))
                  return(-1);
            }
#endif /* NT */
        } else {
#ifdef NT
            int change = 0;
            memset(&timeouts, 0, sizeof(COMMTIMEOUTS) ) ;
            GetCommTimeouts( (HANDLE) ttyfd, &timeouts ) ;
            if ( timeouts.ReadIntervalTimeout != MAXDWORD ) {
                timeouts.ReadIntervalTimeout = MAXDWORD ;
                change++;
            }
            if ( timeouts.ReadTotalTimeoutMultiplier != MAXDWORD ) {
                timeouts.ReadTotalTimeoutMultiplier = MAXDWORD ;
                change++;
            }
            if ( timeouts.ReadTotalTimeoutConstant != timo  ) {
                timeouts.ReadTotalTimeoutConstant = timo ;
                change++;
            }
            if ( change ) {
                debug(F111,"OLD rdch SetCommTimeouts","WriteTotalTimeoutMultiplier",
                       timeouts.WriteTotalTimeoutMultiplier);
                debug(F111,"OLD rdch SetCommTimeouts","WriteTotalTimeoutConstant",
                       timeouts.WriteTotalTimeoutConstant);
                SetCommTimeouts( (HANDLE) ttyfd, &timeouts ) ;
            }
#else /* NT */
            if (ttydcb.usReadTimeout != timo) { /* Set timeout value */
                ttydcb.usReadTimeout = (timo/10)-1; /* 0.01 sec units.  0 = 0.01 sec */
                if (DosDevIOCtl(NULL,0,&ttydcb,sizeof(ttydcb),
                                ASYNC_SETDCBINFO,IOCTL_ASYNC,ttyfd))
                  return(-1);
            }
#endif /* NT */
        }

#ifdef NT
        if ( overlapped_read[0].hEvent == (HANDLE) -1 ) {
            overlapped_read[0].hEvent = CreateEvent(
                                            NULL,   /* no security */
                                            TRUE,   /* explicit reset req */
                                            FALSE,  /* initial event set */
                                            NULL    /* no name */
                                                );

            if (overlapped_read[0].hEvent == (HANDLE) -1) {
                debug(F101,"rdch CreateEvent error","",GetLastError() ) ;
            }
        }

        if ( !OldReadPending ) {
            overlapped_read[0].Offset = overlapped_read[0].OffsetHigh = 0 ;
            ResetEvent( overlapped_read[0].hEvent ) ;
        }

        if (OldReadPending ||
             !ReadFile((HANDLE) ttyfd,
                       rdchbuf.buffer,
                       sizeof(rdchbuf.buffer),
                       &nActuallyRead,
                         &overlapped_read[0])
            ) {
            DWORD error = GetLastError() ;
            if ( OldReadPending || error == ERROR_IO_PENDING ) {
                int timedout=FALSE;
#ifndef COMMENT
                debug(F100,"rdch ReadFile ERROR_IO_PENDING","",0);
#endif /* COMMENT */
                while(!GetOverlappedResult( (HANDLE) ttyfd,
                                            &overlapped_read[0],
                                            &nActuallyRead,
                                            FALSE )
                       ) {
                    DWORD error = GetLastError() ;
                    if ( error == ERROR_IO_INCOMPLETE ) {
                        int rc=0;
                        debug(F100,"rdch ReadFile ERROR_IO_INCOMPLETE","",0);
                        if ( timedout ) {
                            debug(F110,"rdch ReadFile","timedout",0);
                            OldReadPending = TRUE;
                            return(-1);
                        }
                        rc = WaitForSingleObjectEx( overlapped_read[0].hEvent, timo, TRUE ) ;
                        debug(F111,"rdch WaitForSingleObjectEx","rc",rc);
                        if ( rc != WAIT_OBJECT_0 )
                            timedout = TRUE;
                        continue;
                    } else if ( error == ERROR_OPERATION_ABORTED ) {
                        debug(F100,"rdch ReadFile OPERATION ABORTED","",0);
                        ResetEvent( overlapped_read[0].hEvent ) ;
                        if (nActuallyRead > 0 )
                            rdchbuf.length = nActuallyRead ;
                        OldReadPending = FALSE;
                        return (-1); /* fdc */
                    } else {
                        DWORD errorflags ;
                        COMSTAT comstat ;
                        debug(F101,"rdch ReadFile Overlapped I/O Error",
                               "",error);
                        ClearCommError( (HANDLE) ttyfd,
                                        &errorflags,
                                        &comstat );
                        debug(F101,"rdch ClearCommError","",errorflags);
                        if ( errorflags && deblog )
                        {
                            if ( errorflags & CE_RXOVER )
                                debug(F110,"ClearCommError","Receive Queue overflow",0);
                            if ( errorflags & CE_OVERRUN )
                                debug(F110,"ClearCommError","Receive Overrun error",0);
                            if ( errorflags & CE_RXPARITY )
                                debug(F110,"ClearCommError","Receive Parity error",0);
                            if ( errorflags & CE_FRAME )
                                debug(F110,"ClearCommError","Receive Framing error",0);
                            if ( errorflags & CE_BREAK )
                                debug(F110,"ClearCommError","Break detected",0);
                            if ( errorflags & CE_TXFULL )
                                debug(F110,"ClearCommError","TX Queue is full",0);
                            if ( errorflags & CE_PTO )
                                debug(F110,"ClearCommError","LPTx Timeout",0);
                            if ( errorflags & CE_IOE )
                                debug(F110,"ClearCommError","LPTx I/O Error",0);
                            if ( errorflags & CE_DNS )
                                debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                            if ( errorflags & CE_OOP )
                                debug(F110,"ClearCommError","LPTx Out Of Paper",0);
                        }
                        if ( deblog ) {
                            debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                            debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                            debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                            debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                            debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                            debug(F111,"ClearCommError","Eof",comstat.fEof);
                            debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                            debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                            debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
                        }
                        if ( deblog ) {
                            debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                            debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                            debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                            debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                            debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                            debug(F111,"ClearCommError","Eof",comstat.fEof);
                            debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                            debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                            debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
                        }
                        ResetEvent( overlapped_read[0].hEvent ) ;
                        if (nActuallyRead > 0 )
                            rdchbuf.length = nActuallyRead ;
                        OldReadPending = FALSE;
                        return -1;
                    }
                }
            } else {
                DWORD errorflags ;
                COMSTAT comstat ;
                debug(F101,"rdch ReadFile - real error occurred","",error) ;
                ClearCommError( (HANDLE) ttyfd,
                                &errorflags,
                                &comstat );
                debug(F101,"rdch ClearCommError","",errorflags);
                if ( errorflags && deblog )
                {
                    if ( errorflags & CE_RXOVER )
                        debug(F110,"ClearCommError","Receive Queue overflow",0);
                    if ( errorflags & CE_OVERRUN )
                        debug(F110,"ClearCommError","Receive Overrun error",0);
                    if ( errorflags & CE_RXPARITY )
                        debug(F110,"ClearCommError","Receive Parity error",0);
                    if ( errorflags & CE_FRAME )
                        debug(F110,"ClearCommError","Receive Framing error",0);
                    if ( errorflags & CE_BREAK )
                        debug(F110,"ClearCommError","Break detected",0);
                    if ( errorflags & CE_TXFULL )
                        debug(F110,"ClearCommError","TX Queue is full",0);
                    if ( errorflags & CE_PTO )
                        debug(F110,"ClearCommError","LPTx Timeout",0);
                    if ( errorflags & CE_IOE )
                        debug(F110,"ClearCommError","LPTx I/O Error",0);
                    if ( errorflags & CE_DNS )
                        debug(F110,"ClearCommError","LPTx Device Not Selected",0);
                    if ( errorflags & CE_OOP )
                        debug(F110,"ClearCommError","LPTx Out Of Paper",0);
                }
                if ( deblog ) {
                    debug(F111,"ClearCommError","Cts Hold",comstat.fCtsHold);
                    debug(F111,"ClearCommError","Dsr Hold",comstat.fDsrHold);
                    debug(F111,"ClearCommError","Rlsd Hold",comstat.fRlsdHold);
                    debug(F111,"ClearCommError","Xoff Hold",comstat.fXoffHold);
                    debug(F111,"ClearCommError","Xoff Sent",comstat.fXoffSent);
                    debug(F111,"ClearCommError","Eof",comstat.fEof);
                    debug(F111,"ClearCommError","Tx Immed",comstat.fTxim);
                    debug(F111,"ClearCommError","In Que",comstat.cbInQue);
                    debug(F111,"ClearCommError","Out Que",comstat.cbOutQue);
                }
                ResetEvent( overlapped_read[0].hEvent ) ;
                if (nActuallyRead > 0 )
                    rdchbuf.length = nActuallyRead ;
                OldReadPending = FALSE;
                return -2 ;
            }
        }
        ResetEvent( overlapped_read[0].hEvent ) ;
#ifndef COMMENT
        debug(F101,"rdch nActuallyRead","",nActuallyRead ) ;
        debug(F101,"rdch overlapped_read.Offset","",overlapped_read[0].Offset) ;
#endif /* COMMENT */
        rdchbuf.length = nActuallyRead ;
#else /* NT */
        DosEnterMustComplete( &Nesting ) ;
        if (DosRead(ttyfd,rdchbuf.buffer,sizeof(rdchbuf.buffer),
                    &rdchbuf.length)) {
            rdchbuf.length = 0;
            DosExitMustComplete( &Nesting );
            return(-1);
        }
        DosExitMustComplete( &Nesting );
#endif /* NT */
    }

#ifdef COMMENT
    debug(F101,"rdch() index","",rdchbuf.index);
    debug(F101,"rdch() length","",rdchbuf.length);
#endif /* COMMENT */

#ifdef NT
    OldReadPending = FALSE;
#endif /* NT */
    return((rdchbuf.index < rdchbuf.length) ?
            rdchbuf.buffer[rdchbuf.index++] :
            -1 );
}
#endif /* NEWRDCH */


/*  T T S C A R R  --  Set ttcarr variable, controlling carrier handling.
 *
 *  0 = Off: Always ignore carrier. E.g. you can connect without carrier.
 *  1 = On: Heed carrier, except during dialing. Carrier loss gives disconnect.
 *  2 = Auto: For "modem direct": The same as "Off".
 *            For real modem types: Heed carrier during connect, but ignore
 *                it anytime else.  Compatible with pre-5A Kermit versions.
 */

int
ttscarr(int carrier)
{
    ttcarr = carrier;
    debug(F101, "ttscarr","",ttcarr);
    return(ttcarr);
}

/*  T T G M D M  --  Get modem signals  */
/*
 Looks for the modem signals CTS, DSR, and CTS, and returns those that are
 on in as its return value, in a bit mask as described for ttwmdm.  Returns:
 -3 Not implemented
 -2 if the line does not have modem control
 -1 on error.
 >= 0 on success, with a bit mask containing the modem signals that are on.
*/

int
ttgmdm() {
#ifdef NT
    DWORD ModemStat ;
#else /* NT */
    BYTE instat, outstat;
#endif /* NT */
    int modem = 0;

    if (ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
         )
        return(-1);

#ifdef TN_COMPORT
    if (network)
        return(tngmdm());
#endif /* TN_COMPORT */

#ifdef NT
#ifdef CK_TAPI
    if ( tttapi && !tapipass )
    {
        HANDLE hModem = NULL;

        if (ttyfd == -2)
            return(-1);

        hModem = GetModemHandleFromLine( (HLINE) 0 );
        if ( hModem == NULL )
            return(-1);

        if (!GetCommModemStatus( hModem, &ModemStat )) {
            CloseHandle( hModem );
            return -1 ;
        }
        CloseHandle( hModem );
    }
    else
#endif /* CK_TAPI */
    if (!GetCommModemStatus( (HANDLE) ttyfd, &ModemStat ))
        return -1 ;

    if (ModemStat & MS_CTS_ON) modem |= BM_CTS ;
    if (ModemStat & MS_DSR_ON) modem |= BM_DSR ;
    if (ModemStat & MS_RING_ON) modem |= BM_RNG ;
    if (ModemStat & MS_RLSD_ON) modem |= BM_DCD ;

#else /* NT */
    if(DosDevIOCtl(&instat,sizeof(instat),NULL,0,
                   ASYNC_GETMODEMINPUT,IOCTL_ASYNC,ttyfd))
       return(-1);
    if(DosDevIOCtl(&outstat,sizeof(outstat),NULL,0,
                   ASYNC_GETMODEMOUTPUT,IOCTL_ASYNC,ttyfd))
       return(-1);

    /* Clear To Send */
    if (instat & CTS_ON) modem |= BM_CTS;
    /* Data Set Ready */
    if (instat & DSR_ON) modem |= BM_DSR;
    /* Carrier */
    if (instat & DCD_ON) modem |= BM_DCD;
    /* Ring Indicate */
    if (instat & RI_ON)  modem |= BM_RNG;

    /* Data Terminal Ready */
    if (outstat & DTR_ON) modem |= BM_DTR;
    /* Request To Send */
    if (outstat & RTS_ON) modem |= BM_RTS;
#endif /* NT */
    return(modem);
}

/*  T T S N D B  --  Send a BREAK signal  */
int
ttsndb() {
#ifdef OS2ONLY
    MODEMSTATUS ms;
    UINT data, i;
#endif /* OS2ONLY */
#ifdef NETCONN
    if (network) {
#ifdef TN_COMPORT
        if (istncomport()) {
            if (tnsndb(275L) >= 0)
                return(0);
        }
#endif /* TN_COMPORT */
        return(os2_netbreak());
    }
#endif /* NETCONN */
#ifdef NT
    SetCommBreak( (HANDLE) ttyfd ) ;
    msleep(275L);
    ClearCommBreak( (HANDLE) ttyfd ) ;
#else /* NT */
    ms.fbModemOn = RTS_ON;
    ms.fbModemOff = 255;
    DosDevIOCtl(&data,sizeof(data),&ms,sizeof(ms),
                ASYNC_SETMODEMCTRL,IOCTL_ASYNC,ttyfd);

    DosDevIOCtl(&i,sizeof(i),NULL,0,
                ASYNC_SETBREAKON,IOCTL_ASYNC,ttyfd);    /* Break on */
    msleep(275L);                                       /* ZZZzzz */
    DosDevIOCtl(&i,sizeof(i),NULL,0,
                ASYNC_SETBREAKOFF,IOCTL_ASYNC,ttyfd);   /* Break off */
#endif /* NT */
    return 0;
}

/*  T T S N D L B  --  Send a LONG BREAK signal  */
int
ttsndlb() {
#ifndef NT
    MODEMSTATUS ms;
    UINT data, i;
#endif /* NT */
#ifdef NETCONN
    if (network) {
#ifdef TN_COMPORT
        if (istncomport()) {
            if (tnsndb(1800L) >= 0)
                return(0);
        }
#endif /* TN_COMPORT */
        return(os2_netbreak());
    }
#endif /* NETCONN */
#ifdef NT
    SetCommBreak( (HANDLE) ttyfd ) ;
    msleep(1800L);
    ClearCommBreak( (HANDLE) ttyfd ) ;
        #else /* NT */
    ms.fbModemOn = RTS_ON;
    ms.fbModemOff = 255;
    DosDevIOCtl(&data,sizeof(data),&ms,sizeof(ms),
                ASYNC_SETMODEMCTRL,IOCTL_ASYNC,ttyfd);

    DosDevIOCtl(&i,sizeof(i),NULL,0,
                ASYNC_SETBREAKON,IOCTL_ASYNC,ttyfd);    /* Break on */
    msleep(1800L);                                      /* ZZZzzz */
    DosDevIOCtl(&i,sizeof(i),NULL,0,
                ASYNC_SETBREAKOFF,IOCTL_ASYNC,ttyfd);   /* Break off */
#endif /* NT*/
    return 0;
}

#ifndef __EMX__
/*  S L E E P  --  Emulate the Unix sleep function  */

/* MustComplete section required because of a bug in OS/2 that fails to
    allow us to reset the SIGINT signal after it has occurred during this
    call. */

unsigned
sleep(t) unsigned int t; {
    ULONG start_t, now_t, ms;
    int tt, tr, i;
#ifndef NT
    unsigned long p ;
#endif /* NT */
#ifdef NT
    start_t = GetTickCount();        /* msecs since Windows was started */
#else /* NT */
    DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&start_t,4);
#endif /* NT */

    tt = (t * 1000L) / ck_sleepint;
    tr = (t * 1000L) % ck_sleepint;

    for (i = 0; i < tt; i++) {
#ifdef NT
        Sleep((long) ck_sleepint);
#else /* NT */
        DosEnterMustComplete(&p);
        DosSleep((long)ck_sleepint);
        DosExitMustComplete(&p);
#endif /* NT */
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */
#ifdef NT
        now_t = GetTickCount();
#else /* NT */
        DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&now_t,4);
#endif /* NT */

        if ( now_t < start_t ) {
        /* we wrapped */
#ifdef NT
            ms = (MAXDWORD - start_t + now_t);
#else /* NT */
            ms = (ULONG_MAX - start_t + now_t);
#endif /* NT */
        }
        else {
            ms = (now_t - start_t);
        }
        if ( ms >= (t * 1000L) )
            return(0);
    }
    if ( tr )
    {
#ifdef NT
        Sleep((long) tr);
#else /* NT */
        DosEnterMustComplete(&p);
        DosSleep((long)tr);
        DosExitMustComplete(&p);
#endif /* NT */
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */
    }
    return(0);
}
#endif /* __EMX__ */

/*  M S L E E P  --  Millisecond version of sleep().  */

/* Intended only for small intervals.  For big ones, just use sleep(). */

/*
  MustComplete section required because of a bug in OS/2 that fails to allow
  us to reset the SIGINT signal after it has occurred during this call.
*/
int
msleep(int m) {
    ULONG start_t, now_t, ms;
    int tt, tr, ti, i;
#ifndef NT
   unsigned long p ;
#endif
#ifdef NT
    start_t = GetTickCount();        /* msecs since Windows was started */
#else /* NT */
    DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&start_t,4);
#endif /* NT */

    if ( m <= 500 ) {
#ifdef NT
        Sleep((long) m);
#else /* NT */
        DosEnterMustComplete(&p);
        DosSleep((long)m);
        DosExitMustComplete(&p);
#endif /* NT */
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */
        return(0);
    }

    tt = m / ck_sleepint;
    tr = m % ck_sleepint;
    ti = ck_sleepint;

    for (i = 0; i < tt; i++) {
#ifdef NT
        Sleep((long) ti);
#else /* NT */
        DosEnterMustComplete(&p);
        DosSleep((long)ti);
        DosExitMustComplete(&p);
#endif /* NT */
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */

#ifdef NT
        now_t = GetTickCount();
#else /* NT */
        DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&now_t,4);
#endif /* NT */

        if ( now_t < start_t ) {
        /* we wrapped */
#ifdef NT
            ms = (MAXDWORD - start_t + now_t);
#else /* NT */
            ms = (ULONG_MAX - start_t + now_t);
#endif /* NT */
        }
        else {
            ms = (now_t - start_t);
        }
        if ( ms >= m )
            return(0);
    }
    if ( tr ) {
#ifdef NT
        Sleep((long) tr);
#else /* NT */
        DosEnterMustComplete(&p);
        DosSleep((long)tr);
        DosExitMustComplete(&p);
#endif /* NT */
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */
    }
    return (0);
}

/*  R T I M E R --  Reset elapsed time counter  */

void
rtimer() {
#ifdef NT
    tcount = GetTickCount();        /* msecs since Windows was started */
#else /* NT */
    DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&tcount,4);
#endif /* NT */
    debug(F101,"rtimer tick_count","",tcount);
}

/*  G T I M E R --  Get current value of elapsed time counter in seconds  */

int
gtimer(void) {
#ifdef NT
    DWORD tick_now=0L;
#else /* NT */
    ULONG tick_now=0L;
#endif /* NT */
    int s=0;

#ifdef NT
    tick_now = GetTickCount();
#else /* NT */
    DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&tick_now,4);
#endif /* NT */

    if ( tick_now < tcount ) {
        /* we wrapped */
#ifdef NT
        s = (MAXDWORD - tcount + tick_now) / 1000;
#else /* NT */
        s = (ULONG_MAX - tcount + tick_now) / 1000;
#endif /* NT */
    }
    else {
        s = (tick_now - tcount) / 1000;
    }

    if (s < 0) s=0;
    return(s);
}

#ifdef GFTIMER
#ifdef NT
DWORD tick_count=0L;
DWORD tick_count2=0L;
#else /* NT */
ULONG tick_count=0L;
ULONG tick_count2=0L;
#endif /* NT */

VOID
rftimer() {
#ifdef NT
    tick_count = GetTickCount();        /* msecs since Windows was started */
                                        /* wraps at 49.7 days */
#else /* NT */
    DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&tick_count,4);
#endif /* NT */
    debug(F101,"rftimer tick_count","",tick_count);
}

CKFLOAT
gftimer() {
#ifdef NT
    DWORD tick_now=0L;
#else /* NT */
    ULONG tick_now=0L;
#endif /* NT */
    double s=0.0;
#ifdef DEBUG
    char fpbuf[64];
#endif /* DEBUG */

#ifdef NT
    tick_now = GetTickCount();
#else /* NT */
    DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&tick_now,4);
#endif /* NT */

    if ( tick_now < tick_count ) {
        /* we wrapped */
#ifdef NT
        s = (double) (MAXDWORD - tick_count + tick_now) / 1000;
#else /* NT */
        s = (double) (ULONG_MAX - tick_count + tick_now) / 1000;
#endif /* NT */
    }
    else {
        s = (double) (tick_now - tick_count) / 1000;
    }

    if (s == 0.0) s = 0.000001;
#ifdef DEBUG
    if (deblog) {
        sprintf(fpbuf,"%f",s);
        debug(F111,"gftimer",fpbuf,tick_now);
    }
#endif /* DEBUG */
    return((CKFLOAT)s);
}

VOID
rmstimer() {
#ifdef NT
    tick_count2 = GetTickCount();        /* msecs since Windows was started */
                                        /* wraps at 49.7 days */
#else /* NT */
    DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&tick_count2,4);
#endif /* NT */
    debug(F101,"rmstimer tick_count2","",tick_count2);
}

ULONG
gmstimer() {
#ifdef NT
    DWORD tick_now=0L;
#else /* NT */
    ULONG tick_now=0L;
#endif /* NT */
    ULONG s;
#ifdef DEBUG
    char fpbuf[64];
#endif /* DEBUG */

#ifdef NT
    tick_now = GetTickCount();
#else /* NT */
    DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&tick_now,4);
#endif /* NT */

    if ( tick_now < tick_count2 ) {
        /* we wrapped */
#ifdef NT
        s = (MAXDWORD - tick_count2 + tick_now);
#else /* NT */
        s = (ULONG_MAX - tick_count2 + tick_now);
#endif /* NT */
    }
    else {
        s = (tick_now - tick_count2);
    }

#ifdef DEBUG
    if (deblog) {
        sprintf(fpbuf,"%d",s);
        debug(F111,"gmstimer",fpbuf,tick_now);
    }
#endif /* DEBUG */
    return(s);
}
#endif /* GFTIMER */

/*  Z T I M E  --  Return date/time string  */

void
ztime(char **s) {
    time_t clock_storage;

    clock_storage = time( (long *) 0 );
    *s = ctime( &clock_storage );
}

void
loadtod( int hh, int mm )
{
#ifdef NT
    SYSTEMTIME systime ;
    GetLocalTime( &systime ) ;
    systime.wHour = hh ;
    systime.wMinute = mm ;
    systime.wSecond = 0 ;
    systime.wMilliseconds = 0 ;
    SetLocalTime( &systime ) ;
#else /* NT */
    DATETIME   DateTime = {0};       /* Structure to hold date/time info.   */
    APIRET     rc       = NO_ERROR;  /* Return code                         */

    /* Retrieve the current date and time  */
    if ( DosGetDateTime(&DateTime) == NO_ERROR ) {

        DateTime.hours = (UCHAR) hh;
        DateTime.minutes = (UCHAR) mm;
        DateTime.seconds = 0 ;
        DateTime.hundredths = 0 ;

        rc = DosSetDateTime(&DateTime);  /* Update the date and time            */
    }
#endif /* NT */
}

/*  C O N O C  --  Output a character to the console terminal  */

int
conoc(char c) {
    extern unsigned char colorcmd;
    extern int wherex[];    /* Screen column, 1-based */
    extern int wherey[];        /* Screen row, 1-based */

#ifdef IKSD
    if ( inserver ) {
        debug(F100,"conoc inserver","",0);
        ttoc(c);
    }

#endif /* IKSD */
#ifndef NOLOCAL
    if ( k95stdout )
        fwrite(&c,1,1,stdout);
    else {
        VscrnWrtCharStrAtt( VCMD, &c, 1, wherey[VCMD], wherex[VCMD],
                            &colorcmd ) ;
    }
#endif /* NOLOCAL */
    return 1;
}


/*  C O N X O  --  Write x characters to the console terminal  */

int
conxo(int x, char *s) {
    int i, rc;

    if ( s == NULL )
        return(-1);

#ifdef IKSD
    if ( inserver ) {
        debug(F100,"conxo inserver","",0);
        return(ttol(s,x));
    }
#endif /* IKSD */

    for ( i=0 ; i<x ; i++ )
      conoc(s[i]);
    return(x);
}


/*  C O N O L  --  Write a line to the console terminal  */

int
conol(char *s) {
    extern unsigned char colorcmd ;

    if ( s == NULL )
        return(-1);

#ifdef IKSD
    if ( inserver ) {
        debug(F100,"conol inserver","",0);
        ttol(s,strlen(s));
    }
#endif /* IKSD */
#ifndef NOLOCAL
    if ( k95stdout )
        fwrite(s,1,strlen(s),stdout);
    else
        VscrnWrtCharStrAtt( VCMD, s, strlen(s),
                            VscrnGetCurPos(VCMD)->y+1,
                            VscrnGetCurPos(VCMD)->x+1,
                            &colorcmd
                            ) ;
#endif /* NOLOCAL */
    return 0;
}


/*  C O N O L A  --  Write an array of lines to the console terminal */
int
conola(s) char *s[]; {
    int i;
    if ( s == NULL )
        return(-1);
    for (i=0 ; *s[i] ; i++)
        conol(s[i]);
    return 0 ;
}


/*  C O N O L L  --  Output a string followed by CRLF  */
int
conoll(s) char *s; {
    if ( s == NULL )
        return(-1);
    conol(s);
    conol("\r\n");
    return 0 ;
}


/*  C O N C H K  --  Return how many characters available at console  */

int
conchk() {
#ifdef IKSD
    if ( inserver ) {
        return(ttchk());
    } else
#endif /* IKSD */
    return charinbuf(VCMD) ;
}


/*  C O N I N C  --  Get a character from the console  */
/*
  Call with timo > 0 to do a timed read, timo == 0 to do an untimed blocking
  read.  Upon success, returns the character.  Upon failure, returns -1.
  A timed read that does not complete within the timeout period returns -2.
*/

int
coninc(timo) int timo; {
    int c, rc = -1, cm;
    extern int what;
#ifndef NOTERM
    extern enum markmodes markmodeflag[VNUM];
#endif /* NOTERM */
    time_t t;

    debug(F111,"coninc","timo",timo);
#ifdef IKSD
    if ( inserver ) {
        debug(F100,"coninc inserver","",0);
        c = ttinc(timo);
        if ( c == -1 )
            return(-2);
        else if ( c < 0 )
            return(-1);
        else
            return(c & 0xFF);
    }
#endif /* IKSD */

    t = time(NULL) ;
    do {
        c = congks(timo) ;
        switch ( c ) {
        case KEY_SCAN | BS:     /* The Backspace Key */
        case DEL:
        case XEM:
#ifndef NOKVERBS
        case F_KVERB | K_WYBS:
        case F_KVERB | K_WYSBS:
        case F_KVERB | K_TVIBS:
        case F_KVERB | K_TVISBS:
        case F_KVERB | K_DGBS:
#endif /* NOKVERBS */
            debug(F111,"coninc","BS conversion",c);
            c = BS ;
            break;
        case KEY_SCAN | HT:
#ifndef NOKVERBS
        case F_KVERB | K_WYTAB:
        case F_KVERB | K_TVITAB:
        case F_KVERB | K_SNI_TAB:
        case F_KVERB | K_SNI_S_TAB:
        case F_KVERB | K_SNI_C_TAB:
#endif /* NOKVERBS */
            debug(F111,"coninc","TAB conversion",c);
            c = HT;
            break;
        case KEY_SCAN | ESC:
#ifndef NOKVERBS
        case F_KVERB | K_WYESC:
        case F_KVERB | K_TVIESC:
#endif /* NOKVERBS */
            debug(F111,"coninc","ESC conversion",c);
            c = ESC ;
            break;
        case KEY_SCAN | CR:
#ifndef NOKVERBS
        case F_KVERB | K_KPENTER:
        case F_KVERB | K_WYENTER:
        case F_KVERB | K_WYRETURN:
        case F_KVERB | K_TVIENTER:
        case F_KVERB | K_TVIRETURN:
        case F_KVERB | K_HPENTER:
        case F_KVERB | K_HPRETURN:
#endif /* NOKVERBS */
            debug(F111,"coninc","ENTER conversion",c);
            c = CR ;
            break;
#ifndef NOKVERBS
        case F_KVERB | K_KPMINUS:
            debug(F111,"coninc","MINUS conversion",c);
            c = '-';
            break;
        case F_KVERB | K_KPCOMA:
            debug(F111,"coninc","COMA conversion",c);
            c = ',';
            break;
        case F_KVERB | K_KPDOT:
            debug(F111,"coninc","PERIOD conversion",c);
            c = '.';
            break;
        case F_KVERB | K_KP0:
            debug(F111,"coninc","0 conversion",c);
            c = '0' ;
            break;
        case F_KVERB | K_KP1:
            debug(F111,"coninc","1 conversion",c);
            c = '1' ;
            break;
        case F_KVERB | K_KP2:
            debug(F111,"coninc","2 conversion",c);
            c = '2' ;
            break;
        case F_KVERB | K_KP3:
            debug(F111,"coninc","3 conversion",c);
            c = '3' ;
            break;
        case F_KVERB | K_KP4:
            debug(F111,"coninc","4 conversion",c);
            c = '4' ;
            break;
        case F_KVERB | K_KP5:
            debug(F111,"coninc","5 conversion",c);
            c = '5' ;
            break;
        case F_KVERB | K_KP6:
            debug(F111,"coninc","6 conversion",c);
            c = '6' ;
            break;
        case F_KVERB | K_KP7:
            debug(F111,"coninc","7 conversion",c);
            c = '7' ;
            break;
        case F_KVERB | K_KP8:
            debug(F111,"coninc","8 conversion",c);
            c = '8' ;
            break;
        case F_KVERB | K_KP9:
            debug(F111,"coninc","9 conversion",c);
            c = '9' ;
            break;
#endif /* NOKVERBS */
        case KEY_SCAN | KEY_CTRL | 66: /* Ctrl-B */
        case KEY_SCAN | 38:
        case KEY_SCAN | KEY_ENHANCED | 38:
#ifndef NOKVERBS
        case F_KVERB | K_UPARR:
        case F_KVERB | K_ANSIF50:
#endif /* NOKVERBS */
#ifndef NOTERM
            if ( markmodeflag[VCMD] != notmarking )
                break;
            if (!IsConnectMode()) { /* Up arrow = Ctrl-B */
                debug(F111,"coninc","Ctrl-B conversion",c);
                c = 2;
            }
#endif /* NOTERM */
            break;
        case KEY_SCAN | KEY_CTRL | 78:
        case KEY_SCAN | 40:
        case KEY_SCAN | KEY_ENHANCED | 40:
#ifndef NOKVERBS
        case F_KVERB | K_DNARR:
        case F_KVERB | K_ANSIF58:
#endif /* NOKVERBS */
#ifndef NOTERM
            if ( markmodeflag[VCMD] != notmarking )
                break;
            if (!IsConnectMode()) { /* Down arrow = Ctrl-N */
                debug(F111,"coninc","Ctrl-N conversion",c);
                c = 14;
            }
#endif /* NOTERM */
            break;
        default: ;
        }
        if ( c == CR )
            c = NL;
#ifndef NOKVERBS
        if (c >= 0) {
            debug(F111,"coninc","c >= 0",c);
            if ( IS_KVERB(c) ) {
                dokverb(VCMD,c);
            }
            /* Here we could handle F_CSI, F_ESC, ... */
        }
#endif /* NOKVERBS */
        if ( timo && (time( NULL ) - t >= timo) )
        {
            c = -1 ; /* timeout */
            break;
        }
    } while ( c >= 0x100 ) ;

    if ( c == -1 )
        return(-2);
    else if ( c == -2 )
        return(-3);
    else if ( c < 0 )
        return(-1);
    else
        return(c & 0xFF);
}

con_event
congev( int vmode, int timo ) {
    APIRET rc ;
    ULONG timeout = 0;
    con_event evt ;
    int tt,tr,interval,i ;

#ifdef IKSD
    if ( inserver ) {
        int ch;
        debug(F100,"congev inserver","",0);
        ch = ttinc(timo);
        if (ch >= 0) {
            evt.type = key;
            evt.key.scancode = ch;
        } else {
            evt.type = error;
        }
        return evt;
    }
#endif /* IKSD */

#ifdef NT
    if ( timo == 0 )
    {
       interval = ck_sleepint ;
       tt = 1 ;
       timeout = 0;
    }
    else if ( timo == -1 )
    {
       interval = -1 ;
       tt = 1 ;
       timeout = 0;
    }
    else
    {
       if ( timo < -1 )
          timeout = -timo ;
       else /* ( timo > 0 ) */
          timeout = timo * 1000 ;

       tt = timeout / ck_sleepint ;
       tr = timeout % ck_sleepint ;
       if ( tt )
          interval = ck_sleepint + tr / tt ;
       else
       {
          tt = 1 ;
          interval = tr ;
       }
    }
    for ( i = 0 ; (timeout && (i < tt)) || !timeout ; i++ )
    {
        rc = WaitEventAvailSem( vmode, interval ) ;
#ifdef NTSIG
        ck_ih() ;
#endif /* NTSIG */
        switch ( rc ) {
        case NO_ERROR:
            if ( getevent( vmode, &evt ) )
                return(evt);
            break;
        default:
            if ( interval == -1 ) {
                evt.type = error;
                return(evt);
            }
        }
    }
#else /* NT */
    if ( timo < -1 )
      timeout = -timo ;
    if ( timo == -1 || timo == 0 )
      timeout = SEM_INDEFINITE_WAIT ;
    if ( timo > 0 )
      timeout = timo * 1000 ;

   rc = WaitEventAvailSem( vmode, timeout ) ;
   if ( rc == NO_ERROR )
       if ( getevent( vmode, &evt ) )
           return evt ;
#endif /* NT */
   evt.type=error;
   return evt ;
}


int
congks(int timo) {
    APIRET rc ;
    ULONG timeout = 0 ;
    con_event evt ;
    int tt, i, ch ;
    static char * macrostr = NULL, * s = NULL;

    debug(F111,"congks","timo",timo);

    if ( macrostr && s ) {
        debug(F100,"congks macrostr","",0);
        if ( *s == NUL ) {
            free(macrostr);
            macrostr = NULL ;
            s = NULL;
            ch = CR;
        }
        else {
            ch = *s++;
        }
        return(ch);
    }

#ifdef IKSD
    if ( inserver ) {
        debug(F100,"congks inserver","",0);
        return(ttinc(timo));
    }
#endif /* IKSD */

   if ( timo < -1 )
      timeout = -timo ;
   else if ( timo == -1 || timo == 0 )
      timeout = SEM_INDEFINITE_WAIT ;
   else /* ( timo > 0 ) */
      timeout = timo * 1000;

    debug(F111,"congks","timeout",timeout);

#ifdef NT
   tt = timeout / ck_sleepint ;
   for ( i = 0 ; i <= tt ; )
   {
      rc = WaitEventAvailSem( VCMD, ck_sleepint ) ;
       debug(F111,"congks","WaitEventAvailSem()",rc);
#ifdef NTSIG
      ck_ih() ;
#endif /* NTSIG */
      switch ( rc )
      {
      case NO_ERROR:
         if ( getevent( (BYTE)VCMD, &evt ) )
         {
            switch ( evt.type )
            {
            case key:
                debug(F111,"congks","evt.key.scancode",evt.key.scancode);
                return evt.key.scancode;
            case macro:
                macrostr = evt.macro.string ;
                s = macrostr ;
                ch = *s++;
                if ( !*s ) {
                    free(macrostr);
                    macrostr = NULL ;
                    s = NULL;
                }
                debug(F111,"congks","evt.macro.string ch",ch);
                return(ch);
            case literal:
                macrostr = evt.literal.string ;
                s = macrostr ;
                ch = *s++;
                if ( !*s ) {
                    free(macrostr);
                    macrostr = NULL ;
                    s = NULL;
                }
                debug(F111,"congks","evt.literal.string ch",ch);
                return(ch);
#ifndef NOKVERBS
            case kverb:
                debug(F111,"congks","evt.kverb.id",evt.kverb.id);
                return F_KVERB | evt.kverb.id;
#endif /* NOKVERBS */
            case csi:
                debug(F111,"congks","evt.csi.key",evt.csi.key);
                return F_CSI | evt.csi.key;
            case esc:
                debug(F111,"congks","evt.esc.key",evt.esc.key);
                return F_ESC | evt.esc.key ;
            case error:
                return(-2);
            default:
                debug(F110,"congks","default",0);
               break;
            }
         }
         else break ;
      default:
         break ;
      }
      if (timeout != (ULONG) SEM_INDEFINITE_WAIT )
         i++ ;
   }
#else /* NT */
   while ( 1 )
   {
      rc = WaitEventAvailSem( VCMD, timeout ) ;
      if ( rc == NO_ERROR )
         if ( getevent( VCMD, &evt ) )
         {
            switch ( evt.type )
            {
            case key:
               return evt.key.scancode;
            case macro:
                macrostr = evt.macro.string ;
                s = macrostr ;
                ch = *s++;
                if ( !*s ) {
                    free(macrostr);
                    macrostr = NULL ;
                    s = NULL;
                }
                return(ch);
            case literal:
                macrostr = evt.literal.string ;
                s = macrostr ;
                ch = *s++;
                if ( !*s ) {
                    free(macrostr);
                    macrostr = NULL ;
                    s = NULL;
                }
                return(ch);
#ifndef NOKVERBS
            case kverb:
                return F_KVERB | evt.kverb.id;
#endif /* NOKVERBS */
            case csi:
                return F_CSI | evt.csi.key;
            case esc:
                return F_ESC | evt.esc.key ;
            case error:
                return(-2);
            default:
               break;
            }
         }
         else break ;
   }
#endif /* NT */
   return -1;
}

int
conraw() {
#ifdef NT
   DWORD mode ;
   extern int mouseon ;
#ifndef KUI
#ifdef COMMENT
   /* Don't call this directly - It's fatal in Windows 95 */
   SetConsoleCtrlHandler( NULL, TRUE ) ;
#endif /* COMMENT */
   GetConsoleMode( KbdHandle, &mode ) ;
   mode &= ~(ENABLE_LINE_INPUT | ENABLE_WINDOW_INPUT |
              ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | 0x40 );
#ifdef OS2MOUSE
    if ( mouseon )
      mode |= ENABLE_MOUSE_INPUT;
#endif /* OS2MOUSE */
   SetConsoleMode( KbdHandle, mode ) ;
#endif /* KUI */
    ckconraw = 1 ;
#else /* NT */
    KBDINFO k;

    memset( &k, 0, sizeof(k) ) ;

    if (!islocal) return(0);
    conmode = 1;
    k.cb = sizeof(k);
    KbdGetStatus(&k,KbdHandle);
    k.fsMask &= ~(KEYBOARD_ECHO_ON |
                  KEYBOARD_ASCII_MODE);
    k.fsMask |=  (KEYBOARD_ECHO_OFF
                | KEYBOARD_BINARY_MODE
      | KEYBOARD_SHIFT_REPORT);  /* Generates excessive key reports */
    return(KbdSetStatus(&k,KbdHandle));  /* But lets us see ScrollLock, Ctrl, Alt */
#endif /* NT */
    return(0);
}

int
concooked() {
#ifdef NT
#ifndef KUI
    DWORD mode ;
    extern int mouseon ;
#ifdef COMMENT
    /* Don't call this directly - It's fatal in Windows 95 */
    SetConsoleCtrlHandler( NULL, FALSE ) ;
#endif /* COMMENT */
    GetConsoleMode( KbdHandle, &mode ) ;
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT |
               ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT | 0x40);
    mode |= ENABLE_PROCESSED_INPUT ;
#ifdef OS2MOUSE
    if ( mouseon )
        mode |= ENABLE_MOUSE_INPUT;
#endif /* OS2MOUSE */
    SetConsoleMode( KbdHandle, mode ) ;
#endif /* KUI */
    ckconraw = 0 ;
    return(0);
#else /* NT */
    KBDINFO k;

    memset( &k, 0, sizeof(k) ) ;

    if (!islocal) return(0);
    conmode = 0;
    k.cb = sizeof(k);
    KbdGetStatus(&k,KbdHandle);
    k.fsMask &= ~(KEYBOARD_ECHO_OFF |
                  KEYBOARD_BINARY_MODE |
                  KEYBOARD_SHIFT_REPORT);
    k.fsMask |=  (KEYBOARD_ECHO_ON |
                  KEYBOARD_ASCII_MODE);
    return(KbdSetStatus(&k,KbdHandle));
#endif /* NT */
}

/*  C O N B I N  --  Put console in binary mode  */

/*  Returns 0 if ok, -1 if not  */

conbin(char esc) {
    if (!islocal) return(0);          /* only for real ttys */
#ifndef NOLOCAL
    if ( IsConnectMode() )
#endif /* NOLOCAL */
        conraw();
    return 0;
}

/*  C O N C B  -- Put console into single char mode with no echo. */

concb(char esc) {
    if (!islocal)
        return(0);          /* only for real ttys */
#ifndef NOLOCAL
    if (!IsConnectMode())
#endif /* NOLOCAL */
        concooked();
    return 0 ;
}


/*  C O N G M  -- Get console terminal mode. */

congm() { return 0 ; }

/*  C O N R E S -- Restore console to congm mode. */

conres() { return 0 ; }


/*  C O N I N T -- Enable console terminal interrupts. */

void
conint(f, s) SIGTYP (*f)(int), (*s)(int); {
    if ( signal(SIGINT, f) == SIG_ERR )
        debug(F101,"conint() could not set SIGINT","",f);

    if ( !quitonbreak )
        if ( signal(SIGBREAK, f) == SIG_ERR )
            debug(F101,"conint() could not set SIGBREAK","",f);
}


/*  C O N N O I -- No console interrupts. */

void connoi() {
    signal(SIGINT, SIG_IGN);
    if ( !quitonbreak )
        signal(SIGBREAK, SIG_IGN);
}


/* privilege dummy */

int priv_chk() {return 0;}


#ifndef __EMX__

/* alarm() implementation for all others, emx/gcc already has it built-in */
#define STACK
static BOOL alrm, running, isalarm;
static UINT delay;

#ifdef  WIN95_IN_AN_EMERGENCY
/* we are not using this code and should never need to */
alarm_thread(VOID *args) {
#ifdef NT
    setint();
#endif /* NT */
    SetThreadPrty(XYP_REG,15);

    while (1) {
       sleep(1) ;
       RequestAlarmMutex( SEM_INDEFINITE_WAIT ) ;
       if ( alrm )
          if ( --delay == 0 )
          {
             debug( F100, "alarm() triggered", "", 0 ) ;
             alrm = FALSE;
             isalarm = TRUE;
             isinterrupted = 1 ;
             PostAlarmSigSem() ;
          }
          else
          {
             debug( F101, "alarm() delay","",delay ) ;
          }
       ReleaseAlarmMutex() ;
    }

    running = FALSE;
    ckThreadEnd(args);
}

unsigned
alarm(unsigned sec) {
    unsigned old;
    APIRET rc = 0 ;
    ULONG TimeInterval = sec * 1000 ;

    RequestAlarmMutex( SEM_INDEFINITE_WAIT ) ;
    if ( delay ) {
       debug(F101,"alarm(0) StopAlarmTimer","",rc) ;
    }
    old = delay;
    delay = sec;

    if ( alrm = (delay > 0) ) {
       debug(F101,"alarm(t) StartAlarmTimer: t","",TimeInterval) ;
    } else
       debug(F100,"alarm() reset","",0);
    ReleaseAlarmMutex() ;

    if ( !running ) {
       running = TRUE;
       tidAlarm = (TID) ckThreadBegin( &alarm_thread,THRDSTKSIZ,0,FALSE,0);
    }
    return old;
}
#else /* not WIN95_IN_AN_EMERGENCY */
static VOID
alarm_thread(VOID *args) {
#ifdef NT
    setint();
#endif /* NT */

    SetThreadPrty(XYP_REG,15);

    while (1) {

        WaitAlarmSem( SEM_INDEFINITE_WAIT ) ;
        RequestAlarmMutex( SEM_INDEFINITE_WAIT ) ;
        ResetAlarmSem() ;
        if ( alrm ) {
            debug( F100, "alarm() triggered", "", 0 ) ;
            alrm = FALSE;
            isalarm = TRUE;
#ifdef NT
#ifdef NTSIG
            isinterrupted = 1 ;
            PostAlarmSigSem() ;
#else /* NTSIG */
            raise(SIGALRM) ;
#endif /* NTSIG */
#else /* NT */

            KillProcess(pid) ;
#endif /* NT */
        }
        ReleaseAlarmMutex() ;
    }

   /* this will never execute */
   running = FALSE;
   ckThreadEnd(args);
}

static void
alarm_signal(int sig) {
    debug(F101,"alarm_signal handler","",sig) ;
    signal(SIGTERM, term_signal);
    if (isalarm) {
        isalarm = FALSE;
        raise(SIGALRM);
    } else {
        KillProcess(pid);
    }
}

unsigned
alarm(unsigned sec) {
    unsigned old;
    APIRET rc = 0 ;
    ULONG TimeInterval = sec * 1000 ;

    RequestAlarmMutex( SEM_INDEFINITE_WAIT ) ;
    if ( delay ) {
        rc = StopAlarmTimer() ;
        debug(F111,"alarm(0) StopAlarmTimer","rc",rc) ;
    }
    old = delay;
    delay = sec;

    if ( alrm = (delay > 0) ) {
#ifndef NTSIG
        signal(SIGTERM, alarm_signal);
#endif /* NTSIG */
        rc = StartAlarmTimer( TimeInterval ) ;
        debug(F111,"alarm() StartAlarmTimer","TimeInterval",TimeInterval) ;
        debug(F111,"alarm() StartAlarmTimer","rc",rc) ;
    } else
      debug(F100,"alarm() reset","",0);

    ReleaseAlarmMutex() ;

    if ( !running ) {
        running = TRUE;
        tidAlarm = (TID) ckThreadBegin( &alarm_thread,THRDSTKSIZ,0,FALSE,0);
    }
    return old;
}
#endif /* WIN95_IN_AN_EMERGENCY */

/*
 *  A public domain implementation of BSD directory routines for
 *  MS-DOS.  Written by Michael Rendell ({uunet,utai}michael@garfield),
 *  August 1897
 *  Ported to OS/2 by Kai Uwe Rommel
 *  December 1989, February 1990
 *  Change for HPFS support, October 1990
 */

int attributes = A_DIR | A_HIDDEN | A_RONLY | A_SYSTEM | A_ARCHIVE ;

static char *getdirent(char *);
static void free_dircontents(struct _dircontents *);

static HDIR hdir;
static U_INT count;
#ifdef NT
static WIN32_FIND_DATA find;
#else /* NT */
static FILEFINDBUF find;
#endif /* NT */

int IsFileSystemFAT(char *dir)
{
   USHORT nResult;
#ifdef NT
   char rootpath[4] ;
   char filesys[20];
   DWORD namemax = 0;

    if ( isalpha(dir[0]) && (dir[1] == ':') )
    {
        rootpath[0] = dir[0] ;
        rootpath[1] = dir[1] ;
        rootpath[2] = '\\' ;
        rootpath[3] = '\0' ;
        GetVolumeInformation( rootpath, NULL, 0, &namemax, NULL, NULL, filesys, 20 ) ;
    }
    else
    {
        GetVolumeInformation( NULL, NULL, 0, &namemax, NULL, NULL, filesys, 20 ) ;
    }
    debug(F111,"File system max name length",filesys,namemax) ;
#ifdef COMMENT
    nResult = !strcmp(filesys, "FAT");
#else
    nResult = ( namemax == 11 ) ;
#endif
#else /* NT */
    static USHORT nLastDrive = -1;
    ULONG lMap;
    BYTE bData[64], bName[3];
    U_INT nDrive, cbData;
    FSQBUFFER *pData = (FSQBUFFER *) bData;

    /* We separate FAT and HPFS file systems here. */

    if ( isalpha(dir[0]) && (dir[1] == ':') )
        nDrive = toupper(dir[0]) - '@';
    else
        DosQueryCurrentDisk(&nDrive, &lMap);

    if ( nDrive == nLastDrive )
        return nResult;

    bName[0] = (char) (nDrive + '@');
    bName[1] = ':';
    bName[2] = 0;

    nLastDrive = nDrive;
    cbData = sizeof(bData);

    if ( !DosQueryFSAttach(bName, 0, FSAIL_QUERYNAME, (PVOID) pData, &cbData) )
        nResult = !strcmp(pData -> szFSDName + pData -> cbName, "FAT");
    else
        nResult = FALSE;

#endif /* NT */
    return nResult;
}

DIR *
opendir(char *name)
{
#ifdef NT
    struct _stat statb;
#else /* NT */
    struct stat statb;
#endif /* NT */
    DIR *dirp;
    char c;
    char *s;
    struct _dircontents *dp;
    char nbuf[MAXPATHLEN + 1];
    int len;

    ckstrncpy(nbuf, name, MAXPATHLEN+1);
    if ((len = strlen(nbuf)) == 0)
        return NULL;

    if ( ((c = nbuf[len - 1]) == '\\' || c == '/') && (len > 1) )
    {
        nbuf[len - 1] = 0;
        --len;

        if ( nbuf[len - 1] == ':' )
        {
            ckstrncpy(nbuf + len, "\\.",MAXPATHLEN+1-len);
            len += 2;
        }
    }
    else
        if ( nbuf[len - 1] == ':' )
        {
            ckstrncpy(nbuf+len, ".",MAXPATHLEN+1-len);
            ++len;
        }

#ifdef NT
    if (_stat(nbuf, &statb) < 0 || (statb.st_mode & _S_IFMT) != _S_IFDIR)
#else /* NT */
    if (stat(nbuf, &statb) < 0 || (statb.st_mode & _S_IFMT) != _S_IFDIR)
#endif /* NT */
        return NULL;

    if ( (dirp = malloc(sizeof(DIR))) == NULL )
        return NULL;

    if ( nbuf[len - 1] == '.' && (len == 1 || nbuf[len - 2] != '.') )
        ckstrncpy(nbuf + len - 1, "*",MAXPATHLEN+1-len);
    else
        if ( ((c = nbuf[len - 1]) == '\\' || c == '/') && (len == 1) )
            ckstrncpy(nbuf + len, "*",MAXPATHLEN+1-len);
        else
            ckstrncpy(nbuf + len, "/*",MAXPATHLEN+1-len);

    dirp -> dd_loc = 0;
    dirp -> dd_contents = dirp -> dd_cp = NULL;

    if ((s = getdirent(nbuf)) == NULL)
        return dirp;

    do
    {
        if (((dp = malloc(sizeof(struct _dircontents))) == NULL) ||
             ((dp -> _d_entry = malloc(strlen(s) + 1)) == NULL)      )
        {
            if (dp)
                free(dp);
            free_dircontents(dirp -> dd_contents);

            return NULL;
        }

        if (dirp -> dd_contents)
        {
            dirp -> dd_cp -> _d_next = dp;
            dirp -> dd_cp = dirp -> dd_cp -> _d_next;
        }
        else
            dirp -> dd_contents = dirp -> dd_cp = dp;

        strcpy(dp -> _d_entry, s);
        dp -> _d_next = NULL;

#ifdef NT
        /* I am not going to worry about this data because it is never
        used anywhere within Kermit */
        dp->_d_size = 0 ;
        dp->_d_mode = 0 ;
        dp->_d_time = 0 ;
        dp->_d_date = 0 ;
#else /* NT */
        dp -> _d_size = find.cbFile;
        dp -> _d_mode = find.attrFile;
        dp -> _d_time = *(unsigned *) &(find.ftimeLastWrite);
        dp -> _d_date = *(unsigned *) &(find.fdateLastWrite);
#endif /* NT */
    }
    while ((s = getdirent(NULL)) != NULL);

    dirp -> dd_cp = dirp -> dd_contents;

    return dirp;
}


void
closedir(DIR * dirp)
{
  free_dircontents(dirp -> dd_contents);
  free(dirp);
}


struct dirent *
readdir(DIR * dirp)
{
    static struct dirent dp;

    if (dirp -> dd_cp == NULL)
        return NULL;

    dp.d_namlen = dp.d_reclen =
        strlen(strcpy(dp.d_name, dirp -> dd_cp -> _d_entry));

    dp.d_ino = 1;

    dp.d_size = dirp -> dd_cp -> _d_size;
    dp.d_mode = dirp -> dd_cp -> _d_mode;
    dp.d_time = dirp -> dd_cp -> _d_time;
    dp.d_date = dirp -> dd_cp -> _d_date;

    dirp -> dd_cp = dirp -> dd_cp -> _d_next;
    dirp -> dd_loc++;

    return &dp;
}


void
seekdir(DIR * dirp, long off)
{
    long i = off;
    struct _dircontents *dp;

    if (off >= 0)
    {
        for (dp = dirp -> dd_contents; --i >= 0 && dp; dp = dp -> _d_next);

        dirp -> dd_loc = off - (i + 1);
        dirp -> dd_cp = dp;
    }
}


long
telldir(DIR * dirp)
{
  return dirp -> dd_loc;
}


static void
free_dircontents(struct _dircontents * dp)
{
  struct _dircontents *odp;

  while (dp)
  {
    if (dp -> _d_entry)
      free(dp -> _d_entry);

    dp = (odp = dp) -> _d_next;
    free(odp);
  }
}


char *
getdirent(char *dir)
{
    int done;
    static int lower = TRUE;

    if (dir != NULL)
    {                                  /* get first entry */
        lower = IsFileSystemFAT(dir);

        count = 1;
#ifdef NT
        hdir = FindFirstFile( dir, &find ) ;
        done = (hdir == INVALID_HANDLE_VALUE) ;
#else /* NT */
        hdir = HDIR_CREATE;
        done = DosFindFirst(dir, &hdir, attributes, &find, sizeof(find), &count);
#endif /* NT */
    }
    else                                       /* get next entry */
#ifdef NT
      done = !FindNextFile( hdir, &find ) ;
    count++ ;
#else /* NT */
    done = DosFindNext(hdir, &find, sizeof(find), &count);
#endif /* NT */

    if (done == 0)
    {
        if ( lower )
#ifdef NT
          _strlwr(find.cFileName) ;
        return find.cFileName;
#else /* NT */
        strlwr(find.achName);
        return find.achName;
#endif /* NT */
    }
    else
    {
#ifdef NT
        FindClose(hdir) ;
#else /* NT */
        DosFindClose(hdir);
#endif /* NT */
        return NULL;
    }
}

#endif /* __EMX__ */

#ifdef COMMENT
#ifdef NT
/* This code was used to allow external commands to be executed on Win95 */
/* by redirecting output to a file and then reading from the file        */

#define POPENMAX 4
static struct {
    char filename[MAX_PATH+1];
    int  read ;
    char *cmd ;
    FILE *file;
} win95p[POPENMAX] =
{
    "",0,NULL,NULL,
    "",0,NULL,NULL,
    "",0,NULL,NULL,
    "",0,NULL,NULL
};
FILE *
win95popen(char *cmd, char *mode) {
    /* Another Windows 95 bug ... */
    char popenpathname[MAX_PATH+1];
    char fullcmd[MAX_PATH*2];
    HFILE end1, end2, std, old1, old2, temp;
    char fail[256], cmd_line[256], *cmd_exe, *args, *p;
    int rc,i,n;
    extern char * tempdir ;

    for ( i=0; i<POPENMAX; i++ )
    {
        if ( !win95p[i].file )
          break;
    }
    if ( i == POPENMAX )
      return ((FILE *) NULL);

    /* Get TEMP or TMP area, if any */

    if (tempdir) {
        ckstrncpy(popenpathname,tempdir,MAX_PATH+1);
    } else {
        p = getenv("K95TMP");           /* ifdef NT */
        if (!p)
          p = getenv("TEMP");
        if (!p)
          p = getenv("TMP");
        if (p)
        {
            ckstrncpy(popenpathname,p,MAX_PATH+1);
        }
        else
        {
            /* None, then use K95's */
            /* own TMP directory. */
            n = ckstrncpy(popenpathname,exedir,MAX_PATH+1);
            ckstrncpy(&popenpathname[n],"TMP",MAX_PATH+1-n);
            if (!isdir(popenpathname))        /* If none, */
                if (_mkdir(popenpathname) < 0)  /* make one. */
                    return((FILE *) NULL);
        }
    }

    /* Get temporary file name */
    if (!GetTempFileName(popenpathname,"K95",0,(LPTSTR) win95p[i].filename))
        return((FILE *) NULL);

    switch ( mode[0] ) {
    case 'r':
        win95p[i].read = 1 ;
        sprintf(fullcmd, "%s > %s", cmd, win95p[i].filename); /* Command */
        system(fullcmd);
        return (win95p[i].file = fopen(win95p[i].filename,"r"));
    case 'w':
        win95p[i].read = 0 ;
        win95p[i].cmd = strdup(cmd);
        return (win95p[i].file = fopen(win95p[i].filename,"w"));
    default:
        return ((FILE *) NULL);
    }
}

int
win95pclose(FILE *pipe) {
    int i;

    for ( i=0; i<POPENMAX ; i++)
    {
       if ( win95p[i].file == pipe )
         break;
    }
    if ( i == POPENMAX )
      return 1;

    fclose(pipe);

    if ( !win95p[i].read && win95p[i].cmd )
    {
        char fullcmd[MAX_PATH*2];
        sprintf(fullcmd, "%s < %s", win95p[i].cmd, win95p[i].filename); /* Command */
        system(fullcmd);
        free(win95p[i].cmd);
        win95p[i].cmd = NULL ;
    }

    zdelet(win95p[i].filename);
    win95p[i].filename[0] = '\0';
    win95p[i].file = NULL ;
    return(0);
}
#endif /* NT */
#endif /* COMMENT */

#ifdef NT
#ifdef COMMENT
/* This code was originally used when we did not think that there was a */
/* popen()/pclose() combination in the C Run Time Library               */
HANDLE pids[64];

FILE *
win95popen(char *cmd, char *mode) {
    HANDLE end1, end2;
    FILE *file;
    int  ofile;
    char fail[256], cmd_line[256], *cmd_exe, *args, *p;
    int rc, n;
    STARTUPINFO si;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;

    debug(F110,"popen",cmd,0);

    memset( &sa, 0, sizeof(SECURITY_ATTRIBUTES) );     //  Initialize struct
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=1;

    if (!CreatePipe(&end1, &end2, &sa, 4096))
        return NULL;

    cmd_exe = getenv("SHELL");
    if ( !cmd_exe )
        cmd_exe = getenv("COMSPEC");
    if ( !cmd_exe )
        cmd_exe = "cmd.exe";

    args = cmd_line + ckstrncpy(cmd_line, cmd_exe,256); /* do not skip zero */

    /* Look for MKS Shell, if found use -c instead of /c */
#ifdef NT
    _strlwr(cmd_exe);
#else
    strlwr(cmd_exe);
#endif /* NT */
    p = strstr(cmd_exe,"sh.exe");
    if ( !p )
        p = strstr(cmd_exe,"bash.exe");
    if ( !p )
        p = strstr(cmd_exe,"ash.exe");
    if ( p && (p == cmd_exe || *(p-1) == '\\' || *(p-1) == '/')) {
        sprintf(args, " -c \"%s\"",cmd);
    }
    else {
        n = ckstrncpy(args, " /c ", 256);
        ckstrncpy(args, cmd, 256-n);
    }

    memset( &si, 0, sizeof(STARTUPINFO) );     //  Initialize struct
    si.cb          = sizeof(STARTUPINFO);
    si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = 0;        //  Don't show the console window (DOS box)

    if ( *mode == 'r' )
        si.hStdOutput  = end2;     //  Redirect command line
    si.hStdError   = NULL;
    if ( *mode == 'w' )
        si.hStdInput   = end1;

    if ( !CreateProcess ( NULL,
                          cmd_line,
                          NULL,
                          NULL,
                          TRUE, // bInheritHandler
                          0,
                          NULL,
                          NULL,
                          &si,
                          &pi) )
    {
        rc = GetLastError();
        debug(F111,"popen CreateProcess failed","GetLastError",rc);
        CloseHandle(end1);
        CloseHandle(end2);
        return NULL;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if ( *mode == 'w' )
    {
        CloseHandle(end1);
        ofile = _open_osfhandle( (long) end2, _O_APPEND );
        if ( ofile == -1 )
        {
            debug(F111,"popen _open_osfhandle failed","error",error);
            CloseHandle(end2);
            return NULL;
        }
        file = _fdopen( ofile, mode );
        pids[ofile] = pi.hProcess;
    }
    else
    {
        CloseHandle(end2);
        ofile = _open_osfhandle( (long) end1, _O_RDONLY );
        if ( ofile == -1 )
        {
            debug(F111,"popen _open_osfhandle failed","error",error);
            CloseHandle(end1);
            return NULL;
        }
        file = _fdopen( ofile, mode );
        pids[ofile] = pi.hProcess;
    }
    debug(F111,"popen","PID",pi.hProcess);
    debug(F111,"popen","FILE*",file);
    return file;
}

int
win95pclose(FILE *pipe) {
    int    fileno = _fileno(pipe);
    DWORD  exitcode=0;
    HANDLE handle = (HANDLE) _get_osfhandle(_fileno(pipe));

    debug(F111,"pclose","FILE*",pipe);
    fclose(pipe);
    if (pids[fileno]) {
        WaitForSingleObject ((HANDLE) pids[(int)fileno], INFINITE);
        if (!GetExitCodeProcess((HANDLE) pids[(int)fileno], &exitcode)) {
            exitcode = 128;
        }
        pids[fileno] = 0;
    }

    return (exitcode == 128 ? -1 : exitcode<<8);
}
#endif /* COMMENT */
#else /* NT */
#ifdef __IBMC__

/* quick hack because IBM C lacks popen() and pclose() */

int pids[64];

FILE *
popen(char *cmd, char *mode) {
  HFILE end1, end2, std, old1, old2, temp;
  FILE *file;
  char fail[256], cmd_line[256], *cmd_exe, *args, *p;
  RESULTCODES res;
  int rc, n;

    debug(F110,"popen",cmd,0);
    if (DosCreatePipe(&end1, &end2, 4096))
        return NULL;

    std = (*mode == 'w') ? 0 /* stdin */ : 1 /* stdout */;
    if (std == 0) {
        temp = end1; end1 = end2; end2 = temp;
    }

    old1 = -1; /* save stdin or stdout */
    DosDupHandle(std, &old1);
    DosSetFHState(old1, OPEN_FLAGS_NOINHERIT);
    temp = std; /* redirect stdin or stdout */
    DosDupHandle(end2, &temp);

    if ( std == 1 ) {
        old2 = -1; /* save stderr */
        DosDupHandle(2, &old2);
        DosSetFHState(old2, OPEN_FLAGS_NOINHERIT);
        temp = 2;   /* redirect stderr */
        DosDupHandle(end2, &temp);
    }

    DosClose(end2);
    DosSetFHState(end1, OPEN_FLAGS_NOINHERIT);

    cmd_exe = getenv("SHELL");
    if ( !cmd_exe )
        cmd_exe = getenv("COMSPEC");
    if ( !cmd_exe )
        cmd_exe = "cmd.exe";

    debug(F110,"popen cmd_exe",cmd_exe,0);
    n = ckstrncpy(cmd_line, cmd_exe, 256);
    args = cmd_line + n + 1; /* skip zero */

    /* Look for MKS Shell, if found use -c instead of /c */
    strlwr(cmd_exe);
    p = strstr(cmd_exe,"sh.exe");
    if ( !p )
        p = strstr(cmd_exe,"bash.exe");
    if ( !p )
        p = strstr(cmd_exe,"ash.exe");
    if ( p && (p == cmd_exe || *(p-1) == '\\' || *(p-1) == '/')) {
        sprintf(args, "-c%c%s%c",' ',cmd,0);
    }
    else {
        sprintf(args, "/c%c%s%c",' ',cmd,0);
    }
    ckhexrump("popen cmd_line",cmd_line,256);

    rc = DosExecPgm(fail, sizeof(fail), EXEC_ASYNCRESULT,
                  cmd_line, 0, &res, cmd_exe);

    temp = std; /* restore stdin or stdout */
    DosDupHandle(old1, &temp);
    DosClose(old1);

    if ( std == 1 ) {
        temp = 2;   /* restore stderr */
        DosDupHandle(old2, &temp);
        DosClose(old2);
    }

    if (rc) {
        debug(F111,"popen failed",fail,rc);
        DosClose(end1);
        return NULL;
    }

    file = fdopen(end1, mode);
    pids[end1] = res.codeTerminate;
    debug(F111,"popen","PID",res.codeTerminate);
    debug(F111,"popen","FILE*",file);
    return file;
}

int
pclose(FILE *pipe) {
    RESULTCODES rc;
    PID pid;
    int handle = fileno(pipe);

    debug(F111,"pclose","FILE*",pipe);
    fclose(pipe);
    if (pids[handle]) {
        DosWaitChild(DCWA_PROCESSTREE, DCWW_WAIT, &rc, &pid, pids[handle]);
        pids[handle] = 0;
        debug(F111,"pclose","rc.codeResult",rc.codeResult);
    }
    return rc.codeTerminate == 0 ? (rc.codeResult<<8) : -1;
}
#endif /* IBMC */
#endif /* NT */

#ifdef CK_REDIR
#ifdef NT
static DWORD exitcode;

void
ttruncmd2( HANDLE pipe )
{
    int success = 1;
    CHAR outc;
    DWORD io;

    while ( success && exitcode == STILL_ACTIVE ) {
        if ( success = ReadFile( pipe, &outc, 1, &io, NULL ) )
        {
            ttoc(outc) ;
        }
    }
    exitcode = 0;
}

int
ttruncmd(char * cmd)
{ /* Return: 0 = failure, 1 = success */
    extern int pexitstat;
    int rc = 0, n;

    if (!cmd) return(0);
    if (!cmd[0]) return(0);

#ifdef COMMENT
    if ( 0 && isWin95() )
    {
        char buf[512] ;
        int n ;
        FILE * fd = win95popen(cmd,"rb");
        if ( fd )
        {
            while ( !feof(fd) )
            {
                n = fread( buf, sizeof(char), (size_t) 512, fd ) ;
                if ( n )
                    ttol( buf, n ) ;
            }
            pexitstat = win95pclose(fd);
            if ( pexitstat & 0xff )
                pexitstat >>= 8 ;
            else
                pexitstat = -4;
            return 1;
        }
        else {
            pexitstat = -4;
            return 0;
        }
    }
    else
#endif /* COMMENT */
    {
        HANDLE hSaveStdIn, hSaveStdOut, hSaveStdErr;
        HANDLE hChildStdinRd, hChildStdinWr, hChildStdinWrDup,
        hChildStdoutRd, hChildStdoutWr, hChildStdoutRdDup,
        hInputFile, hSaveStdin, hSaveStdout;
        SECURITY_ATTRIBUTES saAttr;
        BOOL fSuccess;
        PROCESS_INFORMATION procinfo ;
        STARTUPINFO         startinfo ;
        char cmd_line[256], *cmd_exe, *args, *p;

        pexitstat = -4;
        if (ttyfd == -1
#ifdef CK_TAPI
             || (tttapi && ttyfd == -2)
#endif /* CK_TAPI */
             ) {
            printf("?Sorry, device is not open\n");
            return 0;
        }

        hSaveStdOut = GetStdHandle( STD_OUTPUT_HANDLE ) ;
        hSaveStdIn  = GetStdHandle( STD_INPUT_HANDLE ) ;
        hSaveStdErr = GetStdHandle( STD_ERROR_HANDLE ) ;

        /* Set the bInheritHandle flag so pipe handles are inherited. */

        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        /*
        * The steps for redirecting child's STDOUT:
        *     1.  Save current STDOUT, to be restored later.
        *     2.  Create anonymous pipe to be STDOUT for child.
        *     3.  Set STDOUT of parent to be write handle of pipe, so
        *         it is inherited by child.
        */

        /* Create a pipe for the child's STDOUT. */

        if (! CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
            debug(F100,"Stdout pipe creation failed\n","",0);

        /* Set a write handle to the pipe to be STDOUT. */

        if (! SetStdHandle(STD_OUTPUT_HANDLE, hChildStdoutWr) ||
             ! SetStdHandle(STD_ERROR_HANDLE, hChildStdoutWr) )
            debug(F100,"Redirecting STDOUT/STDERR failed","",0);

        /*
        * The steps for redirecting child's STDIN:
        *     1.  Save current STDIN, to be restored later.
        *     2.  Create anonymous pipe to be STDIN for child.
        *     3.  Set STDIN of parent to be read handle of pipe, so
        *         it is inherited by child.
        *     4.  Create a noninheritable duplicate of write handle,
        *         and close the inheritable write handle.
        */

        /* Create a pipe for the child's STDIN. */

        if (! CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
            debug(F100,"Stdin pipe creation failed\n","",0);

        /* Set a read handle to the pipe to be STDIN. */

        if (! SetStdHandle(STD_INPUT_HANDLE, hChildStdinRd))
            debug(F100,"Redirecting Stdin failed","",0);

        /* Duplicate the write handle to the pipe so it is not inherited. */

        fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
                                  GetCurrentProcess(), &hChildStdinWrDup, 0,
                                  FALSE,       /* not inherited */
                                  DUPLICATE_SAME_ACCESS);
        if (! fSuccess) {
            debug(F100,"DuplicateHandle failed","",0);

            SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdOut );
            SetStdHandle( STD_INPUT_HANDLE, hSaveStdIn );
            SetStdHandle( STD_ERROR_HANDLE, hSaveStdErr );

            CloseHandle(hChildStdoutRd);  hChildStdoutRd = NULL;
            CloseHandle(hChildStdoutWr);  hChildStdoutWr = NULL;
            CloseHandle(hChildStdinRd);  hChildStdinRd = NULL;
            CloseHandle(hChildStdinWr);  hChildStdinWr = NULL;
            return(0);
        }

        fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
                                    GetCurrentProcess(), &hChildStdoutRdDup, 0,
                                    FALSE,       /* not inherited */
                                    DUPLICATE_SAME_ACCESS);
        if (! fSuccess) {
            debug(F100,"DuplicateHandle failed","",0);

            SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdOut );
            SetStdHandle( STD_INPUT_HANDLE, hSaveStdIn );
            SetStdHandle( STD_ERROR_HANDLE, hSaveStdErr );

            CloseHandle(hChildStdoutRd);  hChildStdoutRd = NULL;
            CloseHandle(hChildStdoutWr);  hChildStdoutWr = NULL;
            CloseHandle(hChildStdinRd);  hChildStdinRd = NULL;
            CloseHandle(hChildStdinWr);  hChildStdinWr = NULL;
            return(0);
        }

      CloseHandle(hChildStdinWr);
        hChildStdinWr = NULL;

      cmd_line[0] = '\0' ;
      /* Now create the child process. */


        cmd_exe = getenv("SHELL");
        if ( !cmd_exe )
            cmd_exe = getenv("COMSPEC");
        if ( !cmd_exe )
            cmd_exe = "cmd.exe";

      n = ckstrncpy(cmd_line, cmd_exe, 256);
      args = cmd_line + n; /* don't skip zero */

        /* Look for MKS Shell, if found use -c instead of /c */
        _strlwr(cmd_exe);
        p = strstr(cmd_exe,"sh.exe");
        if ( !p )
            p = strstr(cmd_exe,"bash.exe");
        if ( !p )
            p = strstr(cmd_exe,"ash.exe");
        if ( p && (p == cmd_exe || *(p-1) == '\\' || *(p-1) == '/')) {
            sprintf(args, " -c \"%s\"",cmd);
        }
        else {
            int m;
            m = ckstrncpy(args, " /c ", 256 - n);
            ckstrncpy(&args[m], cmd, 256 - n - m);
        }

      memset( &startinfo, 0, sizeof(STARTUPINFO) ) ;
      startinfo.cb = sizeof(STARTUPINFO) ;

      fSuccess = CreateProcess( NULL,       /* application name */
                     cmd_line,              /* command line */
                     NULL,                  /* process security attributes */
                     NULL,                  /* primary thread security attrs */
                     TRUE,                  /* inherit handles */
                     NORMAL_PRIORITY_CLASS, /* creation flags */
                     NULL,                  /* use parent's environment */
                     NULL,                  /* use parent's current directory */
                     &startinfo,            /* startup info */
                     &procinfo ) ;          /* proc info returned */

        if ( !SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdOut ) ||
             !SetStdHandle( STD_INPUT_HANDLE, hSaveStdIn ) ||
             !SetStdHandle( STD_ERROR_HANDLE, hSaveStdErr ) )
        {
            debug( F101,"ttruncmd Unable to restore standard handles","",GetLastError() ) ;
            CloseHandle(hChildStdoutRd);  hChildStdoutRd = NULL;
            CloseHandle(hChildStdoutWr);  hChildStdoutWr = NULL;
            CloseHandle(hChildStdinRd);  hChildStdinRd = NULL;
            CloseHandle(hChildStdinWrDup);  hChildStdinWrDup = NULL;
        }

        if ( !fSuccess ) {
            CloseHandle(hChildStdoutRd);  hChildStdoutRd = NULL;
            CloseHandle(hChildStdoutWr);  hChildStdoutWr = NULL;
            CloseHandle(hChildStdinRd);  hChildStdinRd = NULL;
            CloseHandle(hChildStdinWrDup);  hChildStdinWrDup = NULL;
            return(0);
        }

        CloseHandle(procinfo.hProcess);
        CloseHandle(procinfo.hThread);

        exitcode = STILL_ACTIVE;
        _beginthread( ttruncmd2, 65536, hChildStdoutRd );
        do {
            DWORD io ;
            int  inc ;
            unsigned char outc ;

            inc = ttinc( 1 ) ;
            if ( inc < -1 )
                break ;
            else if ( inc >= 0 )
            {
                if ( !WriteFile( hChildStdinWrDup, &inc, 1, &io, NULL ) )
                {
                    debug(F101,"ttruncmd unable to write to child","",GetLastError());
                    break;
                }
                continue;
            }
            // Give the process time to execute and finish
            if (WaitForSingleObject(procinfo.hProcess, 0L) == WAIT_OBJECT_0) {
                GetExitCodeProcess(procinfo.hProcess, &exitcode);
            }
        } while ( exitcode == STILL_ACTIVE );
        debug(F111, "ttruncmd","exitcode",exitcode);

        if ( exitcode == STILL_ACTIVE )
            exitcode = 128;

        /* Close the pipe handle so the child stops reading. */
        CloseHandle(hChildStdoutRd);    hChildStdoutRd = NULL;
        CloseHandle(hChildStdoutWr);    hChildStdoutWr = NULL;
        CloseHandle(hChildStdinRd);     hChildStdinRd = NULL;
        CloseHandle(hChildStdinWrDup);  hChildStdinWrDup = NULL;

        CloseHandle( procinfo.hProcess ) ;
        CloseHandle( procinfo.hThread ) ;

        pexitstat = exitcode;
        return (exitcode>=0 ? 1 : 0);
    }
    return 0;   /* Should never be reached */
}
#else /* not NT */
#define STILL_ACTIVE -1L
static ULONG exitcode = 0;
void
ttruncmd2( HFILE pipe )
{
    int success = 1;
    CHAR outc;
    ULONG io;

    while ( success && exitcode == STILL_ACTIVE ) {
        if ( success = !DosRead( pipe, &outc, 1, &io ) )
        {
            ttoc(outc) ;
        }
    }
    exitcode = 0;
}

int
ttruncmd(cmd) char *cmd; { /* Return: 0 = failure, 1 = success */
  extern int pexitstat;
    HFILE hSaveStdOut=-1, hSaveStdIn=-1, hSaveStdErr=-1;
    HFILE hChildStdoutRd, hChildStdoutWr, hChildStdinRd, hChildStdinWr;
    HFILE temp;
    FILE *file;
    char fail[256], cmd_line[256], *cmd_exe, *args, *p;
    RESULTCODES res;
    PID pid, pid2;
    int n, rc;

    debug(F110,"ttruncmd",cmd,0);

    /* Save existing handles */
    DosDupHandle(0,&hSaveStdIn);
    DosSetFHState(hSaveStdIn, OPEN_FLAGS_NOINHERIT);
    DosDupHandle(1,&hSaveStdOut);
    DosSetFHState(hSaveStdOut, OPEN_FLAGS_NOINHERIT);
    DosDupHandle(2,&hSaveStdErr);
    DosSetFHState(hSaveStdErr, OPEN_FLAGS_NOINHERIT);

    /* Create a pipe for the child's STDOUT */
    if (DosCreatePipe(&hChildStdoutRd, &hChildStdoutWr, 4096))
        debug(F100,"Stdout pipe creation failed\n","",0);

    /* Set a write handle to the pipe to be STDOUT */
    temp = 1;   /* stdout */
    DosDupHandle(hChildStdoutWr, &temp);
    temp = 2;   /* stderr */
    DosDupHandle(hChildStdoutWr, &temp);

    /* Create a pipe for the child's STDOUT */
    if (DosCreatePipe(&hChildStdinRd, &hChildStdinWr, 4096))
        debug(F100,"Stdout pipe creation failed\n","",0);

    /* Set a write handle to the pipe to be STDOUT */
    temp = 0;   /* stdin */
    DosDupHandle(hChildStdinRd, &temp);

    DosSetFHState(hChildStdoutRd, OPEN_FLAGS_NOINHERIT);
    DosSetFHState(hChildStdinWr, OPEN_FLAGS_NOINHERIT);

    cmd_exe = getenv("SHELL");
    if ( !cmd_exe )
        cmd_exe = getenv("COMSPEC");
    if ( !cmd_exe )
        cmd_exe = "cmd.exe";

    n = ckstrncpy(cmd_line, cmd_exe, 256);
    args = cmd_line + n + 1; /* skip zero */

    /* Look for MKS Shell, if found use -c instead of /c */
    strlwr(cmd_exe);
    p = strstr(cmd_exe,"sh.exe");
    if ( !p )
        p = strstr(cmd_exe,"bash.exe");
    if ( !p )
        p = strstr(cmd_exe,"ash.exe");
    if ( p && (p == cmd_exe || *(p-1) == '\\' || *(p-1) == '/')) {
        sprintf(args, "-c%c%s%c",0,cmd,0);
    }
    else {
        sprintf(args, "/c%c%s%c",0,cmd,0);
    }
    rc = DosExecPgm(fail, sizeof(fail), EXEC_ASYNCRESULT,
                    args, 0, &res, cmd_exe);

    /* Restore Standard Handles for this process */
    temp = 0;
    DosDupHandle(hSaveStdIn,&temp);
    temp = 1;
    DosDupHandle(hSaveStdOut,&temp);
    temp = 2;
    DosDupHandle(hSaveStdErr,&temp);

    if (rc) {
        debug(F111,"ttruncmd failed",fail,rc);
        DosClose(hChildStdinRd);
        DosClose(hChildStdinWr);
        DosClose(hChildStdoutRd);
        DosClose(hChildStdoutWr);
        return 1;
    }

    pid = res.codeTerminate;
    debug(F111,"ttruncmd","PID",pid);

    exitcode = STILL_ACTIVE;
    _beginthread( ttruncmd2, 0, 65536, hChildStdoutRd );
    do {
        ULONG io ;
        int inc ;
        CHAR outc ;

        inc = ttinc( 1 ) ;
        if ( inc < -1 )
            break ;
        else if ( inc >= 0 )
        {
            if ( rc = DosWrite( hChildStdinWr, &inc, 1, &io ) )
            {
                debug(F101,"ttruncmd unable to write to child","",rc);
                break;
            }
            continue;
        }
        res.codeTerminate = 99;
        rc = DosWaitChild(DCWA_PROCESSTREE, DCWW_NOWAIT, &res, &pid2, pid);
        if ( (rc==0) && (res.codeTerminate != 99) ) {
            exitcode = res.codeResult;
        }

    } while ( exitcode == STILL_ACTIVE );
    if ( exitcode == STILL_ACTIVE && pid )
    {
        exitcode = 128;
        if (!DosWaitChild(DCWA_PROCESSTREE, DCWW_WAIT, &res, &pid2, pid))
            exitcode = res.codeResult;
        debug(F111,"ttruncmd","res.codeResult",res.codeResult);

    }

    DosClose(hChildStdinRd);
    DosClose(hChildStdinWr);
    DosClose(hChildStdoutRd);
    DosClose(hChildStdoutWr);

    pexitstat = exitcode;
    return (exitcode>=0 ? 1 : 0);
}
#endif /* NT */
#endif /* CK_REDIR */

/*
  Keyboard                        Hardware ID
  PC AT* Standard Keyboard        0001H
  101 Key Enhanced Keyboard and   AB41H
  102 Key Enhanced Keyboard
   88 Key Enhanced Keyboard and   AB54H
   89 Key Enhanced Keyboard
  122 Key Mainframe Interactive   AB86H
      (MFI) Keyboard
*/
static char os2kbt[20];
char *
conkbg(void) {
    int x=0;
    char * p=os2kbt;
#ifdef NT
    os2kbt[0] = '\0' ;
    switch (GetKeyboardType(0))  /* Get Keyboard subtype */
    {
    case 1:
        x=83;
        break;
    case 2:
    case 4:
        x=102;
        break;
    case 3:
        x = 84 ;
        break;
    case 5:
        x=1050;
        break;
    case 6:
        x=9140;
        break;
    default:
        x=0 ;
    }

    if (x)                              /* If it's known model */
        sprintf(p,"%d",x);              /* use its "name" */
    else                                /* otherwise */
        sprintf(p,"unknown"); /* unknown */
#else /* NT */
    KBDHWID kbID        ;
    int rc=0;

    *p = '\0';

    memset( &kbID, 0, sizeof(kbID) ) ;

    kbID.cb = sizeof(kbID);
    KbdGetHWID(&kbID, KbdHandle);
    debug(F101,"conkbg","",kbID.idKbd);
    switch (kbID.idKbd) {
    case 0x0001:                        /* PC/AT keyboard */
    case 0xab54:                        /* PC or PC/XT 88 or 89 key */
        x = 88;
        break;
    case 0xab41:                        /* 101 or 102 enhanced keyboard */
    case 0xab83:
        x = 101;
        break;
    case 0xab86:                        /* 122-key "mainframe interactive" */
        x = 122;
        break;
    default:
        x = 0; break;   /* Something else... */
    }
    if (x)                              /* If it's known model */
        sprintf(p,"%d",x);              /* use its "name" */
    else                                /* otherwise */
        sprintf(p,"%04X",(int) kbID.idKbd); /* use the hex code */
#endif /* NT */

    return(p);                          /* Return string pointer */
};


#ifdef CK_LABELED
static CHAR os2version[50] ;

char *
get_os2_vers() {
    APIRET rc ;
#ifdef NT
    OSVERSIONINFO verinfo ;
    verinfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);

    rc = !GetVersionEx( &verinfo ) ;
    if ( rc ) {
        os2version[0] = '\0';
    } else {
        sprintf(os2version,"%s %02d.%02d.%02d",
                verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT ? "WinNT" :
                verinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ?
                 (verinfo.dwMinorVersion == 0 ? "Win95" : "Win98")  :
                verinfo.dwPlatformId == VER_PLATFORM_WIN32_WIN32S ? "Win32s" :
                "unknown",
                verinfo.dwMajorVersion,
                verinfo.dwMinorVersion,
                verinfo.dwBuildNumber );
    }
#else /* NT */
    ULONG StartIndex = QSV_VERSION_MAJOR ; /* Major Version Number */
    ULONG EndIndex   = QSV_VERSION_REVISION ; /* Revision Letter      */
    ULONG DataBuf[3] ;
    ULONG DataBufLen = 3 * sizeof(ULONG) ;

    rc = DosQuerySysInfo( StartIndex,
                          EndIndex,
                          DataBuf,
                          DataBufLen );

    if (rc)
      os2version[0] = '\0' ;
    else
      sprintf(os2version,"%02d.%02d%c",DataBuf[0],DataBuf[1],DataBuf[2]) ;
#endif /* NT */
    return os2version ;
}
#endif /* CK_LABELED */

#ifdef CK_REXX
extern char * mrval[] ;
extern int maclvl ;

/* This is the CkCommand/CKermit function handler.  It is an undocumented  */
/* Kermit feature.  Do not remove this code.                             */

ULONG APIENTRY
os2rexxckcmd(
    PUCHAR Name,
    ULONG Argc,
    PRXSTRING Argv,
    PSZ Queuename,
    PRXSTRING Retstr) {
    int rc = 0 ;
    int i ;

    for ( i = 0 ; i < Argc ; i++ ) {
        rc = domac("_rexx_commands",RXSTRPTR(Argv[i]),CF_REXX);
        debug(F111,"os2rexxckcmd",RXSTRPTR(Argv[i]),rc);
        delmac("_rexx_commands",0);
        if (rc < 0) break ;
    }
    if ( mrval[maclvl+1] ) {
       MAKERXSTRING(*Retstr,
                    strdup(mrval[maclvl+1]),
                    strlen(mrval[maclvl+1])
                    ) ;
    } else
       MAKERXSTRING( *Retstr, strdup(""), 0 ) ;

    return (rc < 0 ? rc : 0 ) ;
}

ULONG APIENTRY
os2rexxsubcom(
    PRXSTRING Command,
    PUSHORT pFlags,
    PRXSTRING Retstr)
{
   int rc = 0 ;

   rc = domac("_rexx_commands",RXSTRPTR(Command[0]),CF_REXX);
   debug(F111,"os2rexxsubcom",RXSTRPTR(Command[0]),rc);
   delmac("_rexx_commands",0);

   *pFlags = rc < 0 ? RXSUBCOM_ERROR : RXSUBCOM_OK;

   if ( mrval[maclvl+1] ) {
      MAKERXSTRING(*Retstr,
      strdup(mrval[maclvl+1]),
      strlen(mrval[maclvl+1])) ;
   } else
      MAKERXSTRING( *Retstr, strdup(""), 0 ) ;
   return 0;
}

int
os2rexx( char * rexxcmd, char * rexxbuf, int rexxbuflen ) {
    long return_code  ;  /* rexx interpreter return code */
    short     rc      ;  /* converted return code   */
    char      return_buffer[256] ; /*returned buffer*/
    RXSTRING  Instore[2] ; /* Instorage rexx procedure */
    RXSTRING  retstr  ;  /* program return value    */
    int       retval  ;  /* os2rexx return value    */

    MAKERXSTRING( Instore[0], rexxcmd, strlen(rexxcmd) ) ;
    MAKERXSTRING( Instore[1], 0, 0 ) ;
    MAKERXSTRING( retstr, return_buffer, sizeof(return_buffer) ) ;

    debug(F110,"os2rexx: procedure",rexxcmd,0);
    return_code = RexxStart( 0,   /* no program arguments */
                             0,   /* null argument list   */
                            "Kermit for OS/2 REXX Command",
                                               /* default program name */
                            Instore, /* rexx procedure to interpret */
                            "CKermit",         /* default address name */
                            RXFUNCTION,         /* calling as a function */
                            0,                  /* no exits used */
                            &rc,                /* converted return code */
                            &retstr);           /* returned result */

    debug(F111,"os2rexx: returns",RXSTRPTR(retstr),return_code);
    if ( !return_code && RXSTRLEN( retstr ) < rexxbuflen ) {
        ckstrncpy( rexxbuf, RXSTRPTR(retstr), RXSTRLEN( retstr ) );
        retval = 0 ;                    /* Success */
    } else {
        rexxbuf[0] = '\0' ;
        retval = 1 ;                    /* Failure */
    }
    if (RXSTRPTR(retstr) != return_buffer)
      DosFreeMem(RXSTRPTR(retstr));

    return retval ;
}
int
os2rexxinit()
{
   /* this next line installs the CkCommand statement into the Rexx    */
   /* interpretter environment.  We have replaced it with a subcommand */
   /* handler instead.  Both mechanisms can co-exist, so we leave in   */
   /* the CkCommand/CKermit as an undocumented function.               */

   RexxRegisterFunctionExe("CKermit",(PFN)os2rexxckcmd) ;
   RexxRegisterFunctionExe("CKCommand",(PFN)os2rexxckcmd) ;
   RexxRegisterSubcomExe("CKermit",(PFN)os2rexxsubcom, NULL);
    return 0 ;
}

#endif /* CK_REXX */

int
os2settitle(char *newtitle, int newpriv ) {
#ifndef NOLOCAL
    extern int StartedFromDialer ;
    extern LONG KermitDialerID ;
#ifndef NT
    HSWITCH hSwitch;
    SWCNTRL swctl;
#endif /* NT */
    static char title[80]="not yet set";
    static int  private = 1;
    char titlebuf[128] ;
    extern enum markmodes markmodeflag[];
    extern bool scrollflag[] ;
    extern int vmode;
    extern int inserver;
    char * videomode = "";

    switch ( markmodeflag[vmode] ) {
    case notmarking:
        if ( scrollflag[vmode] )
            videomode = " [Scrollback]";
        break;
    case inmarkmode:
        videomode = " [Mark Mode]";
        break;
    case marking:
        videomode = " [Marking]";
        break;
    }

    if ( newtitle ) {
        ckstrncpy(title,newtitle,80);
        private = newpriv;
    }

    if ( usertitle[0] ) {
        if ( StartedFromDialer ) {
            sprintf( titlebuf, "%d::%s%s%s",KermitDialerID,usertitle,
                 private ? (inserver ? " - IKS" : " - C-Kermit") : "",
                     videomode
                 );
        }
        else {
            sprintf( titlebuf, "%s%s%s",usertitle,
                 private ? (inserver ? " - IKS" : " - C-Kermit") : "", videomode
                 );
        }
    }
    else if ( StartedFromDialer ) {
        sprintf( titlebuf, "%d::%s%s%s%s",KermitDialerID,title,(*title&&private)?" - ":"",
                 private ? (inserver ? "IKS" : "C-Kermit") :  "", videomode
                 );
    }
    else {
        sprintf( titlebuf, "%s%s%s%s",title,(*title&&private)?" - ":"",
                 private ? (inserver ? "IKS" : "C-Kermit") : "" , videomode
                 );
    }

#ifdef NT
#ifdef KUI
    KuiSetProperty(KUI_TITLE,(long)titlebuf,(long)0) ;
    return 1;
#else /* KUI */
    return !SetConsoleTitle(titlebuf);
#endif /* KUI */
#else /* NT */
    /* This changes the text in the task list. */
    /* That the window handle (first parameter) in the WinQuerySwitchHandle */
    /* call can be NULL is fully documented in the API description. */

    hSwitch = WinQuerySwitchHandle((HWND) NULL, pid);
    WinQuerySwitchEntry(hSwitch, &swctl);
    strcpy(swctl.szSwtitle, titlebuf);
    WinChangeSwitchEntry(hSwitch, &swctl);
#ifdef CK_SETTITLE
    {
        /* and this undocumented call changes the session title */
        extern APIRET16 APIENTRY16 Win16SetTitle(HWND hwnd, PSZ szTitle);
        extern APIRET16 APIENTRY16 Win16SetTitleAndIcon(HWND hwnd, PSZ szTitle,
        PSZ szIconFile );
        Win16SetTitle( 0, titlebuf ) ;
/*     Win16SetTitleAndIcon( 0, title, "ckermit.ico" ) ; */
    }
#else /* CK_SETTITLE */
    zsyscmd( "exit" ) ;
#endif /* CK_SETTITLE */
    return 0;
#endif /* NT */
#else /* NOLOCAL */
    return(0);
#endif /* NOLOCAL */
}

int
os2gettitle(char *buffer, int size) {
#ifndef NOLOCAL
    extern enum markmodes markmodeflag[];
    extern bool scrollflag[] ;
    extern int vmode;
    int len;
#ifdef OS2ONLY
    HSWITCH hSwitch;
    SWCNTRL swctl;
#endif /* OS2ONLY */

    if ( buffer == NULL )
        return(-1);
    buffer[0] = '\0';

#ifdef NT
    if ( GetConsoleTitle(buffer, size) ) {
        len = strlen(buffer);

        switch ( markmodeflag[vmode] ) {
        case notmarking:
            if ( scrollflag[vmode] && len >= 13)
                buffer[len-13] = '\0';
            break;
        case inmarkmode:
            if ( len >= 12 )
                buffer[len-12] = '\0';
            break;
        case marking:
            if ( len >= 10 )
                buffer[len-10] = '\0';
            break;
        }
        return(0);
    }
    else return GetLastError();
#else /* NT */

    /* Query the text in the task list */

    hSwitch = WinQuerySwitchHandle( (HWND) NULL, pid);
    WinQuerySwitchEntry(hSwitch, &swctl);
    ckstrncpy(buffer, swctl.szSwtitle, size);

#ifdef OS2PM
    /* Only documented for PM applications */
    WinQuerySessionTitle(hab, 0, buffer, size);
#endif /* OS2PM */

    len = strlen(buffer);

    switch ( markmodeflag[vmode] ) {
    case notmarking:
        if ( scrollflag[vmode] && len >= 13)
            buffer[len-13] = '\0';
        break;
    case inmarkmode:
        if ( len >= 12 )
            buffer[len-12] = '\0';
        break;
    case marking:
        if ( len >= 10 )
            buffer[len-10] = '\0';
        break;
    }
    return 0;
#endif /* NT */
#else /* NOLOCAL */
    return(-1);
#endif /* NOLOCAL */
}


#ifndef NT
HFILE ReadPipe = (HFILE) 0;
APIRET
ConnectToPM( void ) {
    APIRET rc = 0 ;
    UCHAR PipeName[256] = {""};
    ULONG ulOpenMode = NP_ACCESS_INBOUND | NP_NOINHERIT,
    ulPipeMode = NP_WMESG | NP_RMESG | NP_NOWAIT | 0x01,
    ulOutBufSize = 1024,
    ulInpBufSize = 1024,
    ulTimeout = 2000L,
    ulBytesDone = 0 ;


    if ( StartedFromDialer ) {
        sprintf( PipeName, "\\PIPE\\K2DIAL\\%d",KermitDialerID );
        rc = DosCreateNPipe(PipeName, &ReadPipe, ulOpenMode, ulPipeMode,
                             ulOutBufSize, ulInpBufSize, ulTimeout);
        debug(F111,"DosCreateNPipe",PipeName,rc);
        if ( !rc ) {
            rc = DosConnectNPipe( ReadPipe );
            debug(F111,"DosConnectNPipe",PipeName,rc);
            DialerSend( OPT_KERMIT_HWND, (ULONG) 0 ) ;
        }

    }
    return rc;
}

APIRET
ReadFromPM( char * Buf, ULONG len, ULONG * BytesRead ) {
    APIRET rc ;
    AVAILDATA availdata = {0,0};
    ULONG  State = 0;
    static query = 0 ;

    if ( !StartedFromDialer || !ReadPipe || !Buf || !len || !BytesRead )
        return -1 ;

    query = (query + 1)%20 ;    /* Only check once per second or there abouts */
    if ( query )
        return -1;

    rc = DosPeekNPipe( ReadPipe, Buf, 1, BytesRead,
                       &availdata, &State ) ;
    switch ( rc ) {
    case ERROR_BAD_PIPE:
        StartedFromDialer = 0 ;
        DisconnectFromPM() ;
        return -1;
    case NO_ERROR:
        if ( State == NP_STATE_CONNECTED && availdata.cbmessage )
            break;
    default:
        return -1;
    }

    *Buf = '\0' ;
    *BytesRead = 0 ;
    rc = DosRead ( ReadPipe, Buf, len, BytesRead ) ;
    debug(F111,"ReadFromPM",Buf,rc);
    return rc;
}

APIRET
DisconnectFromPM( void ) {
    APIRET rc = 0 ;

    if ( StartedFromDialer ) {
        rc = DosDisConnectNPipe( ReadPipe ) ;
        rc = DosClose( ReadPipe );
        ReadPipe = 0L ;
    }
    return rc;
}
#endif /* OS2PM */

long
getmsec( void ) {
#ifdef NT
    SYSTEMTIME LocalTime ;

    GetLocalTime(&LocalTime);
    return (LocalTime.wMilliseconds +
        1000 * (LocalTime.wSecond +
                 (60 * LocalTime.wMinute +
                   (60 * LocalTime.wHour +
                     (24 * LocalTime.wDay)))));
#else
    return 0;
#endif
}

#ifdef NT
void
DisplayCommProperties(HANDLE h)
{
    COMMPROP *     lpCommProp = NULL;
    LPMODEMDEVCAPS lpModemDevCaps = NULL;
    int rc=0;

    /* leave enough room for provider specific information */
    lpCommProp = (COMMPROP *) malloc( 1024 );
    if ( lpCommProp == NULL )
        return;
    memset( lpCommProp, 0, 1024 );
    lpCommProp->wPacketLength = 1024;
    lpCommProp->dwProvSpec1 = COMMPROP_INITIALIZED;

    rc = GetCommProperties( h, lpCommProp );
    if ( !rc ) {
        debug(F111,"GetCommProperties","GetLastError",GetLastError());
        free(lpCommProp);
        return ;
    }

    printf("GetCommProperties:\n");
    printf("  PacketLength       = %d (bytes)\n",lpCommProp->wPacketLength);
    printf("  PacketVersion      = %x\n",lpCommProp->wPacketVersion);
    printf("  Services Implemented:\n");
    if ( lpCommProp->dwServiceMask & SP_SERIALCOMM )
        printf("    Serial Communication\n");
    if ( lpCommProp->dwServiceMask & SP_PARITY )
        printf("    Parity\n");
    if ( lpCommProp->dwServiceMask & SP_BAUD )
        printf("    Baud\n");
    if ( lpCommProp->dwServiceMask & SP_DATABITS )
        printf("    DataBits\n");
    if ( lpCommProp->dwServiceMask & SP_STOPBITS )
        printf("    StopBits\n");
    if ( lpCommProp->dwServiceMask & SP_HANDSHAKING )
        printf("    Handshaking\n");
    if ( lpCommProp->dwServiceMask & SP_PARITY_CHECK )
        printf("    Parity Check\n");
    if ( lpCommProp->dwServiceMask & SP_RLSD )
        printf("    Carrier Detect\n");
    printf("  Max Tx Queue       = %d (bytes) [0 - unlimited]\n",lpCommProp->dwMaxTxQueue);
    printf("  Max Rx Queue       = %d (bytes) [0 - unlimited]\n",lpCommProp->dwMaxRxQueue);
    printf("  Max Baud Rate      = ");
    switch ( lpCommProp->dwMaxBaud ) {
    case BAUD_075:   printf("75 bps    \n"); break;
    case BAUD_110:   printf("110 bps   \n"); break;
    case BAUD_134_5: printf("134.5 bps \n"); break;
    case BAUD_150:   printf("150 bps   \n"); break;
    case BAUD_300:   printf("300 bps   \n"); break;
    case BAUD_600:   printf("600 bps   \n"); break;
    case BAUD_1200:  printf("1200 bps  \n"); break;
    case BAUD_1800:  printf("1800 bps  \n"); break;
    case BAUD_2400:  printf("2400 bps  \n"); break;
    case BAUD_4800:  printf("4800 bps  \n"); break;
    case BAUD_7200:  printf("7200 bps  \n"); break;
    case BAUD_9600:  printf("9600 bps  \n"); break;
    case BAUD_14400: printf("14400 bps \n"); break;
    case BAUD_19200: printf("19200 bps \n"); break;
    case BAUD_38400: printf("38400 bps \n"); break;
    case BAUD_56K:   printf("56K bps   \n"); break;
    case BAUD_57600: printf("57600 bps \n"); break;
    case BAUD_115200:printf("115200 bps\n"); break;
    case BAUD_128K:  printf("128K bps  \n"); break;
    case BAUD_USER:  printf("Programmable baud rates available\n"); break;
    default:         printf("(unknown)\n"); break;
    }
    printf("  Provider Sub Type  = ");
    switch ( lpCommProp->dwProvSubType ) {
    case PST_UNSPECIFIED     :
        printf("Unspecified");
        break;
    case PST_RS232           :
        printf("RS232");
        break;
    case PST_PARALLELPORT    :
        printf("Parallel Port");
        break;
    case PST_RS422           :
        printf("RS422");
        break;
    case PST_RS423           :
        printf("RS423");
        break;
    case PST_RS449           :
        printf("RS449");
        break;
    case PST_MODEM           :
        printf("Modem");
        break;
    case PST_FAX             :
        printf("Fax");
        break;
    case PST_SCANNER         :
        printf("Scanner");
    case PST_NETWORK_BRIDGE  :
        printf("Network Bridge");
        break;
    case PST_LAT             :
        printf("LAT");
        break;
    case PST_TCPIP_TELNET    :
        printf("TCP/IP Telnet");
        break;
    case PST_X25             :
        printf("X.25");
        break;
    default:
        printf("Unknown");
    }
    printf("\n");
    printf("  Provider Capabilities:\n");
    if ( lpCommProp->dwProvCapabilities & PCF_16BITMODE )
        printf("    Special 16 Bit Mode supported\n");
    if ( lpCommProp->dwProvCapabilities & PCF_DTRDSR )
        printf("    DTR (data-terminal-ready)/DSR (data-set-ready) supported\n");
    if ( lpCommProp->dwProvCapabilities & PCF_INTTIMEOUTS )
        printf("    Interval time-outs supported\n");
    if ( lpCommProp->dwProvCapabilities & PCF_PARITY_CHECK )
        printf("    Parity checking supported\n");
    if ( lpCommProp->dwProvCapabilities & PCF_RLSD )
        printf("    RLSD (receive-line-signal-detect) supported\n");
    if ( lpCommProp->dwProvCapabilities & PCF_RTSCTS )
        printf("    RTS (request-to-send)/CTS (clear-to-send) supported\n");
    if ( lpCommProp->dwProvCapabilities & PCF_SETXCHAR )
        printf("    Settable XON/XOFF supported\n");
    if ( lpCommProp->dwProvCapabilities & PCF_SPECIALCHARS )
        printf("    Special character support provided\n");
    if ( lpCommProp->dwProvCapabilities & PCF_TOTALTIMEOUTS )
        printf("    Total (elapsed) time-outs supported\n");
    if ( lpCommProp->dwProvCapabilities & PCF_XONXOFF )
        printf("    XON/XOFF flow control supported\n");
    printf("  Settable Parameters:\n");
    if ( lpCommProp->dwSettableParams & SP_BAUD )
        printf("    Baud Rate\n");
    if ( lpCommProp->dwSettableParams & SP_DATABITS )
        printf("    DataBits\n");
    if ( lpCommProp->dwSettableParams & SP_HANDSHAKING )
        printf("    Handshaking (flow control)\n");
    if ( lpCommProp->dwSettableParams & SP_PARITY )
        printf("    Parity\n");
    if ( lpCommProp->dwSettableParams & SP_PARITY_CHECK )
        printf("    Parity checking\n");
    if ( lpCommProp->dwSettableParams & SP_RLSD )
        printf("    RLSD (receive-line-signal-detect)\n");
    if ( lpCommProp->dwSettableParams & SP_STOPBITS )
        printf("    StopBits\n");
#ifdef COMMENT
    DWORD dwSettableBaud;      // allowable baud rates
    WORD  wSettableData;       // allowable byte sizes
    WORD  wSettableStopParity; // stop bits/parity allowed
#endif
    printf("  Current Tx Queue   = %d (bytes)\n",lpCommProp->dwCurrentTxQueue);
    printf("  Current Rx Queue   = %d (bytes)\n",lpCommProp->dwCurrentRxQueue);

    if ( lpCommProp->dwProvSubType == PST_MODEM && lpCommProp->wcProvChar[0]) {
        lpModemDevCaps = (LPMODEMDEVCAPS) lpCommProp->wcProvChar;
        printf("Modem Device Capabilities:\n");
        printf("  Modem Provider Version = %x\n",lpModemDevCaps->dwModemProviderVersion);
        if ( isWin95() ) {
        printf("  Modem Manufacturer     = %s\n",
                lpModemDevCaps->dwModemManufacturerOffset ?
                (LPCSTR) lpModemDevCaps + lpModemDevCaps->dwModemManufacturerOffset :
                "(not specified)");
        printf("  Modem Model            = %s\n",
                lpModemDevCaps->dwModemModelOffset ?
                (LPCSTR) lpModemDevCaps + lpModemDevCaps->dwModemModelOffset :
                "(not specified)");
        printf("  Modem Version          = %s\n",
                lpModemDevCaps->dwModemVersionOffset ?
                (LPCSTR) lpModemDevCaps + lpModemDevCaps->dwModemVersionOffset :
                "(not specified)");
        }
        else {
            char strbuf[256];
            WORD * word;
            int    i,len;
            if ( lpModemDevCaps->dwModemManufacturerOffset ) {
                len = lpModemDevCaps->dwModemManufacturerSize;
                word = (WORD *)((LPCSTR) lpModemDevCaps +
                                 lpModemDevCaps->dwModemManufacturerOffset);
                for ( i=0 ; i < len ; i++ )
                    strbuf[i] = (char) word[i];
                strbuf[i] = '\0';
                printf("  Modem Manufacturer     = %s\n",strbuf);
            }
            else
                printf("  Modem Manufacturer     = (not specified)\n");

            if ( lpModemDevCaps->dwModemModelOffset ) {
                len = lpModemDevCaps->dwModemModelSize;
                word = (WORD *)((LPCSTR) lpModemDevCaps +
                                 lpModemDevCaps->dwModemModelOffset);
                for ( i=0 ; i < len ; i++ )
                    strbuf[i] = (char) word[i];
                strbuf[i] = '\0';
                printf("  Modem Model            = %s\n",strbuf);
            }
            else
                printf("  Modem Model            = (not specified)\n");

            if ( lpModemDevCaps->dwModemVersionOffset ) {
                len = lpModemDevCaps->dwModemVersionSize;
                word = (WORD *)((LPCSTR) lpModemDevCaps +
                                 lpModemDevCaps->dwModemVersionOffset);
                for ( i=0 ; i < len ; i++ )
                    strbuf[i] = (char) word[i];
                strbuf[i] = '\0';
                printf("  Modem Version          = %s\n",strbuf);
            }
            else
                printf("  Modem Version          = (not specified)\n");
        }
        printf("  Dial Options:\n");
        if ( lpModemDevCaps->dwDialOptions & DIALOPTION_BILLING )
            printf("    Specifies that the modem supports waiting for billing tone (bong).\n");
        if ( lpModemDevCaps->dwDialOptions & DIALOPTION_QUIET )
            printf("    Specifies that the modem supports waiting for quiet.\n");
        if ( lpModemDevCaps->dwDialOptions & DIALOPTION_DIALTONE )
            printf("    Specifies that the modem supports waiting for a dial tone.\n");
        printf("  Max CallSetupFailTimer = %d (seconds)\n",
                lpModemDevCaps->dwCallSetupFailTimer);
        printf("  Max Inactivity Timeout = %d (1/10 seconds)\n",
                lpModemDevCaps->dwInactivityTimeout);
        printf("  Speaker Volumes:\n");
        if ( lpModemDevCaps->dwSpeakerVolume & MDMVOLFLAG_LOW )
            printf("    Low\n");
        if ( lpModemDevCaps->dwSpeakerVolume & MDMVOLFLAG_MEDIUM )
            printf("    Medium\n");
        if ( lpModemDevCaps->dwSpeakerVolume & MDMVOLFLAG_HIGH )
            printf("    High\n");
        printf("  Speaker Modes:\n");
        if ( lpModemDevCaps->dwSpeakerVolume & MDMSPKRFLAG_OFF )
            printf("    Off\n");
        if ( lpModemDevCaps->dwSpeakerVolume & MDMSPKRFLAG_DIAL )
            printf("    Dial\n");
        if ( lpModemDevCaps->dwSpeakerVolume & MDMSPKRFLAG_ON )
            printf("    On\n");
        if ( lpModemDevCaps->dwSpeakerVolume & MDMSPKRFLAG_CALLSETUP )
            printf("    Call Setup\n");
        printf("  Modem Options:\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_BLIND_DIAL )
            printf("    Blind Dial\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_FLOWCONTROL_SOFT )
            printf("    Software Flow Control\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_CCITT_OVERRIDE )
            printf("    CCITT modulation override\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_FORCED_EC )
            printf("    Forced Error Correction\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_CELLULAR )
            printf("    Cellular\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_SPEED_ADJUST )
            printf("    Speed Adjustment\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_COMPRESSION )
            printf("    Compression\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_TONE_DIAL )
            printf("    Tone Dial\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_ERROR_CONTROL )
            printf("    Error Control\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_V23_OVERRIDE )
            printf("    V.23 override\n");
        if ( lpModemDevCaps->dwModemOptions & MDM_FLOWCONTROL_HARD )
            printf("    Hardware flow control\n");
        printf("  Max DTE Rate           = %d (bits/second)\n",
                lpModemDevCaps->dwMaxDTERate);
        printf("  Max DCE Rate           = %d (bits/second)\n",
                lpModemDevCaps->dwMaxDCERate);
    }
    printf("\n");
    free(lpCommProp);
    return;
}
#endif /* NT */

#ifdef BPRINT
extern int  printbidi;                  /* SET BPRINTER (bidirectional) */
extern long pportspeed;                 /* Bidirection printer port speed, */
extern int  pportparity;                /*  parity, */
extern int  pportflow;                  /*  and flow control */
extern char *printername;
#ifdef NT
TID         tidBPrint=NULL;
#else
TID         tidBPrint=0;
#endif /* NT */

#ifdef NT
HANDLE hBPrinter = NULL;
#else /* NT */
HFILE  hBPrinter = 0;
#endif /* NT */

#ifdef NT
int
prtcfg(HANDLE hPrt) {
    DCB dcbBPrint;
    COMMTIMEOUTS timeouts ;

    if ( pportflow == FLO_KEEP && !pportspeed && !pportparity )
        return(TRUE);

    SetCommMask( (HANDLE) hPrt, EV_RXCHAR ) ;
    SetupComm( (HANDLE) hPrt, 20000, 20000 ) ;
    PurgeComm( (HANDLE) hPrt,
               PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR ) ;

    dcbBPrint.DCBlength = sizeof(DCB);
    if (!GetCommState( (HANDLE) hPrt, &dcbBPrint )) {
        return FALSE ;
    }

    if (!GetCommTimeouts(hPrt, &timeouts)) {
        return FALSE ;
    }

    if ( deblog )
        debugComm( "prtcfg initial values", &dcbBPrint, &timeouts );

    dcbBPrint.fBinary = TRUE ;
    dcbBPrint.fErrorChar = FALSE ;
    dcbBPrint.fAbortOnError = FALSE ;
    dcbBPrint.ErrorChar = '?' ;
    dcbBPrint.fParity = TRUE ;
    dcbBPrint.StopBits = ONESTOPBIT ;
    dcbBPrint.BaudRate = pportspeed;
    dcbBPrint.fDsrSensitivity = FALSE ;
    dcbBPrint.fDtrControl = DTR_CONTROL_ENABLE ;

    switch (pportparity) {
        case 'o':
            dcbBPrint.ByteSize = 7;     /* Data bits */
            dcbBPrint.Parity   = 1;
            break;
        case 'e':
            dcbBPrint.ByteSize = 7;     /* Data bits */
            dcbBPrint.Parity   = 2;
            break;
        case 'm':
            dcbBPrint.ByteSize = 7;     /* Data bits */
            dcbBPrint.Parity   = 3;
            break;
        case 's':
            dcbBPrint.ByteSize = 7;     /* Data bits */
            dcbBPrint.Parity   = 4;
            break;
        default :
            dcbBPrint.ByteSize = 8;     /* Data bits */
            dcbBPrint.Parity   = 0;     /* No parity */
    }
    switch(pportflow) {
    case FLO_XONX:
        dcbBPrint.fOutX = TRUE ;
        dcbBPrint.fInX = TRUE ;
        dcbBPrint.fRtsControl = RTS_CONTROL_ENABLE ;
        dcbBPrint.fOutxCtsFlow = FALSE ;
        dcbBPrint.fTXContinueOnXoff = /* TRUE */ FALSE ;
        break;
    case FLO_RTSC:
        dcbBPrint.fOutX = FALSE ;
        dcbBPrint.fInX = FALSE ;
        dcbBPrint.fRtsControl = RTS_CONTROL_HANDSHAKE ;
        dcbBPrint.fOutxCtsFlow = TRUE ;
        dcbBPrint.fTXContinueOnXoff = TRUE ;
        break;
    case FLO_KEEP:
        /* leave things exactly as they are */
        break;
    case FLO_NONE:
        /* turn off all flow control completely */
        dcbBPrint.fOutX = FALSE ;
        dcbBPrint.fInX = FALSE ;
        dcbBPrint.fRtsControl = RTS_CONTROL_ENABLE ;
        dcbBPrint.fOutxCtsFlow = FALSE ;
        dcbBPrint.fTXContinueOnXoff = TRUE ;
        break;
    }

    dcbBPrint.XonChar = 0x11 ;
    dcbBPrint.XoffChar = 0x13;
    dcbBPrint.XonLim = 10 ;
    dcbBPrint.XoffLim = 10 ;

    if (!SetCommState( hPrt, &dcbBPrint )) {
        return FALSE ;
    }

    /* Timeouts for Overlapped I/O from MS TTY.C example */
    timeouts.ReadIntervalTimeout = MAXDWORD ;
    timeouts.ReadTotalTimeoutMultiplier = MAXDWORD ;
    timeouts.ReadTotalTimeoutConstant = 10 ;
    timeouts.WriteTotalTimeoutMultiplier = 1000 ;
    timeouts.WriteTotalTimeoutConstant   = 1000 ;

    if (!SetCommTimeouts( hPrt, &timeouts )) {
        return FALSE ;
    }
    return(TRUE);
}
#endif /* NT */

int
bprtstart( void )
{
#ifdef NT
    SECURITY_ATTRIBUTES security ;

    if ( hBPrinter )
        return TRUE;

    memset(&security, 0, sizeof(SECURITY_ATTRIBUTES));
    security.nLength = sizeof(SECURITY_ATTRIBUTES);
    security.lpSecurityDescriptor = NULL ;
    security.bInheritHandle = FALSE ;

    if ((hBPrinter = CreateFile( printername ? printername : "PRN",
                            GENERIC_READ | GENERIC_WRITE,
                            FALSE, /* TRUE, /* do not share */
                            &security,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL)) == INVALID_HANDLE_VALUE) {
        hBPrinter = NULL;
        return FALSE;
    }

    if ( !prtcfg(hBPrinter) ) {
        CloseHandle( hBPrinter );
        hBPrinter = NULL;
        return FALSE;
    }

    /* The Port should now be configured, so lets start the thread */
    tidBPrint = (TID) ckThreadBegin( &bprtthread,THRDSTKSIZ,0,FALSE,0);
    if ( !tidBPrint )
    {
        CloseHandle( hBPrinter );
        hBPrinter = NULL;
        return FALSE ;
    }
    return TRUE;
#else /* NT */
    DCBINFO dcbBPrint;
    LINECONTROL lc;
    UINT parm = 0, data = 0;
    ULONG action = 0;
    struct {
      long rate;
      char fract;
    } speed;
    ULONG   error;

    if (error = DosOpen(printername ? printername : "PRN",
                        (PHFILE)&hBPrinter,&action,0L,0,FILE_OPEN,
        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_FAIL_ON_ERROR,
                       0L)) {
        hBPrinter = 0;
        return FALSE;
    }

    DosDevIOCtl(&data,sizeof(data),&parm,sizeof(parm),
                DEV_FLUSHINPUT,IOCTL_GENERAL,hBPrinter); /* Flush driver */

    /* Get the current settings */
    if (DosDevIOCtl(&dcbBPrint,sizeof(dcbBPrint),NULL,0,
                     ASYNC_GETDCBINFO,IOCTL_ASYNC,hBPrinter))
    {
        DosClose(hBPrinter);
        hBPrinter = 0;
        return FALSE;
    }

    dcbBPrint.fbCtlHndShake = MODE_DTR_CONTROL;
    dcbBPrint.fbFlowReplace &= ~(MODE_AUTO_RECEIVE | MODE_AUTO_TRANSMIT |
    /* clear only a few */    MODE_RTS_CONTROL  | MODE_RTS_HANDSHAKE);

    if (pportflow == FLO_XONX) {
        dcbBPrint.fbFlowReplace |=
            (MODE_AUTO_RECEIVE | MODE_AUTO_TRANSMIT | MODE_RTS_CONTROL);
    }
    else if (pportflow == FLO_RTSC) {
        dcbBPrint.fbCtlHndShake |= MODE_CTS_HANDSHAKE;
        dcbBPrint.fbFlowReplace |= MODE_RTS_HANDSHAKE;
    }
    else if ( pportflow != FLO_KEEP ) {
        dcbBPrint.fbFlowReplace |= MODE_RTS_CONTROL;
    }

    /* set write timeout */
    dcbBPrint.fbTimeout &= ~MODE_NO_WRITE_TIMEOUT;
    dcbBPrint.usWriteTimeout = 15 * 100;        /* 15-second timeout */

    /* Read "some" data from line mode */
    dcbBPrint.fbTimeout &= ~MODE_NOWAIT_READ_TIMEOUT;
    dcbBPrint.fbTimeout |= MODE_WAIT_READ_TIMEOUT;

    /* Timeouts */
    dcbBPrint.usReadTimeout = 9;        /* Test every  0.1s per call */

    /* Set DCB */
    if (DosDevIOCtl(NULL,0,&dcbBPrint,sizeof(dcbBPrint),
                     ASYNC_SETDCBINFO,IOCTL_ASYNC,hBPrinter))
    {
        DosClose(hBPrinter);
        hBPrinter = 0;
        return FALSE;
    }

    if (pportflow != FLO_RTSC && pportflow != FLO_KEEP) {/* keep RTS permanently on */
        MODEMSTATUS ms;
        UINT data;
        ms.fbModemOn = RTS_ON;
        ms.fbModemOff = 255;
        DosDevIOCtl(&data,sizeof(data),&ms,sizeof(ms),
                    ASYNC_SETMODEMCTRL,IOCTL_ASYNC,hBPrinter);
    }

    if (DosDevIOCtl(&lc,sizeof(lc),NULL,0,
                    ASYNC_GETLINECTRL,IOCTL_ASYNC,hBPrinter))
    {
        DosClose(hBPrinter);
        hBPrinter = 0;
        return FALSE;
    }

    switch (pportparity) {
        case 'o':
            lc.bDataBits = 7;   /* Data bits */
            lc.bParity   = 1;
            break;
        case 'e':
            lc.bDataBits = 7;   /* Data bits */
            lc.bParity   = 2;
            break;
        case 'm':
            lc.bDataBits = 7;   /* Data bits */
            lc.bParity   = 3;
            break;
        case 's':
            lc.bDataBits = 7;   /* Data bits */
            lc.bParity   = 4;
            break;
        default :
            lc.bDataBits = 8;   /* Data bits */
            lc.bParity   = 0;   /* No parity */
    }
    switch (1) {
        case 2:
            lc.bStopBits = 2;   /* Two stop bits */
            break;
        case 1:
            lc.bStopBits = 0;   /* One stop bit */
            break;
        default:                /* No change */
            break;
    }
    if (DosDevIOCtl(NULL,0,&lc,sizeof(lc),
                    ASYNC_SETLINECTRL,IOCTL_ASYNC,hBPrinter))
    {
        DosClose(hBPrinter);
        hBPrinter = 0;
        return FALSE;
    }

    if (pportspeed > 65535L) {
      speed.rate = pportspeed;
      speed.fract = 0;
      if (DosDevIOCtl(NULL,0,&speed,sizeof(speed),0x0043,IOCTL_ASYNC,hBPrinter))
      {
          DosClose(hBPrinter);
          hBPrinter = 0;
          return FALSE;
      }

    } else if (DosDevIOCtl(NULL,0,&pportspeed,sizeof(pportspeed),
                         ASYNC_SETBAUDRATE,IOCTL_ASYNC,hBPrinter))
    {
        DosClose(hBPrinter);
        hBPrinter = 0;
        return FALSE;
    }

    /* Still must start the thread */
    tidBPrint = (TID) ckThreadBegin( &bprtthread,THRDSTKSIZ,0,FALSE,0);
    if ( !tidBPrint )
    {
        DosClose( hBPrinter );
        hBPrinter = 0;
        return FALSE ;
    }

    return TRUE;
#endif /* NT */
}

int
bprtstop( void )
{
#ifdef NT
    if ( hBPrinter == NULL )
        return TRUE;

    CloseHandle( hBPrinter );
    hBPrinter = NULL;
    msleep( 500 );
    CloseHandle( tidBPrint );
    tidBPrint = NULL;
    return TRUE;
#else /* NT */
    if ( !hBPrinter )
        return TRUE;

    DosClose(hBPrinter);
    hBPrinter = 0;
    DosClose(tidBPrint);
    tidBPrint = 0;
    msleep(500);
    return TRUE;
#endif /* NT */
}

void
bprtthread( void * dummy )
{
    CHAR  buf[64];
#ifdef NT
    DWORD read;
#else
    ULONG read;
    ULONG Nesting;
#endif

#ifdef NT
    while ( hBPrinter ) {
#ifdef COMMENT
        if ( WaitForSingleObject( hBPrinter, -1L ) != WAIT_OBJECT_0 ) {
            debug(F111,"BPrinter WaitForSingleObject",
                   "GetLastError",GetLastError());
            continue;
        }
#endif

        if ( ReadFile( hBPrinter, buf, 64, &read, NULL ) )
        {
            if ( read ) {
                buf[read] = '\0';
                debug(F111,"BPrinter Read",buf,read);
                ttxout( buf, read );
            }
            else
                msleep(100);
        }
        else {
            DWORD error = GetLastError();
            debug(F111,"BPrinter ReadFile","GetLastError",error);
            switch ( error ) {
            default:
                break;
            }
            msleep(100);
        }
    }
#else /* NT */
    while ( hBPrinter ) {
        ULONG error;
        DosEnterMustComplete( &Nesting ) ;
        if ( !(error =DosRead(hBPrinter,buf,64,&read)) )
        {
            DosExitMustComplete( &Nesting );
            if ( read ) {
                buf[read] = '\0';
                debug(F111,"BPrinter Read",buf,read);
                sendchars( buf, read );
            }
            else
                msleep(100);
        }
        else {
            DosExitMustComplete( &Nesting );
            debug(F111,"BPrinter DosRead","Error",error);
            switch ( error ) {
            default:
                break;
            }
            msleep(100);
        }
    }
#endif /* NT */
}

int
bprtwrite( char * s, int len )
{
#ifdef NT
    DWORD written=0;
    debug(F111,"BPrinter Write", s,len);
    if (!WriteFile( hBPrinter, s, len, &written, NULL )) {
        debug(F111,"BPrinter WriteFile",
               "GetLastError",GetLastError());
        return -1;
    }
    else {
        return written;
    }
#else /* NT */
    UINT written;
    if(DosWrite(hBPrinter,s,len,(PVOID)&written))
        return -1 ;
    else
        return written ;
#endif /* NT */
}
#endif /* BPRINT */

struct tm *
#ifdef CK_ANSIC
cmdate2tm(char * date, int gmt)         /* date as "yyyymmdd hh:mm:ss" */
#else
cmdate2tm(date,gmt) char * date; int gmt;
#endif
{
    /* date as "yyyymmdd hh:mm:ss" */
    static struct tm _tm;
    time_t now;

    if ( strlen(date) != 17 ||
         date[8] != ' ' ||
         date[11] != ':' ||
         date[14] != ':')
        return(NULL);

    time(&now);
    if ( gmt )
        _tm = *gmtime(&now);
    else
        _tm = *localtime(&now);
    _tm.tm_year = (date[0]-'0')*1000 + (date[1]-'0')*100 +
                   (date[2]-'0')*10   + (date[3]-'0')-1900;
    _tm.tm_mon  = (date[4]-'0')*10   + (date[5]-'0')-1;
    _tm.tm_mday = (date[6]-'0')*10   + (date[7]-'0');
    _tm.tm_hour = (date[9]-'0')*10   + (date[10]-'0');
    _tm.tm_min  = (date[12]-'0')*10  + (date[13]-'0');
    _tm.tm_sec  = (date[15]-'0')*10  + (date[16]-'0');

    _tm.tm_wday = 0;
    _tm.tm_yday = 0;

    return(&_tm);
}

/*---------------------------------------------------------------------------*/
/* IsWARPed()                                                                */
/* In OS/2 terms, this means we have support for flexible column widths      */
/*---------------------------------------------------------------------------*/
bool
IsWARPed( void ) {
#ifdef NT
    return TRUE ;
#else
    ULONG StartIndex = QSV_VERSION_MAJOR ; /* Major Version Number */
    ULONG EndIndex   = QSV_VERSION_REVISION ; /* Revision Letter      */
    ULONG DataBuf[3] ;
    ULONG DataBufLen = 3 * sizeof(ULONG) ;
    APIRET rc ;

    DataBuf[0] = DataBuf[1] = DataBuf[2] = 0 ;

    rc = DosQuerySysInfo( StartIndex,
                          EndIndex,
                          DataBuf,
                          DataBufLen );

    return ( DataBuf[0] > 20 ||
             DataBuf[0] == 20 && DataBuf[1] >= 30 ) ;
#endif /* NT */
}



#ifdef COMMENT
WINBASEAPI DWORD WINAPI SetConsoleDisplayMode(HANDLE, DWORD, LPDWORD);
WINBASEAPI BOOL WINAPI  GetConsoleDisplayMode(LPDWORD);

are not declared in any header, they are in KERNEL32.LIB.  So, after
declaring them (as above, for example) you can link with them.  Use:

        SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), 1, &Result);
or
        SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), 0, &Result);

to go to fullscreen or windowed (respectively).  I don't recall what
"Result" holds when you're done, the curent status (post-execution) I think;
a DejaNews search for the function name ought to turn up that info.

 - Vince
#endif /* Undocumented Windows functions */
