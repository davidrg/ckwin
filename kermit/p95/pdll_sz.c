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

/* Routines for sending with Zmodem protocol */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_common.h"
#include "pdll_defs.h"
#include "pdll_dev.h"
#include "pdll_global.h"
#include "pdll_omalloc.h"
#include "pdll_modules.h"
#include "pdll_s.h"
#include "pdll_z.h"
#include "pdll_z_global.h"
#include "p_status.h"

VOID 
#ifdef CK_ANSIC
sz_parse_zrinit(void) 
#else
sz_parse_zrinit()
#endif
{
    U32 recv_blk_size;

    rx_flags = rx_hdr[ZF0];
    recv_blk_size = ((U16)rx_hdr[ZP1] << 8) | rx_hdr[ZP0];
    if (blk_size == 0 ||          /* Not defined on the command-line    */
         blk_size > recv_blk_size)/* or the local blk size is too large */
	blk_size = recv_blk_size;

    if (!use_alternative_checking)
	tx_bin32 = rx_flags & CANFC32;

    if (rx_flags & TESCCTL)
	esc_control = 1;
    if (rx_flags & TESC8)
	esc_8th_bit = 1;

    if (p_cfg->status_func(PS_Z_RECEIVER_FLAGS, STDATA(rx_flags)))
	user_aborted();
    if (p_cfg->status_func(PS_Z_RECEIVER_WINDOW_SIZE, STDATA(recv_blk_size)))
	user_aborted();
    if (tx_bin32) {
	if (p_cfg->status_func(PS_CHECKING_METHOD, STDATA(CHECKING_CRC32)))
	    user_aborted();
    } else {
	if (p_cfg->status_func(PS_CHECKING_METHOD, STDATA(CHECKING_CRC16)))
	    user_aborted();
    }
}

VOID 
#ifdef CK_ANSIC
sz_file_header(void) 
#else
sz_file_header()
#endif
{
    U32 c;
    U32 resend_header = 1;
    time_t t_started;
    time_t t_now;
    status_args status;

    time(&t_started);
    tx_buf_len = s_make_file_header(tx_buf);
    while (1) {
	time(&t_now);
	if (t_now - t_started >= 60) {
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
		user_aborted();
	    pdll_aborted = A_MISC;
	    return;
	}
	if (resend_header) {
	    tx_hdr[ZF0] = local_zconv;
	    tx_hdr[ZF1] = local_zmanag;
	    tx_hdr[ZF2] = local_ztrans;
	    tx_hdr[ZF3] = 0;
      
	    tx_hdr_type = ZFILE;
	    tx_frame_end = ZCRCW;
	    z_send_block();
	    resend_header = 0;
	}
	c = z_get_header();
	status.arg0 = outheader(c);
	status.arg1 = *(U32 *)rx_hdr;
	if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
	    user_aborted();
	switch (c) {
	case ZCAN:
	    pdll_aborted = A_REMOTE;
	    return;

	case ZABORT:		/* Abort gracefully, doing ZFINs */
	    got_zabort = 1;
	    return;

	case ZRINIT:		/* These tend to happen - dunno why */
	    sz_parse_zrinit();
	    break;

	case DEV_TIMEOUT:
	    resend_header = 1;
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
		user_aborted();
	    break;

	case GOTERROR:
	    resend_header = 1;
	    break;

	case ZFIN:
	    return;

	case ZFERR:
	    if (p_cfg->status_func(PS_FILE_SKIPPED, NULL))
		user_aborted();
	    skip_file = 1;
	    return;

	case ZSKIP:
	    if (p_cfg->status_func(PS_FILE_SKIPPED, NULL))
		user_aborted();
	    skip_file = 1;
	    return;

	case ZRPOS:
	    offset = *(U32 *)rx_hdr;
	    if (offset != 0) {
		if (p_cfg->status_func(PS_Z_CRASH_RECOVERY, STDATA(offset)))
		    user_aborted();
		if (p_cfg->seek_func(offset))
		    user_aborted();
	    }
	    return;

	case ZCRC:
	    *(U32 *)tx_hdr = 0xFFFFFFFF;
	    if (tx_bin32)
		z_send_bin32_header(ZCRC);
	    else
		z_send_bin16_header(ZCRC);
	    break;

	default:
	    status.arg0 = outheader(c);
	    status.arg1 = *(U32 *)rx_hdr;
	    if (p_cfg->status_func(PS_Z_UNEXPECTED_HEADER, STDATA(&status)))
		user_aborted();
	    break;
	}
    }
}

VOID 
#ifdef CK_ANSIC
sz_sinit(void) 
#else
sz_sinit()
#endif
{
  U32 c;
  status_args status;

    if (!esc_control && !query_serial_num && attn_len == 0)
	return;

    while (1) {
	tx_hdr_type = ZSINIT;
	*(U32 *)tx_hdr = 0;
	if (esc_control)
	    tx_hdr[ZF0] |= TESCCTL;

	memcpy(tx_buf, attn, attn_len);
	tx_buf_len = attn_len + 1;	/* + 1 for the terminating null */
	tx_frame_end = ZCRCW;
	z_send_block();

	c = z_get_header();
	status.arg0 = outheader(c);
	status.arg1 = *(U32 *)rx_hdr;
	if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
	    user_aborted();

	switch (c) {
	case ZCAN:
	    pdll_aborted = A_REMOTE;
	    return;

	case ZACK:
	    if (p_cfg->status_func(PS_Z_SERIAL_NUM, STDATA(*(U32 *)rx_hdr)))
		user_aborted();
	    return;

	default:
	    status.arg0 = outheader(c);
	    status.arg1 = *(U32 *)rx_hdr;
	    if (p_cfg->status_func(PS_Z_UNEXPECTED_HEADER, STDATA(&status)))
		user_aborted();
	    break;
	}
    }
}

VOID 
#ifdef CK_ANSIC
sz_init(void) 
#else
sz_init()
#endif
{
    U32 send_zrqinit = 1;
    U32 c;
    time_t t_started;
    time_t t_now;
    status_args status;

    time(&t_started);
    while (1) {
	time(&t_now);
	if (t_now - t_started >= 60) {
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
		user_aborted();
	    pdll_aborted = A_MISC;
	    return;
	}
	if (send_zrqinit) {
	    if (send_rz_cr) {
		dev_putstr_buf("rz\r");
		dev_flush_outbuf();
	    }
	    *(U32 *)tx_hdr = 0L;
	    z_send_hex_header(ZRQINIT);
	    send_zrqinit = 0;
	}
	c = z_get_header();
	status.arg0 = outheader(c);
	status.arg1 = *(U32 *)rx_hdr;
	if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
	    user_aborted();
	switch (c) {
	case ZRINIT:
	    sz_parse_zrinit();
	    sz_sinit();
	    return;

	case ZCHALLENGE:
	    *(U32 *)tx_hdr = *(U32 *)rx_hdr;
	    z_send_hex_header(ZACK);
	    break;

	case ZABORT:		/* Dunno for sure if this can happen here... */
	    got_zabort = 1;
	    return;

	case ZCAN:
	    pdll_aborted = A_REMOTE;
	    return;

	case DEV_TIMEOUT:
	    send_zrqinit = 1;
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
		user_aborted();
	    break;

	case GOTERROR:
	    send_zrqinit = 1;
	    break;

	default:
	    status.arg0 = outheader(c);
	    status.arg1 = *(U32 *)rx_hdr;
	    if (p_cfg->status_func(PS_Z_UNEXPECTED_HEADER, STDATA(&status)))
		user_aborted();
	    break;
	}	
    }
}

U32 
#ifdef CK_ANSIC
sz_eof(void) 
#else
sz_eof()
#endif /* CK_ANSIC */
{
    U32 c = 0;
    time_t t_started;
    time_t t_now;
    status_args status;

    time(&t_started);
    while (1) {
	time(&t_now);
	if (t_now - t_started >= 60) {
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
		user_aborted();
	    pdll_aborted = A_MISC;
	    return(0);
	}
	*(U32 *)tx_hdr = offset;
	if (tx_bin32)
	    z_send_bin32_header(ZEOF);
	else
	    z_send_bin16_header(ZEOF);

	c = z_get_header();
	status.arg0 = outheader(c);
	status.arg1 = *(U32 *)rx_hdr;
	if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
	    user_aborted();
	switch (c) {
	case ZCAN:
	    pdll_aborted = A_REMOTE;
	    return(c);

	case ZABORT:		/* Abort gracefully, doing ZFINs */
	case ZFERR:		/* Receiver couldn't close the file properly */
	    return(c);

	case ZRINIT:
	    sz_parse_zrinit();
	    return(c);

	case ZRPOS:
	    z_handle_zrpos();
	    return(c);

	case DEV_TIMEOUT:
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
		user_aborted();
	    break;

	case GOTERROR:
	    break;

	default:
	    status.arg0 = outheader(c);
	    status.arg1 = *(U32 *)rx_hdr;
	    if (p_cfg->status_func(PS_Z_UNEXPECTED_HEADER, STDATA(&status)))
		user_aborted();
	    break;
	}
    }
}

VOID 
#ifdef CK_ANSIC
sz_fin(void) 
#else
sz_fin()
#endif
{
    U32 c;
    time_t t_started;
    time_t t_now;
    status_args status;

    time(&t_started);
    while (1) {
	time(&t_now);
	if (t_now - t_started >= 60) {
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
		user_aborted();
	    pdll_aborted = A_MISC;
	    return;
	}
	*(U32 *)tx_hdr = offset;
	if (tx_bin32)
	    z_send_bin32_header(ZFIN);
	else
	    z_send_bin16_header(ZFIN);
	c = z_get_header();
	status.arg0 = outheader(c);
	status.arg1 = *(U32 *)rx_hdr;
	if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
	    user_aborted();
	switch (c) {
	case ZFIN:
	    dev_putstr_buf("OO");
	    dev_flush_outbuf();
	    return;

	case ZCAN:
	    pdll_aborted = A_REMOTE;
	    return;

	case GOTERROR:
	    break;

	case DEV_TIMEOUT:
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
		user_aborted();
	    break;

	default:
	    status.arg0 = outheader(c);
	    status.arg1 = *(U32 *)rx_hdr;
	    if (p_cfg->status_func(PS_Z_UNEXPECTED_HEADER, STDATA(&status)))
		user_aborted();
	    break;
	}
    }
}

U32 
#ifdef CK_ANSIC
sz_file(void) 
#else
sz_file()
#endif
{
    got_to_eof = 0;
    if (pdll_aborted)
	return(1);
    length = -1;
    date = -1;
    if (p_cfg->s_open_func(&path, &length, (U32 *)&date, &mode,
			    &pdll_files_left, &pdll_bytes_left,
			    &local_zconv, &local_zmanag, &local_ztrans))
	user_aborted();
    if (path != NULL) {		/* Yes -- there are more files */
	offset = 0;
	skip_file = 0;
	sz_file_header();
	if (!pdll_aborted && !got_zabort && !skip_file) {
	    retransmits = 0;
	    tx_hdr_type = ZDATA;
	    *(U32 *)tx_hdr = offset;
	    while (1) {
		if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
		    user_aborted();
		if ( last_sync_pos == offset && tx_frame_end == ZCRCW ) {
		    tx_frame_end = ZCRCW ;
		    *tx_buf = '\0' ;
		    tx_buf_len = 0 ;
		}
		else {
		    if (p_cfg->read_func(tx_buf, tx_buf_size, &tx_buf_len))
			user_aborted();
		    if (tx_buf_len < tx_buf_size) {
			got_to_eof = 1;
			tx_frame_end = ZCRCE;
		    }
		    else if (blk_size && (tx_wincnt += tx_buf_len) >= blk_size) {
			tx_wincnt = 0;
			tx_frame_end = ZCRCQ;
		    } else {
			tx_frame_end = ZCRCG;
		    }
		}
		z_send_block();
		offset += tx_buf_len;
		if (tx_frame_end == ZCRCW) {
		    z_wait_for_ack();
		    tx_frame_end = ZCRCG ;
		    tx_hdr_type = ZDATA; /* Next frame will be sent with a header... */
		} else if (tx_frame_end == ZCRCQ) {
		    z_wait_for_ack();
		} else if (tx_frame_end != ZCRCE) /* ZCRCE will be checked in */
		    /* the sz_eof() function */
		    z_chk_response();
		if (pdll_aborted || got_zabort || skip_file)
		    break;
		if (got_to_eof) {
		    if (sz_eof() == ZRPOS) {
			continue;		
		    } else
			break;
		}
	    }
	    if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
		user_aborted();
	}
	if (p_cfg->close_func(&path,
			       length,
			       date,
			       retransmits,
			       (pdll_aborted || got_zabort || 
				 (skip_file && move_file)) ?  /* fail if a move was not transferred */
			       FILE_FAILED : FILE_SUCCESSFUL,
			       offset))
	    user_aborted();

	if (pdll_aborted || got_zabort)
	    return(1);
    } else 	/* No more files... */
	return(1);
    return(0);
}

VOID 
#ifdef CK_ANSIC
sz(void) 
#else
sz()
#endif
{
    if (blk_size && blk_size < 1024)
	tx_buf_size = blk_size;
    else
	tx_buf_size = 1024;
    pdll_malloc((void **)&tx_buf, 
                 /* don't let the buffer be too small */
                 tx_buf_size < 64 ? 64 : tx_buf_size, 
                 MODULE_SZ, __LINE__);

    sz_init();

    if (!got_zabort) {
	while (!sz_file())
	    ;
    }
    if (!pdll_aborted)
	sz_fin();
    pdll_free((void **)&tx_buf, MODULE_SZ, __LINE__);
}
