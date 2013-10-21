/* This example code shows how to start K95 from within another Win32 */
/* application. */

#include <windows.h>

HANDLE
StartK95WithScript( char * script ) {
    PROCESS_INFORMATION K95ProcessInfo ;
    OSVERSIONINFO osverinfo ;
    STARTUPINFO si ;
    HANDLE hOut, hIn ;
    CHAR cmdstr[256];

    /* Initialize the process Startup Information structure */
    memset( &si, 0, sizeof(STARTUPINFO) ) ;
    si.cb = sizeof(STARTUPINFO);

    /* Find out what platform we are running on */
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    
    /* Are we running on Windows 95? */
    if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
	/*
	This really bad hack of code is here only because Windows 95 doesn't do
	what it is supposed to be doing, namely properly constructing the 
	console window for its child process.  Therefore, we must do it 
	manually.  This is only necessary when starting K95 from a GUI
	application.
	*/
	AllocConsole() ;

	/* Get a handle to Standout Out */
	hOut = CreateFile( "CONOUT$", GENERIC_READ | GENERIC_WRITE, 
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  0) ;

	/* Get a handle to Standout In */
	hIn = CreateFile( "CONIN$", GENERIC_READ | GENERIC_WRITE, 
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  0) ;

	/* Setup the StartupInfo structure to use our new handles */
	si.dwFlags = (DWORD) STARTF_USESTDHANDLES ;
	DuplicateHandle( GetCurrentProcess(), hOut, GetCurrentProcess(), 
			 &si.hStdOutput,
                       DUPLICATE_SAME_ACCESS, TRUE, 
			 DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) ;
	si.hStdError = si.hStdOutput ;
	DuplicateHandle( GetCurrentProcess(), hIn, GetCurrentProcess(), 
			 &si.hStdInput,
                       DUPLICATE_SAME_ACCESS, TRUE, 
			 DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) ;
    }

    si.dwFlags |= (DWORD) STARTF_USECOUNTCHARS | 
	STARTF_USEFILLATTRIBUTE |
	STARTF_USESHOWWINDOW;
    si.dwXCountChars = (DWORD) 80;	/* width is 80 cols   */
    si.dwYCountChars = (DWORD) 25;	/* height is 25 lines */
    si.dwFillAttribute = (DWORD) 0x07;	/* white on black */
    si.wShowWindow = SW_SHOWMAXIMIZED ;

    /* wShowWindow may be set to any of these values
       SW_SHOWNORMAL     
       SW_SHOWMINIMIZED  
       SW_SHOWMAXIMIZED  
       SW_SHOWNOACTIVATE 
       SW_SHOW           
       SW_SHOWMINNOACTIVE
       SW_SHOWNA         
       SW_SHOWDEFAULT    
    */

    /* build executable string */
    sprintf( cmdstr, "k95.exe %s", script ) ;

    /* Start K95 */
    if (!CreateProcess((LPSTR)NULL,                    /* start K-95  */
			(LPSTR)cmdstr,                 /* give it the file */
			(LPSECURITY_ATTRIBUTES)NULL,   /* fix if necessary */
			(LPSECURITY_ATTRIBUTES)NULL,   /* fix if necessary */
			FALSE,               	       /* fix if necessary */
			(DWORD) CREATE_NEW_PROCESS_GROUP,    
			(LPVOID)NULL,                  /* fix if necessary */
			(LPSTR)NULL,                   /* Current directory */
			&si,                           /* Startup info, fix */
			&K95ProcessInfo        /* Process info */
			)) 
    {
	/* Error has occurred, call GetLastError() to find out why */

	/* If Windows 95, cleanup the console we created */
	if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
	    FreeConsole();
	}
	return(NULL);
    } else
    {
	/* If Windows 95, cleanup the console we created */
	if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
	    FreeConsole();
	}
	return ((HANDLE) K95ProcessInfo.hProcess);
    }
}
