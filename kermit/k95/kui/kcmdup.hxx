#ifndef kcmdup_hxx_included
#define kcmdup_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmdup.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command processor for upload property page.

========================================================================*/

#include "kcmdproc.hxx"

class KCmdUpload : public KCmdProc
{
public:
    KCmdUpload( K_GLOBAL* );
    ~KCmdUpload();

    Bool doInitDialog( HWND );
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
    Bool doScroll( HWND hPar, WORD pos, WORD scrollcode, HWND hCtrl );
    Bool killActive();

protected:
    void doFilenameGroup( int );
    void doDispositionGroup( int );

};


#endif
