#include <windows.h>
#include <stdio.h>

int
main( int argc, char * argv[] )
{
    INPUT_RECORD k ;
    KEY_EVENT_RECORD * pKey=&k.Event.KeyEvent;
    DWORD count = 0;
    int rc=0, c, i ;
    HWND hWin = NULL;
    HANDLE hKbd = NULL; 
    char title[128];


    if ( argc >= 2 ) {
        sprintf(title,"%s - K-95",argv[1]);
    }
    else
        strcpy(title,"K-95");
    hWin = FindWindow(NULL, title);

    if ( !hWin ) {
        printf("Unable to find window handle\n");
        return(1);
    }

    printf("Window Handle = 0x%x\n",hWin);
        
    hKbd = GetStdHandle( STD_INPUT_HANDLE ) ;

    while ( 1 ) {
        if ( WAIT_OBJECT_0 == WaitForSingleObject(hKbd,-1) ) {
            rc = ReadConsoleInput( hKbd, &k, 1, &count ) ;
        }
        if ( count && 
             k.EventType == KEY_EVENT &&
             pKey->bKeyDown) {
            rc = PostMessage(hWin,
                         WM_KEYDOWN,
                         pKey->wVirtualKeyCode,
                         (pKey->dwControlKeyState & ENHANCED_KEY?1:0)<<24 |
                         pKey->wVirtualScanCode << 16 | 
                         1
                         );
            if ( !rc )
                printf("%c down error=%d\n",GetLastError());

              rc = PostMessage(hWin,
                         WM_KEYUP,
                         pKey->wVirtualKeyCode,
                         1 << 31 |
                         1 << 30 |
                         (pKey->dwControlKeyState & ENHANCED_KEY?1:0)<<24 |
                         pKey->wVirtualScanCode << 16 | 
                         1
                         );
            if ( !rc )
                printf("%c up error=%d\n",GetLastError());

        }
    }

    return(0);
}
