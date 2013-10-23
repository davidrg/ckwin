#ifndef kcommand_hxx_included
#define kcommand_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcommand.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command window.

========================================================================*/
#include "kappwin.hxx"

class KClient;
class KCommand : public KAppWin
{
public:
    KCommand( K_GLOBAL* );
    ~KCommand();

    void getCreateInfo( K_CREATEINFO* info );
    void createWin( KWin* par );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

private:
    KClient* client;
};

#endif

