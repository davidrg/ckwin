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

#ifndef XYZ_DLL 
#include "ckcdeb.h"
#endif /* XYZ_DLL */

#include "pdll_os2incl.h"
#include "pdll_defs.h"
#include "pdll_error.h"
#include "pdll_common.h"
#include "pdll_global.h"
#include "pdll_tcpipapi.h"
#include "pdll_modules.h"
#ifdef __EMX__
#define _Inline extern inline
#endif

#pragma pack(1)

typedef struct _DEV_CFG {

  U16 write_timeout;
  U16 read_timeout;
  U8 flags1;
  U8 flags2;
  U8 flags3;
  U8 err_replacement_ch;
  U8 brk_replacement_ch;
  U8 xon_ch;
  U8 xoff_ch;
} DEV_CFG;

#pragma pack()

extern U32 dev_ready;
extern U8 *dev_path;
extern U32 dev_handle;
extern U32 passive_socket;
extern U8 *socket_remote;
extern U16 socket_port;
extern U8 *outbuf;
extern U32 outbuf_idx;
extern U8 *inbuf;
extern U32 outbuf_size;
extern U32 inbuf_size;
extern U32 timeouts_per_call;
extern U32 inbuf_idx;
extern U32 inbuf_len;

extern U32 dev_type;
extern U32 dev_opened;
extern U32 dev_shared;
extern U32 dev_server;
extern U32 dev_telnet;
extern U32 dev_telnet_u_binary ;
extern U32 dev_telnet_me_binary ;
extern U32 dev_telnet_iac_escaped ;

extern U32 watch_carrier;	/* From pdll_async.h */

extern void dev_open(void);
extern U32 dev_connect(void);
extern void dev_close(void);

enum {
  DEV_TIMEOUT =	256,
  DEV_TN_SUBNEGO_END,
  DEV_TN_NO_OPERATION,
  DEV_TN_DATA_MARK,
  DEV_TN_BREAK,
  DEV_TN_INTERRUPT,
  DEV_TN_ABORT,
  DEV_TN_ARE_YOU_THERE,
  DEV_TN_ERASE_CH,
  DEV_TN_ERASE_LINE,
  DEV_TN_GO_AHEAD,
  DEV_TN_SUBNEGO_START,
  DEV_TN_WILL,
  DEV_TN_WONT,
  DEV_TN_DO,
  DEV_TN_DONT,
  DEV_FIRST_FREE		/* First free code that can */
				/* be used for other purposes */
};

extern void dev_purge_inbuf(void);

extern void dev_purge_outbuf(void);
extern void dev_flush_outbuf(void);

extern void dev_get_cfg(DEV_CFG *);
extern void dev_set_cfg(DEV_CFG *);
extern void dev_chg_cfg(U16, U16, U8, U8, U8);

extern void dev_set_break_on(void);
extern void dev_set_break_off(void);

extern U32 dev_set_speed(U32);
extern void dev_dtr(U32);

enum {
  TN_SUBNEGO_END = 240,
  TN_NO_OPERATION,
  TN_DATA_MARK,
  TN_BREAK,
  TN_INTERRUPT,
  TN_ABORT,
  TN_ARE_YOU_THERE,
  TN_ERASE_CH,
  TN_ERASE_LINE,
  TN_GO_AHEAD,
  TN_SUBNEGO_START,
  TN_WILL,
  TN_WONT,
  TN_DO,
  TN_DONT,
  TN_IAC
};

#define TN_OPT_BINARY	0
#define TN_OPT_ECHO	1
#define TN_OPT_SUPP_GA	3

_PROTOTYP( VOID dev_send_tn_cmd, (U8, U8));

/* Here are the inline functions */
#ifdef XYZ_DLL
#include "pdll_async.h"
#ifndef NT
#include "pdll_pipe.h"
#endif
#include "pdll_socket.h"
#endif /* XYZ_DLL */
#include "pdll_exeio.h"

/* Inline functions */

_Inline U16 
#ifdef CK_ANSIC
dev_incoming(void) 
#else
dev_incoming()
#endif
{
    switch (dev_type) {
    case DEV_TYPE_EXE_IO:
	return(exe_incoming());

#ifdef XYZ_DLL
    case DEV_TYPE_ASYNC:
	return(async_incoming());

#ifndef NT
    case DEV_TYPE_PIPE:
	return(pipe_incoming());
#endif 

    case DEV_TYPE_SOCKET:
    default:
	return(tcp_incoming());
#endif /* XYZ_DLL */ 
  }
}

_Inline U16 
#ifdef CK_ANSIC
dev_pushback_buf(void) 
#else
dev_pushback_buf()
#endif
{
    switch (dev_type) {
    case DEV_TYPE_EXE_IO:
	exe_pushback_buf();
    }
    return(0);
}

_Inline U16 
#ifdef CK_ANSIC
dev_getch_buf(void) 
#else
dev_getch_buf()
#endif
{
    static U8 prev_ch;

    prev_ch = 0;
    while (1) {
	if (inbuf_idx == inbuf_len) {   /* If there's nothing in input buffer */
	    switch (dev_type) {
	    case DEV_TYPE_EXE_IO:
		exe_getch_buf();
		break;

#ifdef XYZ_DLL
	    case DEV_TYPE_ASYNC:
		async_getch_buf();
		break;
	
#ifndef NT
	    case DEV_TYPE_PIPE:
		pipe_getch_buf();
		break;
#endif
	
	    case DEV_TYPE_SOCKET:
	    default:
		tcp_getch_buf();
		break;
#endif /* XYZ_DLL */
	    }
	    if (inbuf_len == 0)		/* We didn't get a byte */
		return(DEV_TIMEOUT);
	}
	if (!dev_telnet)
	    break;

	if (prev_ch) {
	    switch (prev_ch) {
	    case TN_IAC:
		if (inbuf[inbuf_idx] == TN_IAC) {	/* IAC was escaped */
		    inbuf_idx++;
		    prev_ch = '\0' ;
		    return(TN_IAC);
		}
		prev_ch = '\0' ;
		return(inbuf[inbuf_idx++] + 17);

	    case '\r':
		if (inbuf[inbuf_idx] == '\0')
		    inbuf_idx++;		/* We won't return the NUL */
		prev_ch = '\0' ;
		return('\r');

	    default:			/* To shut up the compiler */
		break;
	    }
	}
	if ((dev_telnet_iac_escaped && inbuf[inbuf_idx] == TN_IAC) ||
	     (!dev_telnet_u_binary && inbuf[inbuf_idx] == '\r')) 
	{
	    prev_ch = inbuf[inbuf_idx++];
	} 
	else
	    break;
    }
    return(inbuf[inbuf_idx++]);
}

_Inline VOID
#ifdef CK_ANSIC
dev_putch_buf(U8 ch) 
#else
dev_putch_buf() U8 ch ;
#endif
{
    if (dev_telnet) {
	switch (ch) {
	case TN_IAC:
	    if (outbuf_idx == outbuf_size)
		dev_flush_outbuf();
	    outbuf[outbuf_idx++] = TN_IAC;
	    break;

	case '\r':
	    if ( !dev_telnet_me_binary )
	    {
		if (outbuf_idx == outbuf_size)
		    dev_flush_outbuf();
		outbuf[outbuf_idx++] = '\r';
		ch = '\0';		/* We'll send CR followed by NUL */
	    }
	    break;
	}
    }
    if (outbuf_idx == outbuf_size) {
	dev_flush_outbuf();
    }
    if (outbuf_idx < outbuf_size)
	outbuf[outbuf_idx++] = ch;
}

_Inline VOID
#ifdef CK_ANSIC
dev_putstr_buf(U8 *str) 
#else
dev_putstr_buf() U8 *str ;
#endif
{
    while (*str != '\0') {
	dev_putch_buf(*str);
	str++;
    }
}

_Inline U32 
#ifdef CK_ANSIC
dev_connected(void) 
#else
dev_connected()
#endif
{
    switch (dev_type) {
    case DEV_TYPE_EXE_IO:
	return(exe_connected());

#ifdef XYZ_DLL
    case DEV_TYPE_ASYNC:
	return(async_connected());

#ifndef NT
    case DEV_TYPE_PIPE:
	return(pipe_connected());
#endif

    case DEV_TYPE_SOCKET:
    default:
	return(tcp_connected());
#endif /* XYZ_DLL */
    }
}

