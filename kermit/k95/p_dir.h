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


#ifndef _DIR_H_
#define _DIR_H_

typedef struct _DIR_ENTRY {

  U8 *name;
  U32 *size;
  U16 *date;
  U16 *time;
  U32 *attribute;

#ifdef NT
  HANDLE handle ;
  LPWIN32_FIND_DATA find_buf ;
#else
  U32 handle;
  U8 *find_buf; /* FILEFINDBUF3 */
#endif
  U8  search_attr ;
} DIR_ENTRY;

#ifdef NT
#define DIR_FIND_DIRECTORY      1
#else
#define DIR_FIND_DIRECTORY      MUST_HAVE_DIRECTORY
#endif
_PROTOTYP( U32 dir_find_first,(U8 *, U32, DIR_ENTRY *));
_PROTOTYP( U32 dir_find_next,(DIR_ENTRY *));
_PROTOTYP( U32 dir_find_close,(DIR_ENTRY *));

#endif /* _DIR_H_ */
