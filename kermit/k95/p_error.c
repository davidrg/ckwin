/*
 * Copyright 1995 Jyrki Salmi, Online Solutions Oy (www.online.fi)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


/*
   Error routines used to report errors occurred in the P.DLL as well as
   in the P.EXE.
*/

#include "ckcdeb.h"
#ifndef NOXFER
#include "ckcker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef OS2
#ifdef NT
#include <windows.h>
#ifndef __GNUC__
#define APIRET DWORD
#else
#define APIRET unsigned __LONG32
#endif
#else
#define INCL_DOSMISC
#include <os2.h>
#undef COMMENT
#include "ckocon.h"
#endif /* OS2 */
#endif /* NT */

#include "p_type.h"
#include "p_errmsg.h"
#include "ckcnet.h"

/* Report an error occurred in the OS/2 API */

VOID
#ifdef CK_ANSIC
os2_error(U32 num,
               U32 ret_code,
               U32 module,
               U32 line_num,
               U8 *opt_arg)
#else
os2_error(num,ret_code,module,line_num,opt_arg)
     U32 num;
     U32 ret_code;
     U32 module;
     U32 line_num;
     U8 *opt_arg;
#endif
{

    UCHAR buf[4096];
    UCHAR buf2[4096];

#ifdef OS2
#ifdef NT
    sprintf(buf, "Win32 Error %lu:%lu@%lu:%lu: %s",
             num, ret_code,             /* First %lu:%lu */
             module, line_num,          /* Second %lu:%lu */
             os2_error_msg[num - 1]);
    /* Show the verbal description */
    sprintf(buf2, buf, opt_arg);

    if (ret_code != 0) {
        ckstrncat(buf2,", Win32 reports: ",4096);
        /* need to add format message call */
        if ( FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            ret_code,
                            MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                            buf,
                            4096,
                            (va_list *) NULL
                            ) )
        {
            ckstrncat(buf2,buf,4096);
        }
        else
        {
            ckstrncat(buf2,"Couldn't read error message.",4096);
        }
    }
#else /* not NT */
    APIRET rc=0;
    ULONG  msg_len=0;

    sprintf(buf, "OS/2 Error %lu:%lu@%lu:%lu: %s",
             num, ret_code,             /* First %lu:%lu */
             module, line_num,          /* Second %lu:%lu */
             os2_error_msg[num - 1]);
    /* Show the verbal description */
    sprintf(buf2, buf, opt_arg);
    if (ret_code != 0) {
        ckstrncat(buf2,", OS/2 reports: ",4096);
        rc = DosGetMessage(NULL,
                            0,
                            buf,
                            4096,
                            ret_code,
                            "OSO001.MSG",
                            &msg_len);
        if (rc) {
            ckstrncat(buf2,
            "Couldn't read error message from file OSO001.MSG which is a part of OS/2 and usually located in \\OS2\\SYSTEM directory.",
                       4096);
        } else {
            ckstrncat(buf2, buf,4096);
        }
    }

#endif /* NT */
    debug(F110,"p_error",buf2,0);
    ckscreen(SCR_ST,ST_ERR,0l,buf2);
#endif /* OS2 */
}

#ifdef XYZ_DLL
/* Report an error occurred in the TCP/IP API */

VOID tcpip_error(U32 num,
                 U32 ret_code,
                 U32 module,
                 U32 line_num,
                 U8 *opt_arg) {

  fprintf(stderr, "\rTCP/IP Error %lu:%lu@%lu:%lu: ",
          num, ret_code,                /* First %lu:%lu */
          module, line_num);            /* Second %lu:%lu */
  /* Show the verbal description */
  fprintf(stderr, tcpip_error_msg[num - 0x30], opt_arg);
  if (ret_code != 0) {
    fprintf(stderr, ", API reports: %d\n\n",ret_code);
  } else
    fprintf(stderr, "\n");

  /*exit(1);*/
}
#endif /* XYZ_DLL */
#endif /* NOXFER */
