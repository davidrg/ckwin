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

/* Routines for receiving with Zmodem protocol */

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_common.h"
#include "pdll_defs.h"
#include "pdll_dev.h"
#include "pdll_global.h"
#include "pdll_modules.h"
#include "pdll_omalloc.h"
#include "pdll_r.h"
#include "pdll_z.h"
#include "pdll_z_global.h"
#include "p_status.h"

U32 
#ifdef CK_ANSIC
rz_open_file(void) 
#else
rz_open_file()
#endif
{

  offset = 0;
  if (p_cfg->r_open_func(&path, length, date, mode,
			 pdll_files_left, pdll_bytes_left,
			 remote_zconv, remote_zmanag, remote_ztrans,
			 &offset))
    user_aborted();
  if (path == NULL) {
    /* An error occurred while opening the file, we'll skip it... */
    offset = 0;
    return(ZSKIP);
  }
  return(ZRPOS);
}

VOID
#ifdef CK_ANSIC
rz_over_and_out(void) 
#else
rz_over_and_out()
#endif
{

  U32 c;
  U32 try_cnt;
  U32 old_timeouts_per_call;

  old_timeouts_per_call = timeouts_per_call;
  timeouts_per_call = 33;	/* About 1 sec */
  *(U32 *)tx_hdr = 0L;
  for (try_cnt = 0; try_cnt < 3; try_cnt++) {
    z_send_hex_header(ZFIN);
    c = dev_getch_buf();
    switch (c) {
    case 'O':
      timeouts_per_call = 3;	/* About .1 sec */
      dev_getch_buf();
      timeouts_per_call = old_timeouts_per_call;
      return;

    default:
      /* Do nothing, getting this acknowledged isn't really that important */
      break;
    }
  }
  timeouts_per_call = old_timeouts_per_call;
}

VOID
#ifdef CK_ANSIC
rz_file_info(void) 
#else
rz_file_info()     
#endif
{
  U32 c;
  U32 send_zrinit = 1;
  U32 retry_cnt;
  time_t time_started;
  time_t time_now;
  time_t time_last_zrinit = 0;	/* Just to shut up the compiler */
  status_args status;

  time(&time_started);
  time_last_zrinit = time_started; /* To prevent us from timeoutting */
				   /* right from the beginning */
  while (1) {
    time(&time_now);
    if (time_now - time_started >= 40) {
      if (p_cfg->status_func(PS_TIMEOUT, STDATA(40)))
	user_aborted();
      pdll_aborted = A_MISC;
      return;
    } else if (time_now - time_last_zrinit >= 10) {
      if (p_cfg->status_func(PS_TIMEOUT, STDATA(10)))
	user_aborted();
      send_zrinit = 1;
    }
    if (send_zrinit) {
      send_zrinit = 0;
      time_last_zrinit = time_now;
      if (use_alternative_checking) {
	tx_hdr[ZF0] = CANFDX | CANOVIO;
	if (p_cfg->status_func(PS_CHECKING_METHOD, STDATA(CHECKING_CRC16)))
	  user_aborted();
      } else {
	tx_hdr[ZF0] = CANFC32 | CANFDX | CANOVIO;
	if (p_cfg->status_func(PS_CHECKING_METHOD, STDATA(CHECKING_CRC32)))
	  user_aborted();
      }
      if (esc_control)
	tx_hdr[ZF0] |= TESCCTL;
      if (esc_8th_bit)
	tx_hdr[ZF0] |= TESC8;
      tx_hdr[ZF1] = 0;
      tx_hdr[ZP0] = blk_size & 0xff;
      tx_hdr[ZP1] = blk_size >> 8;
      z_send_hex_header(ZRINIT);
      *(U32 *)tx_hdr = 0;
    }
    c = z_get_header();
    status.arg0 = outheader(c);
    status.arg1 = *(U32 *)rx_hdr;
    if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
      user_aborted();

    switch (c) {
    case ZFIN:
      rz_over_and_out();
      zfin_recvd = 1;
      return;
      
    case ZFILE:
      time(&time_last_zrinit);	/* This prevents us from timeouting if we */
				/* get a lot of ZSKIPs, that is, for */
				/* over 10 secs */
      remote_zconv = rx_hdr[ZF0];
      remote_zmanag = rx_hdr[ZF1];
      remote_ztrans = rx_hdr[ZF2];
      c = z_recv_block();
      if (p_cfg->status_func(PS_Z_FRAME_END, STDATA(outframe(c))))
	user_aborted();
      switch (c) {
      case GOTCRCW:		/* Got what we wanted */
	process_file_info();
	if (pdll_aborted)
	  return;

	switch (rz_open_file()) {
	case ZSKIP:
	  *(U32 *)tx_hdr = 0L;
	  z_send_hex_header(ZSKIP);
	  continue;
	  
	case ZRPOS:
	  *(U32 *)tx_hdr = offset;
	  z_send_hex_header(ZRPOS);
	  return;	/* Let's go and transfer the file */

	default:
	  /* Never gets here because rz_open_file() returns */
	  /* only ZSKIP or ZRPOS */
	  break;
	}
	break;

      case GOTCAN:
	pdll_aborted = A_REMOTE;
	return;

      case DEV_TIMEOUT:
	break;
	
      case GOTERROR:
	break;
	
      case GOTCRCE:
      case GOTCRCG:
      case GOTCRCQ:
	if (p_cfg->status_func(PS_Z_INVALID_FRAME_END, STDATA(outframe(c))))
	  user_aborted();
	*(U32 *)tx_hdr = 0L;
	z_send_hex_header(ZNAK);
	break;
	
      default:
	/* We never get here */
	break;
      }
      break;

    case ZRQINIT:
      send_zrinit = 1;
      break;
      
    case ZSINIT:
      if (rx_hdr[ZF0] & TESCCTL)
	esc_control = 1;
      if (rx_hdr[ZF0] & TESC8)
	esc_8th_bit = 1;
      if (p_cfg->status_func(PS_Z_SENDER_FLAGS, STDATA(rx_hdr[ZF0])))
	user_aborted();
      
      rx_buf_size = 32;
      c = z_recv_block();
      if (p_cfg->status_func(PS_Z_FRAME_END, STDATA(outframe(c))))
	user_aborted();
      rx_buf_size = 1024;
      switch (c) {
      case GOTCRCW:
	/**********************************************/
	/* Let's save the received attention sequence */
	/**********************************************/
	attn_len = rx_buf_len - 1; /* Don't include the null */
	memcpy(attn, rx_buf, rx_buf_len);
	
	*(U32 *)tx_hdr = p_cfg->serial_num;
	z_send_hex_header(ZACK);
	break;

      case GOTCAN:
	pdll_aborted = A_REMOTE;
	return;

      case DEV_TIMEOUT:
	break;

      case GOTERROR:
	break;

      case GOTCRCE:
      case GOTCRCG:
      case GOTCRCQ:
	if (p_cfg->status_func(PS_Z_INVALID_FRAME_END, STDATA(outframe(c))))
	  user_aborted();
	*(U32 *)tx_hdr = 0L;
	z_send_hex_header(ZNAK);
	break;
	
      default:
	/* We never get here */
	break;
      }
      break;
      
    case ZFREECNT:
      *(U32 *)tx_hdr = ~0L;
      z_send_hex_header(ZACK);
      break;

    case ZCOMMAND:
      c = z_recv_block();
      if (p_cfg->status_func(PS_Z_FRAME_END, STDATA(outframe(c))))
	user_aborted();
      *(U32 *)tx_hdr = 0L;
      switch (c) {
      case GOTCRCW:
	if (p_cfg->status_func(PS_Z_COMMAND, STDATA(rx_buf)))
	  user_aborted();
	for (retry_cnt = 0; retry_cnt < 10; retry_cnt++) {
	  z_send_hex_header(ZCOMPL);
	  c = z_get_header();
      status.arg0 = outheader(c);
      status.arg1 = *(U32 *)rx_hdr;
	  if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
	    user_aborted();
	  switch (c) {
	  case ZCAN:
	    pdll_aborted = A_REMOTE;
	    return;

	  case ZFIN:
	    rz_over_and_out();
	    zfin_recvd = 1;
	    return;

	  default:
        status.arg0 = outheader(c);
        status.arg1 = *(U32 *)rx_hdr;
	    if (p_cfg->status_func(PS_Z_UNEXPECTED_HEADER, STDATA(&status)))
	      user_aborted();
	    break;
	  }
	}
	break;

      case GOTCAN:
	pdll_aborted = A_REMOTE;
	return;

      case DEV_TIMEOUT:
	break;

      case GOTERROR:
	break;

      case GOTCRCE:
      case GOTCRCG:
      case GOTCRCQ:
	if (p_cfg->status_func(PS_Z_INVALID_FRAME_END, STDATA(outframe(c))))
	  user_aborted();
	*(U32 *)tx_hdr = 0L;
	z_send_hex_header(ZNAK);
	break;
	
      default:
	/* We never get here */
	break;
      }
      break;

    case DEV_TIMEOUT:
      break;

    case GOTERROR:
      break;
      
    case ZCAN:
      pdll_aborted = A_REMOTE;
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

U32 
#ifdef CK_ANSIC
rz_file(void) 
#else
rz_file()
#endif
{
    U32 bail = 0;
    U32 c;
    U32 bail_from_block_loop;
    U32 ret_val = 0;
    time_t t_prev = 0;
    time_t t_now;
    status_args status;

    offset = 0;
    retransmits = 0;
    rx_buf_size = 1024;		/* Maximum subpacket size */

    rz_file_info();
    if (zfin_recvd || pdll_aborted || path == NULL)
	return(1);
    while (!bail) {
	c = z_get_header();
    status.arg0 = outheader(c);
    status.arg1 = *(U32 *)rx_hdr;
	if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
	    user_aborted();
	if (c == ZEOF) {
	    if (*(U32 *)rx_hdr != offset) {
		if (p_cfg->status_func(PS_Z_PHONY_ZEOF, NULL))
		    user_aborted();
		continue;
	    }
	    break;			/* Break out from the loop */
	}
	if (c != ZDATA) {
	    time(&t_now);
	    if (t_prev) {
		if (t_now - t_prev >= 180) {	     /* We've got non-ZDATA headers */
		    /* for 3 minutes, it's time to */
		    /* call quits */
		    if (p_cfg->status_func(PS_TIMEOUT, STDATA(180)))
			user_aborted();
		    pdll_aborted = A_MISC;
		    ret_val = 1;
		    break;			/* Break out from the loop */
		}
	    } else
		t_prev = t_now;
	} else
	    t_prev = 0;
	switch (c) {
	case ZDATA:
	    if (*(U32 *)rx_hdr != offset) {
        status.arg0 = *(U32 *)rx_hdr;
        status.arg1 = offset;
		if (p_cfg->status_func(PS_Z_DATA_FROM_INVALID_POS, STDATA(&status)))
		    user_aborted();
		z_send_attn();
		*(U32 *)tx_hdr = offset;
		z_send_hex_header(ZRPOS);
		break;
	    }
	    bail_from_block_loop = 0;
	    while (!bail_from_block_loop) {
		if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
		    user_aborted();
		c = z_recv_block();
		if (p_cfg->status_func(PS_Z_FRAME_END, STDATA(outframe(c))))
		    user_aborted();

		/* Note the continue keywords in following switch () thing */
		switch (c) {
		case GOTCAN:
		    pdll_aborted = A_REMOTE;
		    ret_val = bail = bail_from_block_loop = 1;
		    continue;

		case DEV_TIMEOUT:
		    *(U32 *)tx_hdr = offset;
		    z_send_hex_header(ZRPOS);
		    bail_from_block_loop = 1;
		    continue;

		case GOTERROR:
		    z_send_attn();
		    *(U32 *)tx_hdr = offset;
		    z_send_hex_header(ZRPOS);
		    bail_from_block_loop = 1;
		    retransmits++;
		    continue;

		case GOTCRCE:
		case GOTCRCG:
		case GOTCRCQ:
		case GOTCRCW:
		    /* Do nothing yet, wait for buffer to be written */
		    break;

		default:
		    /* We never get here... */
		    break;	
		}
		if (p_cfg->write_func(rx_buf, rx_buf_len))
		    user_aborted();
		offset += rx_buf_len;
		switch (rx_frame_end) {
		case ZCRCW:
		    dev_putch_buf(XON);
		    *(U32 *)tx_hdr = offset;
		    z_send_hex_header(ZACK);
		    bail_from_block_loop = 1; /* Bail out to get a new header */
		    break;

		case ZCRCQ:
		    *(U32 *)tx_hdr = offset;
		    z_send_hex_header(ZACK);
		    /* Keep on getting data */
		    break;	

		case ZCRCG:
		    break;

		case ZCRCE:
		    bail_from_block_loop = 1;
		    break;
		}
	    }
	    if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
		user_aborted();
	    break;

	case DEV_TIMEOUT:
	    *(U32 *)tx_hdr = offset;
	    z_send_hex_header(ZRPOS);
	    break;

	case ZCAN:
	    pdll_aborted = A_REMOTE;
	    ret_val = bail = 1;
	    break;

	case GOTERROR:
	    break;

	case ZNAK:
	    *(U32 *)tx_hdr = offset;
	    z_send_hex_header(ZRPOS);
	    break;

	default:
        status.arg0 = outheader(c);
        status.arg1 = *(U32 *)rx_hdr;
	    if (p_cfg->status_func(PS_Z_UNEXPECTED_HEADER, STDATA(&status)))
		user_aborted();
	    break;
	}
    }
    if (p_cfg->close_func(&path,
			   length,
			   date,
			   retransmits,
			   pdll_aborted ? FILE_FAILED : FILE_SUCCESSFUL,
			   offset))
	user_aborted();
    return(ret_val);
}

VOID
#ifdef CK_ANSIC
rz(void) 
#else
rz()
#endif
{

  pdll_malloc((void **)&rx_buf, 1024, MODULE_RZ, __LINE__);
  while (!rz_file())
    ;
  pdll_free((void **)&rx_buf, MODULE_RZ, __LINE__);
}
