#ifndef ikui_h_included
#define ikui_h_included
/*========================================================================
    Copyright (c) 1996, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: ikui.h
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: January 2, 1996

    Description: Interface between Kui and CKermit

========================================================================*/

/* defines used by KuiSetProperty */
#define KUI_TITLE           1000
#define KUI_FILE_TRANSFER   1001
#define KUI_TERM_TYPE       1002
#define KUI_TERM_FONT       1003
#define KUI_TERM_CMASK      1004
#define KUI_TERM_AUTODOWNLOAD 1005
#define KUI_TERM_CAPTURE    1006
#define KUI_TERM_PRINTERCOPY 1007
#define KUI_TERM_DEBUG      1008
#define KUI_TERM_PCTERM     1009
#define KUI_TERM_RESIZE     1010
#define KUI_TERM_URL_HIGHLIGHT 1011
#define KUI_TERM_REMCHARSET  1012
#define KUI_TERM_KEYCLICK   1013
#define KUI_LOCUS           1014
#define KUI_AUTO_LOCUS      1015
#define KUI_EXIT_WARNING    1016
#define KUI_GUI_DIALOGS     1017
#define KUI_GUI_TOOLBAR     1018
#define KUI_GUI_MENUBAR     1019
#define KUI_GUI_CLOSE       1020

struct _kui_init {
    char pos_init;		/* boolean if position set */
    unsigned long pos_x;
    unsigned long pos_y;
    char face_init;
    char * facename;
    char font_init;
    int  font_size;
    int  nomenubar;
    int  notoolbar;
    int  nostatusbar;
    int  nCmdShow;
    int  resizeMode;
    int  noclose;
};

void KuiSetProperty( int propid, long param1, long param2 );
void KuiGetProperty( int propid, long param1, long param2 );

int KuiInit( void* hInstance, struct _kui_init * );
HANDLE KuiThreadInit( void* hInstance );
int KuiThreadCleanup( void* hInstance );
#endif
void KuiSetTerminalStatusText(int, char *);
void KuiSetTerminalConnectButton(int);
void KuiSetTerminalPosition(int, int);
void KuiSetTerminalSize(int, int);
void KuiSetTerminalResizeMode(int);
int  KuiGetTerminalResizeMode(void);
void KuiSetTerminalRunMode(int);
#define STATUS_CMD_TERM   0
#define STATUS_LOCUS      1
#define STATUS_PROTO      2
#define STATUS_CURPOS     3
#define STATUS_HW         4

int KuiDownloadDialog(char *, char *, char *, int);