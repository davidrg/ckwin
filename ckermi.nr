.TH KERMIT 1C LOCAL 
.SH NAME
kermit \- kermit file transfer
.SH SYNOPSIS
.B kermit
[ option ...] [file ...]
.SH DESCRIPTION
.I Kermit\^
is a file transfer program that allows files to be moved
between many machine of different operating systems and architectures.
This man page describes version 4.2 of the program.
.PP
Arguments are optional. If
.I kermit
is executed without arguments, it will enter command mode. Otherwise,
.I kermit
will read the arguments off the command line and interpret them.
.RE 1
.PP
The following notation is used in command descriptions:
.RE 1
.TP 8
.B fn
A  Unix  file specification, possibly containing the "wildcard"
characters '*' or '?' ('*' matches all character strings, '?'
matches  any single character).
.TP
.B fn1
A Unix file specification which may not contain '*' or '?'.
.TP
.B rfn
A  remote  file  specification in the remote system's own syntax, which
may denote a single file or a group of files.
.TP
.B rfn1
A remote file specification which should denote only a single file.
.TP
.B n
A decimal number between 0 and 94.
.TP
.B c
A decimal number between 0 and 127 representing the value of  an  ASCII
character.
.TP
.B cc
A  decimal  number  between 0 and 31, or else exactly 127, representing
the value of an ASCII control character.
.TP
.B [ ]
Any field in square braces is optional.
.TP
.B {x,y,z}
Alternatives are listed in curly braces.
.RE 1
.P
.I Kermit
command line options may specify  either  actions  or  settings.    If
.I Kermit
is invoked with a command line that specifies no actions, then it will
issue a prompt and begin interactive dialog.   Action  options  specify  either
protocol transactions or terminal connection.
.RE
.SS Command line options

.TP 8
.B \-s fn
Send the specified file or files. If fn contains wildcard (meta)
characters, the Unix shell expands it into a list.  If fn is  '-'  then
.I kermit
sends from standard input, which must come from a file:  

            kermit -s - < foo.bar

or a parallel process:  

            ls -l | kermit -s -
.IP
You  cannot use this mechanism to send terminal typein.  If you want to
send a file whose name is "-" you can precede it with a path  name,  as
in 

            kermit -s ./-
.TP
.B \-r
Receive a file or files.  Wait passively for files to arrive.
.TP
.B \-k
Receive  (passively)  a file or files, sending them to standard output.
This option can be used in several ways:

        kermit -k
.IP
Displays  the  incoming  files  on  your screen; to be used only in
"local mode" (see below).

        kermit -k > fn1
.IP
Sends  the  incoming file or files to the named file, fn1.  If more
than one file arrives,  all  are  concatenated  together  into  the
single file fn1.

        kermit -k | command
.IP
Pipes the incoming data (single or multiple files) to the indicated
command, as in 

        kermit -k | sort > sorted.stuff

.TP
.B \-a fn1
If you have specified a file transfer option, you may specify an
alternate name for a single file with the -a option.  For example, 

        kermit -s foo -a bar
.IP
sends the file foo telling the receiver that its name is bar.  If  more
than  one  file  arrives or is sent, only the first file is affected by
the 
.I \-a
option:  

        kermit -ra baz
.IP
stores the first incoming file under the name baz.
.TP
.B \-x
Begin server operation.  May be used in either local or remote mode.
.RE 1
.PP
Before proceeding, a few words about remote and local operation are  necessary.
.I Kermit
is  "local"  if it is running on a PC or workstation that you are using
directly, or if it is running on a multiuser system and transferring files over
an  external  communication line -- not your job's controlling terminal or
console.
.I Kermit
is remote if it is running on a multiuser system and  transferring
files over its own controlling terminal's communication line, connected to
your PC or workstation.
.PP
If you are running 
.I Kermit
on a PC, it is in local mode by default,  with  the
"back  port"  designated for file transfer and terminal connection.  If you are
running 
.I Kermit
on a multiuser (timesharing) system, it is in remote mode  unless 
you explicitly point it at an external line for file transfer or terminal
connection.  The following command sets 
.I Kermit's
"mode":
.RE 1
.TP 8
.B \-l dev  
Line -- Specify a terminal line to use for file transfer  and  terminal
connection, as in 

            kermit -l /dev/ttyi5
.RE 1
.PP
When  an  external  line is being used, you might also need some additional
options for successful communication with the remote system:
.TP
.B \-b n    
Baud -- Specify the baud rate for the line given in the
.I \-l 
option,  as in 
            kermit -l /dev/ttyi5 -b 9600
.IP
This  option  should  always  be included with the
.I  \-l
option, since the speed of an external line is not necessarily what you expect.
.TP
.B \-p x
Parity -- e,o,m,s,n (even, odd, mark, space, or none).   If  parity  is
other  than none, then the 8th-bit prefixing mechanism will be used for
transferring 8-bit binary data, provided the  opposite 
.I Kermit
agrees. The default parity is none.
.TP
.B \-t
Specifies  half  duplex,  line  turnaround  with  XON  as the handshake
character.
.RE 1
.PP
The following commands may be used only with a 
.I Kermit
which is local  -- either by default or else because the
.I  \-l
option has been specified.
.TP 8
.B \-g rfn
Actively  request  a remote server to send the named file or files; rfn
is a file specification in the remote host's own syntax.  If fn happens
to  contain  any  special  shell  characters,  like  '*', these must be
quoted, as in 

            kermit -g x\\*.\\?
.TP
.B \-f
Send a 'finish' command to a remote server.
.TP
.B \-c
Establish a terminal connection over  the  specified  or  default
communication line, before any protocol transaction takes place.  Get back
to  the  local  system  by  typing  the  escape   character   (normally
Control-Backslash) followed by the letter 'c'.
.TP
.B \-n
Like
.I \-c,
but  after a protocol transaction takes place;
.I \-c
and
.I \-n
may both be used in the same command.  The use of
.I \-n
and
.I \-c
is  illustrated below.
.RE 1
.P
On  a  timesharing  system, the 
.I \-l
and 
.I \-b
options will also have to be included
with the
.I \-r, \-k,
or
.I \-s
options if the other 
.I Kermit
is on a remote system.
.P
If 
.B kermit
is in local mode, the screen (stdout)  is  continously  updated  to
show  the  progress  of the file transer.  A dot is printed for every four data
packets, other packets are shown by type  (e.g.  'S'  for  Send-Init),  'T'  is
printed  when there's a timeout, and '%' for each retransmission.  In addition,
you may type (to stdin) certain "interrupt" commands during file transfer:
.IP
Control-F:  Interrupt the current File, and go on to the next (if any).
.IP
Control-B:  Interrupt the entire Batch of files, terminate the transaction.
.IP
Control-R:  Resend the current packet
.IP
Control-A:  Display a status report for the current transaction.
.RE
.IP
These interrupt characters differ from the ones used in other Kermit
implementations
to  avoid  conflict with Unix shell interrupt characters.  With System
III and System V implementations of Unix, interrupt commands must be  preceeded
by the escape character (e.g. control-\\).
.RE
.P
Several other command-line options are provided:
.TP 8
.B \-i
Specifies that files should be sent or received exactly "as is" with no
conversions.  This option is necessary for transmitting  binary  files.
It may also be used to slightly boost efficiency in Unix-to-Unix transfers
of text files by eliminating CRLF/newline conversion.
.TP
.B \-w
Write-Protect -- Avoid filename collisions for incoming files.
.TP
.B \-q
Quiet -- Suppress screen update during file transfer, for  instance  to
allow a file transfer to proceed in the background.
.TP
.B \-d
Debug -- Record debugging information in the file debug.log in the
current directory.  Use this option if you believe  the  program  is
misbehaving, and show the resulting log to your local
.I  kermit
maintainer.
.TP
.B \-h
Help -- Display a brief synopsis of the command line options.
.RE 1
.P
The command line may contain no more than one protocol action option.
.SS Interactive operation
.RE 1
.P
.B Kermit's
interactive command prompt is "C-Kermit>". In  response  to  this
prompt, you may type any valid command.  
.B Kermit
executes the command and then
prompts you for another command.  The process continues until you instruct  the
program to terminate.
.P
Commands  begin  with a keyword, normally an English verb, such as "send".  You
may omit trailing characters from any keyword, so  long  as  you  specify 
sufficient  characters  to  distinguish  it  from  any other keyword valid in
that 
field.  Certain commonly-used keywords (such as "send",  "receive",  "connect")
have  special  non-unique abbreviations ("s" for "send", "r" for "receive", "c"
for "connect").
.P
Certain characters have special functions in interactive commands:
.P
.TP 8
.B ?
Question mark, typed at any point in a command, will produce a  message
explaining  what  is  possible or expected at that point.  Depending on
the context, the message may be a brief phrase, a menu of keywords,  or
a list of files.
.B ESC
(The  Escape  or  Altmode  key)  --  Request  completion of the current
keyword or filename, or insertion of a default value.  The result  will
be a beep if the requested operation fails.
.TP 8
.B DEL
(The  Delete  or  Rubout key) -- Delete the previous character from the
command.  You may also use BS (Backspace, Control-H) for this function.
.TP
.B ^W
(Control-W) -- Erase the rightmost word from the command line.
.TP
.B ^U
(Control-U) -- Erase the entire command.
.TP
.B ^R
(Control-R) -- Redisplay the current command.
.TP
.B SP
(Space) -- Delimits fields (keywords, filenames, numbers) within a
command.  HT (Horizontal Tab) may also be used for this purpose.
.TP
.B CR
(Carriage  Return)  -- Enters the command for execution.
.B LF
(Linefeed) or
.B FF
(formfeed) may also be used for this purpose.
.TP
.B \e
(Backslash) -- Enter any of the  above  characters  into  the  command,
literally.  To enter a backslash, type two backslashes in a row (\\\\).

.RE 1
.P
You  may  type  the editing characters (DEL, ^W, etc) repeatedly, to delete all
the way back to the prompt.  No action will be performed until the  command  is
entered by typing carriage return, linefeed, or formfeed.  If you make any
mistakes, you will receive an informative error message and a new prompt  --
make
liberal  use  of '?' and ESC to feel your way through the commands.  One 
important command is "help" -- you should use it the first time you run 
.I Kermit.
.P
Interactive 
.I Kermit
accepts commands from files as well as from the  keyboard.
When  you  enter  interactive mode,
.I Kermit
looks for the file .kermrc in your
home or current directory (first it looks in the home directory,  then  in  the
current  one) and executes any commands it finds there.  These commands must be
in interactive format, not Unix command-line format.  A "take" command is  also
provided  for use at any time during an interactive session.  Command files may
be nested to any reasonable depth.
.P
Here is a brief list of 
.I Kermit
interactive commands:
.TP
.B !
Execute a Unix shell command.
.TP
.B bye
Terminate and log out a remote
.I  Kermit
server.
.TP
.B close
Close a log file.
.TP
.B connect
Establish a terminal connection to a remote system.
.TP
.B  cwd
Change Working Directory.
.TP
.B dial
Dial a telephone number.
.TP
.B directory
Display a directory listing.
.TP
.B echo
Display arguments literally.
.TP
.B exit
Exit from the program, closing any open logs.
.TP
.B finish
Instruct a remote 
.I Kermit
server to exit, but not log out.
.TP
.B get
Get files from a remote 
.I Kermit
server.
.TP
.B help
Display a help message for a given command.
.TP
.B log
Open a log file -- debugging, packet, session, transaction.
.TP
.B quit
Same as 'exit'.
.TP
.B receive
Passively wait for files to arrive.
.TP
.B remote
Issue file management commands to a remote
.I Kermit
server.
.TP
.B script
Execute a login script with a remote system.
.TP
.B send
Send files.
.TP
.B server
Begin server operation.
.TP
.B set
Set various parameters.
.TP
.B show
Display values of 'set' parameters.
.TP
.B space
Display current disk space usage.
.TP
.B statistics
Display statistics about most recent transaction.
.TP
.B take 
Execute commands from a file.
.P
The 'set' parameters are:
.TP
.B block-check
Level of packet error detection.
.TP
.B delay
How long to wait before sending first packet.
.TP
.B duplex
Specify which side echoes during 'connect'.
.TP
.B end-of-packet
Terminator for outbound packets.
.TP
.B escape-character
Character to prefix "escape commands" during 'connect'.
.TP
.B file
Set various file parameters.
.TP
.B
flow-control  
Communication line full-duplex flow control.
.TP
.B handshake
Communication line half-duplex turnaround character.
.TP
.B line
Communication line device name.
.TP
.B  modem-dialer
Type of modem-dialer on communication line.
.TP
.B  packet-length
Maximum length for packets.
.TP
.B pad-character
Character to use for inter-packet padding.
.TP
.B padding
How much inter-packet padding to use.
.TP
.B parity
Communication line character parity.
.TP
.B prompt
Change the 
.I Kermit
program's prompt.
.TP
.B speed
Communication line speed.
.TP
.B start-of-packet
Control character to mark beginning of packets.
.TP
.B timeout
Timer interval to detect lost packets.
.P
The 'remote' commands are:
.TP
.B cwd  
Change remote working directory.
.TP
.B delete  
Delete remote files.
.TP
.B directory  
Display a listing of remote file names.
.TP
.B help  
Request help from a remote server.
.TP
.B host  
Issue a command to the remote host in its own command language
.TP
.B  space  
Display current disk space usage on remote system.
.TP
.B type  
Display a remote file on your screen.
.TP
.B who
Display who's logged in, or get information about a user.
.SH FILES
.PD 0
/usr/bin/kermit
.PD
.SH SEE ALSO
Frank da Cruz and Bill Catchings,
.IR "Kermit User's Guide" ,
Columbia University
.I cu(1C), uucp(1C)
.SH DIAGNOSTICS
The diagnostics produced by kermit itself are intended to be
self-explanatory.
.SH BUGS
See recent issues of Info-Kermit for a list of bugs.
