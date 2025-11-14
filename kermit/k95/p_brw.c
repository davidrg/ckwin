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
   Buffered file read and write routines. Not for generic use. Routines will
   break if same file is written and read without first closing and reopening
   it.
*/

#include "ckcdeb.h"
#ifndef NOXFER
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>

#ifdef OS2
#ifdef NT
#include <windows.h>
#define close _close
#define lseek _lseek
#define read  _read
#define sopen _sopen
#define tell  _tell
#define write _write
#else
#include <os2.h>
#undef COMMENT
#endif
#endif /* OS2 */

#include "p_type.h"
#include "p_brw.h"

/* Open the file. Returns a pointer to BRWF structure if successful. If file */
/* does not exist or some other error occurs a NULL value will be returned.  */

BRWF *
#ifdef CK_ANSIC
brw_open(U8 *path,
          U32 inbuf_size,
          U32 outbuf_size,
          S32 oflag,
          S32 shflag,
          S32 pmode)
#else
brw_open(path,inbuf_size,outbuf_size,oflag,shflag,pmode)
          U8 *path;
          U32 inbuf_size;
          U32 outbuf_size;
          S32 oflag;
          S32 shflag;
          S32 pmode;
#endif
{

  S32 fd;
  BRWF *brwf;

  if ((fd = sopen(path, oflag, shflag, pmode)) == -1)
    return(NULL);
  if ((brwf = (BRWF *) malloc(sizeof(BRWF))) == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");
    exit(1);
  }
  brwf->fd = fd;
  if (inbuf_size != 0) {
    if ((brwf->inbuf = (char *) malloc(inbuf_size)) == NULL) {
      fprintf(stderr, "Failed to allocate memory\n");
      exit(1);
    }
  } else
    brwf->inbuf = NULL;
  brwf->inbuf_size = inbuf_size;
  brwf->inbuf_idx = 0;
  brwf->inbuf_len = 0;
  if (outbuf_size != 0) {
    if ((brwf->outbuf = (char *) malloc(outbuf_size)) == NULL) {
      fprintf(stderr, "Failed to allocate memory\n");
      exit(1);
    }
  } else
    brwf->outbuf = NULL;
  brwf->outbuf_size = outbuf_size;
  brwf->outbuf_idx = 0;
  return(brwf);
}

/* Closes the file */

VOID
#ifdef CK_ANSIC
brw_close(BRWF **brwf)
#else
brw_close(brwf) BRWF ** brwf ;
#endif
{

  close((*brwf)->fd);
  if ((*brwf)->inbuf != NULL)
    free((*brwf)->inbuf);
  if ((*brwf)->outbuf != NULL)
    free((*brwf)->outbuf);
  *brwf = NULL;
}

/* Read buf_len bytes from the file and save them to buf. Returns the number */
/* of bytes read, or in case of an error, -1 */

S32
#ifdef CK_ANSIC
brw_read(BRWF *brwf, U8 *buf, U32 buf_len)
#else
brw_read(brwf,buf,buf_len) BRWF * brwf; U8 *buf; U32 buf_len ;
#endif
{

  static S32 rw_ret;
  static U32 buf_idx;

  if (brwf->inbuf == NULL)      /* If no buffering */
    return(read(brwf->fd, buf, buf_len));

  buf_idx = 0;
  while (1) {
    if (brwf->inbuf_len - brwf->inbuf_idx < buf_len - buf_idx) {
      /****************************************/
      /* Let's copy all that is in the buffer */
      /****************************************/
      memcpy(&buf[buf_idx],
             &brwf->inbuf[brwf->inbuf_idx],
             brwf->inbuf_len - brwf->inbuf_idx);
      buf_idx += brwf->inbuf_len - brwf->inbuf_idx;

      /****************************/
      /* Let's fill up the buffer */
      /****************************/
      rw_ret = read(brwf->fd, brwf->inbuf, brwf->inbuf_size);
      if (rw_ret == -1)
        return(-1);
      if (rw_ret == 0)
        return(buf_idx);
      brwf->inbuf_idx = 0;
      brwf->inbuf_len = rw_ret;
    } else {
      memcpy(&buf[buf_idx], &brwf->inbuf[brwf->inbuf_idx], buf_len - buf_idx);
      brwf->inbuf_idx += buf_len - buf_idx;
      buf_idx += buf_len - buf_idx;
      break;
    }
  }
  return(buf_idx);
}

/* Writes data in the write buffer to the disk. Important to call before */
/* closing the file being written to. Returns 0 when buffer written */
/* successfully, -1 when some sort of error occurred and 1 when all of the */
/* data was not written (disk full condition). */

S32
#ifdef CK_ANSIC
brw_flush(BRWF *brwf)
#else
brw_flush(brwf) BRWF * brwf ;
#endif
{

  S32 rw_ret;

  if (brwf->outbuf_idx > 0) {
    rw_ret = write(brwf->fd, brwf->outbuf, brwf->outbuf_idx);
    if (rw_ret != brwf->outbuf_idx) {
      if (rw_ret == -1)
        return(-1);
      else
        return(1);
    }
  }
  return(0);
}

/* Writes data to the file. Returns 0 when successful, -1 when some sort of */
/* error has occurred and 1 when only some of the data got successfully */
/* written (which is an implication of disk full condition). */

S32
#ifdef CK_ANSIC
brw_write(BRWF *brwf, U8 *buf, U32 buf_len)
#else
brw_write(brwf,buf,buf_len) BRWF *brwf; U8 *buf; U32 buf_len;
#endif
{

  static S32 rw_ret;
  static U32 buf_idx;

  if (brwf->outbuf == NULL) {   /* If no buffering */
    rw_ret = write(brwf->fd, buf, buf_len);
    if (rw_ret == buf_len)
      return(0);
    else if (rw_ret == -1)
     return(-1);
    else
     return(1);
  }

  buf_idx = 0;
  while (1) {
    if (brwf->outbuf_size - brwf->outbuf_idx < buf_len - buf_idx) {
        /* If data doesn't fit the buffer */
      memcpy(&brwf->outbuf[brwf->outbuf_idx],
             &buf[buf_idx],
             brwf->outbuf_size - brwf->outbuf_idx);
      buf_idx += brwf->outbuf_size - brwf->outbuf_idx;
      brwf->outbuf_idx += brwf->outbuf_size - brwf->outbuf_idx;

      rw_ret = write(brwf->fd, brwf->outbuf, brwf->outbuf_idx);
      if (rw_ret != brwf->outbuf_idx) {
          /* Something went wrong while writing */
        if (rw_ret == -1)
          return(-1);
        else
          return(1);
      }
      brwf->outbuf_idx = 0;
    } else {                    /* Data fits in the buffer */
      memcpy(&brwf->outbuf[brwf->outbuf_idx], &buf[buf_idx], buf_len - buf_idx);
      brwf->outbuf_idx += buf_len - buf_idx;
      buf_idx += buf_len - buf_idx;
      break;
    }
  }
  return(0);            /* Everything A-OK */
}

/* Change the current read or write position in the file. Returns -1 when */
/* some sort of error has occurred and 0 when seek was successful. */

S32
#ifdef CK_ANSIC
brw_seek(BRWF *brwf, U32 new_pos)
#else
brw_seek(brwf,new_pos) BRWF * brwf; U32 new_pos ;
#endif
{

  static U32 old_pos;

  if (brwf->inbuf == NULL) {    /* If no input buffering */
    if (lseek(brwf->fd, new_pos, SEEK_SET) == -1)
      return(-1);
    else
      return(0);
  }

  if ((old_pos = tell(brwf->fd)) == -1)
    return(-1);

  if (new_pos < old_pos) {      /* If we're moving backwards... */
    if (old_pos - brwf->inbuf_len <= new_pos) { /* If the new position is in the buffer */
      brwf->inbuf_idx = new_pos - (old_pos - brwf->inbuf_len);
    } else {                    /* New position is not in the buffer */
      if (lseek(brwf->fd, new_pos, SEEK_SET) == -1)
        return(-1);
      brwf->inbuf_idx = 0;
      brwf->inbuf_len = 0;
    }
  } else {                      /* Else we're moving backwards... */
    if (old_pos >= new_pos) {   /* If the new position is in the buffer */
      brwf->inbuf_idx += old_pos - new_pos;
    } else {                    /* New position is not in the buffer */
      if (lseek(brwf->fd, new_pos, SEEK_SET) == -1)
        return(-1);
      brwf->inbuf_idx = 0;
      brwf->inbuf_len = 0;
    }
  }
  return(0);                    /* Everything A-OK */
}
#endif /* NOXFER */
