# Index to the C-Kermit for Windows Codebase

The goal of this page is to help find relevant code a little bit faster as the
various filenames are not always obvious.

## /kermit/k95

This directory contains the code for all the text-mode applications, all of the DLLs 
(except p95.dll), and the Windows GUI (k95g.exe, in the kui subdirectory)

### Code shared with C-Kermit for Other Platforms

This code all comes from C-Kermit for UNIX. Its syncronised with upstream on a regular basis.

| Filename            | Title                                                                 | Notes                                                          |
|---------------------|-----------------------------------------------------------------------|----------------------------------------------------------------|
| ckcasc.h            | Mnemonics for ASCII control characters                                |                                                                |
| ckcdeb.h            |                                                                       | This header is included *everywhere*                           |
| ckcfn2.c            | System-independent Kermit protocol support functions, part 2          |                                                                |
| ckcfn3.c            | Packet buffer management for C-Kermit                                 |                                                                |
| ckcfns.c            | System-independent Kermit protocol support functions, part 1          |                                                                |
| ckcftp.c            | FTP Client for C-Kermit                                               |                                                                |
| ckcftp.h            | Prototypes for static functions defined in ckcftp.c                   |                                                                |
| ckcker.h            | Symbol and macro definitions for C-Kermit                             |                                                                |
| ckclib.c            | C-Kermit Library routines                                             |                                                                |
| ckclib.h            | C-Kermit library routine prototypes                                   |                                                                |
| ckcmai.c            | Main program for C-Kermit plus some miscellaneous functions           |                                                                |
| ckcmdb.c            | malloc debugger                                                       |                                                                |
| ckcnet.c            | Network support                                                       |                                                                |
| ckcnet.h            | Symbol and macro definitions for C-Kermit network support             |                                                                |
| ckcpro.c            | C-Kermit Protocol Module                                              | Formerly generated from ckcpro.w with wart                     |
| ckcsig.h            | Definitions and prototypes for signal handling                        |                                                                |
| ckcssl.h            |                                                                       | Mostly redefines a bunch of OpenSSL stuff with a ck_ prefix    |
| ckcsym.h            |                                                                       | Not used                                                       |
| ckctel.c            | Telnet support                                                        |                                                                |
| ckctel.h            | Symbol and macro definitions for C-Kermit telnet support              |                                                                |
| ckcuni.c            | Unicode/Terminal character-set translations                           |                                                                |
| ckcuni.h            | Unicode/Terminal character-set translations                           |                                                                |
| ckcxla.h            | System-independent character-set translation header file for C-Kermit |                                                                |
| cku2tm.c            |                                                                       |                                                                |
| ckuat2.h            | Kerberos headers for C-Kermit                                         |                                                                |
| ckuath.c            | Authentication for C-Kermit                                           |                                                                |
| ckuath.h            | "C-Kermit to Authentication" interface                                |                                                                |
| ckucmd.c            | Interactive command package for Unix                                  |                                                                |
| ckucmd.h            | Header file for interactive command parser                            |                                                                |
| ckucns.c            | Terminal connection to remote system, for UNIX                        |                                                                |
| ckucon.c            | Terminal connection to remote system, for UNIX                        |                                                                |
| ckudia.c            | Module for automatic modem dialing                                    |                                                                |
| ckupty.c            | C-Kermit pseudoterminal control functions for UNIX                    |                                                                |
| ckupty.h            | Includes and definitions for ckupty.c                                 |                                                                |
| ckuscr.c            | expect-send script implementation                                     |                                                                |
| ckusig.c            | Kermit signal handling for Unix and OS/2 systems                      |                                                                |
| ckusig.h            | Definitions and prototypes for signal handling                        |                                                                |
| ckustr.c            | string extraction/restoration routines                                |                                                                |
| ckuus2.c            | User interface strings & help text module for C-Kermit                |                                                                |
| ckuus3.c            | "User Interface" for C-Kermit, part 3                                 |                                                                |
| ckuus4.c            | "User Interface" for C-Kermit, part 4                                 |                                                                |
| ckuus5.c            | "User Interface" for C-Kermit, part 5                                 |                                                                |
| ckuus6.c            | "User Interface" for C-Kermit, part 6                                 |                                                                |
| ckuus7.c            | "User Interface" for C-Kermit, part 7                                 |                                                                |
| ckuusr.c            | "User Interface" for C-Kermit (Part 1)                                |                                                                |
| ckuusr.h            | Symbol definitions for C-Kermit ckuus*.c modules                      |                                                                |
| ckuusx.c            | "User Interface" common functions.                                    |                                                                |
| ckuusy.c            | "User Interface" for C-Kermit Kermit, part Y                          |                                                                |
| ckuver.h            | C-Kermit UNIX Version heralds                                         |                                                                |
| ckuxla.c            | C-Kermit tables and functions supporting character set translation.   |                                                                |
| ckuxla.h            | C-Kermit tables and functions supporting character set translation.   |                                                                |
| ck_crp.c            | Encryption Engine                                                     | Mostly DES/CAST stuff for telnet                               |
| ck_des.c            | libDES interface for Kermit 95                                        |                                                                |
| ck_ssl.c            | OpenSSL Interface for C-Kermit                                        |                                                                |
| ck_ssl.h            | OpenSSL Interface Header for C-Kermit                                 |                                                                |
| README.TXT          | C-Kermit Readme file                                                  | for the cross-platform Unix/linux/VMS version, not windows/os2 |

### C-Kermit for Windows and OS/2

This code is used on both Windows and OS/2.

| Filename            | Title                                                      | Notes                                                                                                                          |
|---------------------|------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------|
| ckoadm.c            | ADM-3A Emulation                                           |                                                                                                                                |
| ckoadm.h            | ADM-3A Emulation                                           |                                                                                                                                |
| ckoath.c            | Authentication for Kermit 95                               |                                                                                                                                |
| ckoath.h            | Authentication for Kermit 95                               |                                                                                                                                |
| ckoava.c            | AVATAR Emulation                                           |                                                                                                                                |
| ckoava.h            | Header for AVATAR Emulation                                |                                                                                                                                |
| ckoco2.c            | Kermit connect command for OS/2 systems and Windows        |                                                                                                                                |
| ckoco3.c            | Kermit connect command for OS/2 systems and Windows        | A lot of escape sequence processing for the terminal emulator happens in here                                                  |
| ckoco4.c            | Kermit connect command for OS/2 systems and Windows        |                                                                                                                                |
| ckoco5.c            | Kermit connect command for OS/2 systems and Windows        |                                                                                                                                |
| ckocon.c            | Kermit connect command for OS/2 systems and Windows        |                                                                                                                                |
| ckocon.h            | OS/2 C-Kermit keyboard verb definitions & tables.          |                                                                                                                                |
| ckodg.c             | Data General Emulation                                     |                                                                                                                                |
| ckodg.h             | Header for Data General Emulation                          |                                                                                                                                |
| ckodir.h            |                                                            | A public domain implementation of BSD directory routines for MS-DOS                                                            |
| ckoetc.c            | OS/2 etc module                                            | Contains some encryption bits                                                                                                  |
| ckoetc.h            | OS/2 etc module                                            |                                                                                                                                |
| ckoetc2.c           | Blowfish algorithm                                         |                                                                                                                                |
| ckofio.c            | Kermit file system support for OS/2                        |                                                                                                                                |
| ckohp.c             | Hewlett-Packard Emulation                                  |                                                                                                                                |
| ckohp.h             | Header for Hewlett-Packard Emulation                       |                                                                                                                                |
| ckohzl.c            | Hazeltine 1500 Emulation                                   |                                                                                                                                |
| ckohzl.h            | Header for Hazeltine Emulation                             |                                                                                                                                |
| ckoi31.c            | IBM 31x1 Emulation                                         |                                                                                                                                |
| ckoi31.h            | Header for IBM 31x1 Emulation                              |                                                                                                                                |
| ckoker.h            |                                                            | some common definitions for C-Kermit on OS/2 and Windows                                                                       |
| ckoker.mak          | Makefile                                                   | Makefile for C-Kermit on OS/2 and Windows                                                                                      |
| ckokey.c            | Kermit Keyboard support for OS/2 and Win32 Systems         |                                                                                                                                |
| ckokey.h            | Kermit Keyboard support for OS/2 and Win32 Systems         |                                                                                                                                |
| ckokvb.h            |                                                            | Keyboard key definitions                                                                                                       |
| ckolat.h            |                                                            | LAT support definitions                                                                                                        |
| ckomou.c            | Kermit mouse support for OS/2 systems                      |                                                                                                                                |
| ckonbi.h            | NetBios support                                            |                                                                                                                                |
| ckonet.c            | OS/2 and Win32 specific network support                    |                                                                                                                                |
| ckonet.h            | OS/2 and Win32 specific network support                    |                                                                                                                                |
| ckop.c              |                                                            | P.DLL/P95.DLL support                                                                                                          |
| ckop.h              |                                                            | P.DLL/P95.DLL support                                                                                                          |
| ckoqnx.c            | QNX Emulation                                              |                                                                                                                                |
| ckoqnx.h            | Header for QNX Emulation                                   |                                                                                                                                |
| ckoreg.c            | Kermit interface for MS Win32 Registry                     |                                                                                                                                |
| ckoreg.h            | Kermit interface for MS Win32 Registry                     |                                                                                                                                |
| ckosftp.c           | SFTP support                                               | Not currently used                                                                                                             |
| ckosftp.h           | SFTP support                                               | Not currently used                                                                                                             |
| ckosig.c            | Kermit signal handling for OS/2 and Win32 systems          |                                                                                                                                |
| ckoslp.c            | Kermit interface to the IBM SLIP driver                    |                                                                                                                                |
| ckoslp.h            | Kermit interface to the IBM SLIP driver                    |                                                                                                                                |
| ckossl.c            | OpenSSL Interface for Kermit 95                            |                                                                                                                                |
| ckossl.h            | OpenSSL Interface for Kermit 95                            |                                                                                                                                |
| ckosslc.c           | OpenSSL Interface for Kermit 95                            |                                                                                                                                |
| ckosslc.h           | OpenSSL Interface for Kermit 95                            |                                                                                                                                |
| ckosyn.c            | Kermit synchronization functions for OS/2 and NT systems   |                                                                                                                                |
| ckosyn.h            | OS/2 C-Kermit Synchronization function prototypes          |                                                                                                                                |
| ckotek.c            | Tektronix Emulation                                        |                                                                                                                                |
| ckotek.h            | Tektronix Emulation                                        |                                                                                                                                |
| ckothr.c            | Kermit thread management functions for OS/2 and NT systems |                                                                                                                                |
| ckothr.h            | Kermit thread management functions for OS/2 and NT systems |                                                                                                                                |
| ckotio.c            | Kermit communications I/O support for OS/2 systems         |                                                                                                                                |
| ckotvi.c            | Televideo Emulation                                        |                                                                                                                                |
| ckotvi.h            | Header for Televideo Emulation                             |                                                                                                                                |
| ckouni.h            | Unicode/Terminal character-set translations                |                                                                                                                                |
| ckovc.c             | Volker Craig VC4404/404 Emulation                          |                                                                                                                                |
| ckovc.h             | Header for Volker Craig VC4404/404 Emulation               |                                                                                                                                |
| ckowin.h            | Kermit GUI Windows support - Generic                       |                                                                                                                                |
| ckowys.c            | Wyse Emulation                                             |                                                                                                                                |
| ckowys.h            | Header for Wyse Emulations                                 |                                                                                                                                |
| ckozli.c            | ZLIB Interface Source for Kermit 95                        | Not used?                                                                                                                      |
| ckozli.h            | ZLIB Interface Source for Kermit 95                        | Not used?                                                                                                                      |
| compiler_detect.mak |                                                            | Makefile that tries to detect the compiler currently in use                                                                    |
| feature_flags.mak   |                                                            | Makefile for processing enabled/disabled feature flags                                                                         |
| getopt.c            |                                                            | Used by SRP support                                                                                                            |
| getopt.h            |                                                            | Used by SRP support                                                                                                            |
| p.h                 |                                                            | P.DLL/P95.DLL interface header                                                                                                 |
| p_brw.c             |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_brw.h             |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_callbk.c          |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_callbk.h          |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_common.c          |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_common.h          |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_dir.c             |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_dir.h             |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_errmsg.h          |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_error.c           |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_error.h           |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_global.c          |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_global.h          |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_module.h          |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_omalloc.c         |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_omalloc.h         |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_tl.c              |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_tl.h              |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| p_type.h            |                                                            | P.DLL/P95.DLL interface                                                                                                        |
| rlogin.c            | Rlogin stub for Kermit 95                                  |                                                                                                                                |
| setup.c             | Kermit 95: Text-mode setup program                         | setup.exe: The old Kermit 95 setup program. I assume this was only used on 1.x - the final release (2.1.3) used InstallShield. |
| srp-passwd.c        |                                                            | srp-passwd.exe: part of SRP support                                                                                            |
| srp-tconf.c         |                                                            | srp-tconf.exe: part of SRP support                                                                                             |
| telnet.c            | Telnet stub for Kermit 95                                  |                                                                                                                                |
| textps.c            | Convert plain text to Postscript                           | textps.exe                                                                                                                     |

### Windows-Only

This code and other stuff is only used on Windows (if its absent, OS/2 should still build fine)

| Filename            | Title                                                | Notes                                                                                     |
|---------------------|------------------------------------------------------|-------------------------------------------------------------------------------------------|
| cknalm.c            | Kermit alarm functions for Windows 95 systems        |                                                                                           |
| cknalm.h            | Kermit alarm functions for Windows 95 systems        |                                                                                           |
| ckndde.c            | Kermit Dynamic Data Exchange support for Win32       | unused?                                                                                   |
| cknker.def          |                                                      | Module definition file for k95.exe/cknker.exe                                             |
| cknker.exe.manifest |                                                      |                                                                                           |
| cknker.ico          |                                                      |                                                                                           |
| cknker.rc           |                                                      | Resource script for k95.exe/cknker.exe                                                    |
| cknlat.h            |                                                      | DECnet/LAT support for Kermit 95                                                          |
| cknnbi.c            | Win32-specific NetBios support                       |                                                                                           |
| cknprt.c            |                                                      | Win32 Printing support?                                                                   |
| cknpty.c            | Windows ConPTY Interface for C-Kermit                |                                                                                           |
| cknpty.h            | Windows ConPTY Interface for C-Kermit                |                                                                                           |
| cknsig.c            | Kermit signal handling for Win32 systems             |                                                                                           |
| ckntap.c            | Kermit Telephony interface for MS Win32 TAPI systems |                                                                                           |
| ckntap.h            | Kermit Telephony interface for MS Win32 TAPI systems |                                                                                           |
| ckntel.c            | Kermit Telephony interface for MS Win32 TAPI systems |                                                                                           |
| ckntel.h            | Kermit Telephony interface for MS Win32 TAPI systems |                                                                                           |
| cknver.h            |                                                      | Internal version numbers used by C-Kermit for Windows                                     |
| cknwin.c            | Kermit GUI Windows support for Win32 systems         |                                                                                           |
| cknwin.h            | Kermit GUI Windows support for Win32 systems         |                                                                                           |
| ckorbf.c            |                                                      | A simple ring-buffer implementation for the SSH Subsystem                                 |
| ckorbf.h            |                                                      | A simple ring-buffer implementation for the SSH Subsystem                                 |
| ckoshs.c            | C-Kermit for Windows SSH Subsystem                   |                                                                                           |
| ckoshs.h            | C-Kermit for Windows SSH Subsystem                   |                                                                                           |
| ckossh.c            | SSH Subsystem Interface for C-Kermit                 |                                                                                           |
| ckossh.h            | SSH Subsystem Interface for C-Kermit                 |                                                                                           |
| ck_crp.def          |                                                      | Module definition file for k95crypt.dll                                                   |
| clean-dist.bat      |                                                      | Cleans the dist directory                                                                 |
| clean.bat           |                                                      | Deletes all files produced by a windows build                                             |
| conptydll.c         |                                                      | Example Net DLL that uses ConPTY                                                          |
| conptydll.def       |                                                      | Example Net DLL that uses ConPTY                                                          |
| conptydll.mak       |                                                      | Example Net DLL that uses ConPTY                                                          |
| ctl3dins.c          |                                                      | ctl3dins.exe: Tool for installing ctl3dins and maybe the C Runtime on windows             |
| iksd.c              |                                                      | Utility to start the Internet Kermit Service as a user process (for Windows 9x)           |
| iksdsvc.c           | Windows Internet Kermit Service Daemon               | Windows NT Service                                                                        |
| k95.ini             |                                                      | Default initialisation file for Windows                                                   |
| k95custom.ini       |                                                      | Default user customisation file for Windows                                               |
| k95d.c              |                                                      | k95d.exe - Host mode server                                                               |
| k95d.cfg            |                                                      | Configuration file for k95d.exe                                                           |
| k95f.ico            |                                                      |                                                                                           |
| k95g.exe.manifest   |                                                      | Gives the GUI version of CKW modern UI controls on Windows XP and newer                   |
| k95g.ico            |                                                      |                                                                                           |
| mk.bat              |                                                      | Windows build script - builds all GUI + console                                           |
| mkdist-os2.bat      |                                                      | Windows to OS/2 cross compile build script - copies built artifacts to an os2 dist folder |
| mkdist.bat          |                                                      | Windows build script - copies build results to a dist folder                              |
| mkg.bat             |                                                      | Windows build script - builds the GUI (k95g.exe)                                          |
| mkgd.bat            |                                                      | Windows build script - debug build of the GUI (k95g.exe)                                  |
| mknt.bat            |                                                      | Windows build script - everything but k95g.exe                                            |
| mkntd.bat           |                                                      | Windows build script - debug build of everything but k95g.exe                             |
| mkos2.bat           |                                                      | Windows build script - build C-Kermit for OS/2 on a Windows host (cross-compile)          |
| mkos2d.bat          |                                                      | Windows build script - debug build of C-Kermit for OS/2 on a WIndows host (cross-compile) |
| se.c                |                                                      | se.exe - shell execute utility                                                            |
| settapi.c           |                                                      | setup.exe: TAPI code for the old console-based install program                            |
| welcome.txt         |                                                      | Welcome banner shown from the default k95custom.ini                                       |
| wsetup.def          | C-Kermit for Windows NT Setup                        | setup.exe module definition file                                                          |
| wtelnet.def         | C-Kermit for Win32 Telnet                            | telnet.exe module definition file                                                         |

#### GUI Only
| Filename            | Title                                    | Notes                                                                                         |
|---------------------|------------------------------------------|-----------------------------------------------------------------------------------------------|
| ckcxxx.c            | Kermit 95 variable declarations for GUI  |                                                                                               |
| ckcxxx.h            | Kermit 95 variable declarations for GUI  |                                                                                               |
| resource.h          |                                          | Resource header for kui.rc (part of k95g.exe)                                                 |
| kui/altx.bmp        |                                          | Terminal/command icon. Not used?                                                              |
| kui/bitmap1.bmp     | IDB_BITMAP1                              |                                                                                               |
| kui/bmp00001.bmp    | IDR_TOOLBARCOMMAND                       |                                                                                               |
| kui/bmp00003.bmp    |                                          | Phone hangup icon. Not used.                                                                  |
| kui/ckcxxx.h        | Kermit 95 variable declarations for GUI  |                                                                                               |
| kui/dial.bmp        |                                          | Not used?                                                                                     |
| kui/icon1.ico       |                                          |                                                                                               |
| kui/iconk95.ico     |                                          |                                                                                               |
| kui/ikcmd.c         |                                          |                                                                                               |
| kui/ikcmd.h         |                                          |                                                                                               |
| kui/ikextern.h      |                                          |                                                                                               |
| kui/ikterm.cxx      | Class: IKTerm, Constructed by: KClient   | Terminal interaction, keyboard events, etc                                                    |
| kui/ikterm.h        | Class: IKTerm, Constructed by: KClient   |                                                                                               |
| kui/ikui.cxx        |                                          | C Interface to KUI (Functions called by the rest of CKW to do GUI things)                     |
| kui/ikui.h          |                                          |                                                                                               |
| kui/k95.ico         |                                          |                                                                                               |
| kui/k95a.ico        |                                          |                                                                                               |
| kui/k95f.ico        |                                          |                                                                                               |
| kui/k95g.ico        |                                          |                                                                                               |
| kui/kabout.cxx      | Class: KAbout                            |                                                                                               |
| kui/kabout.hxx      | Class: KAbout                            | handles the display of the about box                                                          |
| kui/kappwin.cxx     | Class: KAppWin                           | Window sizing, display user manual                                                            |
| kui/kappwin.hxx     | Class: KAppWin                           | base class for application windows.                                                           |
| kui/karray.cxx      | Class: KArray                            |                                                                                               |
| kui/karray.hxx      | Class: KArray                            | simple utility array class used to hold anything                                              |
| kui/kclient.cxx     | Class: KClient                           | Terminal drawing code lives here.                                                             |
| kui/kclient.hxx     | Class: KClient                           | base class for main window's client area.                                                     |
| kui/kcmd.h          |                                          |                                                                                               |
| kui/kcustdlg.cxx    | Class: KStatusCustomDlg                  | Not currently user-accessible (menu item not present)                                         |
| kui/kcustdlg.hxx    | Class: KStatusCustomDlg                  | status bar customization dialog box.                                                          |
| kui/kdefs.h         |                                          | Misc defs                                                                                     |
| kui/kdwnload.cxx    | Class: KDownLoad                         | File dialog primarily used for saving file downloads                                          |
| kui/kdwnload.hxx    | Class: KDownLoad                         | download dialog box                                                                           |
| kui/kermit95.bmp    |                                          |                                                                                               |
| kui/kflstat.cxx     | Class: KFileStatus                       |                                                                                               |
| kui/kflstat.h       |                                          | Screen line numbers for status dialog                                                         |
| kui/kflstat.hxx     | Class: KFileStatus                       | file transfer status dialog box                                                               |
| kui/Kfont.cxx       | Class: KFont                             |                                                                                               |
| kui/kfont.hxx       | Class: KFont                             | a font used by main app window.                                                               |
| kui/kfontdlg.cxx    | Class: KFontDialog                       | Not currently user-accessible (menu item not present)                                         |
| kui/kfontdlg.hxx    | Class: KFontDialog                       | font dialog box                                                                               |
| kui/khwndset.cxx    | Class: KHwndSet                          |                                                                                               |
| kui/khwndset.hxx    | Class: KHwndSet                          | keeps track of KWin - HWND pairings                                                           |
| kui/kmenu.cxx       | Class: KMenu                             |                                                                                               |
| kui/kmenu.hxx       | Class: KMenu                             | main window's menu bar.                                                                       |
| kui/kprogres.cxx    | Class: KProgress                         | Progress bar used on the file transfer status window                                          |
| kui/kprogres.hxx    | Class: KProgress                         |                                                                                               |
| kui/kregedit.cxx    | Class: KRegistryEdit                     | set/get Font and coordinates to/from the registry                                             |
| kui/kregedit.hxx    | Class: KRegistryEdit                     | win95 registry functions                                                                      |
| kui/kscroll.cxx     | Class: KScroll                           |                                                                                               |
| kui/kscroll.hxx     | Class: KScroll                           | client area scrollbar.                                                                        |
| kui/kstatus.cxx     | Class: KStatus                           |                                                                                               |
| kui/kstatus.hxx     | Class: KStatus                           | main window's status bar.                                                                     |
| kui/ksysmets.cxx    | Class: KSysMetrics                       |                                                                                               |
| kui/ksysmets.hxx    | Class: KSysMetrics                       | System Metrics encapsulation                                                                  |
| kui/kszpopup.cxx    | Class: KSizePopup                        |                                                                                               |
| kui/kszpopup.hxx    | Class: KSizePopup                        | Popup that shows window size during sizing event                                              |
| kui/ktermin.cxx     | Class: KTerminal                         | owns & configures the toolbar, status bar, menu bar, etc. Processes toolbar & menu events     |
| kui/ktermin.hxx     | Class: KTerminal                         | terminal window (the primary user interface window)                                           |
| kui/ktoolbar.cxx    | Class: KToolBar                          |                                                                                               |
| kui/ktoolbar.hxx    | Class: KToolBar                          | main window's toolbar.                                                                        |
| kui/kui.cxx         | Class: Kui                               |                                                                                               |
| kui/kui.hxx         | Class: Kui                               | message redirector for all main windows. Used for terminal, command and clientserver windows. |
| kui/kui.rc          |                                          | Resource script                                                                               |
| kui/kuidef.cxx      |                                          |                                                                                               |
| kui/kuidef.h        |                                          | K_GLOBAL, K_CREATEINFO, various consts and other such things                                  |
| kui/kuikey.cxx      |                                          | static key array used to process keystrokes in KClient                                        |
| kui/kwin.cxx        | Class: KWin                              |                                                                                               |
| kui/kwin.hxx        | Class: KWin                              | base class for display gui windows.  Used to encapsulate basic gui functionality.             |
| kui/makefile        |                                          | Makefile for KUI stuff                                                                        |
| kui/mkg.bat         |                                          |                                                                                               |
| kui/partdrag.cur    |                                          |                                                                                               |
| kui/resource.h      |                                          |                                                                                               |
| kui/toolbar1.bmp    |                                          |                                                                                               |
| kui/toolbar2.bmp    | IDR_TOOLBARTERMINAL                      |                                                                                               |

### OS/2 Only

This code is only used on OS/2 (if its absent, Windows should still build fine)

| Filename            | Title                                                                   | Notes                                                         |
|---------------------|-------------------------------------------------------------------------|---------------------------------------------------------------|
| ckermit.ico         |                                                                         | OS.2 icon                                                     |
| cko32i20.def        |                                                                         | IBM TCP/IP 2.0 interface DLL for C-Kermit (32-bit)            |
| cko32rt.c           |                                                                         | Dummy module so we can link resources to it                   |
| ckoclip.c           |                                                                         | clipboard app for OS/2                                        |
| ckoclip.def         |                                                                         | clipboard app for OS/2                                        |
| ckoclip.h           |                                                                         | clipboard app for OS/2                                        |
| ckoclip.ico         |                                                                         | clipboard app for OS/2                                        |
| ckoclip.rc          |                                                                         | clipboard app for OS/2                                        |
| ckoftp.c            | TCP/IP interface DLL between C-Kermit and FTP PC/TCP                    |                                                               |
| ckoibm.c            | TCP/IP interface DLL between C-Kermit and IBM TCP/IP                    |                                                               |
| ckoker.msb          |                                                                         | No idea what this is, but it *was* referenced in the makefile |
| ckoker.rc           |                                                                         | OS/2 Resource Script                                          |
| ckoker32.def        |                                                                         | C-Kermit for OS/2 (32-bit)                                    |
| ckonbi.c            | OS/2-specific NetBios support                                           |                                                               |
| ckonov.c            | TCP/IP interface DLL between C-Kermit and Novell LAN Workplace OS/2 3.0 |                                                               |
| ckopcf.c            |                                                                         | pcfonts.dll: Dummy module so we can link resources to it      |
| ckopcf.h            |                                                                         | pcfonts.dll: defines PC Font IDs for resource file            |
| ckotcp.c            | TCP/IP interface DLL between C-Kermit and IBM TCP/IP                    |                                                               |
| ckotcp.h            | TCP/IP interface for OS/2 C-Kermit                                      |                                                               |
| ckotel.c            | Kermit stub to fake IBM Telnet and IBM TelnetPM                         | telnet.exe and telnetpm.exe for OS/2                          |
| ckotel.def          |                                                                         | Telnet front-end for C-Kermit for OS/2                        |
| k2.ini              |                                                                         | Default initialisation file for OS/2                          |
| k2crypt.def         |                                                                         | Module definition file for k2crypt.dll                        |
| osetup.def          | Kermit/2 Setup                                                          | Module definition file                                        |
| k95f_os2.ico        |                                                                         |                                                               |
| k95g_os2.ico        |                                                                         |                                                               |
| mkos2.cmd           |                                                                         | OS/2 build script - build everything                          |
| mkos2d.cmd          |                                                                         | OS/2 build script - debug build of everything                 |

### Misc files

Some of these should probably be included in the distribution, some are just nots and other misc things.

| Filename            | Title                                                        | Notes                                                                      |
|---------------------|--------------------------------------------------------------|----------------------------------------------------------------------------|
| 00README.TXT        | KERMIT 95 SOURCE FILES                                       | Readme from the original Kermit 95 source release                          |
| CHANGES.TXT         | K95 2.2 Changelog                                            | List of changes between Kermit 95 2.1.3 and the unreleased Kermit 95 2.2   |
| COPYING.TXT         | C-Kermit for Windows License                                 |                                                                            |
| default.ini         | C-Kermit SAVE KEYMAP file                                    |                                                                            |
| dialing.ipf         | C-Kermit's Dialing Directory                                 |                                                                            |
| emacs.ini           | EMACS key map for Kermit 95                                  |                                                                            |
| host.ksc            | "Host mode" script for K-95.                                 |                                                                            |
| hostcom.ksc         | Sample host mode operation for dialin connections            |                                                                            |
| hostmdm.ksc         | Kermit 95 host mode listener for dialin connections          |                                                                            |
| hostmode.ksc        | Utility and management functions for Kermit 95 host mode     |                                                                            |
| hosttcp.ksc         | Kermit 95 Host-Mode Listener for Incoming TCP/IP Connections |                                                                            |
| iksdpy.ksc          |                                                              | Kermit script for viewing the IKS database                                 |
| K95-NOTES.txt       |                                                              | Kermit 95 notes from Frank da Cruz (bugs, ideas, etc)                      |
| login.ksc           |                                                              | a general-purpose login script                                             |
| m2zmodem.ini        | Macros for invoking M2ZMODEM from OS/2 C-Kermit              |                                                                            |
| NOTES.TXT           |                                                              | Notes on the current C-Kermit release                                      |
| scoansi.ini         |                                                              | Extended Keyboard definitions for Kermit 95, Works with ANSI terminal type |
| vt220.ini           | Extended Keyboard definitions for Kermit 95                  | Works with any VT terminal type: VT52, VT100, VT102, VT220, or VT320       |

### Unusued/unreferenced stuff

Some of this is likely old junk that can be deleted, some of it may be unfinished features 
or utilities that aren't enabled. Further investigation required for all of it.

| Filename            | Title                                                    | Notes                                                                                                          |
|---------------------|----------------------------------------------------------|----------------------------------------------------------------------------------------------------------------|
| ckcb64.c            |                                                          | Looks like a base64 implementation form IBM. Appears unused                                                    |
| ckcfil.h            |                                                          | No longer used, replaced with ckcker.h                                                                         |
| ckctox.c            |                                                          | Appears unused                                                                                                 |
| ckcvar.c            |                                                          | Appears unusued                                                                                                |
| ckcvar.h            |                                                          | Appears unusued                                                                                                |
| ckczli.c            |                                                          | Looks like just an example app from zlib. Appears unused                                                       |
| ckuuid.c            |                                                          | Test program for C-Kermit's uid-managing code                                                                  |
| ck_old.ico          |                                                          |                                                                                                                |
| eklogin.c           | eklogin stub for Kermit 95                               |                                                                                                                |
| findfile.c          |                                                          | Sample code from Microsoft? "C find file functions"                                                            |
| fneval.c            |                                                          | An email?                                                                                                      |
| ftime.bat           |                                                          | ? junk probably                                                                                                |
| geterm.c            |                                                          | ? junk probably                                                                                                |
| GetUserInfo.c       |                                                          | ? junk probably                                                                                                |
| iksdc.c             |                                                          | ? Maybe an OS/2 version of iksd.exe - doesn't appear to be used                                                |
| k2dc.c              |                                                          | k2dc.exe - maybe an OS/2 variant of k95d.exe?                                                                  |
| k95dial3.ico        |                                                          |                                                                                                                |
| keystuff.c          |                                                          | Probably junk - keyboard test program?                                                                         |
| klogin.c            | Klogin stub for Kermit 95                                | doesn't appear to be used                                                                                      |
| listcomx.c          |                                                          | Probably junk - program to list serial ports?                                                                  |
| match.ksc           |                                                          | Probably junk                                                                                                  |
| mkiksd.bat          |                                                          | old windows build script - iksd                                                                                |
| mkkui.bat           |                                                          | old windows build script - KUI                                                                                 |
| mknt-noiksd.bat     |                                                          | old windows build script - everything but k95g.exe and iksd                                                    |
| mksrczip.bat        |                                                          | builds a zip of the CKW source code - probably junk                                                            |
| mnem.dat            |                                                          | No idea - junk probably                                                                                        |
| mnem.syms           |                                                          | No idea - junk probably                                                                                        |
| modems.ipf          |                                                          | Source for a book or manual: C-Kermit's Modem Support                                                          |
| modetst.c           |                                                          | Probably junk - program to get OS/2 console mode                                                               |
| print.c             |                                                          | Probably Junk - sample code for to send binary data to a printer on windows                                   |
| prof.bat            |                                                          | Probably junk - script for profiling cknker.exe with some tool                                                 |
| prof2.bat           |                                                          | Probably junk - script for profiling cknker.exe with some tool                                                 |
| ptest.c             |                                                          | Probably junk - a printer test app for windows                                                                 |
| rcmd.c              |                                                          | Probably junk. Code from BSD. Appears unused.                                                                  |
| rlogind.c           |                                                          | Probably junk. Code from BSd. Appears unused.                                                                  |
| setup.mak           |                                                          | Probably junk. Makefile for setup.exe which I think can be built by the main makefile now                      |
| srpfltr.c           |                                                          | Probably junk - appears unusued                                                                                |
| srpw95pp.reg        |                                                          | Maybe junk - registry entries for some SRP thing                                                               |
| startk95.c          |                                                          | Example code: how to start CKW from another Win32 application                                                  |
| stub.c              | Stub for Kermit 95                                       | Example code I guess - for building things like the telnet stub                                                |
| timesync.c          | "timesync stuff for leash"                               | Probably old Kerberos junk. Appears unused.                                                                    |
| trim.c              | Trim trailing whitespace from file lines and/or untabify | Utility program. Appears unusued (not built by any makefile)                                                   |
| url.c               |                                                          | Probably junk - appears unused                                                                                 |
| vttime.c            |                                                          | Probably junk - appears unused. Program to set the clock on a VT series terminal based on the system time.     |
| wtest.def           | C-Kermit for Windows NT Test                             | Almost certainly junk. Module definition file for test.exe, the source code for which does not exist.          |
| zip.sh              |                                                          | Script used to create the original Kermit 95 open source source release. Junk at this point, its done its job. |