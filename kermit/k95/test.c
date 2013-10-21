#include <stdlib.h>
#include <stdio.h>

__declspec(dllimport) unsigned long _stdcall
validate_serial_no(char * serialno) ;

__declspec(dllexport) unsigned long _stdcall
init_printer_list(void);

__declspec(dllexport) long _stdcall
get_printer_display_string(int index, char * str, int len);

__declspec(dllexport) long _stdcall
get_printer_config_string(int index, char * str, int len);

__declspec(dllexport) unsigned long _stdcall
init_serial_port_list(void);

__declspec(dllexport) long _stdcall
get_serial_port_display_string(int index, char * str, int len);

__declspec(dllexport) long _stdcall
get_serial_port_config_string(int index, char * str, int len);

__declspec(dllexport) unsigned long _stdcall
init_tapi_list(void);

__declspec(dllexport) long _stdcall
get_tapi_display_string(int index, char * str, int len);

__declspec(dllexport) long _stdcall
get_tapi_config_string(int index, char * str, int len);

__declspec(dllexport) long _stdcall
register_k95(char * instdir, char * serial, char * name, char * corp, int license );

__declspec(dllexport) long _stdcall
get_reg_info(char * instdir, 
             char * serial, int serial_len,
             char * name, int name_len,
             char * corp, int corp_len );



int 
main(void)
{
    char str[256], s1[64], s2[64], s3[64];
    int  n;

    printf("Testing validate serial number ....\n");

    if ( validate_serial_no("K95-00010038454-1.1") )
        printf("valid serial number test passed\n");
    else
        printf("valid serial number test failed\n");

    if ( !validate_serial_no("K95-00010038450-1.1") )
        printf("invalid serial number test passed\n");
    else
        printf("invalid serial number test failed\n");

    printf("\nTesting Printers\n");
    n = init_printer_list();
    printf("%d printers\n");
    while ( n-- > 0 ) {
        get_printer_display_string(n,str,sizeof(str));
        printf("%s\n",str);
        get_printer_config_string(n,str,sizeof(str));
        printf("%s\n",str);
    }

    printf("\nTesting Serial Devices\n");
    n = init_serial_port_list();
    printf("%d serials\n");
    while ( n-- > 0 ) {
        get_serial_port_display_string(n,str,sizeof(str));
        printf("%s\n",str);
        get_serial_port_config_string(n,str,sizeof(str));
        printf("%s\n",str);
    }

    printf("\nTesting TAPI devices\n");
    n = init_tapi_list();
    printf("%d tapi devices\n");
    while ( n-- > 0 ) {
        get_tapi_display_string(n,str,sizeof(str));
        printf("%s\n",str);
        get_tapi_config_string(n,str,sizeof(str));
        printf("%s\n",str);
    }

    n = register_k95("d:/kermit","K95-00010038454-1.1","Deborah Lerman","Girlfriend",5);
    printf("register test = %d\n",n);

    n = get_reg_info("d:/kermit",s1,sizeof(s1),s2,sizeof(s2),s3,sizeof(s3));
    printf("get reg info = %d\n",n);
    if ( !n ) {
        printf("\n%s\n%s\n%s\n",s1,s2,s3);
    }
    return(0);
}
