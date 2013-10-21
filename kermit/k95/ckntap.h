/* C K N T A P  --  Kermit Telephony interface for MS Win32 TAPI systems */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
          Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#ifdef CK_TAPI
#ifdef COMMENT
#undef CK_TAPI
#endif /* COMMENT */
#else
#ifndef COMMENT
#ifndef NODIAL
#define CK_TAPI
#endif /* NODIAL */
#endif /* COMMENT */
#endif

_PROTOTYP( int cktapiinit, (void) ) ;
_PROTOTYP( int cktapiopen, (void) ) ;
_PROTOTYP( int cktapiclose, (void) ) ;
_PROTOTYP( int cktapiload, (void) ) ;
_PROTOTYP( int cktapiunload, (void) ) ;
_PROTOTYP( int cktapidial, (char *) ) ;
_PROTOTYP( int cktapihangup, (void) ) ;
_PROTOTYP( int cktapianswer, (void) ) ;
_PROTOTYP( int cktapiBuildLineTable, (struct keytab **, struct keytab **, int *) ) ;
_PROTOTYP( int cktapiBuildLocationTable, (struct keytab **, int *) ) ;
_PROTOTYP( int cktapiFetchLocationInfoByID, (int) ) ;
_PROTOTYP( int cktapiFetchLocationInfoByName, (char *) ) ;
_PROTOTYP( int cktapiGetCurrentLocationID, (void) ) ;
_PROTOTYP( void CopyTapiLocationInfoToKermitDialCmd, ( void ));
_PROTOTYP( void cktapiConfigureLine, (int) );
_PROTOTYP( void cktapiDialingProp, (void) );
_PROTOTYP( int cktapiConvertPhoneNumber, (char * source, char ** converted));
_PROTOTYP( int cktapiCallInProgress, (void));
_PROTOTYP( int tapi_open, ( char * ) ) ;
_PROTOTYP( int tapi_clos, ( void ) ) ;
_PROTOTYP( HANDLE GetModemHandleFromLine, ( HLINE ) );
_PROTOTYP( struct mdminf * cktapiGetModemInf, (DWORD,HANDLE) );
_PROTOTYP( void DisplayCommProperties, (HANDLE));
_PROTOTYP( void cktapiDisplayRegistryModemInfo, (LPCSTR));
_PROTOTYP( void cktapiDisplayTapiLocationInfo, (VOID));

#define SUCCESS 0
extern int TAPIAvail ;

#ifdef COMMENT
From "comm/datamodem":

When using the lineGetDevConfig and lineSetDevConfig functions, some service
providers require that the configuration data for this device class have the
following format.
#endif /* COMMENT */
// Device setting information
typedef struct  tagDEVCFGDR  {
  DWORD       dwSize;
  DWORD       dwVersion;
  WORD        fwOptions;
  WORD        wWaitBong;
} DEVCFGHDR;

typedef struct  tagDEVCFG  {
  DEVCFGHDR   dfgHdr;
  COMMCONFIG  commconfig;
} DEVCFG, *PDEVCFG, FAR* LPDEVCFG;

#ifdef COMMENT
Contains device configuration information for use with the lineGetDevConfig
and lineSetDevConfig functions.

dwSize          Sum of the size of the DEVCFGHDR structure and the actual
                size of COMMCONFIG structure.
dwVersion       Version number of the Unimodem DevConfig structure. This
                member can be MDMCFG_VERSION (0x00010003).
fwOptions       Option flags that appear on the Unimodem Option page.
                This member can be a combination of these values:

TERMINAL_PRE  (1)       Displays the preterminal screen.
TERMINAL_POST (2)       Displays the post terminal screen.
MANUAL_DIAL   (4)       Dials the phone manually, if capable of doing so.
LAUNCH_LIGHTS (8)       Displays the modem tray icon.

                Only the LAUNCH_LIGHTS value is set by default.
wWaitBong       Number of seconds (in two seconds granularity) to replace
                the wait for credit tone ($).
commconfig      COMMCONFIGstructure that can be used with the Win32
                communications and MCX functions.
#endif /* COMMENT */

#ifdef COMMENT
From lineSetDevConfig:

Typically, an application will call lineGetID to identify the media stream
device associated with a line, and then call lineConfigDialog to allow the
user to set up the device configuration. It could then call lineGetDevConfig
and save the configuration information in a phone book or other database
associated with a particular call destination. When the user later wants
to call the same destination again, this function lineSetDevConfig can be
used to restore the configuration settings selected by the user.
lineSetDevConfig, lineConfigDialog, and lineGetDevConfigcan be used, in
that order, to allow the user to view and update the settings.

The exact format of the data contained within the structure is specific
to the line and media stream API (device class), is undocumented, and is
undefined. The application must treat it as "opaque" and not manipulate the
contents directly. Generally, the structure can be sent using this function
only to the same device from which it was obtained. Certain Telephony
service providers may, however, permit structures to be interchanged between
identical devices (that is, multiple ports on the same multi-port modem card).
Such interchangability is not guaranteed in any way, even for devices of
the same device class.

Note that some service providers may permit the configuration to be set
while a call is active, and others may not.
#endif /* COMMENT */

#ifdef COMMENT
Service providers that support LINEBEARERMODE_PASSTHROUGH indicate it in the
dwBearerModes member of the LINEDEVCAPS structure. When
LINEBEARERMODE_PASSTHROUGH is indicated, the Unimodem service provider will
also include in the DevSpecific area of the LINEDEVCAPS structure the registry
key used to access information about the modem associated with the line device,
in the following format:
#endif

typedef struct  tagDEVREGKEY {
    DWORD dwContents;   // Set to 1 (indicates containing key)
    DWORD dwKeyOffset;  // Offset to key from start of this
                        // structure (not from start of
                        // LINEDEVCAPS structure). 8 in
                        // our case.
    BYTE rgby[1];       // place containing null-terminated
                        // registry key.
} DEVREGKEY;

#ifdef COMMENT
For example:
    00000001 00000008 74737953 435c6d65  ........System\C
    65727275 6f43746e 6f72746e 7465536c  urrentControlSet
    7265535c 65636976 6c435c73 5c737361  urrentControlSet
    65646f4d 30305c6d xx003030 xxxxxxxx  Modem\0000.

This registry key could then be opened using this function:
RegOpenKey(HKEY_LOCAL_MACHINE, pszDevSpecificRegKey, &phkResult)
#endif

int cktapiGetModemSettings(LPDEVCFG *, LPMODEMSETTINGS *, LPCOMMCONFIG *, DCB **);
int cktapiDisplayModemSettings(LPDEVCFG, LPMODEMSETTINGS, LPCOMMCONFIG, DCB *);
int cktapiSetModemSettings(LPDEVCFG,LPCOMMCONFIG);

