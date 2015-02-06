#ifndef kuidef_h_included
#define kuidef_h_included

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>    // includes common dialog functionality
#include <dlgs.h>       // includes common dialog template defines
#include <stdio.h>
#include <stdlib.h>
#include <shellapi.h>

#include "resource.h"   // for all dlg template values

// Visual C++ 6.0 for-loop scope bug workaround (_MSC_VER 1300 is Visual C++ 7 
// which behaves properly)
#if _MSC_VER < 1300
#define for if (0) {} else for
#endif

// some useful functions

char* newstr( char* );      // duplicate the passed string
int GetGlobalID( void );    // return a unique id for CreateWindowEx

#define TRUE  1
#define FALSE 0

typedef int Bool;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

const char KWinClassName[] = "KERMIT95:UI";

const char CmdTitle[] = "K95 Command Window";
const char TermTitle[] = "C-Kermit Terminal Window";
const char CServerTitle[] = "K95 Client Server Window";

const uint TERMINAL_WINDOW      = 1;
const uint COMMAND_WINDOW       = 2;
const uint FILETRANSFER_WINDOW  = 4;

const uint WM_GOTO_TERMINAL_WINDOW      = WM_USER + TERMINAL_WINDOW;
const uint WM_GOTO_COMMAND_WINDOW       = WM_USER + COMMAND_WINDOW;
const uint WM_GOTO_FILETRANSFER_WINDOW  = WM_USER + FILETRANSFER_WINDOW;
const uint WM_REQUEST_CLOSE_KERMIT      = WM_USER + 10;
const uint WM_CLOSE_KERMIT              = WM_USER + 11;

typedef struct _K_CREATEINFO {
    const char* classname;      // class name
    const char* text;           // caption or text
    long exStyle;               // extended window styles
    long style;                 // window styles
    int x;                      // x position
    int y;                      // y position
    int width;                  // window width
    int height;                 // window height
    long objId;                 // ID or hmenu
} K_CREATEINFO;

// registry information
const int numSettings = 9;
enum { Terminal_Left, Terminal_Top,
       Command_Left, Command_Top,
       CServer_Left, CServer_Top,
       Font_Facename, Font_Height, 
       Font_Width
};

// used by termMouseEffect in K_GLOBAL
#define TERM_MOUSE_NO_EFFECT        0   // no change (scrollbars active)
#define TERM_MOUSE_CHANGE_FONT      1   // dynamically size font
#define TERM_MOUSE_CHANGE_DIMENSION 2   // dynamically size terminal

class Kui;
class KHwndSet;
class KSysMetrics;
typedef struct _K_GLOBAL {
    HINSTANCE hInst;             // application instance
    KHwndSet* hwndset;           // KWin-HWND global pairings
    DWORD settings[numSettings]; // read from registry
    KSysMetrics* sysMets;        // system metrics
    int mouseEffect;             // effect mouse resizing has on terminal
    char hostName[256];          // current host connected
    char faceName[256];          // Facename
    DWORD fontWidth, fontHeight;
    int saveMouseEffect;        // for Maximize mode
    double saveLineSpacing;
    int noMenuBar;
    int noToolBar;
    int noStatus;
    int nCmdShow;
    int noClose;
} K_GLOBAL;

extern K_GLOBAL* kglob;
extern Kui* kui;

#endif
