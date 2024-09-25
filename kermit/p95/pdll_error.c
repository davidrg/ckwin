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

/* Error routine */

#ifdef NT
#include <windows.h>
#endif

#include "ckcdeb.h"

#include <stdio.h>
#include <setjmp.h>
#include "p_type.h"
#include "pdll_global.h"
#include "pdll_main.h"
#include "pdll_common.h"
#include "p_status.h"

U32 p_error_visited;

VOID
#ifdef CK_ANSIC
p_error(U32 num,
         U32 error_code,
         U32 module,
         U32 line,
         U32 opt_arg) 
#else 
p_error() U32 num;
     U32 error_code;
     U32 module;
     U32 line;
     U32 opt_arg);
#endif
{
  status_args   status;

  if (p_error_visited)		/* Yeah right, we are already closing down.. */
				/* We'll just ignore all errors after the */
				/* first one... */
    return;

  p_error_visited = 1;

  status.arg0 = num;
  status.arg1 = error_code;
  status.arg2 = module;
  status.arg3 = line;
  status.arg4 = opt_arg;
  if (p_cfg->status_func(PS_ERROR, STDATA(&status)))
    user_aborted();
  longjmp(p_jmp_buf, 1);
}
