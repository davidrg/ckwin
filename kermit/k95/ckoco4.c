/* C K O C O 4 . C */

/*
  Author: Frank da Cruz  (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
            Columbia University Academic Information Systems, New York City.
          Jeffrey E Altman (jaltman@secure-endpoints.com)
            Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include "ckcdeb.h"

#ifdef NT
#include <windows.h>
#define strnicmp _strnicmp
#else /* NT */

#define INCL_WIN
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#define INCL_DOSDATETIME
#define INCL_DOSMEMMGR
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */
#endif /* NT */

#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckocon.h"
#include "ckokey.h"
#include "ckcxla.h"             /* Character set translation */
#include "ckcuni.h"             /* Unicode Character Set Translations */
#include <stdio.h>

extern int tnlm, tn_nlm;        /* Terminal newline mode, ditto for TELNET */
#ifndef NOTERM
extern int tt_status[VNUM] ;
#endif /* NOTERM */
#ifndef NOLOCAL
extern videobuffer vscrn[] ;
extern enum markmodes markmodeflag[] ;
extern bool xprintff, printon ;
#ifndef NT
extern HAB hab ;
#endif /* NT */

static char * selection = NULL ;
static unsigned short * Uselection = NULL;
static int    nselect   = 0 ;
static char * url       = NULL ;
static int    nurl      = 0 ;
static int    url_type  = 0 ;

#define MAXURLLEN 4095

const char *
GetSelection()
{
    return selection;
}

int
GetURLType(void)
{
    return url_type;
}

const char *
GetURL()
{
    return url;
}

static char *
FindURL( const char * s )
{
    /* We have a string.  Is it a URL? */
    /*  telnet      */
    /*  http(s)     */
    /*  gopher      */
    /*  (s)news     */
    /*  mailto      */
    /*  ftp         */
    /*  tcp, udp, ... */
    /*          Does it have a valid host?  If not, return               */
    /*          Is it telnet: rlogin: ssh:? If so, start a new K95 session */
    /*          Otherwise, start the browser                             */
    /* If not, */
    /*          Check the string to see if it is a mail message          */
    /*               if so, does it have a mailto: prefix? Start Browser */
    /*                          If not, add one and start browser        */
    /*          Otherwise, is it a valid host?  If so, http://<host>/    */
    /*          Does adding 'www' or '.edu', '.com', make it valid?      */
    /*          If so, start browser                                     */

    int i, len;
    char * newurl = NULL;
    int mailto = 0;

    if ( strnicmp(s,"telnet:",7) &&
         strnicmp(s,"login:",6)  &&
         strnicmp(s,"ssh:",4)  &&
         strnicmp(s,"http:",5) &&
         strnicmp(s,"gopher:",7) &&
         strnicmp(s,"news:",5) &&
         strnicmp(s,"snews:",6) &&
         strnicmp(s,"ftp:",4) &&
         strnicmp(s,"mailto:",7) &&
         strnicmp(s,"https:",6) &&
         strnicmp(s,"ftps:",5) &&
         strnicmp(s,"telnets:",8) &&
         strnicmp(s,"iksd:",5) &&
         strnicmp(s,"kermit:",7) &&
         strnicmp(s,"tcp:", 4) &&
         strnicmp(s,"udp:", 4) )
    {
        /* Not one of the well known URL services */

        /* we are not going to perform host checking */
        /* if it looks like a mail address (contains an @) we will preface */
        /* mailto: */
        len = strlen(s);
        for ( i=0;i<len;i++ )
        {
            if ( !mailto && s[i] == '@' ) {
                mailto = 1;
            }
            else if ( mailto ) {
                switch ( s[i] ) {
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
    }

    if ( mailto ) {
        newurl = (char *) malloc(strlen(s) + strlen("mailto:") + 1);
        if ( !newurl )
            return(NULL);

        strcpy( newurl, "mailto:" );
        strcat( newurl, s);
    } else {
        newurl = (char *) malloc(strlen(s) + 1);
        if ( !newurl )
            return(NULL);
        strcpy( newurl, s );
    }
    len = strlen(newurl);
    if ( newurl[len-1] == '.' || newurl[len-1] == ',' || newurl[len-1] == ';' )
        /* remove unnecessary punctuation */
        newurl[len-1] = '\0';
    return(newurl);
}


APIRET
VscrnURL( BYTE mode, USHORT row, USHORT col )
{
#ifdef BROWSER
    videoline * line=NULL;
    viocell * cells = NULL;
    char    ch, * str=NULL;
    int i,j,len;
    int brow, bcol, erow, ecol;
    extern bool scrollflag[] ;
    extern char browsurl[];

    if ( url )
    {
        free( url ) ;
        url = NULL ;
        nurl = 0 ;
        url_type = 0;
    }

    /* First check the hyperlink attribute */
    if ( scrollflag[mode] )
        line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + row );
    else
        line = VscrnGetLineFromTop( mode, row );
    if ( line->vt_char_attrs[col] & VT_CHAR_ATTR_HYPERLINK ) {
        hyperlink * link = hyperlink_get(line->hyperlinks[col]);
        if ( link ) {
            url = strdup(link->str);
            if ( url ) {
                nurl = strlen( url );
                url_type = link->type;
                ckstrncpy( browsurl, url, 4096 );
                return 0;               /* a valid URL/UNC was found */
            }
        }
        return -1;
    }

    /* From the current row,col we need to determine which line contains
       the beginning and end of the string we think might contain a URL.

       Compute the length of the string

       Then copy the whole string into a temporary buffer

     */

    /* Find the beginning of the URL */

    brow = row;
    bcol = col;
    while(1) {
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + brow );
        else
            line = VscrnGetLineFromTop( mode, brow );
        cells = line->cells;

        while ( bcol >= 0 ) {
            ch = (CHAR) cells[bcol].c;
            if (ch && (isalnum(ch) || ch == '/' || ch == ':' || ch == '.' || ch == '#' ||
                  ch == '|' || ch == '@' || ch == '!' || ch == '-' || ch == '_' ||
                  ch == '?' || ch == '%' || ch == '&' || ch == '+' || ch == '\\' ||
                  ch == '=' || ch == ';' || ch == '~' || ch == ',' || ch == '$'))
            {
                bcol--;
            } else {
                bcol++;
                if ( bcol >= line->width ) {
                    bcol = 0;
                    brow++;
                }
                goto bfound;
            }
        }

        bcol = line->width-1;
        brow--;
    }
  bfound:

    /* Find the end of the URL */
    erow = row;
    ecol = col;
    while(1) {
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + erow );
        else
            line = VscrnGetLineFromTop( mode, erow );
        cells = line->cells;

        while ( ecol < line->width ) {
            ch = (CHAR) cells[ecol].c;
            if (ch && (isalnum(ch) || ch == '/' || ch == ':' || ch == '.' || ch == '#' ||
                  ch == '|' || ch == '@' || ch == '!' || ch == '-' || ch == '_' ||
                  ch == '?' || ch == '%' || ch == '&' || ch == '+' || ch == '\\' ||
                  ch == '=' || ch == ';' || ch == '~' || ch == ',' || ch == '$'))
            {
                ecol++;
            } else {
                ecol--;
                if ( ecol < 0 ) {
                    ecol = line->width - 1;
                    erow--;
                }
                goto efound;
            }
        }

        ecol = 0;
        erow++;
    }
  efound:

    str = (char *) malloc((erow - brow + 1) * line->width);
    if ( !str )
        return(-1);

    if ( brow == erow ) {
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + brow );
        else
            line = VscrnGetLineFromTop( mode, brow );
        cells = line->cells;

        for ( i=bcol,len=0;i<=ecol;i++,len++ )
            str[len] = (CHAR) cells[i].c;
        str[len] = '\0';
    } else {
        /* handle the first row - bcol to end */
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + brow );
        else
            line = VscrnGetLineFromTop( mode, brow );
        cells = line->cells;

        for ( i=bcol,len=0;i<line->width;i++,len++ )
            str[len] = (CHAR) cells[i].c;

        /* handle the complete rows if there are any */
        for ( j=brow+1; j<erow; j++ ) {
            if ( scrollflag[mode] )
                line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + j );
            else
                line = VscrnGetLineFromTop( mode, j );
            cells = line->cells;

            for ( i=0;i<line->width;i++,len++ )
                str[len] = (CHAR) cells[i].c;
        }

        /* handle the last row - begin to ecol */
        if ( scrollflag[mode] )
            line = VscrnGetLine( mode, VscrnGetScrollTop(mode) + erow );
        else
            line = VscrnGetLineFromTop( mode, erow );
        cells = line->cells;

        for ( i=0;i<=ecol;i++,len++ )
            str[len] = (CHAR) cells[i].c;
        str[len] = '\0';
    }

    debug(F000,"mouseurl detected",str,0);
    if (len == 0) {
        free(str);
        return -1;
    }

    url = FindURL(str);
    free(str);
    if ( url ) {
        nurl = strlen( url );
        url_type = HYPERLINK_URL;
        ckstrncpy( browsurl, url, 4096 );
        return 0;               /* a valid URL was found */
    }
#endif /* BROWSER */
    return -1;                  /* failure to find a valid URL */
}

/* 
 * mode = 0 - word wrap with EOL
 * mode = 1 - word wrap without EOL
 * mode = 2 - box with EOL (not yet implemented)
 */

APIRET
VscrnSelect( BYTE vmode, int mode )
{
    /* This code does not work for Hebrew and other BiDi languages */
    videoline * line = NULL;
    ULONG y = 0;
    USHORT x = 0;
    BYTE   *p = NULL;
    USHORT *pU = NULL;
    APIRET rc = 0 ;

    if ( selection )
    {
        free( selection ) ;
        selection = NULL ;
        nselect = 0 ;
    }
    if ( Uselection ) {
        free( Uselection ) ;
        Uselection = NULL ;
    }

    if (vscrn[vmode].marktop == -1 || vscrn[vmode].markbot == -1 ) {
        bleep(BP_WARN) ;
        return -1 ;
        }

    /* Determine size of Selected Text */

    y = vscrn[vmode].marktop ;
    while (1) {
        line = &vscrn[vmode].lines[y] ;
        if ( line->markbeg != -1 && line->markshowend != -1 ) {
            for ( x  = line->markbeg ; x <= line->markshowend ; x++ )
                nselect++ ;
#ifndef NT
            /* there is an OS/2 compiler bug here */
            /* when optimized it generates an infinite loop */
            debug(F100,"VscrnSelect() infinite loop?","",0);
#endif /* NT */
        }

        if ( y == vscrn[vmode].markbot ) {
            break;
        }
        else {
            if ( mode == 0 ) {
                /* Add line termination */
                nselect++ ; /* '\r' */
                nselect++ ; /* '\n' */
            }
            /* Advance counter */
            y++ ;
            if ( y == vscrn[vmode].linecount )
                y = 0 ;
        }
    }
        nselect++ ;  /* terminating NUL */

    selection = (char *) malloc( nselect ) ;
    if ( !selection )
    {
        nselect = 0 ;
        return -1;
    }
    Uselection = (unsigned short *) malloc( nselect * 2 ) ;
    if ( !Uselection )
    {
        free(selection);
        selection = NULL;
        nselect = 0 ;
        return -1;
    }

    /* Copy the data to the selection buffer */
    p = selection ;
    pU = Uselection ;

    while (1) {
        line = &vscrn[vmode].lines[vscrn[vmode].marktop] ;
        if ( line->markbeg != -1 && line->markshowend != -1 )
            for ( line->markbeg ;
                line->markbeg <= line->markshowend;
                line->markbeg++ ) {
#ifdef NT
                if ( isunicode() ) {
                    extern int tcsl;
                    *p = xl_tx[tcsl](line->cells[line->markbeg].c);
                    *pU = line->cells[line->markbeg].c;
                } else
#endif /* NT */
                {
                    *pU = *p = line->cells[line->markbeg].c;
                }
                p++ ;
                pU++;
            }
        line->markbeg = line->markshowend = line->markend = -1 ;

        if ( vscrn[vmode].marktop == vscrn[vmode].markbot ) {
            vscrn[vmode].marktop = vscrn[vmode].markbot = -1 ;
            break;
        }
        else {
            if ( mode == 0 ) {
                /* Add line termination */
                *pU++ = *p++ = '\r' ;
                *pU++ = *p++ = '\n' ;
            }

            /* Advance counter */
            vscrn[vmode].marktop++ ;
            if ( vscrn[vmode].marktop == vscrn[vmode].linecount )
                vscrn[vmode].marktop = 0 ;
        }
    }
    *pU = *p = '\0' ;  /* Terminating NUL */
    return rc;
}

/* This code does not work for Hebrew and other BiDi languages */
APIRET
CopyVscrnToKbdBuffer( BYTE vmode, int select_mode ) {
    APIRET rc = 0 ;
    BYTE * pData = NULL ;
    int i = 0, j = 0, len = 0 ;

    if ( VscrnSelect(vmode,select_mode) )
        return -1 ;

    len = nselect - 1;          /* nselect includes terminating NUL */
    if ( len == 0 )
        return(0);

    if ( isunicode() ) {
        /* Vscrn is in Unicode.  We must perform translations to both */
        /* lcs for keyboard and rcs for sending.  Do not allow        */
        /* sendcharsduplex() to translate.                            */

        if ( vmode == VTERM ) {
            unsigned char * bytes;
            int nbytes;

            nbytes = utorxlat(Uselection[0], &bytes);
            while ( nbytes-- > 0 )
                sendcharduplex(*bytes++,TRUE);
            for ( i=1; i<len; i++ ) {
                if ( Uselection[i-1] != CR || Uselection[i] != LF ) {
                    nbytes = utorxlat(Uselection[i], &bytes);
                    while ( nbytes-- > 0 )
                        sendcharduplex(*bytes++,TRUE);
                }
            }
        } else {
            pData = (char *)malloc(len+1);
            if ( pData == NULL )
                return -1;

            pData[0] = utolxlat(Uselection[0]);
            for ( i=1, j=1 ; i<len ; i++ )
                if ( Uselection[i-1] != CR || Uselection[i] != LF )
                    pData[j++] = utolxlat(selection[i]);
            pData[j] = '\0';
            putkeystr( vmode, pData );
        }
    } else {
        /* The Vscrn is in the local character set                */
        /* Allow translation to be performed by sendcharsduplex() */

        pData = (char *)malloc(len+1);
        if ( pData == NULL )
            return -1;

        pData[0] = selection[0];
        for ( i=1, j=1 ; i<len ; i++ )
            if ( selection[i-1] != CR || selection[i] != LF )
                pData[j++] = selection[i] ;
        pData[j] = '\0';

        if ( vmode == VTERM )
            sendcharsduplex(pData,j,FALSE);
        else
            putkeystr( vmode, pData );

        free(pData);
    }
    return rc ;
}

APIRET
CopyClipboardToKbdBuffer( BYTE vmode )
{
    int i = 0 ;
    APIRET rc = -1 ;
    BYTE * hClipbrdData ;
    USHORT * pUClipbrdData = 0;
    BYTE * pClipbrdData = 0 ;
    BYTE * pClipboard ;
    int use_unicode = (isunicode() && !isWin95());

#ifdef NT
    HGLOBAL hClipboard = NULL ;

    if ( OpenClipboard( NULL ) )
    {
        if ( use_unicode ) {
            if ( hClipboard = (BYTE *) GetClipboardData( CF_UNICODETEXT ) )
            {
                hClipbrdData = GlobalLock(hClipboard) ;
                pUClipbrdData = wcsdup( (const wchar_t *) hClipbrdData ) ;
                GlobalUnlock( hClipboard ) ;
            }
        } else {
            if ( hClipboard = (BYTE *) GetClipboardData( CF_TEXT ) )
            {
                hClipbrdData = GlobalLock(hClipboard) ;
                pClipbrdData = strdup( hClipbrdData ) ;
                GlobalUnlock( hClipboard ) ;
            }
        }
        CloseClipboard() ;
    }
    if ( pClipbrdData && !use_unicode )
        CharToOem( pClipbrdData, pClipbrdData );
#else /* NT */
    pClipbrdData = GetTextFromClipboardServer() ;

    if ( !pClipbrdData ) {
        if ( rc = WinOpenClipbrd(hab) ) {
            hClipbrdData = (BYTE *) WinQueryClipbrdData( hab, CF_TEXT ) ;
            if ( !DosGetSharedMem( hClipbrdData, PAG_READ ) )
            {
                pClipbrdData = strdup( hClipbrdData ) ;

                /* We must copy the text back to the Clipboard because the   */
                /* GetSharedMemory call screwed up the clipboard.  We're not */
                /* supposed to do things like that.                          */
                if ( !DosAllocSharedMem( (PPVOID) &pClipboard, 0, strlen(pClipbrdData)+1,
                                         PAG_COMMIT | PAG_READ | PAG_WRITE |
                                         OBJ_GIVEABLE | OBJ_GETTABLE ) )
                {
                    strcpy( pClipboard, pClipbrdData ) ;
                    WinSetClipbrdData( hab, (ULONG) pClipboard, CF_TEXT, CFI_POINTER ) ;
                    DosFreeMem( pClipboard ) ;
                }
            }
            WinCloseClipbrd( hab ) ;
        }
    }
#endif /* NT */

    if ( use_unicode ) {
        if ( pUClipbrdData ) {
            int i=0, j=0, len = 0;

            len = wcslen(pUClipbrdData);
            debug(F111,"Clipboard","pUClipbrdData length",len);

            if ( vmode == VTERM ) {
                unsigned char * bytes, * buf;
                int nbytes, bytecount=0;

                nbytes = utorxlat(pUClipbrdData[0], &bytes);
                if ( nbytes > 0 )
                    bytecount = nbytes;
                for ( i=1; i<len; i++ ) {
                    if ( pUClipbrdData[i-1] != CR || pUClipbrdData[i] != LF ) {
                        nbytes = utorxlat(pUClipbrdData[i], &bytes);
                        if ( nbytes > 0 )
                            bytecount += nbytes;
                    }
                }

                buf = (unsigned char *) malloc(bytecount+1);

                nbytes = utorxlat(pUClipbrdData[0], &bytes);
                while ( nbytes-- > 0 )
                    buf[j++] = *bytes++;
                for ( i=1; i<len; i++ ) {
                    if ( pUClipbrdData[i-1] != CR || pUClipbrdData[i] != LF ) {
                        nbytes = utorxlat(pUClipbrdData[i], &bytes);
                        while ( nbytes-- > 0 )
                            buf[j++] = *bytes++;
                    }
                }
                sendcharsduplex(buf,bytecount,TRUE);
                free(buf);
                rc = 0;
            } else {
                char * pData = NULL;
                pData = (char *)malloc(len+2);
                debug(F111,"Clipboard","pData",pData);
                if ( pData != NULL ) {
                    pData[0] = utolxlat(pUClipbrdData[0]);
                    for ( i=1, j=1 ; i<len ; i++ )
                        if ( pUClipbrdData[i-1] != CR || pUClipbrdData[i] != LF )
                            pData[j++] = utolxlat(pUClipbrdData[i]);
                    pData[j] = '\0';
                    debug(F111,"Clipboard","pData length",j);
                    putkeystr( vmode, pData );
                    free(pData);
                    rc = 0 ;
                }
                else {
                    rc = -1;
                }
            }
            free( pUClipbrdData ) ;
        }
    } else {
        if ( pClipbrdData ) {
            char * pData = NULL;
            int i=0, j=0, len = 0;

            len = strlen(pClipbrdData);
            debug(F111,"Clipboard","pClipbrdData length",len);
            pData = (char *)malloc(len+2);
            debug(F111,"Clipboard","pData",pData);
            if ( pData != NULL ) {
                pData[0] = pClipbrdData[0];
                for ( i=1, j=1 ; i<len ; i++ )
                    if ( pClipbrdData[i-1] != CR || pClipbrdData[i] != LF )
                        pData[j++] = pClipbrdData[i] ;
                pData[j] = '\0';
                debug(F111,"Clipboard","pData length",j);

                if ( vmode == VTERM )
                    sendcharsduplex(pData,j,FALSE);
                else
                    putkeystr( vmode, pData );
                free(pData);
                rc = 0 ;
            }
            else {
                rc = -1;
            }
            free( pClipbrdData ) ;
        }
    }

    if ( rc )
        bleep( BP_FAIL ) ;
    return rc ;
}

APIRET
CopyVscrnToClipboard( BYTE vmode, int select_mode )
{
    ULONG  ClipBoardSz = 0 ;
    BYTE * pClipboard = 0 ;
#ifdef NT
    HGLOBAL hClipboard = NULL ;
#endif /* NT */
    APIRET rc = 0 ;
    int use_unicode = (isunicode() && !isWin95());

    if ( VscrnSelect(vmode, select_mode) )
        return -1 ;

    /* Determine size of Clipboard */
    ClipBoardSz = use_unicode ? 2 * nselect : nselect ;

    /* Allocate Clipboard Buffer */
#ifdef NT
    if ( (hClipboard = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, ClipBoardSz )) == NULL )
#else /* NT */
    if ( rc = DosAllocSharedMem( (PPVOID) &pClipboard, 0, ClipBoardSz,
                   PAG_COMMIT | PAG_READ | PAG_WRITE |
                   OBJ_GIVEABLE | OBJ_GETTABLE ) )
#endif /* NT */
    {
        bleep( BP_FAIL ) ;
        return -1 ;
    }
#ifdef NT
    else
    {
        if ( ( pClipboard = GlobalLock( hClipboard )) == NULL )
        {
            GlobalFree( hClipboard ) ;
            bleep( BP_FAIL ) ;
            return -1 ;
        }
        memset( pClipboard, 0, GlobalSize( hClipboard ) ) ;
    }
#endif /* NT */

    /* Copy the data to the clibboard buffer */
    if ( use_unicode )
        memcpy( pClipboard, Uselection, ClipBoardSz );
    else
        strcpy( pClipboard, selection ) ;

#ifdef NT
    if ( !use_unicode )
        OemToChar( pClipboard, pClipboard );
    GlobalUnlock( hClipboard ) ;
    if ( OpenClipboard(NULL) )
    {
       EmptyClipboard() ;
       if ( !SetClipboardData( use_unicode ? CF_UNICODETEXT : CF_TEXT, hClipboard ) )
       {
          GlobalFree( hClipboard ) ;
          debug(F101,"PutClipboardData failed","",GetLastError() ) ;
       }
       CloseClipboard() ;
    }
#else /* NT */
    if ( PutTextToClipboardServer( pClipboard ) ) {
        DosFreeMem( pClipboard ) ;
        }
    else if ( WinOpenClipbrd( hab ) ) {
       WinEmptyClipbrd( hab ) ;
       if ( !WinSetClipbrdData( hab, (ULONG) pClipboard, CF_TEXT, CFI_POINTER ) )
           DosFreeMem( pClipboard ) ;
       WinCloseClipbrd( hab ) ;
       }
   else {
      DosFreeMem( pClipboard ) ;
      rc = -1 ;
      }
#endif /* NT */
    return rc ;
}


APIRET
CopyVscrnToPrinter(BYTE vmode, int select_mode)
{
    bool printstate = printon ;
    APIRET rc = 0 ;

    if ( VscrnSelect(vmode,select_mode) )
        return -1 ;

    /* Copy the data to the printer buffer */

    if (!printstate)
        printeron() ;

    if (!printon)
	bleep( BP_FAIL ) ;
    else {
	prtstr(selection,nselect-1);        /* nselect includes NUL */
	if (xprintff)
	    prtchar(FF);
    }
    if (!printstate)
        printeroff();
    return rc ;
}


void
markstart( BYTE vmode )
{
    if ( markmodeflag[vmode] == marking ) {
        VscrnUnmarkAll(vmode) ;
        }
    markmodeflag[vmode] = marking ;
    VscrnMark( vmode,(VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y)%vscrn[vmode].linecount,
        vscrn[vmode].cursor.x, vscrn[vmode].cursor.x ) ;
}

void
markcancel( BYTE vmode )
{
    markmodeflag[vmode] = notmarking ;
    VscrnUnmarkAll(vmode) ;
}

void
markdownone( BYTE vmode )
{
   LONG curline = (VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y
      +vscrn[vmode].linecount)%vscrn[vmode].linecount ;

    if ( markmodeflag[vmode] == marking &&
        curline != VscrnGetEnd(vmode)) {
        if ( vscrn[vmode].marktop == curline &&
            vscrn[vmode].lines[vscrn[vmode].marktop].markbeg == vscrn[vmode].cursor.x ) {
            if ( vscrn[vmode].marktop == vscrn[vmode].markbot ) {
                if ( vscrn[vmode].lines[vscrn[vmode].marktop].markbeg !=
                     vscrn[vmode].lines[vscrn[vmode].marktop].markend )
                   VscrnUnmark( vmode, vscrn[vmode].marktop,
                                 vscrn[vmode].lines[vscrn[vmode].marktop].markbeg,
                                 vscrn[vmode].lines[vscrn[vmode].marktop].markend-1 ) ;
               VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].lines[vscrn[vmode].markbot].markend,
                           vscrn[vmode].lines[vscrn[vmode].markbot].width ) ;
               VscrnMark( vmode, (vscrn[vmode].markbot+1)%vscrn[vmode].linecount, 0,
                           vscrn[vmode].cursor.x ) ;
            }
            else {
                VscrnUnmark( vmode, vscrn[vmode].marktop, vscrn[vmode].cursor.x, MAXTERMCOL ) ;
                if ( vscrn[vmode].marktop == vscrn[vmode].markbot &&
                    vscrn[vmode].cursor.x >= vscrn[vmode].lines[vscrn[vmode].markbot].markend ) {
                    VscrnUnmark( vmode, vscrn[vmode].marktop, 0,
                        vscrn[vmode].lines[vscrn[vmode].markbot].markend-1 ) ;
                    VscrnMark( vmode, vscrn[vmode].markbot,
                        vscrn[vmode].lines[vscrn[vmode].markbot].markend, vscrn[vmode].cursor.x ) ;
                    }
                else {
                    if ( vscrn[vmode].cursor.x > 0 )
                        VscrnUnmark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].cursor.x-1 );
                    }
                }
            }
        else {
            VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x,
                vscrn[vmode].lines[vscrn[vmode].markbot].width ) ;
            if ( vscrn[vmode].markbot != vscrn[vmode].end )
                VscrnMark( vmode, (vscrn[vmode].markbot+1)%vscrn[vmode].linecount, 0,
                    vscrn[vmode].cursor.x ) ;
            }
        }

    if ( markmodeflag[vmode] == inmarkmode ||
        markmodeflag[vmode] == marking && curline != VscrnGetEnd(vmode) ) {
        if ( vscrn[vmode].cursor.y == VscrnGetHeight(vmode)-(tt_status[vmode]?2:1) ) {
            if ( VscrnMoveScrollTop(vmode,1) < 0 )
                bleep(BP_FAIL);
            }
        else
            vscrn[vmode].cursor.y++ ;
        }
    else {
        bleep(BP_FAIL);
        }

}

void
markupone( BYTE vmode )
{
   LONG curline = (VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y
      +vscrn[vmode].linecount)%vscrn[vmode].linecount ;

    if ( markmodeflag[vmode] == marking &&
        curline != VscrnGetBegin(vmode) ) {
        if ( vscrn[vmode].markbot == curline &&
            vscrn[vmode].lines[vscrn[vmode].markbot].markend == vscrn[vmode].cursor.x ) {
            if ( vscrn[vmode].marktop == vscrn[vmode].markbot ) {
                if ( vscrn[vmode].lines[vscrn[vmode].markbot].markbeg !=
                    vscrn[vmode].lines[vscrn[vmode].markbot].markend )
                    VscrnUnmark( vmode, vscrn[vmode].markbot,
                        vscrn[vmode].lines[vscrn[vmode].markbot].markbeg+1,
                        vscrn[vmode].lines[vscrn[vmode].markbot].markend ) ;
                VscrnMark( vmode, vscrn[vmode].marktop, 0,
                    vscrn[vmode].lines[vscrn[vmode].marktop].markbeg ) ;
                VscrnMark( vmode, (vscrn[vmode].marktop-1+vscrn[vmode].linecount)%vscrn[vmode].linecount,
                    vscrn[vmode].cursor.x,
                    vscrn[vmode].lines[(vscrn[vmode].marktop-1+vscrn[vmode].linecount)
                    %vscrn[vmode].linecount].width ) ;
                }
            else {
                VscrnUnmark( vmode, vscrn[vmode].markbot, 0, vscrn[vmode].cursor.x ) ;
                if ( vscrn[vmode].marktop == vscrn[vmode].markbot &&
                    vscrn[vmode].cursor.x <= vscrn[vmode].lines[vscrn[vmode].marktop].markbeg ) {
                    VscrnUnmark( vmode, vscrn[vmode].marktop,
                        vscrn[vmode].lines[vscrn[vmode].marktop].markbeg+1, MAXTERMCOL ) ;
                    VscrnMark( vmode, vscrn[vmode].marktop, vscrn[vmode].cursor.x,
                        vscrn[vmode].lines[vscrn[vmode].marktop].markbeg ) ;
                    }
                else {
                    VscrnUnmark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x+1, MAXTERMCOL );
                    }
                }
            }
        else {
            VscrnMark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].cursor.x ) ;
            if ( vscrn[vmode].marktop != vscrn[vmode].beg )
                VscrnMark( vmode, (vscrn[vmode].marktop-1+vscrn[vmode].linecount)%vscrn[vmode].linecount,
                    vscrn[vmode].cursor.x,
                    vscrn[vmode].lines[(vscrn[vmode].marktop-1+vscrn[vmode].linecount)
                    %vscrn[vmode].linecount].width ) ;
            }

        }

    if ( markmodeflag[vmode] == inmarkmode ||
        markmodeflag[vmode] == marking &&
        curline != VscrnGetBegin(vmode) ) {
        if ( vscrn[vmode].cursor.y == 0 ) {
            if ( VscrnMoveScrollTop(vmode,-1) < 0 )
                bleep(BP_FAIL);
            }
        else
            vscrn[vmode].cursor.y-- ;
        }
    else {
        bleep(BP_FAIL);
        }
}

void
markleftone( BYTE vmode )
{
   LONG curline = (VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y
      +vscrn[vmode].linecount)%vscrn[vmode].linecount ;

    if ( markmodeflag[vmode] == marking && vscrn[vmode].cursor.x > 0 ) {
        if ( vscrn[vmode].marktop == curline &&
            vscrn[vmode].lines[vscrn[vmode].marktop].markbeg == vscrn[vmode].cursor.x ) {
            VscrnMark( vmode, vscrn[vmode].marktop, vscrn[vmode].cursor.x-1, vscrn[vmode].cursor.x-1 ) ;
        }
        else {
            VscrnUnmark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x, vscrn[vmode].cursor.x ) ;
        }
    }

    if ( vscrn[vmode].cursor.x == 0 )
        bleep(BP_FAIL);
    else
        vscrn[vmode].cursor.x-- ;
}

void
markrightone( BYTE vmode )
{
   LONG curline = (VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y
      +vscrn[vmode].linecount)%vscrn[vmode].linecount ;

    if ( markmodeflag[vmode] == marking && vscrn[vmode].cursor.x < VscrnGetWidth(VTERM)-1 ) {
        if ( vscrn[vmode].markbot == curline &&
            vscrn[vmode].lines[vscrn[vmode].markbot].markend == vscrn[vmode].cursor.x ) {
            VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x+1, vscrn[vmode].cursor.x+1 ) ;
        }
        else {
            VscrnUnmark( vmode, vscrn[vmode].marktop, vscrn[vmode].cursor.x, vscrn[vmode].cursor.x ) ;
        }
    }

    if ( vscrn[vmode].cursor.x == VscrnGetWidth(VTERM)-1 )
        bleep(BP_FAIL);
    else
        vscrn[vmode].cursor.x++ ;
}

void
markdownscreen( BYTE vmode )
{
   int count = (tt_status[vmode]?1:0) ;
   LONG curline = (VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y
      +vscrn[vmode].linecount)%vscrn[vmode].linecount ;

    if ( markmodeflag[vmode] == marking &&
        curline != VscrnGetEnd(vmode) ) {
        if ( vscrn[vmode].marktop == curline &&
             vscrn[vmode].marktop != vscrn[vmode].markbot &&
            vscrn[vmode].lines[vscrn[vmode].marktop].markbeg == vscrn[vmode].cursor.x ) {
            while ( count < VscrnGetHeight(vmode) &&
                    ( vscrn[vmode].marktop < ( vscrn[vmode].markbot - 1 + vscrn[vmode].linecount)%vscrn[vmode].linecount ) )
            {
               count++ ;
               VscrnUnmark( vmode, vscrn[vmode].marktop, 0, MAXTERMCOL ) ;
               if ( vscrn[vmode].cursor.x > 0 ) {
                  VscrnUnmark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].cursor.x-1 );
               }
            }
        }

        if ( count < VscrnGetHeight(vmode) && vscrn[vmode].marktop ==
            (vscrn[vmode].markbot-1+vscrn[vmode].linecount)%vscrn[vmode].linecount ) {
            count++ ;
            VscrnUnmark( vmode, vscrn[vmode].marktop, 0, MAXTERMCOL ) ;
            if ( vscrn[vmode].cursor.x <= vscrn[vmode].lines[vscrn[vmode].marktop].markend ) {
                VscrnUnmark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].cursor.x-1 ) ;
                }
            else { /* vscrn[vmode].cursor.x > vscrn[vmode].lines[vscrn[vmode].marktop].markend */
                VscrnUnmark( vmode, vscrn[vmode].marktop, 0,
                    vscrn[vmode].lines[vscrn[vmode].marktop].markend-1 ) ;
                VscrnMark( vmode, vscrn[vmode].marktop, vscrn[vmode].lines[vscrn[vmode].marktop].markend,
                    vscrn[vmode].cursor.x ) ;
                }
            }

        if ( count < VscrnGetHeight(vmode) &&
            vscrn[vmode].marktop == vscrn[vmode].markbot ) {
            count++ ;
            if ( vscrn[vmode].cursor.x == vscrn[vmode].lines[vscrn[vmode].markbot].markbeg ) {
                VscrnUnmark( vmode, vscrn[vmode].markbot, 0,
                    vscrn[vmode].lines[vscrn[vmode].markbot].markend-1 ) ;
                }
            VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].lines[vscrn[vmode].markbot].markend,
                MAXTERMCOL ) ;
            if ( vscrn[vmode].markbot != vscrn[vmode].end )
                VscrnMark( vmode, (vscrn[vmode].markbot+1)%vscrn[vmode].linecount, 0,
                    vscrn[vmode].cursor.x ) ;
            }

        while ( count < VscrnGetHeight(vmode) &&
                vscrn[vmode].markbot != vscrn[vmode].end ) {
            count++ ;
            VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x,
                vscrn[vmode].lines[vscrn[vmode].markbot].width ) ;
            if ( vscrn[vmode].markbot != vscrn[vmode].end )
                VscrnMark( vmode, (vscrn[vmode].markbot+1)%vscrn[vmode].linecount, 0,
                vscrn[vmode].cursor.x ) ;
            }
        }

    if ( VscrnMoveScrollTop(vmode, VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)) < 0 ) {
        LONG oldscrolltop = VscrnGetScrollTop(vmode);
        if ( VscrnSetScrollTop(vmode, VscrnGetTop(vmode)) < 0 )
            bleep(BP_WARN);
        else if ( oldscrolltop != VscrnGetScrollTop(vmode) )
            vscrn[vmode].cursor.y +=
            (VscrnGetHeight(vmode) - (tt_status[vmode]?1:0)
              - (vscrn[vmode].scrolltop - oldscrolltop)
              + vscrn[vmode].linecount) % vscrn[vmode].linecount ;
        else vscrn[vmode].cursor.y = VscrnGetHeight(vmode)
            -(tt_status[vmode]?1:0)-1 /* zero based */ ;
        }
}

void
markupscreen( BYTE vmode )
{
    int count = 0 ;
   LONG curline = (VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y
      +vscrn[vmode].linecount)%vscrn[vmode].linecount ;

    if ( markmodeflag[vmode] == marking &&
        curline != VscrnGetBegin(vmode) ) {
        if ( vscrn[vmode].markbot == curline &&
             vscrn[vmode].marktop != vscrn[vmode].markbot &&
            vscrn[vmode].lines[vscrn[vmode].markbot].markend == vscrn[vmode].cursor.x ) {
            while ( count < VscrnGetHeight(vmode)-(tt_status[vmode]?1:0) && ( vscrn[vmode].markbot >
                ( vscrn[vmode].marktop + 1 + vscrn[vmode].linecount)%vscrn[vmode].linecount ) ) {
                count++ ;
                VscrnUnmark( vmode, vscrn[vmode].markbot, 0, MAXTERMCOL ) ;
                if ( vscrn[vmode].cursor.x > 0 ) {
                    VscrnUnmark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x+1,
                        MAXTERMCOL );
                    }
                }
            }

        if ( count < VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)
             && vscrn[vmode].markbot ==
            (vscrn[vmode].marktop+1+vscrn[vmode].linecount)%vscrn[vmode].linecount ) {
            count++ ;
            VscrnUnmark( vmode, vscrn[vmode].markbot, 0, MAXTERMCOL ) ;
            if ( vscrn[vmode].cursor.x >= vscrn[vmode].lines[vscrn[vmode].markbot].markbeg ) {
                VscrnUnmark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x+1, MAXTERMCOL ) ;
                }
            else { /* vscrn[vmode].cursor.x < vscrn[vmode].lines[vscrn[vmode].markbot].markbeg */
                VscrnUnmark( vmode, vscrn[vmode].markbot,
                    vscrn[vmode].lines[vscrn[vmode].markbot].markbeg+1, MAXTERMCOL ) ;
                VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x,
                    vscrn[vmode].lines[vscrn[vmode].markbot].markbeg ) ;
                }
            }

        if ( count < VscrnGetHeight(vmode)-(tt_status[vmode]?1:0) &&
            vscrn[vmode].marktop == vscrn[vmode].markbot ) {
            count++ ;
            if ( vscrn[vmode].cursor.x == vscrn[vmode].lines[vscrn[vmode].marktop].markend ) {
                VscrnUnmark( vmode, vscrn[vmode].marktop,
                    vscrn[vmode].lines[vscrn[vmode].marktop].markbeg+1, MAXTERMCOL ) ;
                }
            VscrnMark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].lines[vscrn[vmode].marktop].markbeg );
            if ( vscrn[vmode].marktop != vscrn[vmode].beg )
                VscrnMark( vmode, (vscrn[vmode].marktop-1+vscrn[vmode].linecount)%vscrn[vmode].linecount, vscrn[vmode].cursor.x,
                MAXTERMCOL ) ;
            }

        while ( count < VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)
                && vscrn[vmode].marktop != vscrn[vmode].beg ) {
            count++ ;
            VscrnMark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].cursor.x );
            if ( vscrn[vmode].marktop != vscrn[vmode].beg )
                VscrnMark( vmode, (vscrn[vmode].marktop-1+vscrn[vmode].linecount)%vscrn[vmode].linecount, vscrn[vmode].cursor.x,
                    MAXTERMCOL) ;
            }
        }

    if ( VscrnMoveScrollTop(vmode,-(VscrnGetHeight(vmode)-(tt_status[vmode]?1:0))) < 0 ) {
        LONG oldscrolltop = VscrnGetScrollTop(vmode);
        if ( VscrnSetScrollTop(vmode,VscrnGetBegin(vmode)) < 0 )
            bleep(BP_WARN);
        else if ( oldscrolltop != VscrnGetScrollTop(vmode) )
            vscrn[vmode].cursor.y -=
            (oldscrolltop - vscrn[vmode].scrolltop + vscrn[vmode].linecount) % vscrn[vmode].linecount ;
        else vscrn[vmode].cursor.y = 0 ;
        }
}

void
markhomescreen(BYTE vmode )
{
   LONG curline = (VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y
      +vscrn[vmode].linecount)%vscrn[vmode].linecount ;

    if ( markmodeflag[vmode] == marking &&
        ( curline != VscrnGetBegin(vmode) ||
        curline == VscrnGetBegin(vmode) &&
        vscrn[vmode].cursor.x != 0 ) ) {
        if ( vscrn[vmode].markbot == curline &&
            vscrn[vmode].lines[vscrn[vmode].markbot].markend == vscrn[vmode].cursor.x ) {
            while ( vscrn[vmode].markbot >
                ( vscrn[vmode].marktop + 1 + vscrn[vmode].linecount)%vscrn[vmode].linecount ) {
                VscrnUnmark( vmode, vscrn[vmode].markbot, 0, MAXTERMCOL ) ;
                if ( vscrn[vmode].cursor.x > 0 ) {
                    VscrnUnmark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x+1,
                        MAXTERMCOL );
                    }
                }
            }

        if ( vscrn[vmode].markbot ==
            (vscrn[vmode].marktop+1+vscrn[vmode].linecount)%vscrn[vmode].linecount ) {
            VscrnUnmark( vmode, vscrn[vmode].markbot, 0, MAXTERMCOL ) ;
            if ( vscrn[vmode].cursor.x >= vscrn[vmode].lines[vscrn[vmode].markbot].markbeg ) {
                VscrnUnmark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x+1, MAXTERMCOL ) ;
                }
            else { /* vscrn[vmode].cursor.x < vscrn[vmode].lines[vscrn[vmode].markbot].markbeg */
                VscrnUnmark( vmode, vscrn[vmode].markbot,
                    vscrn[vmode].lines[vscrn[vmode].markbot].markbeg+1, MAXTERMCOL ) ;
                VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x,
                    vscrn[vmode].lines[vscrn[vmode].markbot].markbeg ) ;
                }
            }

        if ( vscrn[vmode].marktop == vscrn[vmode].markbot ) {
            if ( vscrn[vmode].cursor.x == vscrn[vmode].lines[vscrn[vmode].marktop].markend ) {
                VscrnUnmark( vmode, vscrn[vmode].marktop,
                    vscrn[vmode].lines[vscrn[vmode].marktop].markbeg+1, MAXTERMCOL ) ;
                }
            VscrnMark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].lines[vscrn[vmode].marktop].markbeg );
            if ( vscrn[vmode].marktop != vscrn[vmode].beg )
                VscrnMark( vmode, (vscrn[vmode].marktop-1+vscrn[vmode].linecount)%vscrn[vmode].linecount, vscrn[vmode].cursor.x,
                MAXTERMCOL ) ;
            }

        while ( vscrn[vmode].marktop != vscrn[vmode].beg ) {
            VscrnMark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].cursor.x );
            if ( vscrn[vmode].marktop != vscrn[vmode].beg )
                VscrnMark( vmode, (vscrn[vmode].marktop-1+vscrn[vmode].linecount)%vscrn[vmode].linecount, vscrn[vmode].cursor.x,
                    MAXTERMCOL) ;
            }

        VscrnMark( vmode, vscrn[vmode].marktop, 0, MAXTERMCOL ) ;
        }


    if ( VscrnSetScrollTop( vmode, VscrnGetBegin(vmode) ) < 0 )
        bleep( BP_FAIL ) ;
    else {
        vscrn[vmode].cursor.x = 0 ;
        vscrn[vmode].cursor.y = 0 ;
        }
}

void
markendscreen( BYTE vmode )
{
   LONG curline = (VscrnGetScrollTop(vmode)+vscrn[vmode].cursor.y
      +vscrn[vmode].linecount)%vscrn[vmode].linecount ;

    if ( markmodeflag[vmode] == marking &&
        ( curline != VscrnGetEnd(vmode) ||
        curline == VscrnGetEnd(vmode) &&
        vscrn[vmode].cursor.x != VscrnGetWidth(vmode)-1 ) ) {
        if ( vscrn[vmode].marktop == curline &&
            vscrn[vmode].lines[vscrn[vmode].marktop].markbeg == vscrn[vmode].cursor.x ) {
            while ( vscrn[vmode].marktop <
                ( vscrn[vmode].markbot - 1 + vscrn[vmode].linecount)%vscrn[vmode].linecount &&
               vscrn[vmode].markbot != vscrn[vmode].marktop ) {
                VscrnUnmark( vmode, vscrn[vmode].marktop, 0, MAXTERMCOL ) ;
                if ( vscrn[vmode].cursor.x > 0 ) {
                    VscrnUnmark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].cursor.x-1 );
                    }
                }
            }

        if ( vscrn[vmode].marktop ==
            (vscrn[vmode].markbot-1+vscrn[vmode].linecount)%vscrn[vmode].linecount ) {
            VscrnUnmark( vmode, vscrn[vmode].marktop, 0, MAXTERMCOL ) ;
            if ( vscrn[vmode].cursor.x <= vscrn[vmode].lines[vscrn[vmode].marktop].markend ) {
                VscrnUnmark( vmode, vscrn[vmode].marktop, 0, vscrn[vmode].cursor.x-1 ) ;
                }
            else { /* vscrn[vmode].cursor.x > vscrn[vmode].lines[vscrn[vmode].marktop].markend */
                VscrnUnmark( vmode, vscrn[vmode].marktop, 0,
                    vscrn[vmode].lines[vscrn[vmode].marktop].markend-1 ) ;
                VscrnMark( vmode, vscrn[vmode].marktop, vscrn[vmode].lines[vscrn[vmode].marktop].markend,
                    vscrn[vmode].cursor.x ) ;
                }
            }

        if ( vscrn[vmode].marktop == vscrn[vmode].markbot ) {
            if ( vscrn[vmode].cursor.x == vscrn[vmode].lines[vscrn[vmode].markbot].markbeg ) {
                VscrnUnmark( vmode, vscrn[vmode].markbot, 0,
                    vscrn[vmode].lines[vscrn[vmode].markbot].markend-1 ) ;
                }
            VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].lines[vscrn[vmode].markbot].markend,
                MAXTERMCOL ) ;
            if ( vscrn[vmode].markbot != vscrn[vmode].end )
                VscrnMark( vmode, (vscrn[vmode].markbot+1)%vscrn[vmode].linecount, 0,
                    vscrn[vmode].cursor.x ) ;
            }

        while ( vscrn[vmode].markbot != vscrn[vmode].end-(tt_status[vmode]?1:0) ) {
            VscrnMark( vmode, vscrn[vmode].markbot, vscrn[vmode].cursor.x,
                vscrn[vmode].lines[vscrn[vmode].markbot].width ) ;
            if ( vscrn[vmode].markbot != vscrn[vmode].end )
                VscrnMark( vmode, (vscrn[vmode].markbot+1)%vscrn[vmode].linecount, 0,
                    vscrn[vmode].cursor.x ) ;
            }
        VscrnMark( vmode, vscrn[vmode].markbot, 0, MAXTERMCOL ) ;
        }

    if ( VscrnSetScrollTop( vmode, VscrnGetTop(vmode) ) < 0 )
        bleep( BP_FAIL ) ;
    else {
        vscrn[vmode].cursor.x = VscrnGetWidth(vmode)-1 ;
        vscrn[vmode].cursor.y = VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)-1 ;
        }

}

void
markleftpage(BYTE vmode)
{
    if ( markmodeflag[vmode] == marking ) {

        }

}

void
markrightpage(BYTE vmode)
{
    if ( markmodeflag[vmode] == marking ) {

        }

}

void
markcopyclip( BYTE vmode, int select )
{
    if ( markmodeflag[vmode] == marking ) {
        CopyVscrnToClipboard(vmode,select);
        markmodeflag[vmode] = notmarking ;
        VscrnUnmarkAll(vmode) ;
        }
    else
        bleep( BP_FAIL ) ;
}

void
markselect( BYTE vmode, int select )
{
    if ( markmodeflag[vmode] == marking ) {
        VscrnSelect(vmode,select);
        markmodeflag[vmode] = notmarking ;
        VscrnUnmarkAll(vmode) ;
        }
    else
        bleep( BP_FAIL ) ;
}

void
markcopyhost( BYTE vmode, int select )
{
    if ( markmodeflag[vmode] == marking ) {
        CopyVscrnToKbdBuffer(vmode,select);
        markmodeflag[vmode] = notmarking ;
        VscrnUnmarkAll(vmode) ;
        }
    else
        bleep( BP_FAIL ) ;
}

void
markprint( BYTE vmode, int select )
{
    if ( markmodeflag[vmode] == marking ) {
        CopyVscrnToPrinter(vmode,select);
        markmodeflag[vmode] = notmarking ;
        VscrnUnmarkAll(vmode) ;
        }
    else
        bleep( BP_FAIL ) ;
}

#define NUM_OF_ACTIVE_LINKS 256
static unsigned short next_hyperlink_index = 1;
static hyperlink hyperlinks[NUM_OF_ACTIVE_LINKS];

int
hyperlink_add(int type, char * link)
{
    unsigned short idx = next_hyperlink_index % NUM_OF_ACTIVE_LINKS;

    hyperlinks[idx].index = next_hyperlink_index;
    next_hyperlink_index++;
    if ( hyperlinks[idx].str )
        free(hyperlinks[idx].str);
    hyperlinks[idx].str = link;
    hyperlinks[idx].type = type;
    return(hyperlinks[idx].index);
}

hyperlink *
hyperlink_get(int index)
{
    unsigned short idx = index % NUM_OF_ACTIVE_LINKS;
    
    if ( index == hyperlinks[idx].index )
        return &hyperlinks[idx];
    return NULL;
}
#endif /* NOLOCAL */
