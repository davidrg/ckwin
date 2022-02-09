/*
 * connect command for Venix
 *
 */

/*
 *  c o n n e c t
 *
 *  Establish a virtual terminal connection with the remote host, over an
 *  assigned tty line. 
 */

#include <stdio.h>
#include <sgtty.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0
#define ctl(x) ((x) ^ 64)
#define unpar(ch) ((ch) & 127)

extern int remote, ttyfd, lecho;
extern char escchr;

connect()
{
    int pid,				/* Holds process id of child */
	pid2,				/* process id of second child */
	parray[2],
	tt2fd,				/* FD for the console tty */
	connected;			/* Boolean connect flag */
    char bel = '\07',
	c;

   struct sgttyb oldcon;

    if (remote)				/* Nothing to connect to in remote */
    {					/* mode, so just return */
	printmsg("No line specified for connection.");
	return;
    }

    tt2fd = ttopen(0);			/* open up the console */
    ttbin(tt2fd,&oldcon);		/* Put it in binary */

    pipe(parray);
    pid = fork();			/* Start fork to get typeout */

    if (!pid)				/* child1: read remote output */
    {
     nice(-110);			/* real-time priority */
     close(parray[0]);			/* don't need this side */
     while (1)
     {
      struct sgttyb tmp;
      char cbuf[300];
      int cnt;
      ioctl(ttyfd,TIOCQCNT,&tmp);
      cnt = tmp.sg_ispeed;			/* how many chars in buffer */
      if (cnt == 0) cnt=1;			/* if none, wait on read */
      read(ttyfd,cbuf,cnt);			/* read a character */
      write(parray[1],cbuf,cnt);		/* write to the pipe */
     }
    }
    pid2 = fork();			/* make a second child */
    if (!pid2)				/* child2: write remote to screen */
    {
     close(parray[1]);			/* don't need this side */
     while (1)
     {
      int cnt;
      char buf[100];
      if (cnt = read(parray[0],buf,100)) write(1,buf,cnt);
     }
    }
/* resume parent: read from terminal and send out port */
      printmsg("connected...\r");
      connected = TRUE;		/* Put us in "connect mode" */
      while (connected)
      {
	  read(tt2fd,&c,1);		/* Get a character */
	  c = unpar(c);		/* Turn off the parity */
	  if (c == escchr)		/* Check for escape character */
	  {
	      read(tt2fd,&c,1);
	      c = unpar(c);		/* Turn off the parity */
	      if (c == escchr)
	      {
		  c = dopar(c);	/* Do parity if the user requested */
		  write(ttyfd,&c,1);
	      }
	      else
	      switch (toupper(c))
	      {
		  case 'C':
		      connected = FALSE;
		      write(tt2fd,"\r\n",2);
		      break;

		  case 'H':
			{
			  char hlpbuf[100],e;
			  sprintf(hlpbuf,"\r\n C to close the connection\r\n");
			  write(tt2fd,hlpbuf,strlen(hlpbuf));
			  e = escchr;
			  if (e < ' ') {
				write(tt2fd,"^",1);
				e = ctl(e); }
			  sprintf(hlpbuf,"%c to send itself\r\n",e);
		      	  write(tt2fd,hlpbuf,strlen(hlpbuf));
			}
		      break;

		  default:
		      write(tt2fd,&bel,1);
		      break;
	      }
	  }
	  else
	  {				/* If not escape charater, */
	      if (lecho) write(1,&c,1); /* Echo char if requested */
	      c = dopar(c);		/* Do parity if the user requested */
	      write(ttyfd,&c,1);	/* write it out */
	      c = NULL;		/* Nullify it (why?) */
	  }
      }
      kill(pid,9);			/* Done, kill the child */
      kill(pid2,9);
      while (wait(0) != -1);		/* and bury him */
      ttres(tt2fd,&oldcon);
      printmsg("disconnected.");
      return;				/* Done */
}
