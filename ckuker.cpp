.\" @(#) kermit.1 5A(190) 94/10/04 Columbia University
#ifdef COMMENT
This man page must be run through the C-Kermit makefile before it can
be used.  Or you can do the following:
  cc -E ckuker.cpp | grep -v ^$ | grep -v ^\# > ckuker.nr
to produce the nroff source file.  THIS FILE MUST NOT CONTAIN ANY BLANK LINES!
Use .sp or similar commands to produce blank lines in the nroff output.
#endif /* COMMENT */
#ifdef SUNOS4
#define SUNOS
#else
#ifdef SUNOS41
#define SUNOS
#else
#ifdef SOLARIS
#define SUNOS
#endif
#endif
#endif
#ifdef HPUX10
.TH KERMIT 1 "4 Oct 94" "HP-UX C-Kermit"
#define DIALOUT /dev/cul0p0
#define HARDWIRE /dev/tty0p0
#else
.TH KERMIT 1C "4 Oct 94" "UNIX C-Kermit"
#define DIALOUT /dev/cua
#define HARDWIRE /dev/tty01
#endif
.SH NAME
kermit \- C-Kermit 5A(190) communications software for serial and network
connections: modem dialing, file transfer and management, terminal connection,
character-set translation, and script programming.
.SH SYNOPSIS
.B kermit
[ command-file ] [ options ... ]
.SH DESCRIPTION
.I Kermit
is a family of file transfer, management, and communication software programs
from Columbia University available for most computers and operating systems.
#ifdef HPUX10
The version of Kermit for Hewlett-Packard HP-UX, called
#else
#ifdef SUNOS
The version of Kermit for SunOS and Solaris, called
#else
The UNIX version of Kermit, called
#endif
#endif
.IR "C-Kermit",
#ifdef HPUX10
supports both serial connections (direct or dialed) and TCP/IP connections.
#else
#ifdef SUNOS
supports serial connections (direct or dialed),
TCP/IP connections, and on systems equipped with
SunLink X.25, C-Kermit can also make X.25 connections.
#else
supports serial connections (direct or dialed) and, in most UNIX
implementations, also TCP/IP connections.  On SunOS systems equipped with
SunLink X.25, C-Kermit can also make X.25 connections.
#endif
#endif
C-Kermit can be thought of as a user-friendly and powerful alternative to cu,
tip, uucp, ftp, and telnet; a single package for both network and serial
communications, offering automation, convenience, and language features not
found in the other packages, and having a great deal in common with its
cousins, C-Kermit on other UNIX platforms, MS-DOS Kermit for PCs with DOS and
Windows, and IBM Mainframe Kermit-370 for VM/CMS, MVS/TSO, and CICS.  C-Kermit
itself also runs on OS/2, Digital VMS, Data General AOS/VS, Stratus VOS, OS-9,
Apollo Aegis, the Apple Macintosh, Commodore Amiga, and the Atari ST.
Together, C-Kermit, MS-DOS Kermit, and IBM Mainframe Kermit offer a consistent
and nearly universal approach to inter-computer communications.
.PP
C-Kermit 5A(190) is Copyright (C) 1985, 1994 by the Trustees of Columbia
University in the City of New York.  The C-Kermit software may not be, in
whole or in part, licensed or sold for profit as a software product itself,
nor may it be included in or distributed with commercial products or otherwise
distributed by commercial concerns to their clients or customers without
written permission of the Office of Kermit Development and Distribution,
Columbia University.  This copyright notice must not be removed, altered, or
obscured.
.PP
#ifdef HPUX10
C-Kermit 5A(190) is included with HP-UX 10.0 by Hewlett-Packard in partnership
with the Office of Kermit Development and Distribution, Columbia University.
#endif
.PP
C-Kermit is thoroughly documented in the book
.IR "Using C-Kermit"
by Frank da Cruz and Christine M. Gianone, Digital Press, 1993; see REFERENCES
at the end of this manual page.  New features added since the book was
published are documented in the online file
.IR "ckcker.upd".
Hints, tips, limitations, restrictions are listed in
.IR "ckcker.bwr"
(general C-Kermit) and
.IR "ckuker.bwr"
(UNIX-specific); see FILES below.  Please consult all of these references
before reporting problems or asking for technical support.
.PP
Kermit software is available for hundreds of different computers and operating
systems from Columbia University.  For best file-transfer results, please use
C-Kermit in conjunction with real Columbia University Kermit software on other
computers, such as MS-DOS Kermit for DOS or Windows.  See CONTACTS below.
.SH "MODES OF OPERATION"
C-Kermit can be used in two "modes": remote and local.  In
.IR "remote mode",
you connect to the UNIX system from a desktop PC, Macintosh, or workstation
and transfer files between your desktop computer and UNIX C-Kermit.  In that
case, connection establishment (dialing, TELNET connection, etc) is handled
by the Kermit program on your desktop computer.
.PP
In
.IR "local mode",
C-Kermit establishes a connection to another computer
by direct serial connection, by dialing a modem, or by making a network
connection.  When used in local mode, C-Kermit gives you a terminal connection
to the remote computer, using your actual terminal, emulator, or UNIX
workstation terminal window or console driver for specific terminal emulation.
.PP
C-Kermit also has two types of commands: the familiar UNIX-style command-line
options, and an interactive dialog with a prompt.
.IR "Command-line options"
give
you access to a small but useful subset of C-Kermit's features for terminal
connection and file transfer, plus the ability to pipe files into or out of
Kermit for transfer.
.PP
.IR "Interactive commands"
give you access to dialing, script programming,
character-set translation, and, in general, detailed control and display of all
C-Kermit's features.  Interactive commands can also be collected into command
files or macros.
.PP
.SH "STARTING C-KERMIT"
.PP
#ifdef HPUX10
You can start C-Kermit by typing "/usr/bin/kermit", or just "kermit" if your
PATH includes "/usr/bin", possibly followed by command-line options.
#else
C-Kermit should be available as "kermit" somewhere in your PATH, perhaps as
/usr/local/bin/kermit, in which case you can
start C-Kermit just by typing "kermit", possibly followed by command-line
options.
#endif
If there are no "action options" on the command line (explained
below), C-Kermit starts in interactive command mode; you will see a greeting
message and then the "C-Kermit>" prompt.  If you do include action options on
the command line, C-Kermit takes the indicated actions and then exits directly
back to UNIX.  Either way, C-Kermit executes the commands in its
initialization file,
#ifdef HPUX10
.IR "/usr/share/lib/kermit/ckermit.ini" ,
#else
.IR ".kermrc" ,
in your home directory (or a system-wide directory if C-Kermit was built to
do this)
#endif
before it executes any other commands, unless you have
included the `\|\c
.B \-Y\c
\&\|' (uppercase) command-line option, which means to skip the
initialization file, or you have included the `\|\c
.B -y \c
\&\|
.IR "filename" \c
\&\|'
option to specify an alternative initialization file.
.PP
.SH "FILE TRANSFER"
.PP
Here is the most common scenario for Kermit file transfer.  Many other
methods are possible, most of them more convenient, but this basic method
should work in all cases.
.PP
.in +0.5i
.ll -0.5i
.ta +0.2i
.ti -0.2i
\(bu	Start Kermit on your local computer and establish a connection to the
remote computer.  If C-Kermit is on your local
computer, use the sequence SET MODEM
.IR "modem-name" \c
\&\|, SET LINE
.IR "device-name" \c
\&\|, SET SPEED
.IR "bits-per-second" \c
\&\|, and DIAL
.IR "phone-number"
if you are dialing; SET LINE and SPEED for direct connections;
SET NETWORK
.IR "network-type"
and SET HOST
.IR "host-name-or-address"
for network connections.
.sp
.ti -0.2i
\(bu	SET any other necessary communication parameters, such as PARITY,
DUPLEX, and FLOW-CONTROL.
.sp
.ti -0.2i
\(bu	Give the CONNECT command.
.sp
.ti -0.2i
\(bu	Log in to the remote computer.
.sp
.ti -0.2i
\(bu	Start Kermit on the remote computer, give it any desired SET commands
for file-, communication-, or protocol-related parameters.  If you will be
transferring binary files, give the command SET FILE TYPE BINARY to the
remote Kermit program.
.sp
.ti -0.2i
\(bu	To
.IR download
a file or file group, give the remote Kermit a SEND command, following by
a filename or "wildcard" file specification, for example:
.nf
.sp
  send oofa.txt            (send one file)
.sp
.fi
or:
.nf
.sp
  send oofa.*              (send a group of files)
.sp
.fi
To
.IR upload
a file or files, give the remote Kermit a RECEIVE command.  The sending Kermit
will tell the receiving Kermit the name (and other attributes) of each file.
.sp
.ti -0.2i
\(bu	Escape back to the Kermit program on your local (desktop) computer.  If
your local computer is running C-Kermit, type Ctrl-\\ c (Control-backslash
followed by the letter 'c') (on NeXT workstations, use Ctrl-] c).  If MS-DOS
or OS/2 Kermit, use Alt-x (hold down the Alt key, press 'x').  Now you should
see your local Kermit program's prompt.
.sp
.ti -0.2i
\(bu	If you will be transferring binary files, give the command SET FILE
TYPE BINARY to your local Kermit program.
.sp
.ti -0.2i
\(bu	If you are
.IR downloading
files, tell the local Kermit program to RECEIVE.  If you are
.IR "uploading",
give your local Kermit program a SEND command, specifying a filename
or wildcard file specification.  In other words, tell the
.IR remote
Kermit program what to do first, SEND or RECEIVE, then escape back to
the
.IR local
Kermit and give it the opposite command, RECEIVE or SEND.
.sp
.ti -0.2i
\(bu When you finish transferring files, give a CONNECT command.  Now you are
talking to Kermit on the remote computer again.  Type EXIT to get back to the
command prompt on the remote computer.  When you are finished using the remote
computer, log out and then (if necessary) escape back to Kermit on your local
computer.  Then you can make another connection or EXIT from the local Kermit
program.
.ll +0.5i
.in -0.5i
.fi
.PP
C-Kermit's file transfer protocol defaults are deliberately conservative,
resulting in file transfer that almost always works, but might be somewhat
slow.  To increase file transfer performance on computers and connections that
permit it, use SET RECEIVE PACKET-LENGTH to increase the packet length, SET
WINDOW to increase the window size, and use SET CONTROL UNPREFIX to reduce the
overhead of control-character prefixing.  On serial connections, use hardware
flow control (SET FLOW RTS/CTS) if available, rather than software (XON/XOFF)
flow control.  On TCP/IP connections, SET FLOW NONE.  For details, read
Chapter 8 of
.IR "Using C-Kermit"
and see the section on control-character unprefixing in the
.IR "ckcker.upd"
file.
.SH OTHER FEATURES
C-Kermit includes features too numerous to be explained in a man page.  For
further information about connection establishment, modem dialing, networks,
terminal connection, key mapping, logging, file transfer options and features,
troubleshooting, client/server operation, character-set translation during
terminal connection and file transfer, "raw" up- and downloading of files,
macro construction, script programming, convenience features, and shortcuts,
plus a command reference, numerous tables, examples, and illustrations, please
consult
.IR "Using C-Kermit".
.SH HELP
.PP
C-Kermit has extensive built-in help.  You can find out what commands exist by
typing ? at the C-Kermit> prompt.  You can type HELP at the C-Kermit> prompt
for "getting-started" message, or HELP followed by the name
of a particular command for information about that command, for example:
.nf
.sp
  help send
.sp
.fi
or:
.nf
.sp
  help set file
.sp
.fi
You can type ? anywhere within a command to get brief help about the
current command field.  You can also type the INTRO command to get a brief
introduction to C-Kermit, and the NEWS command to find out what's new in
your version.  Finally, you can use the BUG command to learn how to report
bugs.
.sp
.SH "ENTERING COMMANDS"
.sp
You can use upper or lower case for interactive-mode commands, but remember
that UNIX filenames are case-sensitive.  You can abbreviate commands as long
as the abbreviation matches only one possibility.  While typing a command, you
can use the following editing characters:
.nf
.sp
  Delete, Backspace, or Rubout erases the rightmost character.
  Ctrl-W erases the rightmost "word".
  Ctrl-U erases the current command line.
  Ctrl-R redisplays the current command.
  Ctrl-P recalls a previous command (scrolls back in command buffer).
  Ctrl-N scrolls forward in a scrolled-back command buffer.
  Ctrl-C cancels the current command.
  Tab, Esc, or Ctrl-I tries to complete the current keyword or filename.
  ? gives help about the current field.
.sp
.fi
To enter the command and make it execute, press the Return or Enter key.
.sp
.SH BACKSLASH NOTATION
Within an interactive command, the "\\" character (backslash) is a prefix used
to enter special quantities, including ordinary characters that would
otherwise be illegal.  At the end of a line, \\ or - (dash) makes the next
line a continuation of the current line.  Other than that, the character
following the \\ identifies what the special quantity is:
.nf
.sp
  % A user-defined simple (scalar) variable such as \\%a or \\%1
  & an array reference such as \\&a[3]
  $ an environment variable such as \\$(TERM)
  v (or V) a built-in variable such as \\v(time)
  f (or F) a function such as \\Fsubstring(\\%a,3,2)
  d (or D) a decimal (base 10) number (1 to 3 digits, 0..255) such as \\d27
  o (or O) an octal (base 8) number (1 to 3 digits, 0..377) such as \\o33
  x (or X) a hexadecimal (base 16) number (2 digits, 00..ff) like \\x1b
  \\ the backslash character itself
  b (or B) the BREAK signal (OUTPUT command only)
  l (or L) a Long BREAK signal (OUTPUT only)
  a decimal digit (a 1-3 digit decimal number) such as \\27
  {} used for grouping, e.g. \\{27}123
  anything else: following character taken literally.
.sp
.fi
Note that numbers turn into the character with that binary code (0-255), so
you can use \\7 for a bell, \\13 for carriage return, \\10 for linefeed.
For example, to have C-Kermit send a BELL to your screen, type:
.nf
.sp
  echo \\7
.sp
.fi
.SH "COMMAND LIST"
.PP
The commands most commonly used, and important for beginners to
know, are marked with "*":
.nf
.in 0
.ll 80
.ta 16
.sp
Program Management:
  BUG	Learn how to report bugs.
  CHECK	See if a particular feature is configured.
  CLOSE	Close a log or other local file.
  COMMENT	Introduce a full-line comment.
* EXIT	Leave the program, return to UNIX.
* HELP	Display a help message for a given command.
* INTRO	Print a brief introduction to C-Kermit.
  LOG	Open a log file -- debugging, packet, session, transaction.
  PUSH	Invoke local system's interactive command interpreter.
  QUIT	Synonym for EXIT.
  REDIRECT	Redirect standard i/o of command to communication device.
  RUN	Run a program or system command.
  SET COMMAND	Command-related parameters: bytesize, recall buffer size.
  SET DEBUG	Log or display debugging information.
  SET EXIT	Items related to C-Kermit's action upon exit.
  SET PROMPT	The C-Kermit program's interactive command prompt.
  SHOW EXIT	Display SET EXIT parameters.
  SHOW FEATURES	Show features that C-Kermit was built with.
  SHOW VERSIONS	Show version numbers of each source module.
  SUSPEND	Suspend Kermit (use only if shell supports job control!).
* SHOW	Display values of SET parameters.
* TAKE	Execute commands from a file.
  VERSION	Display the C-Kermit program version number.
  Z	Synonym for SUSPEND.
  Ctrl-C	Interrupt a C-Kermit command in progress.
  Ctrl-Z	Synonym for SUSPEND.
  ; or #	Introduce a full-line or trailing comment.
  ! or @	Synonym for RUN.
  <	Synonym for REDIRECT.
.sp
Connection Establishment and Release:
* DIAL	Dial a telephone number.
* HANGUP	Hang up the phone or network connection.
  PAD	Command for X.25 PAD.
  PING	Check status of remote TCP/IP host.
  REDIAL	The the most recently DIALed number again.
  SET CARRIER	Treatment of carrier on terminal connections.
* SET DIAL	Parameters related to modem dialing.
* SET FLOW	Communication line flow control: RTS/CTS, XON/XOFF, etc.
* SET HOST	Specify remote network host name or address.
#ifdef HPUX10
* SET LINE	Specify serial communication device name, like /dev/cul0p0.
#else
* SET LINE	Specify serial communication device name, like /dev/cua.
#endif
* SET MODEM	Specify type of modem on SET LINE device, like HAYES.
* SET NETWORK	Network type, TCP/IP or X.25.
  SET PAD	X.25 X.3 PAD parameters.
* SET PARITY	Character parity (none, even, etc) for communications.
* SET SPEED	Serial communication device speed, e.g. 2400, 9600, 57600.
  SET X.25	Specify X.25 connection parameters.
  SHOW COMM	Display all communications settings.
  SHOW DIAL	Display SET DIAL values.
  SHOW MODEM	Display modem type, signals, etc.
  SHOW NETWORK	Display network-related items.
* TELNET	= SET NETWORK TCP/IP, SET HOST ..., CONNECT.
.sp
Terminal Connection:
* C	Special abbreviation for CONNECT.
* CONNECT	Establish a terminal connection to a remote computer.
  SET COMMAND	Bytesize between C-Kermit and your keyboard and screen.
* SET DUPLEX	Specify which side echoes during CONNECT.
  SET ESCAPE	Prefix for "escape commands" during CONNECT.
  SET KEY	Key mapping and macros for use in CONNECT mode.
  SET TERMINAL	Terminal connection items: bytesize, character-set, echo, etc.
  SHOW ESCAPE	Display current CONNECT-mode escape character.
  SHOW KEY	Display keycode and assigned value or macro.
  SHOW TERMINAL	Display SET TERMINAL items.
* Ctrl-\\	CONNECT-mode escape character, follow by another character:
	  C to return to C-Kermit> prompt.
	  B to send BREAK signal.
	  ? to see other options.
.sp
File Transfer:
  LOG SESSION   Download a file with no error checking.
  MSEND	Multiple SEND - accepts a list of files, separated by spaces.
* RECEIVE	Passively wait for files to arrive from other Kermit.
* R	Special abbreviation for RECEIVE.
* SEND	Send files.
* S	Special abbreviation for SEND.
  RESEND	Continue a incomplete transmission.
  PSEND	Send part of a file.
  SET ATTRIB	Control transmission of file attributes.
* SET BLOCK	Choose error-checking level, 1, 2, or 3.
  SET BUFFERS	Size of send and receive packet buffers.
  SET CONTROL	Which control characters to "unprefix" during file transfer.
  SET DELAY	How long to wait before sending first packet.
* SET FILE	Transfer mode (type), character-set, collision action, etc.
* SET RECEIVE	Parameters for inbound packets: packet-length, etc.
  SET REPEAT	Repeat-count compression parameters.
  SET RETRY	Packet retransmission limit.
  SET SEND	Parameters for outbound packets: length, etc.
  SET HANDSHAKE	Communication line half-duplex packet turnaround character.
  SET LANGUAGE	Enable language-specific character-set translations.
  SET SESSION-LOG	File type for session log, text or binary.
  SET TRANSFER	File transfer parameters: character-set, etc.
  SET TRANSMIT	Control aspects of TRANSMIT command execution.
  SET UNKNOWN	Specify handling of unknown character sets.
* SET WINDOW	File transfer packet window size, 1-31.
  SHOW ATTRIB	Display SET ATTRIBUTE values.
  SHOW CONTROL	Display control-character prefixing map.
* SHOW FILE	Display file-related settings.
  SHOW PROTOCOL	Display protocol-related settings.
  SHOW LANGUAGE	Display language-related settings.
  SHOW TRANSMIT	Display SET TRANSMIT values.
* STATISTICS	Display statistics about most recent file transfer.
  TRANSMIT	Upload a file with no error checking.
  XMIT	Synonym for TRANSMIT.
.sp
File Management:
* CD	Change Working Directory (also, CWD).
* DELETE	Delete a file or files.
* DIRECTORY	Display a directory listing.
  MAIL	Send a file to other Kermit, to be delivered as e-mail.
  PRINT	Print a local file on a local printer.
* PWD	Display current working directory.
  RENAME	Change the name of a local file.
  SPACE	Display current disk space usage.
  SHOW CHARACTER-SETS	Display character-set translation info.
  TRANSLATE	Translate a local file's character set.
  TYPE	Display a file on the screen.
  XLATE	Synonym for TRANSLATE.
.sp
Client/Server operation:
  BYE	Terminate a remote Kermit server and log out its job.
  DISABLE	Disallow access to selected features during server operation:
  E-PACKET	Send an Error packet.
  ENABLE	Allow access to selected features during server operation.
  FINISH	Instruct a remote Kermit server to exit, but not log out.
  G	Special abbreviation for GET.
  GET	Get files from a remote Kermit server.
  REMOTE CD	Tell remote Kermit server to change its directory.
  REMOTE DELETE	Tell server to delete a file.
  REMOTE DIRECTORY	Ask server for a directory listing.
  REMOTE HELP	Ask server to send a help message.
  REMOTE HOST	Ask server to ask its host to execute a command.
  REMOTE KERMIT	Send an interactive Kermit command to the server.
  REMOTE LOGIN	Authenticate yourself to a remote Kermit server.
  REMOTE LOGOUT	Log out from a Kermit server previously LOGIN'd to.
  REMOTE PRINT	Print a local file on the server's printer.
  REMOTE SET	Send a SET command to a remote server.
  REMOTE SPACE	Ask server how much disk space it has left.
  REMOTE TYPE	Ask server to display a file on your screen.
  REMOTE WHO	Ask server for a "who" or "finger" listing.
  SERVER	Be a Kermit server.
  SET SERVER	Parameters for server operation.
  SHOW SERVER	Show SET SERVER, ENABLE/DISABLE items.
.sp
Script programming:
  ASK	Prompt the user, store user's reply in a variable.
  ASKQ	Like ASK, but, but doesn't echo (useful for passwords).
  ASSIGN	Assign an evaluated string to a variable or macro.
  CLEAR	Clear communication device input buffer.
  CLOSE	Close a log or other local file.
  DECLARE	Declare an array.
  DECREMENT	Subtract one (or other number) from a variable.
  DEFINE	Define a variable or macro.
  DO	Execute a macro ("DO" can be omitted).
  ECHO	Display text on the screen.
  ELSE	Used with IF.
  END	A command file or macro.
  FOR	Execute commands repeatedly in a counted loop.
  GETC	Issue a prompt, get one character from keyboard.
  GETOK	Ask question, get Yes or No answer, set SUCCESS or FAILURE.
  GOTO	Go to a labeled command in a command file or macro.
  IF	Conditionally execute the following command.
  INCREMENT	Add one (or other number) to a variable.
  INPUT	Match characters from another computer against a given text.
  MINPUT	Like INPUT, but allows several match strings.
  MSLEEP	Sleep for given number of milliseconds.
  OPEN	Open a local file for reading or writing.
  O	Special abbreviation for OUTPUT.
  OUTPUT	Send text to another computer.
  PAUSE	Do nothing for a given number of seconds.
  READ	Read a line from a local file into a variable.
  REINPUT	Reexamine text previously received from another computer.
  RETURN	Return from a user-defined function.
  SCRIPT	Execute a UUCP-style login script.
  SET CASE	Treatment of alphabetic case in string comparisons.
  SET COUNT	For counted loops.
  SET INPUT	Control behavior of INPUT command.
  SET MACRO	Control aspects of macro execution.
  SET TAKE	Control aspects of TAKE file execution.
  SHOW ARGUMENTS	Display arguments to current macro.
  SHOW ARRAYS	Display information about active arrays.
  SHOW COUNT	Display current COUNT value.
  SHOW FUNCTIONS	List names of available \\f() functions.
  SHOW GLOBALS	List defined global variables \\%a..\\%z.
  SHOW MACROS	List one or more macro definitions.
  SHOW SCRIPTS	Show script-related settings.
  SHOW VARIABLES	Display values all \\v() variables.
  SLEEP	Sleep for given number of seconds.
  STOP	Stop executing macro or command file, return to prompt.
  WAIT	Wait for the specified modem signals.
  WHILE	Execute commands repeatedly while a condition is true.
  WRITE	Write material to a local file.
  WRITE-LINE	Write a line (record) to a local file.
  WRITELN	Synonym for WRITE-LINE.
  XIF	Extended IF command.
.ll
.in
.fi
.SH "BUILT-IN VARIABLES"
Built-in variables are referred to by \\v(name), can be used in any command,
usually used in script programming.  They cannot be changed.  Type SHOW
VARIABLES for a current list.
.nf
.sp
  \\v(argc)      number of arguments in current macro
  \\v(args)      number of program command-line arguments
  \\v(cmdfile)   name of current command file, if any
  \\v(cmdlevel)  current command level
  \\v(cmdsource) where command are currently coming from, macro, file, etc.
  \\v(connection)connection type: serial, tcp/ip, etc.
  \\v(count)     current COUNT value
  \\v(cps)       speed of most recent file transfer in chars per second
  \\v(cpu)       CPU type C-Kermit was built for
  \\v(date)      date as 8 Feb 1993
  \\v(day)       day of week
  \\v(directory) current/default directory
  \\v(dialstatus)return code from DIAL command (0 = OK, 22 = BUSY, etc)
  \\v(exitstatus)current EXIT status (0 = good, nonzero = something failed)
  \\v(filespec)  filespec given in most recent SEND/RECEIVE/GET command
  \\v(fsize)     size of file most recently transferred
  \\v(home)      home directory
  \\v(host)      computer host name
  \\v(input)     current INPUT buffer contents
  \\v(inchar)    character most recently INPUT
  \\v(incount)   how many characters arrived during last INPUT
  \\v(line)      current communications device, set by LINE or HOST
  \\v(local)     0 if in remote mode, 1 if in local mode
  \\v(macro)     name of currently executing macro, if any
  \\v(ndate)     Current date as 19930208 (yyyymmdd)
  \\v(nday)      Numeric day of week (0 = Sunday)
  \\v(newline)   System-independent newline character or sequence
  \\v(ntime)     Current local time in seconds since midnight (noon = 43200)
  \\v(parity)    Current parity setting
  \\v(platform)  Specific machine and/or operating system
  \\v(program)   Name of this program ("C-Kermit")
  \\v(return)    Most recent RETURN value
  \\v(speed)     Current speed, if known, or "unknown"
  \\v(status)    0 or 1 (SUCCESS or FAILURE of previous command)
  \\v(system)    UNIX
  \\v(terminal)  terminal type
  \\v(tfsize)    total size of file group most recently transferred
  \\v(time)      time as 13:45:23 (hh:mm:ss)
  \\v(ttyfd)     file descriptor of current communication device
  \\v(version)   numeric version of Kermit, e.g. 501190.
.fi
.SH "BUILT-IN FUNCTIONS"
Builtin functions are invoked as \\Fname(args), can be used in any command,
and are usually used in script programs.  Type SHOW FUNCTIONS for a current
list.
.nf
.sp
   \\Fcharacter(arg)    convert numeric arg to character
   \\Fcode(char)        numeric code for character
   \\Fcontents(v)       return current definition of variable
   \\Fdate(filename)    return file's modification date/time
   \\Fdefinition(m)     return current definition of macro
   \\Feval(expr)        evaluate arithmetic expression
   \\Fexecute(m a)      execute macro "m" with parameters "a"
   \\Ffiles(f)          number of files matching file spec
   \\Findex(a1,a2,a3)   position of string a2 in a1, starting at pos a3
   \\Flength(arg)       length of the string "arg"
   \\Fliteral(arg)      copy argument literally, no evaluation
   \\Flower(arg)        convert to lower case
   \\Flpad(text,n,c)    left pad text to length n with char c
   \\Fmax(a1,a2)        max of two numbers
   \\Fmin(a1,a2)        min of two numbers
   \\Fnextfile()        next file name from list in last \\Ffiles
   \\Frepeat(a1,a2)     repeat a1 a2 times
   \\Freplace(a1,a2,a3) replace a2 by a3 in a1
   \\Freverse(arg)      reverse characters in arg
   \\Fright(a1,a2)      rightmost a2 characters of string a1
   \\Frpad(text,n,c)    right pad text to length n with char c
   \\Fsize(filename)    return file's length in bytes
   \\Fsubstr(a1,a2,a3)  substring of a1, starts at a2, length a3
   \\Fupper(arg)        convert to upper case
.sp
.fi
\\Feval() allows the following operators in the expression.  The expression
can contain variables.  Only integer arithmetic is supported.
Precedences are shown as numbers, 1 is highest
precedence, 6 is lowest.
.in +.2i
.nf
.ta \w'n @ n  'u +\w'5   'u
.sp
( )	1	parentheses
n !	2	factorial
~ n	3	logical NOT
- n	4	negative
n ^ n	4	power
n * n	5	multiplication
n / n	5	division
n % n	5	modulus
n & n	5	logical AND
n + n	6	plus
n - n	6	minus
n | n	6	logical OR
n # n	6	exclusive OR
n @ n	6	greatest common divisor
.sp
.fi
.SH "COMMAND LINE OPTIONS"
.PP
C-Kermit accepts commands (or "options") on the command line, in the
time-honored UNIX style.  Alphabetic case is significant.  All options
are optional.  If one or more action options are included, Kermit exits
immediately after executing the command-line options, otherwise it enters
interactive command mode.
.nf
.ll 80
.sp
  kermit [filename] [-x arg [-x arg]...[-yyy]..]]
.sp
where:
.sp
  filename is the name of a command file to execute,
  -x is an option requiring an argument,
  -y an option with no argument.
.ta 15
.sp
Actions:
  -s files	send files
  -s -	send files from stdin
  -r	receive files
  -k	receive files to stdout
  -x	enter server mode
  -f	finish remote server
  -g files	get remote files from server (quote wildcards)
  -a name	alternate file name, used with -s, -r, -g
  -c	connect (before file transfer), used with -l or -j
  -n	connect (after file transfer), used with -l or -j
.sp
Settings:
  -l line	communication line device (to make a serial connection)
  -l n	open file descriptor of communication device
  -j host	TCP/IP network host name (to make a network connection)
  -l n	open file descriptor of TCP/IP connection
  -X	X.25 network address
  -Z	open file descriptor of X.25 connection
  -o n	X.25 closed user group call info
  -u	X.25 reverse-charge call
  -q	quiet during file transfer
  -i	transfer files in binary mode
  -b bps	serial line speed, e.g. 1200
  -m name	modem type, e.g. hayes
  -p x	parity, x = e,o,m,s, or n
  -t	half duplex, xon handshake
  -e n	receive packet-length
  -v n	window size
  -w	write over files of same name, don't backup old file
  -D n	delay n seconds before sending a file
.sp
Other:
  -y name	alternate init file name
  -Y	Skip init file
  -R	Advise C-Kermit it will be used only in remote mode
  -d	log debug info to file debug.log
  -S	Stay, don't exit, after action command
  -C "cmds"	Interactive-mode commands, comma-separated
  -z	Force foreground operation
  -h	print command-line option help screen
  =	Ignore all text that follows
.sp
Remote-mode example (C-Kermit is on the far end):
.nf
.sp
  kermit -v 4 -i -s oofa.bin
.sp
.fi
sends file oofa.bin in binary mode (-i) using a window size of 4 (-v 4).
.sp
Local-mode example (C-Kermit makes the connection):
.nf
.sp
  kermit -l HARDWIRE -b 19200 -c -r -n
.sp
.fi
makes a 19200-bps direct connection out through HARDWIRE, CONNECTs (-c) so you
can log in and, presumably start a remote Kermit program and tell it to send a
file, then it RECEIVEs the file (-r), then it CONNECTs back (-n) so you can
finish up and log out.
.sp
For dialing out, you must specify a modem type, and you might have to use a
different device name:
.nf
.sp
  kermit -m hayes -l DIALOUT -b 2400 -c -r -n
.ll
.in
.fi
.SH FILES
.nf
.ta 16
#ifndef HPUX10
$HOME/.kermrc	Standard C-Kermit initialization commands.
#endif /* HPUX10 */
$HOME/.mykermrc	Your personal C-Kermit customization file.
$HOME/.kdd	Your personal dialing directory.
$HOME/.ksd	Your personal services directory.
#ifdef HPUX10
.sp
/usr/share/lib/kermit/READ.ME      Overview of HP-UX C-Kermit, please read
/usr/share/lib/kermit/ckermit.ini  System-wide initialization file
/usr/share/lib/kermit/ckermod.ini  Sample customization file
/usr/share/lib/kermit/ckermit.kdd  Sample dialing directory
/usr/share/lib/kermit/ckermit.ksd  Sample services directory
/usr/share/lib/kermit/ckcker.upd   Supplement to "Using C-Kermit"
/usr/share/lib/kermit/ckcker.bwr   C-Kermit "beware" file - hints & tips
/usr/share/lib/kermit/ckuker.bwr   UNIX-specific beware file
/usr/share/lib/kermit/ckedemo.ini  Macros from "Using C-Kermit"
/usr/share/lib/kermit/ckevt.ini    Ditto
/usr/share/lib/kermit/cketest.ini  Ditto
/var/spool/locks/LCK..*            UUCP lockfiles
#else
.fi
.PP
The following should be in a publicly accessible plain-text documentation area,
such as usr/local/lib/kermit, /usr/local/doc, or /usr/share/lib/kermit, or
available via an information server such as gopher:
.nf
.ta 16
.sp
ckaaaa.hlp      Explanation of C-Kermit files.
ckermit.ini     Standard initialization file.
ckermod.ini	Sample C-Kermit customization file.
ckermit.kdd	Sample dialing directory.
ckermit.ksd	Sample services directory.
ckcker.upd	Supplement to "Using C-Kermit".
ckcker.bwr	C-Kermit "beware" file - hints & tips.
ckuker.bwr	UNIX-specific beware file.
ckedemo.ini     Demonstration macros from "Using C-Kermit".
ckevt.ini       Ditto.
cketest.ini     Ditto.
#endif
.fi
.PP
#ifndef HPUX10
If C-Kermit has not been installed on your system with the system-wide
initialization file feature, then the ckermit.ini file should be copied to
your home (login) directory and renamed to 
.IR ".kermrc" .
You should not modify
this file.
.sp
#endif
To make
.IR "personalized customizations" ,
copy the file
#ifdef HPUX10
/usr/share/lib/kermit/ckermod.ini
#else
ckermod.ini
#endif
file to your home directory, make any desired changes, and rename it to
.IR ".mykermrc" .
.sp
You may also create a personalized
.IR "dialing directory"
like the sample one in
#ifdef HPUX10
/usr/share/lib/kermit/ckermit.kdd.
#else
ckermit.kdd.
#endif
Your personalized dialing directory
should be stored in your home directory as
.IR ".kdd" .
See Chapter 3 of
.IR "Using C-Kermit"
for details.
.sp
And you may also create a personalized
.IR "services directory"
like the sample one in
#ifdef HPUX10
/usr/share/lib/kermit/ckermit.ksd.
#else
ckermit.ksd.
#endif
Your personalized services directory should be stored in your home directory as
.IR ".ksd" .
See Chapter 13 of
.I "Using C-Kermit"
for instructions.
.sp
The demonstration files illustrate C-Kermit's script programming constructs;
they are discussed in chapters 11-13 of the book.  You can run them by typing
the appropriate TAKE command at the C-Kermit> prompt, for example:
"take /usr/share/lib/kermit/cketest.ini".
.PD
.SH AUTHORS
Frank da Cruz, Columbia University, with contributions from hundreds of
other volunteer programmers all over the world.  See Acknowledgements in
.IR "Using C-Kermit".
.SH REFERENCES
.TP
Frank da Cruz and Christine M. Gianone,
.IR "Using C-Kermit",
Digital Press / Butterworth-Heinemann, 225 Wildwood Street,
Woburn, MA 01801, USA (1993).
ISBN 1-55558-108-0.  (In the USA, call +1 800 366-2665 to order Digital Press
books.)  Also available in a German edition from Verlag Heinze Heise,
Hannover.
.sp
.TP
Frank da Cruz,
.IR "Kermit, A File Transfer Protocol",
Digital Press / Butterworth-Heinemann, Woburn, MA, USA (1987).
ISBN 0-932376-88-6.  The Kermit file transfer protocol specification.
.TP
Christine M. Gianone,
.IR "Using MS-DOS Kermit",
Digital Press / Butterworth-Heinemann, Woburn, MA, USA (1992).
ISBN 1-5558-082-3.  Also available in a German edition from Heise, and a
French edition from Heinz Schiefer & Cie, Versailles.
.TP
.IR "Kermit News",
Issues 4 (1990) and 5 (1993), Columbia University,
for detailed discussions of Kermit file transfer performance.
.SH DIAGNOSTICS
The diagnostics produced by
.I C-Kermit
itself are intended to be self-explanatory.  In addition, every command
returns a SUCCESS or FAILURE status that can be tested by IF FAILURE or
IF SUCCESS.  In addition, the program itself returns an exit status code of
0 upon successful operation or nonzero if any of various operations failed.
.SH BUGS
See recent issues of the Info-Kermit digest (on BITNET/EARN and the Internet),
or the comp.protocols.kermit.* newsgroups on Usenet for discussion, or the 
files  ckcker.bwr and ckuker.bwr, for a list of bugs, hints, tips. etc.  Report
bugs via e-mail to kermit@columbia.edu.
Subscribe to Info-Kermit by sending e-mail to LISTSERV@CUVMA.BITNET or
LISTSERV@CUVMA.CC.COLUMBIA.EDU containing the text "subscribe i$kermit"
followed by your name.
.SH CONTACTS
For more information about Kermit software and documentation, write to:
.nf
.sp
  Kermit Development and Distribution
  Columbia University
  612 West 115th Street
  New York, NY  10025-7221
  USA
.sp
.fi
Or send e-mail to kermit@columbia.edu.
Or call +1 212 854-3703.  Or fax +1 212 663-8202.
.br
