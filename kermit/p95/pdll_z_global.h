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

extern U8 rx_hdr[16];
extern U8 tx_hdr[16];
extern U8 rx_hdr_type;
extern U8 tx_hdr_type;
extern U8 rx_frame_type;
extern U8 tx_frame_end;
extern U8 rx_frame_end;
extern U8 attn[32];
extern U32 attn_len;
extern U8 *rx_buf;
extern U32 rx_buf_len;
extern U32 rx_buf_size;
extern U8 *tx_buf;
extern U32 tx_buf_len;
extern U32 tx_buf_size;
extern U32 last_sync_pos;
extern U32 tx_wincnt;
extern U32 sync_cnt;
extern U8 tx_bin32;
extern U32 rx_flags;
extern U8 local_zconv;
extern U8 local_zmanag;
extern U8 local_ztrans;
extern U8 remote_zconv;
extern U8 remote_zmanag;
extern U8 remote_ztrans;

extern U32 got_zabort;
extern U32 got_to_eof;
extern U32 zfin_recvd;
extern U32 esc_control;
extern U32 esc_8th_bit;
extern U32 esc_minimal;
extern U32 esc_table ;
extern U16 * control_prefix_table ;
extern U32 query_serial_num;
extern U32 getting_frame;
extern U32 recovering;
