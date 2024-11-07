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

/* TCP/IP stream socket communication routines as inline functions */

#ifdef XYZ_DLL
#ifdef NT
#define sock_errno WSAGetLastError
#define soclose closesocket
#ifdef __GNUC__
#define _Inline static
#endif /* __GNUC__ */
#endif 

#ifdef NT
#define socket_handle   (*(SOCKET *)&dev_handle)
#else /* NT */
#define socket_handle   (*(int *)&dev_handle)
#endif /* NT */

_Inline void tcp_open(void) {

  U32 true = 1;
  struct hostent *hostnm;    /* server host name information */
  struct sockaddr_in server; /* server address information */
#ifdef NT
   WSADATA wsadata ;
#endif 

#ifdef NT
  if (!socket_handle) 
      if( WSAStartup( MAKEWORD( 2, 0 ), &wsadata ) )
#else
   if (sock_init())
#endif
    p_error(P_ERROR_SOCK_INIT, 0,
	    MODULE_SOCKET, __LINE__, 0);

  if (!socket_handle) {
    if (dev_server) {
      if ((passive_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	p_error(P_ERROR_SOCKET, sock_errno(),
		MODULE_SOCKET, __LINE__, 0);

      if (setsockopt(passive_socket, SOL_SOCKET, SO_REUSEADDR,
		     (char *)&true, sizeof(true)))
	p_error(P_ERROR_SETSOCKOPT, sock_errno(),
		MODULE_SOCKET, __LINE__, passive_socket);

      server.sin_family = AF_INET;
      server.sin_port = htons(socket_port);
      server.sin_addr.s_addr = INADDR_ANY;
      if (bind(passive_socket,
	       (struct sockaddr *)&server, sizeof(server)) == -1)
	p_error(P_ERROR_BIND, sock_errno(),
		MODULE_SOCKET, __LINE__, passive_socket);
      if (listen(passive_socket, 1) == -1)
	p_error(P_ERROR_LISTEN, sock_errno(),
		MODULE_SOCKET, __LINE__, passive_socket);
      /***********************************************/
      /* Set the passive handle to non-blocking mode */
      /***********************************************/
      if (
#ifdef NT
ioctlsocket( passive_socket, FIONBIO, &true )
#else
ioctl(passive_socket, FIONBIO, (char *)&true, sizeof(U32))
#endif
   )
	p_error(P_ERROR_IOCTL, sock_errno(),
		MODULE_SOCKET, __LINE__,
		passive_socket);
    } else {			/* We are the client? */
      server.sin_family = AF_INET;
      server.sin_port = htons(socket_port);
      if ((server.sin_addr.s_addr = inet_addr(socket_remote)) == -1) {
	/* Remote's address wasn't in the dotted-decimal format */
	if ((hostnm = gethostbyname(socket_remote)) == NULL)
	  p_error(P_ERROR_GETHOSTBYNAME, 
#ifdef NT
              WSAGetLastError(),
#else
              *h_errno,
#endif
		  MODULE_SOCKET, __LINE__, (intptr_t)socket_remote);
	server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
      }
      if ((socket_handle = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	p_error(P_ERROR_SOCKET, sock_errno(),
		MODULE_SOCKET, __LINE__, 0);
      if (connect(socket_handle,
		  (struct sockaddr *)&server, sizeof(server)) == -1)
	p_error(P_ERROR_CONNECT, sock_errno(),
		MODULE_SOCKET, __LINE__, socket_handle);
      if (
#ifdef NT
ioctlsocket( socket_handle, FIONBIO, &true )
#else
ioctl(socket_handle, FIONBIO, (char *)&true, sizeof(U32))
#endif
   )
	p_error(P_ERROR_IOCTL, sock_errno(),
		MODULE_SOCKET, __LINE__,
		socket_handle);
    }
  }
}

_Inline U32 tcp_connect(void) {

  U32 namelen;
  U32 true = 1;
  struct hostent *hostnm;    /* client host name information */
  struct sockaddr_in client; /* client address information */

  namelen = sizeof(client);
  socket_handle = accept(passive_socket,
		      (struct sockaddr *)&client,
		      (int *)&namelen);
  if (socket_handle == -1) {
    if (sock_errno() == 
#ifdef NT
         WSAEWOULDBLOCK
#else
         SOCEWOULDBLOCK
#endif 
         ) {
      return(0);
    } else
      p_error(P_ERROR_ACCEPT, sock_errno(),
	      MODULE_SOCKET, __LINE__, passive_socket);
  }
  if ((hostnm = gethostbyaddr((char *)&client.sin_addr,
			      sizeof(client.sin_addr),
			      AF_INET)) == NULL)
    p_error(P_ERROR_GETHOSTBYADDR, 
#ifdef NT
             WSAGetLastError(),
#else
             *h_errno,
#endif 
	    MODULE_SOCKET, __LINE__, (intptr_t)inet_ntoa(client.sin_addr));
  socket_remote = hostnm->h_name;

      if (
#ifdef NT
ioctlsocket( socket_handle, FIONBIO, &true )
#else
ioctl(socket_handle, FIONBIO, (char *)&true, sizeof(U32))
#endif
   )
    p_error(P_ERROR_IOCTL, sock_errno(),
	    MODULE_SOCKET, __LINE__,
	    socket_handle);
  return(1);
}

_Inline void tcp_close(void) {

  if (dev_server)
    soclose(passive_socket);
  soclose(socket_handle);
}

_Inline U32 tcp_incoming(void) {

  static S32 rc = 0;
#ifdef NT
   fd_set rfds;
   struct timeval tv;
   FD_ZERO(&rfds);
   FD_SET(socket_handle, &rfds);
   tv.tv_sec  = tv.tv_usec = 0L;
   if ( select(FD_SETSIZE, &rfds, NULL, NULL, &tv ) > 0 &&
         FD_ISSET(socket_handle, &rfds) )
      return 1 ;
   else return 0 ;
#else 
  rc = select(&socket_handle,
		  1, 0, 0,		/* Num of read, write and */
					/* special sockets */
		  0);		/* No timeout */
  if (rc == -1)
    p_error(P_ERROR_SELECT, sock_errno(),
	    MODULE_SOCKET, __LINE__, socket_handle);
#endif 
  if (rc)
    return(1);
  else
    return(0);
}

_Inline void tcp_getch_buf(void) {

  U32 tmout_cnt;

  inbuf_idx = 0;
  inbuf_len = 0;
  for (tmout_cnt = 0; tmout_cnt < timeouts_per_call; tmout_cnt++) {
    inbuf_len = recv(socket_handle, inbuf, inbuf_size, 0);
    if ((S32)inbuf_len > 0) {		/* Data received ok? */
      return;
    } else if (inbuf_len == -1 && sock_errno() == 
#ifdef NT
                WSAEWOULDBLOCK
#else
                SOCEWOULDBLOCK
#endif
                ) {
      inbuf_len = 0;
      DosSleep(30);
    } else
      p_error(P_ERROR_RECV, sock_errno(),
		MODULE_SOCKET, __LINE__, socket_handle);
  }
}

_Inline void tcp_flush_outbuf(void) {

  static U8 *buf;
  static U32 cnt;
  static S32 ret_val;

  buf = outbuf;
  for (cnt = 0; cnt < 300; cnt++) {
    ret_val = send(socket_handle, buf, outbuf_idx, 0);
    if (ret_val == outbuf_idx) {	/* Data sent ok? */
      outbuf_idx = 0;
      return;
    } else if (ret_val == -1 && sock_errno() == 
#ifdef NT
                WSAEWOULDBLOCK
#else
                SOCEWOULDBLOCK
#endif
                ) {
      DosSleep(10);
    }
    else if (ret_val == -1)
      p_error(P_ERROR_SEND, sock_errno(),
	      MODULE_SOCKET, __LINE__,
	      socket_handle);
    else {			/* We got something transferred... */
      DosSleep(10);
      buf += ret_val;
      outbuf_idx -= ret_val;
    }
  }
  p_error(P_ERROR_SEND, sock_errno(),
	  MODULE_SOCKET, __LINE__,
	  socket_handle);
}

_Inline U32 tcp_connected(void) {

  return(0);
}

_Inline void tcp_set_break_on(void) {

}

_Inline void tcp_set_break_off(void) {

}
#endif /* XYZ_DLL */
