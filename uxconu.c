/*
 * connect command for "standard" unix
 *
 */

/*
 *  c o n n e c t
 *
 *  Establish a virtual terminal connection with the remote host, over an
 *  assigned tty line. 
 */

#include <stdio.h>
#include <ctype.h>
#include <sgtty.h>

#define TRUE 1
#define FALSE 0
#define ctl(x) ((x) ^ 64)
#define unpar(ch) ((ch) & 127)

extern int remote, ttyfd, lecho, speed;
extern char escchr;

connect()
{
    int pid,				/* Holds process id of child */
	tt2fd,				/* FD for the console tty */
	connected;			/* Boolean connect flag */
    char bel = '\07',
	c;

    struct sgttyb oldcon;		/* [???] should this be here? */

    if (remote)				/* Nothing to connect to in remote */
    {					/* mode, so just return */
	printmsg("No line specified for connection.");
	return;
    }

    speed = 0;				/* Don't set the console's speed */
    tt2fd = ttopen(0);			/* Open up the console */
    ttbin(tt2fd,&oldcon);		/* Put it in binary */

    pid = fork();           /* Start fork to get typeout from remote host */

    if (!pid)			/* child1: read remote output */
    {
     while (1)
     {
      char c;
      read(ttyfd,&c,1);			/* read a character */
      write(1,&c,1);			/* write to terminal */
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
      while (wait(0) != -1);		/* and bury him */
      ttres(tt2fd,&oldcon);
      printmsg("disconnected.");
      return;				/* Done */
}
