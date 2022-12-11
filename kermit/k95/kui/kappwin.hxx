#ifndef kappwin_hxx_included
#define kappwin_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kappwin.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: January 5, 1996

    Description: base class for application windows 
        e.g. terminal, command, clientserver.

========================================================================*/
#include "kwin.hxx"
#include "kfont.hxx"
#include "kclient.hxx"
#include "kstatus.hxx"
#include "ktoolbar.hxx"

#define WM_USER_SIZING WM_USER+12
#define WM_USER_EXITSIZEMOVE WM_USER+13

#ifndef WM_SIZING
#define WM_SIZING 0x0214
#endif

#ifndef WM_EXITSIZEMOVE
#define WM_EXITSIZEMOVE 0x0232
#endif

#ifndef WMSZ_LEFT
#define WMSZ_LEFT           1
#endif
#ifndef WMSZ_RIGHT
#define WMSZ_RIGHT          2
#endif
#ifndef WMSZ_TOP
#define WMSZ_TOP            3
#endif
#ifndef WMSZ_TOPLEFT
#define WMSZ_TOPLEFT        4
#endif
#ifndef WMSZ_TOPRIGHT
#define WMSZ_TOPRIGHT       5
#endif
#ifndef WMSZ_BOTTOM
#define WMSZ_BOTTOM         6
#endif
#ifndef WMSZ_BOTTOMLEFT
#define WMSZ_BOTTOMLEFT     7
#endif
#ifndef WMSZ_BOTTOMRIGHT
#define WMSZ_BOTTOMRIGHT    8
#endif


class KSizePopup;
class KMenu;
class KToolBar;
class KStatus;
class KClient;
class KAppWin : public KWin
{
public:
    KAppWin( K_GLOBAL*, int menuid, int toolbarid );
    ~KAppWin();
    void createWin( KWin* par );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    void size( int width, int height );
    void getClientCoord( int& x, int& y, int& w, int& h );
    void setClient( KClient* c ) { client = c; }
    KClient* getClient(void) { return client; }
    void takeFocus();
    virtual void setKFont( KFont*f );
    KFont * getKFont() { return client->getFont(); };
    virtual void setFont( char * fnt, int h ) {};
    Bool sizing( UINT fwSide, LPRECT lpr );
    virtual void initMenu(void);
    KMenu * getMenu(void) {return menu;};
    KToolBar* getToolbar(void) { return toolbar; };
    int     readManual(void);
    int     browse(char * url);
    void    setStatusText(int item, char * buf) { if (status) status->setText(item,buf); }
    virtual int sizeFont( LPRECT lpr, int force );
    virtual void sizeFontSetDim( UINT fwSide, LPRECT lpr );
    void    createMenu(void);
    void    destroyMenu(void);

  protected:
    int Win32ShellExecute( char * );
    void sizeLimit( UINT fwSide, LPRECT lpr );
    void sizeFixed( UINT fwSide, LPRECT lpr );

    KMenu* menu;
    KToolBar* toolbar;
    KStatus* status;

  private:
    KClient* client;
    KSizePopup* sizepop;
    int      wmSize;
    int      menuId;
};

#endif
