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
 
#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_common.h"
#include "p.h"
#include "pdll_global.h"
#include "pdll_dev.h"
#include "pdll_modules.h"

extern U8 *dev_path;
extern U32 outbuf_size;
extern U32 inbuf_size;
extern U32 timeouts_per_call;
extern U8 *outbuf;
extern U32 outbuf_idx;
extern U8 *inbuf;
extern U32 inbuf_idx;
extern U32 inbuf_len;

VOID
#ifdef CK_ANSIC
exe_pushback_buf(void)
#else
exe_pushback_buf()
#endif
{
    if ( inbuf_idx != inbuf_len )
        p_cfg->exe_pushback_func( &inbuf[inbuf_idx], 
                                  inbuf_len - inbuf_idx );
    inbuf_len = inbuf_idx = 0;
}

VOID 
#ifdef CK_ANSIC
exe_getch_buf(void) 
#else
exe_getch_buf()
#endif
{

    APIRET rc=0;
    U32 tmout_cnt;

    inbuf_idx = 0;
    inbuf_len = 0;
    for (tmout_cnt = 0; 
          !pdll_aborted && tmout_cnt < timeouts_per_call; 
          tmout_cnt++) 
    {
        rc = p_cfg->exe_in_func( inbuf,	/* BufferArea */
                                 inbuf_size,	/* BufferLength */
                                 &inbuf_len);	/* BytesWritten */
        if (rc && rc != ERROR_NO_DATA) 
        {
            p_error(P_ERROR_DOSREAD, rc, 
                     MODULE_EXE, __LINE__, (intptr_t)"exe_io");
            pdll_aborted = A_CARRIER_LOST;
            return;
        }
        if (inbuf_len)		/* Got something */
            break;
        else
            DosSleep(30);
    }
}

VOID 
#ifdef CK_ANSIC
exe_flush_outbuf(void) 
#else
exe_flush_outbuf()
#endif
{
    static APIRET rc=0;
    static U8 *buf;
    static U32 BytesWritten;
    static U32 cnt;

    buf = outbuf;
    for (cnt = 0; !pdll_aborted && cnt < 300; cnt++) {
	rc = p_cfg->exe_out_func( buf,			/* BufferArea */
				  outbuf_idx,		/* BufferLength */
				  &BytesWritten); /* BytesWritten */
	if (rc)
	    p_error(P_ERROR_DOSWRITE, rc,
		     MODULE_EXE, __LINE__,
		     (intptr_t)"exe_io");
	if (BytesWritten == outbuf_idx) {
	    outbuf_idx = 0;
	    return;
	} else {
	    DosSleep(10);
	    if (BytesWritten) {	/* We got something transferred? */
		buf += BytesWritten;
		outbuf_idx -= BytesWritten;
	    }
	}
    }
    p_error(P_ERROR_DOSWRITE, rc, MODULE_EXE, __LINE__, (intptr_t)"exe_io");
}


VOID 
#ifdef CK_ANSIC
exe_set_break_on(void) 
#else
exe_set_break_on()
#endif
{
   p_cfg->exe_break_func(1) ;
}

VOID 
#ifdef CK_ANSIC
exe_set_break_off(void) 
#else
exe_set_break_off()
#endif
{
   p_cfg->exe_break_func(0);
}
