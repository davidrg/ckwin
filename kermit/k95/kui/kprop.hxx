#ifndef kprop_hxx_included
#define kprop_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kprop.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: main property dialog box.

========================================================================*/

#include "kprop.h"
#include "kwin.hxx"

class KCmdProc;
class KProperty : public KWin
{
public:
    KProperty( K_GLOBAL* );
    ~KProperty();

    void createWin( KWin* par );
    void show( Bool bVisible = TRUE );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    void turnPage( int tp );

    cmdCallback* findCmdCallback( HWND hfind );

protected:
    Bool initDialog( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

private:
    cmdCallback** cmdArray;
    KCmdProc* currentCmdProc;

    int topPage;

};

#endif
