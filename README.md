C-Kermit for Windows
====================

This is C-Kermit for Windows. It is a free and open source version of the
program formerly known as Kermit-95, a commercial product of Columbia
University from 1994 to 2011. For more information on Kermit, visit the
Kermit Project website: http://www.kermitproject.org.

This software is currently based on C-Kermit version 9.0.304 Dev.13 of
6-FEB-2015.

OS/2 support is still present but untested. It should still work if someone can
get it building with freely available tools.

This version has been released under the 3-clause BSD license and does not
include any features which rely on proprietary libraries or SDKs. This
includes:

* DECnet (formerly provided by DEC PATHWORKS) 
* LAT (formerly provided by either SuperLAT or PATHWORKS)
* X, Y and Z MODEM (formerly provided by a library called 'P' from Oy Online 
Solutions Ltd).
* The Dialer (relied on Zinc, a proprietary GUI framework)
* SSH

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

To build Kermit for Windows you will need Microsoft Visual 2019 or
higher. 

To build open the solution <your src directory>kermit\k95\build\ckwin.sln

Building this should leave you with a number of binaries in the current directory, the
most interesting being:

* cknker.exe   - Console version of C-Kermit for Windows
* k95g.exe     - Graphical version of C-Kermit for Windows

Future stuff to do:
-------------------
* Restore use of fsetpos in ckofio.c (see function zfseek(CK_OFF_T) around
  line 5418)
* Turn long long support back on (remove -DNOLONGLONG in makefile)
* Re-enable/rewrite features that were disabled due to missing or obsolete
dependencies. This will require upgrading to current versions or finding/writing
replacements.
  - SSH
  - Kerberos (use Heimdal instead of MIT Kerberos for Windows)
  - zlib
  - SSL (upgrade to current OpenSSL release)
  - SRP
  - Z/Y/Z Modem 
  - LAT (port from linux-decnet?)
