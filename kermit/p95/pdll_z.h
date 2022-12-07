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

_PROTOTYP( VOID z_send_byte_as_hex,(U8));
_PROTOTYP( U16 zdl_getch,(void));
extern U32 zdl_putch_last_c;
_PROTOTYP( VOID zdl_putch,(U8));
_PROTOTYP( U32 z_get_bin_header,(void));
_PROTOTYP( U32 z_get_bin32_header,(void));
_PROTOTYP( U32 zhex_getch,(void));
_PROTOTYP( U32 z_get_hex_header,(void));
_PROTOTYP( U32 z_get_header,(void));
_PROTOTYP( VOID z_send_hex_header,(U16));
_PROTOTYP( VOID z_send_data16,(void));
_PROTOTYP( VOID z_send_data32,(void));
_PROTOTYP( VOID z_send_bin16_header,(U16));
_PROTOTYP( VOID z_send_bin32_header,(U16));
_PROTOTYP( VOID zdl_add_ch,(U8));
_PROTOTYP( U32 z_get_in_sync,(void));
_PROTOTYP( VOID z_handle_zrpos,(void));
_PROTOTYP( VOID z_wait_for_ack,(void));
_PROTOTYP( VOID z_chk_response,(void));
_PROTOTYP( U32 z_recv_block,(void));
_PROTOTYP( VOID z_send_block,(void));
_PROTOTYP( VOID z_send_attn,(void));
_PROTOTYP( U32 z_calc_file_crc,(U32));
