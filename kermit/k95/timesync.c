/* timesync stuff for leash - 7/28/94 - evanr */

#include <winsock.h>
#include <setjmp.h>
#include <time.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>

#ifdef WSHELPER
    #include <wshelper.h>
#endif
#ifdef WIN32
#define _fmemcpy memcpy
#endif

extern HANDLE hThisInstance;
#define TM_OFFSET 2208988800

struct timezone {               /* timezone.h has a winsock.h conflict */
  int     tz_minuteswest;
  int     tz_dsttime;
};

#ifdef WIN32
int settimeofday (struct timeval *tv,struct timezone *tz) {
    SYSTEMTIME systime;
    struct tm *newtime;
    int rc = 0;


    newtime = gmtime((time_t *)&(tv->tv_sec));
    systime.wYear = 1900+newtime->tm_year;
    systime.wMonth = 1+newtime->tm_mon;
    systime.wDay = newtime->tm_mday;
    systime.wHour = newtime->tm_hour;
    systime.wMinute = newtime->tm_min;
    systime.wSecond = newtime->tm_sec;
    systime.wMilliseconds = 0;
    rc = SetSystemTime(&systime);
    if ( !rc )
        return(rc);
    else 
        return(GetLastError());
}  
#else
int settimeofday (struct timeval *tv,struct timezone *tz) {
  struct _dostime_t dostime;      
  struct _dosdate_t dosdate;
  struct tm *newtime;
  
  newtime = localtime((time_t *)&(tv->tv_sec));
  dostime.hour = newtime->tm_hour;
  dostime.minute = newtime->tm_min;   
  dostime.second = newtime->tm_sec;
  dostime.hsecond = 0;
  dosdate.day = newtime->tm_mday;
  dosdate.month = 1+newtime->tm_mon; /* burp! Microsft really sucks */
  dosdate.year = 1900+newtime->tm_year; /* burp! Microsoft sucks again... */
  dosdate.dayofweek = newtime->tm_wday;
  
  if (_dos_setdate(&dosdate) == 0 && _dos_settime(&dostime) == 0)
    return 0;
  return 1;    
}       
#endif

int gettimeofday (struct timeval *tv,struct timezone *tz) {
  time_t long_time;

#ifdef COMMENT
  if (getenv ("TZ") == NULL)  /* Default New York, New York, USA time */
    {
      _timezone = 18000;
      _daylight = 1;
      _tzname[0] = "EST";
      _tzname[1] = "EDT";
    }
  else
#endif
    _tzset();
  tz->tz_minuteswest = _timezone;
  tz->tz_dsttime = _daylight;
  time(&long_time);
  tv->tv_sec = long_time;
  tv->tv_usec = 0;
  return 0;       
}       

int
main(int argc, char * argv[])
{
  char buffer[512];
  struct timeval tv;
  struct timezone tz;
  jmp_buf top_level;
  struct sockaddr_in sin;
  struct servent FAR * sp;
  struct hostent FAR *host;
  register int s;
  long hosttime;
  register long *nettime; /* used to be an int */
  char tmpstr[80], hostname[64], *phostname=NULL;
  int attempts = 0, cc, host_retry;
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
  int check;
  wVersionRequested = 0x0101;
  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) {
    sprintf(tmpstr, "Couldn't initialize WinSock to synchronize time\n\r"
	     "Error Number: %d", err);
    printf(tmpstr);
    WSACleanup( );
    return(1);
  }
  /* OK, now try the resource -- to be added */
  sp = getservbyname("time", "udp");
  if (sp == 0)
    sin.sin_port = htons(IPPORT_TIMESERVER);
  else
    sin.sin_port = sp->s_port;
  host_retry = 0;

  if( argc > 1 ){
      strncpy (hostname, argv[1], 64 );
      hostname[63] = '\0';
      phostname = hostname;
  } else {
      hostname[0] = '\0';
      phostname = "time";
  }
  
  get_host:     
  while (host_retry < 5) {
      host = gethostbyname(phostname); 
      if ((host != NULL) && WSAGetLastError() != WSATRY_AGAIN) {
          break;
      }
      if ( phostname == hostname ) {
          printf("trouble resolving \"%s\", trying \"time\" ...\n",
                  hostname);
          phostname = "time";
      } else if ( !strcmp(phostname,"time") ) {
          printf("trouble resolving \"time\", trying \"timehost\" ...\n");
          phostname = "timehost";
      } else if ( !strcmp(phostname,"timehost") ) {
          if ( hostname[0] ) {
              printf("trouble resolving \"timehost\", retrying \"%s\" ...\n",
                      hostname);
              phostname = hostname;
          } else {
            printf("trouble resolving \"timehost\", retrying \"time\" ...\n");
            phostname = "time";
          }
          host_retry++;
      }
  }
  if (host == NULL) {
    sprintf (tmpstr, "The timeserver host cannot be found\n");
      printf(tmpstr);
      WSACleanup();
      return(2);
  }
  sin.sin_family = host->h_addrtype;
  _fmemcpy ((struct sockaddr *)&sin.sin_addr, host->h_addr, 
	    host->h_length);
  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) {                                          
    sprintf(tmpstr, "Couldn't open socket - Error: %d", WSAGetLastError());
      printf(tmpstr);
    WSACleanup();
    return(3);
  }
  
  if (connect (s, (struct sockaddr *)&sin, sizeof (sin)) < 0) {
      sprintf(tmpstr, "Couldn't connect to timeserver - Error: %d", 
               WSAGetLastError());
      printf(tmpstr);
      WSACleanup();
      return(4);
  }
    setjmp(top_level);
    if (attempts++ > 5) {
        closesocket (s);
        sprintf (tmpstr, "Failed to get time from %s - Error: %d",
                  phostname, WSAGetLastError());
        printf(tmpstr);
        WSACleanup();
        return(5);
    }        
    send (s, buffer, 40, 0); /* Send an empty packet */
    if (gettimeofday (&tv, &tz) < 0) {
        sprintf(tmpstr,"Unable to get local time\n");
        printf(tmpstr);
        WSACleanup();
        return(6);
    }
  
    cc = recv (s, buffer, 512, 0);
    if (cc < 0) {
        sprintf(tmpstr, "Error receiving time from %s - Error: %d", 
                 phostname,
                 WSAGetLastError());
        printf(tmpstr);
        closesocket(s);
        WSACleanup();
        return(7);
    }

    if (cc != 4) {
        closesocket(s);
        sprintf(tmpstr, "Protocol error -- received %d bytes; expected 4",
                 cc);
        printf(tmpstr);
        WSACleanup();
        return(8);
    }
    nettime = (long *)buffer;
    hosttime = (long) ntohl (*nettime) - TM_OFFSET;
    (&tv)->tv_sec = hosttime;
    if (settimeofday (&tv, &tz) < 0) {
        printf("Couldn't set local time of day.");
        WSACleanup();
        return(9);
  }
  
  strcpy(tmpstr, ctime((time_t *)&hosttime));
  tmpstr[strlen(tmpstr)-1] = '\0';
  printf("Local time set to %s",tmpstr);
  closesocket(s);
  WSACleanup();
  return(0);
}
