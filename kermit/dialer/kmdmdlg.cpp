#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include <stdio.h>
#define USE_HELP_CONTEXTS
#define USE_WINDOW_MODEMS
#define USE_WINDOW_NEW_MODEM
#define USE_WINDOW_MODEM_CONFIG
#include <stdio.h>
#include "dialer.hpp"
#include "kmdmdlg.hpp"
#include "kconnect.hpp"
#include "usermsg.hpp"
#include "kmodem.hpp"

extern "C" {
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckucmd.h"
#if defined(WIN32)
#include <windows.h>            	/* Windows Definitions */
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
    extern struct keytab * tapilinetab, * _tapilinetab;
    extern int ntapiline;

#include "ktapi.h"
#endif
#include "kmdminf.h"
    /* BEGIN MDMINF STRUCT DEFINITIONS */

    /*
    Declare structures containing modem-specific information.
    REMEMBER that only the first SEVEN characters of these names are
    guaranteed to be unique.

    First declare the three types that are allowed for MINIDIAL versions.
    */

    /* A dummy getok() */
int getok( int a, int b)
{
    return a ;
}

#ifdef K95_1120_MODEMS
static
MDMINF CCITT =				/* CCITT / ITU-T V.25bis autodialer */
/*
  According to V.25bis:
  . Even parity is required for giving commands to the modem.
  . Commands might or might not echo.
  . Responses ("Indications") from the modem are terminated by CR and LF.
  . Call setup is accomplished by:
    - DTE raises DTR (V.24 circuit 108)              [ttopen() does this]
    - Modem raises CTS (V.24 circuit 106)            [C-Kermit ignores this]
    - DTE issues a call request command ("CRN")
    - Modem responds with "VAL" ("command accepted")
    - If the call is completed:
        modem responds with "CNX" ("call connected");
        modem turns CTS (106) OFF;
        modem turns DSR (107) ON;
      else:
        modem responds with "CFI <parameter>" ("call failure indication").
  . To clear a call, the DTE turns DTR (108) OFF.
  . There is no mention of the Carrier Detect circuit (109) in the standard.
  . There is no provision for "escaping back" to the modem's command mode.

  It is not known whether there exists in real life a pure V.25bis modem.
  If there is, this code has never been tested on it.  See the Digitel entry.
*/
    {
    "CCITT / ITU-T V.25bis autodialer",
    "",			/* pulse command */
    "",			/* tone command */
    40,			/* dial_time -- programmable -- */
    ",:",		/* pause_chars -- "," waits for programmable time */
                        /* ":" waits for dial tone */
    10,			/* pause_time (seconds, just a guess) */
    "",			/* wake_str (none) */
    200,		/* wake_rate (msec) */
    "VAL",		/* wake_prompt */
    "",			/* dmode_str (none) */
    "",			/* dmode_prompt (none) */
    "CRN%s\\{13}",        /* dial_str */
    200,		/* dial_rate (msec) */
    0,			/* No esc_time */
    0,			/* No esc_char  */
    "",			/* No hup_str  */
    "",			/* hwfc_str */
    "",			/* swfc_str */
    "",			/* nofc_str */
    "",			/* ec_on_str */
    "",			/* ec_off_str */
    "",			/* dc_on_str */
    "",			/* dc_off_str */
    "CIC\\{13}",		/* aa_on_str */
    "DIC\\{13}",		/* aa_off_str */
    "",			/* sb_on_str */
    "",			/* sb_off_str */
    "",			/* sp_off_str */
    "",			/* sp_on_str */
    "",			/* vol1_str */
    "",			/* vol2_str */
    "",			/* vol3_str */
    "",			/* ignoredt */
    "",			/* last minute init string */
    0L,			/* max_speed */
    CKD_V25,		/* capas */
    NULL		/* No ok_fn    */
};

static
MDMINF HAYES =				/* Hayes 2400 and compatible modems */
    {
    "Hayes Smartmodem 2400 and compatibles",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1&S0&C1&D2\\{13}",		/* wake_str */
#else
    "ATQ0\\{13}",				/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str, user supplies D or T */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        2400L,				/* max_speed */
    CKD_AT,				/* capas */
    getok				/* ok_fn */
};

/*
  The intent of the "unknown" modem is to allow KERMIT to support
  unknown modems by having the user type the entire autodial sequence
  (possibly including control characters, etc.) as the "phone number".
  The protocol and other characteristics of this modem are unknown, with
  some "reasonable" values being chosen for some of them.  The only way to
  detect if a connection is made is to look for carrier.
*/
static
MDMINF UNKNOWN =			/* Information for "Unknown" modem */
    {
    "Unknown",				/* name */
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s\\{13}",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF ATTISN =				/* AT&T ISN Network */
    {
    "",					/* pulse command */
    "",					/* tone command */
    "AT&T ISN Network",
    30,					/* Dial time */
    "",					/* Pause characters */
    0,					/* Pause time */
    "\\{13}\\{13}\\{13}\\{13}",			/* Wake string */
    900,				/* Wake rate */
    "DIAL",				/* Wake prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "%s\\{13}",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF ATTMODEM =	/* information for AT&T switched-network modems */
			/* "Number" following "dial" can include: p's and
			 * t's to indicate pulse or tone (default) dialing,
			 * + for wait for dial tone, , for pause, r for
			 * last number dialed, and, except for 2224B, some
			 * comma-delimited options like o12=y, before number.

 * "Important" options for the modems:
 *
 *	All:		Except for 2224B, enable option 12 for "transparent
 *			data," o12=y.  If a computer port used for both
 *			incoming and outgoing calls is connected to the
 *			modem, disable "enter interactive mode on carriage
 *			return," EICR.  The Kermit "dial" command can
 *			function with EIA leads standard, EIAS.
 *
 *	2212C:		Internal hardware switches at their default
 *			positions (four rockers down away from numbers)
 *			unless EICR is not wanted (rocker down at the 4).
 *			For EIAS, rocker down at the 1.
 *
 *	2224B:		Front-panel switch position 1 must be up (at the 1,
 *			closed).  Disable EICR with position 2 down.
 *			For EIAS, position 4 down.
 *			All switches on the back panel down.
 *
 *	2224CEO:	All front-panel switches down except either 5 or 6.
 *			Enable interactive flow control with o16=y.
 *			Select normal asynchronous mode with o34=0 (zero).
 *			Disable EICR with position 3 up.  For EIAS, 1 up.
 *			Reset the modem after changing switches.
 *
 *	2296A:		If option 00 (zeros) is present, use o00=0.
 *			Enable interactive flow control with o16=y.
 *			Select normal asynchronous mode with o34=0 (zero).
 *                      (available in Microcom Networking version, but
 *                      not necessarily other models of the 2296A).
 *			Enable modem-port flow control (if available) with
 * 			o42=y.  Enable asynchronous operation with o50=y.
 * 			Disable EICR with o69=n.  For EIAS, o66=n, using
 * 			front panel.
 */
    {
   "AT&T switched-network modems",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "+",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "at%s\\{13}",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    CKD_AT,				/* capas */
    NULL				/* ok_fn */
};

static
MDMINF ATTDTDM = /* AT&T Digital Terminal Data Module  */
		 /* For dialing: KYBD switch down, others usually up. */
    {
    "AT&T Digital Terminal Data Module",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "%s\\{13}",				/* dial_str */ 
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF KEEPINTOUCH =                  /* AT&T KeepinTouch Card Modem */
{
    "AT&T KeepinTouch V.32bis Card Modem", /* Name */
    "ATP\\{13}",                                /* pulse command */
    "ATT\\{13}",                                /* tone command */
    35,                                       /* dial_time */
    ",",                              /* pause_chars */
    2,                                        /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\\\K5\\{13}",    /* wake_str */
#else
    "ATQ0E1X4\\\\K5\\{13}",                       /* wake_str */
#endif /* OS2 */
    0,                                        /* wake_rate */
    "OK\\{13}",                         /* wake_prompt */
    "",                                       /* dmode_str */
    "",                                       /* dmode_prompt */
    "ATD%s\\{13}",                      /* dial_str */
    0,                                        /* dial_rate */
    1100,                             /* esc_time */
    43,                                       /* esc_char */
    "ATQ0H0\\{13}",                     /* hup_str */
    "AT\\\\Q3\\{13}",                     /* hwfc_str */
    "AT\\\\Q1\\\\X0\\{13}",                 /* swfc_str */
    "AT\\\\Q0\\{13}",                     /* nofc_str */
    "AT\\\\N3-J1\\{13}",                  /* ec_on_str */
    "AT\\\\N1\\{13}",                     /* ec_off_str */
    "AT%C3\"H3\\{13}",                  /* dc_on_str */
    "AT%C0\"H0\\{13}",                  /* dc_off_str */
    "ATS0=1\\\\{13}",                     /* aa_on_str */
    "ATS0=0\\\\{13}",                     /* aa_off_str */
    "ATN0\\\\J0\\{13}",                   /* sb_on_str */
    "ATN1\\\\J1\\{13}",                   /* sb_off_str */
    "ATM1\\{13}",                               /* sp_on_str */
    "ATM0\\{13}",                               /* sp_off_str */
    "",                                       /* vol1_str */
    "",                                       /* vol2_str */
    "",                                       /* vol3_str */
    "ATX3\\{13}",			/* ignoredt */
    "",			/* last minute init string */
    57600L,                           /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                             /* ok_fn */
};

static
MDMINF DIGITEL =        /* Digitel DT-22 CCITT variant used in Brazil */
/*
  Attempts to adhere strictly to the V.25bis specification do not produce good
  results in real life.  The modem for which this code was developed: (a)
  ignores parity; (b) sometimes terminates responses with LF CR instead of CR
  LF; (c) has a Hayes-like escape sequence; (d) supports a hangup ("HUP")
  command.  Information from Fernando Cabral in Brasilia.
*/
    {
    "Digitel DT-22 CCITT dialer",
    "",				/* pulse command */
    "",				/* tone command */
    40,				/* dial_time -- programmable -- */
    ",:",		/* pause_chars -- "," waits for programmable time */
                        /* ":" waits for dial tone */
    10,			/* pause_time (seconds, just a guess) */
    "HUP\\{13}",          /* wake_str (Not Standard CCITT) */
    200,		/* wake_rate (msec) */
    "VAL",		/* wake_prompt */
    "",			/* dmode_str (none) */
    "",			/* dmode_prompt (none) */
    "CRN%s\\{13}",        /* dial_str */
    200,		/* dial_rate (msec) */
    1100,		/* esc_time (Not Standard CCITT) */
    43,			/* esc_char  (Not Standard CCITT) */
    "HUP\\{13}",		/* hup_str  (Not Standard CCITT) */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "CIC\\{13}",				/* aa_on_str */
    "DIC\\{13}",				/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    CKD_V25,				/* capas */
    getok				/* ok_fn */
};

static
MDMINF H_1200 =		/* Hayes 1200 and compatible modems */
    {
    "Hayes Smartmodem 1200 and compatibles",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1\\{13}",			/* wake_str */
#else
    "ATQ0\\{13}",				/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        1200L,				/* max_speed */
    CKD_AT,				/* capas */
    getok				/* ok_fn */
};

static
MDMINF H_ULTRA =			/* Hayes high-speed */
    {
    "Hayes Ultra/Optima/Accura 96/144/288", /* U,O,A */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4N1Y0&S0&C1&D2S37=0S82=128\\{13}", /* wake_str */
#else
    "ATQ0X4N1Y0S37=0S82=128\\{13}",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */ 
    "AT&K3\\{13}",			/* hwfc_str */   /* OK for U,O */
    "AT&K4\\{13}",			/* swfc_str */   /* OK for U,O */
    "AT&K0\\{13}",			/* nofc_str */   /* OK for U,O */
    "AT&Q5S36=7S48=7\\{13}",		/* ec_on_str */  /* OK for U,O */
    "AT&Q0\\{13}",			/* ec_off_str */ /* OK for U,O */
    "ATS46=2\\{13}",			/* dc_on_str */
    "ATS46=0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */  /* (varies) */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF H_ACCURA =			/* Hayes Accura */
    {					/* GUESSING IT'S LIKE ULTRA & OPTIMA */
    "Hayes Accura",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4N1Y0&S0&C1&D2S37=0\\{13}",	/* wake_str */
#else
    "ATQ0X4N1Y0S37=0\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */ 
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5S36=7S48=7\\{13}",		/* ec_on_str */
    "AT&Q0\\{13}",			/* ec_off_str */
    "ATS46=2\\{13}",			/* dc_on_str */
    "ATS46=0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */  /* (varies) */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF PPI =				/* Practical Peripherals  */
    {
    "Practical Peripherals V.22bis or higher with V.42 and V.42bis",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef COMMENT
/* In newer models S82 (BREAK handling) was eliminated, causing an error. */
#ifdef OS2
    "ATQ0X4N1&S0&C1&D2S37=0S82=128\\{13}", /* wake_str */
#else
    "ATQ0X4N1S37=0S82=128\\{13}",		/* wake_str */
#endif /* OS2 */
#else /* So now we use Y0 instead */
#ifdef OS2
    "ATE1Q0V1X4N1&S0&C1&D2Y0S37=0\\{13}",	/* wake_str */
#else
    "ATQ0X4N1Y0S37=0\\{13}",		/* wake_str */
#endif /* OS2 */
#endif /* COMMENT */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5S36=7S48=7\\{13}",		/* ec_on_str */
    "AT&Q0S36=0S48=128\\{13}",		/* ec_off_str */
    "ATS46=2\\{13}",			/* dc_on_str */
    "ATS46=0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str  */
    "",					/* sb_off_str  */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF DATAPORT =			/* AT&T Dataport  */
    {
    "AT&T / Paradyne DataPort V.32 or higher",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    /*
       Note: S41=0 (use highest modulation) omitted, since it is not
       supported on the V.32 and lower models.  So let's not touch it.
    */
#ifdef OS2
    "ATQ0E1V1X6&S0&C1&D2&Q0S78=0\\{13}",	/* wake_str */
#else
    "ATQ0E1X6&Q0S78=0\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT\\\\Q3\\{13}",			/* hwfc_str */
    "AT\\\\Q1\\X0\\{13}",			/* swfc_str */
    "AT\\\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N7\\{13}",			/* ec_on_str */
    "AT\\\\N0\\{13}",			/* ec_off_str */
    "AT%C1\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF UCOM_AT =			/* Microcom DeskPorte FAST ES 28.8 */
    {
    "Microcom DeskPorte FAST 28.8",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4\\\\N0F0&S0&C1&D2\\\\K5\\{13}", /* wake_str */
#else
    "ATQ0X4F0\\K5\\{13}",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT\\\\Q3\\{13}",			/* hwfc_str */
    "AT\\\\Q1\\{13}",			/* swfc_str */
    "AT\\\\H0\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N3\\{13}",			/* ec_on_str */
    "AT\\\\N0\\{13}",				/* ec_off_str */
    "AT%C3\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT-J0\\{13}",			/* sb_on_str */
    "AT-J1\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOOM =				/* Zoom Telephonics V.32bis  */
    {
    "Zoom Telephonics V.32bis",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1N1W1X4&S0&C1&D2S82=128S95=47\\{13}", /* wake_str */
#else
    "ATQ0E1N1W1X4S82=128S95=47\\{13}",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5S36=7S48=7\\{13}",		/* ec_on_str */
    "AT&Q0\\{13}",			/* ec_off_str */
    "ATS46=138\\{13}",			/* dc_on_str */
    "ATS46=136\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZYXEL =				/* ZyXEL U-Series */
    {
    "ZyXEL U-Series V.32bis or higher",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1&S0&C1&D2&N0X5&Y1\\{13}",	/* wake_str */
#else
    "ATQ0E1&N0X5&Y1\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&H3\\{13}",			/* hwfc_str */
    "AT&H4\\{13}",			/* swfc_str */
    "AT&H0\\{13}",			/* nofc_str */
    "AT&K3\\{13}",			/* ec_on_str */
    "AT&K0\\{13}",			/* ec_off_str */
    "AT&K4\\{13}",			/* dc_on_str */
    "AT&K3\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOLTRIX =			/* Zoltrix */
    {
    "Zoltrix V.32bis and V.34 modems with Rockwell ACI chipset",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
   "ATE1Q0V1F0W1X4Y0&S0&C1&D2\\\\K5S82=128S95=41\\{13}", /* wake_str */
#else
   "ATQ0E1F0W1X4Y0\\\\K5S82=128S95=41\\{13}", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",		/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT\\\\N3\\{13}",			/* ec_on_str */
    "AT\\\\N1\\{13}",			/* ec_off_str */
    "ATS46=138%C3\\{13}",			/* dc_on_str */
    "ATS46=136%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT\\\\N0\\{13}",			/* sb_on_str */
    "AT&Q0\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MOTOROLA = {			/* Motorola FasTalk II or Lifestyle */
/*
  "\E" and "\X" commands removed - Motorola Lifestyle doesn't have them.
     \E0 = Don't echo while online
     \X0 = Process Xon/Xoff but don't pass through
*/
    "Motorola FasTalk II or Lifestyle",	/* Name */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2\\\\K5\\\\V1\\{13}",	/* wake_str */
#else
    "ATQ0E1X4\\\\K5\\\\V1\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT\\\\Q3\\{13}",			/* hwfc_str */
    "AT\\\\Q1\\{13}",			/* swfc_str */
    "AT\\\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N6\\{13}",			/* ec_on_str */
    "AT\\\\N1\\{13}",			/* ec_off_str */
    "AT%C1\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT\\\\J0\\{13}",			/* sb_on_str */
    "AT\\\\J1\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",			/* ignoredt */
    "",			/* last minute init string */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF BOCA =				/* Boca */
    {
    "BOCA 14.4 Faxmodem",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1F1N1W1&S0&C1&D2\\\\K5S37=11S82=128S95=47X4\\{13}", /* wake_str */
#else
    "ATQ0E1F1N1W1\\\\K5S37=11S82=128S95=47X4\\{13}", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT\\\\N3S36=7S48=7\\{13}",		/* ec_on_str */
    "AT\\\\N1\\{13}",			/* ec_off_str */
    "ATS46=138\\{13}",			/* dc_on_str */
    "ATS46=136\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF INTEL =				/* Intel */
    {
    "Intel High-Speed Faxmodem",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1Y0X4&S0&C1&D2\\\\K1\\\\V2S25=50\\{13}", /* wake_str */
#else
    "ATQ0E1Y0X4\\\\K1\\\\V2S25=50\\{13}",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "ATB1+FCLASS=0\\{13}",		/* dmode_str */ 
    "OK\\{13}",				/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT\\\\G1\\\\Q3\\{13}",			/* hwfc_str */
    "AT\\\\G1\\\\Q1\\X0\\{13}",		/* swfc_str */
    "AT\\\\G0\\{13}",			/* nofc_str */
    "AT\\\\J0\\\\N3\"H3\\{13}",		/* ec_on_str */
    "AT\\\\N1\\{13}",			/* ec_off_str */
    "AT%C1\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MULTITECH =			/* Multitech */
    {
    "Multitech MT1432 or MT2834 Series",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2&E8&Q0%E1\\{13}", /* wake_str */
#else
    "ATQ0E1X4&E8&Q0%E1#P0\\{13}",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&E4&E7&E8&E11&E13\\{13}",		/* hwfc_str */
    "AT&E5&E6&E8&E11&E13\\{13}",		/* swfc_str */
    "AT&E3&E7&E8&E10&E12\\{13}",		/* nofc_str */
    "AT&E1\\{13}",			/* ec_on_str */
    "AT#L0&E0\\{13}",			/* ec_off_str */
    "AT&E15\\{13}",			/* dc_on_str */
    "AT&E14\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT$BA0\\{13}",			/* sb_on_str (= "baud adjust off") */
    "AT$BA1\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SUPRA =				/* Supra */
    {
    "SupraFAXModem 144 or 288",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\\\\K5S82=128\\{13}", /* wake_str */
#else
    "ATQ0E1N1W0X4Y0\\\\K5S82=128\\{13}",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5\\\\N3S48=7\\{13}",		/* ec_on_str */
    "AT&Q0\\\\N1\\{13}",			/* ec_off_str */
    "AT%C1S46=138\\{13}",			/* dc_on_str */
    "AT%C0S46=136\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM\\{13}",				/* sp_off_str */
    "ATL\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SUPRAX =				/* Supra Express */
    {
    "Diamond Supra Express V.90",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1W0X4&C1&D2&S0\\K5015",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1W0X4&S1\\K5015",		/* wake_str */
#else
    "ATQ0E1W0X4\\K5015",		/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT\\\\N3\\{13}",			/* ec_on_str */
    "AT\\\\N1\\{13}",			/* ec_off_str */
    "AT%C2\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM\\{13}",				/* sp_off_str */
    "ATL\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",				/* ignoredt */
    "",					/* ini2 */
    230400L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};


static
MDMINF MAXTECH =			/* MaxTech */
    {
    "MaxTech XM288EA or GVC FAXModem",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2&L0&M0\\\\K5\\{13}", /* wake_str */
#else
    "ATQ0E1X4Y0&L0&M0\\\\K5\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT\\\\Q3\\{13}",			/* hwfc_str */
    "AT\\\\Q1\\\\X0\\{13}",			/* swfc_str */
    "AT\\\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N6\\{13}",			/* ec_on_str */
    "AT\\\\N0\\{13}",			/* ec_off_str */
    "AT\\\\N6%C1\\{13}",			/* dc_on_str */
    "AT\\\\N6%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ROLM =		/* IBM / Siemens / Rolm 8000, 9000, 9751 CBX DCM */
    {
    "IBM/Siemens/Rolm CBX Data Communications Module",
    "",					/* pulse command */
    "",					/* tone command */
    60,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "\\{13}\\{13}",			/* wake_str */
    50, 				/* wake_rate */
    "MODIFY?",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "CALL %s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        19200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF USR =				/* USR Courier and Sportster modems */
    {
    "US Robotics Courier or Sportster",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&A3&S0&C1&D2&N0&Y3\\{13}",	/* wake_str */
#else
    "ATQ0X4&A3&N0&Y3\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&H1&R2&I0\\{13}",			/* hwfc_str */
    "AT&H2&R1&I2\\{13}",			/* swfc_str */
    "AT&H0&R1&I0\\{13}",			/* nofc_str */
    "AT&M4&B1\\{13}",			/* ec_on_str */
    "AT&M0\\{13}",			/* ec_off_str */
    "AT&K1\\{13}",			/* dc_on_str */
    "AT&K0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

#ifdef OLDTBCODE
static
MDMINF TELEBIT =			/* All Telebits */
    {
    "Telebit - all models",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    60,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
/*
  NOTE: The wake_string MUST contain the I command (model query), and otherwise
  must contain commands that work on ALL Telebit models.  Here we ensure that
  result codes are returned (Q0), and ask for extended result codes (X1), and
  ensure that the escape sequence is +++ and it is enabled.  And also, make
  sure the final character is not a digit (whose echo might be mistaken for a
  result code).  The Ctrl-Q (\021) and multiple A's are recommended by Telebit.
*/
#ifdef OS2
    "\\{17}AAAAATQ0X1&S0&C1&D2S12=50 S50=0 I\\{13}", /* wake_str. */
#else
	"\\{17}AAAAATQ0X1S12=50 S50=0 I\\{13}", /* wake_str. */
#endif /* OS2 */
    100,				/* wake_rate = 100 msec */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str, Note: no T or P */
    80,					/* dial_rate */
    1100,				/* esc_time (guard time) */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "ATS58=2S68=2\\{13}",			/* hwfc_str */
    "ATS58=3S68=3\\{13}",			/* swfc_str */
    "ATS58=0S68=0\\{13}",			/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB, /* capas */
    getok				/* ok_fn */
};

#else

static
MDMINF OLDTB =				/* Old Telebits */
    {
    "Telebit TrailBlazer, T1000, T1500, T2000, T2500",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    60,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "\\{17}AAAAATQ0E1V1X1&S0&C1&D2S12=50S50=0S54=3\\{13}", /* wake_str. */
#else
    "\\{17}AAAAATQ0X1S12=50S50=0S54=3\\{13}", /* wake_str. */
#endif /* OS2 */
    100,				/* wake_rate = 100 msec */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str, Note: no T or P */
    80,					/* dial_rate */
    1100,				/* esc_time (guard time) */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "ATS58=2S68=2\\{13}",			/* hwfc_str */
    "ATS58=3S68=3S69=0\\{13}",		/* swfc_str */
    "ATS58=0S68=0\\{13}",			/* nofc_str */
    "ATS66=1S95=2\\{13}",			/* ec_on_str */
    "ATS95=0\\{13}",			/* ec_off_str */
    "ATS110=1S96=1\\{13}",		/* dc_on_str */
    "ATS110=0S96=0\\{13}",		/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        19200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB|CKD_KS, /* capas */
    getok				/* ok_fn */
};

static
MDMINF NEWTB =				/* New Telebits */
    {
    "Telebit T1600, T3000, QBlazer, WorldBlazer, etc.",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    60,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "\\{17}AAAAATQ0E1V1X2&S0&C1&D2S12=50S50=0S61=1S63=0\\{13}", /* wake_str. */
#else
    "\\{17}AAAAATQ0X2S12=50S50=0S61=1S63=0\\{13}", /* wake_str. */
#endif /* OS2 */
    100,				/* wake_rate = 100 msec */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str, Note: no T or P */
    80,					/* dial_rate */
    1100,				/* esc_time (guard time) */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "ATS58=2S68=2\\{13}",			/* hwfc_str */
    "ATS58=3S68=3\\{13}",			/* swfc_str */
    "ATS58=0S68=0\\{13}",			/* nofc_str */
    "ATS180=3\\{13}",			/* ec_on_str */
    "ATS180=0\\{13}",			/* ec_off_str */
    "ATS190=1\\{13}",			/* dc_on_str */
    "ATS190=0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        38400L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB|CKD_KS, /* capas */
    getok				/* ok_fn */
};
#endif /* OLDTBCODE */
#endif /* MINIDIAL */

static
MDMINF DUMMY = /* dummy information for modems that are handled elsewhere */
    {
    "(dummy)",
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s\\{13}",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF RWV32 =				/* Generic Rockwell V.32 */
    {
    "Generic Rockwell V.32 modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4W1Y0&S0&C1&D2%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",		/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5\\\\N0\\{13}",			/* ec_on_str */
    "AT&Q0\\\\N1\\{13}",			/* ec_off_str */
    "AT%C1\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF RWV32B =				/* Generic Rockwell V.32bis */
    {
    "Generic Rockwell V.32bis modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4W1Y0&S0&C1&D2%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",		/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5S36=7S48=7\\\\N3\\{13}",		/* ec_on_str */
    "AT&Q0S48=128\\\\N1\\{13}",		/* ec_off_str */
    "ATS46=138%C1\\{13}",			/* dc_on_str */
    "ATS46=136%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF RWV34 =				/* Generic Rockwell V.34 Data/Fax */
    {
    "Generic Rockwell V.34 modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4W1Y0%E2&S0&C1&D2\\\\K5+FCLASS=0+MS=11,1\\{13}", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\\\K5+FCLASS=0+MS=11,1\\{13}", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",		/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5S36=7S48=7\\\\N3\\{13}",		/* ec_on_str */
    "AT&Q0S48=128\\\\N1\\{13}",		/* ec_off_str */
    "ATS46=138%C1\\{13}",			/* dc_on_str */
    "ATS46=136%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF RWV90 =				/* Generic Rockwell V.90 Data/Fax */
    {
    "Generic Rockwell V.90 56K modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "AT&F0Q0E1V1&S0&C1&D1W1%E2\\\\K5+FCLASS=0N1S0=0S37=0\\\\V1\\{13}",
#else
#ifdef VMS
    "AT&F0Q0&S1W1%E2\\\\K5+FCLASS=0N1S0=0S37=0\\\\V1\\{13}", /* wake_str */
#else
    "AT&F0Q0W1%E2\\\\K5+FCLASS=0N1S0=0S37=0\\\\V1\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",		/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5S36=7S48=7\\\\N3\\{13}",		/* ec_on_str */
    "AT&Q0S48=128\\\\N1\\{13}",		/* ec_off_str */
    "AT%C3\\{13}",			/* dc_on_str */
    "ATS46=136%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MWAVE =				/* IBM Mwave */
    {
    "IBM Mwave Adapter",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2&M0&Q0&N1\\\\K3\\\\T0%E2S28=0\\{13}", /* wake_str */
#else
    "ATQ0X4Y0&M0&Q0&N1&S0\\\\K3\\\\T0%E2S28=0\\{13}", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT\\\\Q3\\{13}",			/* hwfc_str */
    "",					/* swfc_str (it doesn't!) */
    "AT\\\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N7\\{13}",			/* ec_on_str */
    "AT\\\\N0\\{13}",			/* ec_off_str */
    "AT%C1\"H3\\{13}",			/* dc_on_str */
    "AT%C0\"H0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF TELEPATH =			/* Gateway 2000 Telepath */
    {
    "Gateway 2000 Telepath II 28.8",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2&N0&Y2#CLS=0S13=0S15=0S19=0\\{13}", /* wake_str */
#else
    "ATQ0X4&N0&Y1#CLS=0S13=0S15=0S19=0\\{13}", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&H1&R2\\{13}",			/* hwfc_str */
    "AT&H2&I2S22=17S23=19\\{13}",		/* swfc_str */
    "AT&H0&I0&R1\\{13}",			/* nofc_str */
    "AT&M4&B1\\{13}",			/* ec_on_str -- also fixes speed */
    "AT&M0\\{13}",			/* ec_off_str */
    "AT&K1\\{13}",			/* dc_on_str */
    "AT&K0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF CARDINAL =			/* Cardinal - based on Rockwell V.34 */
    {
    "Cardinal MVP288X Series",		/* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4W1Y0%E2&S0&C1&D2\\\\K5+FCLASS=0+MS=11,1\\{13}", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\\\K5+FCLASS=0+MS=11,1\\{13}", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",		/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT&Q5S36=7S48=7\\\\N3\\{13}",		/* ec_on_str */
    "AT&Q0S48=128\\\\N1\\{13}",		/* ec_off_str */
    "ATS46=138%C1\\{13}",			/* dc_on_str */
    "ATS46=136%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

/*
  Now the "old" modems, all grouped together, and also within 
  "if not defined MINIDIAL"...
*/
#ifdef OLDMODEMS

static
MDMINF CERMETEK =	/* Information for "Cermetek Info-Mate 212 A" modem */
    {
    "Cermetek Info-Mate 212 A",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "BbPpTt",				/* pause_chars */
    0,					/* pause_time */
    "  XY\016R\\{13}",			/* wake_str */
    200,				/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "\016D '%s'\\{13}",			/* dial_str */
    200,				/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        1200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF03 =		/* information for "DEC DF03-AC" modem */
    {
    "Digital DF03-AC",
    "",					/* pulse command */
    "",					/* tone command */
    27,					/* dial_time */
    "=",				/* pause_chars */
    15,					/* pause_time */
    "\001\002",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF100 =		/* information for "DEC DF100-series" modem */
			/*
			 * The telephone "number" can include "P"s and/or "T"s
			 * within it to indicate that subsequent digits are
			 * to be dialed using pulse or tone dialing.  The
			 * modem defaults to pulse dialing.  You may modify
			 * the dial string below to explicitly default all
			 * dialing to pulse or tone, but doing so prevents
			 * the use of phone numbers that you may have stored
			 * in the modem's memory.
			 */
    {
    "Digital DF-100",
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "=",				/* pause_chars */
    15,					/* pause_time */
    "\001",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s#",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF200 =		/* information for "DEC DF200-series" modem */
			/*
			 * The telephone "number" can include "P"s and/or "T"s
			 * within it to indicate that subsequent digits are
			 * to be dialed using pulse or tone dialing.  The
			 * modem defaults to pulse dialing.  You may modify
			 * the dial string below to explicitly default all
			 * dialing to pulse or tone, but doing so prevents
			 * the use of phone numbers that you may have stored
			 * in the modem's memory.
			 */
    {
    "Digital DF-200",
    "",			/* pulse command */
    "",			/* tone command */
    30,			/* dial_time */
    "=W",		/* pause_chars */	/* =: second tone; W: 5 secs */
    15,			/* pause_time */	/* worst case */
    "\002",		/* wake_str */		/* allow stored number usage */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
#ifdef COMMENT
    "%s!",		/* dial_str */
#else
    "   d %s\\{13}",
#endif /* COMMENT */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF GDC =		/* information for "GeneralDataComm 212A/ED" modem */
    {
    "GeneralDataComm 212A/ED",
    "",					/* pulse command */
    "",					/* tone command */
    32,					/* dial_time */
    "%",				/* pause_chars */
    3,					/* pause_time */
    "\\{13}\\{13}",				/* wake_str */
    500,				/* wake_rate */
    "$",				/* wake_prompt */
    "D\\{13}",				/* dmode_str */
    ":",				/* dmode_prompt */
    "T%s\\{13}",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        1200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF PENRIL =		/* information for "Penril" modem */
    {
    "Penril modem",
    "",					/* pulse command */
    "",					/* tone command */
    50,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "\\{13}\\{13}",				/* wake_str */
    300,				/* wake_rate */
    ">",				/* wake_prompt */
    "k\\{13}",				/* dmode_str */
    ":",				/* dmode_prompt */
    "%s\\{13}",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF RACAL =				/* Racal Vadic VA4492E */
    {
    "Racal Vadic VA4492E",
    "",					/* pulse command */
    "",					/* tone command */
    35,			/* dial_time (manual says modem is hardwired to 60) */
    "Kk",				/* pause_chars */
    5,					/* pause_time */
    "\005\\{13}",				/* wake_str, ^E^M */
    50,					/* wake_rate */
    "*",				/* wake_prompt */
    "D\\{13}",				/* dmode_str */
    "?",				/* dmode_prompt */
    "%s\\{13}",				/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    5,					/* esc_char, ^E */
    "\003\004",				/* hup_str, ^C^D */
    0,					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF VENTEL =				/* Information for Ven-Tel modem */
    {
    "Ven-Tel",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "%",				/* pause_chars */
    5,					/* pause_time */
    "\\{13}\\{13}\\{13}",			/* wake_str */
    300,				/* wake_rate */
    "$",				/* wake_prompt */
    "K\\{13}",				/* dmode_str (was "") */
    "Number to call: ",			/* dmode_prompt (was NULL) */
    "%s\\{13}",				/* dial_str (was "<K%s\r>") */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF CONCORD =	/* Info for Condor CDS 220 2400b modem */
    {
    "Concord Condor CDS 220 2400b",
    "",					/* pulse command */
    "",					/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "\\{13}\\{13}",				/* wake_str */
    20,					/* wake_rate */
    "CDS >",				/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "<D M%s\\{13}>",			/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_off_str */
    "",					/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
        "",			/* ignoredt */
        "",			/* last minute init string */
        2400L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};
#endif /* OLDMODEMS */

static
MDMINF MICROCOM =	/* Microcom modems in native SX mode */
			/* (long answer only) */
{
    "Microcom MNP modems in SX command mode",
    "DP\\{13}",				/* pulse command */
    "DT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",!@",		/* pause_chars (! and @ aren't pure pauses) */
    3,					/* pause_time */
/*
  The following sets 8 bits, no parity, BREAK passthru, and SE0 disables the
  escape character, which is a single character with no guard time, totally
  unsafe, so we have no choice but to disable it.  Especially since, by
  default, it is Ctrl-A, which is Kermit's packet-start character.  We would
  change it to something else, which would enable "mdmhup()", but the user
  wouldn't know about it.  Very bad.  Note: SE1 sets it to Ctrl-A, SE2
  sets it to Ctrl-B, etc (1..31 allowed).  Also SE/Q sets it to "Q".
*/
    "SE0;S1P4;SBRK5\\{13}",		/* wake_str */
    100,				/* wake_rate */
    "!",				/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "D%s\\{13}",				/* dial_str - number up to 39 chars */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char - we can't use this */
    "",					/* hup_str - it's "H" but can't use */
    "SF13\\{13}",				/* hwfc_str */
    "SF11\\{13}",				/* swfc_str */
    "SF10\\{13}",				/* nofc_str */
    "BAOFF;SMAUT\\{13}",			/* ec_on_str */
    "BAON;SMDIR\\{13}",			/* ec_off_str */
    "COMP1\\{13}",			/* dc_on_str */
    "COMP0\\{13}",			/* dc_off_str */
    "AA",				/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "SA2",				/* sp_off_str */
    "SA0",				/* sp_on_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",         			/* ignoredt */
    "",			/* last minute init string */
    0L,					/* max_speed */
    CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_KS, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MICROLINK =			/* MicroLink ... */
    {					/* 14.4TQ,TL,PC;28.8TQ,TQV;2440T/TR */
    "ELSA MicroLink 14.4 28.8, 33.6, or 56K", 	/* ELSA GmbH, Aachen */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0\\\\D0&C1&D2\\\\K5\\{13}",	/* wake_str */
#else
    "ATQ0X4\\\\K5\\{13}",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\\{13}",			/* hup_str */
    "AT\\\\Q3\\{13}",			/* hwfc_str */
    "AT\\\\Q1\\\\X0\\{13}",			/* swfc_str */
    "AT\\\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N3\\{13}",			/* ec_on_str */
    "AT\\\\N0\\{13}",			/* ec_off_str */
    "AT%C3\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "\\J0",				/* sb_on_str (?) */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ULINKV250 =			/* MicroLink V.250 */
    {					/* 56Kflex, V.90; V.250 command set */
    "ELSA MicroLink 56K V.250",		/* ELSA GmbH, Aachen */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    /* \\D0 = DSR & CTS always on but hwfc overrides on CTS. */
    "ATQ0E1V1X4&S0\\\\D0&C1&D2\\{13}",	/* wake_str */
#else
#ifdef VMS
    "ATQ0X4&S1\\{13}",			/* wake_str */
#else
    "ATQ0X4\\{13}",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT+IFC=2,2\\{13}",			/* hwfc_str */
    "AT+IFC=1,1\\{13}",			/* swfc_str */
    "AT+IFC=0,0\\{13}",			/* nofc_str */
    "AT+ES=3,0\\{13}",			/* ec_on_str */
    "AT+ES=1,0\\{13}",			/* ec_off_str */
    "AT+DS=3,0,2048,32\\{13}",		/* dc_on_str */
    "AT+DS=0,0\\{13}",			/* dc_off_str */

    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str (?) */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};
#endif /* MINIDIAL */

static
MDMINF ITUTV250 =			/* ITU-T V.250 conforming modem */
{
    "Any ITU-T V.25ter/V.250 conformant modem",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\{13}",		/* wake_str */
#else
#ifdef VMS
    "ATQ0X4&S1\\{13}",			/* wake_str */
#else
    "ATQ0X4\\{13}",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT+IFC=2,2\\{13}",			/* hwfc_str */
    "AT+IFC=1,1\\{13}",			/* swfc_str */
    "AT+IFC=0,0\\{13}",			/* nofc_str */
    "AT+ES=3,0,2;+EB=1,0,30\\{13}",	/* ec_on_str */
    "AT+ES=0\\{13}",			/* ec_off_str */
    "AT+DS=3,0\\{13}",			/* dc_on_str */
    "AT+DS=0,0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",				/* ignoredt */
    "",					/* ini2 */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

#ifndef CK_TAPI
static
#endif /* CK_TAPI */
MDMINF GENERIC =			/* Generic high speed ... */
    {
    "Generic high-speed AT command set",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "AT&F\\{13}",				/* wake_str */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\\{13}",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok				/* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF XJACK =				/* Megahertz X-Jack */
    {
    "Megahertz X-Jack XJ3144 / CC6144",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4N1&C1&D2\\\\K5\\{13}",	/* wake_str */
#else
    "ATQ0X4N1\\\\K5\\{13}",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT\\\\N3&Q5\\{13}",			/* ec_on_str */
    "AT\\\\N1&Q0\\{13}",			/* ec_off_str */
    "AT%C3\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SPIRITII =			/* QuickComm Spirit II */
    {
    "QuickComm Spirit II",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "AT&F\\{13}",				/* wake_str */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\\{13}",			/* hup_str */
    "AT*F3\\{13}",			/* hwfc_str */
    "AT*F2\\{13}",			/* swfc_str */
    "AT*F0\\{13}",			/* nofc_str */
    "AT*E6\\{13}",			/* ec_on_str */
    "AT*E0\\{13}",			/* ec_off_str */
    "AT*E9\\{13}",			/* dc_on_str */
    "AT*E0\\{13}",			/* dc_off_str */
    "ATS0=2\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MONTANA = {			/* Motorola Montana */
    "Motorola Montana",			/* Name */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\\\K5\\\\V1\\{13}",	/* wake_str */
#else
    "ATQ0E1X4\\\\K5\\\\V1\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT\\\\Q3\\{13}",			/* hwfc_str */
    "AT\\\\Q1\\{13}",			/* swfc_str */
    "AT\\\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N4\\{13}",			/* ec_on_str */
    "AT\\\\N1\\{13}",			/* ec_off_str */
    "AT%C1\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT\\\\J0\\{13}",			/* sb_on_str */
    "AT\\\\J1\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",			/* ignoredt */
    "",			/* last minute init string */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF COMPAQ = {			/* Compaq Data+Fax Modem */
    "Compaq Data+Fax Modem",		/* Name */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\{13}",		/* wake_str */
#else
    "ATQ0E1X4\\{13}",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT\\\\Q3\\{13}",			/* hwfc_str */
    "AT\\\\Q1\\{13}",			/* swfc_str */
    "AT\\\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N3\\{13}",			/* ec_on_str */
    "AT\\\\N0\\{13}",			/* ec_off_str */
    "AT%C1\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT\\\\N3\\{13}",			/* sb_on_str */
    "AT\\\\N1\\{13}",   		/* sb_off_str */
    "ATM1\\{13}",			/* sp_on_str */
    "ATM0\\{13}",			/* sp_off_str */
    "ATL1\\{13}",			/* vol1_str */
    "ATL2\\{13}",			/* vol2_str */
    "ATL3\\{13}",			/* vol3_str */
    "ATX3\\{13}",			/* ignoredt */
    "",	                		/* last minute init string */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};


static
MDMINF FUJITSU = {			/* Fujitsu */
    "Fujitsu Fax/Modem Adapter",	/* Name */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\\\K5\\\\N3\\{13}",	/* wake_str */
#else
    "ATQ0E1X4\\\\K5\\\\N3\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\\\Q3\\{13}",			/* hwfc_str */
    "AT&K4\\\\Q1\\{13}",			/* swfc_str */
    "AT&K0\\\\Q0\\{13}",			/* nofc_str */
    "AT\\\\N3\\{13}",			/* ec_on_str */
    "AT\\\\N0\\{13}",			/* ec_off_str */
    "AT%C1",				/* dc_on_str */
    "AT%C0",				/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT\\\\J0\\{13}",			/* sb_on_str */
    "AT\\\\J1\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",			/* ignoredt */
    "",			/* last minute init string */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MHZATT =				/* Megahertz AT&T V.34 */
    {
    "Megahertz AT&T V.34",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4N1&C1&D2\\\\K5\\{13}",	/* wake_str */
#else
    "ATQ0X4N1\\\\K5\\{13}",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT\\\\N3\\{13}",			/* ec_on_str */
    "AT\\\\N0\\{13}",			/* ec_off_str */
    "AT%C1\"H3\\{13}",			/* dc_on_str */
    "AT%C0\"H0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT\\\\J0\\{13}",			/* sb_on_str */
    "AT\\\\J1\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SUPRASON =			/* SupraSonic */
    {
    "Diamond SupraSonic 288V+",		/* Diamond Multimedia Systems Inc */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\\{13}",	/* wake_str */
#else
    "ATQ0E1N1W0X4Y0\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K\\{13}",				/* nofc_str */
    "AT&Q5\\\\N3S48=7\\{13}",		/* ec_on_str */
    "AT&Q0\\\\N1\\{13}",			/* ec_off_str */
    "AT%C3S46=138\\{13}",			/* dc_on_str */
    "AT%C0S46=136\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM\\{13}",				/* sp_off_str */
    "ATL\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF BESTDATA =			/* Best Data */
    {
    "Best Data Fax Modem",		/* Best Data Fax Modem */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\\{13}",	/* wake_str */
#else
    "ATQ0E1N1W0X4Y0\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K\\{13}",				/* nofc_str */
    "AT&Q6\\\\N3\\{13}",			/* ec_on_str */
    "AT&Q0\\\\N1\\{13}",			/* ec_off_str */
    "AT%C3\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT\\\\N3\\{13}",			/* sb_on_str */
    "AT\\\\N0\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ATT1900 =			/* AT&T Secure Data STU III 1900 */
    {
    "AT&T Secure Data STU III Model 1900", /* name */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4\\{13}",	/* wake_str */
#else
    "ATQ0E1X4+Q0\\{13}",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        9600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_HW,		/* capas */
    getok				/* ok_fn */
};

static
MDMINF ATT1910 =			/* AT&T Secure Data STU III 1910 */
    {					/* Adds V.32bis, V.42, V.42bis */
    "AT&T Secure Data STU III Model 1910", /* name */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&C1&S0&D2+Q0S36=1\\{13}",	/* wake_str */
#else
    "ATQ0E1X4+Q0S36=1\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "ATS46=138S47=0\\{13}",		/* ec_on_str */
    "ATS46=138S47=128\\{13}",		/* ec_off_str */
    "ATS46=138S47=0\\{13}",		/* dc_on_str */
    "ATS46=138S47=128\\{13}",		/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
        "ATX3\\{13}",			/* ignoredt */
        "",			/* last minute init string */
        9600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW,	/* capas */
    getok				/* ok_fn */
};


static
MDMINF USRX2 =				/* USR XJ-CC1560 X2 56K */
    {
    "US Robotics / Megahertz CC/XJ-CC1560 X2",
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&A3&S0&B2&C1&D2&N0\\{13}",	/* wake_str */
#else
    "ATQ0X4&A3&B2&N0\\{13}",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&H1&I0\\{13}",			/* hwfc_str */
    "AT&H2&I2\\{13}",			/* swfc_str */
    "AT&H0&I0\\{13}",			/* nofc_str */
    "AT&M4\\{13}",			/* ec_on_str */
    "AT&M0\\{13}",			/* ec_off_str */
    "AT&K1\\{13}",			/* dc_on_str */
    "AT&K0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT&B1\\{13}",			/* sb_on_str */
    "AT&B0\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ROLM_AT =		/* Rolm data phone with AT command set */
    {
    "Rolm 244PC or 600 Series with AT Command Set",
    "",					/* pulse command */
    "",					/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1\\{13}",			/* wake_str */
#else
    "ATQ0\\{13}",				/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATDT%s\\{13}",			/* dial_str -- always Tone */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    "",					/* sp_on_str */
    "",					/* sp_off_str */
    "",					/* vol1_str */
    "",					/* vol2_str */
    "",					/* vol3_str */
    "",					/* ignoredt */
    "",					/* ini2 */
    19200L,				/* max_speed */
    CKD_AT,				/* capas */
    getok				/* ok_fn */
};

static
MDMINF ATLAS =				/* Atlas / Newcom ixfC 33.6 */
    {
    "Atlas / Newcom 33600ixfC Data/Fax Modem", /* Name */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATZ0&FQ0V1&C1&D2\\{13}",		/* wake_str */
#else
    "ATZ0&FQ0V1\\{13}",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT\"H3\\{13}",			/* ec_on_str */
    "AT\"H0\\{13}",			/* ec_off_str */
    "AT%C1\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "ATN0\\\\J0\\{13}",			/* sb_on_str */
    "ATN1\\\\J1\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF CODEX = {			/* Motorola Codex */
    "Motorola Codex 326X Series",	/* Name - AT&V to see settings */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    /* &M0=Async (not sync) */
    /* *MM0=Automatic modulation negotiation */
    /* *DE22=Automatic data rate */
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0&S0&D2\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0&S1\\{13}", /* wake_str */
#else
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0\\{13}",	/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT*MF1*FL3\\{13}",			/* hwfc_str */
    "AT*MF1*FL1\\{13}",			/* swfc_str */
    "AT*MF0*FL0\\{13}",			/* nofc_str */
    "AT*EC0*SM3*SC0\\{13}",		/* ec_on_str */
    "AT*SM0\\{13}",			/* ec_off_str */
    "AT*DC1\\{13}",			/* dc_on_str */
    "AT*DC0\\{13}",			/* dc_off_str */
    "AT*AA5S0=1\\{13}",			/* aa_on_str */
    "AT*AA5S0=0\\{13}",			/* aa_off_str */
    "AT*SC1\\{13}",			/* sb_on_str */
    "AT*SC0\\{13}",			/* sb_off_str */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3*BD2\\{13}",			/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MT5634ZPX =			/* Multitech */
    {
    "Multitech MT5634ZPX",		/* name */
    "ATP\\{13}",				/* pulse command */
    "ATT\\{13}",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2&Q0\\{13}",	/* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1&Q0\\{13}",		/* wake_str */
#else
    "ATQ0E1X4&Q0\\{13}",			/* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\\{13}",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\\{13}",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\\{13}",			/* hup_str */
    "AT&K3\\{13}",			/* hwfc_str */
    "AT&K4\\{13}",			/* swfc_str */
    "AT&K0\\{13}",			/* nofc_str */
    "AT\\\\N3\\{13}",			/* ec_on_str */
    "AT\\\\N1\\{13}",			/* ec_off_str */
    "AT%C1\\{13}",			/* dc_on_str */
    "AT%C0\\{13}",			/* dc_off_str */
    "ATS0=1\\{13}",			/* aa_on_str */
    "ATS0=0\\{13}",			/* aa_off_str */
    "AT\\\\J0\\{13}",			/* sb_on_str */
    "AT\\\\J1\\{13}",			/* sb_off_str (NOT SUPPORTED) */
    "ATM1\\{13}",				/* sp_on_str */
    "ATM0\\{13}",				/* sp_off_str */
    "ATL1\\{13}",				/* vol1_str */
    "ATL2\\{13}",				/* vol2_str */
    "ATL3\\{13}",				/* vol3_str */
    "ATX3\\{13}",				/* ignoredt */
    "",					/* ini2 */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};
#endif /* MINIDIAL */
#else /* K95_1120_MODEMS */
/* BEGIN MDMINF STRUCT DEFINITIONS */

/*
  Declare structures containing modem-specific information.
  REMEMBER that only the first SEVEN characters of these names are
  guaranteed to be unique.

  First declare the three types that are allowed for MINIDIAL versions.
*/
static
MDMINF CCITT =                          /* CCITT / ITU-T V.25bis autodialer */
/*
  According to V.25bis:
  . Even parity is required for giving commands to the modem.
  . Commands might or might not echo.
  . Responses ("Indications") from the modem are terminated by CR and LF.
  . Call setup is accomplished by:
    - DTE raises DTR (V.24 circuit 108)              [ttopen() does this]
    - Modem raises CTS (V.24 circuit 106)            [C-Kermit ignores this]
    - DTE issues a call request command ("CRN")
    - Modem responds with "VAL" ("command accepted")
    - If the call is completed:
        modem responds with "CNX" ("call connected");
        modem turns CTS (106) OFF;
        modem turns DSR (107) ON;
      else:
        modem responds with "CFI <parameter>" ("call failure indication").
  . To clear a call, the DTE turns DTR (108) OFF.
  . There is no mention of the Carrier Detect circuit (109) in the standard.
  . There is no provision for "escaping back" to the modem's command mode.

  It is not known whether there exists in real life a pure V.25bis modem.
  If there is, this code has never been tested on it.  See the Digitel entry.
*/
    {
    "Any CCITT / ITU-T V.25bis conformant modem",
    "",                 /* pulse command */
    "",                 /* tone command */
    40,                 /* dial_time -- programmable -- */
    ",:",               /* pause_chars -- "," waits for programmable time */
                        /* ":" waits for dial tone */
    10,                 /* pause_time (seconds, just a guess) */
    "",                 /* wake_str (none) */
    200,                /* wake_rate (msec) */
    "VAL",              /* wake_prompt */
    "",                 /* dmode_str (none) */
    "",                 /* dmode_prompt (none) */
    "CRN%s\\{13}",        /* dial_str */
    200,                /* dial_rate (msec) */
    0,                  /* No esc_time */
    0,                  /* No esc_char  */
    "",                 /* No hup_str  */
    "",                 /* hwfc_str */
    "",                 /* swfc_str */
    "",                 /* nofc_str */
    "",                 /* ec_on_str */
    "",                 /* ec_off_str */
    "",                 /* dc_on_str */
    "",                 /* dc_off_str */
    "CIC\\{13}",          /* aa_on_str */
    "DIC\\{13}",          /* aa_off_str */
    "",                 /* sb_on_str */
    "",                 /* sb_off_str */
    "",                 /* sp_off_str */
    "",                 /* sp_on_str */
    "",                 /* vol1_str */
    "",                 /* vol2_str */
    "",                 /* vol3_str */
    "",                 /* ignoredt */
    "",                 /* ini2 */
    0L,                 /* max_speed */
    CKD_V25,            /* capas */
    NULL                /* No ok_fn    */
};

static
MDMINF HAYES =                          /* Hayes 2400 and compatible modems */
    {
    "Hayes Smartmodem 2400 and compatibles",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1&S0&C1&D2\\{13}",            /* wake_str */
#else
#ifdef VMS
    "ATQ0&S1\\{13}",                      /* wake_str */
#else
    "ATQ0\\{13}",                         /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str, user supplies D or T */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    2400L,                              /* max_speed */
    CKD_AT,                             /* capas */
    getok                               /* ok_fn */
};

/*
  The intent of the "unknown" modem is to allow KERMIT to support
  unknown modems by having the user type the entire autodial sequence
  (possibly including control characters, etc.) as the "phone number".
  The protocol and other characteristics of this modem are unknown, with
  some "reasonable" values being chosen for some of them.  The only way to
  detect if a connection is made is to look for carrier.
*/
static
MDMINF UNKNOWN =                        /* Information for "Unknown" modem */
    {
    "Unknown",                          /* name */
    "",                                 /* pulse command */
    "",                                 /* tone command */
    30,                                 /* dial_time */
    "",                                 /* pause_chars */
    0,                                  /* pause_time */
    "",                                 /* wake_str */
    0,                                  /* wake_rate */
    "",                                 /* wake_prompt */
    "",                                 /* dmode_str */
    NULL,                               /* dmode_prompt */
    "%s\\{13}",                           /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF ATTISN =                         /* AT&T ISN Network */
    {
    "",                                 /* pulse command */
    "",                                 /* tone command */
    "AT&T ISN Network",
    30,                                 /* Dial time */
    "",                                 /* Pause characters */
    0,                                  /* Pause time */
    "\\{13}\\{13}\\{13}\\{13}",                 /* Wake string */
    900,                                /* Wake rate */
    "DIAL",                             /* Wake prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "%s\\{13}",                           /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF ATTMODEM =       /* information for AT&T switched-network modems */
                        /* "Number" following "dial" can include: p's and
                         * t's to indicate pulse or tone (default) dialing,
                         * + for wait for dial tone, , for pause, r for
                         * last number dialed, and, except for 2224B, some
                         * comma-delimited options like o12=y, before number.

 * "Important" options for the modems:
 *
 *      All:            Except for 2224B, enable option 12 for "transparent
 *                      data," o12=y.  If a computer port used for both
 *                      incoming and outgoing calls is connected to the
 *                      modem, disable "enter interactive mode on carriage
 *                      return," EICR.  The Kermit "dial" command can
 *                      function with EIA leads standard, EIAS.
 *
 *      2212C:          Internal hardware switches at their default
 *                      positions (four rockers down away from numbers)
 *                      unless EICR is not wanted (rocker down at the 4).
 *                      For EIAS, rocker down at the 1.
 *
 *      2224B:          Front-panel switch position 1 must be up (at the 1,
 *                      closed).  Disable EICR with position 2 down.
 *                      For EIAS, position 4 down.
 *                      All switches on the back panel down.
 *
 *      2224CEO:        All front-panel switches down except either 5 or 6.
 *                      Enable interactive flow control with o16=y.
 *                      Select normal asynchronous mode with o34=0 (zero).
 *                      Disable EICR with position 3 up.  For EIAS, 1 up.
 *                      Reset the modem after changing switches.
 *
 *      2296A:          If option 00 (zeros) is present, use o00=0.
 *                      Enable interactive flow control with o16=y.
 *                      Select normal asynchronous mode with o34=0 (zero).
 *                      (available in Microcom Networking version, but
 *                      not necessarily other models of the 2296A).
 *                      Enable modem-port flow control (if available) with
 *                      o42=y.  Enable asynchronous operation with o50=y.
 *                      Disable EICR with o69=n.  For EIAS, o66=n, using
 *                      front panel.
 */
    {
   "AT&T switched-network modems",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    20,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
    "+",                                /* wake_str */
    0,                                  /* wake_rate */
    "",                                 /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "at%s\\{13}",                         /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    CKD_AT,                             /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF ATTDTDM = /* AT&T Digital Terminal Data Module  */
                 /* For dialing: KYBD switch down, others usually up. */
    {
    "AT&T Digital Terminal Data Module",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    20,                                 /* dial_time */
    "",                                 /* pause_chars */
    0,                                  /* pause_time */
    "",                                 /* wake_str */
    0,                                  /* wake_rate */
    "",                                 /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "%s\\{13}",                           /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF DIGITEL =        /* Digitel DT-22 CCITT variant used in Brazil */
/*
  Attempts to adhere strictly to the V.25bis specification do not produce good
  results in real life.  The modem for which this code was developed: (a)
  ignores parity; (b) sometimes terminates responses with LF CR instead of CR
  LF; (c) has a Hayes-like escape sequence; (d) supports a hangup ("HUP")
  command.  Information from Fernando Cabral in Brasilia.
*/
    {
    "Digitel DT-22 CCITT dialer",
    "",                         /* pulse command */
    "",                         /* tone command */
    40,                         /* dial_time -- programmable -- */
    ",:",               /* pause_chars -- "," waits for programmable time */
                        /* ":" waits for dial tone */
    10,                 /* pause_time (seconds, just a guess) */
    "HUP\\{13}",          /* wake_str (Not Standard CCITT) */
    200,                /* wake_rate (msec) */
    "VAL",              /* wake_prompt */
    "",                 /* dmode_str (none) */
    "",                 /* dmode_prompt (none) */
    "CRN%s\\{13}",        /* dial_str */
    200,                /* dial_rate (msec) */
    1100,               /* esc_time (Not Standard CCITT) */
    43,                 /* esc_char  (Not Standard CCITT) */
    "HUP\\{13}",          /* hup_str  (Not Standard CCITT) */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "CIC\\{13}",                          /* aa_on_str */
    "DIC\\{13}",                          /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    CKD_V25,                            /* capas */
    getok                               /* ok_fn */
};

static
MDMINF H_1200 =         /* Hayes 1200 and compatible modems */
    {
    "Hayes Smartmodem 1200 and compatibles",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1\\{13}",                     /* wake_str */
#else
    "ATQ0\\{13}",                         /* wake_str */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    1200L,                              /* max_speed */
    CKD_AT,                             /* capas */
    getok                               /* ok_fn */
};

static
MDMINF H_ULTRA =                        /* Hayes high-speed */
    {
    "Hayes Ultra/Optima/Accura 96/144/288", /* U,O,A */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1X4N1Y0&S0&C1&D2S37=0S82=128\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4N1Y0&S1S37=0S82=128\\{13}",    /* wake_str */
#else
    "ATQ0X4N1Y0S37=0S82=128\\{13}",       /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */   /* OK for U,O */
    "AT&K4\\{13}",                        /* swfc_str */   /* OK for U,O */
    "AT&K0\\{13}",                        /* nofc_str */   /* OK for U,O */
    "AT&Q5S36=7S48=7\\{13}",              /* ec_on_str */  /* OK for U,O */
    "AT&Q0\\{13}",                        /* ec_off_str */ /* OK for U,O */
    "ATS46=2\\{13}",                      /* dc_on_str */
    "ATS46=0\\{13}",                      /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */  /* (varies) */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF H_ACCURA =                       /* Hayes Accura */
    {                                   /* GUESSING IT'S LIKE ULTRA & OPTIMA */
    "Hayes Accura",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1X4N1Y0&S0&C1&D2S37=0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4N1Y0&S1S37=0\\{13}",           /* wake_str */
#else
    "ATQ0X4N1Y0S37=0\\{13}",              /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5S36=7S48=7\\{13}",              /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "ATS46=2\\{13}",                      /* dc_on_str */
    "ATS46=0\\{13}",                      /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */  /* (varies) */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF PPI =                            /* Practical Peripherals  */
    {
    "Practical Peripherals V.22bis or higher with V.42 and V.42bis",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef COMMENT
/* In newer models S82 (BREAK handling) was eliminated, causing an error. */
#ifdef OS2
    "ATQ0X4N1&S0&C1&D2S37=0S82=128\\{13}", /* wake_str */
#else
    "ATQ0X4N1S37=0S82=128\\{13}",         /* wake_str */
#endif /* OS2 */
#else /* So now we use Y0 instead */
#ifdef OS2
    "ATE1Q0V1X4N1&S0&C1&D2Y0S37=0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4N1Y0&S1S37=0\\{13}",           /* wake_str */
#else
    "ATQ0X4N1Y0S37=0\\{13}",              /* wake_str */
#endif /* VMS */
#endif /* OS2 */
#endif /* COMMENT */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5S36=7S48=7\\{13}",              /* ec_on_str */
    "AT&Q0S36=0S48=128\\{13}",            /* ec_off_str */
    "ATS46=2\\{13}",                      /* dc_on_str */
    "ATS46=0\\{13}",                      /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str  */
    "",                                 /* sb_off_str  */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF DATAPORT =                       /* AT&T Dataport  */
    {
    "AT&T / Paradyne DataPort V.32 or higher",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
    /*
       Note: S41=0 (use highest modulation) omitted, since it is not
       supported on the V.32 and lower models.  So let's not touch it.
    */
#ifdef OS2
    "ATQ0E1V1X6&S0&C1&D2&Q0Y0\\\\K5S78=0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X6&S1&Q0Y0\\\\K5S78=0\\{13}",    /* wake_str */
#else
    "ATQ0E1X6&Q0Y0\\\\K5S78=0\\{13}",               /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "AT\\\\Q1\\\\X0\\{13}",                   /* swfc_str */
    "AT\\\\Q0\\{13}",                       /* nofc_str */
    "AT\\\\N7\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF UCOM_AT =                        /* Microcom DeskPorte FAST ES 28.8 */
    {
    "Microcom DeskPorte FAST 28.8",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1X4\\\\N0F0&S0&C1&D2\\\\K5\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4F0&S1\\\\K5\\{13}",              /* wake_str */
#else
    "ATQ0X4F0\\\\K5\\{13}",                 /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "AT\\\\Q1\\{13}",                       /* swfc_str */
    "AT\\\\H0\\\\Q0\\{13}",                   /* nofc_str */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT%C3\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT-J0\\{13}",                        /* sb_on_str */
    "AT-J1\\{13}",                        /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ZOOM =                           /* Zoom Telephonics V.32bis  */
    {
    "Zoom Telephonics V.32bis",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1N1W1X4&S0&C1&D2S82=128S95=47\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1N1W1X4&S1S82=128S95=47\\{13}", /* wake_str */
#else
    "ATQ0E1N1W1X4S82=128S95=47\\{13}",    /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5S36=7S48=7\\{13}",              /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "ATS46=138\\{13}",                    /* dc_on_str */
    "ATS46=136\\{13}",                    /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ZYXEL =                          /* ZyXEL U-Series */
    {
    "ZyXEL U-Series V.32bis or higher",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1&S0&C1&D2&N0X5&Y1\\{13}",    /* wake_str */
#else
#ifdef VMS
    "ATQ0E1&S1&N0X5&Y1\\{13}",            /* wake_str */
#else
    "ATQ0E1&N0X5&Y1\\{13}",               /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&H3\\{13}",                        /* hwfc_str */
    "AT&H4\\{13}",                        /* swfc_str */
    "AT&H0\\{13}",                        /* nofc_str */
    "AT&K3\\{13}",                        /* ec_on_str */
    "AT&K0\\{13}",                        /* ec_off_str */
    "AT&K4\\{13}",                        /* dc_on_str */
    "AT&K3\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ZOLTRIX =                        /* Zoltrix */
    {
    "Zoltrix V.32bis and V.34 modems with Rockwell ACI chipset",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
   "ATE1Q0V1F0W1X4Y0&S0&C1&D2\\\\K5S82=128S95=41\\{13}", /* wake_str */
#else
#ifdef VMS
   "ATQ0E1F0W1X4Y0&S1\\\\K5S82=128S95=41\\{13}", /* wake_str */
#else
   "ATQ0E1F0W1X4Y0\\\\K5S82=128S95=41\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "ATS46=138%C3\\{13}",                 /* dc_on_str */
    "ATS46=136%C0\\{13}",                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\N0\\{13}",                       /* sb_on_str */
    "AT&Q0\\{13}",                        /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MOTOROLA = {                     /* Motorola FasTalk II or Lifestyle */
/*
  "\\E" and "\\X" commands removed - Motorola Lifestyle doesn't have them.
     \\E0 = Don't echo while online
     \\X0 = Process Xon/Xoff but don't pass through
*/
    "Motorola FasTalk II or Lifestyle", /* Name */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2\\\\K5\\\\V1\\{13}",  /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\\\K5\\\\V1\\{13}",          /* wake_str */
#else
    "ATQ0E1X4\\\\K5\\\\V1\\{13}",             /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "AT\\\\Q1\\{13}",                       /* swfc_str */
    "AT\\\\Q0\\{13}",                       /* nofc_str */
    "AT\\\\N6\\{13}",                       /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\J0\\{13}",                       /* sb_on_str */
    "AT\\\\J1\\{13}",                       /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF BOCA =                           /* Boca */
    {
    "BOCA 14.4 Faxmodem",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1F1N1W1&S0&C1&D2\\\\K5S37=11S82=128S95=47X4\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1F1N1W1&S1\\\\K5S37=11S82=128S95=47X4\\{13}", /* wake_str */
#else
    "ATQ0E1F1N1W1\\\\K5S37=11S82=128S95=47X4\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT\\\\N3S36=7S48=7\\{13}",             /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "ATS46=138\\{13}",                    /* dc_on_str */
    "ATS46=136\\{13}",                    /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF INTEL =                          /* Intel */
    {
    "Intel High-Speed Faxmodem",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1Y0X4&S0&C1&D2\\\\K1\\\\V2S25=50\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1Y0X4&S1\\\\K1\\\\V2S25=50\\{13}",  /* wake_str */
#else
    "ATQ0E1Y0X4\\\\K1\\\\V2S25=50\\{13}",     /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "ATB1+FCLASS=0\\{13}",                /* dmode_str */
    "OK\\{13}",                           /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\G1\\\\Q3\\{13}",                   /* hwfc_str */
    "AT\\\\G1\\\\Q1\\\\X0\\{13}",               /* swfc_str */
    "AT\\\\G0\\{13}",                       /* nofc_str */
    "AT\\\\J0\\\\N3\\\"H3\\{13}",               /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MULTITECH =                      /* Multitech */
    {
    "Multitech MT1432 or MT2834 Series",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
/* #P0 (= no parity) is not listed in the manual for newer models */
/* so it has been removed from all three copies of the Multitech wake_str */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2&E8&Q0\\{13}",    /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1&E8&Q0\\{13}",            /* wake_str */
#else
    "ATQ0E1X4&E8&Q0\\{13}",               /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&E4&E7&E8&E11&E13\\{13}",          /* hwfc_str */
    "AT&E5&E6&E8&E11&E13\\{13}",          /* swfc_str */
    "AT&E3&E7&E8&E10&E12\\{13}",          /* nofc_str */
    "AT&E1\\{13}",                        /* ec_on_str */
    "AT&E0\\{13}",                        /* ec_off_str */
    "AT&E15\\{13}",                       /* dc_on_str */
    "AT&E14\\{13}",                       /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT$BA0\\{13}",                       /* sb_on_str (= "baud adjust off") */
    "AT$BA1\\{13}",                       /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF SUPRA =                          /* Supra */
    {
    "SupraFAXModem 144 or 288",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\\\\K5S82=128\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1N1W0X4Y0&S1\\\\K5S82=128\\{13}", /* wake_str */
#else
    "ATQ0E1N1W0X4Y0\\\\K5S82=128\\{13}",    /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\\\N3S48=7\\{13}",               /* ec_on_str */
    "AT&Q0\\\\N1\\{13}",                    /* ec_off_str */
    "AT%C1S46=138\\{13}",                 /* dc_on_str */
    "AT%C0S46=136\\{13}",                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM\\{13}",                          /* sp_off_str */
    "ATL\\{13}",                          /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF SUPRAX =                         /* Supra Express */
    {
    "Diamond Supra Express V.90",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1W0X4&C1&D2&S0\\\\K5\\{13}",    /* wake_str */
#else
#ifdef VMS
    "ATQ0E1W0X4&S1\\\\K5\\{13}",            /* wake_str */
#else
    "ATQ0E1W0X4\\\\K5\\{13}",               /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "AT%C2\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM\\{13}",                          /* sp_off_str */
    "ATL\\{13}",                          /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    230400L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MAXTECH =                        /* MaxTech */
    {
    "MaxTech XM288EA or GVC FAXModem",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2&L0&M0\\\\K5\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4Y0&L0&M0&S1\\\\K5\\{13}",      /* wake_str */
#else
    "ATQ0E1X4Y0&L0&M0\\\\K5\\{13}",         /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "AT\\\\Q1\\\\X0\\{13}",                   /* swfc_str */
    "AT\\\\Q0\\{13}",                       /* nofc_str */
    "AT\\\\N6\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT\\\\N6%C1\\{13}",                    /* dc_on_str */
    "AT\\\\N6%C0\\{13}",                    /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ROLM =           /* IBM / Siemens / Rolm 8000, 9000, 9751 CBX DCM */
    {
    "IBM/Siemens/Rolm CBX Data Communications Module",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    60,                                 /* dial_time */
    "",                                 /* pause_chars */
    0,                                  /* pause_time */
    "\\{13}\\{13}",                         /* wake_str */
    50,                                 /* wake_rate */
    "MODIFY?",                          /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "CALL %s\\{13}",                      /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    19200L,                             /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF USR =                            /* USR Courier and Sportster modems */
    {
    "US Robotics Courier, Sportster, or compatible",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&A3&S0&C1&D2&N0&Y3S14=0\\{13}", /* wake_str */
#else
#ifdef SUNOS4
    "ATQ0X4&A3&S0&N0&Y3S14=0\\{13}",      /* wake_str -- needs &S0 in SunOS */
#else
#ifdef VMS
    "ATQ0X4&A3&S1&N0&Y3S14=0\\{13}",      /* wake_str -- needs &S1 in VMS */
#else
    "ATQ0X4&A3&N0&Y3S14=0\\{13}",         /* wake_str */
#endif /* VMS */
#endif /* SUNOS4 */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&H1&R2&I0\\{13}",                  /* hwfc_str */
    "AT&H2&R1&I2\\{13}",                  /* swfc_str */
    "AT&H0&R1&I0\\{13}",                  /* nofc_str */
    "AT&M4&B1\\{13}",                     /* ec_on_str */
    "AT&M0\\{13}",                        /* ec_off_str */
    "AT&K1\\{13}",                        /* dc_on_str */
    "AT&K0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};


static
MDMINF USRX2 =                          /* USR XJ-CC1560 X2 56K */
    {
    "US Robotics / Megahertz CC/XJ-CC1560 X2",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&A3&S0&B2&C1&D2&N0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4&A3&B2&N0&S1\\{13}",           /* wake_str */
#else
    "ATQ0X4&A3&B2&N0\\{13}",              /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&H1&I0\\{13}",                     /* hwfc_str */
    "AT&H2&I2\\{13}",                     /* swfc_str */
    "AT&H0&I0\\{13}",                     /* nofc_str */
    "AT&M4\\{13}",                        /* ec_on_str */
    "AT&M0\\{13}",                        /* ec_off_str */
    "AT&K1\\{13}",                        /* dc_on_str */
    "AT&K0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT&B1\\{13}",                        /* sb_on_str */
    "AT&B0\\{13}",                        /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF OLDTB =                          /* Old Telebits */
    {
    "Telebit TrailBlazer, T1000, T1500, T2000, T2500",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    60,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "\\021AAAAATQ0E1V1X1&S0&C1&D2S12=50S50=0S54=3\\{13}", /* wake_str. */
#else
#ifdef VMS
    "\\021AAAAATQ0X1S12=50S50=0S54=3\\{13}", /* wake_str. */
#else
    "\\021AAAAATQ0X1&S1S12=50S50=0S54=3\\{13}", /* wake_str. */
#endif /* VMS */
#endif /* OS2 */
    100,                                /* wake_rate = 100 msec */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str, Note: no T or P */
    80,                                 /* dial_rate */
    1100,                               /* esc_time (guard time) */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "ATS58=2S68=2\\{13}",                 /* hwfc_str */
    "ATS58=3S68=3S69=0\\{13}",            /* swfc_str */
    "ATS58=0S68=0\\{13}",                 /* nofc_str */
    "ATS66=1S95=2\\{13}",                 /* ec_on_str */
    "ATS95=0\\{13}",                      /* ec_off_str */
    "ATS110=1S96=1\\{13}",                /* dc_on_str */
    "ATS110=0S96=0\\{13}",                /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    19200L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB|CKD_KS, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF NEWTB =                          /* New Telebits */
    {
    "Telebit T1600, T3000, QBlazer, WorldBlazer, etc.",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    60,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "\\021AAAAATQ0E1V1X2&S0&C1&D2S12=50S50=0S61=0S63=0\\{13}", /* wake_str. */
#else
#ifdef VMS
    "\\021AAAAATQ0X2&S1S12=50S50=0S61=0S63=0\\{13}", /* wake_str. */
#else
    "\\021AAAAATQ0X2S12=50S50=0S61=0S63=0\\{13}", /* wake_str. */
#endif /* VMS */
#endif /* OS2 */
    100,                                /* wake_rate = 100 msec */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str, Note: no T or P */
    80,                                 /* dial_rate */
    1100,                               /* esc_time (guard time) */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "ATS58=2S68=2\\{13}",                 /* hwfc_str */
    "ATS58=3S68=3\\{13}",                 /* swfc_str */
    "ATS58=0S68=0\\{13}",                 /* nofc_str */
    "ATS180=3\\{13}",                     /* ec_on_str */
    "ATS180=0\\{13}",                     /* ec_off_str */
    "ATS190=1\\{13}",                     /* dc_on_str */
    "ATS190=0\\{13}",                     /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    38400L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB|CKD_KS, /* capas */
    getok                               /* ok_fn */
};
#endif /* MINIDIAL */

static
MDMINF DUMMY = /* dummy information for modems that are handled elsewhere */
    {
    "(dummy)",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    30,                                 /* dial_time */
    "",                                 /* pause_chars */
    0,                                  /* pause_time */
    "",                                 /* wake_str */
    0,                                  /* wake_rate */
    "",                                 /* wake_prompt */
    "",                                 /* dmode_str */
    NULL,                               /* dmode_prompt */
    "%s\\{13}",                           /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF RWV32 =                          /* Generic Rockwell V.32 */
    {
    "Generic Rockwell V.32 modem",      /* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4Y0&S1%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#else
    "ATQ0X4Y0%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q6\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF RWV32B =                         /* Generic Rockwell V.32bis */
    {
    "Generic Rockwell V.32bis modem",   /* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4Y0&S1%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#else
    "ATQ0X4Y0%E2\\\\K5+FCLASS=0N1S37=0\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "ATS%C1\\{13}",                       /* dc_on_str */
    "ATS%C0\\{13}",                       /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF RWV34 =                          /* Generic Rockwell V.34 Data/Fax */
    {
    "Generic Rockwell V.34 modem",      /* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1X4Y0&C1&D2&S0%E2\\\\K5+FCLASS=0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0V1X4Y0&C1&D2&S1%E2\\\\K5+FCLASS=0\\{13}", /* wake_str */
#else
    "ATQ0V1X4Y0&C1&D2%E2\\\\K5+FCLASS=0\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "ATS%C3\\{13}",                       /* dc_on_str */
    "ATS%C0\\{13}",                       /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF RWV90 =                          /* Generic Rockwell V.90 Data/Fax */
    {
    "Generic Rockwell V.90 56K modem",  /* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\\\K5+FCLASS=0S37=0\\{13}", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\\\K5+FCLASS=0S37=0\\{13}", /* wake_str */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\\\K5+FCLASS=0S37=0\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "AT%C3\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MWAVE =                          /* IBM Mwave */
    {
    "IBM Mwave Adapter",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4Y0&S0&C1&D2&M0&Q0&N1\\\\K3\\\\T0%E2S28=0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4Y0&M0&S1&Q0&N1&S0\\\\K3\\\\T0%E2S28=0\\{13}", /* wake_str */
#else
    "ATQ0X4Y0&M0&Q0&N1&S0\\\\K3\\\\T0%E2S28=0\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "",                                 /* swfc_str (it doesn't!) */
    "AT\\\\Q0\\{13}",                       /* nofc_str */
    "AT\\\\N7\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT%C1\\\"H3\\{13}",                    /* dc_on_str */
    "AT%C0\\\"H0\\{13}",                    /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF TELEPATH =                       /* Gateway 2000 Telepath */
    {
    "Gateway 2000 Telepath II 28.8",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2&N0&Y2#CLS=0S13=0S15=0S19=0\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4&N0&S1&Y1#CLS=0S13=0S15=0S19=0\\{13}", /* wake_str */
#else
    "ATQ0X4&N0&Y1#CLS=0S13=0S15=0S19=0\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&H1&R2\\{13}",                     /* hwfc_str */
    "AT&H2&I2S22=17S23=19\\{13}",         /* swfc_str */
    "AT&H0&I0&R1\\{13}",                  /* nofc_str */
    "AT&M4&B1\\{13}",                     /* ec_on_str -- also fixes speed */
    "AT&M0\\{13}",                        /* ec_off_str */
    "AT&K1\\{13}",                        /* dc_on_str */
    "AT&K0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF CARDINAL =                       /* Cardinal - based on Rockwell V.34 */
    {
    "Cardinal MVP288X Series",          /* ATI3, ATI4, and ATI6 for details */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4W1Y0%E2&S0&C1&D2\\\\K5+FCLASS=0+MS=11,1\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0X4W1Y0&S1%E2\\\\K5+FCLASS=0+MS=11,1\\{13}", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\\\K5+FCLASS=0+MS=11,1\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5S36=7S48=7\\\\N3\\{13}",          /* ec_on_str */
    "AT&Q0S48=128\\\\N1\\{13}",             /* ec_off_str */
    "ATS46=138%C1\\{13}",                 /* dc_on_str */
    "ATS46=136%C0\\{13}",                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF LUCENT =                         /* Lucent Venus or Data/Fax modem */
    {
    "Lucent Venus chipset",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\\\K5+FCLASS=0S37=0\\{13}", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\\\K5+FCLASS=0S37=0\\{13}", /* VMS */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\\\K5+FCLASS=0S37=0\\{13}", /* All others */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF CONEXANT =                       /* Conexant family */
    {
    "Conexant family of modems",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1X4&C1&D2&S0%E1+FCLASS=0\\{13}", /* K95 */
#else
#ifdef VMS
    "ATQ0V1X4&C1&D2&S1%E1+FCLASS=0\\{13}", /* VMS */
#else
    "ATQ0V1X4&C1&D2%E1+FCLASS=0\\{13}", /* UNIX etc */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "AT%C3\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF PCTEL =                          /* PCTel chipset */
    {
    "PCTel chipset",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\\\K5S37=0\\{13}", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\\\K5S37=0\\{13}", /* VMS */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\\\K5S37=0\\{13}", /* UNIX etc */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ZOOMV34 =                        /* Zoom Telephonics V.34  */
    {
    "Zoom Telephonics V.34",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1N1W1X4&S0&C1&D2S82=128\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATQ0V1N1W1X4&S1S82=128\\{13}",       /* wake_str */
#else
    "ATQ0V1N1W1X4S82=128S{13}",          /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}S32=17S33=19",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "ATS%C3\\{13}",                       /* dc_on_str */
    "ATS%C0\\{13}",                       /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ZOOMV90 =                        /* ZOOM V.90 */
    {
    "Zoom V.90 56K",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\\\K5+FCLASS=0S37=0\\{13}", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\\\K5+FCLASS=0S37=0\\{13}", /* VMS */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\\\K5+FCLASS=0S37=0\\{13}", /* All others */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ZOOMV92 =                        /* ZOOM V.92 */
    {
    "Zoom V.92 with V.44 compression",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1N1X4Y0&C1&D2&S0%E2\\\\K5+FCLASS=0S37=0+MS=V92\\{13}", /* K95 */
#else
#ifdef VMS
    "ATQ0V1N1X4Y0&C1&D2&S1%E2\\\\K5+FCLASS=0S37=0+MS=V92\\{13}", /* VMS */
#else
    "ATQ0V1N1X4Y0&C1&D2%E2\\\\K5+FCLASS=0S37=0+MS=V92\\{13}", /* All others */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4S32=17S33=19\\{13}",            /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT&Q5\\{13}",                        /* ec_on_str */
    "AT&Q0\\{13}",                        /* ec_off_str */
    "AT%C1+DCS=1,1\\{13}",                /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};


/*
  Now the "old" modems, all grouped together, and also within
  "if not defined MINIDIAL"...
*/
#ifdef OLDMODEMS

static
MDMINF CERMETEK =       /* Information for "Cermetek Info-Mate 212 A" modem */
    {
    "Cermetek Info-Mate 212 A",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    20,                                 /* dial_time */
    "BbPpTt",                           /* pause_chars */
    0,                                  /* pause_time */
    "  XY\\016R\\{13}",                    /* wake_str */
    200,                                /* wake_rate */
    "",                                 /* wake_prompt */
    "",                                 /* dmode_str */
    NULL,                               /* dmode_prompt */
    "\\016D '%s'\\{13}",                   /* dial_str */
    200,                                /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    1200L,                              /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF DF03 =           /* information for "DEC DF03-AC" modem */
    {
    "Digital DF03-AC",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    27,                                 /* dial_time */
    "=",                                /* pause_chars */
    15,                                 /* pause_time */
    "\\001\\002",                         /* wake_str */
    0,                                  /* wake_rate */
    "",                                 /* wake_prompt */
    "",                                 /* dmode_str */
    NULL,                               /* dmode_prompt */
    "%s",                               /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF DF100 =          /* information for "DEC DF100-series" modem */
                        /*
                         * The telephone "number" can include "P"s and/or "T"s
                         * within it to indicate that subsequent digits are
                         * to be dialed using pulse or tone dialing.  The
                         * modem defaults to pulse dialing.  You may modify
                         * the dial string below to explicitly default all
                         * dialing to pulse or tone, but doing so prevents
                         * the use of phone numbers that you may have stored
                         * in the modem's memory.
                         */
    {
    "Digital DF-100",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    30,                                 /* dial_time */
    "=",                                /* pause_chars */
    15,                                 /* pause_time */
    "\\001",                             /* wake_str */
    0,                                  /* wake_rate */
    "",                                 /* wake_prompt */
    "",                                 /* dmode_str */
    NULL,                               /* dmode_prompt */
    "%s#",                              /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF DF200 =          /* information for "DEC DF200-series" modem */
                        /*
                         * The telephone "number" can include "P"s and/or "T"s
                         * within it to indicate that subsequent digits are
                         * to be dialed using pulse or tone dialing.  The
                         * modem defaults to pulse dialing.  You may modify
                         * the dial string below to explicitly default all
                         * dialing to pulse or tone, but doing so prevents
                         * the use of phone numbers that you may have stored
                         * in the modem's memory.
                         */
    {
    "Digital DF-200",
    "",                 /* pulse command */
    "",                 /* tone command */
    30,                 /* dial_time */
    "=W",               /* pause_chars */       /* =: second tone; W: 5 secs */
    15,                 /* pause_time */        /* worst case */
    "\\002",             /* wake_str */          /* allow stored number usage */
    0,                  /* wake_rate */
    "",                 /* wake_prompt */
    "",                 /* dmode_str */
    NULL,               /* dmode_prompt */
#ifdef COMMENT
    "%s!",              /* dial_str */
#else
    "   d %s\\{13}",
#endif /* COMMENT */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF GDC =            /* information for "GeneralDataComm 212A/ED" modem */
    {
    "GeneralDataComm 212A/ED",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    32,                                 /* dial_time */
    "%",                                /* pause_chars */
    3,                                  /* pause_time */
    "\\{13}\\{13}",                         /* wake_str */
    500,                                /* wake_rate */
    "$",                                /* wake_prompt */
    "D\\{13}",                            /* dmode_str */
    ":",                                /* dmode_prompt */
    "T%s\\{13}",                          /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    1200L,                              /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF PENRIL =         /* information for "Penril" modem */
    {
    "Penril modem",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    50,                                 /* dial_time */
    "",                                 /* pause_chars */
    0,                                  /* pause_time */
    "\\{13}\\{13}",                         /* wake_str */
    300,                                /* wake_rate */
    ">",                                /* wake_prompt */
    "k\\{13}",                            /* dmode_str */
    ":",                                /* dmode_prompt */
    "%s\\{13}",                           /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF RACAL =                          /* Racal Vadic VA4492E */
    {
    "Racal Vadic VA4492E",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    35,                 /* dial_time (manual says modem is hardwired to 60) */
    "Kk",                               /* pause_chars */
    5,                                  /* pause_time */
    "\\005\\{13}",                         /* wake_str, ^E^M */
    50,                                 /* wake_rate */
    "*",                                /* wake_prompt */
    "D\\{13}",                            /* dmode_str */
    "?",                                /* dmode_prompt */
    "%s\\{13}",                           /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    5,                                  /* esc_char, ^E */
    "\\003\\004",                         /* hup_str, ^C^D */
    0,                                  /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF VENTEL =                         /* Information for Ven-Tel modem */
    {
    "Ven-Tel",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    20,                                 /* dial_time */
    "%",                                /* pause_chars */
    5,                                  /* pause_time */
    "\\{13}\\{13}\\{13}",                     /* wake_str */
    300,                                /* wake_rate */
    "$",                                /* wake_prompt */
    "K\\{13}",                            /* dmode_str (was "") */
    "Number to call: ",                 /* dmode_prompt (was NULL) */
    "%s\\{13}",                           /* dial_str (was "<K%s\\r>") */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};

static
MDMINF CONCORD =        /* Info for Condor CDS 220 2400b modem */
    {
    "Concord Condor CDS 220 2400b",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
    "\\{13}\\{13}",                         /* wake_str */
    20,                                 /* wake_rate */
    "CDS >",                            /* wake_prompt */
    "",                                 /* dmode_str */
    NULL,                               /* dmode_prompt */
    "<D M%s\\{13}>",                      /* dial_str */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char */
    "",                                 /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "",                                 /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    2400L,                              /* max_speed */
    0,                                  /* capas */
    NULL                                /* ok_fn */
};
#endif /* OLDMODEMS */

static
MDMINF MICROCOM =       /* Microcom modems in native SX mode */
                        /* (long answer only) */
{
    "Microcom MNP modems in SX command mode",
    "DP\\{13}",                           /* pulse command */
    "DT\\{13}",                           /* tone command */
    35,                                 /* dial_time */
    ",!@",              /* pause_chars (! and @ aren't pure pauses) */
    3,                                  /* pause_time */
/*
  The following sets 8 bits, no parity, BREAK passthru, and SE0 disables the
  escape character, which is a single character with no guard time, totally
  unsafe, so we have no choice but to disable it.  Especially since, by
  default, it is Ctrl-A, which is Kermit's packet-start character.  We would
  change it to something else, which would enable "mdmhup()", but the user
  wouldn't know about it.  Very bad.  Note: SE1 sets it to Ctrl-A, SE2
  sets it to Ctrl-B, etc (1..31 allowed).  Also SE/Q sets it to "Q".
*/
    "SE0;S1P4;SBRK5\\{13}",               /* wake_str */
    100,                                /* wake_rate */
    "!",                                /* wake_prompt */
    "",                                 /* dmode_str */
    NULL,                               /* dmode_prompt */
    "D%s\\{13}",                          /* dial_str - number up to 39 chars */
    0,                                  /* dial_rate */
    0,                                  /* esc_time */
    0,                                  /* esc_char - we can't use this */
    "",                                 /* hup_str - it's "H" but can't use */
    "SF13\\{13}",                         /* hwfc_str */
    "SF11\\{13}",                         /* swfc_str */
    "SF10\\{13}",                         /* nofc_str */
    "BAOFF;SMAUT\\{13}",                  /* ec_on_str */
    "BAON;SMDIR\\{13}",                   /* ec_off_str */
    "COMP1\\{13}",                        /* dc_on_str */
    "COMP0\\{13}",                        /* dc_off_str */
    "AA",                               /* aa_on_str */
    "",                                 /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "SA2",                              /* sp_off_str */
    "SA0",                              /* sp_on_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    0L,                                 /* max_speed */
    CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_KS, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MICROLINK =                      /* MicroLink ... */
    {                                   /* 14.4TQ,TL,PC;28.8TQ,TQV;2440T/TR */
    "ELSA MicroLink 14.4, 28.8, 33.6 or 56K", /* ELSA GmbH, Aachen */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0\\\\D0&C1&D2\\\\K5\\{13}",  /* wake_str */
#else
#ifdef VMS
    "ATQ0X4&S1\\\\K5\\{13}",                /* wake_str */
#else
    "ATQ0X4\\\\K5\\{13}",                   /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H\\{13}",                        /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "AT\\\\Q1\\\\X0\\{13}",                   /* swfc_str */
    "AT\\\\Q0\\{13}",                       /* nofc_str */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT%C3\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "\\\\J0",                             /* sb_on_str (?) */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ULINKV250 =                      /* MicroLink V.250 */
    {                                   /* 56Kflex, V.90; V.250 command set */
    "ELSA MicroLink 56K V.250",         /* ELSA GmbH, Aachen */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    /* \\D0 = DSR & CTS always on but hwfc overrides on CTS. */
    "ATQ0E1V1X4&S0\\\\D0&C1&D2\\{13}",      /* wake_str */
#else
#ifdef VMS
    "ATQ0X4&S1\\{13}",                    /* wake_str */
#else
    "ATQ0X4\\{13}",                       /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT+IFC=2,2\\{13}",                   /* hwfc_str */
    "AT+IFC=1,1\\{13}",                   /* swfc_str */
    "AT+IFC=0,0\\{13}",                   /* nofc_str */
    "AT+ES=3,0\\{13}",                    /* ec_on_str */
    "AT+ES=1,0\\{13}",                    /* ec_off_str */
    "AT+DS=3,0,2048,32\\{13}",            /* dc_on_str */
    "AT+DS=0,0\\{13}",                    /* dc_off_str */

    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str (?) */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};
#endif /* MINIDIAL */

static
MDMINF ITUTV250 =                       /* ITU-T V.250 conforming modem */
{
    "Any ITU-T V.25ter/V.250 conformant modem",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
    "ATQ0E1V1X4&C1&D2\\{13}",             /* wake_str (no &Sn in V.25) */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT+IFC=2,2\\{13}",                   /* hwfc_str */
    "AT+IFC=1,1\\{13}",                   /* swfc_str */
    "AT+IFC=0,0\\{13}",                   /* nofc_str */
    "AT+ES=3,0,2;+EB=1,0,30\\{13}",       /* ec_on_str */
    "AT+ES=0\\{13}",                      /* ec_off_str */
    "AT+DS=3,0\\{13}",                    /* dc_on_str */
    "AT+DS=0,0\\{13}",                    /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

#ifndef CK_TAPI
static
#endif /* CK_TAPI */
MDMINF GENERIC =                        /* Generic high speed ... */
    {
    "Generic high-speed AT command set",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
    "",                                 /* wake_str */
    0,                                  /* wake_rate */
    "",                                 /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* sp_off_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok                               /* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF XJACK =                          /* Megahertz X-Jack */
    {
    "Megahertz X-Jack XJ3144 / CC6144",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4N1&C1&D2\\\\K5\\{13}",       /* wake_str */
#else
    "ATQ0X4N1\\\\K5\\{13}",                 /* wake_str */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H\\{13}",                        /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT\\\\N3&Q5\\{13}",                    /* ec_on_str */
    "AT\\\\N1&Q0\\{13}",                    /* ec_off_str */
    "AT%C3\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF SPIRITII =                       /* QuickComm Spirit II */
    {
    "QuickComm Spirit II",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
    "AT&F\\{13}",                         /* wake_str */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H\\{13}",                        /* hup_str */
    "AT*F3\\{13}",                        /* hwfc_str */
    "AT*F2\\{13}",                        /* swfc_str */
    "AT*F0\\{13}",                        /* nofc_str */
    "AT*E6\\{13}",                        /* ec_on_str */
    "AT*E0\\{13}",                        /* ec_off_str */
    "AT*E9\\{13}",                        /* dc_on_str */
    "AT*E0\\{13}",                        /* dc_off_str */
    "ATS0=2\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MONTANA = {                      /* Motorola Montana */
    "Motorola Montana",                 /* Name */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\\\K5\\\\V1\\{13}",  /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\\\K5\\\\V1\\{13}",          /* wake_str */
#else
    "ATQ0E1X4\\\\K5\\\\V1\\{13}",             /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "AT\\\\Q1\\{13}",                       /* swfc_str */
    "AT\\\\Q0\\{13}",                       /* nofc_str */
    "AT\\\\N4\\{13}",                       /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\J0\\{13}",                       /* sb_on_str */
    "AT\\\\J1\\{13}",                       /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF COMPAQ = {                       /* Compaq Data+Fax Modem */
    "Compaq Data+Fax Modem",            /* Name */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\{13}",          /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\{13}",                  /* wake_str */
#else
    "ATQ0E1X4\\{13}",                     /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str (same as &K3) */
    "AT\\\\Q1\\{13}",                       /* swfc_str (same as &K4) */
    "AT\\\\Q0\\{13}",                       /* nofc_str (same as &K0) */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\N3\\{13}",                       /* sb_on_str */
    "AT\\\\N1\\{13}",                       /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL0\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};


static
MDMINF FUJITSU = {                      /* Fujitsu */
    "Fujitsu Fax/Modem Adapter",        /* Name */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4&S0&C1&D2\\\\K5\\\\N3\\{13}",  /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\\\K5\\\\N3\\{13}",          /* wake_str */
#else
    "ATQ0E1X4\\\\K5\\\\N3\\{13}",             /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\\\Q3\\{13}",                    /* hwfc_str */
    "AT&K4\\\\Q1\\{13}",                    /* swfc_str */
    "AT&K0\\\\Q0\\{13}",                    /* nofc_str */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT%C1",                            /* dc_on_str */
    "AT%C0",                            /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\J0\\{13}",                       /* sb_on_str */
    "AT\\\\J1\\{13}",                       /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MHZATT =                         /* Megahertz AT&T V.34 */
    {
    "Megahertz AT&T V.34",
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4N1&C1&D2\\\\K5\\{13}",       /* wake_str */
#else
    "ATQ0X4N1\\\\K5\\{13}",                 /* wake_str */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H\\{13}",                        /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N0\\{13}",                       /* ec_off_str */
    "AT%C1\\\"H3\\{13}",                    /* dc_on_str */
    "AT%C0\\\"H0\\{13}",                    /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\J0\\{13}",                       /* sb_on_str */
    "AT\\\\J1\\{13}",                       /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF SUPRASON =                       /* SupraSonic */
    {
    "Diamond SupraSonic 288V+",         /* Diamond Multimedia Systems Inc */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\\{13}",    /* wake_str */
#else
#ifdef VMS
    "ATQ0E1N1W0X4Y0&S1\\{13}",            /* wake_str */
#else
    "ATQ0E1N1W0X4Y0\\{13}",               /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K\\{13}",                         /* nofc_str */
    "AT&Q5\\\\N3S48=7\\{13}",               /* ec_on_str */
    "AT&Q0\\\\N1\\{13}",                    /* ec_off_str */
    "AT%C3S46=138\\{13}",                 /* dc_on_str */
    "AT%C0S46=136\\{13}",                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM\\{13}",                          /* sp_off_str */
    "ATL\\{13}",                          /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF BESTDATA =                       /* Best Data */
    {
    "Best Data Fax Modem",              /* Best Data Fax Modem */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1N1W0X4Y0&S0&C1&D2\\{13}",    /* wake_str */
#else
#ifdef VMS
    "ATQ0E1N1W0X4Y0&S1\\{13}",            /* wake_str */
#else
    "ATQ0E1N1W0X4Y0\\{13}",               /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K\\{13}",                         /* nofc_str */
    "AT&Q6\\\\N3\\{13}",                    /* ec_on_str */
    "AT&Q0\\\\N1\\{13}",                    /* ec_off_str */
    "AT%C3\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\N3\\{13}",                       /* sb_on_str */
    "AT\\\\N0\\{13}",                       /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ATT1900 =                        /* AT&T Secure Data STU III 1900 */
    {
    "AT&T Secure Data STU III Model 1900", /* name */
    "",                                 /* pulse command */
    "",                                 /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4\\{13}",                   /* wake_str */
#else
    "ATQ0E1X4\\{13}",                     /* wake_str */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* sp_off_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    9600L,                              /* max_speed */
    CKD_AT|CKD_SB|CKD_HW,               /* capas */
    getok                               /* ok_fn */
};

/*
  Experimentation showed that hardly any of the documented commands did
  anything other that print ERROR.  At first there was no communication at
  all at 9600 bps -- turns out the interface speed was stuck at 2400.
  ATS28=130 (given at 2400 bps) allowed it to work at 9600.
*/
static
MDMINF ATT1910 =                        /* AT&T Secure Data STU III 1910 */
    {                                   /* Adds V.32bis, V.42, V.42bis */
    "AT&T Secure Data STU III Model 1910", /* name */

/* Believe it or not, "ATT" and "ATP" result in ERROR */

    "",                                 /* pulse command */
    "",                                 /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0E1V1X4\\{13}",                   /* wake_str */
#else
    "ATQ0E1X4\\{13}",                     /* wake_str */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
#ifdef COMMENT
/* These are evidently read-only registers */
    "ATS46=138S47=0\\{13}",               /* ec_on_str */
    "ATS46=138S47=128\\{13}",             /* ec_off_str */
    "ATS46=138S47=0\\{13}",               /* dc_on_str */
    "ATS46=138S47=128\\{13}",             /* dc_off_str */
#else
    "",
    "",
    "",
    "",
#endif /* COMMENT */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* sp_off_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    9600L,                              /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF KEEPINTOUCH =                    /* AT&T KeepinTouch Card Modem */
    {
    "AT&T KeepinTouch V.32bis Card Modem", /* Name */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
/* This used to include &C1&S0&D2+Q0 but that gives ERROR */
    "ATQ0E1V1X4&S0&C1&D2\\\\K5\\{13}",      /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1\\\\K5\\{13}",              /* wake_str */
#else
    "ATQ0E1X4\\\\K5\\{13}",                 /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "AT\\\\Q1\\\\X0\\{13}",                   /* swfc_str */
    "AT\\\\Q0\\{13}",                       /* nofc_str */
    "AT\\\\N3-J1\\{13}",                    /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "AT%C3\\\"H3\\{13}",                    /* dc_on_str */
    "AT%C0\\\"H0\\{13}",                    /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "ATN0\\\\J0\\{13}",                     /* sb_on_str */
    "ATN1\\\\J1\\{13}",                     /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    57600L,                             /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ROLM_AT =                /* Rolm data phone with AT command set */
    {
    "Rolm 244PC or 600 Series with AT Command Set",
    "",                                 /* pulse command */
    "",                                 /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1\\{13}",                     /* wake_str */
#else
    "ATQ0\\{13}",                         /* wake_str */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATDT%s\\{13}",                       /* dial_str -- always Tone */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "",                                 /* hwfc_str */
    "",                                 /* swfc_str */
    "",                                 /* nofc_str */
    "",                                 /* ec_on_str */
    "",                                 /* ec_off_str */
    "",                                 /* dc_on_str */
    "",                                 /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "",                                 /* sb_on_str */
    "",                                 /* sb_off_str */
    "",                                 /* sp_on_str */
    "",                                 /* sp_off_str */
    "",                                 /* vol1_str */
    "",                                 /* vol2_str */
    "",                                 /* vol3_str */
    "",                                 /* ignoredt */
    "",                                 /* ini2 */
    19200L,                             /* max_speed */
    CKD_AT,                             /* capas */
    getok                               /* ok_fn */
};

static
MDMINF ATLAS =                          /* Atlas / Newcom ixfC 33.6 */
    {
    "Atlas / Newcom 33600ixfC Data/Fax Modem", /* Name */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATZ0&FQ0V1&C1&D2\\{13}",             /* wake_str */
#else
    "ATZ0&FQ0V1\\{13}",                   /* wake_str */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT\\\"H3\\{13}",                       /* ec_on_str */
    "AT\\\"H0\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "ATN0\\\\J0\\{13}",                     /* sb_on_str */
    "ATN1\\\\J1\\{13}",                     /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF CODEX = {                        /* Motorola Codex */
    "Motorola Codex 326X Series",       /* Name - AT&V to see settings */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    /* &M0=Async (not sync) */
    /* *MM0=Automatic modulation negotiation */
    /* *DE22=Automatic data rate */
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0&S0&D2\\{13}", /* wake_str */
#else
#ifdef VMS
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0&S1\\{13}", /* wake_str */
#else
    "ATZQ0E1V1X4Y0*DE22*MM0&C1&M0\\{13}", /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT*MF1*FL3\\{13}",                   /* hwfc_str */
    "AT*MF1*FL1\\{13}",                   /* swfc_str */
    "AT*MF0*FL0\\{13}",                   /* nofc_str */
    "AT*EC0*SM3*SC0\\{13}",               /* ec_on_str */
    "AT*SM0\\{13}",                       /* ec_off_str */
    "AT*DC1\\{13}",                       /* dc_on_str */
    "AT*DC0\\{13}",                       /* dc_off_str */
    "AT*AA5S0=1\\{13}",                   /* aa_on_str */
    "AT*AA5S0=0\\{13}",                   /* aa_off_str */
    "AT*SC1\\{13}",                       /* sb_on_str */
    "AT*SC0\\{13}",                       /* sb_off_str */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3*BD2\\{13}",                     /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MT5634ZPX =                      /* Multitech */
    {
    "Multitech MT5634ZPX",              /* name */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATE1Q0V1X4&S0&C1&D2&Q0\\{13}",       /* wake_str */
#else
#ifdef VMS
    "ATQ0E1X4&S1&Q0\\{13}",               /* wake_str */
#else
    "ATQ0E1X4&Q0\\{13}",                  /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT&K3\\{13}",                        /* hwfc_str */
    "AT&K4\\{13}",                        /* swfc_str */
    "AT&K0\\{13}",                        /* nofc_str */
    "AT\\\\N3\\{13}",                       /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\J0\\{13}",                       /* sb_on_str */
    "AT\\\\J1\\{13}",                       /* sb_off_str (NOT SUPPORTED) */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};

static
MDMINF MOTSM56 =                        /* Motorola SM56 Chipset */
    {
    "Motorola SM56 V.90 chipset",       /* name */
    "ATP\\{13}",                          /* pulse command */
    "ATT\\{13}",                          /* tone command */
    35,                                 /* dial_time */
    ",",                                /* pause_chars */
    2,                                  /* pause_time */
#ifdef OS2
    "ATQ0V1X4&S0&C1&D2*MM16\\{13}",       /* wake_str */
#else
#ifdef VMS
    "ATQ0V1X4&S1&C1&D2*MM16\\{13}",       /* wake_str */
#else
    "ATQ0V1X4&C1&D2*MM16\\{13}",          /* wake_str */
#endif /* VMS */
#endif /* OS2 */
    0,                                  /* wake_rate */
    "OK\\{13}",                           /* wake_prompt */
    "",                                 /* dmode_str */
    "",                                 /* dmode_prompt */
    "ATD%s\\{13}",                        /* dial_str */
    0,                                  /* dial_rate */
    1100,                               /* esc_time */
    43,                                 /* esc_char */
    "ATQ0H0\\{13}",                       /* hup_str */
    "AT\\\\Q3\\{13}",                       /* hwfc_str */
    "AT\\\\Q1\\{13}",                       /* swfc_str */
    "AT\\\\Q0\\{13}",                       /* nofc_str */
    "AT\\\\N7\\{13}",                       /* ec_on_str */
    "AT\\\\N1\\{13}",                       /* ec_off_str */
    "AT%C1\\{13}",                        /* dc_on_str */
    "AT%C0\\{13}",                        /* dc_off_str */
    "ATS0=1\\{13}",                       /* aa_on_str */
    "ATS0=0\\{13}",                       /* aa_off_str */
    "AT\\\\J0\\{13}",                       /* sb_on_str */
    "AT\\\\J1\\{13}",                       /* sb_off_str (NOT SUPPORTED) */
    "ATM1\\{13}",                         /* sp_on_str */
    "ATM0\\{13}",                         /* sp_off_str */
    "ATL1\\{13}",                         /* vol1_str */
    "ATL2\\{13}",                         /* vol2_str */
    "ATL3\\{13}",                         /* vol3_str */
    "ATX3\\{13}",                         /* ignoredt */
    "",                                 /* ini2 */
    115200L,                            /* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok                               /* ok_fn */
};
#endif /* MINIDIAL */

/* END MDMINF STRUCT DEFINITIONS */

#endif /* 1120_MODEMS */

/* END MDMINF STRUCT DEFINITIONS */

/*
  Table to convert modem numbers to MDMINF struct pointers.
  The entries MUST be in ascending order by modem number, without any
  "gaps" in the numbers, and starting from one (1).
*/

MDMINF *modemp[] = {
    NULL,                               /*  0 */
#ifdef MINIDIAL
    &CCITT,				/*  1 */
    &HAYES,				/*  2 */
    &UNKNOWN,				/*  3 */
    &DUMMY,				/*  4 */
    &GENERIC,				/*  5 */
    &ITUTV250,                          /*  6 */
#else
    &ATTDTDM,				/*  1 */
    &ATTISN,				/*  2 */
    &ATTMODEM,				/*  3 */
    &CCITT,				/*  4 */
#ifdef OLDMODEMS
    &CERMETEK,				/*  5 */
    &DF03,				/*  6 */
    &DF100,				/*  7 */
    &DF200,				/*  8 */
    &GDC,				/*  9 */
#else
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &HAYES,				/* 10 */
#ifdef OLDMODEMS
    &PENRIL,				/* 11 */
    &RACAL,				/* 12 */
#else
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &UNKNOWN,				/* 13 */
#ifdef OLDMODEMS
    &VENTEL,				/* 14 */
    &CONCORD,				/* 15 */
#else
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &DUMMY,				/* 16 */
    &ROLM,				/* 17 */
#ifdef OLDMODEMS
    &MICROCOM,				/* 18 */
#else
    NULL,
#endif /* OLDMODEMS */
    &USR,				/* 19 USR Courier and Sportster */
    &OLDTB,				/* 20 Old Telebits */
    &DIGITEL,				/* 21 Digitel CCITT */
    &H_1200,				/* 22 Hayes 1200 */
    &H_ULTRA,				/* 23 Hayes Ultra */
    &H_ACCURA,				/* 24 Hayes Optima */
    &PPI,				/* 25 PPI */
    &DATAPORT,				/* 26 Dataport */
    &BOCA,				/* 27 Boca */
    &MOTOROLA,				/* 28 Motorola UDS MOTOROLA */
    NULL,				/* 29 Digicomm */
    NULL,				/* 30 Dynalink */
    &INTEL,				/* 31 Intel */
    &UCOM_AT,				/* 32 Microcom in AT mode */
    &MULTITECH,				/* 33 Multitech */
    &SUPRA,				/* 34 Supra */
    &ZOLTRIX,				/* 35 Zoltrix */
    &ZOOM,				/* 36 Zoom */
    &ZYXEL,				/* 37 ZyXEL */
    &DUMMY,				/* 38 TAPI */
#ifdef OLDTBCODE
    &TELEBIT,
#else
    &NEWTB,				/* 39 New-Telebit */
#endif /* OLDTBCODE */
    &MAXTECH,				/* 40 MaxTech */
    &DUMMY,				/* 41 User-defined */
    &RWV32,				/* 42 Rockwell V.32 */
    &RWV32B,				/* 43 Rockwell V.32bis */
    &RWV34,				/* 44 Rockwell V.34 */
    &MWAVE,				/* 45 IBM Mwave */
    &TELEPATH,				/* 46 Gateway 2000 Telepath II 28.8 */
    &MICROLINK,				/* 47 MicroLink modems */
    &CARDINAL,				/* 48 Cardinal */
    &GENERIC,				/* 49 Generic high-speed */
    &XJACK,				/* 50 Megahertz-Xjack */
    &SPIRITII,				/* 51 QuickComm Spirit II */
    &MONTANA,				/* 52 Motorola Montana */
    &COMPAQ,				/* 53 Compaq Data+Fax */
    &FUJITSU,				/* 54 Fujitsu */
    &MHZATT,				/* 55 Megahertz AT&T V.34 */
    &SUPRASON,				/* 56 Suprasonic */
    &BESTDATA,				/* 57 Best Data */
    &ATT1900,				/* 58 AT&T Secure Data STU III 1900 */
    &ATT1910,				/* 59 AT&T Secure Data STU III 1910 */
    &KEEPINTOUCH,			/* 60 AT&T KeepInTouch */
    &USRX2,				/* 61 USR XJ-1560 X2 */
    &ROLM_AT,				/* 62 Rolm with AT command set */
    &ATLAS,				/* 63 Atlas / Newcom */
    &CODEX,				/* 64 Motorola Codex */
    &MT5634ZPX,				/* 65 Multitech MT5634ZPX */
    &ULINKV250,				/* 66 Microlink V.250 56K */
    &ITUTV250,				/* 67 Generic ITU-T V.250 */
    &RWV90,				/* 68 Rockwell V.90 56K */
    &SUPRAX,				/* 69 Diamond Supra Express V.90 */
    &LUCENT,                            /* 70 Lucent Venus chipset */
    &PCTEL,                             /* 71 PCTel */
    &CONEXANT,                          /* 72 Conexant */
    &ZOOMV34,                           /* 73 Zoom V.34 */
    &ZOOMV90,                           /* 74 Zoom V.90 */
    &ZOOMV92,                           /* 75 Zoom V.92 */
    &MOTSM56                            /* 76 Motorola SM56 chipset */
#endif /* MINIDIAL */
};
/*
 * Declare modem names and associated numbers for command parsing,
 * and also for doing number-to-name translation.
 *
 * The entries must be in alphabetical order by modem name.
 */
struct keytab mdmtab[] = {
#ifndef MINIDIAL
    "3com-usr-megahertz-56k", n_USRX2,  0,
    "atlas-newcom-33600ifxC", n_ATLAS,  0,
    "att-1900-stu-iii", n_ATT1900,      0,
    "att-1910-stu-iii", n_ATT1910,      0,
    "att-7300",		n_ATTUPC,	0,
    "att-dataport",	n_DATAPORT,	0,
    "att-dtdm",		n_ATTDTDM,	0,
    "att-isn",          n_ATTISN,       0,
    "att-keepintouch",  n_KEEPINTOUCH,  0,
    "att-switched-net", n_ATTMODEM,	0,
    "att7300",		n_ATTUPC,	CM_INV,	/* old name */
    "attdtdm",		n_ATTDTDM,	CM_INV,	/* old name */
    "attisn",           n_ATTISN,       CM_INV,	/* old name */
    "attmodem",		n_ATTMODEM,	CM_INV,	/* old name */

    "bestdata",         n_BESTDATA,     0,
    "boca",		n_BOCA,		0,
    "cardinal",         n_CARDINAL,     0,
#endif /* MINIDIAL */
    "ccitt-v25bis",	n_CCITT,	CM_INV, /* Name changed to ITU-T */
#ifndef MINIDIAL
#ifdef OLDMODEMS
    "cermetek",		n_CERMETEK,	M_OLD,
#endif /* OLDMODEMS */
    "compaq",           n_COMPAQ,       0,
#ifdef OLDMODEMS
    "concord",		n_CONCORD,	M_OLD,
#endif /* OLDMODEMS */
    "conexant",         n_CONEXANT,     0,
    "courier",          n_USR,          CM_INV,
    "dataport",		n_DATAPORT,	CM_INV,	/* == att-dataport */
#ifdef OLDMODEMS
    "df03-ac",		n_DF03,		M_OLD,
    "df100-series",	n_DF100,	M_OLD,
    "df200-series",	n_DF200,	M_OLD,
#endif /* OLDMODEMS */
    "digitel-dt22",	n_DIGITEL,	0,
#endif /* MINIDIAL */
    "direct",		0,		CM_INV,	/* Synonym for NONE */
#ifndef MINIDIAL
    "fujitsu",          n_FUJITSU,      0,
    "gateway-telepath", n_TELEPATH,     0,
#ifdef OLDMODEMS
    "gdc-212a/ed",	n_GDC,		M_OLD,
    "ge",               n_GENERIC,	CM_INV|CM_ABR,
    "gen",              n_GENERIC,	CM_INV|CM_ABR,
    "gendatacomm",	n_GDC,		CM_INV,	/* Synonym for GDC */
#endif /* OLDMODEMS */
#endif /* MINIDIAL */  
    "generic-high-speed", n_GENERIC,    0,
    "h", 	   	n_HAYES,	CM_INV|CM_ABR,
    "ha", 	   	n_HAYES,	CM_INV|CM_ABR,
    "hay",    		n_HAYES,	CM_INV|CM_ABR,
    "haye", 	   	n_HAYES,	CM_INV|CM_ABR,
    "hayes",    	n_HAYES,	CM_INV|CM_ABR, /* Hayes 2400 */
#ifndef MINIDIAL
    "hayes-1200",	n_H_1200,	0,
#endif /* MINIDIAL */
    "hayes-2400",	n_HAYES,	0,
#ifndef MINIDIAL
    "hayes-high-speed", n_H_ACCURA,     0,
    "hayes-accura",     n_H_ACCURA,     CM_INV,
    "hayes-optima",     n_H_ACCURA,     CM_INV,
    "hayes-ultra",	n_H_ULTRA,	CM_INV,
    "hst-courier",      n_USR,          CM_INV,	/* Synonym for COURIER */
    "intel",		n_INTEL,        0,
#endif /* MINIDIAL */
    "itu-t-v250",       n_ITUTV250,     CM_INV,
    "itu-t-v25ter/v250",n_ITUTV250,     0,
    "itu-t-v25bis",	n_CCITT,	0,	/* New name for CCITT */
#ifndef MINIDIAL
    "lucent",           n_LUCENT,      0,
    "maxtech",		n_MAXTECH,	0,
    "megahertz-att-v34",n_MHZATT,       0,
    "megahertz-xjack",  n_XJACK,        0,
    "megahertz-xjack-33.6", n_XJACK,   0,
    "megahertz-xjack-56k",  n_USRX2,   0, /* 3COM/USR/Megahertz 33.6 PC Card */
    "mi",		n_MICROCOM,	CM_INV|CM_ABR,
    "mic",		n_MICROCOM,	CM_INV|CM_ABR,
    "micr",		n_MICROCOM,	CM_INV|CM_ABR,
    "micro",		n_MICROCOM,	CM_INV|CM_ABR,
    "microc",		n_MICROCOM,	CM_INV|CM_ABR,
    "microco",		n_MICROCOM,	CM_INV|CM_ABR,
    "microcom",		n_MICROCOM,	CM_INV|CM_ABR,
    "microcom-at-mode",	n_UCOM_AT,	0, /* Microcom DeskPorte, etc */
    "microcom-sx-mode",	n_MICROCOM,	0, /* Microcom AX,QX,SX, native mode */
    "microlink",        n_MICROLINK,    0,
    "microlink-v250",   n_ULINKV250,    0,
    "motorola-codex",   n_CODEX,        0,
    "motorola-fastalk", n_MOTOROLA,	0,
    "motorola-lifestyle",n_MOTOROLA,	0,
    "motorola-montana", n_MONTANA,	0,
    "motorola-sm56-v90",n_MOTSM56,      0,
    "mt5634zpx",        n_MT5634ZPX,    0,
    "multitech",	n_MULTI,	0,
    "mwave",		n_MWAVE,	0,
#endif /* MINIDIAL */
    "none",             0,              0,
#ifndef MINIDIAL
#ifndef OLDTBCODE
    "old-telebit",      n_TELEBIT,      0,
#endif /* OLDTBCODE */
    "pctel",            n_PCTEL,        0,
#ifdef OLDMODEMS
    "penril",		n_PENRIL,	M_OLD,
#endif /* OLDMODEMS */
    "ppi",              n_PPI,		0,
#ifdef OLDMODEMS
    "racalvadic",	n_RACAL,	M_OLD,
#endif /* OLDMODEMS */
    "rockwell-v32",	n_RWV32,	0,
    "rockwell-v32bis",	n_RWV32B,	0,
    "rockwell-v34",	n_RWV34,	0,
    "rockwell-v90",	n_RWV90,	0,
    "rolm-244pc",       n_ROLMAT,       0,
    "rolm-600-series",  n_ROLMAT,       0,
    "rolm-dcm",		n_ROLM,		0,
    "spirit-ii",        n_SPIRITII,     0,
    "sportster",        n_USR,          CM_INV,
    "sup",	        n_SUPRA,	CM_INV|CM_INV,
    "supr",	        n_SUPRA,	CM_INV|CM_INV,
    "supra",	        n_SUPRA,	CM_INV|CM_INV,
    "supra-express-v90",n_SUPRAX,       0,
    "suprafaxmodem",	n_SUPRA,	0,
    "suprasonic",	n_SUPRASON,	0,
#ifdef CK_TAPI
    "tapi",		n_TAPI,		0,
#endif /* CK_TAPI */
    "te",               n_TBNEW,        CM_INV|CM_ABR,
    "tel",              n_TBNEW,        CM_INV|CM_ABR,
    "telebit",          n_TBNEW,        0,
    "telepath",         n_TELEPATH,     CM_INV,
#endif /* MINIDIAL */
    "unknown",		n_UNKNOWN,	0,
    "user-defined",     n_UDEF,		0,
#ifndef MINIDIAL
    "usr",              n_USR,          CM_INV|CM_ABR,
/* Keep the next one for backwards compatibility, but it's the same as H2400 */
    "usr-212a",		n_HAYES,	CM_INV,
    "usr-courier",      n_USR,          CM_INV,
    "usr-megahertz-56k", n_USRX2,       0,
    "usr-sportster",    n_USR,          CM_INV,
    "usr-xj1560-x2",    n_USRX2,        0,
    "usrobotics",       n_USR,          0,
    "v25bis",		n_CCITT,	CM_INV, /* Name changed to ITU-T */
#ifdef OLDMODEMS
    "ventel",		n_VENTEL,	M_OLD,
#endif /* OLDMODEMS */
    "zoltrix",		n_ZOLTRIX,	0,
    "zoltrix-v34",      n_ZOLTRIX,      0,
    "zoltrix-hcf-v90",  n_ITUTV250,     0,
    "zoom",		n_ZOOM,		0,
    "zoom-v32bis",      n_ZOOM,         0,
    "zoom-v34",         n_ZOOMV34,      0,
    "zoom-v90",         n_ZOOMV90,      0,
    "zoom-v92",         n_ZOOMV92,      0,
    "zyxel",		n_ZYXEL,	0,
#endif /* MINIDIAL */
    "",                 0,              0
};
int nmdm = (sizeof(mdmtab) / sizeof(struct keytab)) - 1; /* Number of modems */

MDMINF * 
FindMdmInf( char * name )
{
    int i; 
    for ( i=0;i<nmdm;i++ ) {
        if ( !strcmp( name, mdmtab[i].kwd ) )
            return modemp[mdmtab[i].kwval];
    }
    return NULL;
}
}

extern ZIL_UINT8 TapiAvail;

void CopyMdmInfToModem( MDMINF * mdminf, K_MODEM * modem )
{
    if ( mdminf && modem ) {
	strcpy( modem->_pulse_str, mdminf->pulse ? mdminf->pulse : "" );
	strcpy( modem->_tone_str, mdminf->tone ? mdminf->tone : "");
	strcpy( modem->_init_str, mdminf->wake_str ? mdminf->wake_str : "");
	strcpy( modem->_dial_mode_str, mdminf->dmode_str ? mdminf->dmode_str : "" );
	strcpy( modem->_dial_mode_prompt_str, 
		mdminf->dmode_prompt ? mdminf->dmode_prompt : "" );
	strcpy( modem->_dial_str, mdminf->dial_str ? mdminf->dial_str : "" );
	modem->_escape_char = mdminf->esc_char ;
	strcpy( modem->_hup_str, mdminf->hup_str ? mdminf->hup_str : "" );
	strcpy( modem->_hwfc_str, mdminf->hwfc_str ? mdminf->hwfc_str : "" );
	strcpy( modem->_swfc_str, mdminf->swfc_str ? mdminf->swfc_str : "" );
	strcpy( modem->_nofc_str, mdminf->nofc_str ? mdminf->nofc_str : ""  );
	strcpy( modem->_ec_on_str, mdminf->ec_on_str ? mdminf->ec_on_str : ""  );
	strcpy( modem->_ec_off_str, mdminf->ec_off_str ? mdminf->ec_off_str : ""  );
	strcpy( modem->_dc_on_str, mdminf->dc_on_str ? mdminf->dc_on_str : "" );
	strcpy( modem->_dc_off_str, mdminf->dc_off_str ? mdminf->dc_off_str : "" );
	strcpy( modem->_aa_on_str, mdminf->aa_on_str ? mdminf->aa_on_str : "" );
	strcpy( modem->_aa_off_str, mdminf->aa_off_str ? mdminf->aa_off_str : "" );
	strcpy( modem->_sb_on_str, mdminf->sb_on_str ? mdminf->sb_on_str : "" );
	strcpy( modem->_sb_off_str, mdminf->sb_off_str ? mdminf->sb_off_str : "" );
	strcpy( modem->_sp_on_str, mdminf->sp_on_str ? mdminf->sp_on_str : "" );
	strcpy( modem->_sp_off_str, mdminf->sp_off_str ? mdminf->sp_off_str : "" );
	strcpy( modem->_vol1_str, mdminf->vol1_str ? mdminf->vol1_str : ""  );
	strcpy( modem->_vol2_str, mdminf->vol2_str ? mdminf->vol2_str : "" );
	strcpy( modem->_vol3_str, mdminf->vol3_str ? mdminf->vol3_str : "" );
	strcpy( modem->_ignore_dial_tone_str, mdminf->ignoredt ? mdminf->ignoredt : "" );
	modem->_at = (mdminf->capas & CKD_AT ) != 0;
	modem->_v25= (mdminf->capas & CKD_V25) != 0;
	modem->_sb = (mdminf->capas & CKD_SB ) != 0;
	modem->_ec = (mdminf->capas & CKD_EC ) != 0;
	modem->_dc = (mdminf->capas & CKD_DC ) != 0;
	modem->_hw = (mdminf->capas & CKD_HW ) != 0;
	modem->_sw = (mdminf->capas & CKD_SW ) != 0;
	modem->_ks = (mdminf->capas & CKD_KS ) != 0;
	modem->_tb = (mdminf->capas & CKD_TB ) != 0;
	modem->_max_speed = mdminf->max_speed;
    }
}

ZIL_ICHAR K_MODEM_DIALOG::_className[] = "K_MODEM_DIALOG" ;
ZIL_ICHAR K_MODEM_CONFIG::_className[] = "K_MODEM_CONFIG" ;

extern K_CONNECTOR * connector;

K_MODEM_DIALOG::K_MODEM_DIALOG(void)
   : ZAF_DIALOG_WINDOW("WINDOW_MODEMS",defaultStorage)
{
   windowManager->Center(this) ;
   relative.bottom = relative.Height() - 1 ;
   relative.top = 0 ;
}

K_MODEM_DIALOG::K_MODEM_DIALOG( UI_LIST * ModemList )
   : ZAF_DIALOG_WINDOW("WINDOW_MODEMS",defaultStorage),SourceList(ModemList)
{
    windowManager->Center(this) ;

    _modem_list = (UIW_VT_LIST *) Get( LIST_NAME );
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_NAME);

    K_MODEM * item = NULL;
    K_MODEM * newitem = NULL;
    UI_EVENT AddObject( S_ADD_OBJECT );
    _current = NULL;
    for ( item = (K_MODEM *) ModemList->First(); item ; 
	  item = (K_MODEM *) item->Next() ) {
        newitem = new K_MODEM( *item );
	newitem->DataSet( newitem->_name );
	newitem->_dialog = this;
	AddObject.data = newitem;
	//*_modem_list + newitem ;
	_modem_list->Event(AddObject);
	if ( item == ModemList->Current() )
	{
	    _modem_list->SetCurrent( newitem );
	    *combo + newitem;
	    _current = newitem;
	}
    }

    if ( _current == NULL ) {
	_current = (K_MODEM *) _modem_list->First();
	if ( _current ) {
	    _modem_list->SetCurrent( _current );
	    *combo + _current;
	}
    }

    if ( _current ) {
	*combo + _current;

        if ( _current->_is_tapi ) {
            UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_MODIFY );
            if ( button ) {
                button->woFlags |= WOF_NON_SELECTABLE ;
                button->Information(I_CHANGED_FLAGS,NULL) ;
            }
        }
    }

#ifdef WIN32
    Information( I_SET_TEXT, "C-Kermit for Windows Modem Definitions" ) ;
#else
    Information( I_SET_TEXT, "C-Kermit for OS/2 Modem Definitions" ) ;
#ifdef COMMENT
    UIW_ICON * icon = (UIW_ICON *) Get( ICON_K95 ) ;
    *this - icon ;
    delete icon ;
    icon = new UIW_ICON( 0,0,"CKNKER","K95/2 Modems", 
			 ICF_MINIMIZE_OBJECT );
    *this + icon ;
#endif /* COMMENT */
#endif /* WIN32 */

    InitParityList();
    InitStopBitsList();
    InitFlowList();
    InitSpeedList();
#if defined(WIN32)
    if ( _current && _current->_is_tapi )
	InitTapiPortList();
    else
#endif
        InitPortList();
    InitModemTypeList();

    CopyValuesFromCurrent();

    if ( combo->Count() <= 1 ) {
	UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_REMOVE );
	if ( button ) {
	    button->woFlags |= WOF_NON_SELECTABLE ;
	    button->Information(I_CHANGED_FLAGS,NULL) ;
	}
    }
}

void
K_MODEM_DIALOG::CopyValuesFromCurrent()
{
    UIW_COMBO_BOX * combo ;
    UIW_BUTTON * button;
    ZIL_ICHAR itoabuf[40] ;

    if ( _current == NULL )
	return ;

    /* need to configure window settings */
    combo = (UIW_COMBO_BOX *) Get( COMBO_PORT ) ;
    combo->DataSet( _current->_port );
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TYPE ) ;
    combo->DataSet( _current->_type );
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
    itoa( _current->_speed, itoabuf, 10 ) ;
    combo->DataSet( itoabuf ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
    switch ( _current->_parity )
    {
    case NO_PARITY:
	combo->DataSet("None");
	break;
    case SPACE:
	combo->DataSet("Space");
	break;
    case MARK:
	combo->DataSet("Mark");
	break;
    case EVEN:
	combo->DataSet("Even");
	break;
    case ODD:
	combo->DataSet("Odd");
	break;
    case SPACE_8:
	combo->DataSet("Space_8bit");
	break;
    case MARK_8:
	combo->DataSet("Mark_8bit");
	break;
    case EVEN_8:
	combo->DataSet("Even_8Bit");
	break;
    case ODD_8:
	combo->DataSet("Odd_8bit");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
    switch ( _current->_flow ) {
    case NO_FLOW:
	combo->DataSet("None");
	break;
    case XON_XOFF:
	combo->DataSet("Xon/Xoff");
	break;
    case RTS_CTS:
	combo->DataSet("Rts/Cts");
	break;
    case AUTO_FLOW:
	combo->DataSet("Auto");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_ERROR_CORRECTION );
    if ( _current->_correction )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_COMPRESSION );
    if ( _current->_compression )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_CARRIER );
    if ( _current->_carrier )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_SPEED_MATCHING );
    if ( _current->_speed_match )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_SPEAKER );
    if ( _current->_speaker )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    UIW_SCROLL_BAR * slider = (UIW_SCROLL_BAR *) Get( SLIDER_VOLUME );
    int value = _current->_volume;
    slider->Information( I_SET_VALUE, &value );
    if ( _current->_speaker )
	slider->woFlags &= ~WOF_NON_SELECTABLE ;
    else 
	slider->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
    switch ( _current->_stopbits ) {
    case STOPBITS_1_0:
	combo->DataSet("1.0");
	break;
    case STOPBITS_1_5:
	combo->DataSet("1.5");
	break;
    case STOPBITS_2_0:
	combo->DataSet("2.0");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

}

void
K_MODEM_DIALOG::CopyValuesToCurrent()
{
    UIW_COMBO_BOX * combo ;
    ZIL_ICHAR * str;

    if ( _current == NULL )
	return ;

    /* need to configure window settings */
    combo = (UIW_COMBO_BOX *) Get( COMBO_PORT ) ;
    strncpy(_current->_port, combo->DataGet(), 60 );

    combo = (UIW_COMBO_BOX *) Get( COMBO_TYPE ) ;
    strncpy(_current->_type, combo->DataGet(), 60 );

    _current->_is_tapi = !strcmp("tapi",_current->_type);

    combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
    _current->_speed = atoi( combo->DataGet() );

    combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
    str = combo->DataGet();
    if ( !strcmp( str, "None" ) )
	_current->_parity = NO_PARITY;
    else if ( !strcmp( str, "Space" ) )
	_current->_parity = SPACE;
    else if ( !strcmp( str, "Mark") )
	_current->_parity = MARK;
    else if ( !strcmp(str, "Even") )
	_current->_parity = EVEN;
    else if ( !strcmp( str, "Odd") )
	_current->_parity = ODD;
    else if ( !strcmp( str, "Space_8bit" ) )
	_current->_parity = SPACE_8;
    else if ( !strcmp( str, "Mark_8bit") )
	_current->_parity = MARK_8;
    else if ( !strcmp(str, "Even_8bit") )
	_current->_parity = EVEN_8;
    else if ( !strcmp( str, "Odd_8bit") )
	_current->_parity = ODD_8;

    combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
    str = combo->DataGet();
    if ( !strcmp( str, "None" ) )
	_current->_flow = NO_FLOW;
    else if ( !strcmp( str, "Xon/Xoff" ) )
	_current->_flow = XON_XOFF;
    else if ( !strcmp( str, "Rts/Cts") )
	_current->_flow = RTS_CTS;
    else if ( !strcmp(str, "Auto") )
	_current->_flow = AUTO_FLOW;

    _current->_correction = FlagSet(Get( CHECK_ERROR_CORRECTION )->woStatus, WOS_SELECTED );
    _current->_compression = FlagSet(Get( CHECK_COMPRESSION )->woStatus, WOS_SELECTED );
    _current->_carrier = FlagSet(Get( CHECK_CARRIER )->woStatus, WOS_SELECTED );
    _current->_speed_match = FlagSet(Get( CHECK_SPEED_MATCHING )->woStatus, WOS_SELECTED );
    _current->_speaker = FlagSet(Get( CHECK_SPEAKER )->woStatus, WOS_SELECTED );

    UIW_SCROLL_BAR * slider = (UIW_SCROLL_BAR *) Get( SLIDER_VOLUME );
    int value;
    slider->Information( I_GET_VALUE, &value );
    _current->_volume = value ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
    str = combo->DataGet();
    if ( !strcmp( str, "1.0" ) )
	_current->_stopbits = STOPBITS_1_0;
    else if ( !strcmp( str, "1.5" ) )
	_current->_stopbits = STOPBITS_1_5;
    else if ( !strcmp( str, "2.0" ) )
	_current->_stopbits = STOPBITS_2_0;
}

void
K_MODEM_DIALOG::AddModem( ZIL_ICHAR * name )
{
    K_MODEM * newitem;
    if ( _current ) {
	CopyValuesToCurrent();
	newitem = new K_MODEM(*_current);
    }
    else
	newitem = new K_MODEM();
    strcpy( newitem->_name, name );
    newitem->DataSet( name );
    newitem->_dialog = this;
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_NAME);
    UI_EVENT AddObject(S_ADD_OBJECT) ;
    AddObject.data = newitem ;
    _modem_list->Event(AddObject);
    combo->Event(AddObject);
    //*_modem_list + newitem;
    UI_EVENT ReDisplay(S_REDISPLAY) ;
    UI_EVENT Create(S_CREATE) ;
    combo->Event( Create );
    combo->Event( ReDisplay );
#ifdef __OS2__
    combo->Information(I_CHANGED_FLAGS,NULL);
#endif
    *combo + newitem;
    _current = (K_MODEM *) combo->Current();
    if ( combo->Count() > 1 ) {
	UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_REMOVE );
	if ( button ) {
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
	    button->Information(I_CHANGED_FLAGS,NULL) ;
	}
    }
}

void
K_MODEM_DIALOG::ApplyChanges( void )
{
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_NAME);
    K_MODEM * item = NULL;
    K_MODEM * newCurrent = _current ;

    for ( item = (K_MODEM *) _delete_list.First(); item ; 
	  item = (K_MODEM *) _delete_list.First() ) {
	_delete_list - item ;
	if (K_CONNECTOR::_userFile->ChDir("~Modems") == 0) {
            if (K_CONNECTOR::_userFile->DestroyObject( item->_name ) != 0)
            {
                // What went wrong?
                ZAF_MESSAGE_WINDOW * message =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                            "Unable to remove modem \"%s\": errorno %d", 
                                            item->_name,
                                            K_CONNECTOR::_userFile->storageError ) ;
                message->Control();
                delete message ;
                return;
            }
            K_CONNECTOR::_userFile->ChDir("~");
        }
        delete item;
    }
    K_CONNECTOR::_userFile->Save(2) ;

    CopyValuesToCurrent();

    SourceList->Destroy();

    UI_EVENT  SubtractObject(S_SUBTRACT_OBJECT);
    for ( item = (K_MODEM *) _modem_list->First(); item ; 
	  item = (K_MODEM *) _modem_list->First() ) {
	item->_dialog = NULL;
	if ( strcmp( item->_name, item->DataGet() ) )
	    strncpy( item->_name, item->DataGet(), 60 );

	if (K_CONNECTOR::_userFile->ChDir("~Modems") == 0) {
            ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, 
				     item->_name, ID_K_MODEM,
				     UIS_OPENCREATE | UIS_READWRITE ) ;
            item->Store( item->_name, K_CONNECTOR::_userFile, &_fileObj,   
			 ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM)) ;
            if ( K_CONNECTOR::_userFile->storageError )
            {
                // What went wrong?
                ZAF_MESSAGE_WINDOW * message =
		new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
					"Unable to Write modem \"%s\": errorno %d", 
					item->_name,
					K_CONNECTOR::_userFile->storageError ) ;
                message->Control() ;	
                delete message ;
            }   
            K_CONNECTOR::_userFile->ChDir("~");
        }
	SubtractObject.data = item;
	_modem_list->Event(SubtractObject);
	*SourceList + item;
	if ( item == newCurrent )
	    SourceList->SetCurrent( item );
    }
}

K_MODEM_DIALOG::~K_MODEM_DIALOG( void ) 
{
    
}

EVENT_TYPE K_MODEM_DIALOG::Event( const UI_EVENT & event )
{
    EVENT_TYPE ccode = event.type ;

    switch ( event.type ) {
    case OPT_MODEM_HELP:
	helpSystem->DisplayHelp( windowManager, HELP_MODEMS ) ;
	break;

    case OPT_MODEM_SPEAKER: {
	UIW_BUTTON * button = ( UIW_BUTTON  * ) Get( CHECK_SPEAKER ) ;
	UIW_SCROLL_BAR * slider = (UIW_SCROLL_BAR *) Get( SLIDER_VOLUME );

	if ( FlagSet(button->woStatus, WOS_SELECTED) ) 
	    slider->woFlags &= ~WOF_NON_SELECTABLE ;
	else 
	    slider->woFlags |= WOF_NON_SELECTABLE ;
	slider->Information(I_CHANGED_FLAGS,NULL) ;
	break;
    }

    case OPT_MODEM_MODIFY: {
	if ( _current ) {
            K_MODEM_CONFIG * config = new K_MODEM_CONFIG( this, _current );
            config->Control();
            delete config;
        }
	break;
    }

    case OPT_MODEM_OK: {
	*windowManager - this ;
	ApplyChanges() ;
	break;
    }

    case OPT_MODEM_OBJECT: {
	UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_NAME);
	CopyValuesToCurrent();
	_current = (K_MODEM *) combo->Current();
	CopyValuesFromCurrent();
	break;
								   }

    case OPT_MODEM_ADD: {
	K_MODEM_NAME * newname = new K_MODEM_NAME( this );
        newname->Control();
        delete newname;
	break;
    }

    case OPT_MODEM_REMOVE: {
	UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_NAME);
	if ( _current && combo->Count() > 1 ) {
	    if ( connector->IsModemInUse(_current->_name) ) {
		ZAF_MESSAGE_WINDOW * message =
		    new ZAF_MESSAGE_WINDOW( "Error", 
					    "EXCLAMATION", 
					    ZIL_MSG_OK, ZIL_MSG_OK,
		    "Modem \"%s\" is in use. It may not be removed.",
					    _current->_name ) ;
		message->Control();
		delete message ;
		break;
	    }

	    UI_EVENT ReDisplay(S_REDISPLAY) ;
	    UI_EVENT Create(S_CREATE) ;
	    UI_EVENT SubtractObject(S_SUBTRACT_OBJECT) ;
	    K_MODEM * deletion = _current;
	    SubtractObject.data = _current;
	    //*combo - _current ;
	    combo->Event(SubtractObject);
	    _modem_list->Event(ReDisplay);
	    combo->Event( Create );
	    combo->Event( ReDisplay );
#ifdef __OS2__
	    combo->Information(I_CHANGED_FLAGS,NULL);
#endif
	    _current = (K_MODEM *) combo->Current();
	    CopyValuesFromCurrent();
	    _delete_list + deletion ;
	    if ( combo->Count() <= 1 ) {
		UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_REMOVE );
		if ( button ) {
		    button->woFlags |= WOF_NON_SELECTABLE ;
		    button->Information(I_CHANGED_FLAGS,NULL) ;
		}
	    }
	}
	break;
    }

    case OPT_MODEM_IS_TAPI:
    case OPT_MODEM_TYPE: {
	/* The user may have selected a new modem type */
	/* Reset the modem commands               */
	UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_TYPE );
	ZIL_ICHAR * type = combo->DataGet();
        if ( strcmp(type,_current->_type) ) {
            MDMINF * mdminf = FindMdmInf( type );
            CopyMdmInfToModem( mdminf, _current );
        }
#if defined(WIN32)
	UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_MODIFY );
	//combo = (UIW_COMBO_BOX *) Get(COMBO_PORT);
	//ZIL_ICHAR port[61];
	//strcpy(port,combo->DataGet());
	if ( !strcmp("tapi",type) ) {
	    InitTapiPortList();
	    button->woFlags |= WOF_NON_SELECTABLE;
	}
	else {
	    InitPortList();
	    button->woFlags &= ~WOF_NON_SELECTABLE;
	}
	button->Information( I_CHANGED_FLAGS, NULL );
	//combo->DataSet(port);
#endif
	break;
    }

    default:
	ccode = UIW_WINDOW::Event(event);
    }
    return ccode ;
}

void K_MODEM_DIALOG::
InitFlowList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_FLOW ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Xon/Xoff",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Rts/Cts",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Auto",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_MODEM_DIALOG::
InitParityList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PARITY ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Space",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Even",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Odd",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Mark",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"Space_8bit",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Even_8bit",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Odd_8bit",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Mark_8bit",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_MODEM_DIALOG::
InitStopBitsList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_STOPBITS ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"1.0",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"1.5",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"2.0",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_MODEM_DIALOG::
InitPortList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PORT ) ;
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_PORT );
    list->Destroy();
    for ( int i=1; i<=128; i++ ) {
        char buf[12];
        sprintf(buf,"COM%d",i);
        *list + new UIW_BUTTON( 0,0,0,buf,BTF_NO_3D );
    }

    UI_EVENT ReDisplay(S_REDISPLAY) ;
    UI_EVENT Create(S_CREATE) ;
    combo->Event( Create );
    combo->Event( ReDisplay );
#ifdef __OS2__
    combo->Information(I_CHANGED_FLAGS,NULL);
#endif
}

#if defined(WIN32)
void K_MODEM_DIALOG::
InitTapiPortList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PORT ) ;
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_PORT );
    list->Destroy();

    if ( TapiAvail ) {
	cktapiBuildLineTable( &tapilinetab, &_tapilinetab, &ntapiline );
	for ( int i=0 ; i<ntapiline ; i++ )
	    *list + new UIW_BUTTON( 0,0,0,_tapilinetab[i].kwd, BTF_NO_3D ) ;
    }
    UI_EVENT ReDisplay(S_REDISPLAY) ;
    UI_EVENT Create(S_CREATE) ;
    combo->Event( Create );
    combo->Event( ReDisplay );
#ifdef __OS2__
    combo->Information(I_CHANGED_FLAGS,NULL);
#endif
}
#endif

void K_MODEM_DIALOG::
InitModemTypeList( void )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TYPE ) ;
    for (int i=0;mdmtab[i].kwd[0];i++) {
        if (mdmtab[i].flgs == 0) {
#ifdef WIN32
            if ( mdmtab[i].kwval == n_TAPI ) {
                if ( TapiAvail ) {
                    *list
                        + new UIW_BUTTON( 0,0,0,
                                          mdmtab[i].kwd,
                                          BTF_NO_3D|BTF_SEND_MESSAGE,
                                          WOF_NO_FLAGS,
                                          ZIL_NULLF(ZIL_USER_FUNCTION), 
                                          OPT_MODEM_IS_TAPI );
                }
            }
            else 
#endif
            {
                *list 
                    + new UIW_BUTTON( 0,0,0,
                                      mdmtab[i].kwd,
                                      BTF_NO_3D|BTF_SEND_MESSAGE, 
                                      WOF_NO_FLAGS,
                                      ZIL_NULLF(ZIL_USER_FUNCTION), 
                                      OPT_MODEM_TYPE );
            }
        }
    }
}

void InitSpeedList( UIW_VT_LIST * list )
{
    *list
    + new UIW_BUTTON( 0,0,0,"50",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"75",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"110",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"150",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"300",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"600",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"1200",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"2400",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"3600",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"4800",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"7200",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"9600",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"14400",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"19200",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"28800",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"38400",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"57600",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"76800",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"115200",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"230400",BTF_NO_3D, WOF_NO_FLAGS )
    + new UIW_BUTTON( 0,0,0,"460800",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_MODEM_DIALOG::InitSpeedList( void ) 
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SPEED ) ;
    ::InitSpeedList( list ) ;
}



K_MODEM_NAME::K_MODEM_NAME( K_MODEM_DIALOG * Parent )
    : ZAF_DIALOG_WINDOW("WINDOW_NEW_MODEM",defaultStorage), _parent(Parent) 
{
    windowManager->Center(this);
}

EVENT_TYPE K_MODEM_NAME::Event( const UI_EVENT & event )
{
   EVENT_TYPE ccode = event.type ;
   UIW_STRING * string = NULL ;

   switch ( event.type ) {
      case OPT_DIALING_OK: {
	  UIW_STRING * string = (UIW_STRING *) Get( FIELD_NAME );
	  _parent->AddModem( string->DataGet() );
	  *windowManager - this ;
	  break;
      }

      default:
         ccode = UIW_WINDOW::Event(event);
   }
   return ccode ;
}

K_MODEM_NAME::~K_MODEM_NAME( void ) 
{
    
}

K_MODEM_CONFIG::K_MODEM_CONFIG( K_MODEM_DIALOG * Parent, K_MODEM * Modem )
    : ZAF_DIALOG_WINDOW("WINDOW_MODEM_CONFIG",defaultStorage), 
    _parent(Parent), _modem(Modem)
{
    windowManager->Center(this);
    InitSpeedList();

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_INIT );
    string->DataSet(_modem->_init_str);

    string = (UIW_STRING *) Get( FIELD_DIAL_MODE );
    string->DataSet(_modem->_dial_mode_str);

    string = (UIW_STRING *) Get( FIELD_DIAL_PROMPT );
    string->DataSet(_modem->_dial_mode_prompt_str);

    string = (UIW_STRING *) Get( FIELD_DIAL_STR );
    string->DataSet(_modem->_dial_str);

    string = (UIW_STRING *) Get( FIELD_DC_ON );
    string->DataSet(_modem->_dc_on_str);

    string = (UIW_STRING *) Get( FIELD_DC_OFF );
    string->DataSet(_modem->_dc_off_str);

    string = (UIW_STRING *) Get( FIELD_EC_ON );
    string->DataSet(_modem->_ec_on_str);

    string = (UIW_STRING *) Get( FIELD_EC_OFF );
    string->DataSet(_modem->_ec_off_str);

//    string = (UIW_STRING *) Get( FIELD_SB_ON );
//    string->DataSet(_modem->_sb_on_str);

//    string = (UIW_STRING *) Get( FIELD_SB_OFF );
//    string->DataSet(_modem->_sb_off_str);

    string = (UIW_STRING *) Get( FIELD_AA_ON );
    string->DataSet(_modem->_aa_on_str);

    string = (UIW_STRING *) Get( FIELD_AA_OFF );
    string->DataSet(_modem->_aa_off_str);

    string = (UIW_STRING *) Get( FIELD_SP_ON );
    string->DataSet(_modem->_sp_on_str);

    string = (UIW_STRING *) Get( FIELD_SP_OFF );
    string->DataSet(_modem->_sp_off_str);

    string = (UIW_STRING *) Get( FIELD_VOL1 );
    string->DataSet(_modem->_vol1_str);

    string = (UIW_STRING *) Get( FIELD_VOL2 );
    string->DataSet(_modem->_vol2_str);

    string = (UIW_STRING *) Get( FIELD_VOL3 );
    string->DataSet(_modem->_vol3_str);

    string = (UIW_STRING *) Get( FIELD_HUP );
    string->DataSet(_modem->_hup_str);

    string = (UIW_STRING *) Get( FIELD_HWFC );
    string->DataSet(_modem->_hwfc_str);

    string = (UIW_STRING *) Get( FIELD_SWFC );
    string->DataSet(_modem->_swfc_str);

    string = (UIW_STRING *) Get( FIELD_NOFC );
    string->DataSet(_modem->_nofc_str);

    string = (UIW_STRING *) Get( FIELD_PULSE );
    string->DataSet(_modem->_pulse_str);

    string = (UIW_STRING *) Get( FIELD_TONE );
    string->DataSet(_modem->_tone_str);

    string = (UIW_STRING *) Get( FIELD_IGNORE_DIALTONE );
    string->DataSet(_modem->_ignore_dial_tone_str);

    string = (UIW_STRING *) Get( FIELD_ESCAPE );
    ZIL_ICHAR itoabuf[40] ;
    itoabuf[0] = _modem->_escape_char;
    itoabuf[1] = '\0';
    string->DataSet(itoabuf);

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_MAX_SPEED );
    itoa( _modem->_max_speed, itoabuf, 10 ) ;
    combo->DataSet(itoabuf);

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_AT );
    if ( _modem->_at )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_V25 );
    if ( _modem->_v25 )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_SB );
    if ( _modem->_sb )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_EC );
    if ( _modem->_ec )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_DC );
    if ( _modem->_dc )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_HW );
    if ( _modem->_hw )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_SW );
    if ( _modem->_sw )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_KS );
    if ( _modem->_ks )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_TB );
    if ( _modem->_tb )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);

}

void K_MODEM_CONFIG::InitSpeedList( void ) 
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_MAX_SPEED ) ;
    ::InitSpeedList( list ) ;
}

void K_MODEM_CONFIG::ApplyChanges()
{
    UIW_STRING * string = (UIW_STRING *) Get( FIELD_INIT );
    strncpy(_modem->_init_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_DIAL_MODE );
    strncpy(_modem->_dial_mode_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_DIAL_PROMPT );
    strncpy(_modem->_dial_mode_prompt_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_DIAL_STR );
    strncpy(_modem->_dial_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_DC_ON );
    strncpy(_modem->_dc_on_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_DC_OFF );
    strncpy(_modem->_dc_off_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_EC_ON );
    strncpy(_modem->_ec_on_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_EC_OFF );
    strncpy(_modem->_ec_off_str,string->DataGet(),61);

//    string = (UIW_STRING *) Get( FIELD_SB_ON );
//    strncpy(_modem->_sb_on_str,string->DataGet(),61);

//    string = (UIW_STRING *) Get( FIELD_SB_OFF );
//    strncpy(_modem->_sb_off_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_AA_ON );
    strncpy(_modem->_aa_on_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_AA_OFF );
    strncpy(_modem->_aa_off_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_SP_ON );
    strncpy(_modem->_sp_on_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_SP_OFF );
    strncpy(_modem->_sp_off_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_VOL1 );
    strncpy(_modem->_vol1_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_VOL2 );
    strncpy(_modem->_vol2_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_VOL3 );
    strncpy(_modem->_vol3_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_HUP );
    strncpy(_modem->_hup_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_HWFC );
    strncpy(_modem->_hwfc_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_SWFC );
    strncpy(_modem->_swfc_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_NOFC );
    strncpy(_modem->_nofc_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_PULSE );
    strncpy(_modem->_pulse_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_TONE );
    strncpy(_modem->_tone_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_IGNORE_DIALTONE );
    strncpy(_modem->_ignore_dial_tone_str,string->DataGet(),61);

    string = (UIW_STRING *) Get( FIELD_ESCAPE );
    _modem->_escape_char = (string->DataGet())[0];

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_MAX_SPEED );
    ZIL_ICHAR * value = combo->DataGet();
    _modem->_max_speed = atoi(value);
    combo = (UIW_COMBO_BOX *) _parent->Get( COMBO_SPEED );
    combo->DataSet( value );

    _modem->_at  = FlagSet(Get( CHECK_AT  )->woStatus, WOS_SELECTED );
    _modem->_v25 = FlagSet(Get( CHECK_V25 )->woStatus, WOS_SELECTED );
    _modem->_sb  = FlagSet(Get( CHECK_SB  )->woStatus, WOS_SELECTED );
    _modem->_ec  = FlagSet(Get( CHECK_EC  )->woStatus, WOS_SELECTED ) ||
	_modem->_ec_on_str[0];
    _modem->_dc  = FlagSet(Get( CHECK_DC  )->woStatus, WOS_SELECTED ) ||
	_modem->_dc_on_str[0];
    _modem->_hw  = FlagSet(Get( CHECK_HW  )->woStatus, WOS_SELECTED ) ||
	_modem->_hwfc_str[0];
    _modem->_sw  = FlagSet(Get( CHECK_SW  )->woStatus, WOS_SELECTED ) ||
	_modem->_swfc_str[0];
    _modem->_ks  = FlagSet(Get( CHECK_KS  )->woStatus, WOS_SELECTED );
    _modem->_tb  = FlagSet(Get( CHECK_TB  )->woStatus, WOS_SELECTED );

    if ( _modem->_ec ) {
	_modem->_correction = 1;
	UIW_BUTTON * button = (UIW_BUTTON *) _parent->Get( CHECK_ERROR_CORRECTION );
	button->woStatus |= WOS_SELECTED;
	button->Information( I_CHANGED_FLAGS, NULL );
    }
    if ( _modem->_dc ) {
	_modem->_compression = 1;
	UIW_BUTTON * button = (UIW_BUTTON *) _parent->Get( CHECK_COMPRESSION );
	button->woStatus |= WOS_SELECTED;
	button->Information( I_CHANGED_FLAGS, NULL );
    }
    if ( _modem->_sb ) {
	_modem->_speed_match = 0;
	UIW_BUTTON * button = (UIW_BUTTON *) _parent->Get( CHECK_SPEED_MATCHING );
	button->woStatus &= ~WOS_SELECTED;
	button->Information( I_CHANGED_FLAGS, NULL );
    }
}

EVENT_TYPE K_MODEM_CONFIG::Event( const UI_EVENT & event )
{
   EVENT_TYPE ccode = event.type ;
   UIW_STRING * string = NULL ;

   switch ( event.type ) {
   case OPT_MODEM_CFG_OK: {
       *windowManager - this ;
       ApplyChanges();
       break;
   }

   case OPT_MODEM_CFG_HELP:
       helpSystem->DisplayHelp( windowManager, HELP_MODEM_CMDS ) ;
       break;

   default:
       ccode = UIW_WINDOW::Event(event);
   }
   return ccode ;
}

K_MODEM_CONFIG::~K_MODEM_CONFIG( void ) 
{
    
}

