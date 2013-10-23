#include <windows.h>
#include <process.h>
#include <string.h>
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckcasc.h"             /* ASCII character symbols */
#include "ckcxla.h"             /* Character set translation */
#include "ckcuni.h"             /* Unicode */
#include "ckcnet.h"             /* Network support */
#include "ckuusr.h"             /* For terminal type definitions, etc. */
#include "ckopcf.h"             /* PC Fonts resource definitions */
#include "ckocon.h"
#include "ckokey.h"

#include "ckcxxx.h"
#include "ikextern.h"
#include "ikcmd.h"

extern int local, xitwarn, ttyfd;
extern char ttname[];			/* Communication device name */
extern int vmode;
extern int maclvl;                  /* Macro invocation level */
extern CHAR sstate ;
#ifdef DCMDBUF
    extern struct cmdptr *cmdstk;
#else
    extern struct cmdptr cmdstk[];
#endif /* DCMDBUF */
#ifndef NOSPL
extern struct mtab *mactab;             /* Main macro table */
extern int nmac;                        /* Number of macros */
extern int cmdlvl;
#else
extern int tlevel;
#endif
extern int ckxech;
int kui_async = 0;

extern int os2getcp( void );
extern int os2getcplist( int*, int );

static HWND hwndKUI = NULL;

void
setHwndKUI( HWND hwnd ) {
    hwndKUI = hwnd ;
}

HWND 
getHwndKUI( void ) {
    return hwndKUI;
}

int
putCmdString( char* cmd ) {
/*
  If prop sheet not app modal
  we need to suspend the execution of the command thread
*/
    if ( 
#ifndef NOSPL
         cmdlvl > 0
#else
         tlevel > -1
#endif /* NOSPL */
         ) {
        return(-1);
    }

    kui_async = 1;
    if ( vmode == VTERM ) {
        apc_command(APC_LOCAL,cmd);
        kui_async = 0;
        return(0);
    } 
    else {
        if (mlook(mactab,"_kui_commands",nmac) == -1) {
            int vmsave = vmode;
            vmode = VCMD;
            if ( vmsave != VCMD )
                VscrnIsDirty(vmsave);
            VscrnIsDirty(VCMD);
            debug(F110,"kui_command about to execute command",cmd,0);
            domac("_kui_commands",cmd,cmdstk[cmdlvl].ccflgs|CF_IMAC);
            debug(F110,"kui_command finished executing command",cmd,0);
            delmac("_kui_commands",1);
            cmini(ckxech);
            vmode = vmsave;
            VscrnIsDirty(VCMD);
            if ( vmode != VCMD )
                VscrnIsDirty(vmode);
            kui_async = 0;
            return(0);
        } 
        kui_async = 0;
        return(-1);
    }                         
}

void 
putKeyboardBuffer( BYTE vnum, char* cmd ) {
    putmacro( vnum, cmd );
}

extern int viewonly;
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL isConnected()
{
    return ( viewonly ||    /* View-only terminal mode */
             ( (network && ttyfd >= -1) || /* I/O err on network */
              (carrier == CAR_OFF) || /* Carrier dropped */
              ((ttgmdm() & BM_DCD) != 0)));
}


BOOL isScriptRunning()
{
#ifndef NOSPL
    return(cmdlvl != 0);
#else
    return(tlevel != -1);
#endif 
}

extern char
  answerback[],
  sesfil[];

extern unsigned char
  colornormal, colorunderline, colorstatus,
  colorhelp,   colorselect,    colorborder, coloritalic;

extern int
  apcstatus,
  autodl,
  adl_ask,
  cmdmsk,
  debses,
  duplex,
  escape,
  tcsl,
  tcsr,
  tnlm,
  tt_answer,
  tt_arrow,
  tt_bell,
  tt_ctstmo,
  tt_cols,
  tt_cursor,
  tt_roll,
  tt_rows,
  tt_scrsize,
  tt_type,
  tt_type_mode,
  tt_updmode,
  tt_wrap,
  tt_keypad,
  tt_pacing,
  tt_update,
  tt_crd,
  tt_modechg,
  seslog
;

extern struct _vtG G[4], *GL, *GR;
extern struct x_to_unicode* txrinfo[];
extern struct tt_info_rec tt_info[];

static  char * colors[16] = {
    "black","blue","green","cyan","red","magenta","brown","lightgray",
    "dgray","lightblue","lightgreen","lightcyan","lightred"
	,"lightmagenta","yellow","white"
};

static char buf[256];

void
gstring(int n) {
    char* s;
    if (G[n].designation == TX_TRANSP)
      s = "transparent";
    else if (G[n].designation == TX_DECSPEC)
      s = "dec-special";
    else s = txrinfo[G[n].designation]->keywd;
    strcpy(buf,s);
}

char *
getVar(unsigned int idx) {

    int flags = (idx & 0xf0000000) >> 28;
    int type  = (idx & 0x0f000000) >> 24;
    int group = (idx & 0x00ff0000) >> 16;
    int index = (idx & 0x0000ffff);
    char * s;
    int i;

    buf[0] = 0;

    switch (idx) 
    {
    case TERM_AB_MSG:
        updanswerbk();
        strncpy(buf,answerback,255);
        break;

    case TERM_ANSWER:
        strcpy(buf,tt_answer ? "on" : "off");
        break;

    case TERM_APC:
        if (apcstatus == APC_ON) s = "on";
        else if (apcstatus == APC_OFF) s = "off";
        else if (apcstatus == APC_UNCH) s = "unchecked";
        strcpy(buf,s);
        break;

    case TERM_ARROW:
        strcpy(buf, tt_arrow ? "application" : "cursor");
        break;

    case TERM_AUTODL:
        strcpy(buf, autodl ? "on" : "off");
        break;

    case TERM_BELL:
        switch ( tt_bell ) {
        case XYB_NONE:
            s = "none" ;
            break;
        case XYB_VIS:
            s ="visible";
            break;
        case XYB_AUD | XYB_BEEP:
            s="beep";
            break;
        case XYB_AUD | XYB_SYS:
            s="system sounds";
            break;
        default:
            s="(unknown)";
            break;
        }
        strcpy(buf,s);
        break;

        case TERM_BYTESIZE:
        strcpy(buf, cmdmsk == 0377 ? "8" : "7");
        break;

    case TERM_CHAR_LOCAL:
        strcpy(buf,"not implemented - see ikcmd.c");
        break;

    case TERM_CHAR_REMOTE:
        for ( i=0; i<ntxrtab; i++ ) {
            if ( txrtab[i].kwval == tcsr )
                strcpy(buf,txrtab[i].kwd);
        }
        break;

    case TERM_CHAR_G0:
        gstring(0);
        break;

    case TERM_CHAR_G1:
        gstring(1);
        break;

    case TERM_CHAR_G2:
        gstring(2);
        break;

    case TERM_CHAR_G3:
        gstring(3);
        break;

    case TERM_CHAR_GL:
        for (i = 0; i < 4; i++)
            if (GL == &G[i]) break;
        if (i < 4) {
            buf[0] = 'G';
            buf[1] = i + '0';
            buf[2] = '\0';
        } else buf[0] = '\0';
        break;

    case TERM_CHAR_GR:
        for (i = 0; i < 4; i++)
            if (GR == &G[i]) break;
            if (i < 4) {
                buf[0] = 'G';
                buf[1] = i + '0';
                buf[2] = '\0';
            } else buf[0] = '\0';
            break;

        case TERM_CODEPAGE:
            _itoa(os2getcp(), buf, 10);
            break;

        case TERM_COLOR_BORDER:
            strcpy(buf,colors[colorborder & 0x0f]);
            break;

        case TERM_COLOR_CURSOR:
            break;

        case TERM_COLOR_HELP:
            strcpy(buf,colors[colorhelp & 0x0f]);
            strcat(buf,",");
            strcat(buf,colors[colorhelp >> 4]);
            break;

        case TERM_COLOR_SELECTION:
            strcpy(buf,colors[colorselect & 0x0f]);
            strcat(buf,",");
            strcat(buf,colors[colorselect >> 4]);
            break;

        case TERM_COLOR_STATUS:
            strcpy(buf,colors[colorstatus & 0x0f]);
            strcat(buf,",");
            strcat(buf,colors[colorstatus >> 4]);
            break;

        case TERM_COLOR_TERM:
            strcpy(buf,colors[colornormal & 0x0f]);
            strcat(buf,",");
            strcat(buf,colors[colornormal >> 4]);
            break;

        case TERM_COLOR_UNDERLINE:
            strcpy(buf,colors[colorunderline & 0x0f]);
            strcat(buf,",");
            strcat(buf,colors[colorunderline >> 4]);
            break;

       case TERM_COLOR_ITALIC:
        strcpy(buf,colors[coloritalic & 0x0f]);
        strcat(buf,",");
        strcat(buf,colors[coloritalic >> 4]);
        break;

        case TERM_CPLIST: {
            int cplist[8], cps;
            cps = os2getcplist(cplist, sizeof(cplist));
            sprintf(buf,"%3d,%3d,%3d,%3d", 
                cps > 1 ? cplist[1] : 0, cps > 2 ? cplist[2] : 0,
                cps > 3 ? cplist[3] : 0, cps > 4 ? cplist[4] : 0 );
            }
            break;

        case TERM_CRD:
            strcpy(buf,tt_crd ? "crlf" : "normal");
            break;

        case TERM_CTSTMO:
            _itoa(tt_ctstmo, buf, 10);
            break;

        case TERM_CURSOR:
            strcpy(buf,
                (tt_cursor == 2) ? "full" :
                (tt_cursor == 1) ? "half" : "underline" );
            break;

        case TERM_DEBUG:
            strcpy(buf, debses ? "on" : "off");
            break;

        case TERM_ECHO:
            strcpy(buf, duplex ? "local" : "remote");
            break;

        case TERM_ESCAPE:
            _itoa( escape, buf, 4 ); 
            break;

        case TERM_FONT:
           if (term_font) {
               int i;
               for (i = 0; i < ntermfont; i++) {
                   if (tt_font == term_font[i].kwval) {
                       strcpy(buf,term_font[i].kwd);
                       break;
                   }
               }
           } else
                buf[0] = '\0';
            break;

        case TERM_FONT_SIZE:
            _itoa( tt_font_size, buf, 10 );
            break;

        case TERM_HEIGHT:
            _itoa( tt_rows, buf, 10 );
            break;

        case TERM_ID:
            if (tt_info[tt_type].x_id)
            if (strlen(tt_info[tt_type].x_id)) {
                strcpy(buf,"<ESC>");
                strcat(buf,tt_info[tt_type].x_id);
            }
            break;

        case TERM_KEYPAD:
            strcpy(buf, tt_keypad ? "application" : "numeric");
            break;

		case TERM_VIDEO_CHANGE:
			strcpy(buf, tt_modechg ? "enabled" : "disabled" );
			break;

        case TERM_NAME:
            if (tt_info[tt_type].x_name)
            if (strlen(tt_info[tt_type].x_name)) {
                /*strcpy(buf,"<ESC>");
                strcat(buf,tt_info[tt_type].x_name);*/
                strcpy(buf,tt_info[tt_type].x_name);
            }
            break;

        case TERM_NLM:
            strcpy(buf, tnlm ? "on" : "off");
            break;

        case TERM_PACING:
            _itoa( tt_pacing, buf, 10 );
            break;

        case TERM_ROLL:
            strcpy(buf, tt_roll ? "insert" : "overwrite");
            break;

        case TERM_SCRSIZE:
            _itoa( tt_scrsize, buf, 10 );
            break;

        case TERM_SESFIL:
            strncpy(buf,sesfil,255);
            break;

        case TERM_SESLOG:
            strcpy(buf, seslog ? "on" : "off");
            break;

        case TERM_SOSI:
            strcpy(buf, tnlm ? "on" : "off");
            break;

        case TERM_TYPE:
            _itoa( tt_type, buf, 10 );
            break;

        case TERM_UPD_FREQ:
            _itoa( tt_update, buf, 10 );
            break;

        case TERM_UPD_MODE:
            strcpy(buf, (tt_updmode == TTU_FAST) ? "fast" : "smooth");
            break;

        case TERM_WIDTH:
            _itoa( tt_cols, buf, 10 );
            break;

        case TERM_WRAP:
            strcpy(buf, tt_wrap ? "on" : "off");
            break;

        case TERM_TYPE_MODE:
            if (tt_info[tt_type].x_name)
            if (strlen(tt_info[tt_type_mode].x_name)) {
                strcpy(buf,tt_info[tt_type].x_name);
            }
            break;

        case LOG_HOME_DIR:
            strcpy( buf, zhome() );
            break;

        default:
            return 0;   // error!
    }
    return((char *)buf);
}



