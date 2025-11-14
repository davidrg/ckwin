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
   A bit cleaner directory handling routines than the OS/2's DosFindFirst and
   DosFindNext. Note that dir_find_close() should be called when done with
   the directory listing.
*/

#include "ckcdeb.h"
#ifndef NOXFER
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef OS2
#ifdef NT
#include <windows.h>
#ifndef __GNUC__
#define APIRET DWORD
#else
#define APIRET unsigned __LONG32
#endif /* __GNUC__ */
#else
#define INCL_DOSFILEMGR
#include <os2.h>
#undef COMMENT
#endif
#include "ckocon.h"
#endif /* OS2 */

#include "p_type.h"
#include "p_dir.h"


/* Finds a first file */
/* returns 0 if successful */
U32
#ifdef CK_ANSIC
dir_find_first(U8 *mask,
               U32 search_attribute,
               DIR_ENTRY *dir_entry)
#else
dir_find_first(mask,search_attribute,dir_entry)
    U8 *mask;
    U32 search_attribute;
    DIR_ENTRY *dir_entry;
#endif
{
#ifndef NT
    U32 find_count = 1;
#endif /* Not NT */
    APIRET rc = 0;

#ifdef OS2
#ifdef NT
    FILETIME * writetime;
    static WORD date, time;
    if ((dir_entry->find_buf = malloc(sizeof(WIN32_FIND_DATA))) == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    dir_entry->search_attr = search_attribute ;
    if ( !(dir_entry->handle = FindFirstFile( mask, dir_entry->find_buf )) )
      rc = GetLastError();
    else {
        dir_entry->name = ((WIN32_FIND_DATA *)dir_entry->find_buf)->cFileName;
        dir_entry->size =
          (U32 *)&((WIN32_FIND_DATA *)dir_entry->find_buf)->nFileSizeLow;
        writetime =
          &(((WIN32_FIND_DATA *)dir_entry->find_buf)->ftLastWriteTime);
        FileTimeToDosDateTime( writetime, &date, &time);
        dir_entry->date = (U16 *)&date;
        dir_entry->time = (U16 *)&time;
        dir_entry->attribute =
          &(((WIN32_FIND_DATA *)dir_entry->find_buf)->dwFileAttributes) ;
        while (rc == 0 &&
               dir_entry->search_attr  == DIR_FIND_DIRECTORY &&
               !(*dir_entry->attribute & FILE_ATTRIBUTE_DIRECTORY)
               ) {
            rc = dir_find_next( dir_entry ) ;
        }
    }
#else /* NT */
    if ((dir_entry->find_buf = malloc(sizeof(FILEFINDBUF3))) == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    dir_entry->handle = HDIR_CREATE; /* HDIR_SYSTEM; */
    rc = DosFindFirst(mask,
                      &dir_entry->handle,
                      search_attribute,
                      dir_entry->find_buf,
                      sizeof(FILEFINDBUF3),
                      &find_count,
                      FIL_STANDARD);
    dir_entry->name = ((FILEFINDBUF3 *)dir_entry->find_buf)->achName;
    dir_entry->size = (U32 *)&((FILEFINDBUF3 *)dir_entry->find_buf)->cbFile;
    dir_entry->date =
      (U16 *)&((FILEFINDBUF3 *)dir_entry->find_buf)->fdateLastWrite;
    dir_entry->time =
      (U16 *)&((FILEFINDBUF3 *)dir_entry->find_buf)->ftimeLastWrite;
    dir_entry->attribute =
      &((FILEFINDBUF3 *)dir_entry->find_buf)->attrFile;
#endif /* NT */
#else /* OS2 */
    THIS_HAS_TO_BE_FILLED_IN_FOR_UNIX;
    /* In Kermit we call zxpand(name) for this */
    /* But then what? */
#endif /* OS2 */
    return(rc);
}

/* Finds the next file */

U32
#ifdef CK_ANSIC
dir_find_next(DIR_ENTRY *dir_entry)
#else
dir_find_next(dir_entry) DIR_ENTRY * dir_entry ;
#endif
{
#ifndef NT
    U32 find_count = 1;
#endif /* not NT */
    APIRET rc=0;

#ifdef OS2
#ifdef NT
    FILETIME * writetime;
    static WORD date, time;
    if ( !FindFirstFile( dir_entry->handle, dir_entry->find_buf ) )
      rc = GetLastError();
    else {
        dir_entry->name = ((WIN32_FIND_DATA *)dir_entry->find_buf)->cFileName;
        dir_entry->size =
          (U32 *)&((WIN32_FIND_DATA *)dir_entry->find_buf)->nFileSizeLow;

        writetime = &((WIN32_FIND_DATA *)dir_entry->find_buf)->ftLastWriteTime;
        FileTimeToDosDateTime( writetime, &date, &time ) ;
        dir_entry->date = (U16 *)&date ;
        dir_entry->time = (U16 *)&time ;
        dir_entry->attribute =
          &((WIN32_FIND_DATA *)dir_entry->find_buf)->dwFileAttributes ;

        while (rc == 0 &&
               dir_entry->search_attr  == DIR_FIND_DIRECTORY &&
               !(*dir_entry->attribute & FILE_ATTRIBUTE_DIRECTORY)
               ) {
            rc = dir_find_next( dir_entry ) ;
        }
    }
#else /* NT */
    rc = DosFindNext(dir_entry->handle,
                     dir_entry->find_buf,
                     sizeof(FILEFINDBUF3),
                     &find_count);
    dir_entry->name = ((FILEFINDBUF3 *)dir_entry->find_buf)->achName;
    dir_entry->size = (U32 *)&((FILEFINDBUF3 *)dir_entry->find_buf)->cbFile;
    dir_entry->date =
      (U16 *)&((FILEFINDBUF3 *)dir_entry->find_buf)->fdateLastWrite;
    dir_entry->time =
      (U16 *)&((FILEFINDBUF3 *)dir_entry->find_buf)->ftimeLastWrite;
    dir_entry->attribute = &((FILEFINDBUF3 *)dir_entry->find_buf)->attrFile;
#endif /* NT */
#else /* OS2 */
    THIS_HAS_TO_BE_FILLED_IN_FOR_UNIX;
    /* In Kermit we call znext(name), but then what? */
#endif /* OS2 */
    return(rc);
}

/* Ends the directory handling started by dir_find_first() */

U32
#ifdef CK_ANSIC
dir_find_close(DIR_ENTRY *dir_entry)
#else
dir_find_close(dir_entry) DIR_ENTRY * dir_entry ;
#endif
{
    APIRET rc=0;
#ifdef OS2
#ifdef NT
    if ( !FindClose( dir_entry->handle) )
      rc = GetLastError() ;
#else
    rc = DosFindClose(dir_entry->handle);
#endif
#else /* OS2 */
    THIS_HAS_TO_BE_FILLED_IN_FOR_UNIX;
    /* Not needed in Kermit */
#endif /* OS2 */
    return(rc);
}
#endif /* NOXFER */
