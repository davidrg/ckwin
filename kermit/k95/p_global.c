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


/*
   Global variables
*/

#include "ckcdeb.h"
#ifndef NOXFER
#include <stdio.h>
#include <time.h>

#ifdef NT
#include <windows.h>
#endif

#include "p_type.h"
#include "p_tl.h"
#include "p_brw.h"
#include "p.h"

TL *tl = NULL;                  /* A linked list of files given */
                                /* on the command-line */
U8 *full_path = NULL;           /* Used to save temporarily path of the */
                                /* file being sent. Needed to make proper */
                                /* DSZLOG */
FILE *dszlog_stream = NULL;     /* If non-NULL, a DSZLOG will be written */
                                /* to this stream. */

BOOLEAN aborted = 0;            /* Set non-zero when user wants to abort */
BOOLEAN we_aborted = 0;         /* Set non-zero when we have handled the */
                                /* local abortion request */
BOOLEAN carrier_lost = 0;       /* Set non-zero when detected a loss */
                                /* of carrier signal */
BRWF *brwf = NULL;              /* BRWF handle to the file currently being */
                                /* transferred. */
P_CFG p_cfg;                    /* Configuration to be passed to the DLL */
time_t t_started;               /* Current time is saved to here when */
                                /* opening a file to be transferred. Used to */
                                /* calculate the CPS rates. */
U32 files_left = 0;             /* Number of files left to transfer */
U32 bytes_left = 0;             /* Number of bytes left to transfer */
S32 remote_serial_num = -1;     /* The remote's serial number */

/* Things got from the command-line */

BOOLEAN opt_paths = 0;
BOOLEAN opt_clean = 0;
BOOLEAN opt_resume = 0;
BOOLEAN opt_rename = 0;
BOOLEAN opt_headers = 0;
BOOLEAN opt_frameends = 0;
BOOLEAN opt_touch = 0;
BOOLEAN opt_quiet = 0;
BOOLEAN opt_text = 0;
BOOLEAN opt_existing = 0;
BOOLEAN opt_mileage = 0;
BOOLEAN opt_options = 0;
BOOLEAN opt_recursive = 0;
BOOLEAN opt_create = 0;
U8 opt_management = 0;
U32 opt_wait = 0;
U8 *opt_dszlog = NULL;
U8 *opt_directory = NULL;
U32 opt_filebuf = 0;
U32 opt_speed = 0;
U8 *opt_note = NULL;

int savfnc = 0 ;

/* originally statics from status_func */
U32 checking_method = 0;
U32 receiver_flags = 0;
S32 receiver_window_size = -1;


#endif /* NOXFER */
