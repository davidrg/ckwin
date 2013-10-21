/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

#define SECURITY_WIN32
#include <sspi.h>
#if(_WIN32_WINNT < 0x0400)
#define UNISP_NAME    "Microsoft Unified Security Protocol Provider"
#else
#include <wincrypt.h>
ALGIDDEF
#include <schnlsp.h>
#endif
#include <issperr.h>

#define SSLBUFLEN 8192


/* SSL I/O stream */

typedef struct ssl_stream {
  TCPSTREAM *tcpstream;         /* TCP stream */
  CredHandle cred;              /* SSL credentials */
  CtxtHandle context;           /* SSL context */
                                /* stream encryption sizes */
  SecPkgContext_StreamSizes sizes;
  int ictr;                     /* input counter */
  char *iptr;                   /* input pointer */
  int iextractr;                /* extra input counter */
  char *iextraptr;              /* extra input pointer */
  char *ibuf;                   /* input buffer */
  char *obuf;                   /* output buffer */
} SSLSTREAM;


/* SSL stdio stream */

typedef struct ssl_stdiostream {
  SSLSTREAM *sslstream;         /* SSL stream */
  int octr;                     /* output counter */
  char *optr;                   /* output pointer */
  char obuf[SSLBUFLEN];         /* output buffer */
} SSLSTDIOSTREAM;


/* SSL driver */

struct ssl_driver {             /* must parallel NETDRIVER in mail.h */
  SSLSTREAM *(*open) (char *host,char *service,unsigned long port);
  SSLSTREAM *(*aopen) (NETMBX *mb,char *service,char *usrbuf);
  char *(*getline) (SSLSTREAM *stream);
  long (*getbuffer) (SSLSTREAM *stream,unsigned long size,char *buffer);
  long (*soutr) (SSLSTREAM *stream,char *string);
  long (*sout) (SSLSTREAM *stream,char *string,unsigned long size);
  void (*close) (SSLSTREAM *stream);
  char *(*host) (SSLSTREAM *stream);
  char *(*remotehost) (SSLSTREAM *stream);
  unsigned long (*port) (SSLSTREAM *stream);
  char *(*localhost) (SSLSTREAM *stream);
};

/* Function prototypes */

void ssl_onceonlyinit (void);
SSLSTREAM *ssl_open (char *host,char *service,unsigned long port);
SSLSTREAM *ssl_aopen (NETMBX *mb,char *service,char *usrbuf);
char *ssl_getline (SSLSTREAM *stream);
long ssl_getbuffer (SSLSTREAM *stream,unsigned long size,char *buffer);
long ssl_getdata (SSLSTREAM *stream);
long ssl_soutr (SSLSTREAM *stream,char *string);
long ssl_sout (SSLSTREAM *stream,char *string,unsigned long size);
void ssl_close (SSLSTREAM *stream);
long ssl_abort (SSLSTREAM *stream);
char *ssl_host (SSLSTREAM *stream);
char *ssl_remotehost (SSLSTREAM *stream);
unsigned long ssl_port (SSLSTREAM *stream);
char *ssl_localhost (SSLSTREAM *stream);
long auth_plain_valid (void);
long auth_plain_client (authchallenge_t challenger,authrespond_t responder,
                        NETMBX *mb,void *stream,unsigned long *trial,
                        char *user);
char *auth_plain_server (authresponse_t responder,int argc,char *argv[]);
void Server_init (char *server,char *service,char *altservice,char *sasl,
                  void *clkint,void *kodint,void *hupint,void *trmint);
long Server_input_wait (long seconds);
SSLSTDIOSTREAM *ssl_server_init (char *server);
int ssl_getchar (void);
char *ssl_gets (char *s,int n);
int ssl_putchar (int c);
int ssl_puts (char *s);
int ssl_flush (void);

/* Secure Sockets Layer network driver dispatch */

static struct ssl_driver ssldriver = {
  ssl_open,                     /* open connection */
  ssl_aopen,                    /* open preauthenticated connection */
  ssl_getline,                  /* get a line */
  ssl_getbuffer,                /* get a buffer */
  ssl_soutr,                    /* output pushed data */
  ssl_sout,                     /* output string */
  ssl_close,                    /* close connection */
  ssl_host,                     /* return host name */
  ssl_remotehost,               /* return remote host name */
  ssl_port,                     /* return port number */
  ssl_localhost                 /* return local host name */
};

                                /* security function table */
static SecurityFunctionTable *sslfunc = NIL;
                                /* message encryption */
static ENCRYPT_MESSAGE_FN sslencrypt = NIL;
                                /* message decryption */
static DECRYPT_MESSAGE_FN ssldecrypt = NIL;
static char *sslpnm = NIL;      /* SSL package name */
static unsigned long ssltsz = 0;/* SSL maximum token length */
                                /* non-NIL if doing SSL primary I/O */
static SSLSTDIOSTREAM *sslstdio = NIL;


/* Secure sockets layer authenticator */

AUTHENTICATOR auth_ssl = {
  NIL,                          /* insecure authenticator */
  "PLAIN",                      /* authenticator name */
  auth_plain_valid,             /* check if valid */
  auth_plain_client,            /* client method */
  auth_plain_server,            /* server method */
  NIL                           /* next authenticator */
};

/* One-time SSL initialization */

static int sslonceonly = 0;

void ssl_onceonlyinit (void)
{
    if (!sslonceonly++) {               /* only need to call it once */
        HINSTANCE lib;
        FARPROC pi;
        ULONG np;
        SecPkgInfo *pp;
        int i;
                                /* get security library */
        if (((lib = LoadLibrary ("schannel.dll")) ||
              (lib = LoadLibrary ("security.dll"))) &&
             (pi = GetProcAddress (lib,SECURITY_ENTRYPOINT)) &&
             (sslfunc = (SecurityFunctionTable *) pi ()) &&
             !(sslfunc->EnumerateSecurityPackages (&np,&pp)))
        {
                                /* look for an SSL package */
            for (i = 0; (i < (int) np) && !sslpnm; i++)
                if (!strcmp (pp[i].Name,UNISP_NAME)) {
                    /* note maximum token size and name */
                    ssltsz = pp[i].cbMaxToken;
                    sslpnm = cpystr (pp[i].Name);
                    /* Shh!  It's a military secret!! */
                    sslencrypt = sslfunc->Reserved3;
                    /* so is this!!! */
                    ssldecrypt = sslfunc->Reserved4;
                }
        }
    }
}

/* SSL open
 * Accepts: host name
 *          contact service name
 *          contact port number
 * Returns: SSL stream if success else NIL
 */

SSLSTREAM *
ssl_open (char *host,char *service,unsigned long port)
{
    SECURITY_STATUS e;
    ULONG a;
    TimeStamp t;
    SecBuffer ibuf[2],obuf[1];
    SecBufferDesc ibufs,obufs;
    SSLSTREAM *stream = NIL;
    ULONG req = ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT |
        ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY |
            ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM | ISC_REQ_EXTENDED_ERROR;
    int done = 0;
    TCPSTREAM *ts = tcp_open (host,service,port);
    int failed = T;
    if (ts) {                   /* got a TCPSTREAM? */
        char *buf = (char *) fs_get (ssltsz);
        unsigned long size = 0;
                                /* instantiate SSLSTREAM */
        (stream = (SSLSTREAM *) memset (fs_get (sizeof (SSLSTREAM)),0,
                                         sizeof (SSLSTREAM)))->tcpstream = ts;

        if (sslfunc->AcquireCredentialsHandle
             (NIL,sslpnm,SECPKG_CRED_OUTBOUND,NIL,NIL,NIL,NIL,&stream->cred,&t) ==
             SEC_E_OK) {                /* acquire credentials */
            while (!done) {             /* negotiate negotiate security context */
                /* initialize buffers */
                ibuf[0].cbBuffer = size; ibuf[0].pvBuffer = buf;
                ibuf[1].cbBuffer = 0; ibuf[1].pvBuffer = NIL;
                obuf[0].cbBuffer = 0; obuf[0].pvBuffer = NIL;
                ibuf[0].BufferType = obuf[0].BufferType = SECBUFFER_TOKEN;
                ibuf[1].BufferType = SECBUFFER_EMPTY;
                                /* initialize buffer descriptors */
                ibufs.ulVersion = obufs.ulVersion = SECBUFFER_VERSION;
                ibufs.cBuffers = 2; obufs.cBuffers = 1;
                ibufs.pBuffers = ibuf; obufs.pBuffers = obuf;
                                /* negotiate security */
                e = sslfunc->InitializeSecurityContext
                    (&stream->cred,size ? &stream->context : NIL,0,req,0,
                      SECURITY_NETWORK_DREP,size ? &ibufs : NIL,0,
                      &stream->context,&obufs,&a,&t);
                                /* have an output buffer we need to send? */
                if (obuf[0].pvBuffer && obuf[0].cbBuffer) {
                    if ((done >= 0) &&  /* do so if in good state */
                         !tcp_sout (stream->tcpstream,obuf[0].pvBuffer,
                                     obuf[0].cbBuffer)) done = -1;
                                /* free the buffer */
                    sslfunc->FreeContextBuffer (obuf[0].pvBuffer);
                }

                switch (e) {            /* negotiation state */
                case SEC_I_INCOMPLETE_CREDENTIALS:
                    break;              /* server wants client auth */
                case SEC_E_OK:
                    done = T;           /* got security context, all done */
                    /* any data to be regurgitated? */
                    if (ibuf[1].BufferType == SECBUFFER_EXTRA) {
                        /* yes, set this as the new data */
                        memmove (stream->tcpstream->iptr = stream->tcpstream->ibuf,
                                  buf + size - ibuf[1].cbBuffer,ibuf[1].cbBuffer);
                        stream->tcpstream->ictr = ibuf[1].cbBuffer;
                    }
                    if (sslfunc->QueryContextAttributes
                         (&stream->context,SECPKG_ATTR_STREAM_SIZES,&stream->sizes) ==
                         SEC_E_OK) {    /* get stream sizes */
                        /* maximum SSL buffer size */
                        size_t i = stream->sizes.cbHeader +
                            stream->sizes.cbMaximumMessage + stream->sizes.cbTrailer;
                        /* make buffers */
                        stream->ibuf = (char *) fs_get (i);
                        stream->obuf = (char *) fs_get (i);
                        failed = NIL;   /* mark success */
                    }
                    break;
                case SEC_I_CONTINUE_NEEDED:
                    if (size) {         /* continue, read any data? */
                        /* yes, anything regurgiated back to us? */
                        if (ibuf[1].BufferType == SECBUFFER_EXTRA) {
                            /* yes, set this as the new data */
                            memmove (buf,buf + size - ibuf[1].cbBuffer,ibuf[1].cbBuffer);
                            size = ibuf[1].cbBuffer;
                            break;
                        }
                        size = 0;               /* otherwise, read more stuff from server */
                    }
                case SEC_E_INCOMPLETE_MESSAGE:
                    /* need to read more data from server */
                    if (tcp_getdata (stream->tcpstream)) {
                        memcpy(buf+size,stream->tcpstream->iptr,stream->tcpstream->ictr);
                        size += stream->tcpstream->ictr;
                        /* empty it from TCP's buffers */
                        stream->tcpstream->iptr += stream->tcpstream->ictr;
                        stream->tcpstream->ictr = 0;
                        break;
                    }
                default:                /* anything else is an error */
                    done = -1;
                    break;
                }
            }
        }

        if (failed) {           /* failed to negotiate SSL */
            /* only give error if not silent */
            if (!(port & 0x80000000)) {
                sprintf (buf,"Can't establish SSL session to %.80s/%.80s,%ld",
                          host,service ? service + 1 : "SSL",port);
                mm_log (buf,ERROR);
            }
            ssl_close (stream); /* failed to do SSL */
        }
        fs_give ((void **) &buf);       /* flush temporary buffer */
    }
    return failed ? NIL :  stream;
}


/* SSL authenticated open
 * Accepts: host name
 *          service name
 *          returned user name buffer
 * Returns: SSL stream if success else NIL
 */

SSLSTREAM *ssl_aopen (NETMBX *mb,char *service,char *usrbuf)
{
  return NIL;                   /* don't use this mechanism with SSL */
}

/* SSL receive line
 * Accepts: SSL stream
 * Returns: text line string or NIL if failure
 */

char *ssl_getline (SSLSTREAM *stream)
{
  int n,m;
  char *st,*ret,*stp;
  char c = '\0';
  char d;
                                /* make sure have data */
  if (!ssl_getdata (stream)) return NIL;
  st = stream->iptr;            /* save start of string */
  n = 0;                        /* init string count */
  while (stream->ictr--) {      /* look for end of line */
    d = *stream->iptr++;        /* slurp another character */
    if ((c == '\015') && (d == '\012')) {
      ret = (char *) fs_get (n--);
      memcpy (ret,st,n);        /* copy into a free storage string */
      ret[n] = '\0';            /* tie off string with null */
      return ret;
    }
    n++;                        /* count another character searched */
    c = d;                      /* remember previous character */
  }
                                /* copy partial string from buffer */
  memcpy ((ret = stp = (char *) fs_get (n)),st,n);
                                /* get more data from the net */
  if (!ssl_getdata (stream)) fs_give ((void **) &ret);
                                /* special case of newline broken by buffer */
  else if ((c == '\015') && (*stream->iptr == '\012')) {
    stream->iptr++;             /* eat the line feed */
    stream->ictr--;
    ret[n - 1] = '\0';          /* tie off string with null */
  }
                                /* else recurse to get remainder */
  else if (st = ssl_getline (stream)) {
    ret = (char *) fs_get (n + 1 + (m = strlen (st)));
    memcpy (ret,stp,n);         /* copy first part */
    memcpy (ret + n,st,m);      /* and second part */
    fs_give ((void **) &stp);   /* flush first part */
    fs_give ((void **) &st);    /* flush second part */
    ret[n + m] = '\0';          /* tie off string with null */
  }
  return ret;
}

/* SSL receive buffer
 * Accepts: SSL stream
 *          size in bytes
 *          buffer to read into
 * Returns: T if success, NIL otherwise
 */

long ssl_getbuffer (SSLSTREAM *stream,unsigned long size,char *buffer)
{
  unsigned long n;
  while (size > 0) {            /* until request satisfied */
    if (!ssl_getdata (stream)) return NIL;
    n = min (size,stream->ictr);/* number of bytes to transfer */
                                /* do the copy */
    memcpy (buffer,stream->iptr,n);
    buffer += n;                /* update pointer */
    stream->iptr += n;
    size -= n;                  /* update # of bytes to do */
    stream->ictr -= n;
  }
  buffer[0] = '\0';             /* tie off string */
  return T;
}

/* SSL receive data
 * Accepts: TCP/IP stream
 * Returns: T if success, NIL otherwise
 */

long ssl_getdata (SSLSTREAM *stream)
{
  SECURITY_STATUS status;
  SecBuffer buf[4];
  SecBufferDesc msg;
  size_t n = 0;
  size_t i;
  while (stream->ictr < 1) {    /* decrypted buffer empty? */
    do {                        /* yes, make sure have data from TCP */
      if (stream->iextractr) {  /* have previous unread data? */
        memcpy (stream->ibuf + n,stream->iextraptr,stream->iextractr);
        n += stream->iextractr; /* update number of bytes read */
        stream->iextractr = 0;  /* no more extra data */
      }
      else {                    /* read from TCP */
        if (!tcp_getdata (stream->tcpstream)) return ssl_abort (stream);
                                /* maximum amount of data to copy */
        if (!(i = min (stream->sizes.cbMaximumMessage - n,
                       stream->tcpstream->ictr)))
          fatal ("incomplete SecBuffer already cbMaximumMessage");
                                /* do the copy */
        memcpy (stream->ibuf + n,stream->tcpstream->iptr,i);
        stream->tcpstream->iptr += i;
        stream->tcpstream->ictr -= i;
        n += i;                 /* update number of bytes to decrypt */
      }
      buf[0].cbBuffer = n;      /* first SecBuffer gets data */
      buf[0].pvBuffer = stream->ibuf;
      buf[0].BufferType = SECBUFFER_DATA;
                                /* subsequent ones are for spares */
      buf[1].BufferType = buf[2].BufferType = buf[3].BufferType =
        SECBUFFER_EMPTY;
      msg.ulVersion = SECBUFFER_VERSION;
      msg.cBuffers = 4;         /* number of SecBuffers */
      msg.pBuffers = buf;       /* first SecBuffer */
    } while ((status = ssldecrypt (&stream->context,&msg,0,NIL)) ==
             SEC_E_INCOMPLETE_MESSAGE);
    switch (status) {
    case SEC_E_OK:              /* won */
    case SEC_I_RENEGOTIATE:     /* won but lost it after this buffer */
                                /* hunt for a buffer */
      for (i = 0; (i < 4) && (buf[i].BufferType != SECBUFFER_DATA) ; i++);
      if (i < 4) {              /* found a buffer? */
                                /* yes, set up pointer and counter */
        stream->iptr = buf[i].pvBuffer;
        stream->ictr = buf[i].cbBuffer;
                                /* any unprocessed data? */
        while (++i < 4) if (buf[i].BufferType == SECBUFFER_EXTRA) {
                                /* yes, note for next time around */
          stream->iextraptr = buf[i].pvBuffer;
          stream->iextractr = buf[i].cbBuffer;
        }
      }
      break;
    default:                    /* anything else means we've lost */
      return ssl_abort (stream);
    }
  }
  return LONGT;
}

/* SSL send string as record
 * Accepts: SSL stream
 *          string pointer
 * Returns: T if success else NIL
 */

long ssl_soutr (SSLSTREAM *stream,char *string)
{
  return ssl_sout (stream,string,(unsigned long) strlen (string));
}


/* SSL send string
 * Accepts: SSL stream
 *          string pointer
 *          byte count
 * Returns: T if success else NIL
 */

long ssl_sout (SSLSTREAM *stream,char *string,unsigned long size)
{
  SecBuffer buf[4];
  SecBufferDesc msg;
  char *s = stream->ibuf;
  size_t n = 0;
  while (size) {                /* until satisfied request */
                                /* header */
    buf[0].BufferType = SECBUFFER_STREAM_HEADER;
    memset (buf[0].pvBuffer = stream->obuf,0,
            buf[0].cbBuffer = stream->sizes.cbHeader);
                                /* message (up to maximum size) */
    buf[1].BufferType = SECBUFFER_DATA;
    memcpy (buf[1].pvBuffer = stream->obuf + stream->sizes.cbHeader,string,
            buf[1].cbBuffer = min (size,stream->sizes.cbMaximumMessage));
                                /* trailer */
    buf[2].BufferType = SECBUFFER_STREAM_TRAILER;
    memset (buf[2].pvBuffer = ((char *) buf[1].pvBuffer) + buf[1].cbBuffer,0,
            buf[2].cbBuffer = stream->sizes.cbTrailer);
                                /* spare */
    buf[3].BufferType = SECBUFFER_EMPTY;
    msg.ulVersion = SECBUFFER_VERSION;
    msg.cBuffers = 4;           /* number of SecBuffers */
    msg.pBuffers = buf;         /* first SecBuffer */
    string += buf[1].cbBuffer;
    size -= buf[1].cbBuffer;    /* this many bytes processed */
                                /* encrypt and send message */
    if ((sslencrypt (&stream->context,0,&msg,NIL) != SEC_E_OK) ||
        !tcp_sout (stream->tcpstream,stream->obuf,
                   buf[0].cbBuffer + buf[1].cbBuffer + buf[2].cbBuffer))
      return ssl_abort (stream);/* encryption or sending failed */
  }
  return LONGT;
}

/* SSL close
 * Accepts: SSL stream
 */

void ssl_close (SSLSTREAM *stream)
{
  ssl_abort (stream);           /* nuke the stream */
  fs_give ((void **) &stream);  /* flush the stream */
}


/* SSL abort stream
 * Accepts: SSL stream
 * Returns: NIL always
 */

long ssl_abort (SSLSTREAM *stream)
{
  if (stream->tcpstream) {      /* close TCP stream */
    sslfunc->DeleteSecurityContext (&stream->context);
    sslfunc->FreeCredentialHandle (&stream->cred);
    tcp_close (stream->tcpstream);
    stream->tcpstream = NIL;
  }
  if (stream->ibuf) fs_give ((void **) &stream->ibuf);
  if (stream->obuf) fs_give ((void **) &stream->obuf);
  return NIL;
}

/* SSL get host name
 * Accepts: SSL stream
 * Returns: host name for this stream
 */

char *ssl_host (SSLSTREAM *stream)
{
  return tcp_host (stream->tcpstream);
}


/* SSL get remote host name
 * Accepts: SSL stream
 * Returns: host name for this stream
 */

char *ssl_remotehost (SSLSTREAM *stream)
{
  return tcp_remotehost (stream->tcpstream);
}


/* SSL return port for this stream
 * Accepts: SSL stream
 * Returns: port number for this stream
 */

unsigned long ssl_port (SSLSTREAM *stream)
{
  return tcp_port (stream->tcpstream);
}


/* SSL get local host name
 * Accepts: SSL stream
 * Returns: local host name
 */

char *ssl_localhost (SSLSTREAM *stream)
{
  return tcp_localhost (stream->tcpstream);
}

/* Client authenticator
 * Accepts: challenger function
 *          responder function
 *          parsed network mailbox structure
 *          stream argument for functions
 *          pointer to current trial count
 *          returned user name
 * Returns: T if success, NIL otherwise, number of trials incremented if retry
 */

long auth_plain_client (authchallenge_t challenger,authrespond_t responder,
                        NETMBX *mb,void *stream,unsigned long *trial,
                        char *user)
{
  char *s,*t,*u,pwd[MAILTMPLEN];
  void *chal;
  unsigned long cl,sl;
  if (!mb->altflag)             /* snarl if not secure session */
    mm_log ("SECURITY PROBLEM: insecure server advertised AUTH=PLAIN",WARN);
                                /* get initial (empty) challenge */
  if ((chal = (*challenger) (stream,&cl)) && !cl) {
    fs_give ((void **) &chal);
                                /* prompt user */
    mm_login (mb,user,pwd,*trial);
    if (!pwd[0]) {              /* user requested abort */
      (*responder) (stream,NIL,0);
      *trial = 0;               /* don't retry */
      return T;                 /* will get a NO response back */
    }
    t = s = (char *) fs_get (sl = strlen (user) + strlen (pwd) + 2);
    *t++ = '\0';                /* use authentication id as authorization id */
                                /* copy user name as authentication id */
    for (u = user; *u; *t++ = *u++);
    *t++ = '\0';                /* delimiting NUL */
                                /* copy password */
    for (u = pwd; *u; *t++ = *u++);
                                /* send credentials */
    if ((*responder) (stream,s,sl) && !(chal = (*challenger) (stream,&cl))) {
      fs_give ((void **) &s);   /* free response */
      ++*trial;                 /* can try again if necessary */
      return T;                 /* check the authentication */
    }
    fs_give ((void **) &s);     /* free response */
  }
  if (chal) fs_give ((void **) &chal);
  *trial = 0;                   /* don't retry */
  return NIL;                   /* failed */
}

/* Check if PLAIN valid on this system
 * Returns: T, always
 */

long auth_plain_valid (void)
{
                                /* server forbids PLAIN if not SSL */
  if (!sslstdio) auth_ssl.server = NIL;
  return T;                     /* PLAIN is otherwise valid */
}


/* Server authenticator
 * Accepts: responder function
 *          argument count
 *          argument vector
 * Returns: authenticated user name or NIL
 */

char *auth_plain_server (authresponse_t responder,int argc,char *argv[])
{
  char *ret = NIL;
  char *user,*aid,*pass;
  unsigned long len;
                                /* get user name */
  if (aid = (*responder) ("",0,&len)) {
                                /* note: responders null-terminate */
    if ((((unsigned long) ((user = aid + strlen (aid) + 1) - aid)) < len) &&
        (((unsigned long) ((pass = user + strlen (user) + 1) - aid)) < len) &&
        (((unsigned long) ((pass + strlen (pass)) - aid)) == len) &&
        !(*aid && strcmp (aid,user)) && server_login (user,pass,argc,argv))
      ret = myusername ();
    fs_give ((void **) &aid);
  }
  return ret;
}

/* Init server for SSL
 * Accepts: server name
 *          service name
 *          alternative service name
 * Returns: SSL stdio stream, always
 */

void server_init (char *server,char *service,char *altservice,char *sasl,
                  void *clkint,void *kodint,void *hupint,void *trmint)
{
  struct servent *sv;
  long port;
  if (server) {                 /* set server name in syslog */
    openlog (server,LOG_PID,LOG_MAIL);
    fclose (stderr);            /* possibly save a process ID */
  }
  /* Use SSL if alt service, or if server starts with "s" and not service */
  if (service && altservice && ((port = tcp_serverport ()) >= 0) &&
      (((sv = getservbyname (altservice,"tcp")) &&
        (port == ntohs (sv->s_port))) ||
       ((*server == 's') && (!(sv = getservbyname (service,"tcp")) ||
                             (port != ntohs (sv->s_port))))))
    sslstdio = (void *) ssl_server_init (server);
                                /* now call c-client's version */
  Server_init (NIL,service,altservice,sasl,clkint,kodint,hupint,trmint);
}

                                /* link to the real one */
#define server_init Server_init

/* Wait for stdin input
 * Accepts: timeout in seconds
 * Returns: T if have input on stdin, else NIL
 */

long server_input_wait (long seconds)
{
  SECURITY_STATUS status;
  SecBuffer buf[4];
  SecBufferDesc msg;
  struct timeval tmo;
  fd_set fds,efd;
  size_t n = 0;
  size_t i;
  SSLSTREAM *stream;
  if (!sslstdio) return Server_input_wait (seconds);
                                /* if no input available in buffer */
  while ((stream = sslstdio->sslstream)->ictr <= 0) {
    do {
      if (stream->iextractr) {  /* have previous unread data? */
        memcpy (stream->ibuf + n,stream->iextraptr,stream->iextractr);
        n += stream->iextractr; /* update number of bytes read */
        stream->iextractr = 0;  /* no more extra data */
      }
      else {                    /* if nothing in TCP buffer */
        if (stream->tcpstream->ictr < 1) {
          FD_ZERO (&fds);       /* initialize selection vector */
          FD_ZERO (&efd);       /* initialize selection vector */
          FD_SET (stream->tcpstream->tcps,&fds);
          FD_SET (stream->tcpstream->tcps,&efd);
          tmo.tv_sec = seconds; tmo.tv_usec = 0;
                                /* sniff and block until timeout */
          if (!select (stream->tcpstream->tcps+1,&fds,0,&efd,&tmo)) return NIL;
        }
                                /* now read that data */
        if (!tcp_getdata (stream->tcpstream)) return LONGT;
                                /* maximum amount of data to copy */
        if (!(i = min (stream->sizes.cbMaximumMessage - n,
                       stream->tcpstream->ictr)))
          fatal ("incomplete SecBuffer already cbMaximumMessage");
                                /* do the copy */
        memcpy (stream->ibuf + n,stream->tcpstream->iptr,i);
        stream->tcpstream->iptr += i;
        stream->tcpstream->ictr -= i;
        n += i;                 /* update number of bytes to decrypt */
      }
      buf[0].cbBuffer = n;      /* first SecBuffer gets data */
      buf[0].pvBuffer = stream->ibuf;
      buf[0].BufferType = SECBUFFER_DATA;
                                /* subsequent ones are for spares */
      buf[1].BufferType = buf[2].BufferType = buf[3].BufferType =
        SECBUFFER_EMPTY;
      msg.ulVersion = SECBUFFER_VERSION;
      msg.cBuffers = 4; /* number of SecBuffers */
      msg.pBuffers = buf;       /* first SecBuffer */
    } while ((status = ssldecrypt (&stream->context,&msg,0,NIL)) ==
             SEC_E_INCOMPLETE_MESSAGE);
    switch (status) {
    case SEC_E_OK:              /* won */
    case SEC_I_RENEGOTIATE:     /* won but lost it after this buffer */
                                /* hunt for a buffer */
      for (i = 0; (i < 4) && (buf[i].BufferType != SECBUFFER_DATA) ; i++);
      if (i < 4) {              /* found a buffer? */
                                /* yes, set up pointer and counter */
        stream->iptr = buf[i].pvBuffer;
        stream->ictr = buf[i].cbBuffer;
                                /* any unprocessed data? */
        while (++i < 4) if (buf[i].BufferType == SECBUFFER_EXTRA) {
                                /* yes, note for next time around */
          stream->iextraptr = buf[i].pvBuffer;
          stream->iextractr = buf[i].cbBuffer;
        }
      }
      break;
    default:                    /* anything else means we're sick */
      return LONGT;
    }
  }
  return LONGT;
}

                                /* link to the other one */
#define server_input_wait Server_input_wait

/* Init server for SSL
 * Accepts: server name
 * Returns: SSL stdio stream, always
 */

SSLSTDIOSTREAM *ssl_server_init (char *server)
{
  SECURITY_STATUS e;
  ULONG a;
  TimeStamp t;
  SecBuffer ibuf[2],obuf[1];
  SecBufferDesc ibufs,obufs;
  SSLSTDIOSTREAM *ret;
  SSLSTREAM *stream = (SSLSTREAM *) memset (fs_get (sizeof (SSLSTREAM)),0,
                                            sizeof (SSLSTREAM));
  ULONG req = ASC_REQ_REPLAY_DETECT | ASC_REQ_SEQUENCE_DETECT |
    ASC_REQ_CONFIDENTIALITY | ASC_REQ_USE_SESSION_KEY |
    ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_STREAM | ASC_REQ_EXTENDED_ERROR;
  int done = 0;
  int failed = T;
  int repeat = 0;
  char *buf = (char *) fs_get (ssltsz);
  unsigned long size;
  ssl_onceonlyinit ();          /* make sure algorithms added */
                                /* get credentials and inital client data */
  if ((sslfunc->AcquireCredentialsHandle
       (NIL,sslpnm,SECPKG_CRED_INBOUND,NIL,NIL,NIL,NIL,&stream->cred,&t) ==
       SEC_E_OK) && tcp_getdata (stream->tcpstream)) {
                                /* transfer initial client data */
    memcpy (buf,stream->tcpstream->iptr,size = stream->tcpstream->ictr);
                                /* empty it from TCP's buffers */
    stream->tcpstream->iptr += stream->tcpstream->ictr;
    stream->tcpstream->ictr = 0;
    while (!done) {             /* negotiate negotiate security context */
                                /* initialize buffers */
      ibuf[0].cbBuffer = size; ibuf[0].pvBuffer = buf;
      ibuf[1].cbBuffer = 0; ibuf[1].pvBuffer = NIL;
      obuf[0].cbBuffer = 0; obuf[0].pvBuffer = NIL;
      ibuf[0].BufferType = obuf[0].BufferType = SECBUFFER_TOKEN;
      ibuf[1].BufferType = SECBUFFER_EMPTY;
                                /* initialize buffer descriptors */
      ibufs.ulVersion = obufs.ulVersion = SECBUFFER_VERSION;
      ibufs.cBuffers = 2; obufs.cBuffers = 1;
      ibufs.pBuffers = ibuf; obufs.pBuffers = obuf;
                                /* negotiate security */
      e = sslfunc->AcceptSecurityContext
        (&stream->cred,repeat ? &stream->context : NIL,&ibufs,req,
         SECURITY_NETWORK_DREP,&stream->context,&obufs,&a,&t);
      repeat = T;               /* next call will use the context */
                                /* have an output buffer we need to send? */
      if (obuf[0].pvBuffer && obuf[0].cbBuffer) {
        if ((done >= 0) &&      /* do so if in good state */
            !tcp_sout (stream->tcpstream,obuf[0].pvBuffer,
                       obuf[0].cbBuffer)) done = -1;
                                /* free the buffer */
        sslfunc->FreeContextBuffer (obuf[0].pvBuffer);
      }

      switch (e) {              /* negotiation state */
      case SEC_E_OK:
        done = T;               /* got security context, all done */
                                /* any data to be regurgitated? */
        if (ibuf[1].BufferType == SECBUFFER_EXTRA) {
                                /* yes, set this as the new data */
            memmove (stream->tcpstream->iptr = stream->tcpstream->ibuf,
                     buf + size - ibuf[1].cbBuffer,ibuf[1].cbBuffer);
            stream->tcpstream->ictr = ibuf[1].cbBuffer;
          }
        if (sslfunc->QueryContextAttributes
            (&stream->context,SECPKG_ATTR_STREAM_SIZES,&stream->sizes) ==
            SEC_E_OK) { /* get stream sizes */
                                /* maximum SSL buffer size */
          size_t i = stream->sizes.cbHeader +
            stream->sizes.cbMaximumMessage + stream->sizes.cbTrailer;
                                /* make buffers */
          stream->ibuf = (char *) fs_get (i);
          stream->obuf = (char *) fs_get (i);
          failed = NIL; /* mark success */
        }
        break;
      case SEC_I_CONTINUE_NEEDED:
        if (size) {             /* continue, read any data? */
                                /* yes, anything regurgiated back to us? */
          if (ibuf[1].BufferType == SECBUFFER_EXTRA) {
                                /* yes, set this as the new data */
            memmove (buf,buf + size - ibuf[1].cbBuffer,ibuf[1].cbBuffer);
            size = ibuf[1].cbBuffer;
            break;
          }
          size = 0;             /* otherwise, read more stuff from server */
        }
      case SEC_E_INCOMPLETE_MESSAGE:
                                /* need to read more data from server */
        if (tcp_getdata (stream->tcpstream)) {
          memcpy (buf+size,stream->tcpstream->iptr,stream->tcpstream->ictr);
          size += stream->tcpstream->ictr;
                                /* empty it from TCP's buffers */
          stream->tcpstream->iptr += stream->tcpstream->ictr;
          stream->tcpstream->ictr = 0;
          break;
        }
      default:                  /* anything else is an error */
        done = -1;
        break;
      }
    }
  }

  fs_give ((void **) &buf);     /* flush temporary buffer */
  if (failed) {                 /* failed to negotiate SSL */
    ssl_close (stream);         /* punt stream */
    exit (1);                   /* punt this program too */
  }
  ret = (SSLSTDIOSTREAM *)      /* success, return SSL stdio stream */
    memset (fs_get (sizeof(SSLSTDIOSTREAM)),0,sizeof(SSLSTDIOSTREAM));
  ret->sslstream = stream;      /* stream to do SSL I/O */
  ret->octr = SSLBUFLEN;        /* available space in output buffer */
  ret->optr = ret->obuf;        /* current output buffer pointer */
  return ret;
}

/* Get character
 * Returns: character or EOF
 */

int ssl_getchar (void)
{
  if (!sslstdio) return getchar ();
  if (!ssl_getdata (sslstdio->sslstream)) return EOF;
                                /* one last byte available */
  sslstdio->sslstream->ictr--;
  return (int) *(sslstdio->sslstream->iptr)++;
}


/* Get string
 * Accepts: destination string pointer
 *          number of bytes available
 * Returns: destination string pointer or NIL if EOF
 */

char *ssl_gets (char *s,int n)
{
  int i,c;
  if (!sslstdio) return fgets (s,n,stdin);
  for (i = c = 0, n-- ; (c != '\n') && (i < n); sslstdio->sslstream->ictr--) {
    if ((sslstdio->sslstream->ictr <= 0) && !ssl_getdata (sslstdio->sslstream))
      return NIL;               /* read error */
    c = s[i++] = *(sslstdio->sslstream->iptr)++;
  }
  s[i] = '\0';                  /* tie off string */
  return s;
}

/* Put character
 * Accepts: character
 * Returns: character written or EOF
 */

int ssl_putchar (int c)
{
  if (!sslstdio) return putchar (c);
                                /* flush buffer if full */
  if (!sslstdio->octr && ssl_flush ()) return EOF;
  sslstdio->octr--;             /* count down one character */
  *sslstdio->optr++ = c;        /* write character */
  return c;                     /* return that character */
}


/* Put string
 * Accepts: destination string pointer
 * Returns: 0 or EOF if error
 */

int ssl_puts (char *s)
{
  if (!sslstdio) return fputs (s,stdout);
  while (*s) {                  /* flush buffer if full */
    if (!sslstdio->octr && ssl_flush ()) return EOF;
    *sslstdio->optr++ = *s++;   /* write one more character */
    sslstdio->octr--;           /* count down one character */
  }
  return 0;                     /* success */
}


/* Flush output
 * Returns: 0 or EOF if error
 */

int ssl_flush (void)
{
  if (!sslstdio) return fflush (stdout);
                                /* force out buffer */
  if (!ssl_sout (sslstdio->sslstream,sslstdio->obuf,
                 SSLBUFLEN - sslstdio->octr)) return EOF;
                                /* renew output buffer */
  sslstdio->optr = sslstdio->obuf;
  sslstdio->octr = SSLBUFLEN;
  return 0;                     /* success */
}
