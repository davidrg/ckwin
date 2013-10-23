#ifndef kcmdmous_hxx_included
#define kcmdmous_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmdmous.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command processor for mouse property page.

========================================================================*/

#include "kcmdproc.hxx"

class KCmdMouse : public KCmdProc
{
public:
    KCmdMouse( K_GLOBAL* );
    ~KCmdMouse();

    Bool doInitDialog( HWND );
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
    Bool killActive();

protected:
    void processDefinition();
    void doAction();

private:
    long lastAction;
};

#endif
