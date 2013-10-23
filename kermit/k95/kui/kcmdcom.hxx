#ifndef kcmdcom_hxx_included
#define kcmdcom_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmdcom.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command processor for communications property page.

========================================================================*/

#include "kcmdproc.hxx"

class KCmdCommunications : public KCmdProc
{
public:
    KCmdCommunications( K_GLOBAL* );
    ~KCmdCommunications();

    Bool doInitDialog( HWND );
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
    Bool killActive();
};

#endif
