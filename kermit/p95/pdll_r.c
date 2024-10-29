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

/* General routines for receiving with X, Y or Zmodem protocol */
#ifdef NT
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <errno.h>

#include "ckcdeb.h"

#include "p_type.h"
#include "pdll_common.h"
#include "pdll_defs.h"
#include "pdll_global.h"
#include "pdll_omalloc.h"
#include "pdll_x_global.h"
#include "pdll_z_global.h"
#include "pdll_modules.h"

U32 
#ifdef CK_ANSIC
get_header_field(U8 *buf, U32 *buf_idx, U32 buf_end, U32 *value_idx) 
#else
get_header_field() U8 *buf; U32 *buf_idx; U32 buf_end; U32 *value_idx; 
#endif
{
  (*buf_idx)++;			/* Skip the null from previous field */
  *value_idx = *buf_idx;
  while (1) {
    if (*buf_idx == buf_end) {
      if (*buf_idx != *value_idx) { /* Were there some numbers */
				    /* after the size? */
	if (p_cfg->status_func(PS_NON_STD_FILE_INFO, NULL))
	  user_aborted();
      }
      return(1);
    }
    if (!isdigit(buf[*buf_idx]))
      break;
    (*buf_idx)++;
  }
  return(0);
}

VOID
#ifdef CK_ANSIC
process_file_info(void) 
#else
process_file_info()
#endif
{
  U8 *buf;
  U32 buf_idx;
  U32 buf_end;
  U32 value_idx;

  date = -1;			/* These are important */
  length = -1;

  switch (protocol_type) {
  case PROTOCOL_X:
  case PROTOCOL_Y:
  case PROTOCOL_G:
    buf = &blk[3];
    buf_end = blk_size;
    break;

  case PROTOCOL_Z:
  default:			/* Just to shut up the compiler */
    buf = rx_buf;
    buf_end = rx_buf_len;
    break;
  }
  path = NULL;
  buf_idx = 0;
  if (buf[buf_idx] == '\0')
    return;
  while (1) {
    if (buf_idx == buf_end) {
      if (p_cfg->status_func(PS_INVALID_FILE_INFO, NULL))
	user_aborted();
      pdll_aborted = A_MISC;
      return;
    }
    if (buf[buf_idx] == '/')
      buf[buf_idx] = '\\';
    if (buf[buf_idx] == '\0')
      break;
    buf_idx++;
  }
  pdll_malloc((void **)&path, buf_idx + 1, MODULE_R, __LINE__);
  memcpy(path, buf, buf_idx + 1);

  /* File length */
  if (get_header_field(buf, &buf_idx, buf_end, &value_idx))
    return;
  sscanf(&buf[value_idx], "%lu", &length);

  /* File date */
  if (get_header_field(buf, &buf_idx, buf_end, &value_idx))
    return;
  sscanf(&buf[value_idx], "%lo", &date);

  /* File mode */
  if (get_header_field(buf, &buf_idx, buf_end, &value_idx))
    return;
  sscanf(&buf[value_idx], "%lo", &mode);

  /* Something dummy */
  if (get_header_field(buf, &buf_idx, buf_end, &value_idx))
    return;

  /* Files left */
  if (get_header_field(buf, &buf_idx, buf_end, &value_idx))
    return;
  sscanf(&buf[value_idx], "%ld", &pdll_files_left);
  
  /* Bytes left */
  if (get_header_field(buf, &buf_idx, buf_end, &value_idx))
    return;
  sscanf(&buf[value_idx], "%ld", &pdll_bytes_left);
}
