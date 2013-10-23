#ifndef kstatus_hxx_included
#define kstatus_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kstatus.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: main window's status bar.

========================================================================*/
#include "kwin.hxx"

class KArray;
class KStatusCustomDlg;
class KStatus : public KWin
{
public:
enum eAction { NOACTION, REMOVEPANE, ADDPANE };

    KStatus( K_GLOBAL* );
    ~KStatus();

    void getCreateInfo( K_CREATEINFO* info );
    void createWin( KWin* par );
    void size( int width, int height );

    void setParts( int numParts, int* partWidths );
    void setText( int part, char* text );
    void customize();
    void endCustomize();
    Bool endPaneMove( eAction, long xpos );

    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );
    WNDPROC getdefproc() { return defproc; }
    HWND getCustomHwnd() { return customHwnd; }

protected:
    Bool mouseMove( long wParam, long x, long y );
    Bool lButtonDown( long wParam, long x, long y );
    Bool lButtonUp( long wParam, long x, long y );
    Bool rButtonDown( long wParam, long x, long y );
    Bool findDragPane( long wParam, long x, long y );

    Bool addPane( long xpos );
    Bool removePane();

private:
    WNDPROC defproc;
    KArray* paneArray;
    int numParts;
    int* offsets;
    int* statusWidths;

    Bool _customize;
    KStatusCustomDlg* custdlg;
    HWND customHwnd;
    long custXOffset;
    long custXWidth;
    int whichPane;
    int partDrag;
    Bool mouseOverPart;

};

#endif
