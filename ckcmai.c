/*
  ckcsym.h is used for for defining symbols that normally would be defined
  using -D or -d on the cc command line, for use with compilers that don't
  support this feature.  Must be before any tests for preprocessor symbols.
*/
#include "ckcsym.h"

char *ck_ver = "C-Kermit 6.0.192";	/* C-Kermit Version */
#ifdef UNIX
static char sccsid[] = "@(#)C-Kermit 6.0.192";
#endif /* UNIX */

#ifndef MAC
#ifdef OS2
#ifdef NT
/* Version herald */
char *versio = "Kermit-95 1.1.8, 21 Nov 96"; /* Version herald */
long xvernum = 118L;
#else /* NT */
char *versio = "Kermit-95 for OS/2 1.1.8 Beta, 23 Nov 96"; /* Version harald */
long xvernum = 118L;
#endif /* NT */
#else /* OS2 */
char *versio = "C-Kermit 6.0.192, 6 Sep 96";
long xvernum = 0L;
#endif /* OS2 */
#else
/*
  For Macintosh, also remember to change the Mac-specific version in ckmkr2.r.
*/
char *versio = "Mac Kermit 0.993 Pre-Alpha, 6 Sep 96";
long xvernum = 993L;
#endif /* MAC */

long vernum = 600192L;
/*
  String and numeric version numbers, keep these three in sync!
  First digit of vermum = major version, i.e. 5.
  Second 2 digits of vernum: 00 = no minor version, 01 = A, 02 = B, etc.
  Last three digits are edit number. 
*/
#ifndef VERWHO
/* Change verwho in following line, or with -DVERWHO=x in makefile CFLAGS. */
#define VERWHO 0
#endif /* VERWHO */
int verwho = VERWHO; /* Who produced this version, 0 = Columbia University */
/*
  IMPORTANT: If you are working on your own private version of C-Kermit, please
  include some special notation, like your site name or your initials, in the
  "versio" string, e.g. "6.0.192-XXX", and use a nonzero code for the "verwho"
  variable (e.g. in the USA use your zip code).  Unless we stick to this
  discipline, divergent copies of C-Kermit will begin to appear that are
  intistinguishable from each other, which is a big support issue.  Also, if
  you have edited C-Kermit and made copies available to others, please add
  appropriate text to the BUG command (ckuus6.c, function dobug()).
*/
#define CKCMAI

/*  C K C M A I  --  C-Kermit Main program  */

/*
  Author: Frank da Cruz (fdc@columbia.edu),
  Columbia University Academic Information Systems, New York City.

COPYRIGHT NOTICE:
*/

char *copyright[] = {

#ifdef pdp11
"Copyright (C) 1985, 1996, Trustees of Columbia University, NYC.",
"All rights reserved.",
#else
#ifdef OS2
"Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New",
"York.  All rights reserved.  This software is furnished under license",
"and may not be reproduced without license to do so.  This copyright notice",
"must not be removed, altered, or obscured.",
#else
"Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New",
"York.  The C-Kermit software may not be, in whole or in part, licensed or",
"sold for profit as a software product itself, nor may it be included in or",
"distributed with commercial products or otherwise distributed by commercial",
"concerns to their clients or customers without written permission of the",
"Kermit Project, Columbia University.  This copyright notice must not be",
"removed, altered, or obscured.",
#endif /* OS2 */
#endif /* pdp11 */
#ifdef XYZ_INTERNAL
" ",
"Portions Copyright (C) 1995 Oy Online Solutions Ltd., Jyvaskyla, FINLAND\n",
#endif /* XYZ_INTERNAL */
#ifndef pdp11
" ",
"For further information, contact the Kermit Project, Columbia University,",
"612 West 115th Street, New York NY 10025-7799, USA; phone +1 (212) 854 3703,",
"fax +1 (212) 663 8202 or +1 (212) 662 6442, email kermit@columbia.edu,",
"Web http://www.columbia.edu/kermit/.",
#endif /* pdp11 */
""};

/*
DOCUMENTATION:

 "Using C-Kermit" by Frank da Cruz and Christine M. Gianone,
  Digital Press / Butterworth-Heinemann, Woburn MA, USA.
  Second edition (1997), ISBN 1-55558-164-1.
  Order from Digital Press:    +1 (800) 366-2665
  Or from Columbia University: +1 (212) 854-3703

For Kermit 95, also:

  "Kermit 95" by Christine M. Gianone and Frank da Cruz,
  Manning Publications, Greenwich CT, USA (1996)
  Order from Manning:          +1 (203) 629 2078
  Or from Columbia University: +1 (212) 854-3703

DISCLAIMER:

  The C-Kermit software is provided in source code and/or binary form by
  the Kermit Project, Academic Information Systems, Columbia University.
  The software is provided "as is;" no other warranty is provided, express 
  or implied, including without limitations, any implied warranty of
  merchantability or implied warranty of fitness for a particular purpose.

  Neither Columbia University nor any of the contributors to C-Kermit's
  development, including, but not limited to, Manning Software, Digital Press,
  AT&T, Digital Equipment Corporation, Data General Corporation, or
  International Business Machines Corporation, or any individuals affiliated
  with those or other institutions, warrant C-Kermit software or documentation
  in any way.  In addition, neither the authors of any Kermit programs,
  publications or documentation, nor Columbia University nor any contributing
  institutions or individuals acknowledge any liability resulting from program
  or documentation errors.

  Contributions made to C-Kermit by programmers outside of Columbia University
  fall within the provisions of the foregoing copyrights, terms and
  conditions, and disclaimers, and grant to the Kermit Project at Columbia
  University a nonexclusive license to use the contributed code in any and
  all Kermit software without restriction or obligation.

ACKNOWLEDGMENTS:

  The Kermit file transfer protocol was developed at the Columbia University
  Center for Computing Activities (CUCCA), which was since renamed to Columbia
  University Academic Information Systems (AcIS).  Kermit is named after
  Kermit the Frog, star of the television series THE MUPPET SHOW; the name is
  used by permission of Henson Associates, Inc.

  Thanks to at least the following people for their contributions to this
  program over the years, and apologies to anyone who was inadvertantly
  omitted:

   Chris Adie, Edinburgh U, Scotland (OS/2)
   Robert Adsett, University of Waterloo, Canada
   Larry Afrin, Clemson U
   Jeffrey Altman, Columbia University
   Greg Andrews, Telebit Corp
   Barry Archer, U of Missouri
   Robert Andersson, International Systems A/S, Oslo, Norway
   Chris Armstrong, Brookhaven National Lab (OS/2)
   William Bader, Software Consulting Services, Nazareth, PA
   Fuat Baran, Columbia U
   Stan Barber, Rice U
   Jim Barbour, U of Colorado
   Donn Baumgartner, Dell
   Nelson Beebe, U of Utah
   Karl Berry, UMB
   Mark Berryman, SAIC
   Dean W Bettinger, SUNY
   Gary Bilkus
   Peter Binderup, Denmark
   David Bolen, Advanced Networks and Services, Inc.
   Marc Boucher, U of Montreal
   Charles Brooks, EDN
   Bob Brown
   Mike Brown, Purdue U
   Jack Bryans, California State U at Long Beach
   Mark Buda, DEC (VMS)
   Fernando Cabral, Padrao iX, Brasilia
   Bjorn Carlsson, Stockholm University Computer Centre QZ, Sweden
   Bill Catchings, (formerly of) Columbia U
   Bob Cattani, Columbia U CS Dept
   Davide Cervone, Rochester U
   Seth Chaiklin, Denmark
   John Chandler, Harvard U / Smithsonian Astronomical Observatory
   Bernard Chen, UCLA
   Andrew A Chernov, RELCOM Team, Moscow
   John L Chmielewski, AT&T, Lisle, IL
   Howard Chu, U of Michigan
   Bill Coalson, McDonnell Douglas
   Bertie Coopersmith, London
   Chet Creider, U of Western Ontario
   Alan Crosswell, Columbia U
   Jeff Damens, (formerly of) Columbia U
   Mark Davies, Bath U, UK
   Sin-itirou Dezawa, Fujifilm, Japan
   Joe R. Doupnik, Utah State U
   Frank Dreano, Honeywell
   John Dunlap, U of Washington
   Alex Dupuy, SMART.COM
   David Dyck, John Fluke Mfg Co.
   Stefaan A. Eeckels, Eurokom, Luxembourg
   Paul Eggert, Twin Sun, Inc., El Segundo, CA
   Bernie Eiben, DEC
   Peter Eichhorn, Assyst International
   Kristoffer Eriksson, Peridot Konsult AB, Oerebro, Sweden
   John R. Evans, IRS, Kansas City
   Glenn Everhart, RCA Labs
   Charlie Finan, Cray Research
   Herm Fischer, Encino, CA (extensive contributions to version 4.0)
   Carl Fongheiser, CWRU
   Mike Freeman, Bonneville Power Authority
   Marcello Frutig, Catholic University, Sao Paulo, Brazil (X.25 support)
   Hirofumi Fujii, Japan Nat'l Lab for High Energy Physics, Tokyo (Kanji)
   Chuck Fuller, Westinghouse Corporate Computer Services
   Andy Fyfe, Caltech
   Christine M. Gianone, Columbia U
   John Gilmore, UC Berkeley
   Madhusudan Giyyarpuram, HP
   Rainer Glaschick, Siemens AG, Paderborn
   William H. Glass
   German Goldszmidt, IBM
   Chuck Goodhart, NASA
   Alistair Gorman, New Zealand
   Richard Gration, ADFA, Australia
   Chris Green, Essex U, UK
   Alan Grieg, Dundee Tech, Scotland
   Yekta Gursel, MIT
   Jim Guyton, Rand Corp
   Michael Haertel
   Bob Hain, UMN
   Marion Hakanson, ORST
   John Hamilston, Iowa State U
   Simon Hania, Netherlands
   Stan Hanks, Rice U.
   Ken Harrenstein, SRI
   Eugenia Harris, Data General (AOS/VS)
   David Harrison, Kingston Warren Corp
   James Harvey, Indiana/Purdue U (VMS)
   Rob Healey
   Chuck Hedrick, Rutgers U
   Ron Heiby, Technical Systems Division, Motorola Computer Group
   Steve Hemminger, Tektronix
   Christian Hemsing, RWTH Aachen, Germany (OS-9)
   Andrew Herbert, Monash Univ, Australia
   Mike Hickey, ITI
   Dan Hildebrand, QNX Software Systems Inc, Kanata, ON (QNX)
   R E Hill
   Bill Homer, Cray Research
   Ray Hunter, The Wollongong Group
   Randy Huntziger, National Library of Medicine
   Larry Jacobs, Transarc
   Steve Jenkins, Lancaster University, UK
   Dave Johnson, Gradient Technologies
   Mark B Johnson, Apple Computer
   Jyke Jokinen, Tampere University of Technology, Finland (QNX)
   Eric F Jones, AT&T
   Luke Jones, AT&T
   Peter Jones, U of Quebec Montreal
   Phil Julian, SAS Institute
   Peter Kabal, U of Quebec
   Mic Kaczmarczik, U of Texas at Austin
   Sergey Kartashoff, Inst. of Precise Mechanics & Computer Equipment, Moscow
   Howie Kaye, Columbia U
   Rob Kedoin, Linotype Co, Hauppauge, NY (OS/2)
   Phil Keegstra
   Mark Kennedy, IBM
   Terry Kennedy, St Peter's College, Jersey City, NJ (VMS and more)
   "Carlo Kid", Technical University of Delft, Netherlands
   Tim Kientzle
   Paul Kimoto, Cornell U
   Douglas Kingston, morgan.com
   Lawrence Kirby, Wiltshire, UK
   Tom Kloos, Sequent Computer Systems
   Jim Knutson, U of Texas at Austin
   John T. Kohl (BSDI)
   Scott Kramer, SRI International, Menlo Park, CA
   John Kraynack, US Postal Service
   David Kricker, Encore Computer
   Thomas Krueger, UWM
   Bo Kullmar, ABC Klubben, Stockholm, and Central Bank of Sweden, Kista
   R. Brad Kummer, AT&T Bell Labs, Atlanta, GA
   John Kunze, UC Berkeley
   David Lane, BSSI / BellSouth (Stratus VOS, X.25)
   Bob Larson, USC (OS-9)
   Bert Laverman, Groningen U, Netherlands
   Steve Layton
   David Lawyer, UC Irvine
   David LeVine, National Semiconductor Corporation
   Daniel S. Lewart, UIUC
   S.O. Lidie, Lehigh U
   Tor Lillqvist, Helsinki U, Finland
   David-Michael Lincke, U of St Gallen, Switzerland
   Robert Lipe
   Dean Long
   Mike Long, Analog Devices, Norwood MA
   Kevin Lowey, U of Saskatchewan (OS/2)
   Andy Lowry, Columbia U
   James Lummel, Caprica Telecomputing Resources (QNX)
   David MacKenzie, Environmental Defense Fund, U of Maryland
   John Mackin, University of Sidney, Australia
   Martin Maclaren, Bath U, UK
   Chris Maio, Columbia U CS Dept
   Montserrat Mane, HP, Grenoble, France
   Fulvio Marino, Olivetti, Ivrea, Italy
   Arthur Marsh, dircsa.org.au
   Peter Mauzey, AT&T
   Tye McQueen, Utah State U
   Ted Medin
   Hellmuth Michaelis, Hanseatischer Computerservice GmbH, Hamburg, Germany
   Leslie Mikesell, American Farm Bureau
   Martin Minow, DEC (VMS)
   Pawan Misra, Bellcore
   Ken Mizialko, IBM, Manassas, VA
   Ray Moody, Purdue U
   Bruce J Moore, Allen-Bradley Co, Highland Heights, OH (Atari ST)
   Steve Morley, Convex
   Peter Mossel, Columbia U
   Tony Movshon, NYU
   Lou Muccioli, Swanson Analysis Systems
   Dan Murphy
   Neal P. Murphy, Harsof Systems, Wonder Lake IL
   Gary Mussar
   John Nall, FSU
   Jack Nelson, U of Pittsburgh
   Jim Noble, Planning Research Corporation (Macintosh)
   Ian O'Brien, Bath U, UK
   John Owens
   Michael Pins, Iowa Computer Aided Engineering Network
   Andre' Pirard, University of Liege, Belgium
   Paul Placeway, Ohio State U
   Piet W. Plomp, ICCE, Groningen University, Netherlands
   Ken Poulton, HP Labs
   Manfred Prange, Oakland U
   Christopher Pratt, APV Baker, UK
   Frank Prindle, NADC
   Tony Querubin, U of Hawaii
   Anton Rang
   Scott Ribe
   Alan Robiette, Oxford University, UK
   Michel Robitaille, U of Montreal (Mac)
   Huw Rogers, Schweizerische Kreditanstalt, Zuerich
   Nigel Roles, Cambridge, England
   Kai Uwe Rommel, Technische Universitaet Muenchen (OS/2)
   Larry Rosenman (Amiga)
   Jay Rouman, U of Michigan
   Jack Rouse, SAS Institute (Data General and/or Apollo)
   Stew Rubenstein, Harvard U (VMS)
   John Santos, EG&H
   Bill Schilit, Columbia U
   Ulli Schlueter, RWTH Aachen, Germany (OS-9, etc)
   Michael Schmidt, U of Paderborn, Germany
   Eric Schnoebelen, Convex
   Benn Schreiber, DEC
   Dan Schullman, DEC (modems, DIAL command, etc)
   John Schultz, 3M
   Steven Schultz, Contel (PDP-11)
   APPP Scorer, Leeds Polytechnic, UK
   Gordon Scott, Micro Focus, Newbury UK
   Gisbert W. Selke, WIdO, Bonn, Germany
   David Singer, IBM Almaden Research Labs
   David Sizeland, U of London Medical School
   Fridrik Skulason, Iceland
   Rick Sladkey (Linux)
   Dave Slate
   Bradley Smith, UCLA
   Fred Smith, Merk
   Richard S Smith, Cal State
   Ryan Stanisfer, UNT
   Bertil Stenstroem, Stockholm University Computer Centre (QZ), Sweden
   James Sturdevant, CAP GEMENI AMERICA, Minneapolis
   Peter Svanberg, Royal Techn. HS, Sweden
   James R. Swenson, Accu-Weather, Inc.
   Andy Tanenbaum, Vrije U, Amsterdam, Netherlands
   Glen Thobe
   Markku Toijala, Helsinki U of Technology
   Teemu Torma, Helsinki U of Technology
   Linus Torvalds, Helsinki
   Rick Troxel, NIH
   Warren Tucker, Tridom Corp, Mountain Park, GA
   Dave Tweten, AMES-NAS
   G Uddeborg, Sweden
   Walter Underwood, Ford Aerospace
   Pieter Van Der Linden, Centre Mondial, Paris
   Ge van Geldorp, Netherlands
   Fred van Kempen, MINIX User Group, Voorhout, Netherlands
   Wayne Van Pelt, GE/CRD
   Mark Vasoll, Oklahoma State U (V7 UNIX)
   Konstantin Vinogradov, ICSTI, Moscow
   Paul Vixie, DEC
   Bernie Volz, Process Software
   Eduard Vopicka, Prague University of Economics, Czech Republic
   Dimitri Vulis, CUNY
   Roger Wallace, Raytheon
   Stephen Walton, Calif State U, Northridge (Amiga)
   Jamie Watson, Adasoft, Switzerland (RS/6000)
   Rick Watson, U of Texas (Macintosh)
   Robert Weiner, Programming Plus, New York City
   Lauren Weinstein, Vortex Technlogy
   David Wexelblat, AT&T
   Clark Wierda, Illuminati Online
   Joachim Wiesel, U of Karlsruhe
   Lon Willett, U of Utah
   Michael Williams, UCLA
   Nate Williams, U of Montana
   David Wilson
   Joellen Windsor, U of Arizona
   Patrick Wolfe, Kuck & Associates, Inc.
   Gregg Wonderly, Oklahoma State U (V7 UNIX)
   Farrell Woods, Concurrent (formerly Masscomp)
   Dave Woolley, CAP Communication Systems, London
   Jack Woolley, SCT Corp
   Frank Wortner
   Ken Yap, formerly of U of Rochester
   John Zeeff, Ann Arbor, MI */
#include "ckcasc.h"			/* ASCII character symbols */
#include "ckcdeb.h"			/* Debug & other symbols */
#include "ckcker.h"			/* Kermit symbols */
#include "ckcnet.h"			/* Network symbols */
#ifndef NOSPL
#include "ckuusr.h"
#endif /* NOSPL */
#ifdef OS2ONLY
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h>
#endif /* OS2ONLY */
#ifdef NT
#include "ckntap.h"
#endif /* NT */

#ifndef NOSERVER
/* Text message definitions.. each should be 256 chars long, or less. */
#ifdef pdp11
char *hlptxt = "PDP-11 C-Kermit Server - no help available.\n";
#else
#ifdef MAC
char *hlptxt = "\r\
Mac Kermit Server Commands:\r\
\r\
    BYE\r\
    FINISH\r\
    GET filespec\r\
    REMOTE CD directory\r\
    REMOTE HELP\r\
    SEND filespec\r\
\r\0";
#else
#ifdef AMIGA
char *hlptxt = "Amiga C-Kermit server responds to:\n\
\n\
GET filespec, SEND filespec, FINISH, BYE, REMOTE HELP\n\
\n\0";
#else
#ifdef OS2
char *hlptxt = "Kermit server responds to:\r\n\
\r\n\
GET files      REMOTE CD [dir]         REMOTE DIRECTORY [files]\r\n\
REGET files    REMOTE COPY [f1] [f2]   REMOTE HOST command\r\n\
RETRIEVE files REMOTE PRINT files      REMOTE TYPE files\r\n\
SEND files     REMOTE RENAME [f1] [f2] REMOTE SET parameter value\r\n\
FINISH         REMOTE SPACE [dir]      REMOTE QUERY type variable\r\n\
BYE            REMOTE DELETE files     REMOTE ASSIGN variable value\r\n\
               REMOTE LOGIN user pswd  REMOTE HELP\r\n\
\r\n\0";
#else
#ifdef MINIX
char *hlptxt = "C-Kermit Server commands:\n\
GET REGET SEND BYE FINISH REMOTE: CD DEL DIR HELP HOST SET SPACE TYPE WHO\n\0";
#else
#ifdef VMS
char *hlptxt = "VMS C-Kermit server responds to:\r\n\
\r\n\
GET files      REMOTE CD [dir]     REMOTE DIRECTORY [files]\r\n\
REGET files    REMOTE SPACE [dir]  REMOTE HOST command\r\n\
RETRIEVE files REMOTE DELETE files REMOTE WHO [user]\r\n\
SEND files     REMOTE PRINT files  REMOTE SET parameter value\r\n\
MAIL files     REMOTE HELP         REMOTE QUERY type variable\r\n\
BYE            REMOTE LOGIN        REMOTE ASSIGN variable value\r\n\
FINISH         REMOTE TYPE files\r\n\
\0";
#else
#ifdef datageneral
char *hlptxt = "AOS/VS C-Kermit server responds to:\n\
\n\
GET files   REMOTE CD [dir]     REMOTE DIRECTORY [filespec]\n\
REGET files REMOTE SPACE [dir]  REMOTE HOST command\n\
SEND files  REMOTE TYPE file    REMOTE DELETE files\n\
BYE         REMOTE WHO          REMOTE SET\n\
FINISH      REMOTE LOGIN        REMOTE QUERY type variable\n\
RETREIVE files                  REMOTE ASSIGN variable value\n\
\0";
#else
#ifdef NOSPL
char *hlptxt = "C-Kermit Server REMOTE Commands:\n\
\n\
GET files      REMOTE CD [dir]     REMOTE DIRECTORY [files]\n\
SEND files     REMOTE SPACE [dir]  REMOTE HOST command\n\
MAIL files     REMOTE DELETE files REMOTE WHO [user]\n\
RETRIEVE files REMOTE PRINT files  REMOTE TYPE files\n\
FINISH         REMOTE HELP         REMOTE SET parameter value\n\
BYE\n\
\n\0";
#else
char *hlptxt = "C-Kermit Server REMOTE Commands:\n\
\n\
GET files      REMOTE CD [dir]     REMOTE HOST command\n\
REGET files    REMOTE SPACE [dir]  REMOTE DIRECTORY [files]\n\
RETRIEVE files REMOTE DELETE files REMOTE LOGIN user password\n\
SEND files     REMOTE PRINT files  REMOTE SET parameter value\n\
RESEND files   REMOTE TYPE files   REMOTE QUERY type variable\n\
MAIL file user REMOTE WHO [user]   REMOTE ASSIGN variable value\n\
FINISH, BYE    REMOTE HELP\n";
#endif /* NOSPL */
#endif /* datageneral */
#endif /* VMS */
#endif /* MINIX */
#endif /* OS2 */
#endif /* AMIGA */
#endif /* MAC */
#endif /* pdp11 */

#ifdef MINIX
char *srvtxt = "\r\n\
Entering server mode.\r\n\0";
#else
#ifdef OLDMSG
/*
  It seems there was a large installation that was using C-Kermit 5A(165)
  or thereabouts, which had deployed thousands of MS-DOS Kermit scripts in
  scattered locations that looked for strings in the old server message,
  which changed in 5A(183), August 1992.
*/
char *srvtxt = "\r\n\
C-Kermit server starting.  Return to your local machine by typing\r\n\
its escape sequence for closing the connection, and issue further\r\n\
commands from there.  To shut down the C-Kermit server, issue the\r\n\
FINISH or BYE command and then reconnect.\n\
\r\n\0";
#else
#ifdef OSK
char *srvtxt = "\r\012\
Entering server mode.  If your local Kermit software is menu driven, use\r\012\
the menus to send commands to the server.  Otherwise, enter the escape\r\012\
sequence to return to your local Kermit prompt and issue commands from\r\012\
there. Use SEND and GET for file transfer. Use REMOTE HELP for a list of\r\012\
other available services.  Use BYE or FINISH to end server mode.\r\012\0";
#else /* UNIX, VMS, AOS/VS, and all others */
char *srvtxt = "\r\n\
Entering server mode.  If your local Kermit software is menu driven, use\r\n\
the menus to send commands to the server.  Otherwise, enter the escape\r\n\
sequence to return to your local Kermit prompt and issue commands from\r\n\
there.  Use SEND and GET for file transfer.  Use REMOTE HELP for a list of\r\n\
other available services.  Use BYE or FINISH to end server mode.\r\n\0";
#endif /* OSK */
#endif /* OLDMSG */
#endif /* MINIX */
#else  /* server mode disabled */
char *hlptxt = "";
char *srvtxt = "";
#endif /* NOSERVER */

/* Declarations for Send-Init Parameters */

struct ck_p ptab[NPROTOS] = {		/* Initialize the Kermit part ... */
    "Kermit",
    DRPSIZ,				/* Receive packet size */
    DSPSIZ,				/* Send packet size */
    0,					/* Send-packet-size-set flag */
    DFWSIZ,				/* Window size */

#ifdef NEWDEFAULTS
    PX_CAU,				/* Control char unprefixing... */
#else
    PX_ALL,
#endif /* NEWDEFAULTS */

#ifdef VMS				/* Default filename collision action */
    XYFX_X,				/* REPLACE for VAX/VMS */
#else
    XYFX_B,				/* BACKUP for everybody else */
#endif /* VMS */

#ifdef OS2				/* Flag for file name conversion */
    XYFN_L,				/* Literal for OS2 */
#else
    XYFN_C,				/* Converted for others */
#endif /* OS2 */

    1,				/* Send pathnames OFF (1 = strip) */
    1,				/* Receive pathnames OFF  (ditto) */
    NULL,                       /* Host receive initiation string (binary) */
    NULL,                       /* Host receive initiation string (text)   */
    NULL,			/* External protocol send command (binary) */
    NULL,			/* External protocol send command (text)   */
    NULL,			/* External protocol receive command (bin) */
    NULL			/* External protocol receive command (txt) */
#ifdef CK_XYZ
,
    "XMODEM",  128,128,-1,-1,    -1,-1,-1,1,1,NULL,NULL,NULL,NULL,NULL,NULL,
    "YMODEM",   -1, -1,-1,-1,    -1,-1,-1,1,1,NULL,NULL,NULL,NULL,NULL,NULL,
    "YMODEM-g", -1, -1,-1,-1,    -1,-1,-1,1,1,NULL,NULL,NULL,NULL,NULL,NULL,
    "ZMODEM",   -1, -1,-1,-1,PX_WIL,-1,-1,1,1,NULL,NULL,NULL,NULL,NULL,NULL,
    "Other",    -1, -1,-1,-1,    -1,-1,-1,1,1,NULL,NULL,NULL,NULL,NULL,NULL
#endif /* CK_XYZ */
};

int spsiz = DSPSIZ,                     /* Current packet size to send */
    spmax = DSPSIZ,			/* Biggest packet size we can send */
    spsizr = DSPSIZ,			/* Send-packet size requested */
    spsizf = 0,                         /* Flag to override size negotiation */
    rpsiz = DRPSIZ,                     /* Biggest we want to receive */
    urpsiz = DRPSIZ,			/* User-requested receive pkt size */
    maxrps = MAXRP,			/* Maximum incoming long packet size */
    maxsps = MAXSP,			/* Maximum outbound l.p. size */
    maxtry = MAXTRY,			/* Maximum retries per packet */
    wslots = 1,				/* Window size currently in use */
    wslotr = DFWSIZ,			/* Window size from SET WINDOW */
    wslotn = 1,				/* Window size negotiated in S-pkt */
    timeouts = 0,			/* For statistics reporting */
    spackets = 0,			/*  ... */
    rpackets = 0,			/*  ... */
    retrans = 0,			/*  ... */
    crunched = 0,			/*  ... */
    wmax = 0,				/*  ... */
    wcur = 0,				/*  ... */
#ifdef OS2
    srvidl = 0,                         /* Server idle timeout */
#endif /* OS2 */
    srvdis = 1,				/* Server file xfer display */
    srvtim = DSRVTIM,			/* Server command wait timeout */
/*
  timint is the timeout interval I use when waiting for a packet.
  pkttim is the SET RECEIVE TIMEOUT value, sent to the other Kermit.
  rtimo is the SET SEND TIMEOUT value.  rtimo is the initial value of
  timint.  timint is changed by the value in the incoming negotiation
  packet unless a SET SEND TIMEOUT command was given.
*/
    timint = DMYTIM,                    /* Timeout interval I use */
    pkttim = URTIME,			/* Timeout I want you to use */
    rtimo = DMYTIM,			/* Normal packet wait timeout */
    timef = 0,                          /* Flag to override what you ask */
#ifdef CK_TIMERS
    rttflg = 1,				/* Use dynamic round-trip timers */
#else
    rttflg = 0,				/* Use fixed timer */
#endif /* CK_TIMERS */
    mintime = 1,			/* Minimum timeout */
    maxtime = 0,			/* Maximum timeout */

    npad = MYPADN,                      /* How much padding to send */
    mypadn = MYPADN,                    /* How much padding to ask for */
    bctr = DFBCT,			/* Block check type requested */
    bctu = 1,                           /* Block check type used */
    bctl = 1,				/* Block check length */
    ebq =  MYEBQ,                       /* 8th bit prefix */
    ebqflg = 0,                         /* 8th-bit quoting flag */
    rqf = -1,				/* Flag used in 8bq negotiation */
    rq = 0,				/* Received 8bq bid */
    sq = 'Y',				/* Sent 8bq bid */
    rpt = 0,                            /* Repeat count */
    rptq = MYRPTQ,                      /* Repeat prefix */
    rptflg = 0,                         /* Repeat processing flag */
    rptena = 1,				/* Repeat processing enabled */
    xfrcan = 1,				/* Transfer cancellation enabled */
    xfrchr = 3,				/* Transfer cancel char = Ctrl-C */
    xfrnum = 3;				/* Need three of them. */

int epktflg = 0;			/* E-PACKET command active */

int capas  = 9,				/* Position of Capabilities */
    lpcapb = 2,				/* Long Packet capability */
    lpcapr = 1,				/*  requested */
    lpcapu = 0,				/*  used */
    swcapb = 4,				/* Sliding Window capability */
    swcapr = 1,				/*  requested (allowed) */
    swcapu = 0,				/*  used */
    atcapb = 8,				/* Attribute capability */
    atcapr = 1,				/*  requested */
    atcapu = 0,				/*  used */
    rscapb = 16,			/* RESEND capability */
    rscapr = 1,				/*  requested by default */
    rscapu = 0,				/*  used */
    lscapb = 32,			/* Locking Shift capability */
    lscapr = 1,				/*  requested by default */
    lscapu = 0;				/*  used */

/* Flags for whether to use particular attributes */

int atenci = 1,				/* Encoding in */
    atenco = 1,				/* Encoding out */
    atdati = 1,				/* Date in */
    atdato = 1,				/* Date out */
    atdisi = 1,				/* Disposition in/out */
    atdiso = 1,
    atleni = 1,				/* Length in/out (both kinds) */
    atleno = 1,
    atblki = 1,				/* Blocksize in/out */
    atblko = 1,
    attypi = 1,				/* File type in/out */
    attypo = 1,
    atsidi = 1,				/* System ID in/out */
    atsido = 1,
    atsysi = 1,			       /* System-dependent parameters in/out */
    atsyso = 1;

#ifdef STRATUS
int atfrmi = 1,				/* Format in/out */
    atfrmo = 1,
    atcrei = 1,				/* Creator ID in/out */
    atcreo = 1,
    atacti = 1,				/* Account in/out */
    atacto = 1;
#endif /* STRATUS */

CHAR padch = MYPADC,                    /* Padding character to send */
    mypadc = MYPADC,                    /* Padding character to ask for */
    seol = MYEOL,                       /* End-Of-Line character to send */
    eol = MYEOL,                        /* End-Of-Line character to look for */
    ctlq = CTLQ,                        /* Control prefix in incoming data */
    myctlq = CTLQ,                      /* Outbound control character prefix */
    myrptq = MYRPTQ;			/* Repeat prefix I want to use */

int rptmin = 3;				/* Repeat-count minimum */

char whoareu[16] = { NUL, NUL };	/* System ID of other Kermit */
int sysindex = -1;			/* and index to its system ID struct */
int myindex  = -1;
char * cksysid =			/* My system ID */
#ifdef UNIX
    "U1"
#else
#ifdef VMS
    "D7"
#else
#ifdef OSK
    "UD"
#else
#ifdef AMIGA
    "L3"
#else
#ifdef MAC
    "A3"
#else
#ifdef OS2
#ifdef NT
    "UN"
#else /* NT */
    "UO"
#endif /* NT */
#else /* OS2 */
#ifdef datageneral
    "F3"
#else
#ifdef GEMDOS
    "K2"
#else
#ifdef STRATUS
    "MV"
#else
    ""
#endif /* STRATUS */
#endif /* GEMDOS */
#endif /* datageneral */
#endif /* OS2 */
#endif /* MAC */
#endif /* AMIGA */
#endif /* OSK */
#endif /* VMS */
#endif /* UNIX */
    ;

char uidbuf[64] = { NUL, NUL };

struct zattr iattr;			/* Incoming file attributes */

/* File related variables, mainly for the benefit of (Open)VMS */

#ifdef NLCHAR				/* Text-file line terminator */
CHAR feol = NLCHAR;
#else
CHAR feol = 0;
#endif

int fblksiz = DBLKSIZ;		/* File blocksize */
int frecl = DLRECL;		/* File record length */
int frecfm = XYFF_S;		/* File record format (default = stream) */
int forg = XYFO_S;		/* File organization (sequential) */
int fcctrl = XYFP_N;		/* File carriage control (ctrl chars) */

#ifdef VMS
/* VMS labeled file default options - name only. */
int lf_opts = LBL_NAM;
#else
#ifdef OS2
/* OS/2 labeled file default options, all attributes but archived. */
unsigned long int lf_opts = LBL_EXT|LBL_HID|LBL_RO|LBL_SYS;
#else
int lf_opts = 0;
#endif /* OS2 */
#endif /* VMS */

/* Packet-related variables */

int pktnum = 0,                         /* Current packet number */
    sndtyp = 0,				/* Type of packet just sent */
    rcvtyp = 0,				/* Type of packet just received */
    rsn,				/* Received packet sequence number */
    rln,				/* Received packet length */
    size,                               /* Current size of output pkt data */
    osize,                              /* Previous output packet data size */
    maxsize,                            /* Max size for building data field */
    spktl = 0,				/* Length packet being sent */
    rpktl = 0,				/* Length of packet just received */
    pktpaus = 0,			/* Interpacket pause interval, msec */
    rprintf,				/* REMOTE PRINT flag */
    rmailf;				/* MAIL flag */

CHAR
#ifdef pdp11
    srvcmd[MAXRP+4],
#else
#ifdef DYNAMIC
    *srvcmd = (CHAR *)0,		/* Where to decode server command */
    *pktmsg = (CHAR *)0,		/* Packet error message */
#else
    srvcmd[MAXRP+4],
    pktmsg[81],
#endif /* DYNAMIC */
#endif /* pdp11 */
    padbuf[96],				/* Buffer for send-padding */
    *recpkt,
    *rdatap,				/* Pointer to received packet data */
    *data = (CHAR *)0,			/* Pointer to send-packet data */
    *srvptr,                            /* Pointer to srvcmd */
    mystch = SOH,                       /* Outbound packet-start character */
    stchr = SOH;                        /* Incoming packet-start character */

/* File-related variables */

#ifndef NOMSEND				/* Multiple SEND */
struct filelist * filehead = NULL;
struct filelist * filetail = NULL;
struct filelist * filenext = NULL;
int addlist = 0;
#endif /* NOMSEND */

char filnam[CKMAXPATH + 1];		/* Name of current file. */
char cmdfil[CKMAXPATH + 1];		/* Application file name. */
int cfilef = 0;				/* Application file flag. */
#ifndef NOSERVER
int ngetpath = 0;			/* GET search path */
char * getpath[MAXGETPATH];
char * x_user = NULL;			/* Server login information */
char * x_passwd = NULL;
char * x_acct = NULL;
int x_login = 0;			/* Login required */
int x_logged = 0;			/* User is logged in */
#endif /* NOSERVER */
int nfils = 0;				/* Number of files in file group */
long fsize;                             /* Size of current file */
int wildxpand = 0;			/* Who expands wildcards */
int clfils = 0;				/* Flag for command-line files */
int stayflg = 0;			/* Flag for "stay", i.e. "-S" */

/* Communication line variables */

#ifdef BIGBUFOK
char ttname[512];			/* Name of communication device */
#else
#ifdef MAC
char ttname[256];
#else
char ttname[80];
#endif /* MAC */
#endif /* BIGBUFOK */

#ifdef MAC
int connected = 0;			/* True if connected */
int startconnected;			/* initial state of connected */
#endif /* MAC */

long speed = -1L;			/* Line speed */

int parity = DEFPAR,			/* Parity specified, 0,'e','o',etc */
    autopar = 0,			/* Automatic parity change flag */
    sosi = 0,				/* Shift-In/Out flag */
    flow = FLO_XONX,			/* Flow control */
    autoflow = 1,			/* Automatic flow control */
    turn = 0,                           /* Line turnaround handshake flag */
    turnch = XON,                       /* Line turnaround character */
    duplex = 0,                         /* Duplex, full by default */
    escape = DFESC,			/* Escape character for connect */
    delay = DDELAY,                     /* Initial delay before sending */
    tnlm = 0,				/* Terminal newline mode */
    mdmtyp = 0;                         /* Modem type (initially none)  */

/* Networks for SET HOST */

#define MYHOSTL 100
    char myhost[MYHOSTL];		/* Local host name */
    int network = 0;			/* Network vs serial connection */

#ifdef NETCONN
#ifdef TCPSOCKET
    int nettype = NET_TCPB;		/* Assume TCP/IP (BSD sockets) */
#else
#ifdef SUNX25
    int nettype = NET_SX25;
#else
#ifdef STRATUSX25
    int nettype = NET_VX25;
#else
#ifdef DECNET
    int nettype = NET_DEC;
#else
#ifdef SUPERLAT
    int nettype = NET_SLAT;
#else
    int nettype = NET_NONE;
#endif /* SUPERLAT */
#endif /* DECNET */
#endif /* STRATUSX25 */
#endif /* SUNX25 */
#endif /* TCPSOCKET */
#else
    int nettype = NET_NONE;
#endif /* NETCONN */

#ifdef ANYX25
    int revcall = 0;            /* X.25 reverse call not selected */
    int closgr  = -1;		/* X.25 closed user group not selected */
    int cudata = 0;		/* X.25 call user data not specified */
    char udata[MAXCUDATA];	/* X.25 call user data */
    CHAR padparms[MAXPADPARMS+1]; /* X.3 parameters */
#endif /* ANYX25 */

/* Other items */

int isinterrupted = 0;			/* Used in exception handling */
extern int what;

#ifdef NT
extern int StartedFromDialer;
#ifdef NTSIG
extern int TlsIndex;
#endif /* NTSIG */
#ifdef NTASM 
unsigned long ESPToRestore ;		/* Ditto */
#endif /* NTASM */
#endif /* NT */

#ifdef OS2PM
int os2pm = 0;				/* OS/2 Presentation Manager flag */
#endif /* OS2PM */

/* Terminal screen size, if known, -1 means unknown. */
#ifdef OS2
#include "ckocon.h"
int tt_rows[VNUM] = {25,24,25,1};		/* Rows (height) */
int tt_cols[VNUM] = {80,80,80,80};		/* Columns (width) */
#else /* OS2 */
int tt_rows = -1;			/* Rows (height) */
int tt_cols = -1;			/* Columns (width) */
#endif /* OS2 */
int tt_escape = 1;			/* Escaping back is enabled */

#ifdef NETCONN
extern int ttyfd, tn_exit;
#endif /* NETCONN */
    int exitonclose = 0;		/* Exit on close */

    int tlevel = -1;			/* Take-file command level */
#ifdef NOLOCAL
    int remonly = 1;			/* Remote-mode-only advisory (-R) */
#else
    int remonly = 0;
#endif /* NOLOCAL */

#ifndef NOSPL
    extern int cmdlvl;			/* Command level */
    extern int maclvl;			/* Macro invocation level */
#endif /* NOSPL */

    int protocol  = PROTO_K;		/* File transfer protocol = Kermit */
#ifdef NEWDEFAULTS
    int prefixing = PX_CAU;
#else
    int prefixing = PX_ALL;
#endif /* NEWDEFAULTS */
    extern short ctlp[];		/* Control-prefix table */

    int carrier = CAR_AUT;		/* Pay attention to carrier signal */
    int cdtimo = 0;			/* Carrier wait timeout */
    int xitsta = GOOD_EXIT;		/* Program exit status */
#ifdef VMS				/* Default filename collision action */
    int fncact = XYFX_X;		/* REPLACE for VAX/VMS */
#else
    int fncact = XYFX_B;		/* BACKUP for everybody else */
#endif /* VMS */
    int fncsav = -1;			/* For saving & restoring the above */
    int bgset = -1;			/* BACKGROUND mode set explicitly */
#ifdef UNIX
    int suspend = DFSUSP;		/* Whether SUSPEND command, etc, */
#else					/* is to be allowed. */
    int suspend = 0;
#endif /* UNIX */

/* Statistics variables */

long filcnt,                    /* Number of files in transaction */
    filrej,			/* Number of files rejected in transaction */
    flci,                       /* Characters from line, current file */
    flco,                       /* Chars to line, current file  */
    tlci,                       /* Chars from line in transaction */
    tlco,                       /* Chars to line in transaction */
    ffc,                        /* Chars to/from current file */
    tfc,                        /* Chars to/from files in transaction */
    ccu,			/* Control chars unprefixed in transaction */
    ccp,			/* Control chars prefixed in transaction */
    rptn;			/* Repeated characters compressed */

int tsecs = 0;                  /* Seconds for transaction */
int fsecs = 0;			/* Per-file timer */

/* Flags */

int deblog = 0,                         /* Flag for debug logging */
    debses = 0,				/* Flag for DEBUG SESSION */
    pktlog = 0,                         /* Flag for packet logging */
    seslog = 0,                         /* Session logging */
    tralog = 0,                         /* Transaction logging */
    displa = 0,                         /* File transfer display on/off */
    stdouf = 0,                         /* Flag for output to stdout */
    stdinf = 0,				/* Flag for input from stdin */
    xflg   = 0,                         /* Flag for X instead of F packet */
    hcflg  = 0,                         /* Doing Host command */
    dest   = DEST_D,			/* Destination for packet data */

/* If you change this, also see struct ptab above... */

#ifdef OS2				/* Flag for file name conversion */
    fncnv  = XYFN_L,			/* Default is Literal in OS/2, */
    f_save = XYFN_L,			/* (saved copy of same) */
#else
    fncnv  = XYFN_C,			/* elsewhere Convert them */
    f_save = XYFN_C,			/* (ditto) */
#endif /* OS2 */
    fnspath = 1,			/* Send-file path 1 = strip */
    fnrpath = 1,			/* Receive-file path 1 = strip */

#ifdef NEWDEFAULTS
    binary = XYFT_B,			/* Default file transfer mode */
    b_save = XYFT_B,			/* Saved file mode */
#else
    binary = XYFT_T,			/* Default file transfer mode */
    b_save = XYFT_T,			/* Saved file mode */
#endif /* NEWDEFAULTS */

#ifdef OS2
    cursor_save = -1,			/* Cursor state */
#endif /* OS2 */

    xfermode = XMODE_A,			/* Transfer mode, manual or auto */
    sendmode = SM_SEND,			/* Which type of SEND operation */
    slostart  = 1,			/* Slow start (grow packet lengths) */
    cmask  = 0177,			/* CONNECT (terminal) byte mask */
    fmask  = 0377,			/* File byte mask */
    warn   = 0,                         /* Flag for file warning */
    quiet  = 0,                         /* Be quiet during file transfer */
    local  = 0,                         /* 1 = local mode, 0 = remote mode */
    server = 0,                         /* Flag for being a server */
    bye_active = 0,			/* Flag for BYE command active */
    cflg   = 0,				/* Connect before transaction */
    cnflg  = 0,                         /* Connect after transaction */
    cxseen = 0,                         /* Flag for cancelling a file */
    czseen = 0,                         /* Flag for cancelling file group */
    discard = 0,			/* Flag for file to be discarded */
    keep = 1,                           /* Keep incomplete files */
    unkcs = 1,				/* Keep file w/unknown character set */
    nakstate = 0,			/* In a state where we can send NAKs */
    dblchar = -1,			/* Character to double when sending */
    moving = 0;				/* MOVE = send, then delete */

long sendstart = 0L;			/* SEND start position */

/* Variables passed from command parser to protocol module */

#ifndef NOSPL
#ifndef NOICP
_PROTOTYP( int parser, (int) );         /* The parser itself */
#ifdef CK_APC
_PROTOTYP( VOID apconect, (void) );
#endif /* CK_APC */
#endif /* NOICP */
#endif /* NOSPL */
char *clcmds = NULL;			/* Pointer to command-line commands */

#ifdef CK_CURSES
#ifndef OS2
#ifndef COHERENT
_PROTOTYP( VOID fxdinit, (void) );
#endif /* COHERENT */
#endif /* OS2 */
#endif /* CK_CURSES */

CHAR sstate  = (CHAR) 0;                /* Starting state for automaton */
CHAR zstate  = (CHAR) 0;		/* For remembering sstate */
char *cmarg  = "";                      /* Pointer to command data */
char *cmarg2 = "";                      /* Pointer to 2nd command data */
char **cmlist;                          /* Pointer to file list in argv */

int autodl =				/* Autodownload */
#ifdef CK_AUTODL
#ifdef OS2
             1				/* Enabled by default only in */
#else					/* terminal-emulating versions, */
             0				/* disabled by default for others */
#endif /* OS2 */
#else
             0				/* (or if not implemented). */
#endif /* CK_AUTODL */
              ;

int remfile = 0, rempipe = 0, remappd = 0; /* REMOTE output redirection */
char * remdest = NULL;
char * printfile = NULL;		/* NULL if printer not redirected */
int printpipe = 0;			/* For SET PRINTER */

/*
  Server services:
   0 = disabled
   1 = enabled in local mode
   2 = enabled in remote mode
   3 = enabled in both local and remote modes
  only as initial (default) values.
*/
int en_cwd = 3;				/* CD/CWD */
int en_cpy = 3;				/* COPY   */
int en_del = 2;				/* DELETE */
int en_dir = 3;				/* DIRECTORY */
int en_fin = 3;				/* FINISH */
int en_get = 3;				/* GET */
int nopush = 0;				/* PUSH enabled */
#ifndef NOPUSH
int en_hos = 2;				/* HOST enabled */
#else
int en_hos = 0;				/* HOST disabled */
#endif /* NOPUSH */
int en_ren = 3;				/* RENAME */
int en_sen = 3;				/* SEND */
int en_set = 3;				/* SET */
int en_spa = 3;				/* SPACE */
int en_typ = 3;				/* TYPE */
int en_who = 3;				/* WHO */
#ifdef datageneral
/* Data General AOS/VS can't do this */
int en_bye = 0;				/* BYE */
#else
int en_bye = 2;				/* PCs in local mode... */
#endif /* datageneral */
int en_asg = 3;				/* ASSIGN */
int en_que = 3;				/* QUERY */
int en_ret = 2;				/* RETRIEVE */
int en_mai = 3;				/* MAIL */
int en_pri = 3;				/* PRINT */

/* Miscellaneous */

char **xargv;                           /* Global copies of argv */
int  xargc;                             /* and argc  */
int xargs;				/* an immutable copy of argc */
char *xarg0;				/* and of argv[0] */
char *pipedata;				/* Pointer to -P (pipe) data */

extern char *dftty;                     /* Default tty name from ck?tio.c */
extern int dfloc;                       /* Default location: remote/local */
extern int dfprty;                      /* Default parity */
extern int dfflow;                      /* Default flow control */

/*
  Buffered file input and output buffers.  See getpkt() in ckcfns.c
  and zoutdump() in the system-dependent file i/o module (usually ck?fio.c).
*/
#ifndef DYNAMIC
/* Now we allocate them dynamically, see getiobs() below. */
char zinbuffer[INBUFSIZE], zoutbuffer[OBUFSIZE];
#endif /* DYNAMIC */
char *zinptr, *zoutptr;
int zincnt, zoutcnt;

_PROTOTYP( int getiobs, (VOID) );

/*  M A I N  --  C-Kermit main program  */

#include <signal.h>

#ifndef NOCCTRAP
#include <setjmp.h>
#include "ckcsig.h"
ckjmpbuf cmjbuf;
#ifdef GEMDOS				/* Special for Atari ST */
cc_clean();				/* This can't be right? */
#endif /* GEMDOS */
#endif /* NOCCTRAP */

/*  C K I N D E X  --  C-Kermit's index function  */
/*
  We can't depend on C libraries to have one, so here is our own.
  Call with:
    s1 - String to look for.
    s2 - String to look in.
     t - Starting position in s2.
     r - 0 for left-to-right search, non-0 for right-to-left.
  icase  0 for case independence, non-0 if alphabetic case matters.
  Returns 0 if string not found, otherwise a 1-based result.
*/
int
ckindex(s1,s2,t,r,icase) char *s1, *s2; int t, r, icase; {
    int len1, len2, i, j, x;
    char * s;
    char * ss1 = NULL;
    char * ss2 = NULL;

    if (!s1 || !s2) return(0);
    len1 = (int)strlen(s1);		/* length of string to look for */
    len2 = (int)strlen(s = s2);		/* length of string to look in */

    if (len1 < 0) return(0);		/* paranoia */
    if (len2 < 0) return(0);
    j = len2 - len1;			/* length difference */
    if (j < 0 || t > j) {		/* search string is longer */
	return(0);
    } else {				/* Args are OK */
	s = s2 + t;			/* Point to beginning of target */
	if (r == 0) {			/* Index */
	    for (i = 0; i <= (j - t); i++) { /* Now compare */
		x = icase ? strncmp(s1,s++,len1) : xxstrcmp(s1,s++,len1);
		if (!x)
		  return(i+1+t);
	    }
	} else {			/* Reverse Index */
	    for (i = t; i > -1 && s >= s1; i--) { /* Compare */
		x = icase ? strncmp(s1,s--,len1) : xxstrcmp(s1,s--,len1);
		if (!x)
		  return(i+1);
	    }
	}
	return(0);
    }
}

/* Tell if a pathname is absolute (vs relative) */
/* This should be parceled out to each of the ck*fio.c modules... */
int
isabsolute(path) char * path; {
    int rc = 0;
    int x;
    if (!path)
      return(0);
    if (!*path)
      return(0);    
    x = (int) strlen(path);
    debug(F111,"isabsolute",path,x);
#ifdef VMS
    rc = 0;
    if (x = ckindex(":",path,1,0,0))
      if (x = ckindex("[",path,x,0,0))
	if (x = ckindex("]",path,x,0,0))
	  rc = 1;
#else
#ifdef UNIX
    if (*path == '/' ||
#ifdef DTILDE
	*path == '~' ||
#endif /* DTILDE */
	*path == '.'
	)
      rc = 1;
#else
#ifdef OS2
    if (*path == '/' || *path == '\\' || *path == '.')
      rc = 1;
    else if (isalpha(*path) && x > 1)
      if (*(path+1) == ':')
	rc = 1;
#else
#ifdef AMIGA
    if (*path == '/' ||
#ifdef DTILDE
	*path == '~' ||
#endif /* DTILDE */
	*path == '.'
	)
      rc = 1;
#else
#ifdef OSK
    if (*path == '/' ||
#ifdef DTILDE
	*path == '~' ||
#endif /* DTILDE */
	*path == '.'
	)
      rc = 1;
#else
#ifdef datageneral
    if (*path == ':')
      rc = 1;
#else
#ifdef MAC
    rc = 0;				/* Fill in later... */
#else
#ifdef STRATUS
    rc = 0;				/* Fill in later... */     
#else
#ifdef GEMDOS
    if (*path == '/' || *path == '\\' || *path == '.')
      rc = 1;
    else if (isalpha(*path) && x > 1)
      if (*(path+1) == ':')
	rc = 1;
#endif /* GEMDOS */
#endif /* STRATUS */
#endif /* MAC */
#endif /* datageneral */
#endif /* OSK */
#endif /* AMIGA */
#endif /* OS2 */
#endif /* UNIX */
#endif /* VMS */
    debug(F111,"isabsolute returns","",rc);
    return(rc);
}

/*  See if I have direct access to the keyboard  */

int
is_a_tty(n) int n; {
#ifdef KUI
   return 1;
#else /* KUI */
#ifdef NT
    if (isWin95())
      return(1);
    else
      return(_isatty(n));
#else
    return(isatty(n));
#endif /* NT */
#endif /* KUI */
}

/* Info associated with a system ID */

struct sysdata sysidlist[] = {		/* Add others as needed... */
    "0",  "anonymous",    0, NUL,  0, 0, 0,
    "A1", "Apple II",     0, NUL,  0, 0, 3, /* fix this */
    "A3", "Macintosh",    1, ':',  0, 2, 1,
    "D7", "VMS",          0, ']',  1, 0, 0,
    "DA", "RSTS/E",       0, ']',  1, 0, 3, /* (i think...) */
    "DB", "RT11",         0, NUL,  1, 0, 3, /* (maybe...) */
    "F3", "AOS/VS",       1, ':',  0, 0, 2, 
    "I1", "VM/CMS",       0, NUL,  0, 0, 0,
    "I2", "MVS/TSO",      0, NUL,  0, 0, 0,
    "I4", "MUSIC",        0, NUL,  0, 0, 0,
    "I7", "CICS",         0, NUL,  0, 0, 0,
    "I9", "MVS/ROSCOE",   0, NUL,  0, 0, 0,
    "K2", "Atari ST",     1, '\\', 1, 0, 3,
    "L3", "Amiga",        1, '/',  1, 0, 2,
    "MV", "Stratus VOS",  1, '>',  0, 1, 0,
    "N3", "Apollo Aegis", 1, '/',  0, 3, 2,
    "U1", "UNIX",         1, '/',  0, 3, 2,
    "U8", "MS-DOS",       1, '\\', 1, 0, 3,
    "UD", "OS-9",         1, '/',  0, 3, 2,
    "UN", "Windows-32",   1, '\\', 1, 2, 3,
    "UO", "OS/2",         1, '\\', 1, 2, 3
};
static int nxxsysids = (sizeof(sysidlist) / sizeof(struct sysdata));

/* Given a Kermit system ID code, return the associated name string */
/* and some properties of the filenames... */

char *
getsysid(s) char * s; {			/* Get system-type name */
    int i;
    if (!s) return("");
    for (i = 0; i < nxxsysids; i++)
      if (!strcmp(sysidlist[i].sid_code,s))
	return(sysidlist[i].sid_name);
    return(s);
}

int
getsysix(s) char *s; {			/* Get system-type index */
    int i;
    if (!s) return(-1);
    for (i = 0; i < nxxsysids; i++)
      if (!strcmp(sysidlist[i].sid_code,s))
	return(i);
    return(-1);
}

/* Initialize file transfer protocols */

VOID 
initproto(y, upbstr, uptstr, sndbstr, sndtstr, rcvbstr, rcvtstr)
    int y;
    char * upbstr, * uptstr, * sndbstr, * sndtstr, * rcvbstr, * rcvtstr;
/* initproto */ {

    char * p;
    int n;

    if (upbstr)				/* Convert null strings */
      if (!*upbstr)			/* to null pointers */
	upbstr = NULL;

    if (uptstr)				/* Convert null strings */
      if (!*uptstr)			/* to null pointers */
	uptstr = NULL;

    if (sndbstr)
      if (!*sndbstr)
	sndbstr = NULL;

    if (sndtstr)
      if (!*sndtstr)
	sndtstr = NULL;

    if (rcvbstr)
      if (!*rcvbstr)
	rcvbstr = NULL;

    if (rcvtstr)
      if (!*rcvtstr)
	rcvtstr = NULL;

    protocol = y;			/* Set protocol */

    if (ptab[protocol].rpktlen > -1) 
      urpsiz = ptab[protocol].rpktlen;
    if (ptab[protocol].spktflg > -1) 
      spsizf = ptab[protocol].spktflg;
    if (ptab[protocol].spktlen > -1) {
	spsiz = ptab[protocol].spktlen;
	if (spsizf) 
	  spsizr = spmax = spsiz;
    }
    if (ptab[protocol].winsize > -1) 
      wslotr = ptab[protocol].winsize;
    if (ptab[protocol].prefix > -1) 
      prefixing = ptab[protocol].prefix;
    if (ptab[protocol].fnca > -1) 
      fncact  = ptab[protocol].fnca;
    if (ptab[protocol].fncn > -1) 
      fncnv   = ptab[protocol].fncn;
    if (ptab[protocol].fnsp > -1) 
      fnspath = ptab[protocol].fnsp;
    if (ptab[protocol].fnrp > -1) 
      fnrpath = ptab[protocol].fnrp;
    
    makestr(&(ptab[protocol].h_b_init),upbstr);
    makestr(&(ptab[protocol].h_t_init),uptstr);
    makestr(&(ptab[protocol].p_b_scmd),sndbstr);
    makestr(&(ptab[protocol].p_t_scmd),sndtstr);
    makestr(&(ptab[protocol].p_b_rcmd),rcvbstr);
    makestr(&(ptab[protocol].p_t_rcmd),rcvtstr);
}

/*
   M A K E S T R  --  Creates a dynamically allocated string.

   Makes a new copy of string s and sets pointer p to its address.
   Handles degenerate cases, like when buffers overlap or are the same,
   one or both arguments are NULL, etc.
*/
VOID
makestr(p,s) char **p, *s; {
    int x;
    char *q = NULL;

    if (*p == s)			/* The two pointers are the same. */
      return;				/* Don't do anything. */

    if (!s) {				/* New definition is null? */
	if (*p)				/* Free old storage. */
	  free(*p);
	*p = NULL;			/* Return null pointer. */
	return;
    }
    if ((x = strlen(s)) >= 0) {		/* Get length, even of empty string. */
	q = malloc(x + 1);		/* Get and point to temp storage. */
	if (q)
	  strcpy(q,s);
    } else
      q = NULL;				/* Length of string is zero */

    if (*p)				/* Now free the original storage. */
      free(*p);
    *p = NULL;

    if (!q)
      return;

    if (x > 0) {			/* Get length. */
	if (*p = malloc(x + 1))
	  strcpy(*p,q);
    }
    free(q);				/* Free temporary buffer. */
}

/*  X X S T R C M P  --  Caseless string comparison  */
/*
  Call with pointers to the two strings, s1 and s2, and a length, n.
  Compares up to n characters of the two strings and returns:
    1 if s1 > t1
    0 if s1 = s2
   -1 if s1 < t1
*/
int
xxstrcmp(s1,s2,n) char *s1, *s2; int n; { /* Caseless string comparison. */
    char t1, t2;			

    if (n < 1) return(0);
    if (!s1) s1 = "";			/* Watch out for null pointers. */
    if (!s2) s2 = "";
    while (n--) {
	t1 = *s1++;			/* Get next character from each. */
	t2 = *s2++;
	if (!t1) return(t2 ? -1 : 0);
	if (!t2) return(t1 ? -1 : 0);
	if (isupper(t1)) t1 = tolower(t1);
	if (isupper(t2)) t2 = tolower(t2);
	if (t1 < t2) return(-1);	/* s1 < s2 */
	if (t1 > t2) return(1);		/* s1 > s2 */
    }
    return(0);				/* They're equal */
}

#ifndef NOCMDL
VOID
#ifdef CK_ANSIC
docmdline(void * threadinfo)
#else /* CK_ANSIC */
docmdline(threadinfo) VOID * threadinfo; 
#endif /* CK_ANSIC */
{
#ifdef NTSIG
    if (threadinfo) {			/* Thread local storage... */
       TlsSetValue(TlsIndex,threadinfo);
       debug( F100, "docmdline called with threadinfo block", "", 0 );
    }
   else debug( F100, "docmdline threadinfo is NULL","",0) ;
#endif /* NTSIG */
    proto();				/* Take any requested action, then */
    if (!quiet)				/* put cursor back at left margin, */
      conoll("");
#ifndef NOLOCAL
    if (cnflg) doconect(0);		/* connect if requested. */
#endif /* NOLOCAL */

#ifdef NTSIG
     ckThreadEnd(threadinfo);
#endif /* NTSIG */
   return;
}

VOID 
#ifdef CK_ANSIC
failcmdline(void * foo)
#else /* CK_ANSIC */
failcmdline(foo) VOID * foo; 
#endif /* CK_ANSIC */
{
#ifdef GEMDOS
    cc_clean();
#endif /* GEMDOS */
#ifndef NOLOCAL
    if (cnflg) doconect(0);		/* connect again if requested. */
#endif /* NOLOCAL */
}
#endif /* NOCMDL */

#ifndef NOICP
VOID  
#ifdef CK_ANSIC
dotakeini(void * threadinfo)		/* Execute init file. */
#else  /* CK_ANSIC */
dotakeini(threadinfo) VOID * threadinfo; /* Execute init file. */
#endif /* CK_ANSIC */
/* dotakeini */ {
#ifdef NTSIG
    if (threadinfo) {			/* Thread local storage... */
       TlsSetValue(TlsIndex,threadinfo);
       debug( F100, "dotakeini called with threadinfo block","", 0 ) ;
    } else
      debug( F100, "dotakeini - threadinfo is NULL", "", 0 ) ;
#endif /* NTSIG */
    cmdini();				/* Sets tlevel */
    doinit();
    debug(F101,"main executing init file","",tlevel);
    while (tlevel > -1) {
	sstate = (CHAR) parser(1);	/* Execute one command at a time. */
	if (sstate) proto();		/* Enter protocol if requested. */
#ifdef NTSIG
       ck_ih();
#endif /* NTSIG */
    }
    debug(F101,"main exits init file","",tlevel);

#ifdef NTSIG
     ckThreadEnd(threadinfo);
#endif /* NTSIG */
    return;
}

VOID 
#ifdef CK_ANSIC
failtakeini(void * threadinfo) 
#else /* CK_ANSIC */
failtakeini(threadinfo) VOID * threadinfo; 
#endif /* CK_ANSIC */
/* failtakeini */ {
#ifdef GEMDOS
    cc_clean();				/* Atari: Clean up after ^C-trap. */
#endif /* GEMDOS */
    conoll("Interrupt during initialization or command-line processing.");
    conoll("C-Kermit quitting...");
    doexit(BAD_EXIT,-1);		/* Exit with bad status. */
}

VOID 
#ifdef CK_ANSIC
doicp(void * threadinfo) 
#else /* CK_ANSIC */
doicp(threadinfo) VOID * threadinfo; 
#endif /* CK_ANSIC */
/* doicp */ {
#ifdef NTSIG
    if (threadinfo) {			/* Thread local storage... */
       if (!TlsSetValue(TlsIndex,threadinfo))
          debug(F101,"doicp TlsSetValue failed","",GetLastError() ) ;
       debug( F101, "doicp a threadinfo block - TlsIndex", "", TlsIndex ) ;
    } else {
        debug( F100, "doicp received a null threadinfo", "", 0 ) ;
    }
#endif /* NTSIG */
#ifdef MAC
    while (1) {
	extern char *lfiles;		/* Fake pointer cast */

	if (connected) {
	    debug(F100, "main: calling macparser", "", 0);
	    sstate = newparser(1, 1, 0L);

	    /* ignore null command state */
	    if (sstate == 'n')
	      sstate = '\0';

	    if (sstate)
	      proto();
#ifdef NTSIG
       ck_ih();
#endif /* NTSIG */
	} else {
	    /*
	     * process take files the finder gave us.
	     */
	    if ((tlevel == -1) && lfiles)
	      startlfile();

	    debug(F100, "main: calling parser", "", 0);
	    sstate = (CHAR) parser(0);
	    if (sstate == 'c')		/* if MAC connect */
	      sstate = 0;
	    if (sstate)
	      proto();
#ifdef NTSIG
       ck_ih();
#endif /* NTSIG */
	}
    }
#else /* Not MAC */

#ifndef NOSPL
/*
  If interactive commands were given on the command line (using the
  -C "command, command, ..." option), assign them to a macro called
  "cl_commands", then execute the macro and leave it defined for
  subsequent re-execution if desired.
*/
    if (clcmds) {			/* Check for -C commands */
	int x;
	x = addmac("cl_commands",clcmds); /* Put macro in table */
	if (x > -1) {			/* If successful, */
	    dodo(x,NULL,CF_CMDL);	/* set up for macro execution */
	    while (maclvl > -1) {	/* Loop getting macro commands. */
		sstate = (CHAR) parser(1);
		if (sstate) proto();	/* Enter protocol if requested. */
#ifdef NTSIG
		ck_ih();
#endif /* NTSIG */
	    }
	}
	herald();
    }
#endif /* NOSPL */
/*
  Running from an application file, or a command filename was
  specified on the command line.
*/
    debug(F101,"main cfilef","",cfilef);
    if (*cmdfil) {
	cfilef = 1;			/* Remember we did this, */
	dotake(cmdfil);			/* Command file spec'd on cmd line */
    }
    while(1) {				/* Loop getting commands. */
	sstate = (CHAR) parser(0);
        if (sstate) proto();            /* Enter protocol if requested. */
#ifdef NTSIG
       ck_ih();
#endif /* NTSIG */
    }
#ifdef NTSIG
     ckThreadEnd(threadinfo);
#endif /* NTSIG */
   /* return ; */ /* If this routine is void there should be no return */
#endif /* MAC */
}

VOID 
#ifdef CK_ANSIC
failicp(void * threadinfo)
#else /* CK_ANSIC */
failicp(threadinfo) VOID * threadinfo; 
#endif /* CK_ANSIC */
{
#ifdef GEMDOS
    cc_clean();
#endif /* GEMDOS */
    fixcmd();				/* Pop command stacks, etc. */
    clcmds = NULL;
    debug(F100,"ckcmai got interrupt","",0);
}
#endif /* NOICP */

#ifndef NOICP
VOID  
#ifdef CK_ANSIC
docmdfile(void * threadinfo)		/* Execute application file */
#else /* CK_ANSIC */
docmdfile(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
{
#ifdef NTSIG
    if (threadinfo) {			/* Thread local storage... */
	TlsSetValue(TlsIndex,threadinfo);
	debug( F100, "docmdfile called with threadinfo block","", 0 ) ;
    } else debug( F100, "docmdfile - threadinfo is NULL", "", 0 ) ;
#endif /* NTSIG */
    debug(F110,"main cmdfil",cmdfil,0);
    dotake(cmdfil);			/* execute it */
    while (tlevel > -1) {		/* until it runs out. */
	sstate = parser(1);		/* Loop getting commands. */
	if (sstate) proto();		/* Enter protocol if requested. */
#ifdef NTSIG
	ck_ih();
#endif /* NTSIG */
    }
    cfilef = 1;				/* Remember we did this */
#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    return;
}

VOID 
#ifdef CK_ANSIC
failcmdfile(void * threadinfo) 
#else /* CK_ANSIC */
failcmdfile(threadinfo) VOID * threadinfo; 
#endif /* CK_ANSIC */
{
#ifdef GEMDOS
    cc_clean();				/* Atari: Clean up after ^C-trap. */
#endif /* GEMDOS */
    conoll("Interrupt during initialization or command-line processing.");
    conoll("C-Kermit quitting...");
    doexit(BAD_EXIT,-1);		/* Exit with bad status. */
}
#endif /* NOICP */

VOID
setprefix(z) int z; {			/* Initial control-char prefixing */
#ifdef CK_SPEED
    int i, val;

    prefixing = z;
    ptab[protocol].prefix = prefixing;
    
    switch(z) {
      case PX_ALL:			/* All or None */
      case PX_NON:
	val = (z == PX_ALL) ? 1 : 0;
	for (i = 1; i < 32; i++) ctlp[i] = val;
	for (i = 127; i < 160; i++) ctlp[i] = val;
	ctlp[255] = val;
	break;

      case PX_CAU:			/* Cautious or Minimal */
      case PX_WIL:
	for (i = 1; i < 32; i++) ctlp[i] = 0;
	for (i = 127; i < 160; i++) ctlp[i] = 0;
	ctlp[mystch] = ctlp[mystch+128] = 1; /* Kermit start of packet */
	if (seol != 13) ctlp[seol] = ctlp[seol+128] = 1; /* Kermit end */
	ctlp[13] = ctlp[141] = 1;	/* In case of TELNET */
	ctlp[(unsigned)255] = 1;	/* Ditto */
	ctlp[17]  = ctlp[19]  = 1;
	ctlp[145] = ctlp[147] = 1;

	if (prefixing == PX_CAU) {	/* Cautious - add some more */
	    ctlp[3]   = ctlp[13]  = ctlp[16]  = 1;
	    ctlp[28]  = ctlp[29]  = ctlp[30]  = 1;
	    ctlp[131] = ctlp[141] = ctlp[144] = 1;
	    ctlp[(unsigned)255] = ctlp[156] = ctlp[157] = ctlp[158] = 1;
	}
	break;
    }
#endif /* CK_SPEED */
}

#ifdef aegis
/* On the Apollo, intercept main to insert a cleanup handler */
int
ckcmai(argc,argv) int argc; char **argv;
#else
#ifdef MAC				/* Macintosh */
int
main (void)
#else
#ifdef VMSGCC				/* (Open)VMS with GCC compiler */
int
main(argc,argv) int argc; char **argv;
#else
#ifdef __DECC				/* DEC Alpha with DEC C compiler */
#ifdef __ALPHA
int
main(argc,argv) int argc; char **argv;
#else					/* DEC C compiler, not Alpha */
VOID
main(argc,argv) int argc; char **argv;
#endif	/* __ALPHA */
#else
#ifdef STRATUS				/* Stratus VOS */
/* ANSI main returns int, and VOS compiler complains if not so. */
int
main(argc,argv) int argc; char **argv;
#else					/* All others */
#ifdef NT
#ifdef CK_WIN
void
Main( int argc, char ** argv ) 
#else
VOID
main(argc,argv) int argc; char **argv;
#endif /* CK_TAPI */
#else
VOID
main(argc,argv) int argc; char **argv;
#endif /* NT */
#endif /* STRATUS */
#endif /* __DECC */
#endif /* VMSGCC */
#endif /* MAC */
#endif /* aegis */
/* main */ {
#ifdef datageneral
    short *pfha = 016000000036;		/* Get around LANG_RT problem -- */
    *pfha = (short) 0;			/* No user protection fault handler */
#endif /* datageneral */
/* Do some initialization */

#ifndef MAC
    xargc = xargs = argc;		/* Make global copies of argc */
    xargv = argv;                       /* ...and argv. */
    xarg0 = argv[0];
#ifndef NOICP
#ifdef NT
    setOSVer();
#endif /* NT */
    prescan(0);				/* Check for debugging */
#endif /* NOICP */
#endif /* MAC */

    if (sysinit() < 0)			/* System-dependent initialization. */
      fatal("Can't initialize!");
#ifdef CK_CURSES
#ifndef OS2
#ifndef COHERENT
    fxdinit();				/* Init fullscreen package */
#endif /* COHERENT */
#endif /* OS2 */
#endif /* CK_CURSES */

#ifdef TCPSOCKET
#ifdef CK_SOCKS
    SOCKSinit(argv[0]);			/* Internet relay package... */
#endif /* CK_SOCKS */
#endif /* TCPSOCKET */

#ifdef CK_XYZ				/*  Initialize protocols...  */

#ifdef XYZ_INTERNAL /* XYZMODEM are internal ... */

    initproto(PROTO_X, "rx %s","rx %s",          NULL, NULL, NULL, NULL);
    initproto(PROTO_Y, "rb","rb",                NULL, NULL, NULL, NULL);
    initproto(PROTO_G, "rb","rb",                NULL, NULL, NULL, NULL);
    initproto(PROTO_Z, "rz","rz",                NULL, NULL, NULL, NULL);
    initproto(PROTO_K, "kermit -ir","kermit -r", NULL, NULL, NULL, NULL); 
    /* Kermit Must be last */

#else /* XYZMODEM are external protocols ... */

    /*                  s1      s2       s3       s4          s5       s6   */
    initproto(PROTO_X, "rx %s","rx %s", "sx %s", "sx -a %s", "rx %s", "rx %s");
    initproto(PROTO_Y, "rb",   "rb",    "sb %s", "sb -a %s", "rb",    "rb"   );
    initproto(PROTO_G, "rb",   "rb",    "sb %s", "sb -a %s", "rb",    "rb"   );
    initproto(PROTO_Z, "rz",   "rz",    "sz %s", "sz -a %s", "rz",    "rz"   );
    initproto(PROTO_K, "kermit -ir", "kermit -r", NULL, NULL, NULL, NULL);
    /* Kermit must be last */

#endif /* XYZ_INTERNAL */

#else  /* No XYZMODEM support */

    initproto(PROTO_K, "kermit -ir","kermit -r", NULL, NULL, NULL, NULL);

#endif /* CK_XYZ */

    connoi();				/* Console interrupts off */
    sstate = 0;                         /* No default start state. */
#ifdef DYNAMIC
    if (getiobs() < 0)
      fatal("Can't allocate i/o buffers!");
#endif /* DYNAMIC */
    ckhost(myhost,MYHOSTL);		/* Name of local host */
    strcpy(ttname,dftty);               /* Set up default tty name. */
    local = dfloc;                      /* And whether it's local or remote. */
    parity = dfprty;                    /* Set initial parity, */
    flow = dfflow;                      /* and flow control. */
    myindex = getsysix(cksysid);
    if (local) if (ttopen(ttname,&local,0,0) < 0) { /* If default tty line */
#ifndef OS2
	conol("Can't open device: ");
	conoll(ttname);
#endif /* OS2 */
	local = 0;
	strcpy(ttname,CTTNAM);
    }
    speed = ttgspd();			/* Get transmission speed. */

#ifdef ANYX25
    initpad();                          /* Initialize X.25 PAD */
#endif /* ANYX25 */

    if (inibufs(SBSIZ,RBSIZ) < 0)	/* Allocate packet buffers */
      fatal("Can't allocate packet buffers!");
#ifndef NOCKSPEED
    setprefix(prefixing);		/* Set up control char prefixing */
#endif /* NOCKSPEED */

#ifndef NOICP
#ifdef MAC
    cmdini();
#else /* Not MAC */

/* Attempt to take ini file before doing command line */

    *cmdfil = '\0';			/* Assume no command file. */
    prescan(1);				/* But first check for -y option */
    debug(F101,"main argc after prescan()","",argc);

/* Now process any relevant environment variables */

#ifndef NODIAL
    getdialenv();			/* Dialing */
#ifdef NETCONN
    ndinit();				/* Initialize network directory info */
    getnetenv();			/* Network directories */
#endif /* NETCONN */
#endif /* NODIAL */

#ifdef NOCCTRAP
    dotakeini(0);
#else /* NOCCTRAP */
    setint();
    cc_execute( ckjaddr(cmjbuf), dotakeini, failtakeini );
#endif /* NOCCTRAP */
    debug(F101,"main 2 cfilef","",cfilef);
    if (*cmdfil) {			/* If we got one (see prescan())... */
#ifdef NOCCTRAP
	docmdfile(0);			/* execute it. */
#else /* NOCCTRAP */
	setint();
	cc_execute( ckjaddr(cmjbuf), docmdfile, failcmdfile );
#endif /* NOCCTRAP */
    }
    *cmdfil = '\0';			/* Done, nullify the file name */
#endif /* MAC */
#endif /* NOICP */

#ifndef NOCMDL
/* Look for a UNIX-style command line... */

    what = W_NOTHING;
    debug(F101,"main argc","",argc);
    if (argc > 1) {                     /* Command line arguments? */
        sstate = (CHAR) cmdlin();	/* Yes, parse. */
	zstate = sstate;		/* Remember sstate around protocol */

#ifndef NOLOCAL
	if (cflg)			/* Connect first if requested */
	  doconect(0);
#endif /* NOLOCAL */

        if (sstate) {
#ifndef NOLOCAL
	    if (displa) concb((char)escape); /* (for console "interrupts") */
#endif /* NOLOCAL */

#ifdef NOCCTRAP
	    docmdline(1);
#else /* NOCCTRAP */
	    setint();
	    cc_execute( ckjaddr(cmjbuf), docmdline, failcmdline );
#endif /* NOCCTRAP */
	}

#ifndef NOICP
/*
  If a command-line action argument was given and -S ("stay") was not given,
  exit now.
*/
	if ((cflg || cnflg || zstate) && !stayflg)
#endif /* NOICP */
	  doexit(GOOD_EXIT,xitsta);	/* Exit with good status */
#ifndef NOICP
#ifdef NETCONN
	if ((cflg || cnflg) && tn_exit && ttyfd == -1)
	  doexit(GOOD_EXIT,xitsta);	/* Exit with good status */
#endif /* NETCONN */
#endif /* NOICP */
    }
#endif /* NOCMDL */

#ifdef NOICP				/* No interactive command parser */
    else {
#ifndef NOCMDL
	/* Command-line-only version */
	fatal("no command-line options given, type 'kermit -h' for help");
#else					/* Neither one! */
        sstate = 'x';
        proto();                        /* So go into server mode */
        doexit(GOOD_EXIT,xitsta);       /* exit with good status */
#endif /* NOCMDL */
    }
#else /* not NOICP */
/*
  If no action requested on command line, or if -S ("stay") was included,
  enter the interactive command parser.
*/
    if (!clcmds)
      herald();				/* Display program herald. */

#ifdef NOCCTRAP
    debug(F100,"ckcmai setting interrupt trap","",0);
    setint();				/* Set up command interrupt traps */
    doicp(NULL);
#else /* NOCCTRAP */
    while (1) {
	debug(F100,"ckcmai setting interrupt trap","",0);
	setint();			/* Set up command interrupt traps */
	if (!cc_execute(ckjaddr(cmjbuf), doicp, failicp))
          break;
    }
#endif /* NOCCTRAP */
#endif /* NOICP */
#ifdef MAC
    return(1);
#endif /* MAC */
}

#ifdef DYNAMIC
/* Allocate file i/o buffers */

char *zinbuffer, *zoutbuffer;

int
getiobs() {
    zinbuffer = (char *)malloc(INBUFSIZE);
    if (!zinbuffer) return(-1);
    zoutbuffer = (char *)malloc(OBUFSIZE);
    if (!zoutbuffer) return(-1);
    debug(F100,"getiobs ok","",0);
    return(0);
}
#endif /* DYNAMIC */
