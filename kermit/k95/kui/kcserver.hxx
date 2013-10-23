#ifndef kcserver_hxx_included
#define kcserver_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcserver.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: clientserver window.

========================================================================*/
#include "kappwin.hxx"

class KClientClientServ;
class KClientServer : public KAppWin
{
public:
    KClientServer( K_GLOBAL* );
    ~KClientServer();

    void getCreateInfo( K_CREATEINFO* info );
    void createWin( KWin* par );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

private:
    KClientClientServ* client;
};

#endif

