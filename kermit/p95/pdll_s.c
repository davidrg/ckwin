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

/* General routines for sending with X, Y or Zmodem protocol */

#ifdef NT
#include <windows.h>
#endif

#include <stdio.h>

#include "ckcdeb.h"

#include "p_type.h"
#include "pdll_global.h"

U32 
#ifdef CK_ANSIC
s_make_file_header(U8 *buf) 
#else
s_make_file_header() U8 *buf ;
#endif
{

  U32 buf_idx;

  buf_idx = 0;
  while (path[buf_idx] != '\0') {
    if (path[buf_idx] == '\\')
      buf[buf_idx] = '/';
    else
      buf[buf_idx] = path[buf_idx];
    buf_idx++;
  }
  buf[buf_idx++] = '\0'; /* include the null in the result */
  if (length == -1) {
    buf[buf_idx] = '\0';
  } else {
    if (date == -1)
      time(&date);
    buf_idx += sprintf((U8 *)&buf[buf_idx], "%lu %lo %lo 0 %ld %ld",
		       length, date, mode, pdll_files_left, pdll_bytes_left);
  }
  buf_idx++; /* include the null in the result */
  return(buf_idx);
}

