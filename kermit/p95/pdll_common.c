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

/* Routines common for all protocols and both transfer directions */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_common.h"
#include "pdll_defs.h"
#include "pdll_dev.h"
#include "pdll_global.h"
#include "pdll_main.h"
#include "p_status.h"

VOID msg(U32, U8 *, ...);

VOID
#ifdef CK_ANSIC
cancel(void) 
#else
cancel()
#endif
{
    U32 i = 0;
    char caninstr[] = { 24,24,24,24,24,24,24,24,8,8,8,8,8,8,8,8,8,8,0 };

    dev_purge_outbuf();
    while (caninstr[i])
	dev_putch_buf(caninstr[i++]);
    dev_flush_outbuf();
}

VOID 
#ifdef CK_ANSIC
pdll_carrier_lost(void) 
#else 
pdll_carrier_lost()
#endif 
{
  if (p_cfg->status_func(PS_CARRIER_LOST, NULL))
    user_aborted();
  pdll_aborted = A_CARRIER_LOST;
  longjmp(p_jmp_buf, 1);
}

U32 user_aborted_visited = 0 ;

VOID
#ifdef CK_ANSIC
user_aborted(void) 
#else
user_aborted()
#endif
{
    if (user_aborted_visited)
	return;
    else
	user_aborted_visited = 1;

    if (dev_ready)
	cancel();
    /* if you set pdll_aborted before calling */
    /* cancel(), you won't transmit the cancellation */
    /* sequence. */
    pdll_aborted = A_LOCAL;

    /* Let's give remote a few seconds */
    /* to interpret our cancelation */
#ifdef XYZ_DLL
#ifdef NT
    Sleep(2000);
#else
    DosSleep(2000);
#endif
#else /* XYZ_DLL */
    sleep(2);
#endif /* XYZ_DLL */
    longjmp(p_jmp_buf, 1);
}

VOID
#ifdef CK_ANSIC
server_connect(void) 
#else 
server_connect()
#endif
{
  U32 cnt = 0;

  do {
    if (p_cfg->status_func(PS_SERVER_WAITING, STDATA( cnt++ )))
      user_aborted();
  } while (!dev_connect());
}
