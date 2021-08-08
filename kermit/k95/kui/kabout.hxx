#ifndef kabout_hxx_included
#define kabout_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kabout.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: handles the display of the about box.

========================================================================*/

#include "kwin.hxx"

class KAbout : public KWin
{
public:
    KAbout( K_GLOBAL*, KWin* par );
    ~KAbout();

    void show( bool bVisible = true );
    bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

private:
    bool anotherIsRunning;
};

#endif
