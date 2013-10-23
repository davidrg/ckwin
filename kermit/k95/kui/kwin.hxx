#ifndef kwin_hxx_included
#define kwin_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kwin.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: base class for display gui windows.  Used to
        encapsulate basic gui functionality.

========================================================================*/

#include "kuidef.h"

class KHwndSet;
class KWin
{
public:
    KWin( K_GLOBAL* );
    virtual ~KWin();

    virtual void init();
    virtual void getCreateInfo( K_CREATEINFO* info );
    virtual void createWin( KWin* par );
    virtual void size( int width, int height );
    virtual Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    virtual void show( Bool bVisible = TRUE );
    virtual void takeFocus();

    virtual Bool paint();

    Bool inCreate() { return _inCreate; }
    void inCreate( Bool b ) { _inCreate = b; }

    HWND hwnd() { return hWnd; }
    void getSize( int& w, int& h );
    void getPos( int& x, int& y );
    void associateHwnd( HWND );
    virtual int sizeFont( LPRECT lpr, int force ) { return -1; };
    virtual void sizeFontSetDim( UINT fwSide, LPRECT lpr ) {};

  protected:

    KWin* parent;
    HWND hWnd;
    K_GLOBAL* kglob;
    HINSTANCE hInst;
    Bool  firstShow;

private:
    Bool _inCreate;
};

// KWin member class function pointer
typedef long (KWin::* KWinMethod)(long);
#define KWinMethodOf(o,m) (KWinMethod)(long (o::*)(long) )(&o::m)

#endif
