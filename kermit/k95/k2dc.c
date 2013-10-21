/***************************/
/* OS/2 API support        */
/***************************/
#define OS2
#define INCL_DOS
#define INCL_VIO
#define INCL_BASE
#include <os2.h>
#undef COMMENT

/***************************/
/* Microsoft C runtime     */
/***************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <signal.h>
#include <errno.h>
#include <io.h>
#include <direct.h>
#include <memory.h>
#include <sys/types.h>

#ifdef __IBMC__
#define off_t _dummy_off_t  /* avoid warning */
#include <types.h>
#undef off_t
#else
#include <types.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

void _System addsockettolist(int socket);

void
cleanupsocket( int socket )
{
    char * msg = "Unable to start Kermit-95 for OS/2\r\nClosing socket.\r\n\n";

    sock_init();
    addsockettolist( socket );
    send(socket, msg, strlen(msg), 0);
    soclose( socket );
}

int 
main(int argc, char *argv[], char * envp[])
{
    int socket=0;
    int rc=0;
    int i;
    char * script=NULL;
    char * p=NULL;
    char cmdline[1024]="";
    char loadpath[257]="";
    RESULTCODES resultcodes;
    STARTDATA   sdata;
    ULONG       sessionid;
    PID 	pid;
    char        achObjBuf[256] = {0};     /* Error data if DosStart fails */

#ifdef DEBUG
    for ( i=0;i<argc;i++ )
	printf("arg%d=%s\n",i,argv[i]);
#endif

    if (argc < 3) {
        printf("Usage: %s <kermit script> [<kermit command line parameters>] <socket>\n",
		argv[0]);
	if ( argc == 2 ) {
	    socket = atoi(argv[1]);        /* required last parameter = socket  */
	    cleanupsocket(socket);
	}
        DosExit(1,1);
    } /* endif */

    script = argv[1];          		/* required paramenter #1 = script */
    socket = atoi(argv[argc-1]);        /* required last parameter = socket  */

    sprintf(cmdline, "-j !%d -C \"take scripts/%s, exit\"", 
	     socket, script);

    for (i=2;i<argc-1;i++) {     	/* append additional parameters */
	strcat(cmdline," ");
	strcat(cmdline,argv[i]);
    }

    strcpy(loadpath, argv[0]);
    p = loadpath + strlen(loadpath);
    while ( *p != '\\' && *p != '/' )
	p--;
    *p = '\0';
    printf("changing directory to %s\n",loadpath);
    chdir(loadpath);

    strcat(loadpath,"\\k2.exe");

    printf("%s %s\n",loadpath,cmdline);

    sdata.Length  = sizeof(STARTDATA);
    sdata.Related = SSF_RELATED_INDEPENDENT; /* start a Child for PID */
                                             /* not SSF_RELATED_CHILD */
    sdata.FgBg    = SSF_FGBG_BACK;           /* start session in foreground  */
    sdata.TraceOpt = SSF_TRACEOPT_NONE;      /* No trace                     */
                                             /* Start an OS/2 session using "CMD.EXE /K" */
    sdata.PgmTitle = NULL;
    sdata.PgmName = loadpath;
    sdata.PgmInputs = cmdline;                      /* Keep session up           */

    sdata.TermQ = 0;                            /* No termination queue      */
    sdata.Environment = 0;                      /* No environment string     */
    sdata.InheritOpt = SSF_INHERTOPT_PARENT;    /* Inherit parent's environ.  */
    sdata.SessionType = SSF_TYPE_WINDOWABLEVIO; /* Windowed VIO session      */
    sdata.IconFile = 0;                         /* No icon association       */
    sdata.PgmHandle = 0;
    /* Open the session VISIBLE and MAXIMIZED */
    sdata.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_MINIMIZE;
    sdata.InitXPos  = 30;     /* Initial window coordinates              */
    sdata.InitYPos  = 40;
    sdata.InitXSize = 200;    /* Initial window size */
    sdata.InitYSize = 140;
    sdata.Reserved = 0;
    sdata.ObjectBuffer  = achObjBuf; /* Contains info if DosExecPgm fails */
    sdata.ObjectBuffLen = (ULONG) sizeof(achObjBuf);

    rc = DosStartSession(&sdata,&sessionid,&pid);
#ifdef DEBUG
    printf("rc = %d :: %s\n",rc,achObjBuf);
#endif
    if ( rc && rc != 457 ) {
	printf("ERROR: Unable to start K2.EXE -- closing socket %d\n\n",socket);
	cleanupsocket(socket);
    }
    return(rc);
}
