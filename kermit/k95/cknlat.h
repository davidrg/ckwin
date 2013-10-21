/*
  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#ifndef CKNLAT_H
#define CKNLAT_H

/* From DEC Pathworks 7.0 SDK (DECTAL.H) */

#define TALCALL __declspec(dllimport)

/******************************************************************************
;                                                                             *
; Definition Declarations provided in DECTAL.DLL                              *
;                                                                             *
******************************************************************************/
#define SERVICENAME_LENGTH    256      /* max Length of service name         */
#define PASSWORD_LENGTH        32      /* max Length of password             */

#define LAT_ACCESS              1       /* Access the LAT driver            */
#define TELNET_ACCESS           2       /* Access the TELNET driver         */
#define CTERM_ACCESS            3       /* Access the CTERM driver          */

typedef struct _TelnetConnectData {
    LPSTR   TCD_Terminal_Type;
    DWORD   TCD_Port_Number;
} Telnet_Connect_Data;

/***************************************************************************/
/***************************************************************************/

/* Error Return Codes */

#define TAL_SUCCESS           0  /* Generic success return value */
#define TAL_SESSIONFAIL      -1  /* invalid session id, or session gone */
                                 /* or connect failed */
#define TAL_DISCONNECT       -2  /* session ended (prematurely) */
#define TAL_NOTINSTALLED     -3  /* driver not installed */
#define TAL_NOMORENAMES      -4  /* GetNextService ran out of names */
#define TAL_BADPARAMETER     -5  /* input parameter bad */
#define TAL_NORESOURCE       -6  /* ran out of a resource */
#define TAL_BADCONNDATA      -7  /* for LAT, invalid password */
                                 /* for CTERM, ConnectData not NULL */
                                 /* for TELNET, invalid port number */
#define TAL_TXQUEFULL        -8  /* unable to transmit on write */
#define TAL_OTHERERROR       -9  /* use GetDetailError for details */

#endif /* CKNLAT_H */

