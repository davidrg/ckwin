#ifndef kcmdprot_hxx_included
#define kcmdprot_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmdprot.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command processor for protocol property page.

========================================================================*/

#include "kcmdproc.hxx"

class KCmdProtocol : public KCmdProc
{
public:
    KCmdProtocol( K_GLOBAL* );
    ~KCmdProtocol();

    Bool doInitDialog( HWND );
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
    Bool killActive();

protected:
    void doAction( int action );

private:
    HWND hlist;
    int lastAction;
};

#endif
