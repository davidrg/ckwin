/* C K O W I N   --  Kermit GUI Windows support - Generic */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com)
            Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

extern int StartedFromDialer ;
extern HWND hwndDialer ;
extern LONG KermitDialerID ;
#ifndef NT
extern HAB hab;
#endif

_PROTOTYP( void DialerSend, ( UINT, LONG ) ) ;

#define OPT_KERMIT_SUCCESS          12001
#define OPT_KERMIT_FAILURE          12002
#define OPT_KERMIT_CONNECT          12003
#define OPT_KERMIT_HANGUP           12004
#define OPT_KERMIT_HWND             12005
#define OPT_KERMIT_EXIT             12006
#define OPT_KERMIT_FG               12007
#define OPT_KERMIT_PID              12008
#define OPT_KERMIT_HWND2            12009

#define OPT_DIALER_CONNECT          13001
#define OPT_DIALER_HWND             13002
#define OPT_DIALER_EXIT             13003

#define OPT_TAPI_DIAL               14001
#define OPT_TAPI_OPEN               14002
#define OPT_TAPI_INIT               14003
#define OPT_TAPI_SHUTDOWN           14004

#ifdef NT
__inline void
DialerSend(UINT message, LONG lparam)
{
    if ( StartedFromDialer )
        SendMessage( hwndDialer, message, KermitDialerID, lparam ) ;
}
#else
_Inline void
DialerSend(UINT message, LONG lparam)
{
    debug(F111,"DialerSend","message",message);
    debug(F111,"DialerSend","StartedFromDialer",StartedFromDialer);
    if ( StartedFromDialer ) {
        if (!WinPostMsg( hwndDialer, message,
                    MPFROMLONG(KermitDialerID), MPFROMLONG(lparam) ))
            debug(F111,"DialerSend failed","WinGetLastError()",WinGetLastError(hab));
    }
}
#endif

