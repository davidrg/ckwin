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


_PROTOTYP( U8 *d_time, (U32));
#define MSG_NONE        0
#define MSG_CR          1
#define MSG_LF          2
_PROTOTYP( VOID msg, (U32, U8 *, ...));
_PROTOTYP( VOID wait_for_keypress, (void));
_PROTOTYP( VOID make_noise, (void));
_PROTOTYP( VOID add_recv_dir_to_path, (U8 **));
_PROTOTYP( U32 get_dir_len, (U8 *));
_PROTOTYP( VOID strip_drive_and_dir, (U8 *));
_PROTOTYP( U32 create_dirs, (U8 *));
_PROTOTYP( VOID set_priority, (U32, U32));
_PROTOTYP( VOID install_interrupt_handler, (U8));
_PROTOTYP( VOID restore_interrupt_handler, (void));
