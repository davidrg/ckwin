/*  File CKOSYN.H  --  OS/2 C-Kermit Synchronization function prototypes  */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1996,2004, Trustees of Columbia University in the City of New
  York.
*/

#ifndef CKOSYN_H
#define CKOSYN_H
#ifdef NT
#ifndef APIRET
#define APIRET DWORD
#endif /* APIRET */
#ifndef SEM_INDEFINITE_WAIT
#define SEM_INDEFINITE_WAIT INFINITE
#endif /* SEM_INDEFINITE_WAIT */
#ifndef DCWW_WAIT
#define DCWW_WAIT -1
#endif /* DCWW_WAIT */
#ifndef ERROR_VIO_ROW
#define ERROR_VIO_ROW 1
#endif
#ifndef ERROR_VIO_COL
#define ERROR_VIO_COL 2
#endif
#ifndef CCHMAXPATH
#define CCHMAXPATH _MAX_PATH
#endif
#ifndef TID
#define TID HANDLE
#endif
#ifndef PIB
#define PIB HANDLE
#define PPIB HANDLE*
#endif
#ifndef TIB
#define TIB HANDLE
#define PTIB HANDLE*
#endif
#ifndef HVIO
#define HVIO HANDLE
#endif
#ifndef HMTX
#define HMTX HANDLE
#endif
#ifndef HAB
#define HAB HANDLE
#endif
#ifndef HKBD
#define HKBD HANDLE
#endif
#ifndef HDIR
#define HDIR HANDLE
#endif
#endif /* NT */


/* Synchronization Functions */

_PROTOTYP( APIRET CreateEventAvailSem, (BOOL) ) ;
_PROTOTYP( APIRET PostEventAvailSem, (int) ) ;
_PROTOTYP( APIRET WaitEventAvailSem, (int,unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetEventAvailSem, (int,unsigned long) ) ;
_PROTOTYP( APIRET ResetEventAvailSem, (int) ) ;
_PROTOTYP( APIRET CloseEventAvailSem, ( void ) ) ;

_PROTOTYP( APIRET CreateAlarmSem, (BOOL) ) ;
_PROTOTYP( APIRET PostAlarmSem, (void) ) ;
_PROTOTYP( APIRET WaitAlarmSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetAlarmSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetAlarmSem, (void) ) ;
_PROTOTYP( APIRET CloseAlarmSem, ( void ) ) ;

_PROTOTYP( APIRET CreateTerminalModeSem, (BOOL) ) ;
_PROTOTYP( APIRET PostTerminalModeSem, (void) ) ;
_PROTOTYP( APIRET WaitTerminalModeSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetTerminalModeSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetTerminalModeSem, (void) ) ;
_PROTOTYP( APIRET CloseTerminalModeSem, ( void ) ) ;

_PROTOTYP( APIRET CreateCommandModeSem, (BOOL) ) ;
_PROTOTYP( APIRET PostCommandModeSem, (void) ) ;
_PROTOTYP( APIRET WaitCommandModeSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetCommandModeSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetCommandModeSem, (void) ) ;
_PROTOTYP( APIRET CloseCommandModeSem, ( void ) ) ;

_PROTOTYP( APIRET CreateRdComWrtScrThreadSem, (BOOL) ) ;
_PROTOTYP( APIRET PostRdComWrtScrThreadSem, (void) ) ;
_PROTOTYP( APIRET WaitRdComWrtScrThreadSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetRdComWrtScrThreadSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetRdComWrtScrThreadSem, (void) ) ;
_PROTOTYP( APIRET CloseRdComWrtScrThreadSem, ( void ) ) ;

_PROTOTYP( APIRET CreateConKbdHandlerThreadSem, (BOOL) ) ;
_PROTOTYP( APIRET PostConKbdHandlerThreadSem, (void) ) ;
_PROTOTYP( APIRET WaitConKbdHandlerThreadSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetConKbdHandlerThreadSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetConKbdHandlerThreadSem, (void) ) ;
_PROTOTYP( APIRET CloseConKbdHandlerThreadSem, ( void ) ) ;

_PROTOTYP( APIRET CreateTermScrnUpdThreadSem, (BOOL) ) ;
_PROTOTYP( APIRET PostTermScrnUpdThreadSem, (void) ) ;
_PROTOTYP( APIRET WaitTermScrnUpdThreadSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetTermScrnUpdThreadSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetTermScrnUpdThreadSem, (void) ) ;
_PROTOTYP( APIRET CloseTermScrnUpdThreadSem, ( void ) ) ;

_PROTOTYP( APIRET CreateRdComWrtScrThreadDownSem, (BOOL) ) ;
_PROTOTYP( APIRET PostRdComWrtScrThreadDownSem, (void) ) ;
_PROTOTYP( APIRET WaitRdComWrtScrThreadDownSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetRdComWrtScrThreadDownSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetRdComWrtScrThreadDownSem, (void) ) ;
_PROTOTYP( APIRET CloseRdComWrtScrThreadDownSem, ( void ) ) ;

_PROTOTYP( APIRET CreateConKbdHandlerThreadDownSem, (BOOL) ) ;
_PROTOTYP( APIRET PostConKbdHandlerThreadDownSem, (void) ) ;
_PROTOTYP( APIRET WaitConKbdHandlerThreadDownSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetConKbdHandlerThreadDownSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetConKbdHandlerThreadDownSem, (void) ) ;
_PROTOTYP( APIRET CloseConKbdHandlerThreadDownSem, ( void ) ) ;

_PROTOTYP( APIRET CreateTermScrnUpdThreadDownSem, (BOOL) ) ;
_PROTOTYP( APIRET PostTermScrnUpdThreadDownSem, (void) ) ;
_PROTOTYP( APIRET WaitTermScrnUpdThreadDownSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetTermScrnUpdThreadDownSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetTermScrnUpdThreadDownSem, (void) ) ;
_PROTOTYP( APIRET CloseTermScrnUpdThreadDownSem, ( void ) ) ;

_PROTOTYP( APIRET CreateKbdThreadSem, (BOOL) ) ;
_PROTOTYP( APIRET PostKbdThreadSem, (void) ) ;
_PROTOTYP( APIRET WaitKbdThreadSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetKbdThreadSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetKbdThreadSem, (void) ) ;
_PROTOTYP( APIRET CloseKbdThreadSem, ( void ) ) ;

_PROTOTYP( APIRET CreateKeyMapInitSem, (BOOL) ) ;
_PROTOTYP( APIRET PostKeyMapInitSem, (void) ) ;
_PROTOTYP( APIRET WaitKeyMapInitSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetKeyMapInitSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetKeyMapInitSem, (void) ) ;
_PROTOTYP( APIRET CloseKeyMapInitSem, ( void ) ) ;

_PROTOTYP( APIRET CreateVscrnDirtySem, (BOOL) ) ;
_PROTOTYP( APIRET PostVscrnDirtySem, (int) ) ;
_PROTOTYP( APIRET WaitVscrnDirtySem, (int, unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetVscrnDirtySem, (int, unsigned long) ) ;
_PROTOTYP( APIRET ResetVscrnDirtySem, (int) ) ;
_PROTOTYP( APIRET CloseVscrnDirtySem, ( void ) ) ;

_PROTOTYP( APIRET CreateVscrnTimerSem, (BOOL) ) ;
_PROTOTYP( APIRET PostVscrnTimerSem, (int) ) ;
_PROTOTYP( APIRET WaitVscrnTimerSem, (int,unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetVscrnTimerSem, (int,unsigned long) ) ;
_PROTOTYP( APIRET ResetVscrnTimerSem, (int) ) ;
_PROTOTYP( APIRET CloseVscrnTimerSem, ( void ) ) ;

_PROTOTYP( APIRET CreateKeyStrokeMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestKeyStrokeMutex, (int,unsigned long) ) ;
_PROTOTYP( APIRET ReleaseKeyStrokeMutex, (int) ) ;
_PROTOTYP( APIRET CloseKeyStrokeMutex, (void) ) ;

_PROTOTYP( APIRET CreateKeyboardMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestKeyboardMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseKeyboardMutex, (void) ) ;
_PROTOTYP( APIRET CloseKeyboardMutex, (void) ) ;

_PROTOTYP( APIRET CreateAlarmMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestAlarmMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseAlarmMutex, (void) ) ;
_PROTOTYP( APIRET CloseAlarmMutex, (void) ) ;

_PROTOTYP( APIRET CreateScreenMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestScreenMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseScreenMutex, (void) ) ;
_PROTOTYP( APIRET CloseScreenMutex, (void) ) ;

_PROTOTYP( APIRET CreateVscrnMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestVscrnMutex, (int, unsigned long) ) ;
_PROTOTYP( APIRET ReleaseVscrnMutex, (int) ) ;
_PROTOTYP( APIRET CloseVscrnMutex, (void) ) ;

_PROTOTYP( APIRET CreateVscrnDirtyMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestVscrnDirtyMutex, (int, unsigned long) ) ;
_PROTOTYP( APIRET ReleaseVscrnDirtyMutex, (int) ) ;
_PROTOTYP( APIRET CloseVscrnDirtyMutex, (void) ) ;

_PROTOTYP( APIRET CreateConnectModeMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestConnectModeMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseConnectModeMutex, (void) ) ;
_PROTOTYP( APIRET CloseConnectModeMutex, (void) ) ;

_PROTOTYP( APIRET CreateTelnetMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestTelnetMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseTelnetMutex, (void) ) ;
_PROTOTYP( APIRET CloseTelnetMutex, (void) ) ;

_PROTOTYP( APIRET CreateDebugMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestDebugMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseDebugMutex, (void) ) ;
_PROTOTYP( APIRET CloseDebugMutex, (void) ) ;

_PROTOTYP( APIRET StartAlarmTimer, (unsigned long) ) ;
_PROTOTYP( APIRET StopAlarmTimer, (void) ) ;

_PROTOTYP( APIRET StartVscrnTimer, (unsigned long) ) ;
_PROTOTYP( APIRET StopVscrnTimer, (void) ) ;

_PROTOTYP( APIRET CreateVscrnMuxWait, ( int ) );
_PROTOTYP( APIRET WaitVscrnMuxWait, ( int, unsigned long ) ) ;
_PROTOTYP( APIRET CloseVscrnMuxWait, ( int ) ) ;

_PROTOTYP( APIRET ResetThreadPrty, (void) ) ;
_PROTOTYP( APIRET SetThreadPrty, ( int, int ) ) ;
_PROTOTYP( APIRET KillProcess, ( int ) ) ;

_PROTOTYP( APIRET CreateCtrlCSem, (BOOL, unsigned long*) ) ;
_PROTOTYP( APIRET PostCtrlCSem, (void) ) ;
_PROTOTYP( APIRET WaitCtrlCSem, (unsigned long,unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetCtrlCSem, (unsigned long,unsigned long) ) ;
_PROTOTYP( APIRET ResetCtrlCSem, (unsigned long) ) ;
_PROTOTYP( APIRET CloseCtrlCSem, (unsigned long) ) ;

_PROTOTYP( APIRET CreateAlarmSigSem, (BOOL, unsigned long*) ) ;
_PROTOTYP( APIRET PostAlarmSigSem, (void) ) ;
_PROTOTYP( APIRET WaitAlarmSigSem, (unsigned long,unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetAlarmSigSem, (unsigned long,unsigned long) ) ;
_PROTOTYP( APIRET ResetAlarmSigSem, (unsigned long) ) ;
_PROTOTYP( APIRET CloseAlarmSigSem, (unsigned long) ) ;

_PROTOTYP( APIRET CreateLocalEchoMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestLocalEchoMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseLocalEchoMutex, (void) ) ;
_PROTOTYP( APIRET CloseLocalEchoMutex, (void) ) ;

_PROTOTYP( APIRET CreateLocalEchoAvailSem, (BOOL) ) ;
_PROTOTYP( APIRET PostLocalEchoAvailSem, (void) ) ;
_PROTOTYP( APIRET WaitLocalEchoAvailSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetLocalEchoAvailSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetLocalEchoAvailSem, (void) ) ;
_PROTOTYP( APIRET CloseLocalEchoAvailSem, ( void ) ) ;

_PROTOTYP( APIRET CreateTCPIPMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestTCPIPMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseTCPIPMutex, (void) ) ;
_PROTOTYP( APIRET CloseTCPIPMutex, (void) ) ;

_PROTOTYP( APIRET CreateCommMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestCommMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseCommMutex, (void) ) ;
_PROTOTYP( APIRET CloseCommMutex, (void) ) ;

#ifdef NETCMD
_PROTOTYP( APIRET CreateNetCmdMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestNetCmdMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseNetCmdMutex, (void) ) ;
_PROTOTYP( APIRET CloseNetCmdMutex, (void) ) ;

_PROTOTYP( APIRET CreateNetCmdAvailSem, (BOOL) ) ;
_PROTOTYP( APIRET PostNetCmdAvailSem, (void) ) ;
_PROTOTYP( APIRET WaitNetCmdAvailSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetNetCmdAvailSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetNetCmdAvailSem, (void) ) ;
_PROTOTYP( APIRET CloseNetCmdAvailSem, ( void ) ) ;
#endif

#ifdef NT
_PROTOTYP( APIRET PostSem, (HANDLE) ) ;
_PROTOTYP( APIRET WaitSem, (HANDLE, unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetSem, (HANDLE, unsigned long) ) ;
_PROTOTYP( APIRET ResetSem, (HANDLE) ) ;
#else /* NT */
_PROTOTYP( APIRET PostSem, (HEV) ) ;
_PROTOTYP( APIRET WaitSem, (HEV, unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetSem, (HEV, unsigned long) ) ;
_PROTOTYP( APIRET ResetSem, (HEV) ) ;
#endif /* NT */

#ifdef NT
_PROTOTYP( APIRET CreateCtrlCMuxWait, (unsigned long,HANDLE) );
#else /* NT */
_PROTOTYP( APIRET CreateCtrlCMuxWait, (unsigned long,HEV) ) ;
#endif /* NT */
_PROTOTYP( APIRET WaitCtrlCMuxWait, (unsigned long,unsigned long) ) ;
_PROTOTYP( APIRET CloseCtrlCMuxWait, (unsigned long) ) ;

#ifdef NT
_PROTOTYP( APIRET CreateAlarmSigMuxWait, (unsigned long,HANDLE) );
#else /* NT */
_PROTOTYP( APIRET CreateAlarmSigMuxWait, (unsigned long,HEV) ) ;
#endif /* NT */
_PROTOTYP( APIRET WaitAlarmSigMuxWait, (unsigned long,unsigned long) ) ;
_PROTOTYP( APIRET CloseAlarmSigMuxWait, (unsigned long) ) ;

#ifdef NT
_PROTOTYP( APIRET CreateSerialMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestSerialMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseSerialMutex, (void) ) ;
_PROTOTYP( APIRET CloseSerialMutex, (void) ) ;
#endif /* NT */

_PROTOTYP( APIRET CreateThreadMgmtMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestThreadMgmtMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseThreadMgmtMutex, (void) ) ;
_PROTOTYP( APIRET CloseThreadMgmtMutex, (void) ) ;

_PROTOTYP( APIRET CreateTAPIConnectSem, (BOOL) ) ;
_PROTOTYP( APIRET PostTAPIConnectSem, (void) ) ;
_PROTOTYP( APIRET WaitTAPIConnectSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetTAPIConnectSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetTAPIConnectSem, (void) ) ;
_PROTOTYP( APIRET CloseTAPIConnectSem, ( void ) ) ;

_PROTOTYP( APIRET CreateTAPIAnswerSem, (BOOL) ) ;
_PROTOTYP( APIRET PostTAPIAnswerSem, (void) ) ;
_PROTOTYP( APIRET WaitTAPIAnswerSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetTAPIAnswerSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetTAPIAnswerSem, (void) ) ;
_PROTOTYP( APIRET CloseTAPIAnswerSem, ( void ) ) ;

_PROTOTYP( APIRET CreateTAPIInitSem, (BOOL) ) ;
_PROTOTYP( APIRET PostTAPIInitSem, (void) ) ;
_PROTOTYP( APIRET WaitTAPIInitSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetTAPIInitSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetTAPIInitSem, (void) ) ;
_PROTOTYP( APIRET CloseTAPIInitSem, ( void ) ) ;

_PROTOTYP( APIRET CreateRichEditInitSem, (BOOL) ) ;
_PROTOTYP( APIRET PostRichEditInitSem, (void) ) ;
_PROTOTYP( APIRET WaitRichEditInitSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetRichEditInitSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetRichEditInitSem, (void) ) ;
_PROTOTYP( APIRET CloseRichEditInitSem, ( void ) ) ;

_PROTOTYP( APIRET CreateRichEditCloseSem, (BOOL) ) ;
_PROTOTYP( APIRET PostRichEditCloseSem, (void) ) ;
_PROTOTYP( APIRET WaitRichEditCloseSem, (unsigned long) ) ;
_PROTOTYP( APIRET WaitAndResetRichEditCloseSem, (unsigned long) ) ;
_PROTOTYP( APIRET ResetRichEditCloseSem, (void) ) ;
_PROTOTYP( APIRET CloseRichEditCloseSem, ( void ) ) ;

_PROTOTYP( APIRET CreateRichEditMutex, (BOOL) ) ;
_PROTOTYP( APIRET RequestRichEditMutex, (unsigned long) ) ;
_PROTOTYP( APIRET ReleaseRichEditMutex, (void) ) ;
_PROTOTYP( APIRET CloseRichEditMutex, (void) ) ;
#endif /* CKOSYN_H */
