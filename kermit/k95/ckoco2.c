/* C K O C O 2  --  Kermit connect command for OS/2 systems */

/*
  Authors: Frank da Cruz (fdc@columbia.edu),
             Columbia University Academic Information Systems, New York City.
           Jeffrey E Altman (jaltman@secure-endpoints.com)
             Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

/*
 * =============================#includes=====================================
 */

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#ifdef IKSD
#include "ckctel.h"
#endif /* IKSD */
#include "ckcasc.h"             /* ASCII character symbols */
#include "ckuusr.h"             /* For terminal type definitions, etc. */
#ifndef NOLOCAL
#include "ckcxla.h"             /* Character set translation */
#include "ckopcf.h"             /* PC Fonts resource definitions */

#ifdef KUI
#include "kui/ikui.h"
#endif /* KUI */

#include <ctype.h>              /* Character types */
#include <io.h>                 /* File io function declarations */
#include <process.h>            /* Process-control function declarations */
#include <stdlib.h>             /* Standard library declarations */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#ifdef NT
extern int OSVer;
#else /* NT */
#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */

#define INCL_NOPM
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_DOSMODULEMGR
#define INCL_DOSRESOURCES
#define INCL_WINCLIPBOARD
#define INCL_DOSDATETIME
#include <os2.h>
#undef COMMENT                          /* COMMENT is defined in os2.h */
#endif /* NT */

#include "ckcuni.h"
#include "ckocon.h"                     /* defs common to console routines */
#include "ckokey.h"

videobuffer vscrn[VNUM]  = {{0,0,0,0,0,0,{0,0},0,-1,-1,-1,-1,{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},0,0},
                            {0,0,0,0,0,0,{0,0},0,-1,-1,-1,-1,{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},0,0},
                            {0,0,0,0,0,0,{0,0},0,-1,-1,-1,-1,{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},0,0},
                            {0,0,0,0,0,0,{0,0},0,-1,-1,-1,-1,{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},0,0}};
extern int tt_update, tt_updmode, tt_rows[], tt_cols[], tt_font, tt_roll[],
           tt_cursor;
int tt_attr_bug = 0;
int tt_diff_upd = 1;
int tt_url_hilite = 1;
#ifdef KUI
int tt_url_hilite_attr = VT_CHAR_ATTR_BOLD | VT_CHAR_ATTR_UNDERLINE;
#else /* KUI */
int tt_url_hilite_attr = VT_CHAR_ATTR_BOLD;
#endif /* KUI */
extern int updmode ;
extern int priority ;
extern int tt_modechg;
extern unsigned char defaultattribute ;
extern unsigned char colornormal;
extern unsigned char reverseattribute ;
extern unsigned char italicattribute;
extern unsigned char graphicattribute ;
extern unsigned char underlineattribute ;
extern unsigned char borderattribute ;

extern vtattrib attrib, cmdattrib;
extern bool cursoron[], cursorena[],scrollflag[], scrollstatus[], flipscrnflag[] ;
extern TID tidTermScrnUpd ;

extern
#ifdef NT
HANDLE
#else
HVIO
#endif
VioHandle;

#ifdef OS2MOUSE
extern int tt_mouse ;
#endif /* OS2MOUSE */
#ifdef IKSD
extern int inserver;
#endif /* IKSD */

extern int tt_cursor;                   /* Cursor type */
extern int tt_status[];                 /* Status line displayed ? */

extern unsigned char     colorstatus     ;
extern unsigned char     colorselect ;
extern unsigned char     colorcmd;

extern enum markmodes markmodeflag[] ;

#ifdef PCFONTS
HMODULE hPCFonts = 0 ;
#define FONTBUFSIZE  16384
VIOFONTINFO  SavedFontInfo ;
BYTE    SavedFontBuffer[FONTBUFSIZE];
BOOL    FontSaved = 0 ;
#endif /* PCFONTS */

BYTE vmode = VCMD ;
extern int decsasd ;
int pwidth, pheight;            /* Physical screen width, height */

/*---------------------------------------------------------------------------*/
/* ReadCellStr                                                               */
/*---------------------------------------------------------------------------*/
USHORT
ReadCellStr( viocell * CellStr, PUSHORT Length, USHORT Row, USHORT Column )
{
#ifdef NT
    static LPWSTR wchars = NULL ;
    static LPTSTR tchars = NULL ;
    static LPWORD attrs = NULL ;
    static int charlen = 0 ;
    static int attrlen = 0 ;
    COORD coord ;
    DWORD read ;
    int len = *Length ;
    USHORT rc = 0;
    int i ;

    coord.X=Column ;
    coord.Y=Row ;

    if ( len > attrlen )
    {
        if ( attrs )
            free( attrs ) ;
        attrs = malloc( len * sizeof(WORD) ) ;
        if ( attrs )
        {
            attrlen = len ;
        }
        else
        {
            attrlen = 0 ;
            return 1 ;
        }
    }
    rc = ReadConsoleOutputAttribute( VioHandle, attrs, len, coord, &read ) ;
    if ( !rc ) {
        DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"ReadCellStr VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"ReadCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        return 3;
    }

    if ( isunicode() ) {
        if ( !wchars || len > charlen )
        {
            if ( wchars )
                free( wchars ) ;
            wchars = malloc( len * sizeof(WCHAR) ) ;
            if ( wchars )
            {
                charlen = len ;
            }
            else
            {
                charlen = 0 ;
                return 1 ;
            }
        }

        rc = ReadConsoleOutputCharacterW( VioHandle, wchars, len, coord, &read ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"ReadCellStr VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"ReadCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
            return 3;
        }
        if ( read != len )
        {
            rc = 2 ;
        }
        else
        {
            for ( i=0; i<len ; i++ )
            {
                ((viocell *)CellStr)[i].c = wchars[i] ;
                ((viocell *)CellStr)[i].a = attrs[i] ;
            }
        }
    }
    else {
        if ( !tchars || len > charlen )
        {
            if ( tchars )
                free( tchars ) ;
            tchars = malloc( len * sizeof(TCHAR) ) ;
            if ( tchars  )
            {
                charlen = len ;
            }
            else
            {
                charlen = 0 ;
                return 1 ;
            }
        }
        rc = ReadConsoleOutputCharacter( VioHandle, tchars, len, coord, &read ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"ReadCellStr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"ReadCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
        if ( read != len )
        {
            rc = 2 ;
        }
        else
        {
            for ( i=0; i<len ; i++ )
            {
                CellStr[i].c = tchars[i] ;
                CellStr[i].a = attrs[i] ;
            }
        }
    }

    return rc ;

#else /* NT */
    *Length *= sizeof(viocell) ;
    return VioReadCellStr( (PCH) CellStr, Length, Row, Column, VioHandle ) ;
#endif /* NT */
}

/*---------------------------------------------------------------------------*/
/* WrtCellStr                                                                */
/*---------------------------------------------------------------------------*/
#ifdef NT
USHORT
OldWin32WrtCellStr( viocell * CellStr, USHORT Length, USHORT Row, USHORT Column )
{
    static LPWSTR wchars = NULL;
    static LPTSTR tchars = NULL ;
    static LPWORD attrs = NULL ;
    static int charlen = 0 ;
    static int attrlen = 0 ;
    COORD coord ;
    DWORD written ;
    int len = Length ;
    USHORT rc = 0;
    int i ;

    coord.X=Column ;
    coord.Y=Row ;

    if ( len > attrlen )
    {
        if ( attrs )
            free( attrs ) ;
        attrs = malloc( len * sizeof(WORD) ) ;
        if ( attrs )
        {
            attrlen = len ;
        }
        else
        {
            attrlen = 0 ;
            return 1 ;
        }
    }

    if ( isunicode() ) {
        if ( !wchars || len > charlen )
        {
            if ( wchars )
                free( wchars ) ;
            wchars = malloc( len * sizeof(WCHAR) ) ;
            if ( wchars )
            {
                charlen = len ;
            }
            else
            {
                charlen = 0 ;
                return 1 ;
            }
        }

        for ( i=0; i<len ; i++ )
        {
            wchars[i] = CellStr[i].c ;
            attrs[i] = CellStr[i].a ;
        }
        rc = WriteConsoleOutputCharacterW( VioHandle, wchars, len, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"OldWin32WrtCellStr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"OldWin32WrtCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }
    else {
        if ( !tchars || len > charlen )
        {
            if ( tchars )
                free( tchars ) ;
            tchars = malloc( len * sizeof(TCHAR) ) ;
            if ( tchars )
            {
                charlen = len ;
            }
            else
            {
                charlen = 0 ;
                return 1 ;
            }
        }
        for ( i=0; i<len ; i++ )
        {
            tchars[i] = ((viocell *)CellStr)[i].c ;
            attrs[i] = ((viocell *)CellStr)[i].a ;
        }
        rc = WriteConsoleOutputCharacter( VioHandle, tchars, len, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"OldWin32WrtCellStr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"OldWin32WrtCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }

    /* This is to prevent a fatal bug in Win 95 Gold */
    if ( !isWin95() ||
         !tt_attr_bug ||
         ( pwidth == 80 && ( pheight == 25 || pheight == 43 || pheight == 50 ) )
         ) {
        rc = WriteConsoleOutputAttribute( VioHandle, attrs, len, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"OldWin32WrtCellStr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"OldWin32WrtCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }
    if ( written != len )
    {
        rc = 2 ;
    }
    return(0);
}
#endif /* NT */

USHORT
WrtCellStr( viocell * CellStr, USHORT Length, USHORT Row, USHORT Column )
{
#ifdef NT
    static CHAR_INFO *lpBuffer = NULL;  // pointer to buffer with data to write
    static COORD dwBufferSize  = {0,0}; // column-row size of source buffer
    COORD dwBufferCoord = {0,0};        // upper-left cell to write from
    SMALL_RECT WriteRegion = {0,0,0,0}; // pointer to rectangle to write to
    static int buflen = 0 ;

    int len = Length * sizeof(CHAR_INFO) ;
    USHORT rc = 0;
    int i ;

    if ( Row == 0 && Column == 0 && Length == pwidth * pheight) {
        dwBufferSize.X=pwidth ;
        dwBufferSize.Y=pheight ;
        WriteRegion.Left=0;
        WriteRegion.Top=0;
        WriteRegion.Right=pwidth-1;
        WriteRegion.Bottom=pheight-1;
    }
    else {
        return OldWin32WrtCellStr( CellStr, Length, Row, Column );
    }

    if ( len > buflen )
    {
        if ( lpBuffer )
            free( lpBuffer ) ;
        lpBuffer = malloc( len ) ;
        if ( lpBuffer )
        {
            buflen = len ;
        }
        else
        {
            buflen = 0 ;
            return 1 ;
        }
    }

    if ( isunicode() ) {
        for ( i=0; i<Length ; i++ )
        {
            lpBuffer[i].Char.UnicodeChar = CellStr[i].c ;
            lpBuffer[i].Attributes       = CellStr[i].a ;
        }
        rc = WriteConsoleOutputW( VioHandle, lpBuffer, dwBufferSize, dwBufferCoord,
                             &WriteRegion ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"WrtCellStr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"WrtCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }
    else {
        for ( i=0; i<Length ; i++ )
        {
            lpBuffer[i].Char.AsciiChar = CellStr[i].c ;
            lpBuffer[i].Attributes       = CellStr[i].a ;
        }
        rc = WriteConsoleOutput( VioHandle, lpBuffer, dwBufferSize, dwBufferCoord,
                            &WriteRegion ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"WrtCellStr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"WrtCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }

    if ( WriteRegion.Top + WriteRegion.Left + WriteRegion.Right + WriteRegion.Bottom
         != pheight + pwidth - 2 )
    {
        rc = 2 ;
    }
   return(0);
#else /* NT */
   return VioWrtCellStr( (PCH) CellStr, Length*sizeof(viocell), Row, Column, VioHandle ) ;
#endif /* NT */
}




/* WrtCellStrDiff
 *   only write the changes to the screen.
 */

static viocell * last_vio = NULL;
static int       last_len = 0, last_violen = 0;
static USHORT    last_col = -1, last_row = -1;

void
VscrnForceFullUpdate(void)
{
    last_len = 0;
    os2settitle(NULL,1);                /* Force a Title update */
}

USHORT
WrtCellStrDiff( viocell * CellStr, USHORT Length, USHORT Row, USHORT Column,
                USHORT Height, USHORT Width )
{
    USHORT rc = 0;

    /* If we do not have anything to compare it to, or there was a  */
    /* fundamental change write the entire string.                   */
    if ( last_vio == NULL ||
         last_len != Length ||
         last_row != Row ||
         last_col != Column )
    {
        rc = WrtCellStr(CellStr,Length,Row,Column);
        if ( rc ) {                     /* failure */
            last_len = 0;
            last_col = -1;
            last_row = -1;
        } else {
            if ( last_vio && last_violen < Length ) {
                free(last_vio);
                last_violen = 0;
                last_vio = NULL;
            }
            if ( last_vio == NULL ) {
                last_vio = (viocell *) malloc(Length*sizeof(viocell));
                if ( last_vio == NULL ) {
                    last_len = 0;
                    return rc;
                }
            }
            memcpy(last_vio,CellStr,Length*sizeof(viocell));
            last_len = Length;
            last_row = Row;
            last_col = Column;
        }
    } else {
        /* Now we start looking for changes */
        viocell * vpo = last_vio;
        viocell * vpn = CellStr, * vpnc=CellStr;
        int count = 0, ccount=0;

        while ( count < Length ) {
            if ( vpo->c == vpn->c &&
                 vpo->a == vpn->a )
            {
                count++;
                vpo++;
                vpn++;
                continue;
            }

            /* we have found a change */
            vpnc = vpn;
            ccount = 1;
            count++;;
            vpo++;
            vpn++;

            /* Find the length of the change */
            while ( count < Length &&
                (vpn->c != vpo->c ||
                    vpn->a != vpo->a) ) {
                count++;
                ccount++;
                vpo++;
                vpn++;

            }

            /* Write it */
            WrtCellStr(vpnc,ccount,
                        (Row*Width+Column+count-ccount)/Width,
                        (Row*Width+Column+count-ccount)%Width);

            count++;
            vpo++;
            vpn++;

        }
        memcpy(last_vio,CellStr,Length*sizeof(viocell));
    }
    return rc ;

}

/*---------------------------------------------------------------------------*/
/* WrtNCell                                                                  */
/*---------------------------------------------------------------------------*/
USHORT
WrtNCell( viocell Cell, USHORT Times, USHORT Row, USHORT Column )
{
#ifdef NT
    static LPWSTR wchars = NULL;
    static LPTSTR tchars = NULL ;
    static LPWORD attrs = NULL ;
    static int charlen = 0 ;
    static int attrlen = 0 ;
    COORD coord ;
    DWORD written ;
    USHORT rc = 0;
    int i ;

    coord.X=Column ;
    coord.Y=Row ;

    if ( Times > attrlen )
    {
        if ( attrs )
            free( attrs ) ;
        attrs = malloc( Times * sizeof(WORD) ) ;
        if ( attrs )
        {
            attrlen = Times ;
        }
        else
        {
            attrlen = 0 ;
            return 1 ;
        }
    }

    if ( isunicode() ) {
        if ( !wchars || Times > charlen )
        {
            if ( wchars )
                free( wchars ) ;
            wchars = malloc( Times * sizeof(WCHAR) ) ;
            if ( wchars )
            {
                charlen = Times ;
            }
            else
            {
                charlen = 0 ;
                return 1 ;
            }
        }

        for ( i=0; i<Times ; i++ )
        {
            wchars[i] = Cell.c ;
            attrs[i] = Cell.a ;
        }
        rc = WriteConsoleOutputCharacterW( VioHandle, wchars, Times, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"WrtCellStr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"WrtCellStr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }
    else {
        if ( tchars || Times > charlen )
        {
            if ( tchars )
                free( tchars ) ;
            tchars = malloc( Times * sizeof(TCHAR) ) ;
            if ( tchars )
            {
                charlen = Times ;
            }
            else
            {
                charlen = 0 ;
                return 1 ;
            }
        }

        for ( i=0; i<Times ; i++ )
        {
            tchars[i] = Cell.c ;
            attrs[i] = Cell.a ;
        }
        rc = WriteConsoleOutputCharacter( VioHandle, tchars, Times, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"WrtNCell VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"WrtNCell GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }

    /* This is to prevent a fatal bug in Win 95 Gold */
    if ( !isWin95() ||
         !tt_attr_bug ||
         tt_modechg != TVC_ENA ||
         ( pwidth  == 80 && ( pheight == 25 || pheight == 43 || pheight == 50 ) )
         ) {
        rc = WriteConsoleOutputAttribute( VioHandle, attrs, Times, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"WrtNCell VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"WrtNCell GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }
    if ( written != Times )
    {
        rc = 2 ;
    }
    return rc ;

#else /* NT */
   return VioWrtNCell( (PCH) &Cell, Times, Row, Column, VioHandle ) ;
#endif /* NT */
}

/*---------------------------------------------------------------------------*/
/* WrtCharStrAtt                                                             */
/*---------------------------------------------------------------------------*/
USHORT
WrtCharStrAtt( PCH CharStr, USHORT Length, USHORT Row, USHORT Column,
                      PBYTE Attr )
{
#ifdef NT
    static LPWSTR wchars = NULL;
    static LPTSTR tchars = NULL ;
    static LPWORD attrs = NULL ;
    static int charlen = 0 ;
    static int attrlen = 0 ;
    COORD coord ;
    DWORD written ;
    USHORT rc = 0;
    int i ;

    coord.X=Column ;
    coord.Y=Row ;

    if ( Length > attrlen )
    {
        if ( attrs )
            free( attrs ) ;
        attrs = malloc( Length * sizeof(WORD) ) ;
        if ( attrs )
        {
            attrlen = Length ;
        }
        else
        {
            attrlen = 0 ;
            return 1 ;
        }
    }

    if ( isunicode() ) {
        if ( !wchars || Length > charlen )
        {
            if ( wchars )
                free( wchars ) ;
            wchars = malloc( Length * sizeof(WCHAR) ) ;
            if ( wchars )
            {
                charlen = Length ;
            }
            else
            {
                charlen = 0 ;
                return 1 ;
            }
        }

        for ( i=0; i<Length ; i++ )
        {
            wchars[i] = CharStr[i] ;
            attrs[i] = *Attr ;
        }
        rc = WriteConsoleOutputCharacterW( VioHandle, wchars, Length, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"WrtCharStrAttr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"WrtCharStrAttr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }
    else {
        if ( !tchars || Length > charlen )
        {
            if ( tchars )
                free( tchars ) ;
            tchars = malloc( Length * sizeof(TCHAR) ) ;
            if ( tchars )
            {
                charlen = Length ;
            }
            else
            {
                charlen = 0 ;
                return 1 ;
            }
        }

        for ( i=0; i<Length ; i++ )
        {
            tchars[i] = CharStr[i] ;
            attrs[i] = *Attr ;
        }
        rc = WriteConsoleOutputCharacter( VioHandle, tchars, Length, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"WrtCharStrAttr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"WrtCharStrAttr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }

    /* This is to prevent a fatal bug in Win 95 Gold */
    if ( !isWin95() ||
        !tt_attr_bug ||
         ( pwidth == 80 && ( pheight == 25 || pheight == 43 || pheight == 50 ) )
         ) {
        rc = WriteConsoleOutputAttribute( VioHandle, attrs, Length, coord, &written ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"WrtCharStrAttr VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"WrtCharStrAttr GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            return 3;
        }
    }

    if ( written != Length )
    {
        rc = 2 ;
    }
    return rc ;

#else /* NT */
   return VioWrtCharStrAtt( CharStr, Length, Row, Column, Attr, VioHandle ) ;
#endif /* NT */
}

#ifndef KUI
/*---------------------------------------------------------------------------*/
/* GetMode                                                                   */
/*   Determines:                                                             */
/*     VGA video mode */
/*     number of colors */
/*     number of columns */
/*     number of rows */
/*     horizontal and vertical resolution */
/*---------------------------------------------------------------------------*/
USHORT
GetMode( PCK_VIDEOMODEINFO ModeData )
{
    USHORT rc ;
#ifdef NT
    CONSOLE_SCREEN_BUFFER_INFO mi ;

    rc = 1 ;
    memset(ModeData,0,sizeof(CK_VIDEOMODEINFO));
    if ( GetConsoleScreenBufferInfo( VioHandle, &mi ) )
    {
        rc = 0 ;

        ModeData->col      = mi.srWindow.Right-mi.srWindow.Left+1;
        ModeData->row      = mi.srWindow.Bottom-mi.srWindow.Top+1;
        ModeData->sbcol    = mi.dwSize.X ;
        ModeData->sbrow    = mi.dwSize.Y ;
        ModeData->color    = mi.wAttributes ;
    }
    else {
        DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"GetMode VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"GetMode GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        return 3;
    }
    if (!rc) {
        pwidth = ModeData->sbcol;
        pheight = ModeData->sbrow;
    }
#else /* just OS/2 */
    VIOMODEINFO mi ;
    memset(ModeData,0,sizeof(CK_VIDEOMODEINFO));
    mi.cb = 12 ;

    if ((rc = VioGetMode( &mi, VioHandle )) == 0)
    {
        ModeData->fbType = mi.fbType ;
        ModeData->color  = mi.color ;
        ModeData->col    = mi.col ;
        ModeData->row    = mi.row ;
        ModeData->hres   = mi.hres ;
        ModeData->vres   = mi.vres ;
        ModeData->wcol   = 0 ;
        ModeData->wrow   = 0 ;
    }
    if (!rc) {
        pwidth = ModeData->col;
        pheight = ModeData->row;
    }
#endif /* NT */
    return rc ;
}

/*---------------------------------------------------------------------------*/
/* SetMode                                                                   */
/*---------------------------------------------------------------------------*/
USHORT
SetMode( PCK_VIDEOMODEINFO ModeData )
{
    USHORT rc=0 ;
#ifdef NT
    COORD wsize, sbsize;
    SMALL_RECT rect;
    RECT  rConsole, rClient ;
    CONSOLE_SCREEN_BUFFER_INFO csbi ;
    extern HWND hwndConsole;
    extern int k95stdout;
    int    fixedX=0, fixedY=0, pixelsX=0, pixelsY=0, desiredX=0, desiredY=0;
    int    setwin=0;

    if ( k95stdout )                    /* do nothing if stdout */
        return(0);

#ifdef COMMENT
    BOOL MoveWindow( HWND hWnd,     // handle of window
                     int X,         // horizontal position
                     int Y,         // vertical position
                     int nWidth,    // width
                     int nHeight,   // height
                     BOOL bRepaint  // repaint flag
                     );
#endif /* COMMENT */

    if ( ModeData->col <= 0 || ModeData->row <= 0 )
        return -1 ;

    rc = RequestScreenMutex( 5000 );
    if ( rc )
    {
        debug(F101,"SetMode() unable to retrieve Screen Mutex","",rc);
        return -1;
    }

    VscrnForceFullUpdate();

    /* Create a size structure for new specifications */
    wsize.X = ModeData->col ;
    wsize.Y = ModeData->row ;
    sbsize.X = ModeData->sbcol ;
    sbsize.Y = ModeData->sbrow ;
    debug(F111,"SetMode","wsize.X",wsize.X) ;
    debug(F111,"SetMode","wsize.Y",wsize.Y) ;
    debug(F111,"SetMode","sbsize.X",sbsize.X) ;
    debug(F111,"SetMode","sbsize.Y",sbsize.Y) ;

    /* Get current size of the window and screen buffer */
    rc = GetConsoleScreenBufferInfo( VioHandle, &csbi ) ;
    if ( !rc ) {
        DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"SetMode VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"SetMode GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        ReleaseScreenMutex() ;
        return 3;
    }

    debug(F111,"SetMode GetConsoleScreenBufferInfo","dwSize.X",csbi.dwSize.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","dwSize.Y",csbi.dwSize.Y);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","dwCursorPosition.X",csbi.dwCursorPosition.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","dwCursorPosition.Y",csbi.dwCursorPosition.Y);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","wAttributes",csbi.wAttributes);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","srWindow.Top",csbi.srWindow.Top);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","srWindow.Bottom",csbi.srWindow.Bottom);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","srWindow.Left",csbi.srWindow.Left);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","srWindow.Right",csbi.srWindow.Right);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","dwMaximumWindowSize.X",csbi.dwMaximumWindowSize.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo","dwMaximumWindowSize.Y",csbi.dwMaximumWindowSize.Y);

    if (GetWindowRect( hwndConsole, &rConsole )){
        debug(F111,"SetMode GetWindowRect","rConsole.Top",   rConsole.top);
        debug(F111,"SetMode GetWindowRect","rConsole.Left",  rConsole.left);
        debug(F111,"SetMode GetWindowRect","rConsole.Bottom",rConsole.bottom);
        debug(F111,"SetMode GetWindowRect","rConsole.Right", rConsole.right);
    }
    if (GetClientRect( hwndConsole, &rClient )){
        debug(F111,"SetMode GetWindowRect","rClient.Top",   rClient.top);
        debug(F111,"SetMode GetWindowRect","rClient.Left",  rClient.left);
        debug(F111,"SetMode GetWindowRect","rClient.Bottom",rClient.bottom);
        debug(F111,"SetMode GetWindowRect","rClient.Right", rClient.right);
    }

    fixedX = rConsole.right - rConsole.left - rClient.right;
    fixedY = rConsole.bottom - rConsole.top - rClient.bottom;
    pixelsX = 1+ (rClient.right) / csbi.dwSize.X;
    pixelsY = 1+ (rClient.bottom) / csbi.dwSize.Y;
    desiredX = pixelsX * wsize.X + fixedX;
    desiredY = pixelsY * wsize.Y + fixedY;

    debug(F111,"SetMode","fixedX",fixedX);
    debug(F111,"SetMode","fixedY",fixedY);
    debug(F111,"SetMode","pixelsX",pixelsX);
    debug(F111,"SetMode","pixelsY",pixelsY);
    debug(F111,"SetMode","desired Right",desiredX);
    debug(F111,"SetMode","desired Bottom",desiredY);

    /*
    The rules that apply here are quite simple.  We are dealing
    with two boxes that we must keep in sync: window size and
    screen buffer size.  The window cannot be larger in any
    dimension than the screen buffer.  The screen buffer cannot
    be smaller in any dimension than the window.
    */

    /* Why do anything if there is no work to do? */
    if ( (csbi.srWindow.Right - csbi.srWindow.Left +1) == wsize.X  &&
         (csbi.srWindow.Bottom - csbi.srWindow.Top +1) == wsize.Y &&
         sbsize.X == csbi.dwSize.X && sbsize.Y == csbi.dwSize.Y ) {
        debug(F110,"SetMode","nothing to do",0);
        goto setattr;
    }

    /* Adjust the width of the window to the desired size */
    if ( (csbi.srWindow.Right - csbi.srWindow.Left +1) > wsize.X )
    {
        rect.Top = rect.Left = 0 ;
        rect.Right  = wsize.X - 1 ;
        rect.Bottom = (csbi.srWindow.Bottom - csbi.srWindow.Top);
        setwin++;
    }

    /* Now adjust the height of the window */
    if ( (csbi.srWindow.Bottom - csbi.srWindow.Top +1) > wsize.Y )
    {
        rect.Top = rect.Left = 0 ;
        rect.Right  = (wsize.X < csbi.dwMaximumWindowSize.X ? wsize.X :
                      csbi.dwMaximumWindowSize.X)-1;
        rect.Bottom = wsize.Y - 1 ;
        setwin++;
    }

    if (setwin) {
        if ( !SetConsoleWindowInfo( VioHandle, TRUE, &rect ) )
        {
            rc = GetLastError() ;
            debug(F101,"Set Mode SetConsoleWindowInfo failed","",rc ) ;
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"SetMode VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"SetMode GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            ReleaseScreenMutex() ;
            return 3;
        }
        if (isWin95())
            msleep(50);
        setwin = 0;
    }

    /* If the window was getting smaller the screen buffer will
       now be larger than the window size.  So it should be safe
       to decrease the screen buffer to match.   If the window
       is getting larger, the screen buffer must first be increased
       in size.
     */
    if (!(sbsize.X == csbi.dwSize.X && sbsize.Y == csbi.dwSize.Y)) {
        if ( !SetConsoleScreenBufferSize( VioHandle, sbsize ) )
        {
            rc = GetLastError() ;
            debug(F101,"SetMode SetConsoleScreenBufferSize failed","",rc ) ;
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"SetMode VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"SetMode GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            ReleaseScreenMutex() ;
            return 3;
        }
        if (isWin95())
            msleep(50);
    }

        /* Get the current Screen Buffer Info */
        rc = GetConsoleScreenBufferInfo( VioHandle, &csbi ) ;
        if ( !rc ) {
            DWORD error = GetLastError();
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"SetMode VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"SetMode GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            ReleaseScreenMutex() ;
            return 3;
        }

#ifdef COMMENT
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","dwSize.X",csbi.dwSize.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","dwSize.Y",csbi.dwSize.Y);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","dwCursorPosition.X",csbi.dwCursorPosition.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","dwCursorPosition.Y",csbi.dwCursorPosition.Y);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","wAttributes",csbi.wAttributes);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","srWindow.Top",csbi.srWindow.Top);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","srWindow.Bottom",csbi.srWindow.Bottom);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","srWindow.Left",csbi.srWindow.Left);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","srWindow.Right",csbi.srWindow.Right);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","dwMaximumWindowSize.X",csbi.dwMaximumWindowSize.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 2","dwMaximumWindowSize.Y",csbi.dwMaximumWindowSize.Y);
#endif

    /* Change the width of the window if necessary */
    if ( (csbi.srWindow.Right - csbi.srWindow.Left+1) < wsize.X )
    {
        rect.Top = rect.Left = 0 ;
        rect.Right = (csbi.dwMaximumWindowSize.X > wsize.X ?
                       wsize.X : csbi.dwMaximumWindowSize.X) - 1 ;
        rect.Bottom = (csbi.srWindow.Bottom - csbi.srWindow.Top);
        setwin++;
    }

    /* And now the window height if necessary */
    if ( (csbi.srWindow.Bottom - csbi.srWindow.Top+1) < wsize.Y )
    {
        rect.Top = rect.Left = 0 ;
        rect.Right = (csbi.dwMaximumWindowSize.X > wsize.X ?
                       wsize.X : csbi.dwMaximumWindowSize.X) - 1 ;
        rect.Bottom = (csbi.dwMaximumWindowSize.Y > wsize.Y ?
                        wsize.Y : csbi.dwMaximumWindowSize.Y) - 1 ;
        setwin++;
    }

    if (setwin) {
        if ( !SetConsoleWindowInfo( VioHandle, TRUE, &rect ) ) {
            DWORD error = GetLastError();
            debug( F101, "SetConsoleWindowInfo failed","",error);
            if ( error == ERROR_INVALID_HANDLE ) {
                debug(F101,"SetMode VioHandle","",VioHandle);
                VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
                debug(F101,"SetMode GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
            }
            ReleaseScreenMutex() ;
            return 3;
        }
        if (isWin95())
            msleep(50);
        setwin = 0;
    }

#ifdef COMMENT
    /* Get the current Screen Buffer Info */
    rc = GetConsoleScreenBufferInfo( VioHandle, &csbi ) ;
    if ( !rc ) {
        DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"SetMode VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"SetMode GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        ReleaseScreenMutex() ;
        return 3;
    }

    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","dwSize.X",csbi.dwSize.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","dwSize.Y",csbi.dwSize.Y);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","dwCursorPosition.X",csbi.dwCursorPosition.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","dwCursorPosition.Y",csbi.dwCursorPosition.Y);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","wAttributes",csbi.wAttributes);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","srWindow.Top",csbi.srWindow.Top);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","srWindow.Bottom",csbi.srWindow.Bottom);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","srWindow.Left",csbi.srWindow.Left);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","srWindow.Right",csbi.srWindow.Right);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","dwMaximumWindowSize.X",csbi.dwMaximumWindowSize.X);
    debug(F111,"SetMode GetConsoleScreenBufferInfo 3","dwMaximumWindowSize.Y",csbi.dwMaximumWindowSize.Y);
#endif

    /* Put the proper color in place */
  setattr:
    rc = SetConsoleTextAttribute( VioHandle, (WORD) ModeData->color ) ;
    if ( !rc ) {
        DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"SetMode VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"SetMode GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        ReleaseScreenMutex() ;
        return 3;
    }

#ifdef COMMENT
    /* This is an attempt to preserve the Window Size when Win95 is using Auto */
    /* font sizing.                                                            */

    msleep(1);
    GetWindowRect(hwndConsole, &rConsole);
    MoveWindow( hwndConsole,
                  rConsole.left,
                  rConsole.top,
                  desiredX,
                  desiredY,
                  TRUE);

    if (GetWindowRect( hwndConsole, &rConsole )){
        debug(F111,"SetMode GetWindowRect 2","rConsole.Top",   rConsole.top);
        debug(F111,"SetMode GetWindowRect 2","rConsole.Left",  rConsole.left);
        debug(F111,"SetMode GetWindowRect 2","rConsole.Bottom",rConsole.bottom);
        debug(F111,"SetMode GetWindowRect 2","rConsole.Right", rConsole.right);
    }
    if (GetClientRect( hwndConsole, &rClient )){
        debug(F111,"SetMode GetWindowRect 2","rClient.Top",   rClient.top);
        debug(F111,"SetMode GetWindowRect 2","rClient.Left",  rClient.left);
        debug(F111,"SetMode GetWindowRect 2","rClient.Bottom",rClient.bottom);
        debug(F111,"SetMode GetWindowRect 2","rClient.Right", rClient.right);
    }
#endif /* COMMENT */

    pwidth = ModeData->sbcol;
    pheight = ModeData->sbrow;
#else /* just OS/2 */
    VIOMODEINFO mi ;
    mi.cb = 12 ;
    mi.fbType = ModeData->fbType ;
    mi.color  = ModeData->color ;
    mi.col    = ModeData->col ;
    mi.row    = ModeData->row ;
    mi.hres   = ModeData->hres ;
    mi.vres   = ModeData->vres ;

    rc = RequestScreenMutex( 1000 );
    if ( rc )
    {
        debug(F100,"SetMode() unable to retrieve Screen Mutex","",0);
        ReleaseScreenMutex() ;
        return -1;
    }
    rc = VioSetMode( &mi, VioHandle ) ;
    pwidth = ModeData->col;
    pheight = ModeData->row;
#endif /* NT */

    msleep(50);                 /* Give OS time to resize the screen */
    ReleaseScreenMutex() ;
    return rc ;
}
#endif /* KUI */

/*---------------------------------------------------------------------------*/
/* GetCurPos                                                                 */
/*---------------------------------------------------------------------------*/
USHORT GetCurPos( PUSHORT Row, PUSHORT Column )
{
#ifdef NT
    USHORT rc = 1 ;
    CONSOLE_SCREEN_BUFFER_INFO mi ;

    if ( GetConsoleScreenBufferInfo( VioHandle, &mi ) )
    {
        rc = 0 ;
        *Row = mi.dwCursorPosition.Y ;
        *Column = mi.dwCursorPosition.X ;
    }
    else {
        DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"GetCurPos VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"GetCurPos GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        return 3;
    }
    return rc ;
#else /* NT */
    return VioGetCurPos( Row, Column, VioHandle ) ;
#endif /* NT */
}

/*---------------------------------------------------------------------------*/
/* SetCurPos                                                                 */
/*---------------------------------------------------------------------------*/
USHORT SetCurPos( USHORT Row, USHORT Column )
{
#ifdef NT
    COORD pos ;
    int rc = 0;

    pos.X = Column ;
    pos.Y = Row ;
    if ( !SetConsoleCursorPosition( VioHandle, pos ) )
    {
        DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"SetCurPos VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"SetCurPos GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        return 3;
    }
    return rc ;
#else /* NT */
   return VioSetCurPos( Row, Column, VioHandle ) ;
#endif /* NT */
}

/*---------------------------------------------------------------------------*/
/* GetCurType                                                                */
/*---------------------------------------------------------------------------*/
USHORT GetCurType( PCK_CURSORINFO CursorData )
{
    USHORT rc = 0;
#ifdef NT
    CONSOLE_CURSOR_INFO ci ;
    rc = 1 ;
    if ( GetConsoleCursorInfo( VioHandle, &ci ) )
    {
        rc = 0 ;
        CursorData->yStart = ci.dwSize ;
        CursorData->attr   = ci.bVisible ;
    }
    else {
        DWORD error = GetLastError();
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"SetCurPos VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"SetCurPos GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        return 3;
    }

#else /* just OS/2 */
    VIOCURSORINFO ci ;
    if ((rc = VioGetCurType( &ci, VioHandle )) == 0)
        {
        CursorData->yStart = ci.yStart ;
        CursorData->cEnd   = ci.cEnd   ;
        CursorData->cx     = ci.cx     ;
        CursorData->attr   = ci.attr   ;
        }
#endif /* NT */
    return rc ;
}

/*---------------------------------------------------------------------------*/
/* SetCurType                                                                */
/*---------------------------------------------------------------------------*/
USHORT SetCurType( PCK_CURSORINFO CursorData )
{
    USHORT rc = 0 ;
#ifdef NT
    CONSOLE_CURSOR_INFO ci ;

    if ( CursorData->yStart >= 1 && CursorData->yStart <= 100 )
        ci.dwSize = CursorData->yStart ;
    else {
        debug(F101,"Illegal cursor size","",CursorData->yStart);
        ci.dwSize = 37;
    }
    ci.bVisible = (CursorData->attr == (USHORT) -1 || CursorData->attr == 0) ?
        FALSE : TRUE ;
    if ( !SetConsoleCursorInfo( VioHandle, &ci ) )
    {
        DWORD error = GetLastError();
        debug(F101,"SetConsoleCursorInfo fails","",error ) ;
        if ( error == ERROR_INVALID_HANDLE ) {
            debug(F101,"SetCurType VioHandle","",VioHandle);
            VioHandle = GetStdHandle( STD_OUTPUT_HANDLE );
            debug(F101,"SetCurType GetStdHandle(STD_OUTPUT_HANDLE)","",VioHandle);
        }
        return 3;
    }

#else /* just OS/2 */
    VIOCURSORINFO ci ;
    ci.yStart = CursorData->yStart ;
    ci.cEnd   = CursorData->cEnd;
    ci.cx     = CursorData->cx ;
    ci.attr   = CursorData->attr ;
    rc = VioSetCurType( &ci, VioHandle ) ;
#endif /* NT */
    return rc ;
}

BOOL
IsOS2FullScreen( void )
{
#ifdef NT
   /* if Windows 95 then */
#ifdef COMMENT
    return (isWin95());
#else /* COMMENT */
    return FALSE ;
#endif /* COMMENT */
#else /* NT */
    VIOFONTINFO fi;
    static BOOL fs = 2 ;

    if ( fs == 2 ) {
        memset( &fi, 0, sizeof(fi) ) ;
        fi.cb = sizeof(fi) ;
        fs = ( VioGetFont( &fi, VioHandle ) != ERROR_VIO_EXTENDED_SG ) ;
    }
    return fs ;
#endif /* NT */
}


/*---------------------------------------------------------------------------*/
/* Set132Cols                                                                */
/*---------------------------------------------------------------------------*/
USHORT
Set132Cols( int mode )
{
    USHORT rc = 0 ;
#ifdef KUI
    VscrnSetWidth( mode, 132 );
#else /* KUI */
    CK_VIDEOMODEINFO mi;
    extern int tt_modechg;

    if ( tt_modechg == TVC_DIS ) {
        VscrnSetWidth( mode, 132 );
    } else {
        if (GetMode( &mi ))
            return 1;

        if (tt_modechg == TVC_ENA && mi.col == 132 &&
             mi.row == VscrnGetHeight(mode) )
            return rc ;

        if (tt_modechg == TVC_W95 &&
             mi.row == VscrnGetHeight(mode) ) {
            VscrnSetWidth(mode,132);
            return rc ;
        }

#ifdef OS2ONLY
        if ( IsOS2FullScreen() ) {
            if (tt_modechg == TVC_ENA)
                mi.col = 132 ;
            mi.row = VscrnGetHeight(mode) ;
            mi.hres = 1056 ;
            if ( mode == vmode )
            rc = SetMode( &mi ) ;
        }
        else
#endif /* OS2ONLY */
        {
            if (tt_modechg == TVC_ENA)
                mi.col = 132 ;
            mi.row = VscrnGetHeight(mode);
#ifdef NT
            mi.sbcol = mi.col;
            mi.sbrow = mi.row;
#endif /* NT */
#ifdef OS2ONLY
            mi.hres = 0 ;
            mi.vres = 0 ;
#endif /* OS2ONLY */
            if ( mode == vmode )
            rc = SetMode( &mi ) ;
        }

        if (mode == vmode) {
            if (GetMode( &mi ))
                return 1;
        }


		if (mode == VTERM)
			VscrnSetWidth( VTERM, 132 );
		else if ( tt_modechg == TVC_ENA )
            VscrnSetWidth( mode, mi.col );
        else if ( tt_modechg == TVC_W95 )
            VscrnSetWidth(mode, 132);
        VscrnSetHeight( mode, mi.row );
    }
#endif /* KUI */
    VscrnIsDirty(mode) ;
    return rc ;
}

/*---------------------------------------------------------------------------*/
/* Set80Cols                                                                 */
/*---------------------------------------------------------------------------*/
USHORT
Set80Cols( int mode )
{
    USHORT rc = 0 ;
#ifdef KUI
    VscrnSetWidth( mode, 80 );
#else /* KUI */
    CK_VIDEOMODEINFO mi;
    extern int tt_modechg;

    if ( tt_modechg == TVC_DIS ) {
        VscrnSetWidth( mode, 80 );
    } else {
        if (GetMode( &mi ))
            return 1;

        if (tt_modechg == TVC_ENA && mi.col == 80 &&
             mi.row == VscrnGetHeight(mode) )
            return rc ;

        if (tt_modechg == TVC_W95 &&
             mi.row == VscrnGetHeight(mode) ) {
            VscrnSetWidth(mode,80);
            return rc ;
        }

#ifdef OS2ONLY
        if ( IsOS2FullScreen() ) {
            if (tt_modechg == TVC_ENA)
                mi.col = 80 ;
            mi.row = VscrnGetHeight(mode) ;
            mi.hres = 720 ;
            if (mode == vmode)
                rc = SetMode( &mi ) ;
        }
        else
#endif /* OS2ONLY */
        {
            if (tt_modechg == TVC_ENA)
                mi.col = 80 ;
            mi.row = VscrnGetHeight(mode);
#ifdef NT
            mi.sbcol = mi.col;
            mi.sbrow = mi.row;
#endif /* NT */
#ifdef OS2ONLY
            mi.hres = 0 ;
            mi.vres = 0 ;
#endif /* OS2ONLY */
            if (mode == vmode)
                rc = SetMode( &mi ) ;
        }

        if (mode == vmode) {
            if (GetMode( &mi ))
                return 1;
        }

		if ( mode == VTERM )
			VscrnSetWidth( VTERM, 80 );
		else if ( tt_modechg == TVC_ENA )
            VscrnSetWidth( mode, mi.col );
        else if ( tt_modechg == TVC_W95 )
            VscrnSetWidth( mode, 80 );
        VscrnSetHeight( mode, mi.row ) ;
    }
#endif /* KUI */
    VscrnIsDirty(mode) ;

    return rc ;
}

/*---------------------------------------------------------------------------*/
/* SetCols                                                                   */
/*---------------------------------------------------------------------------*/
USHORT
SetCols( int mode )
{
    USHORT rc = 0 ;
#ifdef KUI
    /* do nothing */
#else /* KUI */
    CK_VIDEOMODEINFO mi;
    extern int tt_modechg;

    if ( tt_modechg != TVC_DIS ) {
        if (GetMode( &mi ))
            return 1;

        if ((tt_modechg == TVC_ENA && mi.col == VscrnGetWidth(mode) ||
              tt_modechg == TVC_W95) &&
             mi.row == VscrnGetHeight(mode) )
            return rc ;

#ifdef OS2ONLY
        if ( IsOS2FullScreen() ) {
            if (tt_modechg == TVC_ENA)
                mi.col = VscrnGetWidth(mode)  ;
            else
                mi.col = 80 ;
            mi.row = VscrnGetHeight(mode) ;

            if ( VscrnGetWidth(mode)  == 40 ) {
                switch ( VscrnGetHeight(mode)) {
                case 25:
                    mi.hres = 360 ;
                    mi.vres = 400 ;
                    break;

                case 43:
                    mi.hres = 320 ;
                    mi.vres = 350 ;
                    break;

                case 50:
                    mi.hres = 360 ;
                    mi.vres = 400 ;
                    break;

                case 60:
                    mi.hres = 360 ;
                    mi.vres = 480 ;
                    break;
                }
            }
            else if ( VscrnGetWidth(mode)  == 80 ) {
                switch ( VscrnGetHeight(mode) ) {
                case 25:
                    mi.hres = 720 ;
                    mi.vres = 400 ;
                    break;

                case 43:
                    mi.hres = 640 ;
                    mi.vres = 350 ;
                    break;

                case 50:
                    mi.hres = 720 ;
                    mi.vres = 400 ;
                    break;

                case 60:
                    mi.hres = 720 ;
                    mi.vres = 480 ;
                    break;
                }
            }
            else if ( VscrnGetWidth(mode)  == 132 ) {
                switch ( VscrnGetHeight(mode) ) {
                case 25:
                    mi.hres = 1056 ;
                    mi.vres = 400 ;
                    break;

                case 43:
                    mi.hres = 1056 ;
                    mi.vres = 350 ;
                    break;

                case 50:
                    mi.hres = 1056 ;
                    mi.vres = 400 ;
                    break;

                case 60:
                    mi.hres = 1056 ;
                    mi.vres = 480 ;
                    break;
                }
            }
            if ( vmode == mode )
            rc = SetMode( &mi ) ;
        }
        else
#endif /* OS2ONLY */
        {
            if ( tt_modechg == TVC_ENA )
                mi.col = VscrnGetWidth(mode)  ;
            else
                mi.col = 80;
            mi.row = VscrnGetHeight(mode);
#ifdef NT
            mi.sbcol = mi.col;
            mi.sbrow = mi.row;
#endif /* NT */
#ifdef OS2ONLY
            mi.hres = 0 ;
            mi.vres = 0 ;
#endif /* OS2ONLY */
            if ( vmode == mode )
            rc = SetMode( &mi ) ;
        }

        if (mode == vmode) {
            if (GetMode( &mi ))
                return 1;
        }

        if (tt_modechg == TVC_ENA)
            VscrnSetWidth( mode, mi.col ) ;
        VscrnSetHeight( mode, mi.row );
    }
#endif /* KUI */
    VscrnIsDirty(mode) ;

    return rc ;
}

#ifdef OLDDIRTY
static int isdirty[VNUM] = {0,0,0,0} ;
#endif

/*---------------------------------------------------------------------------*/
/* VscrnIsDirty                                                              */
/*---------------------------------------------------------------------------*/
APIRET
VscrnIsDirty( int vmode )
{
    APIRET rc = 0 ;
    extern int k95stdout;
    if ( k95stdout )
        return 0;
#ifdef OLDDIRTY
   RequestVscrnDirtyMutex( vmode, SEM_INDEFINITE_WAIT );
   if ( !isdirty[vmode] )
   {
      isdirty[vmode]++;
      rc = PostVscrnDirtySem(vmode) ;
   }
   ReleaseVscrnDirtyMutex(vmode);
#else
    rc = PostVscrnDirtySem(vmode) ;
#endif 
   return rc ;
}


APIRET
IsVscrnDirty( int vmode )
{
    extern int k95stdout;
    if ( k95stdout )
        return 0;
#ifdef OLDDIRTY
    return isdirty[vmode];
#else
    return WaitVscrnDirtySem( vmode, 0 ) ? 0 : 1;
#endif
}

APIRET
VscrnClean( int vmode )
{
    APIRET rc = 0 ;
    extern int k95stdout;
    if ( k95stdout )
        return 0;
#ifdef OLDDIRTY
    RequestVscrnDirtyMutex( vmode, SEM_INDEFINITE_WAIT );
    rc = isdirty[vmode] ;
    ResetVscrnDirtySem(vmode) ;
    isdirty[vmode] = 0 ;
    ReleaseVscrnDirtyMutex(vmode);
#else
    rc = IsVscrnDirty(vmode);
    ResetVscrnDirtySem(vmode);
#endif 
    return rc ;
}
/*---------------------------------------------------------------------------*/
/* VscrnScrollLf                                                             */
/*---------------------------------------------------------------------------*/
USHORT
VscrnScrollLf( BYTE vmode, USHORT TopRow, USHORT LeftCol, USHORT BotRow,
                 USHORT RightCol, USHORT Columns, viocell Cell )
{
    /* all position values above start at zero like Vio functions */
    SHORT x=0,y=0 ;
    videoline * line = NULL ;

    if ( Columns == 0 )
        return NO_ERROR ;
    if ( BotRow > VscrnGetHeight(vmode)- (tt_status[vmode]?2:1) || TopRow > BotRow )
        return ERROR_VIO_ROW ;
    if ( RightCol > VscrnGetWidth(vmode) -1 || LeftCol > RightCol )
        return ERROR_VIO_COL ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return(3);

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;

    for ( y = TopRow ; y <= BotRow ; y++ ) {
        line = &vscrn[vmode].lines[(vscrn[vmode].top+y)%vscrn[vmode].linecount] ;
        for ( x = LeftCol ; x <= RightCol ; x++ ){
            if ( x - Columns < LeftCol )
                continue ;
            line->cells[x-Columns] = line->cells[x] ;
            line->vt_char_attrs[x-Columns] = line->vt_char_attrs[x];
            }
        for ( x = RightCol - Columns + 1 ; x <= RightCol ; x++ ){
            line->cells[x] = Cell ;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        }

    ReleaseVscrnMutex( vmode ) ;
    return NO_ERROR ;
}

/*---------------------------------------------------------------------------*/
/* VscrnScrollRt                                                             */
/*---------------------------------------------------------------------------*/
USHORT
VscrnScrollRt( BYTE vmode, USHORT TopRow, USHORT LeftCol, USHORT BotRow,
                 USHORT RightCol, USHORT Columns, viocell Cell )
{
    /* all position values above start at zero like Vio functions */
    SHORT x=0,y=0 ;
    videoline * line = NULL ;

    if ( Columns == 0 )
        return NO_ERROR ;
    if ( BotRow > VscrnGetHeight(vmode)-(tt_status[vmode]?2:1) || TopRow > BotRow )
        return ERROR_VIO_ROW ;
    if ( RightCol > VscrnGetWidth(vmode) -1 || LeftCol > RightCol )
        return ERROR_VIO_COL ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return(3);

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;

    for ( y = TopRow ; y <= BotRow ; y++ ) {
        line = &vscrn[vmode].lines[(vscrn[vmode].top+y)%vscrn[vmode].linecount] ;
        for ( x = RightCol ; x >= LeftCol ; x-- ){
            if ( x + Columns > RightCol )
                continue ;
            line->cells[x+Columns] = line->cells[x] ;
            line->vt_char_attrs[x+Columns] = line->vt_char_attrs[x];
            }
        for ( x = LeftCol + Columns - 1 ; x >= LeftCol ; x-- ){
            line->cells[x] = Cell ;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        }

    ReleaseVscrnMutex( vmode ) ;
    return NO_ERROR ;
}

/*---------------------------------------------------------------------------*/
/* VscrnScrollDn                                                             */
/*---------------------------------------------------------------------------*/
/* WARNING - Up and Dn not fully implemented */

USHORT
VscrnScrollDn( BYTE vmode, USHORT TopRow, USHORT LeftCol, USHORT BotRow,
                 USHORT RightCol, USHORT Rows, viocell Cell )
{
    /* all position values above start at zero like Vio functions */
    SHORT x=0,y=0 ;
    videoline * toline = NULL, * fromline = NULL ;

    if ( Rows == 0 )
        return NO_ERROR ;
    if ( BotRow > VscrnGetHeight(vmode)-(tt_status[vmode]?2:1) || TopRow > BotRow )
        return ERROR_VIO_ROW ;
    if ( RightCol > VscrnGetWidth(vmode) -1 || LeftCol > RightCol )
        return ERROR_VIO_COL ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return(3);

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;

    for ( y = BotRow ; y >= TopRow ; y-- ){
        if ( y + Rows > BotRow )
            continue ;
        toline = &vscrn[vmode].lines[(vscrn[vmode].top+y-Rows)%vscrn[vmode].linecount] ;
        fromline = &vscrn[vmode].lines[(vscrn[vmode].top+y)%vscrn[vmode].linecount] ;
        for ( x = LeftCol ; x <= RightCol ; x++ ) {
            toline->cells[x] = fromline->cells[x] ;
            toline->vt_char_attrs[x] = fromline->vt_char_attrs[x];
        }
    }
    for ( y = TopRow + Rows - 1 ; y >= TopRow ; y-- ){
        toline = &vscrn[vmode].lines[(vscrn[vmode].top+y)%vscrn[vmode].linecount] ;
        for ( x = LeftCol ; x <= RightCol ; x++ ) {
            toline->cells[x] = Cell ;
            toline->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
    }

    ReleaseVscrnMutex( vmode ) ;

    return NO_ERROR ;
}

/*---------------------------------------------------------------------------*/
/* VscrnScrollUp                                                             */
/*---------------------------------------------------------------------------*/
USHORT
VscrnScrollUp( BYTE vmode, USHORT TopRow, USHORT LeftCol, USHORT BotRow,
                 USHORT RightCol, USHORT Rows, viocell Cell )
{
    /* all position values above start at zero like Vio functions */
    SHORT x=0,y=0 ;
    videoline * toline = NULL, * fromline = NULL ;

    if ( Rows == 0 )
        return NO_ERROR ;
    if ( BotRow > VscrnGetHeight(vmode)-(tt_status[vmode]?2:1) || TopRow > BotRow )
        return ERROR_VIO_ROW ;
    if ( RightCol > VscrnGetWidth(vmode) -1 || LeftCol > RightCol )
        return ERROR_VIO_COL ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return(3);

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;

    for ( y = TopRow ; y <= BotRow ; y++ ){
        if ( y - Rows < TopRow )
            continue ;
        toline = &vscrn[vmode].lines[(vscrn[vmode].top+y-Rows)%vscrn[vmode].linecount] ;
        fromline = &vscrn[vmode].lines[(vscrn[vmode].top+y)%vscrn[vmode].linecount] ;
        for ( x = LeftCol ; x <= RightCol ; x++ ) {
            toline->cells[x] = fromline->cells[x] ;
            toline->vt_char_attrs[x] = fromline->vt_char_attrs[x];
            }
    }
    for ( y = BotRow - Rows + 1 ; y <= BotRow ; y++ ){
        toline = &vscrn[vmode].lines[(vscrn[vmode].top+y)%vscrn[vmode].linecount] ;
        for ( x = LeftCol ; x <= RightCol ; x++ ) {
            toline->cells[x] = Cell ;
            toline->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
    }

    ReleaseVscrnMutex( vmode ) ;

    return NO_ERROR ;
}

/*---------------------------------------------------------------------------*/
/* VscrnWrtCell                                                              */
/*---------------------------------------------------------------------------*/
USHORT
VscrnWrtCell( BYTE vmode, viocell Cell, vtattrib att, USHORT Row, USHORT Col )
{
    int i ;
    videoline * line ;
    unsigned char cellcolor = geterasecolor(vmode);

    if ( Row < 0 || Row > VscrnGetHeight(vmode)-(tt_status[vmode]?2:1) )
        return ERROR_VIO_ROW ;

    if ( Col < 0 || Col > VscrnGetWidth(vmode) -1 )
        return ERROR_VIO_COL ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    line = VscrnGetLineFromTop( vmode, Row ) ;

    if (line->width == 0) {
        line->width = VscrnGetWidth(vmode)  ;
        for ( i=0 ; i<MAXTERMCOL  ; i++ ) {
            line->cells[i].c = ' ' ;
            line->cells[i].a = cellcolor ;
            line->vt_char_attrs[i] = VT_CHAR_ATTR_NORMAL ;
        }
    }

    line->cells[Col] = Cell ;
    line->vt_char_attrs[Col] = VT_CHAR_ATTR_NORMAL |
        (att.bold       ? VT_CHAR_ATTR_BOLD      : 0) |
        (att.dim        ? VT_CHAR_ATTR_DIM       : 0) |
        (att.underlined ? VT_CHAR_ATTR_UNDERLINE : 0) |
        (att.blinking   ? VT_CHAR_ATTR_BLINK     : 0) |
        (att.reversed   ? VT_CHAR_ATTR_REVERSE   : 0) |
        (att.italic     ? VT_CHAR_ATTR_ITALIC    : 0) |
        (att.invisible  ? VT_CHAR_ATTR_INVISIBLE : 0) |
        (att.unerasable ? VT_CHAR_ATTR_PROTECTED : 0) |
        (att.graphic    ? VT_CHAR_ATTR_GRAPHIC   : 0) |
        (att.hyperlink  ? VT_CHAR_ATTR_HYPERLINK : 0) |
        (att.wyseattr   ? WY_CHAR_ATTR         : 0) ;
    line->hyperlinks[Col] = att.hyperlink ? att.linkid : 0;
    return NO_ERROR ;
}

/*---------------------------------------------------------------------------*/
/* VscrnWrtCharStrAtt                                                        */
/*---------------------------------------------------------------------------*/
USHORT
VscrnWrtCharStrAtt( BYTE vmode, PCH CharStr, USHORT Length,
                    USHORT Row, USHORT Column, PBYTE Attr )
{
    USHORT rc = 0;
    int i ;
    int height;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    height = VscrnGetHeight(vmode) - (tt_status[vmode]?1:0);

        if (height <= 0)
                return(-1);

    /* write to a Vscrn function */
    for ( i = 0 ; i < Length ; i++ )
    {
        extern int tcsl;
        viocell cell;

        if ( CharStr[i] >= ' ' ) {
            if ( isunicode() ) {
                cell.c = CharStr[i] >= 128 ?
                    (*xl_u[tcsl])(CharStr[i]) : CharStr[i] ;
            }
            else
                cell.c = CharStr[i];
            cell.a = *Attr;

            if (Column > VscrnGetWidth(vmode==VSTATUS?VTERM:vmode)) {
                Column = 1;
                if (Row >= height) {
                    VscrnScroll(vmode,UPWARD, 0, height-1, 1, 1, SP) ;
                    Row = height;
                } else {
                    Row++;
                }
            }
            VscrnWrtCell(vmode, cell, (vmode == VTERM ? attrib : cmdattrib),
                          Row - 1, Column - 1);
            Column++;
        }
        else {                          /* Control character */
            switch (CharStr[i]) {
            case LF:
                if (Row >= height) {
                    VscrnScroll(vmode,UPWARD, 0, height-1, 1, 1, SP);
                    Row = height;
                } else {
                    Row++;
                }
                Column = 1 ;
                break;
            case CR:
                Column = 1 ;
                break;
            case BS:
                if (Column > 1)
                  Column--;
                break;
            case 12:
                if (Column < VscrnGetWidth(vmode))
                  Column++;
                break;
            case BEL:
#ifdef IKSD
                if ( !inserver )                /* Leads to infinite loop */
#endif /* IKSD */
                    bleep(BP_NOTE);
                break;
            case 9: /* Tab */
                Column += 8-(Column%8) ;
                break;
            default: {           /* Don't ignore */
                if ( isunicode() ) {
                    cell.c = (*xl_u[TX_IBMC0GRPH])(CharStr[i]);
                }
                else
                    cell.c = CharStr[i];
                cell.a = *Attr;

                VscrnWrtCell(vmode, cell, (vmode == VTERM ? attrib : cmdattrib),
                              Row - 1, Column - 1);
                if (++Column > VscrnGetWidth(vmode)) {
                    Column = 1;
                    if (Row >= height) {
                        VscrnScroll(vmode,UPWARD, 0,height-1, 1, 1, SP) ;
                        Row = height;
                    }
                    else {
                        Row++;
                    }
                }
            }
            }
        }
    }
    lgotoxy(vmode,Column,Row);
    VscrnIsDirty(vmode);
    return rc ;
}


/*---------------------------------------------------------------------------*/
/* VscrnWrtCharStrAtt                                                        */
/*---------------------------------------------------------------------------*/
USHORT
VscrnWrtUCS2StrAtt( BYTE vmode, PUSHORT UCS2Str, USHORT Length,
                    USHORT Row, USHORT Column, PBYTE Attr )
{
    USHORT rc = 0;
    int i ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* write to a Vscrn function */
    for ( i = 0 ; i < Length ; i++ )
    {
        extern int tcsl;
        viocell cell;

        if ( UCS2Str[i] >= ' ' ) {
            if ( !isunicode() ) {
                cell.c = UCS2Str[i] >= 128 ?
                    (*xl_tx[tcsl])(UCS2Str[i]) : UCS2Str[i] ;
            }
            else
                cell.c = UCS2Str[i];
            cell.a = *Attr;

            if (Column > VscrnGetWidth(vmode==VSTATUS?VTERM:vmode)) {
                Column = 1;
                if (Row == VscrnGetHeight(vmode)
                     -(tt_status[vmode]?1:0)) {
                    VscrnScroll(vmode,UPWARD, 0,
                                 VscrnGetHeight(vmode)
                                 -(tt_status[vmode]?2:1), 1, 1, SP) ;
                } else {
                    Row++;
                    if (Row == VscrnGetHeight(vmode)
                         -(tt_status[vmode]?1:0))
                      Row--;
                }
            }
            VscrnWrtCell(vmode, cell, (vmode == VTERM ? attrib : cmdattrib),
                          Row - 1, Column - 1);
                        Column++;
        }
        else {                          /* Control character */
            switch (UCS2Str[i]) {
            case LF:
                if (Row == VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)) {
                    VscrnScroll(vmode,UPWARD, 0,
                                 VscrnGetHeight(vmode)-(tt_status[vmode]?2:1),
                                 1, 1, SP) ;
                } else {
                    Row++;
                    if (Row > VscrnGetHeight(vmode)-(tt_status[vmode]?1:0))
                      Row = VscrnGetHeight(vmode)-(tt_status[vmode]?1:0);
                }
                Column = 1 ;
                break;
            case CR:
                Column = 1 ;
                break;
            case BS:
                if (Column > 1)
                  Column--;
                break;
            case 12:
                if (Column < VscrnGetWidth(vmode))
                  Column++;
                break;
            case BEL:
#ifdef IKSD
                if ( !inserver )                /* Leads to infinite loop */
#endif /* IKSD */
                    bleep(BP_NOTE);
                break;
            case 9: /* Tab */
                Column += 8-(Column%8) ;
                break;
            default:{           /* Don't ignore */
                if ( isunicode() ) {
                    cell.c = (*xl_u[TX_IBMC0GRPH])(UCS2Str[i]);
                }
                else
                    cell.c = UCS2Str[i];
                cell.a = *Attr;

                VscrnWrtCell(vmode, cell, (vmode == VTERM ? attrib : cmdattrib),
                              Row - 1, Column - 1);
                if (++Column > VscrnGetWidth(vmode)) {
                    Column = 1;
                    if (Row == VscrnGetHeight(vmode)
                         -(tt_status[vmode]?1:0)) {
                        VscrnScroll(vmode,UPWARD, 0,
                                     VscrnGetHeight(vmode)
                                     -(tt_status[vmode]?2:1), 1, 1, SP) ;
                    }
                    else {
                        Row++;
                        if (Row == VscrnGetHeight(vmode)
                             -(tt_status[vmode]?1:0))
                            Row--;
                    }
                }
            }
            }
        }
    }
    lgotoxy(vmode,Column,Row);
    VscrnIsDirty(vmode);
    return rc ;
}


/*---------------------------------------------------------------------------*/
/* VscrnGetLineFromTop                                                       */
/*---------------------------------------------------------------------------*/
videoline *
VscrnGetLineFromTop( BYTE vmode, SHORT y )  /* zero based */
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return(NULL);

    while ( y < 0 )
        y += vscrn[vmode].linecount ;

    return &vscrn[vmode].lines[(vscrn[vmode].top+y)%vscrn[vmode].linecount] ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetLine                                                              */
/*---------------------------------------------------------------------------*/
videoline *
VscrnGetLine( BYTE vmode, SHORT y )  /* zero based */
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return(NULL);

    while ( y < 0 )
        y += vscrn[vmode].linecount ;

    return &vscrn[vmode].lines[y%vscrn[vmode].linecount] ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetWidth                                                             */
/*---------------------------------------------------------------------------*/
VOID
VscrnSetWidth( BYTE vmode, int width )
{
    int y=0;
    videoline * pline = NULL;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return;

    vscrn[vmode].width = width ;

    if (vscrn[vmode].linecount && vscrn[vmode].lines != NULL) {
        for ( y=0;y<vscrn[vmode].height;y++ ) {
            pline = &vscrn[vmode].lines[(vscrn[vmode].top+y)%vscrn[vmode].linecount];
            pline->width = width;
        }
    }
#ifdef KUI
    {
        int cm = IsConnectMode();
        if ( cm && vmode == VTERM || !cm && vmode == VCMD ) {
            char buf[30];
            ckmakmsg(buf,30,ckitoa(vscrn[vmode].width)," x ",
                      ckitoa(vscrn[vmode].height-(tt_status[vmode]?1:0)),NULL);
            KuiSetTerminalStatusText(STATUS_HW, buf);
        }
    }
#endif /* KUI */
}

/*---------------------------------------------------------------------------*/
/* VscrnSetHeight                                                            */
/*---------------------------------------------------------------------------*/
VOID
VscrnSetHeight( BYTE vmode, int height )
{

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    vscrn[vmode].height = height ;
#ifdef KUI
    {
        int cm = IsConnectMode();
        if ( cm && vmode == VTERM || !cm && vmode == VCMD ) {
            char buf[30];
            ckmakmsg(buf,30,ckitoa(vscrn[vmode].width)," x ",
                      ckitoa(vscrn[vmode].height-(tt_status[vmode]?1:0)),NULL);
            KuiSetTerminalStatusText(STATUS_HW, buf);
        }
    }
#endif /* KUI */
}

/*---------------------------------------------------------------------------*/
/* VscrnSetDisplayHeight                                                            */
/*---------------------------------------------------------------------------*/
VOID
VscrnSetDisplayHeight( BYTE vmode, int height )
{

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    vscrn[vmode].display_height = height ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetWidth                                                             */
/*---------------------------------------------------------------------------*/
int
VscrnGetWidth( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

   return vscrn[vmode].width ? vscrn[vmode].width : MAXTERMCOL;
}
/*---------------------------------------------------------------------------*/
/* VscrnGetHeight                                                            */
/*---------------------------------------------------------------------------*/
int
VscrnGetHeight( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

   return vscrn[vmode].height ? vscrn[vmode].height : MAXTERMROW;
}
/*---------------------------------------------------------------------------*/
/* VscrnGetDisplayHeight                                                            */
/*---------------------------------------------------------------------------*/
int
VscrnGetDisplayHeight( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

   return vscrn[vmode].display_height ? vscrn[vmode].display_height : VscrnGetHeight(vmode);
}

/*---------------------------------------------------------------------------*/
/* VscrnSetBookmark                                                          */
/*---------------------------------------------------------------------------*/
VOID
VscrnSetBookmark( BYTE vmode, int mark, int linenum )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

   vscrn[vmode].bookmark[mark] = linenum ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetBookmark                                                          */
/*---------------------------------------------------------------------------*/
int
VscrnGetBookmark( BYTE vmode, int mark )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

   return vscrn[vmode].bookmark[mark];
}
/*---------------------------------------------------------------------------*/
/* VscrnGetLineVtAttr                                                        */
/*---------------------------------------------------------------------------*/
USHORT
VscrnGetLineVtAttr( BYTE vmode, SHORT y ) /* zero based */
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return VscrnGetLineFromTop(vmode,y)->vt_line_attr ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetLineVtAttr                                                        */
/*---------------------------------------------------------------------------*/
USHORT
VscrnSetLineVtAttr( BYTE vmode, SHORT y, USHORT attr ) /* zero based */
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return (VscrnGetLineFromTop(vmode,y)->vt_line_attr = attr) ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetCells                                                             */
/*---------------------------------------------------------------------------*/
viocell *
VscrnGetCells( BYTE vmode, SHORT y )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return VscrnGetLineFromTop(vmode,y)->cells ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetCell                                                              */
/*---------------------------------------------------------------------------*/
viocell *
VscrnGetCell( BYTE vmode, SHORT x, SHORT y )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return &VscrnGetLineFromTop(vmode,y)->cells[x] ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetVtCharAttr                                                        */
/*---------------------------------------------------------------------------*/
vtattrib
VscrnGetVtCharAttr( BYTE vmode, SHORT x, SHORT y )
{
    static vtattrib vta ;
    USHORT attr ;
    videoline * line;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    line = VscrnGetLineFromTop(vmode,y);
    attr = line->vt_char_attrs[x] ;

    vta.bold            = attr & VT_CHAR_ATTR_BOLD ? 1 : 0 ;
    vta.dim             = attr & VT_CHAR_ATTR_DIM ? 1 : 0 ;
    vta.underlined      = attr & VT_CHAR_ATTR_UNDERLINE ? 1 : 0 ;
    vta.blinking        = attr & VT_CHAR_ATTR_BLINK ? 1 : 0 ;
    vta.reversed        = attr & VT_CHAR_ATTR_REVERSE ? 1 : 0 ;
    vta.italic          = attr & VT_CHAR_ATTR_ITALIC ? 1 : 0 ;
    vta.invisible       = attr & VT_CHAR_ATTR_INVISIBLE ? 1 : 0 ;
    vta.unerasable      = attr & VT_CHAR_ATTR_PROTECTED ? 1 : 0 ;
    vta.graphic         = attr & VT_CHAR_ATTR_GRAPHIC ? 1 : 0 ;
    vta.wyseattr        = attr & WY_CHAR_ATTR ? 1 : 0 ;
    vta.hyperlink       = attr & VT_CHAR_ATTR_HYPERLINK ? 1 : 0;
    vta.linkid          = attr & VT_CHAR_ATTR_HYPERLINK ? line->hyperlinks[x] : 0;

    return vta;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetVtCharAttr                                                        */
/*---------------------------------------------------------------------------*/
USHORT
VscrnSetVtCharAttr( BYTE vmode, SHORT x, SHORT y, vtattrib vta )
{
    USHORT attr ;
    videoline * line;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    attr = VT_CHAR_ATTR_NORMAL |
                (vta.bold       ? VT_CHAR_ATTR_BOLD      : 0) |
                (vta.dim        ? VT_CHAR_ATTR_DIM       : 0) |
                (vta.underlined ? VT_CHAR_ATTR_UNDERLINE : 0) |
                (vta.blinking   ? VT_CHAR_ATTR_BLINK     : 0) |
                (vta.reversed   ? VT_CHAR_ATTR_REVERSE   : 0) |
                (vta.italic     ? VT_CHAR_ATTR_ITALIC    : 0) | 
                (vta.invisible  ? VT_CHAR_ATTR_INVISIBLE : 0) |
                (vta.unerasable ? VT_CHAR_ATTR_PROTECTED : 0) |
                (vta.graphic    ? VT_CHAR_ATTR_GRAPHIC   : 0) |
                (vta.hyperlink  ? VT_CHAR_ATTR_HYPERLINK : 0) |
                (vta.wyseattr   ? WY_CHAR_ATTR         : 0) ;
    line = VscrnGetLineFromTop(vmode,y);
    line->vt_char_attrs[x] = attr;
    line->hyperlinks[x] = vta.linkid;
    return attr;
}

/*---------------------------------------------------------------------------*/
/* VscrnMoveTop                                                              */
/*---------------------------------------------------------------------------*/
LONG
VscrnMoveTop( BYTE vmode, LONG y )
{
    LONG newtop, beg, end ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    end = vscrn[vmode].end ;
    beg = vscrn[vmode].beg ;
    newtop = vscrn[vmode].top + y ;

    if ( beg > end ) {
        end += vscrn[vmode].linecount ;
        if ( beg > vscrn[vmode].top )
            newtop += vscrn[vmode].linecount ;
        }

    if ( newtop < beg || newtop > end
         - VscrnGetHeight(vmode) + (tt_status[vmode]?2:2) )
        return -1 ;

    while ( newtop < 0 )
        newtop += vscrn[vmode].linecount ;

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].top = (newtop)%vscrn[vmode].linecount ;
    ReleaseVscrnMutex( vmode );
    return vscrn[vmode].top ;
}

/*---------------------------------------------------------------------------*/
/* VscrnMoveScrollTop                                                        */
/*---------------------------------------------------------------------------*/
LONG
VscrnMoveScrollTop( BYTE vmode, LONG y )
{
    LONG newscrolltop, top, beg, end ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if (!scrollflag[vmode])
        vscrn[vmode].scrolltop = vscrn[vmode].top ;

    end = vscrn[vmode].end ;
    beg = vscrn[vmode].beg ;
    top = vscrn[vmode].top ;
    newscrolltop = vscrn[vmode].scrolltop + y ;

    if ( beg > end ) {
        end += vscrn[vmode].linecount ;
        if ( beg > vscrn[vmode].top )
            top += vscrn[vmode].linecount ;
        if ( beg > vscrn[vmode].scrolltop )
            newscrolltop += vscrn[vmode].linecount ;
        }

    debug(F111,"VscrnMoveScrollTop","newscrolltop",newscrolltop);
    debug(F111,"VscrnMoveScrollTop","end---",
           end-(VscrnGetHeight(vmode)-(tt_status[vmode]?1:0))+1);
    if ( newscrolltop < beg ||
         newscrolltop > end
         - (VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)) + 1)
        return -1 ;

    while ( newscrolltop < 0 )
        newscrolltop += vscrn[vmode].linecount ;

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].scrolltop = (newscrolltop)%vscrn[vmode].linecount ;
    ReleaseVscrnMutex( vmode ) ;
    return vscrn[vmode].scrolltop ;
}

/*---------------------------------------------------------------------------*/
/* VscrnMoveBegin                                                            */
/*---------------------------------------------------------------------------*/
LONG
VscrnMoveBegin( BYTE vmode, LONG y )
{
    if (y<0)
        return -1 ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].beg = (vscrn[vmode].beg + y)%vscrn[vmode].linecount ;
    ReleaseVscrnMutex( vmode ) ;
    return vscrn[vmode].beg ;
}

/*---------------------------------------------------------------------------*/
/* VscrnMoveEnd                                                              */
/*---------------------------------------------------------------------------*/
LONG
VscrnMoveEnd( BYTE vmode, LONG y )
{
    LONG newend ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if (y<0)
        return -1 ;

    newend = vscrn[vmode].end + y ;
    if ( vscrn[vmode].beg < vscrn[vmode].end ) {
        if ( newend >= vscrn[vmode].linecount )
            vscrn[vmode].beg = newend - vscrn[vmode].linecount ;
        }
    else {
        vscrn[vmode].beg = (vscrn[vmode].beg + y)%vscrn[vmode].linecount ;
        }

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].end = (newend)%vscrn[vmode].linecount ;
    ReleaseVscrnMutex( vmode ) ;
    return vscrn[vmode].end ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetLineWidth                                                         */
/*---------------------------------------------------------------------------*/
UCHAR
VscrnGetLineWidth( BYTE vmode, SHORT y )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return VscrnGetLineFromTop(vmode,y)->width ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetTop                                                               */
/*---------------------------------------------------------------------------*/
ULONG
VscrnGetTop( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return vscrn[vmode].top ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetScrollTop                                                         */
/*---------------------------------------------------------------------------*/
ULONG
VscrnGetScrollTop( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return vscrn[vmode].scrolltop ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetScrollTop                                                         */
/*---------------------------------------------------------------------------*/
ULONG
VscrnGetScrollHorz( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return vscrn[vmode].hscroll ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetBegin                                                             */
/*---------------------------------------------------------------------------*/
ULONG
VscrnGetBegin( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return vscrn[vmode].beg ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetEnd                                                               */
/*---------------------------------------------------------------------------*/
ULONG
VscrnGetEnd( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return vscrn[vmode].end ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetTop                                                               */
/*---------------------------------------------------------------------------*/
LONG
VscrnSetTop( BYTE vmode, LONG y )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    while ( y < 0 )
        y += vscrn[vmode].linecount ;

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].top = y%vscrn[vmode].linecount ;
    ReleaseVscrnMutex( vmode );
    return vscrn[vmode].top ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetScrollTop                                                         */
/*---------------------------------------------------------------------------*/
LONG
VscrnSetScrollTop( BYTE vmode, LONG y )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    while ( y < 0 )
        y += vscrn[vmode].linecount ;
    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].scrolltop = y%vscrn[vmode].linecount ;
    ReleaseVscrnMutex( vmode );
    return vscrn[vmode].scrolltop ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetScrollHorz                                                         */
/*---------------------------------------------------------------------------*/
LONG
VscrnSetScrollHorz( BYTE vmode, LONG h )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].hscroll = h;
    ReleaseVscrnMutex( vmode );
    return vscrn[vmode].hscroll ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetBegin                                                             */
/*---------------------------------------------------------------------------*/
LONG
VscrnSetBegin( BYTE vmode, LONG y )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    while ( y < 0 )
        y += vscrn[vmode].linecount ;
    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].beg = y%vscrn[vmode].linecount ;
    ReleaseVscrnMutex( vmode );
    return vscrn[vmode].beg ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetEnd                                                               */
/*---------------------------------------------------------------------------*/
LONG
VscrnSetEnd( BYTE vmode, LONG y )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    while ( y < 0 )
        y += vscrn[vmode].linecount ;
    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    vscrn[vmode].end = y%vscrn[vmode].linecount ;
    ReleaseVscrnMutex( vmode );
    return vscrn[vmode].end ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetBufferSize                                                        */
/*---------------------------------------------------------------------------*/
ULONG
VscrnGetBufferSize( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return vscrn[vmode].linecount ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetCurPos                                                            */
/*---------------------------------------------------------------------------*/
position *
VscrnSetCurPos( BYTE vmode, SHORT x, SHORT y )
{
#ifdef KUI_COMMENT
    char buf[30];
#endif /* KUI */

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    vscrn[vmode].cursor.x = x%VscrnGetWidth(vmode)  ;
    vscrn[vmode].cursor.y = y
      %(VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)) ;

#ifdef KUI_COMMENT
    ckmakmsg(buf,30,ckitoa(vscrn[vmode].cursor.x+1),", ",
              ckitoa(vscrn[vmode].cursor.y+1),NULL);
    KuiSetTerminalStatusText(STATUS_CURPOS, buf);
#endif /* KUI */

    return &vscrn[vmode].cursor ;
}

/*---------------------------------------------------------------------------*/
/* VscrnGetCurPos                                                            */
/*---------------------------------------------------------------------------*/
position *
VscrnGetCurPos( BYTE vmode )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    return &vscrn[vmode].cursor ;
}


static viocell *        cellmem[VNUM] = { NULL, NULL, NULL, NULL } ;
static unsigned short * attrmem[VNUM] = { NULL, NULL, NULL, NULL } ;
static unsigned short * hyperlinkmem[VNUM] = { NULL, NULL, NULL, NULL } ;

/*---------------------------------------------------------------------------*/
/* VscrnSetBufferSize                                                        */
/*---------------------------------------------------------------------------*/
ULONG
VscrnSetBufferSize( BYTE vmode, ULONG newsize )
{
    static ULONG oldsize[VNUM]={0,0,0,0} ;
    int i ;
    videobuffer TmpScrn ;
    videoline * line ;
    ULONG rc = FALSE ;  /* Determines whether clearscreen needs to be called */

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    debug(F101,"SetBufferSize","",newsize);
    debug(F101,"SetBufferSize linecount","",vscrn[vmode].linecount);

    /* Wait for exclusive access to the screen */
    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;

    if ( newsize < oldsize[vmode] ) { /* erase entire buffer and start again. */
        free(vscrn[vmode].lines) ;
        vscrn[vmode].linecount = 0 ;
        vscrn[vmode].lines = 0 ;
        vscrn[vmode].beg = 0 ;
        vscrn[vmode].top = 0 ;
        vscrn[vmode].scrolltop = 0 ;
        vscrn[vmode].end = 0 ;
        vscrn[vmode].cursor.x = vscrn[vmode].cursor.y = 0 ;
        vscrn[vmode].marktop = 0 ;
        vscrn[vmode].markbot = 0 ;
        for ( i=0 ; i < 10 ; i++ )
            vscrn[vmode].bookmark[i] = 0 ;
        free(cellmem[vmode]) ;
        cellmem[vmode]=NULL ;
        free(attrmem[vmode]) ;
        attrmem[vmode]=NULL ;
    }

    if ( vscrn[vmode].linecount == newsize ) {
        ReleaseVscrnMutex( vmode ) ;
        return rc ;
    }
    else if ( vscrn[vmode].linecount == 0 ) {
        debug( F101,"VscrnSetBufferSize malloc size","",
               newsize*sizeof(videoline) ) ;
        vscrn[vmode].linecount = newsize ;
        vscrn[vmode].lines = malloc(vscrn[vmode].linecount * sizeof(videoline)) ;
        if ( !vscrn[vmode].lines )
            fatal("VscrnSetBufferSize: unable to allocate memory for vscrn[].lines!");
        vscrn[vmode].top = 0 ;
        vscrn[vmode].scrolltop = 0 ;
        vscrn[vmode].beg = 0 ;
        vscrn[vmode].width = tt_cols[vmode] ;
        vscrn[vmode].height = tt_rows[vmode]+(tt_status[vmode]?1:0);
        vscrn[vmode].end = vscrn[vmode].height - 1 ;
        vscrn[vmode].cursor.x = 0 ;
        vscrn[vmode].cursor.y = 0 ;
        vscrn[vmode].marktop = -1 ;
        vscrn[vmode].markbot = -1 ;
        vscrn[vmode].popup = NULL ;
        for ( i=0 ; i<10 ; i++)
           vscrn[vmode].bookmark[i] = -1 ;

        cellmem[vmode] = malloc( (vscrn[vmode].linecount + 1) * MAXTERMCOL * sizeof(viocell) ) ;
        if ( !cellmem[vmode] )
            fatal("VscrnSetBufferSize: unable to allocate memory for cellmem[]!");
        attrmem[vmode] = malloc( (vscrn[vmode].linecount + 1) * MAXTERMCOL * sizeof(short) ) ;
        if ( !attrmem[vmode] )
            fatal("VscrnSetBufferSize: unable to allocate memory for attrmem[]!");
        hyperlinkmem[vmode] = malloc( (vscrn[vmode].linecount + 1) * MAXTERMCOL * sizeof(short) ) ;
        if ( !hyperlinkmem[vmode] )
            fatal("VscrnSetBufferSize: unable to allocate memory for hyperlinkmem[]!");

        for ( i=0 ; i<vscrn[vmode].linecount ; i++ ) {
            vscrn[vmode].lines[i].width = 0 ;
            vscrn[vmode].lines[i].cells = cellmem[vmode] + i * MAXTERMCOL ;
            vscrn[vmode].lines[i].vt_char_attrs = attrmem[vmode] + i * MAXTERMCOL ;
            vscrn[vmode].lines[i].vt_line_attr = VT_LINE_ATTR_NORMAL ;
            vscrn[vmode].lines[i].hyperlinks = hyperlinkmem[vmode] + i * MAXTERMCOL ;
            vscrn[vmode].lines[i].markbeg = -1 ;
            vscrn[vmode].lines[i].markshowend = -1 ;
            vscrn[vmode].lines[i].markend = -1 ;
            }

        rc = TRUE ;
        }
    else if ( vscrn[vmode].linecount < newsize ) {
        viocell *        oldcellmem = cellmem[vmode] ;
        unsigned short * oldattrmem = attrmem[vmode] ;
        unsigned short * oldhyperlinkmem = hyperlinkmem[vmode] ;

        memset(&TmpScrn,0,sizeof(TmpScrn));
        TmpScrn.linecount = newsize ;
        TmpScrn.lines = malloc(TmpScrn.linecount * sizeof(videoline)) ;
        if ( !TmpScrn.lines )
            fatal("VscrnSetBufferSize: unable to allocate memory for TmpScrn[].lines!");
        TmpScrn.beg = 0 ;
        TmpScrn.scrolltop = 0 ;
        TmpScrn.top = (vscrn[vmode].beg <= vscrn[vmode].top) ? (vscrn[vmode].top - vscrn[vmode].beg)
                            : (vscrn[vmode].top + vscrn[vmode].linecount - vscrn[vmode].beg) ;
        TmpScrn.end = (TmpScrn.top + vscrn[vmode].height - 1)%TmpScrn.linecount ;
        TmpScrn.cursor = vscrn[vmode].cursor ;
        TmpScrn.popup = vscrn[vmode].popup ;
        TmpScrn.marktop = TmpScrn.markbot = -1 ;
                TmpScrn.width = vscrn[vmode].width ;
                TmpScrn.height = vscrn[vmode].height ;
        for ( i = 0 ; i < 10 ; i++ )
           TmpScrn.bookmark[i] = (vscrn[vmode].beg <= vscrn[vmode].bookmark[i]) ?
              (vscrn[vmode].bookmark[i] - vscrn[vmode].beg) :
              ( vscrn[vmode].bookmark[i] + vscrn[vmode].linecount - vscrn[vmode].beg ) ;

        cellmem[vmode] = malloc( (TmpScrn.linecount + 1) * MAXTERMCOL * sizeof(viocell) ) ;
        if ( !cellmem[vmode] )
            fatal("VscrnSetBufferSize: unable to allocate memory for cellmem[]!");
        memcpy( cellmem[vmode], oldcellmem, (vscrn[vmode].linecount + 1)
                * MAXTERMCOL * sizeof(viocell) ) ;

        attrmem[vmode] = malloc( (TmpScrn.linecount + 1) * MAXTERMCOL * sizeof(short) ) ;
        if ( !attrmem[vmode] )
            fatal("VscrnSetBufferSize: unable to allocate memory for attrmem[]!");
        memcpy( attrmem[vmode], oldattrmem, (vscrn[vmode].linecount + 1)
                * MAXTERMCOL * sizeof(short) ) ;

        hyperlinkmem[vmode] = malloc( (TmpScrn.linecount + 1) * MAXTERMCOL * sizeof(short) ) ;
        if ( !hyperlinkmem[vmode] )
            fatal("VscrnSetBufferSize: unable to allocate memory for hyperlinkmem[]!");
        memcpy( hyperlinkmem[vmode], oldhyperlinkmem, (vscrn[vmode].linecount + 1)
                * MAXTERMCOL * sizeof(short) ) ;

        for ( i=0 ; i<vscrn[vmode].linecount ; i++ ) {
            line = &vscrn[vmode].lines[(vscrn[vmode].beg+i)%vscrn[vmode].linecount] ;
            TmpScrn.lines[i].width = line->width ;
            line->width = 0 ;
            TmpScrn.lines[i].cells = line->cells + (cellmem[vmode] - oldcellmem) ;
            line->cells = NULL ;
            TmpScrn.lines[i].vt_char_attrs = line->vt_char_attrs
                + (attrmem[vmode] - oldattrmem);
            line->vt_char_attrs = 0 ;
            TmpScrn.lines[i].hyperlinks = line->hyperlinks
                + (hyperlinkmem[vmode] - oldhyperlinkmem);
            line->hyperlinks = 0 ;
            TmpScrn.lines[i].vt_line_attr = line->vt_line_attr ;
            line->vt_line_attr = 0 ;
            line->markbeg = -1 ;
            line->markshowend = -1 ;
            line->markend = -1 ;
        }

        for ( i ; i < TmpScrn.linecount ; i++ ) {
            TmpScrn.lines[i].width = 0 ;
            TmpScrn.lines[i].cells = cellmem[vmode] + (i+1) * MAXTERMCOL ;
            TmpScrn.lines[i].vt_char_attrs = attrmem[vmode] + (i+1) * MAXTERMCOL ;
            TmpScrn.lines[i].hyperlinks = hyperlinkmem[vmode] + (i+1) * MAXTERMCOL ;
            TmpScrn.lines[i].vt_line_attr = VT_LINE_ATTR_NORMAL ;
            TmpScrn.lines[i].markbeg = -1 ;
            TmpScrn.lines[i].markshowend = -1 ;
            TmpScrn.lines[i].markend = -1 ;
        }

        free(oldcellmem) ;
        free(oldattrmem) ;
        free(oldhyperlinkmem) ;
        free(vscrn[vmode].lines) ;
        vscrn[vmode] = TmpScrn ;
        }

    ReleaseVscrnMutex( vmode ) ;

    debug(F101,"SetBufferSize linecount","",vscrn[vmode].linecount);
    debug(F101,"SetBufferSize lines","",vscrn[vmode].lines);
    debug(F101,"SetBufferSize beg","",vscrn[vmode].beg);
    debug(F101,"SetBufferSize top","",vscrn[vmode].top);
    debug(F101,"SetBufferSize end","",vscrn[vmode].end);
    debug(F101,"SetBufferSize cursor.x","",vscrn[vmode].cursor.x);
    debug(F101,"SetBufferSize cursor.y","",vscrn[vmode].cursor.y);

    oldsize[vmode] = newsize ;
    return rc ;
}

/*---------------------------------------------------------------------------*/
/* VscrnScroll                                                               */
/*---------------------------------------------------------------------------*/
void
VscrnScroll(BYTE vmode, int updown, int topmargin, int bottommargin,
             int nlines, int savetobuffer, CHAR fillchar) {
    /* topmargin and bottommargin are zero based */
    viocell blankcell;
    videoline * line ;
    videoline   linetodelete ;
    int i,x;
    long  obeg, oend, otop, nbeg, nend, ntop ;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    blankcell.c = fillchar ;
    blankcell.a = cellcolor ;

    if ( updmode == TTU_SMOOTH )
        msleep(1) ;

    debug(F111,"VscrnScroll","vmode",vmode);
    debug(F111,"VscrnScroll","updown",updown);
    debug(F111,"VscrnScroll","topmargin",topmargin);
    debug(F111,"VscrnScroll","bottommargin",bottommargin);

    /* Wait for exclusive access to the screen */
    debug(F101,"VscrnScroll requests VscrnMutex","",vmode);
    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;

    debug(F101,"VscrnScroll has VscrnMutex","",vmode);
    switch (updown) {
        case UPWARD:
            if (savetobuffer && topmargin == 0) {
                if (topmargin)
                    {
                    debug(F101,"WARNING scroll: savetobuffer but topmargin not zero","",topmargin);
                    }

                while ( scrollflag[vmode] && VscrnGetBegin(vmode) == VscrnGetScrollTop(vmode)
                    && (VscrnGetEnd(vmode)+1)%VscrnGetBufferSize(vmode) == VscrnGetBegin(vmode) ) {
                    ReleaseVscrnMutex(vmode);
                    msleep(1000);  /* give up time slice and wait for room */
                    RequestVscrnMutex(vmode,SEM_INDEFINITE_WAIT);
                    }

                obeg = VscrnGetBegin(vmode) ;
                oend = VscrnGetEnd(vmode) ;
                if ( oend < obeg )
                    oend += VscrnGetBufferSize(vmode) ;
                otop = VscrnGetTop(vmode) ;
                if ( otop < obeg )
                    otop += VscrnGetBufferSize(vmode) ;

                ntop = otop + nlines ;

                if ( ntop + VscrnGetHeight(vmode)
                                        -(tt_status[vmode]?2:1) > oend ) {
                    nend = ntop + VscrnGetHeight(vmode)
                                                -(tt_status[vmode]?2:1) ;
                    VscrnSetEnd( vmode, nend ) ;
                    if ( obeg > 0 || obeg == 0 && nend >= VscrnGetBufferSize(vmode) - 1 ) {
                        nbeg = VscrnSetBegin( vmode,nend + 1 ) ;
                        }
                    }

                for ( i = 0 ; i < nlines ; i++ ) {
                    line = VscrnGetLineFromTop(vmode,VscrnGetHeight(vmode)+i
                                                -(tt_status[vmode]?1:0)) ;
                    if (line == NULL || line->cells == NULL) {
                        debug(F100,"VscrnScroll to buffer - line->cells = NULL","",0);
                        break;
                        }
                    if (line->vt_char_attrs == NULL) {
                        debug(F100,"VscrnScroll to buffer - line->vt_char_attrs = NULL","",0);
                        break;
                        }
                    line->width = VscrnGetWidth(vmode)  ;
                    line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
                    for ( x = 0 ; x < MAXTERMCOL ; x++ )
                        {
                        line->cells[x] = blankcell ;
                        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
                        }
                    }

                VscrnSetTop( vmode,ntop ) ;
                if ( bottommargin != VscrnGetHeight(vmode)
                                        -(tt_status[vmode]?2:1) )
                    VscrnScroll(vmode,DOWNWARD, bottommargin, VscrnGetHeight(vmode)
                                 -(tt_status[vmode]?2:1), 1, FALSE, fillchar) ;
                }
            else {
                for ( i = topmargin ; i <= bottommargin - nlines ; i++ ) {
                    /* save line to be deleted */
                    linetodelete = *VscrnGetLineFromTop(vmode,i) ;

                    /* then copy back a line */
                    line = VscrnGetLineFromTop(vmode,nlines+i) ;
                    if ( line == NULL )
                        break;
                    vscrn[vmode].lines[(vscrn[vmode].top+i)%vscrn[vmode].linecount] = *line ;
                    line->cells = linetodelete.cells ;
                    line->vt_char_attrs = linetodelete.vt_char_attrs ;
                    }

                for ( i = nlines-1 ; i >= 0 ; i-- ) {
                    line = VscrnGetLineFromTop(vmode,bottommargin-i) ;
                    if (line == NULL || line->cells == NULL) {
                        debug(F100,"VscrnScroll to buffer - line->cells = NULL","",0);
                        }
                    if (line->vt_char_attrs == NULL) {
                        debug(F100,"VscrnScroll to buffer - line->vt_char_attrs = NULL","",0);
                        }
                    line->width = VscrnGetWidth(vmode)  ;
                    line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
                    for ( x = 0 ; x < MAXTERMCOL ; x++ )
                        {
                        line->cells[x] = blankcell ;
                        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
                        }
                    }
                }
#ifndef NOKVERBS
            if ( scrollstatus[vmode] && !tt_roll[vmode] && !markmodeflag[vmode] ) {
                if ( (VscrnGetTop(vmode)+VscrnGetHeight(vmode)
                                        -(tt_status[vmode]?2:1))%VscrnGetBufferSize(vmode)
                    == VscrnGetEnd(vmode) )
                    putkverb( vmode, K_ENDSCN ) ;
                }
#endif /* NOKVERBS */
            break;

        case DOWNWARD:
            for ( i = bottommargin ; i >= topmargin+nlines ; i-- ) {
                /* save line to be deleted */
                linetodelete = *VscrnGetLineFromTop(vmode,i) ;

                /* then copy back a line */
                line = VscrnGetLineFromTop(vmode,i-nlines) ;
                if ( line == NULL )
                    break;
                vscrn[vmode].lines[(vscrn[vmode].top+i)%vscrn[vmode].linecount] = *line ;
                line->cells = linetodelete.cells ;
                line->vt_char_attrs = linetodelete.vt_char_attrs ;
                }

            for ( i = 0 ; i < nlines ; i++ ) {
                line = VscrnGetLineFromTop(vmode,topmargin+i) ;
                if (line == NULL || line->cells == NULL) {
                    debug(F100,"VscrnScroll to buffer - line->cells = NULL","",0);
                    }
                if (line->vt_char_attrs == NULL) {
                    debug(F100,"VscrnScroll to buffer - line->vt_char_attrs = NULL","",0);
                    }
                line->width = VscrnGetWidth(vmode)  ;
                line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
                for ( x = 0 ; x < MAXTERMCOL ; x++ )
                    {
                    line->cells[x] = blankcell ;
                    line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
                    }
                }
                break;

        default: /* ignore */ ;
    }
    ReleaseVscrnMutex( vmode ) ;
    debug(F100,"VscrnScroll releases mutex","",0);
}


/*---------------------------------------------------------------------------*/
/* VscrnMark                                                                 */
/*---------------------------------------------------------------------------*/
void
VscrnMark( BYTE vmode, LONG y, SHORT xbeg, SHORT xend )
{
    int x ;
    LONG yy, marktop, markbot, beg, end ;

    if ( xbeg < 0 || xend < 0 )
        return ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return;

    while ( y < 0 )
        y += vscrn[vmode].linecount ;
    y = y%vscrn[vmode].linecount ;

    if ( xend >= vscrn[vmode].lines[y].width )
        xend = vscrn[vmode].lines[y].width-1 ;

    yy = ( y - vscrn[vmode].beg + vscrn[vmode].linecount ) % vscrn[vmode].linecount ;
    beg = 0 ;
    end = ( vscrn[vmode].end - vscrn[vmode].beg + vscrn[vmode].linecount ) % vscrn[vmode].linecount ;
    marktop = ( vscrn[vmode].marktop - vscrn[vmode].beg + vscrn[vmode].linecount ) % vscrn[vmode].linecount ;
    markbot = ( vscrn[vmode].markbot - vscrn[vmode].beg + vscrn[vmode].linecount ) % vscrn[vmode].linecount ;

    if ( vscrn[vmode].marktop == -1 || vscrn[vmode].markbot == -1 )
    {
       vscrn[vmode].marktop = vscrn[vmode].markbot = y ;
       vscrn[vmode].lines[y].markbeg = xbeg ;
       vscrn[vmode].lines[y].markend = xend ;
    }
    else
    {
        if ( yy >= marktop && yy <= markbot ) {
            if ( vscrn[vmode].lines[y].markbeg == -1 ||
                xbeg < vscrn[vmode].lines[y].markbeg )
                vscrn[vmode].lines[y].markbeg = xbeg ;
            if ( vscrn[vmode].lines[y].markend == -1 ||
                xend > vscrn[vmode].lines[y].markend )
                vscrn[vmode].lines[y].markend = xend ;
            }
        else {
            if ( yy < marktop ) {
                vscrn[vmode].marktop = y ;
                }
            else {
                vscrn[vmode].markbot = y ;
                }
            vscrn[vmode].lines[y].markbeg = xbeg ;
            vscrn[vmode].lines[y].markend = xend ;
            }
        }

    vscrn[vmode].lines[y].markshowend = vscrn[vmode].lines[y].markend ;
    if ( vscrn[vmode].lines[y].markend >= 0 &&
        vscrn[vmode].lines[y].markbeg >= 0 ) {
        for ( x = vscrn[vmode].lines[y].markend ; x >= vscrn[vmode].lines[y].markbeg ; x-- ) {
            if (vscrn[vmode].lines[y].cells[x].c == ' ')
                vscrn[vmode].lines[y].markshowend-- ;
            else
                break ;
            }
        }

    if ( vscrn[vmode].lines[y].markshowend < vscrn[vmode].lines[y].markbeg )
            vscrn[vmode].lines[y].markshowend = -1 ;
}

/*---------------------------------------------------------------------------*/
/* VscrnUnmark                                                               */
/*---------------------------------------------------------------------------*/
void
VscrnUnmark( BYTE vmode, LONG y, SHORT xbeg, SHORT xend )
{
    int x ;

    if ( xbeg < 0 || xend < 0 )
        return ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return;

    while ( y < 0 )
        y += vscrn[vmode].linecount ;
    y = y%vscrn[vmode].linecount ;

    if ( vscrn[vmode].marktop != -1 && vscrn[vmode].markbot != -1 ) {
        if ( vscrn[vmode].marktop <= vscrn[vmode].markbot ) {
            if ( y >= vscrn[vmode].marktop && y <= vscrn[vmode].markbot ) {
                if ( xbeg <= vscrn[vmode].lines[y].markbeg && xend >= vscrn[vmode].lines[y].markend )
                    vscrn[vmode].lines[y].markbeg = vscrn[vmode].lines[y].markend = -1 ;
                else if ( xbeg <= vscrn[vmode].lines[y].markbeg && xend < vscrn[vmode].lines[y].markend )
                    vscrn[vmode].lines[y].markbeg = xend + 1 ;
                else if ( xbeg > vscrn[vmode].lines[y].markbeg && xend >= vscrn[vmode].lines[y].markend )
                    vscrn[vmode].lines[y].markend = xbeg - 1 ;
                }
            }
        else {
            if ( y >= vscrn[vmode].marktop || y <= vscrn[vmode].markbot ) {
                if ( xbeg <= vscrn[vmode].lines[y].markbeg && xend >= vscrn[vmode].lines[y].markend )
                    vscrn[vmode].lines[y].markbeg = vscrn[vmode].lines[y].markend = -1 ;
                else if ( xbeg <= vscrn[vmode].lines[y].markbeg && xend < vscrn[vmode].lines[y].markend )
                    vscrn[vmode].lines[y].markbeg = xend + 1 ;
                else if ( xbeg > vscrn[vmode].lines[y].markbeg && xend >= vscrn[vmode].lines[y].markend )
                    vscrn[vmode].lines[y].markend = xbeg - 1 ;
                }
            }

        if ( y == vscrn[vmode].marktop &&
            y == vscrn[vmode].markbot &&
            vscrn[vmode].lines[y].markbeg == -1 &&
            vscrn[vmode].lines[y].markend == -1 )
            vscrn[vmode].marktop = vscrn[vmode].markbot = -1 ;  /* marktop,markbot are long */
        else if ( y == vscrn[vmode].marktop &&
            vscrn[vmode].lines[y].markbeg == -1 &&
            vscrn[vmode].lines[y].markend == -1 )
            vscrn[vmode].marktop = (vscrn[vmode].marktop+1)%vscrn[vmode].linecount ;
        else if ( y == vscrn[vmode].markbot &&
            vscrn[vmode].lines[y].markbeg == -1 &&
            vscrn[vmode].lines[y].markend == -1 )
            vscrn[vmode].markbot = (vscrn[vmode].markbot-1+vscrn[vmode].linecount)%vscrn[vmode].linecount ;
        }

    vscrn[vmode].lines[y].markshowend = vscrn[vmode].lines[y].markend ;
    if ( vscrn[vmode].lines[y].markend >= 0 &&
        vscrn[vmode].lines[y].markbeg >= 0 ) {
        for ( x = vscrn[vmode].lines[y].markend ; x >= vscrn[vmode].lines[y].markbeg ; x-- ) {
            if (vscrn[vmode].lines[y].cells[x].c == ' ')
                vscrn[vmode].lines[y].markshowend-- ;
            else
                break ;
            }
        }

    if ( vscrn[vmode].lines[y].markshowend < vscrn[vmode].lines[y].markbeg )
            vscrn[vmode].lines[y].markshowend = -1 ;
}

/*---------------------------------------------------------------------------*/
/* VscrnUnmarkAll                                                            */
/*---------------------------------------------------------------------------*/
void
VscrnUnmarkAll( BYTE vmode )
{
    long y ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return;

    if ( vscrn[vmode].marktop != -1 && vscrn[vmode].markbot != -1 ) {
        if ( vscrn[vmode].marktop <= vscrn[vmode].markbot ) {
            for ( y = vscrn[vmode].marktop ; y <= vscrn[vmode].markbot ; y++ )
                vscrn[vmode].lines[y].markbeg =
                    vscrn[vmode].lines[y].markshowend =
                    vscrn[vmode].lines[y].markend = -1 ;
            }
        else {
            for ( y = vscrn[vmode].marktop ; y < vscrn[vmode].linecount ; y++ )
                vscrn[vmode].lines[y].markbeg =
                    vscrn[vmode].lines[y].markshowend =
                    vscrn[vmode].lines[y].markend = -1 ;
            for ( y = 0 ; y <= vscrn[vmode].markbot ; y++ )
                vscrn[vmode].lines[y].markbeg =
                    vscrn[vmode].lines[y].markshowend =
                    vscrn[vmode].lines[y].markend = -1 ;
            }
        }
    vscrn[vmode].marktop = vscrn[vmode].markbot = -1 ;
}

/*---------------------------------------------------------------------------*/
/* VscrnIsLineMarked                                                         */
/*---------------------------------------------------------------------------*/
BOOL
VscrnIsLineMarked( BYTE vmode, LONG y )
{
    BOOL rc = FALSE ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return rc;

    if ( vscrn[vmode].marktop != -1 && vscrn[vmode].markbot != -1 ) {
        while ( y < 0 )
            y += vscrn[vmode].linecount ;
        y = y%vscrn[vmode].linecount ;

        if ( vscrn[vmode].marktop <= vscrn[vmode].markbot ) {
            rc = ( y >= vscrn[vmode].marktop && y <= vscrn[vmode].markbot ) ;
            }
        else {
            rc = ( y >= vscrn[vmode].marktop || y <= vscrn[vmode].markbot ) ;
            }
        }

    return rc ;
}

/*---------------------------------------------------------------------------*/
/* VscrnIsMarked                                                             */
/*---------------------------------------------------------------------------*/
BOOL
VscrnIsMarked( BYTE vmode, LONG y, SHORT x )
{
    BOOL rc = FALSE ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vscrn[vmode].lines == NULL )
        return rc;

    if ( vscrn[vmode].marktop != -1 && vscrn[vmode].markbot != -1 ) {
        while ( y < 0 )
            y += vscrn[vmode].linecount ;
        y = y%vscrn[vmode].linecount ;

        if ( vscrn[vmode].marktop <= vscrn[vmode].markbot ) {
            rc = ( y >= vscrn[vmode].marktop && y <= vscrn[vmode].markbot &&
                x >= vscrn[vmode].lines[y].markbeg && x <= vscrn[vmode].lines[y].markend ) ;
            }
        else {
            rc = ( ( y >= vscrn[vmode].marktop || y <= vscrn[vmode].markbot ) &&
                x >= vscrn[vmode].lines[y].markbeg && x <= vscrn[vmode].lines[y].markend ) ;
            }
        }

    return rc ;
}

/*---------------------------------------------------------------------------*/
/* VscrnIsPopup                                                              */
/*---------------------------------------------------------------------------*/
bool
VscrnIsPopup( BYTE vmode )
{
#ifdef COMMENT
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;
#endif
    return ( vscrn[vmode].popup != NULL ) ;
}

/*---------------------------------------------------------------------------*/
/* VscrnSetPopup                                                             */
/*---------------------------------------------------------------------------*/
void
VscrnSetPopup( BYTE vmode, videopopup * pu )
{
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* Wait for exclusive access to the screen */
    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;

    if ( vscrn[vmode].popup != NULL )
        free(vscrn[vmode].popup);
    vscrn[vmode].popup = pu;
    ReleaseVscrnMutex(vmode);
    VscrnIsDirty(vmode);

    if ( vmode == VTERM )
        puterror(VTERM);
    msleep(1);
}

/*---------------------------------------------------------------------------*/
/* ResetPopup                                                                */
/*---------------------------------------------------------------------------*/
void
VscrnResetPopup( BYTE vmode )
{
    /* Wait for exclusive access to the screen */
    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;

    if ( vscrn[vmode].popup != NULL )
        free(vscrn[vmode].popup) ;
    vscrn[vmode].popup = NULL ;
    ReleaseVscrnMutex(vmode) ;
    VscrnIsDirty(vmode) ;
    msleep(1);
}

/*
 * IsCellPartOfURL()
 */

int
IsURLChar( USHORT ch )
{
#ifdef COMMENT
    if (ch && (isalnum(ch) || ch == '/' || ch == ':' || ch == '.' || ch == '#' ||
                ch == '|' || ch == '@' || ch == '!' || ch == '-' || ch == '_' ||
                ch == '?' || ch == '%' || ch == '&' || ch == '+' || ch == '\\' ||
                ch == '=' || ch == ';' || ch == '~' || ch == ',' || ch == '$'))
    {
        return 1;
    } else {
        return 0;
    }
#else /* COMMENT */
    if ( ch <= 32 )
        return 0;
    switch ( ch ) {
    case 127:
    case '<':
    case '>':
    case '[':
    case ']':
    case '{':
    case '}':
    case '\'':
    case '"':
    case '`':
    case '^':
        return 0;
    default:
        return 1;
    }
#endif /* COMMENT */
}

int
IsCellPartOfURL( BYTE mode, USHORT row, USHORT col )
{
    videoline * line=NULL;
    viocell * cells = NULL;
    register char ch;
    static char str[MAXTERMSIZE];
    int i,j,len=0, first=1, rc;
    extern bool scrollflag[] ;
    extern char browsurl[];
    /* Let's cache these values */
    static int retval = 0;
    static int brow=-1, bcol=-1, erow=-1, ecol=-1, _mode=-1;

    if ( mode == _mode ) {
        if ( row == brow && col >= bcol ) {
            if ( row == erow && col <= ecol || row < erow ) {
                return(retval);
            }
        } else if ( row == erow && col <= ecol ) {
            if ( row > brow ) {
                return(retval);
            }
            else if ( row > brow && row < erow ) {
                return(retval);
            }
        }
    }
    /* From the current row,col we need to determine which line contains
       the beginning and end of the string we think might contain a URL.

       Compute the length of the string

       Then copy the whole string into a temporary buffer

     */

    /* Find the beginning of the URL */

    _mode = mode;
    brow = erow = row;
    bcol = ecol = col;

    rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
    if ( scrollflag[mode] )
        line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + brow );
    else
        line = VscrnGetLineFromTop( mode, brow );
    rc = ReleaseVscrnMutex( mode ) ;
    if ( !line || line->width == 0 )
        return(retval = 0);
    cells = line->cells;

    while(1) {
        while ( bcol >= 0 ) {
            if ( IsURLChar(cells[bcol].c) ) {
                bcol--;
            } else {
                if (first)
                    return(retval = 0);

                bcol++;
                if ( bcol >= line->width ) {
                    bcol = 0;
                    brow++;
                }
                goto bfound;
            }
            first = 0;
        }

        if ( brow == 0 ) {
            if ( bcol < 0 )
                bcol = 0;
            goto bfound;
        }

        brow--;
        rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + brow );
        else
            line = VscrnGetLineFromTop( mode, brow );
        rc = ReleaseVscrnMutex( mode ) ;
        if ( !line || line->width == 0 )
            return(retval = 0);
        cells = line->cells;
        bcol = line->width-1;
    }
  bfound:

    /* Find the end of the URL */
    rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
    if ( scrollflag[mode] )
        line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + erow );
    else
        line = VscrnGetLineFromTop( mode, erow );
    rc = ReleaseVscrnMutex( mode ) ;
    if ( !line || line->width == 0 )
        return(retval = 0);
    cells = line->cells;

    while(1) {
        while ( ecol < line->width ) {
            if (IsURLChar(cells[ecol].c)) {
                ecol++;
            } else {
                ecol--;
                if ( ecol < 0 ) {
                    erow--;
                    rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
                    if ( scrollflag[mode] )
                        line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + erow );
                    else
                        line = VscrnGetLineFromTop( mode, erow );
                    rc = ReleaseVscrnMutex( mode ) ;
                    if ( !line || line->width == 0 )
                        return(retval = 0);
                    cells = line->cells;
                    ecol = line->width - 1;
                }
                ch  = (CHAR) cells[ecol].c;
                if (ch == '.' || ch == ',' || ch == ';') {
                    ecol--;
                    if ( erow == row && ecol < col )
                        return(retval = 0);
                    if ( ecol < 0 ) {
                        erow--;
                        rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
                        if ( scrollflag[mode] )
                            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + erow );
                        else
                            line = VscrnGetLineFromTop( mode, erow );
                        rc = ReleaseVscrnMutex( mode ) ;
                        if ( !line || line->width == 0 )
                            return(retval = 0);
                        cells = line->cells;
                        ecol = line->width - 1;
                    }
                }
                goto efound;
            }
        }

        if ( erow == VscrnGetHeight(vmode)-(tt_status[vmode]?1:0) )
            goto efound;

        erow++;
        rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + erow );
        else
            line = VscrnGetLineFromTop( mode, erow );
        rc = ReleaseVscrnMutex( mode ) ;
        if ( !line || line->width == 0 )
            return(retval = 0);
        cells = line->cells;
        ecol = 0;
    }
  efound:

    /* We now copy the cells to a string buffer so that we can determine */
    /* whether or not the URL has the proper prefix.  But we are never   */
    /* going to need more than a dozen characters for the comparison, so */
    /* stop after 12. */
#define URL_PREFIX_LEN 12

    str[0] = '\0';

    if ( brow == erow ) {
        rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + brow );
        else
            line = VscrnGetLineFromTop( mode, brow );
        rc = ReleaseVscrnMutex( mode ) ;
        cells = line->cells;

        for ( i=bcol,len=0;i<=ecol && len<URL_PREFIX_LEN;i++,len++ )
            str[len] = (CHAR) cells[i].c;
        str[len] = '\0';
    } else {
        /* handle the first row - bcol to end */
        rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + brow );
        else
            line = VscrnGetLineFromTop( mode, brow );
        rc = ReleaseVscrnMutex( mode ) ;
        cells = line->cells;

        for ( i=bcol,len=0;i<line->width  && len<URL_PREFIX_LEN;i++,len++ )
            str[len] = (CHAR) cells[i].c;

        /* handle the complete rows if there are any */
        for ( j=brow+1; j<erow; j++ ) {
            rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
            if ( scrollflag[mode] )
                line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + j );
            else
                line = VscrnGetLineFromTop( mode, j );
            rc = ReleaseVscrnMutex( mode ) ;
            cells = line->cells;

            for ( i=0;i<line->width  && len<URL_PREFIX_LEN;i++,len++ )
                str[len] = (CHAR) cells[i].c;
        }

        /* handle the last row - begin to ecol */
        rc = RequestVscrnMutex( mode, SEM_INDEFINITE_WAIT ) ;
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + erow );
        else
            line = VscrnGetLineFromTop( mode, erow );
        rc = ReleaseVscrnMutex( mode ) ;
        cells = line->cells;

        for ( i=0;i<=ecol && len<URL_PREFIX_LEN;i++,len++ )
            str[len] = (CHAR) cells[i].c;
        str[len] = '\0';
    }

    if (len == 0)
        return(retval = 0);

    if ( strnicmp(str,"telnet://",9) &&
         strnicmp(str,"ssh://",6)  &&
         strnicmp(str,"login://",8)  &&
         strnicmp(str,"http://",7) &&
         strnicmp(str,"gopher://",9) &&
         strnicmp(str,"news://",7) &&
         strnicmp(str,"snews://",8) &&
         strnicmp(str,"ftp://",6) &&
         strnicmp(str,"mailto:",7) &&
         strnicmp(str,"https://",8) &&
         strnicmp(str,"ftps://",7) &&
         strnicmp(str,"telnets://",10) &&
         strnicmp(str,"iksd://",7) &&
         strnicmp(str,"kermit://",9) )
    {
#ifdef HIGHLIGHT_MAILTO
        int mailto = 0;

        for ( i=0;i<len;i++ )
        {
            if ( !mailto && str[i] == '@' ) {
                mailto = 1;
            }
            else if ( mailto ) {
                switch ( str[i] ) {
                case '@':
                case ',':
                case '/':
                case '\\':
                case '$':
                case '?':
                case '|':
                case '!':
                    mailto = 0 ;
                    break;      /* switch */
                }
                if ( !mailto )
                    break;      /* for */
            }
        }
        if ( mailto && str[len-1] != '@' && str[0] != '@' )
            retval = 1;
        else
#endif /* HIGHLIGHT_MAILTO */
            retval = 0;
    } else
        retval = 1;

    return(retval);
}

/*---------------------------------------------------------------------------*/
/* TermScrnUpd                                                               */
/*---------------------------------------------------------------------------*/
#define URLMINCNT 4096
#define NEW_EXCLUSIVE 1
void
TermScrnUpd( void * threadinfo)
{
    viocell * thecells = NULL, defaultcell, defaultflipcell ;
    videoline * line ;
    viocell   *pScell, *pDcell;
    unsigned short vt_char_attrs;
    unsigned short xs, ys, c, x, y,
                   xho,                 /* horizontal scrolling offset */
                   xo, yo ;             /* offsets for Popup positioning */
    position cursor ;
    char buffer[MAXTERMCOL+1] ;
    APIRET rc ;
    char * status ;
    int cursor_offset = 0 ;
    int blinking = 0 ;          /* Screen contains something that blinks */
    int blinkon  = 0 ;          /* Blinking items are showing            */
    int dirty    = 0 ;          /* is vscrn dirty? */
    extern int colorreset;
    extern int trueblink;
    extern int screenon ;
#ifdef NT
    extern HANDLE KbdHandle ;
    DWORD conmode ;
#endif /* NT */
    extern int decssdt ;
    extern char hoststatusline[] ;
    extern int tt_modechg;
    int old_tt_update ;
    int avm ;                           /* Active vmode */
    int prty = priority;
#ifndef KUI
    CK_VIDEOMODEINFO mi;
#endif /* KUI */
    int incnt = 0;

#ifndef ONETERMUPD
    vmode = VTERM ;
#endif /* ONETERMUPD */

    PostTermScrnUpdThreadSem();
#ifdef NT
    setint();
    rc = SetThreadPrty(priority,isWin95() ? /* 3, 1 */ 2 : 11);
#else
    rc = SetThreadPrty(priority,4);
#endif

    rc = CreateVscrnTimerSem( TRUE );
    rc = CreateVscrnMuxWait(VCMD) ;
    rc = CreateVscrnMuxWait(VTERM) ;
    rc = CreateVscrnMuxWait(VCS) ;

    rc = StartVscrnTimer( tt_update );
    old_tt_update = tt_update ;

    /* Allocate buffer for screen writes and blank the screen */
    thecells = malloc( MAXTERMCOL * MAXTERMROW * sizeof(viocell) ) ;
    if (!thecells) {
        fatal("TermScrnUpd: Unable to allocate memory for Video Display Workspace");
    }
    defaultcell.c = ' ' ;
    defaultcell.a = vmode == VCMD ? colorcmd :
      (colorreset ? colornormal : defaultattribute) ;
    defaultflipcell = defaultcell ;
    byteswapcolors( defaultflipcell.a ) ;
    debug(F101,"VscrnGetWidth() ","",VscrnGetWidth(vmode) ) ;
    debug(F101,"VscrnGetDisplayHeight()","",VscrnGetDisplayHeight(vmode)) ;
    WrtNCell( defaultcell, VscrnGetWidth(vmode)  * (VscrnGetDisplayHeight(vmode)), 0, 0 ) ;
    debug(F110,"TermScrnUpd","WrtNCell defaultcell complete",0);
#ifdef NT
    /* we want to turn on processing of Window Events */
    GetConsoleMode( KbdHandle, &conmode ) ;
    conmode |= ENABLE_WINDOW_INPUT ;
    rc = SetConsoleMode( KbdHandle, conmode ) ;
#endif /* NT */

#ifndef ONETERMUPD
    while (IsConnectMode())
#else
    while ( 1 )
#endif /* ONETERMUPD */
    {
        /* Need to add some code in here to check variables such as  */
        /* tt_update so that the timers can be set at the new speed  */
        /* or move the timer creation to sysinit() and then have the */
        /* SET TERMINAL SCREEN-UPDATE command set the timer rates.   */
        if ( tt_update != old_tt_update ) {
            StopVscrnTimer();
            StartVscrnTimer(tt_update);
            old_tt_update = tt_update ;
        }
        avm = vmode ;

#ifdef NT
        if ( prty != priority ) {
            if ( isWin95() )
                SetThreadPrty(priority,/* 3 */ 2 /* 1 */) ;
            else
                SetThreadPrty(priority,9 /* 11 */) ;
            prty = priority;
        }
#else
        if ( prty != priority ) {
            SetThreadPrty(priority,3) ;
            prty = priority;
        }
#endif /* NT */

        /* wait for something to happen */
        if ( trueblink && blinking )
            msleep(200);
        else if ( updmode == TTU_FAST ) {
            if ( !WaitVscrnMuxWait( avm, SEM_INDEFINITE_WAIT ) ) {
                /* returns semid if success */
                /* reset the timer */
                ResetVscrnTimerSem(avm) ;
                continue;
            }
            else {
                /* reset the timer */
                ResetVscrnTimerSem(avm) ;
            }
        }
        else { /* updmode == TTU_SMOOTH */
            if ( WaitVscrnDirtySem( avm, SEM_INDEFINITE_WAIT ) ) {
                /* returns 0 if success */
                continue;
            }
        }

        /* has anything changed? */
        dirty = VscrnClean(avm) ;
        if ( !dirty && !trueblink && !blinking ) {
            continue ;
        }

        if ( avm != vmode ) {
            cursoron[avm] = 0;        /* Force it back on */
            continue;
        }

        if ( trueblink && blinking ) {
            blinking = 0 ;                      /* reset blinking flag */
            blinkon = (blinkon+1) % 4 ;         /* toggle blinking chars */
            if ( blinkon > 1 && !dirty )
            {
                blinking = 1;                   /* remember we were blinking */
                continue;                       /* don't paint the screen */
            }
        }
        else
            blinkon = 1 ;                               /* show blinking items */

        incnt = (avm == VTERM) ? ttchk() : 0;

#ifdef OLD_EXCLUSIVE
        /* Wait for exclusive access to the screen buffer */
        rc = RequestVscrnMutex( avm, SEM_INDEFINITE_WAIT ) ;
        if ( rc )
            continue ;
#endif /* OLD_EXCLUSIVE */
        xs = (tt_modechg == TVC_ENA ) ? vscrn[avm].width : pwidth ;
        xho = vscrn[avm].hscroll > (MAXTERMCOL-pwidth) ?
            (MAXTERMCOL-pwidth) : vscrn[avm].hscroll ;
        ys = (tt_modechg == TVC_ENA ? vscrn[avm].height : pheight)
            - ((avm==VTERM && tt_status[VTERM] && decssdt!=SSDT_BLANK ||
                avm != VTERM && tt_status[avm]) ? 1 : 0 ) ;
        c = 0 ;

        /* Build the screen to paint */

        /* do we have a popup to display? */
        if ( vscrn[avm].popup == NULL) {
            xo = yo = -1 ;
        }
        else {
            xo = (xs - vscrn[avm].popup->width) / 2 ;
            yo = (ys - vscrn[avm].popup->height) / 2 ;
        }

        /* Handle the real data */

        if ( avm == VTERM && !screenon ) {
            /* Blank out the screen */
            for ( y=0 ; y < ys ; y++ )
                for ( x= 0; x < xs ; x++ )
                    thecells[c++] = defaultcell ;
        }
        else if ( markmodeflag[avm] != marking ) {
            for ( y = 0 ; y < ys ; y++ ) {
#ifdef NEW_EXCLUSIVE
                /* Wait for exclusive access to the screen buffer */
                rc = RequestVscrnMutex( avm, SEM_INDEFINITE_WAIT ) ;
                if ( rc )
                    continue ;
#endif /* NEW_EXCLUSIVE */
                /* Get the next line */
                if (!scrollflag[avm]) {
                    line = &vscrn[avm].lines[(vscrn[avm].top+y)%vscrn[avm].linecount] ;
                }
                else {
                    line = &vscrn[avm].lines[(vscrn[avm].scrolltop+y)%vscrn[avm].linecount] ;
                }

                /* copy the screen data to the buffer */
                if (line != NULL && line->cells != NULL) {
                    if ( line->vt_line_attr & VT_LINE_ATTR_DOUBLE_WIDE )
                    {
                        for ( x = 0 ; x < xs ; x++ ) {
                            vt_char_attrs = line->vt_char_attrs[x+xho];
                            pScell = &line->cells[x+xho];
                            pDcell = &thecells[c+2*x];

                            if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                                vt_char_attrs |= tt_url_hilite_attr;
                            else if ( tt_url_hilite && incnt < URLMINCNT) {
                                if ( IsURLChar(pScell->c) && 
                                     IsCellPartOfURL(avm, y, x+xho ) )
                                    vt_char_attrs |= tt_url_hilite_attr;
                            }
                            if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                            {
                                blinking = 1 ;
                                if ( blinkon )
                                {
                                    pDcell->c = pScell->c ;
                                    pDcell->a = (pDcell+1)->a =
                                        ComputeColorFromAttr( avm, pScell->a,
                                                             vt_char_attrs );
                                }
                                else
                                {
                                    pDcell->c = ' ' ;
                                    pDcell->a = (pDcell+1)->a =
                                        ComputeColorFromAttr( avm, pScell->a,
                                                             vt_char_attrs );
                                }
                            }
                            else
                            {
                                pDcell->c = pScell->c ;
                                pDcell->a = (pDcell+1)->a =
                                    ComputeColorFromAttr( avm, pScell->a,
                                                         vt_char_attrs );
                            }
                            (pDcell+1)->c = ' ' ;
                        }
                    }
                    else
                    {
                        for ( x = 0 ; x < xs ; x++ ) {
                            vt_char_attrs = line->vt_char_attrs[x+xho];
                            pScell = &line->cells[x+xho];
                            pDcell = &thecells[c+x];

                            if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                                vt_char_attrs |= tt_url_hilite_attr;
                            else if ( tt_url_hilite && incnt < URLMINCNT) {
                                if ( IsURLChar(pScell->c) && 
                                     IsCellPartOfURL(avm, y, x+xho ) )
                                    vt_char_attrs |= tt_url_hilite_attr;
                            }
                            if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                            {
                                blinking = 1 ;
                                if ( blinkon )
                                {
                                    pDcell->c = pScell->c ;
                                    pDcell->a =
                                        ComputeColorFromAttr( avm, pScell->a,
                                                             vt_char_attrs );
                                }
                                else
                                {
                                    pDcell->c = ' ' ;
                                    pDcell->a =
                                        ComputeColorFromAttr( avm, pScell->a,
                                                             vt_char_attrs );
                                }
                            }
                            else
                            {
                                pDcell->c = pScell->c ;
                                pDcell->a =
                                    ComputeColorFromAttr( avm, pScell->a,
                                                         vt_char_attrs );
                            }
                        }
                    }
                } else {
                    /* In case we are in the middle of a scroll */
                    for ( x = 0 ; x < xs ; x++ ) {
                        thecells[c+x] = defaultcell ;
                    }
#ifdef VSCRN_DEBUG
                    debug(F111,"TermScrnUpd","OUCH!",
                          (scrollflag[avm]?(vscrn[avm].scrolltop+y)
                           :(vscrn[avm].top+y))%vscrn[avm].linecount);
#endif /* VSCRN_DEBUG */
                }

#ifdef NEW_EXCLUSIVE
                rc = ReleaseVscrnMutex( avm ) ;
#endif /* NEW_EXCLUSIVE */

                /* if there is a popup, display it */
                if ( y >= yo &&
                    vscrn[avm].popup &&
                    y <= yo+vscrn[avm].popup->height-1 ) {
                    int i ;
                    for ( i = 0 ; i < vscrn[avm].popup->width ; i++ ) {
                        if ( isunicode() ) {
                            thecells[c+xo+i].c =
                                vscrn[avm].popup->c[y-yo][i] < 32 ?
                                    (*xl_u[TX_IBMC0GRPH])(vscrn[avm].popup->c[y-yo][i]) :
                                        vscrn[avm].popup->c[y-yo][i] >= 128 ?
                                    (*xl_u[TX_CP437])(vscrn[avm].popup->c[y-yo][i]) :
                                        vscrn[avm].popup->c[y-yo][i] ;
                        }
                        else
                            thecells[c+xo+i].c = vscrn[avm].popup->c[y-yo][i];
                        thecells[c+xo+i].a = vscrn[avm].popup->a;
                    }
                }
                c += xs;        /* advance the pointer in the buffer */
            }
        }
        else {  /* were in marking mode */
            for ( y = 0 ; y < ys ; y++ ) {
                line = &vscrn[avm].lines[(vscrn[avm].scrolltop+y)%vscrn[avm].linecount] ;

                if (line != NULL && line->cells) {
                    if ( VscrnIsLineMarked(avm,vscrn[avm].scrolltop+y) ) {
                        if ( line->vt_line_attr & VT_LINE_ATTR_DOUBLE_WIDE )
                        {
                            for ( x = 0 ; x < xs/2 ; x++ ) {
                                vt_char_attrs = line->vt_char_attrs[x+xho];
                                pScell = &line->cells[x+xho];
                                pDcell = &thecells[c+2*x];
                
                                if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                                    vt_char_attrs |= tt_url_hilite_attr;
                                else if ( tt_url_hilite && incnt < URLMINCNT ) {
                                    if ( IsURLChar(pScell->c) && 
                                         IsCellPartOfURL(avm, y, x+xho ) )
                                        vt_char_attrs |= tt_url_hilite_attr;
                                }
                                if ( line->markshowend != -1 &&
                                    line->markend >= line->width-1 &&
                                    x >= line->markbeg*2 &&
                                    x <= line->markshowend ) {
                                    if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                                    {
                                        blinking = 1 ;
                                        pDcell->c = blinkon ?
                                            pScell->c : ' ' ;
                                    }
                                    else
                                        pDcell->c = pScell->c ;
                                    (pDcell+1)->c = ' ' ;
                                    pDcell->a =
                                        (pDcell+1)->a = colorselect ;
                                }
                                else if ( line->markshowend != -1 &&
                                         x+xho >= line->markbeg*2 &&
                                         x+xho <= line->markend/2 &&
                                         x+xho <= line->markshowend )
                                {
                                    if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                                    {
                                        blinking = 1 ;
                                        pDcell->c = blinkon ?
                                            pScell->c : ' ' ;
                                    }
                                    else
                                        pDcell->c = pScell->c ;
                                    (pDcell+1)->c = ' ' ;
                                    pDcell->a =
                                        (pDcell+1)->a = colorselect ;
                                }
                                else
                                {
                                    if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                                    {
                                        blinking = 1 ;
                                        if ( blinkon )
                                        {
                                            pDcell->c = pScell->c ;
                                            pDcell->a = (pDcell+1)->a =
                                                ComputeColorFromAttr( avm, pScell->a,
                                                                     vt_char_attrs );
                                        }
                                        else
                                        {
                                            pDcell->a = (pDcell+1)->a =
                                                ComputeColorFromAttr( avm, pScell->a,
                                                                     vt_char_attrs );
                                        }
                                    }
                                    else
                                    {
                                        pDcell->c = pScell->c ;
                                        pDcell->a = (pDcell+1)->a =
                                            ComputeColorFromAttr( avm, pScell->a,
                                                                 vt_char_attrs );
                                    }
                                    (pDcell+1)->c = ' ' ;
                                }
                            }
                        }
                        else
                        {
                            for ( x = 0 ; x < xs ; x++ ) {
                                vt_char_attrs = line->vt_char_attrs[x+xho];
                                pScell = &line->cells[x+xho];
                                pDcell = &thecells[c+x];
                                
                                if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                                    vt_char_attrs |= tt_url_hilite_attr;
                                else if ( tt_url_hilite && incnt < URLMINCNT ) {
                                    if ( IsURLChar(pScell->c) && 
                                         IsCellPartOfURL(avm, y, x+xho ) )
                                        vt_char_attrs |= tt_url_hilite_attr;
                                }
                                if ( line->markshowend != -1 &&
                                    x+xho >= line->markbeg &&
                                    x+xho <= line->markshowend ) {
                                    if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                                    {
                                        blinking = 1 ;
                                        pDcell->c = blinkon ?
                                            pScell->c : ' ' ;
                                    }
                                    else
                                        pDcell->c = pScell->c ;
                                    pDcell->a = colorselect ;
                                }
                                else
                                {
                                    if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                                    {
                                        blinking = 1 ;
                                        if ( blinkon )
                                        {
                                            pDcell->c = pScell->c ;
                                            pDcell->a =
                                                ComputeColorFromAttr( avm, pScell->a,
                                                                     vt_char_attrs);
                                        }
                                        else
                                        {
                                            pDcell->c = ' ' ;
                                            pDcell->a =
                                                ComputeColorFromAttr( avm, pScell->a,
                                                                     vt_char_attrs );
                                        }
                                    }
                                    else
                                    {
                                        pDcell->c = pScell->c ;
                                        pDcell->a =
                                            ComputeColorFromAttr( avm, pScell->a,
                                                                 vt_char_attrs );
                                    }
                                }
                            }
                        }
                    }
                    else {      /* line is not marked */
                        if ( line->vt_line_attr & VT_LINE_ATTR_DOUBLE_WIDE )
                        {
                            {
                                for ( x = 0 ; x < xs ; x++ ) {
                                    vt_char_attrs = line->vt_char_attrs[x+xho];
                                    pScell = &line->cells[x+xho];
                                    pDcell = &thecells[c+2*x];
                                
                                    if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                                        vt_char_attrs |= tt_url_hilite_attr;
                                    else if ( tt_url_hilite && incnt < URLMINCNT ) {
                                        if ( IsURLChar(pScell->c) && 
                                             IsCellPartOfURL(avm, y, x+xho ) )
                                            vt_char_attrs |= tt_url_hilite_attr;
                                    }
                                    if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                                    {
                                        blinking = 1 ;
                                        if ( blinkon )
                                        {
                                            pDcell->c = pScell->c ;
                                            pDcell->a = (pDcell+1)->a =
                                                ComputeColorFromAttr( avm, pScell->a,
                                                                     vt_char_attrs);
                                        }
                                        else
                                        {
                                            pDcell->c = ' ' ;
                                            pDcell->a = (pDcell+1)->a =
                                                ComputeColorFromAttr( avm, pScell->a,
                                                                     vt_char_attrs );
                                        }
                                    }
                                    else
                                    {
                                        pDcell->c = pScell->c ;
                                        (pDcell+1)->c = ' ' ;
                                        pDcell->a = (pDcell+1)->a =
                                            ComputeColorFromAttr( avm, pScell->a,
                                                                 vt_char_attrs );                                    }
                                }
                            }
                        }
                        else
                        {
                            {
                                for ( x = 0 ; x < xs ; x++ ) {
                                    vt_char_attrs = line->vt_char_attrs[x+xho];
                                    pScell = &line->cells[x+xho];
                                    pDcell = &thecells[c+x];
                                    
                                    if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                                        vt_char_attrs |= tt_url_hilite_attr;
                                    else if ( tt_url_hilite && incnt < URLMINCNT ) {
                                        if ( IsURLChar(pScell->c) && 
                                             IsCellPartOfURL(avm, y, x+xho ) )
                                            vt_char_attrs |= tt_url_hilite_attr;
                                    }
                                    if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                                    {
                                        blinking = 1 ;
                                        pDcell->c = blinkon ?
                                            pScell->c : ' ' ;
                                    }
                                    else
                                        pDcell->c = pScell->c ;
                                    pDcell->a =
                                        ComputeColorFromAttr( avm, pScell->a,
                                                             vt_char_attrs );
                                }
                            }
                        }
                    }
                } else {
                    /* In case we are in the middle of a scroll */
                    for ( x = 0 ; x < xs ; x++ ) {
                        thecells[c+x] = flipscrnflag[avm] ?
                            defaultflipcell : defaultcell ;
                    }
                }
#ifdef NEW_EXCLUSIVE
                rc = ReleaseVscrnMutex( avm ) ;
#endif /* NEW_EXCLUSIVE */

                /* if there is a popup, display it */
                if ( y >= yo &&
                    vscrn[avm].popup &&
                    y <= yo+vscrn[avm].popup->height-1 ) {
                    int i ;
                    for ( i = 0 ; i < vscrn[avm].popup->width ; i++ ) {
                        if ( isunicode() ) {
                            thecells[c+xo+i].c =
                                vscrn[avm].popup->c[y-yo][i] < 32 ?
                                    (*xl_u[TX_IBMC0GRPH])(vscrn[avm].popup->c[y-yo][i]) :
                                        vscrn[avm].popup->c[y-yo][i] >= 128 ?
                                            (*xl_u[TX_CP437])(vscrn[avm].popup->c[y-yo][i]) :
                                                vscrn[avm].popup->c[y-yo][i] ;
                        }
                        else
                            thecells[c+xo+i].c = vscrn[avm].popup->c[y-yo][i];
                        thecells[c+xo+i].a = vscrn[avm].popup->a;
                    }
                }
                c += xs;
            }
        }
#ifdef OLD_EXCLUSIVE
        rc = ReleaseVscrnMutex( avm ) ;
#endif /* OLD_EXCLUSIVE */
        
        /* Status Line Display */
        if ( avm == VTERM && tt_status[VTERM] && decssdt != SSDT_BLANK ||
             avm != VTERM && tt_status[avm])
        {
            if ( avm == VTERM && decssdt == SSDT_HOST_WRITABLE && tt_status[VTERM] == 1) {
                line = &vscrn[VSTATUS].lines[0] ;
                if ( line != NULL )
                for ( x = 0 ; x < xs ; x++ ) {
                    vt_char_attrs = line->vt_char_attrs[x];
                    pScell = &line->cells[x];
                    pDcell = &thecells[c+x];

                    if (vt_char_attrs & VT_CHAR_ATTR_BLINK)
                    {
                        blinking = 1 ;
                        if ( blinkon )
                        {
                            pDcell->c = pScell->c ;
                            pDcell->a =
                                ComputeColorFromAttr( avm, pScell->a,
                                                     vt_char_attrs );
                        }
                        else
                        {
                            pDcell->c = ' ' ;
                            pDcell->a =
                                ComputeColorFromAttr( avm, pScell->a,
                                                     vt_char_attrs );
                        }
                    }
                    else
                    {
                        pDcell->c = pScell->c ;
                        pDcell->a =
                            ComputeColorFromAttr( avm, pScell->a,
                                                 vt_char_attrs );
                    }
                }
                c += xs ;
            }
            else {/* SSDT_INDICATOR - Kermit Status Line */
                status = line25(avm);

                for ( x = 0 ; x < xs ; x++ ) {
                    if ( isunicode() ) {
                        thecells[c].c = status[x] < 32 ?
                            (*xl_u[TX_IBMC0GRPH])(status[x]) :
                                status[x] >= 128 ?
                                    (*xl_u[TX_CP437])(status[x]) :
                                        status[x] ;
                    }
                    else
                        thecells[c].c = status[x] ;
                    thecells[c].a = colorstatus ;
                    c++ ;
                }
            }
        }

        msleep(0);
        rc = RequestScreenMutex(-1);
        if ( rc )
        {
            continue ;
        }

        /* only calculated an offset if Roll mode is INSERT */
        cursor_offset = (scrollstatus[avm] && tt_roll[avm]
                         && markmodeflag[avm] == notmarking ) ?
                             (vscrn[avm].top + vscrn[avm].linecount
                              - vscrn[avm].scrolltop)%vscrn[avm].linecount : 0 ;
        if ( VscrnIsPopup(avm) || !cursorena[avm] ||
            vscrn[avm].cursor.x - xho < 0 ||  vscrn[avm].cursor.x - xho >= pwidth ||
            vscrn[avm].cursor.y + cursor_offset >= VscrnGetDisplayHeight(avm)
            -(tt_status[avm]?1:0) ) {
            killcursor(avm) ;
        }
        else {
            newcursor(avm) ;
            GetCurPos( &cursor.y, &cursor.x ) ;
#ifdef NEW_EXCLUSIVE
            /* Wait for exclusive access to the screen buffer */
            rc = RequestVscrnMutex( avm, SEM_INDEFINITE_WAIT ) ;
            if ( !rc ) {
#endif /* NEW_EXCLUSIVE */
                line = &vscrn[avm].lines[(vscrn[avm].top+vscrn[avm].cursor.y)%vscrn[avm].linecount];
                if ( line != NULL )
                if ( line->vt_line_attr & VT_LINE_ATTR_DOUBLE_WIDE ) {
                    if ((vscrn[avm].cursor.x - xho)*2 != cursor.x ||
                         vscrn[avm].cursor.y + cursor_offset != cursor.y )
                        SetCurPos( vscrn[avm].cursor.y + cursor_offset,
                                   (vscrn[avm].cursor.x - xho)*2 ) ;
                } else {
                    if ( vscrn[avm].cursor.x - xho != cursor.x ||
                         vscrn[avm].cursor.y + cursor_offset != cursor.y )
                        SetCurPos( vscrn[avm].cursor.y + cursor_offset,
                                   vscrn[avm].cursor.x - xho ) ;
                }
#ifdef NEW_EXCLUSIVE
                rc = ReleaseVscrnMutex( avm ) ;
            }
#endif /* NEW_EXCLUSIVE */

        }
        /* Paint the screen */
        if ( tt_diff_upd && (!isWin95() ||
             !tt_attr_bug ||
             ( pwidth == 80 &&
               ( pheight == 25 || pheight == 43 || pheight == 50 ) ))
             )
            WrtCellStrDiff( thecells, c, 0, 0,
                            (tt_modechg == TVC_ENA ) ? vscrn[avm].height : pheight,
                            (tt_modechg == TVC_ENA ) ? vscrn[avm].width : pwidth);
        else {
            WrtCellStr( thecells, c, 0, 0 );
        }

        ReleaseScreenMutex() ;
    }

    StopVscrnTimer() ;

#ifdef NT
    /* we want to turn on processing of Window Events */
    GetConsoleMode( KbdHandle, &conmode ) ;
    conmode &= ~ENABLE_WINDOW_INPUT ;
    rc = SetConsoleMode( KbdHandle, conmode ) ;
#endif /* NT */

    CloseVscrnMuxWait(VCMD) ;
    CloseVscrnMuxWait(VTERM) ;
    CloseVscrnMuxWait(VCS) ;
    CloseVscrnTimerSem() ;
    free(thecells) ;
    PostTermScrnUpdThreadDownSem();
    ckThreadEnd(threadinfo) ;
}

#ifdef PCFONTS
APIRET
os2LoadPCFonts( void )
{
    extern char exedir[];
    CHAR dll[_MAX_PATH], failure[_MAX_PATH] ;
    char *p, *q;
    APIRET rc = 0 ;

    if ( hPCFonts )  /* PC Fonts DLL already loaded */
        return 0 ;

    p = q = strdup( exedir ) ;
    if (p) {
        while (*p) {
            if (*p == '/') *p = '\\';
            if (islower(*p)) *p = toupper(*p);
            p++ ;
            }
        p = q ;
        }
    else {
        p = exedir;
        }

    ckstrncpy(dll,p,_MAX_PATH);
    ckstrncat(dll,"PCFONTS.DLL",_MAX_PATH);
    debug(F110,"os2LoadPCFonts dll",dll,0);
    rc = DosLoadModule( failure, _MAX_PATH, dll, &hPCFonts ) ;
    if ( rc )
    {
       debug(F111,"os2LoadPCFonts DosLoadModule failed",failure,rc) ;
       hPCFonts = 0 ;
    }
    if ( q )
       free(q) ;
    return rc ;
}

APIRET
os2SetFont( void )
{
    APIRET rc = 0 ;
    VIOFONTINFO fi;
    PBYTE pFont ;
    BYTE  buffer[FONTBUFSIZE] ;
    ULONG FontSize, FontID ;

    memset( &fi, 0, sizeof(fi) ) ;
    fi.cb = sizeof(fi) ;

    if ( tt_font == TTF_ROM ) {
        FontSaved = 0 ;
        }
    else {
        if ( !hPCFonts ) {
            rc = os2LoadPCFonts() ;
            if (rc) {
                debug( F101, "os2LoadPCFonts failed rc","",rc) ;
                return rc ;
            }
        }

        fi.type = VGFI_GETCURFONT ;
        fi.cxCell = 0 ;
        fi.cyCell = 0 ;
        fi.pbData = buffer ;
        fi.cbData = FONTBUFSIZE ;
        rc = VioGetFont( &fi, VioHandle ) ;

        memcpy( &SavedFontInfo, &fi, sizeof(VIOFONTINFO) ) ;
        SavedFontInfo.pbData=SavedFontBuffer ;
        memcpy( SavedFontBuffer, buffer, FONTBUFSIZE ) ;
        FontSaved = 1 ;

        debug( F101, "os2SetFont VioGetFont rc","",rc ) ;
        debug( F101, "type","",fi.type ) ;
        debug( F101, "cxCell","",fi.cxCell ) ;
        debug( F101, "cyCell","",fi.cyCell ) ;
        debug( F101, "cbData","",fi.cbData ) ;
        debug( F101, "loading font","",tt_font);

        FontID = fi.cyCell*1000+tt_font ;
        rc = DosGetResource( hPCFonts,
            (ULONG) PCFONT,
            (ULONG) FontID,
            (PPVOID) &pFont );
        if (!rc) {
            rc = DosQueryResourceSize( hPCFonts, PCFONT, FontID, &FontSize ) ;

            fi.pbData = pFont ;
            fi.cbData = FontSize ;

            rc = VioSetFont( &fi, VioHandle ) ;
            DosFreeResource( pFont ) ;
            debug( F101, "os2SetFont rc","",rc) ;
            }
        else {
            debug( F101, "DosGetResource failed rc","",rc ) ;
            }
        }

    return rc ;
}


APIRET
os2ResetFont( void )
{
    APIRET rc = 0 ;

    if ( FontSaved )
        rc = VioSetFont( &SavedFontInfo, VioHandle ) ;
    debug(F101,"os2ResetFont Set rc","",rc);
    return rc ;
}
#endif /* PCFONTS */

/*---------------------------------------------------------------------------*/
/* killcursor                                                                */
/*---------------------------------------------------------------------------*/
void
killcursor( BYTE vmode ) {
    CK_CURSORINFO crsr_info;
    debug(F100,"killcursor","",0);
    if (!cursoron[vmode])                       /* It's already off */
        return;
    GetCurType(&crsr_info);     /* Store current cursor type */
    crsr_info.attr = -1;
    if ( !SetCurType(&crsr_info) )      /* Hide cursor */
    {
        cursoron[vmode] = FALSE;
    }
}

/*---------------------------------------------------------------------------*/
/* newcursor                                                                 */
/*---------------------------------------------------------------------------*/
void
newcursor( BYTE vmode ) {
    CK_CURSORINFO vci;
    int cell, bottom, top;

    debug(F100,"newcursor","",0);
    if (cursoron[vmode])                        /* It's already on */
      return;

    GetCurType(&vci);

#ifdef NT
    switch (tt_cursor) {
        case 0:
            vci.yStart = 12 ;
            break;
        case 1:
            vci.yStart = 50 ;
            break;
        case 2:
            vci.yStart = 88 ;
            break;
        }
#else /* NT */
    vci.cEnd   = -100;
    switch (tt_cursor) {
        case 0:
            vci.yStart = -90 ;
            break;
        case 1:
            vci.yStart = -50 ;
            break;
        case 2:
            vci.yStart = -10 ;
            break;
        }
#endif /* NT */
    vci.attr = 1 ;

    if ( !SetCurType(&vci) )
    {
        cursoron[vmode] = TRUE;
        VscrnIsDirty(vmode);
    }
}

void
shovscrn(void)
{
    printf("\n");
    printf("Virtual Screen settings:\n");
    printf("                ____VCMD____ ___VTERM____ ____VCS_____\n");
    printf("     linecount: %12d %12d %12d\n", vscrn[VCMD].linecount,
            vscrn[VTERM].linecount, vscrn[VCS].linecount) ;
    printf("         lines: %12s %12s %12s\n",
            vscrn[VCMD].lines ? "allocated" : "Null",
            vscrn[VTERM].lines ? "allocated" : "Null",
            vscrn[VCS].lines ? "allocated" : "Null") ;
    printf("           beg: %12d %12d %12d\n", vscrn[VCMD].beg,
            vscrn[VTERM].beg, vscrn[VCS].beg ) ;
    printf("           top: %12d %12d %12d\n", vscrn[VCMD].top,
            vscrn[VTERM].top, vscrn[VCS].top ) ;
    printf("     scrolltop: %12d %12d %12d\n", vscrn[VCMD].scrolltop,
            vscrn[VTERM].scrolltop, vscrn[VCS].scrolltop) ;
    printf("           end: %12d %12d %12d\n", vscrn[VCMD].end,
            vscrn[VTERM].end, vscrn[VCS].end ) ;
    printf("        cursor:      %3d,%3d      %3d,%3d      %3d,%3d\n",
            vscrn[VCMD].cursor.x, vscrn[VCMD].cursor.y,
            vscrn[VTERM].cursor.x, vscrn[VTERM].cursor.y,
            vscrn[VCS].cursor.x, vscrn[VCS].cursor.y) ;
    printf("         popup: %12s %12s %12s\n",
            vscrn[VCMD].popup ? "yes" : "no",
            vscrn[VTERM].popup ? "yes" : "no",
            vscrn[VCS].popup ? "yes" : "no") ;
    printf("       marktop: %12d %12d %12d\n",
            vscrn[VCMD].marktop,
            vscrn[VTERM].marktop,
            vscrn[VCS].marktop) ;
    printf("       markbot: %12d %12d %12d\n",
            vscrn[VCMD].markbot,
            vscrn[VTERM].markbot,
            vscrn[VCS].markbot) ;
    printf("        height: %12d %12d %12d\n",
            vscrn[VCMD].height,
            vscrn[VTERM].height,
            vscrn[VCS].height) ;
    printf("         width: %12d %12d %12d\n",
            vscrn[VCMD].width,
            vscrn[VTERM].width,
            vscrn[VCS].width) ;
    printf("       hscroll: %12d %12d %12d\n",
            vscrn[VCMD].hscroll,
            vscrn[VTERM].hscroll,
            vscrn[VCS].hscroll) ;
    printf("display_height: %12d %12d %12d\n",
            vscrn[VCMD].display_height,
            vscrn[VTERM].display_height,
            vscrn[VCS].display_height) ;
    printf("\n");
}

APIRET
VscrnInit( BYTE vmode )
{
   extern int tt_szchng[], tt_scrsize[] ;
   extern int cmd_rows, cmd_cols, marginbot ;
   extern int updmode, tt_updmode, SysInited ;
   extern ascreen commandscreen, vt100screen ;
   extern unsigned char                 /* Video attribute bytes */
      attribute,                      /* Current video attribute byte */
      underlineattribute,
      savedattribute,                 /* Saved video attribute byte */
      defaultattribute;               /* Default video attribute byte */
   extern int scrninitialized[] ;
   extern unsigned char colornormal, colorunderline, colorborder,
    colorreverse, colorgraphic, colorcmd, coloritalic ;
   BYTE clrscr = 0 ;
#ifndef KUI
   CK_VIDEOMODEINFO m;
#endif /* KUI */

   debug(F111,"VscrnInit","vmode",vmode);
#ifndef KUI
   if (GetMode(&m))
   {
       debug(F111,"VscrnInit GetMode() fails","vmode",vmode);
   }
#endif /* KUI */
   if ( vmode == VCMD )
   {
       if ( cmd_rows < 0 || cmd_cols < 0 || tt_rows[VCMD] < 0 || tt_cols[VCMD] < 0 )
           ttgcwsz();
   }

   if ( VscrnGetBufferSize(vmode) != tt_scrsize[vmode] || VscrnGetWidth(vmode)  <= 0
        || VscrnGetHeight(vmode) <= 0 || tt_cols[vmode] <= 0 || tt_rows[vmode] <= 0 ) 
   {
       scrninitialized[vmode] = 0;

       debug(F101,"tt_cols","",tt_cols[vmode]);
       debug(F101,"tt_rows","",tt_rows[vmode]);
#ifndef KUI
       debug(F101,"m.row","",m.row);
       debug(F101,"m.col","",m.col);
       if ( tt_cols[vmode] <= 0 || tt_rows[vmode] <= 0 ) {
           tt_cols[vmode] = min(MAXTERMCOL, m.col);
           tt_rows[vmode] = min(MAXTERMROW, tt_status[vmode] ? m.row-1 : m.row);
       }
#else /* KUI */
       if ( tt_cols[vmode] <= 0 || tt_rows[vmode] <= 0 ) {
           tt_cols[vmode] = MAXTERMCOL;
           tt_rows[vmode] = MAXTERMROW;
       }
#endif /* KUI */
   }

   if ( vmode == VTERM )
   {
      if (!scrninitialized[VTERM]) {
          scrninitialized[VTERM] = 1;
          attribute = defaultattribute = colornormal;
          underlineattribute = colorunderline ;
          reverseattribute = colorreverse ;
          italicattribute = coloritalic;
          graphicattribute = colorgraphic ;
          borderattribute = colorborder ;
          updmode = tt_updmode ;  /* Set screen update mode */
      }
      if ( marginbot == VscrnGetHeight(VTERM)-(tt_status[vmode]?1:0) ||
           VscrnGetHeight(VTERM) < 0 ||
           marginbot > tt_rows[VTERM] )
         marginbot = tt_rows[VTERM];
   }

   VscrnSetWidth( vmode, tt_cols[vmode] ) ;
   VscrnSetHeight( vmode, tt_rows[vmode]+(tt_status[vmode]?1:0) );

    /* Initialize paging info */
    clrscr = VscrnSetBufferSize( vmode, tt_scrsize[vmode] ) ;

    if ( tt_szchng[vmode] ) {
        LONG sz ;
        /* Wait for exclusive access to the screen */
        RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
        sz = (VscrnGetEnd(vmode) - VscrnGetTop(vmode)
               + VscrnGetBufferSize(vmode) + 1)%VscrnGetBufferSize(vmode) ;
        if ( !clrscr )
            if ( sz > tt_rows[vmode] )
            {
                if ( !VscrnIsClear(vmode) ) {
                    VscrnScroll( vmode, UPWARD, 0, sz-1, sz-1, TRUE, SP ) ;
                    clrscr = 1 ;
                }
            }
#ifdef COMMENT
            else if ( tt_szchng[vmode] == 2 ) /* Status Line Turned On */
            {
                if (!VscrnIsClear(vmode)) {
                    VscrnScroll( vmode, UPWARD, 0, sz, sz, TRUE, SP ) ;
                    clrscr = 1 ;
                }
                else {
                }
            }
#endif
            else
            {
                int             x,y ;
                videoline *     line ;
                int             foo = 0;

                if ( VscrnGetEnd(vmode) == VscrnGetBufferSize(vmode) ||
                     VscrnGetEnd(vmode) + 1 == VscrnGetBegin(vmode) )
                    foo++;

                VscrnSetEnd( vmode, VscrnGetTop(vmode) +
                             VscrnGetHeight(vmode)-1
                             -(tt_status[vmode]?1:0)) ;
                if ( foo )
                    VscrnSetBegin( vmode, VscrnGetEnd(vmode) + 1);

                for ( y = sz ;
                      y < VscrnGetHeight(vmode)-(tt_status[vmode]?1:0) ;
                      y++ ) {
                    line = VscrnGetLineFromTop(vmode,y) ;
                    if ( line ) {
                        line->width = VscrnGetWidth(vmode)  ;
                        line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
                        for ( x = 0 ; x < MAXTERMCOL ; x++ ) {
                            line->cells[x].c = ' ' ;
                            line->cells[x].a = vmode == VTERM ? attribute : colorcmd;
                            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
                        }
                    }
                }
            }
        ReleaseVscrnMutex(vmode) ;
        tt_szchng[vmode] = 0 ;
    }

    if ( clrscr ) {
        scrollflag[vmode] = FALSE ;
        scrollstatus[vmode] = FALSE ;
        cleartermscreen(vmode);
    }
    scrninitialized[vmode] = TRUE ;
    return 0;
}

BOOL
VscrnIsClear( BYTE vmode )
{
    int             x,y ;
    videoline *     line ;
    unsigned char cellcolor = geterasecolor(vmode);

    for ( y = 0 ; y < VscrnGetHeight(vmode)-(tt_status[vmode]?1:0) ; y++ ) {
        line = VscrnGetLineFromTop(vmode,y) ;
		if (line == NULL)
			return 0;
        for ( x = 0 ; x < line->width ; x++ ) {
            if ( line->cells[x].c != ' ' )
            {
                debug(F100,"VscrnIsClear c!=' '","",0);
                return 0;
            }
            if ( line->cells[x].a != cellcolor )
            {
                debug(F100,"VscrnIsClear a != cellcolor","",0);
                return 0;
            }
        }
    }
    debug(F100,"VscrnIsClear all clear","",0);
    return 1 ;
}
#endif /* NOLOCAL */

#undef printf
#undef fprintf
#undef putchar
extern int k95stdio,k95stdin,k95stdout;
#ifdef IKSD
extern int inserver;
#endif /* IKSD */

static char myprtfstr[2*CMDBL];
static char myprtfstr2[2*CMDBL];
/*
* printf for console
*/
int
Vscrnprintf (const char *format, ...) {
#ifndef NOLOCAL
    extern unsigned char colorcmd ;
    BYTE vmode = VCMD ;
    extern int wherex[], wherey[];
#endif /* NOLOCAL */
    int i, len, rc=0;
    char *cp;
    va_list ap;
    static int printf_inprogress=0;

    va_start(ap, format);
#ifndef ONETERMUPD
    rc = vprintf( format, ap ) ;
#endif /* ONETERMUPD */
#ifdef NT
    rc = _vsnprintf(myprtfstr, sizeof(myprtfstr)-1, format, ap);
#else /* NT */
    rc = vsprintf(myprtfstr, format, ap);
#endif /* NT */
    va_end(ap);

/*
 *  if ((i = strlen(string)) >= sizeof(string))
 *     debug(F101,"Overran allocation for string in printf(), len:","",i);
 */

#ifdef IKSD
    if ( inserver ) {
        int i,j,len,got_cr;
        debug(F111,"Vscrnprintf inserver",
               "printf_inprogress",printf_inprogress);
        if (!printf_inprogress) {
            printf_inprogress = 1;
            for (i = 0, j = 0, len = strlen(myprtfstr), got_cr = 0;
                  i < len && j < 1022;
                  i++, j++ ) {
                switch (myprtfstr[i]) {
                case '\r':
                    if (got_cr && !TELOPT_ME(TELOPT_BINARY))
                        myprtfstr2[j++] = '\0';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 1;
                    break;
                case '\n':
                    if (!got_cr)
                        myprtfstr2[j++] = '\r';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 0;
                    break;
                default:
                    if (got_cr && !TELOPT_ME(TELOPT_BINARY))
                        myprtfstr2[j++] = '\0';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 0;
                }
            }
            ttol(myprtfstr2,j);
            printf_inprogress = 0;
        }
    }
#endif /* IKSD */

#ifndef NOLOCAL
    if ( k95stdout )
        fwrite(myprtfstr,1,strlen(myprtfstr),stdout);
    else
        VscrnWrtCharStrAtt( vmode, myprtfstr, strlen(myprtfstr),
                            wherey[vmode],
                            wherex[vmode],
                            &colorcmd
                            ) ;
#endif /* NOLOCAL */
    return (rc);
}

/*
* fprintf for console
*/
int
Vscrnfprintf (FILE * file, const char *format, ...) {
#ifndef NOLOCAL
    extern unsigned char colorcmd ;
    BYTE vmode = VCMD ;
    extern int wherex[], wherey[];
#endif /* NOLOCAL */
    int i, len, rc=0;
    char *cp;
    va_list ap;
    static int fprintf_inprogress = 0;

    va_start(ap, format);

#ifdef IKSD
    if ( inserver ) {
        int i,j,len,got_cr;
#ifdef NT
        rc = _vsnprintf(myprtfstr, sizeof(myprtfstr)-1, format, ap);
#else /* NT */
        rc = vsprintf(myprtfstr, format, ap);
#endif /* NT */
        debug(F111,"Vscrnfprintf inserver",
               "fprintf_inprogress",fprintf_inprogress);
        if (!fprintf_inprogress) {
            fprintf_inprogress = 1;
            for (i = 0, j = 0, len = strlen(myprtfstr), got_cr = 0;
                  i < len && j < 1022;
                  i++, j++ ) {
                switch (myprtfstr[i]) {
                case '\r':
                    if (got_cr && !TELOPT_ME(TELOPT_BINARY))
                        myprtfstr2[j++] = '\0';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 1;
                    break;
                case '\n':
                    if (!got_cr)
                        myprtfstr2[j++] = '\r';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 0;
                    break;
                default:
                    if (got_cr && !TELOPT_ME(TELOPT_BINARY))
                        myprtfstr2[j++] = '\0';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 0;
                }
            }
            ttol(myprtfstr2,j);
            fprintf_inprogress = 0;
        }
    }
#endif /* IKSD */
#ifndef NOLOCAL
    if ( file == stdout || file == stderr )
    {
#ifdef NT
        rc = _vsnprintf(myprtfstr, sizeof(myprtfstr)-1, format, ap);
#else /* NT */
        rc = vsprintf(myprtfstr, format, ap);
#endif /* NT */
        /*
        *  if ((i = strlen(string)) >= sizeof(string))
        *     debug(F101,"Overran allocation for string in printf(), len:","",i);
        */

        if ( k95stdout )
            fwrite(myprtfstr,1,strlen(myprtfstr),stdout);
        else
            VscrnWrtCharStrAtt( vmode, myprtfstr, strlen(myprtfstr),
                                wherey[vmode],
                                wherex[vmode],
                                &colorcmd
                                ) ;
    }
    else
        rc = vfprintf(file, format, ap);
#endif /* NOLOCAL */
    va_end(ap);
    return (rc);
}

void
Vscrnperror( const char *str )
{
    char * errstr = ck_errstr() ;
    Vscrnprintf("%s%s %s\n",str,*errstr?":":"",errstr) ;
}

#ifdef KUI
int
Vscrnprintw (const char *format, ...) {
#ifndef NOLOCAL
    extern unsigned char colorcmd ;
    BYTE vmode = VCMD ;
    extern int wherex[], wherey[];
#endif /* NOLOCAL */
    int i, len, rc=0;
    char *cp;
    va_list ap;
    static int printf_inprogress=0;

    va_start(ap, format);
#ifndef ONETERMUPD
    rc = vprintf( format, ap ) ;
#endif /* ONETERMUPD */
#ifdef NT
    rc = _vsnprintf(myprtfstr, sizeof(myprtfstr)-1, format, ap);
#else /* NT */
    rc = vsprintf(myprtfstr, format, ap);
#endif /* NT */
    va_end(ap);

/*
 *  if ((i = strlen(string)) >= sizeof(string))
 *     debug(F101,"Overran allocation for string in printf(), len:","",i);
 */

#ifdef IKSD
    if ( inserver ) {
        int i,j,len,got_cr;
        debug(F111,"Vscrnprintf inserver",
               "printf_inprogress",printf_inprogress);
        if (!printf_inprogress) {
            printf_inprogress = 1;
            for (i = 0, j = 0, len = strlen(myprtfstr), got_cr = 0;
                  i < len && j < 1022;
                  i++, j++ ) {
                switch (myprtfstr[i]) {
                case '\r':
                    if (got_cr && !TELOPT_ME(TELOPT_BINARY))
                        myprtfstr2[j++] = '\0';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 1;
                    break;
                case '\n':
                    if (!got_cr)
                        myprtfstr2[j++] = '\r';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 0;
                    break;
                default:
                    if (got_cr && !TELOPT_ME(TELOPT_BINARY))
                        myprtfstr2[j++] = '\0';
                    myprtfstr2[j] = myprtfstr[i];
                    got_cr = 0;
                }
            }
            ttol(myprtfstr2,j);
            printf_inprogress = 0;
        }
    }
#endif /* IKSD */

#ifndef NOLOCAL
    if ( k95stdout )
        fwrite(myprtfstr,1,strlen(myprtfstr),stdout);
    else {
        int tcsl_save;
        extern int tcsl;

        tcsl_save = tcsl;
        tcsl = TX_CP437;
        VscrnWrtCharStrAtt( vmode, myprtfstr, strlen(myprtfstr),
                            wherey[vmode],
                            wherex[vmode],
                            &colorcmd
                            ) ;
        tcsl = tcsl_save;
    }
#endif /* NOLOCAL */
    return (rc);
}
#endif /* KUI */

#ifndef NOLOCAL
#ifdef NT
#ifndef KUI
/* This code works by forcing the standard handles to be valid. */
/* If no console was allocated, the AllocConsole() will succeed */
/* if not, then, well, who cares.  The standard handles might   */
/* be invalid anyway.                                           */

void
Win32ConsoleInit( void )
{
    int hCrt,i;
    FILE *hf;
    HANDLE handle ;
    DWORD filetype;
    extern int k95stdout, k95stdin;

    handle = GetStdHandle(STD_INPUT_HANDLE);
    filetype = GetFileType(handle);
    debug(F111,"Win32ConsoleInit","GetFileType(stdin)",filetype);
    if ( filetype == FILE_TYPE_DISK || filetype == FILE_TYPE_PIPE ) {
        k95stdin = 1;
    }

    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    filetype = GetFileType(handle);
    debug(F111,"Win32ConsoleInit","GetFileType(stdout)",filetype);
    if ( filetype == FILE_TYPE_DISK || filetype == FILE_TYPE_PIPE ) {
        k95stdout = 1;
    }

    if ( !isWin95() || k95stdout || k95stdin )
        return;

    AllocConsole();
    hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE),
                                _O_BINARY);
    hf = _fdopen( hCrt, "wb" );
    *stdout = *hf;
    i = setvbuf( stdout, NULL, _IONBF, 0 );

    if ( filetype != FILE_TYPE_DISK && filetype != FILE_TYPE_PIPE ) {
        handle = CreateFile( "CONIN$",
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
        SetStdHandle( STD_INPUT_HANDLE, handle ) ;
    }
    hCrt = _open_osfhandle((long) handle, _O_BINARY);
    hf = _fdopen( hCrt, "rb" );
    *stdin = *hf;
    i = setvbuf( stdin, NULL, _IONBF, 0 );

    hCrt = _open_osfhandle((long) GetStdHandle(STD_ERROR_HANDLE), _O_BINARY);
    hf = _fdopen( hCrt, "wb" );
    *stderr = *hf;
    i = setvbuf( stderr, NULL, _IONBF, 0 );
}
#endif /* KUI */
#endif /* NT */

#ifdef KUI
#define MAXFNTS 1024
static struct _font_list {
    int count;
    char * name[MAXFNTS];
} * font_list = NULL;

static int CALLBACK
EnumFontFamProc( ENUMLOGFONT *lpelfe,    // logical-font data
                 NEWTEXTMETRICEX *lpntme,  // physical-font data
                 DWORD FontType,           // type of font
                 LPARAM lParam             // application-defined data
                 )
{
    int i;
    char * name;

    if ( font_list == NULL) {
        debug(F110,"EnumFontFamProc()","font_list == NULL",0);
        return(0);
    }

    if (font_list->count == MAXFNTS ) {
        debug(F111,"EnumFontFamProc()","font_list->count == MAXFNTS",MAXFNTS);
        return(MAXFNTS);
    }

    /* Windows 95/98 handles the font names different than NT/2000 */
    /* Why?  You know better than to ask that question.            */
    if ( isWin95() ) {
        if ( FontType == 4 /* True Type */ )
            name = lpelfe->elfFullName;
        else
            name = lpelfe->elfLogFont.lfFaceName;
    } else
        name = lpelfe->elfFullName;

    debug(F110,"EnumFontFamProc() name",name,0);
    for ( i=0; i < font_list->count; i++ ) {
        if ( !strcmp(name,font_list->name[i]) )
            return(font_list->count);
    }
    font_list->name[font_list->count++] = strdup(name);
    return(font_list->count);
}


static int CALLBACK
EnumFontFamExProc( ENUMLOGFONTEX *lpelfe,    // logical-font data
                   NEWTEXTMETRICEX *lpntme,  // physical-font data
                   DWORD FontType,           // type of font
                   LPARAM lParam             // application-defined data
                   )
{
    int i;
    char * name;

    if ( font_list == NULL) {
        debug(F110,"EnumFontFamExProc()","font_list == NULL",0);
        return(0);
    }

    if (font_list->count == MAXFNTS ) {
        debug(F111,"EnumFontFamExProc()","font_list->count == MAXFNTS",MAXFNTS);
        return(MAXFNTS);
    }

    /* Windows 95/98 handles the font names different than NT/2000 */
    /* Why?  You know better than to ask that question.            */
    if ( isWin95() ) {
        if ( FontType == 4 /* True Type */ )
            name = lpelfe->elfFullName;
        else
            name = lpelfe->elfLogFont.lfFaceName;
    } else
        name = lpelfe->elfFullName;

    debug(F110,"EnumFontFamExProc() name",name,0);
    for ( i=0; i < font_list->count; i++ ) {
        if ( !strcmp(name,font_list->name[i]) )
            return(font_list->count);
    }
    font_list->name[font_list->count++] = strdup(name);
    return(font_list->count);
}

static int
EnumerateFonts()
{
    LOGFONT lf;
    HDC hdc;
    int i,rc=0;
    extern int nt351;

    debug(F100,"EnumerateFonts() start","",0);

    if ( font_list != NULL ) {
        for ( i=0 ; i < font_list->count; i++ ) {
            free(font_list->name[i]);
            font_list->name[i] = NULL;
        }
        font_list->count = 0;
    } else {
        font_list = malloc(sizeof(struct _font_list));
        if ( !font_list )
            return(0);
        memset(font_list,0,sizeof(struct _font_list));
    }

    hdc = CreateDC("DISPLAY",NULL,NULL,NULL);
    debug(F111,"EnumerateFonts() CreateDC()","DISPLAY",hdc);
    if ( hdc ) {
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfFaceName[0] = '\0';
        lf.lfPitchAndFamily = 0;

        if ( nt351 )
            rc = EnumFontFamilies( (HDC) hdc, NULL,
                                     (FONTENUMPROC) EnumFontFamProc,
                                     0);
        else
            rc = EnumFontFamiliesEx( (HDC) hdc, (LPLOGFONT)&lf,
                                 (FONTENUMPROC) EnumFontFamExProc,
                                 0, 0);
        debug(F111,"EnumerateFonts()","EnumFontFamiliesEx()",rc);
        DeleteDC(hdc);
    }
    debug(F111,"EnumerateFonts() end","rc",rc);
    return rc;
}


int
BuildFontTable( struct keytab ** pTable, struct keytab ** pTable2, int * pN )
{
    int i, n ;
    if ( *pTable )
    {
        for ( i=0 ; i < *pN ; i++ )
            free( (*pTable)[i].kwd ) ;
        free ( *pTable )  ;
    }

    if ( *pTable2 )
    {
        for ( i=0 ; i < *pN ; i++ )
            free( (*pTable2)[i].kwd ) ;
        free ( *pTable2 )  ;
    }

    *pTable = NULL ;
    *pTable2 = NULL ;
    *pN = 0 ;

    debug(F100,"BuildFontTable","",0);
    n = EnumerateFonts();
    if ( n )
    {
        *pTable = malloc( sizeof(struct keytab) * n ) ;
        *pTable2 = malloc( sizeof(struct keytab) * n ) ;

        for ( i=0 ; i < font_list->count ; i++ ) {
            if ( font_list->name[i] == NULL )
            {
                debug(F111,"font_list name entry is NULL","i",i);
            }
            else
            {

               char * newstr = _strdup(font_list->name[i]) ;
               char * newstr2 = _strdup(font_list->name[i]) ;
               int    newval = i;
               int j = 0, len = 0;

               debug(F111,"Font Name",newstr,i);

               /* Make a version that uses Underscores instead of spaces */
                           /* And lowercases the letters */
               len = strlen(newstr2);
               for ( j=0 ; j<len ; j++) {
                   newstr2[j] = tolower(newstr2[j]);
                   switch ( newstr2[j] ) {
                   case ' ':
                       newstr2[j] = '_';
                       break;
                   case ',':
                       newstr2[j] = '.';
                       break;
                   case ';':
                       newstr2[j] = ':';
                       break;
                   case '\\':
                       newstr2[j] = '/';
                       break;
                   case '?':
                       newstr2[j] = '!';
                       break;
                   case '{':
                       newstr2[j] = '[';
                       break;
                   case '}':
                       newstr2[j] = ']';
                       break;
                   }
               }

               for ( j=0 ; j < (*pN) ; j++ ) {
                   int tempval=0;
                   char * tempstr=NULL,*tempstr2=NULL;

                   if ( _stricmp( (*pTable2)[j].kwd, newstr2 ) > 0 )
                   {
                       tempval = (*pTable)[j].kwval;
                       tempstr = (*pTable)[j].kwd;
                       tempstr2 = (*pTable2)[j].kwd;
                       (*pTable)[j].kwd = newstr ;
                       (*pTable)[j].kwval = newval;
                       (*pTable2)[j].kwd = newstr2 ;
                       (*pTable2)[j].kwval = newval;
                       newval = tempval;
                       newstr = tempstr;
                       (*pTable)[j].flgs = 0;
                       newstr2 = tempstr2;
                       (*pTable2)[j].flgs = 0;
                   }
               }
               (*pTable)[*pN].kwd = newstr ;
               (*pTable)[*pN].kwval = newval;
               (*pTable)[*pN].flgs = 0 ;
               (*pTable2)[*pN].kwd = newstr2 ;
               (*pTable2)[*pN].kwval = newval;
               (*pTable2)[*pN].flgs = 0 ;
               (*pN)++ ;
           }
       }

       if ( *pN == 0 ) {
           free ( *pTable )  ;
           free ( *pTable2 )  ;
           *pTable = NULL;
           *pTable2 = NULL;
       }
    }
    return(1);
}
#endif /* KUI */
#endif /* NOLOCAL */
