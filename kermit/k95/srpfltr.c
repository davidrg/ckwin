#include <windows.h>
#include "t_pwd.h"

BOOLEAN WINAPI InitializeChangeNotify()
{       
    struct t_conf *tc;
    struct t_confent *tcent;

    tc = t_openconf(NULL);
    if ( tc == NULL )
        return FALSE;

    tcent = t_getconflast(tc);
    if ( tcent == NULL ) {
        t_closeconf(tc);
        return FALSE;
    }

    return TRUE;
}

BOOLEAN WINAPI PasswordFilter(PUNICODE_STRING AccountName,
                        PUNICODE_STRING FullName,
                        PUNICODE_STRING Password,
                        BOOLEAN SetOperation)
{
    return TRUE;
}


NTSTATUS WINAPI PasswordChangeNotify( PUNICODE_STRING UserName,
                                      ULONG RelativeId,
                                      PUNICODE_STRING NewPassword)
{
    struct t_conf *tc;
    struct t_confent *tcent;
    struct t_pw eps_passwd;
    FILE *passfp;



    tc = t_openconf(NULL);
    tcent = t_getconflast(tc);
    
    t_makepwent (&eps_passwd, name, pass, NULL, tcent);

    if ((passfp = fopen( (char *)t_defaultpwd(), "rb+")) == NULL) {
        int fh = creat ((char *)t_defaultpwd(), 0400);
        if ( fc == -1 )
            return STATUS_FAILURE;
        close(fh);
    } else
        fclose (passfp);

    /* change the password */
    t_changepw ((char *)t_defaultpwd(), &(eps_passwd.pebuf);

    t_closeconf(tc);
    return STATUS_SUCCESS;
}
 


