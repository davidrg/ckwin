#define INCL_NOPM
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#include <os2.h>
#include <stdio.h>

int main( void )
{
    USHORT rc = 0 ;
    VIOMODEINFO mi;

    mi.cb = sizeof(mi) ;
    VioGetMode( &mi, 0 ) ;

    printf("\ncol = %d\n",mi.col);
    printf("row = %d\n",mi.row);
    printf("hres = %d\n",mi.hres);
    printf("vres = %d\n\n",mi.vres);

    return 0 ;
}
