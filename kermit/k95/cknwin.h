/* C K N W I N   --  Kermit GUI Windows support for Win32 systems */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com)
          Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

extern HWND hwndGUI ;
extern HWND hwndConsole ;

_PROTOTYP( int WindowThreadInit, (void *) ) ;
_PROTOTYP( void WindowThreadClose, (void) ) ;
_PROTOTYP( HWND GetConsoleHwnd, (void) ) ;


