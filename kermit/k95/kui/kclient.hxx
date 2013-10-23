#ifndef kclient_hxx_included
#define kclient_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kclient.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: base class for main window's client area.
        Used by terminal, command and clientserver.

========================================================================*/

#include "kwin.hxx"
#include "kscroll.hxx"

const int MAXNUMCOL = 256;
struct _K_CLIENT_PAINT;
struct _K_WORK_STORE;

class IKTerm;
class KFont;
class KClient : public KWin
{
public:
    KClient( K_GLOBAL*, BYTE );
    ~KClient();

    void getCreateInfo( K_CREATEINFO* info );
    void createWin( KWin* par );
    void size( int width, int height );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    void setDimensions( Bool sizeparent );
    void setFont( KFont* );
    void clearPaintRgn();
    void setInterSpacing( KFont* );

    Bool getMaxpDim( int& rw, int& rh );
    KFont* getFont() { return font; }
    HDC hdc() { return _hdc; }
    HDC hdcScreen() { return _hdcScreen; }
    int margin() { return _margin; }

    void writeMe();
    Bool paint();
    void endSizing( Bool doAnyway = FALSE );

    // scrollbar callbacks
    long vertScroll( long );
    long horzScroll( long );

    BYTE getClientID() { return clientID; }
    long msgret() { return _msgret; }

    void checkBlink();
    void getDrawInfo();
    void drawDisabledState( int w, int h );

    void getEndSize( int& w, int& h );
    void calculateSize( int& w, int& h );

    void syncSize(void);

    KScroll* getVertSB(void) { return vert; }
    KScroll* getHorzSB(void) { return horz; }

private:    // this section is for performance
    uchar* workTemp;
    size_t workTempSize;

    ushort* textBuffer;
    uchar* attrBuffer;
    ushort* effectBuffer;
    ushort* lineAttr;
    _K_WORK_STORE* kws;

    uchar prevAttr;
    ushort prevEffect;

    int wc;
    int vscrollpos;
    int hscrollpos;

    void ToggleCursor(HDC, LPRECT);

    IKTerm* ikterm;
    BYTE clientID;
    _K_CLIENT_PAINT* clientPaint;
    _K_WORK_STORE* workStore;
    Bool  ws_blinking;
    Bool  cursor_displayed;

    HDC _hdc;
    HDC _hdcScreen;
    HDC _hdcScratch;
    HBITMAP compatBitmap;
    HBITMAP scratchBitmap;
    HRGN hrgnPaint;
    HBRUSH disabledBrush;
    HBRUSH bgBrush;
    DWORD savebgcolor;
    int interSpace[MAXNUMCOL];

    Bool _inFocus;
    RECT cursorRect;
    int cursorCount;
    int maxCursorCount;
    int blinkInterval;

    UINT timerID;
    KFont* font;

    KScroll* vert;
    KScroll* horz;

    int saveTermHeight;
    int saveTermWidth;
    double saveLineSpacing;
    int saveHorzIsV;

    int maxpHeight;     // max parent window height
    int maxpWidth;      // max parent window width
    int maxHeight;      // max client window height
    int maxWidth;       // max client window width

    int _margin;        // margin around the display area (bottom & right)

    int _xoffset;       // x and y offsets used by scrolling
    int _yoffset;

    Bool processKey;
    long _msgret;
};

#endif
