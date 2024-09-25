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

/* Routines used by both, Zmodem sending, and receiving */

#include <stdio.h>

#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_common.h"
#include "pdll_crc.h"
#include "pdll_defs.h"
#include "pdll_dev.h"
#include "pdll_global.h"
#include "pdll_z.h"
#include "pdll_z_global.h"
#include "p_status.h"

VOID 
#ifdef CK_ANSIC
z_send_byte_as_hex(U8 c) 
#else
z_send_byte_as_hex() U8 c ;
#endif
{

  static U8 *hex = "0123456789abcdef";

  dev_putch_buf(hex[(c & 0xF0) >> 4]);
  dev_putch_buf(hex[c & 0x0F]);
}

U16 
#ifdef CK_ANSIC
zdl_getch(void) 
#else
zdl_getch()
#endif
{

  static U16 c;
  static U16 can_cnt;

  while (1) {
    c = dev_getch_buf();
    if (c > 255)
      return(c);
    if (c == 17 ||	/* CTRL-Q */
	c == 145 ||	/* CTRL-Q + 128 */
	c == 19 ||	/* CTRL-S */
	c == 147) {	/* CTRL-S + 128 */
      continue;
    }
    if (c == ZDLE)
      break;
    else if (esc_control && !(c & 96)) {
      if (p_cfg->status_func(PS_Z_CTRL_CHAR_IGNORED, STDATA(c)))
	user_aborted();
      continue;
    } else
      return(c);
  }
  can_cnt = 0;
  while (1) {
    c = dev_getch_buf();
    if (c > 255)
      return(c);
    if (c == 17 ||	/* CTRL-Q */
	c == 145 ||	/* CTRL-Q + 128 */
	c == 19 ||	/* CTRL-S */
	c == 147) {	/* CTRL-S + 128 */
      continue;
    }
    switch (c) {
    case CAN:
      can_cnt++;
      if (can_cnt == 4) /* 4 on purpose, ZDLE and CAN are the same */
	return(GOTCAN);
      break;

    case ZCRCE:
    case ZCRCG:
    case ZCRCQ:
    case ZCRCW:
      if (getting_frame)
	return(c + GOTADD);
      else
	return(GOTERROR);

    case ZRUB0:
      return(127);

    case ZRUB1:
      return(255);

    default:
      if (esc_control && !(c & 96)) {
	if (p_cfg->status_func(PS_Z_CTRL_CHAR_IGNORED, STDATA(c)))
	  user_aborted();
	continue;
      }
      if ((c & 96) == 64) { /* bit6 set, bit5 reset? */
	return(c ^ 64);
      } else {
	if (p_cfg->status_func(PS_Z_INVALID_ZDLE_SEQUENCE, NULL))
	  user_aborted();
	return(GOTERROR);
      }
    }
  }
}

U32 zdl_putch_last_c;

VOID 
#ifdef CK_ANSIC
zdl_putch(U8 c) 
#else
zdl_putch() U8 c;
#endif 
{
#ifdef COMMENT
    if (c & 96) { /* Quick check for non-control characters */
	dev_putch_buf(c);
	zdl_putch_last_c = c;
	return;
    }
#endif /* COMMENT */
    /* Regardless of anything else, these must be escaped */
    switch ( c ) {
    case 16:			/* CTRL-P */
    case 16 + 128:
    case XON:
    case XON + 128:
    case XOFF:
    case XOFF + 128:
    case ZDLE:
    case ZDLE + 128:
	dev_putch_buf(ZDLE);
	dev_putch_buf(c ^ 64);
	break;

    case '\r':
    case '\r' + 128:
	if ( dev_telnet ) {
	    dev_putch_buf(ZDLE);
	    dev_putch_buf(c ^ 64);
	    break;
	}

    default:
	if ( esc_table ) {
	    if ( control_prefix_table[c] ) {
		switch ( c ) {
		case 127:
		    dev_putch_buf(ZDLE);
		    dev_putch_buf(ZRUB0);
		    break;

		case 255:
		    dev_putch_buf(ZDLE);
		    dev_putch_buf(ZRUB1);
		    break;

		default:
		    if (!(c & 96)) { /* ch < 32 */
			dev_putch_buf(ZDLE);
			dev_putch_buf(c | 64);
		    } else {
			zdl_putch_last_c = c;
			dev_putch_buf(c);
		    }
		    break;
		}
	    }
	    else {
		zdl_putch_last_c = c;
		dev_putch_buf(c);
	    }
	} 
	else if (esc_minimal) {		/* Escape only the ZDLE, XON and XOFF? */
	    dev_putch_buf(c);
	} else {
	    if (esc_control && !(c & 96)) { /* ch < 32 */
		dev_putch_buf(ZDLE);
		dev_putch_buf(c | 64);
	    } else {
		zdl_putch_last_c = c;
		dev_putch_buf(c);
	    }
	    break;
	}
    }
}

U32 
#ifdef CK_ANSIC
z_get_bin_header(void) 
#else
z_get_bin_header()
#endif
{

   static U32 c;
   static U32 crc;
   static U32 n;

   rx_frame_type = ZBIN;
   c = zdl_getch();
   if (c > 255)
     return(c);
   rx_hdr_type = c;

   crc = updcrc16(c, 0);

   for (n = 0; n < 4; n++) {
     c = zdl_getch();
     if (c > 255)
       return(c);
     crc = updcrc16(c, crc);
     rx_hdr[n] = c;
   }
   c = zdl_getch();
   if (c > 255)
     return(c);
   crc = updcrc16(c, crc);

   c = zdl_getch();
   if (c > 255)
     return(c);
   crc = updcrc16(c, crc);

   if (crc & 0xFFFF) {
     if (!recovering) {
       if (p_cfg->status_func(PS_Z_CHECK_FAILED_FOR_HEADER, STDATA(CHECKING_CRC16)))
	 user_aborted();
     }
     return(GOTERROR);
   }
   return(rx_hdr_type);
}

U32 
#ifdef CK_ANSIC
z_get_bin32_header(void) 
#else
z_get_bin32_header()
#endif
{

  static U32 c;
  static U32 crc;
  static U32 n;

  rx_frame_type = ZBIN32;
  c = zdl_getch();
  if (c > 255)
    return(c);
  rx_hdr_type = c;
  
  crc = 0xFFFFFFFF;
  crc = updcrc32(c, crc);
  
  for (n = 0; n < 4; n++) {
    c = zdl_getch();
    if (c > 255)
      return(c);
    crc = updcrc32(c, crc);
    rx_hdr[n] = c;
  }
  for (n = 0; n < 4; n++) {
    c = zdl_getch();
    if (c > 255)
      return(c);
    crc = updcrc32(c, crc);
  }
  if (crc != 0xDEBB20E3) {
    if (!recovering) {
      if (p_cfg->status_func(PS_Z_CHECK_FAILED_FOR_HEADER, STDATA(CHECKING_CRC32)))
	user_aborted();
    }
    return(GOTERROR);
  }
  return(rx_hdr_type);
}

U32 
#ifdef CK_ANSIC
z_getch(void) 
#else
z_getch()
#endif
{

  static U32 can_cnt;
  static U32 c;

  can_cnt = 0;
  while (1) {
    c = dev_getch_buf();
    if (c > 255)
      return(c);
    if (c == CAN) {
      can_cnt++;
      if (can_cnt == 5)
	return(GOTCAN);
    } else
      can_cnt = 0;
    switch (c) {
    case XON:
    case XOFF:
      break;

    case '\n':
    case '\r':
    case ZDLE:
      return(c);

    default:
      if (esc_control && !(c & 96)) {
	if (p_cfg->status_func(PS_Z_CTRL_CHAR_IGNORED, STDATA(c)))
	  user_aborted();
	continue;
      } else
	return(c);
    }
  }
}

U32 
#ifdef CK_ANSIC
zhex_getch(void) 
#else
zhex_getch()
#endif
{

  static U32 c;
  static U32 c1;

  c = z_getch();
  if (c > 255)
    return(c);
  c -= '0';
  if (c > 9)
     c -= ('a' - ':');
  if (c > 15) {
    if (!recovering) {
      if (p_cfg->status_func(PS_Z_INVALID_HEX_HEADER, NULL))
	user_aborted();
    }
    return(GOTERROR);
  }
  c1 = c;

  c = z_getch();
  if (c > 255)
    return(c);
  c -= '0';
  if (c > 9)
    c -= ('a' - ':');
  if (c > 15) {
    if (!recovering) {
      if (p_cfg->status_func(PS_Z_INVALID_HEX_HEADER, NULL))
	user_aborted();
    }
    return(GOTERROR);
  }
  return((c1 << 4) | c);
}

U32 
#ifdef CK_ANSIC
z_get_hex_header(void) 
#else
z_get_hex_header()
#endif
{

  static U32 crc;
  static U32 c;
  static U32 n;
  static U32 old_timeouts_per_call;
  
  rx_frame_type = ZHEX;
  c = zhex_getch();
  if (c > 255)
    return(c);
  rx_hdr_type = c;

  crc = updcrc16(c, 0);

  for (n = 0; n < 4; n++) {
    c = zhex_getch();
    if (c > 255)
      return(c);
    crc = updcrc16(c, crc);
    rx_hdr[n] = c;
  }
  c = zhex_getch();
  if (c > 255)
    return(c);
  crc = updcrc16(c, crc);

  c = zhex_getch();
  if (c > 255)
    return(c);
  crc = updcrc16(c, crc);
  
  if (crc & 0xFFFF) {
    if (!recovering) {
      if (p_cfg->status_func(PS_Z_CHECK_FAILED_FOR_HEADER, STDATA(CHECKING_CRC16)))
	user_aborted();
    }
    return(GOTERROR);
  }
  /*****************************/
  /* Throw away possible cr/lf */
  /*****************************/
  old_timeouts_per_call = timeouts_per_call;
  timeouts_per_call = 1;

  if (dev_getch_buf() == '\r')
    dev_getch_buf();		/* Eat the possible linefeed */

  /***************************/
  /* Throw away possible XON */
  /***************************/
  c = dev_getch_buf();

  timeouts_per_call = old_timeouts_per_call;
  return(rx_hdr_type);
}

U32 
#ifdef CK_ANSIC
z_get_header(void) 
#else
z_get_header()
#endif
{

  static U32 can_cnt;
  static U32 state;
  static U32 c;

  can_cnt = 0;
  state = 0;
  while (1) {
    c = dev_getch_buf();
    if (c > 255) {
      *(U32 *)rx_hdr = 0;
      return(c);
    }
    switch (state) {
    case 0:
      switch (c) {
      case ZPAD:
	state++;
	break;

      case XON:
      case XOFF:
	break;

      default:
	/* Got a junk ch, let's ignore */
	break;
      }
      break;

    case 1:
      switch (c) {
      case ZDLE:
	state++;
	break;
	
      case ZPAD:
	/* Let's retry with this state */
	break;
	
      case XON:
      case XOFF:
	break;

      default:
	/* Got a junk ch, let's ignore */
	break;
      }
      break;
      
    case 2:
      switch (c) {
      case ZBIN:
	c = z_get_bin_header();
	if (c > 255)
	  *(U32 *)rx_hdr = 0;
	if (c == GOTCAN)
	  c = ZCAN;
	state++;
	break;

      case ZBIN32:
	c = z_get_bin32_header();
	if (c > 255)
	  *(U32 *)rx_hdr = 0;
	if (c == GOTCAN)
	  c = ZCAN;
	state++;
	break;

      case ZHEX:
	c = z_get_hex_header();
	if (c > 255)
	  *(U32 *)rx_hdr = 0;
	state++;
	break;

      case XON:
      case XOFF:
	break;

      default:
	/* Got a junk ch, let's ignore */
	break;
      }
      break;
    }
    if (state == 3) {
      if (c == GOTERROR)
	recovering = 1;
      else if (recovering)
	recovering = 0;
      break;
    }
    if (c == CAN) {
      can_cnt++;
      if (can_cnt == 5)
	return(ZCAN);
    } else
      can_cnt = 0;
  }
  return(c);
}

VOID 
#ifdef CK_ANSIC
z_send_hex_header(U16 type) 
#else
z_send_hex_header() U16 type ;
#endif
{

  static U32 n;
  static U16 crc;

  dev_putch_buf(ZPAD);
  dev_putch_buf(ZPAD);
  dev_putch_buf(ZDLE);
  dev_putch_buf(ZHEX);
  
  z_send_byte_as_hex(type);
  
  crc = updcrc16(type, 0);

  for (n = 0; n < 4; n++) {
    z_send_byte_as_hex(tx_hdr[n]);
    crc = updcrc16(tx_hdr[n], crc);
  }
  z_send_byte_as_hex(crc >> 8);
  z_send_byte_as_hex(crc);

  /* Make it printable on remote machine */
  dev_putch_buf('\r');
  dev_putch_buf(138);		/* rzsz does send this... wonder why... */
				/* but let's do it by the book... */

  /* Uncork the remote in case a fake XOFF has stopped data flow */
  if (type != ZFIN && type != ZACK)
    dev_putch_buf(021);
  dev_flush_outbuf();
}

VOID 
#ifdef CK_ANSIC
z_send_data16(void) 
#else
z_send_data16()
#endif
{
  
  static U8 *p;
  static U16 crc;
  static S32 len;
  
  p = tx_buf;
  len = tx_buf_len;

  crc = 0;
  for (; --len >= 0;) {
    zdl_putch(*p);
    crc = updcrc16(*p++, crc);
  }
  dev_putch_buf(ZDLE);
  dev_putch_buf(tx_frame_end);
  crc = updcrc16(tx_frame_end, crc);
  zdl_putch((crc >> 8));
  zdl_putch(crc);
  
  if (tx_frame_end == ZCRCW)
    dev_putch_buf(XON);
  if (tx_frame_end != ZCRCG)
    dev_flush_outbuf();
}

VOID 
#ifdef CK_ANSIC
z_send_data32(void) 
#else
z_send_data32()
#endif
{

  static U8 *p; 
  static U32 crc;
  static S32 len;

  p = tx_buf;
  len = tx_buf_len;

  crc = 0xFFFFFFFF;
  for (; --len >= 0; ++p) {
    zdl_putch(*p);
    crc = updcrc32(*p, crc);
  }
  dev_putch_buf(ZDLE);
  dev_putch_buf(tx_frame_end);
  crc = updcrc32(tx_frame_end, crc);

  crc = ~crc;

  for (len = 4; --len >= 0;) {
    zdl_putch(crc);
    crc >>= 8;
  }
  if (tx_frame_end == ZCRCW)
    dev_putch_buf(XON);
  if (tx_frame_end != ZCRCG)
    dev_flush_outbuf();
}

VOID 
#ifdef CK_ANSIC
z_send_bin16_header(U16 type) 
#else
z_send_bin16_header() U16 type ;
#endif
{

  static U16 crc;
  static U32 n;

  dev_putch_buf(ZPAD);
  dev_putch_buf(ZDLE);
  
  dev_putch_buf(ZBIN);
  zdl_putch(type);
  
  crc = updcrc16(type, 0);
  
  for (n = 0; n < 4; n++) {
    zdl_putch(tx_hdr[n]);
    crc = updcrc16(tx_hdr[n], crc);
  }
  zdl_putch(crc >> 8);
  zdl_putch(crc);
  dev_flush_outbuf();
}

VOID 
#ifdef CK_ANSIC
z_send_bin32_header(U16 type) 
#else
z_send_bin32_header() U16 type ;
#endif
{
  
  static U32 crc;
  static U32 n;
  
  dev_putch_buf(ZPAD);
  dev_putch_buf(ZDLE);

  dev_putch_buf(ZBIN32);
  zdl_putch(type);

  crc = 0xFFFFFFFF;
  crc = updcrc32(type, crc);

  for (n = 0; n < 4; n++) {
    zdl_putch(tx_hdr[n]);
    crc = updcrc32(tx_hdr[n], crc);
  }
  crc = ~crc;
  for (n = 0; n < 4; n++) {
    zdl_putch(crc);
    crc >>= 8;
  }
  dev_flush_outbuf();
}

VOID 
#ifdef CK_ANSIC
z_handle_zrpos(void) 
#else
z_handle_zrpos()
#endif
{
    offset = *(U32 *)rx_hdr;
    if (p_cfg->seek_func(offset))
	user_aborted();

    if (last_sync_pos == offset) {
	sync_cnt++;
	if (sync_cnt > 12) {
	    if (p_cfg->status_func(PS_CANNOT_SEND_BLOCK, NULL))
		user_aborted();
	    pdll_aborted = A_MISC;
	    return;
	}
	if (sync_cnt > 4) {
	    if (tx_buf_size > 32)
		tx_buf_size /= 2;
	}
    } else
	sync_cnt = 0;
    last_sync_pos = offset;

    dev_purge_outbuf();		/* Empty any unsent data */
#ifdef COMMENT
    tx_buf_len = 0 ;
    /* Send a ZCRCW frame to end current window */
    tx_frame_end = ZCRCW ;
    z_send_block() ;
#endif 

    tx_hdr_type = ZDATA ;
    tx_frame_end = ZCRCW ;

    *(U32 *)tx_hdr = offset;
    if (got_to_eof)
	got_to_eof = 0;	/* There's more to come */
}

U32 
#ifdef CK_ANSIC
z_get_in_sync(void) 
#else
z_get_in_sync()
#endif
{
  static U32 c;
  static U32 bail;
  status_args status;

  bail = 0;
  while (!bail) {
    c = z_get_header();
    status.arg0 = outheader(c);
    status.arg1 = *(U32 *)rx_hdr;
    if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
      user_aborted();
    switch (c) {
    case ZCAN:
      pdll_aborted = A_REMOTE;
      bail = 1;
      break;

    case ZABORT:		/* Abort gracefully, doing ZFINs */
      got_zabort = 1;
      bail = 1;
      break;

    case DEV_TIMEOUT:
      *(U32 *)tx_hdr = 0;	/* Should there be something?  */
      z_send_hex_header(ZNAK);
	if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
	    user_aborted();
	break;

    case ZRPOS:
      z_handle_zrpos();
      retransmits++;
      bail = 1;
      break;

    case GOTERROR:
      break;

    default:
      *(U32 *)tx_hdr = 0;
      z_send_hex_header(ZNAK);
      break;
    }
  }
  return(c);
}

VOID 
#ifdef CK_ANSIC
z_wait_for_ack(void) 
#else
z_wait_for_ack()
#endif
{

  static U32 bail;
  static U32 c;
  static time_t t_now;
  static time_t t_started;
  status_args status;

  time(&t_started);
  bail = 0;
  while (!bail) {
    /* This is up here, so that we don't need to */
    /* check if bail is true or not... */
    time(&t_now);
    if (t_now - t_started > 180) {
      if (p_cfg->status_func(PS_TIMEOUT, STDATA(180)))
	user_aborted();
      pdll_aborted = A_MISC;
      break;			/* Bail out from the loop  */
    }
    c = z_get_header();

    status.arg0 = outheader(c);
    status.arg1 = *(U32 *)rx_hdr;
    if (p_cfg->status_func(PS_Z_HEADER, STDATA(&status)))
      user_aborted();
    switch (c) {
    case ZCAN:
      pdll_aborted = A_REMOTE;
      bail = 1;
      break;

    case ZABORT:		/* Abort gracefully, doing ZFINs */
      got_zabort = 1;
      bail = 1;
      break;

    case DEV_TIMEOUT:
	if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
	    user_aborted();
	break;

    case ZRPOS:
      z_handle_zrpos();
      retransmits++;
      bail = 1;
      break;

    case ZACK:
      if (*(U32 *)rx_hdr == offset) {
	bail = 1;
	*(U32 *)tx_hdr = offset; /* THIS IS IMPORTANT!!! Next */
				 /* frame will be sent with a header and */
				 /* header must be up to date or we in do-do */
      }
      break;

    case GOTERROR:
      break;

    default:
      *(U32 *)tx_hdr = 0;
      z_send_hex_header(ZNAK);
      break;
    }
  }
}

VOID 
#ifdef CK_ANSIC
z_chk_response(void) 
#else
z_chk_response()
#endif
{

  static U16 c;
  static U32 old_timeouts_per_call;

  if (dev_incoming()) {
    do {
      c = dev_getch_buf();
      switch (c) {
      case CAN:
      case ZPAD:
	z_get_in_sync();
	return;
	  
      case XOFF:
	/* Eat the possible XON */
	old_timeouts_per_call = timeouts_per_call;
	timeouts_per_call = 1;
	dev_getch_buf();
	timeouts_per_call = old_timeouts_per_call;
	break;
	
      default:
	/* Skip the possible line garbage */
	break;
      }
    } while (dev_incoming());
  }
  if (tx_hdr_type != 255)	/* Remote had nothing to say, let's send   */
    tx_hdr_type = 255;		/* following subpackets without the header */
}

VOID 
#ifdef CK_ANSIC
z_send_block(void) 
#else
z_send_block()
#endif
{

  if (tx_bin32) {
    if (tx_hdr_type != 255)
      z_send_bin32_header(tx_hdr_type);
    z_send_data32();
  } else {
    if (tx_hdr_type != 255)
      z_send_bin16_header(tx_hdr_type);
    z_send_data16();
  }
}

U32 
#ifdef CK_ANSIC
z_recv_block(void) 
#else
z_recv_block()
#endif
{

  static U8 crc16[2];
  static U8 crc32[4];
  static U16 c;
  static U16 true_crc16;
  static U32 true_crc32;
  static U32 crc_idx;

  if (rx_frame_type == ZBIN32) {
    true_crc32 = 0xFFFFFFFF;
    rx_buf_len = 0;
    getting_frame = 1;
    while (1) {
      c = zdl_getch();
      if (c > 255)
	break;
      if (rx_buf_len == rx_buf_size) {
	if (p_cfg->status_func(PS_Z_SUBPACKET_TOO_LONG, STDATA(rx_buf_size)))
	  user_aborted();
	c = GOTERROR;
	break;
      }
      true_crc32 = updcrc32(c, true_crc32);
      rx_buf[rx_buf_len] = c;
      rx_buf_len++;
    }
    getting_frame = 0;
    switch (c) {
    case GOTCRCE:
    case GOTCRCG:
    case GOTCRCQ:
    case GOTCRCW:
      rx_frame_end = c - GOTADD;
      break;
      
    case GOTERROR:
      recovering = 1;
      /* Fall thru */

    default:
      return(c);
    }
    true_crc32 = updcrc32(rx_frame_end, true_crc32);
      
    true_crc32 = ~true_crc32;
    crc_idx = 0;
    while (1) {
      c = zdl_getch();
      if (c > 255)
	break;
      crc32[crc_idx] = c;
      crc_idx++;
      if (crc_idx == 4) {
	if (*(U32 *)crc32 != true_crc32) {
	  if (p_cfg->status_func(PS_CHECK_FAILED, STDATA(CHECKING_CRC32)))
	    user_aborted();
	  c = GOTERROR;
	  break;
	} else
	  return(rx_frame_end + GOTADD);
      }
    }
    if (c == GOTERROR)
      recovering = 1;
    return(c);
  } else {
    true_crc16 = 0;
    rx_buf_len = 0;
    getting_frame = 1;
    while (1) {
      c = zdl_getch();
      if (c > 255)
	break;
      if (rx_buf_len == rx_buf_size) {
	if (p_cfg->status_func(PS_Z_SUBPACKET_TOO_LONG, STDATA(rx_buf_size)))
	  user_aborted();
	c = GOTERROR;
	break;
      }
      true_crc16 = updcrc16(c, true_crc16);
      rx_buf[rx_buf_len] = c;
      rx_buf_len++;
    }
    getting_frame = 0;
    switch (c) {
    case GOTCRCE:
    case GOTCRCG:
    case GOTCRCQ:
    case GOTCRCW:
      rx_frame_end = c - GOTADD;
      break;

    case GOTERROR:
      recovering = 1;
      /* Fall thru */
      
    default:
      return(c);
    }
    true_crc16 = updcrc16(rx_frame_end, true_crc16);
    crc_idx = 1;
    while (1) {
      c = zdl_getch();
      if (c > 255)
	break;
      crc16[crc_idx] = c;
      crc_idx--;
      if (crc_idx == -1) {
	if (*(U16 *)crc16 != true_crc16) {
	  if (p_cfg->status_func(PS_CHECK_FAILED, STDATA(CHECKING_CRC16)))
	    user_aborted();
	  c = GOTERROR;
	  break;
	} else
	  return(rx_frame_end + GOTADD);
      }
    }
    if (c == GOTERROR)
      recovering = 1;
    return(c);
  }
}

VOID 
#ifdef CK_ANSIC
z_send_attn(void) 
#else
z_zend_attn()
#endif
{

  U8 *p;

  for (p = attn; *p; p++) {
    switch (*p) {
    case 221: /* Send a break signal */
      dev_flush_outbuf();
      dev_set_break_on();
      DosSleep(275);		/* 275ms is adopted from C-kermit */
      dev_set_break_off();
      break;

    case 222: /* Pause for one second */
      dev_flush_outbuf();
      DosSleep(1000);
      break;

    default:
      dev_putch_buf(*p);
      break;
    }
  }
  dev_flush_outbuf();
}

