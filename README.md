C-Kermit for Windows
====================

This is C-Kermit for Windows. It is a free and open source version of the
program formerly known as Kermit-95, a commercial product of Columbia
University from 1994 to 2011. For more information on Kermit, visit the
Kermit Project website: http://www.kermitproject.org.

This software is currently based on C-Kermit version 10.0 Beta.04 of
3-JUN-2022. 

OS/2 support is still present but untested. It was last built with the
IBM VisualAge compiler but has not been tested in a long time. Future 
OS/2 support should probably be achieved using OpenWatcom.

This version has been released under the 3-clause BSD license and does not
include any features which rely on proprietary libraries or SDKs. This
includes:

* DECnet (formerly provided by DEC PATHWORKS) 
* LAT (formerly provided by either SuperLAT or PATHWORKS)
* X, Y and Z MODEM (formerly provided on windows by a custom port of an OS/2 library 
    called 'P' written by Jyrki Salmi and now owned by [Oy Online Solutions Ltd](https://online.fi)
* The Dialer (relied on a modified version of Zinc 4.2, a 
[formerly proprietary GUI framework](http://openzinc.com/))
* SSH (based on OpenSSH)

A number of other features have been removed as they relied on ancient
versions of 3rd-party libraries. These are:

* Kerberos
* SSL
* SRP

Other features may be missing as a result of the above features being disabled.
For a full list of features available, type the following at the Kermit prompt:

        SHOW FEATURES

This code is based on what was going to be Kermit 95 v2.2. Compared to the
final K95 release (2.1.3) a number of bugs have been fixed and a few new
features have been added. A full list of these is available here:

  http://www.kermitproject.org/k95-fixes-since-213.txt

Compiling
---------

To build C-Kermit for Windows you will need either Microsoft Visual C++ 6.0 SP6 or newer 
(Visual C++ 2022 is the newest tested version), or OpenWatcom 1.9 (other versions are untested but may work).

Edit `/setenv.bat` and change `set root=` to point to your source directory then run through the following:
 
1. Open up a console window
2. Setup the environment for your compiler. For Visual C++ you'll want to run `vcvars32.bat` or `vcvarsall.bat`
   (depending on your compiler version) or `owsetenv.bat` for OpenWatcom. This will be where ever you installed your compiler. For example:

        C:\Program Files\Microsoft Visual Studio 8\VC\bin\vcvars32.bat

   For OpenWatcom, it will be `owsetenv.bat`:

        C:\WATCOM\owsetenv.bat

4. CD into your source directory and run the following:

        setenv.bat
        cd kermit/k95
        mk.bat
        mkdist.bat

This should leave you with a number of bits in the dist subdirectory, the most
most interesting being:

* k95.exe      - Console version of C-Kermit for Windows
* k95g.exe     - Graphical version of C-Kermit for Windows

Future stuff to do:
-------------------
* Remove need for /noBool switch and #defines in kui code.
* Restore use of fsetpos in ckofio.c (see function zfseek(CK_OFF_T) around
  line 5418)
* Port to OpenWatcom and revive the OS/2 version of C-Kermit.
* Re-enable/rewrite features that were disabled due to missing or obsolete
dependencies. This will require upgrading to current versions or finding/writing
replacements.
  - SSH
  - Kerberos (use Heimdal instead of MIT Kerberos for Windows)
  - zlib
  - SSL (upgrade to current OpenSSL release)
  - SRP
  - Z/Y/Z Modem (Ask Oy Online Solutions Ltd for a relicensed library?)
  - LAT (port from linux-decnet? Ask [VSI](https://vmssoftware.com/) for the pathworks SDK?)

Shared Codebase
---------------
Any files matching the pattern ck[cu]*.[cwh] are shared by
implementations of C-Kermit for other platforms (UNIX, Linux, VMS, and others)
and are not specific to the Windows and OS/2 port in this repository. Any changes 
to these files should be sent to [The Kermit Project](https://www.kermitproject.org/)
to be included in future C-Kermit releases for other platforms.