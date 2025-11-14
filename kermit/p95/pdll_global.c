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

/* Global variables used by X, Y and Zmodem procotols */

#ifdef NT
#include <windows.h>
#endif 
#include <stdio.h>
#include <time.h>
#include "p_type.h"
#include "p.h"

P_CFG *p_cfg;
U8 pdll_aborted = 0;		/* Reason of abortation */
U8 skip_file = 0;
U32 move_file = 0 ;
U32 offset = 0;
U8 *path = NULL;
U32 length = -1;
time_t date = -1;
U32 mode = 0;
U32 pdll_files_left = 0;
U32 pdll_bytes_left = 0;
U32 time_started = 0;
U32 retransmits = 0;
U32 blk_size = 0;

U32 protocol_type = 0;
U32 transfer_direction = 0;
U32 use_1k_blocks = 0;
U32 use_alternative_checking = 0;
U32 send_rz_cr = 0;

#ifdef NT
OVERLAPPED overlapped_read = {0L,0L,0L,0L,(HANDLE)-1};
U32 nActuallyRead = 0;
OVERLAPPED overlapped_write = {0L,0L,0L,0L,(HANDLE)-1};
U32 nActuallyWritten = 0;
#endif /* NT */

