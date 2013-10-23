#ifndef ikcmd_h_included
#define ikcmd_h_included

#ifdef __cplusplus
extern "C" {
#endif 

#include "kdefs.h"  // kermit variable definitions

int putCmdString( char* );
void putKeyboardBuffer( BYTE, char* );
char* getVar( unsigned int idx );
BOOL isConnected(void);
BOOL DialerExists();
void StartDialer(void);
BOOL isScriptRunning(void);
void setHwndKUI( HWND hwnd );
HWND getHwndKUI( void );
#ifdef __cplusplus
}
#endif 

#endif
