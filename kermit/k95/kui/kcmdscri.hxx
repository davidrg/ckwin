#ifndef kcmdscri_hxx_included
#define kcmdscri_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmdscri.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command processor for script property page.

========================================================================*/

#include "kcmdproc.hxx"

class KCmdScript : public KCmdProc
{
public:
    KCmdScript( K_GLOBAL* );
    ~KCmdScript();

    Bool doInitDialog( HWND );
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
    Bool doScroll( HWND hPar, WORD pos, WORD scrollcode, HWND hCtrl );
    Bool killActive();
};

#endif
