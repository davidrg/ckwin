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
   Routines used to create a linked list of file names specified on the
   command-line and in the listfiles.
*/

#include "ckcdeb.h"
#ifndef NOXFER
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef CKOKER_H
/* ckoker.h defines kfree, kmalloc and krealloc */
#include <malloc.h>
#endif /* CKOKER_H */
#include <string.h>

#ifdef OS2
#ifdef NT
#include <windows.h>
#define stricmp _stricmp
#else
#define INCL_DOSFILEMGR
#include <os2.h>
#undef COMMENT
#endif
#include "ckocon.h"
#endif /* OS2 */

#include "p_type.h"
#include "p_common.h"
#include "p_dir.h"
#include "p_tl.h"
#include "p_global.h"

/* Adds a file of PATH to the list TL. SIZE is the file size in bytes. */

VOID
#ifdef CK_ANSIC
tl_add(TL **tl, U8 *path, U32 size, U8* as_name, U8 convert )
#else
tl_add(tl,path,size,as_name,convert)
     TL **tl; U8 *path; U32 size; U8* as_name; U8 convert;
#endif
{

    U32 buf_idx;
    TE *te;
    char tmpbuf[257] ;

    debug(F110,"tl_add path",path,0);
    debug(F101,"tl_add size","",size);
    debug(F110,"tl_add as_name",as_name,0);
    debug(F101,"tl_add convert","",convert);

    if ((te = malloc(sizeof(TE))) == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }

    te->path_len = strlen(path);
    if ((te->path = malloc(te->path_len + 1)) == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    memcpy(te->path, path, te->path_len + 1);

    buf_idx = te->path_len + 1;
    while (buf_idx > 0) {
        buf_idx--;
        if (te->path[buf_idx] == '/'
#ifdef OS2
             || te->path[buf_idx] == '\\'
#endif /* OS2 */
             ) {
            buf_idx++;
            break;
        }
    }
    te->name_len = te->path_len - buf_idx;
    te->name = &te->path[buf_idx];

    if ( as_name )
    {
        te->as_name_len = strlen(as_name);
        if ((te->as_name = malloc(te->as_name_len + 1)) == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
        memcpy(te->as_name, as_name, te->as_name_len + 1);
    }
    else
    {
        te->as_name = NULL ;
        te->as_name_len = 0 ;
    }
    te->convert = convert ;

    if (*tl == NULL) {
        /* The list is empty */
        if ((*tl = malloc(sizeof(TL))) == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
        (*tl)->f = te;
        (*tl)->l = te;
        (*tl)->c = te;
        (*tl)->cnt = 0;
        (*tl)->size = 0;
        te->p = NULL;
    } else {
        (*tl)->l->n = te;
        te->p = (*tl)->l;
    }
    (*tl)->cnt++;
    (*tl)->size += size;
    (*tl)->l = te;
    te->n = NULL;
}

/* Add files of PATH to the transfer list. Possible wildcards in PATH will */
/* be expanded. If -recursive option is specified, files will be searched */
/* recursively. */

VOID
#ifdef CK_ANSIC
tl_expanded_add(TL **tl, U8 *path)
#else
tl_expanded_add(tl,path) TL **tl; U8 *path ;
#endif
{

  U8 expanded_path[260];
  BOOLEAN found = 0;
  DIR_ENTRY dir_entry;

  /* Search for files */
  if (dir_find_first(path, 0, &dir_entry) == 0) {
    found = 1;
    do {
      sprintf(expanded_path, "%.*s%s",
              (int)get_dir_len(path), path,
              dir_entry.name);
      tl_add(tl, expanded_path, *dir_entry.size, NULL, opt_text);
    } while (!dir_find_next(&dir_entry));
  }
  dir_find_close(&dir_entry);

  /* Search for directories */
  if (opt_recursive) {
    if (dir_find_first(path, DIR_FIND_DIRECTORY, &dir_entry) == 0) {
      found = 1;
      do {
        if (strcmp(dir_entry.name, ".") != 0 &&
            strcmp(dir_entry.name, "..") != 0) {
          sprintf(expanded_path, "%.*s%s\\*",
                  (int)get_dir_len(path), path,
                  dir_entry.name);
          tl_expanded_add(tl, expanded_path);
        }
      } while (!dir_find_next(&dir_entry));
    }
    dir_find_close(&dir_entry);
  }
  if (!found)
    tl_add(tl, path, 0, NULL, opt_text);        /* If we are sending, we'll */
                                /* notify about non-existing */
                                /* files later... */
}

/* Reads the files from a listfile */

VOID
#ifdef CK_ANSIC
tl_read_from_list(TL **tl, U32 expand, U8 *path)
#else
tl_read_from_list(tl,expand,path) TL **tl; U32 expand; U8 *path ;
#endif
{

  U8 str[512];
  U32 str_len;
  FILE *stream;

  path++; /* skip the @-character */
  if ((stream = fopen(path, "r")) == NULL) {
    perror(path);
    return;
  }
  while (fgets(str, 511, stream) != NULL) {
    if (str[0] != '\0') {
      str_len = strlen(str);
      str_len--;
      if (str[str_len] == '\n')
        str[str_len] = '\0';
      if (expand)
        tl_expanded_add(tl, str);
      else
        tl_add(tl, str, 0, NULL, opt_text);
    }
  }
  fclose(stream);
}

/* Checks if file of PATH exists in the transfer list. This is used when */
/* receiving to check that the file was specified on the command-line */

BOOLEAN
#ifdef CK_ANSIC
tl_exists(TL *tl, U8 *path)
#else
tl_exists(tl,path) TL *tl; U8 *path;
#endif
{

  TE *te;

  te = tl->f;
  while (te != NULL) {
    if (stricmp(path, te->path) == 0)
      return(1);
    te = te->n;
  }
  return(0);
}

/* Frees the memory reserved by the transfer list */

VOID
#ifdef CK_ANSIC
tl_free(TL **tl)
#else
tl_free(tl) TL **tl;
#endif
{

  TE *te1;
  TE *te2;

  if (*tl != NULL) {
    te1 = (*tl)->f;
    while (te1 != NULL) {
      te2 = te1->n;
      free(te1->path);
      free(te1);
      te1 = te2;
    }
    free(*tl);
    *tl = NULL;
  }
}

#endif /* NOXFER */