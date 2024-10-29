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

/* Main function of P.DLL */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <setjmp.h>

#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_common.h"
#include "pdll_dev.h"
#include "pdll_defs.h"
#include "pdll_global.h"
#include "pdll_modules.h"
#include "pdll_error.h"
#include "pdll_ryx.h"
#include "pdll_rz.h"
#include "pdll_syx.h"
#include "pdll_sz.h"
#include "pdll_x_global.h"
#include "pdll_z.h"
#include "pdll_z_global.h"

jmp_buf p_jmp_buf;

static DEV_CFG dev_cfg;

VOID
#ifdef CK_ANSIC
cleanup(void) 
#else
cleanup()
#endif
{
    /* restore unprocessed characters */
    dev_pushback_buf();

    if (path != NULL) {
        if (p_cfg->close_func(&path,
			  length,
			  date,
			  retransmits,
			  FILE_FAILED,
			  offset))
            user_aborted();
    }
    if (dev_ready) {
	dev_set_cfg(&dev_cfg);
	dev_close();
    }
}

U32 _System 
#ifdef CK_ANSIC
p_transfer(P_CFG *param_p_cfg) 
#else
p_transfer() P_CFG *param_p_cfg ;
#endif
{
    pdll_aborted = 0;		/* Reason of abortation */
    user_aborted_visited = 0 ;
    skip_file = 0;
    move_file = 0 ;
    offset = 0;
    path = NULL;
    length = -1;
    date = -1;
    time_started = 0;
    retransmits = 0;
    blk_size = 0;
  
    protocol_type = 0;
    transfer_direction = 0;
    use_1k_blocks = 0;
    use_alternative_checking = 0;
    send_rz_cr = 0;

    if (setjmp(p_jmp_buf)) { 	/* If non-zero, we got here by jumping... */
	cleanup();
	return(1);
    }
    /* Initialization of dev variables */

    dev_path = NULL;
    dev_handle = 0;
    passive_socket = 0;
    socket_remote = NULL;
    socket_port = 0;

    dev_ready = 0;

    outbuf_size = 0;
    inbuf_size = 0;
    timeouts_per_call = 1000;	/* About 10 seconds */
    outbuf = NULL;
    outbuf_idx = 0;
    inbuf = NULL;
    inbuf_idx = 0;
    inbuf_len = 0;
    dev_type = 0;
    dev_opened = 0;
    dev_shared = 0;
    dev_server = 0;
    dev_telnet = 0;
    dev_telnet_u_binary = 0 ;
    dev_telnet_me_binary = 0 ;
    dev_telnet_iac_escaped = 1 ;

    watch_carrier = 0;

    /* Done */

    /* Pseudo-static function's initialization */

    user_aborted_visited = 0;
    p_error_visited = 0;

    zdl_putch_last_c = 0;

    /* Done */

    /* x_global variables initialization */

    chk_type = 0;
    can_cnt = 0;
    blk = NULL;
    blk_end = 0;
    chk_data_len = 0;
    blk_len = 0;
    sending_header = 0;

    /* Done */

    /* z_global variables initialization */

    rx_hdr_type = 0;
    tx_hdr_type = 0;
    rx_frame_type = 0;
    tx_frame_end = 0;
    zfin_recvd = 0;
    rx_frame_end = 0;
    esc_8th_bit = 0;
    getting_frame = 0;
    recovering = 0;	    /* Are we recovering from a CRC or other ERROR? */
    attn_len = 0;
    rx_buf = NULL;
    rx_buf_len = 0;
    rx_buf_size = 0;
    tx_buf = NULL;
    tx_buf_len = 0;
    tx_buf_size = 0;
    last_sync_pos = -1;		/* -1 is to prevent first block */
                                /* from being send with a ZCRCW */
    tx_wincnt = 0;
    sync_cnt = 0;
    tx_bin32 = 0;
    rx_flags = 0;
    got_zabort = 0;
    got_to_eof = 0;

    /* Done */

    p_cfg = param_p_cfg;
    /* Let's transfer parameters from P_CFG structure */
    /* to their corresponding slots */
    transfer_direction = p_cfg->transfer_direction;
    protocol_type = p_cfg->protocol_type;

    use_alternative_checking = (p_cfg->attr & CFG_ALTERNATIVE_CHECKING);
    use_1k_blocks = (p_cfg->attr & CFG_1K_BLOCKS);
    send_rz_cr = (p_cfg->attr & CFG_SEND_RZ_CR);
    dev_shared = (p_cfg->attr & CFG_SHARED_DEVICE);
    dev_server = (p_cfg->attr & CFG_DEV_SERVER);
    dev_telnet = (p_cfg->attr & CFG_DEV_TELNET);
    if ( dev_telnet )
    {
	dev_telnet_u_binary = (p_cfg->attr & CFG_DEV_TELNET_U_BINARY);
	dev_telnet_me_binary = (p_cfg->attr & CFG_DEV_TELNET_ME_BINARY);
	dev_telnet_iac_escaped = !(p_cfg->attr & CFG_DEV_TELNET_IAC_NOT_ESCAPED);
    }
    watch_carrier = (p_cfg->attr & CFG_WATCH_CARRIER);
    esc_minimal = (p_cfg->attr & CFG_ESC_MINIMAL);
    esc_control = (p_cfg->attr & CFG_ESC_CTRL);
    esc_table   = (p_cfg->attr & CFG_ESC_TABLE) && p_cfg->control_prefix_table;
    control_prefix_table = esc_table ? p_cfg->control_prefix_table : NULL ;
    esc_8th_bit = (p_cfg->attr & CFG_ESC_8TH);
    query_serial_num = (p_cfg->attr & CFG_QUERY_SERIAL_NUM);
    move_file = (p_cfg->attr & CFG_FILE_MOVE) ;
    if (p_cfg->attn_seq != NULL) {
	strncpy(attn, p_cfg->attn_seq, 31);
	attn[31] = '\0';
	attn_len = strlen(attn);
    }
    dev_type = p_cfg->dev_type;
    dev_path = p_cfg->dev_path;
#ifdef NT
    dev_handle = (HANDLE)p_cfg->dev_handle;
#else
    dev_handle = (HFILE)p_cfg->dev_handle;
#endif
    socket_remote = p_cfg->socket_host;
    socket_port = p_cfg->socket_port;

    inbuf_size = p_cfg->inbuf_size;
    outbuf_size = p_cfg->outbuf_size;
  
    blk_size = p_cfg->blk_size;

    /* Done */

    if (blk_size == 0) {		/* If not specified explicitly */
	switch (protocol_type) {
	case PROTOCOL_X:
	    blk_size = 128;
	    break;
      
	case PROTOCOL_Y:
	case PROTOCOL_G:
	    blk_size = 1024;
	    break;
      
	case PROTOCOL_Z:
	    blk_size = 0;
	    break;
	}
    }
    dev_open();
#ifdef XYZ_DLL
    if ( dev_type == DEV_TYPE_ASYNC ){
        dev_get_cfg(&dev_cfg);
        dev_chg_cfg(1000,		/* write timeout, used to be 300 */ 
                     2,		/* read timeout */
                     dev_cfg.flags1,
                     dev_cfg.flags2,
                     (dev_cfg.flags3 & 248) | 2); 
        /* Write timeout processing and */
        /* Normal read timeout processing */
    }
#endif /* XYZ_DLL */

    if (dev_server)
	server_connect();

    switch (transfer_direction) {
    case DIR_SEND:
	switch (protocol_type) {
	case PROTOCOL_X:
	case PROTOCOL_Y:
	case PROTOCOL_G:
	    syx();
	    break;
      
	case PROTOCOL_Z:
	    sz();
	    break;
	}
	break;

    case DIR_RECV:
	switch (protocol_type) {
	case PROTOCOL_X:
	case PROTOCOL_Y:
	case PROTOCOL_G:
	    ryx();
	    break;

	case PROTOCOL_Z:
	    rz();
	    break;
	}
	break;
    }
    if (!pdll_aborted) {
	if (p_cfg->status_func(PS_TRANSFER_DONE))
	    user_aborted();
    } else if (pdll_aborted == A_REMOTE) {
	if (p_cfg->status_func(PS_REMOTE_ABORTED))
	    user_aborted();
    }
    cleanup();
    if ( pdll_aborted )
	return(-1);
    else
	return(0);
}
