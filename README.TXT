C-KERMIT 9.0.304 PRERELEASE TESTING

Fri Apr 21 15:37:35 2017

C-Kermit 9.0.304 Dev.21, a development (pre-alpha, pre-beta).  The only
siginifcant change is a new ability to allow the FOPEN command to "open"
the standard input stream via a new command FOPEN /STDIN.  This allows 
Kermit scripts to read the output of another program through a pipe,
as in Unix, using its regular FREAD command.  There are also some minor
additions to the S-Expression (LISP) command repertoire.  Dev.20 of
February 16, 2016, was just a bug fix for Mac OS X.

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
