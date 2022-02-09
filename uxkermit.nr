.tr ~ \ 
.TH KERMIT 1 local
.SH NAME
kermit \- file transfer, virtual terminal over tty link
.SH SYNOPSIS
.BR "kermit c" [ hlbpe ]
.RI [ line ]
.RI [ baud ]
.RI [ parity ]
.RI [ esc-chr ]
.PP
.BR "kermit r" [ ddziftlbp ]
.RI [ line ]
.RI [ baud ]
.RI [ parity ]
.PP
.BR "kermit s" [ ddziftlbp ]
.RI [ line ]
.RI [ baud ]
.RI [ parity ]
.IR file ~...
.SH DESCRIPTION
.I Kermit
provides reliable file transfer and primitive virtual terminal communication
between machines.  It has been implemented on many different computers,
including microprocessors (see below).  The files transferred may be
arbitrary ASCII data (7-bit characters) and may be of any length.  The file
transfer protocol uses small (96 character) checksummed packets, with
ACK/NACK responses and timeouts.  \fIKermit\fR currently uses a five
second timeout and ten retries.
.PP
The arguments to \fIkermit\fR are a set of flags (no spaces between the
flags), three optional args (which, if included, must be in the same
order as the flags which indicate their presence), and, if this is a
Send operation, a list of one or more files.  (It is similar in some way
to the \fItar\fR command structure).
.PP
.I Kermit
has three modes: Connect, Send, and Receive.  The first is for a virtual
terminal connection, the other two for file transfer.  These modes are
specified by the first flag, which should be
.BR c ,~ s ", or " r ,
respectively.  Exactly one mode must be specified.
.PP
The \fBd\fR flag (debug) makes \fIkermit\fR a bit more verbose.  The
states \fIkermit\fR goes through are printed along with other traces of
it's operation.  A second \fBd\fR flag will cause \fIkermit\fR to give
an even more detailed trace.
.PP
The \fBz\fR flag creates a file called PACKET.LOG in the current
directory. In it is stored a detailed account of the packet transactions
which occur during a Send or Receive operation.
.PP
The \fBi\fR flag (image) allows slightly more efficient file transfer
between Unix machines.  Normally (on Kermits defined to run on Unix
systems) newline is mapped to CRLF on output, CR's are discarded on
input, and bytes are masked to 7 bits.  If this is set, no mapping is
done on newlines, and all eight bits of each byte are sent or received.
This is the default for non-Unix kermits.
.PP
The \fBl\fR flag (line) specifies the tty line that \fIkermit\fR should
use to communicate with the other machine.  This is specified as a
regular filename, like "/dev/ttyh1".  If no \fBl\fR option is specified,
standard input is used and \fIkermit\fR assumes it is running on the
remote host (ie. NOT the machine to which your terminal is attached).
.PP
The \fBb\fR flag (baud) sets the baud rate on the line specified by the
\fBl\fR flag.  No changes are made if the \fBb\fR flag is not used.
Legal speeds are: 110, 150, 300, 1200, 2400, 4800, 9600.  Note that this
version of \fIkermit\fR supports this option on Unix systems only.
.PP
The \fBe\fR flag (escape) allows the user to set the first character of
the two character escape sequence for Connect mode.  When the escape
character (default, '^') is typed, \fIkermit\fR will hold it
and wait for the next
character.  If the next character is \fBc\fR or \fBC\fR, \fIkermit\fR
will close the connection with the remote host.  If the second character
is the same as the escape character, the escape character itself is
passed.  Any character other than these two results in a bell being sent
to the user's terminal and no characters passed to the remote host.  All
other typed characters are passed through unchanged.
.PP
The \fBp\fR flag (parity) allows parity to be set on outgoing packets and
stripped on incoming ones.  This is useful for communicating with IBM
hosts or over networks, such as TELENET, that usurp the parity bit.
The possible values for parity are \fIm\fRark, \fIs\fRpace,
\fIe\fRven, \fIo\fRdd or \fIn\fRone (the default). 
.PP
The \fBt\fR flag (turnaround) tells \fIkermit\fR while in protocol
mode (sending or receiving) to wait for a turnaround character (XON)
from the other host after receiving every packet.  This is necessary
to run \fIkermit\fR with a half duplex host such as an IBM mainframe.
.PP
The \fBh\fR flag (half duplex) makes \fIkermit\fR echo locally any characters
typed in connect mode.  This is also necessary to communicate with a
half duplex system like an IBM 370.
.PP
The file arguments are only meaningful to a Send \fIkermit\fR.  The
Receiving \fIkermit\fR will attempt to store the file with the
same name that was used to send it.  Unix \fIkermit\fRs normally convert
outgoing file names to uppercase and incoming ones to lower case.
The \fBf\fR flag supresses this filename conversion. 
If a filename contains a slash (/) all outgoing
kermits will strip off the leading part of the name through the last
slash. Receiving \fIkermit\fRs, of course, store incoming files in the current
directory.
.SH EXAMPLE
For this example we will assume two Unix machines.  We are logged onto
"unixa" (the local machine), and want to communicate with "unixb" (the
remote machine).  There is a modem on "/dev/tty03".
.PP
We want to connect to "unixb", then transfer "file1" to that machine.
.sp 1
We type:   kermit clb /dev/tty03 1200
.sp 1
Kermit answers:   Kermit: connected...
.sp 1
Now we dial the remote machine and connect the modem.  Anything typed on
the terminal will be sent to the remote machine and any output from that
machine will be displayed on our terminal.  We hit RETURN, get a
"login:" prompt and login.
.PP
Now we need to start a \fIkermit\fR on the remote machine so that we can
send the file over.  First we start up the remote, (in this case
receiving) \fIkermit\fR, then the local, (sending) one.  Remember that
we are talking to unixb right now.
.PP
We type:   kermit r
.br
	(there is now a Receive \fIkermit\fR on unixb)
.PP
We type ^ (the escape character) and then \fBc\fR to kill the local
(Connecting) \fIkermit\fR.
.PP
Kermit answers:   Kermit: disconnected.
.PP
We type:   kermit slb /dev/tty03 1200 file1
.PP
Kermit answers:	Sending file1 as FILE1
.PP
When the transmission is finished, \fIkermit\fR will type either "Send
complete", or "Send failed.", depending on the success of the transfer.
If we now wanted to transfer a file from unixb (remote) to unixa
(local), we would use these commands:
.PP
	kermit clb /dev/tty03 1200
.br
	~~(connected to unixb)
.br
	kermit s file9
.br
	^c (up-arrow c not control-c)
.br
	~~(talking to unixa again)
.br
	kermit rl /dev/tty03 1200
.PP
After all the transfers were done, we should connect again, log off
of unixb, kill the Connect \fIkermit\fR and hang up the phone.
.SH FEATURES
\fIKermit\fR can interact strangely with the tty driver.  In particular,
a tty with "hangup on last close" set (stty hup), will reset to 300 Baud
between \fIkermit\fR commands.  It will also hang up a modem at that
time.  It is better to run with "stty -hup", and use "stty 0" to
explicitly hang up the modem.
.PP
The \fIKERMIT\fR Protocol uses only printing ASCII characters,
Ctrl-A, and CRLF.  Ctrl-S/Ctrl-Q flow control can be used "underneath"
the Kermit protocol (TANDEM line discipline on Berkeley Unix).
.PP
Since BREAK is not an ASCII character, \fIkermit\fR cannot send a BREAK
to the remote machine.  On some systems, a BREAK will be read as a NUL.
(In any case, UNIX \fIkermit\fRs attempt to send a BREAK through the
TIOC?BRK 'ioctl' of the tty driver).
.PP
This \fIkermit\fR does have timeouts when run under Unix, so the
protocol is stable when communicating with "dumb" kermits (that don't
have timeouts).
.SH OTHER IMPLEMENTATIONS
.I Kermits
have been written for TOPS-20, TOPS-10, IBM VM/CMS, Unix, VAX/VMS,
RT-11, MS-DOS, CP/M, and Apple DOS.  The Unix \fIkermit\fR in use
at Ford Aerospace has been tested on v6/PWB, v7, Onyx System III,
Bell System V, Berkeley 4.1 and recently Berkekey 4.2.
.SH OTHER DOCUMENTATION
.PP
Details on other implementations and on the protocol itself is given in
the \fIKermit Users Guide\fR, and the \fIKermit Protocol Handbook\fR.
Two articles explaining \fBkermit\fR's design, specifications and
implementaion can be found in \fIBYTE\fR, June and July, 1984, Vol. 9,
#6 and #7. The authors are Frank da Cruz and Bill Catchings.
.PP
\fIKermit Users Guide\fR, Fourth Edition (4 May 83), Frank da Cruz,
Daphne Tzoar, Bill Catchings
.PP
\fIKermit Protocol Manual\fR, Protocol Version 3 (29 April 83), Frank da Cruz,
Bill Catchings
.PP
Both of the above documents are from the Columbia University Center for
Computing Activities, New York, New York, 10027.
.SH SEE ALSO
stty(1), tty(4)
.SH DIAGNOSTICS
.IP "cannot open \fIdevice\fR"
The file named in the \fIline\fR argument did not exist or had the wrong
permissions (it may be in use by another communications program).
.IP "bad line speed"
The \fIbaud\fR argument was not a legal speed.
.IP "Could not create \fIfile\fR"
A Receive \fIkermit\fR could not create the file being sent to it.
.IP "nothing to connect to"
A Connect \fIkermit\fR was started without a \fIline\fR argument.
.SH BUGS AND CAVEATS
There is no locking on the use of the outgoing line, although the UNIX
tty driver has the facilities to implement this feature. 
Several users
could run \fIkermit\fR (or anything else) on the line simultaneously,
although \fIkermit\fR will not run on a line already in use by another
program that does use the proper locking protocol.
On BSD or V7 systems, the \fIuucp\fR locking system should be imitated. As
a rudimentary form of protection on such systems, creating the file /usr/spool/uucp/LCK..ttyNN,
where NN is the line specified for the \fIl\fR parameter, will prevent
\fIuucp\fR, \fIcu\fR or \fItip\fR from running on that line.
Nothing will prevent another kermit
from running on that line. Don't forget to delete the lock file
after finishing with kermit. Beware also that uucp will respect this lock
file only for a system-dependent period of about 90 minutes.
.PP
This implementation does not send or process error-message packets.
.PP
Repeat-count prefixing and eighth-bit quoting are not implemented.
.SH AUTHORS
.PP
KERMIT kernel by Bill Catchings, Columbia University Center for Computing
Activities
.PP
KERMIT-Unix adaptation by Chris Maio and Bob Cattani, Columbia University
Computer Science Dept.
.PP
Local mods for v6, System III, and System V by Walter Underwood.  Includes
bug fixes from Jim Guyton at RAND-Unix.
.PP
\fIkermit\fR was named after Kermit
the Frog, the star of
\fIThe Muppet Show\fR; the name is used with permission of
Henson Associates Inc.
