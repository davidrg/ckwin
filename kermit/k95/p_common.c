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
   Some common utility functions
*/

#include "ckcdeb.h"
#ifndef NOXFER
#include "ckcker.h"

#include <stdio.h>
#ifdef OS2
#include <stdarg.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <io.h>
#include <direct.h>

#ifdef OS2
#ifdef NT
#include <windows.h>
#define DosBeep Beep
#define DosSleep Sleep
#define access _access
#else
#define INCL_KBD
#define INCL_DOSPROCESS
#include <os2.h>
#undef COMMENT
#endif
#include "ckocon.h"
#endif /* OS2 */

#include "p_type.h"
#include "p_common.h"
#include "p_error.h"
#include "p_module.h"
#include "p_global.h"

/* Returns time difference in verbal form. timer parameter is the difference */
/* in number of seconds. */

U8 *
#ifdef CK_ANSIC
d_time(U32 timer)
#else
d_time(timer) U32 timer ;
#endif
{

  static U8 ret[32];

  if (timer < 60)
    sprintf(ret, "%lu s", timer);
  else if (timer < 60 * 60)
    sprintf(ret, "%lu min %lu s", timer / 60, timer % 60);
  else if (timer < 60 * 60 * 24)
    sprintf(ret, "%lu h %lu min %lu s", timer / (60 * 60), (timer / 60) % 60, timer % 60);
  else
    sprintf(ret, "%lu d %lu h %lu min %lu s", timer / (60 * 60 * 24),
            timer / (60 * 60) % 24, (timer / 60) % 60, timer % 60);
  return(ret);
}


/* Pauses until a key is pressed. Call to this function is installed to the */
/* exit functions list if -pause option is specified. */

VOID
#ifdef CK_ANSIC
wait_for_keypress(void)
#else
wait_for_keypress()
#endif
{

#ifdef OS2
#ifndef NT
  APIRET rc=0;
  KBDKEYINFO kki;
#endif
#endif /* OS2 */

  printf("Press any key to continue...");
  fflush(stdout);
#ifdef OS2
#ifdef NT
   getchar() ;
#else
   rc = KbdCharIn(&kki, IO_WAIT, 0);
  if (rc)
    os2_error(P_ERROR_KBDCHARIN, rc,
              MODULE_COMMON, __LINE__,
              NULL);
#endif /* NT */
#else
    getchar();
#endif  /* OS2 */

  printf("\n");
}

/* Makes a beep if -quiet option is not specified. Installed to the exit */
/* functions list. */

VOID
#ifdef CK_ANSIC
make_noise(void)
#else
make_noise()
#endif
{
  if (!opt_quiet) {
    DosBeep(500, 100);
    DosSleep(250);
    DosBeep(1500, 50);
  }
}

/* Adds the receive directory specified with -directory option to the */
/* beginning of file name being received */

VOID
#ifdef CK_ANSIC
add_recv_dir_to_path(U8 **path)
#else
add_recv_dir_to_path(path) U8 **path;
#endif
{

  U8 new_path[4096];
  U32 i;
  U32 l;

  if (opt_directory == NULL || opt_paths)
    return;

  l = strlen(*path) + strlen(opt_directory) + 2; /* 2 is for possible */
                                                    /* '\' and the null */
  ckstrncpy(new_path, opt_directory, 4096);
  i = strlen(new_path);
  if (new_path[i - 1] != '\\')
    new_path[i++] = '\\';
  ckstrncpy(&new_path[i], *path, 4096-i);
  memcpy(*path, new_path, 4096);
}

/* Gets the length of directory portion of path */

U32
#ifdef CK_ANSIC
get_dir_len(U8 *path)
#else
get_dir_len(path) U8 *path;
#endif
{

  S32 i;

  i = strlen(path);
  while (--i >= 0) {
    if (path[i] == '\\' || path[i] == '/' || path[i] == ':')
      break;
  }
  return(i + 1);
}

/* Strips drive and directory information from a file path */

VOID
#ifdef CK_ANSIC
strip_drive_and_dir(U8 *path)
#else
strip_drive_and_dir(path) U8 *path ;
#endif
{

  U8 new_path[4096];

  ckstrncpy(new_path, &path[get_dir_len(path)], 4096);
  strcpy(path, new_path);
}

/* Creates a directory structure if it does not already exist */

U32
#ifdef CK_ANSIC
create_dirs(U8 *path)
#else
create_dirs(path) U8 * path ;
#endif
{

  U8 dir[4096];
  U32 path_idx = 0;

  while (path[path_idx] != '\0' && path_idx < 4096) {
    if (path[path_idx] == '\\' || path[path_idx] == '/') {
      if (path_idx == 0 || path[path_idx - 1] == ':') {
        memcpy(dir, path, path_idx + 1);
        dir[path_idx + 1] = '\0';
      } else {
        memcpy(dir, path, path_idx);
        dir[path_idx] = '\0';
      }
      if (access(dir, 0) != 0) {
        if (_mkdir(dir) == -1)
          return(1);
      }
    }
    path_idx++;
  }
  return(0);
}

/* Sets a priority of current process */

VOID
#ifdef CK_ANSIC
set_priority(U32 priority_class, U32 priority_delta)
#else
set_priority(priority_class,priority_delta)
     U32 priority_class; U32 priority_delta ;
#endif
{
#ifdef OS2
  APIRET rc=0;

#ifdef NT
   SetPriorityClass( GetCurrentProcess(), priority_class ) ;
   SetThreadPriority( GetCurrentThread(), priority_delta ) ;
#else
   rc = DosSetPriority(PRTYS_PROCESS,
                      priority_class,
                      priority_delta,
                      0);
#endif
  if (rc)
    os2_error(P_ERROR_DOSSETPRIORITY, rc,
              MODULE_COMMON, __LINE__,
              NULL);
#endif /* OS2 */
}

/* Handler for SIGINT signals. Sets the aborted variable to non-zero, it */
/* will be checked in status_func() and a proper return value will be */
/* returned to the DLL. */

VOID
#ifdef CK_ANSIC
interrupt_handler(int sig)
#else
interrupt_handler(sig) int sig ;
#endif
{

  if (aborted)
    ckscreen(SCR_EM,0,0l, "Please wait, transfer is being cancelled...");
  else {
    ckscreen(SCR_EM,0,0l, "Ctrl-C pressed, transfer is being cancelled...");
    aborted = 1;
  }
    signal(SIGINT,interrupt_handler);
}

/* Installs a handler for SIGINT signals */

VOID
#ifdef CK_ANSIC
install_interrupt_handler(U8 phandler)
#else
install_interrupt_handler(phandler) U8 phandler ;
#endif
{
   static void (*saved)(int) = NULL ;

   if ( phandler )
   {
      if ((saved = signal(SIGINT, interrupt_handler)) == SIG_ERR) { /* Install our own */
         /* handler for CTRL-C */
         fprintf(stderr, "Failed to install an interrupt handler\n");
         exit(1);
      }
   }
   else
   {
      if (signal(SIGINT, saved) == SIG_ERR) { /* Restore previous */
         /* handler for CTRL-C */
         fprintf(stderr, "Failed to install an interrupt handler\n");
         exit(1);
      }
   }
}

#endif /* NOXFER */
