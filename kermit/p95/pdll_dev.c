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

/* General communication library. Calls for apropriate inline */
/* functions depending on communications type */ 

#include <stdio.h>

#include "ckcdeb.h"

#include "pdll_os2incl.h"
#include "p_type.h"
#include "pdll_omalloc.h"
#include "pdll_dev.h"

U8 *dev_path = NULL;
U32 dev_handle = 0;
U32 passive_socket = 0;
U8 *socket_remote = NULL;
U16 socket_port = 0;

U32 dev_ready = 0;

U32 outbuf_size = 0;
U32 inbuf_size = 0;
U32 timeouts_per_call = 0;
U8 *outbuf = NULL;
U32 outbuf_idx = 0;
U8 *inbuf = NULL;
U32 inbuf_idx = 0;
U32 inbuf_len = 0;
U32 dev_type = 0;
U32 dev_opened = 0;
U32 dev_shared = 0;
U32 dev_server = 0;
U32 dev_telnet = 0;
U32 dev_telnet_u_binary = 0 ;
U32 dev_telnet_me_binary = 0 ;
U32 dev_telnet_iac_escaped = 1 ;

U32 watch_carrier = 0;

VOID dev_open(void) {

  APIRET rc=0;

  if (!dev_handle)
    dev_opened = 1;

  switch (dev_type) {
  case DEV_TYPE_EXE_IO:
     /* do nothing - exe responsible for opening */
     break;

#ifndef XYZ_DLL
  case DEV_TYPE_ASYNC:
    async_open();
    break;

#ifdef OS2
#ifndef NT
  case DEV_TYPE_PIPE:
    pipe_open();
    break;
#endif
#endif

  case DEV_TYPE_SOCKET:
  default:
#ifdef OS2
#ifndef NT
   load_tcpip();
#endif
#endif
    tcp_open();
#endif /* XYZ_DLL */
    break;

  }

    /* allocate one more char than needed so we can safely tack on a NULL */
    /* at the end for debug() purposes                                    */
    pdll_malloc( (void *)&inbuf, inbuf_size+1, MODULE_DEV, __LINE__ );
    pdll_malloc( (void *)&outbuf, outbuf_size+1, MODULE_DEV, __LINE__ );

  if (!dev_server)
    dev_ready = 1;
}

U32 
#ifdef CK_ANSIC
dev_connect(void) 
#else
dev_connect()
#endif
{

  U32 ret_val;

  switch (dev_type) {
  case DEV_TYPE_EXE_IO:
      /* EXE is responsible for the connection */
      dev_ready = 1 ;
      return(0);

#ifdef XYZ_DLL
  case DEV_TYPE_ASYNC:
    /* ASYNC can't connect this way */
    return(0);
    
#ifdef OS2
#ifndef NT
   case DEV_TYPE_PIPE:
    ret_val = pipe_connect();
    if (ret_val)
      dev_ready = 1;
    return(ret_val);
#endif
#endif

  case DEV_TYPE_SOCKET:
  default:
    ret_val = tcp_connect();
    if (ret_val)
      dev_ready = 1;
    return(ret_val);
#else
      return(0);
#endif /* XYZ_DLL */
  }
}

VOID 
#ifdef CK_ANSIC
dev_close(void) 
#else
dev_close()
#endif
{
  APIRET rc=0;
  
  if (dev_opened) {
    switch (dev_type) {
    case DEV_TYPE_EXE_IO:
        /* EXE is responsible for closing the device */
        break;

#ifdef XYZ_DLL
    case DEV_TYPE_ASYNC:
      async_close();
      break;

#ifdef OS2
#ifndef NT
    case DEV_TYPE_PIPE:
      pipe_close();
      break;
#endif
#endif

    case DEV_TYPE_SOCKET:
    default:
      tcp_close();
#ifdef OS2
#ifndef NT
      unload_tcpip();
#endif
#endif
      break;
#endif /* XYZ_DLL */
    }
  }
    dev_ready = 0;
    pdll_free( (void *) &outbuf, MODULE_DEV, __LINE__ );
    pdll_free( (void *) &inbuf, MODULE_DEV, __LINE__ );
}


VOID 
#ifdef CK_ANSIC
dev_purge_inbuf(void) 
#else
dev_purge_infbuf()
#endif
{

  while (1) {
    inbuf_idx = 0;		/* Abandon the stuff in buffer */
    inbuf_len = 0;
    if (!dev_incoming())
      break;
    dev_getch_buf();
  }
}

VOID 
#ifdef CK_ANSIC
dev_purge_outbuf(void) 
#else
dev_purge_outbuf()
#endif
{ 
    /* This is done by function because */
    /* we are accessing a static variable */

    outbuf_idx = 0;
}

VOID 
#ifdef CK_ANSIC
dev_flush_outbuf(void) 
#else
dev_flush_outbuf()
#endif
{
  switch (dev_type) {
  case DEV_TYPE_EXE_IO:
      exe_flush_outbuf();
      break;

#ifdef XYZ_DLL
  case DEV_TYPE_ASYNC:
    async_flush_outbuf();
    break;

#ifdef OS2
#ifndef NT
   case DEV_TYPE_PIPE:
    pipe_flush_outbuf();
    break;
#endif
#endif 

  case DEV_TYPE_SOCKET:
  default:
    tcp_flush_outbuf();
    break;
#endif /* XYZ_DLL */
  }
}

#ifdef XYZ_DLL
VOID 
dev_get_cfg(DEV_CFG *dev_cfg) {

  static APIRET rc=0;
  static U32 LengthInOut;
  
  if (dev_type == DEV_TYPE_ASYNC) {
#ifdef NT
#ifdef COMMENT
     {
        DCB dcb ;
        if ( !GetCommState( (HANDLE) dev_handle, &dcb ) )
           rc = GetLastError() ;
        else 
        {

        }
     }
#endif
#else /* NT */
    LengthInOut = sizeof(DEV_CFG);
    rc = DosDevIOCtl(dev_handle,		/* FileHandle */
		     IOCTL_ASYNC, 		/* Category */
		     ASYNC_GETDCBINFO,		/* Function */
		     NULL,			/* ParmList */
		     0L,				/* ParmLengthMax */
		     NULL,			/* ParmLengthInOut */
		     dev_cfg,			/* DataArea */
		     sizeof(DEV_CFG),		/* DataLengthMax */
		     &LengthInOut);		/* DataLengthInOut */
#endif 
    if (rc)
      p_error(P_ERROR_DOSDEVIOCTL, rc,
		MODULE_DEV, __LINE__, (U32)dev_path);
  }
}    

VOID 
dev_set_cfg(DEV_CFG *dev_cfg) {

  static APIRET rc=0;
  static U32 LengthInOut;

  if (dev_type == DEV_TYPE_ASYNC) {
#ifdef NT
#ifdef COMMENT
     {
        DCB dcb ;
        if ( !SetCommState( (HANDLE) dev_handle, &dcb ) )
           rc = GetLastError() ;
        else 
        {

        }
     }
#endif
#else /* NT */
    LengthInOut = sizeof(DEV_CFG);
    rc = DosDevIOCtl(dev_handle,		/* FileHandle */
		     IOCTL_ASYNC, 		/* Category */
		     ASYNC_SETDCBINFO,		/* Function */
		     dev_cfg,			/* ParmList */
		     sizeof(DEV_CFG),		/* ParmLengthMax */
		     &LengthInOut,		/* ParmLengthInOut */
		     NULL,			/* DataArea */
		     0,				/* DataLengthMax */
		     NULL);			/* DataLengthInOut */
#endif
    if (rc)
      p_error(P_ERROR_DOSDEVIOCTL, rc,
		MODULE_DEV, __LINE__, (U32)dev_path);
  }
}

VOID 
dev_chg_cfg(U16 write_timeout,
		 U16 read_timeout,
		 U8 flags1,
		 U8 flags2,
		 U8 flags3) {

  DEV_CFG dev_cfg;

  if (dev_type == DEV_TYPE_ASYNC) {
    dev_get_cfg(&dev_cfg);
    dev_cfg.write_timeout = write_timeout;
    dev_cfg.read_timeout = read_timeout;
    dev_cfg.flags1 = flags1;
    dev_cfg.flags2 = flags2;
    dev_cfg.flags3 = flags3;
    dev_set_cfg(&dev_cfg);
  }
}
#endif /* XYZ_DLL */

VOID 
#ifdef CK_ANSIC
dev_set_break_on(void) 
#else
dev_set_break_on()
#endif
{
  switch (dev_type) {
  case DEV_TYPE_EXE_IO:
      exe_set_break_on() ;
      break;

#ifdef XYZ_DLL
  case DEV_TYPE_ASYNC:
    async_set_break_on();
    break;

#ifndef NT
   case DEV_TYPE_PIPE:
    pipe_set_break_on();
    break;
#endif

  case DEV_TYPE_SOCKET:
  default:
    tcp_set_break_on();
    break;
#endif /* XYZ_DLL */
  }
}

VOID 
#ifdef CK_ANSIC
dev_set_break_off(void) 
#else
dev_set_break_off()
#endif
{

  switch (dev_type) {
  case DEV_TYPE_EXE_IO:
      exe_set_break_off();
      break;

#ifdef XYZ_DLL
  case DEV_TYPE_ASYNC:
      async_set_break_off();
      break;

#ifndef NT
  case DEV_TYPE_PIPE:
    pipe_set_break_off();
    break;
#endif

  case DEV_TYPE_SOCKET:
  default:
    tcp_set_break_off();
    break;
#endif /* XYZ_DLL */
  }
}

VOID 
#ifdef CK_ANSIC
dev_send_tn_cmd(U8 cmd, U8 opt) 
#else
dev_send_tn_cmd() U8 cmd; U8 opt;
#endif
{

  U8 buf[5];
  U32 i;

  buf[0] = 0xFF;		/* IAC */
  buf[1] = cmd;
  buf[2] = opt;
  buf[3] = '\0';
  for (i = 0; buf[i]; i++) {
    if (outbuf_idx == outbuf_size)
      dev_flush_outbuf();
    outbuf[outbuf_idx++] = buf[i];
  }
  dev_flush_outbuf();
}
