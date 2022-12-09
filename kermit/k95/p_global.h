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


#include <time.h>
#include "p_brw.h"
#include "p_tl.h"
#include "p.h"

/* See global.c for comments */

extern TL *tl;
extern U8 *full_path;
extern FILE *dszlog_stream;

extern BOOLEAN aborted;
extern BOOLEAN we_aborted;
extern BOOLEAN carrier_lost;
extern BRWF *brwf;
extern P_CFG p_cfg;
extern time_t t_started;
extern U32 files_left;
extern U32 bytes_left;
extern S32 remote_serial_num;

extern BOOLEAN opt_paths;
extern BOOLEAN opt_clean;
extern BOOLEAN opt_resume;
extern BOOLEAN opt_rename;
extern BOOLEAN opt_headers;
extern BOOLEAN opt_frameends;
extern BOOLEAN opt_touch;
extern BOOLEAN opt_quiet;
extern BOOLEAN opt_text;
extern BOOLEAN opt_existing;
extern BOOLEAN opt_mileage;
extern BOOLEAN opt_options;
extern BOOLEAN opt_recursive;
extern BOOLEAN opt_create;
extern U8 opt_management;
extern U32 opt_wait;
extern U8 *opt_dszlog;
extern U8 *opt_directory;
extern U32 opt_filebuf;
extern U32 opt_speed;
extern U8 *opt_note;
extern int savfnc ;

/* Receiver Status variables */
extern U32 checking_method;
extern U32 receiver_flags;
extern S32 receiver_window_size;


