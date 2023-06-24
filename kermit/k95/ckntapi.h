//
// Created by david on 29/04/2023.
//

#ifndef CKWIN_CKNTAPI_H
#define CKWIN_CKNTAPI_H

typedef LONG (WINAPI *cklineInitialize_t)(LPHLINEAPP, HINSTANCE, LINECALLBACK, LPCSTR, LPDWORD );
typedef LONG (WINAPI *cklineNegotiateAPIVersion_t)(HLINEAPP, DWORD, DWORD, DWORD, LPDWORD, LPLINEEXTENSIONID);
typedef LONG (WINAPI *cklineGetDevCaps_t)(HLINEAPP, DWORD, DWORD, DWORD, LPLINEDEVCAPS);
typedef LONG (WINAPI *cklineShutdown_t)(HLINEAPP);
typedef LONG (WINAPI *cklineOpen_t)(HLINEAPP, DWORD, LPHLINE, DWORD, DWORD, DWORD, DWORD, DWORD,
    LPLINECALLPARAMS);
typedef LONG (WINAPI *cklineMakeCall_t)(HLINE hLine, LPHCALL lphCall, LPCSTR lpszDestAddress,
        DWORD dwCountryCode, LPLINECALLPARAMS const lpCallParams);
typedef LONG (WINAPI *cklineDial_t)(HCALL hCall, LPCSTR lpszDestAddress, DWORD dwCountryCode);
typedef LONG (WINAPI *cklineDrop_t)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize);
typedef LONG (WINAPI *cklineAnswer_t)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize);
typedef LONG (WINAPI *cklineAccept_t)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize);
typedef LONG (WINAPI *cklineDeallocateCall_t)(HCALL hCall);
typedef LONG (WINAPI *cklineSetCallPrivilege_t)(HCALL,DWORD);
typedef LONG (WINAPI *cklineClose_t)(HLINE hLine);
typedef LONG (WINAPI *cklineHandoff_t)(HCALL,LPCSTR,DWORD);
typedef LONG (WINAPI *cklineGetID_t)(HLINE hLine, DWORD dwAddressID, HCALL hCall,
        DWORD dwSelect, LPVARSTRING lpDeviceID, LPCSTR lpszDeviceClass) ;
typedef LONG (WINAPI *cklineGetTranslateCaps_t)( HLINEAPP hLineApp, DWORD,
        LPLINETRANSLATECAPS lpLineTranslateCaps);
typedef LONG (WINAPI *cklineSetCurrentLocation_t)( HLINEAPP hLineApp, DWORD dwLocationID );
typedef LONG (WINAPI *cklineSetStatusMessages_t)( HLINE hLine, DWORD dwLineStates,
DWORD dwAddressStates );
typedef LONG (WINAPI *cklineConfigDialog_t)( DWORD dwLine, HWND hwin, LPCSTR lpszTypes );
typedef LONG (WINAPI *cklineTranslateDialog_t)( HLINEAPP hTAPI, DWORD dwLine, DWORD dwVersionToUse,
        HWND hwndOwner, LPCSTR lpszAddressIn );
typedef LONG (WINAPI *cklineTranslateAddress_t)( HLINEAPP hTAPI, DWORD dwLine, DWORD dwVersionToUse,
        LPCSTR lpszAddressIn, DWORD dwCard,
        DWORD dwTranslateOptions,
LPLINETRANSLATEOUTPUT lpTranslateOutput);
typedef LONG (WINAPI *cklineGetCountry_t)( DWORD, DWORD, LPLINECOUNTRYLIST );
typedef LONG (WINAPI *cklineGetDevConfig_t)(DWORD, LPVARSTRING, LPCSTR);
typedef LONG (WINAPI *cklineGetLineDevStatus_t)(HLINE hLine,LPLINEDEVSTATUS lpLineDevStatus);
typedef LONG (WINAPI *cklineSetDevConfig_t)(DWORD,LPVOID const,DWORD,LPCSTR);
typedef LONG (WINAPI *cklineGetCallInfo_t)(HCALL, LPLINECALLINFO);
typedef LONG (WINAPI *cklineMonitorMedia_t)(HCALL,DWORD);
typedef LONG (WINAPI *cklineGetAppPriority_t)(LPCSTR,DWORD,LPLINEEXTENSIONID,
DWORD,LPVARSTRING,LPDWORD);
typedef LONG (WINAPI *cklineSetAppPriority_t)(LPCSTR,DWORD,LPLINEEXTENSIONID,
DWORD,LPCSTR,DWORD);
typedef LONG (WINAPI *cklineGetNumRings_t)(HLINE,DWORD,LPDWORD);
typedef LONG (WINAPI *cklineSetNumRings_t)(HLINE,DWORD,DWORD);
typedef LONG (WINAPI *cklineSetCallParams_t)(HCALL,DWORD,DWORD,DWORD,LPLINEDIALPARAMS);

#endif //CKWIN_CKNTAPI_H
