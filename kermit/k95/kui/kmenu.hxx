#ifndef kmenu_hxx_included
#define kmenu_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kmenu.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: main window's menu bar.

========================================================================*/

#include "kuidef.h"

class KWin;
class KMenu
{
public:
    KMenu( int resid );
    ~KMenu();

    void createMenu( HINSTANCE hinst, KWin* par );
    void getHeight( int& h );
    void setCheck( UINT id );
    void unsetCheck( UINT id );
    bool toggleCheck( UINT id );
    void setCopyPaste( bool textselected, bool clipboarddata );
    void enable(UINT id, bool on = true);
    void remove(UINT id);

private:
    HMENU hMenu;
    int menuid;
    KWin* parent;

};

#endif
