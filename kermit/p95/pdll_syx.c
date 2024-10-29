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

/* Routines for sending with X or Ymodem protocol */

#include <stdio.h>
#include <time.h>

#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_common.h"
#include "pdll_crc.h"
#include "pdll_defs.h"
#include "pdll_dev.h"
#include "pdll_omalloc.h"
#include "pdll_global.h"
#include "pdll_modules.h"
#include "pdll_s.h"
#include "pdll_syx.h"
#include "pdll_x_global.h"

VOID
#ifdef CK_ANSIC
syx_start(void) 
#else
syx_start()
#endif
{

  static U16 ch;
  static time_t time_started;
  static time_t time_now;

  can_cnt = 0;
  time(&time_started);
  dev_flush_outbuf();		/* Ymodem-g needs this */
  while (1) {
    ch = dev_getch_buf();
    switch (ch) {
    case NAK:
      if (chk_type != CHK_SUM) {	/* Prevents us from repeating this */
	if (p_cfg->status_func(PS_CHECKING_METHOD, STDATA(CHECKING_CHECKSUM)))
	  user_aborted();
      }
      chk_type = CHK_SUM;
      return;

    case 'C':
    case 'G':
      if (chk_type != CHK_CRC16) { /* Prevents us from repeation this */
	if (p_cfg->status_func(PS_CHECKING_METHOD, STDATA(CHECKING_CRC16)))
	  user_aborted();
      }
      chk_type = CHK_CRC16;
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
    time(&time_now);
    if (time_now - time_started >= 60) {
      if (p_cfg->status_func(PS_TIMEOUT, STDATA(60)))
	user_aborted();
      pdll_aborted = A_MISC;
      return;
    }
  }
}

VOID
#ifdef CK_ANSIC
syx_end(void) 
#else
syx_end()
#endif
{

  static U16 ch;
  static U32 cnt;
  static time_t t_started;
  static time_t t_now;

  cnt = 10;	/* 10 is part of XMODEM specification */
  can_cnt = 0;
  while (1) {
    dev_putch_buf(EOT);
    dev_flush_outbuf();

    time(&t_started);
    while (1) {
      ch = dev_getch_buf();
      switch (ch) {
      case ACK:
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
      if (t_now - t_started >= 10)
	break;			/* Bail out from the loop */
    }
    if (--cnt == 0)
      return;
  }
}

VOID
#ifdef CK_ANSIC
syx_null_header(void) 
#else
syx_null_header()
#endif
{

  static U32 i;

  blk[1] = 0;
  blk[2] = 255;
  for (i = 3; i < blk_end; i++)
    blk[i] = '\0';
  sending_header = 1;
  syx_block();
  sending_header = 0;
  if (pdll_aborted)
    return;
}

VOID
#ifdef CK_ANSIC
syx_header(void) 
#else
syx_header()
#endif
{

  static U32 i;

  blk[1] = 0;
  blk[2] = 255;
  i = 3;
  i += s_make_file_header(&blk[i]);
  sending_header = 1;
  syx_block();
  sending_header = 0;
  if (pdll_aborted)
    return;
  syx_start();
}

VOID 
#ifdef CK_ANSIC
syx_block(void) 
#else
syx_block()
#endif
{

  static U8 bail;
  static U16 ch;
  static U16 checksum;
  static U32 i;
  static U32 this_block_retransmitted;
  static time_t t_getting_ack_started;
  static time_t t_sending_blk_started;
  static time_t t_now;

  checksum = 0;
  i = 0;			/* Just to shut up the compiler */
  this_block_retransmitted = 0;
  time(&t_sending_blk_started);
  while (1) {
    if (!sending_header) {		/* If not file info block */
      if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
	user_aborted();
    }
    if (p_cfg->status_func(PS_PACKET_LENGTH, STDATA(blk_size)))
	  user_aborted();

    switch (blk_size) {
    case 128:
      dev_putch_buf(SOH);
      break;
      
    case 1024:
      dev_putch_buf(STX);
      break;
    }
    dev_putch_buf(blk[1]);	/* Block numbers */
    dev_putch_buf(blk[2]);

    switch (chk_type) {
    case CHK_CRC16:
      for (i = 3; i < blk_end; i++) {
	dev_putch_buf(blk[i]);
	checksum = updcrc16(blk[i], checksum);
      }
      dev_putch_buf((checksum >> 8) & 0xFF);
      dev_putch_buf(checksum & 0xFF);
      break;
      
    case CHK_SUM:
      for (i = 3; i < blk_end; i++) {
	dev_putch_buf(blk[i]);
	checksum = (checksum + blk[i]) & 0xFF;
      }
      dev_putch_buf(checksum);
      break;
    }
    if (protocol_type == PROTOCOL_G) {
      /***********************************************/
      /* To prevent us from eating the 'G' character */
      /* transmitted after header blocks             */
      /***********************************************/
      if (!sending_header) {
	while (dev_incoming()) {
	  ch = dev_getch_buf();
	  switch (ch) {
	  case CAN:
	    can_cnt++;
	    if (can_cnt == 5) {
	      pdll_aborted = A_REMOTE;
	      return;
	    }
	    break;

	  default:
	    can_cnt = 0;
	    break;
	  }
	}
      }
      break;			/* Bail out the loop */
    } else {			/* Get an acknowledge for the packet sent */
      dev_flush_outbuf();	/* We'll have to do this to get acknowledged */
      time(&t_getting_ack_started);
      bail = 0;
      can_cnt = 0;
      while (!bail) {
	ch = dev_getch_buf();
	switch (ch) {
	case ACK:
	  bail = 1;
	  break;

	case NAK:
	  if (p_cfg->status_func(PS_XYG_NAK, STDATA(offset)))
	    user_aborted();
	  bail = 1;
	  break;
	  
	case CAN:
	  can_cnt++;
	  if (can_cnt == 5) {
	    pdll_aborted = A_REMOTE;
	    bail = 1;
	  }
	  break;

	case DEV_TIMEOUT:
	  break;
	  
	default:
	  can_cnt = 0;
	  break;
	}
	if (!bail) {
	  time(&t_now);
	  if (t_now - t_getting_ack_started >= 10) {
	    if (p_cfg->status_func(PS_TIMEOUT, STDATA(10)))
	      user_aborted();
	    bail = 1;
	  }
	}
      }
    }
    if (ch == ACK || pdll_aborted)
      break;
    this_block_retransmitted++;
    time(&t_now);
    if (t_now - t_sending_blk_started >= 120) { /* So that we won't loop */
						/* forever */
      if (p_cfg->status_func(PS_CANNOT_SEND_BLOCK, NULL))
	user_aborted();
      pdll_aborted = A_MISC;
      break;
    }
  }
    if (p_cfg->status_func(PS_PROGRESS, STDATA(offset)))
	user_aborted();
  retransmits += this_block_retransmitted;
  blk[1]++;
  blk[2]--;
}

VOID 
#ifdef CK_ANSIC
syx_file(void) 
#else
syx_file()
#endif
{

  static U32 i;
  static U32 rw_ret;

  syx_start();
  if (pdll_aborted)
    return;
  if (path == NULL) {		/* No more files to send */
    syx_null_header();
    dev_flush_outbuf();		/* Ymodem-g needs this */
    return;
  }
  offset = 0;
  retransmits = 0;

  if (protocol_type == PROTOCOL_X) {	/* X doesn't have headers */
    blk[1] = 1;
    blk[2] = 254;
  } else
    syx_header();
  
  if (!pdll_aborted) {
    while (1) {
      if (p_cfg->read_func(&blk[3], blk_size, &rw_ret))
	user_aborted();
      /************************************/
      /* Fill the rest of block with EOFs */
      /************************************/
      for (i = 3 + rw_ret; i < blk_end; i++)
	blk[i] = 0x1A;	/* EOF */
      syx_block();
      if (pdll_aborted)
	break;
      offset += rw_ret;
      if (rw_ret < blk_size)
	break;
    }
  }
  if (!pdll_aborted)
    syx_end();
}

VOID 
#ifdef CK_ANSIC
syx(void) 
#else
syx()
#endif
{

  timeouts_per_call = 10;	/* We'll just wait about 3 seconds */
  if (use_1k_blocks)
    blk_size = 1024;
  else
    blk_size = 128;

  blk_end = 3 + blk_size;
  pdll_malloc((void **)&blk, 3 + blk_size + 2, MODULE_SYX, __LINE__);

  while (1) {
    length = -1;
    date = -1;
    if (p_cfg->s_open_func(&path, &length, (U32 *)&date, &mode,
			   &pdll_files_left, &pdll_bytes_left,
			   NULL, NULL, NULL))
      user_aborted();
    if (protocol_type == PROTOCOL_X && path == NULL) /* No more files and */
						     /* doing X? */
      break;
    syx_file();
    if (pdll_aborted || path == NULL) /* Aborted or no more files? */
      break;
    if (path != NULL) {
      if (p_cfg->close_func(&path,
			    length,
			    date,
			    retransmits,
			    pdll_aborted ? FILE_FAILED : FILE_SUCCESSFUL,
			    offset))
	user_aborted();
    }
  }
  pdll_free((void **)&blk, MODULE_SYX, __LINE__);
}
