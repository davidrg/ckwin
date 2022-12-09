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

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <time.h>
#include "p.h"

#define CHK_SUM		0x01
#define CHK_CRC16	0x02

#define FILE_FAILED	0
#define FILE_SUCCESSFUL	1

extern P_CFG *p_cfg;
extern U8 pdll_aborted;
extern U8 skip_file;
extern U32 move_file;
extern U32 offset;
extern U8 *path;
extern U32 length;
extern time_t date;
extern U32 mode;
extern U32 pdll_files_left;
extern U32 pdll_bytes_left;
extern U32 time_started;
extern U32 retransmits;
extern U32 blk_size;

extern U32 protocol_type;
extern U32 transfer_direction;

extern U32 use_1k_blocks;
extern U32 use_alternative_checking;
extern U32 send_rz_cr;

#endif /* _GLOBAL_H_ */
