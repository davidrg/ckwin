#ifndef khwndset_hxx_included
#define khwndset_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: khwndset.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: keeps track of KWin - HWND pairings.

========================================================================*/

#include "kuidef.h"

class KArray;
class KWin;

typedef struct _KWIN_HWND {
    KWin* win;
    HWND hwnd;
} KWIN_HWND;

class KHwndSet
{
public:
    KHwndSet();
    ~KHwndSet();

    KWin* find( HWND );
    void add( KWin*, HWND );
    void remove( KWin* );
    void remove( HWND );

private:
    KWIN_HWND* locate( KWin* );
    KWIN_HWND* locate( HWND );

    KArray* array;
    long arraycount;
};

#endif
