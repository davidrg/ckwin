@Part(CKMKER,root="kuser")
@string(-ckmver="@q<0.9(40)>")

@Chapter<MACINTOSH KERMIT>

@case(device,file="@*--------@*
This document is formatted as an ordinary, plain text ASCII disk file, from
SCRIBE text formatter source.  Typeset copies are available from Columbia
University.@*--------@*")

@Begin<Description,spread 0>
@i(Program:)@\Bill Catchings, Bill Schilit, Frank da Cruz (Columbia
University),@*
Davide Cervone (University of Rochester),@*
Matthias Aebi (ECOFIN Research and Consulting, Ltd., Zuerich),@*
Paul Placeway (Ohio State University).

@i(Language:)@\C (MPW)

@i(Documentation:)@\Christine Gianone, Frank da Cruz, Paul Placeway

@i(Version:)@\@value(-ckmver)

@i(Date: )@\May 26, 1988
@end<Description>

@subheading<MacKermit Capabilities At A Glance:>
@begin<format,leftmargin +2,above 1,below 1>
@tabclear()@tabset(3.5inches,4.0inches)
Local operation:@\Yes
Remote operation:@\Yes (server mode only)
Login scripts:@\No
Transfer text files:@\Yes
Transfer binary files:@\Yes
MacBinary transfers:@\No
Wildcard send:@\Yes (whole HFS folders)
File transfer interruption:@\Yes
Filename collision avoidance:@\Yes
Can time out:@\Yes
8th-bit prefixing:@\Yes
Repeat count prefixing:@\Yes
Alternate block checks:@\Yes
Terminal emulation:@\Yes (VT100,VT102)
Communication settings:@\Yes (Speed, Parity, Echo)
XON/XOFF:@\Yes
Transmit BREAK:@\Yes
IBM mainframe communication:@\Yes
Transaction logging:@\Yes
Session logging:@\Yes
Debug logging:@\No
Packet logging:@\No
Act as server:@\Yes
Talk to server:@\Yes
Advanced server functions:@\Yes
Local file management:@\Yes
Command/Init files:@\No
Key redefinition/macros:@\Yes
File attributes packets:@\No
Command macros:@\No
Raw file transmit:@\No
Long packets:@\Yes
Sliding windows:@\No
@end<format>

@section(Introduction)
@index<Apple Macintosh>
@index<Macintosh Kermit>
@index<CKMKER>
 Macintosh Kermit, or "MacKermit", is an implementation of the Kermit file
transfer protocol for the Apple Macintosh family of computers. It was
developed at Columbia University, based on C-Kermit (which also forms the
nucleus of Unix Kermit and many other Kermit programs).  Version 0.9 of
MacKermit runs on the Macintosh 512, XL (Apple Lisa running MacWorks), 512e,
Plus, SE, and II, under the regular Finder and the Multifinder, with which it
can transfer files in the background@index<Background>.  MacKermit 0.9
probably does not run on a 128k (original, classic) Macintosh, due to lack of
sufficient memory, but should run OK on a "fat Mac" (a 128K Mac upgraded to
512K).  Version 0.8 should be retained for 128K Macs.

This manual assumes you are acquainted with your Macintosh, and that you are
familiar with the general ideas of data communication and Kermit file transfer.
A very brief overview is given here, but for details consult the early chapters
of the @i<Kermit User Guide> (of which this document is a chapter), or the book
@ux<Kermit, A File Transfer Protocol>, by Frank @w<da Cruz>, Digital Press
(1987).  For further information about Kermit documentation, updates, lists of
current available versions, and ordering information, write to:
@begin<example,use r,need 5>
Kermit Distribution
Columbia University Center for Computing Activities
612 West 115th Street
New York, NY  10025  (USA)
@end<example>

@Section<Installation>

Before you can use Macintosh Kermit or any other communication program on your
Mac, you must have a way to connect it to the other computers you wish to
communicate with.  This means either a direct cable connection (usually using
a "null modem" cable), or a modem connected to your Mac and to a telephone.
The Macintosh poses two special problems at this level.  First, its connectors
are not the standard 25-pin RS-232 style, but either 9-pin or 8-pin special
connectors which you need special adapters for.  Second, the Macintosh does
not supply a Data Terminal Ready (DTR@index<DTR>) signal, which is required by
most modems before they will operate.  To use your Mac with a modem that is
not designed specifically for the Mac, you have to either (a) configure the
modem to ignore the DTR signal, or (b) feed some other active signal into the
modem's DTR input.  The former is usually done with DIP switches on the modem,
the latter can be done in the connector that plugs into the modem by
installing a jumper wire between DTR (pin 20) and DSR (pin 6), or by
connecting the Mac's +12V output (pin 6 on the Mac's 9-pin connector) to DTR
(pin 20) on the modem end.

If you have received a Macintosh Kermit diskette from Columbia University,
there's no special software installation procedure -- just insert the
diskette, double-click on the appropriate start-up file, or on MacKermit
itself, and go.  If all the communication and other settings agree with your
requirements, there's nothing else you need to do.  This process is
illustrated in the next section, just below.

MacKermit is not copy-protected, and nothing out of the ordinary is required
to make copies onto other diskettes, or onto your hard disk if you have one.
Just use the ordinary Macintosh methods of copying files, folders, etc.

Later, you may wish to create settings files tailored to your communication
environment, and you might also want to customize the keyboard configuration.
Use the various Settings options for this, and then select Save Settings from
the File menu.  Settings and settings files are explained in Sections
@ref<-macset> and @ref<-macsfile>.

@section(Getting Started)
@label<-macstart>

Kermit programs perform two major functions, terminal emulation and file
transfer.  Before transferring files between two systems you must establish a
terminal connection from your system to the other one, either direct or else
dialed up using a modem.  Then to transfer files, login to the remote system
if necessary, start up a Kermit program there, and then tell the two Kermit
programs which files to transfer, and in what direction.

Most Kermit programs present you with a prompt, in response to which you type
a command, repeating the process until you exit from the program.  If you want
to establish a terminal connection to another system, you must give the
CONNECT command.  Unlike these programs, MacKermit is @i<always> connected,
and whatever keystrokes you type are sent to the other system.  To give
commands to MacKermit itself, you must use the mouse@index(Mouse) to pull down
menus from the menu bar that overlays your terminal session, or type special
Command-key equivalents.

The following example shows how to transfer a file with MacKermit.  The remote
computer is a Unix system, but the method is the same with most others.
@begin(itemize)
First insert the MacKermit diskette.  It should appear on the screen as a
diskette icon titled @b<Kermit 0.9(40)>.  Click on it twice to open if it did
not open automatically when you inserted it in the drive.

Once the disk is opened, you will see three MacKermit icons across the top of
the screen.  For the Unix system and most others you can use the "@b(Normal
Settings)" icon -- to start the Kermit program click twice on it.  For linemode
connections to IBM mainframes, you would click twice on the "@b(IBM Mainframe
Linemode Settings)" icon.

You will see a white backround with menus stored under the headings @b(File),
@b(Edit), @b(Settings), @b(Remote), and @b(Log).

Move the mouse pointer to the @b(Settings) menu and select
@b(Communications...) by clicking on it once.

MacKermit normally sets the communication speed to 9600 bits per second.
Click on the circle in front of 1200 (or whatever speed you need to match the
baud rate of your modem and/or remote system).  Check to see that the other
communication settings like parity are as required, and make any necessary
changes.

Click on the "@q<OK>" box to accept the settings.

If you have a Hayes-like dialout modem, follow the next two steps:
@begin<enumerate>
Type AT (uppercase) and then press the Enter key.  The modem should respond
with "OK" or the digit "0" (zero).  If it doesn't, check the cable, the modem,
etc (consult your modem manual for details).

Now type ATDT 7654321 followed by Enter (replace 7654321 by the actual phone
number).  If the connection succeeds, you'll get a message like CONNECT (or the
digit "1"), otherwise you'll see an error message like NO CARRIER, ERROR, etc,
or a digit like 3 or 4 (see your modem manual).
@end<enumerate>
For non-Hayes-compatible modems, follow the instructions in your modem manual.
For direct connections, skip this step altogether.
@end<itemize>
Now you should be talking to the Unix system.  Type a carriage return to get
its attention, and it will issue its login prompt.  In the examples below,
underlining is used to show the parts that you would type.
@begin<example>
@tabclear()@tabset(2.5inches)
Login: @ux<christin>@\@i(Login to the host.)
password:@ux<      >@\@i<(Passwords normally don't echo.)>

% @ux<kermit>@\@i(Run Kermit on the host.)

C-Kermit>@ux<receive>@\@i(Tell it to receive a file.)
@end<example>

Now tell MacKermit what file to send:
@begin<itemize>
Use the mouse to point to the @b(File) menu and select the @b(Send File...)
option.  You can either type in the name of the file (if you know the name) or
select the alternate drive to see what files are on the disk.  Once you see
the file you want to send, click on the filename and then click on the SEND
option (or you can just click twice on the filename).

A "File Transfer Status" box will appear to report the progress of the
transer.  @i(NOTE:) If the number of retries is increasing but not the number
of packets, you should check your @b(Communications...) settings under the
@b(Settings) menu.

When the file transfer is completed, the "File Transfer Status" box should
disappear and the C-Kermit prompt should reappear.  
@end(itemize)

You have just transferred a file from the Macintosh to the Unix system.  To
transfer a file in the other direction, use the "@q(send )@i<filename>"
command on Unix instead of "@q(receive)", and click on "@b(Receive File...)"
from the Mac's @b(File) menu, instead of "@b(Send File...)".

After the file is transferred, your terminal connection is automatically
resumed.  Once your Unix session is complete, you can log out, and then
exit from MacKermit:
@begin<example>
@tabclear()@tabset(2.5inches)
C-Kermit>@ux<exit>

% @ux<^D>@\@i(Logout from Unix by typing Ctrl-D.)
@end<example>

@begin<enumerate,spread 0.5>
Select the @b(Quit) option in the @b(File) menu by clicking on it.

Select the @b(Close) option in the @b(File) menu by clicking on it (assuming
you want to close the current folder).

Select the @b(Eject) option in the @b(File) menu by clicking on it (assuming
you ran Kermit from a diskette that you want to eject).
@end<enumerate>
That's the easiest and quickest way to use Kermit.  If this simple scenario
does not work for you, look for any obvious incorrect settings (speed, parity),
fix them, and try again.  Otherwise, read on.

@Section(The Macintosh File System)

The Macintosh file system consists of one or more disks, each disk containing
files.  There are actually two Macintosh file systems, which work slightly
differently.

Disks formatted for the older Macintosh File System (MFS)@index<MFS> are
essentially "flat".  All files on one of these disks must have a unique name.
Files may be collected together into "folders"@index<Folder>, but folders are
not analogous to directories on other file systems, and no two folders on the
same disk may contain files of the same name; the folders exist only to make
things look nicer in the Finder.  All Macintoshes have support for MFS.

Disks formatted with the newer Hierarchical File System (HFS)@index<HFS> are
not "flat"; each folder is a directory.  There may not be more than one file
with the same name in a single folder, but there may be identically named
files in different folders.

Macintosh file names may contain practically any printable characters,
including space and punctuation -- but colon (@qq[:]) may not be used; it is
used in device names and as the HFS path element separator.

@section(Menus)

The major menus are @b(Apple), @b(File), @b(Edit), @b(Settings), @b(Remote),
and @b(Log).  The @b(Apple) menu gives some information about the program,
including the MacKermit version number and the C-Kermit protocol mudule version
number (useful in reporting bugs).  It also shows statistics about the most
recent file transfer.

The @b(File) menu invokes Kermit's file transfer functions, @b(Send), @b(Get),
and @b(Receive).  It also allows settings to
be saved and restored, and like most Macintosh applications, includes a "quit"
entry for leaving the program, and a "transfer" entry for transferring to
another program without going through the Finder.

The @b(Edit) menu provides support for Macintosh desk accessories that need to
have this menu to do cut and paste.  This menu does not currently do anything
in MacKermit.

The @b(Settings) menu provides dialog boxes for file, communications, and
protocol settings; these will be discussed below.

The @b(Remote) menu has the commands that can be sent to Kermit servers, as
well as an option to turn Macintosh Kermit itself into a server (also
discussed below).

The @b(Log) menu contains commands to start and stop session and transaction
logging.  It also has an entry to dump the current screen image to the session
log, which is only enabled when the session log is open.

@Section<Terminal Emulation>

Before you can transfer files, you have to establish a terminal connection
with the other computer.  You don't have to give MacKermit any special command
to do this, just start the program.  Assuming you have a physical connection,
then the software will use it.  If you think you have a physical connection,
but don't see any results, click on the @b(Settings) menu and select
@b(Communications) to make sure you have the right speed and parity.  If you
have to dial out to make the connection, you must do this yourself -- Mac
Kermit won't do it for you.  Depending on the type of modem, you must either
type dialing commands to it directly (like the Hayes ATDT command in the
example in section @ref<-macstart>), or else dial the phone manually, wait for
an answer and a carrier tone, and then put the modem in data mode.

Once you've made the connection, you can use MacKermit's terminal emulator,
@index<VT102 Emulation>
which conforms to ANSI standard X3.64, providing a subset of the features of
the DEC VT102 terminal (a VT100 with line and character insert and delete
functions added).  The functions provided are sufficient to allow MacKermit to
act as a terminal for the EMACS full-screen editor as it exists on most
timesharing systems, and for most host-@|resident display-oriented applications
that expect to do cursor positioning and editing on the VT100 or VT102 screen,
such as VAX TPU.  MacKermit does not currently support the following VT100/102
functions:
@Begin(Itemize,spread 0)
Double height or double width lines

Blinking

132 columns

DEC-style line wrapping

Control characters embedded in escape sequences

VT52 mode
@End(Itemize)
(this is not an exhaustive list)

The keyboard is set up by default as follows: If your Macintosh has a Control
key (ie. an SE or II), Kermit uses it, and the Command (Fan, Cloverleaf) key
can be used for keyboard equivalents for menus.  If your Mac does not have a
Control key, then the Command key is used as the Control key.  The CAPS LOCK
key forces all alphabetic characters to upper case.  The terminal emulator
sends ESC (escape) when the @qq(`) (accent grave) key is pressed unshifted
(even if your keyboard has an ESC key).  The character @qq(`) can be sent by
typing Control (or Command) and the same key.  The Backspace key sends a
Delete (Rubout) and Control-@|Backspace sends a Backspace.  On the original
Mac keyboards, the main keypad Enter key sends a "short" (250ms) BREAK signal.
The Mac+, Mac SE, and Mac II do not have a main keypad Enter key, so the BREAK
function must be reassigned to another key.

@index<Key Redefinition>
You can modify the keyboard layout any way you like, defining keyboard macros,
defining or moving the Control and Escape keys, etc., using MacKermit's
built-in key configuration features.  Older MacKermits (version 0.8 and
earlier) came with a separate key configuration program called CKMKEY.  This
should not be used, because it does not understand the format of the 0.9 and
later keyboard configuration software.

MacKermit includes a mouse-controlled@index<Mouse> cursor positioning feature
for use during terminal emulation.  If the "Mouse -> Arrow Keys" feature is
turned on (via the @b[Terminal] entry of the @b[Settings] menu), then when the
mouse button is pressed, the program acts as if you typed the VT100 keypad
arrow keys to move the terminal cursor to where the mouse cursor is.
MacKermit does this by sending the absolute strings for arrow keys,
independant of what is bound to the actual arrow keys of the keyboard.

MacKermit sets the Mac hardware to do 8-bit data communication with no
parity@index(Parity), and then correctly sets the parity bit of each character
itself in software, as requested in the @b<Communication> settings menu.  This
has the benefit of avoiding the problem of a machine which requires a
different input parity than it sends back.  MacKermit will correctly receive
all of the characters sent to it, no matter which parity they are.

To allow useful coexistence of desk accessories and Kermit, the terminal
emulation window may be dragged using the drag bar.  If a desk accessory
overlays the emulation window, the emulation window can be clicked upon to move
it in front of the DA, and later dragged to reveal the hidden desk accessory so
that it can be restored to the foreground.  The same thing can be done with
Kermit's own remote response window as well.  Note that Kermit's terminal
emulation window does not accept input when any other window is in the
foreground.

MacKermit uses XON/XOFF (control-Q and control-S) flow control during
terminal emulation and file transfer.  If the other system does not
understand XON/XOFF, problems may result at high speeds.  The terminal
emulator can normally keep up at 9600 baud, and has a very large input
buffer, but after several continuous scrolling screens at this speed, some
characters may be lost.  When running at high baud rates on a system that
does not understand XON/XOFF flow control, either keep your terminal in
page mode, use a text paging program such as Unix "more", or view text with
a non-@|scrolling screen editor.  Also, don't drag the terminal emulation
window while characters are arriving; if you do, the characters may be lost
and the display may become confused.

@index(Session Log)@index(Raw Download)
During terminal emulation, the characters displayed on the screen may also
be saved on the disk.  This allows you to record interactions with the
remote system, or to "capture" files that you can't transfer with Kermit
protocol, for example when the remote system does not have a Kermit program.
Use the @b<Log> menu, and choose session logging to activate this feature.
The result goes into a file called "Kermit Session" in the current folder,
which is always appended to, rather than overwritten.  To create a new session
log, delete or rename the old one first.

The following features are missing from the MacKermit terminal emulator,
and may be added in subsequent releases:
@Begin(Itemize,Spread 0)
Restoration of character attributes such as underlining or highlighting.

Cutting text from screen to clipboard.

Transmission of raw text to host (e.g. pasting to screen).

Screen rollback.

Screen resizing.

Explicit modem or dialer control.

Login scripts.

Printer support.

Ability to use the printer port for terminal emulation.

A way to disable XON/XOFF flow control, or select other flow controls.
@End(Itemize)

@Section(File Transfer)

Like most Kermit programs, MacKermit allows you to send and receive text or
binary files singly or in groups.  It will interact with a remote Kermit
server, and it can act as a server itself.  However, due to the unique nature
of the Macintosh file system, there are some special considerations:
@Begin(Itemize)
@begin<multiple>
@u<Mode> - Text or Binary@index<Binary Files>.  Binary means the data is sent
or stored without modification.  Text means that every carriage return
character (CR) in a Macintosh file is translated to a carriage-return-linefeed
(CRLF) sequence when sending, and every CRLF in an incoming file is turned
into a CR when stored on the Mac disk.  A text file is produced when you save
a file from MacWrite or other applications using the "text only" option; text
files are not associated with any particular Macintosh application and can be
sent in a useful fashion to other kinds of computers.

A word of caution about Macintosh text files: The Macintosh supports an
extended version of ASCII@index<Extended ASCII>, with characters like accented
and umlauted vowels in the 128-255 range.  These characters allow
representation of Roman-based languages other than English, but they do not
follow any of the ISO standards for extended character sets, and thus are only
useful on a Mac.  When transferring text files, you should ensure that either
there are no extended characters in the file, or that the other system can
understand the Mac's 8-bit characters.
@end<multiple>

@u<Fork> - Data or Resource.  Macintosh files may have two
"forks"@index(Fork).  The data fork contains data for an application; the
resource fork contains icons, strings, dialog boxes, and so forth.  For
instance, a MacWrite document contains text and formatting information in the
data fork, and fonts in the resource fork.  For applications, the executable
code is stored in the resource fork.
@End(Itemize)

File transfer is initiated when you select @b(Send file...), @b(Receive
File...), or @b(Get file from server...) from MacKermit's @b(File) menu.

File transfers can be canceled by clicking on the Cancel File or Cancel Group
buttons.  These will always work when sending.  When receiving, they will work
if the opposite Kermit honors this (optional) feature of the protocol.  There
is also an "emergency exit" from any protocol operation, which can be taken at
any time by typing "Command-@q(.)" -- that is, hold down the Command (Fan,
Cloverleaf) key and type period.

The progress of file transfer operations can be logged into a Macintosh file
called a "transaction log".  This log will show the names of the files
transferred, the date and time, and the completion status.  This feature is
useful with long unattended transfers -- you can come back later and read the
transaction log to find out what happened.  The transaction log is called
"Kermit Log".

The current version of Mac Kermit can only send one fork of a file at a time.
When a file has two forks, there is no provision for sending both forks
together.  This restriction may be lifted in future releases of MacKermit, for
example by converting applications to MacBinary@index<MacBinary> format during
transmission.

@subsection(Sending Files)
To send files, first put the remote Kermit in server mode, or else give it the
RECEIVE command.  Then use the mouse to select @b(Send file...) from the
@b(File) menu.  This will give you a MacKermit file-open box, which includes
the standard Macintosh dialog items -- a file list, Disk and Eject buttons,
etc.  You can either send one file at a time, by clicking on its name in the
file list, or send the entire contents of the current HFS folder (for HFS
disks only, of course).  Clicking the Disk button will switch the file list to
another physical disk.  If desired, you can type an alternate name to send the
file under.  When you select a file, MacKermit examines its type; if the type
is APPL, then MacKermit expects to send the resource fork in binary mode,
otherwise the data fork in text mode.  The Mode and Fork radio buttons will
display these choices; you may change them before clicking the Send button.

@subsection(Receiving Files)
You can receive or get multiple files, providing the opposite Kermit is
capable of sending multiple files in a single transaction (most are).  To
receive files, first give the remote Kermit a SEND command and then select
@b(Receive file...) from the @b(File) menu.  To get files from a server,
first put the remote Kermit into server mode, then select the @b(Get file from
server...) option from the @b(File menu), and type in the name of the file you
want to get, or a wildcard designator for multiple files, in the remote
system's filename syntax.

As each file arrives at the Mac, it will be decoded according to the current
mode (text or binary), and stored in the default fork (data or resource).  The
file names will be either the names the files arrive with (overwriting
existing files of the same names) or new unique names (when name conflicts
occur), according to the current default for name collisions.  You may also
elect to perform an "attended" receive, in which you have an opportunity to
override file defaults on a per-file basis (do this in the @b(Protocol)
section of the @b(Settings) menu).  But attended operation must be used with
caution -- if you take too long (more than about a minute) to execute an
incoming file's dialog box, the opposite Kermit could time out and terminate
the transaction.  If this happens, tell the opposite Kermit to send again and
try again with the receive dialog.

The folder for new files is the same as the location of the settings file, or
if no settings file was used then the new files appear on the desktop.  If you
are transferring a lot of files and want to keep them together, create a
folder, drag the settings file into it, and double click on the settings file;
all created files will appear in that folder.

@section(Remote Commands)

When connected to a Kermit server, MacKermit is capable of issuing special
file management and other commands to it.  The @b(Remote) menu contains these
commands.  You may request directory listings, you can delete files, change
directories, etc, on server's machine.  The response from these commands (if
any) is displayed in a special pop-up window.  Responses to multiple Remote
commands are separated by a dashed line.  The response window can be scrolled,
sized, and positioned, and can be hidden by clicking the menu item "@b(Hide
Response)" or the window's go-away box; all text remains intact and will be
appended to the next time you do a Remote command; it can also be brought to
the foreground by clicking the @b(Show Response) menu item.  Note that typein
to the terminal emulator will not take effect when the response window -- or
any other window (such as a desk accessory) -- is up front.  This is not a
bug, but a feature of the Macintosh user interface guidelines.

If the response buffer gets too full (greater than 30,000 characters),
MacKermit will remove enough text from the beginning of the buffer, in 512 byte
chunks, to make it less than 30,000 characters again.

A Remote command can be canceled by taking the Emergency Exit (Command-@q<.>).
To disengage from the remote Kermit server, click on @b(Bye) or @b(Finish) in
the @b(Remote) menu.

@section(Server Operation)

MacKermit may itself act as a Kermit server.  Just set the desired parameters
in the @b(Settings) menu, then click on @b(Be a Server) in the @b(Remote)
menu.  The MacKermit server can respond to SEND, GET, REMOTE DIRECTORY,
FINISH, and BYE commands.  You can send single or multiple files to a
MacKermit server, and you can get a single file from it by name.  You can also
get all the files in the current folder by using a colon (@qq<:>) as the file
specification in the GET command:
@example<GET :>
If you give the FINISH command, MacKermit will return to terminal mode.  If
you give the BYE command, the Macintosh will reboot itself.

You can take MacKermit out of server mode from the Mac keyboard by typing the
emergency exit sequence, Command-dot.

@section(Settings)
@label<-macset>

You can change File, Communications, Protocol, Terminal, Keyboard macros, and
Keyboard modifier settings by using the @b(Settings) pull-@|down menu.  You can
save and load these settings by invoking the appropriate selection in the
@b(File) menu.  If the "bundle bit" has been correctly set on your version of
MacKermit (it should be), then you can double-@|click on the resulting document
to start MacKermit with those settings.

The @b(File) settings establish the defaults for file transfer:
@Begin(Itemize)
@u<Attended> versus @u<Unattended> operation for incoming files.

@u<Naming>: When doing @u(unattended) file reception, whether incoming
files should supersede existing files of the same name, or a new
unique name should be assigned to them.  If the latter, the new name
is formed by adding a dot and a number to the end.  For instance, if a
file called FOO exists and a file called FOO arrives, MacKermit will
store the arriving file as FOO.1; if FOO.1 exists, then FOO.2, etc.

@u<Mode>: text or binary.  Used for received files only.  When sending,
MacKermit tries to figure out an appropriate mode for the file being
sent (but then lets you override it the Send File dialog).

@u<Fork>: which fork -- data or resource -- to send, or to store an incoming
file into.
@End(Itemize)

The @b<Communications> settings allow you to set the baud rate (anywhere
between 300 baud and 57@q<.>6K baud, except 38@q<.>4K baud), and parity (odd,
even, mark, space, or none).  When the parity is set to @u(none) the Macintosh
uses an 8-bit-wide connection.  All other parity settings tell the Macintosh
to use a 7-bit-wide connection, and to request 8th-bit prefixing@index<8th-bit
Prefixing> when transferring 8-bit data.  If the remote host or the
communication path uses any kind of parity, then you won't be able to transfer
files successfully unless you tell MacKermit (and in most cases also the
Kermit on the other end) about it.  Duplex is selected in the @b(Terminal)
settings.

The @b<Protocol> settings allow you to set packet parameters for both incoming
and outbound packets.  These include the block check type (1 or 2 character
checksum, 3-character 16-bit CRC-CCITT), line turnaround handshake
character (for file transfer with half duplex systems), packet start and
end characters, padding, packet length, timeout interval, and packet length.
Characters are specified by entering their ASCII value in decimal, e.g. 1
for Control-A, 13 for Control-M (Carriage Return), etc.  The RECEIVE
parameters are conveyed by MacKermit to the other Kermit.  For instance,
if you set the receive-packet-length to 500, MacKermit will tell the other
Kermit to send 500-character packets.  The SEND parameters are used to
override negotiated values, and need rarely be used.

@index<Long Packets>
Long packets are selected by setting the RECEIVING packet length between 95
and 1000.  Normally, you should not change the sending length because
MacKermit, and most other Kermits, will configure themselves correctly.
Note also that the fastest file transfers will happen with long packets in
the range of 300-500.  Very long packets actually end up being much slower,
because the operating systems in both the Mac and the other machine have to
do more work to cope with such long inputs, and, under noisy conditions,
the probability is higher that a longer packet will be struck by noise, and
will take longer to retransmit.

The @b<Terminal> settings let you modify the characteristics of the VT102
emulator, such as auto-linefeed, autowrap, autorepeat keys, block vs underline
cursor, blinking vs steady cursor, inverted screen (reverse video), and smooth
scrolling.  There is also a "visible bell@index<Bell>" for those who can't
hear the audible bell produced upon receipt of a Control-G, and an option to
display control characters visibly by showing their numeric ASCII values (in
decimal) in a single character cell.  If local echo is needed, as in
half-duplex connections, that must be specified here also.

@Section(Settings Files)
@label<-macsfile>

@index(Settings Files)@index(MacKermit Settings Files)
 You can start MacKermit with all its "factory settings" by double clicking on
the MacKermit icon.  Factory settings are designed for direct communication
with most other microcomputers, DEC minis and mainframes, etc: 9600 bps, no
parity, XON/XOFF, remote echo, etc.  You can change the communication,
protocol, file, keyboard, and terminal settings by going through the options
in the @b(Settings) menu.  Once you have set all parameters as desired, you
can save your settings in a "MacKermit settings file" by selected "@b(Save
Settings...)" from the @b(File) menu.  A settings file is, in Macintosh
terminology, a "MacKermit document".  You'll recognize it because it looks
like a dog-eared piece of paper with the MacKermit icon superimposed.  You can
have more than one settings file.

There are two ways to use a settings file.  First, you can double-click on it,
just as you can double-click on a MacWrite document to start up MacWrite to
edit a particular file.  This method starts up MacKermit with all the saved
settings.  The other method is to click on the "@b(Load Settings...)" option
in the @b(File) menu from inside MacKermit.  This lets you change settings
without leaving and restarting the program.  @b(Load Settings...)  shows all
MacKermit settings files in the selected folder.  Opening one of them loads
all its settings, removing all current settings.

You can "edit" a MacKermit settings file by loading it, going through the
@q(Settings) menu, and then saving the settings either in a new file, or
overwriting the same file.

As distributed by Columbia, Mac Kermit comes with two settings files.  One is
called "Normal Settings", and is pretty much identical to Mac Kermit's factory
settings.  The other is "IBM Mainframe Linemode Settings".  It selects mark
parity, local echo, XON half-duplex line turnaround handshake.  You can
use these files as-is, customize them for your own environment, or create new
settings files for all the different kinds of systems that you use.

@Section<Reconfiguring the Keyboard>

@index(Key Redefinition)
Beginning with version 0.9, MacKermit has keyboard configuration functions
built in.  These are accessed through the @b[Set Key Macros] and the @b[Set
Modifiers] entries in the @b[Settings] menu.

The Macintosh keyboard is composed of normal keys and modifier keys.
Modifier keys are those keys that, when held down, change the meaning of
other keys.  On the Mac these are: SHIFT, CAPS LOCK, OPTION, CONTROL (only
on the Mac II and SE), and COMMAND (also known as APPLE, CLOVER, or FAN).
Normal keys are the letters, numbers, special symbols, arrow keys, space
bar, and function keys.  Only one normal key can be typed at a time, but
one or more modifier keys can be pressed down along with it.

When you type a key, Kermit reads both the ASCII value, and the
keyboard-independent scan code for that key.  Kermit looks in its table of key
macros to see if there is a macro for this combination of key and modifiers,
and if so sends the macro.  If there is no macro, Kermit then looks in its
modifier table to see if any of the modifiers do special things to the
character; if so, it does these to the character.  Finally, Kermit sends the
character.  In the normal case when there is no macro and no modifiers apply,
the character sent is simply the ASCII value for that character.

It is important to keep in mind that if the parity setting is something other
than @u[none], the high (8th) bit will be stripped off of the characters when
they are transmitted.  Since most systems do not understand characters in the
range 128 -- 255 (decimal), you should avoid using the Apple extended
characters (accented vowels, for example) during terminal connection.

@SubSection<Defining Key Macros>

To define a new key macro, select the @b[Key Macros] entry.  A dialog window
will appear, asking you to press the key to define.  Type the key (including
any of the modifiers).  A new dialog will appear, with an editable text field
in it.  Enter the definition for the key here.  Your definition may be up to
255 characters long, and can include all of the control characters (including
NUL).  Special characters can be included in the macro by entering a @qq<\>
(backslash), followed by up to 3 @i<octal> (base 8) digits for the value (just
like in the C programming language).  For example, an ASCII NUL (value 0)
would be written as @qq<\000>, carriage return (ASCII 13) would be written
@qq<\015> (1 x 8 + 5 = 13).  Also, control characters may be entered with a
backslash, followed by a caret (or circumflex, @qq<^>), followed by the
corresponding letter.  Thus a Control-G (value 7) could be entered as
@qq<\007>, @qq<\^G>, or @qq<\^g>.  To include a literal backslash in a
definition, type in two of them: @qq<\\>.

@index<BREAK>
BREAK conditions are also programmable as macros.  If the entire macro the
string is @qq<\break>, then typing the defined key will send a short (1/4
second) break.  A long (3.5 second) BREAK is defined with @qq<\longbreak>.
Note that a macro can define either a BREAK, or a string of normal characters,
but not both.

@SubSection<Defining Key Modifiers>

Skip ahead to the next section if you already know about things like SHIFT,
CAPS LOCK, CONTROL, and META.

On a typewriter the only modifier key is SHIFT.  Typing a character with no
modifier key depressed selects a lowercase letter or the character printed on
the lower face of the keytop (say, the digit "4").  Typing a character with
SHIFT depressed selects an uppercase letter or the character printed on the
upper face of the keytop (say, a dollar sign).  Some keyboards also have a
SHIFT LOCK key, which stays down once pressed and pops up the next time it's
pressed; its operation is equivalent to holding down SHIFT.  And some keyboards
have a CAPS LOCK key which operates like SHIFT LOCK, but only upon letters.

Computer terminals also have a modifier key called CONTROL (or CTRL).  Its
function is a little less obvious: it is intended to produce one of the 33
characters in the "control range" of the ASCII alphabet.  Control
characters are not graphic -- they are intended for use as format effectors
(like carriage return, formfeed, tab, backspace), for transmission control,
or for device control.  The remaining 95 characters -- letters, digits,
punctuation, and space -- are the graphic characters.  When a character is
typed with the CONTROL modifier pressed, its "control equivalent" (if any) is
transmitted.  By convention, the control equivalent of A is Control-A, B is
Control-B, etc, and there are also seven special control characters
generally associated with punctuation characters or special keys.  For the
"alphabetic" control characters Control-A through Control-Z, SHIFT or CAPS
LOCK modifiers are ignored; for the others, operation varies from terminal
to terminal.

The SHIFT and CONTROL modifiers allow all 128 ASCII characters to be sent
from a normal typewriter-like keyboard that has about 50 keys.  However,
certain host-resident computer applications -- notably the full screen text
editor EMACS and its descendents -- can be used to greater advantage with a
256 character 8-bit alphabet (EMACS responds to single-character commands,
and the more characters a terminal can send, the more commands are directly
available).

@index<META Key>
For this purpose, some terminals also provide a META modifier key.  This key
simply causes the high-order ("8th") bit of the selected 7-bit ASCII value to
be set to 1 upon transmission.  This can only work when the connection is
8-data-bits-@|no-parity.  When parity is in use, EMACS allows a sequence of
two 7-bit ASCII characters to represent a single meta character.  The
advantage of having a real META modifier key is that it can be held down while
the actual key is struck repeatedly or even autorepeats, whereas a use of a
"meta prefix" such as <escape> requires much more typing.  To illustrate,
suppose META-F is the command to go forward one word.  If you want to execute
this operation repeatedly, just hold down META and F and let it autorepeat.
If you don't have a META key, then you'd have to type
<escape>F@|<escape>F@|<escape>F..., etc.

A common problem faced by computer users who switch from one terminal or PC
to another is the placement of the modifiers and other special keys.
DEC, IBM, Apple, and other manufacturers consistently move these keys
around on new models of their keyboards.  MacKermit allows you to assign
any of various functions to any of the Mac's modifier keys, and to assign
any desired character or character sequence to the regular keys, so that
you can tailor the layout of your Mac's keyboard to suit your taste.

@subsection<Modifiers Dialog>

To change the action of any of the modifier keys, select @b[Modifiers]
from the @b[Settings] menu.  A dialog will appear that looks roughly
like the one in Figure @ref<-macmkey> (the @qq(%) represents
the Apple or Clover key).

@Begin(Figure)
@bar()
@blankspace(1)
@begin(example,group)
     Modifier Pattern:  -->   Modification:

  Ctrl Opt Lock Shift  %      Unmodify Caps Ctrl Meta Prefix string:
                                                       ____________
  [X]  [ ] [ ]  [ ]   [ ]   |   [ ]    [ ]  [X]  [ ]  [____________]
  [ ]  [ ] [ ]  [ ]   [X]   |   [ ]    [ ]  [X]  [ ]  [____________]
  [ ]  [X] [ ]  [ ]   [ ]   |   [x]    [ ]  [ ]  [ ]  [@ux<\033        >]
  [ ]  [ ] [ ]  [ ]   [ ]   |   [ ]    [ ]  [ ]  [ ]  [____________]

                (Cancel)   (Help)   ( OK )
@end(example)
@caption(MacKermit Key Modifier Dialog)
@tag<-macmkey>
@bar()
@End(Figure)

The check boxes are divided into rows, each one describing a modification.
The left half of each row describes the modifier combination to look for; a
checked box means that this key is down, and an unchecked box means "don't
care".  Note that there is no way to specify a key being up, and lines with
nothing checked on the left side will be ignored; the character will be
modified in the normal Macintosh way.

The right half describes what modification to do to the characters.  The
Unmodify modification says "make this the character that would be sent from the
same key with no modifer keys pressed".  In other words, un-Option, un-Caps,
un-Control, and un-Shift this character.  The Caps modification translates all
letters to upper case, Ctrl makes the letter a contol character, Meta sets the
high (8th) bit on the character, and if a Prefix string is present, it is sent
before the character is.

@i<Hints about modifiers:>
@begin<itemize>
Beware of the Option key.  It changes the value of any characters you use with
it.  If you type Option-F, the Mac will send a D, if you type Option-B, the
Mac will send a @qq(:), etc.  If you want to use the option key as a modifier,
be sure to check the "Unmodify" box.

To use MacKermit with a version of EMACS that does not accept 8-bit Meta
characters, define a key, like Option, to be unmodified, with a prefix string
of @q(\033) (ASCII Escape), as in Figure @ref<-macmkey>.  Then you can hold
down Option and type F (or any other key) repeatedly, or let it autorepeat,
and MacKermit will send the correct prefix-Meta sequence.

When interpreting a keystoke, MacKermit checks the list of modifiers from
top to bottom, applying the first one that matches.  This means that if you
want a different modifier for Command-Option and just plain Command, you
must put the definition for Command-Option first in the list.
@end<itemize>

@section<Bootstrapping>
@index<Bootstrapping MacKermit>
This section applies if you do not have a MacKermit diskette, but MacKermit is
available for downloading from some other computer.

MacKermit is distributed in source form for building on a Macintosh,
running Apple's Macintosh Programmers Workbench (in MPW C), in
@index<Binhex> @q(.HQX) "BinHex 4" form, and sometimes also as a binary
resource file.  Those who want to work from the source are referred to the
file @q(CKMKER.BLD) for instructions.

If you're downloading, it's best to work with @q<CKMKER.HQX>, a textual
encoding of the MacKermit application.  Download this using any technique
available to you -- an old release of Kermit, an Xmodem implementation, even
raw screen capture.  Then run @index<BinHex> BinHex (version 4) to convert it
into a working application (select @b(Upload -> Application) from the @b(File)
menu).  Eveything will be set up correctly -- icons, forks, etc.

If you don't have the @q<.HQX> file available, but you do have access to the
binary resource file (its name will be @q(CKMKER.RSRC), @q(ckmker.rsrc),
@q(CKMKER.RSR), @q(ckmker.rsr), @q(%ckmker) or some variation on these,
depending on what system it's stored on and how it got there), AND if you have
"MacPut" on your system and MacTerminal on your Mac, AND if you have an
8-bit-wide (no parity) data path between your Mac and your system, then you
can use MacPut to download the binary resource file to your Mac using
MacTerminal's "MacBinary" format (a variant of XMODEM).  After doing this you
must use a program such as @index<Setfile> SetFile or @index<ResEdit> ResEdit
on the Mac to set the author to KR09, the type to APPL, and turn on the bundle
bit.  Do not bother with the CKMKEY program, as it is not used with newer
MacKermits.  If you have an earlier release of MacKermit, you may use it in
place of MacTerminal and MacPut.

@section(Differences Between Versions 0.8 and 0.9)

MacKermit 0.8(34) runs on the 128K Mac, the 512K Mac, and the Mac Plus,
but not on the Macintosh II or SE.  MacKermit 0.9(40) runs on all Macs except
the 128K original.  You should use version 0.9 unless you have a 128K Mac.

The second major difference is that the program is has been translated into
Apple MPW C, so that it can be edited, compiled, and built on the Macintosh
itself.  This was done originally by Jim Noble of Planning Research
Corporation, who converted MacKermit from SUMACC C (which had to be cross
compiled on a UNIX system) to Megamax C.  Jim's version was converted to MPW
C by Matthias Aebi, who also added most of the new features listed below.
Paul Placeway integrated the program with the current (long packet) version
of C-Kermit and added additional new features.

Besides these important differences, there were many other changes from version
0.8 to version 0.9, including:
@begin<itemize,spread 0.2>
The Cursor with open desk accessories now works correctly

Long packet support

New program icon

New settings files are no longer TEXT

Settings can now be written back to an already existing settings file

Key redefinition function built in to Kermit, no more CKMKEY

Server mode directory listing feature

Multifile (folder) send

Server "Delete" file command

Server "Space" command

Get whole folder content from the server with filename @qq<:>

Recognition of all the different Mac keyboards

Support of menu command keys (key macros)

Terminal settings dialog separated from communication settings

Non-transparent terminal mode

Display of statistics and protocol version to "About Kermit" dialog.

Parity problems fixed

Session logging

Transaction logging

Multifinder support

Additions to the VT102 emulator (smooth scrolling, etc)

Rearrangement of menus and displays

Program no longer hangs if remote response window gets too full

Program now works correctly on 64K ROM machines

A new manual
@end<itemize>
This manual applies in large part to version 0.8(34), except that the
older version is missing the new features listed above, and it comes in two
pieces: CKMKER and CKMKEY.  The CKMKEY program is used to program the
keys, like the @b(Set Key Macros...) and @b(Set Modifiers) described in
this manual, and creates a settings file which Kermit itself uses.  The old
version only works well with early Macintosh keyboards.
