/* ckmres.h - MAC C Kermit resource file equates */

#define TERMBOXID	1000	/* terminal window ID */
#define RCMDBOXID	1001	/* remote command window ID */

#define RCMDHSCROLL	1001
#define RCMDVSCROLL	1000

#define ALERT_HELP	1009	/* key macro help alert */
#define ALERT_ERROR	1010	/* error alert */
#define ALERT_DEBUG	1011	/* debug alert */
#define ALERT_ABORT	1012	/* abort protocol */
#define ALERT_MODHELP	1013	/* key modifier help alert */

#define KSET_TYPE	'KSET'	/* key set */
#define KSVER		4	/* current version number for bit table */
#define MSET_TYPE	'MSET'	/* meta prefix string */
#define KMVER 		5	/* current version number for macro table */
#define SAVI_TYPE	'SAVI'	/* rsrc integer types */
#define SIVER 		5	/* version & rsrc ID number for integers */
#define SAVC_TYPE 	'SAVC'	/* rsrc character types */
#define SCVER 		5	/* version & rsrc ID number for characters */

#define NODEAD_KCHR	13000	/* I.D. of our special KCHR resource */
#define NODEAD_SICN	13000	/* I.D. of our special SICN res. */

/* MENUS */


#define APPL_MENU	1	/* APPLE: */
#define  ABOUT_APL	1	/* about kermit */

#define MIN_MENU	2	/* first menu resource ID */
				/* (Apple menu doesn't count) */

#define FILE_MENU   2		/* FILE: */
#define FILE_MEN2   34		/* FILE: (w/o menu keys) */
#define  LOAD_FIL	 1	/* load settings */
#define  SAVE_FIL	 2	/* save settings */
 /* (- (leave a space) */
#define  GETS_FIL	 4	/* get a file from server */
#define  SEND_FIL	 5	/* send a file */
#define  RECV_FIL	 6	/* receive a file */
#define  STATS_FIL	 7	/* show transfer stats */
#define  CWD_FIL	 8	/* set transfer directory */
 /* (- (leave a space) */
#define  XFER_FIL  	10	/* transfer... (to another pgm) */
#define  QUIT_FIL	11	/* quit this program */

#define EDIT_MENU  3		/* EDIT: */
#define EDIT_MEN2  35		/* EDIT: (w/o menu keys) */
#define  UNDO_EDIT	 1	/* undo */
 /* (- (space) */
#define  CUT_EDIT	 3	/* cut */
#define  COPY_EDIT	 4	/* copy */
#define  PASTE_EDIT	 5	/* paste */
#define  CLEAR_EDIT 	 6	/* clear */
 /* (- (space) */
#define  BREAK_EDIT 	 8	/* send break */
#define  LBREAK_EDIT 	 9	/* send long break */
#define  XON_EDIT	10	/* send XON */
#define  DTR_EDIT	11	/* toggle DTR */

#define	SETG_MENU  4		/* SETTINGS: */
#define	SETG_MEN2  36		/* SETTINGS: */
#define	 FILE_SETG 	 1	/* file settings */
#define	 COMM_SETG 	 2	/* communications settings */
#define	 PROT_SETG 	 3	/* protocol settings */
#define	 TERM_SETG 	 4	/* terminal settings */
#define	 CHARS_SETG 	 5	/* character set */
 /* (- (leave a space) */
#define	 SCRD_SETG 	 7	/* screen dumping... */
#define	 MCDM_SETG	 8	/* menu command keys */
#define  KEYM_SETG	 9	/* keyboard macros */
#define  MODF_SETG	10	/* modifier settings */

#define	REMO_MENU  5		/* REMOTE: */
#define	REMO_MEN2  37		/* REMOTE: */
#define	 RESP_REMO 	 1	/* show/hide response window */
 /* (- (leave a space) */
#define	 FIN_REMO	 3	/* finish, exit remote server */
#define	 BYE_REMO	 4	/* bye (log out server) */
 /* (- (leave a space) */
#define	 CWD_REMO	 6	/* cwd - change working directory */
#define	 DEL_REMO	 7	/* delete remote file */
#define	 DIR_REMO	 8	/* remote directory */
#define	 HELP_REMO	 9	/* remote help */
#define	 HOST_REMO	10	/* arbitrary command */
#define	 SPAC_REMO	11	/* disk usage */
#define	 TYPE_REMO	12	/* type a file */
#define	 WHO_REMO	13	/* who is logged in */
 /* (- */
#define	SERV_REMO 15		/* be a server */

#define LOG_MENU   6		/* LOG: */
#define LOG_MEN2   38		/* LOG: */
 /* (- (leave a space) */
#define  SLOG_LOG	 1	/* log session */
#define  SDMP_LOG	 2	/* dump screen to session log */
#ifdef TLOG
#define  TLOG_LOG	 3	/* log transactions */
#endif TLOG

/* John A. Oberschelp for Emory University -- vt102 printer support 22 May 1989 */
/*                    Emory contact is Peter W. Day, ospwd@emoryu1.cc.emory.edu */ 

#define PRNT_MENU  7		/* PRINT: */		/*JAO*/
#define PRNT_MEN2  39		/* PRINT: */		/*JAO*/
#define  NOW_PRNT	 1  /* Send the print buffer now */
#define  STAT_PRNT	 2  /* Give print buffer status  */
#define  DISC_PRNT	 3  /* Discard the print buffer  */

#define MAX_MENU PRNT_MENU	/* last menu resource id */
#define MAX_MEN2 PRNT_MEN2

/* DLOG 257 - About Kermit */

#define ABOUTID 257		/* ID of dialog in resource file */
#define  AB_VERS	 2	/* The MacKermit version string */
#define  AB_PROV	 3	/* the protocol version string */
#define  AB_COPY	16	/* "Permission is granted..." */
#define  AB_BUGS	17	/* "Bugs to:" */

/* DLOG 230 - Status display (during transfer) */

#define SCRBOXID 230		/* resource ID for status display */

#define SRES_UNDEF	 1	/* do nothing */
#define  SRES_CANF	 1	/* cancel xmit of single file */
#define  SRES_CANG	 2	/* cancel xmit of entire group */
#define  SRES_DIR	 3	/* "receiving" or "sending" */
#define  SRES_FILN	 4	/* filename goes here */
#define  SRES_AS	 5	/* "as" or blank */
#define  SRES_AFILN	 6	/* as filename or blank */
#define  SRES_KXFER	 8	/* numeric for "Kbytes" */
#define  SRES_NPKT	10	/* numeric for "Packets" */
#define  SRES_NRTY	12	/* numeric for "Retries" */
#define  SRES_FFORK	13	/* "RSRC FORK" or "DATA FORK" */
#define  SRES_FMODE	14	/* "TEXT MODE" or "BINARY MODE" */
#define  SRES_BTEXT	15	/* arbitrary big text */
#define  SRES_PACSZ	17	/* numeric for "Rec'd pkt size" */
#define  SRES_CKSUM	19	/* numeric for "Checksum" */
#define  SRES_WINSZ	21	/* numeric for "Window size" */
#define  SRES_ITEXT	23	/* info text */
#define  SRES_PTEXT	24	/* previous file status text */

/* DLOG 1001 - Communications setup dialog */

#define COMMBOXID	1001	/* resource ID for comm setup */
#define CR_BAUD300	5
#define CR_BAUD600	6
#define CR_BAUD1200	7
#define CR_BAUD1800	8
#define CR_BAUD2400	9
#define CR_BAUD3600	10	/* RAM driver only */
#define CR_BAUD4800	11
#define CR_BAUD7200	12
#define CR_BAUD9600	13
#define CR_BAUD14400	14	/* RAM driver only */
#define CR_BAUD19200	15
#define CR_BAUD28800	16	/* RAM driver only */
#define CR_BAUD38400	17	/* RAM driver only */
#define CR_BAUD57600	18
		/* 19: "Parity/Bits" */
#define CR_PARMARK	20
#define CR_PARSPACE	21
#define CR_PAREVEN	22
#define CR_PARODD	23
#define CR_PARNONE	24

#define CR_XONXOFF	25	/* flow control check box */
		/* 26: "Serial Port" */
#define CR_PMODEM	27	/* modem port radio button */
#define CR_IMODEM	28	/* modem port icon */
#define CR_PPRINTER	29	/* printer port radio button */
#define CR_IPRINTER	30	/* printer port icon */

#define CR_DROPDTR	31	/* drop DTR on Quit */

/* DLOG 1002 - Protocol setup dialog */

#define PROTOBOXID 1002

#define PR_BLK1		 5	/* block check types */
#define PR_BLK2		 6
#define PR_BLK3		 7

#define PR_HSBELL	 9	/* hand shake character */
#define PR_HSCR		10
#define PR_HSESC	11
#define PR_HSLF		12
#define PR_HSNONE	13
#define PR_HSXON	14
#define PR_HSXOFF	15

#define PR_INSOP	19	/* start of packet edit text */
#define PR_INEOP	21	/* end of packet edit text */
#define PR_INPADC	23	/* pad character edit text */
#define PR_INPADN	25	/* pad count edit text */
#define PR_INTIMEO	27	/* seconds timeout edit text */
#define PR_INPKTLEN	29	/* packet length edit text */

#define PR_OUTSOP	30	/* start of packet edit text */
#define PR_OUTEOP	31	/* end of packet edit text */
#define PR_OUTPADC	32	/* pad character edit text */
#define PR_OUTPADN	33	/* pad count edit text */
#define PR_OUTTIMEO	34	/* seconds timeout edit text */
#define PR_OUTPKTLEN	35	/* packet length edit text */

/* DLOG 1003 - remote dialog */

#define REMOTEBOXID 1003
#define RRES_ARG1	5
#define RRES_ARG2	7


/* DLOG 1004 - file default dialog */

#define FILEBOXID 1004
#define FSET_ATTEND	4
#define FSET_UNATTEND	5
#define FSET_SUPERSEDE 	6
#define FSET_NEWNAMES 	7
#define FSET_KEEP	16
#define FSET_XMITTOO	17

/* DLOG 1005 - remote dialog */

#define TERMINALBOXID 1005
#define TR_AUTOWRAP 	4	/* auto wrap around at end of line */
#define TR_AUTOREPT 	5	/* auto repeat keys */
#define TR_INVERT	6	/* inverted terminal window */
#define TR_SMOOTH	7	/* smooth scrolling */
#define TR_AUTOLF	8	/* send LF after CR */
#define TR_LOCLECHO	9	/* local character echo */
#define TR_TRANSP	10	/* transparent mode */
#define TR_BLOCKC	11	/* block cursor */
#define TR_MOUSE	12	/* mouse -> arrow keys */
#define TR_VISBELL	13	/* do visible bell */
#define TR_EIGHTBIT 	14	/* accept eight bit input */
#define TR_BLINKC	15	/* blinking cursor */
#define TR_RESETBTN	16	/* button: reset the terminal */
#define TR_NUMLINES	17	/* edit text: number of lines on the screen */

/* DLOG 1006 - key dialog (main) */

#define KEYBOXID 	1006
#define KY_MODIFIER 	3		/* modifier text display */


/* DLOG 1007 - key dialog (subdialog) */

#define KEY2BOXID 1007
#define KY_TEXT		3	/* function text */
#define KY_HELP		4	/* function text */


/* DLOG 1008 - modifier dialog */

#define MODBOXID 1008
#define MOD_CHK1		3	/* 1st check box */
#define MOD_CHKL	 38	/* last check box */
#define MOD_PRF1	 39	/* 1st prefix string */
#define MOD_PRFL	 42	/* last prefix string */
#define MOD_LIN1	 43	/* 1st line */
#define MOD_LINL	 46	/* last line  */
#define MOD_HELP	 51	/* PWP: help button */

/* DLOG 1014 - character set dialog */

#define CHARBOXID	1014

/* the order of these must match that of ckmdef.h */
#define CS_USA		4
#define CS_UK		5
#define	CS_DUTCH	6
#define	CS_FINNISH	7
#define CS_FRENCH	8
#define CS_FRCAN	9
#define CS_GERMAN	10
#define CS_ITALIAN	11
#define CS_NORDAN	12
#define CS_PORTUGUESE	13
#define CS_SPANISH	14
#define CS_SWEDISH	15
#define CS_SWISS	16

#define CS_ISOLATIN1	17
#define CS_ISOLATIN2	18
#define CS_ISOLATIN3	19
#define CS_ISOLATIN4	20
#define CS_ISOCYRILLIC	21
#define CS_ISOARABIC	22
#define CS_ISOGREEK	23
#define CS_ISOHEBREW	24
#define CS_ISOLATIN5	25
#define CS_VTGRAPH	26
#define CS_VTTECH	27

/* DLOG 1015 - set transfer directory dialog */

#define CWDBOXID	1015

/* DLOG 1016 - show transfer stats */

#define STATBOXID	1016
#define  ST_TSEC	 8	/* total seconds */
#define  ST_TLCI	 9	/* serial characters in */
#define  ST_TLCO	10	/* serial characters out */
#define  ST_TFC		11	/* total file characters xfered */
#define  ST_EBAUD	12	/* effective baud rate (10*cps) */
#define  ST_EEFF	14	/* efficiency of line */

/* DLOG 2001 - print buffer dialog */			/*JAO*/

#define PRINTBOXID 2001
#define PRINT_ARG1	6
#define PRINT_ARG2	5
#define PRINT_ARG3	4

/* DLOG 2002 - buffering dialog */			/*JAO*/

#define BUFFERINGBOXID 2002

/* DLOG 2003 - overflowing dialog */			/*JAO*/

#define OVERFLOWINGBOXID 2003

/* DLOG 2004 - overflowed dialog */			/*JAO*/

#define OVERFLOWEDBOXID 2004

/* DLOG 2005 - command-. dialog */			/*JAO*/

#define PRINTINGBOXID 2005


/* DLOG 3998 - SFPutFile DLOG replacement */

#define DLG_GETF 3998
#define GETF_GETB ok		/* "Get" button */
#define GETF_REMF 4		/* remote file name */


/* DLOG 3999 - SFPutFile DLOG replacement */

#define DLG_RECV 3999
#define RECV_RBTN putSave	/* "Receive" button */
#define RECV_PROC 16		/* proceed without more dialogs */
#define RADITM_DATA 11
#define RADITM_RSRC 12
#define RADITM_TEXT 13
#define RADITM_BINA 14
#define RADITM_MACB 15		/* MacBinary */
#define RADITM_FIRST RADITM_DATA
#define RADITM_LAST RADITM_BINA	/* should be last of data,rsrc,text,binary */


/* DLOG 4000 - SFGetFile DLOG replacement */

#define DLG_SEND 4000
#define SEND_ASFN 18		/* "As" file name */
#define SEND_INVT 17		/* invisible edit text */
#define SEND_ALL	20	/* send all files check box */
