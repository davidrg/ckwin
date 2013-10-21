/* C K N A L M  --  Kermit alarm functions for Windows 95 systems */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
          Secure Endpoints Inc., New York City.

  Copyright (C) 1996,2004 Trustees of Columbia University in the City of New
  York.
*/

/* We are performing this ugly hack only because Windows 95 can not implement 
   a timer that works accurately on all computer systems with Console
   Applications.  This code should not be used for Windows NT or other Win32
   implementations
*/

UINT
ckTimerStart( UINT interval, UINT precision, LPTIMECALLBACK pTimeProc, DWORD userdata, UINT event ) ;

UINT
ckTimerKill( UINT i ) ;
