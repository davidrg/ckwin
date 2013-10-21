:userdoc.
:title.C-Kermit's Dialing Directory
:docprof toc=12345.
:h1 id=preface.Preface
  ---   PREPUBLICATION OUTLINE/DRAFT   ---
:p.
  Copyright (C) 1995, Frank da Cruz and Christine M. Gianone,
.br
  All rights reserved.
:p.
Most recent update&colon. Sun Jan 28 13&colon.34&colon.02 1996
:p.
:hp4.
THIS IS AN UPDATE TO CHAPTER 3, "GETTING CONNECTED", OF "USING C-KERMIT",
AND A SUPPLEMENT TO THE "KERMIT 95" MANUAL.
:ehp4.
:p.
:h1 id=note.Important Notes to Users of Kermit 95 and Kermit/2
This file describes the dialing mechanisms that are invoked by the
the point-and-click K-95 Dialer.  If the Dialer works satisfactorily
for you, you should not have to read this file unless you are curious
about what's going on behind the scenes.  Also, please be aware of the
distinction between the K-95 Dialer, which has its own database of
connections, and Kermit's dialing and network directory files, which are
simply "phone lists" or "address lists", some of which are used by the
Dialer, and others are furnished for your reference.
:p.
When a K-95 Dialer dialup entry (telephone icon) has a "phone number" that
starts with a letter (rather than a digit or a plus sign), that means that
it will be looked up in the specified Kermit dialing directory file(s) and
replaced by the phone number(s) found there.  This is normally done when
we have more than one phone number for the same service.  For example, we
have over 1400 numbers for Tymnet, and so the "Tymnet" entry builds a list
of all the numbers and tries them (normally) in "cheapest-first" order
until one of them answers.  This file contains the details of how it all
works.
:h1 id=dirfiles.Dialing Directory Files
Kermit's dialing directory is a plain-text file that associates names with
phone numbers, allowing you to dial by name so you don't have to remember
specific numbers.  For example, you can type "dial mcimail" rather than
"dial 18004566245".
:p.
This file can be maintained by a text editor that reads and saves plain-text
files in plain-text (ASCII) format, such as Windows EDIT or WordPad.

:h1 id=notation.Notation

The following notation is used in this document&colon.
:xmp.
:font facename=Courier size=10x8.
<xxxx> = parameter to be replaced by a real value (number, filename, etc)
[    ] = items inside brackets are optional, e.g. [ <number> ]
{    } = encloses a comma-separated list of options, e.g. { ON, OFF }
:font facename=default size=0x0.
:exmp.

:h1 id=dirformat.Dialing Directory Format

Each entry consists of a name followed by a telephone number, followed by an
optional comment.  The name is a "word" that does not contain any spaces(*);
alphabetic case distinctions in names are ignored.  The number can be in any
format at all that is acceptable to your dialout modem, meaning, in most
cases, it can include spaces and/or certain punctuation.  The comment, if any,
begins with a semicolon preceded by at least one space or tab.
:xmp.
:font facename=Courier size=10x8.
IBM       +1 (800) 874 2881    ; IBM BBS
Hayes      1 800 874-2937      ; Hayes Online BBS
DJNR       1 212 7418100       ; Dow Jones News Retrieval via NYC Sprint
DECstore   18002341998         ; Digital Equipment Corporation store
Heise      011 49 511 5352301  ; Verlag Heinz Heise c't BBS, Germany
:font facename=default size=0x0.
:exmp.
:p.
(*) A name may contain spaces if it is surrounded by braces, for example&colon.
:xmp.
:font facename=Courier size=10x8.
{Heise MailBox}   011 49 511 5352301
:font facename=default size=0x0.
:exmp.
:p.
Full-line comments may also be entered.  These begin with a semicolon.  Blank
lines in the dialing directory are ignored.

:h1 id=portable.Portable and Literal Entries

Each entry can "portable" or "literal".  In portable entries, phone numbers
start with a plus sign (+), as in the first entry in the sample directory
above.  The advantage of a portable-mode entry is that the same entry can be
used from different locations -- the drawback is that it in order to use it,
you must first tell Kermit where you are (usually just two commands).
:p.
The advantage of literal mode is that it is very easy to use.  When you use
the directory in this way, the number is dialed exactly as given in the
directory and you need not be concerned with any of the locale-related
features described below.  Literal mode is perfectly adequate for
directories that are always used from the same place.

:h1 id=whynot.Why You Might Not Want to Use Literal Mode

The problem with literal mode is that if you are going to be using the same
dialing directory to call the same number from different locations, you'll
need multiple entries.  For example, for the Dow Jones / Sprint number&colon.
:xmp.
:font facename=Courier size=10x8.
DJNRNYC     741 8100               ; From within NYC
DJNRNYCPBX  9,741 8100             ; From a PBX in NYC
DJNRNA      1 212 741 8100         ; From elsewhere in North America
DJNRNAPBX   93,1 212 741 8100      ; From a different PBX outside NYC
DJNRUK      00 1 212 741 8100      ; From the UK
DJNRUKHR    102 00 1 212 741 8100  ; From a hotel room in the UK
:font facename=default size=0x0.
:exmp.
:p.
Thus you'll have to remember which name depending on where you are calling
from.  Or else you will need multiple dialing directories, one for each place
that you call from, which becomes problematic when numbers change.

:h1 id=portmode.Portable Mode

Portable mode is for people who travel, or for dialing directories that are
to be distributed to and used from diverse locations, such as the sample
dialing directory that you received with Kermit.
:p.
By entering the phone number in a specific format, and providing some
additional "locale" information, you enable Kermit to figure out, in most
cases, whether a call is internal, local, toll-free, long-distance, or
international, and then dial it in the appropriate way automatically.
:p.
The format for a portable entry is&colon.
:xmp.
:font facename=Courier size=10x8.
+ <country-code> ( <area-code> ) <local-number>
:font facename=default size=0x0.
:exmp.
:p.
For example&colon.
:xmp.
:font facename=Courier size=10x8.
IBM        +1 (800) 874 2881    ; IBM BBS
Hayes      +1 (800) 874 2937    ; Hayes Online BBS
DJNR       +1 (212) 741 8100    ; Dow Jones News Retrieval NYC
DECstore   +1 (800) 234 1998    ; Digital Equipment Corporation store
Heise     +49 (511) 535 2301    ; Verlag Heinz Heise c't BBS, Germany
:font facename=default size=0x0.
:exmp.
:p.
The plus sign (+) means two things.  First, it means that the phone number is
in a standard format that Kermit can parse.  Second, it means that the digits
between the plus sign and the opening parenthesis are a country code.  And
then&colon.
:ul.
:li.The number in parentheses is the area code within the country.
:li.The number following the number in parentheses is the local number
within the area.
:li.The plus sign and parentheses are discarded before passing the number
to the modem.
:eul.
:p.
The use of punctuation characters other than plus sign, parentheses, and space
is not recommended unless you know what you are doing, nor is the use of
characters other than digits in the phone number.  Such characters are passed
to the modem literally, so the result depends on your modem.  For example,
most Hayes and compatible modems ignore hyphens and periods in phone numbers,
so the following notation works with Hayes modems, but might not work with
other kinds&colon.
:xmp.
:font facename=Courier size=10x8.
+1 (212) 555-1212  ; Phone number contains hyphen
+1 (212) 555.1212  ; Phone number contains period
:font facename=default size=0x0.
:exmp.

:h1 id=mixedmode.Mixed-Mode Directories

You can mix literal- and portable-mode entries in the same directory.  Each
entry is treated individually, depending on whether it starts with a plus
sign.

:h1 id=mentries.Multiple Entries

If you have multiple entries with the same name, then each such entry is tried
until the phone is answered successfully or until no more entries remain.
These entries need not be adjacent, thus the directory can be sorted and
shuffled any way you like.
:p.
Entry names can be abbreviated in the DIAL command, as long as there are no
entries in the dialing directory that start with the same abbreviation but
which are different from each other.  For example, if your directory has
entries with the following names&colon.
:xmp.
:font facename=Courier size=10x8.
aardvark
abba
abccorp
abdicate
abccorp
acorn
:font facename=default size=0x0.
:exmp.
:p.
then "abc" would select the two "abccorp" entries, but "ab" would be
ambiguous and the lookup would fail.

:h1 id=mdirs.Multiple Directories

You can have more than one dialing directory active at once.  If you specify a
list of dialing directory files, Kermit searches through all of them looking
for matching entries when building up its list of numbers to dial.
:p.
On LANs or computers that are shared by multiple users, it might be
desirable to have one central dialing directory shared by all users, another
shared by users within a particular group, another in each user's home
directory, and still another in each directory the user might CD to (just
one of many possible setups).

:h1 id=orderof.Order Of Dialing

After Kermit builds up its list of numbers to dial, it sorts them into
six categories, which are supposed to be (but obviously cannot be guaranteed
to be) "cheapest first"&colon.
:xmp.
:font facename=Courier size=10x8.
Internal PBX   (0)
Toll-Free      (1)
Local          (2)
Literal        (3)
Long-Distance  (4)
International  (5)
:font facename=default size=0x0.
:exmp.
:p.
and dials them in that order (the category numbers are displayed by Kermit
when you give a LOOKUP command).  No sorting is done within each category.
Literal numbers are those that do not begin with "+" in the dialing directory,
and therefore can't be categorized.
:p.
In some cases, this sorting might produce unwanted effects -- e.g. when a
toll-free number actually results in higher charges to you than (say) a local
call, or when literal numbers (whose "geography" can't be recognized by
Kermit) don't fall where you want them to, or when a long-distance call is
actually cheaper than a local call.  In such cases, you can inhibit
sorting and force Kermit to dial the numbers in the same order in which they
are encountered with the command&colon.
:p.
:hp4.SET DIAL SORT { OFF, ON }:ehp4.
.br
:lm margin=3.
OFF inhibits sorting, ON (the default) enables it.
:lm margin=1.
:p.
You also have another control.  In the general case, when there are lots of
numbers for a particular entry that range from local to international, you
can put a cap on how "far" Kermit is allowed to dial&colon.
:p.
:hp4.SET DIAL RESTRICT { INTERNATIONAL, LOCAL, LONG-DISTANCE, NONE }:ehp4.
.br
:lm margin=3.
INTERNATIONAL means that international calls are restricted, i.e. not
allowed.  LONG-DISTANCE means that long-distance and international
calls are restricted.  LOCAL means all calls except internal PBX calls
are restricted.  NONE is the default, meaning that all calls are allowed.
.br
:hp8.
NOTE&colon. This command has nothing to do with your phone -- telephones may or
may not have their own restrictions, which are independent of Kermit's.
:ehp8.
:lm margin=1.
:p.
Use the LOOKUP command to check the sort order to see whether you might
want to SET DIAL SORT OFF before calling a particular destination from a
particular location.

:h1 id=using.Using the Dialing Directory

There are three basic commands for setting up and using a dialing directory&colon.
:p.
:hp4.
SET DIAL DIRECTORY [ <file1> [ <file2> [ <file3> [ ... ] ] ] ]
:ehp4.
.br
:lm margin=3.
Selects zero, one, or more dialing directories.  If you do not include any
filenames, then the dialing directory feature is disabled and phone numbers
must be given directly to the DIAL command.  If you include one or more
filenames, then the DIAL command looks in each file and collects all the
matching entries, if any, prior to dialing.
:lm margin=1.
:p.
When you give a SET DIAL DIRECTORY command, specifying a filename, Kermit
tries to find the file in the normal way.  If it can't find the file, then, if
the filename that you gave was not "absolute" -- that is, it did not include a
full path specification -- then Kermit looks in either your home directory or
your Kermit software installation area, depending on the operating system.  In
OS/2, it looks in the CKERMIT directory.  In Windows NT or Windows 95, it
looks in the PHONES subdirectory of the Kermit 95 directory.
:p.
So, for example, in Windows 95, if your Kermit 95 directory is D&colon.\K95, then
your dialing directory (phonelist) files go in D&colon.\K95\PHONES, and you can give
SET DIAL DIRECTORY commands for them by name only, no matter what your current
directory is&colon.
:xmp.
:font facename=Courier size=10x8.
[C&colon.\TMP] K-95> :hp4.set dial directory oofa.kdd:ehp4.
:font facename=default size=0x0.
:exmp.
:p.
Of course, you can also specify a full path name.
:p.
If you installed C-Kermit according to instructions and you are using the
standard initialization file (*), then the default dialing directory name is&colon.
:xmp.
:font facename=Courier size=10x8.
&period.kdd        (UNIX, OS-9)
CKERMIT.KDD (elsewhere)(*)
:font facename=default size=0x0.
:exmp.
:p.
and if such a file exists, it it used.  In the absence of an initialization
file, C-Kermit uses the file(s) given by the environment variable
K_DIAL_DIRECTORY, if it is defined, for example (in UNIX ksh)&colon.
:xmp.
:font facename=Courier size=10x8.
export K_DIAL_DIRECTORY="/usr/share/lib/kermit/ckermit.kdd  $HOME/.kdd"
:font facename=default size=0x0.
:exmp.
:p.
In this example, which might be added to the system-wide login profile on
a multiuser UNIX computer, sets up two dialing directories&colon. the first is a
corporate-wide directory, centrally administered and shared by everyone,
and the second is a personal dialing directory in each user's home directory.
A similar technique might be useful on a corporate PC-based LAN, where the
corporate-wide directory would be placed on the file server, and the user's
directory on her or his own disk; the LAN login procedure would then set up
the environment variable.
:p.
(*) The concept of "initialization file" is less important in Kermit 95,
because the graphical Dialer sets everything up for you.  In Kermit 95,
the dialing directory file(s) is/are specified in the File menu, and
there are several of them.
:p.
:hp4.
LOOKUP <name>
:ehp4.
.br
:lm margin=3.
Looks up the <name> in your dialing directory or directories, and displays
the entries, if any, that match.  If any matching entries are found, then
the <name> becomes the default name for the next DIAL command.
:lm margin=1.
:p.
:hp4.
DIAL [ <text> ]
:ehp4.
.br
:lm margin=3.
Dials a phone call.  If the <text> is omitted, then the name, if any, from
the most recent DIAL command or successful LOOKUP command becomes the
<text>.  Then&colon.
:ul.
:li.If the name does not begin with a letter, it is not looked up in the
dialing directory / directories.
:li.If the name begins with an equals sign ("="), the equals sign is
discarded (the <text> becomes the part after the equals sign) and the
name is not looked up in the dialing directory/ies, even if it does
begin with a letter.
:li.If the original name began with a letter, and if one or more dialing
directories have been specified, the <text> is looked up.  If it is
found, it is replaced by one or more phone numbers; if the <text> is
not found, then it becomes the phone number.
:li.If the phone number (as given, or as found in the dialing directory) does
not start with "+", it is sent as-is to the modem.
:li.If the phone number starts with "+", it is converted to the appropriate
form before dialing.
:eul.
:p.
If <text> is found more than once in the directory (or directories), then each
number listed under <text> is dialed until one answers.
:p.
If the DIAL command is successful, then&colon.
:ul.
:li.If it was given directly from the Kermit command prompt, Kermit enters
terminal emulation (CONNECT mode) automatically.
:li.If the DIAL command was executed from a command file or macro, Kermit
does not enter CONNECT mode automatically, but rather, reads and executes
the next command, if any, from the command file or macro definition or,
if there are no more commands left in the command file or macro
definition, Kermit returns to its command prompt.
:eul.
:lm margin=1.
:p.
You can modify this behavior if you want to with the command&colon.
:p.
:hp4.
SET DIAL CONNECT { ON, OFF, AUTO }
:ehp4.
.br
:lm margin=3.
Tells Kermit whether to CONNECT automatically after successful dialing.
AUTO (the default) means do so only if DIAL command was given from top
level, i.e. not from a macro or command file.  ON means always CONNECT
automatically after successful dialing; OFF means never CONNECT
automatically.
:lm margin=1.
:p.
The next two commands make it easy to dial repeatedly&colon.
:p.
:hp4.
SET DIAL RETRIES <number>
:ehp4.
.br
:lm margin=3.
Tells Kermit to make up to <number> tries to dial the number again if the
original call fails (the default is 0).  When multiple numbers for the same
name have been fetched from the dialing directory, the entire sequence is
retried <number> times.  Retries are not done if there is an unrecoverable
error, but they are done if there is a busy signal, no answer, etc.
:lm margin=1.
:p.
:hp4.
SET DIAL INTERVAL <seconds>
:ehp4.
.br
:lm margin=3.
How many seconds to pause between dial retries.  The default interval
is 10 seconds.  This interval applies when retrying the same number or
group, but not between multiple entries in a group because in that case,
presumably, each number is different so there is no reason to pause.
:lm margin=1.
:p.
:hp8.
IMPORTANT&colon. THE LAWS OF MANY COUNTRIES PROHIBIT AUTOMATIC REDIALING, or
restrict the intervals at which and/or number of times a call can be redialed.
Do not use Kermit's automatic redial feature in any way that is against the
your local laws.
:ehp8.
:p.
Several other commands that control dialing, but are not related to dialing
directory, are presented here for completeness&colon.
:p.
:hp4.
ANSWER [ <timeout> ]
:ehp4.
.br
:lm margin=3.
Answer an incoming call.  If a <timeout> value is given, it specifies the
number of seconds to wait for the call to come in.  If a <timeout> is not
given, Kermit is to wait forever, or until interrupted.
:lm margin=1.
:p.
:hp4.
SET DIAL METHOD { DEFAULT, PULSE, TONE }
:ehp4.
.br
:lm margin=3.
Specifies pulse or tone dialing.  The default is, of course, DEFAULT,
meaning don't tell the modem which method to use, i.e.  the modem should
use its default dialing method.
:p.
Note that pulse dialing is not available in many areas, which is why tone
dialing is not the default method.  However, pulse dialing is not the default
method either, because the extra telephone keys "*" and "#" cannot be dialed
on a non-tone telephone line.
:p.
Hence we expect the modem to already be configured to use the appropriate type
of dialing.  In case it is not, errors can result.  For example, if you try to
send "*70" to the modem to disable call waiting, but the modem is configured
for pulse dialing, this will usually result in a modem command error.
:lm margin=1.
:p.
:hp4.
SET DIAL HANGUP { ON, OFF }
:ehp4.
.br
:lm margin=3.
Normally, Kermit hangs up the phone when you give a DIAL or ANSWER
command, on the assumption that if you are going to dial a phone
number, you want to be sure that any previous connection is hung up.
In cases where that assumption is incorrect, or when the very act of
hanging up causes problems, you can SET DIAL HANGUP OFF.
:lm margin=1.
:p.
:hp4.
SET DIAL TIMEOUT <seconds> [ <differential> ]
:ehp4.
.br
:lm margin=3.
The dial timeout is the number of seconds that Kermit waits to get back
a response from the modem telling whether a call succeeded or failed.
Normally Kermit figures this out itself, but you can use this command to
give your own timeout interval.  0 means Kermit decides.  The differential,
if given, is the number of seconds between when the modem is supposed to
time out and report NO CARRIER (or whatever), and when Kermit itself
times out in case the modem does not respond at all; the default
differential is 10 seconds.  Adjust the differential only if the default
causes problems for you.
:lm margin=1.
:p.
You can display these and all other DIAL-related settings with the SHOW
DIAL command.
:p.

:h1.About Country and Area Codes
Every country has a country code -- a short number, presently one to three
digits long, that identifies your country to the worldwide telephone system.
Most countries have their own country code, such as 39 for Italy, 47 for
Norway, 351 for Portugal.  Some countries share a country code; for example,
country code 1 is shared under the North American Numbering Plan (NANP) by the
USA, Canada, and various Carribean and Pacific islands.  Likewise, country
code 7 is presently shared among Russia and some of the other former Soviet
Republics.  A partial list of the world's country codes appears in Appendix I
(according to our best knowledge as of September 1995).  In case you don't
know what your country code is, you can probably find it there.  If not, look
in your telephone book.
:p.
Just as the world is divided into countries, most countries are divided into
calling areas, identified by area codes, sometimes called city codes.  For
example, in North America, 212 is the area code for the Borough of Manhattan
in New York City; 416 is the area code for Toronto, Ontario; 808 is the area
code for Hawaii.  In other countries area codes might be longer or shorter,
while some countries, such as Costa Rica, Luxembourg, and Singapore,
do not have area codes at all.
:p.
Be careful not to confuse your country code or area code with your
long-distance dialing prefix.  For example, in the NANP dialing area, the
country code is 1 and the normal long-distance dialing prefix is also 1,
whereas in most other places, the long-distance dialing prefix is different
from the country code.
:p.
In most West European countries, the long-distance dialing prefix is 0, and it
is not part of the area code.  So, for example, if you live in Central London,
your country code is 44 and your area code is 171 (not 0171).  Only a few
countries have area codes that begin with 0; examples include Australia,
Finland, and Russia.

:h1.Using Portable Mode

To use portable dialing directory entries, Kermit must know the country code
and area code you are dialing from.  If it doesn't have this information,
portable ("+") entries can't be dialed.  The commands are&colon.
:p.
:hp4.
SET DIAL COUNTRY-CODE <number>
:ehp4.
.br
:lm margin=3.
The numeric country code of the country you are dialing FROM.  Examples&colon. 1
for USA and Canada (etc), 44 for the UK, 33 for France, 49 for Germany.
This command tells Kermit what your local country code is, so it can compare
it with the country codes given in portable dialing directory entries to
tell whether calls are national or international.  If you have not set a
long-distance or international dialing prefix at the time you give this
command, Kermit sets default ones for you&colon. for country code 1, these are 1
and 011, respectively, and for all others they are 0 and 00.  If these are
not correct (as they will not be for many countries), please use the
commands described below to specify the right ones.
:lm margin=1.
:p.
:hp4.
SET DIAL AREA-CODE [ <number> ]
:ehp4.
.br
:lm margin=3.
The numeric area code you are dialing FROM.  If your country does not have
area codes, omit the <number>.  Be careful not to include your long-distance
dialing prefix as part of the area code (e.g. 0 in England or Germany, 9 in
Finland).  Examples&colon.
:sl compact.
:li.0      for Helsinki, Finland (prior to 12 October 1996)
:li.9      for Helsinki, Finland (beginning 12 October 1996)
:li.69     for Frankfurt, Germany
:li.171    for Central London, England (note&colon. not 0171)
:li.212    for Manhattan, New York City, USA
:li.516    for Long Island, New York, USA
:li.511    for Hannover, Germany
:li.6431   for Marburg, Germany
:li.38427  for Blowatz, Germany
:li.(blank) for Singapore, no area codes
:esl.
:lm margin=1.
:p.
:hp4.
SET DIAL LD-PREFIX <number>
:ehp4.
.br
:lm margin=3.
The prefix for dialing long-distance (non-local) calls from where you are.
This is the access code you must dial in order to place a call outside your
own dialing area, but inside your country (or countries covered by your
country code). Examples&colon. 1 for USA and Canada (etc), 0 for Germany, England,
and most others.  Strictly speaking, this item need not be a number; it may
contain commas or other dial modifiers if needed, which are sent to the
modem literally.  It can also be a long-distance-carrier access code, such
as 10288 for "10-ATT" or 10652 for "10-NJB".
:lm margin=1.
:p.
:hp4.
SET DIAL INTL-PREFIX <number>
:ehp4.
.br
:lm margin=3.
The prefix for dialing international calls from where you are.  Examples&colon.
011 for USA and Canada (etc), 00 for many other countries, with many
notable exceptions (reportedly, 0061 for Japan, 810 for Russia, 98 for
Mexico, etc).  This one, too, might also contain non-numeric characters if
necessary, and can also be an international-carrier access code.
:lm margin=1.
:p.
Versions of C-Kermit that come with installation scripts collect this
information from you at install time.  Otherwise, you will have to add this
information to your C-Kermit customization file yourself.
:p.
Kermit does not have built-in defaults for these items, but it will
pick up initial values for them from the environment, if they are defined
there.  The environment variable names are K_AREACODE, K_COUNTRYCODE,
K_INTL_PREFIX, K_LD_PREFIX.
:p.
Kermit also makes these values available to you in variables so you can
use them in macros and script programs&colon.
:sl compact.
:li.\v(d$ac) = DIAL AREA-CODE
:li.\v(d$cc) = DIAL COUNTRY-CODE
:li.\v(d$lp) = DIAL LD-PREFIX
:li.\v(d$ip) = DIAL INTL-PREFIX
:esl.
:p.
(See chapters on script programming in "Using C-Kermit"; a use for these
variables is suggested below.)

:h1.France

Beginning 18 October 1996, France, like Germany, England, and other European
Union countries, will have "0" as its long-distance prefix and "00" as its
international dialing prefix, but with a difference.
:p.
All calls within France, even local ones, MUST be dialed with an area code.
Thus all calls within France will start with "01", "02", ... "06".
:p.
In fact, the area code consists of a one-digit "long-distance" prefix ("0")
and a one-digit area code ("1"-"6").  This is a subtle distinction inside
France, because it makes no difference what the digits are called.  When
calling into France from outside, however, the leading zero must be dropped.
:p.
So for new French numbers to entered portably in the dialing directory, the
leading zero must be dropped (according to our usual rule)&colon.
:xmp.
:font facename=Courier size=10x8.
paris  +33 (1) 55 55 55 55
:font facename=default size=0x0.
:exmp.
:p.
Kermit users within France, beginning 18 October 1996, should configure
themselves as follows&colon.
:sl compact.
:li.SET DIAL COUNTRY-CODE 33
:li.SET DIAL AREA-CODE 999
:li.SET DIAL LD-PREFIX 0
:li.SET DIAL INTL-PREFIX 00
:esl.:p.
The use of an area code, "999", that will never occur in a real phone number
forces all calls to be dialed with the long-distance prefix and the area code.

:h1.Toll-Free Calls

The following command allows Kermit to recognize toll-free calls in your
dialing directory, and to dial toll-free numbers in preference to (presumably)
more-expensive calls when you have multiple entries with the same name&colon.
:p.
:hp4.
SET DIAL TOLL-FREE-AREA-CODE [ <number> [ <number> [ ... ] ] ]
:ehp4.
.br
:lm margin=3.
Specify zero, one, or more toll-free area codes in your country, such as 800
and 888 in the North American dialing area.  If one or more toll-free area
codes are specified, then dialing-directory entries with these area codes
within your country are considered toll-free, rather than long-distance, for
purposes of "cheapest-first" sorting.  Synonym&colon. SET DIAL TF-AREA-CODE.
:lm margin=1.
:p.
The following command can be used to specify a dialing prefix for toll-free
calls that is different from your long-distance dialing prefix&colon.
:p.
:hp4.
SET DIAL TOLL-FREE-PREFIX [ <number> ]
:ehp4.
.br
:lm margin=3.
Prefix to be used when making toll-free calls.  If not set, then your
long-distance prefixe (DIAL LD-PREFIX) is used.  Synonym&colon. SET DIAL TF-PREFIX.
:lm margin=1.
:p.
These items will also be picked up by Kermit at startup from the environment
variables K_TF_PREFIX and K_TF_AREACODE.  The latter can include multiple
area codes separated by spaces.

:h1.Unconditional Prefixes and Suffixes

The following two commands give you a simple and straightforward way to modify
phone numbers -- a prefix and a suffix that are blindly added to all phone
numbers prior to dialing, no matter whether the number was obtained from the
dialing directory, nor whether it is internal, local, long-distance, or
international.  (If you are dialing out from a PBX, however, the PBX
outside-line prefix precedes the dial prefix.) 
:p.
:hp4.
SET DIAL PREFIX [ <text> ]
:ehp4.
.br
:lm margin=3.
An item to precede the phone number, for example, to get a tie-line or to
disable call waiting.  This goes before anything else in the phone number
except the PBX-OUTSIDE-PREFIX (if any).  Example&colon.
:sl compact.
:li.SET DIAL PREFIX *70,
:esl.
:p.
which is used in many parts of North America to disable call-waiting for the
duration of the call, generally desirable when making modem calls.
:lm margin=1.
:p.
:hp4.
SET DIAL SUFFIX [ <text> ]
:ehp4.
.br
:lm margin=3.
An item to follow the phone number.
:lm margin=1.
:p.
There is no default prefix or suffix.  Use these commands to create them.
Use these commands without including any <text> to remove them, once created.

:h1.Characters Used In Telephone Numbers

Kermit does not translate letters in phone number to digits.  If a letter is
included in a phone number, Kermit sends the letter to the modem literally.
Thus, 1 800 COLLECT should be entered in the dialing directory as&colon.
:xmp.
:font facename=Courier size=10x8.
+1 (800) 2655328
:font facename=default size=0x0.
:exmp.
:p.
if that is, indeed, the number that should be dialed.  In a portable entry,
Kermit discards the plus sign and parentheses before passing the phone number
to the dialer.
:p.
Kermit does NOT understand or interpret non-numeric characters (other than
plus sign and parentheses) in telephone numbers.  Nevertheless, certain
conventions apply to Hayes and compatible modems and are widely used; even
many non-Hayes compatibles accept the same characters and treat them the
same way.  So in many cases they can be safely used in phone numbers (but NOT
in country codes or area codes)&colon.
:sl compact.
:li., = pause (usually 2 seconds)
:li.W = wait for secondary dialtone, e.g. when getting an outside line
:li.@ = wait for quiet answer, i.e. phone is answered, but not by a modem
:li.! = flash (try to get dialtone)
:li.; = return to command state immediately after dialing
:li.$ = wait for "bong", e.g. to enter calling-card number
:esl.
:p.
At present, Kermit ignores these characters (and all others) them and passes
them directly to the modem, but in the future Kermit might be able to handle
them intelligently (e.g. translating to non-Hayes notation when necessary).
Most likely, if this happens, the notation listed above will be used.  

:h1.Having the Last Word

When obtaining a phone number from the directory, Kermit performs various
transformations prior to dialing, which are described immediately following
this section.  But the result might not be exactly what is needed.  In cases
where Kermit is likely to err, you can ask it to display the phone number
before it dials, and give you a chance to change it&colon.
:p.
:hp4.
SET DIAL CONFIRMATION { ON, OFF }
:ehp4.
.br
:lm margin=3.
Requests confirmation of the phone number before dialing.  Normally OFF.
When ON, Kermit displays the number, exactly as it is about to be dialed,
and you may respond Yes or No.  If you respond No, Kermit prompts you for
a replacement number (editing in place is not supported at this time).
You may also enter Ctrl-C to cancel dialing altogether and return to
C-Kermit prompt.  (Not recommended for unattended dialing &colon.-)
:lm margin=1.
:p.

:h1.Simple Portable Mode

Kermit understands several types of dialing including local, long-distance,
and international.  When a dialing-directory entry is in portable format, or
you give a portable-format number directly in your DIAL command, Kermit
chooses the dialing method as follows&colon.
:ol.
:li.If your local COUNTRY-CODE is unknown, then Kermit prints a warning
message and stops dialing.  Otherwise&colon.
:li.If your COUNTRY-CODE is different from the number's country-code,
then Kermit makes an international call using its INTL-PREFIX,
the country code, area code (if any), and subscriber number.  Otherwise&colon.
:li.If the country codes match, but your AREA-CODE is not blank and it differs
from the number's area-code, Kermit makes a long-distance call&colon. the
country code is ignored and the call is placed using the LD-PREFIX, the
area code, and the subscriber number (toll-free calls fall into this
category too).  Otherwise;
:li.Kermit makes a local call.  The country code and area code are ignored,
and Kermit dials the subscriber number directly.
:eol.
:p.
If this is sufficient for your needs, you are now portable enough; read no
further.

:h1.Permissive Portable Mode

:hp8.
* * * *
.br
Most readers can (and should) skip over this section.  It is intended
only for people for whom Kermit's normal handling of country and area
codes is not flexible enough.
:ehp8.
:p.
It is permissible to omit parentheses from around the area code in a
portable dialing directory entry, for example&colon.
:xmp.
:font facename=Courier size=10x8.
+1 212 555 1212
:font facename=default size=0x0.
:exmp.
:p.
or&colon.
:xmp.
:font facename=Courier size=10x8.
+12125551212
:font facename=default size=0x0.
:exmp.
:p.
(Remember, spaces are ignored.)  If you do this, Kermit treats the entry a bit
differently.  Since it can't "parse" this type of phone number (i.e. separate
its country code, area code, and subscriber number), it tries to match the
concatenated DIAL COUNTRY-CODE and DIAL AREA-CODE (if any) with the beginning
of the phone number, ignoring the inital "+", as well as any "/", ".", and
spaces.  Thus permissive entries can also look like this&colon.
:xmp.
:font facename=Courier size=10x8.
+1 / 212 / 555-1212
+1/212/555-1212
+1.212.555.1212
:font facename=default size=0x0.
:exmp.
:p.
(This notation is not recommended, but it does agree with notation that is
commonly used in some parts of the world, and might aid in importing dialing
directories from other applications or databases.)
:p.
This type of entry is intended for use in parts of the world where the
distinction between area code and subscriber number are blurry.  Thus, by
setting a different length DIAL AREA-CODEs, you can "move" digits between
the area code and subscriber number for purposes of matching.  For example,
suppose you have this dialing directory entry&colon.
:xmp.
:font facename=Courier size=10x8.
+9876543210
:font facename=default size=0x0.
:exmp.
:p.
If you&colon.
:xmp.
:font facename=Courier size=10x8.
SET DIAL COUNTRY-CODE 9
SET DIAL AREA-CODE 87
:font facename=default size=0x0.
:exmp.
:p.
then Kermit dials "6543210".  But if you&colon.
:xmp.
:font facename=Courier size=10x8.
SET DIAL COUNTRY-CODE 9
SET DIAL AREA-CODE 8765
:font facename=default size=0x0.
:exmp.
:p.
then Kermit dials "43210".

:h1.Private Branch Exchanges

A private branch exchange (PBX) is a telephone system installed within a
building, company, hotel, organization, or other thing, that (for our
purposes) has the following properties&colon.
:ol.
:li.If you are dialing out from the PBX to the public telephone network, you
must dial a special prefix, such as "9", to get an outside line (the
prefix could be anything at all, and there might be more than one of
them, and it might include some kind of account code for internal
charging purposes).

:li.If you are dialing from your PBX phone to another phone on the same
PBX, the dialing method is different from what it would be if you were
making a local call into the PBX from outside.  For example, if your
local phone number (as seen from outside) is 987-6543, it might be
dialed internally as 6543, or 8-6543.
:eol.
:p.
The following commands allow you to use the same dialing directory entry for
internal calls within the PBX, calls to the outside from within the PBX, and
calls into the PBX from outside&colon.
:p.
:hp4.
SET DIAL PBX-OUTSIDE-PREFIX [ <number> ]
:ehp4.
.br
:lm margin=3.
Tells the prefix you must dial in order to get an outside line.  
Issue this command to specify the prefix when you are using a PBX phone.
And be sure that no PBX-OUTSIDE-PREFIX is defined when you are
dialing on the public telephone network (issuing this command without
a <number> removes the prefix).
:lm margin=1.
:p.
:hp4.
SET DIAL PBX-EXCHANGE [ <number> ]
:ehp4.
.br
:lm margin=3.
Tells the leading digits of a subscriber number within the AREA-CODE
that identify it as belonging to the PBX, typically a 3- or 4-digit number.
Kermit does not presently handle PBXs with multiple exchanges.
:lm margin=1.
:p.
:hp4.
SET DIAL PBX-INSIDE-PREFIX [ <number> ]
:ehp4.
.br
:lm margin=3.
Specifies the prefix, if any, that must be used to dial an internal number.
:lm margin=1.
:p.
These commands work as follows&colon.
:p.
If a PBX-OUTSIDE-PREFIX is defined, then if a call is determined to be
local (same area code), the PBX-EXCHANGE (if any) is compared with the
beginning of the local phone number.  If they match, then those digits are
removed from the local phone number before dialing.  In addition, if a
PBX-INSIDE-PREFIX is specified, it is added to the beginning of the phone
number before dialing.
:p.
Here's an example.  Suppose we have the following dialing directory entries&colon.
:xmp.
:font facename=Courier size=10x8.
MARKETING  +1 (617) 555 1234
BOONDOCKS  +1 (617) 444 6789
:font facename=default size=0x0.
:exmp.
:p.
and suppose we have given the following commands&colon.
:xmp.
:font facename=Courier size=10x8.
SET PBX-OUTSIDE-PREFIX 9
SET PBX-EXCHANGE 555
SET PBX-INSIDE-PREFIX 4
:font facename=default size=0x0.
:exmp.
:p.
Then, if we "dial boondocks", the number that is actually dialed is
916174446789 because, since our PBX exchange does not match the beginning of
the "boondocks" local number, we treat it as an external local call; the
outside-line prefix is dialed, followed by the local number.
:p.
But if we "dial marketing", we note that the exchange "555" matches ours, so
we strip the "555" and replace it by "4", resulting in an internal call,
"41234".
:p.
On the other hand, if we are dialing from outside (e.g. from home), we use&colon.
:xmp.
:font facename=Courier size=10x8.
SET PBX-OUTSIDE-PREFIX
:font facename=default size=0x0.
:exmp.
:p.
(no prefix) to indicate we are not using a PBX (the normal case), and then
"dial marketing" is just like dialing any other number; it is dialed as a
local, long-distance, or international call, depending on our location.
:p.
On LANs or multiuser computers that use PBXs for dialing out, the following
environment variables can be set to ensure that Kermit does the right thing
for all users by default&colon.
:sl compact.
:li.K_PBX_INSIDE   -  PBX-INSIDE-PREFIX
:li.K_PBX_OUTSIDE  -  PBX-OUTSIDE-PREFIX
:li.K_PBX_EXCHANGE -  PBX-EXCHANGE
:esl.
:h1.Looking Up and Checking Numbers

If you want to see the effect that the various SET DIAL settings have on
telephone numbers, make a test dialing directory containing many entries
with the same name (say "test"), but with different country and area codes,
and then see the effect of setting different DIAL COUNTRY-CODEs and DIAL
AREA-CODEs (and PREFIXes, and SUFFIXes, and PBX items, and TOLL-FREE items,
etc etc) by changing these settings and then giving a LOOKUP command on the
test name.  Example for "lookup test" (all phone numbers are fictional)&colon.
:xmp.
:font facename=Courier size=10x8.
7 telephone numbers found for "test"&colon.
  1. Test         +1 (212) 555 1234    =>  7 1234                (0)
  2. Test         +1 (800) 555 4321    =>  1800555 4321          (1)
  3. Test         +1 (888) 555 4321    =>  1888555 4321          (1)
  4. Test         +1 (212) 555 5432    =>  555 5432              (2)
  5. Test         5559924              =>  5559924               (3)
  6. Test         +1 (201) 555-6543    =>  1201555-6543          (4)
  7. Test         +49 (551) 7654321    =>  011495517654321       (5)
:font facename=default size=0x0.
:exmp.
:p.
This lists all the matching entries, in the order they will be dialed,
showing the name, the number from the directory entry and then, after the
arrow (=>), the number that will actually be dialed, and finally the type
of call&colon.
:sl compact.
:li.Internal PBX   (0)
:li.Toll-Free      (1)
:li.Local          (2)
:li.Literal        (3)
:li.Long-Distance  (4)
:li.International  (5)
:esl.
:p.
If the ordering is not what you desire, tell Kermit to SET DIAL SORT OFF
and do the LOOKUP again.  This forces Kermit to dial the numbers in the
same order they were encountered in the directory or directories.

:h1.When Portable Mode Isn't Portable Enough

The rules presented above are adequate in many cases, but there will always
be exceptions(*)&colon.
:p.
:lm margin=3.
:hp1.
"For example, here in Toronto, we're in area code 416.  If I want to call
416-488-XXXX, that's a local call, and it's in my area code, so I dial
7 digits, 488-XXXX.  If I want to call 905-276-XXXX, that's a local call,
and it's outside my area code, so I dial 10 digits&colon. 905-276-XXXX.  And
if I want to call 905-528-XXXX, that's long distance, so I dial 11 digits&colon.
1-905-528-XXXX.  Finally, if I want to dial 976-XXXX, that's a premium
call, which is like long distance, so I have to dial 1-416-976-XXXX,
using the 416 even though that's my own area code.  (It happens that there
are no long distance calls within 416, so I use the premium call to show
the dialing.)
:p.
"Ottawa is in area code 613.  If they dial 232-XXXX, that's a local call
and reaches 613-232-XXXX.  But if they dial 778-XXXX, they reach
819-778-XXXX.  Why?  Because that number is in their local calling area,
while 613-778-XXXX either would be long distance, or doesn't exist.  When
the call *is* long distance (or premium), they dial it the same as in
Toronto&colon. 11 digits.  (Historically, until quite recently they would have
dialed long distance calls within their area code as 8 digits&colon. 1-778-XXXX
to reach 613-778-XXXX.  This had to change when area codes 334, 360, 630,
etc. were introduced in 1995.)"
:ehp1.
:lm margin=1.
:p.
(*) Mark Brader, SoftQuad Inc., Toronto
:p.
Another story(*)&colon.
:p.
:lm margin=3.
:hp1.
"There are 400-500 active exchanges in area code 508.  Of those, 27 are in
what Nynex has defined as my local area.  I am not permitted to dial
1-508 before any of those 27 exchanges, but I am REQUIRED to dial 1-508
to reach any of the remaining exchanges in area code 508.
:p.
"I could put these in as fixed numbers, but all I would have to do would
be to take my laptop computer 1.5 miles north from here and then I would 
be in a different local area where some of my old local area would still be
local, but some would now require a 1-508, and some that previously needed
1-508 would now only be able to be dialed without the 1-508."
:lm margin=1.
:ehp1.
:p.
(*) Ken Levitt, Informed Computer Solutions, Wayland, MA.
:p.
And another(*)&colon.
:p.
:lm margin=3.
:hp1.
"San Marino is country code 378, and Italy is country code 39.  If you're
dialing from Italy to any other country code except San Marino, you dial
00 country-code city-code local-number, but if you're dialing San Marino,
you must dial it as 0549 local-number (instead of 00-378 local-number).
Likewise, if you dial from San Marino to any country except Italy, you
dial 00 country-code city-code local-number, but if you're dialing Italy,
you dial 0 city-code local-number (instead of 00-39 city-code
local-number).  Such special arrangements also exist between Singapore
and Malaysia, Mexico to US/Canada/Carribean, Ireland and Northern
Ireland, Estonia and Russia, Switzerland/Austria/France/Germany/Italy
(often between just pairs of cities across the border from each other),
Uganda/Kenya/Tanzania/Rwanda/Burundi, etc."
:ehp1.
:lm margin=1.
:p.
(*) Toby Nixon, Program Manager, Windows Telephony (TAPI),
Microsoft Corporation. 
:p.
Rather than build error-prone and inadequate knowledge-based "intelligence"
into the software, we draw the line at this point and recommend that such
pathological cases be handled by&colon.
:ul.
:li.Inserting literal-mode entries into the directory, even if that means
different entries are needed for dialing the same number from different
locations, or&colon. 

:li.SET DIAL CONFIRMATION ON, or&colon.

:li.Bypassing the dialing directory and dialing such numbers literally, i.e.
"dial 987654321" (hint&colon. use LOOKUP to look it up first, then enter it
correctly in the DIAL command).

:li.Sneaky tricks.
:eul.
:p.
Here are some sneaky tricks that can be used to handle certain common cases&colon.
:ol.
:li.North American 10-digit dialing to a different area code&colon.
.br
Just "set dial ld-prefix" to nothing prior to dialing the number.
Then set it back to your normal long-distance prefix before dialing
any numbers that need it (illustrated below).

:li.
North American 10-digit dialing to same area code&colon.
.br
Normally, Kermit removes the area code if it is the same as your local
one prior to dialing.  To force Kermit to keep it on, "set dial
area-code" to nothing prior to dialing the number.  Then Kermit will
treat it as a "long" local number.  This is the same technique that is
used in countries that do not have area codes (such as Singapore).

:li.North American 11-digit dialing to same area code&colon.
.br
Set your area code to one that does not exist, e.g. "set dial area-code
000" prior to dialing and then put it back to normal afterwards.  This is
the same technique that is used in countries in which all calls must be
dialed with the area code, such as France beginning 18 October 1996.
:eol.
:p.
Use your imagination to extend these examples to other difficult
situations.  Tricks such as these can be accomplished conveniently by
defining macros that save and restore your long-distance prefix or area code
around dialing, for example&colon.
:xmp.
:font facename=Courier size=10x8.
DEFINE SN1 -                ; Sneaky Trick 1
  ASSIGN \%9 \v(d$lp), -    ; Save current LD-PREFIX
  SET DIAL LD-PREFIX, -     ; Set LD-PREFIX to nothing
  DIAL \%1, -               ; DIAL the given number
  ASSIGN \%8 \v(status), -  ; Save DIAL command status
  SET DIAL LD-PREFIX \%9, - ; Restore LD-PREFIX
  END \%8                   ; Return with DIAL's status
:font facename=default size=0x0.
:exmp.
:p.
If you added this macro definition to your C-Kermit customization file,
then you would type "sn1 oofa" to force 10-digit dialing for the dialing
directory entry named "oofa".
:p.
See Chapters 11-13 of "Using C-Kermit" for a complete explanation of
macros and script programming.

:h1.Long-distance Carriers and Credit-Card/Calling-Card Numbers

Thanks, at least in the USA, to deregulation, it is now possible to use
different companies to make long-distance and international calls.  Each
telephone has a default long-distance company, which is used if you make
these calls in the normal way, but you can also choose a different
long-distance company on a per-call basis by dialing phone numbers in a
different way.  Furthermore, you can charge calls made on other phones to
your own account by dialing in (perhaps another) special way and then
specifying your account number.
:p.
The easy case involves specification of an alternative long-distance prefix.
For example, to use New Jersey Bell in the USA, the prefix is "10-NJB" (i.e.
"10652") rather than the customary "1".  Everything works exactly as described
previously, except your long-distance bill comes from a different company&colon.
:xmp.
:font facename=Courier size=10x8.
SET DIAL LD-PREFIX 10652
:font facename=default size=0x0.
:exmp.
:p.
Now suppose you also need to supply a credit-card number, but only on
long-distance calls.  We assume that this must come either before the area
code or after the phone number (rather than imbedded in the phone number),
so therefore it can be part of the LD-PREFIX or it can be a
suffix.  Not the DIAL SUFFIX, which is always applied (if defined), but
rather&colon.
:p.
:hp4.
SET DIAL LD-SUFFIX [ <text> ]
:ehp4.
.br
:lm margin=3.
The text, if any, is appending to the phone number prior to dialing
if Kermit has determined the call is long-distance and not toll-free.
:lm margin=1.
:p.
For example, suppose that in order to have a long-distance call billed
to your credit card, you must dial 0, the area code, the number, and
then pause for several seconds, and then enter your credit-card number&colon.
:xmp.
:font facename=Courier size=10x8.
SET DIAL LD-PREFIX 0
SET DIAL LD-SUFFIX ,,xxxxxxxxxxxxxxxxxx
:font facename=default size=0x0.
:exmp.
:p.
where the x's represent your card number.  Depending on the capabilities of
your modem and the behavior of the long-distance carrier, you might be
able to replace the comma(s) by a "wait for bong" character (normally "$").
:p.
In another scenario, you might be using your default long-distance carrier
to call another long-distance carrier, and then calling the desired number
from there, also supplying a credit- or calling-card number.  Here you are
really making a phone-call-within-a-phone-call.  For example, you dial&colon.
:xmp.
:font facename=Courier size=10x8.
1 800 nnnnnnn
:font facename=default size=0x0.
:exmp.
:p.
(where the n's are replaced by the second carrier's number), wait for a
"quiet answer" (no carrier), then enter 0, the area code, the subscriber
number, then wait for a bong, then enter the credit card number.  The
appropriate commands would be&colon.
:xmp.
:font facename=Courier size=10x8.
SET LD-PREFIX 1,800,nnnnnnn@0
SET LD-SUFFIX $xxxxxxxxxxxxxxxxxx
:font facename=default size=0x0.
:exmp.
:p.
Finally, allowing for the possibility that the billing method for
international calls might be different from long-distance calls within one's
country, we also have&colon.
:xmp.
:font facename=Courier size=10x8.
SET DIAL INTL-PREFIX [ <text> ]
SET DIAL INTL-SUFFIX [ <text> ]
:font facename=default size=0x0.
:exmp.
:p.
OBVIOUSLY, you DO NOT want to put credit-card or account information in the
dialing directory or any other file, as that is a classic security risk.
Therefore, you are going to have to provide this information to Kermit each
time you run it and you know that you will be making toll calls.  In other
chapters, you can find out how to do this easily with macros, script
programs, etc.

:h1.When the Dialing String is Too Long

Some modems, even modern ones, might have a command buffer that is too
short for a long dialing string like&colon.
:xmp.
:font facename=Courier size=10x8.
ATDT1,800,nnnnnnn@02125551234$xxxxxxxxxxxxxxxxxx
:font facename=default size=0x0.
:exmp.
:p.
In some cases, 40 characters is the maximum length for a command, including
a dialing command, as shown above.  If you dial a number that's too long,
the modem will respond with a message or code indicating a command error
(such as ERROR from Hayes compatible modems in word-result mode).  Rather
than try to build additional "magic" into the software to handle this
situation, we recommend that you dial too-long numbers in multiple stages.

:h1.Partial and Multistage Dialing

Some types of modems can be told to return to command mode immediately after
being given a dial command, without waiting for carrier.  For Hayes and
compatible modems, this is done by including a semicolon (;) as the last
character of the phone number.  This is useful in several circumstances&colon.
:ol.
:li.The number you are calling is not a modem and it will never send carrier.
For example, it is a voice number, a numeric pager, or a long-distance
or international carrier number.

:li.The dial string is too long for the modem and needs to be broken into
pieces.
:eol.
Thus, to dial a numeric pager, whose number is 555-1234, you might&colon.
:xmp.
:font facename=Courier size=10x8.
DIAL 5551234@xxxxxxxxx#;
:font facename=default size=0x0.
:exmp.
:p.
where "xxxxxxxxx" is the pager message.  This way, Kermit returns immediately
rather than waiting a long time only to get a NO CARRIER message.
:p.
Similarly to dial a credit-card call that is too long for your modem, you
could&colon.
:xmp.
:font facename=Courier size=10x8.
dial 10288,0,212,5551234,,,;
dial 4114 9999 9999 9999
:font facename=default size=0x0.
:exmp.
:p.
This is fine when typing in the phone number directly at the DIAL command.
But you don't want to include semicolons in your dialing directory, because
you might also want to dial the same number in other ways.  So we need a
special command for partial dialing&colon.
:p.
:hp4.
PDIAL <number>
:ehp4.
.br
:lm margin=3.
Like DIAL, but issues the dialing command in the modem-specific way
that causes the modem to return to command state immediately after
dialing the number, rather than waiting for carrier.  Furthermore,
Kermit remembers that the previous call was partial, and so does not
reinitialize or hang up the modem when the next DIAL or PDIAL command
is issued.
:lm margin=1.
:p.
So now, if the numbers you need to call are recorded in your dialing
directory in the normal way, for example&colon.
:xmp.
:font facename=Courier size=10x8.
OFFICE +1 (212) 555 1234
:font facename=default size=0x0.
:exmp.
:p.
and you have to make a credit-card call to them, but you have to use
multistage dialing because the combined numbers are too long&colon.
:xmp.
:font facename=Courier size=10x8.
SET LONG-DISTANCE PREFIX 10288,0,
SET LONG-DISTANCE SUFFIX
PDIAL OFFICE
DIAL ,,,4114 9999 9999 9999
:font facename=default size=0x0.
:exmp.
:p.
Here again, a macro can make this easier for you&colon.
:xmp.
:font facename=Courier size=10x8.
DEFINE LONGDIAL -
  SET LONG-DISTANCE PREFIX 10288,0,,-
  SET LONG-DISTANCE SUFFIX,-
  PDIAL \%1,-
  IF SUCCESS DIAL ,,,4114 9999 9999 9999
:font facename=default size=0x0.
:exmp.
:p.
REDIAL does not work with partially dialed numbers, nor does the automatic
retry feature.
:p.
Finally, suppose you issue a partial dialing command but then wish to cancel
the partially-dialed status, so that the next DIAL or PDIAL command does not
skip the modem hangup or initialization stages.  To accomplish this, either&colon.
:p.
:hp4.
HANGUP
:ehp4.
.br
:lm margin=3.
Which hangs up any open connection and also clears the dial status.
:lm margin=1.
:p.
or&colon.
:hp4.
CLEAR DIAL-STATUS
:ehp4.
.br
:lm margin=3.
Which clears Kermit's dial status without doing anything to the modem.
:lm margin=1.
:p.

:h1.Declaring Your Locale

If you've read this far, you know that it's important to separate the concepts
related to the number to be dialed from those relating to the location where
dialing occurs and the method of dialing, at least if you want to have a
portable dialing directory.  So the dialing directory contains no information
at all about your locale.
:p.
You have also learned the commands for setting up your dialing locale and
method.  All that's left to be said is that the process need not be painful
and laborious.  Kermit's command-file and macro capabilities, explained in
Chapters 11-13 of "Using C-Kermit", let you assign complicated configurations
to friendly words of your choice.  For example, suppose you carry a laptop
around to various locations&colon.
:xmp.
:font facename=Courier size=10x8.
DEFINE USA -
  SET DIAL COUNTRY-CODE 1,-
  SET DIAL LD-PREFIX 1,-
  SET DIAL INTL-PREFIX 011
:font facename=default size=0x0.
:exmp.
:p.
:xmp.
:font facename=Courier size=10x8.
DEFINE GERMANY -
  SET DIAL COUNTRY-CODE 49,-
  SET DIAL LD-PREFIX 0,-
  SET DIAL INTL-PREFIX 00
:font facename=default size=0x0.
:exmp.
:p.
:xmp.
:font facename=Courier size=10x8.
DEFINE MANHATTAN   USA,     SET DIAL AREA-CODE 212
DEFINE LONG-ISLAND USA,     SET DIAL AREA-CODE 516
DEFINE HANNOVER    GERMANY, SET DIAL AREA-CODE 511
DEFINE MARBURG     GERMANY, SET DIAL AREA-CODE 6421
:font facename=default size=0x0.
:exmp.
:p.
And when at work in Manhattan where you have a PBX&colon.
:xmp.
:font facename=Courier size=10x8.
DEFINE WORK -
  MANHATTAN, -
  SET DIAL PBX-OUT 93, SET DIAL PBX-EX 987, SET DIAL PBX-IN 7
:font facename=default size=0x0.
:exmp.
:p.
And then for Hoboken, New Jersey, where you need to dial using a particular
long-distance carrier&colon.
:xmp.
:font facename=Courier size=10x8.
DEFINE HOBOKEN -
  USA, -
  SET DIAL AREA-CODE 201, -
  SET DIAL LD-PREFIX 10652
:font facename=default size=0x0.
:exmp.
:p.
These macro definitions would be kept in your Kermit customization file.  And
then, whenever you start up Kermit to make a call, just enter one word&colon.
MANHATTAN, HOBOKEN, HANNOVER, WORK, etc, to declare your location.  Then DIAL
away.
:p.
A macro library of such "rule sets" can be built up over time and included
with in your Kermit customization file, so these macros will be available to
you whenever you use Kermit.  You can create and modify macro definitions
easily, since they are plain text and not part of the binary executable
Kermit program or any particular kind of database.

:h1.APPENDIX I&colon. Country Codes
:xmp.
:font facename=Courier size=10x8.
Afghanistan ......................................  +93
Albania .......................................... +355
Algeria .......................................... +213
American Samoa ...................................   +1
Andorra .......................................... +376
Angola ........................................... +244
Anguilla .........................................   +1
Antigua &amp. Barbuda ................................   +1
Argentina ........................................  +54
Armenia ..........................................   +7
Aruba ............................................ +297
Ascension Island ................................. +247
Australia ........................................  +61
Austria ..........................................  +43
Azerbaijani Republic ............................. +994
Azores ........................................... +351
Bahamas ..........................................   +1
Bahrain .......................................... +973
Balearic Islands .................................  +34
Bangladesh ....................................... +880
Barbados .........................................   +1
Belarus ..........................................   +7
Belgium ..........................................  +32
Belize ........................................... +501
Benin ............................................ +229
Bermuda ..........................................   +1
Bhutan, Kingdom of ............................... +975
Bolivia .......................................... +591
Bosnia-Herzegovina (former Yugoslav Republic of) . +387
Botswana ......................................... +267
Brazil ...........................................  +55
Brunei Darussalam ................................ +673
Bulgaria ......................................... +359
Burkina Faso (Upper Volta) ....................... +226
Burma (Union of Myanmar) .........................  +95
Burundi .......................................... +257
Caicos and Turks Islands .........................   +1
Cambodia (Kampuchea) ............................. +855
Cameroon ......................................... +237
Canada ...........................................   +1
Canal Zone (Panama) .............................. +507
Canary Islands ...................................  +34
Cape Verde Islands ............................... +238
Caroline Islands ................................. +691
Cayman Islands ...................................   +1
Central African Republic ......................... +236
Chad ............................................. +235
Chile ............................................  +56
China, People's Republic of ......................  +86
China, Republic of ............................... +886
Colombia .........................................  +57
Congo ............................................ +242
Cook Islands ..................................... +682
Costa Rica ....................................... +506
Croatia .......................................... +385
Cuba .............................................  +53
Cyprus ........................................... +357
Czech Republic ...................................  +42
Dahomey (Benin) .................................. +229
Denmark ..........................................  +45
Diego Garcia ..................................... +246
Djibouti Republic ................................ +253
Dominica .........................................   +1
Dominican Republic ...............................   +1
Dubai ............................................ +971
Ecuador .......................................... +593
Egypt, Arab Republic of ..........................  +20
El Salvador ...................................... +503
Equitorial Guinea ................................ +240
Eritrea .......................................... +291
Estonia .......................................... +372
Ethiopia ......................................... +251
Falkland Islands ................................. +500
Faroe Islands .................................... +298
Fiji ............................................. +679
Finland .......................................... +358
France ...........................................  +33
French Antilles .................................. +596
French Guiana .................................... +594
French Polynesia ................................. +689
Gabon Republic ................................... +241
Gambia ........................................... +220
Georgia ..........................................   +7
Germany ..........................................  +49
Ghana ............................................ +233
Gibraltar ........................................ +350
Greece ...........................................  +30
Greenland ........................................ +299
Grenada ..........................................   +1
Grenadines .......................................   +1
Guadeloupe ....................................... +590
Guam ............................................. +671
Guatemala ........................................ +502
Guinea-Bissau .................................... +245
Guyana ........................................... +592
Haiti ............................................ +509
Honduras ......................................... +504
Hong Kong ........................................ +852
Hungary ..........................................  +36
Iceland .......................................... +354
India ............................................  +91
Indonesia ........................................  +62
Iran .............................................  +98
Iraq ............................................. +964
Ireland, Repuplic of (Eire) ...................... +353
Israel ........................................... +972
Italy ............................................  +39
Ivory Coast ...................................... +225
Jamaica ..........................................   +1
Japan ............................................  +81
Jordan ........................................... +962
Kazahkstan .......................................   +7
Kenya ............................................ +254
Kirghizstan ......................................   +7
Kiribati Republic ................................ +686
Korea, Democratic Peoples Repulic of ............. +850
Korea, Republic of ...............................  +82
Kuwait ........................................... +965
Laos ............................................. +856
Latvia ........................................... +371
Lebanon .......................................... +961
Lesotho .......................................... +266
Liberia .......................................... +231
Libya ............................................ +218
Liechtenstein ....................................  +41
Lithuania ........................................ +370
Luxembourg ....................................... +352
Macao ............................................ +853
Macedonia (former Yugoslav Republic of) .......... +389
Madagascar ....................................... +261
Madeira .......................................... +351
Malawi ........................................... +265
Malaysia .........................................  +60
Maldives ......................................... +960
Mali ............................................. +223
Malta ............................................ +356
Mariana Islands .................................. +670
Martinique ....................................... +596
Mauritania ....................................... +222
Mauritius ........................................ +230
Mexico ...........................................  +52
Micronesia ....................................... +691
Moldova .......................................... +373
Monaco ...........................................  +33
Mongolian People's Republic ...................... +976
Monserrat ........................................   +1
Morocco .......................................... +212
Mozambique ....................................... +258
Myanmar, Union of (Burma) ........................  +95
Namibia .......................................... +264
Nauru ............................................ +674
Nepal ............................................ +977
Netherlands ......................................  +31
Netherlands Antilles ............................. +599
Nevis ............................................   +1
New Caledonia .................................... +687
New Guinea, Papua ................................ +675
New Zealand ......................................  +64
Nicaragua ........................................ +505
Niger ............................................ +227
Nigeria .......................................... +234
Niue Island ...................................... +683
Norfold Island ................................... +672
Norway ...........................................  +47
Oman ............................................. +968
Pakistan .........................................  +92
Palau ............................................ +680
Panama ........................................... +507
Papua New Guinea ................................. +675
Paraguay ......................................... +595
Peru .............................................  +51
Philippines ......................................  +63
Poland ...........................................  +48
Portugal ......................................... +351
Puerto Rico ......................................   +1
Qatar ............................................ +974
Reunion .......................................... +262
Romania ..........................................  +40
Russia ...........................................   +7
Rwanda ........................................... +250
Saint Helena ..................................... +290
Saint Kitts ......................................   +1
Saint Lucia ......................................   +1
Saint Vincent and Grenadines .....................   +1
Samoa, American ..................................   +1
Samoa, Western ................................... +685
San Marino .......................................  +39
Sao Tome and Principe ............................ +239
Saudi Arabia ..................................... +966
Senegal .......................................... +221
Serbia-Montenegro ................................ +381
Seychelles ....................................... +248
Sierra Leone ..................................... +232
Singapore ........................................  +65
Slovak Republic ..................................  +42
Solomon Islands .................................. +677
Somali ........................................... +252
South Africa .....................................  +27
Spain ............................................  +34
Sri Lanka ........................................  +94
Sudan ............................................ +249
Suriname ......................................... +597
Swaziland ........................................ +268
Sweden ...........................................  +46
Switzerland ......................................  +41
Syrian Arab Republic ............................. +963
Tahiti ........................................... +689
Taiwan (Republic of China) ....................... +886
Tajikistan .......................................   +7
Tanzania ......................................... +255
Thailand .........................................  +66
Togo ............................................. +228
Tonga ............................................ +676
Trinidad &amp. Tobago ................................   +1
Tunisia .......................................... +216
Turkey ...........................................  +90
Turkmenistan .....................................   +7
Turks &amp. Caicos Islands ...........................   +1
Tuvalu ........................................... +688
Uganda ........................................... +256
Ukraine ..........................................   +7
United Arab Emirates ............................. +971
United Kingdom ...................................  +44
United States of America .........................   +1
Uruguay .......................................... +598
Uzbekistan .......................................   +7
Vanuatu .......................................... +678
Vatican City .....................................  +39
Venezuela ........................................  +58
Vietnam ..........................................  +84
Virgin Islands, British ..........................   +1
Virgin Islands, USA ..............................   +1
Yemen Arab Republic .............................. +967
Yugoslavia (Serbia-Montenegro) ................... +381
Zaire ............................................ +243
Zambia ........................................... +260
Zimbabwe ......................................... +263
:font facename=default size=0x0.
:exmp.

:h1.APPENDIX II&colon. Summary of Kermit Environment Variables

:table cols='20 20 20'.
:row.
:c.Environment variable   
:c.Corresponding command      
:c.Kermit variable
:row.
:c.K_DIAL_DIRECTORY       
:c.SET DIAL DIRECTORY
:row.
:c.K_COUNTRYCODE          
:c.SET DIAL COUNTRY-CODE           
:c.\v(d$cc)
:row.
:c.K_AREACODE             
:c.SET DIAL AREA-CODE              
:c.\v(d$ac)
:row.
:c.K_INTL_PREFIX          
:c.SET DIAL INTL-PREFIX            
:c.\v(d$ip)
:row.
:c.K_LD_PREFIX            
:c.SET DIAL LD-PREFIX              
:c.\v(d$lp)
:row.
:c.K_PBX_OUTSIDE          
:c.SET DIAL PBX-OUTSIDE-PREFIX
:row.
:c.K_PBX_INSIDE           
:c.SET DIAL PBX-INSIDE-PREFIX
:row.
:c.K_PBX_EXCHANGE         
:c.SET DIAL PBX-EXCHANGE
:row.
:c.K_TF_PREFIX            
:c.SET DIAL TOLL-FREE-PREFIX
:row.
:c.K_TF_AREACODE          
:c.SET DIAL TOLL-FREE-AREACODE
:etable.

:h1.APPENDIX III&colon. Dialing Command List

Actions&colon.
:sl compact.
:li.DIAL [ <number> ]
:li.PDIAL [ <number> ]
:li.REDIAL
:li.ANSWER [ <seconds> ]
:li.HANGUP
:li.LOOKUP <name>
:li.CLEAR DIAL-STATUS
:esl.
:p.
Configuration&colon.
:sl compact.
:li.SET DIAL AREA-CODE [ <number> ]
:li.SET DIAL CONFIRMATION { ON, OFF }
:li.SET DIAL COUNTRY-CODE [ <number> ]
:li.SET DIAL CONVERT-DIRECTORY { ASK, ON, OFF }
:li.SET DIAL DIRECTORY [ <filename> ]
:li.SET DIAL DISPLAY { ON, OFF }
:li.SET DIAL HANGUP { ON, OFF }
:li.SET DIAL INTERVAL <seconds>
:li.SET DIAL INTL-PREFIX <number>
:li.SET DIAL INTL-SUFFIX [ <text> ]
:li.SET DIAL LD-PREFIX <number>
:li.SET DIAL LD-SUFFIX [ <text> ]
:li.SET DIAL METHOD { PULSE, TONE, DEFAULT }
:li.SET DIAL PBX-EXCHANGE [ <number> ]
:li.SET DIAL PBX-INSIDE-PREFIX [ <number> ]
:li.SET DIAL PBX-OUTSIDE-PREFIX [ <number> ]
:li.SET DIAL PREFIX [ <text> ]
:li.SET DIAL RETRIES <number>
:li.SET DIAL SORT { ON, OFF }
:li.SET DIAL SUFFIX [ <text> ]
:li.SET DIAL TIMEOUT <number>
:li.SET DIAL TOLL-FREE-AREA-CODE [ <number> ]
:li.SET DIAL TOLL-FREE-PREFIX [ <number> ]
:li.SHOW DIAL
:esl.
:euserdoc.i