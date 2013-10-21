/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

#include <windows.h>
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckucmd.h"

HANDLE hPrinter=NULL;
DWORD  dwJob=0;

int
Win32PrtOpen( char * prtname )
{
    DOC_INFO_1 DocInfo;

    if( ! OpenPrinter( prtname, &hPrinter, NULL ) ) {
        int error = GetLastError();
        debug(F111,"Win32PrtOpen OpenPrinter() failed",prtname,error);
        printf("ERROR: Unable to open printer: %s (%d)\n",
                prtname,error);
        return FALSE;
    }

    // Fill in the structure with info about this "document"
    DocInfo.pDocName = "Kermit-95";
    DocInfo.pOutputFile = NULL;
    DocInfo.pDatatype = "RAW";
    // Inform the spooler the document is beginning
    if( (dwJob = StartDocPrinter( hPrinter, 1, (LPSTR)&DocInfo )) == 0 )
    {
        int error = GetLastError();
        debug(F111,"Win32PrtOpen StartDocPrinter() failed",prtname,error);
        printf("ERROR: Unable to start doc on printer: %s (%d)\n",
                prtname,error);
        ClosePrinter( hPrinter );
        hPrinter = NULL;
        return FALSE;
    }
    // Start a page
    if( ! StartPagePrinter( hPrinter ) )
    {
        int error = GetLastError();
        debug(F111,"Win32PrtOpen StartPagePrinter() failed",prtname,error);
        printf("ERROR: Unable to start page on printer: %s (%d)\n",
                prtname,error);
        EndDocPrinter( hPrinter );
        ClosePrinter( hPrinter );
        hPrinter = NULL;
        return FALSE;
    }
    return TRUE;
}


int
Win32PrtWrite( char * pData, int bytes )
{
    DWORD dwBytesWritten;

    if ( hPrinter == NULL )
        return -1;

    // Send the data to the printer
    if( ! WritePrinter( hPrinter, pData, bytes, &dwBytesWritten ) )
    {
        EndPagePrinter( hPrinter );
        EndDocPrinter( hPrinter );
        ClosePrinter( hPrinter );
        hPrinter = NULL;
        return -2;
    }

    return (int) dwBytesWritten;
}


int
Win32PrtClose( void )
{
    // End the page
    if( ! EndPagePrinter( hPrinter ) )
    {
        EndDocPrinter( hPrinter );
        ClosePrinter( hPrinter );
        hPrinter = NULL;
        return FALSE;
    }
    // Inform the spooler that the document is ending
    if( ! EndDocPrinter( hPrinter ) )
    {
        ClosePrinter( hPrinter );
        hPrinter = NULL;
        return FALSE;
    }
    // Tidy up the printer handle
    ClosePrinter( hPrinter );
    hPrinter = NULL;

    return TRUE;
}

int
Win32PrtFile( char * filename, char * prtname )
{
    HANDLE     hPrinter,hFile;
    SECURITY_ATTRIBUTES security ;
    DOC_INFO_1 DocInfo;
    DWORD      dwBytesWritten;
    CHAR       lpData[4096];
    DWORD      dwCount=0;


    // First try to open the file
    security.nLength = sizeof(SECURITY_ATTRIBUTES);
    security.lpSecurityDescriptor = NULL ;
    security.bInheritHandle = FALSE ;

    hFile = CreateFile( filename,
                        GENERIC_READ,
                        TRUE,
                        &security,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );
    if ( !hFile )
        return FALSE;

    // Need a handle to the printer
    if( ! OpenPrinter( prtname, &hPrinter, NULL ) ) {
        CloseHandle( hFile );
        return FALSE;
    }

    // Fill in the structure with info about this "document"
    DocInfo.pDocName = filename;
    DocInfo.pOutputFile = NULL;
    DocInfo.pDatatype = "RAW";
    // Inform the spooler the document is beginning
    if( (dwJob = StartDocPrinter( hPrinter, 1, (LPSTR)&DocInfo )) == 0 )
    {
        ClosePrinter( hPrinter );
        CloseHandle( hFile );
        return FALSE;
    }
    // Start a page
    if( ! StartPagePrinter( hPrinter ) )
    {
        EndDocPrinter( hPrinter );
        ClosePrinter( hPrinter );
        CloseHandle( hFile );
        return FALSE;
    }

    do {
        // Read data from the file
        if ( !ReadFile(hFile,
                        lpData,
                        4096,
                        &dwCount,
                        NULL) )
        {
            EndPagePrinter( hPrinter );
            EndDocPrinter( hPrinter );
            ClosePrinter( hPrinter );
            CloseHandle( hFile );
            return FALSE;
        }

        // Send the data to the printer
        if( ! WritePrinter( hPrinter, lpData, dwCount, &dwBytesWritten ) )
        {
            EndPagePrinter( hPrinter );
            EndDocPrinter( hPrinter );
            ClosePrinter( hPrinter );
            CloseHandle( hFile );
            return FALSE;
        }
    } while ( dwCount > 0 );
    // End the page
    if( ! EndPagePrinter( hPrinter ) )
    {
        EndDocPrinter( hPrinter );
        ClosePrinter( hPrinter );
        return FALSE;
    }
    // Inform the spooler that the document is ending
    if( ! EndDocPrinter( hPrinter ) )
    {
        ClosePrinter( hPrinter );
        CloseHandle( hFile );
        return FALSE;
    }
    // Tidy up the printer handle
    ClosePrinter( hPrinter );
    CloseHandle( hFile );

    return TRUE;

}


#ifdef COMMENT
typedef struct _PRINTER_INFO_2 { // pri2
    LPTSTR    pServerName;
    LPTSTR    pPrinterName;
    LPTSTR    pShareName;
    LPTSTR    pPortName;
    LPTSTR    pDriverName;
    LPTSTR    pComment;
    LPTSTR    pLocation;
    LPDEVMODE pDevMode;
    LPTSTR    pSepFile;
    LPTSTR    pPrintProcessor;
    LPTSTR    pDatatype;
    LPTSTR    pParameters;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD     Attributes;
    DWORD     Priority;
    DWORD     DefaultPriority;
    DWORD     StartTime;
    DWORD     UntilTime;
    DWORD     Status;
    DWORD     cJobs;
    DWORD     AveragePPM;
} PRINTER_INFO_2;
#endif

int
Win32EnumPrt( struct keytab ** pTable, struct keytab ** pTable2,
             int * pN, int *pDefault )
{
    DWORD  dwBytesNeeded;
    DWORD  dwPrtRet2;
    DWORD  dwMaxPrt;
    LPTSTR lpName = NULL;
    DWORD  dwEnumFlags = PRINTER_ENUM_LOCAL;
    DWORD  dwLevel = 2;
    LPPRINTER_INFO_1 pPrtInfo1=NULL;
    LPPRINTER_INFO_2 pPrtInfo2=NULL;
    int i, n, rc ;
    CHAR   szDefault[256]="";
    int    iDefault = -1;

    BOOL (WINAPI * p_GetDefaultPrinter)( LPTSTR pszBuffer,   // printer name buffer
                              LPDWORD pcchBuffer  // size of name buffer
                              ) = NULL;
    static HANDLE hWinSpool = INVALID_HANDLE_VALUE;

    if ( !p_GetDefaultPrinter ) {
        if ( hWinSpool == INVALID_HANDLE_VALUE )
            hWinSpool = LoadLibrary("winspool.drv");
        if ( hWinSpool != INVALID_HANDLE_VALUE )
            (FARPROC) p_GetDefaultPrinter =
                GetProcAddress( hWinSpool, "GetDefaultPrinterA" );
    }

    if ( p_GetDefaultPrinter ) {
        DWORD dwSize = 256;
        debug(F100,"Win32EnumPrt calling GetDefaultPrinter","",0);
        rc = p_GetDefaultPrinter( szDefault, &dwSize );
        debug(F111,"Win32EnumPrt GetDefaultPrinter returns",szDefault,rc);
    } else {
        debug(F100,"Win32EnumPrt calling GetProfileString","",0);
        rc = GetProfileString("windows","device",",,,",szDefault,256);
        debug(F111,"Win32EnumPrt GetProfileString returns",szDefault,rc);
    }
    if (rc) {
        for ( i=0;szDefault[i];i++ ) {
            if ( szDefault[i] == ',' ) {
                szDefault[i] = '\0';
                break;
            }
        }
    }

    if ( *pTable )
    {
        for ( i=0 ; i < *pN ; i++ )
            free( (*pTable)[i].kwd ) ;
        free ( *pTable )  ;
    }
    *pTable = NULL;
    if ( *pTable2 )
    {
        for ( i=0 ; i < *pN ; i++ )
            free( (*pTable2)[i].kwd ) ;
        free ( *pTable2 )  ;
    }
    *pTable2 = NULL;
    *pN = 0;
    *pDefault = -1;

    if ( !isWin95() ) {
        // dwEnumFlags |= PRINTER_ENUM_CONNECTIONS;
        if ( dwLevel == 1 ) {
            dwEnumFlags |= PRINTER_ENUM_NETWORK;
            dwEnumFlags |= PRINTER_ENUM_REMOTE;
        }
        debug(F111,"Win32EnumPrt","dwEnumFlags",dwEnumFlags);
    }

    //
    // get byte count needed for buffer, alloc buffer, the enum the printers
    //

    debug(F100,"Win32EnumPrt calling EnumPrinters","",0);
    rc = EnumPrinters (dwEnumFlags, lpName, dwLevel, NULL, 0, &dwBytesNeeded,
                   &dwPrtRet2);
    debug(F101,"Win32EnumPrt EnumPrinters returns","",0);

    //
    // (simple error checking, if these work assume rest will too)
    //

    if ( dwLevel == 1 ) {
        if (!(pPrtInfo1 = (LPPRINTER_INFO_1) LocalAlloc (LPTR, dwBytesNeeded)))
        {
            return(FALSE);
        }
    } else {
        if (!(pPrtInfo2 = (LPPRINTER_INFO_2) LocalAlloc (LPTR, dwBytesNeeded)))
        {
            return(FALSE);
        }
    }

    debug(F100,"Win32EnumPrt calling EnumPrinters","",0);
    rc = EnumPrinters (dwEnumFlags, lpName, dwLevel,
                      (dwLevel == 1 ? (LPBYTE) pPrtInfo1 : (LPBYTE) pPrtInfo2),
                        dwBytesNeeded, &dwBytesNeeded, &dwPrtRet2);
    debug(F101,"Win32EnumPrt EnumPrinters returns","",0);
    if (!rc || dwPrtRet2 == 0)
    {
        LocalFree( dwLevel == 1 ? (LPBYTE) pPrtInfo1 : (LPBYTE) pPrtInfo2 );
        return(FALSE);
    }

    /* we now have an enumeration of all printer names */

    (*pTable) = (struct keytab *) malloc( dwPrtRet2 * sizeof(struct keytab) );
    if ( !(*pTable) )
    {
        LocalFree( dwLevel == 1 ? (LPBYTE) pPrtInfo1 : (LPBYTE) pPrtInfo2 );
        return(FALSE);
    }
    (*pTable2) = (struct keytab *) malloc( dwPrtRet2 * sizeof(struct keytab) );
    if ( !(*pTable2) )
    {
        free(pTable);
        pTable = NULL;
        LocalFree( dwLevel == 1 ? (LPBYTE) pPrtInfo1 : (LPBYTE) pPrtInfo2 );
        return(FALSE);
    }

    for ( i=0 ; i<dwPrtRet2 ; i++ ) {
        char * s = NULL;
        char * p1 = NULL, * p2 = NULL;
        int j, k;

        if ( dwLevel == 1 ) {
            debug(F111,"Win32EnumPrt queue",pPrtInfo1[i].pName,i);
            if ( iDefault == -1 ) {
                if ( !strcmp(pPrtInfo1[i].pName,szDefault) )
                    iDefault = i;
            }
            p1 = strdup( pPrtInfo1[i].pName );
            p2 = strdup( pPrtInfo1[i].pName );
        } else {
            debug(F111,"Win32EnumPrt queue",pPrtInfo2[i].pPrinterName,i);
            if (pPrtInfo2[i].Attributes & PRINTER_ATTRIBUTE_DEFAULT)
                iDefault = i;
            if ( iDefault == -1 ) {
                if ( !strcmp(pPrtInfo2[i].pPrinterName,szDefault) )
                    iDefault = i;
            }
            p1 = strdup( pPrtInfo2[i].pPrinterName );
            p2 = strdup( pPrtInfo2[i].pPrinterName );
        }

        /* Now we must remove illegal characters from the pTable2 kwd */
        s = p2;
        while ( *s ) {
            switch ( *s ) {
            case ' ':
                *s = '_';
                break;
            case ',':
                *s = '.';
                break;
            case ';':
                *s = ':';
                break;
            case '\\':
                *s = '/';
                break;
            case '?':
                *s = '!';
                break;
            case '{':
                *s = '[';
                break;
            case '}':
                *s = ']';
                break;
            default:
                *s = tolower(*s);
            }
            s++;
        }

        for ( j=0;j<i;j++ ) {
            if ( _stricmp(p2,(*pTable2)[j].kwd) < 0 )
                break;
        }
        for ( k = i ; k > j ; k--) {
            (*pTable)[k].kwd = (*pTable)[k-1].kwd;
            (*pTable2)[k].kwd = (*pTable2)[k-1].kwd;
            (*pTable)[k].kwval = (*pTable)[k-1].kwval;
            (*pTable2)[k].kwval = (*pTable2)[k-1].kwval;
            (*pTable)[k].flgs = (*pTable)[k-1].flgs;
            (*pTable2)[k].flgs = (*pTable2)[k-1].flgs;
        }
        (*pTable)[j].kwd = p1;
        (*pTable2)[j].kwd = p2;
        (*pTable)[j].kwval = i;
        (*pTable2)[j].kwval = i;
        (*pTable)[j].flgs = 0;
        (*pTable2)[j].flgs = 0;
    }
    *pN = dwPrtRet2 ;

    if ( iDefault == -1 )
        *pDefault = 0;
    else {
        for ( i=0 ; i<dwPrtRet2 ; i++ )
            if ( (*pTable)[i].kwval == iDefault ) {
                *pDefault = i;
                break;
            }
    }

    LocalFree( dwLevel == 1 ? (LPBYTE) pPrtInfo1 : (LPBYTE) pPrtInfo2 );
    debug(F101,"Win32EnumPrt returns","",*pN);
    return(TRUE);
}


