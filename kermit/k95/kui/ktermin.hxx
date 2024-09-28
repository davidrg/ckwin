#ifndef ktermin_hxx_included
#define ktermin_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: ktermin.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: terminal window 
        (the primary user interface window).

========================================================================*/
#include "kappwin.hxx"

enum eFileType { eCommandFile, eKeyMapFile, eTextFile, eCaptureFile };
class KClient;
class KTerminal : public KAppWin
{
public:
    KTerminal( K_GLOBAL* );
    ~KTerminal();

    void getCreateInfo( K_CREATEINFO* info );
    void createWin( KWin* par );
    Bool message( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
    virtual void show( Bool bVisible = TRUE );

    void setTermType( char* );
    void setFont( char *, int );
    virtual void setKFont( KFont*f );
    void setRemoteCharset( char* );
 
    void browseFile( eFileType filetype );
    void initMenu( void );
    void disableMenu( void );
    void disableToolbar( void );
    void disableClose( void );
    void setMenubarVisible(Bool visible);
    void setToolbarVisible(Bool visible);
    void setStatusbarVisible(Bool visible);

    void setMenuItemChecked(UINT menuItemId, Bool checked);
    void setConnectMode(Bool on);

  private:
    Bool OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y,
                         UINT codeHitTest);
    void disableMenuItem(UINT menuItemId);

    Bool toolbar_disabled;
    KClient* client;
    Bool menuInitialized;
    int  firstShow;
    Bool noDialer;

    Bool menuDisabled;
    HMENU sysActions;

    // Track menu state for items that aren't backed by some global variable.
    // These all default to FALSE
    Bool menuCapture, menuPrinterCopy, menuDebug, menuPcTerm, menuKeyClick,
         menuTerminalScreen;
    // These all default to TRUE
    Bool menuGuiDialogs;

};

#endif
