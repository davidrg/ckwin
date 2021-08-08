#ifndef kregedit_hxx_included
#define kregedit_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kregedit.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: win95 registry functions.

========================================================================*/
#include "kuidef.h"

class KRegistryEdit
{
public:
    KRegistryEdit();
    ~KRegistryEdit();

    bool getCoordinates( K_GLOBAL* );
    bool setCoordinates( K_GLOBAL* );
    bool getFontinfo( K_GLOBAL* );
    bool setFontinfo( K_GLOBAL* );

protected:
    HKEY openSettings();

};

#endif
