/*  C K U U S 2  --  User interface strings & help text module for C-Kermit  */
 
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/
 
/*
 This module separates long strings from the body of the other ckuus* modules.
*/

#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcnet.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"
 
extern xx_strp xxstring;
extern char *ccntab[];

#ifndef NOICP
#ifdef DCMDBUF
extern char *cmdbuf;
#else
extern char cmdbuf[];
#endif /* DCMDBUF */
#endif /* NOICP */

#ifdef DEBUG
extern char debfil[];
#endif /* DEBUG */
#ifdef TLOG
extern char trafil[];
#endif

extern char *xarg0;
extern int nrmt, nprm, dfloc, local, parity, escape;
extern int turn, flow;
extern int binary, warn, quiet, keep;
extern int success;
#ifdef OS2
extern int tt_rows[], tt_cols[];
#else /* OS2 */
extern int tt_rows, tt_cols;
#endif /* OS2 */
extern int cmd_rows, cmd_cols;

extern long speed;
extern char *dftty, *versio, *ckxsys;
extern struct keytab prmtab[];
extern struct keytab remcmd[];
 
/* Command-Line help (Unix command-line arguments) */

#ifndef NOCMDL
static
char *hlp1[] = {
#ifndef NOICP
" [cmdfile] [-x arg [-x arg]...[-yyy]..] [ = text ] ]\n",
#else
"[-x arg [-x arg]...[-yyy]..]\n",
#endif /* NOICP */
"  -x is an option requiring an argument, -y an option with no argument.\n",
#ifdef COMMENT /* No room for this any more */
#ifndef NOICP
#ifndef NOSPL
"     = means ignore following words, but place them in array \\&@[].\n",
#else
"     = means ignore following material.\n",
#endif /* NOSPL */
#else
"     = means ignore following material.\n",
#endif /* NOICP */
#endif /* COMMENT */
"actions:\n",
"  -s files  send files                  -r  receive files\n",
"  -s -      send from stdin             -k  receive files to stdout\n",
#ifndef NOSERVER
"  -x        enter server mode           -f  finish remote server\n",
#else
"  -f        finish remote server\n",
#endif /* NOSERVER */
"  -g files  get remote files from server (quote wildcards)\n",
"  -a name   alternate file name, used with -s, -r, -g\n",
#ifndef NOLOCAL
"  -c        connect (before file transfer), used with -l and -b\n",
"  -n        connect (after file transfer), used with -l and -b\n",
#endif /* NOLOCAL */
"settings:\n",
#ifndef NOLOCAL
"  -l dev    communication line device   -q  quiet during file transfer\n",
#ifdef NETCONN
"  -j host   network host name[:port]    -i  binary transfer (-T = text)\n",
#else
"  -i        binary file transfer\n",
#endif /* NETCONN */
"  -b bps    line speed, e.g. 19200      -t  half duplex, xon handshake\n",
#else
"  -i        binary file transfer\n",
#endif /* NOLOCAL */
#ifdef DEBUG
"  -p x      parity, x = e,o,m,s, or n   -d  log debug info to debug.log\n",
#else
"  -p x      parity, x = e,o,m,s, or n\n",
#endif /* DEBUG */
#ifndef NOICP
"  -y name   alternate init file name    -Y  no init file\n",
#else
#endif /* NOICP */
"  -e n      receive packet length       -w  write over files\n",
#ifdef UNIX
"  -v n      sliding window slots        -z  force foreground\n",
#else
"  -v n      sliding window slots\n",
#endif /* UNIX */
#ifndef NODIAL
"  -m name   modem type                  -R  remote-only advisory\n",
#endif /* NODIAL */
/*
  If all this stuff is defined, we run off the screen...
*/
#ifdef CK_NETBIOS
"  -N n      NetBIOS adapter number\n",
#endif /* CK_NETBIOS */
#ifdef ANYX25
" -o index   X.25 closed user group call -X  X.25 address\n",
" -U string  X.25 call user data         -u  X.25 reverse charge call\n",
" -Z n       X.25 connection open file descriptor\n",
#endif /* ANYX25 */
#ifndef NOSPL
"other:\n",
"  -C \"command, command, ...\"  (interactive-mode commands to execute)\n",
"   =  means ignore following words, but place them in array \\&@[].\n",
#else
"   =  means ignore following text.\n",
#endif /* NOSPL */
#ifdef NOICP
"Operation by command-line options only.\n",
#else
"If no action command is included, or -S is, enter interactive dialog.\n",
#endif /* NOICP */
""
};
 
/*  U S A G E */
 
VOID
usage() {
#ifndef MINIX
    conol("Usage: ");
    conol(xarg0);
    conola(hlp1);
#else
    conol("Usage: ");
    conol(xarg0);
    conol(" [-x arg [-x arg]...[-yyy]..] ]\n");
#endif /* MINIX */
}
#endif /* NOCMDL */

#ifndef NOICP

/*  Interactive help strings  */
 
static char *tophlp[] = { 
"Trustees of Columbia University in the City of New York.\n",

#ifndef NOHELP
#ifdef NT
"  Type INTRO   for a brief introduction to the Kermit Command screen.",
#else
"  Type INTRO   for a brief introduction to C-Kermit.",
#endif /* NT */
"  Type VERSION for version and copyright information.",
"  Type HELP    followed by a command name for help about a specific command.",
"  Type NEWS    for news about new features.",
#ifdef OS2
#ifndef NT
"  Type UPDATES to view updates to the \"Using C-Kermit\" manual.",
#endif /* NT */
#endif /* OS2 */
"  Type BUG     to learn how to get technical support.",
"  Press ?      (question mark) at the prompt, or anywhere within a command,",
"               for a menu (context-sensitive help, menu on demand).",
#else
"Press ? for a list of commands; see documentation for detailed descriptions.",
#endif /* NOHELP */

#ifndef NOCMDL
#ifdef NT
"\n\
  From DOS level, type \"k95 -h\" for help about command-line options.",
#else
#ifdef OS2
"\n\
  From system level, type \"k2 -h\" for help about command-line options.",
#else
"\n\
  From system level, type \"kermit -h\" for help about command-line options.",
#endif /* OS2 */
#endif /* NT */
#endif /* NOCMDL */
" ",
#ifdef NT
"Documentation: \"Kermit 95\" by Christine M. Gianone and Frank da Cruz,",
"Manning Publications, 1996.  Technical Reference: \"Using C-Kermit\" by",
"Frank da Cruz and Christine M. Gianone, 2nd Ed., Digital Press, 1997.",
"To order call +1 (212) 854-3703.",
#else
#ifdef MAC
"Documentation for Command Window: \"Using C-Kermit\" by Frank da Cruz and",
"Christine M. Gianone, Digital Press, 1997, ISBN: 1-55558-164-1.  To order,",
"call +1 212 854-3703 or +1 800 366-2665.",
#else
"DOCUMENTATION: \"Using C-Kermit\" by Frank da Cruz and Christine M. Gianone,",
"2nd Edition, Digital Press / Butterworth-Heinemann 1997, ISBN 1-55558-164-1.",
"To order: +1 212 854-3703 or +1 800 366-2665.  PLEASE PURCHASE THE MANUAL.",
"It shows you how to use C-Kermit and answers your questions; sales of this",
"book are the primary source of funding for C-Kermit development and support.",
#endif /* MAC */
#endif /* NT */
" ",
#ifdef OS2ONLY
"Use the UPDATES command to access hypertext documentation for features added",
"since the book was published. Use UPDATES <keyword> to search for a keyword.",
#else
#ifdef MAC
"Also see the Mac Kermit Doc and Bwr files on the Mac Kermit diskette.\n",
#else
#ifdef HPUX10
"See /usr/share/lib/kermit/*.* for additional information.", 
#endif /* HPUX10 */
#endif /* MAC */
#endif /* OS2 */
""
};

#ifndef NOHELP 
char *newstxt[] = {
" Welcome to C-Kermit 6.0.  This version of C-Kermit was released",
" concurrently with the second edition of the manual, \"Using C-Kermit\",",
" and it is completely described there, with supplementary material found",
" in system-specific publications, such as the Kermit 95 manual.  The",
" update files that accompanied edits 189, 190, and 191 are no longer needed.",
" ",
" If the release date shown by the VERSION command is long past, be sure to",
" check with the Kermit Project to see if have been updates.",
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
" . Error-free and efficient file transfer",
#ifdef OS2
" . VT320/220/102/100/52, ANSI, Wyse, DG, Televideo, and other emulations",
#else
#ifdef MAC
" . VT220 terminal emulation",
#else
" . Terminal connection",
#endif /* MAC */
#endif /* OS2 */
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
"  SET NETWORK DIRECTORY  specify a network services direction (optional)",
"  LOOKUP                 lookup entries in your network directory",
"  SET NETWORK   select network type",
"  SET HOST      select network host",
"  CONNECT       begin terminal connection",
#ifdef TNCODE
"  TELNET        select a TCP/IP host and CONNECT to it",
#endif /* TNCODE */
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
"\nTo display your escape character:",
"  SHOW ESCAPE",
"\nTo display other settings:",
"  SHOW COMMUNICATIONS, SHOW TERMINAL, SHOW FILE, SHOW PROTOCOL, etc.",
#else  /* !NOLOCAL */
"\nTo display settings:",
"  SHOW COMMUNICATIONS, SHOW TERMINAL, SHOW FILE, SHOW PROTOCOL, etc.",
#endif /* NOLOCAL */
"\nTo speed up file transfers:",
"  SET RECEIVE PACKET-LENGTH  (use bigger packets)",
"  SET WINDOW                 (use sliding windows)",
"  SET PREFIXING              (reduce prefixing overhead)",
"  FAST                       (combination of the three above)",

"\nFor further information about a particular command, type HELP xxx,",
"where xxx is the name of the command.  For documentation, news of new",
"releases, and information about other Kermit software, contact:\n",
"  The Kermit Project         E-mail: kermit-orders@columbia.edu",
"  Columbia University        Web:    http://www.columbia.edu/kermit/",
"  612 West 115th Street      Voice:  +1 (212) 854-3703",
"  New York NY  10025-7799    Fax:    +1 (212) 663-8202",
"  USA",
""
};

static char *hxxinp[] = {
"Syntax:  INPUT { number-of-seconds, time-of-day } [ text ]",
"Example: INPUT 5 Login:  or  INPUT 23:59:59 RING",
" ",
"Waits up to the given number of seconds, or until the given time of day",
"for the given text to arrive on the connection.  If no text is given, INPUT",
"waits for any character.  For use in script programs with IF FAILURE and",
"IF SUCCESS.  Also see MINPUT, REINPUT, SET INPUT.  See PAUSE for details on",
"time-of-day format.",
""};

#ifdef CK_XYZ
#ifdef XYZ_INTERNAL
static char *hxyixyz[] = {
"Syntax: SET PROTOCOL { KERMIT, XMODEM, YMODEM, ZMODEM } [ s1 s2 ]",
" ",
"Selects protocol to use for transferring files.  String 1 is a command to",
"output prior to SENDing with this protocol in binary mode; string 2 is the",
"same thing but for text mode.  Use \"%\" in any of these strings to",
"represent the filename(s).  Use { braces } if any command contains spaces.",
"Example:",
" ",
"  set proto xmodem {rx %s} {rx -a %s}",
" ",
""};
#else
static char *hxyxyz[] = {
"Syntax: SET PROTOCOL { KERMIT, XMODEM, YMODEM, ZMODEM } [ s1 s2 s3 s4 s5 s6 ]",
" ",
"Selects protocol to use for transferring files.  s1 and s2 are commands to",
"output prior to SENDing with this protocol, to automatically start the",
"RECEIVE process on the other end in binary or text mode, respectively.",
"s3 and s4 are commands used on this computer for sending files with this",
"protocol in binary or text mode.  s5 and s6 are the commands for receiving",
"files with this protocol.  Use \"%s\" in any of these strings to represent",
"the filename(s).  Use { braces } if any command contains spaces.  Example:",
" ",
"  set proto ymodem rb {rb -a} {sb %s} {sb -a %s} rb rb",
" ",
"NOTE: This feature requires REDIRECT and external protocols that can be",
"redirected.",
""};
#endif /* CK_XYZ */
#endif /* XYZ_INTERNAL */

static char *hmxxbye = "Syntax: BYE\n\
Shut down and log out a remote Kermit server";
 
static char *hmxxclo[] = {
"Syntax:  CLOSE name",
"Example: CLOSE PACKET\n",
"Close one of the following logs or files:",
#ifndef NOLOCAL
"  SESSION",
#endif /* NOLOCAL */
#ifdef TLOG
"  TRANSACTION",
#endif /* TLOG */
"  PACKET",
#ifdef DEBUG
"  DEBUGGING",
#endif /* DEBUG */
#ifndef NOSPL
"  READ",
"  WRITE",
#endif /* NOSPL */
"Type HELP LOG and HELP OPEN for further info.", "" };
 
#ifdef CK_MINPUT
static char *hmxxminp[] = {
"Syntax:  MINPUT n [ string1 [ string2 [ ... ] ] ]",
"Example: MINPUT 5 Login: {Username: } {NO CARRIER} BUSY RING\n",
"For use in script programs.  Waits up to n seconds for any one of the",
"strings to arrive on the communication device.  If no strings are given, the",
"command waits for any character at all to arrive.  Strings are separated by",
"spaces; use { braces } for grouping.  If any of the strings is encountered",
"within the timeout interval, the command succeeds and the \\v(minput)",
"variable is set to the number of the string that was matched: 1, 2, 3, etc.",
"If none of the strings arrives, the command times out, fails, and",
"\\v(minput) is set to 0.\n",
"Also see: INPUT, REINPUT, SET INPUT.",
"" };
#endif /* CK_MINPUT */

#ifndef NOLOCAL
static char *hmxxcon[] = {
#ifdef OS2
"Syntax: CONNECT (or C)\n",
#else
"Syntax: CONNECT (or C) [/QUIETLY]\n",
#endif /* OS2 */
"Connect to a remote computer via the serial communications device given in",
#ifdef OS2
"the most recent SET PORT command, or to the network host named in the most",
#else
"the most recent SET LINE command, or to the network host named in the most",
#endif /* OS2 */
"recent SET HOST command.  Type the escape character followed by C to get",
"back to the C-Kermit prompt, or followed by ? for a list of CONNECT-mode",
#ifdef OS2
"escape commands.  You can also assign the \\Kexit verb to the key or",
"key-combination of your choice; by default it is assigned to Alt-x.",
#else
"escape commands.",
"\nInclude the /QUIETLY switch to suppress the informational message that",
"tells you how to escape back, etc.",
#endif /* OS2 */
"" };
#endif /* NOLOCAL */
 
static char *hmxxget = "Syntax: GET filespec\n\
Tell the remote Kermit server to send the named file or files.\n\
If the filespec is omitted, then you are prompted for the remote and\n\
local filenames separately.";

#ifdef OS2ONLY
static char *hmxxupd[] = {
"Syntax: UPDATES [ keyword ]",
"Loads the C-Kermit INF files with the OS/2 VIEW command for complete",
"documentation of all changes and new features in OS/2 C-Kermit since the",
"first edition of \"Using C-Kermit\" was published.  This command is not a",
"substitute for the manual, but a supplement to it.",
" ",
"If a keyword is given, VIEW searches the index for it and, if it is found,",
"goes straight to the first panel indexed for the keyword.",
" ",
"Upon quitting from VIEW (via Services:Exit or by closing the window) you",
"are returned to the C-Kermit> prompt.",
""
};
#endif /* OS2ONLY */

static char *hmxxlg[] = { "Syntax: LOG (or L) name [ { NEW, APPEND } ]",
"Record information in a log file:\n",
#ifdef DEBUG
"DEBUGGING     Debugging information, to help track down bugs in the C-Kermit",
"              program (default log name is debug.log).\n",
#endif /* DEBUG */
"PACKETS       Kermit packets, to help with protocol problems (packet.log)",
#ifndef NOLOCAL
"SESSION       Terminal session, during CONNECT command (session.log)",
#endif /* NOLOCAL */
#ifdef TLOG
"TRANSACTIONS  Names and statistics about files transferred (transact.log)\n",
#endif /* TLOG */
"If you include the APPEND keyword after the filename, the existing log file,",
"if any, is appended to; otherwise a new file is created.",
"Use CLOSE to stop logging.",
"" };
 
#ifndef NOSCRIPT
static char *hmxxlogi[] = { "\
Syntax: SCRIPT text\n",
"A limited and cryptic \"login assistant\", carried over from old C-Kermit",
"releases for comptability, but not recommended for use.  Instead, please",
"use the full script programming language described in chapters 11-13 of",
"\"Using C-Kermit\".\n",
"Login to a remote system using the text provided.  The login script",
"is intended to operate similarly to UNIX uucp \"L.sys\" entries.",
"A login script is a sequence of the form:\n",
"  expect send [expect send] . . .\n",
"where 'expect' is a prompt or message to be issued by the remote site, and",
"'send' is the names, numbers, etc, to return.  The send may also be the",
"keyword EOT, to send control-d, or BREAK (or \\\\b), to send a break signal.",
"Letters in send may be prefixed by ~ to send special characters.  These are:",
"~b backspace, ~s space, ~q '?', ~n linefeed, ~r return, ~c don\'t",
"append a return, and ~o[o[o]] for octal of a character.  As with some",
"uucp systems, sent strings are followed by ~r unless they end with ~c.\n",
"Only the last 7 characters in each expect are matched.  A null expect,",
"e.g. ~0 or two adjacent dashes, causes a short delay.  If you expect",
"that a sequence might not arrive, as with uucp, conditional sequences",
"may be expressed in the form:\n",
"  -send-expect[-send-expect[...]]\n",
"where dashed sequences are followed as long as previous expects fail.",
"" };
#endif /* NOSCRIPT */
 
static char *hmxxrc[] = { "Syntax: RECEIVE (or R) [filespec]\n",
"Wait for a file to arrive from the other Kermit, which must be given a",
"SEND command.  If the optional filespec is given, the (first) incoming",
"file will be stored under that name, otherwise it will be stored under",
#ifndef CK_TMPDIR
"the name it arrives with.",
#else
#ifdef OS2
"the name it arrives with.  If the filespec denotes a disk and/or directory,",
"the incoming file or files will be stored there.",
#else
"the name it arrives with.  If the filespec denotes a directory, the",
"incoming file or files will be placed in that directory.",
#endif /* OS2 */
#endif /* CK_TMPDIR */
"" };
 
static char *hmxxsen = "\
Syntax: SEND (or S) filespec [name]\n\n\
Send the file or files specified by filespec.\n\
filespec may contain wildcard characters '*' or '?'.  If no wildcards,\n\
then 'name' may be used to specify the name 'filespec' is sent under; if\n\
'name' is omitted, the file is sent under its own name.  Also see ADD.";
 
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
static char *hmssmse = "\
Syntax: MSEND filespec [ filespec [ ... ] ]\n\n\
Send the files specified by the filespecs.  One or more filespecs may be\n\
listed, separated by spaces.  Any or all filespecs may contain wildcards\n\
and they may be in different directories.  An alternate name cannot be given.";

static char *hmssadd = "\
Syntax: ADD filespec [ <mode> [ <as-name> ] ]\n\n\
Add the file or files to the current SEND list.  Use SHOW SEND-LIST and\n\
CLEAR SEND-LIST to display and clear the list, use SEND by itself to send it.";
#endif /* NOMSEND */

#ifndef NOSERVER
static char *hmxxser = "Syntax: SERVER\n\n\
Enter server mode on the currently selected line.  All further commands\n\
will be taken in packet form from the other Kermit program.  Use FINISH\n\
or BYE to get C-Kermit out of server mode.";
#endif /* NOSERVER */
 
static char *hmhset[] = { "\
The SET command is used to establish various communication or file",
"parameters.  The SHOW command can be used to display the values of",
"SET parameters.  Help is available for each individual parameter;",
"type HELP SET ? to see what's available.",
"" };
 
#ifndef NOSETKEY
static char *hmhskey[] = {
"Syntax: SET KEY k text",
"Or:     SET KEY CLEAR\n",
"Configure the key whose \"scan code\" is k to send the given text when",
"pressed during CONNECT mode.  SET KEY CLEAR restores all the default",
"key mappings.  If there is no text, the default key binding is restored for",
#ifndef NOCSETS
"the key k.  SET KEY mappings take place before terminal character-set",
"translation.",
#else
"the key k.",
#endif /* NOCSETS */
#ifdef OS2
"\nThe text may contain \"\\Kverbs\" to denote actions, to stand for DEC",
"keypad, function, or editing keys, etc.  For a list of available keyboard",
"verbs, type SHOW KVERBS.",
#endif /* OS2 */
"\nTo find out the scan code and mapping for a particular key, use the",
"SHOW KEY command.",
""};
#endif /* NOSETKEY */

static char *hmxychkt[] = { "Syntax: SET BLOCK-CHECK type\n",
"Type of packet block check to be used for error detection, 1, 2, 3, or",
"BLANK-FREE-2.  Type 1 is standard, and catches most errors.  Types 2 and 3",
"specify more rigorous checking at the cost of higher overhead.  The",
"BLANK-FREE-2 type is the same as Type 2, but is guaranteed to contain no",
"blanks.",
"" };
 
#ifdef CK_SPEED
static char *hmxyqctl[] = {
"Syntax: SET CONTROL-CHARACTER { PREFIXED, UNPREFIXED } { <code>..., ALL }\n",
"<code> is the numeric ASCII code for a control character 1-31, 127-159, 255.",
"The word \"ALL\" means the command applies to all characters in this range.",
"\nPREFIXED <code> means the given control character must be converted to a",
" printable character and prefixed, the default for all control characters.",
"\nUNPREFIXED <code> means you think it is safe to send the given control",
" character as-is, without a prefix.  USE THIS OPTION AT YOUR OWN RISK!",
"\nSHOW CONTROL to see current settings.  SET CONTROL PREFIXED ALL is",
"recommended for safety.  You can include multiple <code> values in one",
"command, separated by spaces.",
"" };
#endif /* CK_SPEED */

#ifndef NODIAL
static char *hxymodm[] = {
"Syntax: SET MODEM <parameter> <value> ...\n",
"Note: Many of the SET MODEM parameters are configured automatically when",
"you SET MODEM TYPE, according to the modem's capabilities.  SHOW MODEM to",
"see them.  Also see HELP DIAL and HELP SET DIAL.\n",
"SET MODEM TYPE <name>",
"  Tells Kermit which kind of modem you have, so it can issue the appropriate",
"  modem-specific commands for configuration, dialing, and hanging up.  For a",
"  list of the modem types known to Kermit, type \"set modem type ?\".",
"  Use SET MODEM TYPE NONE (the default) for direct serial connections.  Use",
"  SET MODEM TYPE USER-DEFINED to use a type of modem that is not built in",
"  to Kermit, and then user SET MODEM CAPABILITIES, SET MODEM DIAL-COMMAND,",
"  and SET MODEM COMMAND to tell Kermit how to configure and control it.\n",
#ifdef UNIX
"  Give the SET MODEM TYPE command BEFORE the SET LINE command so Kermit can",
"  open the communications device in the correct mode for dialing.\n",
#endif /* UNIX */

"SET MODEM CAPABILITIES <list>",
"  Use this command for changing Kermit's idea of your modem's capabilities,",
"  for example, if your modem is supposed to have built-in error correction",
"  but in fact does not.  Also use this command to define the capabilities", 
"  of a USER-DEFINED modem.  Capabilities are:\n",
"   AT      AT-commands",
"   DC      data-compression",
"   EC      error-correction",
"   HWFC    hardware-flow",
"   ITU     v25bis-commands",
"   SWFC    software-flow",
"   KS      kermit-spoof",
"   SB      speed-buffering",
"   TB      Telebit\n",
"SET MODEM CARRIER-WATCH { AUTO, ON, OFF }",
"  Synonym for SET CARRIER-WATCH (q.v.)\n",
"SET MODEM COMPRESSION { ON, OFF }",
"  Enables/disables the modem's data compression feature, if any.\n",
"SET MODEM DIAL-COMMAND <text>",
"  The text replaces Kermit's built-in modem dialing command.  It must",
"  include '%s' (percent s) as a place-holder for the telephone numbers",
"  given in your DIAL commands.\n",
"SET MODEM ERROR-CORRECTION { ON, OFF }",
"  Enables/disables the modem's error-correction feature, if any.\n",
"SET MODEM ESCAPE-CHARACTER number",
"  Numeric ASCII value of modem's escape character, e.g. 43 for '+'.",
"  For Hayes-compatible modems, Kermit uses three copies, e.g. \"+++\".\n",
"SET MODEM FLOW-CONTROL {AUTO, NONE, RTS/CTS, XON/XOFF}",
"  Selects the type of local flow control to be used by the modem.\n",
"SET MODEM HANGUP-METHOD { MODEM-COMMAND, RS232-SIGNAL }",
"  How hangup operations should be done.  MODEM-COMMAND means try to",
"  escape back to the modem's command processor and give a modem-specific",
"  hangup command.  RS232-SIGNAL means turn off the DTR signal.\n",
"SET MODEM KERMIT-SPOOF {ON, OFF}",
"  If the selected modem type supports the Kermit protocol directly,",
"  use this command to turn its Kermit protocol function on or off.\n",
"SET MODEM MAXIMUM-SPEED <number>",
"  Specify the maximum interface speed for the modem.\n",
"SET MODEM NAME <text>",
"  Descriptive name for a USER-DEFINED modem.\n",
"SET MODEM SPEED-MATCHING {ON, OFF}",
"  ON means that C-Kermit changes its serial interface speed to agree with",
"  the speed reported by the modem's CONNECT message, if any.  OFF means",
"  Kermit should not change its interface speed.\n",
"SET MODEM COMMAND commands are used to override built-in modem commands for",
"each modem type, or to fill in commands for the USER-DEFINED modem type.",
"Omitting the optional [ text ] restores the built-in modem-specific command,",
"if any:\n",
"SET MODEM COMMAND AUTOANSWER {ON, OFF} [ text ]",
"  Modem commands to turn autoanswer on and off.\n",
"SET MODEM COMMAND COMPRESSION {ON, OFF} [ text ]",
"  Modem commands to turn data compression on and off.\n",
"SET MODEM COMMAND ERROR-CORRECTION {ON, OFF} [ text ]",
"  Modem commands to turn error correction on and off.\n",
"SET MODEM COMMAND HANGUP [ text ]",
"  Command that tells the modem to hang up the connection.\n",
"SET MODEM COMMAND INIT-STRING [ text ]",
"  The 'text' is a replacement for C-Kermit's built-in initialization command",
"  for the modem.\n",
"SET MODEM COMMAND HARDWARE-FLOW [ text ]",
"  Modem command to enable hardware flow control (RTS/CTS) in the modem.\n",
"SET MODEM COMMAND SOFTWARE-FLOW [ text ]",
"  Modem command to enable local software flow control (Xon/Xoff) in modem.\n",
"SET MODEM COMMAND NO-FLOW-CONTROL [ text ]",
"  Modem command to disable local flow control in the modem.\n",
"SET MODEM COMMAND PULSE [ text ]",
"  Modem command to select pulse dialing.\n",
"SET MODEM COMMAND TONE [ text ]",
"  Modem command to select tone dialing.\n",
"",
};

static char *hmxydial[] = {
"The SET DIAL command establishes or changes all parameters related to",
"dialing the telephone.  Also see HELP DIAL and HELP SET MODEM.  Use SHOW",
"DIAL to display all of the SET DIAL values.\n",
"SET DIAL COUNTRY-CODE <number>",
"  Tells Kermit the telephonic country-code of the country you are dialing",
"  from, so it can tell whether a portable-format phone number from your",
"  dialing directory will result in a national or an international call.",
"  Examples: 1 for USA, Canada, Puerto Rico, etc; 7 for Russia, 39 for Italy,",
"  351 for Portugal, 47 for Norway, 44 for the UK, 972 for Israel, 81 for",
"  Japan, ...\n",
"  If you have not already set your DIAL INTL-PREFIX and LD-PREFIX, then this",
"  command sets default values for them: 011 and 1, respectively, for country",
"  code 1; 00 and 0, respectively, for all other country codes.  If these are",
"  not your true international and long-distance dialing prefixes, then you",
"  should follow this command by DIAL INTL-PREFIX and LD-PREFIX to let Kermit",
"  know what they really are.\n",
"SET DIAL AREA-CODE [ <number> ]",
"  Tells Kermit the area or city code that you are dialing from, so it can",
"  tell whether a portable-format phone number from the dialing directory is",
"  local or long distance.  Be careful not to include your long-distance",
"  dialing prefix as part of your area code; for example, the area code for",
"  central London is 171, not 0171.\n",
"SET DIAL CONFIRMATION {ON, OFF}",
"  Kermit does various transformations on a telephone number retrieved from",
"  the dialing directory prior to dialing (use LOOKUP <name> to see them).",
"  In case the result might be wrong, you can use SET DIAL CONFIRM ON to have",
"  Kermit ask you if it is OK to dial the number, and if not, to let you type",
"  in a replacement.\n",
"SET DIAL CONNECT { AUTO, ON, OFF }",
"  Whether to CONNECT (enter terminal mode) automatically after successfully",
"  dialing.  ON means to do this; OFF means not to.  AUTO (the default) means",
"  do it if the DIAL command was given interactively, but don't do it if the",
"  DIAL command was issued from a macro or command file.  If you specify ON",
"  or AUTO, you may follow this by one of the keywords VERBOSE or QUIET, to",
"  indicate whether the verbose 4-line 'Connecting...' message is to be",
"  displayed if DIAL succeeds and Kermit goes into CONNECT mode.\n",
"SET DIAL CONVERT-DIRECTORY {ASK, ON, OFF}",
"  The format of Kermit's dialing directory changed in version 5A(192).  This",
"  command tells Kermit what to do when it encounters an old-style directory:",
"  ASK you whether to convert it, or convert it automatically (ON), or leave",
"  it alone (OFF).  Old-style directories can still be used without",
"  conversion, but the parity and speed fields are ignored.\n",
"SET DIAL DIRECTORY [ filename [ filename [ filename [ ... ] ] ] ]",
"  The name(s) of your dialing directory file(s).  If you do not supply any",
"  filenames, the  dialing directory feature is disabled and all numbers are",
"  dialed literally as given in the DIAL command.  If you supply more than",
"  one directory, all of them are searched.\n",
"SET DIAL SORT {ON, OFF}",
"  When multiple entries are obtained from your dialing directory, they are",
"  sorted in \"cheapest-first\" order.  If this does not produce the desired",
"  effect, SET DIAL SORT OFF to disable sorting, and the numbers will be",
"  dialed in the order in which they were found.\n",
"SET DIAL DISPLAY {ON, OFF}",
"  Whether to display dialing progress on the screen; default is OFF.\n",
"SET DIAL HANGUP {ON, OFF}",
"  Whether to hang up the phone prior to dialing; default is ON.\n",
"SET DIAL METHOD {DEFAULT, TONE, PULSE}",
"  Whether to use the modem's DEFAULT dialing method, or to force TONE or",
"  PULSE dialing.\n",
"SET DIAL TIMEOUT number",
"  How many seconds to wait for a dialed call to complete.  Use this command",
"  to override the DIAL command's automatic timeout calculation.  A value",
"  of 0 turns off this feature and returns to Kermit's automatic dial",
"  timeout calculation.\n",
"SET DIAL RESTRICT { INTERNATIONAL, LOCAL, LONG-DISTANCE, NONE }",
"  Prevents placing calls of the type indicated, or greater.  For example",
"  SET DIAL RESTRICT LONG prevents placing of long-distance and international",
"  calls.  If this command is not given, there are no restrictions.\n",
"SET DIAL RETRIES <number>",
"  How many times to redial each number if the dialing result is busy or no",
"  no answer, until the call is succesfully answered.  The default is 0",
"  because automatic redialing is illegal in some countries.\n",
"SET DIAL INTERVAL <number>",
"  How many seconds to pause between automatic redial attempts; default 10.\n",

"The following commands apply to all phone numbers, whether given literally",
"or found in the dialing directory:\n",
"SET DIAL PREFIX [ text ]",
"  Establish a prefix to be applied to all phone numbers that are dialed,",
"  for example to disable call waiting.\n",
"SET DIAL SUFFIX [ text ]",
"  Establish a suffix to be added after all phone numbers that are dialed.\n",
"The following commands apply only to portable-format numbers obtained from",
"the dialing directory; i.e. numbers that start with a \"+\" sign and",
"country code, followed by area code in parentheses, followed by the phone",
"number.\n",
"SET DIAL LD-PREFIX [ <text> ]",
"  Your long-distance dialing prefix, to be used with portable dialing",
"  directory entries that result in long-distance calls.\n",
"SET DIAL LD-SUFFIX [ <text> ]",
"  Long-distance dialing suffix, if any, to be used with portable dialing",
"  directory entries that result in long-distance calls.  This would normally",
"  be used for appending a calling-card number to the phone number.\n",
"SET DIAL TOLL-FREE-AREA-CODE [ <number> [ <number> [ ... ] ] ]",
"  Tells Kermit the toll-free area code(s) in your country.\n",
"SET DIAL TOLL-FREE-PREFIX [ <text> ]",
"  You toll-free dialing prefix, in case it is different from your long-",
"  distance dialing prefix.\n",
"SET DIAL INTL-PREFIX <text>",
"  Your international dialing prefix, to be used with portable dialing",
"  directory entries that result in international calls.\n",
"SET DIAL INTL-SUFFIX <text>",
"  International dialing suffix, if any, to be used with portable dialing",
"  directory entries that result in international calls.\n",
"SET DIAL PBX-OUTSIDE-PREFIX <text>",
"  Use this to tell Kermit how to get an outside line when dialing from a",
"  Private Branch Exchange (PBX).\n",
"SET DIAL PBX-EXCHANGE <text>",
"  If PBX-OUTSIDE-PREFIX is set, then you can use this command to tell Kermit",
"  the leading digits of a local phone number that identify it as being on",
"  your PBX, so it can make an internal call by deleting those digits from",
"  the phone number.\n",
"SET DIAL PBX-INTERNAL-PREFIX <text>",
"  If PBX-EXCHANGE is set, and Kermit determines from it that a call is",
"  internal, then this prefix, if any, is added to the number prior to",
"  dialing.  Use this if internal calls require a special prefix.",
"" };
#endif /* NODIAL */

static char *hmxyflo[] = { "Syntax: SET FLOW value\n",
#ifndef NOLOCAL
"Type of flow control to use during file transfer and CONNECT mode.",
#else
"Type of flow control to use during file transfer.",
#endif /* NOLOCAL */
"Choices: AUTOMATIC (the default, let Kermit choose for you); KEEP (don't",
"change the device's current setting), XON/XOFF (software flow control),",
"NONE (no flow control at all), and possibly others including RTS/CTS",
"(hardware) depending on the capabilities of your computer and operating",
"system.  Type SET FLOW ? for a list.",
""};

static char *hmxyf[] = { "Syntax: SET FILE parameter value",
"Parameters:\n",

"BYTESIZE { 7, 8 }: normally 8.  If 7, truncate the 8th bit of all file \
bytes.\n",

#ifndef NOCSETS
"CHARACTER-SET: tells the encoding of the local file, ASCII by default.",
"The names ITALIAN, PORTUGUESE, NORWEGIAN, etc, refer to 7-bit ISO-646",
"national character sets.  LATIN1 is the 8-bit ISO 8859-1 Latin Alphabet 1",
"for Western European languages.",
"NEXT is the 8-bit character set of the NeXT workstation.",
"The CPnnn sets are for IBM PCs.  MACINTOSH-LATIN is for the Macintosh.",
#ifndef NOLATIN2
"LATIN2 is ISO 8859-2 for Eastern European languages that are written with",
"Roman letters.",
#endif /* NOLATIN2 */
#ifdef CYRILLIC
"KOI-CYRILLIC, CYRILLIC-ISO, and CP866 are 8-bit Cyrillic character sets.",
"SHORT-KOI is a 7-bit ASCII coding for Cyrillic.",
#endif /* CYRILLIC */
#ifdef HEBREW
"HEBREW-ISO is ISO 8859-8 Latin/Cyrillic.  CP862 is the Hebrew PC code page.",
"HEBREW-7 is like ASCII with the lowercase letters replaced by Hebrew.",
#endif /* HEBREW */
#ifdef KANJI
"JAPANESE-EUC, JIS7-KANJI, DEC-KANJI, and SHIFT-JIS-KANJI are Japanese",
"Kanji character sets.",
#endif /* KANJI */
"Type SET FILE CHAR ? for a complete list of file character sets.\n",
#endif /* NOCSETS */

"COLLISION tells what to do when a file arrives that has the same name as",
"an existing file.  The options are:",
"  BACKUP (default) - Rename the old file to a new, unique name and store",
"    the incoming file under the name it was sent with.",
"  OVERWRITE - Overwrite (replace) the existing file.",
"  APPEND - Append the incoming file to the end of the existing file.",
"  DISCARD - Refuse and/or discard the incoming file.",
"  RENAME - Give the incoming file a unique name.",
"  UPDATE - Accept the incoming file only if it is newer than the existing",
"    file.",
"Example: SET FILE COLLISION UPDATE\n",

"SET FILE DISPLAY selects the format of the file transfer display for",
"local-mode file transfer.  The choices are:",
"  SERIAL (the default).  One dot is printed for every K bytes transferred.",
"    This format works on any kind of terminal, even a hardcopy.",
"  CRT.  Numbers are continuously updated on a single screen line.  This",
"    format can be used on any video display terminal.",
#ifdef CK_CURSES
"  FULLSCREEN.  A fully formatted 24x80 screen showing lots of information.",
"    This requires a video display terminal whose control sequences are",
"    understood by Kermit.",
#endif /* CK_CURSES */
"  NONE.  No file transfer display at all.\n",

"DOWNLOAD-DIRECTORY [ <directory-name> ]",
"Directory into which all received files should be placed.  By default,",
"received files go into your current directory.\n",

"INCOMPLETE - what to do with an incompletely received file: KEEP",
"(default), or DISCARD.\n",

#ifdef VMS
"LABEL { ACL, BACKUP-DATE, NAME, OWNER, PATH } { ON, OFF } - Tells which",
"items to include (ON) or exclude (OFF) in labeled file transfer.",
#else
#ifdef OS2
"LABEL { ARCHIVE, READ-ONLY, HIDDEN, SYSTEM, EXTENDED } { ON, OFF }",
"Tells which items to include (ON) or exclude (OFF) in labeled file transfer.",
#endif /* OS2 */
#endif /* VMS */

"NAMES are normally CONVERTED to 'common form' during transmission; LITERAL",
"means use filenames literally (useful between like systems).  Also see",
"SET SEND PATHNAMES and SET RECEIVE PATHNAMES.\n",

#ifdef VMS
"RECORD-LENGTH sets the record length for received files of type BINARY. Use",
"this to receive VMS BACKUP savesets or other fixed-format files. The default",
"of 512 is suitable for executable (.EXE) files, etc.\n",
"Example: SET FILE REC 8192\n",
#endif /* VMS */

"TYPE: How file contents are to be treated during file transfer.",
"TYPE is normally TEXT, with conversion of record format and character set.",
"BINARY means to do no conversion.  Use BINARY for executable programs or",
"binary data.  Example: SET FILE TYPE BINARY.\n",

#ifdef VMS
"For VMS, you may include an optional record-format after the word",
"BINARY.  This may be FIXED (the default) or UNDEFINED.",
"Two additional VMS file types are also supported: IMAGE and LABELED.  IMAGE",
"means raw block i/o, no interference from RMS, and applies to file transmis-",
"sion only.  LABELED means to send or interpret RMS attributes with the file.",
"\n",
#endif /* VMS */

"WARNING.  SET FILE WARNING is superseded by the newer command, SET FILE",
"COLLISION.  SET FILE WARNING ON is equivalent to SET FILE COLLISION RENAME",
"and SET FILE WARNING OFF is equivalent to SET FILE COLLISION OVERWRITE.\n",

"" };
 
static char *hmxyhsh[] = { "Syntax: SET HANDSHAKE value\n",
"Character to use for half duplex line turnaround handshake during file",
"transfer.  C-Kermit waits for this character from the other computer before",
"sending its next packet.  Default is NONE, others are XON, LF, BELL, ESC,",
"etc.  SET HANDSHAKE CODE <n> lets you specify the numeric ASCII value of the",
"handshake character.  Type SET HANDSH ? for a list.",
"" };

#ifndef NOSERVER
static char *hsetsrv[] = {"\
SET SERVER DISPLAY {ON,OFF}",
"Tell whether local-mode C-Kermit during server operation should put a",
"file transfer display on the screen.  Default is OFF.\n",
"SET SERVER GET-PATH [ directory [ directory [ ... ] ] ]",
"Tells the C-Kermit server where to search for files whose names it receives",
"from client GET commands when the names are not fully specified pathnames.",
"Default is no GET-PATH, so C-Kermit looks only in its current directory.\n",
"SET SERVER IDLE-TIMEOUT seconds",
"Idle time limit while in server mode, 0 for no limit.\n",
"SET SERVER LOGIN [ username [ password [ account ] ] ]",
"Sets up a username and optional password which must be supplied before",
"the server will respond to any commands other than REMOTE LOGIN.  The",
"account is ignored.  If you enter SET SERVER LOGIN by itself, then login",
"is no longer required.\n",
"SET SERVER TIMEOUT n",
"Server command wait timeout interval, how often the C-Kermit server issues",
"a NAK while waiting for a command packet.  Specify 0 for no NAKs at all.",
"Default is 0.",
"" };
#endif /* NOSERVER */

static char *hmhrmt[] = { "\
The REMOTE command is used to send file management instructions to a",
"remote Kermit server.  There should already be a Kermit running in server",
"mode on the other end of the currently selected line.  Type REMOTE ? to",
"see a list of available remote commands.  Type HELP REMOTE x to get",
"further information about a particular remote command 'x'.",
"" };

#ifndef NOSPL
static char *ifhlp[] = { "Syntax: IF [NOT] condition command\n",
"If the condition is (is not) true, do the command.  Only one command may",
"be given, and it must appear on the same line as the IF.  Conditions are:\n",
"  SUCCESS     - the previous command succeeded",
"  FAILURE     - the previous command failed",
"  BACKGROUND  - C-Kermit is running in the background",
#ifdef CK_IFRO
"  FOREGROUND  - C-Kermit is running in the foreground",
"  REMOTE-ONLY - C-Kermit was started with the -R command-line option\n",
#else
"  FOREGROUND  - C-Kermit is running in the foreground\n",
#endif /* CK_IFRO */
"  DEFINED variablename or macroname - The named variable or macro is defined",
#ifdef CK_TMPDIR
"  DIRECTORY string                  - The string is the name of a directory",
#endif /* CK_TMPDIR */
#ifdef ZFCDAT
"  NEWER file1 file2                 - The 1st file is newer than the 2nd one",
#endif /* ZFCDAT */
"  NUMERIC variable or constant      - The variable or constant is numeric",
"  EXIST filename                    - The named file exists\n",
"  COUNT   - subtract one from COUNT, execute the command if the result is",
"            greater than zero (see SET COUNT)\n",
"  EQUAL s1 s2 - s1 and s2 (character strings or variables) are equal",
"  LLT s1 s2   - s1 is lexically (alphabetically) less than s2",
"  LGT s1 s1   - s1 is lexically (alphabetically) greater than s2\n",
"  = n1 n2 - n1 and n2 (numbers or variables containing numbers) are equal",
"  < n1 n2 - n1 is arithmetically less than n2",
"  > n1 n2 - n1 is arithmetically greater than n2\n",
"The IF command may be followed on the next line by an ELSE command. Example:",
"  IF < \\%x 10 ECHO It's less",
"  ELSE echo It's not less\n",
"See also XIF.",
"" };

static char *hmxxeval[] = {"Syntax: EVALUATE expression\n",
"Evaluate an integer arithmetic expression and print its value.  The",
"expression can contain numbers and/or numeric-valued variables or functions.",
"Operators include +-/*(), etc.  Example: EVAL (1+1) * (\\%a / 3).",
"" };
#endif /* NOSPL */

#ifndef NOSPL
static char *ifxhlp[] = { "\
Syntax: XIF condition { commandlist } [ ELSE { commandlist } ]\n",
"Extended IF command.  The conditions are the same as for IF (type HELP IF)",
"but multiple comma-separated commands may be grouped within braces in both", 
"the IF and ELSE parts.  The ELSE part, if any, must be on the same line as",
"the XIF (or use dash for line continuation).  Example:\n",
"  XIF equal \\%a YES { echo OK, goto begin } ELSE { echo Not OK, stop }",
"" };

static char *forhlp[] = { "\
Syntax: FOR variablename initial-value final-value increment { commandlist }",
"\nFOR loop.  Execute the comma-separated commands in the commandlist the",
"number of times given by the initial value, final value and increment.",
"Example:  FOR \\%i 10 1 -1 { pause 1, echo \\%i }", "" };

static char *whihlp[] = { "\
Syntax: WHILE condition { commandlist }",
"\nWHILE loop.  Execute the comma-separated commands in the bracketed",
"commandlist while the condition is true.  Conditions are the same as for",
"IF commands.",
"" };

static char *swihlp[] = {
"Syntax: SWITCH <variable> { case-list }",
"The case-list is a series of lines like these:\n",
"  :x, command, command, ..., break\n",
"where \"x\" is a possible value for the variable.  At the end of the",
"case-list, you can put a \"default\" label to catch when the variable does",
"not match any of the labels:\n",
"  :default, command, command, ...",
""
};

static char *openhlp[] = {
"Syntax:  OPEN mode filename\n",
"For use with READ and WRITE commands.  Open the local file in the specified",
"mode: READ, WRITE, or APPEND.  !READ and !WRITE mean to read from or write",
"to a system command rather than a file.  Examples:\n",
"  OPEN READ oofa.txt",
"  OPEN !READ sort foo.bar",
"" };

static char *hxxaskq[] = {
"Syntax:  ASKQ variablename [ prompt ]",
"Example: ASKQ \\%p { Password:}\n",
"Issues the prompt and defines the variable to be whatever you type in.",
"The characters that you type do not echo on the screen.",
"Use braces to preserve leading and/or trailing spaces in the prompt.",
"To include a question mark, precede it by backslash (\\).",""
};

static char *hxxask[] = {
"Syntax:  ASK variablename [ prompt ]",
"Example: ASK \\%n { What is your name\\? }\n",
"Issues the prompt and defines the variable to be whatever you type in.",
"Use braces to preserve leading and/or trailing spaces in the prompt.",
"To include a question mark, precede it by backslash (\\).",""
};

static char *hxxgetc[] = {
"Syntax:  GETC variablename [ prompt ]",
"Example: GETC \\%c { Type any character to continue...}\n",
"Issues the prompt and sets the variable to the first character you type.",
"Use braces to preserve leading and/or trailing spaces in the prompt.", ""
};

static char *hxxdef[] = {
"Syntax: DEFINE name [ definition ]\n",
"Defines a macro or variable.  Its value is the definition, taken literally.",
"No expansion or evaluation of the definition is done.  Thus if the", 
"definition includes any variable or function references, their names are",
"included, rather than their values (compare with ASSIGN).  If the definition",
"is omitted, then the named variable or macro is undefined.\n",
"A typical macro definition looks like this:\n",
"  DEFINE name command, command, command, ...\n",
"for example:\n",
"  DEFINE vax set parity even, set duplex full, set flow xon/xoff\n",
"which defines a Kermit command macro called 'vax'.  The definition is a",
"comma-separated list of Kermit commands.  Use the DO command to execute",
"the macro, or just type its name, followed optionally by arguments.\n",
"The definition of a variable can be anything at all, for example:\n",
"  DEFINE \\%a Monday",
"  DEFINE \\%b 3\n",
"These variables can be used almost anywhere, for example:\n",
"  ECHO Today is \\%a",
"  SET BLOCK-CHECK \\%b",
"" };

static char *hxxass[] = {
"Syntax:  ASSIGN variablename string.",
"Example: ASSIGN \\%a My name is \\%b.\n",
"Assigns the current value of the string to the variable (or macro).",
"The definition string is fully evaluated before it is assigned, so that",
"the values of any variables are contained are used, rather than their",
"names.  Compare with DEFINE.  To illustrate the difference, try this:\n",
"  DEFINE \\%a hello",
"  DEFINE \\%x \\%a",
"  ASSIGN \\%y \\%a",
"  DEFINE \\%a goodbye",
"  ECHO \\%x \\%y\n",
"This will print 'goodbye hello'.", "" };

static char *hxxdec[] = {
"Syntax: DECREMENT variablename [ number ]\n",
"Decrement (subtract one from) the value of a variable if the current value",
"is numeric.  If the number argument is given, subtract that number instead.",
"\nExamples: DECR \\%a, DECR \\%a 7, DECR \\%a \\%n", "" };

static char *hxxinc[] = {
"Syntax: INCREMENT variablename [ number ]\n",
"Increment (add one to) the value of a variable if the current value is",
"numeric.  If the number argument is given, add that number instead.\n",
"Examples: INCR \\%a, INCR \\%a 7, INCR \\%a \\%n", "" };
#endif /* NOSPL */

#ifdef ANYX25
static char *hxxpad[] = {
"Syntax: PAD command",
"X.25 PAD commands:\n",
"    PAD CLEAR     - Clear the virtual call",
"    PAD STATUS    - Return the status of virtual call",
"    PAD RESET     - Send a reset packet",
"    PAD INTERRUPT - Send an interrupt packet",
""};

static char *hxyx25[] = {
"Syntax: SET X.25 option { ON [ data ], OFF }\n",
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

#ifdef OS2
static char *hxyprtr[] = {
"Syntax: SET PRINTER filename\n",
"Where to send transparent-print and screen-dump material during CONNECT.",
"Default is PRN.  You can also specify a disk file name, in which case the",
"given file is created if it does not exist, or is appended to if it already",
"exists.  Use SET PRINTER NUL to discard transparent print and screen-dump",
"material.  SHOW PRINTER displays the current setting.  Note: SET PRINTER",
"does not affect the PRINT command.",
""};
#endif /* OS2 */

static char *hxyexit[] = {
"Syntax: SET EXIT ON-DISCONNECT { ON, OFF }",
"  When ON, C-Kermit EXITs automatically when a network connection",
"  is terminated either by the host or by issuing a HANGUP command.",
"  This command is not currently supported on serial connections.\n",
"Syntax: SET EXIT STATUS number",
#ifdef NOSPL
"  Set C-Kermit's program return code to the given number.",
#else
"  Set C-Kermit's program return code to the given number, which can be a",
"  constant, variable, or an \\feval() expression.",
#endif /* NOSPL */
"\nSyntax: SET EXIT WARNING { ON, OFF, ALWAYS }", 
"  When EXIT WARNING is ON, issue a warning message and ask for confirmation",
"  before EXITing if a connection to another computer might still be open.",
"  When EXIT WARNING is ALWAYS, confirmation is always requested.  The",
"  default is ON.",
"" };

#ifndef NOSPL
static char *hxxpau[] = {
"Syntax:  PAUSE [ { number-of-seconds, hh:mm:ss } ]",
"Example: PAUSE 3  or  PAUSE 14:52:30\n",
"Do nothing for the specified number of seconds or until the given time of",
"day in 24-hour hh:mm:ss notation.  If the time of day is earlier than the",
"current time, it is assumed to be tomorrow.  If no argument given, one",
"second is used.  The pause can be interrupted by typing any character on the",
"keyboard.  If interrupted, PAUSE fails, otherwise it succeeds.",
"Synonym: SLEEP.",
"" };

static char *hxxmsl[] = {
"Syntax:  MSLEEP [ number ]",
"Example: MSLEEP 500\n",
"Do nothing for the specified number of milliseconds; if no number given,",
"100 milliseconds.","" };
#endif /* NOSPL */

#ifndef NOPUSH
extern int nopush;
static char *hxxshe[] = {
"Syntax: ! [ command ] or RUN [ command ] or PUSH [ command ]\n",
"Give a command to the local operating system's command processor, and",
"display the results on the screen.\n",
"If the command is omitted, enter interactive mode; return to Kermit",
"by exiting from the system's command parser.  The command is usually",
"EXIT or QUIT or LOGOUT.",  "" };
#endif /* NOPUSH */

#ifndef NOXMIT
static char *hxxxmit[] = {
"Syntax: TRANSMIT file\n",
"The TRANSMIT command is used for sending single files to other computers",
"that don't have Kermit.  Text files are sent a line at a time; binary files",
"are sent a character at a time.  There is no guarantee that the other",
"computer will receive the file correctly and completely.  Before you start",
"the TRANSMIT command, you must put the other computer in data collection",
"mode, for example by starting a text editor.  TRANSMIT may be interrupted by",
"Ctrl-C.  Synonym: XMIT.",
"" };
#endif /* NOXMIT */

#ifndef NOCSETS
static char *hxxxla[] = {
"Syntax: TRANSLATE file1 cs1 cs2 [ file2 ]\n",
"Translates file1 from the character set cs1 into the character set cs2",
"and stores the result in file2.  The character sets can be any of",
"C-Kermit's file character sets.  If file2 is omitted, the translation",
"is displayed on the screen.  An appropriate intermediate character-set",
"is chosen automatically, if necessary.",
"Synonym: XLATE.  Example:\n",
"TRANSLATE lasagna.lat latin1 italian lasagna.nrc",
"" };
#endif /* NOCSETS */

#ifndef NOSPL
static char *hxxwai[] = {
"Syntax:  WAIT { number-of-seconds, hh:mm:ss } [modem-signal(s)]",
"Example: WAIT 5 \\cd \\cts",
"Or:      WAIT 23:59:59 \\cd",
"Waits up to the given number of seconds or the given time of day for all of",
"the specified modem signals to appear on the serial communication device.",
"Sets FAILURE if signals do not appear in the given time or if interrupted by",
"typing anything at the keyboard during the waiting period.\n",
"Signals: \\cd = Carrier Detect, \\dsr = Dataset Ready, \\cts = Clear To Send",
"" };
#endif /* NOSPL */

static char *hxxwri[] = {
"Syntax: WRITE name text\n",
"Writes the given text to the named log or file.  The text text may include",
"backslash codes, and is not terminated by a newline unless you include the",
"appropriate code.  The name parameter can be any of the following:\n",
"  DEBUG-LOG",
"  ERROR (standard error)",
#ifndef NOSPL
"  FILE (the OPEN WRITE, OPEN !WRITE, or OPEN APPEND file, see HELP OPEN)",
#endif /* NOSPL */
"  PACKET-LOG",
"  SCREEN (compare with ECHO)",
#ifndef NOLOCAL
"  SESSION-LOG",
#endif /* NOLOCAL */
"  TRANSACTION-LOG", "" };

#ifndef NODIAL
static char *hxxlook[] = { "Syntax: LOOKUP name\n",
"Looks up the given name in the dialing directory or directories, if any,",
"specified in the most recent SET DIAL DIRECTORY command.  Each matching",
"entry is shown, along with any transformations that would be applied to",
"portable-format entries based on your locale.  HELP DIAL, HELP SET DIAL",
"for further info.",
""
};

static char *hxxansw[] = { "Syntax:  ANSWER [ <seconds> ]\n",
#ifdef OS2
"Waits for a modem call to come in.  Prior SET MODEM TYPE and SET PORT", 
#else
"Waits for a modem call to come in.  Prior SET MODEM TYPE and SET LINE", 
#endif /* OS2 */
"required.  If <seconds> is 0 or not specified, Kermit waits forever or until",
"interrupted, otherwise Kermit waits the given number of seconds.  The ANSWER",
"command puts the modem in autoanswer mode.  Subsequent DIAL commands will",
"automatically put it (back) in originate mode.  SHOW MODEM, HELP SET MODEM",
"for more info.",
""
};

static char *hxxdial[] = { "Syntax:  DIAL phonenumber",
"Example: DIAL 7654321\n",
"Dials a number using an autodial modem.  First you must SET MODEM, then",
#ifdef OS2
"SET PORT, then SET SPEED.  Then give the DIAL command, including the phone",
#else
"SET LINE, then SET SPEED.  Then give the DIAL command, including the phone",
#endif /* OS2 */
"number, for example:\n",
"  DIAL 7654321\n",
#ifdef NETCONN
"If the modem is on a network modem server, SET HOST first, then SET MODEM,",
"then DIAL.  See also SET DIAL, SET MODEM, SET LINE, SET HOST, SET SPEED,",
"and REDIAL.\n",
#else
"See also SET DIAL, SET MODEM, SET LINE, SET SPEED, REDIAL.\n",
#endif /* NETCONN */
"If the phonenumber starts with a letter, and if you have used the SET DIAL",
"DIRECTORY command to specify one or more dialing-directory files, Kermit",
"looks it up in the given file(s); if it is found, the name is replaced by",
"the number or numbers associated with the name.  If it is not found, the",
"name is sent to the modem literally.\n",
"If the phonenumber starts with an equals sign (\"=\"), this forces the part",
"after the = to be send literally to the modem, even if it starts with a",
"letter, without any directory lookup.\n",
"A dialing directory is a plain text file, one entry per line:\n",
"  name  phonenumber  ;  comments\n",
"for example:\n",
"  work    9876543              ; This is a comment",
"  e-mail  +1  (212) 555 4321   ; My electronic mailbox",
"  heise   +49 (511) 535 2301   ; Verlag Heinz Heise BBS\n",
"If a phone number starts with +, then it must include country code and",
"area code, and C-Kermit will try to handle these appropriately based on",
"the current locale (HELP SET DIAL for further info); these are called",
"PORTABLE entries.  If it does not start with +, it is dialed literally.\n",
"If more than one entry is found with the same name, Kermit dials all of",
"them until the call is completed; if the entries are in portable format,",
"Kermit dials then in cheap-to-expensive order: internal, then local, then",
"long-distance, then international, based on its knowledge of your local",
"country code and area code (see HELP SET DIAL).\n",
"Specify your dialing directory file(s) with the SET DIAL DIRECTORY command.",
 "" };
#endif /* NODIAL */

#endif /* NOHELP */

/*  D O H L P  --  Give a help message  */
 
_PROTOTYP( int dohset, (int) );

int
dohlp(xx) int xx; {
    int x,y;
 
    debug(F101,"DOHELP xx","",xx);
    if (xx < 0) return(xx);

    switch (xx) {
 
#ifdef NOHELP

case XXHLP:
    if ((x = cmcfm()) < 0)
      return(x);
    printf("\n%s, Copyright (C) 1985, 1996,",versio);
    return(hmsga(tophlp));

#else /* help is available */

#ifndef NOSPL
case XXASS:				/* assign */
    return(hmsga(hxxass));

case XXASK:				/* ask */
    return(hmsga(hxxask));

case XXASKQ:
    return(hmsga(hxxaskq));

case XXAPC:
    return(hmsg("Syntax: APC text\n\
Echoes the text in the form of a VT220/320/420 Application Program Command.\n\
Use the APC command to send commands to MS-DOS Kermit 3.13 or later."));
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

case XXBYE:				/* BYE */
    return(hmsg(hmxxbye));
 
case XXCHK:				/* check */
    return(hmsg("\
Syntax: CHECK name\n\
Checks to see if the named feature is included in this version of C-Kermit.\n\
To list the features you can check, type \"check ?\"."));

#ifndef NOFRILLS
case XXCLE:				/* clear */
#ifdef OS2
    return(hmsg("\
Syntax: CLEAR [ { COMMAND-SCREEN, DEVICE, DEVICE-AND-INPUT, INPUT,\n\
                  SCROLLBACK, SEND-LIST, TERMINAL-SCREEN } ]\n\
COMMAND-SCREEN clears the current command screen\n\
DEVICE clears the current port or network input buffer\n\
DEVICE-AND-INPUT, the default, clears both the device and the INPUT buffer\n\
INPUT clears the INPUT command buffer\n\
SCROLLBACK empties the scrollback buffer including the current screen\n\
SEND-LIST clears the current SEND list (see ADD)\n\
TERMINAL-SCREEN clears the current screen a places it into the scrollback\n\
    buffer."));
#else /* not OS2 */    
    return(hmsg("\
Syntax: CLEAR [ { DEVICE, INPUT, DEVICE-AND-INPUT, DIAL-STATUS, SEND-LIST }\
 ]\n\
Clears the communications device input buffer, the INPUT command buffer,\n\
or both.  CLEAR SEND-LIST clears the SEND list (see ADD).  CLEAR DIAL-STATUS\n\
clears the \\v(dialstatus) variable.  The default is DEVICE-AND-INPUT."));
#endif /* OS2 */
#endif /* NOFRILLS */

case XXCLO:				/* close */
    return(hmsga(hmxxclo));
 
case XXCOM:				/* comment */
#ifndef STRATUS /* Can't use # for comments in Stratus VOS */
    return(hmsg("\
Syntax: COMMENT text\n\
Example: COMMENT - this is a comment.\n\n\
Introduces a comment.  Beginning of command line only.  Commands may also\n\
have trailing comments, introduced by ; or #."));
#else
    return(hmsg("\
Syntax: COMMENT text\n\
Example: COMMENT - this is a comment.\n\n\
Introduces a comment.  Beginning of command line only.  Commands may also\n\
have trailing comments, introduced by ; (semicolon)."));
#endif /* STRATUS */

#ifndef NOLOCAL
case XXCON:				/* connect */
    hmsga(hmxxcon);
    printf("Your escape character is Ctrl-%c (ASCII %d, %s)\r\n",
	   ctl(escape), escape, (escape == 127 ? "DEL" : ccntab[escape]));
    return(0);
#endif /* NOLOCAL */
 
#ifndef NOFRILLS
case XXCPY:
    return(hmsg("Syntax: COPY source destination\n\n\
Copy the file 'source' to 'destination'."));
#endif /* NOFRILLS */

case XXCWD:				/* cd / cwd */
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
specified by HOME environment variable, if any."));
#else
    return(hmsg("Syntax: CD [ directoryname ]\n\
Change Directory.  \n\
If directory name omitted, changes to your home directory."));
#endif /* OS2 */
#endif /* datageneral */
#endif /* vms */
 
#ifndef NOSPL
case XXDCL:
    return(hmsg("Syntax:  DECLARE arrayname[size]\n\
Example: DECLARE \\&a[20]\n\n\
Declares an array of the given size.  Array elements can be used just like\n\
any other variables."));

case XXDEF:				/* DEFINE */
    return(hmsga(hxxdef));

case XXUNDEF:				/* UNDEFINE */
    return(hmsg("Syntax:  UNDEFINE variable-name\n\
Undefines a macro or variable."));
#endif /* NOSPL */

#ifndef NOFRILLS
case XXDEL:				/* delete */
    return(hmsg("Syntax: DELETE filespec\n\n\
Delete a local file or files.  RM is a synonym for DELETE."));
#endif /* NOFRILLS */
 
#ifndef NODIAL
case XXDIAL:				/* DIAL, etc... */
    return(hmsga(hxxdial));

case XXPDIA:				/* PDIAL */
    return(hmsg("Syntax: PDIAL phonenumber\n\n\
Partially dial a phone number.  Like DIAL but does not wait for carrier\n\
or CONNECT message."));

case XXRED:
    return(hmsg("Redial the number given in the most recent DIAL commnd."));

case XXANSW:				/* ANSWER */
    return(hmsga(hxxansw));

case XXLOOK:				/* LOOKUP number in directory */
    return(hmsga(hxxlook));
#endif /* NODIAL */
 
case XXDIR:				/* directory */
    return(hmsg("Syntax: DIRECTORY [ filespec ]\n\
Display a directory listing of local files."));
 
#ifndef NOSERVER
#ifndef NOFRILLS
case XXDIS:
    return(hmsg("Syntax: DISABLE command\n\n\
Security for the C-Kermit server.  Prevent the client Kermit program from\n\
executing the named REMOTE command, such as CD, DELETE, RECEIVE, etc."));
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
case XXDO:				/* do */
    return(hmsg("Syntax: [ DO ] macroname [ arguments ]\n\n\
Execute a macro that was defined by the DEFINE command.  The word DO\n\
can be omitted.  Trailing argument words, if any, are automatically\n\
assigned to the macro argument variables \\%1, \\%2, etc."));
#endif /* NOSPL */

#ifndef NOSPL
case XXDEC:
    return(hmsga(hxxdec));
#endif /* NOSPL */

case XXECH:				/* echo */
    return(hmsg("Syntax: ECHO text\n\
Display the text on the screen, followed by a line terminator.  The ECHO\n\
text may contain backslash codes.  Example: ECHO \\7Wake up!\\7")); 
 
case XXXECH:				/* xecho */
    return(hmsg("Syntax: XECHO text\n\
Just like ECHO but does not add a line terminator to the text.  See ECHO."));

#ifndef NOSERVER
#ifndef NOFRILLS
case XXENA:
    return(hmsg("Syntax: ENABLE capability\n\n\
For use with server mode.  Allow the client Kermit program access to the\n\
named capability, such as CD, DELETE, RECEIVE, etc.  Opposite of DISABLE."));
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
case XXEND:				/* end */
    return(hmsg("Syntax: END [ number [ message ] ]\n\
Exit from the current macro or TAKE file, back to wherever invoked from.\n\
Number is return code.  Message, if given, is printed."));

case XXEVAL:				/* evaluate */
    return(hmsga(hmxxeval));
#endif /* NOSPL */

#ifndef NOFRILLS
case XXERR:				/* e-packet */
    return(hmsg("Syntax: E-PACKET\n\
Send an Error packet to the other Kermit."));
#endif /* NOFRILLS */

case XXEXI:				/* exit */
case XXQUI:
    return(hmsg("Syntax: QUIT (or EXIT) [ number ]\n\
Exit from the Kermit program, closing all open files and devices, optionally\n\
setting the program's return code to the given number."));
 
case XXFIN:
    return(hmsg("Syntax: FINISH\n\
Tell the remote Kermit server to shut down without logging out."));
 
#ifndef NOSPL
case XXFOR:
    return(hmsga(forhlp));
#endif /* NOSPL */

case XXGET:
    return(hmsg(hmxxget));
 
#ifndef NOSPL
#ifndef NOFRILLS
  case XXGOK:
    return(hmsg("Syntax: GETOK prompt\n\
Print the prompt, make user type 'yes', 'no', or 'ok', and set SUCCESS or\n\
FAILURE accordingly."));
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
    printf("\n%s, Copyright (C) 1985, 1996,",versio);
    return(hmsga(tophlp));

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
    return(hmsg("Syntax: REINPUT n string\n\n\
Look for the string in the text that has recently been INPUT, set SUCCESS\n\
or FAILURE accordingly.  Timeout, n, must be specified but is ignored."));
#endif /* NOSPL */

#ifndef NOFRILLS
case XXREN:
    return(hmsg("Syntax: RENAME oldname newname\n\n\
Change the name of file 'oldname' to 'newname'."));
#endif /* NOFRILLS */

#ifndef NOSPL
case XXLBL:
    return(hmsg("\
Introduce a label, like :loop, for use with GOTO in TAKE files or macros.\n\
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
    return(hmsg("Syntax: MAIL filename address\n\n\
Send the file to the remote Kermit, which must be in RECEIVE or SERVER mode,\n\
and request that the remote host deliver the file as electronic mail to the\n\
given address.  Example: MAIL BUG.TXT KERMIT@CUVMA"));
#endif /* NOFRILLS */

#ifndef NOMSEND
case XXMSE:
    return(hmsg(hmssmse));

case XXADD:
    return(hmsg(hmssadd));

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
" Print news of new features since publication of \"Using C-Kermit\"."));

#ifndef NOSPL
case XXOUT:
    return(hmsg("Syntax: OUTPUT text\n\n\
Send the text out the currently selected line, as if you had typed it\n\
during CONNECT mode.  The text may contain backslash codes.  Example:\n\n\
  OUTPUT help\\13"));
#endif /* NOSPL */

#ifdef ANYX25
case XXPAD:
    return(hmsga(hxxpad));
#endif /* ANYX25 */

#ifndef NOSPL
case XXPAU:
    return(hmsga(hxxpau));

case XXMSL:
    return(hmsga(hxxmsl));
#endif /* NOSPL */

#ifdef TCPSOCKET
case XXPNG:
    return(hmsg("Syntax: PING [ IP-hostname-or-number ]\n\n\
Check if given IP network host is reachable.  Default host is from most\n\
recent SET HOST or TELNET command.  Runs system PING program, if any."));

case XXFTP:
    return(hmsg("Syntax: FTP [ IP-hostname-or-number ]\n\n\
Make an FTP connection to the given IP host or, if no host specified, to the\n\
current host.  Uses the system's FTP program, if any.\n"));
#endif /* TCPSOCKET */

#ifndef NOFRILLS
case XXPRI:
    return(hmsg("Syntax: PRINT file [ options ]\n\n\
Print the local file on a local printer with the given options."));
#endif /* NOFRILLS */

case XXPWD:
    return(hmsg("Syntax: PWD\n\
Print the name of the current working directory."));

#ifndef NOSPL
case XXREA:
    return(hmsg("Syntax: READ variablename\n\
Read a line from the currently open READ or !READ file into the variable\n\
(see OPEN)."));
#endif /* NOSPL */

case XXREC:
    return(hmsga(hmxxrc));
 
case XXREM:
    y = cmkey(remcmd,nrmt,"Remote command","",xxstring);
    return(dohrmt(y));
 
#ifndef NOSPL
case XXRET:
    return(hmsg("Syntax: RETURN [ value ]\n\
Return from a macro.  An optional return value can be given for use with\n\
with \\fexecute(macro), which allows macros to be used like functions."));
#endif /* NOSPL */
case XXSEN:
    return(hmsg(hmxxsen));
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
 
#ifndef NOJC
case XXSUS:
    return(hmsg("Syntax: SUSPEND or Z\n\
Suspend Kermit.  Continue Kermit with the appropriate system command,\n\
such as fg."));
#endif /* NOJC */

case XXSET:
    y = cmkey(prmtab,nprm,"Parameter","",xxstring);
    debug(F101,"HELP SET y","",y);
    return(dohset(y));
 
#ifndef NOPUSH
case XXSHE:
    if ( nopush ) {
        if ((x = cmcfm()) < 0) return(x);
        printf("Sorry, help not available for \"%s\"\n",cmdbuf);
        break;
    }
    else return(hmsga(hxxshe));
#ifdef CK_REDIR
case XXFUN:
    return(hmsg("Syntax: REDIRECT command\n\
Run the given local command with its standard input and output redirected\n\
to the current SET LINE or SET HOST communications path.\n\
Synonym: < (Left angle bracket)."));
#endif /* CK_REDIR */

#ifdef CK_REXX
case XXREXX:
    return(hmsg("Syntax: REXX text\n\
The text is a Rexx command to be executed. The \\v(rexx) variable is set to\n\
the Rexx command's return value.\n\
To execute a rexx program file, use:  REXX call <filename>\n\
Rexx programs may call C-Kermit functions by placing the C-Kermit command in\n\
single quotes.  For instance:  'set parity none'."));
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
    return(hmsg("Syntax: STATISTICS\n\
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
case XXTEL:
    return(hmsg("Syntax: TELNET [ host [ service ] ]\n\
Equivalent to SET NETWORK TCP/IP, SET HOST host [ service ], followed by\n\
CONNECT.  If host is omitted, previous connection (if any) is resumed."));
#ifdef RLOGCODE
case XXRLOG:
    return(hmsg("Syntax: RLOGIN [ host [ username ] ]\n\
Makes an interactive connection to the host using RLOGIN protocol.\n\
If host is omitted, previous connection (if any) is resumed."));
#endif /* RLOGCODE */
#endif /* TCPSOCKET */

#ifndef NOXMIT
case XXTRA:
    return(hmsga(hxxxmit));
#endif /* NOXMIT */

#ifndef NOFRILLS
case XXTYP:
    return(hmsg("Syntax: TYPE file\n\
Display a file on the screen.  Pauses if you type Ctrl-S, resumes if you\n\
type Ctrl-Q, returns immediately to C-Kermit prompt if you type Ctrl-C."
));
#endif /* NOFRILLS */

#ifdef OS2ONLY
case XXUPD:
    return(hmsga(hmxxupd));
#endif /* OS2ONLY */

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

case XXGETC:				/* GETC */
    return(hmsga(hxxgetc));

case XXFWD:				/* FORWARD */
    return(hmsg(
"Like GOTO, but searches only forward for the label.  See GOTO."));

case XXLOCAL:				/* LOCAL */
    return(hmsg(
"Declares a variable to be local to the current macro or command file."));
#endif /* NOSPL */

case XXVIEW:
    return(hmsg(
"View the terminal emulation screen even when there is no connection.\n"));

case XXASC:
    return(hmsg("Synonym for SET FILE TYPE TEXT.\n"));

case XXBIN:
    return(hmsg("Synonym for SET FILE TYPE BINARY.\n"));

case XXDATE:
    return(hmsg("Prints the current date and time.\n"));

case XXRETR:
    return(hmsg(
"Just like GET but asks the server to delete each file that has been\n\
sent successfully.\n"));

case XXEIGHT:
    return(hmsg(
"Equivalent to SET PARITY NONE, SET COMMAND BYTE 8, SET TERMINAL BYTE 8.\n"));

case XXSAVE:
    return(hmsg("Syntax: SAVE KEYMAP <file>\n\
Saves current keymap definitions to file, \"keymap.ini\" by default.")); 
#endif /* NOHELP */

default:
    if ((x = cmcfm()) < 0) return(x);
    printf("Sorry, help not available for \"%s\"\n",cmdbuf);
    break;
    }
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

int					/* Print an array of lines, */
hmsga(s) char *s[]; {			/* cheap version. */
    int i;
    if ((i = cmcfm()) < 0) return(i);
    printf("\n");			/* Start off with a blank line */
    for (i = 0; *s[i]; i++) {		/* Print each line. */
	printf("%s\n",s[i]);
    }
    printf("\n");
    return(0);
}

#else /* NOHELP not defined... */

int					/* Print an array of lines, */
hmsga(s) char *s[]; {			/* pausing at end of each screen. */
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

    printf("\n");			/* Start off with a blank line */
    n = 1;				/* Line counter */
    for (i = 0; *s[i]; i++) {
	printf("%s\n",s[i]);		/* Print a line. */
        y = (int)strlen(s[i]);
        k = 1;
        for (j = 0; j < y; j++)		/* See how many newlines were */
          if (s[i][j] == '\n') k++;	/* in the string... */
        n += k;
	if (n > (cmd_rows - 3) && *s[i+1]) /* After a screenful, give them */
          if (!askmore()) return(0);	/* a "more?" prompt. */
          else n = 0;
    }
    printf("\n");
    return(0);
}
 
#ifndef NOXMIT
static char *hsetxmit[] = {
"Syntax: SET TRANSMIT parameter value\n",
"Controls the behavior of the TRANSMIT command, used for uploading files",
"to computers that don't have Kermit programs.  Parameters are:\n",
"ECHO ON/OFF:     Whether to echo text as it is being transmitted.",
"EOF text:        Text to send after end of file is reached.",
"FILL number:     ASCII value of character to insert into blank lines.",
"LINEFEED ON/OFF: Transmit LF as well as CR at the end of each line.",
"                 Normally, only CR is sent.",
"LOCKING-SHIFT ON/OFF: Whether to use SO/SI for transmitting 8-bit data",
"                 when PARITY is not NONE.",
"PAUSE number:    How many milliseconds to pause after transmitting each line",
"                 (text mode), or each character (binary mode).",
"PROMPT number:   ASCII value of character to look for from host before",
"                 sending next line, normally LF (10).",
"Synonym: SET XMIT.",
"" };
#endif /* NOXMIT */

static char *hsetbkg[] = {
"Syntax: SET BACKGROUND { OFF, ON }\n",
"SET BACKGROUND OFF forces prompts and messages to appear on your screen",
"even though Kermit thinks it is running in the background.", "" };

#ifdef DYNAMIC
static char *hsetbuf[] = {
"Syntax: SET BUFFERS n1 [ n2 ]\n",
"Change the overall amount of memory allocated for SEND and RECEIVE packet",
"buffers, respectively.  Bigger numbers let you have longer packets and more",
"window slots.  If n2 is omitted, the same value as n1 is used.",
"" };
#endif /* DYNAMIC */

static char *hsetcmd[] = {
"Syntax: SET COMMAND parameter value\n",
"SET COMMAND BYTESIZE { 7, 8 }",
"  Informs C-Kermit of the bytesize of the communication path between itself",
"  and your keyboard and screen.  7 is assumed.  SET COMMAND BYTE 8 to allow",
"  entry and display of 8-bit characters.\n",
#ifdef OS2
"SET COMMAND COLOR <foreground-color> <background-color>",
"  Choose colors for Command screen.  Use ? to get lists of available \
colors.\n",
#endif /* OS2 */

"SET COMMAND HEIGHT <number>",
"  Informs C-Kermit of the number of rows in your command screen for the",
"  purposes of More?-prompting.\n",

"SET COMMAND WIDTH <number>",
"  Informs C-Kermit of the number of characters across your screen for",
"  purposes of screen formatting.\n",

"SET COMMAND MORE-PROMPTING { ON, OFF }",
"  ON (the default) enables More?-prompting when C-Kermit needs to display",
"  text that does not fit vertically on your screen.  OFF allows the text to",
"  scroll by without intervention.  If your command window has scroll bars,",
"  you might prefer OFF.\n",

#ifdef CK_RECALL
"SET COMMAND RECALL-BUFFER-SIZE number",
"  How big you want C-Kermit's command recall buffer to be.  By default, it",
"  holds 10 commands.  You can make it any size you like, subject to memory",
"  constraints of the computer.  A size of 0 disables command recall.",
"  Whenever you give this command, previous command history is lost.",
#endif /* CK_RECALL */
"\nSET COMMAND QUOTING { ON, OFF }",
"  Whether to treat backslash and question mark as special characters (ON),",
"  or as ordinary data characters (OFF) in commands.  ON by default.",
#ifdef CK_RECALL
"\nSET COMMAND RETRY { ON, OFF }",
"  Whether to reprompt you with the correct but incomplete portion of a",
"  syntactically incorrect command.  ON by default.",
#endif /* CK_RECALL */
"\nUse SHOW COMMAND to display these settings.",
"" };

#ifndef NOLOCAL
static char *hsetcar[] = {
"Syntax: SET CARRIER ON, AUTO, or OFF\n",
"Attempts to control treatment of carrier on the communication device.",
"ON means that carrier is required at all times except during the DIAL",
"command.  OFF means that carrier is never required.  AUTO (the default)",
"means that carrier is required only during CONNECT.", "" };
#endif /* NOLOCAL */

static char *hsetat[] = {
"Syntax: SET ATTRIBUTES name ON or OFF\n",
"Use this command to enable (ON) or disable (OFF) the transmission of",
"selected file attributes along with each file, and to handle or ignore",
"selected incoming file attributes, including:\n",
#ifndef NOCSETS
"  CHARACTER-SET:  The transfer character set for text files",
#endif /* NOCSETS */
"  DATE:           The file's creation date",
"  DISPOSITION:    Unusual things to do with the file, like MAIL or PRINT",
"  LENGTH:         The file's length",
"  SYSTEM-ID:      Machine/Operating system of origin",
"  TYPE:           The file's type (text or binary)\n",
"You can also specify ALL to select all of them.  Examples:\n",
"  SET ATTR DATE OFF\n  SET ATTR LENGTH ON\n  SET ATTR ALL OFF", ""
};

#ifndef NOSPL
static char *hxyinp[] = {
"Syntax: SET INPUT parameter value\n",
"The SET INPUT command controls the behavior of the INPUT command:\n",
"SET INPUT BUFFER-LENGTH number-of-bytes",
"Removes the old INPUT buffer and creates a new one with the given length.\n",
"SET INPUT CASE { IGNORE, OBSERVE }",
"Tells whether alphabetic case is to be significant in string comparisons.",
"This setting is local to the current macro or command file, and is inherited",
"by subordinate macros and take files.\n",
"SET INPUT ECHO { ON, OFF }",
"Tells whether to display arriving characters read by INPUT on the screen.\n",
"SET INPUT SILENCE <number>",
"The maximum number to seconds of silence (no input at all) before the INPUT",
"command times out, 0 for no maximum.\n",
"SET INPUT TIMEOUT-ACTION { PROCEED, QUIT }",
"Tells whether to proceed or quit from a script program if an INPUT command",
"fails.  PROCEED (default) allows use of IF SUCCESS and IF FAILURE commands.",
"This setting is local to the current macro or command file, and is inherited",
"by subordinate macros and take files.",
"" };
#endif /* NOSPL */

static char *hxytak[] = {
"Syntax: SET TAKE parameter value\n",
"Controls behavior of TAKE command.\n",
"SET TAKE ECHO { ON, OFF } tells whether commands read from a TAKE file",
"should be displayed on the screen.\n",
"SET TAKE ERROR { ON, OFF } tells whether a TAKE command file should be",
"automatically terminated when a command fails.  This setting is local to",
"the current command file, and inherited by subordinate command files.",
 "" };

#ifndef NOLOCAL
#ifdef OS2MOUSE
static char *hxymouse[] = {
"Syntax: SET MOUSE ACTIVATE { ON, OFF }",
"Enables or disables the mouse in Connect mode.  Default is ON\n",
"Syntax: SET MOUSE BUTTON <number> <key-modifier> <action> [ <text> ]",
"where:",
"<number> is the mouse button number, 1, 2, or 3;",
"<key-modifier> denotes modifier keys held down during the mouse event:\n",
"  ALT, ALT-SHIFT, CTRL, CTRL-ALT CTRL-ALT-SHIFT, CTRL-SHIFT, SHIFT, or NONE;",
"\nand <action> is the mouse action, CLICK, DRAG, or DOUBLE-CLICK\n.",
"The <text> has exactly the same properties as the <text> from the SET KEY",
"command -- it can be a character, a string, one or more Kverbs, a macro",
"invoked as a Kverb, or any combination of these.  Thus, anything that can be",
"assigned to a key can also be assigned to the mouse -- and vice versa.  If",
"the <text> is omitted, the action will be ignored.  Examples:\n",
"  SET MOUSE BUTTON 1 NONE DOUBLE \\KmouseCurPos",
"  SET MOU B 2 SHIFT CLICK help\13\n",
"DRAG operations perform a \"mark mode\" selection of Text. You should assign",
"only the following actions to drag operations:",
"  \\Kdump         - copy marked text to printer (or file)",
"  \\Kmarkcopyclip - copy marked text to PM Clipboard",
"  \\Kmarkcopyhost - copy marked text direct to Host",
"  \\Kmousemark    - mark text, no copy operation performed\n",
"The following Kverb is only for use with the mouse:\n",
"  \\KmouseCurPos",
"which represents the mouse-directed terminal cursor feature.\n",
"Syntax: SET MOUSE CLEAR",
"Restores all mouse events to their default definitions",
"  Button 1 Double-Click = Kverb: \\Kmousecurpos",
"  Button 1 Drag         = Kverb: \\Kmarkcopyclip",
"  Button 1 Ctrl Drag    = Kverb: \\Kmarkcopyhost",
"  Button 2 Double-Click = Kverb: \\Kpaste",
" ",
"Type UPDATES MOUSE at the C-Kermit> prompt for details.",
""};
#endif /* OS2MOUSE */

static char *hxyterm[] = {
"Syntax: SET TERMINAL parameter value\n",
#ifdef OS2
"SET TERMINAL TYPE { ANSI, VT52, VT100, VT102, VT220, VT320, ... }",
"to select emulation.  Type SET TERMINAL TYPE ? to see a complete list.\n",
"SET TERMINAL ANSWERBACK { OFF, ON }",
"disables/enables the ENQ/Answerback sequence (\"K-95 version term-type\").\n",
"SET TERMINAL ANSWERBACK MESSAGE <extension>",
"allows you to specify an extension to the default answerback message.\n",
#else
"SET TERMINAL TYPE ...",
"This command is not available because this version of C-Kermit does not",
"include a terminal emulator.  Instead, it is a \"semitransparent pipe\"",
"(or a totally transparent one, if you configure it that way) to the computer",
"or service you have made a connection to.  Your console, workstation window,",
"or the terminal emulator or terminal from which you are running C-Kermit",
"provides the emulation.\n",
#endif /* OS2 */
#ifdef CK_APC
"SET TERMINAL APC { ON, OFF, UNCHECKED }",
#ifdef OS2
"controls execution of Application Program Commands sent by the host while",
"K-95 is either in CONNECT mode or processing INPUT commands.  ON allows",
"execution of \"safe\" commands and disallows potentially dangerous commands",
"such as DELETE, RENAME, OUTPUT, and RUN.  OFF prevents execution of APCs.",
"UNCHECKED allows execution of all APCs.  OFF is the default.\n",
#else /* OS2 */
"controls execution of Application Program Commands sent by the host while",
"C-Kermit is in CONNECT mode.  ON allows execution of \"safe\" commands and",
"disallows potentially dangerous commands such as DELETE, RENAME, OUTPUT, and",
"RUN.  OFF prevents execution of APCs.  UNCHECKED allows execution of all",
"APCs.  OFF is the default.\n",
#endif /* OS2 */
#endif /* CK_APC */
#ifdef OS2
"SET TERMINAL ARROW-KEYS { APPLICATION, CURSOR }",
"sets the mode for the arrow keys during VT terminal emulation.\n", 
"SET TERMINAL ATTRIBUTE { BLINK, REVERSE, UNDERLINE }",
"determines whether a true attribute is used, ON, or whether a color",
"simulation (as specified by SET TERMINAL COLOR) is used instead.\n",

"SET TERMINAL ATTRIBUTE { BLINK, REVERSE, UNDERLINE } { ON, OFF }",
"Determines whether real Blinking, Reverse, and Underline are used in the",
"terminal display.  When BLINK is turned OFF, reverse background intensity is",
"used.  When REVERSE and UNDERLINE are OFF, the colors selected with SET",
"TERMINAL COLOR { REVERSE,UNDERLINE } are used instead.  This command affects",
"the entire current screen and the entire terminal scrollback buffer.\n",
#endif /* OS2 */

"SET TERMINAL AUTODOWNLOAD { ON, OFF }",
#ifdef OS2
"enables/disables automatic switching into file-transfer mode when a valid",
#ifdef CK_XYZ
"Kermit or ZMODEM packet of the appropriate type is received during CONNECT",
"mode or while an INPUT command is active.  Default is OFF.\n",
#else
"Kermit packet of the appropriate type is received during CONNECT mode or",
"while an INPUT command is active.  Default is OFF.\n",
#endif /* CK_XYZ */
#else /* OS2 */
"enables/disables automatic switching into file-transfer mode when a valid",
#ifdef CK_XYZ
"Kermit or ZMODEM packet of the appropriate type is received during CONNECT",
"mode.  Default is OFF.\n",
#else
"Kermit packet of the appropriate type is received during CONNECT mode.",
"Default is OFF.\n",
#endif /* CK_XYZ */
#endif /* OS2 */

#ifdef OS2
"SET TERMINAL BELL { AUDIBLE, VISIBLE, NONE }",
"specifies how Control-G (bell) characters are handled.  AUDIBLE means",
"a beep is sounded; VISIBLE means the screen is flashed momentarily.\n",
"This command has been superseded by SET BELL.\n",
#endif /* OS2 */

"SET TERMINAL BYTESIZE 7 or 8, to use 7- or 8-bit terminal characters",
"between C-Kermit and the remote computer or service during CONNECT.\n",

#ifndef NOCSETS
#ifdef OS2
"SET TERMINAL CHARACTER-SET",
    "  see SET TERMINAL { LOCAL-CHARACTER-SET, REMOTE-CHARACTER-SET }\n",
#else  /* not OS2 */
"SET TERMINAL CHARACTER-SET <remote-cs> [ <local-cs> ]",
"to specify the character set used by the remote host, <remote-cs>, and the",
"character set used by C-Kermit locally, <local-cs>.  If you don't specify",
"the local character set, the current FILE CHARACTER-SET is used.  When",
"you specify two different character sets, C-Kermit translates between them",
"during CONNECT.  By default, both character sets are TRANSPARENT, and",
"no translation is done.\n",
#endif /* OS2 */
#endif /* NOCSETS */

#ifdef OS2
"SET TERMINAL CODE-PAGE <number> lets you change the PC code page.  Only",
"works for code pages that are successfully prepared in CONFIG.SYS.  Use",
"SHOW TERMINAL to display the current code page and the available code pages.",
#ifdef OS2ONLY
"See also SET TERMINAL FONT if the desired code page in not available in",
"your version of OS/2.\n",
#endif /* OS2ONLY */
#endif /* OS2 */

#ifdef OS2
#ifndef NT
"SET TERMINAL COLOR BORDER <foreground>",
#endif /* NT */
"SET TERMINAL COLOR <screenpart> <foreground> <background>, to set",
"the colors of the terminal emulation screen.  <screenpart> may be one of:",
"DEBUG, HELP-TEXT, REVERSE, SELECTION, STATUS-LINE, TERMINAL-SCREEN, or",
"UNDERLINED-TEXT.",
"<foreground> and <background> may be one of: BLACK, BLUE, GREEN, CYAN, RED",
"MAGENTA, BROWN, LGRAY, DGRAY, LBLUE, LGREEN, LCYAN, LRED, LMAGENTA, YELLOW",
"or WHITE.  The L prefix for the color names means Light.\n",
"SET TERMINAL CONTROLS { 7, 8 } to specify whether or not C1 control",
"characters (CSI and SS3) should be used when communicating with the host.\n"

"SET TERMINAL COLOR ERASE { CURRENT-COLOR, DEFAULT-COLOR }",
"Determines whether the current color as set by the host or the default color",
"as set by the user (SET TERMINAL COLOR TERMINAL) is used to clear the screen",
"when erase commands are received from the host.\n",

"SET TERMINAL COLOR RESET-ON-ESC[0m { CURRENT-COLOR, DEFAULT-COLOR }",
"Determines whether the current color or the default color is used after",
"an ESC [ 0 m (\"reset attributes\") command sequence is received from the",
"host.\n",

"SET TERMINAL CONTROLS { 7, 8 }",
"Determines whether VT220/320 or Wyse 370 function keys, arrow keys, etc,",
"that generate ANSI-format escape sequences should send 8-bit control",
"characters or 7-bit escape sequences.\n",
#endif /* OS2 */

"SET TERMINAL CR-DISPLAY { CRLF, NORMAL } to specify how incoming",
"carriage return characters are to be displayed on your screen.\n",

#ifdef OS2
"SET TERMINAL CURSOR { FULL, HALF, UNDERLINE } selects cursor style.\n",
#endif /* OS2 */

"SET TERMINAL DEBUG { ON, OFF } controls terminal session debugging.\n",

"SET TERMINAL ECHO { LOCAL, REMOTE } specifies which side does the echoing",
"during terminal connection.\n",

"SET TERMINAL ESCAPE-CHARACTER { ENABLED, DISABLED } turns on/off the ability",
"to escape back from CONNECT mode using the SET ESCAPE character.",
#ifdef OS2
"If you disable it you can still get back using Alt-key combinations as shown",
"in the status line.\n",
#else
"If you disable it, Kermit returns to its prompt only if the connection is",
"closed by the other end.  USE WITH EXTREME CAUTION.\n",
#endif /* OS2 */

#ifdef OS2
#ifdef OS2ONLY
"SET TERMINAL FONT { CP437, CP850, CP852, CP862, CP866, DEFAULT }",
"  CP437 - Original PC code page",
"  CP850 - \"Multilingual\" (West Europe) code page",
"  CP852 - East Europe Roman Alphabet code page (for Czech, Polish, etc)",
"  CP862 - Hebrew code page",
"  CP866 - Cyrillic (Russian, Belorussian, and Ukrainian) code page",
"Loads a soft into the video adapter for use during terminal emulation.",
"Use this command when your OS/2 system does not have the desired code page.",
"Can be used only in full-screen sessions.  Also see SET TERMINAL CODE-PAGE",
"and SET TERMINAL CHARACTER-SET.\n",
#endif /* OS2ONLY */
#endif /* OS2 */

#ifdef OS2
#ifdef NT
"SET TERMINAL HEIGHT <number> Changes the number of rows (lines) to use",
"during terminal emulation, not counting the status line.  Recommended",
"values are 24, 42, and 49 (or 25, 43, and 50 if SET TERMINAL STATUSLINE",
"is OFF.)\n",
#else
"SET TERMINAL HEIGHT <number> Changes the number of rows (lines) to use",
"during terminal emulation, not counting the status line.  Windowed sessions",
"can use any value from 8 to 101.  Fullscreen sessions are limited to 24, 42,"
"49, or 59.  Not all heights are supported by all video adapters.\n",
#endif /* NT */
#else
"SET TERMINAL HEIGHT <number> tells C-Kermit how many rows (lines) are on",
"your CONNECT-mode screen.\n",
#endif /* OS2 */

#ifdef OS2
"SET TERMINAL KEY <mode> <keycode> <text>",
"SET TERMINAL KEY <mode> DEFAULT",
"SET TERMINAL KEY <mode> CLEAR",
"Configure the key whose <keycode> is k to send the given text when",
"pressed while <mode> is active.  <mode> may be any of the valid terminal",
"types or the special modes \"EMACS\", \"HEBREW\" or \"RUSSIAN\".  DEFAULT", 
"restores all the default key mappings for the specified mode.  CLEAR erases",
"all the key mappings.  If there is no text, the default key binding is",
#ifndef NOCSETS
"restored for the key k.  SET TERMINAL KEY mappings take place before",
"terminal character-set translation.  SET KEY mappings take precedence over",
"SET TERMINAL KEY <terminal type> settings.",    
#else
"restored for the key k.  SET KEY mappings take precedence over SET TERMINAL",
"KEY <terminal type> settings."
#endif /* NOCSETS */
"\nThe text may contain \"\\Kverbs\" to denote actions, to stand for DEC",
"keypad, function, or editing keys, etc.  For a list of available keyboard",
"verbs, type SHOW KVERBS.",
"\nTo find out the scan code and mapping for a particular key, use the",
"SHOW KEY command.  Use the SAVE KEYS command to save all settings to a file.",

"SET TERMINAL KEYPAD-MODE { APPLICATION, NUMERIC } to specify the numeric",
"keypad mode for VT terminal emulation.\n",
#endif /* OS2 */

#ifndef NOCSETS
#ifdef OS2
"SET TERMINAL LOCAL-CHARACTER-SET <local-cs>",
"to specify the character set used by C-Kermit locally.  If you don't specify",
#ifdef OS2ONLY
"the local character-set, the current TERMINAL FONT is used, if you have",
"given a SET TERMINAL FONT command, otherwise the current code page is used.",
#else
"the local character-set, the current code page is used.",
#endif /* OS2ONLY */
"When the local and remote character sets differ, C-Kermit translates between",
"them during CONNECT.  By default, the remote character set is Latin1 and",
"the local one is your current code page.",
#ifdef NT
"In Windows NT, Unicode is used as the local character-set regardless of this",
"setting.",
#endif /* NT */
"See also SET TERMINAL REMOTE-CHARACTER-SET\n"
#endif /* OS2 */
#endif /* NOCSETS */

#ifdef OS2
"SET TERMINAL LOCKING-SHIFT { OFF, ON } tells whether to send",
"Shift-In/Shift-Out (Ctrl-O and Ctrl-N) to switch between 7-bit and 8-bit",
"characters sent during terminal emulation over 7-bit connections.",
"OFF by default.\n",
#else
"SET TERMINAL LOCKING-SHIFT { OFF, ON } tells C-Kermit whether to use",
"Shift-In/Shift-Out (Ctrl-O and Ctrl-N) to switch between 7-bit and 8-bit",
"characters during CONNECT.  OFF by default.\n",
#endif /* OS2 */
"SET TERMINAL NEWLINE-MODE { OFF, ON } tells whether to send CRLF when you",
"type CR during CONNECT mode.\n",
#ifdef OS2

"SET TERMINAL OUTPUT-PACING <milliseconds> tells how long to pause between",
"sending each character to the host during CONNECT mode.  Normally not needed",
"but sometimes required to work around TRANSMISSION BLOCKED conditions when",
"pasting into the terminal window.\n",

#ifndef NOCSETS
#ifdef OS2
"SET TERMINAL REMOTE-CHARACTER-SET <remote-cs> [ { G0,G1,G2,G3 }... ]",
"to specify the character set used by the remote host, <remote-cs>.",
"When the local and remote character sets differ, C-Kermit translates between",
"them during CONNECT.  By default, the remote character set is Latin1 and",
"the local one is your current code page.  Optionally, you can also designate",
"the character set to the G0..G3 graphic tables.\n",
#endif /* OS2 */
#endif /* NOCSETS */

"SET TERMINAL ROLL-MODE { INSERT, OVERWRITE } tells whether new data when",
"received from the host is entered into the scrollback buffer at the current",
"rollback position (OVERWRITE) or at the end of the buffer (INSERT).  The",
"default is INSERT.  Typing is allowed during rollbacks in either mode.\n",

"SET TERMINAL SCREEN-UPDATE { FAST, SMOOTH } [ <milliseconds> ] chooses",
"the mechanism used for screen updating and the update frequency.",
"Defaults are FAST scrolling with updates every 100 milliseconds.\n",

"SET TERMINAL SCROLLBACK <lines> sets size of CONNECT virtual screen buffer.",
"<lines> includes the active terminal screen.  The minimum is 256.  The",
"maximum is 2 million.  The default is 2000.\n",

"SET TERMINAL SEND-DATA { ON, OFF } determines whether ASCII emulations such",
"as WYSE 30,50,60 or TVI 910+,925,950 may send their screen contents to the",
"host upon request.  Allowing the screen to be read by the host is considered",
"a significant security risk.  The default is OFF and should only be changed",
"after a security evaluation of host environment.\n",

"SET TERMINAL SEND-END-OF-BLOCK { CRLF_ETX, US_CR } determines which set of",
"characters should be used as end of line and end of transmission indicators",
"when sending screen data to the host\n",

"SET TERMINAL SGR-COLORS { ON, OFF }",
"ON (default) means allow host control of colors; OFF means ignore host",
"escape sequences to set color.\n",

"SET TERMINAL STATUSLINE { ON, OFF }",
"ON (default) enables the Kermit status line in the terminal screen.",
"OFF removes it, making the line available for use by the host.\n",

"SET TERMINAL TRANSMIT-TIMEOUT <seconds> specifies the maximum amount of time",
"C-Kermit waits before returning to the prompt if your keystrokes can't be",
"transmitted for some reason, such as a flow-control deadlock.\n",

"SET TERMINAL VIDEO-CHANGE { DISABLED, ENABLED }",
"Tells whether Kermit should change video modes automatically in response",
#ifdef NT
"to escape sequences from the other computer.  ENABLED by default (except",
"on Windows 95).\n",
#else /* NT */
"to escape sequences from the other computer.  ENABLED by default.\n",
#endif /* NT */

#ifdef NT
"SET TERMINAL WIDTH <cols> tells how many columns define the terminal size.\n",
"Default is 80.  You can also use 132.  Other widths can be chosen but are",
"usually not supported by host software.\n",
#else
"SET TERMINAL WIDTH <cols> tells how many columns define the terminal size.\n",
"Default is 80.  In Windowed OS/2 2.x sessions, this value may not be changed",
"In Windowed OS/2 WARP 3.x sessions, this value may range from 20 to 255.",
"In Full screen sessions, values of 40, 80, and 132 are valid.  Not all",
"combinations of height and width are supported on all adapters.\n",
#endif /* NT */
"SET TERMINAL WRAP { OFF, ON } to tell whether the terminal emulator should",
"automatically wrap long lines on your screen.\n",
#else

"SET TERMINAL WIDTH <number> tells C-Kermit how many columns (characters) are",
"on your CONNECT-mode screen.\n",

#endif /* OS2 */
"Type SHOW TERMINAL to see current terminal settings.",
"" };
#endif /* NOLOCAL */

#ifdef NETCONN
static char *hxyhost[] = {
"TCP/IP Syntax: SET HOST hostname-or-address [ service ]\n",
"Establish a connection to the specified network host on the currently",
"selected network type.  For TCP/IP connections, the default service is",
"TELNET; specify a different TCP port number or service name to choose a",
"different service.  TCP/IP Examples:\n",
"  SET HOST kermit.columbia.edu",
"  SET HOST 128.59.39.2",
"  SET HOST madlab.sprl.umich.edu 3000\n",
#ifdef SUPERLAT
"\nSUPERLAT Syntax: SET HOST {service or node/port} [ password ]\n",
"Establish a connection to the specified network service or node and port",
"on the LAT network.  Passwords may be specified if required by the",
"desired service.  SUPERLAT Examples:\n",
"  SET HOST KERVAX",
"  SET HOST KERVAX/234",
"  SET HOST KERVAX password",
#endif /* SUPERLAT */
"Also see SET NETWORK, TELNET.",
"" };

#ifdef TNCODE
static char *hxytel[] = {
"Syntax: SET TELNET parameter value\n",
"For TCP/IP TELNET connections, which are in NVT (ASCII) mode by default:\n",
"SET TELNET BINARY-MODE { ACCEPTED, REFUSED, REQUESTED }",
"  ACCEPT or REFUSE binary-mode bids, or actively REQUEST binary mode.",
"  ACCEPTED by default.",
"SET TELNET BUG BINARY-ME-MEANS-U-TOO { ON, OFF }",
"  Set this to ON to try to overcome TELNET binary-mode misnegotiations by",
"  C-Kermit's TELNET partner.",
"SET TELNET ECHO { LOCAL, REMOTE }",
"  C-Kermit's initial echoing state for TELNET connections, LOCAL by default.",
"  After the connection is made, TELNET negotiations determine the echoing.",
#ifdef CK_ENVIRONMENT
"SET TELNET ENVIRONMENT variable value",
"  C-Kermit supports the TELNET NEW-ENVIRONMENT protocol negotiation RFC-1572",
"  variable may be any of the \"well known\" variables \"USER\", \"JOB\",",
"  \"ACCT\", \"PRINTER\", \"SYSTEMTYPE\", or \"DISPLAY\".  The format of the",
"  value to be used for each variable is system dependent as determined by",
"  the remote system.  (See RFC-1572 for further details.)",
#endif /* CK_ENVIRONMENT */
"SET TELNET { NVT, BINARY-MODE } NEWLINE-MODE { OFF, ON, RAW }",
"  Determines how carriage returns are handled on TELNET connections.",
"  There are separate settings for NVT (ASCII) mode and binary mode.",
"  ON (default for NVT mode) means CRLF represents CR.",
"  OFF means CR followed by NUL represents CR.",
"  RAW (default for BINARY mode) means CR stands for itself.",
"SET TELNET TERMINAL-TYPE name",
"  The terminal type to send to the remote TELNET host.  If none is given,",
#ifdef OS2
"  your current SET TERMINAL TYPE value is sent, e.g. VT220.\n",
#else
"  your local terminal type is sent.\n",
#endif /* OS2 */
"Type SHOW NETWORK to see the current values of these parameters.",
"" };
#endif /* TNCODE */

static char *hxynet[] = {
"Syntax: SET NETWORK { network-type, DIRECTORY [ file [ file [ ... ] ] ] }\n",
"Select the type of network to be used with SET HOST connections:\n",
#ifdef TCPSOCKET
"  SET NETWORK TCP/IP",
#endif /* TCPSOCKET */
#ifdef ANYX25
"  SET NETWORK X.25",
#endif /* ANYX25 */
#ifdef DECNET
"  SET NETWORK DECNET",
#endif /* DECNET */
#ifdef NPIPE
"  SET NETWORK NAMED-PIPE <pipename>",
#endif /* NPIPE */
#ifdef CK_NETBIOS
"  SET NETWORK NETBIOS",
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
"  SET NETWORK SUPERLAT",
#endif /* SUPERLAT */
"\nIf only one network type is listed above, that is the default network for",
"SET HOST commands.  Also see SET HOST, TELNET.\n",
"SET NETWORK DIRECTORY specifies the name(s) of zero or more network",
"directory files, similar to dialing directories (HELP DIAL for details).",
"The general format of a network directory entry is:\n",
"  name network-type address [ network-specific-info ] [ ; comment ]\n",
"For TCP/IP, the format is:\n",
"  name tcp/ip ip-hostname-or-address [ socket ] [ ; comment ]\n",
"You can have multiple network directories and you can have multiple entries",
"with the same name.  SET HOST <name> and TELNET <name> commands look up the",
"given <name> in the directory and, if found, fill in the additional items",
"from the entry, and then try all matching entries until one succeeds.",
""};

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
static char *hxytcp[] = {
"SET TCP KEEPALIVE { ON, OFF }",
"  Setting this ON might help to detect broken connections more quickly.",
"  (default is ON.)\n",
"SET TCP LINGER { ON [timeout], OFF }",
"  Setting this ON ensures that a connection doesn't close before",
"  all outstanding data has been transferred and acknowledged.",
"  timeout measured in 10th of a millisecond.",
"  (default is ON with a timeout of 0.)\n",
"SET TCP NODELAY { ON, OFF }",
"  ON means send short TCP packets immediately rather than waiting",
"  to accumulate a bunch of them before transmitting (Nagle Algorithm).",
"  (default is OFF.)\n",
"SET TCP RECVBUF <number>",
"SET TCP SENDBUF <number>",
"  TCP receive and send buffer sizes.  (default is -1, use system defaults.)\n",
"These items are designed to let you tune TCP networking performance on a",
"per-connection basis by adjusting items that you normally would not have",
"access to.  You should use these commands only if you feel that the TCP/IP",
"protocol stack that Kermit is using is giving you inadequate performance, and",
"then only if you understand the concepts (see, for example, the Comer TCP/IP",
"books), and then at your own risk.  These settings are displayed by SHOW",
"NETWORK.  Not all options are necessarily available in all Kermit versions;",
"it depends on the underlying TCP/IP services.",
""};
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */
#endif /* NETCONN */
 
#ifndef NOSPL
static char *hxymacr[] = {
"Syntax: SET MACRO parameter value\n",
"Controls the behavior of macros.\n",
"SET MACRO ECHO { ON, OFF } tells whether commands executed from a macro",
"definition should be displayed on the screen.\n",
"SET MACRO ERROR { ON, OFF } tells whether a macro should be automatically",
"terminated upon a command error.  This setting is local to the current",
"macro, and inherited by subordinate macros.",
"" };
#endif /* NOSPL */

static char *hmxyprm[] = {
"Syntax: SET PROMPT [ text ]\n",
#ifdef MAC
"Prompt text for this program, normally 'Mac-Kermit>'.  May contain backslash",
#else
"Prompt text for this program, normally 'C-Kermit>'.  May contain backslash",
#endif /* MAC */
"codes for special effects.  Surround by { } to preserve leading or trailing",
#ifdef MAC
"spaces.  If text omitted, prompt reverts to Mac-Kermit>.  Prompt can include",
#else
"spaces.  If text omitted, prompt reverts to C-Kermit>.  Prompt can include",
#endif /* MAC */
"variables like \\v(dir) or \\v(time) to show current directory or time.",
"" };

static char *hxywind[] = {
"Syntax: SET WINDOW-SIZE number\n",
"Specify number of window slots for sliding windows, the number of packets",
"that can be transmitted before pausing for acknowledgement.  The default",
#ifdef XYZ_INTERNAL
"for Kermit protocol is one, the maximum is 32; for ZMODEM, the default",
"is no windowing (0).  For ZMODEM, the window size is really the packet",
"length, and is used only when non-windowed (streaming) transfers fail; the",
"ZMODEM window size should be a largish number, like 1024, and it should be",
"a multiple of 64.",
#else
"is one, the maximum is 32.  Increased window size may result in reduced",
"maximum packet length.  Use sliding windows for improved efficiency on",
"connections with long delays.  A full duplex connection is required, as well",
"as a cooperating Kermit on the other end.",
#endif /* XYZ_INTERNAL */
"" };

static char *hxyrpt[] = {
"Syntax: SET REPEAT { COUNTS { ON, OFF }, PREFIX <code> }\n",
"SET REPEAT COUNTS turns the repeat-count compression mechanism ON and OFF.",
" The default is ON.",
"SET REPEAT PREFIX <code> sets the repeat-count prefix character to the",
" given code.  The default is 126 (tilde).",
"" };

static char *hxyrcv[] = { 
"Syntax: SET RECEIVE parameter value\n",
"Specify parameters for inbound packets:\n",
"CONTROL-PREFIX number",
" ASCII value of prefix character used for quoting control characters in",
" packets that C-Kermit receives, normally 35 (number sign).  Don't change",
" this unless something is wrong with the other Kermit program.",
"END-OF-PACKET number",
" ASCII value of control character that terminates incoming packets,",
" normally 13 (carriage return).",
"PACKET-LENGTH number",
" Maximum length packet the other Kermit should send.",
"PADDING number",
" Number of prepacket padding characters to ask for (normally 0).",
"PAD-CHARACTER number",
" ASCII value of control character to use for padding (normally 0).",
"PATHNAMES ON or OFF",
" If a recognizable path (directory, device, etc) specification appears in",
" an incoming filename, leave it ON and try to use it, or strip it OFF before",
#ifdef CK_MKDIR
" trying to create the output file.  When ON (the default), then if any of",
" the directories in the path don't exist, C-Kermit tries to create them.",
#else
" trying to create the output file.  The default is to leave it ON.",
#endif /* CK_MKDIR */
"PAUSE number",
" Milliseconds to pause in between packets, normally 0.",
"START-OF-PACKET number",
" ASCII value of character that marks start of inbound packet.",
"TIMEOUT number",
" Number of seconds the other Kermit should wait for a packet before sending",
" a NAK or retransmitting.",
"" };

static char *hxysnd[] = {
"Syntax: SET SEND parameter value\n",
"Specify parameters for outbound packets.  This command should be used only",
"to override the normal negotiated parameters and is rarely needed:\n",
"CONTROL-PREFIX number",
" ASCII value of prefix character used for quoting control characters in",
" packets that C-Kermit sends, normally 35 (number sign).",
"END-OF-PACKET number",
" ASCII value of control character to terminate an outbound packet,",
" normally 13 (carriage return).",
"PACKET-LENGTH number",
" Maximum length packet to send, even if other Kermit asks for longer ones.",
"PADDING number",
" Number of prepacket padding characters to send.",
"PAD-CHARACTER number",
" ASCII value of control character to use for padding.",
"PATHNAMES ON or OFF",
" When FILE NAMES is set to LITERAL, leave the path (device, directory, etc)",
" portion ON or OFF the file name when sending it.  Applies to the actual",
" filename, not the \"as-name\".  The default is ON.",
"PAUSE number",
" Milliseconds to pause in between packets, normally 0.",
"START-OF-PACKET number",
" ASCII value of character to mark start of outbound packet.",
"TIMEOUT number",
" Number of seconds to wait for a packet before sending NAK or",
#ifdef CK_TIMERS
" retransmitting.  Include the word DYNAMIC after the number in the",
" SET SEND TIMEOUT command to have Kermit compute the timeouts dynamically",
" throughout the transfer based on the packet rate.  Include the word FIXED",
" to use the \"number\" given throughout the transfer.  DYNAMIC is the",
" default.  After DYNAMIC you may include minimum and maximum values.",
#else
" retransmitting.",
#endif /* CK_TIMERS */
"" };

static char *hxyxfer[] = {
"Synonym: SET XFER\n",
#ifdef XFRCAN
"Syntax: SET TRANSFER CANCELLATION { OFF, ON [ <code> [ <number> ] ] }\n",
"OFF disables remote-mode packet-mode cancellation from the keyboard.",
"ON enables it.  The optional <code> is the control character to use for",
"cancellation; the optional <number> is how many consecutive occurrences",
"of the given control character are required for cancellation.",
#endif /* XFRCAN */
#ifndef NOSPL
"\nSyntax: SET TRANSFER CRC-CALCULATION { OFF, ON }\n",
"Tells whether C-Kermit should accumulate a Cyclic Redundancy Check for each",
"file transfer.  Normally ON, in which case the CRC value is available in the",
"\\v(crc16) variable after the transfer.  Adds a little bit of overhead.  Use",
"SET TRANSFER CRC OFF to disable.",
#endif /* NOSPL */
#ifndef NOCSETS
"\nSyntax: SET TRANSFER CHARACTER-SET name\n",
"Select the character set used to represent textual data in Kermit packets.",
"Text characters are translated to/from the FILE CHARACTER-SET.  Choices:\n",
" TRANSPARENT (no translation, the default)",
" ASCII",
" LATIN1 (ISO 8859-1 Latin Alphabet 1)",
#ifndef NOLATIN2
" LATIN2 (ISO 8859-2 Latin Alphabet 2)",
#endif /* NOLATIN2 */
#ifdef CYRILLIC
" CYRILLIC-ISO (ISO 8859-5 Latin/Cyrillic)",
#endif /* CYRILLIC */
#ifdef HEBREW
" HEBREW-ISO (ISO 8859-8 Latin/Hebrew)",
#endif /* HEBREW */
#ifdef KANJI
" JAPANESE-EUC (JIS X 0208 Kanji + Roman and Katakana)\n",
#endif /* KANJI */
#endif /* NOCSETS */
"\nSyntax: SET TRANSFER LOCKING-SHIFT { OFF, ON, FORCED }\n",
"Tell whether locking-shift protocol should be used during file transfer",
"to achieve 8-bit transparency on a 7-bit connection.  ON means to request",
"its use if PARITY is not NONE and to use it if the other Kermit agrees,",
"OFF means not to use it, FORCED means to use it even if the other Kermit",
"does not agree.",
"\nSyntax: SET TRANSFER MODE { AUTOMATIC, MANUAL }\n",
"Automatic (the default) means C-Kermit should automatically go into binary",
"file-transfer mode and use literal filenames if the other Kermit says it has",
"a compatible file system, e.g. UNIX-to-UNIX, but not UNIX-to-DOS.",
#ifdef CK_XYZ
"\nSyntax: SET TRANSFER PROTOCOL { KERMIT, XMODEM, ... }\n",
"Synonym for SET PROTOCOL (q.v.).",
#endif /* CK_XYZ */
"\nSyntax: SET TRANSFER SLOW-START { OFF, ON }\n",
"ON (the default) tells C-Kermit, when sending files, to gradually build up",
"the packet length to the maximum negotiated length.  OFF means start sending",
"the maximum length right away.",
"" };

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

#ifdef OS2
case XYBELL:
    return(hmsg("SET BELL { AUDIBLE [ { BEEP, SYSTEM-SOUNDS } ], VISIBLE, NONE }\n\
specifies how incoming Ctrl-G (bell) characters are handled in CONNECT mode\n\
and how command warnings are presented in command mode.  AUDIBLE means\n\
either a beep or a system-sound is generated; VISIBLE means the screen is\n\
flashed momentarily.\n"));

case XYPRTY:
    return(hmsg("SET PRIORITY { REGULAR, FOREGROUND-SERVER, TIME-CRITICAL }\n\
specifies at which priority level the communication and screen update threads\n\
should operate at.  The default value is FOREGROUND-SERVER.\n"));
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
Discard or Keep incompletely received files, default is KEEP."));

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

#ifndef NOLOCAL
case XYDEBU:
#ifdef DEBUG    
    return(hmsg("Syntax: SET DEBUG { SESSION, ON, OFF }\n\
SESSION means display control and 8-bit characters symbolically during\n\
CONNECT mode.  ON means log debugging information to file debug.log."));
#else
    return(hmsg("Syntax: SET DEBUG { SESSION, OFF }\n\
SESSION means display control and 8-bit characters symbolically during\n\
CONNECT mode."));
#endif /* DEBUG */
#else
#ifdef DEBUG
case XYDEBU:
    return(hmsg("Syntax: SET DEBUG { ON, OFF }\n\
ON means log debugging information to file debug.log."));
#endif /* DEBUG */
#endif /* NOLOCAL */

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
    return(hmsg("Syntax: SET SUSPEND { OFF, ON }\n\n\
Disables SUSPEND command, suspend signals, and <esc-char>Z during CONNECT."));
#endif

#ifndef NOSCRIPT
case XYSCRI:
    return(hmsg("Syntax: SET SCRIPT ECHO { OFF, ON }\n\n\
Disables/Enables echoing of SCRIPT command operation."));
#endif /* NOSCRIPT */

case XYTAKE:
    return(hmsga(hxytak));

#ifndef NOLOCAL
case XYTERM:
    return(hmsga(hxyterm));

case XYDUPL:
    return(hmsg("Syntax: SET DUPLEX { FULL, HALF }\n\n\
During CONNECT: FULL means remote host echoes, HALF means C-Kermit\n\
does its own echoing."));

case XYLCLE:
    return(hmsg("Syntax: SET LOCAL-ECHO { OFF, ON }\n\n\
During CONNECT: OFF means remote host echoes, ON means C-Kermit\n\
does its own echoing.  Synonym for SET DUPLEX { FULL, HALF }."));

case XYESC:
#ifdef OS2
    return(hmsg("Syntax: SET ESCAPE number\n\n\
Decimal ASCII value for escape character during CONNECT, normally 29\n\
(Control-]).  Type the escape character followed by C to get back to the\n\
C-Kermit prompt or followed by ? to see other options."));
#else
    return(hmsg("Syntax: SET ESCAPE number\n\n\
Decimal ASCII value for escape character during CONNECT, normally 28\n\
(Control-\\).  Type the escape character followed by C to get back to the\n\
C-Kermit prompt or followed by ? to see other options."));
#endif /* OS2 */
#endif /* NOLOCAL */
 
#ifdef OS2
case XYPRTR:
    return(hmsga(hxyprtr));
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

case XYPAD:
    return(hmsg("Syntax: SET PAD name value\n\
Set a PAD X.3 parameter with a desired value."));
#endif /* ANYX25 */ 
#endif /* NETCONN */

#ifndef NOSPL
case XYOUTP:
    return(hmsg("Syntax: SET OUTPUT PACING <number>\n\
How many milliseconds to pause after sending each OUTPUT character."));
#endif /* NOSPL */

#ifndef NOSETKEY
case XYKEY:				/* SET KEY */
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
    printf("\nSyntax: SET LINE devicename\n\
or:     SET PORT devicename\n\n\
Select communication device to use.  Normally %s.\n",dftty);
    if (!dfloc) {
	printf("\
If you SET LINE to other than %s, then Kermit\n",dftty);
	printf("\
will be in 'local' mode; SET LINE alone will reset Kermit to remote mode.\n\
To use the modem to dial out, first SET MODEM TYPE (e.g., to HAYES), then");
	printf("\
\nSET LINE xxx, next issue the DIAL command, and finally CONNECT.\n\n");
    }
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
    return(hmsg("Syntax: SET PARITY name\n\n\
Parity to use during terminal connection and file transfer: EVEN, ODD, MARK,\n\
SPACE, or NONE.  Normally NONE."));
 
case XYPROM:
    return(hmsga(hmxyprm));
 
case XYQUIE:
    return(hmsg("Syntax: SET QUIET {ON, OFF}\n\n\
Normally OFF.  ON disables most information messages during interactive\n\
operation."));

#ifdef CK_SPEED
case XYQCTL:
    return(hmsga(hmxyqctl));
#endif /* CK_SPEED */

case XYRETR:
    return(hmsg("Syntax: SET RETRY number\n\n\
How many times to retransmit a particular packet before giving up."));

#ifndef NOLOCAL
#ifdef UNIX
case XYSESS:
    return(hmsg("Syntax: SET SESSION-LOG { BINARY, TEXT }\n\n\
If BINARY, record all CONNECT characters in session log.  If TEXT, strip\n\
out carriage returns."));
#else
#ifdef OSK
case XYSESS:
    return(hmsg("Syntax: SET SESSION-LOG { BINARY, TEXT }\n\n\
If BINARY, record all CONNECT characters in session log.  If TEXT, strip\n\
out linefeeds."));
#endif /* OSK */
#endif /* UNIX */

case XYSPEE:
    return(hmsg("Syntax: SET SPEED number\n\n\
Communication line speed for external tty line specified in most recent\n\
SET LINE command, in bits per second.  Type SET SPEED ? for a list of\n\
possible speeds."));
#endif /* NOLOCAL */

case XYRECV:
    return(hmsga(hxyrcv));
case XYSEND:
    return(hmsga(hxysnd));
case XYREPT:
    return(hmsga(hxyrpt));

#ifndef NOSERVER
case XYSERV:
    return(hmsga(hsetsrv));
#endif /* NOSERVER */

#ifdef TNCODE
case XYTEL:
    return(hmsga(hxytel));
#endif /* TNCODE */

#ifndef NOXMIT
case XYXMIT:
    return(hmsga(hsetxmit));
#endif /* NOXMIT */

#ifndef NOCSETS
case XYUNCS:
    return(hmsg("Syntax: SET UNKNOWN-CHAR-SET action\n\n\
DISCARD (default) means reject any arriving files encoded in unknown\n\
character sets.  KEEP means to accept them anyway."));
#endif /* NOCSETS */

#ifdef UNIX
case XYWILD:
    return(hmsg("Syntax: SET WILDCARD-EXPANSION { KERMIT, SHELL }\n\n\
KERMIT (the default) means C-Kermit expands filename wildcards in SEND\n\
and MSEND commands and incoming GET commands.  SHELL means your shell does \
it."));
#endif /* UNIX */

case XYWIND:
    return(hmsga(hxywind));

case XYXFER:
    return(hmsga(hxyxfer));

#ifdef OS2MOUSE
case XYMOUSE:
    return(hmsga(hxymouse));
#endif /* OS2MOUSE */

case XYALRM:
    return(hmsg("Syntax: SET ALARM [ <seconds> ]\n\n\
Number of seconds from now after which IF ALARM will succeed.\n\
0 means no alarm."));

case XYPROTO:
#ifndef CK_XYZ
    return(hmsg("Syntax: SET PROTOCOL KERMIT\n\n\
Chooses the protocol to be used for transferring files.  KERMIT is the\n\
only choice in this version of C-Kermit."));
#else
#ifdef XYZ_INTERNAL
    return(hmsga(hxyixyz));
#else
    return(hmsga(hxyxyz));
#endif /* XYZ_INTERNAL */
#endif /* CK_XYZ */

#ifdef CK_SPEED
case XYPREFIX:
    return(hmsg("Syntax: SET PREFIXING { ALL, CAUTIOUS, MINIMAL, NONE }\n\n\
Selects the degree of control-character prefixing.  Also see HELP SET CONTROL."
));
#endif /* CK_SPEED */

#ifdef OS2
case XYLOGIN:
    return(hmsg("Syntax: SET LOGIN { USERID, PASSWORD, PROMPT } <text>\n\n\
Provides access information for use by login scripts."));
#endif /* OS2 */

#ifndef NOSPL
case XYTMPDIR:
    return(hmsg("Syntax: SET TEMP-DIRECTORY [ <directory-name> ]\n\n\
Overrides automatic assignment of \\v(tmpdir) variable."));
#endif /* NOSPL */

case XYTITLE:
    return(hmsg("Syntax: SET TITLE <text>\n\
Sets window title to text instead of using current host/port name.")); 

default:
    printf("Not available - \"%s\"\n",cmdbuf);
    return(0);
    }
}

 
/*  D O H R M T  --  Give help about REMOTE command  */
 
static char *hrset[] = {
"Syntax:  REMOTE SET parameter value",
"Example: REMOTE SET FILE TYPE BINARY\n",
"Ask the remote Kermit server to set the named parameter to the given value.",
"Equivalent to typing the corresponding SET command directly to the other",
"Kermit if it were in interactive mode.", "" };

int
dohrmt(xx) int xx; {
    int x;
    if (xx == -3) return(hmsga(hmhrmt));
    if (xx < 0) return(xx);
    if ((x = cmcfm()) < 0) return(x);
    switch (xx) {

case XZCPY:
    return(hmsg("Syntax: REMOTE COPY source destination\n\n\
Ask the remote Kermit server to copy the source file to destination."));

case XZCWD:
    return(hmsg("Syntax: REMOTE CD [ name ]\n\n\
Ask remote Kermit server to change its working directory or device.\n\
If the device or directory name is omitted, restore the default."));
 
case XZDEL:
    return(hmsg("Syntax: REMOTE DELETE filespec\n\n\
Ask the remote Kermit server to delete the named file(s)."));
 
case XZDIR:
    return(hmsg("Syntax: REMOTE DIRECTORY [ filespec ]\n\n\
Ask the remote Kermit server to provide a directory listing of the named\n\
file(s) or if no file specification is given, of all files in the current\n\
directory."));
 
case XZHLP:
    return(hmsg("Syntax: REMOTE HELP\n\n\
Ask the remote Kermit server to list the services it provides."));
 
case XZHOS:
    return(hmsg("Syntax: REMOTE HOST command\n\n\
Send a command to the remote host computer in its own command language\n\
through the remote Kermit server."));
 
#ifndef NOFRILLS
case XZKER:
    return(hmsg("Syntax: REMOTE KERMIT command\n\n\
Send a command to the remote Kermit server in its own command language."));

case XZLGI:
    return(hmsg("Syntax: REMOTE LOGIN user password [ account ]\n\n\
Log in to a remote Kermit server that requires you login."));

case XZLGO:
    return(hmsg("Syntax: REMOTE LOGOUT\n\n\
Log out from a remote Kermit server to which you have previously logged in."));

case XZPRI:
    return(hmsg("Syntax: REMOTE PRINT filespec [ options ]\n\n\
Send the specified file(s) to the remote Kermit and ask it to have the\n\
file printed on the remote system's printer, using any specified options."));
#endif /* NOFRILLS */

case XZREN:
    return(hmsg("Syntax: REMOTE RENAME filespec newname\n\n\
Ask the remote Kermit server to rename the file."));

case XZSET:
    return(hmsga(hrset));

case XZSPA:
    return(hmsg("Syntax: REMOTE SPACE [ name ]\n\n\
Ask the remote Kermit server to tell you about its disk space on the current\n\
disk or directory, or in the one that you name."));
 
#ifndef NOFRILLS
case XZTYP:
    return(hmsg("Syntax: REMOTE TYPE file\n\n\
Ask the remote Kermit server to type the named file(s) on your screen."));
 
case XZWHO:
    return(hmsg("Syntax: REMOTE WHO [ name ]\n\n\
Ask the remote Kermit server to list who's logged in, or to give information\n\
about the named user."));
#endif /* NOFRILLS */

#ifndef NOSPL
case XZQUE:
    return(hmsg(
"Syntax: REMOTE QUERY { KERMIT, SYSTEM, USER } variable-name\n\n\
Ask the remote Kermit server to send the value of the named variable of the\n\
given type, and make it available in the \\v(query) variable."));

case XZASG:
    return(hmsg(
"Syntax: REMOTE ASSIGN variable-name [ value ]\n\n\
Assign the given value to the named global variable on the server."));

#endif /* NOSPL */
default:
    if ((x = cmcfm()) < 0) return(x);
    printf("not working yet - %s\n",cmdbuf);
    return(-2);
    }
}
#endif /* NOHELP */
#endif /* NOICP */
