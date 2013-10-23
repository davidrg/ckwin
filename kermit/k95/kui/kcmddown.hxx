#ifndef kcmddown_hxx_included
#define kcmddown_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmddown.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command processor for download property page.

========================================================================*/

#include "kcmdproc.hxx"

class KCmdDownload : public KCmdProc
{
public:
    KCmdDownload( K_GLOBAL* );
    ~KCmdDownload();

    Bool doInitDialog( HWND );
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
    Bool doScroll( HWND hPar, WORD pos, WORD scrollcode, HWND hCtrl );
    Bool killActive();

protected:
    void doFilenameGroup( int );

};


#endif
