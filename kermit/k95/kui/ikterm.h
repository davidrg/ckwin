#ifndef ikterm_h_included
#define ikterm_h_included

#include "ikextern.h"

typedef struct _K_CLIENT_PAINT {
    unsigned short*  textBuffer;
    unsigned char*  attrBuffer;
    unsigned short* effectBuffer;
    unsigned short* lineAttr;
    int             len;
    BOOL            cursorVisible;
    POINT           cursorPt;

    unsigned long   beg;            // for vertical scrollbar
    unsigned long   top;
    unsigned long   scrolltop;
    unsigned long   end; 
    unsigned long   maxWidth;
} K_CLIENT_PAINT;

struct videobuffer_struct;
struct videoline_struct;
class IKTerm 
{
public:
    IKTerm( BYTE whichbuffer, K_CLIENT_PAINT* );
    ~IKTerm();
    BOOL getDrawInfo();
    BOOL newKeyboardEvent( UINT chCharCode, LONG lKeyData, UINT keyDown, UINT sysKey );
    BOOL keyboardEvent( UINT chCharCode, LONG lKeyData, UINT keyDown );
    BOOL virtkeyEvent( UINT virtkey, LONG lKeyData, UINT keyDown );
    void mouseEvent( HWND hwnd, UINT msg, WPARAM wParam, int x, int y );
    BOOL getCursorPos();

protected:
    DWORD getControlKeyState();

private:
    BYTE vnum;
    K_CLIENT_PAINT* kcp;
    INPUT_RECORD inpEvt;

    int x, y;
    int xs, ys, c, xo, yo, xho;
    videobuffer_struct* vbuf;
    videoline_struct* line;
    int cursor_offset;
    unsigned short *textBuffer;
    unsigned char  *attrBuffer;
    unsigned short *effectBuffer, *lineAttr;
    int incnt;
    unsigned short vt_char_attrs;
    int mouseCaptured;
};

long getMaxDim();
void getMaxSizes( int* column, int* row );
void getDimensions( BYTE vnum, int* w, int* h );

#endif
