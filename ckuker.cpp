.\" @(#) kermit.1 7.0.196 2000/01/01 Columbia University
#ifdef COMMENT
This man page must be run through the C-Kermit makefile before it can
be used: "make manpage".  Or you can do the following:
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
.TH KERMIT 1 "1 Jan 2000" "HP-UX C-Kermit"
#define DIALOUT /dev/cul0p0
#define HARDWIRE /dev/tty0p0
#else
.TH KERMIT 1C "1 Jan 2000" "UNIX C-Kermit"
#ifdef LINUX
#define DIALOUT /dev/ttyS0
#else
#define DIALOUT /dev/cua
#endif
#define HARDWIRE /dev/tty01
#endif
.SH NAME
kermit \- C-Kermit 7.0 communications software for serial and network
connections: modem dialing, file transfer and management, terminal connection,
character-set translation, numeric and alpha paging, and script programming.
.SH SYNOPSIS
.B kermit
[ command-file ] [ options ... ]
.SH DESCRIPTION
.I Kermit
is a family of file transfer, management, and communication software programs
from the Kermit Project at Columbia University available for most computers
and operating systems.
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
implementations, also TCP/IP connections.  On certain platforms,
C-Kermit can also make X.25 connections.
#endif
#endif
C-Kermit can be thought of as a user-friendly and powerful alternative to cu,
tip, uucp, ftp, telnet, rlogin, expect, and even your shell; a single package
for both network and serial communications, offering automation, convenience,
and language features not found in the other packages, and having a great deal
in common with its cousins, C-Kermit on other UNIX platforms, Kermit 95 for
Windows 95, Windows 98, Windows NT and 2000, and OS/2; MS-DOS Kermit for PCs
with DOS and Windows 3.x, and IBM Mainframe Kermit-370 for VM/CMS, MVS/TSO,
and CICS.  C-Kermit itself also runs on Digital VMS, Data General AOS/VS,
Stratus VOS, OS-9, QNX, Plan 9, the Commodore Amiga, and elsewhere.
Together, C-Kermit, Kermit 95, MS-DOS Kermit, and IBM Mainframe Kermit offer a
consistent and nearly universal approach to inter-computer communications.
.PP
C-Kermit 7.0 is Copyright (C) 1985, 2000 by the Trustees of Columbia
University in the City of New York.  For use and redistribution rights,
see the C-Kermit COPYING.TXT file or give the C-Kermit COPYRIGHT command
(summary: no license is required for own use;
no license is required for distribution with Open Source operating systems;
a license is required for certain other forms of redistribution).
.PP
#ifdef HPUX10
C-Kermit 7.0 is included with HP-UX 10.00 and later by Hewlett-Packard in
partnership with the Kermit Project at Columbia University.
#endif
.PP
C-Kermit 6.0 is thoroughly documented in the book
.IR "Using C-Kermit"
by Frank da Cruz and Christine M. Gianone, Digital Press, Second Edition,
1997; see REFERENCES
at the end of this manual page.  This manual page is not a substitute for the
book.  If you are a serious user of C-Kermit, particularly if plan to write
C-Kermit script programs, you should purchase the manual.  Book sales are the
primary source of funding for the nonprofit Kermit Project.
.PP
Any new features added since the second edition of the book was
published are documented in the online file
.IR "ckermit2.upd"
until such time as the Third Edition of the book is ready.
Hints, tips, limitations, restrictions are listed in
.IR "ckcbwr.txt"
(general C-Kermit) and
.IR "ckuker.bwr"
(UNIX-specific); see FILES below.  Please consult all of these references
before reporting problems or asking for technical support.
.PP
Kermit software is available for hundreds of different computers and operating
systems from Columbia University.  For best file-transfer results, please use
C-Kermit in conjunction with real Columbia University Kermit software on other
computers, such as Kermit 95 for Windows 95 and NT or MS-DOS Kermit for DOS
3.x or Windows.  See CONTACTS below.
.SH "MODES OF OPERATION"
C-Kermit can be used in two "modes": remote and local.  In
.IR "remote mode",
you connect to the
#ifdef HPUX10
HP-UX
#else
UNIX
#endif
system from a desktop computer
and transfer files between your desktop computer and
#ifdef HPUX10
HP-UX
#else
UNIX
#endif
C-Kermit.  In that
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
C-Kermit also has two commands interfaces: the familiar UNIX-style command-line
options, and an interactive dialog with a prompt.
.IR "Command-line options"
give
you access to a small but useful subset of C-Kermit's features for terminal
connection and file transfer, plus the ability to pipe files into or out of
Kermit for transfer.
.PP
.IR "Interactive commands"
give you access to dialing, script programming,
character-set translation, and, in general, detailed control and display, as
well as automation, of all C-Kermit's features.  Interactive commands can also
be collected into command files or macros.  C-Kermit's command and script
language is portable to many and diverse platforms.
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
computer, use the sequence SET MODEM TYPE
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
Kermit program that will be sending them.
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
Kermit or Kermit 95, use Alt-x (hold down the Alt key, press 'x').  Now
you should see your local Kermit program's prompt.
.sp
.ti -0.2i
\(bu	If you will be transferring binary files, give the command SET FILE
TYPE BINARY to the Kermit program that is sending the files.
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
\(bu When the transfer is complete, give a CONNECT command.  Now you are
talking to Kermit on the remote computer again.  Type EXIT to get back to the
command prompt on the remote computer.  When you are finished using the remote
computer, log out and then (if necessary) escape back to Kermit on your local
computer.  Then you can make another connection or EXIT from the local Kermit
program.
.ll +0.5i
.in -0.5i
.fi
.PP
Note that other methods can be used to simplify the file-transfer process:
.IR "client/server operation" ,
in which all commands are given to the client and
passed on automatically to the server, and
.IR autodownload
(and upload), in which the remote Kermit initiates file transfers
automatically through your terminal emulator.
.PP
The file transfer protocol defaults in C-Kermit 7.0, unlike those for earlier
releases, favor speed over robustness, on the assumption that connections in
these times are usually reliable (over TCP/IP and/or error-correcting modems
with hardware flow control).  If you experience file transfer failures, use
the CAUTIOUS or ROBUST commands to choose more conservative (and therefore
slower) protocol settings.  For fine tuning of performance, you can choose
specific packet lengths, window sizes, and control-character prefixing
strategies as explained in Chapter 12 of the manual,
.IR "Using C-Kermit".
.PP
If you are accessing a remote host where C-Kermit resides via Telnet or other
connection that is guaranteed reliable from end to end, and both Kermits
support it (C-Kermit 7.0 does), a new "streaming" form of the Kermit protocol
is used automatically to give ftp-like speeds (the limiting factor being the
overhead from the remote Telnet or Rlogin server and/or PTY driver).
.SH OTHER FEATURES
C-Kermit includes features too numerous to be explained in a man page.  For
further information about connection establishment, modem dialing, networks,
terminal connection, key mapping, logging, file transfer options and features,
troubleshooting, client/server operation, character-set translation during
terminal connection and file transfer, "raw" up- and downloading of files,
macro construction, script programming, sending numberic and alphanumeric
pages, convenience features, and shortcuts, plus numerous tables, examples,
and illustrations, please consult the manual and the C-Kermit 7.0 release
notes.
.SH "GETTING HELP"
.PP
C-Kermit has extensive built-in help.  You can find out what commands exist by
typing ? at the C-Kermit> prompt.  You can type HELP at the C-Kermit> prompt
for a "getting-started" message, or HELP followed by the name
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
introduction to C-Kermit, and the MANUAL command to access this (or another)
man page.  Finally, you can use the SUPPORT command for instructions on
obtaining technical support.
.sp
.SH "ENTERING COMMANDS"
.sp
You can use upper or lower case for interactive-mode commands, but remember
that UNIX filenames are case-sensitive.  You can abbreviate command words
(but not filenames) as long as the abbreviation matches only one possibility.
While typing a command, you can use the following editing characters:
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
otherwise be illegal or misinterpreted.  Other than that, the character
following the \\ identifies what the special quantity is:
.nf
.sp
  % A user-defined simple (scalar) variable such as \\%a or \\%1
  & an array reference such as \\&a[3]
  $ an environment variable such as \\$(TERM)
  v (or V) a built-in variable such as \\v(time)
  f (or F) a function such as \\Fsubstring(\\%a,3,2)
  s (or S) compact substring notation, macronames, like \\s(foo[3:12])
  : compact substring notation, all variables, like \\:(\%a[3:12])
  d (or D) a decimal (base 10) number (1 to 3 digits, 0..255) such as \\d27
  o (or O) an octal (base 8) number (1 to 3 digits, 0..377) such as \\o33
  x (or X) a hexadecimal (base 16) number (2 digits, 00..ff) like \\x1b
  \\ the backslash character itself
  b (or B) the BREAK signal (OUTPUT command only)
  l (or L) a Long BREAK signal (OUTPUT only)
  n (or n) a NUL (0) character (OUTPUT only)
  a decimal digit (a 1-, 2-, or 3-digit decimal number) such as \\27
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
  BACK	Return to previous directory.
  BROWSE	Invoke Web browser.
* CD	Change Directory.
* PWD	Print Working Directory.
  CHECK	See if the given feature is configured.
  CLOSE	Close the connection or a log or other local file.
  COMMENT	Introduce a full-line comment.
  COPYRIGHT	Display copyright notice.
  DATE	Display date & time.
* EXIT	Leave the program, return to UNIX.
* HELP	Display a help message for a given command.
* INTRO	Print a brief introduction to C-Kermit.
  KERMIT	Give command-line options at the prompt.
  LOG	Open a log file -- debugging, packet, session, transaction.
  PUSH	Invoke local system's interactive command interpreter.
  QUIT	Synonym for EXIT.
  REDO	Re-execute a previous command.
  RUN	Run a program or system command.
  SET COMMAND	Command-related parameters: bytesize, recall buffer size.
  SET PROMPT	The C-Kermit program's interactive command prompt.
  SET EXIT	Items related to C-Kermit's action upon exit or SET LINE/HOST.
  SHOW EXIT	Display SET EXIT parameters.
  SHOW FEATURES	Show features that C-Kermit was built with.
  SHOW VERSIONS	Show version numbers of each source module.
  SUPPORT	Find out how to get tech support.
  SUSPEND	Suspend Kermit (use only if shell supports job control!).
* SHOW	Display values of SET parameters.
* TAKE	Execute commands from a file.
  VERSION	Display the C-Kermit program version number.
  Z	Synonym for SUSPEND.
* Ctrl-C	Interrupt a C-Kermit command in progress.
  Ctrl-Z	Synonym for SUSPEND.
  ; or #	Introduce a full-line or trailing comment.
  ! or @	Synonym for RUN.
  <	Synonym for REDIRECT.
.sp
Connection Establishment and Release:
* DIAL	Dial a telephone number.
  PDIAL	Partially dial a telephone number.
* LOOKUP	Lookup a phone number, test dialing rules.
  ANSWER	Wait for a phone call and answer it when it comes.
* HANGUP	Hang up the phone or network connection.
  EIGHTBIT	Shortcut to set all i/o to 8 bits.
  PAD	Command for X.25 PAD (SunOS / Solaris / VOS only).
  PING	Check status of remote TCP/IP host.
  REDIAL	The the most recently DIALed number again.
  LOG CONNECTIONS	Keep a record of each connection.
  REDIRECT	Redirect standard i/o of command to communication connection.
  PIPE	Make a connection through an external command or program.
  SET CARRIER	Treatment of carrier on terminal connections.
* SET DIAL	Parameters related to modem dialing.
* SET FLOW	Communication line flow control: AUTO, RTS/CTS, XON/XOFF, etc.
* SET HOST	Open connection to network host name or address.
#ifdef HPUX10
* SET LINE	Open serial communication device, like /dev/cul0p0.
#else
* SET LINE	Open serial communication device, like /dev/cua0.
#endif
  SET PORT	Synonym for SET LINE.
* SET MODEM TYPE	Specify type of modem on SET LINE device, like USR.
* SET NETWORK	Network type, X.25 (SunOS / Solaris / VOS only) or TCP/IP.
  SET TCP	Specify TCP protocol options (advanced).
  SET TELNET	Specify TELNET protocol options.
  SET X.25	Specify X.25 connection parameters (SunOS/Solaris/VOS only).
  SET PAD	Specify X.25 X.3 PAD parameters (SunOS/Solaris/VOS only).
* SET PARITY	Character parity (none, even, etc) for communications.
* SET SPEED	Serial communication device speed, e.g. 2400, 9600, 57600.
  SET SERIAL    Set serial communications data size, parity, stop bits.
  SET STOP-BITS	Set serial communications stop bits.
  SHOW COMM	Display communications settings.
  SHOW CONN     Display info about current connection.
  SHOW DIAL	Display SET DIAL values.
  SHOW MODEM	Display modem type, signals, etc.
  SHOW NETWORK	Display network-related items.
* TELNET	= SET NETWORK TCP/IP, SET HOST ..., CONNECT.
  RLOGIN	Makes an RLOGIN connection (requires privilege).
  TELOPT	Send a TELNET option negotiation (advanced).
  CLOSE	Close the current connection.
.sp
Terminal Connection:
* C	Special abbreviation for CONNECT.
* CONNECT	Establish a terminal connection to a remote computer.
  LOG SESSION	Record terminal session.
  SET COMMAND	Bytesize between C-Kermit and your keyboard and screen.
* SET DUPLEX	Specify which side echoes during CONNECT.
  SET ESCAPE	Prefix for "escape commands" during CONNECT.
  SET KEY	Key redefinitions in CONNECT mode.
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
  ADD SEND-LIST	Add a file specification to the SEND-LIST
  ADD BINARY-PATTERNS Add a pattern to the binary file pattern list
  ADD TEXT-PATTERNS Add a pattern to the text file pattern list
  ASSOCIATE	A file character-set with a transfer character-set
  LOG SESSION   Download a file with no error checking
* SEND	Send a file or files.
  MSEND	Multiple SEND - accepts a list of files, separated by spaces.
  MOVE	SEND and then delete source file(s) if successful.
  MMOVE	Multiple MOVE - accepts a list of files, separated by spaces.
  MAIL	SEND a file to other Kermit, to be delivered as e-mail.
  RESEND	Continue a incomplete SEND.
  PSEND	Send part of a file.
* RECEIVE	Passively wait for files to arrive from other Kermit.
* R	Special abbreviation for RECEIVE.
* S	Special abbreviation for SEND.
  GET	Ask server to send the specified file(s).
  MGET	Like GET but accepts a list of files.
  REGET	Continue a incomplete download from a server.
  G	Special abbreviation for GET.
  FAST	Shortcut for fast file-transfer settings.
  CAUTIOUS	Shortcut for medium file-transfer settings.
  ROBUST	Shortcut for conservative file-transfer settings.
  SET ATTRIB	Control transmission of file attributes.
* SET BLOCK	Choose error-checking level, 1, 2, or 3.
  SET BUFFERS	Size of send and receive packet buffers.
  SET PREFIX	Which control characters to "unprefix" during file transfer.
  SET DELAY	How long to wait before sending first packet.
  SET DESTINATION DISK, PRINTER, or SCREEN for incoming files.
* SET FILE	Transfer mode (type), character-set, collision action, etc.
* SET RECEIVE	Parameters for inbound packets: packet-length, etc.
  SET REPEAT	Repeat-count compression parameters.
  SET RETRY	Packet retransmission limit.
  SET SEND	Parameters for outbound packets: length, etc.
  SET HANDSHAKE	Communication line half-duplex packet turnaround character.
  SET LANGUAGE	Enable language-specific character-set translations.
  SET PATTERNS	Turn off filename-pattern-based text/binary mode switching.
  SET SESSION-LOG File type for session log, text or binary.
  SET TRANSFER	File transfer parameters: character-set, display, etc.
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
  TRANSMIT	Send a file with no error checking.
  XMIT	Synonym for TRANSMIT.
.sp
SEND Command switches:
  /AS-NAME:	Name to send file under
  /AFTER:	Send files modified after date-time
  /BEFORE:	Send files modified before date-time
  /BINARY	Send in binary mode
  /COMMAND	Send from standard output of a command
  /DELETE	Delete file after successfully sending
  /EXCEPT:	Don't send files whose names match given pattern(s)
  /FILTER:	Pass file contents through given filter program
  /FILENAMES:	Specify how to send filenames
  /LARGER-THAN:	Send files larger than given size
  /LIST:	Send files whose names are listed in given file
  /MAIL:	Send file(s) as e-mail to given address
  /MOVE-TO:	Move source file to given directory after successfully sending
  /NOT-AFTER:	Send files modified not after given date-time
  /NOT-BEFORE:	Send files modified not before given date-time
  /PATHNAMES:	Specifiy how to send pathnames
  /PRINT:	Send files to be printed
  /PROTOCOL:	Send files using given protocol
  /QUIET	Don't display file-transfer progress
  /RECOVER	Recover interrupted transfer from point of failure
  /RECURSIVE	Send a directory tree
  /RENAME-TO:	Rename files as specified after successfully sending
  /SMALLER-THAN:	Send files smaller than given size
  /STARTING-AT:	Send file starting at given byte number
  /SUBJECT:	Subject for SEND /MAIL
  /TEXT	Send in text mode
.sp
GET and RECEIVE Command switches:
  /AS-NAME:	Store incoming file under given name
  /BINARY	Receive in binary mode if transfer mode not specified
  /COMMAND:	Send incoming file data to given command
  /EXCEPT:      Don't accept incoming files whose names match
  /FILENAMES:	How to treat incoming file names
  /FILTER:	Filter program for incoming file data
  /MOVE-TO:	Where to move a file after succussful receipt.
  /PATHNAMES:	How to treat incoming path names
  /PROTOCOL:	Protocol to use for receiving (RECEIVE only)
  /RENAME-TO:   New name for file after succussful receipt.
  /QUIET:	Suppress file-transfer display
  /TEXT	Receive in text mode if transfer mode not specified
.sp
Switches only for GET:
  /DELETE	Tells server to delete each file after successful transmission
  /RECOVER	Resume interrupted file transfer from point of failure
  /RECURSIVE	Tells server to send a directory tree
.sp
File Management:
* CD	Change Directory.
* PWD	Display current working directory.
  COPY	Copy a file.
* DELETE	Delete a file or files.
* DIRECTORY	Display a directory listing.
  EDIT	Edit a file.
  MKDIR	Create a directory.
  PRINT	Print a local file on a local printer.
  PURGE	Remove backup files.
  RENAME	Change the name of a local file.
  RMDIR	Remove a directory.
  SET PRINTER	Choose printer device.
  SPACE	Display current disk space usage.
  SHOW CHARACTER-SETS Display character-set translation info.
  TRANSLATE	Translate a local file's character set.
  TYPE	Display a file on the screen
  TYPE /PAGE	Display a file on the screen, pausing after each screenful.
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
  QUERY	(Same as REMOTE QUERY)
  RETRIEVE	Like GET but server deletes files after.
  REMOTE xxx	Command for server, can be redirected with > or |.
  REMOTE ASSIGN	(RASG) Assign a variable
  REMOTE CD	(RCD) Tell remote Kermit server to change its directory.
  REMOTE COPY	(RCOPY) Tell server to copy a file.
  REMOTE DELETE	(RDEL) Tell server to delete a file.
  REMOTE DIR	(RDIR) Ask server for a directory listing.
  REMOTE EXIT	(REXIT) Ask the server program to exit.
  REMOTE HELP	(RHELP) Ask server to send a help message.
  REMOTE HOST	(RHOST) Ask server to ask its host to execute a command.
  REMOTE KERMIT	(RKER) Send an interactive Kermit command to the server.
  REMOTE LOGIN	Authenticate yourself to a remote Kermit server.
  REMOTE LOGOUT	Log out from a Kermit server previously LOGIN'd to.
  REMOTE MKDIR	(RMKDIR) Tell the server to create a directory.
  REMOTE PRINT	(RPRINT) Print a local file on the server's printer.
  REMOTE PWD	(RPWD) Ask server to reveal its current (working) directory.
  REMOTE QUERY	(RQUERY) Get value of a variable.
  REMOTE RENAME	(RRENAME) Tell server to rename a file.
  REMOTE RMDIR	(RRMDIR) Tell server to remove a directory.
  REMOTE SET	(RSET) Send a SET command to a remote server.
  REMOTE SPACE	(RSPACE) Ask server how much disk space it has left.
  REMOTE TYPE	(RTYPE) Ask server to display a file on your screen.
  REMOTE WHO	(RWHO) Ask server for a "who" or "finger" listing.
  SERVER	Enter server mode - be a Kermit server.
  SET SERVER	Set parameters for server operation.
  SHOW SERVER	Show SET SERVER, ENABLE/DISABLE items.
.sp
Script programming:
  ASK	Prompt the user, store user's reply in a variable.
  ASKQ	Like ASK, but, but doesn't echo (useful for passwords).
  ASSERT	Evaluate condition and set SUCCESS/FAILURE accordingly.
  ASSIGN	Assign an evaluated string to a variable or macro.
  CLEAR	Clear communication device input buffer or other item.
  CLOSE	Close the connection, or a log or other file.
  DECLARE	Declare an array.
  DECREMENT	Subtract one (or other number) from a variable.
  DEFINE	Define a variable or macro.
  DO	Execute a macro ("DO" can be omitted).
  ECHO	Display text on the screen.
  ELSE	Used with IF.
  END	A command file or macro.
  EVALUATE	an arithmetic expression.
  FAIL	Set FAILURE.
  FOPEN	Open a local file
  FREAD	Read from a file opened with FOPEN
  FWRITE	Write to an FOPEN'd file
  FSEEK	Seeks to given position in FOPEN'd file
  FCLOSE	Close an FOPEN'd file
  FOR	Execute commands repeatedly in a counted loop.
  FORWARD	GOTO in the forward direction only.
  GETC	Issue a prompt, get one character from keyboard.
  GETOK	Ask question, get Yes or No answer, set SUCCESS or FAILURE.
  GOTO	Go to a labeled command in a command file or macro.
  IF	Conditionally execute the following command.
  INCREMENT	Add one (or other number) to a variable.
  INPUT	Match characters from another computer against a given text.
  LOCAL	Declare local variables in a macro
  MINPUT	Like INPUT, but allows several match strings.
  MSLEEP	Sleep for given number of milliseconds.
  OPEN	Open a local file for reading or writing.
  OUTPUT	Send text to another computer.
  O	Special abbreviation for OUTPUT.
  PAUSE	Do nothing for a given number of seconds.
  READ	Read a line from a local file into a variable.
  REINPUT	Reexamine text previously received from another computer.
  RETURN	Return from a user-defined function.
  SCREEN	Screen operations - clear, position cursor, etc.
  SCRIPT	Execute a UUCP-style login script.
  SET ALARM	Set a timer to be used with IF ALARM; SHOW ALARM shows it.
  SET CASE	Treatment of alphabetic case in string comparisons.
  SET COMMAND	QUOTING turns on/off interpretation of backslash notation.
  SET COUNT	For counted loops.
  SET INPUT	Control behavior of INPUT command.
  SET MACRO	Control aspects of macro execution.
  SET TAKE	Control aspects of TAKE file execution.
  SHIFT	Shift macro arguments left the given number of places.
  SHOW ARGUMENTS Display arguments to current macro.
  SHOW ARRAYS	Display information about active arrays.
  SHOW COUNT	Display current COUNT value.
  SHOW FUNCTIONS List names of available \\f() functions.
  SHOW GLOBALS	List defined global variables \\%a..\\%z.
  SHOW MACROS	List one or more macro definitions.
  SHOW SCRIPTS	Show script-related settings.
  SHOW VARIABLES Display values all \\v() variables.
  SLEEP	Sleep for given number of seconds.
  SORT	Sort an array (many options).
  STATUS	Show SUCCESS or FAILURE of previous command.
  STOP	Stop executing macro or command file, return to prompt.
  SUCCEED	Set SUCCESS.
  SWITCH	Execute selected command(s) based on value of variable.
  TAKE	Execute commands from a file.
  UNDEFINE	Undefine a variable
  WAIT	Wait for the specified modem signals.
  WHILE	Execute commands repeatedly while a condition is true.
  WRITE	Write material to a local file.
  WRITE-LINE	Write a line (record) to a local file.
  WRITELN	Synonym for WRITE-LINE.
  XECHO	Like ECHO but no CRLF at end.
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
  \\v(blockcheck)current SET BLOCK-CHECK type
  \\v(browser)   current Web browser
  \\v(browsopts) current Web browser options
  \\v(browsurl)  most recent Web browser site (URL)
  \\v(byteorder) hardware byte order
  \\v(charset)   current file character-set
  \\v(cmdbufsize)size of command buffer
  \\v(cmdfile)   name of current command file, if any
  \\v(cmdlevel)  current command level
  \\v(cmdsource) where command are currently coming from, macro, file, etc.
  \\v(cols)      number of screen columns
  \\v(connection)connection type: serial, tcp/ip, etc.
  \\v(count)     current COUNT value
  \\v(cps)       speed of most recent file transfer in chars per second
  \\v(cpu)       CPU type C-Kermit was built for
  \\v(crc16)     16-bit CRC of most recent file transfer
  \\v(ctty)      device name of controlling terminal
  \\v(d$ac)      SET DIAL AREA-CODE value
  \\v(d$cc)      SET DIAL COUNTRY-CODE value
  \\v(d$ip)      SET DIAL INTL-PREFIX value
  \\v(d$lc)      SET DIAL LD-PREFIX value
  \\v(d$px)      SET DIAL PBX-EXCHANGE value
  \\v(date)      date as 8 Feb 1999
  \\v(day)       day of week (English 3-letter abbreviation)
  \\v(dialcount) current value of DIAL retry counter
  \\v(dialnumber)phone number most recently dialed
  \\v(dialresult)most recent dial result message or code from modem
  \\v(dialstatus)return code from DIAL command (0 = OK, 22 = BUSY, etc)
  \\v(dialsuffix)current SET DIAL SUFFIX value
  \\v(dialtype)  code for type of call most recently placed
  \\v(directory) current/default directory
  \\v(download)  current download directory if any
  \\v(editor)    your preferred editor
  \\v(editfile)  file most recently edited
  \\v(editopts)  options for editor
  \\v(errno)     current "errno" (system error number) value
  \\v(errstring) error message string associated with errno
  \\v(escape)    decimal ASCII value of CONNECT-mode escape character
  \\v(evaluate)  result of most recent EVALUATE command
  \\v(exitstatus)current EXIT status (0 = good, nonzero = something failed)
  \\v(filename)  name of file currently being transferred
  \\v(filenumber)number of file currently being transferred (1 = first, etc)
  \\v(filespec)  filespec given in most recent SEND/RECEIVE/GET command
  \\v(fsize)     size of file most recently transferred
  \\v(ftype)     SET FILE TYPE value (text, binary)
  \\v(herald)    C-Kermit's program herald
  \\v(home)      home directory
  \\v(host)      computer host name (comuter where C-Kermit is running)
  \\v(hwparity)  SET PARITY HARDWARE setting (if any)
  \\v(input)     current INPUT buffer contents
  \\v(inchar)    character most recently INPUT
  \\v(incount)   how many characters arrived during last INPUT
  \\v(inidir)    directory where initialization file was found
  \\v(inmatch)   [M]INPUT material that matched given \\fpattern().
  \\v(instatus)  status of most recent INPUT command
  \\v(intime)    how long it took most recent INPUT to succeed (msec)
  \\v(inwait)    most recent [M]INPUT time limit
  \\v(ipaddress) IP address of C-Kermit's computer if known
  \\v(kbchar)    keyboard character that interrupted PAUSE, INPUT, etc.
  \\v(line)      current communications device, set by LINE or HOST
  \\v(local)     0 if in remote mode, 1 if in local mode
  \\v(lockdir)   UUCP lockfile directory on this platform
  \\v(lockpid)   Process ID found in lockfile when port is in use
  \\v(maclevel)  Current macro stack level
  \\v(macro)     name of currently executing macro, if any
  \\v(math_e)    Floating-point constant e
  \\v(math_pi)   Floating-point constant pi
  \\v(math_precision) Floating point number precision (digits)
  \\v(minput)    Result of most recent MINPUT command
  \\v(model)     Computer hardware model if known
  \\v(modem)     Current modem type or "none"
  \\v(m_aa_off)  Modem command to turn autoanswer off
  \\v(m_aa_on)   Modem command to turn autoanswer on
  \\v(m_xxxxx)   (many other modem commands)
  \\v(m_sig_xx)  Value of modem signal xx
  \\v(name)      Name by which C-Kermit was called (kermit, wermit, etc)
  \\v(ndate)     Current date as 19930208 (yyyymmdd)
  \\v(nday)      Numeric day of week (0 = Sunday)
  \\v(newline)   System-independent newline character or sequence
  \\v(ntime)     Current local time in seconds since midnight (noon = 43200)
  \\v(osname)    Operating System name
  \\v(osrelease) Operating System release
  \\v(osversion) Operating System version
  \\v(packetlen) Current SET RECEIVE PACKET-LENGTH value
  \\v(parity)    Current parity setting
  \\v(pexitstat) Exit status of most recently forked process
  \\v(pid)       C-Kermit's process ID
  \\v(platform)  Specific machine and/or operating system
  \\v(printer)   Current SET PRINTER value or "(default)"
  \\v(program)   Name of this program ("C-Kermit")
  \\v(protocol)  Currently selected file transfer protocol
  \\v(p_8bit)    Current 8th-bit prefix (Kermit protocol)
  \\v(p_ctl)     Current control-character prefix (Kermit protocol)
  \\v(p_rpt)     Current repeat-count prefix (Kermit protocol)
  \\v(query)     Result of most recent REMOTE QUERY command
  \\v(return)    Most recent RETURN value
  \\v(rows)      Number of rows on the terminal screen
  \\v(sendlist)  Number of entries in SEND-LIST
  \\v(serial)    Serial port settings in 8N1 format
  \\v(speed)     Current serial device speed, if known, or "unknown"
  \\v(startup)   Current directory when C-Kermit was started
  \\v(status)    0 or 1 (SUCCESS or FAILURE of previous command)
  \\v(sysid)     Code for platform ID of C-Kermit's computer (U1=UNIX)
  \\v(system)    UNIX (name of operating system family)
  \\v(terminal)  Terminal type
  \\v(test)      C-Kermit test version, if any (e.g. Beta.10)
  \\v(textdir)   Where C-Kermit thinks its text files are
  \\v(tfsize)    Total size of file group most recently transferred
  \\v(tftime)    Transfer time of most recent file group
  \\v(time)      Time as 13:45:23 (hh:mm:ss, 24-hour format)
  \\v(tmpdir)    Temporary directory
  \\v(trigger)   Most recent string to trigger return from CONNECT
  \\v(ttyfd)     File descriptor of current communication device
  \\v(ty_xx)     Used internally by TYPE
  \\v(userid)    User ID of person running C-Kermit
  \\v(version)   Numeric version of Kermit, e.g. 501190.
  \\v(window)    Current window size (SET WINDOW value)
  \\v(xferstatus)Status of most recent file transfer
  \\v(xfermsg)   Error message, if any, terminating most recent transfer
  \\v(xfer_xxx)  Various statistics from last file transfer.
  \\v(xprogram)  C-Kermit
  \\v(xversion)  Same as \\v(version)
.fi
.SH "BUILT-IN FUNCTIONS"
Builtin functions are invoked as \\Fname(args), can be used in any command,
and are usually used in script programs.  Type SHOW FUNCTIONS for a current
list.  Type "help function <name>" for a description of the arguments and
return value, e.g. "help function basename".
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
  -O	like -x but exits after one transaction
  -f	finish remote server
  -g files	get remote files from server (quote wildcards)
  -G files	like -g but sends file to standard output
  -a name	alternate file name, used with -s, -r, -g
  -c	connect (before file transfer), used with -l or -j
  -n	connect (after file transfer), used with -l or -j
.sp
Settings:
  -l line	communication line device (to make a serial connection)
  -l n	open file descriptor of communication device
  -j host	TCP/IP network host name (to make a network connection)
  -J host	Connect like TELNET, exit when connection closes
  -l n	open file descriptor of TCP/IP connection (n = number)
  -X	X.25 network address
  -Z	open file descriptor of X.25 connection
  -o n	X.25 closed user group call info
  -u	X.25 reverse-charge call
  -q	quiet during file transfer
  -I    connection is reliable (e.g. TCP or X.25)
  -8	connection is 8-bit clean
  -0    100% transparency in CONNECT mode (and no escaping back)
  -i	transfer files in binary mode
  -T	transfer files in text mode
  -P    send/accept literal path (file) names
  -V	= SET FILE PATTERNS OFF and SET XFER MODE MANUAL
  -b bps	serial line speed, e.g. 1200
  -m name	modem type, e.g. hayes
  -p x	parity, x = e,o,m,s, or n
  -t	half duplex, xon handshake
  -e n	receive packet-length
  -v n	window size
  -L    used with -s to select recursive directory transfer
  -Q	Quick file-transfer settings
  -w	write over files of same name, don't backup old file
  -D n	delay n seconds before sending a file
  -V    "manual mode" = SET FILE PATTERNS OFF, SET TRANSFER MODE MANUAL.
.sp
Other:
  -y name	alternate init file name
  -Y	Skip init file
  -R	Advise C-Kermit it will be used only in remote mode
  -d	log debug info to file debug.log
  -S	Stay, don't exit, after action command
  -C "cmds"	Interactive-mode commands, comma-separated
  -z	Force foreground operation
  -B	Force background (batch) operation
  -h	print command-line option help screen
  =	Ignore all text that follows but assign them to \\%1..\\%9
  --    Same as =
.sp
.fi
.SH "COMMAND LINE EXAMPLES"
.PP
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
makes a 19200-bps direct connection out through HARDWIRE, CONNECTs (-c)
so you can log in and, presumably start a remote Kermit program and tell it to
send a file, then it RECEIVEs the file (-r), then it CONNECTs back (-n) so you
can finish up and log out.
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
/usr/share/lib/kermit/COPYING.TXT  Copyright, permissions, disclaimer
/usr/share/lib/kermit/ckermit.ini  System-wide initialization file
/usr/share/lib/kermit/ckermod.ini  Sample customization file
/usr/share/lib/kermit/ckermit.kdd  Sample dialing directory
/usr/share/lib/kermit/ckermit.ksd  Sample services directory
/usr/share/lib/kermit/ckermit2.txt Updates to "Using C-Kermit" 2nd Ed
/usr/share/lib/kermit/ckcbwr.txt   C-Kermit "beware" file - hints & tips
/usr/share/lib/kermit/ckubwr.txt   UNIX-specific beware file
/usr/share/lib/kermit/ck*.txt      Other plain-text documentation
/usr/share/lib/kermit/ckedemo.ksc  Macros from "Using C-Kermit"
/usr/share/lib/kermit/ckevt.ksc    Ditto
/usr/share/lib/kermit/ckepage.ksc  Alpha pager script
/var/spool/locks/LCK..*            UUCP lockfiles
#else
.fi
.PP
The following should be in a publicly accessible plain-text documentation area,
such as usr/local/lib/kermit, /usr/local/doc, or /usr/share/lib/kermit, or
available via an information server such as gopher, or by ftp from Columbia:
.nf
.ta 16
.sp
COPYING.TXT     Copyright, permissions, disclaimer.
ckaaaa.hlp      Explanation of C-Kermit files.
ckermit.ini     Standard initialization file.
ckermod.ini	Sample C-Kermit customization file.
ckermit.kdd	Sample dialing directory.
ckermit.ksd	Sample services directory.
ckermit2.txt	Supplement to "Using C-Kermit" 2nd Ed.
ckcbwr.txt	C-Kermit "beware" file - hints & tips.
ckubwr.txt	UNIX-specific beware file.
ck*.txt		Other plain-text documentation.
ckedemo.ksc     Demonstration macros from "Using C-Kermit".
ckevt.ksc       Ditto.
ckepage.ksc     Alpha pager script.
#endif
.fi
.PP
#ifndef HPUX10
If C-Kermit has not been installed on your system with the system-wide
initialization file feature, then the ckermit.ini file should be copied to
your home (login) directory and renamed to
.IR ".kermrc" .
You should not modify this file.
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
.IR ".kdd"
and your personal network directory as
.IR ".knd" .
See Chapters 5 and 6 of
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
See Chapter 7 of
.I "Using C-Kermit"
for instructions.
.sp
The demonstration files illustrate C-Kermit's script programming constructs;
they are discussed in chapters 17-19 of the book.  You can run them by typing
the appropriate TAKE command at the C-Kermit> prompt, for example:
"take /usr/share/lib/kermit/ckedemo.ini".
.PD
.SH AUTHORS
Frank da Cruz, Columbia University, with contributions from hundreds of
other volunteer programmers all over the world.  See Acknowledgements in
.IR "Using C-Kermit".
.SH REFERENCES
.TP
Frank da Cruz and Christine M. Gianone,
.IR "Using C-Kermit",
Second Edition, 1997, 622 pages,
Digital Press / Butterworth-Heinemann, 313 Washington Street,
Newton, MA 02158-1626, USA.
ISBN 1-55558-164-1.  (In the USA, call +1 800 366-2665 to order Digital Press
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
See the comp.protocols.kermit.* newsgroups on Usenet for discussion, or the
files ckcbwr.txt and ckubwr.txt, for a list of bugs, hints, tips. etc.  Report
bugs via e-mail to kermit-support@columbia.edu.  Visit
http://www.columbia.edu/kermit/support.html for details about tech support.
.SH CONTACTS
For more information about Kermit software and documentation, visit the
Kermit Web site:
.nf
.sp
  http://www.columbia.edu/kermit/
.sp
.fi
Or write to:
.nf
.sp
  The Kermit Project
  Columbia University
  612 West 115th Street
  New York NY  10025-7799
  USA
.sp
.fi
Or send e-mail to kermit@columbia.edu.
Or call +1 212 854-3703.  Or fax +1 212 663-8202.
.br
