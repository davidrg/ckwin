@comment[	-*-Text-*-	]
@Part(CKUNIX,root="KER:KUSER")
@string(-ckversion="@q<4.2>")
@define(exx=example,above 2,below 1)
@Chapter<UNIX KERMIT>

@Begin<Description,Leftmargin +12,Indent -12,spread 0>
@i(Program:)@\Frank da Cruz, Bill Catchings, Jeff Damens, Columbia
University; Herm Fischer, Litton Data Systems, Van Nuys CA; contributions by
many others.

@i(Language:)@\C

@i(Documentation:)@\Frank da Cruz, Columbia University; Herm
Fischer, Litton Data Systems, Van Nuys CA.

@i(Version:)@\@value(-ckversion)

@i(Date: )@\March 1985
@end<Description>
@Center(D R A F T)

@begin(quotation)
NOTE -- This source for this
documentation is written as input for the Scribe text formatter.
It needs to become input for two text formatters - Scribe and Nroff, the
former for inclusion in the Kermit User Guide, and the latter for
Unix man pages.  This might be done by defining formatting macros
in M4, which generate the appropriate Scribe and Nroff commands for
sectioning, itemization, description, etc.  (Volunteers?)
@end(quotation)

C-Kermit is a completely new implementation of Kermit, written modularly and
transportably in C.  The protocol state transition table is written in
@i'wart', a (not proprietary) lex-like preprocessor for C.  System-dependent
primitive functions are isolated into separately compiled modules so that the
program should be easily portable among Unix systems and also to non-Unix
systems that have C compilers.

@subheading<Unix Kermit Capabilities At A Glance:>
@begin<format,leftmargin +2,above 1,below 1>
@tabclear()@tabset(3.5inches,4.0inches)
Local operation:@\Yes
Local operation:@\Yes
Remote operation:@\Yes
Login scripts:@\Yes
Transfer text files:@\Yes
Transfer binary files:@\Yes
Wildcard send:@\Yes
File transfer interruption:@\Yes
Filename collision avoidance:@\Yes
Can time out:@\Yes
8th-bit prefixing:@\Yes
Repeat count prefixing:@\Yes
Alternate block checks:@\Yes
Terminal emulation:@\Yes
Communication settings:@\Yes
Transmit BREAK:@\Yes
Support for dialout modems:@\Yes
IBM mainframe communication:@\Yes
Transaction logging:@\Yes
Session logging:@\Yes
Debug logging:@\Yes
Packet logging:@\Yes
Act as server:@\Yes
Talk to server:@\Yes
Advanced server functions:@\Yes
Local file management:@\Yes
Command/Init files:@\Yes
UUCP and multiuser line locking:@\Yes
File attributes:@\No
Command macros:@\No
Raw file transmit:@\No
@end<format>

@index(C-Kermit)@index(Unix Kermit)
C-Kermit provides traditional Unix command line operation as well as
interactive command prompting and execution.  The command line options
provide access to a minimal subset of C-Kermit's capabilities; the
interactive command set is far richer.

On systems with dialout modems, C-Kermit can use command files and login
scripts to essentially duplicate the file transfer functionality of uucp among
heterogeneous operating systems, including by the use of scheduled (e.g. late
night) unattended operation.

@section(The Unix File System)

Consult your Unix manual for details about the file system under your
version of Unix.  For the purposes of Kermit, several things are worth
briefly noting.  Unix files generally have lowercase names.  Unix
directories are tree-@|structured.  Directory levels are separated by
"@q(/)" characters.  For example,
@example(/usr/foo/bar)
denotes the file @q(bar) in the directory @q(/usr/foo).  Wildcard or
"meta" characters allow groups of files to be specified.  "@q(*)"
matches any string; "@q(?)" matches any single character.

When C-Kermit is invoked with files specified on the Unix command line,
the Unix shell (Bourne Shell, C-Shell, etc) expands meta characters, and
in this case, a wider variety is available.  For example,
@example(kermit -s ~/ck[x-z]*.[ch])
is expanded by the Berkeley C-Shell into a list of all the files in the
user's home directory (@q[~/]) that start with the characters "@q(ck)",
followed by a single character @q(x), @q(y), or @q(z), followed by zero
or more characters, followed by a dot, followed by one of the characters
@q(c) or @q(h).  Internally, the C-Kermit program itself expands only
the @q("*") and @q("?") meta characters.

Unix files are linear streams of 8-bit bytes.  Text files consist of
7-bit ASCII characters, with the high bit off (0), and lines separated by
the Unix newline character, which is linefeed (LF, ASCII 10).  This
distinguishes Unix text files from those on most other ASCII systems, in
which lines are separated by a carriage-@|return linefeed sequence
(CRLF, ASCII 13 followed by ASCII 10).  Binary files are likely to contain
data in the high bits of the file bytes, and are not treated in terms of
lines.

When transferring files, C-Kermit will convert between upper and lower
case filenames and between LF and CRLF line terminators automatically,
unless told to do otherwise.  When binary files must be transferred, the
program must be instructed not to perform LF/CRLF conversion (@q[-i] on the
command line or "set file type" interactively; see below).

@section(Command Line Operation)

The C-Kermit command line syntax has been changed from that of earlier
releases of Unix Kermit to conform to the "Proposed Syntax Standards for
Unix System Commands" put forth by Kathy Hemenway and Helene Armitage of
AT&T Bell Laboratories in @ux(Unix/World), Vol.1, No.3, 1984.  The rules that
apply are:

@begin(itemize,spread 0)
Command names must be between 2 and 9 characters ("kermit" is 6).

Command names must include lower case letters and digits only.

An option name is a single character.

Options are delimited by '@q(-)'.

Options with no arguments may be grouped (bundled) behind one delimiter.

Option-arguments cannot be optional.

Arguments immediately follow options, separated by whitespace.

The order of options does not matter.

'@q(-)' preceded and followed by whitespace means standard input.
@end(itemize)
A group of bundled options may end with an option that has an argument.

The following notation is used in command descriptions:
@begin(description,leftmargin +8,indent -8)
@i(fn)@\A Unix file specification, possibly containing the "wildcard"
characters '@q[*]' or '@q[?]' ('@q[*]' matches all character strings, '@q[?]'
matches any single character).

@i(fn1)@\A Unix file specification which may not contain '@q[*]' or '@q[?]'.

@i(rfn)@\A remote file specification in the remote system's own syntax, which
may denote a single file or a group of files.

@i(rfn1)@\A remote file specification which should denote only a single file.

@i(n)@\A decimal number between 0 and 94.

@i(c)@\A decimal number between 0 and 127 representing the value of an
ASCII character.

@i(cc)@\A decimal number between 0 and 31, or else exactly 127,
representing the value of an ASCII control character.

@q([ ])@\Any field in square braces is optional.

@q({x,y,z})@\Alternatives are listed in curly braces.
@end(description)

C-Kermit command line options may specify either actions or settings.  If
C-Kermit is invoked with a command line that specifies no actions, then it
will issue a prompt and begin interactive dialog.
Action options specify either protocol transactions or terminal connection.

@begin<description,leftmargin +8,indent -8>
@q(-s )@i(fn)@\Send the specified file or files.  If @i(fn) contains
wildcard (meta) characters, the Unix shell expands it into a list.  If @i(fn)
is '@q[-]' then kermit sends from standard input, which must
come from a file:
@example(kermit -s - < foo.bar)
or a parallel process:
@example(ls -l | kermit -s -)
You cannot use this mechanism to send
terminal typein.  If you want to send a file whose name is "-"
you can precede it with a path name, as in
@example(kermit -s ./-)

@q(-r)@\Receive a file or files.  Wait passively for files to arrive.

@q(-k)@\Receive (passively) a file or files, sending them to standard
output.  This option can be used in several ways:
@begin(description,leftmargin +4,indent -4)
@q(kermit -k)@\Displays the incoming files on your screen; to be used only
in "local mode" (see below).

@q(kermit -k > )@i(fn1)@\Sends the incoming file or files to the named file,
@i(fn1).  If more than one file arrives, all are concatenated together
into the single file @i(fn1).

@q(kermit -k | command)@\Pipes the incoming data (single or multiple
files) to the indicated command, as in
@example'kermit -k | sort > sorted.stuff'
@end(description)

@q(-a )@i(fn1)@\If you have specified a file transfer option, you may specify
an alternate name for a single file with the @q(-a) option.  For example,
@example'kermit -s foo -a bar'
sends the file @q(foo) telling the receiver that its name is @q(bar).
If more than one file arrives or is sent, only the first file is
affected by the @q(-a) option:
@example'kermit -ra baz'
stores the first incoming file under the name @q(baz).

@q(-x)@\Begin server operation.  May be used in either local or remote mode.
@end(description)

Before proceeding, a few words about remote and local operation are
necessary.  C-Kermit is "local" if it is running on PC or workstation that
you are using directly, or if it is running on a multiuser system and
transferring files over an external communication line -- not your job's
controlling terminal or console.  C-Kermit is remote if it is running on a
multiuser system and transferring files over its own controlling terminal's
communication line, connected to your PC or workstation.

If you are running C-Kermit on a PC, it is in local mode by default,
with the "back port" designated for file transfer and terminal connection.
If you are running C-Kermit on a multiuser (timesharing) system, it is
in remote mode unless you explicitly point it at an external line for
file transfer or terminal connection.  The following command sets
C-Kermit's "mode":

@begin(description,leftmargin +8,indent -8)
@q(-l )@i(dev)@\Line  -- Specify a terminal line to use for file
transfer and terminal connection, as in
@example'kermit -l /dev/ttyi5'
@end(description)

When an external line is being used, you might also need some additional
options for successful communication with the remote system:

@begin(description,leftmargin +8,indent -8)
@q(-b )@i(n)@\Baud  -- Specify the baud rate for the line given in the
@q(-l) option, as in
@example'kermit -l /dev/ttyi5 -b 9600'
This option should always be included with the @q(-l) option, since the
speed of an external line is not necessarily what you expect.

@q(-p )@i(x)@\Parity -- e,o,m,s,n (even, odd, mark, space, or none).  If parity
is other than none, then the 8th-bit prefixing mechanism will be
used for transferring 8-bit binary data, provided the opposite
Kermit agrees.  The default parity is none.

@q(-t)@\Specifies half duplex, line turnaround with XON as the handshake
character.
@end(description)

The following commands may be used only with a C-Kermit which is local --
either by default or else because the -l option has been specified.

@begin(description,leftmargin +8,indent -8)
@q(-g )@i(rfn)@\Actively request a remote server to send the named file
or files; @i(rfn) is a file specification in the remote host's own syntax.  If
@i(fn) happens to contain any special shell characters, like '@q(*)',
these must be quoted, as in
@example'kermit -g x\*.\?'

@q(-f)@\Send a 'finish' command to a remote server.

@q(-c)@\Establish a terminal connection over the specified or default
communication line, before any protocol transaction takes place.
Get back to the local system by typing the escape character
(normally Control-Backslash) followed by the letter 'c'.

@q(-n)@\Like @q(-c), but after a protocol transaction takes place;
@q(-c) and @q(-n) may both be used in the same command.  The use of @q(-n)
and @q(-c) is illustrated below.
@end(description)
On a timesharing system, the @q(-l) and @q(-b) options will also have to
be included with the @q(-r), @q(-k), or @q(-s) options if the other
Kermit is on a remote system.

If C-Kermit is in local mode, the screen (stdout) is continously updated to
show the progress of the file transer.  A dot is printed for every four data
packets, other packets are shown by type (e.g. '@q(S)' for Send-Init),
'@q(T)' is printed when there's a timeout, and '@q(%)' for each
retransmission.  In addition, you may type (to stdin) certain "interrupt"
commands during file transfer:
@begin(description,leftmargin +16,indent -12,spread 0)
Control-F:@\Interrupt the current File, and go on to the next (if any).

Control-B:@\Interrupt the entire Batch of files, terminate the transaction.

Control-R:@\Resend the current packet

Control-A:@\Display a status report for the current transaction.
@end(description)
These interrupt characters differ from the ones used in other Kermit
implementations to avoid conflict with Unix shell interrupt characters.
With System III and System V implementations of Unix, interrupt commands
must be preceeded by the escape character (e.g. control-@q[\]).

Several other command-line options are provided:

@begin(description,leftmargin +8,indent -8)
@q(-i)@\Specifies that files should be sent or received exactly "as is"
with no conversions.  This option is necessary for transmitting
binary files.  It may also be used to slightly boost efficiency
in Unix-to-Unix transfers of text files by eliminating CRLF/newline
conversion.

@q(-w)@\Write-Protect -- Avoid filename collisions for incoming files.

@q(-q)@\Quiet -- Suppress screen update during file transfer, for instance
to allow a file transfer to proceed in the background.

@q(-d)@\Debug -- Record debugging information in the file @q(debug.log) in 
the current directory.  Use this option if you believe the program
is misbehaving, and show the resulting log to your local
kermit maintainer.

@q(-h)@\Help -- Display a brief synopsis of the command line options.
@end(description)
The command line may contain no more than one protocol action option.

Files are sent with their own names, except that lowercase letters are
raised to upper, pathnames are stripped off, tilde ('@q[~]') characters
changed to '@q(X)', and if the file name begins with a period, an
'@q(X)' is inserted before it.  Incoming files are stored under their
own names except that uppercase letters are lowered, and, if @q(-w) was
specified, a "generation number" is appended to the name if it has the
same name as an existing file which would otherwise be overwritten.  If
the @q(-a) option is included, then the same rules apply to its
argument.  The file transfer display shows any transformations performed
upon filenames.

During transmission, files are encoded as follows:
@begin(itemize)
Control characters are converted to prefixed printables.

Sequences of repeated characters are collapsed via repeat counts, if
the other Kermit is also capable of repeated-character compression.

If parity is being used on the communication line, data characters with 
the 8th (parity) bit on are specially prefixed, provided the other Kermit
is capable of 8th-bit prefixing (if not, 8-bit binary files cannot be
successfully transferred).

Conversion is done between Unix newlines and carriage-return-linefeed 
sequences unless the @q(-i) option was specified.
@end(itemize)

@subheading(Command Line Examples:)

@example(kermit -l /dev/ttyi5 -b 1200 -cn -r)
This command connects you to the system on the other end of ttyi5 at
1200 baud, where you presumably log in and run Kermit with a 'send'
command.  After you escape back, C-Kermit waits for a file (or files) to
arrive.  When the file transfer is completed, you are again connected to
the remote system so that you can logout.

@example(kermit -l /dev/ttyi4 -b 1800 -cntp m -r -a foo)
This command is like the preceding one, except the remote system in this
case uses half duplex communication with mark parity.  The first file
that arrives is stored under the name @q(foo).

@exx(kermit -l /dev/ttyi6 -b 9600 -c | tek)
This example uses Kermit to connect your terminal to the system at the
other end of ttyi6.  The C-Kermit terminal connection does not
provide any particular terminal emulation, so C-Kermit's standard i/o is
piped through a (hypothetical) program called tek, which performs (say)
Tektronix emulation.

@exx(kermit -l /dev/ttyi6 -b 9600 -nf)
This command would be used to shut down a remote server and then connect
to the remote system, in order to log out or to make further use of it.
The @q(-n) option is invoked @i(after) @q(-f) (@q[-c] would have been invoked
before).

@exx(kermit -l /dev/ttyi6 -b 9600 -qg foo.\* &)
This command causes C-Kermit to be invoked in the background, getting a group
of files from a remote server (note the quoting of the '@q[*]' character).  No
display occurs on the screen, and the keyboard is not sampled for
interruption commands.  This allows other work to be done while file
transfers proceed in the background.

@exx(kermit -l /dev/ttyi6 -b 9600 -g foo.\* > foo.log < /dev/null &)
This command is like the previous one, except the file transfer display has
been redirected to the file @q(foo.log).  Standard input is also redirected, to
prevent C-Kermit from sampling it for interruption commands.

@exx(kermit -iwx)
This command starts up C-Kermit as a server.  Files are transmitted with
no newline/@|carriage-@|return-@|linefeed conversion; the @q(-i) option is
necessary for binary file transfer and useful for Unix-@|to-@|Unix transfers.
Incoming files that have the same names as existing files are given new, unique
names.

@exx(kermit -l /dev/ttyi6 -b 9600)
This command sets the communication line and speed.  Since no action is
specified, C-Kermit issues a prompt and enters an interactive dialog with
you.  Any settings given on the command line remain in force during the
dialog, unless explicitly changed.

@exx(kermit)
This command starts up Kermit interactively with all default settings.

A final example shows how Unix Kermit might be used to send an entire directory
tree from one Unix system to another, using the tar program as Kermit's
standard input and output.  On the orginating system, in this case the remote,
type (for instance):@label(-uxtar)

@exx(tar cf - /usr/fdc | kermit -is -)
This causes tar to send the directory @q(/usr/fdc) (and all its files and all
its subdirectories and all their files...) to standard output instead of to a
tape; kermit receives this as standard input and sends it as a binary file.
On the receiving system, in this case the local one, type (for instance):

@exx(kermit -il /dev/ttyi5 -b 9600 -k | tar xf -)
Kermit receives the tar archive, and sends it via standard output to its own
copy of tar, which extracts from it a replica of the original directory tree.

@subheading(Exit Status Codes:)

Kermit returns an exit status of zero, except when a fatal error is
encountered, where the exit status is set to one.  With background
operation (e.g., '@q(&)' on invoking command line), driven by scripted
interactive commands (redirected standard input and/or take files),
any failed interactive command (such as failed dial or script attempt)
causes the fatal error exit.


@section(Interactive Operation)

C-Kermit's interactive command prompt is "@q(C-Kermit>)".  In response to this
prompt, you may type any valid command.  C-Kermit executes the command and
then prompts you for another command.  The process continues until you
instruct the program to terminate.

Commands begin with a keyword, normally an English verb, such as "send".
You may omit trailing characters from any keyword, so long as you specify
sufficient characters to distinguish it from any other keyword valid in that
field.  Certain commonly-used keywords (such as "send", "receive", "connect")
have special non-unique abbreviations ("s" for "send", "r" for "receive",
"c" for "connect").

Certain characters have special functions in interactive commands:
@Begin(Description,leftmargin +8,indent -4)
@q(?)@\Question mark, typed at any point in a command, will produce a
message explaining what is possible or expected at that point.  Depending on
the context, the message may be a brief phrase, a menu of keywords, or a list
of files.

@q(ESC)@\(The Escape or Altmode key) -- Request completion of the current
keyword or filename, or insertion of a default value.  The result will be a
beep if the requested operation fails.

@q(DEL)@\(The Delete or Rubout key) -- Delete the previous character from the
command.  You may also use BS (Backspace, Control-H) for this function.

@q(^W)@\(Control-W) -- Erase the rightmost word from the command line.

@q(^U)@\(Control-U) -- Erase the entire command.

@q(^R)@\(Control-R) -- Redisplay the current command.

@q(SP)@\(Space) -- Delimits fields (keywords, filenames, numbers) within
a command.  HT (Horizontal Tab) may also be used for this purpose.

@q(CR)@\(Carriage Return) -- Enters the command for execution.  LF (Linefeed)
or FF (formfeed) may also be used for this purpose.

@q(\)@\(Backslash) -- Enter any of the above characters into the command,
literally.  To enter a backslash, type two backslashes in a row (@q[\\]).
@End(Description)
You may type the editing characters (@q[DEL], @q[^W], etc) repeatedly, to
delete all the way back to the prompt.  No action will be performed until the
command is entered by typing carriage return, linefeed, or formfeed.  If you
make any mistakes, you will receive an informative error message and a new
prompt -- make liberal use of '@q[?]' and ESC to feel your way through the
commands.  One important command is "help" -- you should use it the first time
you run C-Kermit.

Interactive C-Kermit accepts commands from files as well as from the
keyboard.  When you enter interactive mode, C-Kermit looks for the file
@q(.kermrc) in your home or current directory (first it looks in the home
directory, then in the current one) and executes any commands it finds there.
These commands must be in interactive format, not Unix command-line format.
A "take" command is also provided for use at any time during an interactive
session.  Command files may be nested to any reasonable depth.

Here is a brief list of C-Kermit interactive commands:
@begin(format,spread 0)
@tabclear()@tabset(1.5inches,2.0inches,2.5inches)
@>!@\  Execute a Unix shell command.
@>bye@\  Terminate and log out a remote Kermit server.
@>close@\  Close a log file.
@>connect@\  Establish a terminal connection to a remote system.
@>cwd@\  Change Working Directory.
@>dial@\  Dial a telephone number.
@>directory@\  Display a directory listing.
@>echo@\  Display arguments literally.
@>exit@\  Exit from the program, closing any open logs.
@>finish@\  Instruct a remote Kermit server to exit, but not log out.
@>get@\  Get files from a remote Kermit server.
@>help@\  Display a help message for a given command.
@>log@\  Open a log file -- debugging, packet, session, transaction.
@>quit@\  Same as 'exit'.
@>receive@\  Passively wait for files to arrive.
@>remote@\  Issue file management commands to a remote Kermit server.
@>script@\  Execute a login script with a remote system.
@>send@\  Send files.
@>server@\  Begin server operation.
@>set@\  Set various parameters.
@>show@\  Display values of 'set' parameters.
@>space@\  Display current disk space usage.
@>statistics@\  Display statistics about most recent transaction.
@>take@\  Execute commands from a file.
@end(format)

The 'set' parameters are:
@begin(format,spread 0)
@tabclear()@tabset(1.5inches,2.0inches,2.5inches)
@>block-check@\  Level of packet error detection.
@>delay@\  How long to wait before sending first packet.
@>duplex@\  Specify which side echoes during 'connect'.
@>end-of-packet@\  Terminator for outbound packets.
@>escape-character@\  Character to prefix "escape commands" during 'connect'.
@>file@\  Set various file parameters.
@>flow-control@\  Communication line full-duplex flow control.
@>handshake@\  Communication line half-duplex turnaround character.
@>line@\  Communication line device name.
@>modem-dialer@\  Type of modem-dialer on communication line.
@>packet-length@\  Maximum length for packets.
@>pad-character@\  Character to use for inter-packet padding.
@>padding@\  How much inter-packet padding to use.
@>parity@\  Communication line character parity.
@>prompt@\  Change the C-Kermit program's prompt.
@>speed@\  Communication line speed.
@>start-of-packet@\  Control character to mark beginning of packets.
@>timeout@\  Timer interval to detect lost packets.
@end(format)

The 'remote' commands are:
@begin(format,spread 0)
@tabclear()@tabset(1.5inches,2.0inches,2.5inches)
@>cwd@\  Change remote working directory.
@>delete@\  Delete remote files.
@>directory@\  Display a listing of remote file names.
@>help@\  Request help from a remote server.
@>host@\  Issue a command to the remote host in its own command language.
@>space@\  Display current disk space usage on remote system.
@>type@\  Display a remote file on your screen.
@>who@\  Display who's logged in, or get information about a user.
@end(format)

Most of these commands are described adequately in the Kermit User Guide.
Special aspects of certain Unix Kermit commands are described below.

@heading<The 'send' command>

Syntax:  @q<send >@i(fn)@q<@ @ - >@i<or>@q< -@ @ >@q<send >@i(fn1)@q< >@i<rfn1>

Send the file or files denoted by fn to the other Kermit, which should be
running as a server, or which should be given the 'receive' command.  Each
file is sent under its own name (as described above, or as
specified by the 'set file names' command).  If the second form is used,
i.e. with @i(fn1) denoting a single Unix file, @i(rfn1) may be specified as a
name to send it under.  The 'send' command may be abbreviated to 's', even
though 's' is not a unique abbreviation for a top-level C-Kermit command.

The wildcard (meta) characters '@q[*]' and '@q[?]' are accepted in @i(fn).  If
'@q[?]' is to be included, it must be prefixed by '@q[\]' to override its
normal function of providing help.  '@q[*]' matches any string, '@q[?]' matches
any single character.  Other notations for file groups, like '@q([a-z]og)', are
not available in interactive commands (though of course they are available on
the command line).  When @i(fn) contains '@q[*]' or '@q[?]' characters, there
is a limit to the number of files that can be matched, which varies from system
to system.  If you get the message "Too many files match" then you'll have to
make a more judicious selection.  If @i(fn) was of the form
@example(usr/longname/anotherlongname/*)
then C-Kermit's string space will fill up rapidly -- try doing a cwd (see
below) to the path in question and reissuing the command.

@i<Note> -- C-Kermit sends only from the current or specified directory.
It does not traverse directory trees.  If the source directory contains
subdirectories, they will be skipped.  Conversely, C-Kermit does not create
directories when receiving files.  If you have a need to do this, you can
pipe tar through C-Kermit, as shown in the example on page @pageref(-uxtar),
or under System III/V Unix you can use cpio.

@i<Another Note> -- C-Kermit does not skip over "invisible" files that match
the file specification; Unix systems usually treat files whose names start
with a dot (like @q(.login), @q(.cshrc), and @q(.kermrc)) as invisible.

@heading<The 'receive' command>

Syntax:  @q<receive@ @ - >@i<or>@q< -@ @ receive >@i<fn1>

Passively wait for files to arrive from the other Kermit, which must be given
the 'send' command -- the 'receive' command does not work in conjunction with a
server (use 'get' for that).  If @i(fn1) is specified, store the first incoming
file under that name.  The 'receive' command may be abbreviated to 'r'.


@heading<The 'get' command:>

Syntax:@q<  get >@i<rfn>
@begin(example)
    @i<or>: get
            @i(rfn)
            @i(fn1)
@end(example)
Request a remote Kermit server to send the named file or files.  Since a
remote file specification (or list) might contain spaces, which normally
delimit fields of a C-Kermit command, an alternate form of the command is
provided to allow the inbound file to be given a new name: type 'get' alone
on a line, and you will be prompted separately for the remote and local
file specifications, for example
@Begin(Example)
C-Kermit>@ux(get)
 Remote file specification: @ux(foo)
 Local name to store it under: @ux(bar)
@End(Example)
As with 'receive', if more than one file arrives as a result of the 'get'
command, only the first will be stored under the alternate name given by
@i(fn1); the remaining files will be stored under their own names if possible.
If a '@q[?]' is to be included in the remote file specification, you must
prefix it with '@q[\]' to suppress its normal function of providing help.

@heading(The 'server' command:)

The 'server' command places C-Kermit in "server mode" on the currently selected
communication line.  All further commands must arrive as valid Kermit packets
from the Kermit on the other end of the line.  The Unix Kermit server can
respond to the following commands:
@begin(format,spread 0,above 1,below 1)
@tabclear()@tabset(2.25inches)
@u<Command>@\@ux<Server Response>
  get@\  Sends files
  send@\  Receives files
  bye@\  Attempts to log itself out
  finish@\  Exits to level from which it was invoked
  remote directory@\  Sends directory lising
  remote delete@\  Removes files
  remote cwd@\  Changes working directory
  remote type@\  Sends files to your screen
  remote space@\  Reports about its disk usage
  remote who@\  Shows who's logged in
  remote host@\  Executes a Unix shell command
  remote help@\  Lists these capabilities
@end(format)
Note that the Unix Kermit server cannot always respond to a BYE command.
It will attempt to do so using "@q<kill(0,9)>", but this will not work
on all systems or under all conditions.  For instance, the C-Shell
changes your process group, so that the process id of 0 does not refer
to what you might expect.

If the Kermit server is directed at an external line (i.e. it is in
"local mode") then the console may be used for other work if you have
'set file display off'; normally the program expects the
console to be used to observe file transfers and enter status queries or
interruption commands.  The way to get C-Kermit into background operation from
interactive command level varies from system to system (e.g. on Berkeley Unix
you would halt the program with @q(^Z) and then use the C-Shell 'bg' command to
continue it in the background).  The more common method
is to invoke the program with the desired command line arguments, including
"@q(-q)", and with a terminating "@q(&)".

When the Unix Kermit server is given a 'remote host' command, it executes it
using the shell invoked upon login to the remote system, e.g. the Bourne shell
or the Berkeley C-Shell.  (Note -- this is in distinction to the local "@q<!>"
shell escape, which always uses the Bourne shell; see below).

@Heading(The 'remote', 'bye', and 'finish' commands:)

C-Kermit may itself request services from a remote Kermit server.  In
addition to the 'send' and 'get' commands, the following may also be used:

@begin(description,leftmargin +8,indent -4)
remote cwd [@i(directory)]@\If the optional remote directory specification is
included, you will be prompted on a separate line for a password, which will
not echo as you type it.
@end(description)
@begin(description,leftmargin +28, indent -24,spread 0,above 1)
remote delete rfn@\delete remote file or files.

remote directory [@i(rfn)]@\directory listing of remote files.

remote host @i(command)@\command in remote host's own command language.

remote space@\disk usage report from remote host.

remote type [@i(rfn)]@\display remote file or files on the screen.

remote who [@i(user)]@\display information about who's logged in.

remote help@\display remote server's capabilities.
@end(description)
@begin(description,leftmargin +8,indent -4)
bye @i(and) finish:@\When connected to a remote Kermit server, these commands
cause the remote server to terminate; 'finish' returns it to Kermit or system
command level (depending on the implementation or how the program was invoked);
'bye' also requests it to log itself out.
@end(description)
@heading(The 'log' and 'close' commands:)

Syntax: @q<log {debugging, packets, session, transactions} >[ @i(fn1) ]

C-Kermit's progress may be logged in various ways.  The 'log' command
opens a log, the 'close' command closes it.  In addition, all open logs
are closed by the 'exit' and 'quit' commands.  A name may be specified for
a log file; if the name is omitted, the file is created with a default
name as shown below.

@begin(description,leftmargin +4,indent -4)
log debugging@\This produces a voluminous log of the internal workings of
C-Kermit, of use to Kermit developers or maintainers in tracking down suspected
bugs in the C-Kermit program.  Use of this feature dramatically slows down the
Kermit protocol.  Default name: @q(debug.log).

log packets@\This produces a record of all the packets that go in and out of
the communication port.  This log is of use to Kermit maintainers who are
tracking down protocol problems in either C-Kermit or any Kermit that
C-Kermit is connected to.  Default name:  @q(packet.log).

log session@\This log will contain a copy of everything you see on your screen
during the 'connect' command, except for local messages or interaction with
local escape commands.  Default name:  @q(session.log).

log transactions@\The transaction log is a record of all the files that were
sent or received while transaction logging was in effect.  It includes time
stamps and statistics, filename transformations, and records of any
errors that may have occurred.  The transaction log allows you to have
long unattended file transfer sessions without fear of missing some
vital screen message.  Default name:  @q(transaction.log).
@end(description)
The 'close' command explicitly closes a log, e.g. 'close debug'.

@Heading(Local File Management Commands:)

Unix Kermit allows some degree of local file management from interactive
command level:
@begin(description,leftmargin +4,indent -4)
directory [@i(fn)]@\
Displays a listing of the names, modes, sizes, and dates of files
matching @i(fn) (which defaults to '@q[*]').  Equivalent to '@q(ls -l)'.

cwd [directory-name]@\
Changes Kermit's working directory to the one given, or to the your
default directory if the directory name is omitted.  Equivalent to 'cd'.

space@\
Display information about disk space and/or quota in the current
directory and device.

@q(! )@i(command)@\
The command is executed by the Unix shell.  Use this for all other
file management commands.  This command has certain peculiarities:
@begin(itemize,spread 0)	
The Bourne shell is used.

At least one space must separate the '!' from the shell command.

A 'cd' command executed in this manner will have no effect -- use the
C-Kermit 'cwd' command instead. 
@end(itemize)
@end(description)

@heading(The 'set' and 'show' Commands:)

Since Kermit is designed to allow diverse systems to communicate, it is
often necessary to issue special instructions to allow the program to adapt
to peculiarities of the another system or the communication path.  These
instructions are accomplished by the 'set' command.  The 'show' command may
be used to display current settings.  Here is a brief synopsis of settings
available in the current release of C-Kermit:

@begin(description,leftmargin +4,indent -4)
block-check {1, 2, 3}@\ Determines the level of per-packet error detection.
"1" is a single-@|character 6-bit checksum, folded to include the values of all
bits from each character.  "2" is a 2-character, 12-bit checksum.  "3" is a
3-character, 16-bit cyclic redundancy check (CRC).  The higher the block check,
the better the error detection and correction and the higher the resulting
overhead.  Type 1 is most commonly used; it is supported by all Kermit
implementations, and it has proven adequate in most circumstances.  Types 2 or
3 would be used to advantage when transferring 8-bit binary files over noisy
lines.

delay @i(n)@\How many seconds to wait before sending the first packet after a
'send' command.  Used in remote mode to give you time to escape back to your
local Kermit and issue a 'receive' command.  Normally 5 seconds.

duplex {full, half}@\For use during 'connect'.  Specifies which side is doing
the echoing; 'full' means the other side, 'half' means C-Kermit must echo
typein itself.

end-of-packet @i(cc)@\Specifies the control character needed by the other
Kermit to recognize the end of a packet.  C-Kermit sends this character at the
end of each packet.  Normally 13 (carriage return), which most Kermit
implementations require.  Other Kermits require no terminator at all, still
others may require a different terminator, like linefeed (10).

escape-character @i(cc)@\For use during 'connect' to get C-Kermit's attention.
The escape character acts as a prefix to an 'escape command', for instance to
close the connection and return to C-Kermit or Unix command level.
The normal escape character is Control-Backslash (28).
The escape character is also used in System III/V implementations,
to prefix interrupt commands during file transfers.

file {display, names, type, warning}@\
Establish various file-related parameters:
@begin(description,leftmargin +4,indent -4)
display {on, off}@\Normally 'on'; when in local mode, display progress of file
transfers on the screen (stdout), and listen to the keyboard (stdin)
for interruptions.  If off (-q on command line) none of this is
done, and the file transfer may proceed in the background oblivious
to any other work concurrently done at the console terminal.

names {converted, literal}@\
Normally converted, which mean that outbound filenames have path
specifications stripped, lowercase letters raised to upper,
tildes and extra periods changed to X's, and an X inserted in
front of any name that starts with period.  Incoming files have
uppercase letters lowered.  Literal means that none of these
conversions are done; therefore, any directory path appearing in a
received file specification must exist and be write-accessible.
When literal naming is being used, the sender should not use path
names in the file specification unless the same path exists on the
target system and is writable.

type {binary, text}@\Normally text, which means that conversion is done between
Unix newline characters and the carriage-@|return/@|linefeed sequences
required by the canonical Kermit file transmission format, and in
common use on non-@|Unix systems.  Binary means to transmit file
contents without conversion.  Binary ('@q(-i)' in command line notation)
is necessary for binary files, and desirable in all Unix-@|to-@|Unix
transactions to cut down on overhead.

warning {on, off}@\Normally off, which means that incoming files will silently
overwrite existing files of the same name.  When on ('@q(-w)' on command line)
Kermit will check if an arriving file would overwrite an existing file; if so,
it will construct a new name for the arriving file, of the form @q(foo~)@i(n),
where foo is the name they share and @i(n) is a "generation number"; if @i(foo)
exists, then the new file will be called @q(foo~1).  If @q(foo) and @q(foo~1)
exist, the new file will be @q(foo~2), and so on.
@end(description)

flow-control {none, xon/xoff}@\Normally xon/xoff for full duplex flow control.
Should be set to 'none' if the other system cannot do xon/xoff flow control.

handshake {xon, xoff, cr, lf, bell, esc, none}@\
Normally none.  Otherwise, half-duplex communication line turnaround
handshaking is done, which means Unix Kermit will not reply to a packet
until it has received the indicated handshake character or has timed out
waiting for it.

line [device-name]@\
The device name for the communication line to be used for file transfer
and terminal connection, e.g. @q(/dev/ttyi3).  If you specify a device name,
Kermit will be in local mode, and you should remember to issue any other
necessary 'set' commands, such as 'set speed'.  If you omit the device
name, Kermit will revert to its default mode of operation.

modem-dialer {direct, hayes, ventel}@\ The type of modem dialer on the
communication line.  "Direct" indicates either there is no dialout modem, or
that if the line requires carrier detection to open, then 'set line' will hang
waiting for an incoming call.  "Hayes" and "Ventel" indicate that the
subsequent 'set line' (or the -l argument) will prepare for a subsequent 'dial'
command for Hayes and Ventel dialers, respectively.

packet-length @i(n)@\Specify the maximum packet length to use.  Normally 90.
Shorter packet lengths can be useful on noisy lines, or with systems or front
ends or networks that have small buffers.  The shorter the packet, the higher
the overhead, but the lower the chance of a packet being corrupted by noise,
and the less time to retransmit corrupted packets.

pad-character @i(cc)@\C-Kermit normally does not need to have incoming packets
preceded with pad characters.  This command allows C-Kermit to request the
other Kermit to use cc as a pad character.  Default cc is NUL, ASCII 0.

padding @i(n)@\How many pad characters to ask for, normally 0.

parity {even, odd, mark, space, none}@\Specify character parity for use in
packets and terminal connection, normally none.  If other than none, C-Kermit
will seek to use the 8th-bit prefixing mechanism for transferring 8-bit binary
data, which can be used successfully only if the other Kermit agrees; if not,
8-bit binary data cannot be successfully transferred.

prompt [string]@\The given string will be substituted for "@q(C-Kermit)>" as
this program's prompt.  If the string is omitted, the prompt will revert to
"@q(C-Kermit>)".

speed {0, 110, 150, 300, 600, 1200, 1800, 2400, 4800, 9600}@\The baud rate for
the external communication line.  This command cannot be used to change the
speed of your own console terminal.  Many Unix systems are set up in such a way
that you must give this command after a 'set line' command before you can use
the line.

start-of-packet @i(cc)@\The Kermit packet prefix is Control-A (1).  The only
reasons it should ever be changed would be: Some piece of equipment somewhere
between the two Kermit programs will not pass through a Control-A; or, some
piece of of equipment similarly placed is echoing its input.  In the latter
case, the recipient of such an echo can change the packet prefix for outbound
packets to be different from that of arriving packets, so that the echoed
packets will be ignored.  The opposite Kermit must also be told to change the
prefix for its inbound packets.  Unix Kermit presently can be told to change
only its outbound packet prefix.

timeout @i(n)@\Normally, each Kermit partner sets its packet timeout interval
based on what the opposite Kermit requests.  This command allows you to
override the normal procedure and specify a timeout interval.  If you specify
0, then no timeouts will occur, and Unix Kermit will wait forever for expected
packets to arrive.
@end(description)

@heading(The 'show' Command:)

Syntax: @q<show {parameters, versions}>

The show command displays the values of all the 'set' parameters described
above.  If you type 'show versions', then C-Kermit will display the version
numbers and dates of all its internal modules.  You should use the 'show
versions' command to ascertain the vintage of your Kermit program before
reporting problems to Kermit maintainers.

@heading(The 'statistics' Command:)

The statistics command displays information about the most recent Kermit
protocol transaction, including file and communication line i/o, as well
as what encoding options were in effect (such as 8th-bit prefixing,
repeat-@|count compression).

@heading(The 'take' and 'echo' Commands:)

Syntax: @q<take >@i<fn1>

The 'take' command instructs C-Kermit to execute commands from the named
file.  The file may contain any interactive C-Kermit commands, including
'take'; command files may be nested to any reasonable depth.  The 'echo'
command may be used within command files to issue greetings, announce
progress, etc.

Command files are in exactly the same syntax as interactive commands.
Note that this implies that if you want to include special characters like
question mark or backslash that you would have to quote with backslash when
typing interactive commands, you must quote these characters the same way
in command files.

Command files may be used in lieu of command macros, which have not been
implemented in this version of C-Kermit.  For instance, if you commonly
connect to a system called 'B' that is connected to ttyh7 at 4800 baud,
you could create a file called @q(b) containing the commands
@begin(example)
set line /dev/ttyh7
set speed 4800
echo Connecting to System B...
connect
@end(example)
and then simply type 'take b' (or 't b' since no other commands begin with
the letter 't') whenever you wished to connect to system B.

For connecting to IBM mainframes, a number of 'set' commands are required;
these, too, are conveniently collected into a 'take' file like this one:
@begin(example)
set speed 1200
set parity mark
set handshake xon
set flow-control none
set duplex half
@end(example)

An implicit 'take' command is executed upon your @q(.kermrc) file upon
C-Kermit's initial entry into interactive dialog.  The @q(.kermrc) file should
contain 'set' or other commands you want to be in effect at all times.
For instance, you might want override the default action when incoming files
have the same names as existing files -- in that case, put the command
@example(set file warning on)
in your @q(.kermrc) file.

Commands executed from take files are not echoed at the terminal.  If you
want to see the commands as well as their output, you could feed the
command file to C-Kermit via redirected stdin, as in
@example('kermit < cmdfile')
Errors encountered during execution of take files (such as failure to complete
dial or script operations) cause termination of the current take file, popping
to the take file that invoked it, or to interactive level.  When kermit is
executed in the background, errors during execution of a take file are fatal.

@heading(The 'connect' Command:)

The connect command links your terminal to another computer as if it were
a local terminal to that computer, through the device specified in the most
recent 'set line' command, or through the default device if your system
is a PC or workstation.  All characters you type at your keyboard
are sent out the communication line, all characters arriving at the
communication port are displayed on your screen.  Current settings of speed,
parity, duplex, and flow-@|control are honored.  If you have issued a 'log
session' command, everything you see on your screen will also be recorded
to your session log.  This provides a way to "capture" files from systems
that don't have Kermit programs available.

To get back to your own system, you must type the escape character, which is
Control-@|Backslash (@q[^\]) unless you have changed it with the 'set escape'
command, followed by a single-@|character command, such as 'c' for "close
connection".  Single-@|character commands include:
@begin(description,leftmargin +8,indent -6,spread 0.4)
c@\Close the connection

b@\Send a BREAK signal

0@\(zero) send a null

s@\Give a status report about the connection

@q[^\]@\Send Control-Backslash itself (whatever you have defined the
escape character to be, typed twice in a row sends one copy of it).
@end(description)
Lowercase and control equivalents for these letters are also accepted.  A
space typed after the escape character is ignored.  Any other character will
produce a beep.

The connect command simply displays incoming characters on the screen.  It is
assumed any screen control sequences sent by the host will be handled by
the firmware in your terminal or PC.  If terminal emulation is desired,
then the connect command can invoked from the Unix command line (@q(-c) or
@q(-n)), piped through a terminal emulation filter, e.g.
@example(kermit -l /dev/acu -b 1200 -c | tek)
'c' is an acceptable non-unique abbreviation for 'connect'.

@heading(The 'dial' command:)

Syntax: @q(dial )@i(telephone-number-string)

This command controls dialout modems.  The telephone-@|number-@|string may
contain modem-@|dialer commands, such as comma for Hayes pause, or '@q(&)'
for Ventel dialtone wait and '@q(%)' for Ventel pause.

Because modem dialers have strict requirements to override the carrier-@|detect
signal most Unix implementations expect, the sequence for dialing is more rigid
than with the rest of kermit's features.

Example one:
@begin(example)
@ux<kermit -l /dev/cul0 -b 1200>
C-Kermit>@ux<set modem-dialer hayes>	@i(hint: abbreviate) set m h
C-Kermit>@ux<dial 9,5551212>
Connected!
C-Kermit>@u<connect>			@i(hint: abbreviate) c
@i(logon, request remote server, etc.)
C-Kermit> ...
C-Kermit>@u<quit>			@i(hint: abbreviate) q
@end(example)
this disconnects modem, and unlocks line.

Example two:
@begin(example)
@u(kermit)
C-Kermit>@ux(set modem-dialer ventel)
C-Kermit>@ux(set line /dev/cul0)
C-Kermit>@ux(dial 9&5551212%)
Connected!
C-Kermit> ...
@end(example)
Example three:
@begin(example)
kermit
C-Kermit>@ux(take my-dial-procedure)
Connected!

@i(file my-dial-procedure):
set modem hayes
set line /dev/tty99
dial 5551212
connect
@end(example)
For Hayes dialers, two important switch settings are #1 and #6.  #1
should be up so that the DTR is only asserted when the line is 'open'.
#6 should be up so carrier-@|detect functions properly.  Switches #2
(English versus digit result codes) and #4 (Hayes echoes modem commands)
may be in either position.

@heading(The 'script' Command:)

Syntax: @q(script )@i(expect send [expect send] . . .)

"expect" has the syntax: @i(expect[-send-expect[-send-expect[...]]])

This command facilitates logging into a remote system and/or invoking
programs or other facilities after login on a remote system.

This login script facility operates in a manner similar to that commonly
used by the Unix uucp System's "@q(L.sys)" file entries.  A login script 
is a sequence of the form:
@example(@i<expect send [expect send] . . .>)
where @i(expect) is a prompt or message to be issued by the remote site, and
@i(send) is the string (names, numbers, etc) to return.  The send may also be
the keyword EOT, to send Control-D, or BREAK, to send a break signal.  Letters
in send may be prefixed by '@q[~]' to send special characters.  These are:
@begin(description,leftmargin +8,indent -4,spread 0)
@q(~b)@\backspace

@q(~s)@\space

@q(~q)@\'@q[?]'(trapped by Kermit's command interpreter)

@q(~n)@\linefeed

@q(~r)@\carriage return

@q(~t)@\tab

@q(~')@\single quote

@q(~~)@\tilde

@q(~")@\double quote

@q(~c)@\don't append a carriage return

@q(~)@i(o[o[o]])@\an octal character
@end(description)
As with some uucp systems, sent strings are followed by @q(~r) unless they have
a @q(~c).

Only the last 7 characters in each expect are matched.  A null @i(expect),
e.g. @q(~0) or two adjacent dashes, causes a short delay before proceeding
to the next send sequence.  A null expect always succeeds.

As with uucp, if the expect string does not arrive, the script attempt
fails.  If you expect that a sequence might not arrive, as with uucp, 
conditional sequences may be expressed in the form:
@example(@i<-send-expect[-send-expect[...]]>)
where dashed sequences are followed as long as previous expects fail.

@i(Expect/send) transactions can be easily be debugged by logging
transactions.  This records all exchanges, both expected and actual.

Note that '@q[\]' characters in login scripts, as in any other C-Kermit
interactive commands, must be doubled up.

Example one:

Using a modem, dial a unix host site.  Expect "login" (...gin), and if it
doesn't come, simply send a null string with a @q(~r).  (Some Unixes
require either an EOT or a BREAK instead of the null sequence, depending
on the particular site's "logger" program.)  After providing user id
and password, respond "x" to a question-mark prompt, expect the Bourne
shell "@q($)" prompt (and send return if it doesn't arrive).  Then cd to
directory kermit, and run the program called "wermit", entering the
interactive connect state after wermit is loaded.
@begin(example)
set modem-dialer ventel
set line /dev/tty77
set baud 1200
dial 9&5551212
script gin:--gin:--gin: smith ssword: mysecret ~q x $--$ 
	cd~skermit $ wermit
connect
@end(example)

Example two:

@index(TELENET)
Using a modem, dial the Telenet network.  This network expects three
returns with slight delays between them.  These are sent following
null expects.  The single return is here sent as a null string, with
a return appended by default.  Four returns are sent to be safe before
looking for the prompt.  Then the telenet id and password are entered.
Then telenet is instructed to connect to a host site (c 12345).  The
host has a data switch, and to "which system" it responds "myhost".
This is followed by a TOPS-20 logon, and a request to load Kermit,
set even parity, and enter the server mode.  Files
are then exchanged.  The commands are in a take file.  The
login command is split onto two lines for readability, though it
is a single long line in the take file.
@begin(example)
set modem-dialer hayes
set line /dev/cul0
set baud 1200
dial 9,5551212
set parity even
script ~0 ~0 ~0 ~0 ~0 ~0 ~0 ~0 @@--@@--@@ id~saa001122 = 002211 @@ 
    c~s12345 ystem-c~s12345-ystem myhost @@ joe~ssecret @@ kermit 
    > set~sparity~seven > server
send some.stuff
get some.otherstuff
bye
quit
@end(example)
Since these commands may be executed totally in the background, they
can also be scheduled.  A typical shell script, which might be scheduled
by cron, would be as follows (csh used for this example):

@begin(example)
#
#keep trying to dial and log onto remote host and exchange files
#wait 10 minutes before retrying if dial or script fail.
# 
while ( 1 )
	kermit < tonight.cmd &
	if ( ! $status ) break
	sleep 600
end
@end(example)
File tonight.cmd might have two takes in it, for example, one to take
a file with the set modem, set line, set baud, dial, and script, and
a second take of a file with send/get commands for the remote server.
The last lines of @q(tonight.cmd) should be a bye and a quit.

@heading(The 'help' Command:)

@begin(example,leftmargin 0)
@r(Syntax:) help
@ @ @ @i(or): help @i(keyword)
@ @ @ @i(or): help {set, remote} @i(keyword)
@end(example)

Brief help messages or menus are always available at interactive command
level by typing a question mark at any point.  A slightly more verbose form
of help is available through the 'help' command.  The 'help' command with
no arguments prints a brief summary of how to enter commands and how to
get further help.  'help' may be followed by one of the top-level C-Kermit
command keywords, such as 'send', to request information about a command.
Commands such as 'set' and 'remote' have a further level of help.  Thus you
may type 'help', 'help set', or 'help set parity'; each will provide a
successively more detailed level of help.


@heading(The 'exit' and 'quit' Commands:)

These two commands are identical.  Both of them do the following:

@begin(itemize,spread 0)
Attempt to insure that the terminal is returned to normal.

Relinquish access to any communication line assigned via 'set line'.

Close any open log files.

Relinquish any uucp and multiuser locks on the communications line.

Hang up the modem, if the communications line supports data terminal ready.
@end(itemize)
After exit from C-Kermit, your default directory will be the same as when
you started the program.  The 'exit' command is issued implicitly whenever
C-Kermit halts normally, e.g. after a command line invocation, or after certain
kinds of interruptions.

@section(C-Kermit under Berkeley or System III/V Unix:)

C-Kermit may be interrupted at command level or during file transfer by
typing Control-C.  The program will perform its normal exit function,
restoring the terminal.  If a protocol transaction was in progress, an
error packet will be sent to the opposite Kermit so that it can terminate
cleanly.

C-Kermit may be invoked in the background ("&" on shell commmand line).
If a background process is "killed", the user will have to manually
remove any lock file and may need to restore the modem.  This is because 
the kill signal (@q<kill(@i[x],9)>) cannot be trapped by Kermit.

During execution of a system command, C-Kermit can often be returned to
command level by typing a single Control-C.  (With System III/V, the
usual interrupt function (often the DEL key) is replaced by Control-C.)
On detecting Control-C, C-Kermit takes its normal exit, removing
lock files and restoring the communication line, modem, and/or console
terminal.

@begin(description,leftmargin +4, indent -4)
Under Berkeley Unix only:@\
C-Kermit may also be interrupted by ^Z to put the process in the background.
In this case the terminal is not restored.  You will have to type
Control-J followed by "reset" followed by another Control-J to get your
terminal back to normal.  C-Kermit can be halted in a similar manner by
typing Control-@\Backslash, except that instead of moving it to the
background, a core dump is produced.

Under System III/V Unix:@\
The Control-@q(\) character (or whatever control character has been
selected via "set escape") at the C-Kermit command level
is ignored; it is trapped and will not core-@|dump or interrupt Kermit.
@end(description)

Control-C, Control-Z, and Control-@q(\) lose their normal functions during
terminal connection and also during file transfer when the controlling tty
line is being used for packet i/o.

The BSD implementation of C-Kermit has code to take advantage of a special
nonstandard kernel-mode line driver, which boosts the speed of packet i/o
significantly.  The problem is that "raw" mode, needed for packet i/o, also
implies "cbreak" (character wakeup) mode, which is very expensive.  The new
line driver is a modification of the "berknet" driver, which allowed raw mode
i/o to take place with process wakeup only upon receipt of a linefeed.  The
Berknet driver, unfortunately, strips off the high order bit of each
character and does not allow the line terminator to be specified.  The
modification allows all 8 bits to pass through unmolested, allows the wakeup
character to be specified, and allows the buffer to be tested or cleared.

The System III/V implementation uses regular kernel drivers, but "gulps"
rawmode input in large blocks, thus overcomming the usual system call
overheads.

If you are running C-Kermit in "quiet mode" in the foreground, then
interrupting the program with a console interrupt like Control-C will not
restore the terminal to normal conversational operation.  This is because
the system call to enable console interrupt traps will cause the program to
block if it's running in the background, and the primary reason for quiet
mode is to allow the program to run in the background without blocking, so
that you can do other work in the foreground.

If C-Kermit is run in the background ("&" on shell commmand line), then
the interrupt signal (Control-C) (and System III/V quit signal) are
ignored.  This prevents an interrupt signal intended for a foreground
job (say a compilation) from being trapped by a background Kermit session.


@section(C-Kermit on the DEC Pro-3xx with Venix 1@q(.)0)

The DEC Professional 300 series are PDP-11/23 based personal computers.
Venix 1@q(.)0 is a Unix v7 derivative.  It should not be confused with
Venix 2@q(.)0, which resembles ATT System V.
C-Kermit runs in local mode on the Pro-3@i(xx) when invoked from the
console; the default device is @q(/dev/com1).  When connected to a
remote system (using C-Kermit's 'connect' command), Pro/Venix itself (not
Kermit) provides VT52 terminal emulation.

During file transfer, the interruption and status commands (Control-A,
Control-F, etc) are not available.

@section(C-Kermit Restrictions and Known Bugs)

@begin(enumerate)
@ux(File renaming):  When filename collision avoidance ("warning") is
selected, C-Kermit constructs unique names by appending a generation number
to the end of the file name.  Currently, no checking is done to ensure that
the result is still within the maximum length for a file name.

@ux(UUCP line locking):  C-Kermit locks lines, to prevent UUCP and
multiuser conflicts, when it first opens a communications line.  This
occurs either when 'set line' is issued, or if the '-l' argument is
used, when the first 'dial',
'connect', or protocol operation occurs.  The lock is released if 
another 'set line' is issued, or if the program quits, exits, or is
terminated by Control-C.
If a user connects and returns to the shell command level, for example
to initiate kermit by piped commands, on a communications line, the
line lock is released when returning to the shell.
Locking is not needed, or used, if communications occur over the
local terminal line (e.g. @q[/dev/tty]).  In that case, there is no
difficultly with "piped" operations releasing locks and lines.

@begin(multiple)
@ux(Removing stale lock files):
For various reasons, lock files sometimes get left about after
uucp or C-Kermit activities.  (The most common reason is that
the uucp or C-Kermit activity was "killed" by a shell command.)
If the lock file is owned by yourself, clearly you may remove
it (presuming you are not running C-Kermit or uucp in the background
when you discovered it).

Uucp supports a function, called uuclean, which is customarily
used to remove these files after a predetermined age.  If in doubt
about a lock file on the dial-out line you need, contact your
system's operator.
@end(multiple)

@ux(Modem controls):
If connection is made over a communication line (rather than on
the controlling terminal line), and that line has modem controls,
(e.g. data terminal ready and carrier detection implementation),
returning to the shell level will disconnect the conversation.
In that case, one should use interactive mode commands, and
avoid use of piped shell-@|level operation (also see 'set modem-dialer'
and 'dial' commands.)

@ux(Login Scripts):  The present login scripts implementation follows
the Unix conventions of uucp's "@q(L.sys)" file, rather than the normal
Kermit "INPUT/@|OUTPUT" style.  Volunteers have indicated an intent to
implement the Kermit standard for login scripts, and indeed even others
may be implemented in the future as needed.

@begin(multiple)
@ux(Dial-out vs dial-in communications lines)
C-Kermit requires a dial-out line for the "set line" or "-l" options.
Most systems have some lines dedicated to dial-in, which they enable
"loggers" on, and some lines available for dial-out.  Where a line
must be shared between dial-in and dial-out, several options are
available (though they are, strictly speaking, outside the pervue of
C-Kermit).

A simple shell program can be used to change directionality of the line
if your Unix has the enable(8) and disable(8) commands.  In that
case, the shell program could "grep" a "who" to see if anybody is
logged onto the desired line;  if not, it could "disable" the line.
The shell program will need to be set-uID'ed to root.  The shell
program can be called from kermit prior to a dial command, e.g.,
"! mydisable.shellprog".  Prior to the final "quit" from C-Kermit,
another shell program could be executed to "enable" the line
again.  This program also needs to be set-uID'ed to root.

If your Unix lacks the enable(8) and disable(8) commands, another
common technique works if your system supports the /etc/ttys file.
A shell program could call up an awk program to find the line
in the file and set the enable byte to 0 (to directly disable the
line).  Likewise, it can be reenabled by a counterpart at the end.
It may be necessary to pause for 60 seconds after modifying that
file before the logger sees it and actually disables the line.
@end(multiple)

@begin(multiple)
@ux(Using C-Kermit on Local Area Networks):
C-Kermit can successfully operate at speeds up to 9600 baud over
LANs.  Testing has, however, shown that most processors, whether
PC/XTs with PC/IX, or Vaxes, need flow control at these rates.
A command, "set flow x" should be issued to each end of this form
of connection. 

If the LAN is the sort with an interface card (or box, like the
Sytek), then the interface card/box must be programmed to handle
the flow control characters (xon and xoff) at the card/box level
(rather than forwarding these characters to the remote site).  This
is because packetizing LANs will not deliver the xoff character
to the other end, after packetization, until long after the receive
buffer has been overrun.
@end(multiple)

@ux(Resetting terminal after abnormal termination or kill): When C-Kermit
terminates abnormally (say, for example, by a kill
command issued by the operator) the user may need to reset the terminal state.
If commands do not seem to be accepted at the shell prompt, try
Control-J "stty sane" Control-J (use "reset" on Berkeley Unix).
That should take the terminal out of "raw mode" if it was stuck there.

@ux(Remote host commands may time-out on lengthy activity):
Using "remote host" to instruct the C-Kermit server to invoke Unix
functions (like "make") that might take a long time to produce output can cause
timeout conditions.


@ux(PC/IX Login Scripts -- Unfound Bug):
Though login scripts appear to work properly on most processors, in the
case of the PC/XT with PC/IX, it appears that longer scripts
need to be broken up into shorter scripts (invoked sequentially from
the take file).  This is because the portion of the script handler
which checks if an operation timed out seems to leave the processor
in a strange state (i.e. hung).
@end(enumerate)

@section(How to Build C-Kermit for a Unix System)

The C-Kermit files, as distributed from Columbia, all begin with the
prefix "ck".  You should make a directory for these files and then cd
to it.  A makefile is provided to build C-Kermit for various Unix systems.
As distributed, the makefile has the name "@q(ckermi.mak)".  You should
rename it to "@q(makefile)" and then type "make xxx", where xxx is the
symbol for your system, for instance "make bsd" to make C-Kermit for
4.2 BSD Unix.  The result will be a program called "wermit".  You should
test this to make sure it works; if it does, then you can rename it to
"kermit" and install it for general use.  See the makefile for a list of
the systems supported and the corresponding "make" arguments.

@section(Adapting C-Kermit to Other Systems)

C-Kermit is designed for portability.  The level of portability is indicated
in parentheses after the module name: "C" means any system that has a C
compiler that conforms to the description in "The C Programming Language" by
Kernighan & Ritchie (Prentice-Hall, 1978).  "Cf" is like "C", but also
requires "standard" features like printf and fprintf, argument passing via
argv/argc, and so on, as described in Kernighan & Ritchie.  "Unix" means the
module should be useful under any Unix implementation; it requires features
such as fork() and pipes.  Anything else means that the module is particular
to the indicated system.  The modules are:

@begin(description,leftmargin +4, indent -4)
@q<ckmain.c, ckermi.h, ckdebu.h (Cf)>:@\This is the main program.  It contains
declarations for global variables and 
a small amount of code to initialize some variables and invoke the command
parser.  In its distributed form, it assumes that command line arguments are
passed to it via argc and argv.  Since this portion of code is only several
lines long, it should be easy to replace for systems that have different
styles of user interaction.  The header files define symbols and macros used
by the various modules of C-Kermit.  @q(ckdebu.h) is the only header file
that is included by all the C-Kermit modules, so it contains not only the
debug format definitions, but also any system-@|dependent typedefs.

@q<wart.c (Cf), ckprot.w (C)>:@\The ckprot module embodies the Kermit protocol
state table and the code to accomplish state switching.  It is written in
"wart", a language which may be regarded as a subset of the Unix "lex" lexical
analyzer generator.  Wart implements enough of lex to allow the ckprot module
to function.  Lex itself was not used because it is proprietary (but lex may be
used in place of wart with minor reformatting of @q(ckprot.w) -- removal of
@q(#includes) and comments, which must be reinserted into lex's output C
program).  The protocol module @q(ckprot.w) is read by wart, and a
system-independent C program is produced.  The syntax of a Wart program is
illustrated by @q(ckprot.w), and is described in @q(ckwart.doc).

@q<ckfns.c (C)>:@\The module contains all the Kermit protocol support functions
-- packet formation, encoding, decoding, block check calculation, filename and
data conversion, protocol parameter negotiation, and high-level interaction
with the communication line and file system.
To accommodate small systems, this module has been split into two --
@q(ckfns.c) and @q(ckfns2.c).

@q(ckx???.c) (specific to system ???):@\For instance, @q(ckxunx.c)
(Berkeley, Venix, ATT System III/V, and other Unix systems) -- The ckx
module contains the system-@|dependent primitives for communication line
i/o, timers, and interrupts.  Certain important variables are defined in
this module, which determine whether C-Kermit is by default remote or
local, what the default communication device is, and so forth.  The ckx
module maintains its own private database of file descriptors and modes
for the console terminal and the file transfer communication line so
that other modules (like ckfns or the terminal connect module) need not
be concerned with them.  This module will vary significantly among Unix
implementations since the sgtty/@|ioctl/@|termio functions and their
arguments are among the least compatible areas of Unix.  The @q(ckxunx.c)
file has conditional compilation for the variations of Unix.

@q(ckz???.c) (specific to system ???):@\For instance, @q(ckzunx.c) (Berkeley,
Venix, System III/V Unix) -- The ckz module contains system-dependent
primitives for file i/o, wildcard (meta character) expansion, file existence
and access checking, and system command execution.  It maintains an internal
database of i/o "channels" (file pointers in the case of Unix) for the files
C-Kermit cares about -- the input file (the file which is being sent), the
output file (the file being received), the various logs, the screen, and so
forth.  This module will vary little among Unix implementations except for the
wildcard expansion code, which requires detailed knowledge of the directory
structure.  The ckz module also defines variables containing the system command
strings used to perform certain functions like directory listing, file
deletion, etc.  The @q(ckzunx.c) file has conditional compilation for the
variations of Unix.

@begin(multiple)
@q(ckuser.h, ckuser.c, ckusr2.c, ckusr3.c) (Unix):@\This is the "user
interface" for C-Kermit.  It includes the command parser,
the screen output functions, and console input functions.  The command
parser comes in two pieces -- the traditional Unix command line decoder
(which is quite small and compact), and the interactive keyword parser
(which is rather large).  This module is fully replacable; its interface to
the other modules is very simple, and is explained at the beginning of the
source file.  The ckuser module also includes code to execute any commands
directly which don't require the Kermit protocol -- local file management,
etc.  The module is rated "Unix" because it makes occasional use of the
system() function.

Note that while ckuser is logically one module, it has been split up into
three C source files, plus a header file for the symbols they share in common.
This is to accommodate small systems that cannot handle big modules.
@q(ckuser.c) has the command line and top-level interactive command parser;
@q(ckusr2.c) has the help command and strings; @q(ckusr3) has the set
and remote commands along with the logging, screen, and "interrupt" functions.
@end(multiple)

@q(ckcmd.c, ckcmd.h) (Cf):@\This is an interactive command parsing package
developed for C-Kermit.
It is written portably enough to be usable on any system that has a C
compiler that supports functions like printf.  The file name parsing
functions depend upon primitives defined in the ckz module; if these
primitives cannot be supplied for a certain system, then the filename
parsing functions can be deleted, and the package will still be useful
for parsing keywords, numbers, arbitrary text strings, and so forth.
The style of interaction is the same as that found on the DECSYSTEM-20.

@q(ckconu.c) (Unix):@\This is the connect module.  As supplied, it should
operate in any Unix environment, or any C-based environment that provides the
fork() function.  The module requires access to global variables that specify
line speed, parity, duplex, flow control, etc, but invokes functions from the
ckx module to accomplish the desired settings and input/output, and functions
from the ckz module to perform session logging.  There is no code for
controlling modem signals.  No terminal emulation is performed, but since
standard i/o is used for the console, this may be piped through a terminal
emulation filter.  The ckconu function may be entirely replaced, so long as
the global settings are honored by its replacement.  PC implementations of
C-Kermit may require the ckcon module to do screen control, escape sequence
interpretation, etc, and may also wish to write special code to get the best
possible performance.

@q(ckdial.c) (Unix):@\This is the dialer module.  As supplied, it handles hayes
and ventel modems.

@q(cklogi.c) (Unix):@\This is the login script module.  As supplied, it handles
uucp-@|style logins. 
@end(description)

Moving C-Kermit to a new system entails:
@begin(enumerate)
Creating a new ckx module in C, assembler, or whatever language is
most appropriate for system programming on the new system.

Creating a new ckz module, as above.

If the system is not Unix-like, then a new ckuser module may be required,
as well as a different invocation of it from ckmain.

If the distributed connect module doesn't work or performs poorly, then
it may be replaced.  For instance, interrupt-driven i/o may be required,
especially if the system doesn't have forks.
@end(enumerate)
Those who favor a different style of user/program interaction from that
provided in @q(ckuser.c) may replace the entire module, for instance with one
that provides a mouse/@|window/@|icon environment, a menu/@|function-@|key
environment, etc.

A few guidelines should be followed to maintain portability:
@begin(itemize)
Keep variable and function names to 6 characters or less.

Keep modules small.  For instance, on a PDP-11 it is necessary to keep
the code segment of each module below 8K in order to allow the segment
mapping to occur which is necessary to run programs larger than 64K on a
non-I-&-D-space machine.

Keep strings short; many compilers have restrictive maximum lengths.

Keep (f,s)printf arguments short.  If these exceed some compiler dependent
maximum (perhaps as short as 128, after expansion) memory will be overwritten
and the program will probably core dump.

Do not introduce system dependencies into @q(ckprot.w) or @q(ckfns*.c).
@End(Itemize)
In general, remember that this program will have to be compilable by old
compilers and runnable on small systems.


