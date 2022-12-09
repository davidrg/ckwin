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

/* Named pipe communication routines as inline functions */

#ifdef XYZ_DLL
#ifdef NT

#else /* NT */
_Inline void pipe_open(void) {

  APIRET rc=0;
  U32 ActionTaken;
  U32 mode;

  if (!dev_handle) {
    if (dev_server) {
      rc = DosCreateNPipe(dev_path,               /* FileName */
			  &dev_handle,		/* PipeHandle */
			  NP_NOWRITEBEHIND |
			  NP_INHERIT |
			  NP_ACCESS_DUPLEX,	/* OpenMode */
			  NP_NOWAIT |
			  NP_TYPE_BYTE |
			  NP_READMODE_BYTE |
			  1,			/* PipeMode */
			  outbuf_size,		/* OutBufSize */
			  inbuf_size,		/* InBufSize */
			  0);			/* TimeOut */
      if (rc)
	p_error(P_ERROR_DOSCREATENPIPE, rc,
		MODULE_PIPE, __LINE__, (U32)dev_path);
    } else {
      if (dev_shared)
	mode = OPEN_SHARE_DENYNONE;
      else
	mode = OPEN_SHARE_DENYREADWRITE;
      rc = DosOpen(dev_path,			/* FileName */
		   &dev_handle,			/* FileHandle */
		   &ActionTaken,		        /* ActionTaken */
		   0L,       			/* FileSize */
		   FILE_OPEN,			/* FileAttribute */
		   OPEN_ACTION_OPEN_IF_EXISTS,	/* OpenFlag */
		   OPEN_FLAGS_NO_CACHE |	       	/* OpenMode */
		   mode |
		   OPEN_ACCESS_READWRITE,
		   0);				/* EABuf */
      if (rc)
	p_error(P_ERROR_DOSOPEN, rc,
		MODULE_PIPE, __LINE__, (U32)dev_path);
      rc = DosSetNPHState(dev_handle, NP_NOWAIT | NP_READMODE_BYTE);
      if (rc)
	p_error(P_ERROR_DOSSETNPHSTATE, rc,
		MODULE_PIPE, __LINE__, (U32)dev_path);
    }
  }
}

_Inline U32 pipe_connect(void) {

  APIRET rc=0;

  rc = DosConnectNPipe(dev_handle);
  if (rc == ERROR_PIPE_NOT_CONNECTED)
    return(0);
  else if (rc)
    p_error(P_ERROR_DOSCONNECTNPIPE, rc,
	    MODULE_PIPE, __LINE__, (U32)dev_path);
  return(1);
}

_Inline void pipe_close(void) {

  APIRET rc=0;

  if (dev_server) {
    rc = DosDisConnectNPipe(dev_handle);
    if (rc)
      p_error(P_ERROR_DOSDISCONNECTNPIPE, rc,
	      MODULE_PIPE, __LINE__, (U32)dev_path);
    rc = DosClose(dev_handle);
    if (rc)
      p_error(P_ERROR_DOSCLOSE, rc,
		MODULE_PIPE, __LINE__, (U32)dev_path);
  } else {
    rc = DosClose(dev_handle);
    if (rc)
      p_error(P_ERROR_DOSCLOSE, rc,
	      MODULE_PIPE, __LINE__, (U32)dev_path);
  }
}

_Inline U32 pipe_incoming(void) {

  static APIRET rc=0;
  static U8 buf[1];
  static U32 bytes_read;
  static AVAILDATA avail_data;
  static U32 pipe_state;

  rc = DosPeekNPipe(dev_handle,			/* Handle */
		    buf,			/* Buffer */
		    0,				/* BufferLen */
		    &bytes_read,		/* BytesRead */
		    &avail_data,		/* BytesAvail */
		    &pipe_state);		/* PipeState */
  if (rc)
    p_error(P_ERROR_DOSPEEKNPIPE, rc,
	    MODULE_PIPE, __LINE__, (U32)dev_path);
  if (avail_data.cbpipe)
    return(1);
  else
    return(0);
}

_Inline void pipe_getch_buf(void) {

  APIRET rc=0;
  U32 tmout_cnt;

  inbuf_idx = 0;
  inbuf_len = 0;
  for (tmout_cnt = 0; tmout_cnt < timeouts_per_call; tmout_cnt++) {
    rc = DosRead(dev_handle,	/* FileHandle */
		 inbuf,	/* BufferArea */
		 inbuf_size,	/* BufferLength */
		 &inbuf_len);	/* BytesWritten */
    if (rc && rc != ERROR_NO_DATA)
      p_error(P_ERROR_DOSREAD, rc,
	      MODULE_PIPE, __LINE__, (U32)dev_path);
    if (inbuf_len)		/* Got something */
      break;
    else
      DosSleep(30);
  }
}

_Inline void pipe_flush_outbuf(void) {

  static APIRET rc=0;
  static U8 *buf;
  static U32 BytesWritten;
  static U32 cnt;

  buf = outbuf;
  for (cnt = 0; cnt < 300; cnt++) {
    rc = DosWrite(dev_handle,		/* FileHandle */
		  buf,			/* BufferArea */
		  outbuf_idx,		/* BufferLength */
		  &BytesWritten);	/* BytesWritten */
    if (rc)
      p_error(P_ERROR_DOSWRITE, rc,
	      MODULE_PIPE, __LINE__,
	      (U32)dev_path);
    if (BytesWritten == outbuf_idx) {
      outbuf_idx = 0;
      return;
    } else {
      DosSleep(10);
      if (BytesWritten) {	/* We got something transferred? */
	buf += BytesWritten;
	outbuf_idx -= BytesWritten;
      }
    }
  }
  p_error(P_ERROR_DOSWRITE, rc,
	  MODULE_PIPE, __LINE__, (U32)dev_path);
}

_Inline U32 pipe_connected(void) {

  return(0);
}

_Inline void pipe_set_break_on(void) {

}

_Inline void pipe_set_break_off(void) {

}
#endif /* NT */
#endif /* XYZ_DLL */
