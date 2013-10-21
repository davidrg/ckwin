#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

static char ** serial_list = NULL;
static int     serial_cnt  = 0;

__declspec(dllexport) DWORD _stdcall
init_serial_port_list(void)
{
    HKEY  hk=0;
    DWORD dwType=0;
    DWORD dwValueSize=0, dwDataSize=0;
    CHAR *lpszValueName=NULL;
    char  serialdevice[64]="";
    char  dosname[12]="";
    int i;
    DWORD rc;

    if ( serial_list )
    {
        for ( i=0 ; i < serial_cnt ; i++ )
            free( serial_list[i] ) ;
        free ( serial_list )  ;
    }
    serial_list = NULL;
    serial_cnt = 0;

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0,
                        KEY_READ, &hk) )
        goto init_exit;

    for ( ;; ) {
        dwValueSize = sizeof(serialdevice);
        dwDataSize = sizeof(dosname);
        rc = RegEnumValue(hk, serial_cnt, serialdevice, &dwValueSize, 0,
                           &dwType, dosname, &dwDataSize);
        if ( rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA )
            serial_cnt++;
        else 
            break;
    }
    if (serial_cnt == 0)
        goto init_exit;

    serial_list = (char **) malloc( serial_cnt * sizeof(char *) );
    if ( !serial_list ) {
        serial_cnt = 0;
        goto init_exit;
    }

    for ( i=0;i<serial_cnt;i++ ) {
        dwValueSize = sizeof(serialdevice);
        dwDataSize = sizeof(dosname);
        rc = RegEnumValue(hk, i, serialdevice, &dwValueSize, 0,
                           &dwType, dosname, &dwDataSize);
        if ( rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA ) {
            serial_list[i] = strdup(dosname);
        }
    }

  init_exit:
    RegCloseKey( hk );
    return(serial_cnt);
}

__declspec(dllexport) DWORD _stdcall
get_serial_port_display_string(int index, char * str, int len)
{
    if ( index < 0 || index >= serial_cnt || !serial_list )
        return(0);

    if ( len < strlen(serial_list[index]) + 1 )
        return(-(long)strlen(serial_list[index]));

    strncpy(str,serial_list[index],len);
    str[len-1] = '\0';
    return(1);
}


int 
main(void)
{
    char str[256], s1[64], s2[64], s3[64];
    int  n;

    n = init_serial_port_list();
    printf("%d serial ports\n");
    while ( n-- > 0 ) {
        get_serial_port_display_string(n,str,sizeof(str));
        printf("%s\n",str);
    }

    return(0);
}

