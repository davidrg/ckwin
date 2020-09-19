C-KERMIT 9.0.305 ALPHA TESTING

9.0.305 Alpha.02 19 Sep 2020

No functional changes since last time, but about 30 compile-time warnings
that were reported by gcc 9.3.0 on Ubuntu 20.04.1 are fixed.  The result
compiles without warnings there, and also on OpenBSD 9 and Red Hat 6.1.

9.0.305 Alpha.01 24 Jul 2020

Contains to patch to work around the disappearance of the GNU standard I/O
library symbol "__FILE_defined" from glibc-based Linux distributions such
as Debian and Ubuntu.  I was not ready to release a new version of C-Kermit
yet, but this forces the issue.

C-KERMIT 9.0.304 PRERELEASE TESTING

9.0.304 Dev.24 25 Apr 2020

Some improvements to date/time parsing to allow the straightforward
processing of Apache web logs, and fixed big in which the CHANGE command
could cause a segmentation fault.

9.0.304 Dev.23 4 Sep 2018

After using Dev.22 heavily myself for 3 years, I put it up for download as
Dev.23 on 31 January 2020 but dated 4 Sep 2018.

9.0.304 Dev.22 30 April 2017

Improved parsing of macro arguments that are grouped versus those that
contained grouping characters as data.  Command-parsing error messages
improved somewhat but more work is needed.  Some improvements to \fsplit().
Some improvements to the handling of built-in internal macros such as IF,
FOR, and SWITCH.  The command parsing changes were actually quite massive
and fundamental, so this version was never uploaded to the Web/FTP site.

9.0.304 Dev.21 21 April 2017

A development (pre-alpha, pre-beta) release.  The only siginifcant change is
a new ability to allow the FOPEN command to "open" the standard input stream
via a new command FOPEN /STDIN.  This allows Kermit scripts to read the
output of another program through a pipe, as in Unix, using its regular
FREAD command.  There are also some minor additions to the S-Expression
(LISP) command repertoire.  Dev.20 of February 16, 2016, was just a bug fix
for Mac OS X.

Fri Feb  5 20:38:11 2016

This is C-Kermit 9.0.304 Dev.19, a development (pre-alpha, pre-beta)
version of the first new release of C-Kermit since the cancelation of
the Kermit Project at Columbia University.  Since 1 July 2011, C-Kermit
is Open Source software, released under the modified 3-clause Berkeley
License.  The changes since version 9.0.302 are detailed at the bottom
of the NOTES.TXT file.

Major New Features:

 . Android support.
 . The CHANGE command (type "help change" for details).
 . The TOUCH command (type "help touch" for details).
 . Locale support for dates, times, error messages, etc.
 . A way to direct messages to stderr.
 . A new \ffilecompare() function.
 . GREP /ARRAY:&x puts the results in the array &x.

Major Problems Fixed:

 1. Crash when receiving files with Kermit protocol on certain 64-bit
    platforms such as OpenBSD on Sparc64, caused by conflicting int/long
    declarations.

 2. Custom builds using certain combinations of feature-selection flags
    would fail.

 3. Linux builds becoming increasingly problematic with the neverending
    proliferation of Linux distributions, each one doing things
    differently.

 4. Problems parsing certain REMOTE commands.

 5. DIRECTORY /BRIEF /EXCEPT:xxx didn't work, now it does.

 6. Numerous problems with OpenSSL as it changes out from under us.a

All these need testing.  To see a summary of the changes in each
development version, see:

  http://www.kermitproject.org/ckdaily.html

The current development version is packaged as follows:

 1. In ftp://ftp.kermitproject.org/kermit/test/text/ : 
    
    README.TXT           This file
    cku304dev19.tar      Unix source code, tar archive
    cku304dev19.tar.Z    Ditto, compressed
    cku304dev19.tar.gz   Ditto, gzipped
    cku304dev19.zip      Unix and VMS source code, Zip archive

 2. In ftp://ftp.kermitproject.org/kermit/test/text/ : 

    Individual source files

The documentation files that accompany a real release are not included,
since they have not changed and remain available in the normal places:

  http://www.kermitproject.org/ckermit.html     (C-Kermit Web)
  ftp://ftp.kermitproject.org/kermit/ckermit/   (C-Kermit FTP area)
  ftp://ftp.kermitproject.org/kermit/archives/  (Tar/Zip FTP area)

Executable binaries are not distributed because there is not enough space or
bandwidth for them on the Kermit Project's new host.  Binaries prior to
October 2011 remain available on the Columbia University Kermit website
(which is frozen and will not change):

  http://kermit.columbia.edu/                   (CU Kermit home page)
  http://kermit.columbia.edu/ckbinaries.html    (C-Kermit binaries)

Unpacking and building instructions are here:

  http://www.kermitproject.org/ckdaily.html

Briefly: Unpack the archive into a fresh directory and run the build
procedure.  In Unix do "make linux" (or whatever); in VMS do "@ckvker.com".
Upon success, the result will be an executable wermit file (WERMIT.EXE
in VMS) in the same directory.  It's called wermit instead of kermit
to avoid overriding any production version that might be in the same
directory.

Frank da Cruz
Bronx, NY
