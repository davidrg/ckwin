#ifndef kcustdlg_hxx_included
#define kcustdlg_hxx_included
/*========================================================================
    Copyright (c) 1996, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcustdlg.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: May 28, 1996

    Description: status bar customization dialog box

========================================================================*/
#include "kwin.hxx"

typedef struct _dragPaneInfo {
    char text[128];
} dragPaneInfo;

class KStatus;
class KStatusCustomDlg : public KWin
{
public:
    KStatusCustomDlg( K_GLOBAL*, KStatus* );
    ~KStatusCustomDlg();

    void createWin( KWin* par );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    void endCustomize();
    WNDPROC getLBProc() { return LBProc; }

    Bool mouseMove( long wParam, long x, long y );
    Bool rButtonDown( long wParam, long x, long y );
    Bool rButtonUp( long wParam, long x, long y );

    void beginPaneMove( long x, long y, long xx, long yy );
    dragPaneInfo* getDragPane() { return &dragPane; }

private:
enum eDisposition { NONE, FROMSTATUSBAR, TOSTATUSBAR };
    
    KStatus* status;
    WNDPROC LBProc;
    HWND dragPopup;
    HWND hList;

    eDisposition disposition;
    long xoff;
    long yoff;

    long numEntries;
    dragPaneInfo dragPane;
};

#endif
