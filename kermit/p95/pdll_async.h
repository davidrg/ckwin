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

/* Asynchronous communication routines as inline functions */

#ifdef XYZ_DLL

#ifdef NT
#ifdef __GNUC__
#define _Inline static
#endif /* __GNUC__ */
#endif /* NT */

_Inline U32 async_connected(void) {

  static APIRET rc=0;
#ifdef NT
   DWORD ModemStat ;
   if ( !GetCommModemStatus( (HANDLE) dev_handle, &ModemStat ) )
   {
      p_error(P_ERROR_DOSDEVIOCTL, GetLastError(),
               MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
      return 0;
   }
   return ( ModemStat & MS_RLSD_ON ) ;
#else /* NT */
  static U8 DataArea;
  static U32 DataLengthInOut;
  
  DataLengthInOut = sizeof(U8);
  rc = DosDevIOCtl(dev_handle,		/* FileHandle */
		   IOCTL_ASYNC, 		/* Category */
		   ASYNC_GETMODEMINPUT,		/* Function */
		   NULL,			/* ParmList */
		   0L,				/* ParmLengthMax */
		   NULL,			/* ParmLengthInOut */
		   &DataArea,			/* DataArea */
		   sizeof(U8),		/* DataLengthMax */
		   &DataLengthInOut);		/* DataLengthInOut */
  if (rc)
    p_error(P_ERROR_DOSDEVIOCTL, rc,
	    MODULE_ASYNC, __LINE__, (U32)dev_path);
  if (DataArea & DCD_ON)
    return(1);
  else
    return(0);
#endif /* NT */
}

_Inline void async_open(void) {

  APIRET rc=0;
  U32 ActionTaken;
  U32 mode;

  if (!dev_handle) {
#ifdef NT
    if ( (HANDLE)(dev_handle = (intptr_t) CreateFile(dev_path,
        GENERIC_READ | GENERIC_WRITE,
        dev_shared,  
        NULL,               /* no security specified */
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL)) == INVALID_HANDLE_VALUE)
        rc = GetLastError() ;
#else /* NT */
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
#endif /* NT */
    if (rc)
      p_error(P_ERROR_DOSOPEN, rc,
	      MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
  }
}

_Inline void async_close(void) {

  APIRET rc = 0;

#ifdef NT
   if ( !CloseHandle( (HANDLE) dev_handle ) )
      rc = GetLastError() ;
#else /* NT */
   rc = DosClose(dev_handle);
#endif /* NT */
  if (rc)
    p_error(P_ERROR_DOSCLOSE, rc,
	    MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
}

_Inline U32 async_incoming(void) {

  static APIRET rc = 0 ;
#ifdef NT
    DWORD errors ;
    COMSTAT comstat ;
#else /* NT */
   static U32 DataLengthInOut;
   static U16 DataArea[2];
#endif /* NT */
  
  if (inbuf_idx != inbuf_len)	/* We have data in input buffer */
    return(1);

#ifdef NT
   if ( !ClearCommError( (HANDLE) dev_handle, &errors, &comstat ) )
      rc = GetLastError() ;
#else /* NT */
   DataLengthInOut = 2 * sizeof(U16);
   rc = DosDevIOCtl(dev_handle,		/* FileHandle */
		   IOCTL_ASYNC, 		/* Category */
		   ASYNC_GETINQUECOUNT,		/* Function */
		   NULL,			/* ParmList */
		   0L,				/* ParmLengthMax */
		   NULL,			/* ParmLengthInOut */
		   &DataArea,			/* DataArea */
		   2 * sizeof(U16),		/* DataLengthMax */
		   &DataLengthInOut);		/* DataLengthInOut */
#endif /* NT */
  if (rc)
    p_error(P_ERROR_DOSDEVIOCTL, rc,
	    MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
#ifdef NT
   if ( comstat.cbInQue )
#else /* NT */
   if (DataArea[0] > 0)
#endif /* NT */
    return(1);
  else
    return(0);
}


_Inline void async_getch_buf(void) {

#ifdef NT
extern OVERLAPPED overlapped_read;
extern int nActuallyRead ;
#endif /* NT */
  APIRET rc=0;
  U32 tmout_cnt;

  if (watch_carrier && !async_connected())
    pdll_carrier_lost();

  inbuf_idx = 0;
  inbuf_len = 0;
#ifdef NT
       if ( overlapped_read.hEvent == (HANDLE) -1 )
       {
          overlapped_read.hEvent = CreateEvent( NULL,    // no security
                                                 TRUE,    // explicit reset req
                                                 FALSE,   // initial event reset
                                                 NULL ) ; // no name
          if ( overlapped_read.hEvent == (HANDLE) -1 )
          {
             p_error(P_ERROR_DOSREAD, GetLastError(),
                      MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
          }
       }
       overlapped_read.Offset = overlapped_read.OffsetHigh = 0 ;
       ResetEvent( overlapped_read.hEvent ) ;

       if ( !ReadFile( (HANDLE) dev_handle, inbuf, inbuf_size, &nActuallyRead,
                         &overlapped_read) )
       {
          DWORD error = GetLastError() ;
          if ( error == ERROR_IO_PENDING )
          {
             while(!GetOverlappedResult( (HANDLE) dev_handle, &overlapped_read, &nActuallyRead, TRUE ))
             {
               DWORD error = GetLastError() ;
                if ( error == ERROR_IO_INCOMPLETE )
                {
                   continue;
                }
                else if ( error == ERROR_OPERATION_ABORTED )
                {
                   p_error(P_ERROR_DOSREAD, error,
                            MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
                }
                else
                {
                   DWORD errorflags ;
                   COMSTAT comstat ;
                   p_error(P_ERROR_DOSREAD, error,
                            MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
                   ClearCommError( (HANDLE) dev_handle, &errorflags, &comstat ) ;
                   return ;
                }
             }            
          }
          else 
          {
             p_error(P_ERROR_DOSREAD, error,
                      MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
             return ;
          }
       }
       ResetEvent( overlapped_read.hEvent ) ;
       inbuf_len = nActuallyRead ;
#else 
   for (tmout_cnt = 0; tmout_cnt < timeouts_per_call; tmout_cnt++) {
    rc = DosRead(dev_handle,	/* FileHandle */
		 inbuf,		/* BufferArea */
		 inbuf_size,	/* BufferLength */
		 &inbuf_len);	/* BytesWritten */
    if (rc)
      p_error(P_ERROR_DOSREAD, rc,
	      MODULE_ASYNC, __LINE__, (U32)dev_path);
    if (inbuf_len)		/* Got something */
      break;
  }
#endif

}

_Inline void async_flush_outbuf(void) {

  static APIRET rc=0;
  static U32 BytesWritten;

#ifdef NT
   extern OVERLAPPED overlapped_write;
   extern int nActuallyWritten ;

       if ( overlapped_write.hEvent == (HANDLE) -1 )
       {
          overlapped_write.hEvent = CreateEvent( NULL,    // no security
                                                 TRUE,    // explicit reset req
                                                 FALSE,   // initial event reset
                                                 NULL ) ; // no name
          if ( overlapped_write.hEvent == (HANDLE) -1 )
          {
                   p_error(P_ERROR_DOSWRITE, GetLastError(),
                            MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
          }
       }
       overlapped_write.Offset = overlapped_write.OffsetHigh = 0 ;
       ResetEvent( overlapped_write.hEvent ) ;
       nActuallyWritten = 0 ;
       if ( !WriteFile( (HANDLE) dev_handle, outbuf, outbuf_idx, &nActuallyWritten,
                         &overlapped_write) )
       {
          DWORD error = GetLastError() ;
          if ( error == ERROR_IO_PENDING )
          {
             while(!GetOverlappedResult( (HANDLE) dev_handle, &overlapped_write, &nActuallyWritten, TRUE ))
             {
               DWORD error = GetLastError() ;
                if ( error == ERROR_IO_INCOMPLETE )
                   continue;
                else if ( error == ERROR_OPERATION_ABORTED )
                {
                   p_error(P_ERROR_DOSWRITE, error,
                            MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
                    return ;
                }
                else
                {
                   DWORD errorflags ;
                   COMSTAT comstat ;
                   p_error(P_ERROR_DOSWRITE, error,
                            MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
                   ClearCommError( (HANDLE) dev_handle, &errorflags, &comstat ) ;
                   return ;
                }
             }            
          }
          else 
          {
             p_error(P_ERROR_DOSWRITE, error,
                      MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
             return ;
          }
       }
       BytesWritten = nActuallyWritten ;
       ResetEvent( overlapped_write.hEvent ) ;
#else 
  rc = DosWrite(dev_handle,	/* FileHandle */
		outbuf,		/* BufferArea */
		outbuf_idx,	/* BufferLength */
		&BytesWritten);	/* BytesWritten */
  if (rc)
    p_error(P_ERROR_DOSWRITE, rc,
	    MODULE_ASYNC, __LINE__, (U32)dev_path);
#endif

  if (BytesWritten != outbuf_idx)
    p_error(P_ERROR_DOSWRITE, rc,
	    MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
  if (watch_carrier && !async_connected())
    pdll_carrier_lost();
  outbuf_idx = 0;
}

_Inline void async_set_break_on(void) {

  static APIRET rc = 0;
  static U16 DataArea;		/* com error */
  static U32 DataLengthInOut;
  
  DataLengthInOut = sizeof(U16);
#ifdef NT
   if ( SetCommBreak( (HANDLE) dev_handle ) )
      rc = GetLastError() ;
#else
   rc = DosDevIOCtl(dev_handle,		/* FileHandle */
		   IOCTL_ASYNC, 		/* Category */
		   ASYNC_SETBREAKON,		/* Function */
		   NULL,			/* ParmList */
		   0L,				/* ParmLengthMax */
		   NULL,			/* ParmLengthInOut */
		   &DataArea,			/* DataArea */
		   sizeof(U16),		/* DataLengthMax */
		   &DataLengthInOut);		/* DataLengthInOut */
#endif 
  if (rc)
    p_error(P_ERROR_DOSDEVIOCTL, rc,
	    MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
}

_Inline void async_set_break_off(void) {

  static APIRET rc = 0;
  static U16 DataArea;		/* com error */
  static U32 DataLengthInOut;
  
#ifdef NT
   if ( ClearCommBreak( (HANDLE) dev_handle ) )
      rc = GetLastError() ;
#else
  DataLengthInOut = sizeof(U16);
  rc = DosDevIOCtl(dev_handle,		/* FileHandle */
		   IOCTL_ASYNC, 		/* Category */
		   ASYNC_SETBREAKOFF,		/* Function */
		   NULL,			/* ParmList */
		   0L,				/* ParmLengthMax */
		   NULL,			/* ParmLengthInOut */
		   &DataArea,			/* DataArea */
		   sizeof(U16),		/* DataLengthMax */
		   &DataLengthInOut);		/* DataLengthInOut */
#endif
  if (rc)
    p_error(P_ERROR_DOSDEVIOCTL, rc,
	    MODULE_ASYNC, __LINE__, (intptr_t)dev_path);
}

#endif /* XYZ_DLL */
