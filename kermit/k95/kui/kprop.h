#ifndef kprop_h_included
#define kprop_h_included

#include "kuidef.h"

class KCmdProc;
typedef struct _cmdCallback {
    KCmdProc* cmdproc;
    HWND hwndCmd;
    PROPSHEETPAGE* propsheetpage;
} cmdCallback;

typedef struct _propPageAssoc {
    int command;
    int dlgid;
    char* pagename;
} propPageAssoc;

const int numPropPages = 8;
const char kpropTitle[] = "Kermit95 Properties:  ";

#endif
