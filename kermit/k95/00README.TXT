
   [1]The Columbia Crown The Kermit Project | Columbia University
   612 West 115th Street, New York NY 10025 USA o [2]kermit@columbia.edu
   ...since 1981
   [3]Home [4]Kermit 95 [5]C-Kermit [6]Scripts [7]Current [8]New [9]FAQ
   [10]Support

   Download:   [11]Source files   [12]Zip archive   Libraries:   [13]MIT
   Kerberos   [14]OpenSSL   [15]OpenSSH

KERMIT 95 SOURCE FILES

     Frank da Cruz
     [16]fdc@columbia.edu
     20 July 2011
     Most recent update:  Thu Jul 21 10:23:02 2011

   As part of the [17]cancellation of the Kermit Project, the source code
   for Kermit 95 (K95 for short, the Kermit software for 32-bit and 64-bit
   versions of Microsoft Windows and for IBM OS/2, which previously was
   available only as a commercial product) is being published for the
   first time under the open-source [18]Revised 3-Clause BSD License so
   that anybody who wants to can download, build, customize, embed,
   package, redistribute, or do anything else with it that is legal and
   permitted by the license. It is hoped that this will result in the Open
   Source community creating a new and free version of Kermit 95. In the
   meantime, those who still need a ready-to-run Kermit 95 application can
   obtain it from the retailers listed on the [19]Kermit 95 page, and
   those who need bulk licenses can obtain them from [20]Columbia
   Technology Ventures.

   The new [21]k95source directory in the Kermit software archive now
   contains the Columbia-written source files for Kermit 95 2.1.3,
   released 1 January 2003, with updates by Jeffrey Altman through 2007,
   which are listed [22]HERE.

   This source code does not include the various libraries with which K95
   is linked, notably [23]MIT Kerberos and [24]OpenSSL. Those libraries
   have undergone considerable change and development since K95 2.1.3 was
   released, and any new release of K95 should be adapted to the current
   releases (use buttons above).

   The modules which are shared with C-Kermit for Unix, VMS, and other
   platforms (ckc*.*, cku*.*) correspond to those of C-Kermit 8.0.207
   (which was never formally released), but with updates either from the
   main C-Kermit development branch, or K95-specific by Jeff. No attempt
   has been made to reconcile these sources with those of [25]C-Kermit
   9.0; that would be a big job indeed (but one that is worth doing if a
   new Open Source Kermit 95 release is to be created). Only the minimum
   changes to convert the K95 source code to an Open Source license have
   been made:

     * A new [26]COPYING.TXT file.
     * New copyright notices in [27]ckcmai.c (the K95 main program
       module).
     * Elimination of restrictive license text from all files that are
       Copyright Trustees of Columbia University in the City of New York
       (these files have 19 July 2011 modification dates). The COPYING.TXT
       file applies to all files in this directory bearing a Columbia
       University copyright.
     * Exclusion of any module whose license prohibits open distribution
       (primarily the XYZMODEM code).

   No attempt has been made to compile or link these source files on
   Windows or OS/2 or anywhere else (actually I did attempt it, but did
   not get very far; see below).

   Any new version of K95 will have to deal with the following
   considerations:

     * A new (non-Installshield) installer is needed.
     * Many bugs need to be addressed. They are listed [28]HERE and
       [29]HERE.
     * Assorted ideas (mine) for a new release are [30]HERE.
     * Large file support should be added (K95 uses 32-bit C Library APIs
       for file access). C-Kermit itself supports large files so it's
       "just" a matter of changing the file i/o routines in ckufio.c (?).
     * All registration, serial number, and license enforcement code
       should be removed.
     * There can be no Dialer unless a new one is written from scratch.
       The Dialer was built using a commercial GUI-builder called Zinc;
       that company ceased to exist many years ago. Columbia had a source
       license to Zinc and made modifications and bug fixes necessary to
       build the Dialer. But Columbia does not have the right to
       distribute Zinc nor any patches to it.
     * If there is no Dialer, the Dialer activation button should be
       removed from the K95G.EXE Toolbar.
     * The Console version for Windows is obsolete since later Windows
       versions do not support it; only the GUI version (K95G.EXE) should
       be built. Also there is probably not much reason to carry forward
       support for Window 95, 98, and ME.
     * The XYZMODEM protocol code was contributed from Finland. The
       license under which this code was included in K95 (see [31]below)
       forbids publication of the source code.
     * The SSH code contains known security weaknesses that need to be
       fixed. K95's SSH module was adapted from a 2002 version
       [32]OpenSSH, but OpenSSH itself has changed so much since then that
       a new SSH module should be developed from scratch.
     * Other security code (Kerberos, SSL/TLS, and SRP) needs to brought
       up to date with current releases of those libraries.
     * To take advantage of all the new developments in C-Kermit since
       2007 and to return to a common code base for C-Kermit and K95, K95
       should be integrated with the C-Kermit source files (ck[cu]*.[ch]
       and ckcpro.w). The current release of C-Kermit is [33]here.

   K95 was last built with Microsoft Visual Studio 6.0. Some adaptation
   may be required to build it with current Microsoft developers tools
   and/or non-Microsoft tools.

   K95-specific source files have names starting with "cko" and "ckn".
   These files were not written to be read by the public so in some cases
   comments might be inexistent, sketchy, or misleading.

   The makefile is [34]ckoker.mak. Before running it, it's necessary to
   set the "include" and "lib" environment variables for the external
   libraries K95 is linked with (such as OpenSSL, SRP, and Kerberos), for
   example [each of these is one long line broken for readability]:

    set include=%include%;c:\src\kermit\k95;
  c:\src\openssl\0.9.7\inc32;
  c:\src\srp\include;
  c:\src\pwsdk\inc32;
  c:\src\kerberos\kfw-2.2-beta-2\athena\wshelper\include;
  c:\src\superlat\include;
  c:\src\kerberos\kfw-2.2-beta-2\athena\auth\krb5\src\include;
  c:\src\kermit\k95\kui;
  c:\src\zinc\include;

    set lib=%lib%;c:\src\kerberos\kfw-2.2-beta-2\target\lib\i386\rel;
  c:\src\zinc\lib\mvcpp500

   Note that Zinc is not included in this distribution, nor Meridian
   Technologies SuperLAT, support for which was included in K95 under
   license. Any -DSUPERLAT definition should be removed the makefile. I'm
   not sure how avoid Zinc. Once the lib and include environments are
   defined, the makefile can be invoked something like this:

  SET PLATFORM=NT
  SET K95BUILD=K95
  nmake /nologo /e /f ckoker.mak msvc |& tee comp.out.nt | list /s

   In the source code and makefile you will see many references to "KUI".
   This was a 100% graphical version of K95 that was never completed. The
   sources contain a fair amount of vestigial KUI code.

   The principal authors of Kermit 95, Jeff Altman and Frank da Cruz, no
   longer have jobs at Columbia University and may or may not be available
   for advice due to the exigencies of real life.

Appendix: Directory Structure

   The makefile [35]ckoker.mak assumes data files for Kermit 95 are in a
   directory tree (lowercase names are directories):
     * Root (name doesn't matter)
          + SETENV.BAT - Defines the "include" and "lib" environment
            variables
          + infozip - Zip and Unzip sources and binaries (not used in
            build)
          + kerberos - Kerberos for Windows sources, libraries, and
            utilities
          + kermit - Kermit 95 source files
               o cif - k95_21_master.cif file used in creating CD image
                 (not used in build)
               o dialer - source code for K95 Dialer (not released because
                 there is no way to build it)
               o k95 - Source code for Kermit 95 itself, plus .BAT files
                 for launching builds of various configurations, plus the
                 following subdirectories:
                    # iksd - Directory used for object files when building
                      [36]WIKSD
                    # kui - Directory used for object files for building
                      KUI (see above)
                    # nt - Directory for building Kermit 95
                    # ssh - OpenSSH libraries and other materials
                    # k95dlls - Example of how to create a K95 DLL (not
                      used in build)
                    # k95inst - Files used in K95 installer and
                      registration (not used in build)
                    # p - Original XYZMODEM code Oy Online Solutions Ltd
                      including [37]license
                    # p95 - XYZMODEM code modified for use in K95
          + openssl - OpenSSL
          + pwsdk - Digital Equipment Corporation PATHWORKS Software
            Developers Kit
          + srp - Stanford Secure Remote Password libraries
          + superlat - Meridian Technologies SuperLAT
          + zinc - the Zinc GUI developer

   Using this arrangement, you could first CD to the root directory,
   execute SETENV.BAT, then CD to the Kermit subdirectory, then CD to the
   k95 directory and execute the makefile.

   I tried this myself with mknt.bat, but evidently these batch files are
   to be used with some other shell than CMD.EXE because it fails with "&
   was not expected at this time". Giving the nmake command without the
   redirectors:

     nmake /nologo /e /f ckoker.mak msvc

   failed because nmake wasn't found. I located it in:

     C:\Program Files\Microsoft Visual Studio\VC98\Bin\nmake.exe

   and set my PATH accordingly, tried the nmake command again but it
   failed with "mspdb60.dll not found" and advised me to "reinstall". I'm
   not a Windows programmer, so that's as far as I got. If I obtain any
   new information and I still have access to this page, I will improve
   these instructions.

Appendix: XYZMODEM Code ("P") License

   LICENSE AGREEMENT (21 July 1995)

     (1) Introduction: This agreement details when and how you may use
     the source code aquired from Oy Online solutions Ltd.

     In this agreement:

     * "Program" means the binary code generated using "P" source code or
       portions of it.
     * "source" or "source code" means the "P" source code.
     * "we" or "us" means Oy Online Solutions Ltd., a corporation based in
       Jyvaskyla, FINLAND.
     * "you" means the person or persons working for the Kermit project at
       the Columbia University in the City of New York, USA.

     (2) Copyright: "P" and its source code is protected by copyright
     under Finnish law and international treaty provisions.

     Rights have been granted to the Columbia University in the City of
     New York to include "P" with its C-Kermit program in compiled form.
     Responsibilty for the program stays with Oy Online Solutions Ltd.,
     Finland.

     Any binaries generated from this source code or portions of it must
     mention, where it is appropriate, the copyright holder. (e.g.
     Portions Copyright (c) 1995 Oy Online Solutions Ltd.)

     (3) Use of the source code: The source code is released to be used
     only by the persons working for the Kermit project at Columbia
     University in the City of New York, USA. Such persons may use this
     source code only to make binaries to be released with C-Kermit, not
     as an individual program.

     (4) Distibution of source code: Distribution of source code, or
     portions of it and any documentation relating to it in any form,
     file or hardcopy, is strictly forbidden.

     You may distribute program generated from the source code to be used
     in accordance with C-Kermit.

     (5) DISCLAIMER AND LIMITATIONS:

     THE SOURCE CODE IS PROVIDED "AS-IS" AND WITHOUT ANY WARRANTY OF ANY
     KIND.

   END OF LICENSE AGREEMENT
     __________________________________________________________________


    Kermit 95 Source Files / Columbia University /
    [38]kermit@columbia.edu / July 2011

References

   1. http://www.columbia.edu/
   2. mailto:kermit@columbia.edu
   3. http://www.columbia.edu/kermit/index.html
   4. http://www.columbia.edu/kermit/k95.html
   5. http://www.columbia.edu/kermit/ckermit.html
   6. http://www.columbia.edu/kermit/ckscripts.html
   7. http://www.columbia.edu/kermit/current.html
   8. http://www.columbia.edu/kermit/whatsnew.html
   9. http://www.columbia.edu/kermit/k95faq.html
  10. http://www.columbia.edu/kermit/support.html
  11. ftp://kermit.columbia.edu/kermit/k95source
  12. ftp://kermit.columbia.edu/kermit/archives/k95source.zip
  13. http://web.mit.edu/kerberos/
  14. http://www.openssl.org/
  15. http://www.openssh.com/
  16. mailto:fdc@columbia.edu
  17. http://www.columbia.edu/kermit/announce.html
  18. http://www.columbia.edu/kermit/cu-bsd-license.html
  19. http://www.columbia.edu/kermit/k95.html
  20. http://techventures.columbia.edu/kermit/
  21. ftp://kermit.columbia.edu/kermit/k95source
  22. http://www.columbia.edu/kermit/k95-fixes-since-213.txt
  23. http://web.mit.edu/kerberos/
  24. http://www.openssl.org/
  25. http://www.columbia.edu/kermit/ck90.html
  26. ftp://kermit.columbia.edu/kermit/k95source/COPYING.TXT
  27. ftp://kermit.columbia.edu/kermit/k95source/ckcmai.c
  28. http://www.columbia.edu/kermit/k95faq.html#bugs
  29. ftp://kermit.columbia.edu/kermit/k95/newbugs.txt
  30. ftp://kermit.columbia.edu/kermit/k95source/NOTES.TXT
  31. http://www.columbia.edu/kermit/k95sourcecode.html#plicense
  32. http://www.openssh.com/
  33. http://www.columbia.edu/kermit/ckermit.html
  34. ftp://kermit.columbia.edu/kermit/k95source/ckoker.mak
  35. ftp://kermit.columbia.edu/kermit/k95source/ckoker.mak
  36. http://www.columbia.edu/kermit/wiksd.html
  37. http://www.columbia.edu/kermit/k95sourcecode.html#license
  38. mailto:fdc@columbia.edu
