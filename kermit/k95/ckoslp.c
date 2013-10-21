/* C K O S L P -- Kermit interface to the IBM SLIP driver */

/*
  Authors: Jeffrey Altman (jaltman@secure-endpoints.com),
             Secure Endpoints Inc., New York City.
           David Bolen (db3l@ans.net)

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include "ckcdeb.h"
#ifndef NT
#define INCL_ERRORS
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define  INCL_DOSNMPIPES
#include <os2.h>
#undef COMMENT

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "ckoslp.h"

/*--------------------------------------------------------------------------*/
/* Definitions used for interface information                               */
/*--------------------------------------------------------------------------*/

/* VJ Compression Options */
#define slc_NOCOMPRESSION       0       /* No compression at all            */
#define slc_COMPRESSION         1       /* Always send compression          */
#define slc_AUTOCOMPRESSION     2       /* Enable compression when received */


/*--------------------------------------------------------------------------*/
/* Interface structure (used to return interface configuration information) */
/*--------------------------------------------------------------------------*/
typedef struct slcS_INTERFACE_ {
                                        /* -- INET Attachment Information -- */

   char           if_name[4];           /* sl?   (?=0-9)                     */
   unsigned short if_mtu;               /* MTU for interface                 */

   unsigned short if_rtt;               /* Estimated rtt, rttvar and rttmin  */
   unsigned short if_rttvar;            /*   for routes making use of this   */
   unsigned short if_rttmin;            /*   interface (in ms)               */

   unsigned short if_sendpipe;          /* Maximum send/recv pipes (socket   */
   unsigned short if_recvpipe;          /*   buffers and TCP windows)        */
   unsigned short if_ssthresh;          /* Slow-start threshold (segments)   */

   unsigned short if_maxqueue;          /* Maximum interface queue size      */
   unsigned short if_maxfastq;          /* Maximum fast queue size           */

                                        /* -- Other Interface Settings  --   */

   int allowfastq,                      /* Should fast queueing be used      */
       compression;                     /* VJ compression options            */
   char *device;                        /* OS/2 device for interface         */

                                        /* -- Interface Command Scripts --   */
   char *attachcmd,
        *attachparms;                   /* and their parameters              */


   struct slcS_INTERFACE_ *next;        /* Pointer to next parsed interface  */

} slcS_INTERFACE, *slcPS_INTERFACE;


/*--------------------------------------------------------------------------*/
/* Function prototypes.                                                     */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------*/
/*                Parsing Functions                 */
/*--------------------------------------------------*/

int slcParseConfiguration (char *Filename, slcPS_INTERFACE *Interfaces);
int slcFreeConfiguration  (slcPS_INTERFACE *Interfaces);

/*
 *  Variables
 */

/* Semaphores and Pipe for communicating with SLIP.EXE */
HEV hevSlipMonitor = 0,
    hevSlipPause = 0,
    hevSlipPaused = 0,
    hevSlipContinue = 0;
HPIPE hpipeSlip = 0 ;

PSZ
SlipCfgFile( void )
{
    static PSZ slipcfgfile = 0 ;       /* Config file name */
    char * etc ;

    if ( slipcfgfile )
        return slipcfgfile ;

    if ( ( etc = getenv( "ETC" ) ) == NULL ) {
        slipcfgfile = "" ;
        return slipcfgfile ;
        }

    slipcfgfile = malloc( strlen( etc ) + 10 ) ;
    strcpy( slipcfgfile, etc ) ;
    strcat( slipcfgfile, "\\slip.cfg" ) ;
    return slipcfgfile ;
}

PSZ
PPPCfgFile( void )
{
    static PSZ pppcfgfile = 0 ;       /* Config file name */
    char * etc ;

    if ( pppcfgfile )
        return pppcfgfile ;

    if ( ( etc = getenv( "ETC" ) ) == NULL ) {
        pppcfgfile = "" ;
        return pppcfgfile ;
        }

    pppcfgfile = malloc( strlen( etc ) + 10 ) ;
    strcpy( pppcfgfile, etc ) ;
    strcat( pppcfgfile, "\\ppp.cfg" ) ;
    return pppcfgfile ;
}

APIRET
SlipOpen( char * device )
{
    APIRET rc ;
    ULONG action, actual ;
    char  interface[4] ;
    slcPS_INTERFACE pInterfaces = 0,
        pInterface              = 0 ; /* Ptr to S_INTERFACE structure */

    if ( rc = slcParseConfiguration ( SlipCfgFile(), &pInterfaces ) ) {
        printf("Unable to parse %s\n",SlipCfgFile() );
        return rc ;
        }

    /* Find the proper interface to use */
    interface[0] = '\0' ;
    pInterface = pInterfaces ;
    do
        {
        if ( !strcmp( strlwr(pInterface->device), device ) ) {
            strcpy( interface, pInterface->if_name ) ;
            break;
            }
        pInterface = pInterface->next ;
        }
    while ( pInterface != NULL );

    if ( interface[0] == '\0' ) {
        printf("%s is not listed in %s\n",device,SlipCfgFile());
        slcFreeConfiguration( &pInterfaces ) ;
        return 1 ;
        }

    if (!(rc = DosOpenEventSem( "\\sem32\\slip\\monitor",
        &hevSlipMonitor )))
        if(!(rc = DosOpenEventSem( "\\sem32\\slip\\com\\pause",
            &hevSlipPause )))
            if(!(rc = DosOpenEventSem( "\\sem32\\slip\\com\\paused",
                &hevSlipPaused)))
                if (!(rc = DosOpenEventSem( "\\sem32\\slip\\com\\continue",
                    &hevSlipContinue)))
                    rc = DosOpen( "\\pipe\\slip", &hpipeSlip, &action, 0,
                        FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READWRITE |
                        OPEN_SHARE_DENYNONE, NULL ) ;

    if ( !rc ) {
        int PauseCount = 0 ;
        DosWrite( hpipeSlip, interface, 4, &actual ) ;
        do {
            if (PauseCount)
                msleep(500);
            PauseCount++ ;
            rc = DosPostEventSem( hevSlipPause ) ;
        } while ( PauseCount <= 10 && rc == ERROR_ALREADY_POSTED ) ;

        if (!rc)
            rc =DosWaitEventSem( hevSlipPaused, SEM_INDEFINITE_WAIT ) ;
        }

    slcFreeConfiguration( &pInterfaces ) ;
    return rc ;
}

APIRET
PPPOpen( char * device )
{
    extern int ttppp;
    APIRET rc ;
    ULONG action, actual ;
    UCHAR MonitorSem[32] ;
    UCHAR PauseSem[32];
    UCHAR PausedSem[32];
    UCHAR ContinueSem[32];
    char  interface[8] ;
    slcPS_INTERFACE pInterfaces = 0,
        pInterface              = 0 ; /* Ptr to S_INTERFACE structure */

    sprintf(interface, "ppp%d", ttppp-1 );

    /* Create the Sem strings to use */
    sprintf(MonitorSem, "\\sem32\\ppp%d\\monitor", ttppp-1);
    sprintf(PauseSem, "\\sem32\\ppp%d\\pause", ttppp-1);
    sprintf(PausedSem, "\\sem32\\ppp%d\\paused", ttppp-1);
    sprintf(ContinueSem, "\\sem32\\ppp%d\\continue", ttppp-1);

    /* Find the proper interface to use */
    if (!(rc = DosOpenEventSem( MonitorSem, &hevSlipMonitor )))
        if(!(rc = DosOpenEventSem( PauseSem, &hevSlipPause )))
            if(!(rc = DosOpenEventSem( PausedSem, &hevSlipPaused)))
                if (!(rc = DosOpenEventSem( ContinueSem, &hevSlipContinue)))
                    rc = DosOpen( "\\pipe\\ppp", &hpipeSlip, &action, 0,
                        FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READWRITE |
                        OPEN_SHARE_DENYNONE, NULL ) ;

    if ( !rc ) {
        int PauseCount = 0 ;
        DosWrite( hpipeSlip, interface, 4, &actual ) ;
        do {
            if (PauseCount)
                msleep(500);
            PauseCount++ ;
            rc = DosPostEventSem( hevSlipPause ) ;
        } while ( PauseCount <= 10 && rc == ERROR_ALREADY_POSTED ) ;

        if (!rc)
            rc =DosWaitEventSem( hevSlipPaused, SEM_INDEFINITE_WAIT ) ;
        }
    return rc ;
}

void
PPPSlipClose( void )
{
   if ( hevSlipMonitor )
      {
        DosPostEventSem( hevSlipContinue ) ;   /* give the port back */
                                               /* to the slip driver */
        DosCloseEventSem( hevSlipContinue ) ;
        DosCloseEventSem( hevSlipPaused ) ;
        DosCloseEventSem( hevSlipPause ) ;
        DosCloseEventSem( hevSlipMonitor ) ;
        hevSlipMonitor = 0 ;
        hevSlipPaused = 0 ;
        hevSlipPause = 0 ;
        hevSlipContinue = 0 ;
      }

    if ( hpipeSlip ) {
        DosClose( hpipeSlip ) ;
        hpipeSlip = 0 ;
        }

}


/*==========================================================================*/
/*                                                                          */
/*            --------------------------------------------------            */
/*                                                                          */
/*                          PRIVATE DATA/FUNCTIONS                          */
/*                                                                          */
/*            --------------------------------------------------            */
/*                                                                          */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/* Macro/Constant definitions                                               */
/*--------------------------------------------------------------------------*/
#define _slcC_BUFSIZE     512   /* Maximum "chunks" used for config file    */
#define _slcC_ERRSIZE     256   /* Maximum size for a single error message  */
#define _slcC_TOKSIZE      64   /* Maximum size for "tokens" in config file */

#define _slcC_TOKEN_NONE    0   /* Token types */
#define _slcC_TOKEN_TOOLONG 1
#define _slcC_TOKEN_ERROR   1   /* < this indicates an error token */

#define _slcC_TOKEN_EOF     10  /* Success tokens */
#define _slcC_TOKEN_TEXT    11
#define _slcC_TOKEN_EQUAL   12
#define _slcC_TOKEN_LBRACE  13
#define _slcC_TOKEN_RBRACE  14

#define _slcC_TOKENTBL_TERM 1000  /* Must be > max entries in any table */

/*--------------------------------------------------------------------------*/
/* Structure declarations                                                   */
/*--------------------------------------------------------------------------*/
typedef struct _slcS_TOKEN_ {
   int type, length;              /* Type of token and text length         */
   char text[_slcC_TOKSIZE+1];    /* Remember room for terminating NULL    */
} _slcS_TOKEN, *_slcPS_TOKEN;

typedef struct _slcS_TOKENTBL_ {  /* Map of text tokens to cmd/parm values */
   int  command;                  /* Really an _slcE* type                 */
   char *name;                    /* Actual text of the cmd/element/value  */
   int  valuetype,                /* For parms, >0 means need value.  1 is */
        minimum, maximum;         /*   string, 2=number what range is ok   */
} _slcS_TOKENTBL, *_slcPS_TOKENTBL;


/*--------------------------------------------------------------------------*/
/* Declarations for commands/elements and values allowed within the         */
/* configuration file.                                                      */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------*/
/*          Configuration file commands             */
/*--------------------------------------------------*/
#define _slcA_CMD_INTERFACE     "interface"

typedef enum {cmd_interface} _slcE_CMD;

/*--------------------------------------------------*/
/*          "Interface" command parameters          */
/*--------------------------------------------------*/
#define _slcA_INTF_DEVICE         "device"
#define _slcA_INTF_MTU            "mtu"
#define _slcA_INTF_RTT            "rtt"
#define _slcA_INTF_RTTVAR         "rttvar"
#define _slcA_INTF_RTTMIN         "rttmin"
#define _slcA_INTF_SENDPIPE       "sendpipe"
#define _slcA_INTF_RECVPIPE       "recvpipe"
#define _slcA_INTF_SSTHRESH       "ssthresh"
#define _slcA_INTF_QUEUESIZE      "queuesize"
#define _slcA_INTF_FASTQUEUESIZE  "fastqueuesize"
#define _slcA_INTF_FASTQUEUE      "fastqueue"
#define _slcA_INTF_NOFASTQUEUE    "nofastqueue"
#define _slcA_INTF_COMPRESSION    "compression"
#define _slcA_INTF_ATTACHCMD      "attachcmd"
#define _slcA_INTF_ATTACHPARMS    "attachparms"

typedef enum {intf_device, intf_mtu, intf_rtt, intf_rttvar, intf_rttmin,
              intf_sendpipe, intf_recvpipe, intf_ssthresh,
              intf_queuesize, intf_fastqueuesize, intf_fastqueue,
              intf_nofastqueue, intf_compression, intf_attachcmd,
              intf_attachparms} _slcE_INTF;


/*--------------------------------------------------------------------------*/
/* Module-global data                                                       */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------*/
/*                Token error table                 */
/*--------------------------------------------------*/
char *_slc_TokenErrorTable[] = {   /* WARNING: Keep in same order as codes */
   "No token", "Token too long" };


/*--------------------------------------------------*/
/*      Token tables for commands/parameters        */
/*--------------------------------------------------*/
_slcS_TOKENTBL _slc_CmdTable[] = {
   {cmd_interface,       _slcA_CMD_INTERFACE,0},
   {_slcC_TOKENTBL_TERM, "",0} };

_slcS_TOKENTBL _slc_IntfTable[] = {
   {intf_device,        _slcA_INTF_DEVICE,1},
   {intf_mtu,           _slcA_INTF_MTU,2,64,2048},
   {intf_rtt,           _slcA_INTF_RTT,2,0,65536},
   {intf_rttvar,        _slcA_INTF_RTTVAR,2,0,65536},
   {intf_rttmin,        _slcA_INTF_RTTMIN,3,0,65536},
   {intf_sendpipe,      _slcA_INTF_SENDPIPE,2,0,65536},
   {intf_recvpipe,      _slcA_INTF_RECVPIPE,2,0,65536},
   {intf_ssthresh,      _slcA_INTF_SSTHRESH,2,0,65536},
   {intf_queuesize,     _slcA_INTF_QUEUESIZE,2,4,52},
   {intf_fastqueuesize, _slcA_INTF_FASTQUEUESIZE,2,4,52},
   {intf_fastqueue,     _slcA_INTF_FASTQUEUE,0},
   {intf_nofastqueue,   _slcA_INTF_NOFASTQUEUE,0},
   {intf_compression,   _slcA_INTF_COMPRESSION,1},
   {intf_attachcmd,     _slcA_INTF_ATTACHCMD,1},
   {intf_attachparms,   _slcA_INTF_ATTACHPARMS,1},
   {_slcC_TOKENTBL_TERM,""} };


/*--------------------------------------------------------------------------*/
/*     int _slcGetCh (FILE *RuleFile; int SkipWS, MergeWS, *Line, *Pos)     */
/*..........................................................................*/
/*                                                                          */
/* Retrieves the next "character" from the rule file, fetching new lines    */
/* from the file as necessary.  If MergeWS is 1, then any whitespace (tabs, */
/* spaces or newlines) is always returned as a space.  If SkipWS is 1, then */
/* any grouping of whitespace is returned as a single space character (or   */
/* whatever WS character is first in the group if MergeWS is 0).            */
/*                                                                          */
/* Comments are set via the # character and can appear anywhere within a    */
/* line.  Any text following the # will be ignored.                         */
/*                                                                          */
/* Parsing state information is returned in one or both of the Line and Pos */
/* parameters (if they are non-NULL).  Line represents the line number in   */
/* the file and Pos the position within the line (both origin 1) of the     */
/* character being returned by this function.                               */
/*                                                                          */
/* The function returns EOF when end of file is reached.                    */
/*                                                                          */
/* To avoid having to export parsing state information from this function,  */
/* a special calling convention of RuleFile==NULL is used in order to       */
/* initializing the internal buffer and counters for a new file.            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int _slcGetCh (FILE *RuleFile, int SkipWS, int MergeWS,
               int *Line, int *Pos)
{
   static char buffer[_slcC_BUFSIZE], *curch;
   static int full_line,        /* Does buffer represent a full line    */
              in_comment,       /* Current buffer holds partial comment */
              curline,          /* Current line/position information    */
              curpos;

   int done, result;
   char *ch;

   /* Special argument of RuleFile==NULL means initialize */
   if (RuleFile == NULL) {
      curline   = curpos = in_comment = 0;
      buffer[0] = '\0';
      curch     = buffer;
      full_line = 0;
      return(0);
   }

   /* Run until we have a suitable character to return */
   done   = 0;
   result = EOF;
   while (!done) {
      /*--------------------------------------------------*/
      /* 1. If we need a new line from the file go get it */
      /*--------------------------------------------------*/
      if (*curch == '\0') {
         buffer[_slcC_BUFSIZE-2] = '\0';        /* For quick full_line check */
         /* For now a read error and EOF are both returned as EOF, */
         /* so I just check for NULL and don't bother with feof()  */
         if (fgets(buffer,_slcC_BUFSIZE,RuleFile) == NULL) {
            result = EOF;
            break;      /* EOF - get out of here */
         } else {
            /* Reset current character pointer */
            curch = buffer;
            /* If last line was full, bump linecount, and init pos count */
            if (full_line) {
               curline++;
               curpos = 0;
            }
            /* Then decide if we got a whole line this time.  By checking */
            /* the next to last character (initialized above), we avoid   */
            /* the need to call strlen for each line read.                */
            full_line = (buffer[_slcC_BUFSIZE-2] == '\0' ||
                         buffer[_slcC_BUFSIZE-2] == '\n');
            /* Early-out.  If already in comment (true if a line with # is */
            /* >_slcC_BUFSIZE characters), just ignore this line entirely, */
            /* resetting in_comment only if we got all of rest of line.    */
            if (in_comment) {
               if (full_line) in_comment = 0;
               /* Nullify line - don't worry about WS - the leading # on */
               /* this comment would already have been treated as a ' '. */
               buffer[0] = '\0';
            } else {
               /* Otherwise, check for \n or #.  If found, convert to space */
               /* and terminate the string at that point.  We need a space  */
               /* to insure parsing "aa\n\nbb" separates "aa" and "bb".     */
               ch = buffer;
               while (*ch) {
                  if (*ch == '#' || *ch == '\n') {
                     /* Might need to set flag for long comments */
                     if (*ch == '#' && !full_line) in_comment = 1;
                     *ch++ = ' ';
                     *ch = '\0';
                     break;
                  } else {
                     ch++;
                  }
               }
            } /* \n and # scan */
         } /* if got good line */

         continue;      /* back and keep trying */
      }

      /*--------------------------------------------------*/
      /* 2. Now we have a null terminated buffer.  If on  */
      /*    WS find first non-WS, otherwise bump curch    */
      /*    by 1.  Then if SkipWS always return non-WS,   */
      /*    otherwise return space if we started on a WS. */
      /*    Note that we only set result if it isn't set  */
      /*    already since this may run several times in   */
      /*    order to skip over lines of whitespace.       */
      /*--------------------------------------------------*/
      if (result == EOF) {
         result = *curch;
         if (MergeWS && isspace(result)) result = ' ';
         /* If caller wants, store line/position information for this char */
         /* (remembering that we are origin 0, but returning origin 1)     */
         if (Line != NULL) *Line = curline+1;
         if (Pos  != NULL) *Pos  = curpos+1;
      }
      if (isspace(result) && SkipWS) {
         /* Find first non-WS */
         while (*curch && isspace(*curch)) {
            /* Account for tabs here - assume standard expansion of 8 */
            if (*curch == '\t') {
               curpos = ((curpos / 8)+1) * 8;
            } else {
               curpos++;
            }
            curch++;
         }
         done = (*curch);       /* We're only done if we didn't run out */
      } else {
         curch++;
         curpos++;
         done = 1;
      }
   } /* while !done */

   return(result);
}


/*--------------------------------------------------------------------------*/
/*  int _slcGetToken (FILE *RuleFile; _slcPS_TOKEN Token; int *Line, *Pos)  */
/*..........................................................................*/
/*                                                                          */
/* Grabs the next "token" from the specified file, identifies it, and       */
/* returns it in the supplied token structure.  The maximum number of       */
/* characters in a token cannot exceed _slcC_TOKSIZE.                       */
/*                                                                          */
/* The rules for breaking apart tokens is pretty simple.  The following     */
/* characters are treated specially:                                        */
/*      WS | ,  Separates tokens                                            */
/*      =       Special token - separates a rule element from its value.    */
/*      {       Special token - begins a list of rule elements.             */
/*      }       Special token - terminates a list of rule elements.         */
/* The text that created the special tokens is still stored in Token.text.  */
/*                                                                          */
/* Any of these characters may be quoted with the use of the \ character    */
/* (\\ for \ itself), or may be enclosed within double quotes (") to quote  */
/* an entire string.  Note that the \ character is processed even inside    */
/* quotes, so it must be doubled to yield itself or can also be used to     */
/* quote a quote, so to speak.                                              */
/*                                                                          */
/* If either of the Line or Pos parameters is non-NULL, then the file line  */
/* and character position of the start of the returned token are stored in  */
/* the address(es) they point to.                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#define _slc_ADDCHAR(token,ch)  if (chcount<_slcC_TOKSIZE) { \
                                   token->text[chcount++] = ch; \
                                   line = pos = NULL; \
                                } else { \
                                   token->type = _slcC_TOKEN_TOOLONG; \
                                   break; \
                                }

int _slcGetToken (FILE *RuleFile, _slcPS_TOKEN Token, int *Line, int *Pos)
{
   static int lastch=EOF;
   char *curch;
   int ch, chcount=0, intoken=1, skipws=1, inbslash=0, inquote=0;
   int *line, *pos;

   curch = Token->text;
   line  = Line;
   pos   = Pos;
   while (intoken) {
      if (lastch != EOF) {
         ch = lastch;
         lastch = EOF;
      } else {
         ch = _slcGetCh(RuleFile,skipws,skipws,line,pos);
      }

      /* Handle special quoting stuff first */
      if (inbslash) {           /* \ always quotes next character, even \   */
         /* NEED EOF CHECK */
         _slc_ADDCHAR(Token,ch);
         inbslash = 0;
         skipws = !inquote;
         continue;
      }
      if (ch == '\\') {         /* Prepare to quote next character          */
         inbslash = 1;
         skipws   = 0;
         continue;
      }
      if (inquote) {            /* Unless terminating ", just add next char */
         /* NEED EOF CHECK */
         if (ch == '"') {
            inquote = 0;
            skipws  = 1;
         } else {
            _slc_ADDCHAR(Token,ch);
         }
         continue;
      }

      /* Then normal character processing */
      switch (ch) {
         case EOF:      /* We've hit end of input file */
            if (chcount > 0) {
               Token->type = _slcC_TOKEN_TEXT;
               lastch = ch;     /* this avoids calling GetCh again */
            } else {
               Token->type = _slcC_TOKEN_EOF;
            }
            intoken = 0;
            break;
         case ' ':      /* Whitespace */
         case ',':      /* Or comma   */
            if (chcount > 0) {
               Token->type = _slcC_TOKEN_TEXT;
               intoken = 0;
            }
            break;
         case '"':      /* Start quoting */
            inquote = 1;
            skipws  = 0;
            break;
         case '{':      /* Start of rule elements */
            if (chcount > 0) {
               Token->type = _slcC_TOKEN_TEXT;
               lastch = ch;
            } else {
               Token->type = _slcC_TOKEN_LBRACE;
               Token->text[0] = ch;
               chcount++;
            }
            intoken = 0;
            break;
         case '}':      /* End of rule elements */
            if (chcount > 0) {
               Token->type = _slcC_TOKEN_TEXT;
               lastch = ch;
            } else {
               Token->type = _slcC_TOKEN_RBRACE;
               Token->text[0] = ch;
               chcount++;
            }
            intoken = 0;
            break;
         case '=':      /* Rule element/value separator */
            if (chcount > 0) {
               Token->type = _slcC_TOKEN_TEXT;
               lastch = ch;
            } else {
               Token->type = _slcC_TOKEN_EQUAL;
               Token->text[0] = ch;
               chcount++;
            }
            intoken = 0;
            break;
         default:       /* Normal character for token */
            _slc_ADDCHAR(Token,ch);
            break;
      } /* switch (ch) */
   } /* while in token */

   /* Terminate any stuff put into the text field */
   Token->length        = chcount;
   Token->text[chcount] = '\0';

   return(0);
}


/*--------------------------------------------------------------------------*/
/*   int _slcMatchToken (_slcPS_TOKEN Token; _slcS_TOKENTBL TokenTable[])   */
/*..........................................................................*/
/*                                                                          */
/* Function to attempt to locate the supplied text token in the token table */
/* supplied by the TokenTable parameter.  If a match is found, the index    */
/* of the token table entry is returned (0-n), otherwise -1.                */
/*                                                                          */
/* (Note - this needs to be improved in the future to accept any number of  */
/*  token table parameters - and probably return a pointer to the correct   */
/*  entry rather than an index into the table)                              */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int _slcMatchToken (_slcPS_TOKEN Token, _slcS_TOKENTBL TokenTable[])
{
   char tokentext[_slcC_TOKSIZE+1], *src, *dest;
   int index;

   /* Copy over token text, lowercasing it for simpler comparison */
   dest = tokentext;
   src  = Token->text;
   while (*dest++ = tolower(*src++))
      ;

   /* Now run through table for comparision */
   index = 0;
   while (TokenTable[index].command != _slcC_TOKENTBL_TERM) {
      if (strcmp(TokenTable[index].name,tokentext) == 0) break;
      index++;
   }
   if (TokenTable[index].command == _slcC_TOKENTBL_TERM) index = -1;

   return(index);
}


/*--------------------------------------------------------------------------*/
/*          int _slcErrorMessage (char *Filename; int Line, Pos;            */
/*                                char *MsgFormat, ...)                     */
/*..........................................................................*/
/*                                                                          */
/* Routine for displaying an error message for the specified Filename,      */
/* using the same semantics as printf.  If the supplied Line/Pos parameters */
/* are >0, they are used automatically as a standard preface to the error   */
/* message.                                                                 */
/*                                                                          */
/* Since this function actually processes the output string, the maximum    */
/* size of the resulting message is _slcC_ERRSIZE characters.               */
/*                                                                          */
/* (Right now this goes to stderr)                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int _slcErrorMessage (char *Filename, int Line, int Pos,
                      char *MsgFormat, ...)
{
   char errmsg[_slcC_ERRSIZE], syslogmsg[_slcC_ERRSIZE];
   va_list args;

   sprintf(errmsg,"SL-Config: Error Parsing File \"%s\"",Filename);
   if (Line > 0 || Pos > 0) {
      strcat(errmsg," (");
   }
   if (Line > 0) {
      sprintf(&errmsg[strlen(errmsg)],"Line=%d",Line);
   }
   if (Line > 0 && Pos > 0) {
      strcat(errmsg,", ");
   }
   if (Pos > 0) {
      sprintf(&errmsg[strlen(errmsg)],"Position=%d",Pos);
   }
   if (Line > 0 || Pos > 0) {
      strcat(errmsg,")");
   }
   fprintf(stderr,"%s\n",errmsg);
   va_start(args,MsgFormat);
   vsprintf(errmsg,MsgFormat,args);
   fprintf(stderr,"SL-Config: %s\n",errmsg);
    return(0);
}


/*--------------------------------------------------------------------------*/
/*      int _slcSetIntfParm (slcPS_INTERFACE Interface,                     */
/*                           _slcPS_TOKENTBL Parameter,                     */
/*                           _slcPS_TOKEN Value)                            */
/*..........................................................................*/
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int _slcSetIntfParm (slcPS_INTERFACE Interface, _slcPS_TOKENTBL Parameter,
                     _slcPS_TOKEN Value)
{
   int result = 0, index, value;

   switch (Parameter->valuetype) {
      case 0:
         /* No value arguments - just set appropriate flags */
         switch (Parameter->command) {
            case intf_nofastqueue:
               Interface->allowfastq = 0;
               break;
            case intf_fastqueue:
               Interface->allowfastq = 1;
               break;
            default:
               /* Should never happen */
               result = 4;
               break;
         }
         break;

      case 1:
         /* String parameter */
         switch (Parameter->command) {
            case intf_device:
               if (Interface->device) {
                  free(Interface->device);
               }
               Interface->device = strdup(Value->text);
               if (Interface->device == NULL) {
                  result = 3;
               }
               break;
            case intf_compression:
               if (!stricmp("on",Value->text)) {
                  Interface->compression = slc_COMPRESSION;
                  if (Interface->if_mtu == 0) {
                     /* Set mtu (since not set already) to smaller chunks */
                     Interface->if_mtu = 296;
                  }
               } else if (!stricmp("off",Value->text)) {
                  Interface->compression = slc_NOCOMPRESSION;
               } else if (!stricmp("auto",Value->text)) {
                  Interface->compression = slc_AUTOCOMPRESSION;
               } else {
                  result = 1;
               }
               break;
            case intf_attachcmd:
               if (Interface->attachcmd) {
                  free(Interface->attachcmd);
               }
               Interface->attachcmd = strdup(Value->text);
               if (Interface->attachcmd == NULL) {
                  result = 3;
               }
               break;
            case intf_attachparms:
               if (Interface->attachparms) {
                  free(Interface->attachparms);
               }
               Interface->attachparms = strdup(Value->text);
               if (Interface->attachparms == NULL) {
                  result = 3;
               }
               break;
         }
         break;

      case 2:
         /* Numeric argument with valid range */
         for (index=0; index < Value->length && !result; index++) {
            if (!isdigit(Value->text[index])) {
               result = 1;
            }
         }
         if (result)
            break;
         value = atoi(Value->text);
         if (value < Parameter->minimum || value > Parameter->maximum) {
            result = 2;
         }
         if (result)
            break;
         /* Store resulting value into interface structure */
         switch (Parameter->command) {
            case intf_mtu:
               Interface->if_mtu = value;
               break;
            case intf_rtt:
               Interface->if_rtt = value;
               break;
            case intf_rttvar:
               Interface->if_rttvar = value;
               break;
            case intf_rttmin:
               Interface->if_rttmin = value;
               break;
            case intf_sendpipe:
               Interface->if_sendpipe = value;
               break;
            case intf_recvpipe:
               Interface->if_recvpipe = value;
               break;
            case intf_ssthresh:
               Interface->if_ssthresh = value;
               break;
            case intf_queuesize:
               Interface->if_maxqueue = value;
               break;
            case intf_fastqueuesize:
               Interface->if_maxfastq = value;
               break;
            default:
               /* Should never happen */
               result = 4;
               break;
         }
   }

   return(result);
}



/*==========================================================================*/
/*                                                                          */
/*            --------------------------------------------------            */
/*                                                                          */
/*                             PUBLIC FUNCTIONS                             */
/*                                                                          */
/*            --------------------------------------------------            */
/*                                                                          */
/*==========================================================================*/



/*--------------------------------------------------------------------------*/
/* int slcParseConfiguration (char *Filename; slcPS_INTERFACE *Interfaces)  */
/*..........................................................................*/
/*                                                                          */
/* Parses a SLIP configuration file, building a linked list of interface    */
/* structures to hold the interface configuration information.              */
/*                                                                          */
/*                                                                          */
/* This function is currently geared to process files of the form:          */
/*                                                                          */
/*      interface sl# | default {                                           */
/*              parameter=value [,] parameter=value [,]  ...                */
/*      }                                                                   */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#define _slc_ERRMSG     _slcErrorMessage
#define _slc_ERRHDR     Filename,line,pos
#define _slc_NEXT_TOKEN _slcGetToken(configfile,&token,&line,&pos)

int slcParseConfiguration (char *Filename, slcPS_INTERFACE *Interfaces)
{
   int             result=0;
   _slcS_TOKEN     token;
   slcS_INTERFACE  intf_default;
   slcPS_INTERFACE curinterface, lastinterface=NULL;
   FILE            *configfile;
   int             done=0, allowvalue=0, wantvalue=0, invalue=0,
                   index, line, pos, parameter, rc;
   int             curcommand;


   /* Clear default structure */
   memset(&intf_default,'\0',sizeof(intf_default));
   /* And then insert fundamental defaults */
   intf_default.if_recvpipe = 4096;
   intf_default.if_maxqueue = 12;
   intf_default.if_maxfastq = 24;
   intf_default.allowfastq  = 1;
   intf_default.compression = slc_NOCOMPRESSION;

   if ((configfile = fopen(Filename,"r")) == NULL) {
      result = 1;
   } else {
      /* Make sure any previous parses are flushed */
      _slcGetCh(NULL,0,0,0,0);

      /* Main rule processing loop - handle a command at a time */
      while (!done) {
         /* Fetch next token */
         _slc_NEXT_TOKEN;

         /* If EOF here, then we're done */
         if (token.type == _slcC_TOKEN_EOF) {
            done = 1;
            break;
         }

         /* If not text or text but not in command table, generate error */
         if (token.type != _slcC_TOKEN_TEXT ||
             (index = _slcMatchToken(&token,_slc_CmdTable)) < 0) {
            _slc_ERRMSG(_slc_ERRHDR,"Invalid command encountered - \"%s\"",
                        token.text);
            result = done = 1;
            break;
         }

         /* We have an interface command (eventually, this will have to */
         /* handle more than just an interface command.                 */

         /* Grab next token - should be "sl#" (#=0-9) or "default" */
         _slc_NEXT_TOKEN;
         if (token.type != _slcC_TOKEN_TEXT) {
            _slc_ERRMSG(_slc_ERRHDR,"Missing interface name");
            result = done = 1;
            break;
         } else {
            if (token.length == 3 &&
                tolower(token.text[0]) == 's' &&
                tolower(token.text[1]) == 'l' &&
                token.text[2] == '0') {   /* currently only 'sl0' allowed */
               /* Normal "sl#" interface - lowercase interface name  */
               /* and then make sure that we haven't already used it */
               strlwr(token.text);
               curinterface = *Interfaces;
               while (curinterface) {
                  if (!strcmp(curinterface->if_name,token.text)) {
                     _slc_ERRMSG(_slc_ERRHDR,"Duplicate interface \"%s\"",
                                 token.text);
                     result = done = 1;
                     break;
                  }
                  curinterface = curinterface->next;
               }
               /* Unique - allocate room for new interface */
               if (lastinterface == NULL) {
                  lastinterface = (slcPS_INTERFACE)
                                 calloc(1,sizeof(slcS_INTERFACE));
                  curinterface = *Interfaces = lastinterface;
               } else {
                  lastinterface->next = (slcPS_INTERFACE)
                                        calloc(1,sizeof(slcS_INTERFACE));
                  curinterface = lastinterface = lastinterface->next;
               }
               if (curinterface == NULL) {
                  _slc_ERRMSG(_slc_ERRHDR,
                              "Error allocating space for new interface");
                  result = done = 1;
                  break;
               }

               /* Copy over default block for initial values */
               memcpy(curinterface,&intf_default,sizeof(intf_default));

               /* Update interface name and insert default device name */
               strcpy(curinterface->if_name,token.text);
               curinterface->device = malloc(5);
               strcpy(curinterface->device,"com");
               curinterface->device[3] = token.text[2]+1;
               curinterface->device[4] = '\0';
            } else if (!stricmp(token.text,"default")) {
               /* Default interface */
               curinterface = &intf_default;
            } else {
               _slc_ERRMSG(_slc_ERRHDR,"Invalid interface \"%s\" "
                          "(use sl0 or 'default')", token.text);
               result = done = 1;
               break;
            }
         }

         /* This is a kludge - TBD: work on control flow in here */
         if (done) break;

         /* Process any parameters for this interface - First, make */
         /* sure that the next parseable element is the brace ({).  */
         _slc_NEXT_TOKEN;
         if (token.type != _slcC_TOKEN_LBRACE) {
            _slc_ERRMSG(_slc_ERRHDR,"Expecting {, read \"%s\"",token.text);
            result = 1;
            break;
         }

         /* Ok - we're into the elements - go until right brace (EOF=error) */
         do {
            _slc_NEXT_TOKEN;
            switch (token.type) {
               case _slcC_TOKEN_TEXT:
                  /* This is either element name or value, according to */
                  /* wantvalue variable - verify in either case.        */
                  if (wantvalue) {
                     if (rc = _slcSetIntfParm(curinterface,
                                              &_slc_IntfTable[parameter],
                                              &token)) {
                        switch (rc) {
                           case 1:
                              _slc_ERRMSG(_slc_ERRHDR,
                                          "Invalid parameter value \"%s\"",
                                          token.text);
                              break;
                           case 2:
                              _slc_ERRMSG(_slc_ERRHDR,"Parameter value (%s) "
                                          "out of range (%d-%d)",token.text,
                                          _slc_IntfTable[parameter].minimum,
                                          _slc_IntfTable[parameter].maximum);
                              break;
                           case 3:
                              _slc_ERRMSG(_slc_ERRHDR,"Memory allocation "
                                          "failure while parsing parameter");
                              break;
                           case 4:
                              _slc_ERRMSG(_slc_ERRHDR,"Internal error - "
                                          "unknown parameter");
                              break;
                           default:
                              _slc_ERRMSG(_slc_ERRHDR,"Internal error - "
                                          "unknown result (%d) from "
                                          "_slcSetIntfParm",rc);
                              break;
                        }
                        result = done = 1;
                     }
                     invalue = 1;
                  } else {
                     if ((parameter = _slcMatchToken(&token,
                                                     _slc_IntfTable)) < 0) {
                        _slc_ERRMSG(_slc_ERRHDR,"Invalid parameter \"%s\"",
                                    token.text);
                        result = done = 1;
                     } else {
                        allowvalue = _slc_IntfTable[parameter].valuetype > 0;
                        if (!allowvalue) {
                           _slcSetIntfParm(curinterface,
                                           &_slc_IntfTable[parameter],NULL);
                        }
                     }
                     invalue = 0;
                  }
                  wantvalue = 0;
                  break;
               case _slcC_TOKEN_EQUAL:
                  /* this is only valid if we don't yet want a value */
                  /* and we're allowed a value.                      */
                  if (wantvalue) {
                     _slc_ERRMSG(_slc_ERRHDR,
                                 "Expecting parameter, read \"%s\"",
                                 token.text);
                     result = done = 1;
                  } else if (!allowvalue) {
                     _slc_ERRMSG(_slc_ERRHDR,
                                 "Parameter \"%s\" does not require a value",
                                 _slc_IntfTable[parameter].name);
                     result = done = 1;
                  } else {
                     wantvalue = 1;
                  }
                  break;
               case _slcC_TOKEN_RBRACE:
                  /* Error if we are waiting for a value */
                  if (wantvalue) {
                     _slc_ERRMSG(_slc_ERRHDR,
                                 "Expecting parameter value, read \"%s\"",
                                 token.text);
                     result = done = 1;
                  } else {
                     invalue = 0;
                  }
                  break;
               case _slcC_TOKEN_EOF:
                  _slc_ERRMSG(_slc_ERRHDR,"Expecting }%s, got End-Of-File",
                              wantvalue ? "" : " or =");
                  result = done = 1;
                  break;
               default:
                  if (token.type <= _slcC_TOKEN_ERROR) {
                     _slc_ERRMSG(_slc_ERRHDR,
                                 "Tokenize error %d (%s) while reading file.",
                                 token.type,_slc_TokenErrorTable[token.type]);
                  } else {
                     _slc_ERRMSG(_slc_ERRHDR,"Expecting }%s, read \"%s\"",
                                 wantvalue ? "" : " or =",token.text);
                  }
                  result = done = 1;
                  break;
            } /* switch token.type */
         } while (token.type != _slcC_TOKEN_RBRACE && !done);

         if (curinterface->if_mtu == 0) {
            curinterface->if_mtu = 1006;  /* Default if not set otherwise */
         }
      } /* while !done */
      fclose(configfile);
   } /* if file ok */

   return(result);
}


/*--------------------------------------------------------------------------*/
/*      int slcFreeConfiguration (slcPS_INTERFACE *Interfaces)              */
/*..........................................................................*/
/*                                                                          */
/* Frees up a previously parsed interface configuration.                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int slcFreeConfiguration (slcPS_INTERFACE *Interfaces)
{
   slcPS_INTERFACE curptr, delptr;

   curptr = *Interfaces;
   while (curptr) {
      if (curptr->device) {
         free(curptr->device);
      }
      delptr = curptr;
      curptr = curptr->next;
      free(delptr);
   }
   *Interfaces = NULL;
    return(0);
}
#endif /* NT */

