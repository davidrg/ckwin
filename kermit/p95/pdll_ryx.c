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

/* Routines for receiving with X or Ymodem protocol */

#include <stdio.h>
#include <time.h>

#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_common.h"
#include "pdll_crc.h"
#include "pdll_defs.h"
#include "pdll_dev.h"
#include "pdll_global.h"
#include "pdll_modules.h"
#include "pdll_omalloc.h"
#include "pdll_r.h"
#include "pdll_x_global.h"

VOID
#ifdef CK_ANSIC
ryx_blk_size(void)
#else
ryx_blk_size()
#endif
{
  U16 ch;
  time_t t_started;
  time_t t_now;

  time(&t_started);
  blk[0] = '\0';
  can_cnt = 0;
  while (1) {
    ch = dev_getch_buf();
    switch (ch) {
    case SOH:
      blk_size = 128;
      blk[0] = SOH;
      return;

    case STX:
      blk_size = 1024;
      blk[0] = STX;
      return;

    case EOT:
      blk[0] = EOT;
      dev_putch_buf(ACK);		/* Also ymodem-g acknowledges */
      dev_flush_outbuf();
      return;

    case CAN:
      can_cnt++;
      if (can_cnt == 5) {
	pdll_aborted = A_REMOTE;
	return;
      }
      break;
      
    case DEV_TIMEOUT:
      break;

    default:
      can_cnt = 0;
      break;
    }
    time(&t_now);
    if (t_now - t_started >= 5) {
      if (p_cfg->status_func(PS_TIMEOUT, STDATA(5)))
	user_aborted();
      return;
    }
  }
}

VOID
#ifdef CK_ANSIC
ryx_handshake(void) 
#else
ryx_handshake()
#endif
{
  U8 ch_to_send;
  U32 cnt = 0;
  
  switch (protocol_type) {
  case PROTOCOL_G:
    ch_to_send = 'G';
    break;

  case PROTOCOL_X:
    if (use_1k_blocks) {
      if (use_alternative_checking)
	ch_to_send = NAK;
      else
	ch_to_send = 'C';
    } else {
      if (use_alternative_checking)
	ch_to_send = 'C';
      else
	ch_to_send = NAK;
    }
    break;


  case PROTOCOL_Y:
  default:			/* Just to shut up the compiler  */
    if (use_alternative_checking)
      ch_to_send = NAK;
    else
      ch_to_send = 'C';
    break;
  }
  while (1) {
    dev_putch_buf(ch_to_send);
    dev_flush_outbuf();

    ryx_blk_size();
    if (pdll_aborted)
      return;
    if (blk[0] != '\0') {
      switch (ch_to_send) {
      case NAK:
	if (chk_data_len != 1) {	/* Prevents us from repeating this */
	  if (p_cfg->status_func(PS_CHECKING_METHOD, STDATA(CHECKING_CHECKSUM)))
	    user_aborted();
	}
	chk_data_len = 1;
	break;

      case 'C':
      case 'G':
	if (chk_data_len != 2) {	/* Prevents us from repeating this */
	  if (p_cfg->status_func(PS_CHECKING_METHOD, STDATA(CHECKING_CRC16)))
	    user_aborted();
	}
	chk_data_len = 2;
	break;
      }
      return;
    }
    if (++cnt == 4 && protocol_type != PROTOCOL_G && ch_to_send == 'C') {
      if (p_cfg->status_func(PS_XY_FALLBACK_TO_CHECKSUM, NULL))
	user_aborted();
      ch_to_send = NAK;
    }
    if (cnt == 12) {
      if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
	user_aborted();
      pdll_aborted = A_MISC;
      return;
    }
  }
}

VOID
#ifdef CK_ANSIC
ryx_open_file(void) 
#else
ryx_open_file()
#endif
{
  if (p_cfg->r_open_func(&path, length, date, mode,
			 pdll_files_left, pdll_bytes_left,
			 0, 0, 0, NULL))
    user_aborted();
  if (path == NULL)		/* File not specified on command-line */
				/* or something like that */
    user_aborted();
  offset = 0;
}

U32 
#ifdef CK_ANSIC
ryx_block(void) 
#else
ryx_block()
#endif /* CK_ANSIC */
{
  
  U16 checksum;
  U16 block_ok;
  U16 ch;
  U32 i;
  U32 chk_data_idx;
  time_t t_started;
  time_t t_now;
  status_args stargs;

  chk_data_idx = 3 + blk_size;
  blk_len = 3 + blk_size + chk_data_len;

  if (p_cfg->status_func(PS_PACKET_LENGTH, STDATA(blk_len)))
      user_aborted();

  while (1) {
    time(&t_started);
    i = 1;
    while (i < blk_len) {
      ch = dev_getch_buf();
      switch (ch) {
      case DEV_TIMEOUT:
	time(&t_now);
	if (t_now - t_started >= 5) {
	  if (p_cfg->status_func(PS_TIMEOUT, STDATA(5)))
	    user_aborted();
	  if (protocol_type == PROTOCOL_G) {
	    if (p_cfg->status_func(PS_G_ABORTED, NULL))
	      user_aborted();
	    pdll_aborted = A_MISC;
	    cancel();
	  } else {
	    dev_purge_inbuf();
	    dev_putch_buf(NAK);
	    dev_flush_outbuf();
	  }
	  return(1);		/* Let's try again */
	}
	break;

      default:
	blk[i++] = ch;
	break;
      }
    }
    block_ok = 1;
    if (block_ok) {
      if (blk[1] != expected_blk_num[0] &&
	  blk[2] != expected_blk_num[1]) {
	stargs.arg0 = blk[1];
	stargs.arg1 = blk[2];
	stargs.arg2 = expected_blk_num[0];
	stargs.arg3 = expected_blk_num[1];
	if (p_cfg->status_func(PS_XYG_BLK_NUM_MISMATCH, &stargs))
	    user_aborted();
	block_ok = 0;
      }
    }
    if (block_ok) {
      checksum = 0;
      switch (chk_data_len) {
      case 1:			/* 8-bit checksum */
	for (i = 3; i < chk_data_idx; i++)
	  checksum = (checksum + blk[i]) & 0xFF;
	if (blk[chk_data_idx] != checksum) {
	  if (p_cfg->status_func(PS_CHECK_FAILED, STDATA(CHECKING_CHECKSUM)))
	    user_aborted();
	  block_ok = 0;
	}
	break;
	
      case 2:			/* 16-bit CRC */
	for (i = 3; i < chk_data_idx; i++)
	  checksum = updcrc16(blk[i], checksum);
	if (blk[chk_data_idx] != ((checksum >> 8) & 0xFF) ||
	    blk[chk_data_idx + 1] != (checksum & 0xFF)) {
	  if (p_cfg->status_func(PS_CHECK_FAILED, STDATA(CHECKING_CRC16)))
	    user_aborted();
	  block_ok = 0;
	}
	break;
      }
    }
    if (block_ok) {
      if (protocol_type != PROTOCOL_G) {
	dev_putch_buf(ACK);
	dev_flush_outbuf();
      }
      expected_blk_num[0]++;
      expected_blk_num[1]--;
      break;
    } else {
      if (protocol_type == PROTOCOL_G) {
	if (p_cfg->status_func(PS_G_ABORTED, NULL))
	  user_aborted();
	pdll_aborted = A_MISC;
	cancel();
      } else {
	retransmits++;
	dev_purge_inbuf();
	dev_putch_buf(NAK);
	dev_flush_outbuf();
      }
      return(1);
    }
  }
  return(0);
}


U32 
#ifdef CK_ANSIC
ryx_file(void) 
#else
ryx_file()
#endif
{

  U32 cnt;
  U32 bytes_to_write;

  if (protocol_type != PROTOCOL_X) {	/* Let's get the file information */
    ryx_handshake();
    if (pdll_aborted)
      return(1);
    expected_blk_num[0] = 0;
    expected_blk_num[1] = 255;
    while (ryx_block()) {
      if (pdll_aborted)
	return(1); /* We can use return because no file has been opened yet */
    }
    process_file_info();
    if (path == NULL) {	 	/* Null header received, indicating */
				/* end of batch transfer */
      return(1);
    }
  } else {
    expected_blk_num[0] = 1;
    expected_blk_num[1] = 254;
    path = NULL;		/* We'll find the path information */
				/* from cfg->te in ryx_open_file() */
  }
  offset = 0;
  ryx_open_file();
  if (!pdll_aborted) {
    ryx_handshake();
    if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
      user_aborted();

    while (1) {
      if (!ryx_block()) {	/* If block received OK */
	if (length != -1 && offset + blk_size > length)   /* If we know the */
							  /* length of the */
							  /* file and we are */
							  /* at the end */
	  bytes_to_write = length - offset;
	else
	  bytes_to_write = blk_size;
	offset += bytes_to_write;
	if (p_cfg->write_func(&blk[3], bytes_to_write))
	  user_aborted();
      } else {
	if (pdll_aborted)
	  break;
      }
      cnt = 0;
      while (1) {
	if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
	  user_aborted();

	ryx_blk_size();
	if (blk[0] != '\0' || pdll_aborted)
	  break;
	cnt++;
	if (cnt == 12) {
	  if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
	    user_aborted();

	  pdll_aborted = A_MISC;
	  break;
	}
	dev_putch_buf(NAK);
	dev_flush_outbuf();
      }
	if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
	  user_aborted();
      if (blk[0] == EOT)
	break;
      if (pdll_aborted)
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
  if (pdll_aborted || protocol_type == PROTOCOL_X)
    return(1);
  else
    return(0);
}

VOID
#ifdef CK_ANSIC
ryx(void) 
#else
ryx()
#endif
{

  timeouts_per_call = 1;	/* We'll just wait 3 tenths of a second */
  pdll_malloc((void **)&blk, 3 + 1024 + 2, MODULE_RYX, __LINE__);
  while (!ryx_file())
    ;
  pdll_free((void **)&blk, MODULE_RYX, __LINE__);
}



