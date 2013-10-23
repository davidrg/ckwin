#ifndef kcolor_hxx_included
#define kcolor_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcolor.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: color dialog box.

========================================================================*/

#include "kwin.hxx"

class KColor : public KWin
{
public:
    KColor( K_GLOBAL*, KWin* par );
    ~KColor();

    void show( Bool bVisible = TRUE );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );
};

#endif
