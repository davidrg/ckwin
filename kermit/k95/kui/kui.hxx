#ifndef kui_hxx_included
#define kui_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kui.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: message redirector for all main windows.
        Used for terminal, command and clientserver windows.

========================================================================*/
#include "kuidef.h"

class KTerminal;
class Kui
{
public:
    Kui( HINSTANCE );
    ~Kui();

    int init();
    Bool message( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
    void start();
    void cleanup();

    void savePositions();
    void setProperty( int propid, intptr_t param1, intptr_t param2 );
    int getProperty( int propid, void* out );

    void setDialogHwnd( HWND h ) { dialogHwnd = h; }
    KTerminal * getTerminal(void) { return terminal; };

private:
    KTerminal* terminal;
    HWND dialogHwnd;
};

#endif
