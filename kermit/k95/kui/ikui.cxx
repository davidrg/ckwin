#include "kui.hxx"
#include "ktermin.hxx"
#include "kregedit.hxx"
#include "ksysmets.hxx"
#include "khwndset.hxx"
#include "kdwnload.hxx"
#include "ikextern.h"

extern "C" {
#include <windows.h> 
#include <process.h> 
#include "ikui.h"
}

// there is only one K_GLOBAL shared by all objects.
// this is used so that C functions (mostly wndprocs) have access to
// the KWin class pointers
//
K_GLOBAL* kglob = 0;
Kui* kui = 0;
KRegistryEdit* kreg = 0;

static HANDLE hevKuiInitialized = NULL;

void KuiWindowThread( void* param );

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
int KuiInit( void* hInstance, struct _kui_init * kui_init )
{
    // The following code was used to create 
    //char* filename = "\\latin2.ttf"; //"e:\\kui\\fonts\\temp.ttf";
    //char* outfile = "\\latin2.fot"; //"e:\\kui\\fonts\\tempout.fot";
    //HRSRC src = FindResource( (HINSTANCE)hInstance, MAKEINTRESOURCE(IDR_FONTLAT2), RT_FONT );
    //DWORD size = SizeofResource( (HINSTANCE)hInstance, src );
    //HGLOBAL hglobal = LoadResource( (HINSTANCE)hInstance, src );
    //LockResource( hglobal );

    //HANDLE file = CreateFile( filename, GENERIC_WRITE, 0, NULL
    //                          , CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL );

    //DWORD numread = 0;
    //BOOL b = WriteFile( file, hglobal, size, &numread, NULL );

    //CloseHandle( file );

    //b = CreateScalableFontResource( 1, outfile, filename, NULL );
    //int numadded = AddFontResource( outfile );

    //DeleteFile( filename );

    // intialize K_GLOBAL
    //
    kglob = new K_GLOBAL;
    memset( kglob, '\0', sizeof(K_GLOBAL) );

    kglob->hInst = (HINSTANCE)hInstance;
    kglob->hwndset = new KHwndSet();
    kglob->sysMets = new KSysMetrics();

    // read initial window placement settings from the registry
    //
    kreg = new KRegistryEdit();
    if (kui_init->pos_init) {
	kglob->settings[Terminal_Left] = kui_init->pos_x;
        kglob->settings[Terminal_Top]  = kui_init->pos_y;
        kglob->settings[Command_Left] = (DWORD)CW_USEDEFAULT;
        kglob->settings[Command_Top]  = 0L;
        kglob->settings[CServer_Left] = (DWORD)CW_USEDEFAULT;
        kglob->settings[CServer_Top]  = 0L;
    } else {
	// kreg->getCoordinates( kglob );
        kglob->settings[Terminal_Left] = (DWORD)CW_USEDEFAULT;
        kglob->settings[Terminal_Top]  = 0L;
        kglob->settings[Command_Left] = (DWORD)CW_USEDEFAULT;
        kglob->settings[Command_Top]  = 0L;
        kglob->settings[CServer_Left] = (DWORD)CW_USEDEFAULT;
        kglob->settings[CServer_Top]  = 0L;
    }

    kreg->getFontinfo( kglob );
    if (kui_init->face_init)
	strcpy(kglob->faceName,kui_init->facename);
    if (kui_init->font_init)
	kglob->fontHeight = kui_init->font_size;

    kglob->noMenuBar = kui_init->nomenubar;
    kglob->noToolBar = kui_init->notoolbar;
    kglob->noStatus  = kui_init->nostatusbar;
    kglob->nCmdShow  = kui_init->nCmdShow;
    kglob->mouseEffect = kui_init->resizeMode;
    kglob->noClose   = kui_init->noclose;
    // kglob->mouseEffect = TERM_MOUSE_CHANGE_DIMENSION;
    // kglob->mouseEffect = TERM_MOUSE_NO_EFFECT;
    // kglob->mouseEffect = TERM_MOUSE_CHANGE_FONT;

    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
HANDLE KuiThreadInit( void* hInstance )
{   
    DWORD rc = 0;
    hevKuiInitialized = CreateEvent( NULL, TRUE, FALSE, NULL );
    _beginthread( KuiWindowThread, 65535, hInstance );
    rc = WaitForSingleObjectEx( hevKuiInitialized, (ULONG) -1L, TRUE );
    if ( rc != WAIT_OBJECT_0 )
        return FALSE;
    CloseHandle(hevKuiInitialized);
    hevKuiInitialized = NULL;
    // the window should now be initialized so change the default to 
    // change dimension.
    return kui->getTerminal()->getClient()->hwnd();
}
   
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
int KuiThreadCleanup( void* hInstance )
{
    //Cleanup Embedded Font
    //BOOL b = RemoveFontResource( outfile );
    //DeleteFile( outfile );

    if ( IsZoomed(kui->getTerminal()->hwnd()) || 
         IsIconic(kui->getTerminal()->hwnd())) {
        ShowWindowAsync(kui->getTerminal()->hwnd(), SW_RESTORE);
        Sleep(50);
    }

    kui->savePositions();
    kui->cleanup();

    // save last window positions
    //
    kreg->setCoordinates( kglob );
    kreg->setFontinfo( kglob );
    delete kreg;

    return TRUE;
}

void KuiSetTerminalStatusText(int item, char * buf)
{
    if (kui)
        kui->getTerminal()->setStatusText(item,buf);
}

void KuiSetTerminalConnectButton(int on) 
{
    if (kui) {
        KToolBar * toolbar = kui->getTerminal()->getToolbar();
        if ( toolbar )
            SendMessage(toolbar->hwnd(),
                     TB_CHECKBUTTON, ID_ACTION_EXIT, 
                     MAKELONG(on,0));
    }
}

void 
KuiSetTerminalPosition( int x, int y)
{
    if ( kui )
        SetWindowPos(kui->getTerminal()->hwnd(), 0, x, y, 0, 0,
                      SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_NOSIZE );
}

void 
KuiSetTerminalSize( int x, int y)
{
    if ( kui ) {
        RECT rect;
        GetWindowRect(kui->getTerminal()->hwnd(),&rect);
        if ( x == 0 )
            x = rect.right - rect.left;
        if ( y == 0 )
            y = rect.bottom - rect.top;
        SetWindowPos(kui->getTerminal()->hwnd(), 0, 0, 0, x, y,
                      SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_NOMOVE );
        msleep(50);
        kui->getTerminal()->getClient()->syncSize();
    }
}

void 
KuiSetTerminalResizeMode(int x)
{
    if ( kui )
        SendMessage(kui->getTerminal()->hwnd(), WM_COMMAND,
                     x == TERM_MOUSE_CHANGE_FONT ? ID_ACTION_RESIZE_FONT : 
                     x == TERM_MOUSE_CHANGE_DIMENSION ? ID_ACTION_RESIZE_DIMENSION :
                     ID_ACTION_RESIZE_NONE, 0);
}

int
KuiGetTerminalResizeMode(void)
{
    return kglob->mouseEffect;
}

void 
KuiSetTerminalRunMode(int x)
{
    if ( kui ) {
        switch ( x ) {
        case 2: /* minimize */
            ShowWindow(kui->getTerminal()->hwnd(), SW_MINIMIZE);
            break;
        case 3: /* maximize */
            ShowWindow(kui->getTerminal()->hwnd(), SW_MAXIMIZE);
            break;
        case 1: /* restore */
        default:
            ShowWindow(kui->getTerminal()->hwnd(), SW_RESTORE);
            break;
                
        }
    }
}

int
KuiDownloadDialog(char * title, char * def, char * result, int rlen)
{
    if ( kui ) {
        KDownLoad download( kglob );
        download.setTitle(title);
        download.setInitialFileName(def);
        download.createWin( kui->getTerminal() );
        download.show();
        if ( !download.wasCancelled() ) {
            ckstrncpy(result, download.getSelectedFileName(), rlen);
            return 1;
        }
    }
    return 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KuiWindowThread( void* hInstance )
{
    kui = new Kui( (HINSTANCE)hInstance );
    kui->init();
    SetEvent(hevKuiInitialized);
    kui->start();
    delete kui;

    delete kglob->hwndset;
    delete kglob->sysMets;
    delete kglob;
} 

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KuiSetProperty( int propid, long param1, long param2 )
{
    if( kui )
        kui->setProperty( propid, param1, param2 );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KuiGetProperty( int propid, long param1, long param2 )
{
    if( kui )
        kui->getProperty( propid, param1, param2 );
}
