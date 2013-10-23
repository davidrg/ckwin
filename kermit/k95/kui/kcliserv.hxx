#ifndef kcliserv_hxx_included
#define kcliserv_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcliserv.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: clientserver window's client area.

========================================================================*/

#include "kclient.hxx"

class KClientClientServ : public KClient
{
public:
    KClientClientServ( K_GLOBAL*, BYTE );
    ~KClientClientServ();
};

#endif
