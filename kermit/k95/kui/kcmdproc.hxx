#ifndef kcmdproc_hxx_included
#define kcmdproc_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmdproc.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: base class for command processing used by 
        property pages.

========================================================================*/

#include "kuidef.h"

class KArray;
class KCmdProc
{
public:
    KCmdProc( K_GLOBAL* );
    virtual ~KCmdProc();

    virtual Bool doInitDialog( HWND h ) { hWnd = h; return FALSE; }
    virtual Bool doCommand( HWND, WORD, WORD, HWND ) = 0;
    virtual Bool doScroll( HWND, WORD, WORD, HWND ) { return FALSE; }
    virtual Bool killActive() = 0;

    void bufferCommand( char* );
    Bool verifyChanges();

protected:
    K_GLOBAL* kglob;
    HINSTANCE hInst;
    HWND hWnd;

private:
    KArray* cmdarray;
    long arraycount;
    
};

#endif
