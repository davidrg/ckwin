/*  C K U U S 2  --  User interface strings & help text module for C-Kermit  */

/*
  Authors:
    Frank da Cruz <fdc@columbia.edu>,
    Jeffrey Altman <jaltman@columbia.edu>,
    The Kermit Project
    Columbia University
    New York City

  Copyright (C) 1985, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  This module contains HELP command and other long text strings.

  IMPORTANT: Character string constants longer than about 250 are not portable.
  Longer strings should be broken up into arrays of strings and accessed with
  hmsga() rather than hmsg().
*/
#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcnet.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"
#ifdef OS2
#ifdef NT
#include <windows.h>
#else /* not NT */
#define INCL_KBD
#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#include <os2.h>		/* This pulls in a whole load of stuff */
#undef COMMENT
#endif /* NT */
#include "ckocon.h"
#include "ckokvb.h"
#include "ckokey.h"
#endif /* OS2 */

extern xx_strp xxstring;
extern char *ccntab[];

#ifndef NOICP
extern int cmflgs;

#ifdef DCMDBUF
extern char *cmdbuf, *atmbuf;
#else
extern char cmdbuf[], atmbuf[];
#endif /* DCMDBUF */
#endif /* NOICP */

extern char *xarg0;
extern int nrmt, nprm, dfloc, local, parity, escape;
extern int turn, flow;
extern int binary, quiet, keep;
extern int success, xaskmore;
#ifdef OS2
extern int tt_rows[], tt_cols[];
#else /* OS2 */
extern int tt_rows, tt_cols;
#endif /* OS2 */
extern int cmd_rows, cmd_cols;

extern long speed;
extern char *dftty, *versio, *ckxsys;
#ifndef NOHELP
extern char *helpfile;
#endif /* NOHELP */
extern struct keytab prmtab[];
#ifndef NOXFER
extern struct keytab remcmd[];
#endif /* NOXFER */

#ifndef NOICP

/*  Interactive help strings  */

static char *tophlp[] = {
"Trustees of Columbia University in the City of New York.\n",

#ifndef NOHELP
#ifdef OS2
"  Type INTRO   for a brief introduction to the Kermit Command screen.",
#else
"  Type INTRO   for a brief introduction to C-Kermit.",
#endif /* OS2 */
"  Type VERSION for version and copyright information.",
"  Type HELP    followed by a command name for help about a specific command.",
#ifndef NOPUSH
#ifdef UNIX
"  Type MANUAL  to access the C-Kermit manual page.",
#else
#ifdef VMS
"  Type MANUAL  to access the C-Kermit help topic.",
#else
#ifdef OS2
"  Type MANUAL  to access the K95 manual.",
#else
"  Type MANUAL  to access the C-Kermit manual.",
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
#endif /* NOPUSH */
"  Type NEWS    for news about new features.",
"  Type SUPPORT to learn how to get technical support.",
"  Press ?      (question mark) at the prompt, or anywhere within a command,",
"               for a menu (context-sensitive help, menu on demand).",
#else
"Press ? for a list of commands; see documentation for detailed descriptions.",
#endif /* NOHELP */

#ifndef NOCMDL
#ifndef NOHELP
" ",
"  Type HELP OPTIONS for help with command-line options.",
#endif /* NOHELP */
#endif /* NOCMDL */
" ",
"DOCUMENTATION: \"Using C-Kermit\" by Frank da Cruz and Christine M. Gianone,",
"Digital Press / Butterworth-Heinemann, latest edition.  Please purchase this",
"book.  It shows you how to use C-Kermit, answers your questions, reduces the",
"load on our help desk, and book sales are the primary source of funding for",
"C-Kermit development.  More info: http://www.columbia.edu/kermit/.",

#ifdef MAC
" ",
"Also see the Mac Kermit Doc and Bwr files on the Mac Kermit diskette.\n",
#else
#ifdef HPUX10
" ",
"See the files in /usr/share/lib/kermit/ for additional information.",
#endif /* HPUX10 */
#endif /* MAC */
""
};

#ifndef NOHELP
char *newstxt[] = {
#ifdef OS2
"Welcome to Kermit 95 1.1.18",
" ",
"Major new features (not necessarily available on all platforms) include:",
#else
"Welcome to C-Kermit 7.0.196.",
" ",
"Major new features (not necessarily available on all platforms) include:",
#endif /* OS2 */

" ",
#ifndef NOIKSD
" . Internet Kermit Service Daemon and client.",
#endif /* NOIKSD */
#ifdef CK_AUTHENTICATION
#ifdef CK_ENCRYPTION
" . Secure network authentication and encryption",
#else
" . Secure network authentication",
#endif /* CK_ENCRYPTION */
#ifdef NETCMD
" . External communication program interface.",
#endif /* NETCMD */
#ifdef NETPTY
" . Scripted control of local programs.",
#endif /* NETPTY */
#endif /* CK_AUTHENTICATION */
" . More-powerful filename-matching patterns",
" . Recursive directory-tree transfer and directory listing.",
" . Automatic per-file text/binary mode switching.",
#ifndef NOFAST
" . Fast Kermit protocol settings are now the default.",
#endif /* NOFAST */
" . File-transfer selection and other option switches (HELP SEND, HELP GET).",
#ifdef PIPESEND
" . File-transfer pipes and filters.",
#endif /* PIPESEND */
" . New streaming Kermit protocol for use on reliable connections.",
#ifdef CKREGEX
" . More-flexible wildcards for filename matching (HELP WILDCARD).",
#endif /* CKREGEX */
#ifndef NOLOCAL
" . Numerous modem and dialing improvements.",
" . More/higher serial speeds on most platforms (SET SPEED ?).",
" . New data-bits/parity/stop-bits options (HELP SET SERIAL).",
#endif /* NOLOCAL */
#ifdef CK_PERMS
" . File-permission preservation in UNIX and VMS.",
#endif /* CK_PERMS */
" . Improvements in CD and other directory-related commands.",
#ifndef NOCSETS
#ifdef UNICODE
" . Unicode - the Universal Character Set (UCS-2 and UTF-8)",
#endif /* UNICODE */
#ifndef NOGREEK
" . Greek character sets.",
#endif /* NOGREEK */
" . New Euro-compliant and other character sets.",
" . Automatic character-set switching (HELP ASSOCIATE).",
#endif /* NOCSETS */
#ifndef NOSPL
" . New script programming features, functions, and variables.",
" . \"kerbang\" scripts.",
" . General-purpose file i/o and floating-point arithmetic packages.",
#endif /* NOSPL */
#ifdef CXLOGDIAL
" . New connection log (HELP LOG).",
#endif /* CXLOGDIAL */
#ifndef NOHELP
" . New MANUAL command to access online documentation (HELP MANUAL).",
#endif /* NOHELP */
" . Performance improvements.",
" . Bug fixes.",
" ",
"These new features are documented in the ckermit2.txt file, to be used as",
"a supplement to the manual, \"Using C-Kermit\", second edition (1997),",
"until the third edition is ready.",
#ifdef OS2
" ",
"And in the online Kermit 95 manual, accessible via the MANUAL command.",
#endif /* OS2 */
" ",
"If the release date shown by the VERSION command is long past, be sure to",
"check with the Kermit Project to see if there have been updates.",
""
};
#endif /* NOHELP */

#ifndef NOHELP
char *introtxt[] = {
#ifdef NT
"Welcome to K-95, Kermit communications software for:",
#else
#ifdef OS2
"Welcome to Kermit/2, Kermit communications software for:",
#else
#ifdef UNIX
#ifdef HPUX
"Welcome to HP-UX C-Kermit communications software for:",
#else
"Welcome to UNIX C-Kermit communications software for:",
#endif /* HPUX */
#else
#ifdef VMS
"Welcome to VMS C-Kermit communications software for:",
#else
#ifdef VOS
"Welcome to VOS C-Kermit communications software for:",
#else
#ifdef MAC
"Welcome to Mac Kermit communications software for:",
#else
"Welcome to C-Kermit communications software for:",
#endif /* MAC */
#endif /* VOS */
#endif /* VMS */
#endif /* UNIX */
#endif /* OS2 */
#endif /* NT */
#ifndef NOXFER
" . Error-free and efficient file transfer",
#endif /* NOXFER */
#ifndef NOLOCAL
#ifdef OS2
" . VT320/220/102/100/52, ANSI, Wyse, DG, Televideo, and other emulations",
#else
#ifdef MAC
" . VT220 terminal emulation",
#else
" . Terminal connection",
#endif /* MAC */
#endif /* OS2 */
#endif /* NOLOCAL */
#ifndef NOSPL
" . Script programming",
#endif /* NOSPL */
#ifndef NOICS
" . International character set conversion",
#endif /* NOICS */
#ifndef NOLOCAL
"\nSupporting:",
" . Serial connections, direct or dialed.",
#ifndef NODIAL
" . Automatic modem dialing",
#endif /* NODIAL */
#ifdef TCPSOCKET
" . TCP/IP network connections",
#endif /* TCPSOCKET */
#ifdef ANYX25
" . X.25 network connections",
#endif /* ANYX25 */
#ifdef OS2
#ifdef DECNET
" . DECnet/PATHWORKS LAT Ethernet connections",
#endif /* DECNET */
#ifdef SUPERLAT
" . Meridian Technologies' SuperLAT connections",
#endif /* SUPERLAT */
#ifdef NPIPE
" . Named-pipe connections",
#endif /* NPIPE */
#ifdef CK_NETBIOS
" . NETBIOS connections",
#endif /* CK_NETBIOS */
#endif /* OS2 */
#endif /* NOLOCAL */

"\nWhile typing commands, you may use the following special characters:",
" . DEL, RUBOUT, BACKSPACE, CTRL-H: Delete the most recent character typed.",
" . CTRL-W:  Delete the most recent word typed.",
" . CTRL-U:  Delete the current line.",
" . CTRL-R:  Redisplay the current line.",
#ifdef CK_RECALL
#ifdef OS2
" . \030 or CTRL-B: Command recall - go backwards in command recall buffer.",
" . \031 or CTRL-N: Command recall - go forward in command recall buffer.",
#else
" . CTRL-P:  Command recall - go backwards in command recall buffer.",
" . CTRL-B:  Command recall - same as Ctrl-P.",
" . CTRL-N:  Command recall - go forward in command recall buffer.",
#endif /* OS2 */
#endif /* CK_RECALL */
" . ?        (question mark) Display a menu for the current command field.",
" . ESC      (or TAB) Attempt to complete the current field.",
" . \\        (backslash) include the following character literally",
#ifndef NOSPL
"            or introduce a backslash code, variable, or function.",
#else
"            or introduce a numeric backslash code.",
#endif /* NOSPL */
"  Command words other than filenames can be abbreviated in most contexts.",

#ifdef OS2
#ifdef NT
"\nBasic K-95 commands:",
"  EXIT          exit from K-95",
#else /* NT */
"\nBasic Kermit/2 commands:",
"  EXIT          exit from Kermit/2",
#endif /* NT */
#else /* OS2 */
"\nBasic C-Kermit commands:",
"  EXIT          exit from C-Kermit",
#endif /* NT */
"  HELP          request general help",
"  HELP command  request help about the given command",
"  TAKE          execute commands from a file",
"  TYPE          display a file on your screen",

#ifndef NOXFER
"\nCommands for file transfer:",
"  SEND          send files",
"  RECEIVE       receive files",
"  GET           get files from a Kermit server",
#ifdef CK_RESEND
"  RESEND        recover an interrupted send",
"  REGET         recover an interrupted get from a server",
#endif /* CK_RESEND */
#ifndef NOSERVER
"  SERVER        be a Kermit server",
#endif /* NOSERVER */

"\nFile-transfer speed selection:",
"  FAST          use fast settings -- THIS IS THE DEFAULT",
"  CAUTIOUS      use slower, more cautious settings",
"  ROBUST        use extremely slow and cautious settings",

"\nFile-transfer performance fine tuning:",
"  SET RECEIVE PACKET-LENGTH  Kermit packet size",
"  SET WINDOW                 number of sliding window slots",
"  SET PREFIXING              amount of control-character prefixing",
#endif /* NOXFER */

"\nImportant settings:",
"  SET PARITY    communications parity",
#ifdef CK_RTSCTS
"  SET FLOW      communications flow control, such as RTS/CTS",
#else
"  SET FLOW      communications flow control, such as XON/XOFF",
#endif /* CK_RTSCTS */
"  SET FILE      file settings, for example TYPE TEXT or TYPE BINARY",

#ifndef NOLOCAL
"\nTo make a direct serial connection:",
#ifdef OS2
#ifdef NT
#ifdef CK_TAPI
"  SET TAPI LINE select TAPI communication device",
#endif /* CK_TAPI */
"  SET PORT      select serial communication device",
#else
"  SET PORT      select serial communication port or server",
#endif /* NT */
#else
"  SET LINE      select serial communication device",
#endif /* OS2 */
"  SET SPEED     select communication speed",
"  CONNECT       begin terminal connection",

#ifndef NODIAL
"\nTo dial out with a modem:",
"  SET DIAL DIRECTORY     specify dialing directory file (optional)",
"  SET DIAL COUNTRY-CODE  country you are dialing from (*)",
"  SET DIAL AREA-CODE     area-code you are dialing from (*)",
"  LOOKUP                 lookup entries in your dialing directory (*)",
"  SET MODEM TYPE         select modem type",
#ifdef OS2
#ifdef NT
#ifdef CK_TAPI
"  SET TAPI LINE select TAPI communication device",
#endif /* CK_TAPI */
"  SET PORT      select serial communication device",
#else
"  SET PORT      select serial communication port or server",
#endif /* NT */
#else
"  SET LINE      select serial communication device",
#endif /* OS2 */
"  SET SPEED     select communication speed",
"  DIAL          dial the phone number",
"  CONNECT       begin terminal connection",
#ifdef OS2
"Further info:   HELP DIAL, HELP SET MODEM, HELP SET PORT, HELP SET DIAL",
#else
"Further info:   HELP DIAL, HELP SET MODEM, HELP SET LINE, HELP SET DIAL",
#endif /* OS2 */
"(*) (For use with optional dialing directory)",
#endif /* NODIAL */

#ifdef NETCONN
"\nTo make a network connection:",
#ifndef NODIAL
"  SET NETWORK DIRECTORY  Specify a network services directory (optional)",
"  LOOKUP                 Lookup entries in your network directory",
#endif /* NODIAL */
"  SET NETWORK TYPE       Select network type (if more than one available)",
"  SET HOST               Make a network connection but stay in command mode",
"  CONNECT                Begin terminal connection",
#ifdef TNCODE
"  TELNET                 Select a TCP/IP host and CONNECT to it",
#endif /* TNCODE */
#ifdef RLOGCODE
"  RLOGIN                 Select a TCP/IP host and RLOGIN to it",
#endif /* RLOGCODE */
#endif /* NETCONN */

#ifdef NT
"\nTo return from the terminal window to the K-95> prompt:",
#else
#ifdef OS2
"\nTo return from the terminal window to the K/2> prompt:",
#else
"\nTo return from a terminal connection to the C-Kermit prompt:",
#endif /* OS2 */
#endif /* NT */
#ifdef OS2
"  \
Press the key or key-combination shown after \"Prompt:\" in the status line",
"  (such as Alt-x) or type your escape character followed by the letter C.",
#else
"  Type your escape character followed by the letter C.",
#endif /* OS2 */
" ",
"To display your escape character:",
"  SHOW ESCAPE",
" ",
"To display other settings:",
"  SHOW COMMUNICATIONS, SHOW TERMINAL, SHOW FILE, SHOW PROTOCOL, etc.",
#else  /* !NOLOCAL */
" ",
"To display settings:",
"  SHOW COMMUNICATIONS, SHOW FILE, SHOW PROTOCOL, etc.",
#endif /* NOLOCAL */
" ",
"For further information about a particular command, type HELP xxx,",
"where xxx is the name of the command.  For documentation, news of new",
"releases, and information about other Kermit software, contact:",
" ",
"  The Kermit Project         E-mail: kermit-orders@columbia.edu",
"  Columbia University        Web:    http://www.columbia.edu/kermit/",
"  612 West 115th Street      Voice:  +1 (212) 854-3703",
"  New York NY  10025-7799    Fax:    +1 (212) 663-8202",
"  USA",
""
};

static char *hmxxscrn[] = {
"Syntax: SCREEN { CLEAR, CLEOL, MOVE row column }",
#ifdef OS2
"  Performs screen-formatting actions.",
#else
"  Performs screen-formatting actions.  Correct operation of these commands",
"  depends on proper terminal setup on both ends of the connection -- mainly",
"  that the host terminal type is set to agree with the kind of terminal or",
"  the emulation you are viewing C-Kermit through.",
#endif /* OS2 */
" ",
"SCREEN CLEAR",
"  Moves the cursor to home position and clears the entire screen.",
#ifdef OS2
"  Synonyms: CLS, CLEAR SCREEN, CLEAR COMMAND-SCREEN ALL",
#else
"  Synonyms: CLS, CLEAR SCREEN.",
#endif /* OS2 */
" ",
"SCREEN CLEOL",
"  Clears from the current cursor position to the end of the line.",
#ifdef OS2
"  Synonym: CLEAR COMMAND-SCREEN EOL",
#endif /* OS2 */
" ",
"SCREEN MOVE row column",
"  Moves the cursor to the indicated row and column.  The row and column",
"  numbers are 1-based so on a 24x80 screen, the home position is 1 1 and",
"  the lower right corner is 24 80.  If a row or column number is given that",
"  too large for what Kermit or the operating system thinks is your screen",
"  size, the appropriate number is substituted.",
" ",
"Also see:",
#ifdef OS2
"  HELP FUNCTION SCRNCURX, HELP FUNCTION SCRNCURY, HELP FUNCTION SCRSTR,",
#endif /* OS2 */
"  SHOW VARIABLE TERMINAL, SHOW VARIABLE COLS, SHOW VAR ROWS, SHOW COMMAND.",
""
};

#ifndef NOSPL
static char *hxxinp[] = {
"Syntax:  INPUT { number-of-seconds, time-of-day } [ text ]",
"Example: INPUT 5 Login:  or  INPUT 23:59:59 RING",
"  Waits up to the given number of seconds, or until the given time of day",
"  for the given text to arrive on the connection.  If no text is given,",
"  INPUT waits for any character.  For use in script programs with IF FAILURE",
"  and IF SUCCESS.  Also see MINPUT, REINPUT, SET INPUT.  See HELP PAUSE for",
"  details on time-of-day format.  The text, if given, can be a \\pattern()",
"  invocation, in which case it is treated as a regular expression rather than"
,
"  a literal string (HELP REGULAR-EXPRESSIONS for details).",
""};

static char *hxxout[] = {
"Syntax: OUTPUT text\n",
"  Sends the text out the communications connection, as if you had typed it",
"  during CONNECT mode.  The text may contain backslash codes, variables,",
"  etc, plus the following special codes:",
" ",
"    \\N - Send a NUL (ASCII 0) character (you can't use \\0 for this).",
"    \\B - Send a BREAK signal.",
"    \\L - Send a Long BREAK signal.",
" ",
"Also see SET OUTPUT.",
"" };
#endif /* NOSPL */

static char *hxypari[] = {
"SET PARITY NONE",
"  Chooses 8 data bits and no parity.",
" ",
"SET PARITY { EVEN, ODD, MARK, SPACE }",
"  Chooses 7 data bits plus the indicated kind of parity.",
"  Forces 8th-bit prefixing during file transfer.",
" ",
#ifdef HWPARITY
"SET PARITY HARDWARE { EVEN, ODD }",
"  Chooses 8 data bits plus the indicated kind of parity.",
" ",
"Also see SET TERMINAL BYTESIZE, SET SERIAL, and SET STOP-BITS.",
#else
"Also see SET TERMINAL BYTESIZE and SET SERIAL.",
#endif /* HWPARITY */
""};

#ifndef NOLOCAL
static char *hxyesc[] = {
#ifdef OS2
"Syntax: SET ESCAPE number",
"  Decimal ASCII value for escape character during CONNECT, normally 29",
"  (Control-]).  Type the escape character followed by C to get back to the",
"  C-Kermit prompt or followed by ? to see other options, or use the \\Kexit",
"  keyboard verb, normally assigned to Alt-x.",
#else
#ifdef NEXT
"Syntax: SET ESCAPE number",
"  Decimal ASCII value for escape character during CONNECT, normally 29",
"  (Control-]).  Type the escape character followed by C to get back to the",
"  C-Kermit prompt or followed by ? to see other options.",
#else
"Syntax: SET ESCAPE number",
"  Decimal ASCII value for escape character during CONNECT, normally 28",
"  (Control-\\).  Type the escape character followed by C to get back to the",
"  C-Kermit prompt or followed by ? to see other options.",
#endif /* NEXT */
#endif /* OS2 */
" ",
"You may also enter the escape character as ^X (circumflex followed by a",
"letter or one of: @, ^, _, [, \\, or ], to indicate a control character;",
"for example, SET ESC ^_ sets your escape character to Ctrl-Underscore.",
"" };
#endif /* NOLOCAL */

#ifndef NOSPL
static char *hxyout[] = {
"SET OUTPUT PACING <number>\n",
"  How many milliseconds to pause after sending each OUTPUT character,",
"  normally 0.",
" ",
"SET OUTPUT SPECIAL-ESCAPES { ON, OFF }\n",
"  Whether to process the special OUTPUT-only escapes \\B, \\L, and \\N.",
"  Normally ON (they are processed).",
"" };

static char *hxyinp[] = {
"Syntax: SET INPUT parameter value",
" ",
#ifdef CK_AUTODL
"SET INPUT AUTODOWNLOAD { ON, OFF }",
"  Controls whether autodownloads are allowed during INPUT command execution.",
" ",
#endif /* CK_AUTODL */
"SET INPUT BUFFER-LENGTH number-of-bytes",
"  Removes the old INPUT buffer and creates a new one with the given length.",
" ",
"SET INPUT CANCELLATION { ON, OFF }",
"Whether an INPUT in progress can be can interrupted from the keyboard.",
" ",
"SET INPUT CASE { IGNORE, OBSERVE }",
"  Tells whether alphabetic case is to be significant in string comparisons.",
"  This setting is local to the current macro or command file, and is",
"  inherited by subordinate macros and take files.",
" ",
"SET INPUT ECHO { ON, OFF }",
"  Tells whether to display arriving characters read by INPUT on the screen.",
" ",
"SET INPUT SILENCE <number>",
"  The maximum number to seconds of silence (no input at all) before the",
"  INPUT command times out, 0 for no maximum.",
" ",
#ifdef OS2
"SET INPUT TERMINAL { ON, OFF }",
"  Determines whether the data received during an INPUT command is displayed",
"  in the terminal window.  Default is ON.",
" ",
#endif /* OS2 */
"SET INPUT TIMEOUT-ACTION { PROCEED, QUIT }",
"  Tells whether to proceed or quit from a script program if an INPUT command",
"  fails.  PROCEED (default) allows use of IF SUCCESS / IF FAILURE commands.",
"  This setting is local to the current macro or command file, and is",
"  inherited by subordinate macros and take files.",
"" };

static char *hxyfunc[] = {
"SET FUNCTION DIAGNOSTICS { ON, OFF }",
"  Whether to issue diagnostic messages for illegal function calls and",
"  references to nonexistent built-in variables.  ON by default.",
" ",
"SET FUNCTION ERROR { ON, OFF }",
"  Whether an illegal function call or reference to a nonexistent built-in",
"  variable should cause a command to fail.  OFF by default.",
"" };
#endif /* NOSPL */

static char *hxyxyz[] = {
#ifdef CK_XYZ
#ifdef XYZ_INTERNAL

/* This is for built-in protocols */

"Syntax: SET PROTOCOL { KERMIT, XMODEM, YMODEM, ZMODEM } [ s1 s2 [ s3 ] ]",
"  Selects protocol to use for transferring files.  String s1 is a command to",
"  send to the remote host prior to SENDing files with this protocol in",
"  binary mode; string s2 is the same thing but for text mode.  Use \"%\" in",
"  any of these strings to represent the filename(s).  If the protocol is",
"  KERMIT, you may also specify a string s3, the command to start a Kermit",
"  server on the remote host when you give a GET, REGET, REMOTE, or other",
"  client command.  Use { braces } if any command contains spaces.  Examples:",
" ",
"    set proto xmodem {rx %s} {rx -a %s}",
"    set proto kermit {kermit -YQir} {kermit -YQTr} {kermit -YQx}",

#else /* This is for when non-Kermit protocols are external */

"Syntax: \
SET PROTOCOL { KERMIT, XMODEM, YMODEM, ZMODEM } [ s1 s2 s3 s4 s5 s6 ]",
"  Selects protocol to use for transferring files.  s1 and s2 are commands to",
"  output prior to SENDing with this protocol, to automatically start the",
"  RECEIVE process on the other end in binary or text mode, respectively.",
"  If the protocol is KERMIT, s3 is the command to start a Kermit server on",
"  the remote computer, and there are no s4-s6 commands.  Otherwise, s3 and",
"  s4 are commands used on this computer for sending files with this protocol",
"  in binary or text mode, respectively; s5 and s6 are the commands for",
"  receiving files with this protocol.  Use \"%s\" in any of these strings",
"  to represent the filename(s).  Use { braces } if any command contains",
"  spaces.  Examples:",
" ",
"    set proto kermit {kermit -YQir} {kermit -YQTr} {kermit -YQx}",
"    set proto ymodem rb {rb -a} {sb %s} {sb -a %s} rb rb",
" ",
"External protocols require REDIRECT and external file transfer programs that",
"use redirectable standard input/output.",
#endif /* XYZ_INTERNAL */
#else
"Syntax: \
SET PROTOCOL KERMIT [ s1 [ s2 [ s3 ] ] ]",
"  Lets you specify the autoupload binary, autoupload text, and autoserver",
"  command strings to be sent to the remote system in advance of any SEND",
"  or GET commands.  By default these are \"kermit -ir\", \"kermit -r\", and",
"  \"kermit -x\".  Use { braces } around any command that contains spaces.",
"  Example:",
" ",
"    set proto kermit {kermit -YQir} {kermit -YQTr} {kermit -YQx}",
#endif /* CK_XYZ */
" ",
"  SHOW PROTOCOL displays the current settings.",
""};

static char *hmxxbye = "Syntax: BYE\n\
  Shut down and log out a remote Kermit server";

static char *hmxxdir[] = {
#ifdef DOMYDIR
"Syntax: DIRECTORY [ switches ] [ filespec ]",
"  Lists files.  The filespec may be a filename, possibly containing wildcard",
"  characters, or a directory name.  If no filespec is given, all files in",
"  the current directory are listed.  If a directory name is given, all the",
"  files in it are listed.  Optional switches:",
" ",
"   /BRIEF         List filenames only.",
#ifdef CK_PERMS
"   /VERBOSE     + Also list permissions, size, and date.",
#else
"   /VERBOSE     + Also list date and size.",
#endif /* CK_PERMS */
"   /FILES         Show files but not directories.",
"   /DIRECTORIES   Show directories but not files.",
"   /ALL         + Show both files and directories.",
"   /ARRAY:&a      Store file list in specified array (e.g. \\%a[]).",
"   /PAGE          Pause after each screenful.",
"   /NOPAGE        Don't pause after each screenful.",
#ifdef UNIXOROSK
"   /DOTFILES      Include files whose names start with dot (period).",
"   /NODOTFILES  + Don't include files whose names start with dot.",
"   /BACKUP      + Include backup files (names end with .~n~).",
"   /NOBACKUP      Don't include backup files.",
#endif /* UNIXOROSK */
"   /HEADING       Include heading and summary.",
"   /NOHEADING   + Don't include heading or summary.",
"   /XFERMODE      Show pattern-based transfer mode (T=Text, B=Binary).",
"   /MESSAGE:text  Add brief message to each listing line.",
"   /NOMESSAGE   + Don't add message to each listing line.",
"   /NOXFERMODE  + Don't show pattern-based transfer mode",
"   /ISODATE     + In verbose listings, show date in ISO 8061 format.",
"   /ENGLISHDATE   In verbose listings, show date in \"English\" format.",
#ifdef RECURSIVE
"   /RECURSIVE     Descend through subdirectories.",
"   /NORECURSIVE + Don't descend through subdirectories.",
#endif /* RECURSIVE */
"   /SORT:key      Sort by key, NAME, DATE, or SIZE; default key is NAME.",
"   /NOSORT      + Don't sort.",
"   /ASCENDING   + If sorting, sort in ascending order.",
"   /REVERSE       If sorting, sort in reverse order.",
" ",
"Factory defaults are marked with +.  Default for paging depends on SET",
"COMMAND MORE-PROMPTING.  Use SET OPTIONS DIRECTORY [ switches ] to change",
"defaults; use SHOW OPTIONS to display customized defaults.",
#else
"Syntax: DIRECTORY [ filespec ]",
"  Lists the specified file or files.  If no filespec is given, all files",
"  in the current directory are listed.",
#endif /* DOMYDIR */
""};

static char *hmxxdel[] = {
"Syntax: DELETE [ switches... ] filespec",
"  Deletes a file or files on the computer where C-Kermit is running.",
"  The filespec may denote a single file or can include wildcard characters",
"  to match multiple files.  RM is a synonym for DELETE.  Switches include:",
" ",
"/AFTER:date-time",
#ifdef VMS
"  Specifies that only those files created after the given date-time are",
#else
"  Specifies that only those files modified after the given date-time are",
#endif /* VMS */
"  to be deleted.  HELP DATE for info about date-time formats.",
" ",
"/BEFORE:date-time",
#ifdef VMS
"  Specifies that only those files modified before the given date-time",
#else
"  Specifies that only those files modified before the given date-time",
#endif /* VMS */
"  are to be deleted.",
" ",
"/NOT-AFTER:date-time",
#ifdef VMS
"  Specifies that only those files modified at or before the given date-time",
#else
"  Specifies that only those files modified at or before the given date-time",
#endif /* VMS */
"  are to be deleted.",
" ",
"/NOT-BEFORE:date-time",
#ifdef VMS
"  Specifies that only those files modified at or after the given date-time",
#else
"  Specifies that only those files modified at or after the given date-time",
#endif /* VMS */
"  are to be deleted.",
" ",
"/LARGER-THAN:number",
"  Specifies that only those files longer than the given number of bytes are",
"  to be deleted.",
" ",
"/SMALLER-THAN:number",
"  Specifies that only those files smaller than the given number of bytes are",
"  to be sent.",
" ",
"/EXCEPT:pattern",
"  Specifies that any files whose names match the pattern, which can be a",
"  regular filename or may contain wildcards, are not to be deleted.  To",
"  specify multiple patterns (up to 8), use outer braces around the group",
"  and inner braces around each pattern:",
" ",
"    /EXCEPT:{{pattern1}{pattern2}...}",
" ",
#ifdef UNIXOROSK
"/DOTFILES",
"  Include (delete) files whose names begin with \".\".",
" ",
"/NODOTFILES",
"  Skip (don't delete) files whose names begin with \".\".",
" ",
#endif /* UNIXOROSK */
"/LIST",
"  List each file and tell whether it was deleted.  Synonyms: /LOG, /VERBOSE.",
" ",
"/NOLIST",
"  Don't list files while deleting.  Synonyms: /NOLOG, /QUIET.",
" ",
"/HEADING",
"  Print heading and summary information.",
" ",
"/NOHEADING",
"  Don't print heading and summary information.",
" ",
"/PAGE",
"  If listing, pause after each screenful.",
" ",
"/NOPAGE",
"  Don't pause after each screenful.",
" ",
"/ASK",
"  Interactively ask permission to delete each file.",
" ",
"/NOASK",
"  Delete files without asking permission.",
" ",
"/SIMULATE",
"  Preview files selected for deletion without actually deleting them.",
"  Implies /LIST.",
" ",
"Use SET OPTIONS DELETE to make selected switches effective for every DELETE",
"command \
unless you override them; use SHOW OPTIONS to see selections currently",
"in effect.  Also see HELP PURGE, HELP WILDCARD.",
""};

#ifndef NOHTTP
static char *hmxxhttp[] = {
"Syntax:",
"HTTP [ <switches> ] GET <remote-filename> [ <local-filename> ]",
"  Retrieves the named file.  If a <local-filename> is given, the file is",
"  stored locally under that name; otherwise it is stored with its own name.",
" ",
"HTTP [ <switches> ] HEAD <remote-filename> <local-filename>",
"  Like GET except without actually getting the file; instead it gets only",
"  the headers, storing them into the given file, whose name must be given,",
"  one line per header item, as shown in the /ARRAY: switch description.",
" ",
"HTTP [ <switches> ] INDEX <remote-directory> [ <local-filename> ]",
"  Retrieves the file listing for the given server directory.",
"  NOTE: This command is not supported by most Web servers.",
" ",
"HTTP [ <switches> ] POST [ /MIME-TYPE:<type> ] <local-file> <remote-file>",
"  Used to send a response as if it were sent from a form.  The data to be",
"  posted must be read from a file.",
" ",
"HTTP [ <switches> ] PUT [ /MIME-TYPE:<type> ] <local-file> <remote-file>",
"  Uploads a local file to a server file.",
" ",
"HTTP [ <switches> ] DELETE <remote-filename>",
"  Instructs the server to delete the specified filename.",
" ",
"where <switches> are:",
"/AGENT:<user-agent>",
"  Identifies the client to the server; \"C-Kermit\" or \"Kermit-95\"",
"  by default.",
" ",
"/HEADER:<header-line>",
"  Used for specifying any optional headers.  A list of headers is provided",
"  using braces for grouping:",
" ",
"    /HEADER:{{<tag>:<value>}{<tag>:<value>}...}",
" ",
"  For a listing of valid <tag> value and <value> formats see RFC 1945:",
"  \"Hypertext Transfer Protocol -- HTTP/1.0\".  A maximum of eight headers",
"  may be specified.",
" ",
"/USER:<name>",
"  In case a page requires a username for access.",
" ",
"/PASSWORD:<password>",
"  In case a page requires a password for access.",
" ",
"/ARRAY:<arrayname>",
"  Tells Kermit to store the response headers in the given array, one line",
"  per element.  The array need not be declared in advance.  Example:",
" ",
"    http /array:c get kermit/index.html",
"    show array c",
"    Dimension = 9",
"    1. Date: Fri, 26 Nov 1999 23:12:22 GMT",
"    2. Server: Apache/1.3.4 (Unix)",
"    3. Last-Modified: Mon, 06 Sep 1999 22:35:58 GMT",
"    4. ETag: \"bc049-f72-37d441ce\"",
"    5. Accept-Ranges: bytes",
"    6. Content-Length: 3954",
"    7. Connection: close     ",
"    8. Content-Type: text/html",
" ",
"As you can see, the header lines are like MIME e-mail header lines:",
"identifier, colon, value.  The /ARRAY switch is the only method available",
"to a script to process the server responses for a POST or PUT command.",
" ",
"The HTTP commands are only applicable when a connection has already been",
"established to a host using the SET HOST command.",
" ",
#ifdef CK_SSL
"HTTP over SSLv3 or TLSv1 made be performed by using the /SSL or /TLS",
"protocol switches when establishing the connection with SET HOST.",
" ",
#endif /* CK_SSL */
""
};
#endif /* NOHTTP */

#ifdef CK_KERBEROS
static char *hmxxauth[] = {
"Syntax:",
"AUTHENTICATE { KERBEROS4, KERBEROS5 [ switches ] } <action> [ switches ]",
"  Obtains or destroys Kerberos tickets and lists information about them.",
"  Actions are INITIALIZE, DESTROY, and LIST-CREDENTIALS.  KERBEROS4 can be",
"  abbreviated K4 or KRB4; KERBEROS5 can be abbreviated K5 or KRB5.  Use ? to",
"  see which keywords, switches, or other quantities are valid at each point",
"  in the command.",
" ",
"  The actions are INITIALIZE, DESTROY, and LIST-CREDENTIALS:",
"   ",
"    AUTH { K4, K5 } { INITIALIZE [switches], DESTROY,",
"      LIST-CREDENTIALS [switches] }",
" ",
"  The INITIALIZE action is the most complex, and its format is different",
"  for Kerberos 4 and Kerberos 5.  The format for Kerberos 4 is:",
" ",
"  AUTH K4 INITIALIZE [ /INSTANCE:<name> /LIFETIME:<minutes> -",
"    /PASSWORD:<password> /PREAUTH /REALM:<name> <principal> ]",
" ",
"  All switches are optional.  Kerberos 4 INITIALIZE switches are:",
" ",
"  /INSTANCE:<name>",
"    Allows an Instance (such as a hostname) to be specified.",
" ",
"  /LIFETIME:<number>",
"    Specifies the requested lifetime in minutes for the ticket.  If no",
"    lifetime is specified, 600 minutes is used.  If the lifetime is greater",
"    than the maximum supported by the ticket granting service, the resulting",
"    lifetime is shortened accordingly.",
" ",
"  /NOT-PREAUTH",
"    Instructs Kermit to send a ticket getting ticket (TGT) request to the",
"    KDC without any preauthentication data.",
" ",
"  /PASSWORD:<string>",
"    Allows a password to be included on the command line or in a script",
"    file.  If no /PASSWORD switch is included, you are prompted on a separate"
,
"    line.  The password switch is provided on a use-at-your-own-risk basis",
"    for use in automated scripts.  WARNING: Passwords should not be stored in"
,
"    files.",
" ",
"  /PREAUTH",
"    Instructs Kermit to send a preauthenticated Ticket-Getting Ticket (TGT)",
"    request to the KDC instead of a plaintext request.  The default when",
"    supported by the Kerberos libraries.",
" ",
"  /REALM:<name>",
"    Allows a realm to be specified (overriding the default realm).",
" ",
"  <principal>",
"    Your identity in the given or default Kerberos realm, of the form:",
"    userid[.instance[.instance]]@[realm]  ",
"    Can be omitted if it is the same as your username or SET LOGIN USERID",
"    value on the client system.",
" ",
"  The format for Kerberos 5 is as follows:",
" ",
"  AUTH K5 [ /CACHE:<filename> ] { INITIALIZE [ switches ], DESTROY,",
"    LIST-CREDENTIALS ...}",
" ",
"The INITIALIZE command for Kerberos 5 can include a number of switches;",
"all are optional:",
" ",
"AUTH K5 [ /CACHE:<filename> ] INITITIALIZE [ /ADDRESSES:<addr-list>",
"  /FORWARDABLE /KERBEROS4 /LIFETIME:<minutes> /PASSWORD:<password>",
"  /POSTDATE:<date-time> /PROXIABLE /REALM:<name> /RENEW /RENEWABLE:<minutes>",
"  /SERVICE:<name> /VALIDATE <principal> ]",
" ",
"  All Kerberos 5 INITIALIZE switches are optional:",
" ",
"  /ADDRESSES:{list of ip-addresses}",
"    Specifies a list of IP addresses that should be placed in the Ticket",
"    Getting Ticket in addition to the local machine addresses.",
" ",
"  /FORWARDABLE",
"    Requests forwardable tickets.",
" ",
"  /INSTANCE:<name>",
"    Allows an Instance (such as a hostname) to be specified.",
" ",
"  /KERBEROS4",
"    Instructs Kermit to get Kerberos 4 tickets in addition to Kerberos 5",
"    tickets.  If Kerberos 5 tickets are not supported by the server, a",
"    mild warning is printed and Kerberos 4 tickets are requested.",
" ",
"  /LIFETIME:<number>",
"    Specifies the requested lifetime in minutes for the ticket.  If no",
"    lifetime is specified, 600 minutes is used.  If the lifetime is greater",
"    than the maximum supported by the ticket granting service, the resulting",
"    lifetime is shortened.",
" ",
"  /NO-KERBEROS4",
"    Instructs Kermit to not attempt to retrieve Kerberos 4 credentials.",
" ",
"  /NOT-FORWARDABLE",
"    Requests non-forwardable tickets.",
" ",
"  /NOT-PROXIABLE",
"    Requests non-proxiable tickets.",
" ",
"  /PASSWORD:<string>",
"    Allows a password to be included on the command line or in a script",
"    file.  If no /PASSWORD switch is included, you are prompted on a separate"
,
"    line.  The password switch is provided on a use-at-your-own-risk basis",
"    for use in automated scripts.  WARNING: Passwords should not be stored in"
,
"    files.",
" ",
"  /POSTDATE:<date-time>",
"    Requests a postdated ticket, valid starting at <date-time>.  Postdated",
"    tickets are issued with the invalid flag set, and need to be fed back to",
"    the KDC before use with the /VALIDATE switch.  Type HELP DATE for info",
"    on date-time formats.",
" ",
"  /PROXIABLE",
"    Requests proxiable tickets.",
" ",
"  /REALM:<string>",
"    Allows an alternative realm to be specified.",
" ",
"  /RENEW",
"    Requests renewal of a renewable Ticket-Granting Ticket.  Note that ",
"    an expired ticket cannot be renewed even if it is within its renewable ",
"    lifetime.",
" ",
"  /RENEWABLE:<number>",
"    Requests renewable tickets, with a total lifetime of <number> minutes.",
" ",
"  /SERVICE:<string>",
"    Allows a service other than the ticket granting service to be specified.",
" ",
"  /VALIDATE",
"    Requests that the Ticket Granting Ticket in the cache (with the invalid",
"    flag set) be passed to the KDC for validation.  If the ticket is within",
"    its requested time range, the cache is replaced with the validated",
"    ticket.",
" ",
"  <principal>",
"    Your identity in the given or default Kerberos realm, of the form:",
"    userid[/instance][@realm]  ",
"    Can be omitted if it is the same as your username or SET LOGIN USERID",
"    value on the client system.",
" ",
"  Note: Kerberos 5 always attempts to retrieve a Ticket-Getting Ticket (TGT)",
"  using the preauthenticated TGT request.",
" ",
"  AUTHORIZE K5 LIST-CREDENTIALS [ /ADDRESSES /FLAGS /ENCRYPTION ]",
" ",
"  Shows start time, expiration time, service or principal name, plus",
"  the following additional information depending the switches:",
" ",
"  /ADDRESSES displays the hostnames and/or IP addresses embedded within",
"    the tickets.",
" ",
"  /FLAGS provides the following information (if applicable) for each ticket:",
"    F - Ticket is Forwardable",
"    f - Ticket was Forwarded",
"    P - Ticket is Proxiable",
"    p - Ticket is a Proxy",
"    D - Ticket may be Postdated",
"    d - Ticket has been Postdated",
"    i - Ticket is Invalid",
"    R - Ticket is Renewable",
"    I - Ticket is the Initial Ticket",
"    H - Ticket has been authenticated by Hardware",
"    A - Ticket has been Pre-authenticated",
" ",
"  /ENCRYPTION displays the encryption used by each ticket (if applicable):",
"    DES-CBC-CRC",
"    DES-CBC-MD4",
"    DES-CBC-MD5",
"    DES3-CBC-SHA",
""
};
#endif /* CK_KERBEROS */

#ifndef NOCSETS
static char *hmxxassoc[] = {
"ASSOCIATE FILE-CHARACTER-SET <file-character-set> <transfer-character-set>",
"  Tells C-Kermit that whenever the given file-character set is selected, and",
"  SEND CHARACTER-SET (q.v.) is AUTOMATIC, the given transfer character-set",
"  is selected automatically.",
" ",
"ASSOCIATE XFER-CHARACTER-SET <xfer-character-set> <file-character-set>",
"  Tells C-Kermit that whenever the given transfer-character set is selected,",
"  either by command or by an announcer attached to an incoming text file,",
"  and SEND CHARACTER-SET is AUTOMATIC, the specified file character-set is",
"  to be selected automatically.  Synonym: ASSOCIATE TRANSFER-CHARACTER-SET.",
" ",
"Use SHOW ASSOCIATIONS to list the current character-set associations, and",
"SHOW CHARACTER-SETS to list the current settings.",
""
};
#endif /* NOCSETS */

static char *hmxxwild[] = {

"A \"wildcard\" or \"regular expression\" is notation used in a filename",
"to match multiple files or in a search string when searching through text.",
"For example, in \"send *.txt\" the asterisk is a wildcard.  Kermit commands",
"that accept filenames also accepts wildcards, except commands that are",
"allowed to operate on only one file, such as TRANSMIT or COPY.",
"This version of Kermit accepts the following wildcards:",
" ",
"* Matches any sequence of zero or more characters.  For example, \"ck*.c\"",
"  matches all files whose names start with \"ck\" and end with \".c\"",
"  including \"ck.c\".",
" ",
#ifdef VMS
"% Matches any single character.  For example, \"ck%.c\" matches all files",
#else
"? Matches any single character.  For example, \"ck?.c\" matches all files",
#endif /* VMS */
"  whose names are exactly 5 characters long and start with \"ck\" and end",
#ifdef VMS
"  with \".c\".",
#else
"  with \".c\".  When typing commands at the prompt, you must precede any",
"  question mark to be used for matching by a backslash (\\) to override the",
"  normal function of question mark, which is providing menus and file lists.",
#endif /* VMS */
" ",
#ifdef OS2ORUNIX
#ifdef CKREGEX
"[abc]",
"  Square brackets enclosing a list of characters matches any character in",
"  the list.  Example: ckuusr.[ch] matches ckuusr.c and ckuusr.h.",
" ",
"[a-z]",
"  Square brackets enclosing a range of characters matches any character in",
"  the range; a hyphen (-) separates the low and high elements of the range.",
"  For example, [a-z] matches any character from a to z.",
" ",
"[acdm-z]",
"  Lists and ranges may be combined.  This example matches a, c, d, or any",
"  letter from m through z.",
" ",
"{string1,string2,...}",
"  Braces enclose a list of strings to be matched.  For example:",
"  ck{ufio,vcon,cmai}.c matches ckufio.c, ckvcon.c, or ckcmai.c.  The strings",
"  may themselves contain *, ?, [abc], [a-z], or other lists of strings.",
#endif /* CKREGEX */
#endif /* OS2ORUNIX */
#ifndef NOSPL
" ",
"To force a special pattern character to be taken literally, precede it with",
"a backslash, e.g. [a\\-z] matches a, hyphen, and z rather than a through z.",
" ",
"The same wildcard syntax can be used for patterns in the IF MATCH command,",
"in SWITCH case labels, the \\fsearch(), \\frsearch(), \\fpattern(), and",
"\\farraylook() functions, and in file binary- and text-patterns (see HELP IF,"
,
"HELP SWITCH, HELP FUNC, HELP SET FILE).",
#endif /* NOSPL */
"" };

#ifndef NOXFER
static char *hmxxfast[] = {
"FAST, CAUTIOUS, and ROBUST are predefined macros that set several",
"file-transfer parameters at once to achieve the desired file-transfer goal.",
"FAST chooses a large packet size, a large window size, and a fair amount of",
"control-character unprefixing at the risk of possible failure on some",
"connections.  FAST is the default tuning in C-Kermit 7.0 and later.  In case",
"FAST file transfers fail for you on a particular connection, try CAUTIOUS.",
"If that fails too, try ROBUST.  You can also change the definitions of each",
"macro with the DEFINE command.  To see the current definitions, type",
"\"show macro fast\", \"show macro cautious\", or \"show macro robust\".",
""
};
#endif /* NOXFER */

#ifdef VMS
static char * hmxxpurge[] = {
"Syntax: PURGE [ switches ] [ filespec ]",
"  Runs the DCL PURGE command.  Switches and filespec are not parsed or",
"  verified by Kermit, but passed directly to DCL.",
""
};
#else
#ifdef CKPURGE
static char * hmxxpurge[] = {
"Syntax: PURGE [ switches ] [ filespec ]",
"  Deletes backup files; that is, files whose names end in \".~n~\", where",
"  n is a number.  PURGE by itself deletes all backup files in the current",
"  directory.  Switches:",

" ",
"/AFTER:date-time",
#ifdef VMS
"  Specifies that only those files created after the given date-time are",
#else
"  Specifies that only those files modified after the given date-time are",
#endif /* VMS */
"  to be purged.  HELP DATE for info about date-time formats.",
" ",
"/BEFORE:date-time",
#ifdef VMS
"  Specifies that only those files modified before the given date-time",
#else
"  Specifies that only those files modified before the given date-time",
#endif /* VMS */
"  are to be purged.",
" ",
"/NOT-AFTER:date-time",
#ifdef VMS
"  Specifies that only those files modified at or before the given date-time",
#else
"  Specifies that only those files modified at or before the given date-time",
#endif /* VMS */
"  are to be purged.",
" ",
"/NOT-BEFORE:date-time",
#ifdef VMS
"  Specifies that only those files modified at or after the given date-time",
#else
"  Specifies that only those files modified at or after the given date-time",
#endif /* VMS */
"  are to be purged.",
" ",
"/LARGER-THAN:number",
"  Specifies that only those files longer than the given number of bytes are",
"  to be purged.",
" ",
"/SMALLER-THAN:number",
"  Specifies that only those files smaller than the given number of bytes are",
"  to be sent.",
" ",
"/EXCEPT:pattern",
"  Specifies that any files whose names match the pattern, which can be a",
"  regular filename or may contain wildcards, are not to be purged.  To",
"  specify multiple patterns (up to 8), use outer braces around the group",
"  and inner braces around each pattern:",
" ",
"    /EXCEPT:{{pattern1}{pattern2}...}",
" ",
#ifdef UNIXOROSK
"/DOTFILES",
"  Include (purge) files whose names begin with \".\".",
" ",
"/NODOTFILES",
"  Skip (don't purge) files whose names begin with \".\".",
" ",
#endif /* UNIXOROSK */
#ifdef RECURSIVE
"/RECURSIVE",
"  Descends through the current or specified directory tree.",
" ",
#endif /* RECURSIVE */
"/KEEP:n",
"  Retain the 'n' most recent (highest-numbered) backup files for each file.",
"  By default, none are kept.  If /KEEP is given without a number, 1 is used.",
" ",
"/LIST",
"  Display each file as it is processed and say whether it is purged or kept.",
"  Synonyms: /LOG, /VERBOSE.",
" ",
"/NOLIST",
"  The PURGE command should operate silently (default).",
"  Synonyms: /NOLOG, /QUIET.",
" ",
"/HEADING",
"  Print heading and summary information.",
" ",
"/NOHEADING",
"  Don't print heading and summary information.",
" ",
"/PAGE",
"  When /LIST is in effect, pause at the end of each screenful, even if",
"  COMMAND MORE-PROMPTING is OFF.",
" ",
"/NOPAGE",
"  Don't pause, even if COMMAND MORE-PROMPTING is ON.",
" ",
"/ASK",
"  Interactively ask permission to delete each backup file.",
" ",
"/NOASK",
"  Purge backup files without asking permission.",
" ",
"/SIMULATE",
"  Inhibits the actual deletion of files; use to preview which files would",
"  actually be deleted.  Implies /LIST.",
" ",
"Use SET OPTIONS PURGE [ switches ] to change defaults; use SHOW OPTIONS to",
"display customized defaults.  Also see HELP DELETE, HELP WILDCARD.",
""
};
#endif /* CKPURGE */
#endif /* VMS */

static char *hmxxclo[] = {
"Syntax:  CLOSE [ item ]",
"  Close the indicated item.  The default item is CONNECTION, which is the",
"  current SET LINE or SET HOST connection.  The other items are:",
" ",
#ifdef CKLOGDIAL
"    CX-LOG          (connection log, opened with LOG CX)",
#endif /* CKLOGDIAL */
#ifndef NOLOCAL
"    SESSION-LOG     (opened with LOG SESSION)",
#endif /* NOLOCAL */
#ifdef TLOG
"    TRANSACTION-LOG (opened with LOG TRANSACTIONS)",
#endif /* TLOG */
"    PACKET-LOG      (opened with LOG PACKETS)",
#ifdef DEBUG
"    DEBUG-LOG       (opened with LOG DEBUG)",
#endif /* DEBUG */
#ifndef NOSPL
"    READ-FILE       (opened with OPEN READ)",
"    WRITE-FILE      (opened with OPEN WRITE or OPEN APPEND)",
#endif /* NOSPL */
" ",
"Type HELP LOG and HELP OPEN for further info.", "" };

#ifdef CK_MINPUT
static char *hmxxminp[] = {
"Syntax:  MINPUT n [ string1 [ string2 [ ... ] ] ]",
"Example: MINPUT 5 Login: {Username: } {NO CARRIER} BUSY RING",
"  For use in script programs.  Waits up to n seconds for any one of the",
"  strings to arrive on the communication device.  If no strings are given,",
"  the command waits for any character at all to arrive.  Strings are",
"  separated by spaces; use { braces } for grouping.  If any of the strings",
"  is encountered within the timeout interval, the command succeeds and the",
"  \\v(minput) variable is set to the number of the string that was matched:",
"  1, 2, 3, etc.  If none of the strings arrives, the command times out,",
"  fails, and \\v(minput) is set to 0.",
" ",
"Also see: INPUT, REINPUT, SET INPUT.",
"" };
#endif /* CK_MINPUT */

#ifndef NOLOCAL
static char *hmxxcon[] = {
"Syntax: CONNECT (or C, or CQ) [ switches ]",
"  Connect to a remote computer via the serial communications device given in",
#ifdef OS2
"  the most recent SET PORT command, or to the network host named in the most",
#else
"  the most recent SET LINE command, or to the network host named in the most",
#endif /* OS2 */
"  recent SET HOST command.  Type the escape character followed by C to get",
"  back to the C-Kermit prompt, or followed by ? for a list of CONNECT-mode",
#ifdef OS2
"  escape commands.  You can also assign the \\Kexit verb to the key or",
"  key-combination of your choice; by default it is assigned to Alt-x.",
#else
"  escape commands.",
" ",
"Include the /QUIETLY switch to suppress the informational message that",
"tells you how to escape back, etc.  CQ is a synonym for CONNECT /QUIETLY.",
#endif /* OS2 */
" ",
"Other switches include:",
#ifdef CK_TRIGGER
" ",
"/TRIGGER:string",
"  One or more strings to look for that will cause automatic return to",
"  command mode.  To specify one string, just put it right after the",
"  colon, e.g. \"/TRIGGER:Goodbye\".  If the string contains any spaces, you",
"  must enclose it in braces, e.g. \"/TRIGGER:{READY TO SEND...}\".  To",
"  specify more than one trigger, use the following format:",
" ",
"    /TRIGGER:{{string1}{string2}...{stringn}}",
" ",
"  Upon return from CONNECT mode, the variable \\v(trigger) is set to the",
"  trigger string, if any, that was actually encountered.  This value, like",
"  all other CONNECT switches applies only to the CONNECT command with which",
"  it is given, and overrides (temporarily) any global SET TERMINAL TRIGGER",
"  string that might be in effect.",
#endif /* CK_TRIGGER */
#ifdef OS2
" ",
"/IDLE-LIMIT:number",
"  The number of seconds of idle time, after which Kermit returns",
"  automatically to command mode; default 0 (no limit).",
" ",
"/IDLE-INTERVAL:number",
"  The number of seconds of idle time, after which Kermit automatically",
"  transmits the idle string.",
" ",
"/IDLE-STRING:string",
"  The string to transmit whenever the idle interval has passed.",
" ",
"/TIME-LIMIT:number",
"  The maximum number of seconds for which the CONNECT session may last.",
"  The default is 0 (no limit).  If a nonzero number is given, Kermit returns",
"  automatically to command mode after this many seconds.",
#endif /* OS2 */
"" };
#endif /* NOLOCAL */

static char *hmxxmget[] = {
"Syntax: MGET [ switches... ] remote-filespec [ remote-filespec ... ]",
" ",
"Just like GET (q.v.) except allows a list of remote file specifications,",
"separated by spaces.",
""
};

static char *hmxxget[] = {
"Syntax: GET [ switches... ] remote-filespec [ as-name ]",
"  Tells the other Kermit, which must be in (or support autoswitching into)",
"  server mode, to send the named file or files.  If the remote-filespec or",
"  the as-name contain spaces, they must be enclosed in braces.  If as-name",
"  is the name of an existing local directory, incoming files are placed in",
"  that directory; if it is the name of directory that does not exist, Kermit",
"  tries to create it.  Optional switches include:",
" ",
"/AS-NAME:text",
"  Specifies \"text\" as the name to store the incoming file under, or",
"  directory to store it in.  You can also specify the as-name as the second",
"  filename on the GET command line.",
" ",
"/BINARY",
"  Performs this transfer in binary mode without affecting the global",
"  transfer mode.",
" ",
"/COMMAND",
"  Receives the file into the standard input of a command, rather than saving",
"  it on  disk.  The /AS-NAME or the second \"filename\" on the GET command",
"  line is interpreted as the name of a command.",
" ",
"/DELETE",
"  Asks the other Kermit to delete the file (or each file in the group)",
"  after it has been transferred successfully.",
" ",
"/EXCEPT:pattern",
"  Specifies that any files whose names match the pattern, which can be a",
"  regular filename, or may contain \"*\" and/or \"?\" metacharacters,",
"  are to be refused.  To specify multiple patterns (up to 8), use outer",
"  braces around the group, and inner braces around each pattern:",
" ",
"    /EXCEPT:{{pattern1}{pattern2}...}",
" ",
"/FILENAMES:{CONVERTED,LITERAL}",
"  Overrides the global SET FILE NAMES setting for this transfer only.",
" ",
"/FILTER:command",
"  Causes the incoming file to passed through the given command (standard",
"  input/output filter) before being written to disk.",
" ",
#ifdef VMS
"/IMAGE",
"  Transfer in image mode.",
" ",
#endif /* VMS */
#ifdef CK_LABELED
"/LABELED",
"  VMS and OS/2 only: Specifies labeled transfer mode.",
" ",
#endif /* CK_LABELED */

"/MOVE-TO:directory-name",
"  Specifies that each file that arrives should be moved to the specified",
"  directory after, and only if, it has been received successfully.",
" ",
"/PATHNAMES:{OFF,ABSOLUTE,RELATIVE,AUTO}",
"  Overrides the global SET RECEIVE PATHNAMES setting for this transfer.",
" ",
"/QUIET",
"  When sending in local mode, this suppresses the file-transfer display.",
" ",
"/RECOVER",
"  Used to recover from a previously interrupted transfer; GET /RECOVER",
"  is equivalent REGET.  Works only in binary mode.",
" ",
"/RECURSIVE",
"  Tells the server to descend through the directory tree when locating",
"  the files to be sent.",
" ",
"/RENAME-TO:string",
"  Specifies that each file that arrives should be renamed as specified",
"  after, and only if, it has been received successfully.  The string should",
"  normally contain variables like \\v(filename) or \\v(filenum).",
" ",
"/TEXT",
"  Performs this transfer in text mode without affecting the global",
"  transfer mode.",
" ",
"Also see HELP MGET, HELP SEND, HELP RECEIVE, HELP SERVER, HELP REMOTE.",
""};

static char *hmxxlg[] = {
"Syntax: LOG (or L) filename [ { NEW, APPEND } ]",
" ",
"Record information in a log file:",
" ",
#ifdef CKLOGDIAL
"CX",
"  Connections made with SET LINE, SET PORT, SET HOST, DIAL, TELNET, etc.",
"  The default filename is CX.LOG in your home directory and APPEND is the",
"  default mode for opening.",
" ",
#endif /* CKLOGDIAL */
#ifdef DEBUG
"DEBUG",
"  Debugging information, to help track down bugs in the C-Kermit program.",
"  The default log name is debug.log in current directory.",
" ",
#endif /* DEBUG */
"PACKETS",
"  Kermit packets, to help with protocol problems.  The default filename is",
"  packet.log in current directory.",
" ",
#ifndef NOLOCAL
"SESSION",
"  Records your CONNECT session (default: session.log in current directory).",
" ",
#endif /* NOLOCAL */
#ifdef TLOG
"TRANSACTIONS",
"  Names and statistics about files transferred (default: transact.log in",
"  current directory; see HELP SET TRANSACTION-LOG for transaction-log format",
"  options.)",
" ",
#endif /* TLOG */
"If you include the APPEND keyword after the filename, the existing log file,",
"if any, is appended to; otherwise a new file is created (except APPEND is",
"the default for the connection log).  Use CLOSE <keyword> to stop logging.",
#ifdef OS2ORUNIX
" ",
"Note: The filename can also be a pipe, e.g.:",
" ",
"  log transactions |lpr",
"  log debug {| grep \"^TELNET\" > debug.log}",
" ",
"Braces are required if the pipeline or filename contains spaces.",
#endif /* OS2ORUNIX */
"" };

#ifndef NOSCRIPT
static char *hmxxlogi[] = { "\
Syntax: SCRIPT text",
"  A limited and cryptic \"login assistant\", carried over from old C-Kermit",
"  releases for comptability, but not recommended for use.  Instead, please",
"  use the full script programming language described in chapters 17-19 of",
"  \"Using C-Kermit\".",
" ",
"  Login to a remote system using the text provided.  The login script",
"  is intended to operate similarly to UNIX uucp \"L.sys\" entries.",
"  A login script is a sequence of the form:",
" ",
"    expect send [expect send] . . .",
" ",
"  where 'expect' is a prompt or message to be issued by the remote site, and",
"  'send' is the names, numbers, etc, to return.  The send may also be the",
"  keyword EOT to send Control-D, or BREAK (or \\\\b) to send a break signal.",
"  Letters in send may be prefixed by ~ to send special characters:",
" ",
"  ~b backspace, ~s space, ~q '?', ~n linefeed, ~r return, ~c don\'t",
"  append a return, and ~o[o[o]] for octal of a character.  As with some",
"  UUCP systems, sent strings are followed by ~r unless they end with ~c.",
" ",
"  Only the last 7 characters in each expect are matched.  A null expect,",
"  e.g. ~0 or two adjacent dashes, causes a short delay.  If you expect",
"  that a sequence might not arrive, as with uucp, conditional sequences",
"  may be expressed in the form:",
" ",
"    -send-expect[-send-expect[...]]",
" ",
"  where dashed sequences are followed as long as previous expects fail.",
"" };
#endif /* NOSCRIPT */

#ifndef NOFRILLS
static char * hmxxtyp[] = {
"Syntax: TYPE [ switches... ] file",
"  Displays a file on the screen.  Pauses automatically at end of each",
"  screenful if COMMAND MORE-PROMPTING is ON.  Optional switches:",
" ",
"  /PAGE",
"     Pause at the end of each screenful even if COMMAND MORE-PROMPTING OFF.",
"     Synonym: /MORE",
"  /NOPAGE",
"     Don't pause at the end of each screen even if COMMAND MORE-PROMPTING ON."
,
"  /HEAD:n",
"     Only type the first 'n' lines of the file.",
"  /TAIL:n",
"     Only type the last 'n' lines of the file.",
"  /MATCH:pattern",
"     Only type lines that match the given pattern.  HELP WILDCARDS for info",
"     info about patterns.  /HEAD and /TAIL apply after /MATCH.",
"  /PREFIX:string",
"     Print the given string at the beginning of each line.",
"  /WIDTH:number",
"     Truncate each line at the given column number before printing.",
"  /COUNT",
"     Count lines (and matches) and print the count(s) but not the lines.",
" ",
"You can use SET OPTIONS TYPE to set the defaults for /PAGE or /NOPAGE and",
"/WIDTH.  Use SHOW OPTIONS to see current TYPE options.",
""
};

static char * hmxxcle[] = {
"Syntax: CLEAR [ item-name ]",
" ",
"Clears the named item.  If no item is named, DEVICE-AND-INPUT is assumed.",
" ",
"  ALARM            Clears any pending alarm (see SET ALARM).",
#ifdef CK_APC
"  APC-STATUS       Clears Application Program Command status.",
#endif /* CK_APC */
#ifdef PATTERNS
"  BINARY-PATTERNS  Clears the file binary-patterns list.",
#endif /* PATTERNS */
#ifdef OS2
"  COMMAND-SCREEN   Clears the current command screen.",
#endif /* OS2 */
"  DEVICE           Clears the current port or network input buffer.",
"  DEVICE-AND-INPUT Clears both the device and the INPUT buffer.",
"  DIAL-STATUS      Clears the \\v(dialstatus) variable.",
"  \
INPUT            Clears the INPUT command buffer and the \\v(input) variable.",
#ifdef OS2
"  \
SCROLLBACK       empties the scrollback buffer including the current screen.",
#endif /* OS2 */
"  SEND-LIST        Clears the current SEND list (see ADD).",
#ifdef OS2
"  \
TERMINAL-SCREEN  Clears the current screen a places it into the scrollback.",
"    buffer.",
#endif /* OS2 */
#ifdef PATTERNS
"  TEXT-PATTERNS    Clears the file text-patterns list.",
#endif /* PATTERNS */
""};
#endif /* NOFRILLS */

static char * hmxxdate[] = {
"Syntax: DATE [ date-time ]",
"  Prints the current date and time in standard format: yyyymmdd_hh:mm:ss.",
"  If a date-time is given, converts it to standard format.  Various date-time"
,"  formats are acceptable.  The rules for the date-time are:",
" ",
"  . The date, if given, must precede the time.",
"  . The year must be four digits.",
"  . If the year comes first, the second field is the month.",
"  . The day, month, and year may be separated by spaces, /, -, or underscore."
,"  . The date and time may be separated by spaces or underscore.",
"  . The month may be numeric (1 = January) or spelled out or abbreviated in",
"    English.",
"  . The time may be in 24-hour format or 12-hour format.",
"  . If the hour is 12 or less, AM is assumed unless AM or PM is included.",
"  . If the date is omitted but a time is given, the current date is assumed.",
"  . If the time is given but date omitted, 00:00:00 is assumed.",
"  . If both the date and time are omitted, the current date and time are",
"    assumed.",
" ",
"  The following shortcuts can also be used:",
" ",
"  TODAY",
"    Today's date, optionally followed by a time; 00:00:00 if no time given.",
" ",
"  YESTERDAY",
"    Yesterday's date, optionally followed by a time (default 00:00:00).",
" ",
"  TOMORROW",
"    Tomorrows's date, optionally followed by a time (default 00:00:00).",
" ",
" + <number> <timeunits>",
"  A date in the future relative to the current date; <timeunits> may be DAYS",
"  WEEKS, MONTHS, or YEARS.  Optionally followed by a time (default 00:00:00)."
,
"  Examples: +3days, +7weeks.",
" ",
" - <number> <timeunits>",
"  A date in the past relative to the current date, optionally followed by a",
"  time (default 00:00:00).  Examples: -1year, -37months.",
" ",
"All the formats shown above are acceptable as arguments to date-time switches"
,
"such as /AFTER: or /BEFORE, and to functions such as \\fcvtdate() that take",
"date-time strings as arguments.",
""
};


#ifndef NOXFER
static char * hmxxsen[] = {
"Syntax: SEND (or S) [ switches...] [ filespec [ as-name ] ]",
"  Sends the file or files specified by filespec.  If the filespec is omitted",
"  the SEND-LIST is used (HELP ADD for more info).  The filespec may contain",
"  wildcard characters.  An 'as-name' may be given to specify the name(s)",
"  the files(s) are sent under; if the as-name is omitted, each file is",
"  sent under its own name.  Also see HELP MSEND, HELP WILDCARD.",
"  Optional switches include:",
" ",
#ifndef NOSPL
"/ARRAY:<arrayname>",
"  Specifies that the data to be sent comes from the given array, such as",
"  \\&a[].  A range may be specified, e.g. SEND /ARRAY:&a[100:199].  Leave",
"  the brackets empty or omit them altogether to send the whole 1-based array."
,
"  Include /TEXT to have Kermit supply a line terminator at the end of each",
"  array element (and translate character sets if character-set translations",
"  are set up), or /BINARY to treat the array as one long string of characters"
,
"  to be sent as-is.  If an as-name is not specified, the array is sent with",
"  the name _ARRAY_X_, where \"X\" is replaced by actual array letter.",
" ",
#endif /* NOSPL */

"/AS-NAME:<text>",
"  Specifies <text> as the name to send the file under instead of its real",
"  name.  This is equivalent to giving an as-name after the filespec.",
" ",
"/BINARY",
"  Performs this transfer in binary mode without affecting the global",
"  transfer mode.",
" ",
"/TEXT",
"  Performs this transfer in text mode without affecting the global",
"  transfer mode.",
" ",
"/NOBACKUP",
"  Skip (don't send) Kermit or EMACS backup files (files with names that",
"  end with .~n~, where n is a number).",
" ",
#ifdef UNIXOROSK
"/DOTFILES",
"  Include (send) files whose names begin with \".\".",
" ",
"/NODOTFILES",
"  Don't send files whose names begin with \".\".",
" ",
#endif /* UNIXOROSK */

#ifdef VMS
"/IMAGE",
"  Performs this transfer in image mode without affecting the global",
"  transfer mode.",
" ",
#endif /* VMS */
#ifdef CK_LABELED
"/LABELED",
"  Performs this transfer in labeled mode without affecting the global",
"  transfer mode.",
" ",
#endif /* CK_LABELED */
"/COMMAND",
"  Sends the output from a command, rather than the contents of a file.",
"  The first \"filename\" on the SEND command line is interpreted as the name",
"  of a command; the second (if any) is the as-name.",
" ",
"/FILENAMES:{CONVERTED,LITERAL}",
"  Overrides the global SET FILE NAMES setting for this transfer only.",
" ",
"/PATHNAMES:{OFF,ABSOLUTE,RELATIVE}",
"  Overrides the global SET SEND PATHNAMES setting for this transfer.",
" ",
"/FILTER:command",
"  Specifies a command \
(standard input/output filter) to pass the file through",
"  before sending it.",
" ",
"/DELETE",
"  Deletes the file (or each file in the group) after it has been sent",
"  successfully (applies only to real files).",
" ",
"/QUIET",
"  When sending in local mode, this suppresses the file-transfer display.",
" ",
"/RECOVER",
"  Used to recover from a previously interrupted transfer; SEND /RECOVER",
"  is equivalent RESEND (use in binary mode only).",
" ",
"/RECURSIVE",
"  Tells C-Kermit to look not only in the given or current directory for",
"  files that match the filespec, but also in all its subdirectories, and",
"  all their subdirectories, etc.",
" ",
"/RENAME-TO:name",
"  Tells C-Kermit to rename each source file that is sent successfully to",
"  the given name (usually you should include \\v(filename) in the new name,",
"  which is replaced by the original filename.",
" ",
"/MOVE-TO:directory",
"  Tells C-Kermit to move each source file that is sent successfully to",
"  the given directory.",
" ",
"/STARTING:number",
"  Starts sending the file from the given byte position.",
"  SEND /STARTING:n filename is equivalent to PSEND filename n.",
" ",
"/SUBJECT:text",
"  Specifies the subject of an email message, to be used with /MAIL.  If the",
"  text contains spaces, it must be enclosed in braces.",
" ",
"/MAIL:address",
"  Sends the file as e-mail to the given address; use with /SUBJECT:.",
" ",
"/PRINT:options",
"  Sends the file to be printed, with optional options for the printer.",
" ",
#ifdef CK_XYZ
"/PROTOCOL:name",
"  Uses the given protocol to send the file (Kermit, Zmodem, etc) for this",
"  transfer without changing global protocol.",
" ",
#endif /* CK_XYZ */
"/AFTER:date-time",
#ifdef VMS
"  Specifies that only those files created after the given date-time are",
#else
"  Specifies that only those files modified after the given date-time are",
#endif /* VMS */
"  to be sent.  HELP DATE for info about date-time formats.",
" ",
"/BEFORE:date-time",
#ifdef VMS
"  Specifies that only those files modified before the given date-time",
#else
"  Specifies that only those files modified before the given date-time",
#endif /* VMS */
"  are to be sent.",
" ",
"/NOT-AFTER:date-time",
#ifdef VMS
"  Specifies that only those files modified at or before the given date-time",
#else
"  Specifies that only those files modified at or before the given date-time",
#endif /* VMS */
"  are to be sent.",
" ",
"/NOT-BEFORE:date-time",
#ifdef VMS
"  Specifies that only those files modified at or after the given date-time",
#else
"  Specifies that only those files modified at or after the given date-time",
#endif /* VMS */
"  are to be sent.",
" ",
"/LARGER-THAN:number",
"  Specifies that only those files longer than the given number of bytes are",
"  to be sent.",
" ",
"/SMALLER-THAN:number",
"  Specifies that only those files smaller than the given number of bytes are",
"  to be sent.",
" ",
"/EXCEPT:pattern",
"  Specifies that any files whose names match the pattern, which can be a",
"  regular filename, or may contain \"*\" and/or \"?\" metacharacters,",
"  are not to be sent.  To specify multiple patterns (up to 8), use outer",
"  braces around the group, and inner braces around each pattern:",
" ",
"    /EXCEPT:{{pattern1}{pattern2}...}",
" ",
"/LISTFILE:filename",
"  Specifies the name of a file that contains the list of names of files",
"  that are to be sent.  The filenames should be listed one name per line",
"  in this file (but a name can contain wildcards).",
" ",
"Also see HELP RECEIVE, HELP GET, HELP SERVER, HELP REMOTE.",
""};

static char *hmxxrc[] = {
"Syntax: RECEIVE (or R) [ switches... ] [ as-name ]",
"  Wait for a file to arrive from the other Kermit, which must be given a",
"  SEND command.  If the optional as-name is given, the incoming file or",
"  files are stored under that name, otherwise it will be stored under",
#ifndef CK_TMPDIR
"  the name it arrives with.",
#else
#ifdef OS2
"  the name it arrives with.  If the filespec denotes a disk and/or",
"  directory, the incoming file or files will be stored there.",
#else
"  the name it arrives with.  If the filespec denotes a directory, the",
"  incoming file or files will be placed in that directory.",
#endif /* OS2 */
#endif /* CK_TMPDIR */
" ",
"Optional switches include:",
" ",
"/AS-NAME:text",
"  Specifies \"text\" as the name to store the incoming file under.",
"  You can also specify the as-name as a filename on the command line.",
" ",
"/BINARY",
"  Skips text-mode conversions unless the incoming file arrives with binary",
"  attribute",
" ",
"/COMMAND",
"  Receives the file into the standard input of a command, rather than saving",
"  it on disk.  The /AS-NAME or the \"filename\" on the RECEIVE command line",
"  is interpreted as the name of a command.",
" ",
"/EXCEPT:pattern",
"  Specifies that any files whose names match the pattern, which can be a",
"  regular filename, or may contain \"*\" and/or \"?\" metacharacters,",
"  are to be refused.  To specify multiple patterns (up to 8), use outer",
"  braces around the group, and inner braces around each pattern:",
" ",
"    /EXCEPT:{{pattern1}{pattern2}...}",
" ",
"/FILENAMES:{CONVERTED,LITERAL}",
"  Overrides the global SET FILE NAMES setting for this transfer only.",
" ",
"/FILTER:command",
"  Causes the incoming file to passed through the given command (standard",
"  input/output filter) before being written to disk.",
" ",
#ifdef VMS
"/IMAGE",
"  Receives the file in image mode.",
" ",
#endif /* VMS */
#ifdef CK_LABELED
"/LABELED",
"  Specifies labeled transfer mode.",
" ",
#endif /* CK_LABELED */

"/MOVE-TO:directory-name",
"  Specifies that each file that arrives should be moved to the specified",
"  directory after, and only if, it has been received successfully.",
" ",
"/PATHNAMES:{OFF,ABSOLUTE,RELATIVE,AUTO}",
"  Overrides the global SET RECEIVE PATHNAMES setting for this transfer.",
" ",
"/PROTOCOL:name",
"  Use the given protocol to receive the incoming file(s).",
" ",
"/QUIET",
"  When sending in local mode, this suppresses the file-transfer display.",
" ",
"/RECURSIVE",
"  Equivalent to /PATHNAMES:RELATIVE.",
" ",
"/RENAME-TO:string",
"  Specifies that each file that arrives should be renamed as specified",
"  after, and only if, it has been received successfully.  The string should",
"  normally contain variables like \\v(filename) or \\v(filenum).",
" ",
"/TEXT",
"  Forces text-mode conversions unless the incoming file has the binary",
"  attribute",
" ",
"Also see HELP SEND, HELP GET, HELP SERVER, HELP REMOTE.",
"" };

#ifndef NORESEND
static char *hmxxrsen = "\
Syntax: RESEND filespec [name]\n\n\
  Resend the file or files, whose previous transfer was interrupted.\n\
  Picks up from where previous transfer left off, IF the receiver was told\n\
  to SET FILE INCOMPLETE KEEP.  Only works for binary-mode transfers.\n\
  Requires the other Kermit to have RESEND capability.";

static char *hmxxrget = "\
Syntax: REGET filespec\n\n\
  Ask a server to RESEND a file to C-Kermit.";

static char *hmxxpsen = "\
Syntax: PSEND filespec position [name]\n\n\
  Just like SEND, except sends the file starting at the given byte position.";
#endif /* NORESEND */

#ifndef NOMSEND
static char *hmxxmse[] = {
"Syntax: MSEND [ switches... ] filespec [ filespec [ ... ] ]",
"  Sends the files specified by the filespecs.  One or more filespecs may be",
"  listed, separated by spaces.  Any or all filespecs may contain wildcards",
"  and they may be in different directories.  Alternative names cannot be",
"  given.  Switches include /BINARY /DELETE /MAIL /PROTOCOL /QUIET /RECOVER",
"  /TEXT; see HELP SEND for descriptions.",
""
};
#endif /* NOMSEND */

static char *hmxxadd[] = {
#ifndef NOMSEND
"ADD SEND-LIST filespec [ <mode> [ <as-name> ] ]",
"  Adds the specified file or files to the current SEND list.  Use SHOW",
"  SEND-LIST and CLEAR SEND-LIST to display and clear the list; use SEND",
"  by itself to send the files from it.",
" ",
#endif /* NOMSEND */
#ifdef PATTERNS
"ADD BINARY-PATTERNS [ <pattern> [ <pattern> ... ] ]",
"  Adds the pattern(s), if any, to the SET FILE BINARY-PATTERNS list.",
" ",
"ADD TEXT-PATTERNS [ <pattern> [ <pattern> ... ] ]",
"  Adds the pattern(s), if any, to the SET FILE TEXT-PATTERNS list.",
"  Use SHOW PATTERNS to see the lists.  See HELP SET FILE for further info.",
#endif /* PATTERNS */
""};

static char *hmxxremv[] = {
#ifdef PATTERNS
"REMOVE BINARY-PATTERNS [ <pattern> [ <pattern> ... ] ]",
"  Removes the pattern(s), if any, from the SET FILE BINARY-PATTERNS list",
" ",
"REMOVE TEXT-PATTERNS [ <pattern> [ <pattern> ... ] ]",
"  Removes the given patterns from the SET FILE TEXT-PATTERNS list.",
"  Use SHOW PATTERNS to see the lists.  See HELP SET FILE for further info.",
#endif /* PATTERNS */
""};
#endif /* NOXFER */

#ifndef NOSERVER
static char *hmxxser = "Syntax: SERVER\n\
  Enter server mode on the current connection.  All further commands\n\
  are taken in packet form from the other Kermit program.  Use FINISH,\n\
  BYE, or REMOTE EXIT to get C-Kermit out of server mode.";
#endif /* NOSERVER */

static char *hmhset[] = {
"  The SET command establishes communication, file, scripting, or other",
"  parameters.  The SHOW command can be used to display the values of",
"  SET parameters.  Help is available for each individual parameter;",
"  type HELP SET ? to see what's available.",
"" };

#ifndef NOSETKEY
static char *hmhskey[] = {
"Syntax: SET KEY k text",
"Or:     SET KEY CLEAR",
"  Configure the key whose \"scan code\" is k to send the given text when",
"  pressed during CONNECT mode.  SET KEY CLEAR restores all the default",
"  key mappings.  If there is no text, the default key binding is restored",
#ifndef NOCSETS
"  for the key k.  SET KEY mappings take place before terminal character-set",
"  translation.",
#else
"  the key k.",
#endif /* NOCSETS */
#ifdef OS2
" ",
"  The text may contain \"\\Kverbs\" to denote actions, to stand for DEC",
"  keypad, function, or editing keys, etc.  For a list of available keyboard",
"  verbs, type SHOW KVERBS.",
#endif /* OS2 */
" ",
"  To find out the scan code and mapping for a particular key, use the",
"  SHOW KEY command.",
""};
#endif /* NOSETKEY */

static char *hmxychkt[] = { "Syntax: SET BLOCK-CHECK type",
" ",
"  Type of packet block check to be used for error detection, 1, 2, 3, or",
"  BLANK-FREE-2.  Type 1 is standard, and catches most errors.  Types 2 and 3",
"  specify more rigorous checking at the cost of higher overhead.  The",
"  BLANK-FREE-2 type is the same as Type 2, but is guaranteed to contain no",
"  blanks.",
"" };

static char * hmxydeb[] = {
"Syntax: SET DEBUG { SESSION, ON, OFF, TIMESTAMP }",
" ",
"SET DEBUG ON",
#ifdef DEBUG
"  Opens a debug log file named debug.log in the current directory.",
"  Use LOG DEBUG if you want specify a different log file name or path.",
#else
"  (Has no effect in this version of Kermit.)",
#endif /* DEBUG */
" ",
"SET DEBUG OFF",
"  Stops debug logging and session debugging.",
" ",
"SET DEBUG SESSION",
#ifndef NOLOCAL
"  Displays control and 8-bit characters symbolically during CONNECT mode.",
"  Equivalent to SET TERMINAL DEBUG ON.",
#else
"  (Has no effect in this version of Kermit.)",
#endif /* NOLOCAL */
" ",
"SET DEBUG TIMESTAMP { ON, OFF }",
"  Enables/Disables timestamps on debug log entries.",
"" };

#ifdef CK_SPEED
static char *hmxyqctl[] = {
"Syntax: SET CONTROL-CHARACTER { PREFIXED, UNPREFIXED } { <code>..., ALL }",
" ",
"  <code> is the numeric ASCII code for a control character 1-31,127-159,255."
,
"  The word \"ALL\" means all characters in this range.",
" ",
"  PREFIXED <code> means the given control character must be converted to a",
"  printable character and prefixed, the default for all control characters.",
" ",
"  UNPREFIXED <code> means you think it is safe to send the given control",
"  character as-is, without a prefix.  USE THIS OPTION AT YOUR OWN RISK!",
" ",
"  SHOW CONTROL to see current settings.  SET CONTROL PREFIXED ALL is",
"  recommended for safety.  You can include multiple <code> values in one",
"  command, separated by spaces.",
"" };
#endif /* CK_SPEED */

#ifndef NODIAL
static char *hxymodm[] = {
"Syntax: SET MODEM <parameter> <value> ...",
" ",
"Note: Many of the SET MODEM parameters are configured automatically when",
"you SET MODEM TYPE, according to the modem's capabilities.  SHOW MODEM to",
"see them.  Also see HELP DIAL and HELP SET DIAL.",
" ",
"SET MODEM TYPE <name>",
"  Tells Kermit which kind of modem you have, so it can issue the appropriate",
"  modem-specific commands for configuration, dialing, and hanging up.  For a",
"  list of the modem types known to Kermit, type \"set modem type ?\".",
"  Use SET MODEM TYPE NONE (the default) for direct serial connections.  Use",
"  SET MODEM TYPE USER-DEFINED to use a type of modem that is not built in",
"  to Kermit, and then user SET MODEM CAPABILITIES, SET MODEM DIAL-COMMAND,",
"  and SET MODEM COMMAND to tell Kermit how to configure and control it.",
" ",
#ifdef UNIX
"  Give the SET MODEM TYPE command BEFORE the SET LINE command so Kermit can",
"  open the communications device in the correct mode for dialing.",
" ",
#endif /* UNIX */

"SET MODEM CAPABILITIES <list>",
"  Use this command for changing Kermit's idea of your modem's capabilities,",
"  for example, if your modem is supposed to have built-in error correction",
"  but in fact does not.  Also use this command to define the capabilities",
"  of a USER-DEFINED modem.  Capabilities are:",
" ",
"    AT      AT-commands",
"    DC      data-compression",
"    EC      error-correction",
"    HWFC    hardware-flow",
"    ITU     v25bis-commands",
"    SWFC    software-flow",
"    KS      kermit-spoof",
"    SB      speed-buffering",
"    TB      Telebit",
" ",
"SET MODEM CARRIER-WATCH { AUTO, ON, OFF }",
"  Synonym for SET CARRIER-WATCH (q.v.)",
" ",
"SET MODEM COMPRESSION { ON, OFF }",
"  Enables/disables the modem's data compression feature, if any.",
" ",
"SET MODEM DIAL-COMMAND <text>",
"  The text replaces Kermit's built-in modem dialing command.  It must",
"  include '%s' (percent s) as a place-holder for the telephone numbers",
"  given in your DIAL commands.",
" ",
"SET MODEM ERROR-CORRECTION { ON, OFF }",
"  Enables/disables the modem's error-correction feature, if any.",
" ",
"SET MODEM ESCAPE-CHARACTER number",
"  Numeric ASCII value of modem's escape character, e.g. 43 for '+'.",
"  For Hayes-compatible modems, Kermit uses three copies, e.g. \"+++\".",
" ",
"SET MODEM FLOW-CONTROL {AUTO, NONE, RTS/CTS, XON/XOFF}",
"  Selects the type of local flow control to be used by the modem.",
" ",
"SET MODEM HANGUP-METHOD { MODEM-COMMAND, RS232-SIGNAL }",
"  How hangup operations should be done.  MODEM-COMMAND means try to",
"  escape back to the modem's command processor and give a modem-specific",
"  hangup command.  RS232-SIGNAL means turn off the DTR signal.",
" ",
"SET MODEM KERMIT-SPOOF {ON, OFF}",
"  If the selected modem type supports the Kermit protocol directly,",
"  use this command to turn its Kermit protocol function on or off.",
" ",
"SET MODEM MAXIMUM-SPEED <number>",
"  Specify the maximum interface speed for the modem.",
" ",
"SET MODEM NAME <text>",
"  Descriptive name for a USER-DEFINED modem.",
" ",
"SET MODEM SPEAKER {ON, OFF}",
"  Turns the modem's speaker on or off during dialing.",
" ",
"SET MODEM SPEED-MATCHING {ON, OFF}",
"  ON means that C-Kermit changes its serial interface speed to agree with",
"  the speed reported by the modem's CONNECT message, if any.  OFF means",
"  Kermit should not change its interface speed.",
" ",
"SET MODEM VOLUME {LOW, MEDIUM, HIGH}",
"  Selects the desired modem speaker volume for when the speaker is ON.",
" ",
"SET MODEM COMMAND commands are used to override built-in modem commands for",
"each modem type, or to fill in commands for the USER-DEFINED modem type.",
"Omitting the optional [ text ] restores the built-in modem-specific command,",
"if any:",
" ",
"SET MODEM COMMAND AUTOANSWER {ON, OFF} [ text ]",
"  Modem commands to turn autoanswer on and off.",
" ",
"SET MODEM COMMAND COMPRESSION {ON, OFF} [ text ]",
"  Modem commands to turn data compression on and off.",
" ",
"SET MODEM COMMAND ERROR-CORRECTION {ON, OFF} [ text ]",
"  Modem commands to turn error correction on and off.",
" ",
"SET MODEM COMMAND HANGUP [ text ]",
"  Command that tells the modem to hang up the connection.",
" ",
"SET MODEM COMMAND IGNORE-DIALTONE [ text ]",
"  Command that tells the modem not to wait for dialtone before dialing.",
" ",
"SET MODEM COMMAND INIT-STRING [ text ]",
"  The 'text' is a replacement for C-Kermit's built-in initialization command",
"  for the modem.",
" ",
"SET MODEM COMMAND PREDIAL-INIT [ text ]",
"  A second INIT-STRING that is to be sent to the modem just prior to \
dialing.",
" ",
"SET MODEM COMMAND HARDWARE-FLOW [ text ]",
"  Modem command to enable hardware flow control (RTS/CTS) in the modem.",
" ",
"SET MODEM COMMAND SOFTWARE-FLOW [ text ]",
"  Modem command to enable local software flow control (Xon/Xoff) in modem.",
" ",
"SET MODEM COMMAND SPEAKER { ON, OFF } [ text ]",
"  Modem command to turn the modem's speaker on or off.",
" ",
"SET MODEM COMMAND NO-FLOW-CONTROL [ text ]",
"  Modem command to disable local flow control in the modem.",
" ",
"SET MODEM COMMAND PULSE [ text ]",
"  Modem command to select pulse dialing.",
" ",
"SET MODEM COMMAND TONE [ text ]",
"  Modem command to select tone dialing.",
" ",
"SET MODEM COMMAND VOLUME { LOW, MEDIUM, HIGH } [ text ]",
"  Modem command to set the modem's speaker volume.",
""};

static char *hmxydial[] = {
"The SET DIAL command establishes or changes all parameters related to",
"dialing the telephone.  Also see HELP DIAL and HELP SET MODEM.  Use SHOW",
"DIAL to display all of the SET DIAL values.",
" ",
"SET DIAL COUNTRY-CODE <number>",
"  Tells Kermit the telephonic country-code of the country you are dialing",
"  from, so it can tell whether a portable-format phone number from your",
"  dialing directory will result in a national or an international call.",
"  Examples: 1 for USA, Canada, Puerto Rico, etc; 7 for Russia, 39 for Italy,",
"  351 for Portugal, 47 for Norway, 44 for the UK, 972 for Israel, 81 for",
"  Japan, ...",
" ",
"  If you have not already set your DIAL INTL-PREFIX and LD-PREFIX, then this",
"  command sets default values for them: 011 and 1, respectively, for country",
"  code 1; 00 and 0, respectively, for all other country codes.  If these are",
"  not your true international and long-distance dialing prefixes, then you",
"  should follow this command by DIAL INTL-PREFIX and LD-PREFIX to let Kermit",
"  know what they really are.",
" ",
"SET DIAL AREA-CODE [ <number> ]",
"  Tells Kermit the area or city code that you are dialing from, so it can",
"  tell whether a portable-format phone number from the dialing directory is",
"  local or long distance.  Be careful not to include your long-distance",
"  dialing prefix as part of your area code; for example, the area code for",
"  central London is 171, not 0171.",
" ",
"SET DIAL CONFIRMATION {ON, OFF}",
"  Kermit does various transformations on a telephone number retrieved from",
"  the dialing directory prior to dialing (use LOOKUP <name> to see them).",
"  In case the result might be wrong, you can use SET DIAL CONFIRM ON to have",
"  Kermit ask you if it is OK to dial the number, and if not, to let you type",
"  in a replacement.",
" ",
"SET DIAL CONNECT { AUTO, ON, OFF }",
"  Whether to CONNECT (enter terminal mode) automatically after successfully",
"  dialing.  ON means to do this; OFF means not to.  AUTO (the default) means",
"  do it if the DIAL command was given interactively, but don't do it if the",
"  DIAL command was issued from a macro or command file.  If you specify ON",
"  or AUTO, you may follow this by one of the keywords VERBOSE or QUIET, to",
"  indicate whether the verbose 4-line 'Connecting...' message is to be",
"  displayed if DIAL succeeds and Kermit goes into CONNECT mode.",
" ",
"SET DIAL CONVERT-DIRECTORY {ASK, ON, OFF}",
"  The format of Kermit's dialing directory changed in version 5A(192).  This",
"  command tells Kermit what to do when it encounters an old-style directory:",
"  ASK you whether to convert it, or convert it automatically (ON), or leave",
"  it alone (OFF).  Old-style directories can still be used without",
"  conversion, but the parity and speed fields are ignored.",
" ",
"SET DIAL DIRECTORY [ filename [ filename [ filename [ ... ] ] ] ]",
"  The name(s) of your dialing directory file(s).  If you do not supply any",
"  filenames, the  dialing directory feature is disabled and all numbers are",
"  dialed literally as given in the DIAL command.  If you supply more than",
"  one directory, all of them are searched.",
" ",
"SET DIAL SORT {ON, OFF}",
"  When multiple entries are obtained from your dialing directory, they are",
"  sorted in \"cheapest-first\" order.  If this does not produce the desired",
"  effect, SET DIAL SORT OFF to disable sorting, and the numbers will be",
"  dialed in the order in which they were found.",
" ",
"SET DIAL DISPLAY {ON, OFF}",
"  Whether to display dialing progress on the screen; default is OFF.",
" ",
"SET DIAL HANGUP {ON, OFF}",
"  Whether to hang up the phone prior to dialing; default is ON.",
" ",
"SET DIAL IGNORE-DIALTONE {ON, OFF}",
"  Whether to ignore dialtone when dialing; default is OFF.",
" ",
#ifndef NOSPL
"SET DIAL MACRO [ name ]",
"  Specify the name of a macro to execute on every phone number dialed, just",
"  prior to dialing it, in order to perform any last-minute alterations.",
" ",
#endif /* NOSPL */
"SET DIAL METHOD {AUTO, DEFAULT, TONE, PULSE}",
"  Whether to use the modem's DEFAULT dialing method, or to force TONE or",
"  PULSE dialing.  AUTO (the default) means to choose tone or pulse dialing",
"  based on the country code.  (Also see SET DIAL TONE-COUNTRIES and SET DIAL",
"  PULSE-COUNTRIES.)",
" ",
"SET DIAL PACING number",
"  How many milliseconds to pause between sending each character to the modem",
"  dialer.  The default is -1, meaning to use the number from the built-in",
" modem database.",
"  ",
"SET DIAL PULSE-COUNTRIES [ cc [ cc [ ... ] ] ]",
"  Sets the list of countries in which pulse dialing is required.  Each cc",
"  is a country code.",
" ",
"SET DIAL TEST { ON, OFF }",
"  OFF for normal dialing.  Set to ON to test dialing procedures without",
"  actually dialing.",
" ",
"SET DIAL TONE-COUNTRIES [ cc [ cc [ ... ] ] ]",
"  Sets the list of countries in which tone dialing is available.  Each cc",
"  is a country code.",
" ",
"SET DIAL TIMEOUT number",
"  How many seconds to wait for a dialed call to complete.  Use this command",
"  to override the DIAL command's automatic timeout calculation.  A value",
"  of 0 turns off this feature and returns to Kermit's automatic dial",
"  timeout calculation.",
" ",
"SET DIAL RESTRICT { INTERNATIONAL, LOCAL, LONG-DISTANCE, NONE }",
"  Prevents placing calls of the type indicated, or greater.  For example",
"  SET DIAL RESTRICT LONG prevents placing of long-distance and international",
"  calls.  If this command is not given, there are no restrictions.  Useful",
"  when dialing a list of numbers fetched from a dialing directory.",
" ",
"SET DIAL RETRIES <number>",
"  How many times to redial each number if the dialing result is busy or no",
"  no answer, until the call is succesfully answered.  The default is 0",
"  because automatic redialing is illegal in some countries.",
" ",
"SET DIAL INTERVAL <number>",
"  How many seconds to pause between automatic redial attempts; default 10.",
" ",
"The following commands apply to all phone numbers, whether given literally",
"or found in the dialing directory:",
" ",
"SET DIAL PREFIX [ text ]",
"  Establish a prefix to be applied to all phone numbers that are dialed,",
"  for example to disable call waiting.",
" ",
"SET DIAL SUFFIX [ text ]",
"  Establish a suffix to be added after all phone numbers that are dialed.",
" ",
"The following commands apply only to portable-format numbers obtained from",
"the dialing directory; i.e. numbers that start with a \"+\" sign and",
"country code, followed by area code in parentheses, followed by the phone",
"number.",
" ",
"SET DIAL LC-AREA-CODES [ <list> ]",
"  Species a list of area codes to which dialing is local, i.e. does not",
"  require the LD-PREFIX.  Up to 32 area codes may be listed, separated by",
"  spaces.  Any area codes in this list will be included in the final dial",
"  string so do not include your own area code if it should not be dialed.",
" ",
"SET DIAL LC-PREFIX [ <text> ]",
"  Specifies a prefix to be applied to local calls made from portable dialing",
"  directory entries.  Normally no prefix is used for local calls.",
" ",
"SET DIAL LC-SUFFIX [ <text> ]",
"  Specifies a suffix to be applied to local calls made from portable dialing",
"  directory entries.  Normally no suffix is used for local calls.",
" ",
"SET DIAL LD-PREFIX [ <text> ]",
"  Your long-distance dialing prefix, to be used with portable dialing",
"  directory entries that result in long-distance calls.",
" ",
"SET DIAL LD-SUFFIX [ <text> ]",
"  Long-distance dialing suffix, if any, to be used with portable dialing",
"  directory entries that result in long-distance calls.  This would normally",
"  be used for appending a calling-card number to the phone number.",
" ",
"SET DIAL FORCE-LONG-DISTANCE { ON, OFF }",
"  Whether to force long-distance dialing for calls that normally would be",
"  local.  For use (e.g.) in France.",
" ",
"SET DIAL TOLL-FREE-AREA-CODE [ <number> [ <number> [ ... ] ] ]",
"  Tells Kermit the toll-free area code(s) in your country.",
" ",
"SET DIAL TOLL-FREE-PREFIX [ <text> ]",
"  You toll-free dialing prefix, in case it is different from your long-",
"  distance dialing prefix.",
" ",
"SET DIAL INTL-PREFIX <text>",
"  Your international dialing prefix, to be used with portable dialing",
"  directory entries that result in international calls.",
" ",
"SET DIAL INTL-SUFFIX <text>",
"  International dialing suffix, if any, to be used with portable dialing",
"  directory entries that result in international calls.",
" ",
"SET DIAL PBX-OUTSIDE-PREFIX <text>",
"  Use this to tell Kermit how to get an outside line when dialing from a",
"  Private Branch Exchange (PBX).",
" ",
"SET DIAL PBX-EXCHANGE <text> [ <text> [ ... ] ]",
"  If PBX-OUTSIDE-PREFIX is set, then you can use this command to tell Kermit",
"  the leading digits of one or more local phone numbers that identify it as",
"  being on your PBX, so it can make an internal call by deleting those digits"
,
"  from the phone number.",
" ",
"SET DIAL PBX-INTERNAL-PREFIX <text>",
"  If PBX-EXCHANGE is set, and Kermit determines from it that a call is",
"  internal, then this prefix, if any, is added to the number prior to",
"  \
dialing.  Use this if internal calls from your PBX require a special prefix.",
"" };
#endif /* NODIAL */

static char *hmxyflo[] = { "Syntax: SET FLOW [ switch ] value",
" ",
#ifndef NOLOCAL
"  Selects the type of flow control to use during file transfer, terminal",
"  connection, and script execution.",
#else
"  Selects the type of flow control to use during file transfer.",
#endif /* NOLOCAL */
" ",
"  Switches let you associate a particular kind of flow control with each",
"  kind of connection: /REMOTE, /MODEM, /DIRECT-SERIAL, /TCPIP, etc; type",
"  \"set flow ?\" for a list of available switches.  Then whenever you make",
"  a connection, the associated flow-control is chosen automatically.",
"  The flow-control values are NONE, KEEP, XON/XOFF, and possibly RTS/CTS",
"  and some others; again, type \"set flow ?\" for a list.  KEEP tells Kermit",
"  not to try to change the current flow-control method for the connection.",
" ",
"  If you omit the switch and simply supply a value, this value becomes the",
"  current flow control type, overriding any default value that might have",
"  been chosen in your most recent SET LINE, SET PORT, or SET HOST, or other",
"  connection-establishment command.",
" ",
"  Type SHOW FLOW-CONTROL to see the current defaults for each connection type"
,
"  as well as the current connection type and flow-control setting.  SHOW",
"  COMMUNICATIONS also shows the current flow-control setting.",
""};

static char *hmxyf[] = {
"Syntax: SET FILE parameter value",
" ",
"Sets file-related parameters.  Use SHOW FILE to view them.  Also see SET",
"TRANSFER and SET PROTOCOL.",
" ",
#ifdef VMS
"SET FILE TYPE { TEXT, BINARY, IMAGE, LABELED }",
#else
#ifdef STRATUS
"SET FILE TYPE { TEXT, BINARY, LABELED }",
#else
#ifdef MAC
"SET FILE TYPE { TEXT, BINARY, MACBINARY }",
#else
"SET FILE TYPE { TEXT, BINARY }",
#endif /* STRATUS */
#endif /* MAC */
#endif /* VMS */
"  How file contents are to be treated during file transfer in the absence",
"  of any other indication.  TYPE can be TEXT for conversion of record format",
"  and character set, which is usually needed when transferring text files",
"  between unlike platforms (such as UNIX and Windows), or BINARY for no",
"  conversion if TRANSFER MODE is MANUAL, which is not the default.  Use",
"  BINARY with TRANSFER MODE MANUAL for executable programs or binary data or",
"  whenever you wish to duplicate the original contents of the file, byte for"
,
"  byte.  In most modern Kermit programs, the file sender informs the receiver"
,
"  of the file type automatically.  However, when sending files from C-Kermit",
"  to an ancient or non-Columbia Kermit implementation, you might need to set",
"  the corresponding file type at the receiver as well.",
" ",
#ifdef VMS
"  FILE TYPE settings of TEXT and BINARY have no effect when sending files,",
"  since VMS C-Kermit determines each file's type automatically from its",
"  record format: binary for fixed, text for others.  For incoming files,",
"  these settings are effective only in the absence of a file-type indication",
"  from the sender.",
" ",
"  You may include an optional record-format after the word BINARY.  This may",
"  be FIXED (the default) or UNDEFINED.  UNDEFINED is used when you need to",
"  receive binary files in binary mode and have them stored with UNDEFINED",
"  record format, which is required by certain VMS applications.",
" ",
"  Two additional VMS file types are also supported: IMAGE and LABELED.",
"  IMAGE means raw block i/o, no interference from RMS, applies to file",
"  transmission only, and overrides the normal automatica file type",
"  determination.   LABELED means to send or interpret RMS attributes",
"  with the file.",
" ",
#else
"  When TRANSFER MODE is AUTOMATIC (as it is by default), various automatic",
"  methods (depending on the platform) are used to determine whether a file",
"  is transferred in text or binary mode; these methods (which might include",
"  filename pattern matching (see SET FILE PATTERNS below), client/server",
"  \"kindred-spirit\" recognition, or source file record format) supersede",
"  the FILE TYPE setting but can, themselves, be superseded by including a",
"  /BINARY or /TEXT switch in the SEND, GET, or RECEIVE command.",
" ",
"  When TRANSFER MODE is MANUAL, the FILE TYPE setting takes precedence.",
#endif /* VMS */
" ",

#ifndef NOXFER
#ifdef PATTERNS
"SET FILE PATTERNS { ON, OFF, AUTO }",
"  ON means to use filename pattern lists to determine whether to send a file",
"  in text or binary mode.  OFF means to send all files in the prevailing",
"  mode.  AUTO (the default) is like ON if the other Kermit accepts Attribute",
"  packets and like OFF otherwise.",
" ",
"SET FILE BINARY-PATTERNS [ <pattern> [ <pattern> ... ] ]",
"  Zero or more filename patterns which, if matched, cause a file to be sent",
"  in binary mode when FILE PATTERNS are ON.  HELP WILDCARDS for a description"
,
"  of pattern syntax.  SHOW PATTERNS to see the current file pattern lists.",
" ",
"SET FILE TEXT-PATTERNS [ <pattern> [ <pattern> ... ] ]",
"  Zero or more filename patterns which, if matched, cause a file to be sent",
"  in text mode when FILE PATTERNS is ON; if a file does not match a text or",
"  binary pattern, the prevailing SET FILE TYPE is used.",
" ",
#endif /* PATTERNS */

"SET FILE BYTESIZE { 7, 8 }",
"  Normally 8.  If 7, truncate the 8th bit of all file bytes.",
" ",
#ifndef NOCSETS
"SET FILE CHARACTER-SET name",
"  Tells the encoding of the local file, ASCII by default.",
"  The names ITALIAN, PORTUGUESE, NORWEGIAN, etc, refer to 7-bit ISO-646",
"  national character sets.  LATIN1 is the 8-bit ISO 8859-1 Latin Alphabet 1",
"  for Western European languages.",
"  NEXT is the 8-bit character set of the NeXT workstation.",
"  The CPnnn sets are for PCs.  MACINTOSH-LATIN is for the Macintosh.",
#ifndef NOLATIN2
"  LATIN2 is ISO 8859-2 for Eastern European languages that are written with",
"  Roman letters.  Mazovia is a PC code page used in Poland.",
#endif /* NOLATIN2 */
#ifdef CYRILLIC
"  KOI-CYRILLIC, CYRILLIC-ISO, and CP866 are 8-bit Cyrillic character sets.",
"  SHORT-KOI is a 7-bit ASCII coding for Cyrillic.  BULGARIA-PC is a PC code",
"  page used in Bulgaria",
#endif /* CYRILLIC */
#ifdef HEBREW
"  HEBREW-ISO is ISO 8859-8 Latin/Hebrew.  CP862 is the Hebrew PC code page.",
"  HEBREW-7 is like ASCII with the lowercase letters replaced by Hebrew.",
#endif /* HEBREW */
#ifdef GREEK
"  GREEK-ISO is ISO 8859-7 Latin/Greek.  CP869 is the Greek PC code page.",
"  ELOT-927 is like ASCII with the lowercase letters replaced by Greek.",
#endif /* GREEK */
#ifdef KANJI
"  JAPANESE-EUC, JIS7-KANJI, DEC-KANJI, and SHIFT-JIS-KANJI are Japanese",
"  Kanji character sets.",
#endif /* KANJI */
#ifdef UNICODE
"  UCS-2 is the 2-byte form of the Universal Character Set.",
"  UTF-8 is the serialized form of the Universal Character Set.",
#endif /* UNICODE */
"  Type SET FILE CHAR ? for a complete list of file character sets.",
" ",
#endif /* NOCSETS */

"SET FILE COLLISION option",
"  Tells what to do when a file arrives that has the same name as",
"  an existing file.  The options are:",
"   BACKUP (default) - Rename the old file to a new, unique name and store",
"     the incoming file under the name it was sent with.",
"   OVERWRITE - Overwrite (replace) the existing file.",
"   APPEND - Append the incoming file to the end of the existing file.",
"   DISCARD - Refuse and/or discard the incoming file.",
"   RENAME - Give the incoming file a unique name.",
"   UPDATE - Accept the incoming file only if newer than the existing file.",
" ",

"SET FILE DESTINATION { DISK, PRINTER, SCREEN, NOWHERE }",
"  DISK (default): Store incoming files on disk.",
"  PRINTER:        Send incoming files to SET PRINTER device.",
"  SCREEN:         Display incoming files on screen (local mode only).",
"  NOWHERE:        Do not put incoming files anywhere (use for calibration).",
" ",
"SET FILE DISPLAY option",
"  Selects the format of the file transfer display for local-mode file",
"  transfer.  The choices are:",
" ",
"  BRIEF      A line per file, showing size, mode, status, and throughput.",
"  SERIAL     One dot is printed for every K bytes transferred.",
"  CRT        Numbers are continuously updated on a single screen line.",
"             This format can be used on any video display terminal.",
#ifdef CK_CURSES
"  FULLSCREEN A fully formatted 24x80 screen showing lots of information.",
"             This requires a terminal or terminal emulator.",
#endif /* CK_CURSES */
"  NONE       No file transfer display at all.",
" ",

"SET FILE DOWNLOAD-DIRECTORY [ <directory-name> ]",
"  The directory into which all received files should be placed.  By default,",
"  received files go into your current directory.",
" ",
#endif /* NOXFER */

#ifdef CK_CTRLZ
"SET FILE EOF { CTRL-Z, LENGTH }",
"  End-Of-File detection method, normally LENGTH.  Applies only to text-mode",
"  transfers.  When set to CTRL-Z, this makes the file sender treat the first",
"  Ctrl-Z in the input file as the end of file (EOF), and it makes the file",
"  receiver tack a Ctrl-Z onto the end of the output file if it does not",
"  already end with Ctrl-Z.",
" ",
#endif /* CK_CTRLZ */

"SET FILE END-OF-LINE { CR, CRLF, LF }",
"  Use this command to specify nonstandard line terminators for text files.",
" ",

#ifndef NOXFER
"SET FILE INCOMPLETE { AUTO, KEEP, DISCARD }",
"  What to do with an incompletely received file: KEEP (default), or DISCARD.",
"  AUTO (the default) means DISCARD if transfer is in text mode, KEEP if it",
"  is in binary mode.",
" ",

#ifdef VMS
"SET FILE LABEL { ACL, BACKUP-DATE, NAME, OWNER, PATH } { ON, OFF }",
"  Tells which items to include (ON) or exclude (OFF) in labeled file",
"  transfers",
" ",
#else
#ifdef OS2
"SET FILE LABEL { ARCHIVE, READ-ONLY, HIDDEN, SYSTEM, EXTENDED } { ON, OFF }",
"  Tells which items to include (ON) or exclude (OFF) in labeled file",
"  transfers.",
" ",
#endif /* OS2 */
#endif /* VMS */

"SET FILE NAMES { CONVERTED, LITERAL }",
"  File names are normally CONVERTED to \"common form\" during transmission",
"  (e.g. lowercase to uppercase, extra periods changed to underscore, etc).",
"  LITERAL means use filenames literally (useful between like systems).  Also",
"  see SET SEND PATHNAMES and SET RECEIVE PATHNAMES.",
" ",

#ifdef UNIX
"SET FILE OUTPUT { { BUFFERED, UNBUFFERED } [ size ], BLOCKING, NONBLOCKING }",
"  Lets you control the disk output buffer for incoming files.  Buffered",
"  blocking writes are normal.  Nonblocking writes might be faster on some",
"  systems but might also be risky, depending on the underlying file service.",
"  Unbuffered writes might be useful in critical applications to ensure that",
"  cached disk writes are not lost in a crash, but will probably also be",
"  slower.  The optional size parameter after BUFFERED or UNBUFFERED lets you",
"  change the disk output buffer size; this might make a difference in",
"  performance.",
" ",
#endif /* UNIX */

#ifdef VMS
"SET FILE RECORD-LENGTH number",
"  Sets the record length for received files of type BINARY.  Use this to",
"  receive VMS BACKUP savesets or other fixed-format files that do not use",
"  the default record length of 512.",
" ",
#endif /* VMS */

#ifdef UNICODE
"SET FILE UCS BOM { ON, OFF }",
"  Whether to write a Byte Order Mark when creating a UCS-2 file.",
" ",
"SET FILE UCS BYTE-ORDER { BIG-ENDIAN, LITTLE-ENDIAN }",
"  Byte order to use when creating UCS-2 files, and to use when reading UCS-2",
"  files that do not start with a Byte Order Mark.",
" ",
#endif /* UNICODE */

"SET FILE WARNING { ON, OFF }",
"  SET FILE WARNING is superseded by the newer command, SET FILE",
"  COLLISION.  SET FILE WARNING ON is equivalent to SET FILE COLLISION RENAME",
"  and SET FILE WARNING OFF is equivalent to SET FILE COLLISION OVERWRITE.",
#endif /* NOXFER */
"" };

static char *hmxyhsh[] = {
"Syntax: SET HANDSHAKE { NONE, XON, LF, BELL, ESC, CODE number }",
"  Character to use for half duplex line turnaround handshake during file",
"  transfer.  C-Kermit waits for this character from the other computer",
"  before sending its next packet.  Default is NONE; you can give one of the",
"  other names like BELL or ESC, or use SET HANDSHAKE CODE to specify the",
"  numeric code value of the handshake character.  Type SET HANDSH ? for a",
"  complete list of possibilities.",
"" };

#ifndef NOSERVER
static char *hsetsrv[] = {
"SET SERVER CD-MESSAGE {ON,OFF}",
"  Tells whether the server, after successfully executing a REMOTE CD",
"  command, should send the contents of the new directory's READ.ME",
"  (or similar) file to your screen.",
" ",
"SET SERVER CD-MESSAGE FILE name",
"  Tells the name of the file to be displayed as a CD-MESSAGE, such as",
"  READ.ME (SHOW SERVER tells the current CD-MESSAGE FILE name).",
"  To specify more than one filename to look for, use {{name1}{name2}..}.",
"  Synonym: SET CD MESSAGE FILE <list>.",
" ",
"SET SERVER DISPLAY {ON,OFF}",
"  Tells whether local-mode C-Kermit during server operation should put a",
"  file transfer display on the screen.  Default is OFF.",
" ",
"SET SERVER GET-PATH [ directory [ directory [ ... ] ] ]",
"  Tells the C-Kermit server where to look for files whose names it receives",
"  from client GET commands when the names are not fully specified pathnames.",
"  Default is no GET-PATH, so C-Kermit looks only in its current directory.",
" ",
"SET SERVER IDLE-TIMEOUT seconds",
"  Idle time limit while in server mode, 0 for no limit.",
#ifndef OS2
"  NOTE: SERVER IDLE-TIMEOUT and SERVER TIMEOUT are mutually exclusive.",
#endif /* OS2 */
" ",
"SET SERVER KEEPALIVE {ON,OFF}",
"  Tells whether C-Kermit should send \"keepalive\" packets while executing",
"  REMOTE HOST commands, which is useful in case the command takes a long",
"  time to produce any output and therefore might cause the operation to time",
"  out.  ON by default; turn it OFF if it causes trouble with the client or",
"  slows down the server too much.",
" ",
"SET SERVER LOGIN [ username [ password [ account ] ] ]",
"  Sets up a username and optional password which must be supplied before",
"  the server will respond to any commands other than REMOTE LOGIN.  The",
"  account is ignored.  If you enter SET SERVER LOGIN by itself, then login",
"  is no longer required.  Only one SET SERVER LOGIN command can be in effect",
"  at a time; C-Kermit does not support multiple user/password pairs.",
" ",
"SET SERVER TIMEOUT n",
"  Server command wait timeout interval, how often the C-Kermit server issues",
"  a NAK while waiting for a command packet.  Specify 0 for no NAKs at all.",
"  Default is 0.",
""
};
#endif /* NOSERVER */

static char *hmhrmt[] = {
"The REMOTE command sends file management instructions or other commands",
"to a Kermit server.  There should already be a Kermit running in server",
"mode on the other end of the currently selected line.  Type REMOTE ? to",
"see a list of available remote commands.  Type HELP REMOTE x to get",
"further information about a particular remote command 'x'.",
" ",
"All REMOTE commands except LOGIN and LOGOUT have R-command shortcuts;",
"for example, RDIR for REMOTE DIR, RCD for REMOTE CD, etc.",
"" };

#ifndef NOSPL
static char *ifhlp[] = { "Syntax: IF [NOT] condition commandlist",
" ",
"If the condition is (is not) true, do the commandlist.  The commandlist",
"can be a single command, or a list of commands separated by commas and",
"enclosed in braces.  The condition can be a single condition or a group of",
"conditions separated by AND (&&) or OR (||) and enclosed in parentheses.",
"Examples:",
" ",
"  IF EXIST oofa.txt <command>",
"  IF ( EXIST oofa.txt || = \\v(nday) 3 ) <command>",
"  IF ( EXIST oofa.txt || = \\v(nday) 3 ) { <command>, <command>, ... }",
" ",
"The conditions are:",
" ",
"  SUCCESS     - The previous command succeeded",
"  OK          - Synonym for SUCCESS",
"  FAILURE     - The previous command failed",
"  ERROR       - Synonym for FAILURE",
"  FLAG        - Succeeds if SET FLAG ON, fails if SET FLAG OFF",
"  BACKGROUND  - C-Kermit is running in the background",
#ifdef CK_IFRO
"  FOREGROUND  - C-Kermit is running in the foreground",
"  REMOTE-ONLY - C-Kermit was started with the -R command-line option",
#else
"  FOREGROUND  - C-Kermit is running in the foreground",
#endif /* CK_IFRO */
"  ALARM       - SET ALARM time has passed",
"  ASKTIMEOUT  - The most recent ASK, ASKQ, GETC, or GETOK timed out",
"  EMULATION   - Succeeds if executed while in CONNECT mode",
#ifdef OS2
"  TAPI        - Current connection is via a Microsoft TAPI device",
#endif /* OS2 */
" ",
"  AVAILABLE CRYPTO                  - Encryption is available",
"  AVAILABLE KERBEROS4               - Kerberos 4 authentication is available",
"  AVAILABLE KERBEROS5               - Kerberos 5 authentication is available",
"  AVAILABLE NTLM                    - NTLM authentication is available",
"  AVAILABLE SRP                     - SRP authentication is available",
"  AVAILABLE SSL                     - SSL/TLS authentication is available",
"  MATCH string pattern              - Succeeds if string matches pattern",
#ifdef CKFLOAT
"  FLOAT number                      - Succeeds if floating-point number",
#endif /* CKFLOAT */
"  COMMAND word                      - Succeeds if word is built-in command",
"  DEFINED variablename or macroname - The named variable or macro is defined",
"  NUMERIC variable or constant      - The variable or constant is numeric",
"  EXIST filename                    - The named file exists",
"  ABSOLUTE filename                 - The filename is absolute, not relative",
#ifdef CK_TMPDIR
"  DIRECTORY string                  - The string is the name of a directory",
#endif /* CK_TMPDIR */
"  READABLE filename                 - Succeeds if the file is readable",
"  WRITEABLE filename                - Succeeds if the file is writeable",
#ifdef ZFCDAT
"  NEWER file1 file2                 - The 1st file is newer than the 2nd one",
#endif /* ZFCDAT */
"  OPEN { READ-FILE,SESSION-LOG,...} - The given file or log is open",
#ifndef NOLOCAL
"  OPEN CONNECTION                   - A connection is open",
#endif /* NOLOCAL */
" ",
"  COUNT   - subtract one from COUNT, execute the command if the result is",
"            greater than zero (see SET COUNT)",
" ",
"  EQUAL s1 s2 - s1 and s2 (character strings or variables) are equal",
"  LLT s1 s2   - s1 is lexically (alphabetically) less than s2",
"  LGT s1 s1   - s1 is lexically (alphabetically) greater than s2",
" ",
"  =  n1 n2 - n1 and n2 (numbers or variables containing numbers) are equal",
"  <  n1 n2 - n1 is arithmetically less than n2",
"  <= n1 n2 - n1 is arithmetically less than or equal to n2",
"  >  n1 n2 - n1 is arithmetically greater than n2",
"  >= n1 n2 - n1 is arithmetically greater than or equal to n2",
" ",
"  (number by itself) - fails if the number is 0, succeeds otherwise",
" ",
"  TRUE     - always succeeds",
"  FALSE    - always fails",
" ",
"The IF command may be followed on the next line by an ELSE command. Example:",
" ",
"  IF < \\%x 10 ECHO It's less",
"  ELSE echo It's not less",
" ",
"It can also include an ELSE part on the same line if braces are used:",
" ",
"  IF < \\%x 10 { ECHO It's less } ELSE { ECHO It's not less }",
" ",
"Also see HELP WILDCARD (for IF MATCH pattern syntax).",
"" };

static char *hmxxeval[] = { "Syntax: EVALUATE variable expression",
"  Evaluates the expression and assigns its value to the given variable.",
"  The expression can contain numbers and/or numeric-valued variables or",
"  functions, combined with mathematical operators and parentheses in",
"  traditional notation.  Operators include +-/*(), etc.  Example:",
"  EVALUATE \\%n (1+1) * (\\%a / 3).",
" ",
"  NOTE: Prior to C-Kermit 7.0, the syntax was \"EVALUATE expression\"",
"  (no variable), and the result was printed.  Use SET EVAL { OLD, NEW }",
"  to choose the old or new behavior, which is NEW by default.",
" ",
"Alse see: HELP FUNCTION EVAL.",
"" };
#endif /* NOSPL */

#ifndef NOSPL
static char *ifxhlp[] = { "\
Syntax: XIF condition { commandlist } [ ELSE { commandlist } ]",
"  Obsolete.  Same as IF (see HELP IF).",
"" };

static char *forhlp[] = { "\
Syntax: FOR variablename initial-value final-value increment { commandlist }",
"  FOR loop.  Execute the comma-separated commands in the commandlist the",
"  number of times given by the initial value, final value and increment.",
"  Example:  FOR \\%i 10 1 -1 { pause 1, echo \\%i }", "" };

static char *whihlp[] = { "\
Syntax: WHILE condition { commandlist }",
"  WHILE loop.  Execute the comma-separated commands in the bracketed",
"  commandlist while the condition is true.  Conditions are the same as for",
"  IF commands.",
"" };

static char *swihlp[] = {
"Syntax: SWITCH <variable> { case-list }",
"  Selects from a group of commands based on the value of a variable.",
"  The case-list is a series of lines like these:",
" ",
"    :x, command, command, ..., break",
" ",
"  where \"x\" is a possible value for the variable.  At the end of the",
"  case-list, you can put a \"default\" label to catch when the variable does",
"  not match any of the labels:",
" ",
"    :default, command, command, ...",
" ",
"The case label \"x\" can be a character, a string, a variable, a function",
"invocation, a pattern, or any combination of these.  See HELP WILDCARDS",
"for information about patterns.",
""};

static char *openhlp[] = {
"Syntax:  OPEN mode filename",
"  For use with READ and WRITE commands.  Open the local file in the",
"  specified mode: READ, WRITE, or APPEND.  !READ and !WRITE mean to read",
"  from or write to a system command rather than a file.  Examples:",
" ",
"    OPEN READ oofa.txt",
"    OPEN !READ sort foo.bar",
"" };

static char *hxxaskq[] = {
"Syntax:  ASKQ variablename [ prompt ]",
"Example: ASKQ \\%p { Password:}",
"  Issues the prompt and defines the variable to be whatever you type in.",
"  The characters that you type do not echo on the screen.",
"  Use braces to preserve leading and/or trailing spaces in the prompt.",
"  To include a question mark, precede it by backslash (\\).",
" ",
"Also see SET ASK-TIMER.",
""};

static char *hxxask[] = {
"Syntax:  ASK variablename [ prompt ]",
"Example: ASK \\%n { What is your name\\? }",
"  Issues the prompt and defines the variable to be whatever you type in.",
"  Use braces to preserve leading and/or trailing spaces in the prompt.",
"  To include a question mark, precede it by backslash (\\).",
" ",
"Also see SET ASK-TIMER.",
""};

static char *hxxgetc[] = {
"Syntax:  GETC variablename [ prompt ]",
"Example: GETC \\%c { Type any character to continue...}",
"  Issues the prompt and sets the variable to the first character you type.",
"  Use braces to preserve leading and/or trailing spaces in the prompt.",
" ",
"Also see SET ASK-TIMER.",
""};

static char *hmxytimer[] = {
"Syntax: SET ASK-TIMER number",
"  For use with ASK, ASKQ, GETOK, and GETC.  If ASK-TIMER is set to a number",
"  greater than 0, these commands will time out after the given number of",
"  seconds with no response.  This command is \"sticky\", so to revert to",
" \
untimed ASKs after a timed one, use SET ASK-TIMER 0.  Also see IF ASKTIMEOUT.",
""};

static char *hxxdef[] = {
"Syntax: DEFINE name [ definition ]",
"  Defines a macro or variable.  Its value is the definition, taken",
"  literally.  No expansion or evaluation of the definition is done.  Thus",
"  if the definition includes any variable or function references, their",
"  names are included, rather than their values (compare with ASSIGN).  If",
"  the definition is omitted, then the named variable or macro is undefined.",
" ",
"A typical macro definition looks like this:",
" ",
"  DEFINE name command, command, command, ...",
" ",
"for example:",
" ",
"  DEFINE vax set parity even, set duplex full, set flow xon/xoff",
" ",
"which defines a Kermit command macro called 'vax'.  The definition is a",
"comma-separated list of Kermit commands.  Use the DO command to execute",
"the macro, or just type its name, followed optionally by arguments.",
" ",
"The definition of a variable can be anything at all, for example:",
" ",
"  DEFINE \\%a Monday",
"  DEFINE \\%b 3",
" ",
"These variables can be used almost anywhere, for example:",
" ",
"  ECHO Today is \\%a",
"  SET BLOCK-CHECK \\%b",
"" };

static char *hxxass[] = {
"Syntax:  ASSIGN variablename string.",
"Example: ASSIGN \\%a My name is \\%b.",
"  Assigns the current value of the string to the variable (or macro).",
"  The definition string is fully evaluated before it is assigned, so that",
"  the values of any variables are contained are used, rather than their",
"  names.  Compare with DEFINE.  To illustrate the difference, try this:",
" ",
"    DEFINE \\%a hello",
"    DEFINE \\%x \\%a",
"    ASSIGN \\%y \\%a",
"    DEFINE \\%a goodbye",
"    ECHO \\%x \\%y",
" ",
"  This prints 'goodbye hello'.", "" };

static char *hxxdec[] = {
"Syntax: DECREMENT variablename [ number ]",
"  Decrement (subtract one from) the value of a variable if the current value",
"  is numeric.  If the number argument is given, subtract that number",
"  instead.",
" ",
"Examples: DECR \\%a, DECR \\%a 7, DECR \\%a \\%n", "" };

static char *hxxinc[] = {
"Syntax: INCREMENT variablename [ number ]",
"  Increment (add one to) the value of a variable if the current value is",
"  numeric.  If the number argument is given, add that number instead.",
" ",
"Examples: INCR \\%a, INCR \\%a 7, INCR \\%a \\%n", "" };
#endif /* NOSPL */

#ifdef ANYX25
#ifndef IBMX25
static char *hxxpad[] = {
"Syntax: PAD command",
"X.25 PAD commands:",
" ",
"    PAD CLEAR     - Clear the virtual call",
"    PAD STATUS    - Return the status of virtual call",
"    PAD RESET     - Send a reset packet",
"    PAD INTERRUPT - Send an interrupt packet",
""};
#endif /* IBMX25 */

static char *hxyx25[] = {
"Syntax: SET X.25 option { ON [ data ], OFF }",
" ",
"X.25 call options:",
"  CLOSED-USER-GROUP { ON index, OFF }",
"    Enable or disable closed user group call, where index is the group",
"    index, 0 to 99.",
"  REVERSE-CHARGE { ON, OFF }",
"    Tell whether you want to reverse the charges for the call.",
"  CALL-USER-DATA { ON string, OFF }",
"    Specify call user-data for the X.25 call.",
""};
#endif /* ANYX25 */

static char *hxyprtr[] = {
#ifdef PRINTSWI
"Syntax: SET PRINTER [ switches ] [ name ]",
" ",
"  Specifies the printer to be used for transparent-print, autoprint, and",
"  screen-dump material during terminal emulation, as well as for the PRINT",
"  command, plus various options governing print behavior.",
" ",
"Switches for specifying the printer by type:",
" ",
"/NONE",
"  Include this switch to specify that all printer actions should simply be",
"  skipped.  Use this, for example, if you have no printer.",
" ",
"/DOS-DEVICE[:name]",
"  Include this to declare a DOS printer and to specify its name, such as",
"  PRN, LPT1, etc.",
" ",
#ifdef NT
"/WINDOWS-QUEUE[:queue-name]",
"  Include this to declare a Windows printer and specify its queue name.",
"  Type question mark (?) after the colon (:) to see a list of known queue",
"  names.",
" ",
#endif /* NT */
"/FILE[:name]",
"  Specifies that all printer material is to be appended to the named file,",
"  rather than being sent to a printer.  If the file does not exist, it is",
"  created the first time any material is to be printed.",
" ",
"/PIPE[:name]",
"  Specifies that all printer material is to be sent as standard input to",
"  the program or command whose name is given.  Example:",
" ",
"    SET PRINTER /PIPE:{textps > lpt1}",
" ",
"If you give a printer name without specifying any of these switches, then it",
"is assumed to be a DOS printer device or filename unless the name given",
"(after removing enclosing braces, if any) starts with \"|\", \
in which case it",
"is a pipe.  Examples:",
" ",
"  SET PRINTER LPT1               <-- DOS device",
"  SET PRINTER {| textps > lpt1}  <-- Pipe",
" ",
"The next group of switches tells whether the printer is one-way or",
"bidirectional (two-way):",
" ",
"/OUTPUT-ONLY",
"  Include this to declare the printer capable only of receiving material to",
"  be printed, but not sending anything back.  This is the normal kind of",
"  printer, Kermit's default kind, and the opposite of /BIDIRECTIONAL.",
" ",
"/BIDIRECTIONAL",
"  Include this to declare the printer bidirectional.  This is the opposite ",
"  of /OUTPUT-ONLY.  You can also use this option with serial printers, even",
"  if they aren't bidirectional, in case you need to specify speed, flow",
"  control, or parity.",
" ",
"The next group applies only to bidirectional and/or serial printers:",
" ",
"/FLOW-CONTROL:{NONE,XON/XOFF,RTS/CTS,KEEP}",
"  Flow control to use with a serial bidirectional printer, default KEEP;",
#ifdef NT
"  i.e. use whatever the Windows driver for the port normally uses.",
#else
"  i.e. use whatever the OS/2 driver for the port normally uses.",
#endif /* NT */
" ",
"/PARITY:{NONE,EVEN,ODD,SPACE,MARK}",
"  Parity to use with a serial printer, default NONE; i.e. use 8 data bits",
"  and no parity.  If you omit the colon and the keyword, NONE is selected.",
" ",
"/SPEED:number",
"  Interface speed, in bits per second, to use with a serial printer, such as",
"  2400, 9600, 19200, etc.  Type SET PRINTER /SPEED:? for a list of possible",
"  speeds.",
" ",
"The next group deals with print jobs -- how to identify them, how to start",
"them, how to terminate them:",
" ",
"/TIMEOUT[:number]",
"  Used with host-directed transparent or auto printing, this is the number",
"  of seconds to wait after the host closes the printer before terminating",
"  the print job if the printer is not opened again during the specified",
"  amount of time.",
" ",
"/JOB-HEADER-FILE[:filename]",
"  The name of a file to be sent to the printer at the beginning of each",
"  print job, as a burst page, or to configure the printer.  Normally no file",
"  is is sent.",
" ",
"/END-OF-JOB-STRING[:string]",
"  String of characters to be sent to the printer at the end of the print",
"  job, usually used to force the last or only page out of the printer.  When",
"  such a string is needed, it usually consists of a single formfeed: \"set",
"  printer /end-of-job:{\\12}\".  No end-of-job string is sent unless you",
"  specify one with this option.  If the string contains any spaces or",
"  control characters (even in backslash notation, as above), enclose it in",
"  braces.",
" ",
"The final group is for use with printers that print only PostScript:",
" ",
"/POSTSCRIPT or /PS",
"  Indicates that K95 should convert all text to PostScript before sending",
"  it to the printer.  The fixed-pitch Courier-11 font is used.",
" ",
"/WIDTH:number",
"  Specifies the width of the page in characters.  If this switch is not",
"  given, 80 is used.",
" ",
"/HEIGHT:number",
"  Specifies the height of the page in lines.  If this switch is not given",
"  66 is used.",
" ",
"/NOPOSTSCRIPT or /NOPS",
"  Indicates that K95 should not convert all text to PostScript before",
"  sending it to the printer.",
" ",
"SHOW PRINTER displays your current printer settings.",
#else
#ifdef UNIX
"Syntax: SET PRINTER [ { |command, filename } ]",
"  Specifies the command (such as \"|lpr\") or filename to be used by the",
"  PRINT command.  If a filename is given, each PRINT command appends to the",
"  given file.  If the SET PRINTER argument contains spaces, it must be",
"  enclosed in braces, e.g. \"set printer {| lpr -Plaser}\". If the argument",
"  is omitted the default value is restored.  SHOW PRINTER lists the current",
"  printer.  See HELP PRINT for further info.",
#else
"Sorry, SET PRINTER not available yet.",
#endif /* UNIX */
#endif /* PRINTSWI */
""};

#ifdef OS2
#ifdef BPRINT
static char *hxybprtr[] = {
"Syntax: SET BPRINTER [ portname speed [ parity [ flow-control ] ] ]",
"  (Obsolete, replaced by SET PRINTER /BIDIRECTIONAL.)",
""};
#endif /* BPRINT */
#endif /* OS2 */

static char *hxyexit[] = {
"Syntax: SET EXIT ON-DISCONNECT { ON, OFF }",
"  When ON, C-Kermit EXITs automatically when a network connection",
"  is terminated either by the host or by issuing a HANGUP command.",
" ",
"Syntax: SET EXIT STATUS number",
#ifdef NOSPL
"  Set C-Kermit's program return code to the given number.",
#else
"  Set C-Kermit's program return code to the given number, which can be a",
"  constant, variable, function result, or arithmetic expression.",
#endif /* NOSPL */
" ",
"Syntax: SET EXIT WARNING { ON, OFF, ALWAYS }",
"  When EXIT WARNING is ON, issue a warning message and ask for confirmation",
"  before EXITing if a connection to another computer might still be open.",
"  When EXIT WARNING is ALWAYS, confirmation is always requested.  When OFF",
"  it is never requested.  The default is ON.",
"" };

#ifndef NOSPL
static char *hxxpau[] = {
"Syntax:  PAUSE [ { number-of-seconds, hh:mm:ss } ]",
"Example: PAUSE 3  or  PAUSE 14:52:30",
"  Do nothing for the specified number of seconds or until the given time of",
"  day in 24-hour hh:mm:ss notation.  If the time of day is earlier than the",
"  current time, it is assumed to be tomorrow.  If no argument given, one",
"  second is used.  The pause can be interrupted by typing any character on",
"  the keyboard unless SLEEP CANCELLATION is OFF.  If interrupted, PAUSE",
"  fails, otherwise it succeeds.  Synonym: SLEEP.",
"" };

static char *hxxmsl[] = {
"Syntax:  MSLEEP [ number ]",
"Example: MSLEEP 500",
"  Do nothing for the specified number of milliseconds; if no number given,",
"  100 milliseconds.","" };
#endif /* NOSPL */

#ifndef NOPUSH
extern int nopush;
static char *hxxshe[] = {
"Syntax: !, @, RUN, PUSH, or SPAWN, optionally followed by a command.",
"  Gives the command to the local operating system's command processor, and",
"  displays the results on the screen.  If the command is omitted, enters the",
"  system's command line interpreter or shell; exit from it (the command for",
"  this is usually EXIT or QUIT or LOGOUT) to return to Kermit.",
""
};
#endif /* NOPUSH */

#ifndef NOXMIT
static char *hxxxmit[] = {
#ifndef NOCSETS
"Syntax: TRANSMIT { /COMMAND, /TEXT, /BINARY, /TRANSPARENT } filename",
#else
"Syntax: TRANSMIT { /COMMAND, /TEXT, /BINARY } filename",
#endif /* NOCSETS */
"  Sends the contents of a file, without any error checking or correction,",
"  to the computer on the other end of your SET LINE or SET HOST connection",
"  (or if C-Kermit is in remote mode, displays it on the screen).  The",
"  filename is the name of a single file (no wildcards) to be sent or, if",
"  the /COMMAND switch is included, the name of a command whose output is",
"  to be sent.",
" ",
"  The file is sent according to your current FILE TYPE setting (BINARY or",
"  TEXT), which you can override with a /BINARY or /TEXT switch without",
"  changing the global setting.  In text mode, it is sent a line at a time,",
"  with carriage return at the end of each line (as if you were typing it at",
"  your keyboard), and C-Kermit waits for a linefeed to echo before sending",
"  the next line.  In binary mode, it is sent a character at a time, with no",
"  feedback required.",
" ",
#ifndef NOCSETS
"  Character sets are translated according to your current FILE and TERMINAL",
"  CHARACTER-SET settings when TRANSMIT is in text mode.  Include /TRANSPARENT"
,
"  to disable character-set translation in text mode (/TRANSPARENT implies",
"  /TEXT).",
" ",
#endif /* NOCSETS */
"  There can be no guarantee that the other computer will receive the file",
"  correctly and completely.  Before you start the TRANSMIT command, you",
"  must put the other computer in data collection mode, for example by",
"  starting a text editor.  TRANSMIT may be interrupted by Ctrl-C.  Synonym:",
"  XMIT.  See HELP SET TRANSMIT for further information.",
"" };
#endif /* NOXMIT */

#ifndef NOCSETS
static char *hxxxla[] = {
"Syntax: TRANSLATE file1 cs1 cs2 [ file2 ]",
"  Translates file1 from the character set cs1 into the character set cs2",
"  and stores the result in file2.  The character sets can be any of",
"  C-Kermit's file character sets.  If file2 is omitted, the translation",
"  is displayed on the screen.  An appropriate intermediate character-set",
"  is chosen automatically, if necessary.  Synonym: XLATE.  Example:",
" ",
"  TRANSLATE lasagna.lat latin1 italian lasagna.nrc",
"" };
#endif /* NOCSETS */

#ifndef NOSPL
static char *hxxwai[] = {
"Syntax: WAIT { number-of-seconds, hh:mm:ss } [modem-signal(s)]",
" ",
"Examples:",
"  wait 5 cd cts",
"  wait 23:59:59 cd",
" ",
"  Waits up to the given number of seconds or the given time of day for all",
"  the specified modem signals to appear on the serial communication device.",
"  Sets FAILURE if the signals do not appear in the given time or interrupted",
"  from the keyboard during the waiting period.  Also see HELP PAUSE.",
" ",
"Signals:",
"  cd  = Carrier Detect;",
"  dsr = Dataset Ready;",
"  cts = Clear To Send;",
"  ri  = Ring Indicate.",
"" };
#endif /* NOSPL */

static char *hxxwri[] = {
"Syntax: WRITE name text",
"  Writes the given text to the named log or file.  The text text may include",
"  backslash codes, and is not terminated by a newline unless you include the",
"  appropriate code.  The name parameter can be any of the following:",
" ",
"   DEBUG-LOG",
"   ERROR (standard error)",
#ifndef NOSPL
"   FILE (the OPEN WRITE, OPEN !WRITE, or OPEN APPEND file, see HELP OPEN)",
#endif /* NOSPL */
"   PACKET-LOG",
"   SCREEN (compare with ECHO)",
#ifndef NOLOCAL
"   SESSION-LOG",
#endif /* NOLOCAL */
"   TRANSACTION-LOG", "" };

#ifndef NODIAL
static char *hxxlook[] = { "Syntax: LOOKUP name",
"  Looks up the given name in the dialing directory or directories, if any,",
"  specified in the most recent SET DIAL DIRECTORY command.  Each matching",
"  entry is shown, along with any transformations that would be applied to",
"  portable-format entries based on your locale.  HELP DIAL, HELP SET DIAL",
"  for further info.",
""
};

static char *hxxansw[] = { "Syntax:  ANSWER [ <seconds> ]",
#ifdef OS2
"  Waits for a modem call to come in.  Prior SET MODEM TYPE and SET PORT",
#else
"  Waits for a modem call to come in.  Prior SET MODEM TYPE and SET LINE",
#endif /* OS2 */
"  required.  If <seconds> is 0 or not specified, Kermit waits forever or",
"  until interrupted, otherwise Kermit waits the given number of seconds.",
"  The ANSWER command puts the modem in autoanswer mode.  Subsequent DIAL",
"  commands will automatically put it (back) in originate mode.  SHOW MODEM,",
"  HELP SET MODEM for more info.",
""
};

static char *hxxdial[] = { "Syntax:  DIAL phonenumber",
"Example: DIAL 7654321",
"  \
Dials a number using an autodial modem.  First you must SET MODEM TYPE, then",
#ifdef OS2
"  SET PORT, then SET SPEED.  Then give the DIAL command, including the phone",
#else
"  SET LINE, then SET SPEED.  Then give the DIAL command, including the phone",
#endif /* OS2 */
"  number, for example:",
" ",
"   DIAL 7654321",
" ",
#ifdef NETCONN
"  If the modem is on a network modem server, SET HOST first, then SET MODEM,",
"  then DIAL.  See also SET DIAL, SET MODEM, SET LINE, SET HOST, SET SPEED,",
"  REDIAL, and PDIAL.",
" ",
#else
"  See also SET DIAL, SET MODEM, SET LINE, SET SPEED, PDIAL, and REDIAL.",
" ",
#endif /* NETCONN */
"If the phonenumber starts with a letter, and if you have used the SET DIAL",
"DIRECTORY command to specify one or more dialing-directory files, Kermit",
"looks it up in the given file(s); if it is found, the name is replaced by",
"the number or numbers associated with the name.  If it is not found, the",
"name is sent to the modem literally.",
" ",
"If the phonenumber starts with an equals sign (\"=\"), this forces the part",
"after the = to be sent literally to the modem, even if it starts with a",
"letter, without any directory lookup.",
" ",
"You can also give a list of phone numbers enclosed in braces, e.g:",
" ",
"  dial {{7654321}{8765432}{+1 (212 555-1212}}",
" ",
"(Each number is enclosed in braces and the entire list is also enclosed in",
"braces.)  In this case, each number is tried until there is an answer.  The",
"phone numbers in this kind of list can not be names of dialing directory",
"entries.",
" ",
"A dialing directory is a plain text file, one entry per line:",
" ",
"  name  phonenumber  ;  comments",
" ",
"for example:",
" ",
"  work    9876543              ; This is a comment",
"  e-mail  +1  (212) 555 4321   ; My electronic mailbox",
"  heise   +49 (511) 535 2301   ; Verlag Heinz Heise BBS",
" ",
"If a phone number starts with +, then it must include country code and",
"area code, and C-Kermit will try to handle these appropriately based on",
"the current locale (HELP SET DIAL for further info); these are called",
"PORTABLE entries.  If it does not start with +, it is dialed literally.",
" ",
"If more than one entry is found with the same name, Kermit dials all of",
"them until the call is completed; if the entries are in portable format,",
"Kermit dials then in cheap-to-expensive order: internal, then local, then",
"long-distance, then international, based on its knowledge of your local",
"country code and area code (see HELP SET DIAL).",
" ",
"Specify your dialing directory file(s) with the SET DIAL DIRECTORY command.",
"" };

#ifdef CK_TAPI
static char *hxxtapi[] = {
"TAPI CONFIGURE-LINE <tapi-line>",
"  Displays the TAPI Configure Line Dialog box and allows you to",
"  alter the default configuration for the specified <tapi-line>.",
" ",
"TAPI DIALING-PROPERTIES",
"  Displays the TAPI Dialing Properties (locations) Dialog box.  The",
"  Dialing rules may be changed and locations created and deleted.",
"  When the dialog box is closed, K-95 imports the current Dialing",
"  Properties' Location into the Kermit DIAL command settings.",
""};

static char *hxytapi[] = {
"SET TAPI LINE <tapi-line>",
"  Opens a TAPI device for use by Kermit.",
" ",
"SET TAPI MODEM-DIALING {ON, [OFF]}",
"  If TAPI MODEM-DIALING is OFF when SET TAPI LINE is issued, Kermit opens",
"  the TAPI device directly as a \"raw port\".  The device is unavailable to",
"  other applications and Kermit performs dialing functions using its",
"  built-in dialing and modem databases.  If TAPI MODEM-DIALING is ON, TAPI",
"  handles all dialing functions and the port may be shared with other",
"  applications when a call in not active.  When TAPI MODEM-DIALING is OFF,",
"  SET MODEM TYPE TAPI Kermit uses the TAPI modem commands imported from the",
"  Windows Registry during the previous SET TAPI LINE call.",
" ",
"SET TAPI LOCATION <tapi-location>",
"  Specifies the TAPI location to make current for the entire system.  The",
"  <tapi-location>'s dialing properties are imported into Kermit's SET DIAL",
"  command database.",
" ",
"SET TAPI PHONE-NUMBER-CONVERSIONS {ON, OFF, [AUTO]}",
"  Controls whether the phone number conversions are performed by TAPI (ON)",
"  or by Kermit (OFF), or according the type of port that was selected",
"  (AUTO); AUTO is the default, and is equivalent to ON if the current",
"  LINE/PORT is a TAPI device and TAPI MODEM-DIALING is ON, OFF otherwise.",
" ",
"SET TAPI MODEM-LIGHTS {[ON], OFF}",
"  Displays a modem lights indicator on the Windows 95 Taskbar.  Does nothing",
"  in Windows NT 4.0.",
" ",
"SET TAPI MANUAL-DIALING {ON, [OFF]}",
"  Displays a dialog box during dialing requesting that you manually dial the",
"  phone before continuing.  Applies only when TAPI MODEM-DIALING is ON.",
" ",
"SET TAPI WAIT-FOR-CREDIT-CARD-TONE <seconds>",
"  Some modems don't support the '$' (BONG) symbol during dialing, which",
"  means \"wait for credit card tone before continuing.\"  If TAPI recognizes",
"  the modem as one that does not support BONG, it replaces the '$' with",
"  <seconds> worth of pauses.  The default is 8 seconds.  This command",
"  applies only when TAPI MODEM-DIALING is ON",
" ",
"SET TAPI PRE-DIAL-TERMINAL {ON, [OFF]}",
"SET TAPI POST-DIAL-TERMINAL {ON, [OFF]}",
"  Displays a small terminal window that may be used to communicate with the",
"  modem or the host prior to or immediately after dialing; applies only when",
"  TAPI MODEM-DIALING is ON",
" ",
"SET TAPI INACTIVITY-TIMEOUT <minutes>",
"  Specifies the number of minutes of inactivity that may go by before TAPI",
"  disconnects the line.  The default is 0 which means disable this function.",
"  Applies only when TAPI MODEM-DIALING is ON.",
" ",
"SET TAPI USE-WINDOWS-CONFIGURATION {ON, [OFF]}",
"  Specifies whether the TAPI modem values for speed, parity, stop bits, flow",
"  control, etc. are used in preference to the current values specified",
"  within Kermit-95.",
" ",
""};
#endif /* CK_TAPI */

#endif /* NODIAL */

#ifdef TNCODE
static char *hmxxtel[] = {
"Syntax: TELNET [ switches ] [ host [ service ] ]",
"  Equivalent to SET NETWORK TYPE TCP/IP, SET HOST host [ service ] /TELNET,",
"  IF SUCCESS CONNECT.  If host is omitted, the previous connection (if any)",
"  is resumed.  Depending on how Kermit has been built switches may be",
"  available to require a secure authentication method and bidirectional",
"  encryption.  See HELP SET TELNET for more info.",
" ",
#ifdef CK_AUTHENTICATION
" /AUTH:<type> is equivalent to SET TELNET AUTH TYPE <type> and",
"   SET TELOPT AUTH REQUIRED with the following exceptions.  If the type",
"   is AUTO, then SET TELOPT AUTH REQUESTED is executed and if the type",
"   is NONE, then SET TELOPT AUTH REFUSED is executed.",
" ",
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
" /ENCRYPT:<type> is equivalent to SET TELNET ENCRYPT TYPE <type>",
"   and SET TELOPT ENCRYPT REQUIRED REQUIRED with the following exceptions.",
"   If the type is AUTO then SET TELOPT AUTH REQUESTED REQUESTED is executed",
"   and if the type is NONE then SET TELOPT ENCRYPT REFUSED REFUSED is",
"   executed.",
" ",
#endif /* CK_ENCRYPTION */
" /USERID:[<name>]",
"   This switch is equivalent to SET LOGIN USERID <name> or SET TELNET",
"   ENVIRONMENT USER <name>.  If a string is given, it sent to host during",
"   Telnet negotiations; if this switch is given but the string is omitted,",
"   no user ID is sent to the host.  If this switch is not given, your",
"   current USERID value, \\v(userid), is sent.  When a userid is sent to the",
"   host it is a request to login as the specified user.",
" ",
#ifdef CK_AUTHENTICATION
" /PASSWORD:[<string>]",
"   This switch is equivalent to SET LOGIN PASSWORD.  If a string is given,",
"   it is treated as the password to be used (if required) by any Telnet",
"   Authentication protocol (Kerberos Ticket retrieval, Secure Remote",
"   Password, or X.509 certificate private key decryption.)  If no password",
"   switch is specified a prompt is issued to request the password if one",
"   is required for the negotiated authentication method.",
#endif /* CK_AUTHENTICATION */
""};

static char *hxtopt[] = {
"TELOPT { AO, AYT, BREAK, CANCEL, EC, EL, EOF, EOR, GA, IP, DMARK, NOP, SE,",
"         SUSP, SB [ option ], DO [ option ], DONT [ option ],",
"         WILL [ option ], WONT [option] }",
"  This command lets you send all the Telnet protocol commands.  Note that",
"  certain commands do not require a response, and therefore can be used as",
"  nondestructive \"probes\" to see if the Telnet session is still open;",
"  e.g.:\n",
"    set host xyzcorp.com",
"    ...",
"    telopt nop",
"    telopt nop",
"    if fail stop 1 Connection lost\n",
"  TELOPT NOP is sent twice because the failure of the connection will not",
"  be detected until the second send is attempted.  This command is meant",
"  primarily as a debugging tool for the expert user.",
""};
#endif /* TNCODE */

#endif /* NOHELP */

/*  D O H L P  --  Give a help message  */

_PROTOTYP( int dohset, (int) );
#ifndef NOCMDL
_PROTOTYP( int dohopts, (void) );
#endif /* NOCMDL */
#ifndef NOSPL
_PROTOTYP( int dohfunc, (int) );
extern struct keytab fnctab[];
extern int nfuncs;
#endif /* NOSPL */
#ifdef OS2
#ifndef NOKVERBS
_PROTOTYP( int dohkverb, (int) );
extern struct keytab kverbs[];
extern int nkverbs;
#endif /* NOKVERBS */
#endif /* OS2 */

#ifndef NOSPL
static char * hxxdcl[] = {
"Syntax: ARRAY verb operands...",
" ",
"Declares arrays and performs various operations on them.  Arrays have",
"the following syntax:",
" ",
"  \\&a[n]",
" ",
"where \"a\" is a letter and n is a number or a variable with a numeric value",
"or an arithmetic expression.  The value of an array element can be anything",
"at all -- a number, a character, a string, a filename, etc.",
" ",
"The following ARRAY verbs are available:",
" ",
"[ ARRAY ] DECLARE arrayname[n] [ = initializers... ]",
"  Declares an array of the given size, n.  The resulting array has n+1",
"  elements, 0 through n.  Array elements can be used just like any other",
"  variables.  Initial values can be given for elements 1, 2, ... by",
"  including = followed by one or more values separated by spaces.  If you",
"  omit the size, the array is sized according to the number of initializers;",
"  if none are given the array is destroyed and undeclared if it already",
"  existed.  The ARRAY keyword is optional.  Synonym: [ ARRAY ] DCL.",
" ",
"ARRAY SHOW [ arrayname ]",
"  Displays the contents of the given array.  A range specifier can be",
"  included to display a segment of the array, e.g. \"array show \\&a[1:24].\""
,
"  If the arrayname is omitted, all declared arrays are listed, but their",
"  contents is not shown.  Synonym: SHOW ARRAY.",
" ",
"ARRAY CLEAR arrayname",
"  Clears all elements of the array, i.e. sets them to empty values.",
"  You may include a range specifier to clear a segment of the array rather",
"  than the whole array, e.g. \"array clear \\%a[22:38]\"",
" ",
"ARRAY SET arrayname value",
"  Sets all elements of the array to the given value.  You may specify a",
"  range to set a segment of the array, e.g. \"array set \\%a[2:9] 0\"",
" ",
"ARRAY DESTROY arrayname",
"  Destroys and undeclares the given array.",
" ",
"ARRAY RESIZE arrayname number",
"  Changes the size of the given array, which must already exist, to the",
"  number given.  If the number is smaller than the current size, the extra",
"  elements are discarded; if it is larger, new empty elements are added.",
" ",
"ARRAY COPY array1 array2",
"  Copys array1 to array2.  If array2 has not been declared, it is created",
"  automatically.  Range specifiers may be given on one or both arrays.",
" ",
"[ ARRAY ] SORT [ switches ] array-name [ array2 ]",
"  Sorts the given array lexically according to the switches.  Element 0 of",
"  the array is excluded from sorting by default.  The ARRAY keyword is",
"  optional.  If a second array name is given, that array is sorted according",
"  to the first one.  Switches:",
" ",
"  /CASE:{ON,OFF}",
"    If ON, alphabetic case matters; if OFF it is ignored.  If this switch is",
"    omitted, the current SET CASE setting applies.",
" ",
"  /KEY:number",
"    \
Position (1-based column number) at which comparisons begin, 1 by default.",
" ",
"  /NUMERIC",
"    Specifies a numeric rather than lexical sort.",
" ",
"  /RANGE:low[:high]",
"    The range of elements, low through high, to be sorted.  If this switch",
"    is not given, elements 1 through the dimensioned size are sorted.  If",
"    :high is omitted, the dimensioned size is used.  To include element 0 in",
"    a sort, use /RANGE:0 (to sort the whole array) or /RANGE:0:n (to sort",
"    elements 0 through n).  You can use a range specifier in the array name",
"    instead of the /RANGE switch.",
" ",
"  /REVERSE",
"    Sort in reverse order.  If this switch is not given, the array is sorted",
"    in ascending order.",
" ",
"Various functions are available for array operations; see HELP FUNCTION for",
"details.  These include \\fdimension(), \\farraylook(), \\ffiles(), \
\\fsplit(),",
"and many more.",
""};
#endif /* NOSPL */

#ifdef ZCOPY
static char * hmxxcpy[] = {
"Syntax: COPY [ switches ] file1 file2",
"  Copies the source file (file1) to the destination file (file2).  If file2",
"  is a directory, this command copies file1 under its own name to the given",
"  directory.  Only one file at a time may be copied; wildcards are not",
"  supported.  Switches:",
" ",
"  /LIST",
"    Print the filenames and status while copying.  Synonyms: /LOG, /VERBOSE",
" ",
"  /NOLIST",
"    Copy silently (default). Synonyms: /NOLOG, /QUIET",
" ",
"  /SWAP-BYTES",
"    Swap bytes while copying.",
#ifndef NOSPL
" ",
"  /FROMB64",
"    Convert from Base64 encoding while copying.",
" ",
"  /TOB64",
"    Convert to Base64 encoding while copying.",
#endif /* NOSPL */
""
};
#endif /* ZCOPY */

#ifndef NOFRILLS
static char * hmxxren[] = {
"Syntax: RENAME [ switches ] file1 file2",
"  Renames the source file (file1) to the destination file (file2).  If file2",
"  is a directory, this command moves file1 under its own name to the given",
"  directory.  Only one file at a time may be renamed; wildcards are not",
"  supported.  Switches:",
" ",
"  /LIST",
"    Print the filenames and status while renaming.  Synonyms: /LOG, /VERBOSE",
" ",
"  /NOLIST",
"    Rename silently (default). Synonyms: /NOLOG, /QUIET",
""
};
#endif /* NOFRILLS */

static char *
cmdlhlp[] = {
"Command-line options are given after the program name in the system",
"command that you use to start C-Kermit.  Example:",
" ",
" kermit -i -s oofa.exe",
" ",
"tells C-Kermit to send (-s) the file oofa.exe in binary (-i) mode.",
" ",
"Command-line options are case-sensitive; \"-s\" is different from \"-S\".",
#ifdef VMS
"In VMS, uppercase options must be enclosed in doublequotes: ",
" ",
" $ kermit \"-Y\" \"-S\" -s oofa.txt ",
#endif /* VMS */
" ",
"If any \"action options\" are included on the command line, C-Kermit exits",
"after executing its command-line options.  If -S is included, or no action",
"options were given, C-Kermit enters its interactive command parser and",
"issues its prompt.",
" ",
"Command-line options are single characters preceded by dash (-).  Some",
"require an \"argument,\" others do not.  If an argument contains spaces, it",
"must be enclosed in doublequotes:",
" ",
" kermit -s \"filename with spaces\"",
" ",
"\
An option that does not require an argument can be bundled with other options:"
,
" ",
" kermit -Qis oofa.exe",
" ",
"Exceptions to the rules:",
" ",
" . If the first command-line option is a filename, Kermit executes commands",
"   from the file.  Additional command-line options can follow the filename.",
" ",
" . The special option \"=\" (equal sign) or \"--\" (double hyphen) means to",
"   treat the rest of the command line as data, rather than commands; this",
"   data is placed in the argument vector array, \\&@[], along with the other",
"   items on the command line, and also in the top-level \\%1..\\%9 variables."
,
" ",
#ifdef KERBANG
" . A similar option \"+\" (plus sign) means: the name of a Kermit script",
"   file follows.  This file is to be executed, and its name assigned to \\%0",
"   and \\&_[0].  All subsequent command-line arguments are to be ignored by",
"   Kermit but made available to the script as \\%1, \\%2, ..., as well as",
"   in the argument-vector arrays.  The initialization file is not executed",
"   automatically in this case.",
" ",
#endif /* KERBANG */
" . The -s option can accept multiple filenames, separated by spaces.",
" ",
" . the -j and -J options allow an optional second argument, the TCP port",
"   name or number.",
" ",
"Type \"help options all\" to list all the command-line options.",
"Type \"help option x\" to see the help message for option x.",
" ",
"C-Kermit also offers a selection of \"extended command-line\" options.",
"These begin with two dashes, followed by a keyword, and then, if the option",
"has arguments, a colon (:) or equal sign (=) followed by the argument.",
"Unlike single-letter options, extended option keywords aren't case sensitive",
"and they can be abbreviated to any length that still distinguishes them from",
"other extended-option keywords.  Example:",
" ",
"  kermit --banner:oofa.txt",
" ",
"which designates the file oofa.txt to be printed upon startup, rather than",
"the built-in banner (greeting) text.  To obtain a list of available",
"extended options, type \"help extended-options ?\".  To get help about all",
"extended options, type \"help extended-options\".  To get help about a",
"particular extended option, type \"help extended-option xxx\", where \"xxx\"",
"is the option keyword.",
#ifndef NOIKSD
" ",
"At present, most of the extended options apply only to the Internet Kermit",
"Service Daemon (IKSD).  Type \"help iksd\" for details.",
#endif /* NOIKSD */
" ",
""
};

#ifndef NOHELP
int
doxopts() {
    extern char * xopthlp[], * xarghlp[];
    extern struct keytab xargtab[];
    extern int nxargs;
    int i, x, y, n = 0;
#ifdef CK_TTGWSIZ
#ifdef OS2
    ttgcwsz();
#else /* OS2 */
    /* Check whether window size changed */
    if (ttgwsiz() > 0) {
        if (tt_rows > 0 && tt_cols > 0) {
            cmd_rows = tt_rows;
            cmd_cols = tt_cols;
        }
    }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */
    y = cmkey(xargtab,
              nxargs,
              "Extended argument without the \"--\" prefix",
              "",
              xxstring
              );
    if (y == -3) {
        printf("\n");
        for (i = 0; i <= XA_MAX; i++) {
            if (xopthlp[i]) {
                printf("%s\n",xopthlp[i]);
                printf("   %s\n",xarghlp[i]);
                printf("\n");
                n += 3;
                if (n > (cmd_rows - 6)) {
                    if (!askmore())
                      return(0);
                    else
                      n = 0;
                }
            }
        }
        return(1);
    } else if (y < 0)
      return(y);
    if ((x = cmcfm()) < 0)
      return(x);
    printf("\n%s\n",xopthlp[y]);
    printf("   %s\n\n",xarghlp[y]);
    return(1);
}

#ifndef NOCMDL
int
dohopts() {
    int i, n, x, y, z, all = 0, msg = 0;
    char *s;
    extern char *opthlp[], *arghlp[];
    extern int optact[];
    if ((x = cmtxt("A command-line option character,\n\
or the word ALL, or carriage return for an overview",
                   "", &s, xxstring)) < 0)
      return(x);
    if (!*s)
      msg = 1;
    else if (!strcmp(s,"all") || (!strcmp(s,"ALL")))
      all = 1;
    else if (*s == '-')                 /* Be tolerant of leading hyphen */
      s++;
    if (!all && (int)strlen(s) > 1) {
        printf("?A single character, please, or carriage to list them all.\n");
        return(-9);
    }
    if (all) {
        y = 33;
        z = 127;
    } else {
        y = *s;
        z = (y == 0) ? 127 : y;
        if (y == 0) y = 33;
    }
#ifdef CK_TTGWSIZ
#ifdef OS2
    ttgcwsz();
#else /* OS2 */
    /* Check whether window size changed */
    if (ttgwsiz() > 0) {
        if (tt_rows > 0 && tt_cols > 0) {
            cmd_rows = tt_rows;
            cmd_cols = tt_cols;
        }
    }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */
    printf("\n");
    for (i = 0, n = 1; msg != 0 && *cmdlhlp[i]; i++) {
        printf("%s\n",cmdlhlp[i]);
        if (++n > (cmd_rows - 3)) {
           if (!askmore())
             return(0);
           else
             n = 0;
        }
    }
    if (all) {
        printf("The following command-line options are available:\n\n");
        n += 2;
    }
    for (i = y; msg == 0 && i <= z; i++) {
        if (!opthlp[i])
          continue;
        if (arghlp[i]) {                /* Option with arg */
            printf(" -%c <arg>%s\n",(char)i,(optact[i]?" (action option)":""));

            printf("     %s\n",opthlp[i]);
            printf("     Argument: %s\n\n",arghlp[i]);
            x = 4;
        } else {                        /* Option without arg */
            printf(" -%c  %s%s\n",
                   (char)i, opthlp[i],
                   (optact[i]?" (action option)":"")
                  );
            printf("     Argument: (none)\n\n");
            x = 3;
        }
        n += x;
        if (n > (cmd_rows - x - 1)) {
            if (!askmore())
              return(0);
           else
              n = 0;
        }
    }
    return(0);
}
#endif /* NOCMDL */
#endif /* NOHELP */

#ifdef CKCHANNELIO
static char * hxxfile[] = {
"Syntax: FILE <subcommand> [ switches ] <channel> [ <data> ]",
"  Opens, closes, reads, writes, and manages local files.",
" ",
"The FILE commands are:",
" ",
"  FILE OPEN   (or FOPEN)   -- Open a local file.",
"  FILE CLOSE  (or FCLOSE)  -- Close an open file.",
"  FILE READ   (or FREAD)   -- Read data from an open file.",
"  FILE WRITE  (or FWRITE)  -- Write data to an open file.",
"  FILE LIST   (or FLIST)   -- List open files.",
"  FILE STATUS (or FSTATUS) -- Show status of a channel.",
"  FILE REWIND (or FREWIND) -- Rewind an open file",
"  FILE COUNT  (or FCOUNT)  -- Count lines or bytes in an open file",
"  FILE SEEK   (or FSEEK)   -- Seek to specified spot in an open file.",
"  FILE FLUSH  (or FFLUSH)  -- Flush output buffers for an open file.",
" ",
"Type HELP FILE OPEN or HELP FOPEN for details about FILE OPEN;",
"type HELP FILE CLOSE or HELP FCLOSE for details about FILE CLOSE, and so on.",
" ",
"The following variables are related to the FILE command:",
" ",
"  \\v(f_max)     -- Maximum number of files that can be open at once",
"  \\v(f_error)   -- Completion code of most recent FILE command or function",
"  \\v(f_count)   -- Result of most recent FILE COUNT command",
" ",
"The following functions are related to the FILE command:",
" ",
"  \\F_eof()      -- Check if channel is at EOF",
"  \\F_pos()      -- Get channel read/write position (byte number)",
"  \\F_line()     -- Get channel read/write position (line number)",
"  \\F_handle()   -- Get file handle",
"  \\F_status()   -- Get channel status",
"  \\F_getchar()  -- Read character",
"  \\F_getline()  -- Read line",
"  \\F_getblock() -- Read block",
"  \\F_putchar()  -- Write character",
"  \\F_putline()  -- Write line",
"  \\F_putblock() -- Write block",
"  \\F_errmsg()   -- Error message from most recent FILE command or function",
" ",
"Type HELP <function-name> for information about each one.",
""
};

static char * hxxf_op[] = {
"Syntax: FILE OPEN [ switches ] <variable> <filename>",
"  Opens the file indicated by <filename> in the mode indicated by the",
"  switches, if any, or if no switches are included, in read-only mode, and",
"  assigns a channel number for the file to the given variable.",
"  Synonym: FOPEN.  Switches:",
" ",
"/READ",
"  Open the file for reading.",
" ",
"/WRITE",
"  Open the file for writing.  If /READ was not also specified, this creates",
"  a new file.  If /READ was specifed, the existing file is preserved, but",
"  writing is allowed.  In both cases, the read/write pointer is initially",
"  at the beginning of the file.",
" ",
"/APPEND",
"  If the file does not exist, create a new file and open it for writing.",
"  If the file exists, open it for writing, but with the write pointer",
"  positioned at the end.",
" ",
"/BINARY",
#ifdef VMS
"  Opens the file in binary mode to inhibit end-of-line conversions.",
#else
#ifdef OS2
"  Opens the file in binary mode to inhibit end-of-line conversions.",
#else
#ifdef UNIX
"  This option is ignored in UNIX.",
#else
"  This option is ignored on this platform.",
#endif /* UNIX */
#endif /* OS2 */
#endif /* VMS */

"Switches can be combined in an way that makes sense and is supported by the",
"underlying operating system.",
""
};

static char * hxxf_cl[] = {
"Syntax: FILE CLOSE <channel>",
"  Closes the file on the given channel if it was open.",
"  Also see HELP FILE OPEN.  Synonym: FCLOSE.",
""
};

static char * hxxf_fl[] = {
"Syntax: FILE FLUSH <channel>",
"  Flushes output buffers on the given channel if it was open, forcing",
"  all material previously written to be committed to disk.  Synonym: FFLUSH.",
"  Also available as \\F_flush().",
""
};

static char * hxxf_li[] = {
"Syntax: FILE LIST",
"  Lists the channel number, name, modes, and position of each file opened",
"  with FILE OPEN.  Synonym: FLIST.",
""
};

static char * hxxf_re[] = {
"Syntax: FILE READ [ switches ] <channel> [ <variable> ]",
"  Reads data from the file on the given channel number into the <variable>,",
"  if one was given; if no variable was given, the result is printed on",
"  the screen.  The variable should be a macro name rather than a \\%x",
"  variable or array element if you want backslash characters in the file to",
"  be taken literally.  Synonym: FREAD.  Switches:",
" ",
"/LINE",
"  Specifies that a line of text is to be read.  A line is defined according",
"  to the underlying operating system's text-file format.  For example, in",
"  UNIX a line is a sequence of characters up to and including a linefeed.",
"  The line terminator (if any) is removed before assigning the text to the",
"  variable.  If no switches are included with the FILE READ command, /LINE",
"  is assumed.",
" ",
"/SIZE:number",
"  Specifies that the given number of bytes (characters) is to be read.",
"  This gives a semblance of \"record i/o\" for files that do not necessarily",
"  contain lines.  The resulting block of characters is assigned to the",
"  variable without any editing.",
" ",
"/CHARACTER",
"  Equivalent to /SIZE:1.  If FILE READ /CHAR succeeds but the <variable> is",
"  empty, this indicates a NUL byte was read.",
" ",
"Synonym: FREAD.",
"Also available as \\F_getchar(), \\F_getline(), \\F_getblock().",
""
};

static char * hxxf_rw[] = {
"Syntax: FILE REWIND <channel>",
"  If the channel is open, moves the read/write pointer to the beginning of",
"  the file.  Equivalent to FILE SEEK <channel> 0.  Synonym: FREWIND.",
"  Also available as \\F_rewind().",
""
};

static char * hxxf_se[] = {
"Syntax: FILE SEEK [ switches ] <channel> { [{+,-}]<number>, EOF }",
"  Switches are /BYTE, /LINE, /RELATIVE, ABSOLUTE.",
"  Moves the file pointer for this file to the given position in the",
"  file.  Subsequent FILE READs or WRITEs will take place at that position.",
"  If neither the /RELATIVE nor /ABSOLUTE switch is given, an unsigned",
"  <number> is absolute; a signed number is relative.  EOF means to move to",
"  the end of the file.  Synonym: FSEEK.  Also available as \\F_seek().",
""
};

static char * hxxf_st[] = {
"Syntax: FILE STATUS <channel>",
"  If the channel is open, this command shows the name of the file, the",
"  switches it was opened with, and the current read/write position.",
"  Synonym: FSTATUS",
""
};

static char * hxxf_co[] = {
"Syntax: FILE COUNT [ { /BYTES, /LINES, /LIST, /NOLIST } ] <channel>",
"  If the channel is open, this command prints the nubmer of bytes (default)",
"  or lines in the file if at top level or if /LIST is included; if /NOLIST",
"  is given, the result is not printed.  In all cases the result is assigned",
"  to \\v(f_count).  Synonym: FCOUNT",
""
};

static char * hxxf_wr[] = {
"FILE WRITE [ switches ] <channel> <text>",
"  Writes the given text to the file on the given channel number.  The <text>",
"  can be literal text or a variable, or any combination.  If the text might",
"  contain leading or trailing spaces, it must be enclosed in braces if you",
"  want to preserve them.  Synonym: FWRITE.  Switches:",
" ",
"/LINE",
"  Specifies that an appropriate line terminator is to be added to the",
"  end of the <text>.  If no switches are included, /LINE is assumed.",
" ",
"/SIZE:number",
"  Specifies that the given number of bytes (characters) is to be written.",
"  If the given <text> is longer than the requested size, it is truncated;",
"  if is shorter, it is padded according /LPAD and /RPAD switches.  Synonym:",
"  /BLOCK.",
" ",
"/LPAD[:value]",
"  If /SIZE was given, but the <text> is shorter than the requested size,",
"  the text is padded on the left with sufficient copies of the character",
"  whose ASCII value is given to write the given length.  If no value is",
"  specified, 32 (the code for Space) is used.  The value can also be 0 to",
"  write the indicated number of NUL bytes.  If /SIZE was not given, this",
"  switch is ignored.",
" ",
"/RPAD[:value]",
"  Like LPAD, but pads on the right.",
" ",
"/STRING",
"  Specifies that the <text> is to be written as-is, with no terminator added."
,
" ",
"/CHARACTER",
"  Specifies that one character should be written.  If the <text> is empty or",
"  not given, a NUL character is written; otherwise the first character of",
"  <text> is given.",
" ",
"Synonym FWRITE.",
"Also available as \\F_putchar(), \\F_putline(), \\F_putblock().",
""
};

static int
dohfile(cx) int cx; {
    extern struct keytab fctab[];
    extern int nfctab;
    int x;
    if (cx == XXFILE) {			/* FILE command was given */
	/* Get subcommand */
	if ((cx = cmkey(fctab,nfctab,"Operation","",xxstring)) < 0) {
	    if (cx == -3) {
                if ((x = cmcfm()) < 0)
                  return(x);
                cx = XXFILE;
	    } else
              return(cx);
	}
        if ((x = cmcfm()) < 0)
          return(x);
	switch (cx) {
	  case FIL_CLS: cx = XXF_CL; break;
	  case FIL_FLU: cx = XXF_FL; break;
	  case FIL_LIS: cx = XXF_LI; break;
	  case FIL_OPN: cx = XXF_OP; break;
	  case FIL_REA: cx = XXF_RE; break;
	  case FIL_REW: cx = XXF_RW; break;
	  case FIL_SEE: cx = XXF_SE; break;
	  case FIL_STA: cx = XXF_ST; break;
	  case FIL_WRI: cx = XXF_WR; break;
	  case FIL_COU: cx = XXF_CO; break;
	}
    }
    switch (cx) {
      case XXFILE: return(hmsga(hxxfile));
      case XXF_CL: return(hmsga(hxxf_cl));
      case XXF_FL: return(hmsga(hxxf_fl));
      case XXF_LI: return(hmsga(hxxf_li));
      case XXF_OP: return(hmsga(hxxf_op));
      case XXF_RE: return(hmsga(hxxf_re));
      case XXF_RW: return(hmsga(hxxf_rw));
      case XXF_SE: return(hmsga(hxxf_se));
      case XXF_ST: return(hmsga(hxxf_st));
      case XXF_WR: return(hmsga(hxxf_wr));
      case XXF_CO: return(hmsga(hxxf_co));
      default:
        return(-2);
    }
}
#endif /* CKCHANNELIO */

int
dohlp(xx) int xx; {
    int x,y;

    debug(F101,"DOHELP xx","",xx);
    if (xx < 0) return(xx);

#ifdef NOHELP
    if ((x = cmcfm()) < 0)
      return(x);
    printf("\n%s, Copyright (C) 1985, 2000,",versio);
    return(hmsga(tophlp));

#else /* help is available */

    if (helpfile)
      return(dotype(helpfile,xaskmore,0,0,NULL,0,NULL));

#ifdef CKCHANNELIO
    if (xx == XXFILE)
      return(dohfile(xx));
    else if (xx == XXF_RE || xx == XXF_WR || xx == XXF_OP ||
	     xx == XXF_CL || xx == XXF_SE || xx == XXF_RW ||
	     xx == XXF_FL || xx == XXF_LI || xx == XXF_ST || xx == XXF_CO)
      return(dohfile(xx));
#endif /* CKCHANNELIO */

    switch (xx) {

#ifndef NOSPL
case XXASS:                             /* ASSIGN */
    return(hmsga(hxxass));

case XXASK:                             /* ASK */
    return(hmsga(hxxask));

case XXASKQ:
    return(hmsga(hxxaskq));

case XXAPC:
    return(hmsg("Syntax: APC text\n\
  Echoes the text within a VT220/320/420 Application Program Command."));
#endif /* NOSPL */

#ifndef NOFRILLS
case XXBUG:
    return(hmsg("Describes how to get technical support."));
#endif /* NOFRILLS */

#ifndef NOSPL
case XXBEEP:
#ifdef OS2
    return(hmsg("Syntax: BEEP [ { ERROR, INFORMATION, WARNING } ]\n\
  Generates a bell according to the current settings.  If SET BELL is set to\n\
  \"system-sounds\" then the appropriate System Sound will be generated.\n\
  Default is INFORMATION."));
#else /* OS2 */
    return(hmsg("Syntax: BEEP\n\
Sends a BEL character to your terminal."));
#endif /* OS2 */
#endif /* NOSPL */

case XXBYE:                             /* BYE */
    return(hmsg(hmxxbye));

case XXCHK:                             /* check */
    return(hmsg("\
Syntax: CHECK name\n\
  Checks\
  to see if the named feature is included in this version of C-Kermit.\n\
  To list the features you can check, type \"check ?\"."));

#ifndef NOFRILLS
case XXCLE:                             /* clear */
    return(hmsga(hmxxcle));
#endif /* NOFRILLS */

case XXCLO:                             /* close */
    return(hmsga(hmxxclo));

case XXCOM:                             /* comment */
#ifndef STRATUS /* Can't use # for comments in Stratus VOS */
    return(hmsg("\
Syntax: COMMENT text\n\
Example: COMMENT - this is a comment.\n\
  Introduces a comment.  Beginning of command line only.  Commands may also\n\
  have trailing comments, introduced by ; or #."));
#else
    return(hmsg("\
Syntax: COMMENT text\n\
Example: COMMENT - this is a comment.\n\
  Introduces a comment.  Beginning of command line only.  Commands may also\n\
  have trailing comments, introduced by ; (semicolon)."));
#endif /* STRATUS */

#ifndef NOLOCAL
case XXCON:                             /* CONNECT */
case XXCQ:                              /* CQ == CONNECT /QUIETLY */
    hmsga(hmxxcon);
    printf("Your escape character is Ctrl-%c (ASCII %d, %s)\r\n",
           ctl(escape), escape, (escape == 127 ? "DEL" : ccntab[escape]));
    return(0);
#endif /* NOLOCAL */

#ifdef ZCOPY
case XXCPY:
    return(hmsga(hmxxcpy));
#endif /* ZCOPY */

#ifdef NOFRILLS
case XXREN:
    return(hmsga(hmxxren));
#endif /* NOFRILLS */

case XXCWD:                             /* CD / CWD */
#ifdef vms
    return(hmsg("Syntax: CD [ directory or device:directory ]\n\
  Change Working Directory, equivalent to VMS SET DEFAULT command"));
#else
#ifdef datageneral
    return(hmsg("Change Working Directory, equivalent to DG 'dir' command"));
#else
#ifdef OS2
  return(hmsg("Syntax: CD [ directoryname ]\n\
  Change Directory.  If directoryname is not specified, changes to directory\n\
  specified by HOME environment variable, if any.  Also see HELP SET CD"));
#else
    return(hmsg("Syntax: CD [ directoryname ]\n\
  Change Directory.  If directory name omitted, changes to your home\n\
  directory.  Also see HELP SET CD."));
#endif /* OS2 */
#endif /* datageneral */
#endif /* vms */

#ifndef NOSPL
case XXARRAY:
case XXDCL:				/* DECLARE */
case XXSORT:
    return(hmsga(hxxdcl));

case XXDEF:                             /* DEFINE */
    return(hmsga(hxxdef));

case XXUNDEF:                           /* UNDEFINE */
    return(hmsg("Syntax:  UNDEFINE variable-name\n\
  Undefines a macro or variable."));
#endif /* NOSPL */

#ifndef NOFRILLS
case XXDEL:                             /* delete */
    return(hmsga(hmxxdel));
#endif /* NOFRILLS */

#ifndef NODIAL
case XXDIAL:                            /* DIAL, etc... */
    return(hmsga(hxxdial));

case XXPDIA:                            /* PDIAL */
    return(hmsg("Syntax: PDIAL phonenumber\n\
  Partially dials a phone number.  Like DIAL but does not wait for carrier\n\
  or CONNECT message."));

case XXRED:
    return(hmsg("Redial the number given in the most recent DIAL commnd."));

case XXANSW:                            /* ANSWER */
    return(hmsga(hxxansw));

case XXLOOK:                            /* LOOKUP number in directory */
    return(hmsga(hxxlook));
#endif /* NODIAL */

case XXDIR:                             /* DIRECTORY */
    return(hmsga(hmxxdir));

case XXLS:
#ifdef UNIXOROSK
    return(hmsg("Syntax: LS [ args ]\n\
  Runs \"ls\" with the given arguments."));
#else
    return(hmsga(hmxxdir));
#endif /* UNIXOROSK */

#ifndef NOSERVER
#ifndef NOFRILLS
case XXDIS:
    return(hmsg("Syntax: DISABLE command\n\
  Security for the C-Kermit server.  Prevents the client Kermit program from\n\
  executing the named REMOTE command, such as CD, DELETE, RECEIVE, etc."));
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
case XXDO:                              /* do */
    return(hmsg("Syntax: [ DO ] macroname [ arguments ]\n\
  Executes a macro that was defined with the DEFINE command.  The word DO\n\
  can be omitted.  Trailing argument words, if any, are automatically\n\
  assigned to the macro argument variables \\%1 through \\%9."));
#endif /* NOSPL */

#ifndef NOSPL
case XXDEC:
    return(hmsga(hxxdec));
#endif /* NOSPL */

case XXECH:                             /* echo */
    return(hmsg("Syntax: ECHO text\n\
  Displays the text on the screen, followed by a line terminator.  The ECHO\n\
  text may contain backslash codes.  Example: ECHO \\7Wake up!\\7.  Also see\n\
  XECHO and WRITE SCREEN."));

case XXXECH:                            /* xecho */
    return(hmsg("Syntax: XECHO text\n\
  Just like ECHO but does not add a line terminator to the text.  See ECHO."));

#ifndef NOSERVER
#ifndef NOFRILLS
case XXENA:
    return(hmsg("Syntax: ENABLE capability\n\
  For use with server mode.  Allows the client Kermit program access to the\n\
  named capability, such as CD, DELETE, RECEIVE, etc.  Opposite of DISABLE."));
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
case XXEND:                             /* end */
    return(hmsg("Syntax: END [ number [ message ] ]\n\
  Exits from the current macro or TAKE file, back to wherever invoked from.\n\
  Number is return code.  Message, if given, is printed."));

case XXEVAL:                            /* evaluate */
    return(hmsga(hmxxeval));
#endif /* NOSPL */

#ifndef NOFRILLS
case XXERR:                             /* e-packet */
    return(hmsg("Syntax: E-PACKET\n\
  Sends an Error packet to the other Kermit."));
#endif /* NOFRILLS */

case XXEXI:                             /* exit */
case XXQUI:
    return(hmsg("Syntax: QUIT (or EXIT) [ number ]\n\
  Exits from the Kermit program, closing all open files and devices,\n\
  optionally setting the program's return code to the given number.  Also\n\
  see SET EXIT."));

case XXFIN:
    return(hmsg("Syntax: FINISH\n\
  Tells the remote Kermit server to shut down without logging out."));

#ifndef NOSPL
  case XXFOR:
    return(hmsga(forhlp));
#endif /* NOSPL */

  case XXGET:
    return(hmsga(hmxxget));
  case XXMGET:
    return(hmsga(hmxxmget));

#ifndef NOSPL
#ifndef NOFRILLS
  case XXGOK:
    return(hmsg("Syntax: GETOK prompt\n\
  Prints the prompt, makes user type 'yes', 'no', or 'ok', and sets SUCCESS\n\
  or FAILURE accordingly.  Also see SET ASK-TIMER."));
#endif /* NOFRILLS */
#endif /* NOSPL */

#ifndef NOSPL
  case XXGOTO:
    return(hmsg("Syntax: GOTO label\n\
  In a TAKE file or macro, go to the given label.  A label is a word on the\n\
  left margin that starts with a colon (:).  Example:\n\n\
  :oofa\n\
  echo Hello!\n\
  goto oofa"));
#endif /* NOSPL */

  case XXHAN:
    return(hmsg("Syntax: HANGUP\n\
Hang up the phone or network connection."));

  case XXHLP:
/*
  We get confirmation here, even though we do it again in hmsga(), to prevent
  the Copyright message from being printed prematurely.  This doesn't do any
  harm, because the first call to cmcfm() sets cmflgs to 1, making the second
  call return immediately.
*/
    if ((x = cmcfm()) < 0)
      return(x);

    if (helpfile) {
        printf("\n%s, Copyright (C) 1985, 2000,\n\
Trustees of Columbia University in the City of New York.\n\n",versio);
        return(dotype(helpfile,xaskmore,3,0,NULL,0,NULL));
    } else {
        printf("\n%s, Copyright (C) 1985, 2000,",versio);
        return(hmsga(tophlp));
    }

case XXINT:
    return(hmsg("Give a brief introduction to C-Kermit."));

#ifndef NOSPL
case XXIF:
    return(hmsga(ifhlp));

case XXINC:
    return(hmsga(hxxinc));

case XXINP:
   return(hmsga(hxxinp));
#endif /* NOSPL */

#ifdef CK_MINPUT
case XXMINP:
    return(hmsga(hmxxminp));
#endif /* CK_MINPUT */

#ifndef NOSPL
case XXREI:
    return(hmsg("Syntax: REINPUT n string\n\
  Looks for the string in the text that has recently been INPUT, set SUCCESS\n\
  or FAILURE accordingly.  Timeout, n, must be specified but is ignored."));
#endif /* NOSPL */

#ifndef NOFRILLS
case XXREN:
    return(hmsga(hmxxren));
#endif /* NOFRILLS */

#ifndef NOSPL
case XXLBL:
    return(hmsg("\
  Introduces a label, like :loop, for use with GOTO in TAKE files or macros.\n\
See GOTO."));
#endif /* NOSPL */

case XXLOG:
    return(hmsga(hmxxlg));

#ifndef NOSCRIPT
case XXLOGI:
    return(hmsga(hmxxlogi));
#endif

#ifndef NOFRILLS
case XXMAI:
    return(hmsg("Syntax: MAIL filename address\n\
  Equivalent to SEND /MAIL /ADDRESS:xxx filename."));
#endif /* NOFRILLS */

#ifndef NOMSEND
case XXMSE:
    return(hmsga(hmxxmse));

case XXADD:
    return(hmsga(hmxxadd));

case XXMMOVE:
    return(hmsg("MMOVE is exactly like MSEND, except each file that is\n\
sent successfully is deleted after it is sent."));
#endif /* NOMSEND */

#ifndef NOSPL
case XXOPE:
    return(hmsga(openhlp));
#endif /* NOSPL */

case XXNEW:
    return(hmsg(
"  Prints news of new features since publication of \"Using C-Kermit\"."));

case XXUPD:
    return(hmsg(
"  New features are described in the file DOCS\\UPDATES.TXT."));

#ifndef NOSPL
case XXOUT:
    return(hmsga(hxxout));
#endif /* NOSPL */

#ifdef ANYX25
#ifndef IBMX25
case XXPAD:
    return(hmsga(hxxpad));
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOSPL
case XXPAU:
    return(hmsga(hxxpau));

case XXMSL:
    return(hmsga(hxxmsl));
#endif /* NOSPL */

#ifdef TCPSOCKET
case XXPNG:
    return(hmsg("Syntax: PING [ IP-hostname-or-number ]\n\
  Checks if the given IP network host is reachable.  Default host is from\n\
  most recent SET HOST or TELNET command.  Runs system PING program, if any.")
           );

case XXFTP:
    return(hmsg("Syntax: FTP [ IP-hostname-or-number ]\n\
  Makes an FTP connection to the given IP host or, if no host specified, to\n\
  the current host.  Uses the system's FTP program, if any."));
#endif /* TCPSOCKET */

#ifndef NOFRILLS
case XXPRI:
#ifdef UNIX
    return(hmsg("Syntax: PRINT file [ options ]\n\
  Prints the local file on a local printer with the given options.  Also see\n\
  HELP SET PRINTER."));
#else
#ifdef VMS
    return(hmsg("Syntax: PRINT file [ options ]\n\
  Prints the local file on a local printer with the given options.  Also see\n\
  HELP SET PRINTER."));
#else
    return(hmsg("Syntax: PRINT file\n\
  Prints the local file on a local printer.  Also see HELP SET PRINTER."));
#endif /* UNIX */
#endif /* VMS */
#endif /* NOFRILLS */

case XXPWD:
    return(hmsg("Syntax: PWD\n\
Print the name of the current working directory."));

#ifndef NOSPL
case XXREA:
    return(hmsg("Syntax: READ variablename\n\
  Reads a line from the currently open READ or !READ file into the variable\n\
  (see OPEN)."));
#endif /* NOSPL */

#ifndef NOXFER
case XXREC:
    return(hmsga(hmxxrc));

case XXREM:
    y = cmkey(remcmd,nrmt,"Remote command","",xxstring);
    return(dohrmt(y));
#endif /* NOXFER */

#ifndef NOSPL
case XXRET:
    return(hmsg("Syntax: RETURN [ value ]\n\
  Return from a macro.  An optional return value can be given for use with\n\
  \\fexecute(macro), which allows macros to be used like functions."));
#endif /* NOSPL */

#ifndef NOXFER
case XXSEN:
    return(hmsga(hmxxsen));
case XXMOVE:
    return(hmsg("MOVE is exactly like SEND, except each file that is\n\
sent successfully is deleted after it is sent."));
#ifndef NORESEND
case XXRSEN:
    return(hmsg(hmxxrsen));
case XXREGET:
    return(hmsg(hmxxrget));
case XXPSEN:
    return(hmsg(hmxxpsen));
#endif /* NORESEND */

#ifndef NOSERVER
case XXSER:
    return(hmsg(hmxxser));
#endif /* NOSERVER */
#endif /* NOXFER */

#ifndef NOJC
case XXSUS:
    return(hmsg("Syntax: SUSPEND or Z\n\
  Suspends Kermit.  Continue Kermit with the appropriate system command,\n\
  such as fg."));
#endif /* NOJC */

case XXSET:
    y = cmkey(prmtab,nprm,"Parameter","",xxstring);
    debug(F101,"HELP SET y","",y);
    return(dohset(y));

#ifndef NOPUSH
case XXSHE:
    if (nopush) {
        if ((x = cmcfm()) < 0) return(x);
        printf("Sorry, help not available for \"%s\"\n",cmdbuf);
        break;
    } else
       return(hmsga(hxxshe));
#ifdef CK_REDIR
case XXFUN:
    return(hmsg("Syntax: REDIRECT command\n\
  Runs the given local command with its standard input and output redirected\n\
  to the current SET LINE or SET HOST communications path.\n\
  Synonym: < (Left angle bracket)."));
#endif /* CK_REDIR */

#ifdef CK_REXX
case XXREXX:
    return(hmsg("Syntax: REXX text\n\
  The text is a Rexx command to be executed. The \\v(rexx) variable is set\n\
  to the Rexx command's return value.\n\
  To execute a rexx program file, use:  REXX call <filename>\n\
  Rexx programs may call C-Kermit functions by placing the C-Kermit command\n\
  in single quotes.  For instance:  'set parity none'."));
#endif /* CK_REXX */
#endif /* NOPUSH */

#ifndef NOSHOW
case XXSHO:
    return(hmsg("\
  Display current values of various items (SET parameters, variables, etc).\n\
  Type SHOW ? for a list of categories."));
#endif /* NOSHOW */

case XXSPA:
#ifdef datageneral
    return(hmsg("\
  Display disk usage in current device, directory,\n\
  or return space for a specified device, directory."));
#else
    return(hmsg("Syntax: SPACE\n\
  Display disk usage in current device and/or directory"));
#endif

case XXSTA:
    return(hmsg("Syntax: STATISTICS [/BRIEF]\n\
  Display statistics about most recent file transfer"));

#ifndef NOSPL
case XXSTO:
    return(hmsg("Syntax: STOP [ number [ message ] ]\n\
  Stop executing the current macro or TAKE file and return immediately to\n\
  the C-Kermit prompt.  Number is a return code.  Message printed if given."));
#endif /* NOSPL */

case XXTAK:
    return(hmsg("Syntax: TAKE filename\n\
  Take Kermit commands from the named file.  Kermit command files may\n\
  themselves contain TAKE commands, up to a reasonable depth of nesting."));

#ifdef TCPSOCKET
#ifdef TNCODE
case XXTEL:
    return(hmsga(hmxxtel));

case XXTELOP:
    return(hmsga(hxtopt));
#endif /* TNCODE */

#ifdef RLOGCODE
case XXRLOG:
    return(hmsg("Syntax: RLOGIN [ switches ] [ host [ username ] ]\n\
  Equivalent to SET NETWORK TYPE TCP/IP, SET HOST host [ service ] /RLOGIN,\n\
  IF SUCCESS CONNECT.  If host is omitted, the previous connection (if any)\n\
  is resumed.  Depending on how Kermit has been built switches may be\n\
  available to require Kerberos authentication and DES encryption."));
#endif /* RLOGCODE */
#endif /* TCPSOCKET */

#ifndef NOXMIT
case XXTRA:
    return(hmsga(hxxxmit));
#endif /* NOXMIT */

#ifndef NOFRILLS
case XXTYP:
    return(hmsga(hmxxtyp));
#endif /* NOFRILLS */

#ifndef NOSPL
case XXWHI:
    return(hmsga(whihlp));

case XXSWIT:
    return(hmsga(swihlp));
#endif /* NOSPL */

#ifndef NOCSETS
case XXXLA:
    return(hmsga(hxxxla));
#endif /* NOCSETS */

case XXVER:
    return(hmsg("Syntax: VERSION\nDisplays the program version number."));

#ifndef NOSPL
case XXWAI:
    return(hmsga(hxxwai));
#endif /* NOSPL */

#ifndef NOFRILLS
case XXWHO:
    return(hmsg("Syntax: WHO [ user ]\nDisplays info about the user."));

case XXWRI:
    return(hmsga(hxxwri));

case XXWRL:
    return(hmsg(
"WRITE-LINE (WRITELN) is just like WRITE, but includes a line terminator\n\
at the end of text.  See WRITE."));
#endif /* NOFRILLS */

#ifndef NOSPL
case XXIFX:
    return(hmsga(ifxhlp));

case XXGETC:                            /* GETC */
    return(hmsga(hxxgetc));

case XXFWD:                             /* FORWARD */
    return(hmsg(
"Like GOTO, but searches only forward for the label.  See GOTO."));

case XXLOCAL:                           /* LOCAL */
    return(hmsg(
"Declares a variable to be local to the current macro or command file."));
#endif /* NOSPL */

case XXVIEW:
    return(hmsg(
"View the terminal emulation screen even when there is no connection."));

case XXASC:
    return(hmsg("Synonym for SET FILE TYPE TEXT."));

case XXBIN:
    return(hmsg("Synonym for SET FILE TYPE BINARY."));

case XXDATE:
    return(hmsga(hmxxdate));

case XXRETR:
    return(hmsg(
"Just like GET but asks the server to delete each file that has been\n\
sent successfully."));

case XXEIGHT:
    return(hmsg(
"Equivalent to SET PARITY NONE, SET COMMAND BYTE 8, SET TERMINAL BYTE 8."));

case XXSAVE:
    return(hmsg("Syntax: SAVE KEYMAP <file>\n\
Saves current keymap definitions to file, \"keymap.ini\" by default."));

#ifndef NOFRILLS
#ifndef NOPUSH
case XXEDIT:
    return(hmsg("Syntax: EDIT [ <file> ]\n\
Starts your preferred editor on the given file, or if none given, the most\n\
recently edited file, if any.  Also see SET EDITOR."));
#endif /* NOPUSH */
#endif /* NOFRILLS */

#ifdef BROWSER
case XXBROWS:
    return(hmsg("Syntax: BROWSE [ <url> ]\n\
Starts your preferred Web browser on the given URL, or if none given, the\n\
most recently visited URL, if any.  Also see SET BROWSER."));
#endif /* BROWSER */

#ifdef CK_TAPI
case XXTAPI:
    return(hmsga(hxxtapi));
#endif /* CK_TAPI */

#ifdef PIPESEND
case XXCSEN:
    return(hmsg("Syntax: CSEND [ switches ] <command> [ <as-name> ]\n\
Sends from the given <command> rather than from a file.  Equivalent to\n\
SEND /COMMAND; see HELP SEND for details."));

case XXCREC:
    return(hmsg("Syntax: CRECEIVE [ switches ] <command>\n\
Receives to the given <command> rather than to a file.  Equivalent to\n\
RECEIVE /COMMAND; see HELP RECEIVE for details."));

case XXCGET:
    return(hmsg("Syntax: CGET <remote-file-or-command> <local-command>\n\
Equivalent to GET /COMMAND; see HELP GET for details."));
#endif /* PIPESEND */

#ifndef NOSPL
case XXFUNC:
/*
  Tricky parsing.  We want to let them type the function name in any format
  at all: \fblah(), \fblah, \\fblah(), fblah, blah, blah(), etc, but of course
  only one of these is recognized by cmkey().  So we call cmkeyx() (the "no
  complaints" version of cmkey()), and if it fails, we try the other formats
  silently, and still allow for <no-name-given>, editing and reparse, etc.
*/
    y = cmkeyx(fnctab,nfuncs,"Name of function","",NULL);
    if (y == -1) {			/* Reparse needed */
	return(y);
    } else if (y == -3) {
	if ((x = cmcfm()) < 0)		/* For recall buffer... */
	  return(x);
        return(dohfunc(y));		/* -3 gives general message */
    }
    if (y < 0) {			/* Something given but didn't match */
        int dummy;
	char * p;
	for (p = atmbuf; *p; p++) {	/* Chop off trailing parens if any */
	    if (*p == '(') {
		*p = NUL;
		break;
	    }
	}
	/* Chop off leading "\\f" or "\f" or "f" */
	p = atmbuf;
	if (*p == CMDQ)			/* Allow for \\f... */
	  p++;
	if (*p == CMDQ && (*(p+1) == 'f' || *(p+1) == 'F')) { /* or \f */
	    p += 2;
	} else if (*p == 'f' || *p == 'F') { /* or just f */
	    p++;
	}
	y = lookup(fnctab,p,nfuncs,&dummy); /* Look up the result */
    }
    if (y < 0) {
	printf("?No such function - \"%s\"\n",atmbuf);
	return(-9);
    }
    x = cmgbrk();			/* Find out how user terminated */
    if (x == LF || x == CR)		/* if with CR or LF */
      cmflgs = 1;			/* restore cmflgs to say so */
    if ((x = cmcfm()) < 0)		/* And THEN confirm so command will */
      return(x);			/* get into recall buffer. */
    return(dohfunc(y));
#endif /* NOSPL */

#ifndef NOCMDL
case XXOPTS:                            /* Command-line options */
    return(dohopts());

case XXXOPTS:                           /* Extended command-line options */
    return(doxopts());
#endif /* NOCMDL */

#ifdef OS2
#ifndef NOKVERBS
case XXKVRB:
    y = cmkey(kverbs,nkverbs,"Name of keyboard verb without \\k","",xxstring);
    return(dohkverb(y));
#endif /* NOKVERBS */
#endif /* OS2 */

case XXKERMI:
    return(hmsg("Syntax: KERMIT [command-line-options]\n\
  Lets you give command-line options at the prompt or in a script.\n\
  HELP OPTIONS for more info."));

case XXBACK:
    return(hmsg("Syntax: BACK\n  Returns to your previous directory."));

case XXWHERE:
    return(hmsg("Syntax: WHERE\n  Tells where your transferred files went."));

#ifndef NOXFER
case XXREMV:
    return(hmsga(hmxxremv));
#endif /* NOXFER */

#ifdef CK_KERBEROS
case XXAUTH:
    return(hmsga(hmxxauth));
#endif /* CK_KERBEROS */

#ifndef NOHTTP
case XXHTTP:
    return(hmsga(hmxxhttp));
#endif /* NOHTTP */

#ifdef NETCMD
case XXPIPE:
    return(hmsg("Syntax: PIPE [ command ]\n\
Makes a connection through the program whose command line is given. Example:\n\
\n pipe rlogin xyzcorp.com"));
#endif /* NETCMD */

case XXSTATUS:
    return(hmsg(
"STATUS is the same as SHOW STATUS; prints SUCCESS or FAILURE for the\n\
previous command."));

#ifndef NOSPL
case XXASSER:
    return(hmsg("Syntax: ASSERT <condition>\n\
Succeeds or fails depending on <condition>; see HELP IF for <condition>s."));

case XXFAIL:
    return(hmsg("Always fails."));

case XXSUCC:
    return(hmsg("Always succeeds."));
#endif /* NOSPL */

#ifdef CK_LOGIN
case XXLOGOUT:
    return(hmsg(
"If you haved logged in to C-Kermit as an Internet Kermit server, the LOGOUT\n\
command, given at the prompt, logs you out and closes your session."));
#endif /* CK_LOGIN */

#ifndef NOIKSD
case XXIKSD:
    return(hmsg(
"The Internet Kermit Service Daemon can be started only by the system\n\
administrator.  The IKSD is documented separately."));
#endif /* NOIKSD */

case XXRESET:
    return(hmsg("Closes all open files and logs."));

#ifndef NOCSETS
case XXASSOC:
    return(hmsga(hmxxassoc));
#endif /* NOCSETS */

#ifndef NOSPL
case XXSHIFT:
    return(hmsg("Syntax: SHIFT [ n ]\n\
  Shifts \\%1..9 variables n places to the left; default n = 1."));
#endif /* NOSPL */

#ifndef NOPUSH
case XXMAN:
#ifdef UNIX
    return(hmsg("Syntax: MANUAL [ topic ]\n\
  Runs the \"man\" command on the given topic (default \"kermit\")."));
#else
#ifdef OS2
    return(hmsg("Syntax: MANUAL\n\
  Accesses the Kermit 95 HTML manual using the current browser."));
#else
    return(hmsg("Syntax: MANUAL [ topic ]\n\
  Runs the \"help\" command on the given topic (default \"kermit\")."));
#endif /* OS2 */
#endif /* UNIX */
#endif /* NOPUSH */

case XXWILD:
    return(hmsga(hmxxwild));

#ifndef NOXFER
case XXFAST:
case XXCAU:
case XXROB:
    return(hmsga(hmxxfast));
#endif /* NOXFER */

#ifdef CKPURGE
case XXPURGE:
    return(hmsga(hmxxpurge));
#else
#ifdef VMS
case XXPURGE:
    return(hmsga(hmxxpurge));
#endif /* VMS */
#endif /* CKPURGE */

#ifndef NOXFER
  case XXRASG:
    return(hmsg("RASG and RASSIGN are short forms of REMOTE ASSIGN."));
  case XXRCWD:
    return(hmsg("RCD and RCWD are short forms of REMOTE CD."));
  case XXRCPY:
    return(hmsg("RCOPY is a short form of REMOTE COPY."));
  case XXRDEL:
    return(hmsg("RDELETE is a short form of REMOTE RELETE."));
  case XXRDIR:
    return(hmsg("RDIRECTORY is a short form of REMOTE DIRECTORY."));
  case XXRXIT:
    return(hmsg("REXIT is a short form of REMOTE EXIT."));
  case XXRHLP:
    return(hmsg("RHELP is a short form of REMOTE HELP."));
  case XXRHOS:
    return(hmsg("RHOST is a short form of REMOTE HOST."));
  case XXRKER:
    return(hmsg("RKERMIT is a short form of REMOTE KERMIT."));
  case XXRMKD:
    return(hmsg("RMKDIR is a short form of REMOTE MKDIR."));
  case XXRPRI:
    return(hmsg("RPRINT is a short form of REMOTE PRINT."));
  case XXRPWD:
    return(hmsg("RPWD is a short form of REMOTE PWD."));
  case XXRQUE:
    return(hmsg("QUERY and RQUERY are short forms of REMOTE QUERY."));
  case XXRREN:
    return(hmsg("RRENAME is a short form of REMOTE RENAME."));
  case XXRRMD:
    return(hmsg("RRMDIR is a short form of REMOTE RMDIR."));
  case XXRSET:
    return(hmsg("RSET is a short form of REMOTE SET."));
  case XXRSPA:
    return(hmsg("RSPACE is a short form of REMOTE SPACE."));
  case XXRTYP:
    return(hmsg("RTYPE is a short form of REMOTE TYPE."));
  case XXRWHO:
    return(hmsg("RWHO is a short form of REMOTE WHO."));
#endif /* NOXFER */

  case XXSCRN:
    return(hmsga(hmxxscrn));

#ifdef CKEXEC
  case XXEXEC:
    return(hmsg("Syntax: EXEC <command> [ <arg1> [ <arg2> [ ... ] ]\n\
  C-Kermit overlays itself with the given system command and starts it with\n\
  the given arguments.  Upon any error, control returns to C-Kermit."));
#endif /* CKEXEC */

#ifndef NOSPL
  case XXTRACE:
    return(hmsg(
"Syntax: TRACE { /ON, /OFF } { ASSIGNMENTS, COMMAND-LEVEL, ALL }\n\
  Turns tracing of the given object on or off."));
#endif /* NOSPL */

  case XXNOTAV:
    return(hmsg(" This command is not configured in this version of Kermit."));

default: {
        char *s;
        if ((x = cmcfm()) < 0) return(x);
        s = cmdbuf + (int)strlen(cmdbuf) -1;
        while (s >= cmdbuf && *s == SP)
          *s-- = NUL;
        while (s >= cmdbuf && *s != SP)
          s--;
        while (*s == SP) s++;
        printf("Sorry, help not available for \"%s\"\n",s);
        break;
      }
    } /* switch */
#endif /* NOHELP */

    return(success = 0);
}

/*  H M S G  --  Get confirmation, then print the given message  */

int
hmsg(s) char *s; {
    int x;
    if ((x = cmcfm()) < 0) return(x);
    printf("\n%s\n\n",s);
    return(0);
}

#ifdef NOHELP

int                                     /* Print an array of lines, */
hmsga(s) char *s[]; {                   /* cheap version. */
    int i;
    if ((i = cmcfm()) < 0) return(i);
    printf("\n");                       /* Start off with a blank line */
    for (i = 0; *s[i]; i++) {           /* Print each line. */
        printf("%s\n",s[i]);
    }
    printf("\n");
    return(0);
}

#else /* NOHELP not defined... */

int                                     /* Print an array of lines, */
hmsga(s) char *s[]; {                   /* pausing at end of each screen. */
    extern int hmtopline;		/* (This should be a parameter...) */
    int x, y, i, j, k, n;
    if ((x = cmcfm()) < 0) return(x);

#ifdef CK_TTGWSIZ
#ifdef OS2
    ttgcwsz();
#else /* OS2 */
    /* Check whether window size changed */
    if (ttgwsiz() > 0) {
        if (tt_rows > 0 && tt_cols > 0) {
            cmd_rows = tt_rows;
            cmd_cols = tt_cols;
        }
    }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

    printf("\n");                       /* Start off with a blank line */
    n = (hmtopline > 0) ? hmtopline : 1; /* Line counter */
    for (i = 0; *s[i]; i++) {
        printf("%s\n",s[i]);            /* Print a line. */
        y = (int)strlen(s[i]);
        k = 1;
        for (j = 0; j < y; j++)         /* See how many newlines were */
          if (s[i][j] == '\n') k++;     /* in the string... */
        n += k;
        if (n > (cmd_rows - 3) && *s[i+1]) /* After a screenful, give them */
          if (!askmore()) return(0);    /* a "more?" prompt. */
          else n = 0;
    }
    printf("\n");
    return(0);
}

#ifndef NOXMIT
static char *hsetxmit[] = {
"Syntax: SET TRANSMIT parameter value",
" ",
"Controls the behavior of the TRANSMIT command (see HELP TRANSMIT):",
" ",
"SET TRANSMIT ECHO { ON, OFF }",
"  Whether to echo text to your screen as it is being transmitted.",
" ",
"SET TRANSMIT EOF text",
"  Text to send after end of file is reached, e.g. \\4 for Ctrl-D",
" ",
"SET TRANSMIT FILL number",
"  ASCII value of a character to insert into blank lines, 0 for none.",
"  Applies only to text mode.  0 by default.",
" ",
"SET TRANSMIT LINEFEED { ON, OFF }",
"  Transmit Linefeed as well as Carriage Return (CR) at the end of each line.",
"  Normally, only CR  is sent.",
" ",
"SET TRANSMIT LOCKING-SHIFT { ON, OFF }",
"  Whether to use SO/SI for transmitting 8-bit data when PARITY is not NONE.",
" ",
"SET TRANSMIT PAUSE number",
"  How many milliseconds to pause after transmitting each line (text mode),",
"  or each character (binary mode).",
" ",
"SET TRANSMIT PROMPT number",
"  ASCII value of character to look for from host before sending next line",
"  when TRANSMITting in text mode; normally 10 (Linefeed).  0 means none;",
"  don't wait for a prompt.",
" ",
"SET TRANSMIT TIMEOUT number",
"  Number of seconds to wait for each character to echo when TRANSMIT ECHO",
"  is ON or TRANSMIT PROMPT is not 0.  If 0 is specified, this means wait",
"  indefinitely for each echo.",
" ",
"Synonym: SET XMIT.  SHOW TRANSMIT displays current settings.",
"" };
#endif /* NOXMIT */

static char *hsetbkg[] = {
"Syntax: SET BACKGROUND { OFF, ON }",
" ",
"  SET BACKGROUND OFF forces prompts and messages to appear on your screen",
"  even though Kermit thinks it is running in the background.",
"" };

#ifdef DYNAMIC
static char *hsetbuf[] = {
"Syntax: SET BUFFERS n1 [ n2 ]",
" ",
"  Changes the overall amount of memory allocated for SEND and RECEIVE packet",
"  buffers, respectively.  Bigger numbers let you have longer packets and",
"  more window slots.  If n2 is omitted, the same value as n1 is used.",
#ifdef BIGBUFOK
" ",
"  NOTE: This command is not needed in this version of C-Kermit, which is",
"  already configured for maximum-size packet buffers.",
#endif /* BIGBUFOK */
"" };
#endif /* DYNAMIC */

static char *hsetcmd[] = {
"Syntax: SET COMMAND parameter value",
" ",

#ifdef CK_AUTODL
"SET COMMAND AUTODOWNLOAD { ON, OFF }",
"  Enables/Disables automatic recognition of Kermit packets while in",
"  command mode.  ON by default.",
" ",
#endif /* CK_AUTODL */

"SET COMMAND BYTESIZE { 7, 8 }",
"  Informs C-Kermit of the bytesize of the communication path between itself",
"  and your keyboard and screen.  7 is assumed.  SET COMMAND BYTE 8 to allow",
"  entry and display of 8-bit characters.",
" ",

#ifdef OS2
"SET COMMAND COLOR <foreground-color> <background-color>",
"  Lets you choose colors for Command screen.  Use ? in the color fields to",
"  to get lists of available colors.",
" ",
"SET COMMAND CURSOR-POSITION <row> <column>",
"  Moves the command-screen cursor to the given position (1-based).  This",
"  command should be used in scripts instead of relying on ANSI.SYS escape",
"  sequences.",
" ",
#endif /* OS2 */

"SET COMMAND HEIGHT <number>",
"  Informs C-Kermit of the number of rows in your command screen for the",
"  purposes of More?-prompting.",
" ",
"SET COMMAND WIDTH <number>",
"  Informs C-Kermit of the number of characters across your screen for",
"  purposes of screen formatting.",
" ",
"SET COMMAND MORE-PROMPTING { ON, OFF }",
"  ON (the default) enables More?-prompting when C-Kermit needs to display",
"  text that does not fit vertically on your screen.  OFF allows the text to",
"  scroll by without intervention.  If your command window has scroll bars,",
"  you might prefer OFF.",
" ",

#ifdef CK_RECALL
"SET COMMAND RECALL-BUFFER-SIZE number",
"  How big you want C-Kermit's command recall buffer to be.  By default, it",
"  holds 10 commands.  You can make it any size you like, subject to memory",
"  constraints of the computer.  A size of 0 disables command recall.",
"  Whenever you give this command, previous command history is lost.",
" ",
#endif /* CK_RECALL */

"SET COMMAND QUOTING { ON, OFF }",
"  Whether to treat backslash and question mark as special characters (ON),",
"  or as ordinary data characters (OFF) in commands.  ON by default.",
" ",

#ifdef CK_RECALL
"SET COMMAND RETRY { ON, OFF }",
"  Whether to reprompt you with the correct but incomplete portion of a",
"  syntactically incorrect command.  ON by default.",
" ",
#endif /* CK_RECALL */

#ifdef OS2
"SET COMMAND SCROLLBACK <lines>",
"  Sets size of virtual Command screen buffer to the given number of lines,",
"  which includes the active Command screen.  The minimum is 256.  The max",
"  is 2 million.  The default is 512.",
" ",
#endif /* OS2 */

"Use SHOW COMMAND to display these settings.",
"" };

#ifndef NOLOCAL
static char *hsetcar[] = {
"Syntax: SET CARRIER-WATCH { AUTO, OFF, ON }",
" ",
"  Attempts to control treatment of carrier (the Data Carrier Detect signal)",
"  on serial communication (SET LINE or SET PORT) devices.  ON means that",
"  carrier is required at all times.  OFF means carrier is never required.",
"  AUTO (the default) means carrier is required at all times except during",
"  the DIAL command.  Correct operation of carrier-watch depends on the",
"  capabilities of the underlying OS, drivers, devices, and cables.  If you",
"  need to CONNECT to a serial device that is not asserting carrier, and",
"  Kermit won't let you, use SET CARRIER-WATCH OFF.  Use SHOW COMMUNICATIONS",
"  to display the CARRIER-WATCH setting.",
"" };
#endif /* NOLOCAL */

static char *hsetat[] = {
"Syntax: SET ATTRIBUTES name ON or OFF",
" ",
"  Use this command to enable (ON) or disable (OFF) the transmission of",
"  selected file attributes along with each file, and to handle or ignore",
"  selected incoming file attributes, including:",
" ",
#ifndef NOCSETS
"   CHARACTER-SET:  The transfer character set for text files",
#endif /* NOCSETS */
"   DATE:           The file's creation date",
"   DISPOSITION:    Unusual things to do with the file, like MAIL or PRINT",
"   LENGTH:         The file's length",
"   PROTECTION:     The files protection (permissions)",
"   SYSTEM-ID:      Machine/Operating system of origin",
"   TYPE:           The file's type (text or binary)",
" ",
"You can also specify ALL to select all of them.  Examples:",
" ",
"   SET ATTR DATE OFF",
"   SET ATTR LENGTH ON",
"   SET ATTR ALL OFF",
""
};

static char *hxytak[] = {
"Syntax: SET TAKE parameter value",
" ",
"  Controls behavior of TAKE command:",
" ",
"SET TAKE ECHO { ON, OFF }",
"  Tells whether commands read from a TAKE file should be displayed on the",
"  screen (if so, each command is shown at the time it is read, and labeled",
"  with a line number).",
" ",
"SET TAKE ERROR { ON, OFF }",
"  Tells whether a TAKE command file should be automatically terminated when",
"  a command fails.  This setting is local to the current command file, and",
"  inherited by subordinate command files.",
"" };

#ifndef NOLOCAL
#ifdef OS2MOUSE
static char *hxymouse[] = {
"Syntax: SET MOUSE ACTIVATE { ON, OFF }",
"  Enables or disables the mouse in Connect mode.  Default is ON",
" ",
"Syntax: SET MOUSE BUTTON <number> <key-modifier> <action> [ <text> ]",
" where:",
"  <number> is the mouse button number, 1, 2, or 3;",
"  <key-modifier> denotes modifier keys held down during the mouse event:",
"   ALT, ALT-SHIFT, CTRL, CTRL-ALT CTRL-ALT-SHIFT, CTRL-SHIFT, SHIFT, NONE;",
"  <action> is the mouse action, CLICK, DRAG, or DOUBLE-CLICK.",
" ",
" The <text> has exactly the same properties as the <text> from the SET KEY",
" command -- it can be a character, a string, one or more Kverbs, a macro",
" invoked as a Kverb, or any combination of these.  Thus, anything that can",
" be assigned to a key can also be assigned to the mouse -- and vice versa.",
" If the <text> is omitted, the action will be ignored.  Examples:",
" ",
" SET MOUSE BUTTON 1 NONE DOUBLE \\KmouseCurPos",
" SET MOU B 2 SHIFT CLICK help\\13",
" ",
" DRAG operations perform a \"mark mode\" selection of Text. You should",
" assign only the following actions to drag operations:",
" ",
"  \\Kdump         - copy marked text to printer (or file)",
"  \\Kmarkcopyclip - copy marked text to PM Clipboard",
"  \\Kmarkcopyhost - copy marked text direct to Host",
"  \\Kmousemark    - mark text, no copy operation performed",
" ",
" The following Kverb is only for use with the mouse:",
" ",
"  \\KmouseCurPos",
" ",
" which represents the mouse-directed terminal cursor feature.",
" ",
"Syntax: SET MOUSE CLEAR",
" Restores all mouse events to their default definitions",
"  Button 1 Double-Click = Kverb: \\Kmousecurpos",
"  Button 1 Drag         = Kverb: \\Kmarkcopyclip",
"  Button 1 Ctrl Drag    = Kverb: \\Kmarkcopyhost",
"  Button 2 Double-Click = Kverb: \\Kpaste",
""};
#endif /* OS2MOUSE */

static char *hxyterm[] = {
"Syntax: SET TERMINAL parameter value",
" ",
#ifdef OS2
"SET TERMINAL TYPE { ANSI, VT52, VT100, VT102, VT220, VT320, ... }",
"  Selects type type of terminal to emulate.  Type SET TERMINAL TYPE ? to",
"  see a complete list.",
" ",
"SET TERMINAL ANSWERBACK { OFF, ON }",
"  Disables/enables the ENQ/Answerback sequence (\"K-95 version term-type\").",
" ",
"SET TERMINAL ANSWERBACK MESSAGE <extension>",
"  Allows you to specify an extension to the default answerback message.",
" ",
#else
"SET TERMINAL TYPE ...",
"  This command is not available because this version of C-Kermit does not",
"  include a terminal emulator.  Instead, it is a \"semitransparent pipe\"",
"  (or a totally transparent one, if you configure it that way) to the",
"  computer or service you have made a connection to.  Your console,",
"  workstation window, or the terminal emulator or terminal from which you",
"  are running C-Kermit provides the emulation.",
" ",
#endif /* OS2 */

#ifdef CK_APC
"SET TERMINAL APC { ON, OFF, UNCHECKED }",
#ifdef OS2
"  Controls execution of Application Program Commands sent by the host while",
"  K-95 is either in CONNECT mode or processing INPUT commands.  ON allows",
"  execution of \"safe\" commands and disallows potentially dangerous ones",
"  such as DELETE, RENAME, OUTPUT, and RUN.  OFF prevents execution of APCs.",
"  UNCHECKED allows execution of all APCs.  OFF is the default.",
#else /* OS2 */
"  Controls execution of Application Program Commands sent by the host while",
"  C-Kermit is in CONNECT mode.  ON allows execution of \"safe\" commands and",
"  disallows potentially dangerous commands such as DELETE, RENAME, OUTPUT,",
"  and RUN.  OFF prevents execution of APCs.  UNCHECKED allows execution of",
"  all APCs.  OFF is the default.",
#endif /* OS2 */
" ",
#endif /* CK_APC */

#ifdef OS2
"SET TERMINAL ARROW-KEYS { APPLICATION, CURSOR }",
"  Sets the mode for the arrow keys during VT terminal emulation.",
" ",
"SET TERMINAL ATTRIBUTE { BLINK, PROTECTED, REVERSE, UNDERLINE }",
"  Determines how attributes are displayed by Kermit-95.",
" ",
"SET TERMINAL ATTRIBUTE { BLINK, REVERSE, UNDERLINE } { ON, OFF }",
"  Determines whether real Blinking, Reverse, and Underline are used in the",
"  terminal display.  When BLINK is turned OFF, reverse background intensity",
"  is used.  When REVERSE and UNDERLINE are OFF, the colors selected with SET",
"  TERMINAL COLOR { REVERSE,UNDERLINE } are used instead.  This command",
"  affects the entire current screen and terminal scrollback buffer.",
" ",
"SET TERMINAL ATTRIBUTE PROTECTED [ -",
"   { BOLD, DIM, INVISIBLE, NORMAL, REVERSE, UNDERLINED } ]",
"  Sets the attributes used to represent Protected text in Wyse and Televideo",
"  terminal emulations.  Any combination of attributes may be used.  The",
"  default is DIM.)",
" ",
#endif /* OS2 */

#ifdef OS2
#ifdef CK_XYZ
"SET TERMINAL AUTODOWNLOAD { ON, OFF }",
"  enables/disables automatic switching into file-transfer mode when a Kermit",
"  or ZMODEM file transfer has been detected during CONNECT mode or while",
"  an INPUT command is active.  Default is OFF.",
" ",
#else
"SET TERMINAL AUTODOWNLOAD { ON, OFF }",
"  enables/disables automatic switching into file-transfer mode when a Kermit",
"  file transfer has been detected during CONNECT mode or while an INPUT",
"  command is active.  Default is OFF.",
" ",
#endif /* CK_XYZ */
#ifdef CK_XYZ
"SET TERM... AUTO... { KERMIT, ZMODEM } C0-CONFLICTS { IGNORED, PROCESSED }",
"  Determines whether the active terminal emulator should process or ignore",
"  C0 control characters which are also used for the specified file transfer",
"  protocol.  Kermit by default uses ^A (SOH) and Zmodem uses ^X (CAN).",
"  Default is PROCESSED.",
" ",
"SET TERM... AUTO... { KERMIT, ZMODEM } DETECTION-METHOD { PACKET, STRING }",
"  Determines whether the specified file transfer protocol should be detected",
"  by looking for valid packets or by identifying a specified text string.",
"  Default is PACKET.",
" ",
"SET TERM... AUTO... { KERMIT, ZMODEM } STRING <text>",
"  Lets you assign an autodownload detection string for use with the",
"  specified file transfer protocol.",
"  Default for Kermit is \"READY TO SEND...\", for Zmodem is \"rz\\{13}\".",
" ",
#else /* CK_XYZ */
"SET TERM... AUTO... KERMIT C0-CONFLICTS { IGNORED, PROCESSED }",
"  Determines whether the active terminal emulator should process or ignore",
"  C0 control characters which are also used for the specified file transfer",
"  protocol.  Kermit by default uses ^A <SOH>.  Default is PROCESSED.",
" ",
"SET TERM... AUTO... KERMIT DETECTION-METHOD { PACKET, STRING }",
"  Determines whether the specified file transfer protocol should be detected",
"  by looking for valid packets or by identifying a specified text string.",
"  Default is PACKET.",
" ",
"SET TERM... AUTO... KERMIT STRING <text>",
"  Lets you assign an autodownload detection string for use with the",
"  specified file transfer protocol.  Default is \"READY TO SEND...\".",
" ",
#endif /* CK_XYZ */
"SET TERMINAL AUTOPAGE { ON, OFF }",
" ",
"SET TERMINAL AUTOSCROLL { ON, OFF }",
" ",
#else /* OS2 */
"SET TERMINAL AUTODOWNLOAD { ON, OFF }",
"  Enables/disables automatic switching into file-transfer mode when a valid",
#ifdef CK_XYZ
"  Kermit or ZMODEM packet of the appropriate type is received during CONNECT",
"  mode.  Default is OFF.",
" ",
#else
"  Kermit packet of the appropriate type is received during CONNECT mode.",
"  Default is OFF.",
" ",
#endif /* CK_XYZ */
#endif /* OS2 */

#ifdef OS2
"SET TERMINAL BELL { AUDIBLE, VISIBLE, NONE }",
"  Specifies how Control-G (bell) characters are handled.  AUDIBLE means",
"  a beep is sounded; VISIBLE means the screen is flashed momentarily.",
" ",
"  (This command has been superseded by SET BELL.)",
" ",
#endif /* OS2 */

"SET TERMINAL BYTESIZE { 7, 8 }",
#ifdef OS2
"  Use 7- or 8-bit terminal characters between K-95 and the remote",
#else
"  Use 7- or 8-bit terminal characters between C-Kermit and the remote",
#endif /* OS2 */
"  computer or service during CONNECT.",
" ",

#ifndef NOCSETS
#ifdef OS2
"SET TERMINAL CHARACTER-SET",
"  (See SET TERMINAL { LOCAL-CHARACTER-SET, REMOTE-CHARACTER-SET })",
" ",
#else  /* not OS2 */
"SET TERMINAL CHARACTER-SET <remote-cs> [ <local-cs> ]",
"  Specifies the character set used by the remote host, <remote-cs>, and the",
"  character set used by C-Kermit locally, <local-cs>.  If you don't specify",
"  the local character set, the current FILE CHARACTER-SET is used.  When",
"  you specify two different character sets, C-Kermit translates between them",
"  during CONNECT.  By default, both character sets are TRANSPARENT, and",
"  no translation is done.",
" ",
#endif /* OS2 */
#endif /* NOCSETS */

#ifdef OS2

"SET TERMINAL CODE-PAGE <number>",
"  Lets you change the PC code page.  Only works for code pages that are",
"  successfully prepared in CONFIG.SYS.  Use SHOW TERMINAL to list the",
"  current code page and the available code pages.",
#ifdef OS2ONLY
" ",
"  Also see SET TERMINAL FONT if the desired code page in not available in",
"  your version of OS/2.",
#endif /* OS2ONLY */
" ",

#ifndef NT
"SET TERMINAL COLOR BORDER <foreground>",
#endif /* NT */
"SET TERMINAL COLOR <screenpart> <foreground> <background>",
" Sets the colors of the terminal emulation screen.",
" <screenpart> may be any of the following:",
"  DEBUG, HELP-TEXT, REVERSE, SELECTION, STATUS-LINE, TERMINAL-SCREEN, or",
"  UNDERLINED-TEXT.",
" <foreground> and <background> may be any of:",
"  BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LGRAY, DGRAY, LBLUE,",
"  LGREEN, LCYAN, LRED, LMAGENTA, YELLOW or WHITE.",
" The L prefix for the color names means Light.",
" ",

"SET TERMINAL COLOR ERASE { CURRENT-COLOR, DEFAULT-COLOR }",
"  Determines whether the current color as set by the host or the default",
"  color as set by the user (SET TERMINAL COLOR TERMINAL) is used to clear",
"  the screen when erase commands are received from the host.",
" ",

"SET TERMINAL COLOR RESET-ON-ESC[0m { CURRENT-COLOR, DEFAULT-COLOR }",
"  Determines whether the current color or the default color is used after",
"  <ESC>[0m (\"reset attributes\") command sequence is received from the",
"  host.",
" ",

"SET TERMINAL CONTROLS { 7, 8 }",
"  Determines whether VT220/320 or Wyse 370 function keys, arrow keys, etc,",
"  that generate ANSI-format escape sequences should send 8-bit control",
"  characters or 7-bit escape sequences.",
" ",
#endif /* OS2 */

"SET TERMINAL CR-DISPLAY { CRLF, NORMAL }",
"  Specifies how incoming carriage return characters are to be displayed",
"  on your screen.",
" ",

#ifdef OS2
"SET TERMINAL CURSOR { FULL, HALF, UNDERLINE }",
"  Selects cursor style for the terminal screen.",
" ",
"SET TERMINAL DG-UNIX-MODE { ON, OFF }",
"  Specifies whether the Data General emulations should accept control",
"  sequences in Unix compatible format or in native DG format.  The",
"  default is OFF, DG format.",
" ",
#endif /* OS2 */

"SET TERMINAL DEBUG { ON, OFF }",
"  Turns terminal session debugging on and off.  When ON, incoming control",
"  characters are displayed symbolically, rather than be taken as formatting",
"  commands.  SET TERMINAL DEBUG ON implies SET TELNET DEBUG ON.",
" ",
#ifdef OS2
"SET TERMINAL DG-UNIX-MODE { ON, OFF }",
" ",
#endif /* OS2 */

"SET TERMINAL ECHO { LOCAL, REMOTE }",
"  Specifies which side does the echoing during terminal connection.",
" ",

"SET TERMINAL ESCAPE-CHARACTER { ENABLED, DISABLED }",
"  Turns on/off the ability to escape back from CONNECT mode using the SET",
#ifdef OS2
"  ESCAPE character.  If you disable it you can still get back using Alt-key",
"  combinations as shown in the status line.  Also see HELP SET ESCAPE.",
#else
"  ESCAPE character.  If you disable it, Kermit returns to its prompt only",
"  when the connection is closed by the other end.  USE WITH EXTREME CAUTION.",
"  Also see HELP SET ESCAPE.",
#endif /* OS2 */
" ",

#ifdef OS2
#ifdef OS2ONLY
"SET TERMINAL FONT { CP437, CP850, CP852, CP862, CP866, DEFAULT }",
"  CP437 - Original PC code page",
"  CP850 - \"Multilingual\" (West Europe) code page",
"  CP852 - East Europe Roman Alphabet code page (for Czech, Polish, etc)",
"  CP862 - Hebrew code page",
"  CP866 - Cyrillic (Russian, Belorussian, and Ukrainian) code page",
" ",
"  Loads a soft into the video adapter for use during terminal emulation.",
"  Use this command when your OS/2 system does not have the desired code.",
"  page.  Can be used only in full-screen sessions.  Also see SET TERMINAL",
"  CODE-PAGE and SET TERMINAL REMOTE-CHARACTER-SET.",
" ",
#endif /* OS2ONLY */

#ifdef NT
"SET TERMINAL HEIGHT <number>",
"  Changes the number of rows (lines) to use during terminal emulation, not",
"  counting the status line.  Recommended values are 24, 42, and 49 (or 25,",
"  43, and 50 if SET TERMINAL STATUSLINE is OFF.)",
#else
"SET TERMINAL HEIGHT <number>"
"  Changes the number of rows (lines) to use during terminal emulation, not",
"  counting the status line.  Windowed sessions can use any value from 8 to",
"  101.  Fullscreen sessions are limited to 24, 42, 49, or 59.  Not all"
"  heights are supported by all video adapters.",
#endif /* NT */
#else  /* OS2 */
"SET TERMINAL HEIGHT <number>",
"  Tells C-Kermit how many rows (lines) are on your CONNECT-mode screen.",
#endif /* OS2 */
" ",

#ifdef OS2
"SET TERMINAL IDLE-SEND <seconds> <string>",
"  Specifies how many seconds without keyboard activity must pass before",
"  <string> is sent to the host when in CONNECT mode.  The default is 0",
"  seconds which disables this function.  <string> may contain Kverbs and",
"  and variables which will be evaluated at time of transmission to the host.",
" ",

"SET TERMINAL KDB-FOLLOWS-GL/GR { ON, OFF }",
" Specifies whether or not the keyboard character set should follow the",
"  active GL and GR character sets.  This feature is OFF by default and",
"  should not be used unless it is specificly required by the host",
"  application.",
" ",

"SET TERMINAL KEY <mode> /LITERAL <keycode> <text>",
"SET TERMINAL KEY <mode> DEFAULT",
"SET TERMINAL KEY <mode> CLEAR",
"  Configures the key whose <keycode> is given to send the given text when",
"  pressed while <mode> is active.  <mode> may be any of the valid terminal",
"  types or the special modes \"EMACS\", \"HEBREW\" or \"RUSSIAN\".  DEFAULT",
"  restores all default key mappings for the specified mode.  CLEAR erases",
"  all the key mappings.  If there is no text, the default key binding is",
#ifndef NOCSETS
"  restored for the key k.  SET TERMINAL KEY mappings take place before",
"  terminal character-set translation.  SET KEY mappings take precedence over",
"  SET TERMINAL KEY <terminal type> settings.",
#else
"  restored for the key.  SET KEY mappings take precedence over SET TERMINAL",
"  KEY <terminal type> settings.",
#endif /* NOCSETS */
"  The /LITERAL switch may be used to instruct Kermit to ignore character-set",
"  translations when sending this definition to the host.",
" ",
"  The text may contain \"\\Kverbs\" to denote actions, to stand for DEC",
"  keypad, function, or editing keys, etc.  For a list of available keyboard",
"  verbs, type SHOW KVERBS.",
" ",
"  To find out the keycode and mapping for a particular key, use the SHOW",
"  KEY command.  Use the SAVE KEYS command to save all settings to a file.",
" ",
"SET TERMINAL KEYBOARD-MODE { NORMAL, EMACS, RUSSIAN, HEBREW }",
"  Select a special keyboard mode for use in the terminal screen.",
" ",

"SET TERMINAL KEYPAD-MODE { APPLICATION, NUMERIC }",
"  Specifies the \"mode\" of the numeric keypad for VT terminal emulation.",
"  Use this command in case the host or application wants the keypad to be",
"  in a different mode than it's in, but did not send the escape sequence",
"  to put it in the needed mode.",
" ",
#endif /* OS2 */

#ifndef NOCSETS
#ifdef OS2
"SET TERMINAL LOCAL-CHARACTER-SET <local-cs>",
"  Specifies the character set used by K-95 locally.  If you don't specify",
#ifdef OS2ONLY
"  the local character-set, the current TERMINAL FONT is used if you have",
"  given a SET TERMINAL FONT command; otherwise the current codepage is used.",
#else
"  the local character-set, the current code page is used.",
#endif /* OS2ONLY */
" ",
"  When the local and remote character sets differ, Kermit translates between",
"  them during CONNECT.  By default, the remote character set is Latin1 and",
"  the local one is your current code page.",
#ifdef NT
" ",
"  In Windows NT, Unicode is used as the local character-set regardless of",
"  this setting.",
#endif /* NT */
" ",
"See also SET TERMINAL REMOTE-CHARACTER-SET",
" ",
#endif /* OS2 */
#endif /* NOCSETS */

#ifdef OS2
"SET TERMINAL LOCKING-SHIFT { OFF, ON }",
"  Tells whether to send Shift-In/Shift-Out (Ctrl-O and Ctrl-N) to switch",
"  between 7-bit and 8-bit characters sent during terminal emulation over",
"  7-bit connections.  OFF by default.",
#else
"SET TERMINAL LOCKING-SHIFT { OFF, ON }",
"  Tells C-Kermit whether to use Shift-In/Shift-Out (Ctrl-O and Ctrl-N) to",
"  switch between 7-bit and 8-bit characters during CONNECT.  OFF by default.",
#endif /* OS2 */
" ",

#ifdef OS2
"SET TERMINAL MARGIN-BELL { ON [column], OFF }",
"  Determines whether the margin-bell is activated and what column it should",
"  ring at.  OFF by default.",
" ",
#endif /* OS2 */

"SET TERMINAL NEWLINE-MODE { OFF, ON }",
"  Tells whether to send CRLF (Carriage Return and Line Feed) when you type",
"  CR (press the Return or Enter key) in CONNECT mode.",
" ",

#ifdef OS2
"SET TERMINAL OUTPUT-PACING <milliseconds>",
"  Tells how long to pause between sending each character to the host during",
"  CONNECT mode.  Normally not needed but sometimes required to work around",
"  TRANSMISSION BLOCKED conditions when pasting into the terminal window.",
" ",

#ifdef PCTERM
"SET TERMINAL PCTERM { ON, OFF }",
"  Activates or deactivates the PCTERM terminal emulation keyboard mode.",
"  When PCTERM is ON all keystrokes in the terminal screen are sent to the",
"  host as make/break (down/up) codes instead of as characters from the",
"  REMOTE-CHARACTER-SET, and all keyboard mappings, including Kverbs and the",
"  escape character are disabled.  To turn off PCTERM keyboard mode while in",
"  CONNECT mode press Control-CapsLock.  PCTERM is OFF by default.",
" ",
#endif /* PCTERM */
#endif /* OS2 */

#ifdef OS2
"SET TERMINAL PRINT { AUTO, COPY, OFF, TRANSPARENT }",
"  \
Allows selective control of various types of printing in the Terminal screen."
,
" ",
#else
#ifdef XPRINT
"SET TERMINAL PRINT { ON, OFF }",
"  Enables and disables host-initiated transparent printing in CONNECT mode.",
" ",
#endif /* XPRINT */
#endif /* OS2 */

#ifdef OS2
#ifndef NOCSETS
"SET TERMINAL REMOTE-CHARACTER-SET <remote-cs> [ { G0,G1,G2,G3 }... ]",
"  Specifies the character set used by the remote host, <remote-cs>.",
"  When the local and remote character sets differ, C-Kermit translates",
"  between them during CONNECT.  By default, the remote character set is",
"  Latin1 and the local one is your current code page.  Optionally, you can",
"  also designate the character set to the G0..G3 graphic tables.",
" ",
#endif /* NOCSETS */
#endif /* OS2 */

#ifdef OS2
"SET TERMINAL ROLL-MODE { INSERT, OVERWRITE }",
"  Tells whether new data when received from the host is entered into the",
"  scrollback buffer at the current rollback position (OVERWRITE) or at the",
"  end of the buffer (INSERT).  The default is INSERT.  Typing is allowed",
"  during rollbacks in either mode.",
" ",

"SET TERMINAL SCREEN-MODE { NORMAL, REVERSE }",
"  When set to REVERSE the foreground and background colors are swapped as",
"  well as the application of the foreground and background intensity bits.",
"  The default is NORMAL.",
" ",

"SET TERMINAL SCREEN-OPTIMIZE { ON, OFF }",
"  When set to ON, the default, Kermit only paints the screen with characters",
"  that have changed since the last screen paint.  When OFF, the screen is",
"  completely repainted each time there is a change.",
" ",

"SET TERMINAL SCREEN-UPDATE { FAST, SMOOTH } [ <milliseconds> ]",
"  Chooses the mechanism used for screen updating and the update frequency.",
"  Defaults are FAST scrolling with updates every 100 milliseconds.",
" ",

"SET TERMINAL SCROLLBACK <lines>",
"  Sets size of CONNECT virtual screen buffer.  <lines> includes the active",
"  terminal screen.  The minimum is 256.  The maximum is 2 million.  The",
"  default is 2000.",
" ",

"SET TERMINAL SEND-DATA { ON, OFF }",
"  Determines whether ASCII emulations such as WYSE 30,50,60 or TVI 910+,925,",
"  950 may send their screen contents to the host upon request.  Allowing the",
"  screen to be read by the host is a significant security risk.  The default",
"  is OFF and should only be changed after a security evaluation of host",
"  environment.",
" ",

"SET TERMINAL SEND-END-OF-BLOCK { CRLF_ETX, US_CR }",
"  Determines which set of characters should be used as end of line and end",
"  of transmission indicators when sending screen data to the host",
" ",

"SET TERMINAL SGR-COLORS { ON, OFF }",
"  ON (default) means allow host control of colors; OFF means ignore host",
"  escape sequences to set color.",
" ",

"SET TERMINAL SNI-CH.CODE { ON, OFF }",
"  This command controls the state of the CH.CODE key.  It is the equivalent",
"  to the SNI_CH_CODE Keyboard verb.  The SNI terminal uses CH.CODE to",
"  easily switch between the National Language character set and U.S. ASCII.",
"  The default is ON which means to display characters as U.S. ASCII.  When",
"  OFF the lanuage specified by SET TERMINAL SNI-LANUAGE is used to display",
"  characters when 7-bit character sets are in use."
" ",
"SET TERMINAL SNI-FIRMWARE-VERSIONS <kbd-version> <terminal-version>",
"  Specifies the Firmware Version number that should be reported to the host",
"  when the terminal is queried.  The default is 920031 for the keyboard",
"  and 830851 for the terminal.",
" ",
"SET TERMINAL SNI-LANGUAGE <national-language>",
"  An alias for SET TERMINAL VT-LANUAGE, this command specifies the national",
"  language character-set that should be used when the NRC mode is activated",
"  for VT emulations or when CH.CODE is OFF for SNI emulations.  The default",
"  language for SET TERMINAL TYPE SNI-97801 is \"German\".",
" ",
"SET TERMINAL SNI-PAGEMODE { ON, OFF }",
"  Determines whether or not page mode is active.  OFF by default.",
" ",
"SET TERMINAL SNI-SCROLLMODE { ON, OFF }",
"  Determines whether or not scroll mode is active.  OFF by default.",
" ",
"SET TERMINAL STATUSLINE { ON, OFF }",
"  ON (default) enables the Kermit status line in the terminal screen.",
"  OFF removes it, making the line available for use by the host.",
" ",

"SET TERMINAL TRANSMIT-TIMEOUT <seconds>",
"  Specifies the maximum amount of time K-95 waits before returning to the",
"  prompt if your keystrokes can't be transmitted for some reason, such as a",
"  flow-control deadlock.",
" ",
#endif /* OS2 */

#ifdef CK_TRIGGER
"SET TERMINAL TRIGGER <string>",
"  Specifies a string that, when detected during any subsequent CONNECT",
"  session, is to cause automatic return to command mode.  Give this command",
"  without a string to cancel the current trigger.  See HELP CONNECT for",
"  additional information.",
" ",
#endif /* CK_TRIGGER */

#ifdef OS2
"SET TERMINAL VIDEO-CHANGE { DISABLED, ENABLED }",
"  Tells whether K-95 should change video modes automatically in response",
#ifdef NT
"  to escape sequences from the other computer.  ENABLED by default (except",
"  on Windows 95).",
#else /* NT */
"  to escape sequences from the other computer.  ENABLED by default.",
#endif /* NT */
" ",

"SET TERMINAL VT-LANGUAGE <language>",
"  Specifies the National Replacement Character Set (NRC) to be used when",
"  NRC mode is activated.  The default is \"North American\".",
" ",
"SET TERMINAL VT-NRC-MODE { ON, OFF }",
"  OFF (default) chooses VT multinational Character Set mode.  OFF chooses",
"  VT National Replacement Character-set mode.  The NRC is selected with",
"  SET TERMINAL VT-LANGUAGE",
" ",

#ifdef NT
"SET TERMINAL WIDTH <cols>",
"  Tells the number of columns in the terminal screen.",
" ",
"  The default is 80.  You can also use 132.  Other widths can be chosen but",
"  are usually not supported by host software.",
" ",
#else
"SET TERMINAL WIDTH <cols>",
"  Tells how many columns define the terminal size.",
" ",
"Default is 80.  In Windowed OS/2 2.x sessions, this value may not be changed",
"In Windowed OS/2 WARP 3.x sessions, this value may range from 20 to 255.",
"In Full screen sessions, values of 40, 80, and 132 are valid.  Not all",
"combinations of height and width are supported on all adapters.",
" ",
#endif /* NT */
"SET TERMINAL WRAP { OFF, ON }",
"  Tells whether the terminal emulator should automatically wrap long lines",
"  on your screen.",
" ",
#else

"SET TERMINAL WIDTH <number>",
" \
Tells C-Kermit how many columns (characters) are on your CONNECT-mode screen.",
" ",
#endif /* OS2 */
"Type SHOW TERMINAL to see current terminal settings.",
"" };
#endif /* NOLOCAL */

#ifdef NETCONN
static char *hxyhost[] = {
"SET HOST [ switches ] hostname-or-address [ service ] [ protocol-switch ]",
"  Establishes a connection to the specified network host on the currently",
"  selected network type.  For TCP/IP connections, the default service is",
"  TELNET; specify a different TCP port number or service name to choose a",
"  different service.  The first set of switches can be:",
" ",
" /NETWORK-TYPE:name",
"   Makes the connection on the given type of network.  Equivalent to SET",
"   NETWORK TYPE name prior to SET HOST, except that the selected network",
"   type is used only for this connection.  Type \"set host /net:?\" to see",
#ifdef NETCMD
"   a list.  /NETWORK-TYPE:COMMAND means to make the connection through the",
"   given system command, such as \"rlogin\" or \"cu\".",
#else
"   a list.",
#endif /* NETCMD */
" ",
" /CONNECT",
"   \
Enter CONNECT (terminal) mode automatically if the connection is successful.",
" ",
" /SERVER",
"   Enter server mode automatically if the connection is successful.",
" ",
" /USERID:[<name>]",
"   This switch is equivalent to SET LOGIN USERID <name> or SET TELNET",
"   ENVIRONMENT USER <name>.  If a string is given, it sent to host during",
"   Telnet negotiations; if this switch is given but the string is omitted,",
"   no user ID is sent to the host.  If this switch is not given, your",
"   current USERID value, \\v(userid), is sent.  When a userid is sent to the",
"   host it is a request to login as the specified user.",
" ",
#ifdef CK_AUTHENTICATION
" /PASSWORD:[<string>]",
"   This switch is equivalent to SET LOGIN PASSWORD.  If a string is given,",
"   it is treated as the password to be used (if required) by any Telnet",
"   Authentication protocol (Kerberos Ticket retrieval, Secure Remote",
"   Password, or X.509 certificate private key decryption.)  If no password",
"   switch is specified a prompt is issued to request the password if one",
"   is required for the negotiated authentication method.",
" ",
#endif /* CK_AUTHENTICATION */
"The protocol-switches can be:",
" ",
" /NO-TELNET-INIT",
"   Do not send initial Telnet negotiations even if this is a Telnet port.",
" ",
" /RAW-SOCKET",
"   This is a connection to a raw TCP socket.",
" ",
#ifdef RLOGCODE
" /RLOGIN",
"   Use Rlogin protocol even if this is not an Rlogin port.",
" ",
#endif /* RLOGCODE */
" /TELNET",
"   Send initial Telnet negotiations even if this is not a Telnet port.",
" ",
#ifdef CK_KERBEROS
#ifdef RLOGCODE
#ifdef KRB4
" /K4LOGIN",
"   Use Kerberos IV klogin protocol even if this is not a klogin port.",
" ",
#ifdef CK_ENCRYPTION
" /EK4LOGIN",
"   Use Kerberos IV Encrypted login protocol even if this is not an eklogin",
"   port.",
" ",
#endif /* CK_ENCRYPTION */
#endif /* KRB4 */
#ifdef KRB5
" /K5LOGIN",
"   Use Kerberos V klogin protocol even if this is not a klogin port.",
" ",
#ifdef CK_ENCRYPTION
" /EK5LOGIN",
"   Use Kerberos V Encrypted login protocol even if this is not an eklogin",
"   port.",
" ",
#endif /* CK_ENCRYPTION */
#endif /* KRB5 */
#endif /* RLOGCODE */
#endif /* CK_KERBEROS */
#ifdef CK_SSL
" /SSL",
"   Perform SSL negotiations.",
" ",
" /SSL-TELNET",
"   Perform SSL negotiations and if successful start Telnet negotiations.",
" ",
" /TLS",
"   Perform TLS negotiations.",
" ",
" /TLS-TELNET",
"   Perform TLS negotiations and if successful start Telnet negotiations.",
" ",
#endif /* CK_SSL */
"Examples:",
"  SET HOST kermit.columbia.edu",
"  SET HOST /CONNECT kermit.columbia.edu",
"  SET HOST * 1649",
"  SET HOST /SERVER * 1649",
"  SET HOST 128.59.39.2",
"  SET HOST madlab.sprl.umich.edu 3000",
"  SET HOST xyzcorp.com 2000 /RAW-SOCKET",
#ifdef NETCMD
"  SET HOST /CONNECT /COMMAND rlogin xyzcorp.com",
#endif /* NETCMD */
" ",
#ifdef SUPERLAT
"Notes:",
" ",
" . The TELNET command is equivalent to SET NETWORK TYPE TCP/IP,",
"   SET HOST name [ port ] /TELNET, IF SUCCESS CONNECT",
" ",
" . For SUPERLAT connections, the hostname-or-address may be either a service",
"   name, or a node/port combination, as required by your LAT host.",
#else
"The TELNET command is equivalent to SET NETWORK TYPE TCP/IP,",
"SET HOST name [ port ] /TELNET, IF SUCCESS CONNECT",
#endif /* SUPERLAT */
" ",
"Also see SET NETWORK, TELNET, SET TELNET.",
"" };

#ifdef TNCODE
static char *hmxyauth[] = {
"Synatx: SET AUTHENTICATION <auth_type> <parameter> <value>",
"  Sets defaults for the AUTHENTICATE command:",
" ",
#ifdef CK_KERBEROS
"SET AUTHENTICATION { KERBEROS4, KERBEROS5 } AUTODESTROY",
"  { ON-CLOSE, ON-EXIT, NEVER }",
"  When ON, Kermit will destroy all credentials in the default",
"  credentials cache upon Kermit termination.  Default is NEVER.",
" ",
"SET AUTHENTICATION { KERBEROS4, KERBEROS5 } AUTOGET { ON, OFF }",
"  When ON, if the host offers Kerberos 4 or Kerberos 5 authentication",
"  and Kermit is configured to use that authentication method and there",
"  is no TGT, Kermit will automatically attempt to retrieve one by",
"  prompting for the password (and principal if needed.)  Default is ON.",
" ",
"SET AUTHENTICATION KERBEROS5 CREDENTIALS-CACHE <filename>",
"  Allows an alternative credentials cache to be specified.  This is useful",
"  when you need to maintain two or more sets of credentials for different",
"  realms or roles.  The default is specified by the environment variable",
"  KRB5CCNAME or as reported by the Kerberos 5 library.",
" ",
"SET AUTHENTICATION KERBEROS5 FORWARDABLE { ON, OFF }",
"  When ON, specifies that Kerberos 5 credentials should be forwardable to",
"  the host.  If SET TELNET AUTHENTICATION FORWARDING is ON, forwardable",
"  credentials are sent to the host.  The default is OFF.",
" ",
"SET AUTHENTICATION KERBEROS5 GET-K4-TGT { ON, OFF }",
"  When ON, specifies that Kerberos 4 credentials should be requested each",
"  time Kerberos 5 credentials are requested with AUTH KERBEROS5 INIT.",
"  Default is OFF.",
" ",
"SET AUTHENTICATION KERBEROS4 INSTANCE <instance>",
"  Allows a Kerberos 4 instance to be specified as a default (if needed).",
" ",
"SET AUTHENTICATION { KERBEROS4, KERBEROS5 } LIFETIME <minutes>",
"  Specifies the lifetime of the TGTs requested from the KDC.  The default",
"  is 600 minutes (10 hours).",
" ",
"SET AUTHENTICATION KERBEROS4 PREAUTH { ON, OFF }",
"  Allows Kerberos 4 preauthenticated TGT requests to be turned off.  The",
"  default is ON.  Only use if absolutely necessary.  We recommend that",
"  preauthenticated requests be required for all tickets returned by a KDC",
"  to a requestor.",
" ",
"SET AUTHENTICATION { KERBEROS4, KERBEROS5 } PRINCIPAL <name>",
"  When Kermit starts, it attempts to set the principal name to that stored",
"  in the current credentials cache.  If no credential cache exists, the",
"  current SET LOGIN USERID value is used.  SET LOGIN USERID is set to the",
"  operating system's current username when Kermit is started.  To force",
"  Kermit to prompt the user for the principal name when requesting TGTs,",
"  place:",
" ",
"    SET AUTH K4 PRINCIPAL {}",
"    SET AUTH K5 PRINCIPAL {}",
" ",
"  in the Kermit initialization file or connection script.",
" ",
"SET AUTHENTICATION { KERBEROS4, KERBEROS5 } PROMPT PASSWORD <prompt>",
"  Specifies a custom prompt to be used when prompting for a password.",
"  The Kerberos prompt strings may contain two %s replacement fields.",
"  The first %s is replaced by the principal name; the second by the realm.",
" ",
"SET AUTHENTICATION { KERBEROS4, KERBEROS5 } PROMPT PRINCIPAL <prompt>",
"  Specifies a custom prompt to be used when prompting for the Kerberos",
"  principal name.  No %s replacement fields may be used.  Kermit prompts",
"  for a principal name when retrieving a TGT if the command:",
" ",
"    SET AUTHENTICATION { KERBEROS4, KERBEROS5 } PRINCIPAL {}",
" ",
"  has been issued.",
" ",
"SET AUTHENTICATION KERBEROS5 PROXIABLE { ON, OFF }",
"  When ON, specifies that Kerberos 5 credentials should be proxiable.",
"  Default is OFF.",
" ",
"SET AUTHENTICATION KERBEROS5 RENEWABLE <minutes>",
"  When <minutes> is greater than the ticket lifetime a TGT may be renewed",
"  with AUTH K5 INIT /RENEW instead of getting a new ticket as long as the",
"  ticket is not expired and its within the renewable lifetime.  Default is",
"  0 (zero) minutes.",
" ",
"SET AUTHENTICATION { KERBEROS4, KERBEROS5 } REALM <name>",
"  If no default is set, the default realm configured for the Kerberos",
"  libraries is used.  Abbreviations accepted.",
" ",
"SET AUTHENTICATION { KERBEROS4, KERBEROS5 } SERVICE-NAME <name>",
"  This command specifies the service ticket name used to authenticate",
"  to the host when Kermit is used as a client; or the service ticket",
"  name accepted by Kermit when it is acting as the host.",
"  If no default is set, the default service name for Kerberos 4 is",
"  \"rcmd\" and for Kerberos 5 is \"host\".",
" ",
#endif /* CK_KERBEROS */
#ifdef CK_SRP
"SET AUTHENTICATION SRP PROMPT PASSWORD <prompt>",
"  Specifies a custom prompt to be used when prompting for a password.",
"  The SRP prompt string may contain one %s replacement fields which is",
"  replaced by the login userid.",
" ",
#endif /* CK_SRP */
#ifdef CK_SSL
"In all of the following commands \"SSL\" and \"TLS\" are aliases.",
" ",
"SET AUTHENTICATE { SSL, TLS } CIPHER-LIST <list of ciphers>",
"Applies to both SSL and TLS.  A colon separated list of any of the following",
"(case sensitive) options depending on the options chosen when OpenSSL was ",
"compiled: ",
" ",
"  Key Exchange Algorithms:",
"    \"kRSA\"      RSA key exchange",
"    \"kDHr\"      Diffie-Hellman key exchange (key from RSA cert)",
"    \"kDHd\"      Diffie-Hellman key exchange (key from DSA cert)",
"    \"kEDH'      Ephemeral Diffie-Hellman key exchange (temporary key)",
" ",
"  Authentication Algorithm:",
"    \"aNULL\"     No authentication",
"    \"aRSA\"      RSA authentication",
"    \"aDSS\"      DSS authentication",
"    \"aDH\"       Diffie-Hellman authentication",
" ",
"  Cipher Encoding Algorithm:",
"    \"eNULL\"     No encodiing",
"    \"DES\"       DES encoding",
"    \"3DES\"      Triple DES encoding",
"    \"RC4\"       RC4 encoding",
"    \"RC2\"       RC2 encoding",
"    \"IDEA\"      IDEA encoding",
" ",
"  MAC Digest Algorithm:",
"    \"MD5\"       MD5 hash function",
"    \"SHA1\"      SHA1 hash function",
"    \"SHA\"       SHA hash function (should not be used)",
" ",
"  Aliases:",
"    \"SSLv2\"     all SSL version 2.0 ciphers (should not be used)",
"    \"SSLv3\"     all SSL version 3.0 ciphers",
"    \"EXP\"       all export ciphers (40-bit)",
"    \"EXPORT56\"  all export ciphers (56-bit)",
"    \"LOW\"       all low strength ciphers (no export)",
"    \"MEDIUM\"    all ciphers with 128-bit encryption",
"    \"HIGH\"      all ciphers using greater than 128-bit encryption",
"    \"RSA\"       all ciphers using RSA key exchange",
"    \"DH\"        all ciphers using Diffie-Hellman key exchange",
"    \"EDH\"       all ciphers using Ephemeral Diffie-Hellman key exchange",
"    \"ADH\"       all ciphers using Anonymous Diffie-Hellman key exchange",
"    \"DSS\"       all ciphers using DSS authentication",
"    \"NULL\"      all ciphers using no encryption",
" ",
"Each item in the list may include a prefix modifier:",
" ",
"    \"+\"         move cipher(s) to the current location in the list",
"    \"-\"         remove cipher(s) from the list (may be added again by",
"                a subsequent list entry)",
"    \"!\"         kill cipher from the list (it may not be added again",
"                by a subsequent list entry)",
" ",
"If no modifier is specified the entry is added to the list at the current ",
"position.  \"+\" may also be used to combine tags to specify entries such as "
,
"\"RSA+RC4\" describes all ciphers that use both RSA and RC4.",
" ",
"For example, all available ciphers not including ADH key exchange:",
" ",
"  ALL:!ADH:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv2:+EXP",
" ",
"All algorithms including ADH and export but excluding patented algorithms: ",
" ",
"  HIGH:MEDIUM:LOW:EXPORT56:EXP:ADH:!kRSA:!aRSA:!RC4:!RC2:!IDEA",
" ",
"The OpenSSL command ",
" ",
"  openssl.exe ciphers -v <list of ciphers> ",
" ",
"may be used to list all of the ciphers and the order described by a specific",
"<list of ciphers>.",
" ",
"SET AUTHENTICATE { SSL, TLS } CRL-DIR <directory>",
"specifies a directory that contains certificate revocation files where each",
"file is named by the hash of the certificate that has been revoked.",
" ",
"  OpenSSL expects the hash symlinks to be made like this:",
" ",
"    ln -s crl.pem `openssl crl -hash -noout -in crl.pem`.r0",
" ",
"  Since not all file systems have symlinks you can use the following command",
"  in Kermit to copy the crl.pem file to the hash file name.",
" ",
"     copy crl.pem {\fcommand(openssl.exe crl -hash -noout -in crl.pem).r0}",
" ",
"  This produces a hash based on the issuer field in the CRL such ",
"  that the issuer field of a Cert may be quickly mapped to the ",
"  correct CRL.",
" ",
"SET AUTHENTICATE { SSL, TLS } CRL-FILE <filename>",
"specifies a file that contains a list of certificate revocations.",
" ",
"SET AUTHENTICATE { SSL, TLS } DEBUG { ON, OFF }",
"specifies whether debug information should be displayed about the SSL/TLS",
"connection.  When DEBUG is ON, the VERIFY command does not terminate",
"connections when set to FAIL-IF-NO-PEER-CERT when a certificate is",
"presented that cannot be successfully verified.  Instead each error",
"is displayed but the connection automatically continues.",
" ",
"SET AUTHENTICATE { SSL, TLS } DH-PARAM-FILE <filename>",
"  Specifies a file containing DH parameters which are used to generate",
"  temporary DH keys.  If a DH parameter file is not provided Kermit uses a",
"  fixed set of parameters depending on the negotiated key length.  Kermit",
"  provides DH parameters for key lengths of 512, 768, 1024, 1536, and 2048",
"  bits.",
" ",
"SET AUTHENTICATE { SSL, TLS } DSA-CERT-FILE <filename>",
"  Specifies a file containing a DSA certificate to be sent to the peer to ",
"  authenticate the host or end user.  The file may contain the matching DH ",
"  private key instead of using the DSA-KEY-FILE command.",
" ",
"SET AUTHENTICATE { SSL, TLS } DSA-KEY-FILE <filename>",
"Specifies a file containing the private DH key that matches the DSA ",
"certificate specified with DSA-CERT-FILE.  This command is only necessary if",
"the private key is not appended to the certificate in the file specified by",
"DSA-CERT-FILE.",
" ",
"Note: When Kermit is running as an IKSD it cannot support encrypted private",
"keys.  If your private key file is encrypted you can use the following",
"command to unencrypt it (provided you know that pass phrase):",
" ",
"  openssl dsa -in <encrypted-key-file> -out <unencrypted-key-file>",
" ",
"SET AUTHENTICATE { SSL, TLS } RSA-CERT-FILE <filename>",
"  Specifies a file containing a RSA certificate to be sent to the peer to ",
"  authenticate the host or end user.  The file may contain the matching RSA ",
"  private key instead of using the RSA-KEY-FILE command.",
" ",
"SET AUTHENTICATE { SSL, TLS } RSA-KEY-FILE <filename>",
"  Specifies a file containing the private RSA key that matches the RSA",
"  certificate specified with RSA-CERT-FILE.  This command is only necessary",
"  if the private key is not appended to the certificate in the file specified"
,
"  by RSA-CERT-FILE.  ",
" ",
"Note: When Kermit is running as an IKSD it cannot support encrypted private",
"keys.  If your private key file is encrypted you can use the following",
"command to unencrypted (provided you know that pass phrase):",
" ",
"  openssl rsa -in <encrypted-key-file> -out <unencrypted-key-file>",
" ",
"SET AUTHENTICATE { SSL, TLS } VERBOSE { ON, OFF }",
"  Specifies whether information about the authentication (ie, the",
"  certificate chain) should be displayed upon making a connection.",
" ",
"SET AUTHENTICATE { SSL, TLS } VERIFY { NO, PEER-CERT, FAIL-IF-NO-PEER-CERT }",
"  Specifies whether certificates should be requested from the peer verified;",
"  whether they should be verified when they are presented; and whether they",
"  should be required.  When set to NO (the default for IKSD), Kermit does",
"  not request that the peer send a certificate; if one is presented it is",
"  ignored.  When set to PEER-CERT (the default when not IKSD), Kermit",
"  requests a certificate be sent by the peer.  If presented, the certificate",
"  is verified.  Any errors during the verification process result in queries",
"  to the end user.  When set to FAIL-IF-NO-PEER-CERT, Kermit asks the peer",
"  to send a certificate.  If the certificate is not presented or fails to",
"  verify successfully, the connection is terminated without querying the",
"  user.",
" ",
"  If an anonymous cipher (i.e., ADH) is desired, the NO setting must be",
"  used.  Otherwise, the receipt of the peer certificate request is",
"  interpreted as a protocol error and the negotiation fails.",
" ",
"  If you wish to allow the peer to authenticate using either an X509",
"  certificate to userid mapping function or via use of a ~/.tlslogin file",
"  you must use either PEER-CERT or FAIL-IF-NO-PEER-CERT.  Otherwise, any",
"  certificates that are presented is ignored.  In other words, use NO if you",
"  want to disable the ability to use certificates to authenticate a peer.",
" ",
"SET AUTHENTICATE { SSL, TLS } VERIFY-DIR <directory>",
"  Specifies a directory that contains root CA certificate files used to",
"  verify the certificate chains presented by the peer.  Each file is named",
"  by a hash of the certificate.",
" ",
"  OpenSSL expects the hash symlinks to be made like this:",
" ",
"    ln -s cert.pem `openssl x509 -hash -noout -in cert.pem`.0",
" ",
"  Since not all file systems have symlinks you can use the following command",
"  in Kermit to copy the cert.pem file to the hash file name.",
" ",
"    copy cert.pem {\\fcommand(openssl.exe x509 -hash -noout -in cert.pem).0}",
" ",
"  This produces a hash based on the subject field in the cert such that the",
"  certificate may be quickly found.",
" ",
"SET AUTHENTICATE { SSL, TLS } VERIFY-FILE <file>",
"  Specifies a file that contains root CA certificates to be used for",
"  verifying certificate chains.",
" ",
#endif /* CK_SSL */
""
};
#endif /* TNCODE */

static char *hxynet[] = {
"Syntax: SET NETWORK { TYPE network-type, DIRECTORY [ file(s)... ] }",
" ",
"Select the type of network to be used with SET HOST connections:",
" ",
#ifdef NETCMD
"  SET NETWORK TYPE COMMAND   ; Make a connection through an external command",
#endif /* NETCMD */
#ifdef TCPSOCKET
"  SET NETWORK TYPE TCP/IP    ; Internet: Telnet, Rlogin, etc.",
#endif /* TCPSOCKET */
#ifdef ANYX25
"  SET NETWORK TYPE X.25      ; X.25 peer-to-peer connections.",
#endif /* ANYX25 */
#ifdef DECNET
"  SET NETWORK TYPE PATHWORKS { LAT, CTERM } ; DEC LAT or CTERM connections.",
#endif /* DECNET */
#ifdef NPIPE
"  SET NETWORK TYPE NAMED-PIPE <pipename>  ; OS/2 Named Pipe connections.",
#endif /* NPIPE */
#ifdef CK_NETBIOS
"  SET NETWORK TYPE NETBIOS                ; NETBIOS peer-to-peer connections",
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
"  SET NETWORK TYPE SUPERLAT ; LAT connections (Meridian Technology SuperLAT)",
#endif /* SUPERLAT */
" ",
"If only one network type is listed above, that is the default network for",
#ifdef RLOGCODE
"SET HOST commands.  Also see SET HOST, TELNET, RLOGIN.",
#else
#ifdef TNCODE
"SET HOST commands.  Also see SET HOST, TELNET.",
#else
"SET HOST commands.  Also see SET HOST.",
#endif /* TNCODE */
#endif /* RLOGCODE */
" ",
"SET NETWORK DIRECTORY [ file [ file [ ... ] ] ]",
"  Specifies the name(s) of zero or more network directory files, similar to",
"  dialing directories (HELP DIAL for details).  The general format of a",
"  network directory entry is:",
" ",
"    name network-type address [ network-specific-info ] [ ; comment ]",
" ",
"  For TCP/IP, the format is:",
" ",
"    name tcp/ip ip-hostname-or-address [ socket ] [ ; comment ]",
" ",
"You can have multiple network directories and you can have multiple entries",
"with the same name.  SET HOST <name> and TELNET <name> commands look up the",
"given <name> in the directory and, if found, fill in the additional items",
"from the entry, and then try all matching entries until one succeeds.",
""};

#ifndef NOTCPOPTS
static char *hxytcp[] = {
#ifdef SOL_SOCKET
"SET TCP ADDRESS <ip-address>",
"  This allows a specific IP Address on a multihomed host to be used",
"  instead of allowing the TCP/IP stack to choose.  This may be necessary",
"  when using authentication or listening for an incoming connection.",
"  Specify no <ip-address> to remove the preference.",
" ",
"SET TCP KEEPALIVE { ON, OFF }",
"  Setting this ON might help to detect broken connections more quickly.",
"  (default is ON.)",
" ",
"SET TCP LINGER { ON [timeout], OFF }",
"  Setting this ON ensures that a connection doesn't close before all",
"  outstanding data has been transferred and acknowledged.  The timeout is",
"  measured in 10ths of milliseconds.  The default is ON with a timeout of 0.",
" ",
"SET TCP NODELAY { ON, OFF }",
"  ON means send short TCP packets immediately rather than waiting",
"  to accumulate a bunch of them before transmitting (Nagle Algorithm).",
"  (default is OFF.)",
" ",
"SET TCP RECVBUF <number>",
"SET TCP SENDBUF <number>",
"   TCP receive and send buffer sizes.  (default is -1, use system defaults.)",
" ",
"These items let you tune TCP networking performance on a per-connection",
"basis by adjusting parameters you normally would not have access to.  You",
"should use these commands only if you feel that the TCP/IP protocol stack",
"that Kermit is using is giving you inadequate performance, and then only if",
"you understand the concepts (see, for example, the Comer TCP/IP books), and",
"then at your own risk.  These settings are displayed by SHOW NETWORK.  Not",
"all options are necessarily available in all Kermit versions; it depends on",
"the underlying TCP/IP services.",
" ",
"The following TCP and/or IP parameter(s) may also be changed:",
" ",
#endif /* SOL_SOCKET */
"SET TCP REVERSE-DNS-LOOKUP { AUTO, ON, OFF }",
"  Tells Kermit whether to perform reverse DNS lookup on TCP/IP connections",
"  so Kermit can determine the actual hostname of the host it is connected",
"  to, which is useful for connections to host pools, and is required for",
"  Kerberos connections to host pools and for incoming connections.  If the",
"  other host does not have a DNS entry, the reverse lookup could take a long",
"  time (minutes) to fail, but the connection will still be made.  Turn this",
"  option OFF for speedier connections if you do not need to know exactly",
"  which host you are connected to and you are not using Kerberos.  AUTO, the",
"  default, means the lookup is done on hostnames, but not on numeric IP",
"  addresses unless Kerberos support is installed.",
#ifdef CK_DNS_SRV
" ",
"SET TCP DNS-SERVICE-RECORDS {ON, OFF}",
"  Tells C-Kermit whether to try to use DNS SRV records to determine the",
"  host and port number upon which to find an advertised service.  For",
"  example, if a host wants regular Telnet connections redirected to some",
"  port other than 23, this feature allows C-Kermit to ask the host which",
"  port it should use.  Since not all domain servers are set up to answer",
"  such requests, this feature is OFF by default.",
#endif /* CK_DNS_SRV */
""};
#endif /* NOTCPOPTS */
#endif /* NETCONN */

#ifdef TNCODE
static char *hxytopt[] = {
"SET TELOPT [ { /CLIENT, /SERVER } ] <option> -",
"    { ACCEPTED, REFUSED, REQUESTED, REQUIRED } -",
"    [ { ACCEPTED, REFUSED, REQUESTED, REQUIRED } ]",
"  SET TELOPT lets you specify policy requirements for Kermit's handling of",
"  Telnet option negotiations.  Setting an option REQUIRED causes Kermit",
"  to offer the option to the peer and disconnect if the option is refused.",
"  REQUESTED causes Kermit to offer an option to the peer.  ACCEPTED results",
"  in no offer but Kermit will attempt to negotiate the option if it is",
"  requested.  REFUSED instructs Kermit to refuse the option if it is",
"  requested by the peer.",
" ",
"  Some options are negotiated in two directions and accept separate policies",
"  for each direction; the first keyword applies to Kermit itself, the second",
"  applies to Kermit's Telnet partner; if the second keyword is omitted, an",
"  appropriate (option-specific) default is applied.  You can also include a",
"  /CLIENT or /SERVER switch to indicate whether the given policies apply",
"  when Kermit is the Telnet client or the Telnet server; if no switch is",
"  given, the command applies to the client.",
" ",
"  Note that some of Kermit's Telnet partners fail to refuse options that",
"  they do not recognize and instead do not respond at all.  In this case it",
"  is possible to use SET TELOPT to instruct Kermit to REFUSE the option",
"  before connecting to the problem host, thus skipping the problematic",
"  negotiation.",
" ",
"  Use SHOW TELOPT to view current Telnet Option negotiation settings.",
"  SHOW TELNET displays current Telnet settings.",
""};

static char *hxytel[] = {
"Syntax: SET TELNET parameter value",
" ",
"For TCP/IP TELNET connections, which are in NVT (ASCII) mode by default:",
" ",
#ifdef CK_AUTHENTICATION
#ifdef COMMENT
"SET TELNET AUTHENICATION { ACCEPTED, REFUSED, REQUESTED, REQUIRED }",
"  ACCEPT or REFUSE authentication bids, or actively REQUEST authentication.",
"  REQUIRED refuses the connection if authentication is not successfully",
"  negotiated.  ACCEPTED by default.",
" ",
#endif /* COMMENT */
"SET TELNET AUTHENTICATION TYPE { AUTOMATIC, KERBEROS_IV, KERBEROS_V, ...",
"  ..., NTLM, SSL, SRP, NONE } [...]",
"  AUTOMATIC allows the host to choose the preferred type of authentication.",
"  Other values allow a specific authentication method to be used.  AUTOMATIC",
"  is the default.  Available options can vary depending on configuration;",
"  type SET TELNET AUTHENTICATION TYPE ? for a list.",
" ",
#ifdef CK_KERBEROS
"SET TELNET AUTHENTICATION FORWARDING { ON, OFF }",
"  Set this to ON to forward Kerberos V ticket-granting-tickets to the host",
"  after authentication is complete.  OFF by default.",
" ",
#endif /* CK_KERBEROS */
"SET TELNET AUTHENTICATION ENCRYPT-FLAG { ANY, NONE, TELOPT }",
"  Use this command to specify which AUTH telopt encryption flags may be",
"  accepted in client mode or offered in server mode.  The default is ANY.",
" ",
"SET TELNET AUTHENTICATION HOW-FLAG { ANY, ONE-WAY, MUTUAL }",
"  Use this command to specify which AUTH telopt how flags may be",
"  accepted in client mode or offered in server mode.  The default is ANY.",
" ",
#endif /* CK_AUTHENTICATION */
#ifdef COMMENT
"SET TELNET BINARY-MODE { ACCEPTED, REFUSED, REQUESTED, REQUIRED }",
"  ACCEPT or REFUSE binary-mode bids, or actively REQUEST binary mode.",
"  REQUIRED refuses the connection if binary mode is not successfully",
"  negotiated in both directions.  ACCEPTED by default.",
" ",
#endif /* COMMENT */
"SET TELNET BINARY-TRANSFER-MODE { ON, OFF }",
"  When ON (OFF by default) and BINARY negotiations are not REFUSED Kermit",
"  will attempt to negotiate BINARY mode in each direction before the start",
"  of each file transfer.  After the transfer is complete BINARY mode will",
"  be restored to the pre-transfer state.",
" ",
"SET TELNET BUG BINARY-ME-MEANS-U-TOO { ON, OFF }",
"  Set this to ON to try to overcome TELNET binary-mode misnegotiations by",
"  C-Kermit's TELNET partner.",
" ",
"SET TELNET BUG BINARY-U-MEANS-ME-TOO { ON, OFF }",
"  Set this to ON to try to overcome TELNET binary-mode misnegotiations by",
"  C-Kermit's TELNET partner.",
" ",
"SET TELNET BUG INFINITE-LOOP-CHECK { ON, OFF }",
"  Set this to ON to prevent Kermit from responding to a telnet negotiation",
"  sequence that enters an infinite loop.  The default is OFF because this",
"  should never occur.",
" ",
"SET TELNET BUG SB-IMPLIES-WILL-DO { ON, OFF }",
"  Set this to ON to allow Kermit to respond to telnet sub-negotiations if",
"  the peer forgets to respond to WILL with DO or to DO with WILL.",
" ",
"SET TELNET ECHO { LOCAL, REMOTE }",
"  C-Kermit's initial echoing state for TELNET connections, LOCAL by default.",
"  After the connection is made, TELNET negotiations determine the echoing.",
" ",
#ifdef CK_ENCRYPTION
#ifdef COMMENT
"SET TELNET ENCRYPTION { ACCEPTED, REFUSED, REQUESTED, REQUIRED }",
"  ACCEPT or REFUSE encryption bids, or actively REQUEST encryption in both.",
"  directions.  REQUIRED refuses the connection if encryption is not",
"  successfully negotiated in both directions.  ACCEPTED by default.",
" ",
#endif /* COMMENT */
"SET TELNET ENCRYPTION TYPE { AUTOMATIC, CAST128_CFB64, CAST128_OFB64, ",
"  CAST5_40_CFB64, CAST5_40_OFB64, DES_CFB64, DES_OFB64, NONE }",
"  AUTOMATIC allows the host to choose the preferred type of encryption.",
"  Other values allow a specific encryption method to be specified.",
"  AUTOMATIC is the default.  The list of options will vary depending",
"  on the encryption types selected at compilation time.",
" ",
#endif /* CK_ENCRYPTION */
#ifdef CK_ENVIRONMENT
#ifdef COMMENT
"SET TELNET ENVIRONMENT { ON, OFF, variable-name [ value ] }",
"  This feature lets Kermit send the values of certain environment variables",
"  to the other computer if it asks for them.  The variable-name can be any",
"  of the \"well-known\" variables \"USER\", \"JOB\", \"ACCT\", \"PRINTER\",",
"  \"SYSTEMTYPE\", or \"DISPLAY\".  Some Telnet servers, if given a USER",
"  value in this way, will accept it and therefore not prompt you for user",
"  name when you log in.  The default values are taken from your environment;",
"  use this command to change or remove them.  See RFC1572 for details.  You",
"  may also specify OFF to disable this feature, and ON to re-enable it.",
" ",
#else
"SET TELNET ENVIRONMENT { variable-name [ value ] }",
"  This feature lets Kermit send the values of certain environment variables",
"  to the other computer if it asks for them.  The variable-name can be any",
"  of the \"well-known\" variables \"USER\", \"JOB\", \"ACCT\", \"PRINTER\",",
"  \"SYSTEMTYPE\", or \"DISPLAY\".  Some Telnet servers, if given a USER",
"  value in this way, will accept it and therefore not prompt you for user",
"  name when you log in.  The default values are taken from your environment;",
"  use this command to change or remove them.  See RFC1572 for details.",
" ",
#endif /* COMMENT */
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
"SET TELNET LOCATION [ text ]",
"  Location string to send to the Telnet server if it asks.  By default this",
"  is picked up from the LOCATION environment variable.  Give this command",
"  with no text to disable this feature.",
" ",
#endif /* CK_SNDLOC */
"SET TELNET NEWLINE-MODE { NVT, BINARY-MODE } { OFF, ON, RAW }",

"  Determines how carriage returns are handled on TELNET connections.  There",
"  are separate settings for NVT (ASCII) mode and binary mode.  ON (default",
"  for NVT mode) means CRLF represents CR.  OFF means CR followed by NUL",
"  represents CR.  RAW (default for BINARY mode) means CR stands for itself.",
" ",
#ifdef TCPSOCKET
"SET TELNET PROMPT-FOR-USERID <prompt>",
"  Specifies a custom prompt to be used when prompting for a userid.  Kermit",
"  prompts for a userid if the command:",
" ",
"    SET LOGIN USERID {}",
" ",
"  has been issued prior to a Telnet authentication negotiation for an",
"  authentication type that requires the transmission of a name, such as",
"  Secure Remote Password.",
" ",
#endif /* TCPSOCKET */
"SET TELNET REMOTE-ECHO { ON, OFF }",
"  Applies only to incoming connections created with:",
"    SET HOST * <port> /TELNET",
"  This command determines whether Kermit will actually echo characters",
"  received from the remote when it has negotiated to do so.  The default",
"  is ON.  Remote echoing may be turned off when it is necessary to read",
"  a password with the INPUT command.",
" ",
"SET TELNET TERMINAL-TYPE name",
"  The terminal type to send to the remote TELNET host.  If none is given,",
#ifdef OS2
"  your current SET TERMINAL TYPE value is sent, e.g. VT220.",
" ",
#else
"  your local terminal type is sent.",
" ",
#endif /* OS2 */
"SET TELNET WAIT-FOR-NEGOTIATIONS { ON, OFF }",
"  Each Telnet option must be fully negotiated either On or Off before the",
"  session can continue.  This is especially true with options that require",
"  subnegotiations such as Authentication, Encryption, and Kermit; for",
"  proper support of these options Kermit must wait for the negotiations to",
"  complete.  Of course, Kermit has no way of knowing whether a reply is",
"  delayed or not coming at all, and so will wait a minute or more for",
"  required replies before continuing the session.  If you know that Kermit's",
"  Telnet partner will not be sending the required replies, you can set this",
"  option of OFF to avoid the long timeouts.  Or you can instruct Kermit to",
"  REFUSE specific options with the SET TELOPT command.",
"",
"Type SHOW TELNET to see the current values of these parameters.",
"" };
#endif /* TNCODE */

#ifndef NOSPL
static char *hxymacr[] = {
"Syntax: SET MACRO parameter value",
"  Controls the behavior of macros.",
" ",
"SET MACRO ECHO { ON, OFF }",
"  Tells whether commands executed from a macro definition should be",
"  displayed on the screen.  OFF by default; use ON for debugging.",
" ",
"SET MACRO ERROR { ON, OFF }",
"  Tells whether a macro should be automatically terminated upon a command",
"  error.  This setting is local to the current macro, and inherited by",
"  subordinate macros.",
"" };
#endif /* NOSPL */

static char *hmxyprm[] = {
"Syntax: SET PROMPT [ text ]",
" ",
#ifdef OS2
"Prompt text for this program, normally 'K-95>'.  May contain backslash",
#else
#ifdef MAC
"Prompt text for this program, normally 'Mac-Kermit>'.  May contain backslash",
#else
"Prompt text for this program, normally 'C-Kermit>'.  May contain backslash",
#endif /* MAC */
#endif /* OS2 */
"codes for special effects.  Surround by { } to preserve leading or trailing",
#ifdef OS2
"spaces.  If text omitted, prompt reverts to Mac-Kermit>.  Prompt can include",
#else
#ifdef MAC
"spaces.  If text omitted, prompt reverts to Mac-Kermit>.  Prompt can include",
#else
"spaces.  If text omitted, prompt reverts to C-Kermit>.  Prompt can include",
#endif /* OS2 */
#endif /* MAC */
"variables like \\v(dir) or \\v(time) to show current directory or time.",
"" };

#ifdef UNIX
static char *hxywild[] = {
"Syntax: SET WILDCARD-EXPANSION { KERMIT [ switch ], SHELL }",
"  KERMIT (the default) means C-Kermit expands filename wildcards in SEND and",
"  similar commands itself, and in incoming GET commands.  Optional switches",
"  are /NO-MATCH-DOT-FILES (\"*\" and \"?\" should not match in initial",
"  period in a filename; this is the default) and /MATCH-DOT-FILES if you",
"  want files whose names begin with \".\" included.  SET WILDCARD SHELL",
"  means that Kermit asks your preferred shell to expand wildcards (this",
"  should not be necessary in C-Kermit 7.0 and later).  HELP WILDCARD for",
"  further information.",
"" };
#endif /* UNIX */

#ifndef NOXFER
static char *hxywind[] = {
"Syntax: SET WINDOW-SIZE number",
"  Specifies number of slots for sliding windows, i.e. the number of packets",
"  that can be transmitted before waiting for acknowledgement.  The default",
#ifdef XYZ_INTERNAL
"  for Kermit protocol is one, the maximum is 32; for ZMODEM, the default",
"  is no windowing (0).  For ZMODEM, the window size is really the packet",
"  length, and is used only when non-windowed (streaming) transfers fail; the",
"  ZMODEM window size should be a largish number, like 1024, and it should be",
"  a multiple of 64.",
#else
"  is one, the maximum is 32.  Increased window size might result in reduced",
"  maximum packet length.  Use sliding windows for improved efficiency on",
"  connections with long delays.  A full duplex connection is required, as",
"  well as a cooperating Kermit on the other end.",
#endif /* XYZ_INTERNAL */
"" };

static char *hxyrpt[] = {
"Syntax: SET REPEAT { COUNTS { ON, OFF }, PREFIX <code> }",
"  SET REPEAT COUNTS turns the repeat-count compression mechanism ON and OFF.",
"  The default is ON.  SET REPEAT PREFIX <code> sets the repeat-count prefix",
"  character to the given code.  The default is 126 (tilde).",
"" };

static char *hxyrcv[] = {
"Syntax: SET RECEIVE parameter value",
"  Specifies parameters for inbound packets:",
" ",
#ifndef NOCSETS
"SET RECEIVE CHARACTER-SET { AUTOMATIC, MANUAL }",
"  Whether to automatically switch to an appropriate file-character set based",
"  on the transfer character-set announcer, if any, of an incoming text file.",
"  AUTOMATIC by default.  Also see HELP ASSOCIATE.",
" ",
#endif /* NOCSETS */
"SET RECEIVE CONTROL-PREFIX number",
"  ASCII value of prefix character used for quoting control characters in",
"  packets that C-Kermit receives, normally 35 (number sign).  Don't change",
"  this unless something is wrong with the other Kermit program.",
" ",
"SET RECEIVE END-OF-PACKET number",
"  ASCII value of control character that terminates incoming packets,",
"  normally 13 (carriage return).",
" ",
#ifdef CKXXCHAR
"SET RECEIVE IGNORE-CHARACTER number",
"  ASCII value of character to be discarded when receiving packets, such as",
"  line folding characters.",
" ",
#endif /* CKXXCHAR */
"SET RECEIVE MOVE-TO [ directory ]",
"  If a directory name is specified, then every file that is received",
"  successfully is moved to the given directory immediately after reception",
"  is complete.  Omit the directory name to remove any previously set move-to",
"  directory.",
" ",
"SET RECEIVE PACKET-LENGTH number",
"  Maximum length packet the other Kermit should send.",
" ",
"SET RECEIVE PADDING number",
"  Number of prepacket padding characters to ask for (normally 0).",
" ",
"SET RECEIVE PAD-CHARACTER number",
"  ASCII value of control character to use for padding (normally 0).",
" ",
"SET RECEIVE PATHNAMES {OFF, ABSOLUTE, RELATIVE, AUTO}",
"  If a recognizable path (directory, device) specification appears in an",
"  incoming filename, strip it OFF before trying to create the output file.",
#ifdef CK_MKDIR
"  Otherwise, then if any of the directories in the path don't exist, Kermit",
"  tries to create them, relative to your current or download directory, or",
"  absolutely, as specified.  RELATIVE means force all incoming names, even",
"  if they are absolute, to be relative to your current or download directory."
,
"  AUTO, which is the default, means RELATIVE if the file sender indicates in",
"  advance that this is a recursive transfer, otherwise OFF.",
#endif /* CK_MKDIR */
" ",
"SET RECEIVE PAUSE number",
"  Milliseconds to pause between packets, normally 0.",
" ",
"SET RECEIVE RENAME-TO [ template ]",
"  If a template is specified, then every file that is received successfully",
"  \
is renamed according to the given template immediately after it is received.",
"  \
The template should include variables like \\v(filename) or \\v(filenumber).",
"  Omit the template to remove any template previously set.",
" ",
"SET RECEIVE START-OF-PACKET number",
"  ASCII value of character that marks start of inbound packet.",
" ",
"SET RECEIVE TIMEOUT number",
"  Number of seconds the other Kermit should wait for a packet before sending",
"  a NAK or retransmitting.",
#ifdef VMS
" ",
"SET RECEIVE VERSION-NUMBERS { ON, OFF }",
"  If ON, and in incoming filename includes a VMS version number, use it when",
"  creating the file.  If OFF (which is the default), strip any VMS version",
"  number from incoming filenames before attempting to create the file, \
causing",
"  the new file to receive the next highest version number.",
#endif /* VMS */
"" };

static char *hxysnd[] = {
"Syntax: SET SEND parameter value",
"  Specifies parameters for outbound files or packets.",
" ",
"SET SEND BACKUP { ON, OFF }",
"  Tells whether to include backup files when sending file groups.  Backup",
"  files are those created by Kermit, EMACS, etc, when creating a new file",
"  that has the same name as an existing file.  A backup file has a version",
"  appended to its name, e.g. oofa.txt.~23~.  ON is the default, meaning",
"  don't exclude backup files.  Use OFF to exclude backup files from group",
"  transfers.",
" ",
#ifndef NOCSETS
"SET SEND CHARACTER-SET { AUTOMATIC, MANUAL }",
"  Whether to automatically switch to an appropriate file-character when a",
"  SET TRANSFER CHARACTER-SET command is given, or vice versa.  AUTOMATIC by",
"  default.  Also see HELP ASSOCIATE.",
" ",
#endif /* NOCSETS */

"SET SEND CONTROL-PREFIX number",
"  ASCII value of prefix character used for quoting control characters in",
"  packets that C-Kermit sends, normally 35 (number sign).",
" ",
#ifdef CKXXCHAR
"SET SEND DOUBLE-CHARACTER number",
"  ASCII value of character to be doubled when sending packets, such as an",
"  X.25 PAD escape character.",
" ",
#endif /* CKXXCHAR */
"SET SEND END-OF-PACKET number",
"  ASCII value of control character to terminate an outbound packet,",
"  normally 13 (carriage return).",
" ",
"SET SEND MOVE-TO [ directory ]",
"  \
If a directory name is specified, then every file that is sent successfully",
"  is moved to the given directory immediately after it is sent.",
"  Omit the directory name to remove any previously set move-to directory.",
" ",
"SET SEND PACKET-LENGTH number",
"  Maximum length packet to send, even if other Kermit asks for longer ones.",
"  This command can not be used to force packets to be sent that are longer",
"  than the length requested by the receiver.  Use this command only to",
"  force shorter ones.",
" ",
"SET SEND PADDING number",
"  Number of prepacket padding characters to send.",
" ",
"SET SEND PAD-CHARACTER number",
"  ASCII value of control character to use for padding.",
" ",
"SET SEND PATHNAMES {OFF, ABSOLUTE, RELATIVE}",
"  Include the path (device, directory) portion with the file name when",
"  sending it as specified; ABSOLUTE means to send the whole pathname,",
"  RELATIVE means to include the pathname relative to the current directory.",
"  Applies to the actual filename, not to the \"as-name\".  The default is",
"  OFF.",
" ",
"SET SEND PAUSE number",
"  Milliseconds to pause between packets, normally 0.",
" ",
"SET SEND RENAME-TO [ template ]",
"  If a template is specified, then every file that is sent successfully",
"  is renamed according to the given template immediately after it is sent.",
"  \
The template should include variables like \\v(filename) or \\v(filenumber).",
"  Omit the template to remove any template previously set.",
" ",
"SET SEND START-OF-PACKET number",
"  ASCII value of character to mark start of outbound packet.",
" ",
#ifdef CK_TIMERS
"SET SEND TIMEOUT number [ { DYNAMIC [ min max ] ], FIXED } ]",
#else
"SET SEND TIMEOUT number",
#endif /* CK_TIMERS */

"  Number of seconds to wait for a packet before sending NAK or",
#ifdef CK_TIMERS
"  retransmitting.  Include the word DYNAMIC after the number in the",
"  SET SEND TIMEOUT command to have Kermit compute the timeouts dynamically",
"  throughout the transfer based on the packet rate.  Include the word FIXED",
"  to use the \"number\" given throughout the transfer.  DYNAMIC is the",
"  default.  After DYNAMIC you may include minimum and maximum values.",
#else
"  retransmitting.",
#endif /* CK_TIMERS */
#ifdef VMS
" ",
"SET SEND VERSION-NUMBERS { ON, OFF }",
"  If ON, include VMS version numbers in outbound filenames.  If OFF (which",
"  is the default), strip version numbers.",
#endif /* VMS */
"" };

static char *hxyxfer[] = {
"Syntax: SET TRANSFER (or XFER) paramater value",
" ",
"Choices:",
" ",
"SET TRANSFER BELL { OFF, ON }",
"  Whether to ring the terminal bell at the end of a file transfer.",
" ",
#ifdef XFRCAN
"SET TRANSFER CANCELLATION { OFF, ON [ <code> [ <number> ] ] }",
"  OFF disables remote-mode packet-mode cancellation from the keyboard.",
"  ON enables it.  The optional <code> is the control character to use for",
"  cancellation; the optional <number> is how many consecutive occurrences",
"  of the given control character are required for cancellation.",
" ",
#endif /* XFRCAN */
"SET TRANSFER INTERRUPTION { ON, OFF }",
"  TRANSFER INTERRUPTION is normally ON, allowing for interruption of a file",
"  transfer in progress by typing certain characters while the file-transfer",
"  display is active.  SET TRANSFER INTERRUPTION OFF disables interruption",
"  of file transfer from the keyboard in local mode.",
" ",
#ifndef NOSPL
"SET TRANSFER CRC-CALCULATION { OFF, ON }",
"  Tells whether C-Kermit should accumulate a Cyclic Redundancy Check for ",
"  each file transfer.  Normally ON, in which case the CRC value is available",
"  in the \\v(crc16) variable after the transfer.  Adds some overhead.  Use",
"  SET TRANSFER CRC OFF to disable.",
" ",
#endif /* NOSPL */
#ifndef NOCSETS
"SET TRANSFER CHARACTER-SET name",
"  Selects the character set used to represent textual data in Kermit",
"  packets.  Text characters are translated to/from the FILE CHARACTER-SET.",
"  Choices:",
" ",
"  TRANSPARENT (no translation, the default)",
"  ASCII",
"  LATIN1 (ISO 8859-1 Latin Alphabet 1)",
#ifndef NOLATIN2
"  LATIN2 (ISO 8859-2 Latin Alphabet 2)",
#endif /* NOLATIN2 */
"  LATIN9 (ISO 8859-15 Latin Alphabet 9)",
#ifdef CYRILLIC
"  CYRILLIC-ISO (ISO 8859-5 Latin/Cyrillic)",
#endif /* CYRILLIC */
#ifdef GREEK
"  GREEK-ISO (ISO 8859-7 Latin/Greek)",
#endif /* GREEK */
#ifdef HEBREW
"  HEBREW-ISO (ISO 8859-8 Latin/Hebrew)",
#endif /* HEBREW */
#ifdef KANJI
"  JAPANESE-EUC (JIS X 0208 Kanji + Roman and Katakana)",
#endif /* KANJI */
#ifdef UNICODE
"  UCS-2 (ISO 10646 / Unicode 2-byte form)",
"  UTF-8 (ISO 10646 / Unicode 8-bit serialized transformation format)",
#endif /* UNICODE */
" ",
#ifdef CK_CURSES
"SET TRANSFER DISPLAY { BRIEF, CRT, FULLSCREEN, NONE, SERIAL }",
#else
"SET TRANSFER DISPLAY { BRIEF, CRT, NONE, SERIAL }",
#endif  /* CK_CURSES */
"  Choose the desired format for the progress report to be displayed on",
"  your screen during file transfers when C-Kermit is in local mode.",
#ifdef CK_CURSES
"  FULLSCREEN requires your terminal type be set correctly; the others",
"  are independent of terminal type.",
#else
"  file transfer.",
#endif  /* CK_CURSES */
" ",
#endif /* NOCSETS */
"SET TRANSFER LOCKING-SHIFT { OFF, ON, FORCED }",
"  Tell whether locking-shift protocol should be used during file transfer",
"  to achieve 8-bit transparency on a 7-bit connection.  ON means to request",
"  its use if PARITY is not NONE and to use it if the other Kermit agrees,",
"  OFF means not to use it, FORCED means to use it even if the other Kermit",
"  does not agree.",
" ",
"SET TRANSFER MODE { AUTOMATIC, MANUAL }",
"  Automatic (the default) means C-Kermit should automatically go into binary",
"  file-transfer mode and use literal filenames if the other Kermit says it",
"  has a compatible file system, e.g. UNIX-to-UNIX, but not UNIX-to-DOS.",
#ifdef PATTERNS
"  Also, when sending files, C-Kermit should switch between binary and text",
"  mode automatically per file based on the SET FILE BINARY-PATTERNS and SET",
"  FILE TEXT-PATTERNS.",
#endif /* PATTERNS */
" ",
#ifdef PIPESEND
"SET TRANSFER PIPES { ON, OFF }",
"  Enables/Disables automatic sending from / reception to command pipes when",
"  the incoming filename starts with '!'.  Also see CSEND, CRECEIVE.",
" ",
#endif /* PIPESEND */
#ifdef CK_XYZ
"SET TRANSFER PROTOCOL { KERMIT, XMODEM, ... }",
"  Synonym for SET PROTOCOL (q.v.).",
" ",
#endif /* CK_XYZ */
"SET TRANSFER SLOW-START { OFF, ON }",
"  ON (the default) tells C-Kermit, when sending files, to gradually build up",
"  the packet length to the maximum negotiated length.  OFF means start",
"  sending the maximum length right away.",
" ",
"Synonym: SET XFER.  Use SHOW TRANSFER (XFER) to see SET TRANSFER values.",
"" };
#endif /* NOXFER */

#ifdef NT
static char *hxywin95[] = {
"SET WIN95 8.3-FILENAMES { ON, OFF }",
"  Instructs K-95 to report all filenames using 8.3 notation instead of the",
"  normal long filenames.  Default is OFF",
" ",
"SET WIN95 ALT-GR { ON, OFF }",
"  Instructs K-95, when used on MS Windows 95, to interpret the Right Alt key",
"  as the Alt-Gr key.  This is necessary to work around the failure of",
"  Windows 95 to properly translate non-US keyboards.  Default is OFF.",
" ",
"SET WIN95 KEYBOARD-TRANSLATION <character-set>",
"  Specifies the character-set that Windows 95 is using to send keystrokes",
"  to Kermit-95 via the keyboard input functions.  Default is Latin1-ISO.",
" ",
"SET WIN95 OVERLAPPED-IO { ON <requests>, OFF }",
"  Determines whether or not K-95 uses Overlapped-I/O methods for reading",
"  from and writing to serial and TAPI communication devices.  <requests>",
"  specifies the number of simultaneous write requests that may be",
"  overlapped, from 1 to 5.  Default is ON 3.",
" ",
"SET WIN95 POPUPS { ON, OFF }",
"  Determines whether or not Kermit 95 uses Popups to query the user for",
"  necessary information such as user IDs or passwords.  Default is ON.",
" ",
"SET WIN95 SELECT-BUG { ON, OFF }"
"  Some TCP/IP (Winsock) implementations for Windows have a defective",
"  select() function.  Use this command to avoid the use of select() if",
"  K95 appears to be unable to send data over TCP/IP.  Default is OFF.",
""};
#endif /* NT */

static char *hmxybel[] = {
#ifdef OS2
"Syntax: SET BELL { AUDIBLE [ { BEEP, SYSTEM-SOUNDS } ], VISIBLE, NONE }",
"  Specifies how incoming Ctrl-G (bell) characters are handled in CONNECT",
"  mode and how command warnings are presented in command mode.  AUDIBLE",
"  means either a beep or a system-sound is generated; VISIBLE means the",
"  screen is flashed momentarily.",
#else
"Syntax: SET BELL { OFF, ON }",
"  ON (the default) enables ringing of the terminal bell (beep) except where",
"  it is disabled in certain circumstances, e.g. by SET TRANSFER BELL.  OFF",
"  disables ringing of the bell in all circumstances, overriding any specific",
"  SET xxx BELL selections.",
#endif /* OS2 */
""};

static char *hmxycd[] = {
"Syntax: SET CD { PATH <path>, MESSAGE { ON, OFF, FILE <list> } }",
" ",
"SET CD PATH <path>",
"  Overrides normal CDPATH environment variable, which tells the CD command",
"  where to look for directories to CD to if you don't specify them fully.",
"  The format is:",
" ",
#ifdef UNIXOROSK
"    set cd path :directory:directory:...",
" ",
"  in other words, a list of directories separated by colons, with a colon",
"  at the beginning, e.g.:",
" ",
"    set cd path :/usr/olga:/usr/ivan/public:/tmp",
#else
#ifdef OS2
"    set cd path disk:directory;disk:directory;...",
" ",
"  just like the DOS PATH; in other words, a list of disk:directory names",
"  separated by semicolons, e.g.:",
" ",
"    SET CD PATH C:\\K95;C:\\HOME;C:\\LOCAL;C:\\",
#else
#ifdef VMS
"    set cd path directory,directory,...",
" ",
"  in other words, a list of directory specifications or logical names that",
"  represent them, e.g.:",
" ",
"    SET CD PATH SYS$LOGIN:,$DISK1:[OLGA],$DISK2[SCRATCH.IVAN].",
#else
"  (unknown for this platform)",
#endif /* VMS */
#endif /* OS2 */
#endif /* UNIXOROSK */
" ",
"SET CD MESSAGE { ON, OFF }",
"  Default is OFF.  When ON, this tells Kermit to look for a file with a",
"  certain name in any directory that you CD to, and if it finds one, to",
"  display it on your screen when you give the CD command.  The filename,",
"  or list of names, is given in the SET CD MESSAGE FILE command.",
" ",
"SET CD MESSAGE FILE name",
"  or:",
"SET CD MESSAGE FILE {{name1}{name2}...{name8}}",
"  Specify up to 8 filenames to look for when when CDing to a new directory",
"  and CD MESSAGE is ON.  The first one found, if any, in the new directory",
#ifndef DFCDMSG
"  is displayed.",
#else
"  is displayed.  The default list is:",
" ",
#ifdef UNIXOROSK
"   {{./.readme}{README.TXT}{READ.ME}}",
#else
"   {{README.TXT}{READ.ME}}",
#endif /* UNIXOROSK */
" ",
#endif /* DFCDMSG */
#ifndef NOSERVER
"Synonym: SET SERVER CD-MESSAGE FILE.",
#endif /* NOSERVER */
" ",
"Type SHOW CD to view current CD settings.  Also see HELP SET SERVER.",
""
};


/*  D O H S E T  --  Give help for SET command  */

int
dohset(xx) int xx; {
    int x;

    if (xx == -3) return(hmsga(hmhset));
    if (xx < 0) return(xx);
    if ((x = cmcfm()) < 0) return(x);
    switch (xx) {

case XYATTR:
    return(hmsga(hsetat));

case XYBACK:
    return(hmsga(hsetbkg));

case XYBELL:
    return(hmsga(hmxybel));

#ifdef OS2
case XYPRTY:
    return(hmsg("SET PRIORITY { REGULAR, FOREGROUND-SERVER, TIME-CRITICAL }\n\
  Specifies at which priority level the communication and screen update\n\
  threads should operate.  The default value is FOREGROUND-SERVER."));
#endif /* OS2 */

#ifdef DYNAMIC
case XYBUF:
    return(hmsga(hsetbuf));
#endif /* DYNAMIC */

#ifndef NOLOCAL
case XYCARR:
    return(hmsga(hsetcar));
#endif /* NOLOCAL */

#ifndef NOSPL
case XYCASE:
    return(hmsg("Syntax: SET CASE { ON, OFF }\n\
  Tells whether alphabetic case is significant in string comparisons\n\
  done by INPUT, IF, and other commands.  This setting is local to the\n\
  current macro or command file, and inherited by subordinates."));

#endif /* NOSPL */

case XYCMD:
    return(hmsga(hsetcmd));

case XYIFD:
    return(hmsg("Syntax: SET INCOMPLETE { DISCARD, KEEP }\n\
  Whether to discard or keep incompletely received files, default is KEEP."));

#ifndef NOSPL
case XYINPU:
    return(hmsga(hxyinp));
#endif /* NOSPL */

case XYCHKT:
    return(hmsga(hmxychkt));

#ifndef NOSPL
case XYCOUN:
    return(hmsg("Syntax:  SET COUNT number\n\
 Example: SET COUNT 5\n\
  Set up a loop counter, for use with IF COUNT.  Local to current macro\n\
  or command file, inherited by subordinate macros and command files."));
#endif /* NOSPL */

case XYDEBU:
    return(hmsga(hmxydeb));

case XYDFLT:
    return(hmsg("Syntax: SET DEFAULT directory\n\
  Change directory.  Equivalent to CD command."));

case XYDELA:
    return(hmsg("Syntax: SET DELAY number\n\
  Number of seconds to wait before sending first packet after SEND command."));

#ifndef NODIAL
case XYDIAL:
    return(hmsga(hmxydial));
#endif /* NODIAL */

#ifdef UNIX
case XYSUSP:
    return(hmsg("Syntax: SET SUSPEND { OFF, ON }\n\
  Disables SUSPEND command, suspend signals, and <esc-char>Z during CONNECT.")
           );
#endif

#ifndef NOSCRIPT
case XYSCRI:
    return(hmsg("Syntax: SET SCRIPT ECHO { OFF, ON }\n\
  Disables/Enables echoing of SCRIPT command operation."));
#endif /* NOSCRIPT */

case XYTAKE:
    return(hmsga(hxytak));

#ifndef NOLOCAL
case XYTERM:
    return(hmsga(hxyterm));

case XYDUPL:
    return(hmsg("Syntax: SET DUPLEX { FULL, HALF }\n\
  During CONNECT: FULL means remote host echoes, HALF means C-Kermit\n\
  does its own echoing."));

case XYLCLE:
    return(hmsg("Syntax: SET LOCAL-ECHO { OFF, ON }\n\
  During CONNECT: OFF means remote host echoes, ON means C-Kermit\n\
  does its own echoing.  Synonym for SET DUPLEX { FULL, HALF }."));

case XYESC:
    return(hmsga(hxyesc));              /* SET ESCAPE */
#endif /* NOLOCAL */

case XYPRTR:                            /* SET PRINTER */
    return(hmsga(hxyprtr));

#ifdef OS2
#ifdef BPRINT
case XYBDCP:                            /* SET BPRINTER */
    return(hmsga(hxybprtr));
#endif /* BPRINT */
#endif /* OS2 */

case XYEXIT:
    return(hmsga(hxyexit));

case XYFILE:
    return(hmsga(hmxyf));

case XYFLOW:
    return(hmsga(hmxyflo));

case XYHAND:
   return(hmsga(hmxyhsh));

#ifdef NETCONN

case XYHOST:
    return(hmsga(hxyhost));

case XYNET:
    return(hmsga(hxynet));

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
case XYTCP:
    return(hmsga(hxytcp));
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

#ifdef ANYX25
case XYX25:
    return(hmsga(hxyx25));

#ifndef IBMX25
case XYPAD:
    return(hmsg("Syntax: SET PAD name value\n\
Set a PAD X.3 parameter with a desired value."));
#endif /* IBMX25 */
#endif /* ANYX25 */
#endif /* NETCONN */

#ifndef NOSPL
case XYOUTP:
    return(hmsga(hxyout));
#endif /* NOSPL */

#ifndef NOSETKEY
case XYKEY:                             /* SET KEY */
    return(hmsga(hmhskey));
#endif /* NOSETKEY */

#ifndef NOCSETS
case XYLANG:
    return(hmsg("Syntax: SET LANGUAGE name\n\
  Selects language-specific translation rules for text-mode file transfers.\n\
  Used with SET FILE CHARACTER-SET and SET TRANSFER CHARACTER-SET when one\n\
  of these is ASCII."));
#endif /* NOCSETS */

case XYLINE:
#ifdef OS2
    printf("\nSyntax: SET LINE (or SET PORT) [ switches ] [ devicename ]\n\
  Selects communication device to use.\n");
#else
    printf("\nSyntax: SET LINE (or SET PORT) [ switches ] [ devicename ]\n\
  Selects communication device to use.  Default is %s.\n",dftty);
    if (!dfloc) {
        printf("\
  If you SET LINE to other than %s, then Kermit\n",dftty);
        printf("\
  will be in \"local mode\"; SET LINE alone resets Kermit to remote mode.\n\
  To use the modem to dial out, first SET MODEM TYPE (e.g., to HAYES), then");
        printf("\n\
  SET LINE xxx, then SET SPEED, and then give a DIAL command.\n\n");
    }
#endif /* OS2 */
    printf(
"Switches:\n\
  /CONNECT - Enter CONNECT mode automatically if SET LINE succeeds.\n");
    printf(
"  /SERVER  - Enter server mode automatically if SET LINE succeeds.\n");
#ifdef VMS
    printf(
"  /SHARE   - Open the device in shared mode.\n");
    printf(
"  /NOSHARE - Open the device in exclusive mode.\n");
#endif /* VMS */
    printf("\n");
    return(0);

#ifndef NOSPL
case XYMACR:
    return(hmsga(hxymacr));
#endif /* NOSPL */

#ifndef NODIAL
case XYMODM:
    return(hmsga(hxymodm));
#endif /* NODIAL */

case XYPARI:
    return(hmsga(hxypari));

case XYPROM:
    return(hmsga(hmxyprm));

case XYQUIE:
    return(hmsg("Syntax: SET QUIET {ON, OFF}\n\
  Normally OFF.  ON disables most information messages during interactive\n\
  operation."));

#ifdef CK_SPEED
case XYQCTL:
    return(hmsga(hmxyqctl));
#endif /* CK_SPEED */

case XYRETR:
    return(hmsg("Syntax: SET RETRY number\n\
  How many times to retransmit a particular packet before giving up."));

#ifndef NOLOCAL
#ifdef OS2ORUNIX
case XYSESS:
    return(hmsg("Syntax: SET SESSION-LOG { BINARY, DEBUG, TEXT }\n\
  If BINARY, record all CONNECT characters in session log.  If TEXT, strip\n\
  out carriage returns.  DEBUG is the same as BINARY but also includes\n\
  Telnet negotiations on TCP/IP connections."));
#else
#ifdef OSK
case XYSESS:
    return(hmsg("Syntax: SET SESSION-LOG { BINARY, TEXT }\n\
  If BINARY, record all CONNECT characters in session log.  If TEXT, strip\n\
  out linefeeds."));
#endif /* OSK */
#endif /* OS2ORUNIX */

case XYSPEE:
    return(hmsg("Syntax: SET SPEED number\n\
  Communication line speed for external tty line specified in most recent\n\
  SET LINE command, in bits per second.  Type SET SPEED ? for a list of\n\
  possible speeds."));
#endif /* NOLOCAL */

#ifndef NOXFER
case XYRECV:
    return(hmsga(hxyrcv));
case XYSEND:
    return(hmsga(hxysnd));
case XYREPT:
    return(hmsga(hxyrpt));
#endif /* NOXFER */

#ifndef NOSERVER
case XYSERV:
    return(hmsga(hsetsrv));
#endif /* NOSERVER */

#ifdef TNCODE
case XYTEL:
    return(hmsga(hxytel));

case XYTELOP:
    return(hmsga(hxytopt));
#endif /* TNCODE */

#ifndef NOXMIT
case XYXMIT:
    return(hmsga(hsetxmit));
#endif /* NOXMIT */

#ifndef NOCSETS
case XYUNCS:
    return(hmsg("Syntax: SET UNKNOWN-CHAR-SET action\n\
  DISCARD (default) means reject any arriving files encoded in unknown\n\
  character sets.  KEEP means to accept them anyway."));
#endif /* NOCSETS */

#ifdef UNIX
case XYWILD:
    return(hmsga(hxywild));
#endif /* UNIX */

#ifndef NOXFER
case XYWIND:
    return(hmsga(hxywind));
case XYXFER:
    return(hmsga(hxyxfer));
#endif /* NOXFER */

#ifdef OS2MOUSE
case XYMOUSE:
    return(hmsga(hxymouse));
#endif /* OS2MOUSE */

case XYALRM:
    return(hmsg("Syntax: SET ALARM [ { seconds, hh:mm:ss } ]\n\
  Number of seconds from now, or time of day, after which IF ALARM\n\
  will succeed.  0, or no time at all, means no alarm."));

case XYPROTO:
    return(hmsga(hxyxyz));

#ifdef CK_SPEED
case XYPREFIX:
    return(hmsg("Syntax: SET PREFIXING { ALL, CAUTIOUS, MINIMAL }\n\
  \
Selects the degree of control-character prefixing.  Also see HELP SET CONTROL."
));
#endif /* CK_SPEED */

#ifdef OS2
case XYLOGIN:
    return(hmsg("Syntax: SET LOGIN { USERID, PASSWORD, PROMPT } <text>\n\
  Provides access information for use by login scripts."));
#endif /* OS2 */

#ifndef NOSPL
case XYTMPDIR:
    return(hmsg("Syntax: SET TEMP-DIRECTORY [ <directory-name> ]\n\
  Overrides automatic assignment of \\v(tmpdir) variable."));
#endif /* NOSPL */

#ifdef OS2
case XYTITLE:
    return(hmsg("Syntax: SET TITLE <text>\n\
  Sets window title to text instead of using current host/port name."));
#endif /* OS2 */

#ifndef NOPUSH
#ifndef NOFRILLS
case XYEDIT:
    return(hmsg("Syntax: SET EDITOR pathname [ options ]\n\
  Specifies the name of your preferred editor, plus any command-line\n\
  options.  SHOW EDITOR displays it."));
#endif /* NOFRILLS */
#endif /* NOPUSH */

#ifdef BROWSER
case XYBROWSE:
#ifdef NT
    return(hmsg("Syntax: SET BROWSER [ pathname [ options ] ]\n\
  Specifies the name of your preferred browser plus any command-line\n\
  options.  SHOW BROWSER displays it.  Omit pathname and options to use\n\
  ShellExecute."));
#else
    return(hmsg("Syntax: SET BROWSER [ pathname [ options ] ]\n\
  Specifies the name of your preferred browser plus any command-line\n\
  options.  SHOW BROWSER displays it."));
#endif /* NT */
#endif /* BROWSER */

#ifdef CK_TAPI
case XYTAPI:
    return(hmsga(hxytapi));
#endif /* CK_TAPI */

#ifdef NT
case XYWIN95:
    return(hmsga(hxywin95));
#endif /* NT */

#ifndef NOSPL
case XYFUNC:
    return(hmsga(hxyfunc));
#endif /* NOSPL */

#ifdef CK_AUTHENTICATION
case XYAUTH:
    return(hmsga(hmxyauth));
#else /* CK_AUTHENTICATION */
#ifdef CK_SSL
case XYAUTH:
    return(hmsga(hmxyauth));
#endif /* CK_SSL */
#endif /* CK_AUTHENTICATION */

#ifdef BROWSER
case XYFTP:
    return(hmsg("Syntax: SET FTP [ pathname [ options ] ]\n\
  Specifies the name of your ftp client, plus any command-line options.\n\
  SHOW FTP displays it."));
#endif /* BROWSER */

case XYSLEEP:
    return(hmsg("Syntax: SET SLEEP CANCELLATION { ON, OFF }\n\
  Tells whether SLEEP (PAUSE) or WAIT commands can be interrupted from the\n\
  keyboard; ON by default."));

case XYCD:
    return(hmsga(hmxycd));

case XYSERIAL:
    return(hmsg("Syntax: SET SERIAL dps\n\
  d is data length in bits, 7 or 8; p is first letter of parity; s is stop\n\
  bits, 1 or 2.  Examples: \"set serial 7e1\", \"set serial 8n1\"."));

#ifdef HWPARITY
case XYSTOP:
    return(hmsg("Syntax: SET STOP-BITS { 1, 2 }\n\
  Number of stop bits to use on SET LINE connections, normally 1."));
#endif /* HWPARITY */

#ifndef NOLOCAL
case XYDISC:
    return(hmsg("Syntax: SET DISCONNECT { ON, OFF }\n\
  Whether to close and release a SET LINE device automatically upon\n\
  disconnection; OFF = keep device open (default); ON = close and release."));
#endif /* NOLOCAL */

#ifdef STREAMING
case XYSTREAM:
    return(hmsg("Syntax: SET STREAMING { ON, OFF, AUTO }\n\
  Tells C-Kermit whether streaming protocol can be used during Kermit file\n\
  transfers.  Default is AUTO, meaning use it if connection is reliable."));
#endif /* STREAMING */

case XYRELY:
    return(hmsg("Syntax: SET RELIABLE { ON, OFF, AUTO }\n\
  Tells C-Kermit whether its connection is reliable.  Default is AUTO,\n\
  meaning C-Kermit should figure it out for itself."));

case XYCLEAR:
    return(hmsg("Syntax: SET CLEAR-CHANNEL { ON, OFF, AUTO }\n\
  Tells C-Kermit whether its connection is transparent to all 8-bit bytes.\n\
  Default is AUTO, meaning C-Kermit figures it out from the connection type.\n\
  When both sender and receiver agree channel is clear, SET PREFIXING NONE\n\
  is used automatically."));

#ifdef TLOG
case XYTLOG:
    return(hmsg("Syntax: SET TRANSACTION-LOG { BRIEF, FTP, VERBOSE }\n\
  Selects the transaction-log format; BRIEF and FTP have one line per file;\n\
  FTP is compatible with FTP log.  VERBOSE (the default) has more info."));
#endif /* TLOG */

case XYOPTS:
    return(hmsg("Syntax: SET OPTIONS command [ switches... ]\n\
  For use with commands that have switches; sets the default switches for\n\
  the given command.  Type SET OPTIONS ? for a list of amenable commands."));

#ifndef NOSPL
case XYTIMER:
    return(hmsga(hmxytimer));
#endif /* NOSPL */

default:
    printf("Not available - \"%s\"\n",cmdbuf);
    return(0);
    }
}

#ifndef NOSPL
/*  D O H F U N C  --  Give help for a function  */

int
dohfunc(xx) int xx; {
    /* int x; */
    if (xx == -3) {
        printf("\n Type SHOW FUNCTIONS to see a list of available functions.\n"
               );
        printf(
        " Type HELP FUNCTION <name> for help on a particular function.\n");
        printf(
        " For function settings use HELP SET FUNCTION and SHOW SCRIPTS.\n\n");
        return(0);
    }
    printf("\n");
    switch (xx) {
      case FN_IND:                      /* Index (of string 1 in string 2) */
      case FN_RIX:                      /* Rindex (index from right) */
        printf("\\f%sindex(s1,s2,n1)\n\
  s1 = string to look for.\n\
  s2 = string to look in.\n\
  n1 = optional 1-based starting position, default = 1.\n",
               xx == FN_RIX ? "r" : ""
               );
        printf("Returns integer:\n\
  1-based position of %smost occurrence of s1 in s2, ignoring the %smost\n\
  (n1-1) characters in s2; returns 0 if s1 not found in s2.\n",
               xx == FN_IND ? "left" : "right",
               xx == FN_IND ? "left" : "right"
        );
        break;
      case FN_SEARCH:			/* Search for pattern */
      case FN_RSEARCH:			/* Search for pattern from right */
        printf("\\f%ssearch(s1,s2,n1)\n\
  s1 = pattern to look for.\n\
  s2 = string to look in.\n\
  n1 = optional 1-based offset, default = 1.\n",
               xx == FN_RSEARCH ? "r" : ""
               );
        printf("Returns integer:\n\
  1-based position of %smost match for s1 in s2, ignoring the %smost\n\
  (n1-1) characters in s2; returns 0 if no match.\n",
               xx == FN_SEARCH ? "left" : "right",
               xx == FN_SEARCH ? "left" : "right"
        );
	printf("  See HELP WILDCARDS for info about patterns.\n");
        break;
      case FN_LEN:                      /* Length (of string) */
        printf("\\flength(s1)\n\
  s1 = string.\n");
        printf("Returns integer:\n\
  Length of string s1.\n");
        break;
      case FN_LIT:                      /* Literal (don't expand the string) */
        printf("\\fliteral(s1)\n\
  s1 = string.\n");
        printf("Returns string:\n\
  s1 literally without evaluation.\n");
        break;
      case FN_LOW:                      /* Lower (convert to lowercase) */
        printf("\\flower(s1)\n\
  s1 = string.\n");
        printf("Returns string:\n\
  s1 with uppercase letters converted to lowercase.\n");
        break;
      case FN_MAX:                      /* Max (maximum) */
        printf("\\fmaximum(n1,n2)\n\
  n1 = integer.\n\
  n2 = integer.\n");
        printf("Returns integer:\n\
  The greater of n1 and n2.\n");
        break;
      case FN_MIN:                      /* Min (minimum) */
        printf("\\fminimum(n1,n2)\n\
  n1 = integer.\n\
  n2 = integer.\n");
        printf("Returns integer:\n\
  The lesser of n1 and n2.\n");
        break;
      case FN_MOD:                      /* Mod (modulus) */
        printf("\\fmodulus(n1,n2)\n\
  n1 = integer.\n\
  n2 = integer.\n");
        printf("Returns integer:\n\
  The remainder after dividing n1 by n2.\n");
        break;
      case FN_EVA:                      /* Eval (evaluate arith expression) */
        printf("\\fevaluate(e)\n\
  e = arithmetic expression.\n");
        printf("Returns integer:\n\
  The result of evaluating the expression.\n");
        break;
      case FN_SUB:                      /* Substr (substring) */
        printf("\\fsubstring(s1,n1,n2)\n\
  s1 = string.\n\
  n1 = integer, 1-based starting position, default = 1.\n\
  n2 = integer, length, default = length(s1) - n1 + 1.\n");
        printf("Returns string:\n\
  Substring of s1 starting at n1, length n2.\n");
        break;
      case FN_UPP:                      /* Upper (convert to uppercase) */
        printf("\\fupper(s1)\n\
  s1 = string.\n");
        printf("Returns string:\n\
  s1 with lowercase letters converted to uppercase.\n");
        break;
      case FN_REV:                      /* Reverse (a string) */
        printf("\\freverse(s1)\n\
  s1 = string.\n");
        printf("Returns string:\n\
  s1 with its characters in reverse order.\n");
        break;
      case FN_REP:                      /* Repeat (a string) */
        printf("\\frepeat(s1,n1)\n\
  s1 = string.\n\
  n1 = integer.\n");
        printf("Returns string:\n\
  s1 repeated n1 times.\n");
        break;
      case FN_EXE:                      /* Execute (a macro) */
        printf("\\fexecute(m1,a1,a2,a3,...)\n\
  m1 = macro name.\n\
  a1 = argument 1.\n\
  a2 = argument 2, etc\n");
        printf("Returns string:\n\
  The return value of the macro (HELP RETURN for further info).\n");
        break;
      case FN_LPA:                      /* LPAD (left pad) */
      case FN_RPA:                      /* RPAD (right pad) */
        printf("\\f%cpad(s1,n1,c1)\n\
  s1 = string.\n\
  n1 = integer.\n\
  c1 = character, default = space.\n",
                xx == FN_LPA ? 'l' : 'r');
        printf("Returns string:\n\
  s1 %s-padded with character c1 to length n1.\n",
               xx == FN_LPA ? "left" : "right");
        break;
      case FN_DEF:                      /* Definition of a macro, unexpanded */
        printf("\\fdefinition(m1)\n\
  m1 = macro name.\n");
        printf("Returns string:\n\
  Literal definition of macro m1.\n");
        break;
      case FN_CON:                      /* Contents of a variable, ditto */
        printf("\\fcontents(v1)\n\
  v1 = variable name such as \\%%a.\n");
        printf("Returns string:\n\
  Literal definition of variable v1, evaluated one level only.\n");
        break;
      case FN_FIL:                      /* Next file */
        printf("\\fnextfile()\n");
        printf("Returns string:\n\
  Name of next file from list created by most recent \\f[r]files() or\n\
  \\f[r]dir()invocation, or an empty string if there are no more files in\n\
  the list.\n");
        break;
      case FN_FC:                       /* File count */
        printf("\\ffiles(f1[,&a]) - File list.\n\
  f1 = file specification, possibly containing wildcards.\n\
  &a = optional name of array to assign file list to.\n");
        printf("Returns integer:\n\
  The number of regular files that match f1.  Use with \\fnextfile().\n");
        break;
      case FN_CHR:                      /* Character (like BASIC CHR$()) */
        printf("\\fcharacter(n1)\n\
  n1 = integer.\n");
        printf("Returns character:\n\
  The character whose numeric code is n1.\n");
        break;
      case FN_RIG:                      /* Right (like BASIC RIGHT$()) */
        printf("\\fright(s1,n1)\n\
  s1 = string.\n\
  n1 = integer, default = length(s1).\n");
        printf("Returns string:\n\
  The rightmost n1 characters of string s1.\n");
        break;
      case FN_LEF:                      /* Left (like BASIC LEFT$()) */
	printf("\\fleft(s1,n1)\n\
  s1 = string.\n\
  n1 = integer, default = length(s1).\n");
	printf("Returns string:\n\
  The leftmost n1 characters of string s1.\n");
	break;
      case FN_COD:                      /* Code value of character */
        printf("\\fcode(c1)\n\
  c1 = character.\n");
        printf("Returns integer:\n\
  The numeric code of character c1.\n");
        break;
      case FN_RPL:                      /* Replace */
        printf("\\freplace(s1,s2,s3)\n\
  s1 = original string.\n\
  s2 = match string.\n\
  s3 = replacement string.\n");
        printf("Returns string:\n\
  s1 with all occurrences of s2 replaced by s3.\n");
        break;
      case FN_FD:                       /* File date */
        printf("\\fdate(f1)\n\
  f1 = filename.\n");
#ifdef VMS
        printf("Returns string:\n\
  Creation date of file f1, format: yyyymmdd hh:mm:ss.\n");
#else
        printf("Returns string:\n\
  Modification date of file f1, format: yyyymmdd hh:mm:ss.\n");
#endif /* VMS */
        break;
      case FN_FS:                       /* File size */
        printf("\\fsize(f1)\n\
  f1 = filename.\n");
        printf("Returns integer:\n\
  Size of file f1.\n");
        break;
      case FN_VER:                      /* Verify */
        printf("\\fverify(s1,s2,n1)\n\
  s1 = string to look in.\n\
  s2 = string of characters to look for.\n\
  n1 = starting position in s1.");
        printf("Returns integer:\n\
  1-based position of first character in s1 that is not also in s2,\n\
  or 0 if all characters in s2 are also in s1.\n");
        break;
      case FN_IPA:                      /* Find and return IP address */
        printf("\\fipaddress(s1,n1)\n\
  s1 = string.\n\
  n1 = 1-based integer starting position, default = 1.\n");
        printf("Returns string:\n\
  First IP address in s1, scanning from left starting at position n1.\n");
        break;
      case FN_HEX:                      /* Hexify */
        printf("\\fhexify(s1)\n\
  s1 = string.\n");
        printf("Returns string:\n\
  The hexadecimal representation of s1.  Also see \\fn2hex().\n");
        break;
      case FN_UNH:                      /* Unhexify */
        printf("\\funhexify(h1)\n\
  h1 = Hexadecimal string.\n");
        printf("Returns string:\n\
  The result of unhexifying s1, or nothing if s1 is not a hex string.\n");
        break;
      case FN_BRK:                      /* Break */
      case FN_SPN:                      /* Span */
        printf("\\f%s(s1,s2,n1)\n\
  s1 = string to look in.\n\
  s2 = string of characters to look for.\n\
  n1 = 1-based integer starting position, default = 1.\n",
              xx == FN_BRK ? "break" : "span"
              );
        printf("Returns string:\n\
  s1 up to the first occurrence of any character%salso in s2,\n\
  scanning from the left starting at position n1.\n",
               xx == FN_SPN ? " not " : " ");
        break;
      case FN_TRM:                      /* Trim */
      case FN_LTR:                      /* Left-Trim */
        printf("\\f%s(s1,s2)\n\
  s1 = string to look in.\n\
  s2 = string of characters to look for, default = blanks and tabs.\n",
               xx == FN_TRM ? "trim" : "ltrim");
        printf("Returns string:\n\
  s1 with all characters that are also in s2 trimmed from the %s.\n.",
               xx == FN_TRM ? "right" : "left");
        break;
      case FN_CAP:                      /* Capitalize */
        printf("\\fcapitalize(s1)\n\
  s1 = string.\n");
        printf("Returns string:\n\
  s1 with its first letter converted to uppercase and the remaining\n\
  letters to lowercase.\n");
        printf("Synonym: \\fcaps(s1)\n");
        break;
      case FN_TOD:                      /* Time-of-day-to-secs-since-midnite */
        printf("\\ftod2secs(s1)\n\
  s1 = time-of-day string, hh:mm:ss, 24-hour format.\n");
        printf("Returns number:\n\
  Seconds since midnight.\n");
        break;
      case FN_FFN:                      /* Full file name */
        printf("\\fpathname(f1)\n\
  f1 = filename, possibly wild.\n");
        printf("Returns string:\n\
  Full pathname of f1.\n");
        break;
      case FN_CHK:                      /* Checksum of text */
        printf("\\fchecksum(s1)\n\
  s1 = string.\n");
        printf("Returns integer:\n\
  16-bit checksum of string s1.\n");
        break;
      case FN_CRC:                      /* CRC-16 of text */
        printf("\\fcrc16(s1)\n\
  s1 = string.\n");
        printf("Returns integer:\n\
  16-bit cyclic redundancy check of string s1.\n");
        break;
      case FN_BSN:                      /* Basename of file */
        printf("\\fbasename(f1)\n\
  f1 = filename, possibly wild.\n");
        printf("Returns string:\n\
  Filename f1 stripped of all device and directory information.\n");
        break;
      case FN_CMD:                      /* Output of a command (cooked) */
        printf("\\fcommand(s1)\n\
  s1 = string\n");
        printf("Returns string:\n\
  Output of system command s1, if any, with final line terminator stripped.\n"
               );
        break;
      case FN_RAW:                      /* Output of a command (raw) */
        printf("\\frawcommand(s1)\n\
  s1 = string\n");
        printf("Returns string:\n\
  Output of system command s1, if any.\n");
        break;
      case FN_STX:                      /* Strip from right */
        printf("\\fstripx(s1,c1)\n\
  s1 = string to look in.\n\
  c1 = character to look for, default = \".\".\n");
        printf("Returns string:\n\
  s1 up to the rightmost occurrence of character c1.\n"
        );
        break;

      case FN_STL:                      /* Strip from left */
        printf("\\flop(s1,c1)\n\
  s1 = string to look in.\n\
  c1 = character to look for, default = \".\".\n");
        printf("Returns string:\n\
  The part of s1 after the leftmost occurrence of character c1.\n"
        );
        break;

      case FN_STN:                      /* Strip n chars */
        printf("\\fstripn(s1,n1)\n\
  s1 = string to look in.\n\
  n1 = integer, default = 0.\n");
        printf("Returns string:\n\
  s1 with n1 characters removed from the right.\n"
        );
        break;

      case FN_STB:                      /* Strip enclosing brackets */
        printf("\\fstripb(s1[,c1[,c2]])\n\
  s1 = original string.\n\
  c1 = optional first character\n");
	printf("\
  c2 = optional final character.\n");
        printf("Returns string:\n\
  s1 with the indicated enclosing characters removed.  If c1 and c2 not\n\
     specified, any matching brackets, braces, parentheses, or quotes are\n");
	printf("\
     assumed.  If c1 is given but not c2, the appropriate c2 is assumed.\n\
     if both c1 and c2 are given, they are used as-is.\n"
        );
        break;

#ifdef OS2
      case FN_SCRN_CX:                  /* Screen Cursor X Pos */
        printf("\\fscrncurx()\n");
        printf("Returns integer:\n\
  The 0-based X coordinate (column) of the Terminal screen cursor.\n");
        break;
      case FN_SCRN_CY:                  /* Screen Cursor Y Pos */
        printf("\\fscrncury()\n");
        printf("Returns integer:\n\
  The 0-based Y coordinate (row) of the Terminal screen cursor.\n");
        break;
      case FN_SCRN_STR:                 /* Screen String */
        printf("\\fscrstr(ny,nx,n1)\n\
  ny = integer.\n\
  nx = integer.\n\
  n1 = integer.\n");
        printf("Returns string:\n\
  The string at Terminal-screen coordinates (nx,ny), length n1,\n\
  blanks included.\n");
        break;
#endif /* OS2 */

      case FN_2HEX:                     /* Num to hex */
        printf("\\fn2hex(n1) - Number to hex\n  n1 = integer.\n");
        printf("Returns string:\n  The hexadecimal representation of n1.\n");
        break;

      case FN_2OCT:                     /* Num to hex */
        printf("\\fn2octal(n1) - Number to octal\n  n1 = integer.\n");
        printf("Returns string:\n  The octal representation of n1.\n");
        break;

#ifdef RECURSIVE
      case FN_DIR:                      /* Recursive directory count */
        printf("\\fdirectories(f1) - Directory list.\n\
  f1 = directory specification, possibly containing wildcards.\n\
  &a = optional name of array to assign directory list to.\n");
        printf("Returns integer:\n\
  The number of directories that match f1; use with \\fnextfile().\n");
        break;

      case FN_RFIL:                     /* Recursive file count */
        printf("\\frfiles(f1[,&a]) - Recursive file list.\n\
  f1 = file specification, possibly containing wildcards.\n\
  &a = optional name of array to assign file list to.\n");
        printf("Returns integer:\n\
  The number of files whose names match f1 in the current or given\n\
  directory tree; use with \\fnextfile().\n");
        break;

      case FN_RDIR:                     /* Recursive directory count */
        printf("\\frdirectories(f1) - Recursive directory list.\n\
  f1 = directory specification, possibly containing wildcards.\n\
  &a = optional name of array to assign directory list to.\n");
        printf("Returns integer:\n\
  The number of directories that match f1 in the current or given directory\n\
  tree.  Use with \\fnextfile().\n");
        break;
#endif /* RECURSIVE */

      case FN_DNAM:                     /* Directory part of a filename */
        printf("\\fdirname(f) - Directory part of a filename.\n\
  f = a file specification.\n");
        printf("Returns directory name:\n\
  The full name of the directory that the file is in, or if the file is a\n\
  directory, its full name.\n");
        break;

#ifndef NORANDOM
      case FN_RAND:                     /* Random number */
        printf("\\frandom(n) - Random number.\n\
  n = a positive integer.\n");
        printf("Returns integer:\n\
  A random number between 0 and n-1.\n");
        break;
#endif /* NORANDOM */

      case FN_WORD:                     /* Word */
        printf("\\fword(s1,n1,s2,s3) - Extract word from string.\n\
  s1 = String\n  n1 = word number (1-based)\n  s2 = optional break set.\n");
        printf("  s3 = optional include set.\n");
        printf("\
  Default break set is all characters except ASCII letters and digits.\n\
  ASCII (C0) control characters are always treated as break characters.\n"
        );
        printf("Returns string:\n\
  Word number n, if there is one, otherwise an empty string.\n");
        break;

      case FN_SPLIT:                    /* Split */
        printf("\\fsplit(s1,&a,s2,s3) - Assign string words to an array.\n\
  s1 = String\n  &a = array designator\n  s2 = optional break set.\n");
        printf("  s3 = optional include set.\n");
        printf("  Break and include sets are as in \\fword().\n");
        printf(
"  All arguments are optional.  If \\&a[] not declared, it is created.\n");
        printf("Returns integer:\n\
  Number of words assigned.\n");
        break;

      case FN_DTIM:                     /* CVTDATE */
        printf("\\fcvtdate([date-time]) - Date/time conversion.\n");
        printf("  Converts date and/or time to standard format.\n");
        printf("  If no date/time given, returns current date/time.\n");
        printf("  [date-time], if given, is free-format date and/or time.\n");
        printf("  HELP DATE for info about date-time formats.\n");
        printf("Returns string:\n\
  Standard-format date and time: yyyymmdd_hh:mm:ss\n");
        break;

      case FN_JDATE:                    /* DOY */
        printf("\\fdoy([date-time]) - Day of Year.\n");
        printf("  Converts date and/or time to day-of-year (DOY) format.\n");
        printf("  If no date/time given, returns current date.\n");
        printf("  [date-time], if given, is free-format date and/or time.\n");
        printf("  HELP DATE for info about date-time formats.\n");
        printf("Returns numeric string:\n\
  DOY: yyyyddd, where ddd is 1-based day number in year.\n");
        break;

      case FN_PNCVT:
        printf("\\fdialconvert(phone-number) - Convert phone number.\n");
        printf("  Converts the given phone number for dialing according\n");
        printf(
"  to the prevailing dialing rules -- country code, area code, etc.\n");
        printf("Returns string:\n\
  The dial string that would be used if the same phone number had been\n\
  given to the DIAL command.\n"
              );
        break;

      case FN_DATEJ:                    /* JDATE */
        printf("\\fdoy2date([doy[-time]]) - Day of Year to Date.\n");
        printf("  Converts yyyymmm to yyyymmdd\n");
        printf("  If time included, it is converted to 24-hour format.");
        printf(
            "Returns standard date or date-time string yyyymmdd hh:mm:ss\n");
        break;

      case FN_MJD:
        printf("\\fmjd([[date][-time]]) - Modified Julian Date (MJD).\n");
        printf(
"  Converts date and/or time to MJD, the number of days since 17 Nov 1858.\n");
        printf("  HELP DATE for info about date-time formats.\n");
        printf("Returns: integer.\n");
        break;

      case FN_MJD2:
        printf("\\fmjd2date(mjd) - Modified Julian Date (MJD) to Date.\n");
        printf("  Converts MJD to standard-format date.\n");
        printf("Returns: yyyymmdd.\n");
        break;

      case FN_DAY:
        printf("\\fday([[date][-time]]) - Day of Week.\n");
        printf("Returns day of week of given date as Mon, Tue, etc.\n");
        printf("HELP DATE for info about date-time formats.\n");
        break;

      case FN_NDAY:
        printf("\\fnday([[date][-time]]) - Numeric Day of Week.\n");
        printf(
    "Returns numeric day of week of given date, 0=Sun, 1=Mon, ..., 6=Sat.\n");
        printf("HELP DATE for info about date-time formats.\n");
        break;

      case FN_TIME:
        printf("\\ftime([[date][-time]]) - Time.\n");
        printf(
"Returns time portion of given date and/or time in hh:mm:ss format.\n");
        printf("If no argument given, returns current time.\n");
        printf("HELP DATE for info about date-time formats.\n");
        break;

      case FN_NTIM:
        printf("\\fntime([[date][-time]]) - Numeric Time.\n");
        printf(
"Returns time portion of given date and/or time as seconds since midnight.\n");
        printf("If no argument given, returns current time.\n");
        printf("HELP DATE for info about date-time formats.\n");
        break;

      case FN_N2TIM:
        printf("\\fn2time(seconds) - Numeric Time to Time.\n");
        printf(
"Returns the given number of seconds in hh:mm:ss format.\n");
        break;

      case FN_PERM:
        printf("\\fpermissions(file) - Permissions of File.\n");
        printf(
#ifdef UNIX
"Returns permissions of given file as they would be shown by \"ls -l\".\n"
#else
#ifdef VMS
"Returns permissions of given file as they would be shown by \"dir /prot\".\n"
#else
"Returns the permissions of the given file.\n"
#endif /* VMS */
#endif /* UNIX */
               );
        break;

      case FN_ALOOK:
        printf("\\farraylook(pattern,&a) - Lookup pattern in array.\n\
  pattern = String or pattern\n");
	printf("  &a = array designator, can include range specifier.\n");
        printf(
"Returns number:\n\
  The index of the first matching array element or -1 if none.\n");
	printf("More info:\n\
  HELP WILDCARD for pattern syntax.\n  HELP ARRAY for arrays.\n");
        break;

      case FN_TLOOK:
        printf(
"\\ftablelook(keyword,&a,[c]) - Lookup keyword in keyword table.\n\
  pattern = String\n");
        printf("  keyword = keyword to look up (can be abbreviated).\n");
	printf("  &a      = array designator, can include range specifier.\n");
        printf("            This array must be in alphabetical order.\n");
        printf("  c       = Optional field delimiter, colon(:) by default.\n");
        printf(
"Returns number:\n\
  1 or greater, index of array element that uniquely matches given keyword;\n"
	       );
        printf(
"or -2 if keyword was ambiguous, or -1 if keyword empty or not found.\n"
	       );
	printf("Also see:\n\
  HELP FUNC ARRAYLOOK for a similar function.\n  HELP ARRAY for arrays.\n");
        break;

      case FN_ABS:                      /* Absolute */
        printf("\\fabsolute(n1)\n\
  n1 = integer.\n");
        printf("Returns integer:\n\
  The absolute (unsigned) value of n1.\n");
        break;

#ifdef FNFLOAT
      case FN_FPABS:
        printf("\\ffpabsolute(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  The absolute (unsigned) value of f1 to d decimal places.\n");
        break;

      case FN_FPADD:
        printf("\\ffpadd(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
        printf("Returns floating-point number:\n\
  The sum of f1 and f2 to d decimal places.\n");
        break;

      case FN_FPSUB:
        printf("\\ffpsubtract(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
        printf("Returns floating-point number:\n\
  f1 minus f2 to d decimal places.\n");
        break;

      case FN_FPMUL:
        printf("\\ffpmultiply(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
        printf("Returns floating-point number:\n\
  The product of f1 and f2 to d decimal places.\n");
        break;

      case FN_FPDIV:
        printf("\\ffpdivide(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
        printf("Returns floating-point number:\n\
  f1 divided by f2 to d decimal places.\n");
        break;

      case FN_FPMAX:
        printf("\\ffpmaximum(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
        printf("Returns floating-point number:\n\
  The maximum of f1 and f2 to d decimal places.\n");
        break;

      case FN_FPMIN:
        printf("\\ffpminimum(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
        printf("Returns floating-point number:\n\
  The minimum of f1 and f2 to d decimal places.\n");
        break;

      case FN_FPMOD:
        printf("\\ffpmodulus(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
        printf("Returns floating-point number:\n\
  The modulus of f1 and f2 to d decimal places.\n");
        break;

      case FN_FPPOW:
        printf("\\ffpraise(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
        printf("Returns floating-point number:\n\
  f1 raised to the power f2, to d decimal places.\n");
        break;

      case FN_FPCOS:
        printf("\\ffpcosine(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  The cosine of angle f1 (in radians) to d decimal places.\n");
        break;

      case FN_FPSIN:
        printf("\\ffpsine(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  The sine of angle f1 (in radians) to d decimal places.\n");
        break;

      case FN_FPTAN:
        printf("\\ffptangent(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  The tangent of angle f1 (in radians) to d decimal places.\n");
        break;

      case FN_FPEXP:
        printf("\\ffpexp(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  e (the base of natural logarithms) raised to the f1 power,\n\
  to d decimal places.\n");
        break;

      case FN_FPINT:
        printf("\\ffpint(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns integer:\n\
  The integer part of f1.\n");
        break;

      case FN_FPLOG:
        printf("\\ffplog10(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  The logarithm, base 10, of f1 to d decimal places.\n");
        break;

      case FN_FPLN:
        printf("\\ffplogn(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  The natural logarithm of f1 to d decimal places.\n");
        break;

      case FN_FPROU:
        printf("\\ffpround(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  f1 rounded to d decimal places.\n");
        break;

      case FN_FPSQR:
        printf("\\ffpsqrt(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
        printf("Returns floating-point number:\n\
  The square root of f1 to d decimal places.\n");
        break;
#endif /* FNFLOAT */

#ifdef CKCHANNELIO
      case FN_FEOF:
        printf("\\f_eof(n1)\n\
  n1 = channel number.\n");
        printf("Returns number:\n\
  1 if channel n1 at end of file, 0 otherwise.\n");
	break;
      case FN_FPOS:
        printf("\\f_pos(n1)\n\
  n1 = channel number.\n");
        printf("Returns number:\n\
  Read/write pointer of channel n1 as byte number.\n");
	break;
      case FN_NLINE:
        printf("\\f_line(n1)\n\
  n1 = channel number.\n");
        printf("Returns number:\n\
  Read/write pointer of channel n1 as line number.\n");
	break;
      case FN_FILNO:
        printf("\\f_handle(n1)\n\
  n1 = channel number.\n");
        printf("Returns number:\n\
  File %s of open file on channel n1.\n",
#ifdef OS2
	       "handle"
#else
	       "descriptor"
#endif /* OS2 */
	       );
	break;
      case FN_FSTAT:
        printf("\\f_status(n1)\n\
  n1 = channel number.\n");
        printf("Returns number:\n\
  Sum of open modes of channel n1: 1 = read; 2 = write; 4 = append, or:\n\
  0 if not open.\n");
	break;
      case FN_FGCHAR:
        printf("\\f_getchar(n1)\n\
  n1 = channel number.\n");
        printf("  Reads a character from channel n1 and returns it.\n");
	break;
      case FN_FGLINE:
        printf("\\f_getline(n1)\n\
  n1 = channel number.\n");
        printf("  Reads a line from channel n1 and returns it.\n");
	break;
      case FN_FGBLK:
        printf("\\f_getblock(n1,n2)\n\
  n1 = channel number, n2 = size\n");
        printf(
"  Reads a block of n2 characters from channel n1 and returns it.\n");
	break;
      case FN_FPCHAR:
        printf("\\f_putchar(n1,c)\n\
  n1 = channel number, c = character\n");
        printf("  Writes a character to channel n1.\n\
Returns number:\n\
  1 if successful, otherwise a negative error code.\n");
	break;
      case FN_FPLINE:
        printf("\\f_putline(n1,s1)\n\
  n1 = channel number, s1 = string\n");
        printf(
"  Writes the string s1 to channel n1 and adds a line terminator.\n\
Returns number:\n");
	printf("  How many characters written if successful;\n\
  Otherwise a negative error code.\n"
	       );
	break;
      case FN_FPBLK:
        printf("\\f_putblock(n1,s1)\n\
  n1 = channel number, s1 = string\n");
        printf(
"  Writes the string s1 to channel n1.\n\
  Returns number:\n");
	printf("  How many characters written if successful;\n\
  Otherwise a negative error code.\n"
	       );
	break;
      case FN_FERMSG:
        printf("\\f_errmsg([n1])\n\
  n1 = numeric error code, \\v(f_error) by default.\n");
        printf("  Returns the associated error message string.\n");
	break;
#endif /* CKCHANNELIO */

      case FN_AADUMP:
        printf("\\faaconvert(name,&a[,&b])\n\
  name = name of associative array, &a and &b = names of regular arrays.\n");
        printf(
"  Converts the given associative array into two regular arrays, &a and &b,\n\
  containing the indices and values, respectively:\n");
        printf("Returns number:\n\
  How many elements were converted.\n");
	break;

#ifdef CK_KERBEROS
      case FN_KRB_TK:
	printf("\\fkrbtickets(n)\n\
  n = Kerberos version number (4 or 5).\n\
  Returns string:\n\
  The number of active Kerberos 4 or 5 tickets.\n\
  Resets the ticket list used by \\fkrbnextticket(n).\n");
	break;

      case FN_KRB_NX:
	printf("\\fkrbnextticket(n)\n\
  n = Kerberos version number (4 or 5).\n\
  Returns string:\n\
    The next ticket in the Kerberos 4 or 5 ticket list that was set up by\n\
    the most recent invocation of \\fkrbtickets(n).\n");
	break;

      case FN_KRB_IV:
	printf("\\fkrbisvalid(n,name)\n\
  n    = Kerberos version number (4 or 5).\n\
  name = a ticket name as returned by \\fkrbnextticket(n).\n\
  Returns number:\n\
    1 if the ticket is valid, 0 if not valid.\n\
    A ticket is valid if all the following conditions are true:\n\n");
	printf("\n\
    (i)   it exists in the current cache file;\n\
    (ii)  it is not expired;\n\
    (iii) it is not marked invalid (K5 only);\n\
    (iv)  it was issued from the current IP address\n");
	printf("\n  This value can be used in an IF statement, e.g.:\n\n");
	printf("    if \\fkrbisvalid(4,krbtgt.FOO.BAR.EDU@FOO.BAR.EDU) ...\n");
	break;

      case FN_KRB_TT:
	printf("\\fkrbtimeleft(n,name)\n\
  n    = Kerberos version number (4 or 5).\n\
  name = a ticket name as returned by \\fkrbnextticket(n).\n\
  Returns string:\n\
    The number of seconds remaining in the ticket's lifetime.\n");
	break;

      case FN_KRB_FG:
	printf("\\fkrbflags(n,name)\n\
  n    = Kerberos version number (4 or 5).\n\
  name = a ticket name as returned by \\fkrbnextticket(n).\n\
  Returns string:\n");
	printf(
"    The flags string as reported with AUTH K5 LIST /FLAGS.  This string can\n\
    be searched for a particular flag using the \\findex() function when\n\
    SET CASE is ON (for case sensitive searches).  Flag strings are only\n\
    available for K5 tickets.\n");
	break;
#endif /* CK_KERBEROS */

      case FN_PATTERN:
	printf("\\fpattern(s)\n\
  s = string\n\
  Returns string: s with any variables, etc, evaluated in the normal manner.\n"
	       );
	printf("\
  For use with INPUT, MINPUT, and REINPUT to declare that a search target is\n\
  a regular expression rather than a literal string.\n");
	break;

      case FN_HEX2N:
	printf("\\fhex2n(s)\n\
  s = hexadecimal number\n\
  Returns decimal equivalent.\n");

      case FN_HEX2IP:
	printf("\\fhex2ip(s)\n\
  s = 8-digit hexadecimal number\n\
  Returns the equivalent decimal dotted IP address.\n");

      case FN_IP2HEX:
	printf("\\fip2hex(s)\n\
  s = decimal dotted IP address\n\
  Returns the equivalent 8-digit hexadecimal number.\n");

      case FN_OCT2N:
	printf("\\foct2n(s)\n\
  s = octal number\n\
  Returns decimal equivalent.\n");

      case FN_RADIX:
	printf("\\fradix(s,n1,n2)\n\
  s = number in radix n1\n\
  Returns the number's representation in radix n2.\n");

      default:
        printf("Sorry, help not available for \"%s\"\n",cmdbuf);
    }
   printf("\n");
   return(0);
}
#endif /* NOSPL */

#ifdef OS2
#ifndef NOKVERBS

/*  D O H K V E R B  --  Give help for a Kverb  */

int
dohkverb(xx) int xx; {
    int x,i,found,button,event;

    if (xx == -3) {
        printf("\n Type SHOW KVERBS to see a list of available Kverbs.\n"
               );
        printf(
" Type HELP KVERB <name> to see the current definition of a given Kverb.\n\n"
              );
        return(-9);
    }
    if (xx < 0) return(xx);
    if ((x = cmcfm()) < 0) return(x);
    switch ( xx ) {
        /* DEC VT keyboard key definitions */

    case  K_COMPOSE :                   /* Compose key */
        printf("\\Kcompose           Compose an accented character\n");
        break;

/* DEC arrow keys */

    case  K_UPARR     :                 /* DEC Up Arrow key */
        printf("\\Kuparr         Transmit Terminal Up Arrow sequence\n");
        break;
    case  K_DNARR     :                 /* DEC Down Arrow key */
        printf("\\Kdnarr         Transmit Terminal Down Arrow sequence\n");
        break;
    case  K_RTARR     :                 /* DEC Right Arrow key */
        printf("\\Krtarr         Transmit Terminal Right Arrow sequence\n");
        break;
    case  K_LFARR     :                 /* DEC Left Arrow key */
        printf("\\Klfarr         Transmit Terminal Left Arrow sequence\n");
        break;

    case  K_PF1       :                 /* DEC PF1 key */
        printf("\\Kpf1,\\Kgold    Transmit DEC PF1 sequence\n");
        break;
    case  K_PF2       :                 /* DEC PF2 key */
        printf("\\Kpf2           Transmit DEC PF2 sequence\n");
        break;
    case  K_PF3       :                 /* DEC PF3 key */
        printf("\\Kpf3           Transmit DEC PF3 sequence\n");
        break;
    case  K_PF4       :                 /* DEC PF4 key */
        printf("\\Kpf4           Transmit DEC PF4 sequence\n");
        break;

    case  K_KP0       :                 /* DEC Keypad 0 */
        printf("\\Kkp0           Transmit DEC Keypad-0 sequence\n");
        break;
    case  K_KP1       :                 /* DEC Keypad 1 */
        printf("\\Kkp1           Transmit DEC Keypad-1 sequence\n");
        break;
    case  K_KP2       :                 /* etc ... through 9 */
        printf("\\Kkp2           Transmit DEC Keypad-2 sequence\n");
        break;
    case  K_KP3       :
        printf("\\Kkp3           Transmit DEC Keypad-3 sequence\n");
        break;
    case  K_KP4       :
        printf("\\Kkp4           Transmit DEC Keypad-4 sequence\n");
        break;
    case  K_KP5       :
        printf("\\Kkp5           Transmit DEC Keypad-5 sequence\n");
        break;
    case  K_KP6       :
        printf("\\Kkp6           Transmit DEC Keypad-6 sequence\n");
        break;
    case  K_KP7       :
        printf("\\Kkp7           Transmit DEC Keypad-7 sequence\n");
        break;
    case  K_KP8       :
        printf("\\Kkp8           Transmit DEC Keypad-8 sequence\n");
        break;
    case  K_KP9       :
        printf("\\Kkp9           Transmit DEC Keypad-9 sequence\n");
        break;
    case  K_KPCOMA    :                 /* DEC keypad comma */
        printf("\\Kkpcoma        Transmit DEC Keypad-Comma sequence\n");
        break;
    case  K_KPMINUS   :                 /* DEC keypad minus */
        printf("\\Kkpminus       Transmit DEC Keypad-Minus sequence\n");
        break;
    case  K_KPDOT     :                 /* DEC keypad period */
        printf("\\Kkpdot         Transmit DEC Keypad-Period sequence\n");
        break;
    case  K_KPENTER   :                 /* DEC keypad enter */
        printf("\\Kkpenter       Transmit DEC Keypad-Enter sequence\n");
        break;

/* DEC Top-Rank F keys */

    case  K_DECF1     :                 /* DEC F1 key */
        printf("\\Kdecf1         Transmit DEC F1 sequence for PC keyboard\n");
        break;
    case  K_DECF2     :                 /* DEC F2 key */
        printf("\\Kdecf2         Transmit DEC F2 sequence for PC keyboard\n");
        break;
    case  K_DECF3     :                 /* DEC F3 key */
        printf("\\Kdecf3         Transmit DEC F3 sequence for PC keyboard\n");
        break;
    case  K_DECF4     :                 /* DEC F4 key */
        printf("\\Kdecf4         Transmit DEC F4 sequence for PC keyboard\n");
        break;
    case  K_DECF5     :                 /* DEC F5 key */
        printf("\\Kdecf5         Transmit DEC F5 sequence for PC keyboard\n");
        break;
    case  K_DECHOME:                    /* DEC HOME key */
       printf("\\Kdechome       Transmit DEC HOME sequence for PC keyboard\n");
       break;

    case  K_DECF6     :                 /* DEC F6 key */
        printf("\\Kdecf6         Transmit DEC F6 sequence\n");
        break;
    case  K_DECF7     :                 /* etc, through F20 */
        printf("\\Kdecf7         Transmit DEC F7 sequence\n");
        break;
    case  K_DECF8     :
        printf("\\Kdecf8         Transmit DEC F8 sequence\n");
        break;
    case  K_DECF9     :
        printf("\\Kdecf9         Transmit DEC F9 sequence\n");
        break;
    case  K_DECF10    :
        printf("\\Kdecf10        Transmit DEC F10 sequence\n");
        break;
    case  K_DECF11    :
        printf("\\Kdecf11        Transmit DEC F11 sequence\n");
        break;
    case  K_DECF12    :
        printf("\\Kdecf12        Transmit DEC F12 sequence\n");
        break;
    case  K_DECF13    :
        printf("\\Kdecf13        Transmit DEC F13 sequence\n");
        break;
    case  K_DECF14    :
        printf("\\Kdecf14        Transmit DEC F14 sequence\n");
        break;
    case  K_DECHELP   :                 /* DEC Help key */
        printf("\\Kdecf15,\\Kdechelp  Transmit DEC HELP sequence\n");
        break;
    case  K_DECDO     :                 /* DEC Do key */
        printf("\\Kdecf16,\\Kdecdo    Transmit DEC DO sequence\n");
        break;
    case  K_DECF17    :
        printf("\\Kdecf17        Transmit DEC F17 sequence\n");
        break;
    case  K_DECF18    :
        printf("\\Kdecf18        Transmit DEC F18 sequence\n");
        break;
    case  K_DECF19    :
        printf("\\Kdecf19        Transmit DEC F19 sequence\n");
        break;
    case  K_DECF20    :
        printf("\\Kdecf20        Transmit DEC F20 sequence\n");
        break;

/* DEC editing keys */

    case  K_DECFIND   :                 /* DEC Find key */
        printf("\\Kdecfind       Transmit DEC FIND sequence\n");
        break;
    case  K_DECINSERT :                 /* DEC Insert key */
        printf("\\Kdecinsert     Transmit DEC INSERT HERE sequence\n");
        break;
    case  K_DECREMOVE :                 /* DEC Remove key */
        printf("\\Kdecremove     Transmit DEC REMOVE sequence\n");
        break;
    case  K_DECSELECT :                 /* DEC Select key */
        printf("\\Kdecfselect    Transmit DEC SELECT sequence\n");
        break;
    case  K_DECPREV   :                 /* DEC Previous Screen key */
        printf("\\Kdecprev       Transmit DEC PREV SCREEN sequence\n");
        break;
    case  K_DECNEXT   :                 /* DEC Next Screen key */
        printf("\\Kdecnext       Transmit DEC NEXT SCREEN sequence\n");
        break;

/* DEC User-Defined Keys */

    case  K_UDKF1     :                 /* F1 - F5 are XTERM extensions */
      printf("\\Kudkf1         Transmit XTERM F1 User Defined Key sequence\n");
      break;
    case  K_UDKF2     :
      printf("\\Kudkf2         Transmit XTERM F2 User Defined Key sequence\n");
      break;
    case  K_UDKF3     :
      printf("\\Kudkf3         Transmit XTERM F3 User Defined Key sequence\n");
      break;
    case  K_UDKF4     :
      printf("\\Kudkf4         Transmit XTERM F4 User Defined Key sequence\n");
      break;
    case  K_UDKF5     :
      printf("\\Kudkf5         Transmit XTERM F5 User Defined Key sequence\n");
      break;
    case  K_UDKF6     :                 /* DEC User Defined Key F6 */
      printf("\\Kudkf6         Transmit DEC F6 User Defined Key sequence\n");
      break;
    case  K_UDKF7     :                 /* DEC User Defined Key F7 */
      printf("\\Kudkf7         Transmit DEC F7 User Defined Key sequence\n");
      break;
    case  K_UDKF8     :                 /* etc ... through F20 */
      printf("\\Kudkf8         Transmit DEC F8 User Defined Key sequence\n");
      break;
    case  K_UDKF9     :
      printf("\\Kudkf9         Transmit DEC F9 User Defined Key sequence\n");
      break;
    case  K_UDKF10    :
      printf("\\Kudkf10        Transmit DEC F10 User Defined Key sequence\n");
      break;
    case  K_UDKF11    :
      printf("\\Kudkf11        Transmit DEC F11 User Defined Key sequence\n");
      break;
    case  K_UDKF12    :
      printf("\\Kudkf12        Transmit DEC F12 User Defined Key sequence\n");
      break;
    case  K_UDKF13    :
      printf("\\Kudkf13        Transmit DEC F13 User Defined Key sequence\n");
      break;
    case  K_UDKF14    :
      printf("\\Kudkf14        Transmit DEC F14 User Defined Key sequence\n");
      break;
    case  K_UDKHELP   :
      printf(
      "\\Kudkhelp,\\Kudkf15  Transmit DEC HELP User Defined Key sequence\n");
      break;
    case  K_UDKDO     :
      printf(
      "\\Kudkdo,\\Kudkf16    Transmit DEC DO User Defined Key sequence\n");
      break;
    case  K_UDKF17    :
      printf("\\Kudkf17        Transmit DEC F17 User Defined Key sequence\n");
      break;
    case  K_UDKF18    :
      printf("\\Kudkf18        Transmit DEC F18 User Defined Key sequence\n");
      break;
    case  K_UDKF19    :
      printf("\\Kudkf19        Transmit DEC F19 User Defined Key sequence\n");
      break;
    case  K_UDKF20    :
      printf("\\Kudkf20        Transmit DEC F20 User Defined Key sequence\n");
      break;

/* Emacs Keys */
    case  K_EMACS_OVER:
      printf(
     "\\Kemacs_overwrite  Transmit EMACS Overwrite toggle command sequence\n");
      break;

/* Kermit screen-scrolling keys */

    case  K_DNONE     :                 /* Screen rollback: down one line */
      printf("\\Kdnone         Screen rollback: down one line\n");
      break;
    case  K_DNSCN     :                 /* Screen rollback: down one screen */
      printf("\\Kdnscn         Screen rollback: down one screen\n");
      break;
    case  K_UPONE     :                 /* Screen rollback: Up one line */
      printf("\\Kupone         Screen rollback: up one line\n");
      break;
    case  K_UPSCN     :                 /* Screen rollback: Up one screen */
      printf("\\Kupscn         Screen rollback: up one screen\n");
      break;
    case  K_ENDSCN    :                 /* Screen rollback: latest screen */
      printf("\\Kendscn        Screen rollback: latest screen\n");
      break;
    case  K_HOMSCN    :                 /* Screen rollback: oldest screen */
      printf("\\Khomscn        Screen rollback: oldest screen\n");
      break;
    case  K_GO_BOOK   :         /* Scroll to bookmark */
      printf("\\Kgobook        Screen rollback: go to bookmark\n");
      break;
    case  K_GOTO      :         /* Scroll to line number */
      printf("\\Kgoto          Screen rollback: go to line number\n");
      break;

    case  K_LFONE     :                 /* Horizontal Scroll: Left one cell */
      printf("\\Klfone         Horizontal Scroll: Left one column\n");
      break;
    case  K_LFPAGE    :                 /* Horizontal Scroll: Left one page */
      printf("\\Klfpage        Horizontal Scroll: Left eight columns\n");
      break;
    case  K_LFALL     :
      printf("\\Klfall         Horizontal Scroll: Left to margin\n");
      break;
    case  K_RTONE     :                 /* Horizontal Scroll: Right one cell */
      printf("\\Krtone         Horizontal Scroll: Right one column\n");
      break;
    case  K_RTPAGE    :                 /* Horizontal Scroll: Right one page */
      printf("\\Krtpage        Horizontal Scroll: Right eight columns\n");
      break;
    case  K_RTALL     :
      printf("\\Krtall         Horizontal Scroll: Right to margin\n");
      break;

/* Keyboard language switching verbs */

    case  K_KB_ENG    :                 /* English keyboard mode */
      printf("\\Kkbenglish     Switch to Normal (English) keyboard mode\n");
      break;
    case  K_KB_HEB    :                 /* Hebrew keyboard mode */
      printf("\\Kkbhebrew      Switch to Hebrew keyboard mode\n");
      break;
    case  K_KB_RUS    :                 /* Russian keyboard mode */
      printf("\\Kkbrussian     Switch to Russian keyboard mode\n");
      break;
    case  K_KB_EMA    :                 /* Emacs keyboard mode */
      printf("\\Kkbemacs       Switch to EMACS keyboard mode\n");
      break;
    case  K_KB_WP     :                 /* Word Perfect 5.1 mode */
      printf("\\Kkbwp          Switch to Word Perfect 5.1 keyboard mode\n");
      break;

/* Mark Mode actions */

    case  K_MARK_START  :       /* Enter Mark Mode/Start marking */
      printf("\\Kmarkstart     Mark Mode: Enter mode or Start marking\n");
      break;
    case  K_MARK_CANCEL :       /* Exit Mark Mode - Do Nothing */
      printf("\\Kmarkcancel    Mark Mode: Cancel mode\n");
      break;
    case  K_MARK_COPYCLIP:      /* Exit Mark Mode - Copy data to clipboard */
      printf("\\Kmarkcopyclip  Mark Mode: Copy marked text to clipboard\n");
      break;
    case  K_MARK_COPYHOST:      /* Exit Mark Mode - Copy data to host   */
      printf("\\Kmarkcopyhost  Mark Mode: Copy marked text to host\n");
      break;
    case  K_MARK_SELECT :       /* Exit Mark Mode - Select */
      printf(
      "\\Kmarkselect    Mark Mode: Place marked text into \v(select)\n");
      break;
    case  K_BACKSRCH    :            /* Search Backwards for text */
      printf("\\Kbacksearch    Search: Begin backward search for text\n");
      break;
    case  K_FWDSRCH     :            /* Search Forwards for text */
      printf("\\Kfwdsearch     Search: Begin forward search for text\n");
      break;
    case  K_BACKNEXT    :     /* Search Backwards for next instance of text */
      printf(
      "\\Kbacknext      Search: Find next instance of text backwards\n");
      break;
    case  K_FWDNEXT     :      /* Search Forwards for next instance of text */
      printf("\\Kfwdnext       Search: Find next instance of text forwards\n");
      break;

/* Miscellaneous Kermit actions */

    case  K_EXIT        :               /* Return to command parser */
      printf("\\Kexit          Toggle between COMMAND and CONNECT modes\n");
      break;
    case  K_BREAK       :               /* Send a BREAK */
      printf("\\Kbreak         Transmit BREAK signal to host\n");
      break;
    case  K_RESET       :               /* Reset emulator */
      printf("\\Kreset         Reset Terminal Emulator to user defaults\n");
      break;
    case  K_DOS         :               /* Push to DOS (i.e. OS/2) */
      printf("\\Kdos,\\Kos2     Push to Command Shell\n");
      break;
    case  K_HANGUP      :               /* Hang up the connection */
      printf("\\Khangup        Hangup the active connection\n");
      break;
    case  K_DUMP        :               /* Dump/Print current screen */
      printf(
     "\\Kdump          Dump/copy current screen to SET PRINTER device/file\n");
      break;
    case  K_LBREAK      :               /* Send a Long BREAK */
      printf("\\Klbreak        Transmit LONG BREAK signal to host\n");
      break;
    case  K_NULL        :               /* Send a NUL */
      printf("\\Knull          Transmit NULL ('\0') character to host\n");
      break;
    case  K_HELP        :               /* Pop-up help */
      printf("\\Khelp          Raise Pop-Up help display\n");
      break;
    case  K_HOLDSCRN    :               /* Hold screen */
      printf("\\Kholdscrn      Pause data input during CONNECT mode\n");
      break;
    case  K_IGNORE      :               /* Ignore this key, don't even beep */
      printf("\\Kignore        Ignore key\n");
      break;

    case  K_LOGOFF      :               /* Turn off session logging */
      printf("\\Klogoff        Turn off session logging (see \\Ksession)\n");
      break;
    case  K_LOGON       :               /* Turn on session logging */
      printf("\\Klogon         Turn on session logging (see \\Ksession)\n");
      break;
    case K_SESSION:
      printf(
         "\\Ksession       Toggle on/off session logging to 'session.log'\n");
      break;
    case K_AUTODOWN:
      printf("\\Kautodown      Toggle on/off terminal autodownload.\n");
      break;
    case K_BYTESIZE:
      printf(
        "\\Kbytesize      Toggle terminal bytesize between 7 and 8 bits.\n");
      break;

#ifdef COMMENT
    case MODELINE:
    case  K_NETHOLD     :               /* Put network connection on hold */
    case  K_NEXTSESS    :               /* Toggle to next network session */
#endif /* COMMENT */

    case  K_STATUS      :               /* Show status */
      printf(
     "\\Kstatus        Toggle statusline (None, Indicator, Host Writeable)\n");
      break;
    case  K_TERMTYPE    :               /* Toggle term type: text/graphics */
      printf("\\Ktermtype      Toggle Terminal Type\n");
      break;
    case  K_PRTCTRL     :               /* Print Controller mode */
      printf("\\Kprtctrl       Toggle Ctrl-Print (transparent) mode\n");
      break;
    case  K_PRINTFF     :               /* Print formfeed */
      printf("\\Kprintff       Output Form Feed to SET PRINTER device\n");
      break;
    case  K_FLIPSCN     :               /* Flip screen */
      printf("\\Kflipscn       Reverse foreground and background colors\n");
      break;
    case  K_DEBUG       :               /* Toggle debugging */
      printf("\\Kdebug         Toggle Terminal Debug mode\n");
      break;
    case  K_TN_AO       :               /* TELNET Cancel Output */
      printf("\\Ktn_ao         TELNET: Transmit Cancel-Output request\n");
      break;
    case  K_TN_AYT      :               /* TELNET Are You There */
      printf("\\Ktnayt         TELNET: Transmit Are You There? request\n");
      break;
    case  K_TN_EC       :               /* TELNET Erase Character */
      printf("\\Ktn_ec         TELNET: Transmit Erase Character request\n");
      break;
    case  K_TN_EL       :               /* TELNET Erase Line */
      printf("\\Ktn_el         TELNET: Transmit Erase Line request\n");
      break;
    case  K_TN_GA       :               /* TELNET Go Ahead */
      printf("\\Ktn_ga         TELNET: Transmit Go Ahead request\n");
      break;
    case  K_TN_IP       :               /* TELNET Interrupt Process */
      printf("\\Ktn_ip         TELNET: Transmit Interrupt Process request\n");
      break;
    case  K_TN_LOGOUT   :               /* TELNET Logout */
      printf("\\Ktn_logout     TELNET: Transmit Do Logout Option\n");
      break;
    case  K_TN_NAWS   :                 /* TELNET NAWS */
      printf(
        "\\Ktn_naws       TELNET: Transmit Window Size if NAWS is active\n");
      break;
    case  K_PASTE       :               /* Paste data from clipboard */
      printf("\\Kpaste         Paste data from clipboard to host\n");
      break;
    case  K_CLRSCRN     :               /* Clear Terminal Screen */
      printf("\\Kclearscreen   Clear the Terminal screen\n");
      break;
    case  K_PRTAUTO     :               /* Print Auto mode */
      printf("\\Kprtauto       Toggle Auto-Print mode\n");
      break;
    case  K_PRTCOPY     :               /* Print Copy mode */
      printf("\\Kprtcopy       Toggle Copy-Print mode\n");
      break;
    case  K_ANSWERBACK  :               /* Transmit Answerback String */
      printf("\\Kanswerback    Transmit answerback string to host\n");
      break;
    case  K_SET_BOOK    :               /* Set Bookmark */
      printf("\\Ksetbook       Set bookmark\n");
      break;
    case  K_QUIT        :               /* Quit Kermit */
      printf("\\Kquit          Hangup connection and quit kermit\n");
      break;
    case  K_KEYCLICK    :               /* Toggle Keyclick */
      printf("\\Kkeyclick      Toggle Keyclick mode\n");
      break;
    case  K_LOGDEBUG    :               /* Toggle Debug Log File */
      printf("\\Klogdebug      Toggle Debug Logging to File\n");
      break;
    case  K_FNKEYS      :               /* Show Function Key Labels */
      printf("\\Kfnkeys        Display Function Key Labels\n");
      break;

#ifdef OS2MOUSE
/* Mouse only Kverbs */

    case  K_MOUSE_CURPOS :
      printf("\\Kmousecurpos   Mouse: Move host cursor to position\n");
      break;
    case  K_MOUSE_MARK   :
      printf(
     "\\Kmousemark     Mouse: Mark text for selection (drag event only)\n");
      break;
    case  K_MOUSE_URL    :
      printf("\\Kmouseurl      Mouse: Start browser with selected URL\n");
      break;
#endif /* OS2MOUSE */

/* ANSI Function Key definitions */
    case  K_ANSIF01          :
      printf("\\Kansif01       Transmit SCOANSI/AT386: F1 \n");
      break;
    case  K_ANSIF02          :
      printf("\\Kansif02       Transmit SCOANSI/AT386: F2 \n");
      break;
    case  K_ANSIF03          :
      printf("\\Kansif03       Transmit SCOANSI/AT386: F3 \n");
      break;
    case  K_ANSIF04          :
      printf("\\Kansif04       Transmit SCOANSI/AT386: F4 \n");
      break;
    case  K_ANSIF05          :
      printf("\\Kansif05       Transmit SCOANSI/AT386: F5 \n");
      break;
    case  K_ANSIF06          :
      printf("\\Kansif06       Transmit SCOANSI/AT386: F6 \n");
      break;
    case  K_ANSIF07          :
      printf("\\Kansif07       Transmit SCOANSI/AT386: F7 \n");
      break;
    case  K_ANSIF08          :
      printf("\\Kansif08       Transmit SCOANSI/AT386: F8 \n");
      break;
    case  K_ANSIF09          :
      printf("\\Kansif09       Transmit SCOANSI/AT386: F9 \n");
      break;
    case  K_ANSIF10          :
      printf("\\Kansif10       Transmit SCOANSI/AT386: F10\n");
      break;
    case  K_ANSIF11          :
      printf("\\Kansif11       Transmit SCOANSI/AT386: F11\n");
      break;
    case  K_ANSIF12          :
      printf("\\Kansif12       Transmit SCOANSI/AT386: F12\n");
      break;
    case  K_ANSIF13          :
      printf("\\Kansif13       Transmit SCOANSI/AT386: Shift-F1 \n");
      break;
    case  K_ANSIF14          :
      printf("\\Kansif14       Transmit SCOANSI/AT386: Shift-F2 \n");
      break;
    case  K_ANSIF15          :
      printf("\\Kansif15       Transmit SCOANSI/AT386: Shift-F3 \n");
      break;
    case  K_ANSIF16          :
      printf("\\Kansif16       Transmit SCOANSI/AT386: Shift-F4 \n");
      break;
    case  K_ANSIF17          :
      printf("\\Kansif17       Transmit SCOANSI/AT386: Shift-F5 \n");
      break;
    case  K_ANSIF18          :
      printf("\\Kansif18       Transmit SCOANSI/AT386: Shift-F6 \n");
      break;
    case  K_ANSIF19          :
      printf("\\Kansif19       Transmit SCOANSI/AT386: Shift-F7 \n");
      break;
    case  K_ANSIF20          :
      printf("\\Kansif20       Transmit SCOANSI/AT386: Shift-F8 \n");
      break;
    case  K_ANSIF21          :
      printf("\\Kansif21       Transmit SCOANSI/AT386: Shift-F9 \n");
      break;
    case  K_ANSIF22          :
      printf("\\Kansif22       Transmit SCOANSI/AT386: Shift-F10\n");
      break;
    case  K_ANSIF23          :
      printf("\\Kansif23       Transmit SCOANSI/AT386: Shift-F11\n");
      break;
    case  K_ANSIF24          :
      printf("\\Kansif24       Transmit SCOANSI/AT386: Shift-F12\n");
      break;
    case  K_ANSIF25          :
      printf("\\Kansif25       Transmit SCOANSI/AT386: Ctrl-F1 \n");
      break;
    case  K_ANSIF26          :
      printf("\\Kansif26       Transmit SCOANSI/AT386: Ctrl-F2 \n");
      break;
    case  K_ANSIF27          :
      printf("\\Kansif27       Transmit SCOANSI/AT386: Ctrl-F3 \n");
      break;
    case  K_ANSIF28          :
      printf("\\Kansif28       Transmit SCOANSI/AT386: Ctrl-F4 \n");
      break;
    case  K_ANSIF29          :
      printf("\\Kansif29       Transmit SCOANSI/AT386: Ctrl-F5 \n");
      break;
    case  K_ANSIF30          :
      printf("\\Kansif30       Transmit SCOANSI/AT386: Ctrl-F6 \n");
      break;
    case  K_ANSIF31          :
      printf("\\Kansif31       Transmit SCOANSI/AT386: Ctrl-F7 \n");
      break;
    case  K_ANSIF32          :
      printf("\\Kansif32       Transmit SCOANSI/AT386: Ctrl-F8 \n");
      break;
    case  K_ANSIF33          :
      printf("\\Kansif33       Transmit SCOANSI/AT386: Ctrl-F9 \n");
      break;
    case  K_ANSIF34          :
      printf("\\Kansif34       Transmit SCOANSI/AT386: Ctrl-F10\n");
      break;
    case  K_ANSIF35          :
      printf("\\Kansif35       Transmit SCOANSI/AT386: Ctrl-F11\n");
      break;
    case  K_ANSIF36          :
      printf("\\Kansif36       Transmit SCOANSI/AT386: Ctrl-F12\n");
      break;
    case  K_ANSIF37          :
      printf("\\Kansif37       Transmit SCOANSI/AT386: Ctrl-Shift-F1 \n");
      break;
    case  K_ANSIF38          :
      printf("\\Kansif38       Transmit SCOANSI/AT386: Ctrl-Shift-F2 \n");
      break;
    case  K_ANSIF39          :
      printf("\\Kansif39       Transmit SCOANSI/AT386: Ctrl-Shift-F3 \n");
      break;
    case  K_ANSIF40          :
      printf("\\Kansif40       Transmit SCOANSI/AT386: Ctrl-Shift-F4 \n");
      break;
    case  K_ANSIF41          :
      printf("\\Kansif41       Transmit SCOANSI/AT386: Ctrl-Shift-F5 \n");
      break;
    case  K_ANSIF42          :
      printf("\\Kansif42       Transmit SCOANSI/AT386: Ctrl-Shift-F6 \n");
      break;
    case  K_ANSIF43          :
      printf("\\Kansif43       Transmit SCOANSI/AT386: Ctrl-Shift-F7 \n");
      break;
    case  K_ANSIF44          :
      printf("\\Kansif44       Transmit SCOANSI/AT386: Ctrl-Shift-F8 \n");
      break;
    case  K_ANSIF45          :
      printf("\\Kansif45       Transmit SCOANSI/AT386: Ctrl-Shift-F9 \n");
      break;
    case  K_ANSIF46          :
      printf("\\Kansif46       Transmit SCOANSI/AT386: Ctrl-Shift-F10\n");
      break;
    case  K_ANSIF47          :
      printf("\\Kansif47       Transmit SCOANSI/AT386: Ctrl-Shift-F11\n");
      break;
    case  K_ANSIF48          :
      printf("\\Kansif48       Transmit SCOANSI/AT386: Ctrl-Shift-F12\n");
      break;
    case  K_ANSIF49          :
      printf("\\Kansif49       Transmit SCOANSI/AT386: Home\n");
      break;
    case  K_ANSIF50          :
      printf("\\Kansif50       Transmit SCOANSI/AT386: Up Arrow\n");
      break;
    case  K_ANSIF51          :
      printf("\\Kansif51       Transmit SCOANSI/AT386: PgUp\n");
      break;
    case  K_ANSIF52          :
      printf("\\Kansif52       Transmit SCOANSI/AT386: Ctrl-Shift-Subtract\n");
      break;
    case  K_ANSIF53          :
      printf("\\Kansif53       Transmit SCOANSI/AT386: Left Arrow\n");
      break;
    case  K_ANSIF54          :
      printf("\\Kansif54       Transmit SCOANSI/AT386: Clear\n");
      break;
    case  K_ANSIF55          :
      printf("\\Kansif55       Transmit SCOANSI/AT386: Right Arrow\n");
      break;
    case  K_ANSIF56          :
      printf("\\Kansif56       Transmit SCOANSI/AT386: Shift-Add\n");
      break;
    case  K_ANSIF57          :
      printf("\\Kansif57       Transmit SCOANSI/AT386: End\n");
      break;
    case  K_ANSIF58          :
      printf("\\Kansif58       Transmit SCOANSI/AT386: Down Arrow\n");
      break;
    case  K_ANSIF59          :
      printf("\\Kansif59       Transmit SCOANSI/AT386: PgDn\n");
      break;
    case  K_ANSIF60          :
      printf("\\Kansif60       Transmit SCOANSI/AT386: Insert\n");
      break;
    case  K_ANSIF61          :
      printf("\\Kansif61       Transmit SCOANSI/AT386: (not named)\n");
      break;

/* WYSE Function Keys (unshifted) */
    case  K_WYF01            :
      printf("\\Kwyf01         Transmit WYSE 30/50/60/160: F1\n");
      break;
    case  K_WYF02            :
      printf("\\Kwyf02         Transmit WYSE 30/50/60/160: F2\n");
      break;
    case  K_WYF03            :
      printf("\\Kwyf03         Transmit WYSE 30/50/60/160: F3\n");
      break;
    case  K_WYF04            :
      printf("\\Kwyf04         Transmit WYSE 30/50/60/160: F4\n");
      break;
    case  K_WYF05            :
      printf("\\Kwyf05         Transmit WYSE 30/50/60/160: F5\n");
      break;
    case  K_WYF06            :
      printf("\\Kwyf06         Transmit WYSE 30/50/60/160: F6\n");
      break;
    case  K_WYF07            :
      printf("\\Kwyf07         Transmit WYSE 30/50/60/160: F7\n");
      break;
    case  K_WYF08            :
      printf("\\Kwyf08         Transmit WYSE 30/50/60/160: F8\n");
      break;
    case  K_WYF09            :
      printf("\\Kwyf09         Transmit WYSE 30/50/60/160: F9\n");
      break;
    case  K_WYF10            :
      printf("\\Kwyf10         Transmit WYSE 30/50/60/160: F10\n");
      break;
    case  K_WYF11            :
      printf("\\Kwyf11         Transmit WYSE 30/50/60/160: F11\n");
      break;
    case  K_WYF12            :
      printf("\\Kwyf12         Transmit WYSE 30/50/60/160: F12\n");
      break;
    case  K_WYF13            :
      printf("\\Kwyf13         Transmit WYSE 30/50/60/160: F13\n");
      break;
    case  K_WYF14            :
      printf("\\Kwyf14         Transmit WYSE 30/50/60/160: F14\n");
      break;
    case  K_WYF15            :
      printf("\\Kwyf15         Transmit WYSE 30/50/60/160: F15\n");
      break;
    case  K_WYF16            :
      printf("\\Kwyf16         Transmit WYSE 30/50/60/160: F16\n");
      break;
    case  K_WYF17            :
      printf("\\Kwyf17         Transmit WYSE 30/50/60/160: F17\n");
      break;
    case  K_WYF18            :
      printf("\\Kwyf18         Transmit WYSE 30/50/60/160: F18\n");
      break;
    case  K_WYF19            :
      printf("\\Kwyf19         Transmit WYSE 30/50/60/160: F19\n");
      break;
    case  K_WYF20            :
      printf("\\Kwyf20         Transmit WYSE 30/50/60/160: F20\n");
      break;

/* WYSE Function Keys (shifted) */
    case  K_WYSF01           :
      printf("\\Kwysf01        Transmit WYSE 30/50/60/160: Shift-F1\n");
      break;
    case  K_WYSF02           :
      printf("\\Kwysf02        Transmit WYSE 30/50/60/160: Shift-F2\n");
      break;
    case  K_WYSF03            :
      printf("\\Kwysf03        Transmit WYSE 30/50/60/160: Shift-F3\n");
      break;
    case  K_WYSF04            :
      printf("\\Kwysf04        Transmit WYSE 30/50/60/160: Shift-F4\n");
      break;
    case  K_WYSF05            :
      printf("\\Kwysf05        Transmit WYSE 30/50/60/160: Shift-F5\n");
      break;
    case  K_WYSF06            :
      printf("\\Kwysf06        Transmit WYSE 30/50/60/160: Shift-F6\n");
      break;
    case  K_WYSF07            :
      printf("\\Kwysf07        Transmit WYSE 30/50/60/160: Shift-F7\n");
      break;
    case  K_WYSF08            :
      printf("\\Kwysf08        Transmit WYSE 30/50/60/160: Shift-F8\n");
      break;
    case  K_WYSF09            :
      printf("\\Kwysf09        Transmit WYSE 30/50/60/160: Shift-F9\n");
      break;
    case  K_WYSF10            :
      printf("\\Kwysf10        Transmit WYSE 30/50/60/160: Shift-F10\n");
      break;
    case  K_WYSF11            :
      printf("\\Kwysf11        Transmit WYSE 30/50/60/160: Shift-F11\n");
      break;
    case  K_WYSF12            :
      printf("\\Kwysf12        Transmit WYSE 30/50/60/160: Shift-F12\n");
      break;
    case  K_WYSF13            :
      printf("\\Kwysf13        Transmit WYSE 30/50/60/160: Shift-F13\n");
      break;
    case  K_WYSF14            :
      printf("\\Kwysf14        Transmit WYSE 30/50/60/160: Shift-F14\n");
      break;
    case  K_WYSF15            :
      printf("\\Kwysf15        Transmit WYSE 30/50/60/160: Shift-F15\n");
      break;
    case  K_WYSF16            :
      printf("\\Kwysf16        Transmit WYSE 30/50/60/160: Shift-F16\n");
      break;
    case  K_WYSF17           :
      printf("\\Kwysf17        Transmit WYSE 30/50/60/160: Shift-F17\n");
      break;
    case  K_WYSF18           :
      printf("\\Kwysf18        Transmit WYSE 30/50/60/160: Shift-F18\n");
      break;
    case  K_WYSF19           :
      printf("\\Kwysf19        Transmit WYSE 30/50/60/160: Shift-F19\n");
      break;
    case  K_WYSF20           :
      printf("\\Kwysf20        Transmit WYSE 30/50/60/160: Shift-F20\n");
      break;

/* WYSE Edit and Special Keys */
    case  K_WYBS         :
      printf("\\Kwybs          Transmit WYSE 30/50/60/160: Backspace\n");
      break;
    case  K_WYCLRLN          :
      printf("\\Kwyclrln       Transmit WYSE 30/50/60/160: Clear Line\n");
      break;
    case  K_WYSCLRLN     :
     printf("\\Kwysclrln      Transmit WYSE 30/50/60/160: Shift-Clear Line\n");
      break;
    case  K_WYCLRPG      :
      printf("\\Kwyclrpg       Transmit WYSE 30/50/60/160: Clear Page\n");
      break;
    case  K_WYSCLRPG     :
    printf("\\Kwysclrpg      Transmit WYSE 30/50/60/160: Shift-Clear Page\n");
      break;
    case  K_WYDELCHAR    :
      printf("\\Kwydelchar     Transmit WYSE 30/50/60/160: Delete Char\n");
      break;
    case  K_WYDELLN      :
      printf("\\Kwydelln       Transmit WYSE 30/50/60/160: Delete Line\n");
      break;
    case  K_WYENTER          :
      printf("\\Kwyenter       Transmit WYSE 30/50/60/160: Enter\n");
      break;
    case  K_WYESC            :
      printf("\\Kwyesc         Transmit WYSE 30/50/60/160: Esc\n");
      break;
    case  K_WYHOME           :
      printf("\\Kwyhome        Transmit WYSE 30/50/60/160: Home\n");
      break;
    case  K_WYSHOME          :
      printf("\\Kwyshome       Transmit WYSE 30/50/60/160: Shift-Home\n");
      break;
    case  K_WYINSERT     :
      printf("\\Kwyinsert      Transmit WYSE 30/50/60/160: Insert\n");
      break;
    case  K_WYINSCHAR    :
      printf("\\Kwyinschar     Transmit WYSE 30/50/60/160: Insert Char\n");
      break;
    case  K_WYINSLN          :
      printf("\\Kwyinsln       Transmit WYSE 30/50/60/160: Insert Line\n");
      break;
    case  K_WYPGNEXT     :
      printf("\\Kwypgnext      Transmit WYSE 30/50/60/160: Page Next\n");
      break;
    case  K_WYPGPREV     :
      printf("\\Kwypgprev      Transmit WYSE 30/50/60/160: Page Previous\n");
      break;
    case  K_WYREPLACE    :
      printf("\\Kwyreplace     Transmit WYSE 30/50/60/160: Replace      \n");
      break;
    case  K_WYRETURN     :
      printf("\\Kwyreturn      Transmit WYSE 30/50/60/160: Return       \n");
      break;
    case  K_WYTAB            :
      printf("\\Kwytab         Transmit WYSE 30/50/60/160: Tab          \n");
      break;
    case  K_WYSTAB           :
      printf("\\Kwystab        Transmit WYSE 30/50/60/160: Shift-Tab    \n");
      break;
    case  K_WYPRTSCN     :
      printf("\\Kwyprtscn      Transmit WYSE 30/50/60/160: Print Screen \n");
      break;
    case  K_WYSESC       :
      printf("\\Kwysesc        Transmit WYSE 30/50/60/160: Shift-Esc    \n");
      break;
    case  K_WYSBS        :
    printf("\\Kwysbs         Transmit WYSE 30/50/60/160: Shift-Backspace\n");
      break;
    case  K_WYSENTER     :
      printf("\\Kwysenter      Transmit WYSE 30/50/60/160: Shift-Enter\n");
      break;
    case  K_WYSRETURN    :
      printf("\\Kwysreturn     Transmit WYSE 30/50/60/160: Shift-Return\n");
      break;
    case  K_WYUPARR          :
      printf("\\Kwyuparr       Transmit WYSE 30/50/60/160: Up Arrow\n");
      break;
    case  K_WYDNARR          :
      printf("\\Kwydnarr       Transmit WYSE 30/50/60/160: Down Arrow\n");
      break;
    case  K_WYLFARR          :
      printf("\\Kwylfarr       Transmit WYSE 30/50/60/160: Left Arrow\n");
      break;
    case  K_WYRTARR          :
      printf("\\Kwyrtarr       Transmit WYSE 30/50/60/160: Right Arrow\n");
      break;
    case  K_WYSUPARR     :
      printf("\\Kwysuparr      Transmit WYSE 30/50/60/160: Shift-Up Arrow\n");
      break;
    case  K_WYSDNARR     :
    printf("\\Kwysdnarr      Transmit WYSE 30/50/60/160: Shift-Down Arrow\n");
      break;
    case  K_WYSLFARR     :
    printf("\\Kwyslfarr      Transmit WYSE 30/50/60/160: Shift-Left Arrow\n");
      break;
    case  K_WYSRTARR     :
    printf("\\Kwysrtarr      Transmit WYSE 30/50/60/160: Shift-Right Arrow\n");
      break;
    case  K_WYSEND:
      printf("\\Kwysend        Transmit WYSE 30/50/60/160: Send\n");
      break;
    case  K_WYSSEND:
      printf("\\Kwyssend       Transmit WYSE 30/50/60/160: Shift-Send\n");
      break;

/* Data General Function Keys (unshifted) */
    case  K_DGF01            :
      printf("\\Kdgf01         Transmit Data General: F1                 \n");
      break;
    case  K_DGF02            :
      printf("\\Kdgf01         Transmit Data General: F2                 \n");
      break;
    case  K_DGF03            :
      printf("\\Kdgf01         Transmit Data General: F3                 \n");
      break;
    case  K_DGF04            :
      printf("\\Kdgf01         Transmit Data General: F4                 \n");
      break;
    case  K_DGF05            :
      printf("\\Kdgf01         Transmit Data General: F5                 \n");
      break;
    case  K_DGF06            :
      printf("\\Kdgf01         Transmit Data General: F6                 \n");
      break;
    case  K_DGF07            :
      printf("\\Kdgf01         Transmit Data General: F7                 \n");
      break;
    case  K_DGF08            :
      printf("\\Kdgf01         Transmit Data General: F8                 \n");
      break;
    case  K_DGF09            :
      printf("\\Kdgf01         Transmit Data General: F9                 \n");
      break;
    case  K_DGF10            :
      printf("\\Kdgf01         Transmit Data General: F10                \n");
      break;
    case  K_DGF11            :
      printf("\\Kdgf01         Transmit Data General: F11                \n");
      break;
    case  K_DGF12            :
      printf("\\Kdgf01         Transmit Data General: F12                \n");
      break;
    case  K_DGF13            :
      printf("\\Kdgf01         Transmit Data General: F13                \n");
      break;
    case  K_DGF14            :
      printf("\\Kdgf01         Transmit Data General: F14                \n");
      break;
    case  K_DGF15            :
      printf("\\Kdgf01         Transmit Data General: F15                \n");
      break;

/* Data General Function Keys (shifted) */
    case  K_DGSF01           :
      printf(
      "\\Kdgsf01        Transmit Data General: Shift-F1                 \n");
      break;
    case  K_DGSF02           :
      printf(
      "\\Kdgsf02        Transmit Data General: Shift-F2                 \n");
      break;
    case  K_DGSF03           :
      printf(
      "\\Kdgsf03        Transmit Data General: Shift-F3                 \n");
      break;
    case  K_DGSF04           :
      printf(
      "\\Kdgsf04        Transmit Data General: Shift-F4                 \n");
      break;
    case  K_DGSF05           :
      printf(
      "\\Kdgsf05        Transmit Data General: Shift-F5                 \n");
      break;
    case  K_DGSF06           :
      printf(
      "\\Kdgsf06        Transmit Data General: Shift-F6                 \n");
      break;
    case  K_DGSF07           :
      printf(
      "\\Kdgsf07        Transmit Data General: Shift-F7                 \n");
      break;
    case  K_DGSF08           :
      printf(
      "\\Kdgsf08        Transmit Data General: Shift-F8                 \n");
      break;
    case  K_DGSF09           :
      printf(
      "\\Kdgsf09        Transmit Data General: Shift-F9                 \n");
      break;
    case  K_DGSF10           :
      printf(
      "\\Kdgsf10        Transmit Data General: Shift-F10                \n");
      break;
    case  K_DGSF11           :
      printf(
      "\\Kdgsf11        Transmit Data General: Shift-F11                \n");
      break;
    case  K_DGSF12           :
      printf(
      "\\Kdgsf12        Transmit Data General: Shift-F12                \n");
      break;
    case  K_DGSF13           :
      printf(
      "\\Kdgsf13        Transmit Data General: Shift-F13                \n");
      break;
    case  K_DGSF14           :
      printf(
      "\\Kdgsf14        Transmit Data General: Shift-F14                \n");
      break;
    case  K_DGSF15           :
      printf(
      "\\Kdgsf15        Transmit Data General: Shift-F15                \n");
      break;

/* Data General Function Keys (control) */
    case  K_DGCF01           :
      printf(
      "\\Kdgcf01        Transmit Data General: Ctrl-F1                 \n");
      break;
    case  K_DGCF02            :
      printf(
      "\\Kdgcf02        Transmit Data General: Ctrl-F2                 \n");
      break;
    case  K_DGCF03            :
      printf(
      "\\Kdgcf03        Transmit Data General: Ctrl-F3                 \n");
      break;
    case  K_DGCF04            :
      printf(
      "\\Kdgcf04        Transmit Data General: Ctrl-F4                 \n");
      break;
    case  K_DGCF05            :
      printf(
      "\\Kdgcf05        Transmit Data General: Ctrl-F5                 \n");
      break;
    case  K_DGCF06            :
      printf(
      "\\Kdgcf06        Transmit Data General: Ctrl-F6                 \n");
      break;
    case  K_DGCF07            :
      printf(
      "\\Kdgcf07        Transmit Data General: Ctrl-F7                 \n");
      break;
    case  K_DGCF08            :
      printf(
      "\\Kdgcf08        Transmit Data General: Ctrl-F8                 \n");
      break;
    case  K_DGCF09            :
      printf(
      "\\Kdgcf09        Transmit Data General: Ctrl-F9                 \n");
      break;
    case  K_DGCF10            :
      printf(
      "\\Kdgcf10        Transmit Data General: Ctrl-F10                \n");
      break;
    case  K_DGCF11            :
      printf(
      "\\Kdgcf11        Transmit Data General: Ctrl-F11                \n");
      break;
    case  K_DGCF12            :
      printf(
      "\\Kdgcf12        Transmit Data General: Ctrl-F12                \n");
      break;
    case  K_DGCF13            :
      printf(
      "\\Kdgcf13        Transmit Data General: Ctrl-F13                \n");
      break;
    case  K_DGCF14            :
      printf(
      "\\Kdgcf14        Transmit Data General: Ctrl-F14                \n");
      break;
    case  K_DGCF15            :
      printf(
      "\\Kdgcf15        Transmit Data General: Ctrl-F15                \n");
      break;

/* Data General Function Keys (control shifted) */
    case  K_DGCSF01          :
      printf(
    "\\Kdgcsf01       Transmit Data General: Ctrl-Shift-F1                \n");
      break;
    case  K_DGCSF02          :
      printf(
    "\\Kdgcsf02       Transmit Data General: Ctrl-Shift-F2                \n");
      break;
    case  K_DGCSF03          :
      printf(
    "\\Kdgcsf03       Transmit Data General: Ctrl-Shift-F3                \n");
      break;
    case  K_DGCSF04          :
      printf(
    "\\Kdgcsf04       Transmit Data General: Ctrl-Shift-F4                \n");
      break;
    case  K_DGCSF05          :
      printf(
    "\\Kdgcsf05       Transmit Data General: Ctrl-Shift-F5                \n");
      break;
    case  K_DGCSF06          :
      printf(
    "\\Kdgcsf06       Transmit Data General: Ctrl-Shift-F6                \n");
      break;
    case  K_DGCSF07          :
      printf(
    "\\Kdgcsf07       Transmit Data General: Ctrl-Shift-F7                \n");
      break;
    case  K_DGCSF08          :
      printf(
    "\\Kdgcsf08       Transmit Data General: Ctrl-Shift-F8                \n");
      break;
    case  K_DGCSF09          :
      printf(
    "\\Kdgcsf09       Transmit Data General: Ctrl-Shift-F9                \n");
      break;
    case  K_DGCSF10          :
      printf(
    "\\Kdgcsf10       Transmit Data General: Ctrl-Shift-F10               \n");
      break;
    case  K_DGCSF11          :
      printf(
    "\\Kdgcsf11       Transmit Data General: Ctrl-Shift-F11               \n");
      break;
    case  K_DGCSF12          :
      printf(
    "\\Kdgcsf12       Transmit Data General: Ctrl-Shift-F12               \n");
      break;
    case  K_DGCSF13          :
      printf(
    "\\Kdgcsf13       Transmit Data General: Ctrl-Shift-F13               \n");
      break;
    case  K_DGCSF14          :
      printf(
    "\\Kdgcsf14       Transmit Data General: Ctrl-Shift-F14               \n");
      break;
    case  K_DGCSF15          :
      printf(
    "\\Kdgcsf15       Transmit Data General: Ctrl-Shift-F15               \n");
      break;

    case  K_DGUPARR          :
      printf("\\Kdguparr       Transmit Data General: Up Arrow          \n");
      break;
    case  K_DGDNARR          :
      printf("\\Kdgdnarr       Transmit Data General: Down Arrow        \n");
      break;
    case  K_DGLFARR          :
      printf("\\Kdglfarr       Transmit Data General: Left Arrow        \n");
      break;
    case  K_DGRTARR          :
      printf("\\Kdgrtarr       Transmit Data General: Right Arrow       \n");
      break;
    case  K_DGSUPARR     :
      printf("\\Kdgsuparr      Transmit Data General: Shift-Up Arrow    \n");
      break;
    case  K_DGSDNARR     :
      printf("\\Kdgsdnarr      Transmit Data General: Shift-Down Arrow  \n");
      break;
    case  K_DGSLFARR     :
      printf("\\Kdgslfarr      Transmit Data General: Shift-Left Arrow  \n");
      break;
    case  K_DGSRTARR     :
      printf("\\Kdgsrtarr      Transmit Data General: Shift-Right Arrow \n");
      break;

    case  K_DGERASEPAGE  :
      printf("\\Kdgerasepage   Transmit Data General: Erase Page        \n");
      break;
    case  K_DGC1             :
      printf("\\Kdgc1          Transmit Data General: C1                \n");
      break;
    case  K_DGC2             :
      printf("\\Kdgc2          Transmit Data General: C2                \n");
      break;
    case  K_DGERASEEOL   :
      printf("\\Kdgeraseeol    Transmit Data General: Erase EOL         \n");
      break;
    case  K_DGC3             :
      printf("\\Kdgc3          Transmit Data General: C3                \n");
      break;
    case  K_DGC4             :
      printf("\\Kdgc4          Transmit Data General: C4                \n");
      break;
    case  K_DGCMDPRINT   :
      printf("\\Kdgcmdprint    Transmit Data General: Command Print     \n");
      break;
    case  K_DGHOME       :
      printf("\\Kdghome        Transmit Data General: Home              \n");
      break;
    case  K_DGSERASEPAGE :
      printf("\\Kdgserasepage  Transmit Data General: Erase Page        \n");
      break;
    case  K_DGSC1            :
      printf("\\Kdgsc1         Transmit Data General: Shift-C1          \n");
      break;
    case  K_DGSC2            :
      printf("\\Kdgsc2         Transmit Data General: Shift-C2          \n");
      break;
    case  K_DGSERASEEOL  :
      printf("\\Kdgseraseeol   Transmit Data General: Shift-Erase EOL  \n");
      break;
    case  K_DGSC3            :
      printf("\\Kdgsc3         Transmit Data General: Shift-C3          \n");
      break;
    case  K_DGSC4            :
      printf("\\Kdgsc4         Transmit Data General: Shift-C4          \n");
      break;
    case  K_DGSCMDPRINT  :
      printf("\\Kdgscmdprint   Transmit Data General: Shift-Command Print\n");
      break;
    case  K_DGBS         :
      printf("\\Kdgbs          Transmit Data General: Backspace         \n");
      break;
    case  K_DGSHOME      :
      printf("\\Kdshome        Transmit Data General: Shift-Home        \n");
      break;


/* Televideo Function Keys (unshifted) */
    case  K_TVIF01           :
      printf("\\Ktvif01        Transmit Televideo: F1       \n");
      break;
    case  K_TVIF02           :
      printf("\\Ktvif02        Transmit Televideo: F2              \n");
      break;
    case  K_TVIF03           :
      printf("\\Ktvif03        Transmit Televideo: F3             \n");
      break;
    case  K_TVIF04           :
      printf("\\Ktvif04        Transmit Televideo: F4              \n");
      break;
    case  K_TVIF05           :
      printf("\\Ktvif05        Transmit Televideo: F5              \n");
      break;
    case  K_TVIF06           :
      printf("\\Ktvif06        Transmit Televideo: F6              \n");
      break;
    case  K_TVIF07           :
      printf("\\Ktvif07        Transmit Televideo: F7              \n");
      break;
    case  K_TVIF08           :
      printf("\\Ktvif08        Transmit Televideo: F8              \n");
      break;
    case  K_TVIF09           :
      printf("\\Ktvif09        Transmit Televideo: F9              \n");
      break;
    case  K_TVIF10           :
      printf("\\Ktvif10        Transmit Televideo: F10             \n");
      break;
    case  K_TVIF11           :
      printf("\\Ktvif11        Transmit Televideo: F11             \n");
      break;
    case  K_TVIF12           :
      printf("\\Ktvif12        Transmit Televideo: F12             \n");
      break;
    case  K_TVIF13           :
      printf("\\Ktvif13        Transmit Televideo: F13             \n");
      break;
    case  K_TVIF14           :
      printf("\\Ktvif14        Transmit Televideo: F14             \n");
      break;
    case  K_TVIF15           :
      printf("\\Ktvif15        Transmit Televideo: F15             \n");
      break;
    case  K_TVIF16           :
      printf("\\Ktvif16        Transmit Televideo: F16             \n");
      break;

/* Televideo Function Keys (shifted) */
    case  K_TVISF01          :
      printf("\\Ktvisf01       Transmit Televideo: Shift-F1 \n");
      break;
    case  K_TVISF02          :
      printf("\\Ktvisf02       Transmit Televideo: Shift-F2 \n");
      break;
    case  K_TVISF03            :
      printf("\\Ktvisf03       Transmit Televideo: Shift-F3 \n");
      break;
    case  K_TVISF04            :
      printf("\\Ktvisf04       Transmit Televideo: Shift-F4 \n");
      break;
    case  K_TVISF05            :
      printf("\\Ktvisf05       Transmit Televideo: Shift-F5 \n");
      break;
    case  K_TVISF06            :
      printf("\\Ktvisf06       Transmit Televideo: Shift-F6 \n");
      break;
    case  K_TVISF07            :
      printf("\\Ktvisf07       Transmit Televideo: Shift-F7 \n");
      break;
    case  K_TVISF08            :
      printf("\\Ktvisf08       Transmit Televideo: Shift-F8 \n");
      break;
    case  K_TVISF09            :
      printf("\\Ktvisf09       Transmit Televideo: Shift-F9 \n");
      break;
    case  K_TVISF10            :
      printf("\\Ktvisf10       Transmit Televideo: Shift-F10\n");
      break;
    case  K_TVISF11            :
      printf("\\Ktvisf11       Transmit Televideo: Shift-F11\n");
      break;
    case  K_TVISF12            :
      printf("\\Ktvisf12       Transmit Televideo: Shift-F12\n");
      break;
    case  K_TVISF13            :
      printf("\\Ktvisf13       Transmit Televideo: Shift-F13\n");
      break;
    case  K_TVISF14            :
      printf("\\Ktvisf14       Transmit Televideo: Shift-F14\n");
      break;
    case  K_TVISF15            :
      printf("\\Ktvisf15       Transmit Televideo: Shift-F15\n");
      break;
    case  K_TVISF16            :
      printf("\\Ktvisf16       Transmit Televideo: Shift-F16\n");
      break;

/* Televideo Edit and Special Keys */
    case  K_TVIBS         :
      printf("\\Ktvibs         Transmit Televideo: Backspace       \n");
      break;
    case  K_TVICLRLN         :
      printf("\\Ktviclrln      Transmit Televideo: Clear Line      \n");
      break;
    case  K_TVISCLRLN     :
      printf("\\Ktvisclrln     Transmit Televideo: Shift-Clear Line\n");
      break;
    case  K_TVICLRPG      :
      printf("\\Ktviclrpg      Transmit Televideo: Clear Page      \n");
      break;
    case  K_TVISCLRPG     :
      printf("\\Ktvisclrpg     Transmit Televideo: Shift-Clear Page\n");
      break;
    case  K_TVIDELCHAR    :
      printf("\\Ktvidelchar    Transmit Televideo: Delete Char     \n");
      break;
    case  K_TVIDELLN      :
      printf("\\Ktvidelln      Transmit Televideo: Delete Line     \n");
      break;
    case  K_TVIENTER         :
      printf("\\Ktvienter      Transmit Televideo: Enter           \n");
      break;
    case  K_TVIESC           :
      printf("\\Ktviesc        Transmit Televideo: Esc             \n");
      break;
    case  K_TVIHOME          :
      printf("\\Ktvihome       Transmit Televideo: Home            \n");
      break;
    case  K_TVISHOME         :
      printf("\\Ktvishome      Transmit Televideo: Shift-Home      \n");
      break;
    case  K_TVIINSERT     :
      printf("\\Ktviinsert     Transmit Televideo: Insert          \n");
      break;
    case  K_TVIINSCHAR    :
      printf("\\Ktviinschar    Transmit Televideo: Insert Char     \n");
      break;
    case  K_TVIINSLN         :
      printf("\\Ktviinsln      Transmit Televideo: Insert Line     \n");
      break;
    case  K_TVIPGNEXT     :
      printf("\\Ktvipgnext     Transmit Televideo: Page Next       \n");
      break;
    case  K_TVIPGPREV     :
      printf("\\Ktvipgprev     Transmit Televideo: Page Previous   \n");
      break;
    case  K_TVIREPLACE    :
      printf("\\Ktvireplace    Transmit Televideo: Replace         \n");
      break;
    case  K_TVIRETURN     :
      printf("\\Ktvireturn     Transmit Televideo: Return          \n");
      break;
    case  K_TVITAB           :
      printf("\\Ktvitab        Transmit Televideo: Tab             \n");
      break;
    case  K_TVISTAB          :
      printf("\\Ktvistab       Transmit Televideo: Shift-Tab       \n");
      break;
    case  K_TVIPRTSCN     :
      printf("\\Ktviprtscn     Transmit Televideo: Print Screen    \n");
      break;
    case  K_TVISESC       :
      printf("\\Ktvisesc       Transmit Televideo: Shift-Esc       \n");
      break;
    case  K_TVISBS        :
      printf("\\Ktvisbs        Transmit Televideo: Shift-Backspace \n");
      break;
    case  K_TVISENTER     :
      printf("\\Ktvisenter     Transmit Televideo: Shift-Enter     \n");
      break;
    case  K_TVISRETURN    :
      printf("\\Ktvisreturn    Transmit Televideo: Shift-Return    \n");
      break;
    case  K_TVIUPARR         :
      printf("\\Ktviuparr      Transmit Televideo: Up Arrow        \n");
      break;
    case  K_TVIDNARR         :
      printf("\\Ktvidnarr      Transmit Televideo: Down Arrow      \n");
      break;
    case  K_TVILFARR         :
      printf("\\Ktvilfarr      Transmit Televideo: Left Arrow      \n");
      break;
    case  K_TVIRTARR         :
      printf("\\Ktvirtarr      Transmit Televideo: Right Arrow     \n");
      break;
    case  K_TVISUPARR     :
      printf("\\Ktvisuparr     Transmit Televideo: Shift-Up Arrow  \n");
      break;
    case  K_TVISDNARR     :
      printf("\\Ktvisdnarr     Transmit Televideo: Shift-Down Arrow\n");
      break;
    case  K_TVISLFARR     :
      printf("\\Ktvislfarr     Transmit Televideo: Shift-Left Arrow\n");
      break;
    case  K_TVISRTARR     :
      printf("\\Ktvisrtarr     Transmit Televideo: Shift-Right Arrow\n");
      break;
    case K_TVISEND:
      printf("\\Ktvisend       Transmit Televideo: Send\n");
      break;
    case K_TVISSEND:
      printf("\\Ktvissend      Transmit Televideo: Shift-Send\n");
      break;

/* HP Function and Edit keys */
    case  K_HPF01            :
      printf("\\Khpf01         Transmit Hewlett-Packard: F1       \n");
      break;
    case  K_HPF02            :
      printf("\\Khpf02         Transmit Hewlett-Packard: F2              \n");
      break;
    case  K_HPF03            :
      printf("\\Khpf03         Transmit Hewlett-Packard: F3             \n");
      break;
    case  K_HPF04            :
      printf("\\Khpf04         Transmit Hewlett-Packard: F4              \n");
      break;
    case  K_HPF05            :
      printf("\\Khpf05         Transmit Hewlett-Packard: F5              \n");
      break;
    case  K_HPF06            :
      printf("\\Khpf06         Transmit Hewlett-Packard: F6              \n");
      break;
    case  K_HPF07            :
      printf("\\Khpf07         Transmit Hewlett-Packard: F7              \n");
      break;
    case  K_HPF08            :
      printf("\\Khpf08         Transmit Hewlett-Packard: F8              \n");
      break;
    case  K_HPF09            :
      printf("\\Khpf09         Transmit Hewlett-Packard: F9              \n");
      break;
    case  K_HPF10            :
      printf("\\Khpf10         Transmit Hewlett-Packard: F10             \n");
      break;
    case  K_HPF11            :
      printf("\\Khpf11         Transmit Hewlett-Packard: F11             \n");
      break;
    case  K_HPF12            :
      printf("\\Khpf12         Transmit Hewlett-Packard: F12             \n");
      break;
    case  K_HPF13            :
      printf("\\Khpf13         Transmit Hewlett-Packard: F13             \n");
      break;
    case  K_HPF14            :
      printf("\\Khpf14         Transmit Hewlett-Packard: F14             \n");
      break;
    case  K_HPF15            :
      printf("\\Khpf15         Transmit Hewlett-Packard: F15             \n");
      break;
    case  K_HPF16            :
      printf("\\Khpf16         Transmit Hewlett-Packard: F16             \n");
      break;
    case  K_HPRETURN     :
      printf("\\Khpreturn      Transmit Hewlett-Packard: Return\n");
      break;
    case  K_HPENTER          :
      printf("\\Khpenter       Transmit Hewlett-Packard: Enter (keypad)\n");
      break;
    case  K_HPBACKTAB        :
      printf("\\Khpbacktab     Transmit Hewlett-Packard: Back Tab\n");
      break;
        /* Siemens Nixdorf International 97801-5xx kverbs */
    case K_SNI_DOUBLE_0      :
        printf("\\Ksni_00          Transmit SNI-97801-5xx: Double-Zero\n");
        break;
    case K_SNI_C_DOUBLE_0    :
        printf(
"\\Ksni_c_00        Transmit SNI-97801-5xx: Ctrl-Double-Zero\n");
        break;
    case K_SNI_C_CE          :
        printf("\\Ksni_c_ce        Transmit SNI-97801-5xx: Ctrl-CE\n");
        break;
    case K_SNI_C_COMPOSE     :
        printf("\\Ksni_c_compose   Transmit SNI-97801-5xx: Ctrl-Compose\n");
        break;
    case K_SNI_C_DELETE_CHAR :
        printf(
"\\Ksni_c_del_char  Transmit SNI-97801-5xx: Ctrl-Delete Char\n");
        break;
    case K_SNI_C_DELETE_LINE :
        printf(
"\\Ksni_c_del_line  Transmit SNI-97801-5xx: Ctrl-Delete Line\n");
        break;
    case K_SNI_C_DELETE_WORD :
        printf(
"\\Ksni_c_del_word  Transmit SNI-97801-5xx: Ctrl-Delete Word\n");
        break;
    case K_SNI_C_CURSOR_DOWN :
        printf(
"\\Ksni_c_dnarr     Transmit SNI-97801-5xx: Ctrl-Cursor Down\n");
        break;
    case K_SNI_C_ENDMARKE    :
        printf("\\Ksni_c_endmarke  Transmit SNI-97801-5xx: Ctrl-End Marke\n");
        break;
    case K_SNI_C_F01         :
        printf("\\Ksni_c_f01       Transmit SNI-97801-5xx: Ctrl-F1\n");
        break;
    case K_SNI_C_F02         :
        printf("\\Ksni_c_f02       Transmit SNI-97801-5xx: Ctrl-F2\n");
        break;
    case K_SNI_C_F03         :
        printf("\\Ksni_c_f03       Transmit SNI-97801-5xx: Ctrl-F3\n");
        break;
    case K_SNI_C_F04         :
        printf("\\Ksni_c_f04       Transmit SNI-97801-5xx: Ctrl-F4\n");
        break;
    case K_SNI_C_F05         :
        printf("\\Ksni_c_f05       Transmit SNI-97801-5xx: Ctrl-F5\n");
        break;
    case K_SNI_C_F06         :
        printf("\\Ksni_c_f06       Transmit SNI-97801-5xx: Ctrl-F6\n");
        break;
    case K_SNI_C_F07         :
        printf("\\Ksni_c_f07       Transmit SNI-97801-5xx: Ctrl-F7\n");
        break;
    case K_SNI_C_F08         :
        printf("\\Ksni_c_f08       Transmit SNI-97801-5xx: Ctrl-F8\n");
        break;
    case K_SNI_C_F09         :
        printf("\\Ksni_c_f09       Transmit SNI-97801-5xx: Ctrl-F9\n");
        break;
    case K_SNI_C_F10         :
        printf("\\Ksni_c_f10       Transmit SNI-97801-5xx: Ctrl-F10\n");
        break;
    case K_SNI_C_F11         :
        printf("\\Ksni_c_f11       Transmit SNI-97801-5xx: Ctrl-F11\n");
        break;
    case K_SNI_C_F12         :
        printf("\\Ksni_c_f12       Transmit SNI-97801-5xx: Ctrl-F12\n");
        break;
    case K_SNI_C_F13         :
        printf("\\Ksni_c_f13       Transmit SNI-97801-5xx: Ctrl-F13\n");
        break;
    case K_SNI_C_F14         :
        printf("\\Ksni_c_f14       Transmit SNI-97801-5xx: Ctrl-F14\n");
        break;
    case K_SNI_C_F15         :
        printf("\\Ksni_c_f15       Transmit SNI-97801-5xx: Ctrl-F15\n");
        break;
    case K_SNI_C_F16         :
        printf("\\Ksni_c_f16       Transmit SNI-97801-5xx: Ctrl-F16\n");
        break;
    case K_SNI_C_F17         :
        printf("\\Ksni_c_f17       Transmit SNI-97801-5xx: Ctrl-F17\n");
        break;
    case K_SNI_C_F18         :
        printf("\\Ksni_c_f18       Transmit SNI-97801-5xx: Ctrl-F18\n");
        break;
    case K_SNI_C_USER1        :
        printf(
"\\Ksni_c_user1     Transmit SNI-97801-5xx: Ctrl-Key below F18\n");
        break;
    case K_SNI_C_F19         :
        printf("\\Ksni_c_f19       Transmit SNI-97801-5xx: Ctrl-F19\n");
        break;
    case K_SNI_C_USER2       :
        printf(
"\\Ksni_c_user2     Transmit SNI-97801-5xx: Ctrl-Key below F19\n");
        break;
    case K_SNI_C_F20         :
        printf("\\Ksni_c_f20       Transmit SNI-97801-5xx: Ctrl-F20\n");
        break;
    case K_SNI_C_USER3       :
        printf(
"\\Ksni_c_user3     Transmit SNI-97801-5xx: Ctrl-Key below F20\n");
        break;
    case K_SNI_C_F21         :
        printf("\\Ksni_c_f21       Transmit SNI-97801-5xx: Ctrl-F21\n");
        break;
    case K_SNI_C_USER4       :
        printf(
"\\Ksni_c_user4     Transmit SNI-97801-5xx: Ctrl-Key below F21\n");
        break;
    case K_SNI_C_F22         :
        printf("\\Ksni_c_f22       Transmit SNI-97801-5xx: Ctrl-F22\n");
        break;
    case K_SNI_C_USER5       :
        printf(
"\\Ksni_c_user5     Transmit SNI-97801-5xx: Ctrl-Key below F22\n");
        break;
    case K_SNI_C_HELP        :
        printf("\\Ksni_c_help      Transmit SNI-97801-5xx: Ctrl-Help\n");
        break;
    case K_SNI_C_HOME        :
        printf("\\Ksni_c_home      Transmit SNI-97801-5xx: Ctrl-Home\n");
        break;
    case K_SNI_C_INSERT_CHAR :
        printf(
"\\Ksni_c_ins_char  Transmit SNI-97801-5xx: Ctrl-Insert Char\n");
        break;
    case K_SNI_C_INSERT_LINE :
        printf(
"\\Ksni_c_ins_line  Transmit SNI-97801-5xx: Ctrl-Insert Line\n");
        break;
    case K_SNI_C_INSERT_WORD :
        printf(
"\\Ksni_c_ins_word  Transmit SNI-97801-5xx: Ctrl-Insert Word\n");
        break;
    case K_SNI_C_LEFT_TAB    :
        printf("\\Ksni_c_left_tab  Transmit SNI-97801-5xx: Ctrl-Left Tab\n");
        break;
    case K_SNI_C_CURSOR_LEFT :
        printf(
"\\Ksni_c_lfarr     Transmit SNI-97801-5xx: Ctrl-Cursor Left\n");
        break;
    case K_SNI_C_MODE        :
        printf("\\Ksni_c_mode      Transmit SNI-97801-5xx: Ctrl-Mode\n");
        break;
    case K_SNI_C_PAGE        :
        printf("\\Ksni_c_page      Transmit SNI-97801-5xx: Ctrl-Page\n");
        break;
    case K_SNI_C_PRINT       :
        printf("\\Ksni_c_print     Transmit SNI-97801-5xx: Ctrl-Print\n");
        break;
    case K_SNI_C_CURSOR_RIGHT:
        printf(
"\\Ksni_c_rtarr     Transmit SNI-97801-5xx: Ctrl-Cursor Right\n");
        break;
    case K_SNI_C_SCROLL_DOWN :
        printf(
"\\Ksni_c_scroll_dn Transmit SNI-97801-5xx: Ctrl-Scroll Down\n");
        break;
    case K_SNI_C_SCROLL_UP   :
        printf("\\Ksni_c_scroll_up Transmit SNI-97801-5xx: Ctrl-Scroll Up\n");
        break;
    case K_SNI_C_START       :
        printf("\\Ksni_c_start     Transmit SNI-97801-5xx: Ctrl-Start\n");
        break;
    case K_SNI_C_CURSOR_UP   :
        printf("\\Ksni_c_uparr     Transmit SNI-97801-5xx: Ctrl-Cursor Up\n");
        break;
    case K_SNI_C_TAB         :
        printf("\\Ksni_c_tab       Transmit SNI-97801-5xx: Ctrl-Tab\n");
        break;
    case K_SNI_CE            :
        printf("\\Ksni_ce          Transmit SNI-97801-5xx: CE\n");
        break;
    case K_SNI_CH_CODE:
        printf("\\Ksni_ch_code     Toggle SNI-97801-5xx: CH.CODE function.\n");
        break;
    case K_SNI_COMPOSE       :
        printf("\\Ksni_compose     Transmit SNI-97801-5xx: Compose\n");
        break;
    case K_SNI_DELETE_CHAR   :
        printf("\\Ksni_del_char    Transmit SNI-97801-5xx: Delete Char\n");
        break;
    case K_SNI_DELETE_LINE   :
        printf("\\Ksni_del_line    Transmit SNI-97801-5xx: Delete Line\n");
        break;
    case K_SNI_DELETE_WORD   :
        printf("\\Ksni_del_word    Transmit SNI-97801-5xx: Delete Word\n");
        break;
    case K_SNI_CURSOR_DOWN   :
        printf("\\Ksni_dnarr       Transmit SNI-97801-5xx: Cursor Down\n");
        break;
    case K_SNI_ENDMARKE      :
        printf("\\Ksni_endmarke    Transmit SNI-97801-5xx: End Marke\n");
        break;
    case K_SNI_F01           :
        printf("\\Ksni_f01         Transmit SNI-97801-5xx: F1\n");
        break;
    case K_SNI_F02           :
        printf("\\Ksni_f02         Transmit SNI-97801-5xx: F2\n");
        break;
    case K_SNI_F03           :
        printf("\\Ksni_f03         Transmit SNI-97801-5xx: F3\n");
        break;
    case K_SNI_F04           :
        printf("\\Ksni_f04         Transmit SNI-97801-5xx: F4\n");
        break;
    case K_SNI_F05           :
        printf("\\Ksni_f05         Transmit SNI-97801-5xx: F5\n");
        break;
    case K_SNI_F06           :
        printf("\\Ksni_f06         Transmit SNI-97801-5xx: F6\n");
        break;
    case K_SNI_F07           :
        printf("\\Ksni_f07         Transmit SNI-97801-5xx: F7\n");
        break;
    case K_SNI_F08           :
        printf("\\Ksni_f08         Transmit SNI-97801-5xx: F8\n");
        break;
    case K_SNI_F09           :
        printf("\\Ksni_f09         Transmit SNI-97801-5xx: F9\n");
        break;
    case K_SNI_F10           :
        printf("\\Ksni_f10         Transmit SNI-97801-5xx: F10\n");
        break;
    case K_SNI_F11           :
        printf("\\Ksni_f11         Transmit SNI-97801-5xx: F11\n");
        break;
    case K_SNI_F12           :
        printf("\\Ksni_f12         Transmit SNI-97801-5xx: F12\n");
        break;
    case K_SNI_F13           :
        printf("\\Ksni_f13         Transmit SNI-97801-5xx: F13\n");
        break;
    case K_SNI_F14           :
        printf("\\Ksni_f14         Transmit SNI-97801-5xx: F14\n");
        break;
    case K_SNI_F15           :
        printf("\\Ksni_f15         Transmit SNI-97801-5xx: F15\n");
        break;
    case K_SNI_F16           :
        printf("\\Ksni_f16         Transmit SNI-97801-5xx: F16\n");
        break;
    case K_SNI_F17           :
        printf("\\Ksni_f17         Transmit SNI-97801-5xx: F17\n");
        break;
    case K_SNI_F18           :
        printf("\\Ksni_f18         Transmit SNI-97801-5xx: F18\n");
        break;
    case K_SNI_USER1          :
        printf("\\Ksni_user1       Transmit SNI-97801-5xx: Key below F18\n");
        break;
    case K_SNI_F19           :
        printf("\\Ksni_f19         Transmit SNI-97801-5xx: F19\n");
        break;
    case K_SNI_USER2          :
        printf("\\Ksni_user2       Transmit SNI-97801-5xx: Key below F19\n");
        break;
    case K_SNI_F20           :
        printf("\\Ksni_f20         Transmit SNI-97801-5xx: F20\n");
        break;
    case K_SNI_USER3          :
        printf("\\Ksni_user3       Transmit SNI-97801-5xx: Key below F20\n");
        break;
    case K_SNI_F21           :
        printf("\\Ksni_f21         Transmit SNI-97801-5xx: F21\n");
        break;
    case K_SNI_USER4          :
        printf("\\Ksni_user4       Transmit SNI-97801-5xx: Key below F21\n");
        break;
    case K_SNI_F22           :
        printf("\\Ksni_f22         Transmit SNI-97801-5xx: F22\n");
        break;
    case K_SNI_USER5          :
        printf("\\Ksni_user5       Transmit SNI-97801-5xx: Key below F22\n");
        break;
    case K_SNI_HELP          :
        printf("\\Ksni_help        Transmit SNI-97801-5xx: Help\n");
        break;
    case K_SNI_HOME          :
        printf("\\Ksni_home        Transmit SNI-97801-5xx: Home\n");
        break;
    case K_SNI_INSERT_CHAR   :
        printf("\\Ksni_ins_char    Transmit SNI-97801-5xx: Insert Char\n");
        break;
    case K_SNI_INSERT_LINE   :
        printf("\\Ksni_ins_line    Transmit SNI-97801-5xx: Insert Line\n");
        break;
    case K_SNI_INSERT_WORD   :
        printf("\\Ksni_ins_word    Transmit SNI-97801-5xx: Insert Word\n");
        break;
    case K_SNI_LEFT_TAB      :
        printf("\\Ksni_left_tab    Transmit SNI-97801-5xx: Left Tab\n");
        break;
    case K_SNI_CURSOR_LEFT   :
        printf("\\Ksni_lfarr       Transmit SNI-97801-5xx: Cursor Left\n");
        break;
    case K_SNI_MODE          :
        printf("\\Ksni_mode        Transmit SNI-97801-5xx: Mode\n");
        break;
    case K_SNI_PAGE          :
        printf("\\Ksni_page        Transmit SNI-97801-5xx: Page\n");
        break;
    case K_SNI_PRINT         :
        printf("\\Ksni_print       Transmit SNI-97801-5xx: Print\n");
        break;
    case K_SNI_CURSOR_RIGHT  :
        printf("\\Ksni_rtarr       Transmit SNI-97801-5xx: Cursor Right\n");
        break;
    case K_SNI_S_DOUBLE_0    :
        printf(
"\\Ksni_s_00        Transmit SNI-97801-5xx: Shift-Double-Zero\n");
        break;
    case K_SNI_S_CE          :
        printf("\\Ksni_s_ce        Transmit SNI-97801-5xx: Shift-CE\n");
        break;
    case K_SNI_S_COMPOSE     :
        printf("\\Ksni_s_compose   Transmit SNI-97801-5xx: Shift-Compose\n");
        break;
    case K_SNI_S_DELETE_CHAR :
        printf(
"\\Ksni_s_del_char  Transmit SNI-97801-5xx: Shift-Delete Char\n");
        break;
    case K_SNI_S_DELETE_LINE :
        printf(
"\\Ksni_s_del_line  Transmit SNI-97801-5xx: Shift-Delete Line\n");
        break;
    case K_SNI_S_DELETE_WORD :
        printf(
"\\Ksni_s_del_word  Transmit SNI-97801-5xx: Shift-Delete Word\n");
        break;
    case K_SNI_S_CURSOR_DOWN :
        printf(
"\\Ksni_s_dnarr     Transmit SNI-97801-5xx: Shift-Cursor Down\n");
        break;
    case K_SNI_S_ENDMARKE    :
        printf("\\Ksni_s_endmarke  Transmit SNI-97801-5xx: Shift-End Marke\n");
        break;
    case K_SNI_S_F01         :
        printf("\\Ksni_s_f01       Transmit SNI-97801-5xx: Shift-F1\n");
        break;
    case K_SNI_S_F02         :
        printf("\\Ksni_s_f02       Transmit SNI-97801-5xx: Shift-F2\n");
        break;
    case K_SNI_S_F03         :
        printf("\\Ksni_s_f03       Transmit SNI-97801-5xx: Shift-F3\n");
        break;
    case K_SNI_S_F04         :
        printf("\\Ksni_s_f04       Transmit SNI-97801-5xx: Shift-F4\n");
        break;
    case K_SNI_S_F05         :
        printf("\\Ksni_s_f05       Transmit SNI-97801-5xx: Shift-F5\n");
        break;
    case K_SNI_S_F06         :
        printf("\\Ksni_s_f06       Transmit SNI-97801-5xx: Shift-F6\n");
        break;
    case K_SNI_S_F07         :
        printf("\\Ksni_s_f07       Transmit SNI-97801-5xx: Shift-F7\n");
        break;
    case K_SNI_S_F08         :
        printf("\\Ksni_s_f08       Transmit SNI-97801-5xx: Shift-F8\n");
        break;
    case K_SNI_S_F09         :
        printf("\\Ksni_s_f09       Transmit SNI-97801-5xx: Shift-F9\n");
        break;
    case K_SNI_S_F10         :
        printf("\\Ksni_s_f10       Transmit SNI-97801-5xx: Shift-F10\n");
        break;
    case K_SNI_S_F11         :
        printf("\\Ksni_s_f11       Transmit SNI-97801-5xx: Shift-F11\n");
        break;
    case K_SNI_S_F12         :
        printf("\\Ksni_s_f12       Transmit SNI-97801-5xx: Shift-F12\n");
        break;
    case K_SNI_S_F13         :
        printf("\\Ksni_s_f13       Transmit SNI-97801-5xx: Shift-F13\n");
        break;
    case K_SNI_S_F14         :
        printf("\\Ksni_s_f14       Transmit SNI-97801-5xx: Shift-F14\n");
        break;
    case K_SNI_S_F15         :
        printf("\\Ksni_s_f15       Transmit SNI-97801-5xx: Shift-F15\n");
        break;
    case K_SNI_S_F16         :
        printf("\\Ksni_s_f16       Transmit SNI-97801-5xx: Shift-F16\n");
        break;
    case K_SNI_S_F17         :
        printf("\\Ksni_s_f17       Transmit SNI-97801-5xx: Shift-F17\n");
        break;
    case K_SNI_S_F18         :
        printf("\\Ksni_s_f18       Transmit SNI-97801-5xx: Shift-F18\n");
        break;
    case K_SNI_S_USER1        :
        printf(
"\\Ksni_s_user1     Transmit SNI-97801-5xx: Shift-Key below F18\n");
        break;
    case K_SNI_S_F19         :
        printf("\\Ksni_s_f19       Transmit SNI-97801-5xx: Shift-F19\n");
        break;
    case K_SNI_S_USER2       :
        printf(
"\\Ksni_s_user2     Transmit SNI-97801-5xx: Shift-Key below F19\n");
        break;
    case K_SNI_S_F20         :
        printf("\\Ksni_s_f20       Transmit SNI-97801-5xx: Shift-F20\n");
        break;
    case K_SNI_S_USER3       :
        printf(
"\\Ksni_s_user3     Transmit SNI-97801-5xx: Shift-Key below F20\n");
        break;
    case K_SNI_S_F21         :
        printf("\\Ksni_s_f21       Transmit SNI-97801-5xx: Shift-F21\n");
        break;
    case K_SNI_S_USER4       :
        printf(
"\\Ksni_s_user4     Transmit SNI-97801-5xx: Shift-Key below F21\n");
        break;
    case K_SNI_S_F22         :
        printf("\\Ksni_s_f22       Transmit SNI-97801-5xx: Shift-F22\n");
        break;
    case K_SNI_S_USER5       :
        printf(
"\\Ksni_s_user5     Transmit SNI-97801-5xx: Shift-Key below F22\n");
        break;
    case K_SNI_S_HELP        :
        printf("\\Ksni_s_help      Transmit SNI-97801-5xx: Shift-Help\n");
        break;
    case K_SNI_S_HOME        :
        printf("\\Ksni_s_home      Transmit SNI-97801-5xx: Shift-Home\n");
        break;
    case K_SNI_S_INSERT_CHAR :
        printf(
"\\Ksni_s_ins_char  Transmit SNI-97801-5xx: Shift-Insert Char\n");
        break;
    case K_SNI_S_INSERT_LINE :
        printf(
"\\Ksni_s_ins_line  Transmit SNI-97801-5xx: Shift-Insert Line\n");
        break;
    case K_SNI_S_INSERT_WORD :
        printf(
"\\Ksni_s_ins_word  Transmit SNI-97801-5xx: Shift-Insert Word\n");
        break;
    case K_SNI_S_LEFT_TAB    :
        printf("\\Ksni_s_left_tab  Transmit SNI-97801-5xx: Shift-Left Tab\n");
        break;
    case K_SNI_S_CURSOR_LEFT :
        printf(
"\\Ksni_s_lfarr     Transmit SNI-97801-5xx: Shift-Cursor Left\n");
        break;
    case K_SNI_S_MODE        :
        printf("\\Ksni_s_mode      Transmit SNI-97801-5xx: Shift-Mode\n");
        break;
    case K_SNI_S_PAGE        :
        printf("\\Ksni_s_page      Transmit SNI-97801-5xx: Shift-Page\n");
        break;
    case K_SNI_S_PRINT       :
        printf("\\Ksni_s_print     Transmit SNI-97801-5xx: Shift-Print\n");
        break;
    case K_SNI_S_CURSOR_RIGHT:
        printf(
"\\Ksni_s_rtarr     Transmit SNI-97801-5xx: Shift-Cursor Right\n");
        break;
    case K_SNI_S_SCROLL_DOWN :
        printf(
"\\Ksni_s_scroll_dn Transmit SNI-97801-5xx: Shift-Scroll Down\n");
        break;
    case K_SNI_S_SCROLL_UP   :
        printf("\\Ksni_s_scroll_up Transmit SNI-97801-5xx: Shift-Scroll Up\n");
        break;
    case K_SNI_S_START       :
        printf("\\Ksni_s_start     Transmit SNI-97801-5xx: Shift-Start\n");
        break;
    case K_SNI_S_CURSOR_UP   :
        printf("\\Ksni_s_uparr     Transmit SNI-97801-5xx: Shift-Cursor Up\n");
        break;
    case K_SNI_S_TAB         :
        printf("\\Ksni_s_tab       Transmit SNI-97801-5xx: Shift-Tab\n");
        break;
    case K_SNI_SCROLL_DOWN   :
        printf("\\Ksni_scroll_dn   Transmit SNI-97801-5xx: Scroll Down\n");
        break;
    case K_SNI_SCROLL_UP     :
        printf("\\Ksni_scroll_up   Transmit SNI-97801-5xx: Scroll Up\n");
        break;
    case K_SNI_START         :
        printf("\\Ksni_start       Transmit SNI-97801-5xx: Start\n");
        break;
    case K_SNI_TAB           :
        printf("\\Ksni_tab         Transmit SNI-97801-5xx: Tab\n");
        break;
    case K_SNI_CURSOR_UP     :
        printf("\\Ksni_uparr       Transmit SNI-97801-5xx: Cursor Up\n");
        break;

    case K_BA80_ATTR:
        printf("\\Kba80_attr       Transmit BA80: Attr\n");
        break;
    case K_BA80_C_KEY:
        printf("\\Kba80_c_key      Transmit BA80: C\n");
        break;
    case K_BA80_CLEAR:
        printf("\\Kba80_clear      Transmit BA80: Clear\n");
        break;
    case K_BA80_CMD:
        printf("\\Kba80_cmd        Transmit BA80: Cmd\n");
        break;
    case K_BA80_COPY:
        printf("\\Kba80_copy       Transmit BA80: Copy\n");
        break;
    case K_BA80_DEL:
        printf("\\Kba80_del        Transmit BA80: Delete\n");
        break;
    case K_BA80_DEL_B:
        printf("\\Kba80_del_b      Transmit BA80: Delete B\n");
        break;
    case K_BA80_DO:
        printf("\\Kba80_do         Transmit BA80: Do\n");
        break;
    case K_BA80_END:
        printf("\\Kba80_end        Transmit BA80: End\n");
        break;
    case K_BA80_ENV:
        printf("\\Kba80_env        Transmit BA80: Env\n");
        break;
    case K_BA80_EOP:
        printf("\\Kba80_eop        Transmit BA80: EOP\n");
        break;
    case K_BA80_ERASE:
        printf("\\Kba80_erase      Transmit BA80: Erase\n");
        break;
    case K_BA80_FMT:
        printf("\\Kba80_fmt        Transmit BA80: Format\n");
        break;
    case K_BA80_HELP:
        printf("\\Kba80_help       Transmit BA80: Help\n");
        break;
    case K_BA80_HOME:
        printf("\\Kba80_home       Transmit BA80: Home\n");
        break;
    case K_BA80_INS:
        printf("\\Kba80_ins        Transmit BA80: Insert\n");
        break;
    case K_BA80_INS_B:
        printf("\\Kba80_ins_b      Transmit BA80: Insert B\n");
        break;
    case K_BA80_MARK:
        printf("\\Kba80_mark       Transmit BA80: Mark\n");
        break;
    case K_BA80_MOVE:
        printf("\\Kba80_move       Transmit BA80: Move\n");
        break;
    case K_BA80_PA01:
        printf("\\Kba80_pa01       Transmit BA80: PA1\n");
        break;
    case K_BA80_PA02:
        printf("\\Kba80_pa02       Transmit BA80: PA2\n");
        break;
    case K_BA80_PA03:
        printf("\\Kba80_pa03       Transmit BA80: PA3\n");
        break;
    case K_BA80_PA04:
        printf("\\Kba80_pa04       Transmit BA80: PA4\n");
        break;
    case K_BA80_PA05:
        printf("\\Kba80_pa05       Transmit BA80: PA5\n");
        break;
    case K_BA80_PA06:
        printf("\\Kba80_pa06       Transmit BA80: PA6\n");
        break;
    case K_BA80_PA07:
        printf("\\Kba80_pa07       Transmit BA80: PA7\n");
        break;
    case K_BA80_PA08:
        printf("\\Kba80_pa08       Transmit BA80: PA8\n");
        break;
    case K_BA80_PA09:
        printf("\\Kba80_pa09       Transmit BA80: PA9\n");
        break;
    case K_BA80_PA10:
        printf("\\Kba80_pa10       Transmit BA80: PA10\n");
        break;
    case K_BA80_PA11:
        printf("\\Kba80_pa11       Transmit BA80: PA11\n");
        break;
    case K_BA80_PA12:
        printf("\\Kba80_pa12       Transmit BA80: PA12\n");
        break;
    case K_BA80_PA13:
        printf("\\Kba80_pa13       Transmit BA80: PA13\n");
        break;
    case K_BA80_PA14:
        printf("\\Kba80_pa14       Transmit BA80: PA14\n");
        break;
    case K_BA80_PA15:
        printf("\\Kba80_pa15       Transmit BA80: PA15\n");
        break;
    case K_BA80_PA16:
        printf("\\Kba80_pa16       Transmit BA80: PA16\n");
        break;
    case K_BA80_PA17:
        printf("\\Kba80_pa17       Transmit BA80: PA17\n");
        break;
    case K_BA80_PA18:
        printf("\\Kba80_pa18       Transmit BA80: PA18\n");
        break;
    case K_BA80_PA19:
        printf("\\Kba80_pa19       Transmit BA80: PA19\n");
        break;
    case K_BA80_PA20:
        printf("\\Kba80_pa20       Transmit BA80: PA20\n");
        break;
    case K_BA80_PA21:
        printf("\\Kba80_pa21       Transmit BA80: PA21\n");
        break;
    case K_BA80_PA22:
        printf("\\Kba80_pa22       Transmit BA80: PA22\n");
        break;
    case K_BA80_PA23:
        printf("\\Kba80_pa23       Transmit BA80: PA23\n");
        break;
    case K_BA80_PA24:
        printf("\\Kba80_pa24       Transmit BA80: PA24\n");
        break;
    case K_BA80_PGDN:
        printf("\\Kba80_pgdn       Transmit BA80: Page Down\n");
        break;
    case K_BA80_PGUP:
        printf("\\Kba80_pgup       Transmit BA80: Page Up\n");
        break;
    case K_BA80_PICK:
        printf("\\Kba80_pick       Transmit BA80: Pick\n");
        break;
    case K_BA80_PRINT:
        printf("\\Kba80_print      Transmit BA80: Print\n");
        break;
    case K_BA80_PUT:
        printf("\\Kba80_put        Transmit BA80: Put\n");
        break;
    case K_BA80_REFRESH:
        printf("\\Kba80_refresh    Transmit BA80: Refresh \n");
        break;
    case K_BA80_RESET:
        printf("\\Kba80_reset      Transmit BA80: Reset\n");
        break;
    case K_BA80_RUBOUT:
        printf("\\Kba80_rubout     Transmit BA80: Rubout\n");
        break;
    case K_BA80_SAVE:
        printf("\\Kba80_save       Transmit BA80: Save\n");
        break;
    case K_BA80_SOFTKEY1:
        printf("\\Kba80_softkey1   Transmit BA80: Softkey 1\n");
        break;
    case K_BA80_SOFTKEY2:
        printf("\\Kba80_softkey2   Transmit BA80: Softkey 2\n");
        break;
    case K_BA80_SOFTKEY3:
        printf("\\Kba80_softkey3   Transmit BA80: Softkey 3\n");
        break;
    case K_BA80_SOFTKEY4:
        printf("\\Kba80_softkey4   Transmit BA80: Softkey 4\n");
        break;
    case K_BA80_SOFTKEY5:
        printf("\\Kba80_softkey5   Transmit BA80: Softkey 5\n");
        break;
    case K_BA80_SOFTKEY6:
        printf("\\Kba80_softkey6   Transmit BA80: Softkey 6\n");
        break;
    case K_BA80_SOFTKEY7:
        printf("\\Kba80_softkey7   Transmit BA80: Softkey 7\n");
        break;
    case K_BA80_SOFTKEY8:
        printf("\\Kba80_softkey8   Transmit BA80: Softkey 8\n");
        break;
    case K_BA80_SOFTKEY9:
        printf("\\Kba80_softkey9   Transmit BA80: Softkey 9\n");
        break;
    case K_BA80_UNDO:
        printf("\\Kba80_undo       Transmit BA80: Undo\n");
        break;

    default:
      printf("No additional help available for this kverb\n");
  }
    printf("\n");

    /* This is not the proper way to do it since it doesn't show  */
    /* all emulations, nor does it show the special modes, but it */
    /* is better than nothing.                                    */

    printf("Current bindings:\n");
    found = 0;
    for (i = 256; i < KMSIZE ; i++) {
        con_event evt = mapkey(i);
        if (evt.type != kverb)
          continue;
        if ((evt.kverb.id & ~F_KVERB) == xx) {
            found = 1;
            printf("  \\%-4d - %s\n",i,keyname(i));
        }
    }
#ifdef OS2MOUSE
    for ( button = 0 ; button < MMBUTTONMAX ; button++ )
      for ( event = 0 ; event < MMEVENTSIZE ; event++ )
        if ( mousemap[button][event].type == kverb ) {
            if ( (mousemap[button][event].kverb.id & ~F_KVERB) == xx ) {
                found = 1;
                printf("  Mouse - %s\n",mousename(button,event));
            }
        }
#endif /* OS2MOUSE */

    if ( !found ) {
        printf("  (none)\n");
    }
    return(0);
}
#endif /* NOKVERBS */
#endif /* OS2 */

#ifndef NOXFER
/*  D O H R M T  --  Give help about REMOTE command  */

static char *hrset[] = {
"Syntax:  REMOTE SET parameter value",
"Example: REMOTE SET FILE TYPE BINARY",
"  Asks the Kermit server to set the named parameter to the given value.",
"  Equivalent to typing the corresponding SET command directly to the other",
"  Kermit if it were in interactive mode.", "" };

int
dohrmt(xx) int xx; {
    int x;
    if (xx == -3) return(hmsga(hmhrmt));
    if (xx < 0) return(xx);
    if ((x = cmcfm()) < 0) return(x);
    switch (xx) {

case XZCPY:
    return(hmsg("Syntax: REMOTE COPY source destination\n\
  Asks the Kermit server to copy the source file to destination."));

case XZCWD:
    return(hmsg("Syntax: REMOTE CD [ name ]\n\
  Asks the Kermit server to change its working directory or device.\n\
  If the device or directory name is omitted, restore the default."));

case XZDEL:
    return(hmsg("Syntax: REMOTE DELETE filespec\n\
  Asks the Kermit server to delete the named file(s)."));

case XZMKD:
    return(hmsg("Syntax: REMOTE MKDIR directory-name\n\
  Asks the Kermit server to create the named directory."));

case XZRMD:
    return(hmsg("Syntax: REMOTE RMDIR directory-name\n\
  Asks the Kermit server to remove the named directory."));

case XZDIR:
    return(hmsg("Syntax: REMOTE DIRECTORY [ filespec ]\n\
  Asks the Kermit server to provide a directory listing of the named\n\
  file(s) or if no file specification is given, of all files in the current\n\
  directory."));

case XZHLP:
    return(hmsg("Syntax: REMOTE HELP\n\
  Asks the Kermit server to list the services it provides."));

case XZHOS:
    return(hmsg("Syntax: REMOTE HOST command\n\
  Sends a command to the other computer in its own command language\n\
  through the Kermit server that is running on that host."));

#ifndef NOFRILLS
case XZKER:
    return(hmsg("Syntax: REMOTE KERMIT command\n\
  Sends a command to the remote Kermit server in its own command language."));

case XZLGI:
    return(hmsg("Syntax: REMOTE LOGIN user password [ account ]\n\
  Logs in to a remote Kermit server that requires you login."));

case XZLGO:
    return(hmsg("Syntax: REMOTE LOGOUT\n\
  Logs out from a remote Kermit server to which you have previously logged in."
));

case XZPRI:
    return(hmsg("Syntax: REMOTE PRINT filespec [ options ]\n\
  Sends the specified file(s) to the remote Kermit and ask it to have the\n\
  file printed on the remote system's printer, using any specified options."));
#endif /* NOFRILLS */

case XZREN:
    return(hmsg("Syntax: REMOTE RENAME filespec newname\n\
  Asks the Kermit server to rename the file."));

case XZSET:
    return(hmsga(hrset));

case XZSPA:
    return(hmsg("Syntax: REMOTE SPACE [ name ]\n\
  Asks the Kermit server to tell you about its disk space on the current\n\
  disk or directory, or in the one that you name."));

#ifndef NOFRILLS
case XZTYP:
    return(hmsg("Syntax: REMOTE TYPE file\n\
  Asks the Kermit server to type the named file(s) on your screen."));

case XZWHO:
    return(hmsg("Syntax: REMOTE WHO [ name ]\n\
  Asks the Kermit server to list who's logged in, or to give information\n\
  about the named user."));
#endif /* NOFRILLS */

#ifndef NOSPL
case XZQUE:
    return(hmsg(
"Syntax: REMOTE QUERY { KERMIT, SYSTEM, USER } variable-name\n\
  Asks the Kermit server to send the value of the named variable of the\n\
  given type, and make it available in the \\v(query) variable.  When the\n\
  type is KERMIT functions may also be specified as if they were variables."));

case XZASG:
    return(hmsg(
"Syntax: REMOTE ASSIGN variable-name [ value ]\n\
  Assigns the given value to the named global variable on the server."));
#endif /* NOSPL */

case XZXIT:
    return(hmsg("Syntax: REMOTE EXIT\n\
  Asks the Kermit server to exit."));

default:
    if ((x = cmcfm()) < 0) return(x);
    printf("not working yet - %s\n",cmdbuf);
    return(-2);
    }
}
#endif /* NOXFER */
#endif /* NOHELP */
#endif /* NOICP */
