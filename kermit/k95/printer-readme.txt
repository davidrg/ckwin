KERMIT 95 PRINTER DIRECTORY

For an overview of printing services in Kermit 95, see the "Local Printing" and
"Host-Initiated Printing" sections of Chapter 7 in the Kermit 95 manual.

The Kermit 95 PRINTER directory contains the printer-related utilities
described below.

PCPRINT.SH, PCPRINT.MAN
  A UNIX shell script to print files on the PC's locally attached printer,
  using Kermit 95's Transparent Print feature.  Works with Kermit 95's
  Terminal screen.  PCPRINT.MAN is the "man page" (UNIX help text).

PCAPRINT.SH
  Like PCPRINT.SH, but for "auto printing" rather than transparent printing.
  Autoprinting takes place AFTER the material has been processed by the
  terminal emulator (formatted, character sets translated, etc), whereas
  transparent printing bypasses the terminal emulator altogether.

PCPRINT.COM
  A VMS DCL command file, equivalent to UNIX PCPRINT.SH.

TEXTPS.TXT
  Documentation for TEXTPS.EXE, the plaint-text-to-PostScript
  conversion program (used with Kermit 95's SET PRINTER command).
  The TEXTPS.EXE program is in the main Kermit 95 directory.

(End of PRINTER\READ.ME)
