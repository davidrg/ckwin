C-KERMIT 9.0.304 PRERELEASE TESTING

  Fri Jul 20 19:39:51 2012

This is C-Kermit 9.0.304 Dev.02, the first public test version
of C-Kermit since the cancelation of the Kermit Project at Columbia
University.  C-Kermit is Open Source software, released under the
modified 3-clause Berkely License.  The changes since version 9.0.302
are detailed at the bottom of the NOTES.TXT file, but briefly:

 1. Android support from Tim Sneddon.  This follows the Linux
    path, but has a separate makefile, android.mk.  A brief 
    cover note is in the android.txt file, and Tim's story about
    this came about is here:

      http://tim.sneddon.id.au/blog/Posts/C-Kermit_for_Android

    Tim's version is based on 9.0.302, whereas the version here also
    embodies the other recent changes:

 2. C-Kermit 9.0 already worked with OpenSSL 1.0.0, but it did not
    react well when OpenSSL was updated; C-Kermit would have to be
    rebuilt each time, even if it was unnecessary according to the
    post-1.0.0 OpenSSL rules for ABI compatibility.  C-Kermit
    9.0.304 has new code from Adam Friedlander to perform the check
    according to the new rules, which weren't set when C-Kermit 9.0
    was first released.  This way, users of binary-only
    distributions like Ubuntu Linux won't have C-Kermit stop working
    for them suddenly for no good reason.  So now if C-Kermit was
    compiled against OpenSSL 1.0.0 or greater, it will disable SSL
    connections only if the installed version has a different major
    and minor version number *or* the installed version is older
    than the version C-Kermit was compiled against.  If C-Kermit was
    compiled against a 0.9.x OpenSSL version, it will disable SSL
    connections unless the installed version is exactly the same.
    Thanks to Jeff Altman for help with this fix.

Problems Fixed:

 1. Crashes when receiving files with Kermit protocol on certain 64-bit
    platforms such as OpenBSD on Sparc64, caused by conflicting int/long
    declarations.

 2. Custom builds using certain combinations of feature-selection flags
    would fail.

 3. Debian Linux builds did not not allow for multiarchitecture
    libraries.  This was fixed in the linux target of the makefile
    by Ian Beckwith of the Debian Project.

All these need testing.

The test version is packaged as follows:

 1. In ftp://ftp.kermitproject.org/kermit/test/text/ : 
    
    README.TXT           This file
    cku304dev02.tar      Unix source code, tar archive
    cku304dev02.tar.Z    Ditto, compressed
    cku304dev02.tar.gz   Ditto, gzipped
    cku304dev02.zip      Unix and VMS source code, Zip archive

 2. In ftp://ftp.kermitproject.org/kermit/test/text/ : 

    Individual source files

The documentation files that accompany a real release are not included,
since they have not changed and remain available in the normal places:

  http://www.kermitproject.org/ckermit.html     (C-Kermit Web)
  ftp://ftp.kermitproject.org/kermit/ckermit/   (C-Kermit FTP area)
  ftp://ftp.kermitproject.org/kermit/archives/  (Tar/Zip FTP area)

Executable binaries are not distributed because there is not enough
space or bandwidth for them on the Kermit Project's new host.
Binaries prior to October 2011 remain available on the Columbia
University Kermit website (which is frozen and will not change):

  http://kermit.columbia.edu/                   (CU Kermit home page)
  http://kermit.columbia.edu/ckbinaries.html    (C-Kermit binaries)

Unpacking and building instructions are here:

  http://www.kermitproject.org/ckdaily.html

Briefly: Unpack the archive into a fresh directory and run the build
procedure.  In Unix do "make linux" (or whatever); in VMS do "@ckvker.com".
Upon success, the result will be an executable wermit file (WERMIT.EXE
in VMS) in the same directory.

Frank da Cruz
New York City
