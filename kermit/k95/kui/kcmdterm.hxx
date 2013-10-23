#ifndef kcmdterm_hxx_included
#define kcmdterm_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmdterm.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command processor for terminal property page.

========================================================================*/

#include "kcmdproc.hxx"

class KArray;
class KCmdTerminal : public KCmdProc
{
public:
    KCmdTerminal( K_GLOBAL* );
    ~KCmdTerminal();

    Bool doInitDialog( HWND );
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
    Bool killActive();

    BOOL advMessage( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

protected:
    void doAction( int action );
    void saveAction( int action );
    char* getCurrentVal( HWND );
	void showDialog( long dlgid );
	void hideDialog( long dlgid );

    Bool doAdvCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
	Bool doGLGR( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );

private:
    HWND hlist;
    int lastAction;
	KArray* dlgArray;
	long dlgArrayCount;

};

#endif
