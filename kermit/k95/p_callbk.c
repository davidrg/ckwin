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
   Callback functions called by P.DLL
*/

#include "ckcdeb.h"
#ifndef NOXFER
#ifdef XYZ_INTERNAL
#include "ckcker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <share.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

#ifdef OS2
#ifdef NT
#include <windows.h>
#else
#define INCL_DOSPROCESS
#include <os2.h>
#undef COMMENT
#endif
#include "ckocon.h"
#endif /* OS2 */

#ifdef NT
#define msleep Sleep
#define DosBeep Beep
#ifndef __WATCOMC__
#define filelength _filelength
#define fstat _fstat
#define unlink _unlink
#define stat _stat
#endif /* __WATCOMC__ */
#endif /* NT */

extern int keep, moving;                /* fdc */
extern char * cmarg, * cmarg2;                  /* fdc */
extern char filnam[], *srvcmd;  /* jaltman */
extern int nfils, fnspath, fncnv, binary, sendmode, sndsrc ;
extern long sendstart ;
extern char *zinptr, *zoutptr;
extern int zincnt, zoutcnt, fncact;
extern long ffc, filcnt, fsize, cps, oldcps ;
extern int fsecs, tsecs, rejection, timeouts, crunched, retrans ;
extern int savfnc, cxseen, czseen, discard ;
#ifdef GFTIMER
extern CKFLOAT fpfsecs, fpxfsecs;
extern CKFLOAT gtv, oldgtv;
#else
extern int  xfsecs;
extern long gtv, oldgtv;
#endif /* GFTIMER */
#include "p_type.h"
#include "p.h"
#include "p_callbk.h"
#include "p_common.h"
#ifdef COMMENT
#include "p_brw.h"
#endif
#include "p_error.h"
#include "p_global.h"
#include "p_module.h"
#include "p_omalloc.h"

#ifdef PIPESEND
#undef PIPESEND
#endif /* PIPESEND */
#ifdef PIPESEND
extern int pipesend, usepipes;
#endif /* PIPESEND */

static U8  msgbuf[512] ;

U8 *z_header[] = {

  "TIMEOUT",
  "ZRQINIT",
  "ZRINIT",
  "ZSINIT",
  "ZACK",
  "ZFILE",
  "ZSKIP",
  "ZNAK",
  "ZABORT",
  "ZFIN",
  "ZRPOS",
  "ZDATA",
  "ZEOF",
  "ZFERR",
  "ZCRC",
  "ZCHALLENGE",
  "ZCOMPL",
  "ZCAN",
  "ZFREECNT",
  "ZCOMMAND",
  "ZSTDERR"
};

U8 *z_frame_end[] = {

  "TIMEOUT",
  "ZCRCE",
  "ZCRCG",
  "ZCRCQ",
  "ZCRCW",
  "ZERROR",
  "ZCAN"
};

static char errbuf[512];

U32
CKDEVAPI
#ifdef CK_ANSIC
status_func(U32 type, status_args *stargs)
#else
status_func(type,stargs) U32 type; status_args *stargs;
#endif
{
    switch (type) {
    case PS_ERROR:
        {
            sprintf(errbuf,"Module: %d, Line %d, Device: %s, Error: %d",
                     stargs->arg2,
                     stargs->arg3,
                     stargs->arg4?(char *)stargs->arg4:"<NULL>",
                     stargs->arg0);
            debug(F111,"P ERROR",errbuf,stargs->arg1);
        }
        if (is_os2_error(stargs->arg0)) {
            os2_error(stargs->arg0,            /* Error num */
                      stargs->arg1,            /* Return code */
                      stargs->arg2,            /* Module */
                      stargs->arg3,            /* Line num */
                      stargs->arg4);           /* Optional argument */
        } else if (is_tcpip_error(stargs->arg0)) {
            tcpip_error(stargs->arg0,          /* Error num */
                      stargs->arg1,            /* Return code */
                      stargs->arg2,            /* Module */
                      stargs->arg3,            /* Line num */
                      stargs->arg4);           /* Optional argument */
        }
        break;

    case PS_CARRIER_LOST:
        carrier_lost = 1;
        ckscreen(SCR_ST,ST_ERR,0l,"Carrier lost");
        break;

    case PS_TIMEOUT:
        timeouts++ ;
        sprintf(msgbuf,"Timeout (%lu secs)", STVAL(stargs));
        ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        debug(F111,"P status_func","TIMEOUT", STVAL(stargs));
        break;

    case PS_TRANSFER_DONE:
        /* Do not use ST_OK as that is called in clsif() */
        ckscreen(SCR_ST,ST_MSG,0l, "Transfer done!");
        debug(F110,"P status_func","TRANSFER DONE",0);
        break;

    case PS_PROGRESS:
        ffc = STVAL(stargs) ;
        ckscreen(SCR_PT,'D', STVAL(stargs)," characters so far");
        break;

    case PS_CANNOT_SEND_BLOCK:
        ckscreen(SCR_ST,ST_ERR,0l, "Can't send block");
        break;

    case PS_CHECKING_METHOD:
        if (checking_method != STVAL(stargs)) { /* Has the checking method */
            /* changed since last displayed? */
            checking_method = STVAL(stargs);
            switch (checking_method) {
            case CHECKING_CHECKSUM:
                ckscreen(SCR_ST,ST_MSG,0l,"Checksum checking will be used");
                break;

            case CHECKING_CRC16:
                ckscreen(SCR_ST,ST_MSG,0l, "CRC-16 checking will be used");
                break;

            case CHECKING_CRC32:
            default:                    /* To shut up the compiler */
                ckscreen(SCR_ST,ST_MSG,0l, "CRC-32 checking will be used");
                break;
            }
        }
        break;

    case PS_INVALID_FILE_INFO:
        crunched++ ;
        ckscreen(SCR_PT,'Q',0L,"");
        ckscreen(SCR_ST,ST_MSG,0l, "Got invalid file info");
        break;

    case PS_NON_STD_FILE_INFO:
        ckscreen(SCR_ST,ST_MSG,0l, "Got non-standard file info");
        break;

    case PS_XY_FALLBACK_TO_CHECKSUM:
        ckscreen(SCR_ST,ST_MSG,0l, "Falling back to checksum checking...");
        break;

    case PS_CHECK_FAILED:
        switch (STVAL(stargs)) {
        case CHECKING_CHECKSUM:
            crunched++ ;
            ckscreen(SCR_PT,'Q',0L,"");
            ckscreen(SCR_ST,ST_ERR,0l, "Checksum mismatch");
            break;

        case CHECKING_CRC16:
            crunched++ ;
            ckscreen(SCR_PT,'Q',0L,"");
            ckscreen(SCR_ST,ST_ERR,0l, "CRC-16 mismatch");
            break;

        case CHECKING_CRC32:
            crunched++ ;
            ckscreen(SCR_PT,'Q',0L,"");
            ckscreen(SCR_ST,ST_ERR,0l, "CRC-32 mismatch");
            break;
        }
        break;

    case PS_REMOTE_ABORTED:
        ckscreen(SCR_ST,ST_ERR,0l, "Remote canceled");
        break;

    case PS_G_ABORTED:
        ckscreen(SCR_ST,ST_ERR,0l, "Cancelling Ymodem-g transfer");
        break;

    case PS_XYG_NAK:
        sprintf(msgbuf,"Got NAK on byte %lu", STVAL(stargs) );
        ckscreen(SCR_ST,ST_ERR,0l,msgbuf);
        ckscreen(SCR_PT,'N',0L,"");
        break;

    case PS_XYG_BLK_NUM_MISMATCH:
        ckscreen(SCR_PT,'E',0L,"");
        crunched++ ;
        sprintf(msgbuf, "Block numbers mismatch (%lu:%lu <> %lu:%lu)",
             stargs->arg0, stargs->arg1,
             stargs->arg2, stargs->arg3);
        ckscreen(SCR_ST,ST_ERR,0l,msgbuf);
        break;

    case PS_Z_HEADER:
        if (opt_headers) {
           sprintf(msgbuf,"%s %lu",
                 z_header[stargs->arg0], stargs->arg1);
            ckscreen(SCR_ST,ST_MSG,0l, msgbuf ) ;
        }
        break;

    case PS_Z_UNEXPECTED_HEADER:
        ckscreen(SCR_PT,'E',0L,"");
        sprintf(msgbuf,"Unexpected %s %lu",
             z_header[stargs->arg0], stargs->arg1);
        ckscreen(SCR_ST,ST_ERR,0l, msgbuf);
        break;

    case PS_Z_FRAME_END:
        if (opt_frameends)
          ckscreen(SCR_ST,ST_MSG,0l, z_frame_end[STVAL(stargs)]);
        break;

    case PS_Z_INVALID_FRAME_END:
        crunched++ ;
        ckscreen(SCR_PT,'E',0L,"");
        sprintf( msgbuf, "Invalid frame end: %s",
             z_frame_end[STVAL(stargs)]);
        ckscreen(SCR_ST,ST_ERR,0l, msgbuf );
        break;

    case PS_Z_PHONY_ZEOF:
        ckscreen(SCR_PT,'E',0L,"");
        ckscreen(SCR_ST,ST_ERR,0l, "Got phony ZEOF");
        break;

    case PS_Z_RETRY_CNT_EXCEEDED:
        ckscreen(SCR_PT,'E',0L,"");
        ckscreen(SCR_ST,ST_ERR,0l, "Retry count exceeded");
        break;

    case PS_Z_DATA_FROM_INVALID_POS:
        crunched++ ;
        ckscreen(SCR_PT,'Q',0L,"");
        sprintf(msgbuf, "Got data from invalid position: %lu, expected from %lu",
             stargs->arg0, stargs->arg1);
        ckscreen(SCR_ST,ST_ERR,0l,msgbuf);
        break;

    case PS_Z_COMMAND:
        sprintf(msgbuf, "Zcommand: \"%s\"", STPTR(stargs));
        ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        break;

    case PS_Z_CTRL_CHAR_IGNORED:
        sprintf(msgbuf, "Unexpected control character ignored: %lu",
             STVAL(stargs));
        ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        break;

    case PS_Z_INVALID_ZDLE_SEQUENCE:
        ckscreen(SCR_PT,'Q',0L,"");
        ckscreen(SCR_ST,ST_ERR,0l, "Invalid ZDLE sequence received");
        break;

    case PS_Z_CHECK_FAILED_FOR_HEADER:
        switch (STVAL(stargs)) {
        case CHECKING_CHECKSUM:
            /* This never happens... Checksum checking isn't used for headers! */
            break;

        case CHECKING_CRC16:
            crunched++ ;
            ckscreen(SCR_PT,'Q',0L,"");
            ckscreen(SCR_ST,ST_ERR,0l, "CRC-16 mismatch for a header");
            break;

        case CHECKING_CRC32:
            crunched++ ;
            ckscreen(SCR_PT,'Q',0L,"");
            ckscreen(SCR_ST,ST_ERR,0l, "CRC-32 mismatch for a header");
            break;
        }
        break;

    case PS_Z_INVALID_HEX_HEADER:
        crunched++ ;
        ckscreen(SCR_PT,'Q',0L,"");
        ckscreen(SCR_ST,ST_ERR,0l, "Invalid zmodem hex header received");
        break;

    case PS_Z_SUBPACKET_TOO_LONG:
        crunched++ ;
        ckscreen(SCR_PT,'Q',0L,"");
        sprintf(msgbuf, "Too long zmodem subpacket received (> %lu)",
            STVAL(stargs));
        ckscreen(SCR_ST,ST_ERR,0l,msgbuf);
        break;

    case PS_Z_CRASH_RECOVERY:
        sprintf(msgbuf, "Crash recovery at %lu", STVAL(stargs));
        ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        debug(F111,"P status_func","CRASH RECOVERY", STVAL(stargs));
        break;

    case PS_Z_RECEIVER_FLAGS:
        if (receiver_flags != STVAL(stargs)) {
            if (receiver_flags != 0)    /* We have parsed zrinit */
              /* at least once before */
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver has changed its parameters");
            receiver_flags = STVAL(stargs);

            if (receiver_flags & RZ_FLAG_CANFDX)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver is capable of true full duplex");
            if (receiver_flags & RZ_FLAG_CANOVIO)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver can receive data during disk I/O");
            if (receiver_flags & RZ_FLAG_CANBRK)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver can send break signal");
            if (receiver_flags & RZ_FLAG_CANCRY)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver can decrypt");
            if (receiver_flags & RZ_FLAG_CANLZW)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver can uncompress");
            if (receiver_flags & RZ_FLAG_CANFC32) {
                ckscreen(SCR_ST,ST_MSG,0l, "Receiver can use 32-bit frame checking");
                if (p_cfg.attr & CFG_ALTERNATIVE_CHECKING)
                  ckscreen(SCR_ST,ST_MSG,0l, "Our parameters override, 16-bit frame checking will be used");
            }
            if (receiver_flags & RZ_FLAG_ESC_CTRL)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver wants control characters to be escaped");
            if (receiver_flags & RZ_FLAG_ESC_8TH)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver wants 8th bit to be escaped");
        }
        break;

    case PS_Z_RECEIVER_WINDOW_SIZE:
        if (receiver_window_size != STVAL(stargs)) {
            if (receiver_window_size != -1)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver has changed its window parameters");
            receiver_window_size = STVAL(stargs);
            if (receiver_window_size == 0)
              ckscreen(SCR_ST,ST_MSG,0l, "Receiver can accept full streaming");
            else
            {
               sprintf(msgbuf, "Receiver wants a frame window of %lu bytes to be used", receiver_window_size);
               ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
            }
            if (p_cfg.blk_size &&
                 p_cfg.blk_size != receiver_window_size)
            {
               sprintf(msgbuf,  "Our parameters override, a frame window of %lu bytes will be used", p_cfg.blk_size);
               ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
            }
        }
        break;

    case PS_Z_SENDER_FLAGS:
        if (STVAL(stargs) & RZ_FLAG_ESC_CTRL)
          ckscreen(SCR_ST,ST_MSG,0l, "Sender wants control characters to be escaped");
        if (STVAL(stargs) & RZ_FLAG_ESC_8TH)
          ckscreen(SCR_ST,ST_MSG,0l, "Sender wants 8th bit to be escaped");
        break;

    case PS_SERVER_WAITING:
        if (STVAL(stargs) == opt_wait) {
            if (opt_wait) {
                ckscreen(SCR_PT,'T',0L,"");
                ckscreen(SCR_ST,ST_ERR,0l, "Timeout");
                DosBeep(750, 1000);
            } else
                ckscreen(SCR_ST,ST_ERR,0l, "No connection, try specifying a waiting time (with -wait option)");
            aborted = 1;
        } else {
            sprintf(msgbuf, "Waiting for connect (%lu secs)", STVAL(stargs) + 1);
            ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
            if (!opt_quiet)
                DosBeep(250, 20);
            msleep(1000);
        }
        break;

    case PS_FILE_SKIPPED:
        ckscreen(SCR_ST,ST_MSG,0l, "File skipped by receiver request");
        break;

    case PS_Z_SERIAL_NUM:
        if (p_cfg.attr & CFG_QUERY_SERIAL_NUM) /* Let's not show it, if not */
                                           /* explicitly asked to */
        {
            sprintf(msgbuf, "Serial number of the receiver is %lu", STVAL(stargs));
            ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        }
        remote_serial_num = STVAL(stargs);
        break;

    case PS_PACKET_LENGTH:
        break;

    default:
        sprintf(msgbuf, "Got unknown P_STATUS: %lu", type);
        ckscreen(SCR_ST,ST_ERR,0l,msgbuf);
        break;
    }
    if (aborted) {              /* User has pressed CTRL-C */
        we_aborted = 1;
        aborted = 0;            /* Once is enough */
        debug(F110,"P status_func","User aborted",0);
        return(1);
    }
    return(0);
}

U32
CKDEVAPI
#ifdef CK_ANSIC
s_open_func(U8 **path, U32 *length, U32 *date, U32 *mode,
                    U32 *f_left, U32 *b_left,
                    U8 *zconv, U8 *zmanag, U8 *ztrans)
#else
s_open_func(path,length,date,mode,f_left,b_left,zconv,zmanag,ztrans)
     U8 **path; U32 *length; U32 *date; U32 *mode;
     U32 *f_left; U32 *b_left;
     U8 *zconv; U8 *zmanag; U8 *ztrans;
#endif
{
    struct stat statbuf;
    U8 convert = 0 ;
    extern long rs_len ;

    debug(F100,"s_open_func","",0);

    rtimer();                       /* Reset the elapsed seconds timer. */
#ifdef GFTIMER
    rftimer();
#endif /* GFTIMER */

    if (tl->c == NULL) {
        debug(F100,"s_open_func tl->c == NULL","",0);
        *path = NULL;
        return(0);
    }

    debug(F101,"s_open_func tl->c->convert","",tl->c->convert);
    debug(F110,"s_open_func tl->c->path",tl->c->path,0);
    debug(F110,"s_open_func tl->c->as_name",tl->c->as_name,0);

    p_omalloc( (void**)path, 4096, MODULE_CALLBACK, __LINE__ ) ;

    full_path = tl->c->path ;
    binary = !tl->c->convert ; /* set the file transfer mode for this file */
    convert = tl->c->convert ;
    nzltor(tl->c->as_name ? tl->c->as_name : tl->c->path,
            *path,fncnv,fnspath,4095);
    tl->c = tl->c->n ; /* advance to next file */

    debug(F110,"s_open_func",full_path,0);      /* Log debugging info */
    debug(F110," (*path)",(*path),0);
    sndsrc = 1 ;                       /* We aren't using STDIO */
    ckscreen(SCR_FN,0,0l,full_path);
    if (openi(full_path) == 0) {        /* Try to open the input file */
        debug(F110,"P open_func","Unable to open input file",0);
        return(1);
    }

    ckscreen(SCR_AN,0,0l,*path);
    tlog(F110,"Sending",full_path,0);
    tlog(F110," as",*path,0);
    if ( binary )
        tlog(F101," mode: binary","",(long) binary);
    else
        tlog(F110," mode: text","",0);

#ifdef CK_RESEND
    if (sendmode == SM_PSEND)   /* PSENDing? */
      if (sendstart > 0L)               /* Starting position */
        if (zfseek(sendstart) < 0)      /* seek to it... */
          return(0);
#endif /* CK_RESEND */

#ifdef PIPESEND
    if ( pipesend )
        *length = 0;
    else
#endif /* PIPESEND */
    *length = zchki(full_path);
    fsize = *length ;
    ckscreen(SCR_FS,0,*length,"");

#ifdef UNIX
    stat(full_path, &statbuf);
    *date = statbuf.st_mtime;
    *mode = statbuf.st_mode;
#else
#ifdef OS2
    stat(full_path, &statbuf);
    *date = statbuf.st_mtime;
    *mode = statbuf.st_mode;
#else
    NEED TO ADD THIS CODE  ;
#endif
#endif
    *f_left = files_left;
    *b_left = bytes_left;

    if (p_cfg.protocol_type == PROTOCOL_Z) {
        /**********************/
        /* Conversion options */
        /**********************/
        *zconv = 0;
        if (convert)
          *zconv |= Z_CONVERSION_TEXT;
        else
          *zconv |= Z_CONVERSION_BINARY;

        if (opt_resume)
          *zconv = Z_CONVERSION_RESUME;

        /**********************/
        /* Management options */
        /**********************/
        *zmanag = 0;
        if (opt_existing)
          *zmanag |= Z_MANAGEMENT_MUST_EXIST;
        *zmanag |= opt_management;

        /*********************/
        /* Transport options */
        /*********************/
        *ztrans = 0;
    }
    if (opt_mileage) {
        if (opt_speed)
        {
            sprintf(msgbuf,
                     "Total of %lu files and %lu bytes (%s) left to transfer",
                     files_left, bytes_left, d_time(bytes_left / (opt_speed / 10)));
            ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        }
        else
        {
            sprintf(msgbuf,  "Total of %lu files and %lu bytes left to transfer",
                     files_left, bytes_left);
            ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        }
    }
    if (opt_speed)
    {
        sprintf(msgbuf,  "Sending %s, %lu bytes, %s",
                 full_path, *length, d_time(*length / (opt_speed / 10)));
        ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
    }
    else
    {
        sprintf( msgbuf,  "Sending %s, %lu bytes", full_path, *length);
        ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
    }
    time(&t_started);

    /* update screen counts */
    ffc = 0L;                           /* Init file character counter. */
    tsecs = -1 ;
    cps = oldcps = 0L ; /* Init cps statistics */
    rs_len = 0L;
    rejection = -1;
    fsecs = gtimer();                   /* Time this file started */
#ifdef GFTIMER
    fpfsecs = gftimer();
#endif /* GFTIMER */
    filcnt++ ;
    intmsg(filcnt);
    return(0);
}

/* Finds an unique name for the file */

U32
CKDEVAPI
#ifdef CK_ANSIC
r_open_func(U8 **path, U32 length, U32 date, U32 mode,
                U32 f_left, U32 b_left,
                U8 zconv, U8 zmanag, U8 ztrans,
                U32 *offset)
#else
r_open_func(path,length,date,mode,f_left,b_left,zconv,zmanag,ztrans,offset)
     U8 **path; U32 length; U32 date; U32 mode;
     U32 f_left; U32 b_left;
     U8 zconv; U8 zmanag; U8 ztrans;
     U32 *offset;
#endif
{
    extern char * zdtstr(time_t);
    extern char *rf_err;
    extern char ofn1[CKMAXPATH+1];
    extern int ofn1x, discard, opnerr, stdouf, fnrpath ;
    extern long rs_len ;
    extern char * ofn2, fspec[];
    extern int  fspeclen;
#ifdef DYNAMIC
    extern char * srvcmd;
#else /* DYNAMIC */
    extern CHAR srvcmd[];
#endif /* DYNAMIC */
    char *newlongname; /* OS/2 long name items */
#ifdef OS2
#ifdef CK_LABELED
#ifdef __32BIT__
    char *zs, *longname; /* OS/2 long name items */
#endif /* __32BIT__ */
#endif /* CK_LABELED */
#endif /* OS2 */
    int dirflg=0;
    BOOLEAN path_was_null=0;
    BOOLEAN path_is_as_name=0;
#ifdef COMMENT
    U32 management = 0;
    U32 open_mode = 0;
#endif /* COMMENT */
    struct zattr zz ;
    int len = 0;

    rtimer();                       /* Reset the elapsed seconds timer. */
#ifdef GFTIMER
    rftimer();
#endif /* GFTIMER */

    ofn2 = NULL ;                       /* No new name (yet) */

    /* Simulate a Kermit Attribute Packet */
    initattr(&zz);
    zz.length = length ;
    zz.date.val = zdtstr(date);
    zz.date.len = strlen(zz.date.val);
    zz.type.val = zconv ? "A" : "B" ;
    zz.type.len = 1 ;

    if (*path == NULL) {                /* Xmodem receive? */
        if ( !(tl && tl->c && tl->c->path && tl->c->path[0]) ) {
            /* No as-name given, can't continue */
            ckscreen(SCR_ST,ST_ERR,0l,"No file name specified.");
            return(0);
        }
        p_omalloc( (void**)path, 4096, MODULE_CALLBACK, __LINE__ ) ;
        ckstrncpy( *path, tl->c->path, 4096 ) ;
        tl->c = tl->c->n ;
        path_was_null = 1;
    }

    ckscreen(SCR_FN,0,0l,*path);
    tlog(F110,"Receiving",*path,0);
    fsize = length;
    /* ckscreen(SCR_FS,0,length,""); called from opena() when fsize is set */

    /* Use the AS-NAME */
    if ( !path_was_null && tl && tl->c && tl->c->path && tl->c->path[0] ) {
        p_ofree( (void*)path, MODULE_CALLBACK, __LINE__ );
        p_omalloc( (void**)path, 4096, MODULE_CALLBACK, __LINE__ ) ;
        ckstrncpy( *path, tl->c->path, 4096 ) ;
        tl->c = tl->c->n ;
        ckscreen(SCR_AN,0,0l,*path);
        tlog(F110," as",*path,0);
        path_is_as_name=1;
#ifdef PIPESEND
        if ( pipesend ) {
            ckstrncpy(srvcmd,*path,MAXRP+4);
        }
#endif /* PIPESEND */
    }
#ifdef PIPESEND
    else if ( (*path)[0] == '!' && usepipes ) {
        (*path)++;
        ckstrncpy(srvcmd,*path,MAXRP+4);
    }
#endif /* PIPESEND */
    else {

        if (!opt_paths)
            strip_drive_and_dir(*path);
        else {
            if (opt_create && create_dirs(*path)) {
                sprintf(msgbuf, "%s: %s, %s...", *path, strerror(errno),
                         p_cfg.protocol_type == PROTOCOL_Z ? "skipping" : "canceling");
                ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
                tlog(F110," error - can't create path",*path,0);
                p_ofree( (void*)path, MODULE_CALLBACK, __LINE__ );
                return(0);
            }
        }
    }
    if (opt_mileage) {
        if (f_left || b_left) {
            if (opt_speed) {
                sprintf(msgbuf,
                         "Total of %lu files and %lu bytes (%s) left to transfer",
                         f_left, b_left, d_time(b_left / (opt_speed / 10)));
                ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
            }
            else
            {
                sprintf(msgbuf, "Total of %lu files and %lu bytes left to transfer",
                         f_left, b_left);
                ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
            }
        }
    }
    if (p_cfg.protocol_type == PROTOCOL_Z && opt_options) {
        switch (zconv) {
        case Z_CONVERSION_UNDEFINED:
            break;

        case Z_CONVERSION_BINARY:
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender suggests a binary conversion to be used");
            break;

        case Z_CONVERSION_TEXT:
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender suggests a text conversion to be used");
            break;

        case Z_CONVERSION_RESUME:
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender suggests that the file transfer should be resumed");
            break;

        default:
            sprintf(msgbuf, "Unknown conversion option received: %lu", zconv);
            ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
            break;
        }

#ifdef COMMENT
        /* Kermit's File Collision Setting always is used instead of */
        /* the sender's request.  Otherwise, it would be a huge      */
        /* security risk.                                            */
        switch (zmanag & Z_MANAGEMENT_MASK) {
        case Z_MANAGEMENT_UNDEFINED:
            break;

        case Z_MANAGEMENT_UPDATE:
            fncact = XYFX_U ;
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender wants to update older and shorter files");
            break;

        case Z_MANAGEMENT_COMPARE:
            fncact = XYFX_U ;
            ckscreen(SCR_ST,ST_MSG,0l,
    "Sender wants to compare possibly existing files before replacing");
            break;

        case Z_MANAGEMENT_APPEND:
            fncact = XYFX_A ;
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender wants to append to already existing files");
            break;

        case Z_MANAGEMENT_REPLACE:
            fncact = XYFX_X ;
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender wants to replace already existing files");
            break;

        case Z_MANAGEMENT_NEWER:
            fncact = XYFX_U ;
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender wants to update older files");
            break;

        case Z_MANAGEMENT_DIFFERENT:
            fncact= XYFX_U ;
            ckscreen(SCR_ST,ST_MSG,0l,
             "Sender wants to replace files with different dates and lengths");
            break;

        case Z_MANAGEMENT_PROTECT:
            fncact = XYFX_D ;
            ckscreen(SCR_ST,ST_MSG,0l,
                "Sender does not want to replace already existing files");
            break;

        case Z_MANAGEMENT_RENAME:
            fncact = XYFX_R ;
            ckscreen(SCR_ST,ST_MSG,0l,
                "Sender wants to use a new name for file if file exists");
            break;

        case Z_MANAGEMENT_BACKUP:
            fncact = XYFX_B ;
            ckscreen(SCR_ST,ST_MSG,0l,
                "Sender wants to backup existing files");
            break;

        default:
              sprintf(msgbuf,
                       "Unknown management option received: %lu", zmanag);
              ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
              break;
        }

        if (zmanag & Z_MANAGEMENT_MUST_EXIST)
          ckscreen(SCR_ST,ST_MSG,0l,
                  "Sender wants to transfer only already existing files");
#endif /* COMMENT */

        switch (ztrans) {
        case Z_TRANSPORT_UNDEFINED:
            break;

        case Z_TRANSPORT_LZW:
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender wants to use Lempel-Ziv compression");
            break;

        case Z_TRANSPORT_CRYPT:
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender wants to use encryption");
            break;

        case Z_TRANSPORT_RLE:
            ckscreen(SCR_ST,ST_MSG,0l,
                    "Sender wants to use RLE compression");
            break;

        default:
              sprintf(msgbuf,
                       "Unknown transport option received: %lu", ztrans);
              ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
              break;
        }
    }
    /******************************/
    /* Process conversion options */
    /******************************/
    if (zconv == Z_CONVERSION_TEXT || opt_text) {
        binary = 0 ;
        tlog(F100," mode: text","",0);
    }
    else {
        binary = 1 ;
        tlog(F100," mode: binary","",0);
    }

#ifdef COMMENT
    /* More things that Kermit is not going to support */
    /******************************/
    /* Process management options */
    /******************************/
    if (!(zmanag & Z_MANAGEMENT_MUST_EXIST) && !opt_existing)
      open_mode |= O_CREAT;
    management = (zmanag & Z_MANAGEMENT_MASK);
    if (opt_management)         /* If management option specified */
                                        /* on the command-line */
      management = opt_management;      /* Command-line overrides remote's */
                                        /* options */
    if (!management && zconv != Z_CONVERSION_RESUME && !opt_resume) {
        /* If no management option or resume specified, we'll default to */
        /* protecting existing files... */
        management = Z_MANAGEMENT_PROTECT;
    }
#endif /* COMMENT */


    if (length != -1) {
        if (opt_speed != 0) {
            sprintf(msgbuf, "Receiving %s, %lu bytes, %s",
                     *path, length, d_time(length / (opt_speed / 10)));
            ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        }
        else
        {
            sprintf(msgbuf, "Receiving %s, %lu bytes", *path, length);
            ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
        }
    }
    else
    {
        sprintf(msgbuf, "Receiving %s", *path);
        ckscreen(SCR_ST,ST_MSG,0l,msgbuf);
    }

    if ( !path_is_as_name ) {
        add_recv_dir_to_path(path);
        ckscreen(SCR_AN,0,0l,*path);
        tlog(F110," as",*path,0);
    }

    if (fncnv)                          /* FILE NAMES CONVERTED? */
      zrtol((char *)(*path),(char *)ofn1); /* Yes, convert to local form */
    else
      ckstrncpy(ofn1,(char *)(*path),CKMAXPATH+1); /* No, copy literally. */

    /* Now the incoming filename, possibly converted, is in ofn1[]. */

#ifdef OS2ONLY
    /* Don't refuse the file just because the name is illegal. */
    if (!IsFileNameValid(ofn1)) {       /* Name is OK for OS/2? */
#ifdef __32BIT__
        char *zs = NULL;
        zstrip(ofn1, &zs);              /* Not valid, strip unconditionally */
        if (zs) {
            if (zz.longname.len &&
                 zz.longname.val)       /* Free previous longname, if any */
                free(zz.longname.val);
            zz.longname.len = strlen(zs); /* Store in attribute structure */
            zz.longname.val = (char *) malloc( zz.longname.len + 1 ) ;
            if (zz.longname.val)        /* Remember this (illegal) name */
                ckstrncpy( zz.longname.val, zs, zz.longname.len + 1);
        }
#endif /* __32BIT__ */
        debug(F110,"rcvfil: invalid file name",ofn1,0);
        ChangeNameForFAT(ofn1); /* Change to an acceptable name */
        debug(F110,"rcvfil: FAT file name",ofn1,0);
    } else {                            /* Name is OK. */
        debug(F110,"rcvfil: valid file name",ofn1,0);
#ifdef __32BIT__
        if (zz.longname.len &&
             zz.longname.val)           /* Free previous longname, if any */
          free(zz.longname.val);
        zz.longname.len = 0;
        zz.longname.val = "";   /* This file doesn't need a longname */
#endif /* __32BIT__ */
    }
#endif /* OS2ONLY */
    debug(F110,"rcvfil as",ofn1,0);

/* Filename collision action section. */

    discard = 0 ;                       /* Reset the flag for this file */
    dirflg =                            /* Is it a directory name? */
#ifdef CK_TMPDIR
        isdir(ofn1)
#else
        0
#endif /* CK_TMPDIR */
          ;
    debug(F101,"rcvfil dirflg","",dirflg);
    ofn1x = (zchki(ofn1) != -1);        /* File already exists? */
    debug(F101,"rcvfil ofn1x",ofn1,ofn1x);

    if ( (
#ifdef UNIX
        strcmp(ofn1,"/dev/null") &&     /* It's not the null device? */
#else
#ifdef OSK
        strcmp(ofn1,"/nil") &&  /* It's not the null device? */
#endif /* OSK */
#endif /* UNIX */
        !stdouf ) &&                    /* Not copying to standard output? */
        ofn1x ||                        /* File of same name exists? */
        dirflg ) {                      /* Or file is a directory? */
        debug(F111,"rcvfil exists",ofn1,fncact);
        switch (fncact) {               /* Yes, do what user said. */
        case XYFX_A:                    /* Append */
            debug(F100,"rcvfil append","",0);
            if (dirflg) {
                rf_err = "Can't append to a directory";
                tlog(F100," error - can't append to directory","",0);
                discard = opnerr = 1;
                p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
                return(0);
            }
            tlog(F110," appending to",ofn1,0);
            break;
        case XYFX_Q:                    /* Query (Ask) */
            break;                      /* not implemented */
        case XYFX_B:                    /* Backup (rename old file) */
            if (dirflg) {
                rf_err = "Can't rename existing directory";
                tlog(F100," error - can't rename directory","",0);
                discard = opnerr = 1;
                p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
                return(0);
            }
            if ( zconv != Z_CONVERSION_RESUME ) {
                znewn(ofn1,&ofn2);              /* Get new unique name */
                tlog(F110," backup:",ofn2,0);
                debug(F110,"rcvfil backup ofn1",ofn1,0);
                debug(F110,"rcvfil backup ofn2",ofn2,0);
#ifdef OS2
#ifdef CK_LABELED
#ifdef __32BIT__
/*
  In case this is a FAT file system, we can't change only the FAT name, we
  also have to change the longname from the extended attributes block.
  Otherwise, we'll have many files with the same longname and if we copy them
  to an HPFS volume, only one will survive.
*/
                if (os2getlongname(ofn1, &longname) > -1) {
                    if (strlen(longname)) {
                        char tmp[10];
                        extern int ck_znewn;
                        sprintf(tmp,".~%d~",ck_znewn);
                        newlongname =
                            (char *) malloc(strlen(longname) + strlen(tmp) + 1 ) ;
                        if (newlongname) {
                            strcpy(newlongname, longname);
                            strcat(newlongname, tmp);
                            os2setlongname(ofn1, newlongname);
                            free(newlongname);
                            newlongname = NULL;
                        }
                    }
                }
                else
                    debug(F100,"rcvfil os2getlongname failed","",0);
#endif /* __32BIT__ */
#endif /* CK_LABELED */
#endif /* OS2 */

#ifdef COMMENT
                /* Do this later, in opena()... */
                if (zrename(ofn1,ofn2) < 0) {
                    rf_err = "Can't transform filename";
                    debug(F110,"rcvfil rename fails",ofn1,0);
                    discard = opnerr = 1;
                    p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
                    return(0);
                }
#endif /* COMMENT */
            }
            break;

        case XYFX_D:                    /* Discard (refuse new file) */
            if ( zconv != Z_CONVERSION_RESUME ) {
                discard = 1;
                rejection = 1;          /* Horrible hack: reason = name */
                debug(F101,"rcvfil discard","",discard);
                tlog(F100," refused: name","",0);
                p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
                return(0);
            }
            break;

        case XYFX_R:                    /* Rename incoming file */
            if ( zconv != Z_CONVERSION_RESUME ) {
            znewn(ofn1,&ofn2);          /* Make new name for it */
#ifdef OS2
#ifdef __32BIT__
            if (zz.longname.len) {
                char tmp[10];
                extern int ck_znewn;
                sprintf(tmp,".~%d~",ck_znewn);
                newlongname =
                    (char *) malloc(zz.longname.len + strlen(tmp) + 1);
                if (newlongname) {
                    strcpy( newlongname, zz.longname.val);
                    strcat( newlongname, tmp);
                    debug(F110,
                           "Rename Incoming: newlongname",newlongname,0);
                    if (zz.longname.len &&
                         zz.longname.val)
                        free(zz.longname.val);
                    zz.longname.len = strlen(newlongname);
                    zz.longname.val = newlongname ;
                }
            }
#endif /* __32BIT__ */
#endif /* OS2 */
            }
            break;
        case XYFX_X:                    /* Replace old file */
            debug(F100,"rcvfil overwrite","",0);
            if (dirflg) {
                rf_err = "Can't overwrite existing directory";
                tlog(F100," error - can't overwrite directory","",0);
                discard = opnerr = 1;
                p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
                return(0);
            }
            if ( zconv != Z_CONVERSION_RESUME )
                tlog(F110,"overwriting",ofn1,0);
            break;

        case XYFX_U:                    /* Refuse if older */
            debug(F100,"rcvfil update","",0);
            if (dirflg) {
                rf_err = "File has same name as existing directory";
                tlog(F110," error - directory exists:",ofn1,0);
                discard = opnerr = 1;
                p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
                return(0);
            }
            if ( zconv != Z_CONVERSION_RESUME && zstime(*path,&zz,1) > 0 ) {
                tlog(F100," refused: date","",0);
                ckscreen(SCR_ST,ST_REFU,0l,"date");
                p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
                return(0);
            }
            break;                      /* Not here, we don't have */
                                        /* the attribute packet yet. */
        default:
            debug(F101,"rcvfil bad collision action","",fncact);
            break;
        }
    }

    debug(F110,"rcvfil ofn1",ofn1,0);
    debug(F110,"rcvfil ofn2",ofn2,0);
    if (zconv != Z_CONVERSION_RESUME &&
         fncact == XYFX_R && ofn1x && ofn2) { /* Renaming incoming file? */
        ckscreen(SCR_AN,0,0l,ofn2);     /* Display renamed name */
        p_ofree( (void*)path, MODULE_CALLBACK, __LINE__ );
        p_omalloc( (void**)path, 4096, MODULE_CALLBACK, __LINE__ ) ;
        ckstrncpy((*path), ofn2, 4096);          /* Return it */
    } else {                            /* No */
        ckscreen(SCR_AN,0,0l,ofn1);     /* Display regular name */
        p_ofree( (void*)path, MODULE_CALLBACK, __LINE__ );
        p_omalloc( (void**)path, 4096, MODULE_CALLBACK, __LINE__ ) ;
        ckstrncpy((*path), ofn1, 4096);          /* and return it. */
    }

#ifdef CK_MKDIR
/*  Create directory(s) if necessary.  */
    if (!discard && !fnrpath) {         /* RECEIVE PATHAMES ON? */
        debug(F110,"rcvfil calling zmkdir",ofn1,0); /* Yes */
        if (zmkdir(ofn1) < 0) {
            debug(F100,"zmkdir fails","",0);
            tlog(F110," error - directory creation failure:",ofn1,0);
            rf_err = "Directory creation failure.";
            discard = 1;
            return(0);
        }
    }
#else
    debug(F110,"sfile CK_MKDIR not defined",ofn1,0);
#endif /* CK_MKDIR */

#ifndef NOICP
/* #ifndef MAC */
/* Why not Mac? */
    ckstrncpy(fspec,ofn1,fspeclen);        /* Here too for \v(filespec) */
/* #endif */
#endif /* NOICP */
    debug(F110,"rcvfil: (*path)",(*path),0);

    len = zchki(*path);          /* if returns < 0, file does not exist */
    if ( p_cfg.protocol_type == PROTOCOL_Z &&
         /* Are we crash-recovering? */
         (zconv == Z_CONVERSION_RESUME || opt_resume) && (len >= 0) ) {
        if ( len != length ) {
            zz.disp.val = "R" ; /* Treat as a RESEND */
            zz.disp.len = 1 ;
            fncact = XYFX_A ;
        }
        else {
            tlog(F100," refused: file complete","",0);
            ckscreen(SCR_ST,ST_REFU,0l,"file complete");
            p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
            return(0);
        }
    }

    if ( discard || opena(*path,&zz) == 0 )
    {
        p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
        return(0);
    }

    if (p_cfg.protocol_type == PROTOCOL_Z && (len >= 0) &&
         /* Are we crash-recovering? */
         (zconv == Z_CONVERSION_RESUME || opt_resume)) {
        if (len != length)
        {
            sprintf( msgbuf,  "Crash recovery at %lu", len);
            ckscreen(SCR_ST,ST_MSG,0l, msgbuf ) ;
            tlog(F101," resume at","",len);
        }
        else {
            ckscreen(SCR_ST,ST_MSG,0l,
                    "We have the whole file already, skipping...");
            tlog(F100," skipping - file match detected","",0);
            clsof(0);   /* don't discard the file */
            p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ ) ;
            return(0);
        }
        if (offset != NULL)     /* If not null, we're using Zmodem */
                      /* Tell the remote to start sending from the */
          *offset = len;        /* existing file length */
    }
    else {              /* Not resuming => appending */
        if ( len >= 0 ) {
            sprintf( msgbuf,  "Appending at %lu", len);
            ckscreen(SCR_ST,ST_MSG,0l,msgbuf ) ;
            tlog(F101," appending at","",len);
        }
        if (offset != NULL)     /* If not null, we're using Zmodem */
          /* Tell the remote to start sending from the */
          *offset = 0;  /* beginning of the file */
    }
    time(&t_started);

    /* update screen counts */
    ffc = 0L;                           /* Init per-file counters */
    cps = oldcps = 0L ;
    rs_len = 0L;
    rejection = -1;
#ifdef GFTIMER
    fpfsecs = gftimer();
#else
    fsecs = gtimer();                   /* Time this file started */
#endif /* GFTIMER */
#ifdef GFTIMER
    gtv = -1.0;
    oldgtv = -1.0;
#else
    gtv = -1L;
    oldgtv = -1L;
#endif /* GFTIMER */
    filcnt++;
    intmsg(filcnt);

    return(0);
}

U32
CKDEVAPI
#ifdef CK_ANSIC
close_func(U8 **path,
               U32 length,
               U32 date,
               U32 retransmits,
               BOOLEAN successful,
               U32 offset)
#else
close_func(path,length,date,retransmits,successful,offset)
     U8 **path;
     U32 length;
     U32 date;
     U32 retransmits;
     BOOLEAN successful;
     U32 offset;
#endif
{
    U8 id=0;
    time_t t_now;
    U32 cps;
    U32 ret_val = 0;
#ifdef NT
    struct _utimbuf times;
#else
    struct utimbuf times;
#endif

    time(&t_now);
    if ( p_cfg.transfer_direction == DIR_SEND ) {
        if ( successful ) {
            cxseen = 0 ;
            czseen = 0 ;
            discard = 0 ;
        if (ffc < fsize)        /* don't let clsif() print interrupted msg */
            ffc=fsize;
        }
        else if ( aborted ) {
            cxseen = 0 ;
            czseen = 1 ;
            discard = 0 ;
        }
        else {
            cxseen = 0 ;
            czseen = 0 ;
            discard = 1 ;
        }
        clsif() ;
    }
    else if ( p_cfg.transfer_direction == DIR_RECV ) {
        if ( successful ) {
            cxseen = 0 ;
            czseen = 0 ;
            discard = 0 ;
        }
        else if ( aborted ) {
            cxseen = 0 ;
            czseen = 1 ;
            discard = 0 ;
        }
        else {
            cxseen = 0 ;
            czseen = 0 ;
            discard = 0 ;
        }
        clsof( !successful ) ;
    }

    if (!opt_touch && date != -1) {     /* Set the file date */
        time(&times.actime);
        times.modtime = date;
#ifdef __EMX__
        utime(*path, &times);
#else /* __EMX__ */
        _utime(*path, &times);
#endif /* __EMX__ */
    }
    if (p_cfg.transfer_direction == DIR_SEND) {
        files_left--;
        bytes_left -= length;
    }
    if (offset) {
        cps = (t_now == t_started ?
                offset : offset / (t_now - t_started));
        sprintf( msgbuf, "%lu bytes, %s, %lu CPS%s",
                 offset, d_time(t_now - t_started), cps,
                 !successful ? ", Transfer incomplete" : "");
        ckscreen(SCR_ST,ST_MSG,0l, msgbuf ) ;

        if (opt_dszlog != NULL) {
            if (dszlog_stream == NULL &&
                 (dszlog_stream = fopen(opt_dszlog, "w")) == NULL) {
                perror(opt_dszlog);
                ret_val = 1;
                 } else {
                     if (successful) {
                         switch (p_cfg.protocol_type) {
                         case PROTOCOL_X:
                             id = 'x';
                             break;

                         case PROTOCOL_Y:
                             id = 'y';
                             break;

                         case PROTOCOL_G:
                             id = 'g';
                             break;

                         case PROTOCOL_Z:
                             id = 'z';
                             break;
                         default:               /* Just to shut up the compiler */
                             break;
                         }
                         if (p_cfg.transfer_direction == DIR_RECV)
                             /* The protocol id */
                             /* should be in */
                             /* uppercase when */
                             /* receiving...*/
                             id = toupper(id);
                     } else if (carrier_lost)
                         id = 'L';                      /* Carrier lost */
                     else
                         id = 'E';                      /* Other error */

                     fprintf(dszlog_stream,
                              "%c %6lu %5lu bps %4lu cps %3lu errors     0 %4lu %12s %ld\n",
                              id,
                              successful ? length : offset,
                              opt_speed,
                              cps,
                              retransmits,
                              1024L, /* block_size */
                              full_path != NULL ? full_path : *path,
                              remote_serial_num);
                 }
        }
    }
    if (p_cfg.transfer_direction == DIR_RECV &&
         (opt_clean || !keep) &&                /* fdc */
         !successful)
    {
        sprintf( msgbuf,   "Deleting: %s", *path);
        ckscreen(SCR_ST,ST_MSG,0l,msgbuf ) ;
        tlog(F110," deleting",*path,0);
        unlink(*path);
    } else if (p_cfg.transfer_direction == DIR_SEND && /* fdc */
                moving &&                               /* fdc */
                successful)
    {                   /* fdc */
        sprintf( msgbuf, "Deleting: %s", *path);
        ckscreen(SCR_ST,ST_MSG,0l, msgbuf);     /* fdc */
        tlog(F110," deleting",*path,0);
        unlink(*path);                  /* fdc */
    }
    p_ofree( (void **) path, MODULE_CALLBACK, __LINE__ );

    fncact = savfnc ;
    return(ret_val);
}

U32
CKDEVAPI
#ifdef CK_ANSIC
seek_func(U32 pos)
#else
seek_func(pos) U32 pos;
#endif
{
  if (zfseek(pos) == -1) {
      sprintf(msgbuf, "\rFailed to seek in file, %s\n", strerror(errno));
      ckscreen(SCR_ST,ST_MSG,0l, msgbuf);     /* fdc */
      debug(F111,"P seek_func","Failed to seek in file",errno);
      return(1);
  }
  else
      return(0);
}

U32
CKDEVAPI
#ifdef CK_ANSIC
read_func(U8 *buf, U32 bytes_wanted, U32 *bytes_got)
#else
read_func(buf,bytes_wanted,bytes_got)
     U8 *buf; U32 bytes_wanted; U32 *bytes_got;
#endif
{
    int c ;
    static int oldc = -1;

    *bytes_got = 0 ;

    if ( oldc >= 0 )
    {
        *buf = (U8) oldc ;
        buf++ ;
        (*bytes_got)++ ;
        bytes_wanted-- ;
        oldc = -1 ;
    }

    for ( ; bytes_wanted ; bytes_wanted-- )
    {
       if ((c = zminchar()) < 0)
       {
           if ( bytes_got == 0 )
           {
               fprintf(stderr, "\rFailed to read from file, %s\n", strerror(errno));
               debug(F111,"P read_func","Failed to read from file",errno);
               return(1);
           }
           else
             return(0);
       }
       else
       {
           /* Text files are converted by the sender in XMODEM and YMODEM   */
           /* and by the receiver in ZMODEM.  So for ZMODEM we must convert */
           /* CR-LF to NL before transmitting.                              */
           if ( p_cfg.protocol_type != PROTOCOL_Z || binary || c != 13 )
           {
               *buf = (U8) c ;
               buf++ ;
               (*bytes_got)++ ;
           }
           else
           {
               *buf = (U8) c ;
               if ((c = zminchar()) < 0)
                 return(0) ;
               if ( c == 10 )
               {
                   *buf = (U8) c ;
                   buf++ ;
                   (*bytes_got)++ ;
               }
               else
               {
                   buf++ ;
                   (*bytes_got)++ ;

                   if ( bytes_wanted > 1 )
                   {
                       *buf = (U8) c ;
                       buf++ ;
                       (*bytes_got)++ ;
                       bytes_wanted-- ;
                   }
                   else
                   {
                       oldc = c ;
                   }
               }
           }
       }
    }


    return(0);

}

U32
CKDEVAPI
#ifdef CK_ANSIC
write_func(U8 *buf, U32 bytes)
#else
write_func(buf,bytes) U8 *buf; U32 bytes ;
#endif
{
    char str[80];

    if (zsoutx(ZOFILE,buf,bytes)<0)
    {
        sprintf(str,"Failed to write to file, %s", strerror(errno));
        ckscreen(SCR_ST,ST_ERR,0l,str);
        debug(F111,"P write_func","Failed to write to file",errno);
        return(1);
    }
    return (0);
}

#endif /* XYZ_INTERNAL */
#endif /* NOXFER */
