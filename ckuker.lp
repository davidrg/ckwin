


                                                       KERMIT(1C)



NAME
     kermit - kermit file transfer

SYNOPSIS
     kermit [ option ...] [file ...]

DESCRIPTION
     Kermit is a file transfer program that allows files to be
     moved between machines of many different operating systems
     and architectures.  This man page describes version 4E(072)
     of the program.  See the C-Kermit documentation (ckuker.doc)
     or the Kermit book for further information.

     Arguments are optional. If Kermit is executed without argu-
     ments, it will enter command mode. Otherwise, kermit will
     read the arguments off the command line and interpret them.

     The following notation is used in command descriptions:

     fn      A Unix file specification, possibly containing
             either of the "wildcard" characters '*' or '?' ('*'
             matches all character strings, '?' matches any sin-
             gle character).

     fn1     A Unix file specification which may not contain '*'
             or '?'.

     rfn     A remote file specification in the remote system's
             own syntax, which may denote a single file or a
             group of files.

     rfn1    A remote file specification which should denote only
             a single file.

     n       A decimal number, in most cases between 0 and 94.

     c       A decimal number between 0 and 127 representing the
             value of an ASCII character.

     cc      A decimal number between 0 and 31, or else exactly
             127, representing the value of an ASCII control
             character.

     [ ]     Any field in square braces is optional.

     {x,y,z} Alternatives are listed in curly braces.

     Kermit command line options may specify either actions or
     settings.  If Kermit is invoked with a command line that
     specifies no actions, then it will issue a prompt and begin
     interactive dialog.  Action options specify either protocol
     transactions or terminal connection.



                                                                1






KERMIT(1C)



     COMMAND LINE OPTIONS

     -s fn   Send the specified file or files. If fn contains
             wildcard (meta) characters, the Unix shell expands
             it into a list.  If fn is '-' then Kermit sends from
             standard input, which may come from a file:

                         kermit -s - < foo.bar

             or a parallel process:

                         ls -l | kermit -s -

             You cannot use this mechanism to send terminal
             typein.  If you want to send a file whose name is
             "-" you can precede it with a path name, as in

                         kermit -s ./-

     -r      Receive a file or files.  Wait passively for files
             to arrive.

     -k      Receive (passively) a file or files, sending them to
             standard output.  This option can be used in several
             ways:

                     kermit -k

             Displays the incoming files on your screen; to be
             used only in "local mode" (see below).

                     kermit -k > fn1

             Sends the incoming file or files to the named file,
             fn1. If more than one file arrives, all are con-
             catenated together into the single file fn1.

                     kermit -k | command

             Pipes the incoming data (single or multiple files)
             to the indicated command, as in

                     kermit -k | sort > sorted.stuff

     -a fn1  If you have specified a file transfer option, you
             may specify an alternate name for a single file with
             the -a option.  For example,

                     kermit -s foo -a bar

             sends the file foo telling the receiver that its
             name is bar.  If more than one file arrives or is
             sent, only the first file is affected by the -a
             option:

                     kermit -ra baz




2






                                                       KERMIT(1C)



             stores the first incoming file under the name baz.

     -x      Begin server operation.  May be used in either local
             or remote mode.

     Before proceeding, a few words about remote and local opera-
     tion are  necessary.  Kermit is "local" if it is running on
     a PC or workstation that you are using directly, or if it is
     running on a multiuser system and transferring files over an
     external communication line - not your job's controlling
     terminal or console.  Kermit is remote if it is running on a
     multiuser system and transferring files over its own con-
     trolling terminal's communication line, connected to your PC
     or workstation.

     If you are running Kermit on most systems, Kermit runs in
     remote mode by default, so on a PC or workstation, you will
     have to put it into local mode.  The following command sets
     Kermit's "mode":

     -l dev  Line - Specify a terminal line to use for file
             transfer and terminal connection, as in

                         kermit -l /dev/ttyi5

     When an external line is being used, you might also need
     some additional options for successful communication with
     the remote system:

     -b n Baud - Specify the baud rate for the line given in the
          -l option, as in

                      kermit -l /dev/ttyi5 -b 9600

          This option should always be included with the -l
          option, since the speed of an external line is not
          necessarily what you might expect.

     -p x Parity - e, o, m, s, n (even, odd, mark, space, or
          none).  If parity is other than none, then the 8th-bit
          prefixing mechanism will be used for transferring 8-bit
          binary data, provided the opposite Kermit agrees. The
          default parity is none.

     -t   Specifies half duplex, line turnaround with XON as the
          handshake character.

     The following commands may be used only with a Kermit which
     is local - either by default or else because the -l option
     has been specified.

     -g rfn  Actively request a remote server to send the named
             file or files; rfn is a file specification in the



                                                                3






KERMIT(1C)



             remote host's own syntax.  If fn happens to contain
             any special shell characters, like '*', these must
             be quoted, as in

                         kermit -g x\*.\?

     -f      Send a 'finish' command to a remote server.

     -c      Establish a terminal connection over the specified
             or default communication line, before any protocol
             transaction takes place.  Get back to the local sys-
             tem by typing the escape character (normally
             Control-Backslash) followed by the letter 'c'.

     -n      Like -c, but after a protocol transaction takes
             place; -c and -n may both be used in the same com-
             mand.  The use of -n and -c is illustrated below.

     On a timesharing system, the -l and -b options will also
     have to be included with the -r, -k, or -s options if the
     other Kermit is on a remote system.

     If kermit is in local mode, the screen (stdout) is con-
     tinously updated to show the progress of the file transer.
     A dot is printed for every four data packets, other packets
     are shown by type (e.g. 'S' for Send-Init), 'T' is printed
     when there's a timeout, and '%' for each retransmission.  In
     addition, you may type (to stdin) certain "interrupt" com-
     mands during file transfer:

          Control-F:  Interrupt the current File, and go on to
          the next (if any).

          Control-B:  Interrupt the entire Batch of files, ter-
          minate the transaction.

          Control-R:  Resend the current packet

          Control-A:  Display a status report for the current
          transaction.


     These interrupt characters differ from the ones used in
     other Kermit implementations to avoid conflict with Unix
     shell interrupt characters.  With System III and System V
     implementations of Unix, interrupt commands must be pre-
     ceeded by the escape character (e.g. control-\).

     Several other command-line options are provided:

     -i      Specifies that files should be sent or received
             exactly "as is" with no conversions.  This option is
             necessary for transmitting binary files.  It may



4






                                                       KERMIT(1C)



             also be used to slightly boost efficiency in Unix-
             to-Unix transfers of text files by eliminating
             CRLF/newline conversion.

     -e  n   Specifies the (extended) receive-packet length, a
             number between 10 and about 1000 (depending on the
             system).  Lengths of 95 or greater require that the
             opposite Kermit support the long packet protocol
             extension.

     -w      Write-Protect - Avoid filename collisions for incom-
             ing files.

     -q      Quiet - Suppress screen update during file transfer,
             for instance to allow a file transfer to proceed in
             the background.

     -d      Debug - Record debugging information in the file
             debug.log in the current directory.  Use this option
             if you believe the program is misbehaving, and show
             the resulting log to your local Kermit maintainer.

     -h      Help - Display a brief synopsis of the command line
             options.

     The command line may contain no more than one protocol
     action option.

     INTERACTIVE OPERATION

     Kermit's interactive command prompt is "C-Kermit>". In
     response to this prompt, you may type any valid command.
     Kermit executes the command and then prompts you for another
     command.  The process continues until you instruct the pro-
     gram to terminate.

     Commands begin with a keyword, normally an English verb,
     such as "send".  You may omit trailing characters from any
     keyword, so long as you specify sufficient characters to
     distinguish it from any other keyword valid in that field.
     Certain commonly-used keywords (such as "send", "receive",
     "connect") have special non-unique abbreviations ("s" for
     "send", "r" for "receive", "c" for "connect").

     Certain characters have special functions in interactive
     commands:

     ?       Question mark, typed at any point in a command, will
             produce a message explaining what is possible or
             expected at that point.  Depending on the context,
             the message may be a brief phrase, a menu of key-
             words, or a list of files.



                                                                5






KERMIT(1C)



     ESC     (The Escape or Altmode key) - Request completion of
             the current keyword or filename, or insertion of a
             default value.  The result will be a beep if the
             requested operation fails.  Tab does the same thing.

     DEL     (The Delete or Rubout key) - Delete the previous
             character from the command.  You may also use BS
             (Backspace, Control-H) for this function.

     ^W      (Control-W) - Erase the rightmost word from the com-
             mand line.

     ^U      (Control-U) - Erase the entire command.

     ^R      (Control-R) - Redisplay the current command.

     SP      (Space) - Delimits fields (keywords, filenames,
             numbers) within a command.

     CR      (Carriage Return) - Enters the command for execu-
             tion.  LF (Linefeed) or FF (formfeed) may also be
             used for this purpose.

     \       (Backslash) - Enter any of the above characters into
             the command, literally.  To enter a backslash, type
             two backslashes in a row (\\).  A single backslash
             immediately preceding a carriage return allows you
             to continue the command on the next line.


     You may type the editing characters (DEL, ^W, etc) repeat-
     edly, to delete all the way back to the prompt.  No action
     will be performed until the command is entered by typing
     carriage return, linefeed, or formfeed.  If you make any
     mistakes, you will receive an informative error message and
     a new prompt - make liberal use of '?' and ESC to feel your
     way through the commands.  One important command is "help" -
     you should use it the first time you run Kermit.

     Interactive Kermit accepts commands from files as well as
     from the keyboard.  Upon startup, Kermit looks for the file
     .kermrc in your home or current directory (first it looks in
     the home directory, then in the current one) and executes
     any commands it finds there.  These commands must be in
     interactive format, not Unix command-line format.  A "take"
     command is also provided for use at any time during an
     interactive session.  Command files may be nested to any
     reasonable depth.

     Here is a brief list of Kermit interactive commands:

     ! command   Execute a Unix shell command.  A space is
                 required after after the !.



6






                                                       KERMIT(1C)



     % text      A comment.  Useful in take-command files.

     bye         Terminate and log out a remote Kermit server.

     close       Close a log file.

     connect     Establish a terminal connection to a remote sys-
                 tem.

     cwd         Change Working Directory (also, "cd").

     dial        Dial a telephone number.

     directory   Display a directory listing.

     echo        Display arguments literally.  Useful in take-
                 command files.

     exit        Exit from the program, closing any open logs.

     finish      Instruct a remote Kermit server to exit, but not
                 log out.

     get         Get files from a remote Kermit server.

     hangup      Hang up the phone.

     help        Display a help message for a given command.

     log         Open a log file - debugging, packet, session,
                 transaction.

     quit        Same as 'exit'.

     receive     Passively wait for files to arrive.

     remote      Issue file management commands to a remote Ker-
                 mit server.

     script      Execute a login script with a remote system.

     send        Send files.

     server      Begin server operation.

     set         Set various parameters.

     show        Display values of 'set' parameters, program ver-
                 sion, etc.

     space       Display current disk space usage.

     statistics  Display statistics about most recent transac-
                 tion.

     take        Execute commands from a file.


     The 'set' parameters are:

     block-check          Level of packet error detection.

     delay                How long to wait before sending first
                          packet.

     duplex               Specify which side echoes during



                                                                7






KERMIT(1C)



                          'connect'.

     escape-character     Character to prefix "escape commands"
                          during 'connect'.

     file                 Set various file parameters.

     flow-control         Communication line full-duplex flow
                          control.

     handshake            Communication line half-duplex tur-
                          naround character.

     line                 Communication line device name.

     modem-dialer         Type of modem-dialer on communication
                          line.

     parity               Communication line character parity.

     prompt               Change the Kermit program's prompt.

     receive              Set various parameters for inbound
                          packets.

     retry                Set the packet retransmission limit.

     send                 Set various parameters for outbound
                          packets.

     speed                Communication line speed.


     The 'remote' commands are:

     cwd         Change remote working directory (also, "remote
                 cd").

     delete      Delete remote files.

     directory   Display a listing of remote file names.

     help        Request help from a remote server.

     host        Issue a command to the remote host in its own
                 command language.

     space       Display current disk space usage on remote sys-
                 tem.

     type        Display a remote file on your screen.

     who         Display who's logged in, or get information
                 about a user.

FILES
     $HOME/.kermrc  Kermit initialization commands
     ./.kermrc      more Kermit initialization commands

SEE ALSO
     cu(1C), uucp(1C)
     Christine Gianone, Kermit User's Guide, Columbia University, 7th
     Edition



8






                                                       KERMIT(1C)



     Frank da Cruz,
          Kermit, A File Transfer Protocol, Digital Press (1987)

     The file ckuker.doc.

DIAGNOSTICS
     The diagnostics produced by Kermit itself are intended to be
     self-explanatory.

BUGS
     See recent issues of the Info-Kermit digest (on the Inter-
     network, BITNET, USENET, etc), or the file ckuker.bwr, for a
     list of bugs.  See the file ckuker.upd for descriptions of
     bugs that were fixed in each release.









































                                                                9



