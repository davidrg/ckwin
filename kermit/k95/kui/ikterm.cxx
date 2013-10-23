#include "ikterm.h"
extern "C" {
#include "ikui.h"
extern enum markmodes markmodeflag[] ;
extern videobuffer vscrn[VNUM]; /* = {0,0,0,0,0,0,{0,0},0,-1,-1}; */
extern int inecho;          /* do we echo script INPUT output? */
extern int updmode ;
extern int priority ;
extern unsigned char defaultattribute ;
extern int cursoron[], cursorena[],scrollflag[], scrollstatus[], flipscrnflag[];
extern int tt_update, tt_updmode, tt_rows[], tt_cols[], tt_font, tt_roll[],
           tt_cursor, scrninitialized[], ttyfd, viewonly, carrier, network,
           tt_scrsize[], tt_modechg, pheight, pwidth, tt_status[], screenon, 
           decssdt, tt_url_hilite, tt_url_hilite_attr, tt_type_mode, ttnum;
extern TID tidTermScrnUpd ;
extern unsigned char     colorstatus;
extern unsigned char     colorselect;
extern unsigned char     colorborder;
extern int vmode;
extern int tcsl;
extern int nt351;

extern void
win32KeyEvent( int mode, KEY_EVENT_RECORD key );
extern void 
win32MouseEvent( int mode, MOUSE_EVENT_RECORD r );

}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void getDimensions( BYTE vnum, int* w, int* h )
{
    vnum = vmode;

	// We use the actual struct members instead of the access methods
	// because the access methods take into account the decsasd state.
	// which could result in the wrong value be returned.
    *w = VscrnGetWidth(vnum);
    *h = VscrnGetDisplayHeight(vnum);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
long getMaxDim()
{
    return MAXTERMSIZE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void getMaxSizes( int* column, int* row )
{
    *column = MAXTERMCOL;
    *row    = MAXTERMROW;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IKTerm::IKTerm( BYTE whichbuffer, K_CLIENT_PAINT* clipaint )
    : vnum( whichbuffer )
    , kcp( clipaint ), mouseCaptured(0)
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IKTerm::~IKTerm()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
#define URLMINCNT 4096
#define EXCLUSIVE 1

BOOL IKTerm::getDrawInfo()
{
	vnum = vmode;
#ifdef EXCLUSIVE
    /* Wait for exclusive access to the screen */
    if ( RequestVscrnMutex( vnum, 200 ) ) {
        debug(F101,"IKTerm::getDrawInfo unable to acquire Mutex","",vnum);
        return FALSE;
    }
    debug(F101,"IKTerm::getDrawInfo has Mutex","",vnum);
#endif /* EXCLUSIVE */

    xs = VscrnGetWidth( vnum );
    ys = VscrnGetHeight( vnum );

    c = 0;
    cursor_offset = 0;

    incnt = 0;

    vbuf = &(vscrn[vnum]);
    if ( vbuf->lines == NULL ) {
        ReleaseVscrnMutex(vnum) ;
        return FALSE;
    }

    xs = (tt_modechg == TVC_ENA ) ? vbuf->width : pwidth;
    xho = vbuf->hscroll > (MAXTERMCOL-pwidth) ? 
        (MAXTERMCOL-VscrnGetWidth(vnum)) : vbuf->hscroll ;
    ys = (tt_modechg == TVC_ENA ? vbuf->height : pheight)
        - ((vnum==VTERM && tt_status[vnum] && decssdt!=SSDT_BLANK ||
             vnum != VTERM && tt_status[vnum]) ? 1 : 0 ) ;

    /* Do we have a popup to display? */
    if ( vbuf->popup == NULL) {
        xo = yo = -1 ;
    }
    else {
        xo = (xs - vbuf->popup->width) / 2 ;
        yo = (ys - vbuf->popup->height) / 2 ;
    }

    textBuffer = kcp->textBuffer;
    attrBuffer = kcp->attrBuffer;
    effectBuffer = kcp->effectBuffer;
    lineAttr = kcp->lineAttr;
    unsigned long maxWidth = 0;

    if ( vnum == VTERM && !screenon ) {
        /* Blank out the screen */
        for ( y=0 ; y < ys ; y++ )
            for ( x= 0; x < xs ; x++ ) {
                textBuffer[c++] =' ';
                attrBuffer[c++] = defaultattribute;
                effectBuffer[c++] = '\0';
            }
    }
    else if ( markmodeflag[vnum] != marking )
    {
        for ( y = 0 ; y < ys ; y++ )
        {
#ifdef NEW_EXCLUSIVE
            /* Wait for exclusive access to the screen */
            if ( RequestVscrnMutex( vnum, -1 ) )
                return FALSE;
#endif /* NEW_EXCLUSIVE */
            /* Get the next line */
            if (!scrollflag[vnum])
                line = &vbuf->lines[(vbuf->top+y)%vbuf->linecount] ;
            else
                line = &vbuf->lines[(vbuf->scrolltop+y)%vbuf->linecount] ;
            lineAttr[y] = line->vt_line_attr;
#ifdef NEW_EXCLUSIVE
            /* Give mutex back */
            ReleaseVscrnMutex(vnum) ;
#endif /* NEW_EXCLUSIVE */

            /* copy the screen data to the buffer */
            if (line->cells != NULL)
            {
                for ( x = 0 ; x < xs ; x++ )
                {
#ifdef NEW_EXCLUSIVE
                    /* Wait for exclusive access to the screen */
                    if ( RequestVscrnMutex( vnum, -1 ) )
                        return FALSE;
#endif /* NEW_EXCLUSIVE */
                    vt_char_attrs = line->vt_char_attrs[x+xho];
#ifdef NEW_EXCLUSIVE
                    /* Give mutex back */
                    ReleaseVscrnMutex(vnum) ;
#endif /* NEW_EXCLUSIVE */

                    /* We should be able to just say cell = line->cells[x] */
                    /* but this doesn't work.  So we go through hell to    */
                    /* compute the offsets properly.                       */
                    viocell cell = line->cells[x+xho] ;

                    if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                        vt_char_attrs |= tt_url_hilite_attr;
                    else if ( tt_url_hilite && incnt < URLMINCNT) {
                        if ( IsURLChar(cell.c) && 
                             IsCellPartOfURL(vnum, y, x+xho ) )
                            vt_char_attrs |= tt_url_hilite_attr;
                    }

                    textBuffer[c+x] = cell.c;
                    attrBuffer[c+x] = ComputeColorFromAttr( vnum,
                                cell.a,
                                vt_char_attrs);
                    effectBuffer[c+x] = vt_char_attrs;
                }
            }
            else
            {
                /* In case we are in the middle of a scroll */
                memset( &(textBuffer[c]), ' ', xs );
                memset( &(attrBuffer[c]), defaultattribute, xs );
                memset( &(effectBuffer[c]), '\0', xs );

#ifdef VSCRN_DEBUG
                debug(F101,"OUCH!","",(scrollflag?(vbuf->scrolltop+y)
                                    :(vbuf->top+y))%vbuf->linecount);
#endif /* VSCRN_DEBUG */
            }

            /* if there is a popup, display it */
            if ( y >= yo && 
                 vscrn[vnum].popup &&
                 y <= yo+vscrn[vnum].popup->height-1 ) {
                int i ;
                for ( i = 0 ; i < vscrn[vnum].popup->width ; i++ ) {
                    if ( isunicode() ) {
                        textBuffer[c+xo+i] = 
                            vscrn[vnum].popup->c[y-yo][i] < 32 ?
                                (*xl_u[TX_IBMC0GRPH])(vscrn[vnum].popup->c[y-yo][i]) : 
                                    vscrn[vnum].popup->c[y-yo][i] >= 128 ?
                                        (*xl_u[TX_CP437])(vscrn[vnum].popup->c[y-yo][i]) : 
                                            vscrn[vnum].popup->c[y-yo][i] ;
                    }
                    else
                        textBuffer[c+xo+i] = vscrn[vnum].popup->c[y-yo][i];
                    attrBuffer[c+xo+i] = vscrn[vnum].popup->a;
                    effectBuffer[c+xo+i] = '\0';
                }
            }
            c += xs;        /* advance the pointer in the buffer */
            if ( line->width > maxWidth )
                maxWidth = line->width;
        }
    }
    else
    {  /* were in marking mode */
        for ( y = 0 ; y < ys ; y++ )
        {
#ifdef NEW_EXCLUSIVE
            /* Wait for exclusive access to the screen */
            if ( RequestVscrnMutex( vnum, -1 ) )
                return FALSE;
#endif /* NEW_EXCLUSIVE */
            line = &vbuf->lines[(vbuf->scrolltop+y)%vbuf->linecount] ;
            lineAttr[y] = line->vt_line_attr;
#ifdef NEW_EXCLUSIVE
            /* Give mutex back */
            ReleaseVscrnMutex(vnum) ;
#endif /* NEW_EXCLUSIVE */

            if (line->cells)
            {
                if ( VscrnIsLineMarked(vnum,vbuf->scrolltop+y) )
                {
                    for ( x = 0 ; x < xs ; x++ )
                    {
                        if ( line->markshowend != -1 &&
                            x+xho >= line->markbeg &&
                            x+xho <= line->markshowend )
                        {
                            textBuffer[c+x] = line->cells[x+xho].c ;
                            attrBuffer[c+x] = colorselect ;
                            effectBuffer[c+x] = line->vt_char_attrs[x+xho];
                        }
                        else
                        {
#ifdef NEW_EXCLUSIVE
                            /* Wait for exclusive access to the screen */
                            if ( RequestVscrnMutex( vnum, -1 ) )
                                return FALSE;
#endif /* NEW_EXCLUSIVE */
                            vt_char_attrs = line->vt_char_attrs[x+xho];
#ifdef NEW_EXCLUSIVE
                            /* Give mutex back */
                            ReleaseVscrnMutex(vnum) ;
#endif /* NEW_EXCLUSIVE */

                            if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                                vt_char_attrs |= tt_url_hilite_attr;
                            else if ( tt_url_hilite && incnt < URLMINCNT) {
                                if ( IsURLChar(line->cells[x+xho].c) &&
                                     IsCellPartOfURL(vnum, y, x+xho ) )
                                    vt_char_attrs |= tt_url_hilite_attr;
                            }

                            textBuffer[c+x] = line->cells[x+xho].c;
                            attrBuffer[c+x] = ComputeColorFromAttr( vnum,
                                    line->cells[x+xho].a,
                                    vt_char_attrs );
                            effectBuffer[c+x] = vt_char_attrs;
                        }
                    }
                }
                else
                {
                    for ( x = 0 ; x < xs ; x++ )
                    {
#ifdef NEW_EXCLUSIVE
                        /* Wait for exclusive access to the screen */
                        if ( RequestVscrnMutex( vnum, -1 ) )
                            return FALSE;
#endif /* NEW_EXCLUSIVE */
                        vt_char_attrs = line->vt_char_attrs[x+xho];
#ifdef NEW_EXCLUSIVE
                        /* Give mutex back */
                        ReleaseVscrnMutex(vnum) ;
#endif /* NEW_EXCLUSIVE */

                        if ( vt_char_attrs & VT_CHAR_ATTR_HYPERLINK )
                            vt_char_attrs |= tt_url_hilite_attr;
                        else if ( tt_url_hilite && incnt < URLMINCNT) {
                            if ( IsURLChar(line->cells[x+xho].c) &&
                                 IsCellPartOfURL(vnum, y, x+xho ) )
                                vt_char_attrs |= tt_url_hilite_attr;
                        }

                        textBuffer[c+x] = line->cells[x+xho].c;
                        attrBuffer[c+x] = ComputeColorFromAttr(vnum,
                            line->cells[x+xho].a,
                            vt_char_attrs);
                        effectBuffer[c+x] = vt_char_attrs;
                    }
                }
            }
            else
            {
                /* In case we are in the middle of a scroll */
                for ( x = 0 ; x < xs ; x++ )
                {
                    textBuffer[c+x] = ' ';
                    attrBuffer[c+x] = ComputeColorFromAttr(vnum,
                        defaultattribute,0);
                    effectBuffer[c+x] = 0;
                }
            }
            c += xs;
            if ( line->width > maxWidth )
                maxWidth = line->width;
        }
    }
#ifdef EXCLUSIVE
    /* Give mutex back */
    ReleaseVscrnMutex(vnum) ;
    debug(F101,"IKTerm::getDrawInfo releases Mutex","",vnum);
#endif /* EXCLUSIVE */

    /* Status Line Display */
    if ( vnum == VTERM && tt_status[vnum] && decssdt != SSDT_BLANK ||
         vnum != VTERM && tt_status[vnum])
    {
        if ( vnum == VTERM && decssdt == SSDT_HOST_WRITABLE && tt_status[vnum] == 1) {
            line = &vscrn[VSTATUS].lines[0] ;
            for ( x = 0 ; x < xs ; x++ ) {
                textBuffer[c+x] = line->cells[x].c;
                attrBuffer[c+x] = ComputeColorFromAttr(vnum,
                                                        line->cells[x].a,
                                                        line->vt_char_attrs[x]);
                effectBuffer[c+x] = line->vt_char_attrs[x];
            }
            c += xs ;
        }
        else {/* SSDT_INDICATOR - Kermit Status Line */
            char * status = line25(vnum);

            for ( x = 0 ; x < xs ; x++ ) {
                if ( isunicode() ) {
                    textBuffer[c] = status[x] < 32 ? 
                        (*xl_u[TX_IBMC0GRPH])(status[x]) : 
                            status[x] >= 128 ? 
                                (*xl_u[TX_CP437])(status[x]) : 
                                    status[x] ;
                }
                else
                    textBuffer[c] = status[x] ;
                attrBuffer[c] = colorstatus ;
                effectBuffer[c] = '\0';
                c++ ;
            }
        }
    }

    getCursorPos();

    kcp->beg = vbuf->beg;
    kcp->top = vbuf->top;
    kcp->scrolltop = vbuf->scrolltop;
    kcp->end = vbuf->end;
    kcp->maxWidth = maxWidth;
    kcp->len = c;
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL IKTerm::getCursorPos()
{
    vnum = vmode;
    /* Wait for exclusive access to the screen */
    if ( RequestVscrnMutex( vnum, -1 ) )
        return FALSE;

    vbuf = &(vscrn[vnum]);

    char buf[30];
    ckmakmsg(buf,30,ckitoa(vbuf->cursor.x+1),", ",
              ckitoa(vbuf->cursor.y+1),NULL);
    KuiSetTerminalStatusText(STATUS_CURPOS, buf);

    /* only calculated an offset if Roll mode is INSERT */
    if( scrollstatus[vnum] && tt_roll[vnum] && markmodeflag[vnum] == notmarking ) {
        cursor_offset = (vbuf->top + vbuf->linecount - vbuf->scrolltop)
                        % vbuf->linecount;
    }
    else {
        cursor_offset = 0;
    }

    ys = VscrnGetHeight( vnum );
    if( !cursorena[vnum] || vbuf->cursor.y + cursor_offset >= ys -(tt_status[vnum]?1:0) ) {
        kcp->cursorVisible = FALSE;
    }
    else {
        kcp->cursorPt.x = vbuf->cursor.x;
        kcp->cursorPt.y = vbuf->cursor.y + cursor_offset;
        kcp->cursorVisible = TRUE;
    }
    ReleaseVscrnMutex(vnum) ;
    return kcp->cursorVisible;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
DWORD IKTerm::getControlKeyState()
{
    SHORT keystate;
    DWORD dwControlKeyState = 0x00000000;

    keystate = GetKeyState( VK_SHIFT );
    if( HIBYTE( keystate ) )
        dwControlKeyState |= SHIFT_PRESSED;

    keystate = GetKeyState( VK_LSHIFT );
    if( HIBYTE( keystate ) )
        dwControlKeyState |= SHIFT_PRESSED;

    keystate = GetKeyState( VK_RSHIFT );
    if( HIBYTE( keystate ) )
        dwControlKeyState |= SHIFT_PRESSED;

    keystate = GetKeyState( VK_LCONTROL );
    if( HIBYTE( keystate ) )
        dwControlKeyState |= LEFT_CTRL_PRESSED;

    keystate = GetKeyState( VK_RCONTROL );
    if( HIBYTE( keystate ) )
        dwControlKeyState |= RIGHT_CTRL_PRESSED;

    if ( !(dwControlKeyState & LEFT_CTRL_PRESSED) &&
         !(dwControlKeyState & RIGHT_CTRL_PRESSED) ) {
        keystate = GetKeyState( VK_CONTROL );
        if( HIBYTE( keystate ) )
            dwControlKeyState |= LEFT_CTRL_PRESSED;
    }

    keystate = GetKeyState( VK_LMENU );
    if( HIBYTE( keystate ) )
        dwControlKeyState |= LEFT_ALT_PRESSED;

    keystate = GetKeyState( VK_RMENU );
    if( HIBYTE( keystate ) )
        dwControlKeyState |= RIGHT_ALT_PRESSED;

    if ( !(dwControlKeyState & LEFT_ALT_PRESSED) &&
         !(dwControlKeyState & RIGHT_ALT_PRESSED) ) {
        keystate = GetKeyState( VK_MENU );
        if( HIBYTE( keystate ) )
            dwControlKeyState |= LEFT_ALT_PRESSED;
    }

    keystate = GetKeyState( VK_SCROLL );
    if( LOBYTE( keystate ) )
        dwControlKeyState |= SCROLLLOCK_ON;

    keystate = GetKeyState( VK_NUMLOCK );
    if( LOBYTE( keystate ) )
        dwControlKeyState |= NUMLOCK_ON;

    keystate = GetKeyState( VK_CAPITAL );
    if( LOBYTE( keystate ) )
        dwControlKeyState |= CAPSLOCK_ON;

    return dwControlKeyState;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL IKTerm::newKeyboardEvent( UINT chCharCode, LONG lKeyData, UINT keyDown, UINT sysKey )
{
    int keycount = 1 ;
    WORD wbuf[8] ;
    CHAR buf[8] ;
    static unsigned char keystate[256] ;
    static int (__stdcall *toUnicodeEx)(UINT,UINT,CONST PBYTE,LPWSTR,int,UINT,HKL)=NULL;
    static int (__stdcall *toUnicode)(UINT,UINT,CONST PBYTE,LPWSTR,int,UINT)=NULL;
    static int toUnicodeLoad = 0;

    vnum = vmode;

    memset( buf, '\0', sizeof(buf) );
    memset( &inpEvt, '\0', sizeof(INPUT_RECORD) );
    inpEvt.EventType = KEY_EVENT;
    inpEvt.Event.KeyEvent.bKeyDown = keyDown;
    inpEvt.Event.KeyEvent.wRepeatCount = (WORD)(lKeyData & 0x0000FFFF);

    long scancode = ( lKeyData & 0x00FF0000 ) >> 16;
    inpEvt.Event.KeyEvent.wVirtualScanCode = (UINT) scancode;

    inpEvt.Event.KeyEvent.wVirtualKeyCode = chCharCode;

    inpEvt.Event.KeyEvent.dwControlKeyState = getControlKeyState();
    if( lKeyData & 0x01000000 )
        inpEvt.Event.KeyEvent.dwControlKeyState |= ENHANCED_KEY;

    GetKeyboardState(keystate);

    if ( 0 ) {
        inpEvt.Event.KeyEvent.uChar.AsciiChar = 0;
#ifndef NOTERM
        if ( ISVTNT(tt_type_mode) &&
             ttnum != -1 
#ifndef NOLOCAL
             && IsConnectMode() 
#endif /* NOLOCAL */
             )
            ttol((unsigned char *)&inpEvt,sizeof(INPUT_RECORD));
        else
#endif /* NOTERM */
        win32KeyEvent( vnum, inpEvt.Event.KeyEvent );
    } else {
        if ( !isWin95() ) {
            int i;
            if ( !toUnicodeLoad ) {
                HINSTANCE hUSER32 = LoadLibrary("USER32");
                toUnicodeEx = (int (__stdcall *)(UINT,UINT,CONST PBYTE,LPWSTR,int,UINT,HKL))
                    GetProcAddress( hUSER32, "ToUnicodeEx" );
                toUnicode = (int (__stdcall *)(UINT,UINT,CONST PBYTE,LPWSTR,int,UINT))
                    GetProcAddress( hUSER32, "ToUnicode" );
                toUnicodeLoad = 1;
                debug(F101,"ToUnicodeEx","",toUnicodeEx);
                debug(F101,"ToUnicode","",toUnicode);
            }
            if ( nt351 )
                keycount = toUnicode( inpEvt.Event.KeyEvent.wVirtualKeyCode,
                        inpEvt.Event.KeyEvent.wVirtualScanCode | (keyDown ? 0x00 : 0x8000),
                        keystate,
                        (WORD *)wbuf,
                        8,
                        FALSE );                         
            else
                keycount = toUnicodeEx( inpEvt.Event.KeyEvent.wVirtualKeyCode,
                        inpEvt.Event.KeyEvent.wVirtualScanCode | (keyDown ? 0x00 : 0x8000),
                        keystate,
                        (WORD *)wbuf,
                        8,
                        FALSE,
                        GetKeyboardLayout(0) );                         
            for ( i=0;i<keycount;i++ )
                buf[i] = xl_tx[tcsl](wbuf[i]);
        } else {
            keycount = ToAsciiEx( inpEvt.Event.KeyEvent.wVirtualKeyCode,
                        inpEvt.Event.KeyEvent.wVirtualScanCode | (keyDown ? 0x00 : 0x8000),
                        keystate,
                        (WORD *)buf,
                        FALSE,
                        GetKeyboardLayout(0) );                         
        }

#ifdef COMMENT
        printf("\n");
        printf("kui KeyDown               0x%X\n",keyDown);
        printf("kui RepeatCount           0x%X\n",inpEvt.Event.KeyEvent.wRepeatCount);
        printf("kui wParam                0x%X\n",chCharCode);
        printf("kui keycount              0x%X\n",keycount);
        if ( keycount > 0 )
            printf("kui buf[0]                0x%X\n",buf[0]);
        if ( keycount > 1 )
            printf("kui buf[1]                0x%X\n",buf[1]);
        printf("\n");
#endif /* COMMENT */

        if ( keycount == 1 ) {
            inpEvt.Event.KeyEvent.uChar.AsciiChar = buf[0];
#ifndef NOTERM
            if ( ISVTNT(tt_type_mode) &&
                 ttnum != -1 
#ifndef NOLOCAL
                 && IsConnectMode() 
#endif /* NOLOCAL */
                 )
                ttol((unsigned char *)&inpEvt,sizeof(INPUT_RECORD));
            else
#endif /* NOTERM */
            win32KeyEvent( vnum, inpEvt.Event.KeyEvent );
        } else if ( keycount == 0 ) {
            inpEvt.Event.KeyEvent.uChar.AsciiChar = 0;
#ifndef NOTERM
            if ( ISVTNT(tt_type_mode) &&
                 ttnum != -1 
#ifndef NOLOCAL
                 && IsConnectMode() 
#endif /* NOLOCAL */
                 )
                ttol((unsigned char *)&inpEvt,sizeof(INPUT_RECORD));
            else
#endif /* NOTERM */
            win32KeyEvent( vnum, inpEvt.Event.KeyEvent );
        } else if ( keycount < 0 ) {
            inpEvt.Event.KeyEvent.wVirtualScanCode = 0;
            inpEvt.Event.KeyEvent.uChar.AsciiChar = 0;
#ifndef NOTERM
            if ( ISVTNT(tt_type_mode) &&
                 ttnum != -1 
#ifndef NOLOCAL
                 && IsConnectMode() 
#endif /* NOLOCAL */
                 )
                ttol((unsigned char *)&inpEvt,sizeof(INPUT_RECORD));
            else
#endif /* NOTERM */
            win32KeyEvent( vnum, inpEvt.Event.KeyEvent );
        } else if (keycount == 2) {
            inpEvt.Event.KeyEvent.uChar.AsciiChar = buf[0];
#ifndef NOTERM
            if ( ISVTNT(tt_type_mode) &&
                 ttnum != -1 
#ifndef NOLOCAL
                 && IsConnectMode() 
#endif /* NOLOCAL */
                 )
                ttol((unsigned char *)&inpEvt,sizeof(INPUT_RECORD));
            else
#endif /* NOTERM */
            win32KeyEvent( vnum, inpEvt.Event.KeyEvent );
            inpEvt.Event.KeyEvent.uChar.AsciiChar = buf[0];
#ifndef NOTERM
            if ( ISVTNT(tt_type_mode) &&
                 ttnum != -1 
#ifndef NOLOCAL
                 && IsConnectMode() 
#endif /* NOLOCAL */
                 )
                ttol((unsigned char *)&inpEvt,sizeof(INPUT_RECORD));
            else
#endif /* NOTERM */
            win32KeyEvent( vnum, inpEvt.Event.KeyEvent );
        }
    }
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL IKTerm::keyboardEvent( UINT chCharCode, LONG lKeyData, UINT keyDown )
{
    vnum = vmode;

    memset( &inpEvt, '\0', sizeof(INPUT_RECORD) );
    inpEvt.EventType = KEY_EVENT;
    inpEvt.Event.KeyEvent.bKeyDown = keyDown;
    inpEvt.Event.KeyEvent.wRepeatCount = (WORD)(lKeyData & 0x0000FFFF);
    inpEvt.Event.KeyEvent.uChar.AsciiChar = chCharCode;

    long scancode = ( lKeyData & 0x00FF0000 ) >> 16;
    inpEvt.Event.KeyEvent.wVirtualScanCode = (UINT) scancode;

    inpEvt.Event.KeyEvent.wVirtualKeyCode = MapVirtualKey( scancode, isWin95() ? 1 : 3 );

    inpEvt.Event.KeyEvent.dwControlKeyState = getControlKeyState();
    if( lKeyData & 0x01000000 )
        inpEvt.Event.KeyEvent.dwControlKeyState |= ENHANCED_KEY;

#ifndef NOTERM
    if ( ISVTNT(tt_type_mode) &&
         ttnum != -1 
#ifndef NOLOCAL
         && IsConnectMode() 
#endif /* NOLOCAL */
         )
        ttol((unsigned char *)&inpEvt,sizeof(INPUT_RECORD));
    else
#endif /* NOTERM */
        win32KeyEvent( vnum, inpEvt.Event.KeyEvent );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL IKTerm::virtkeyEvent( UINT virtkey, LONG lKeyData, UINT keyDown )
{
    vnum = vmode;

    memset( &inpEvt.Event.KeyEvent, '\0', sizeof(KEY_EVENT_RECORD) );

    inpEvt.Event.KeyEvent.bKeyDown = keyDown;
    inpEvt.Event.KeyEvent.wRepeatCount = (WORD)(lKeyData & 0x0000FFFF);
    inpEvt.Event.KeyEvent.uChar.AsciiChar =  virtkey;

    long scancode = ( lKeyData & 0x00FF0000 ) >> 16;
    inpEvt.Event.KeyEvent.wVirtualScanCode = (UINT) scancode;

    inpEvt.Event.KeyEvent.wVirtualKeyCode = MapVirtualKey( scancode, isWin95() ? 1 : 3 );

    inpEvt.Event.KeyEvent.dwControlKeyState = getControlKeyState();
    if( lKeyData & 0x01000000 )
        inpEvt.Event.KeyEvent.dwControlKeyState |= ENHANCED_KEY;

#ifndef NOTERM
    if ( ISVTNT(tt_type_mode) &&
         ttnum != -1 
#ifndef NOLOCAL
         && IsConnectMode() 
#endif /* NOLOCAL */
         )
        ttol((unsigned char *)&inpEvt,sizeof(INPUT_RECORD));
    else
#endif /* NOTERM */
        win32KeyEvent( vnum, inpEvt.Event.KeyEvent );
    return TRUE;
}
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void IKTerm::mouseEvent( HWND hwnd, UINT msg, WPARAM wParam, int x, int y )
{
    vnum = vmode;

    memset( &inpEvt, '\0', sizeof(INPUT_RECORD) );
    inpEvt.EventType = MOUSE_EVENT;

    // translate pixel coordinates into character coordinates
    //
    inpEvt.Event.MouseEvent.dwMousePosition.X = x;
    inpEvt.Event.MouseEvent.dwMousePosition.Y = y;
 
    //debug(F111,"IKTerm::MouseEvent","msg",msg);
    switch( msg )
    {
        case WM_MOUSEMOVE:
        //debug(F111,"IKTerm::MouseEvent","WM_MOUSEMOVE",wParam);
            {
            if( wParam & MK_LBUTTON )
                inpEvt.Event.MouseEvent.dwButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
            else if( wParam & MK_RBUTTON )
                inpEvt.Event.MouseEvent.dwButtonState = RIGHTMOST_BUTTON_PRESSED;
            else if( wParam & MK_MBUTTON )
                inpEvt.Event.MouseEvent.dwButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
            else
                return; // no processing necessary

            inpEvt.Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
            break;
        }

        case WM_LBUTTONDOWN:
            //debug(F110,"IKTerm::MouseEvent","WM_LBUTTONDOWN",0);
            inpEvt.Event.MouseEvent.dwEventFlags = 0;
            inpEvt.Event.MouseEvent.dwButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
            if ( !mouseCaptured )
                SetCapture(hwnd);
            mouseCaptured++;
            break;

        case WM_RBUTTONDOWN:
            //debug(F110,"IKTerm::MouseEvent","WM_RBUTTONDOWN",0);
            inpEvt.Event.MouseEvent.dwEventFlags = 0;
            inpEvt.Event.MouseEvent.dwButtonState = RIGHTMOST_BUTTON_PRESSED;
            if ( !mouseCaptured )
                SetCapture(hwnd);
            mouseCaptured++;
            break;

        case WM_MBUTTONDOWN:
            //debug(F110,"IKTerm::MouseEvent","WM_MBUTTONDOWN",0);
            inpEvt.Event.MouseEvent.dwEventFlags = 0;
            inpEvt.Event.MouseEvent.dwButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
            if ( !mouseCaptured )
                SetCapture(hwnd);
            mouseCaptured++;
            break;

        case WM_LBUTTONUP:
            //debug(F110,"IKTerm::MouseEvent","WM_LBUTTONUP",0);
            inpEvt.Event.MouseEvent.dwEventFlags = 0;
            inpEvt.Event.MouseEvent.dwButtonState = 0; //FROM_LEFT_1ST_BUTTON_PRESSED;
            mouseCaptured--;
            if ( !mouseCaptured )
                ReleaseCapture();
            break;

        case WM_RBUTTONUP:
            //debug(F110,"IKTerm::MouseEvent","WM_RBUTTONUP",0);
            inpEvt.Event.MouseEvent.dwEventFlags = 0;
            inpEvt.Event.MouseEvent.dwButtonState = 0; //RIGHTMOST_BUTTON_PRESSED;
            mouseCaptured--;
            if ( !mouseCaptured )
                ReleaseCapture();
            break;

        case WM_MBUTTONUP:
            //debug(F110,"IKTerm::MouseEvent","WM_MBUTTONUP",0);
            inpEvt.Event.MouseEvent.dwEventFlags = 0;
            inpEvt.Event.MouseEvent.dwButtonState = 0; //FROM_LEFT_2ND_BUTTON_PRESSED;
            mouseCaptured--;
            if ( !mouseCaptured )
                ReleaseCapture();
            break;

        case WM_LBUTTONDBLCLK:
            //debug(F110,"IKTerm::MouseEvent","WM_LBUTTONDBLCLK",0);
            inpEvt.Event.MouseEvent.dwEventFlags = DOUBLE_CLICK;
            inpEvt.Event.MouseEvent.dwButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
            break;

        case WM_RBUTTONDBLCLK:
            //debug(F110,"IKTerm::MouseEvent","WM_RBUTTONDBLCLK",0);
            inpEvt.Event.MouseEvent.dwEventFlags = DOUBLE_CLICK;
            inpEvt.Event.MouseEvent.dwButtonState = RIGHTMOST_BUTTON_PRESSED;
            break;

        case WM_MBUTTONDBLCLK:
            //debug(F110,"IKTerm::MouseEvent","WM_MBUTTONDBLCLK",0);
            inpEvt.Event.MouseEvent.dwEventFlags = DOUBLE_CLICK;
            inpEvt.Event.MouseEvent.dwButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
            break;

        default:
            debug(F111,"IKTerm::MouseEvent","error",msg);
            return; // error!
    }

    inpEvt.Event.MouseEvent.dwControlKeyState = getControlKeyState();
    //debug(F111,"IKTerm::MouseEvent","dwControlKeyState",inpEvt.Event.MouseEvent.dwControlKeyState);

    win32MouseEvent( vnum, inpEvt.Event.MouseEvent );
}
