This code was exhumed in March/April 2020 by Warner Losh, who found it on
an ancient DECUS tape.  It is very first public relase of C-Kermit, 4.0,
announced on the Info-Kermit mailing 5 February 1985:

  ftp://ftp.kermit.columbia.edu/kermit/e/mail.85a

It had been lost for 35 years.

I tried to build on both NetBSD and Linux, which failed horribley in
both places because modern C compilers are super-strict but this code
was written before ANSI C appeared.  The build almost worked after I made
the following adjustments:

. Fixed some minor corruption in the source files (wrapped lines, NULs).
. Added -std=c89 to makefile.
. Changed ckermi.h to also include ansi_c.h, which includes modern .h files.
. Made sure all source modules included ckermi.h (a few of them didn't).
. Corrected some wart/ckwart file naming confusion.
. Changed setstate() in wart.c to xsetstate() to avoid conflict with stdlib.h.

But because terminal i/o has totally changed since 35 years ago, there's
no way to completely build it without rewriting the code in ckxbsd.c to
use termios functions calls rather than setting ioctl bits.  Alternatively,
the work could be done to compile out the code that makes serial port and
modem connections (the CONNECT and DIAL commands, along with related SET
commands), and then you'd have a compact remote-end-only version.

Warner has written up a detailed history of old C-Kermit versions here:

  https://bsdimp.blogspot.com/2020/04/finding-kermit-4x.html


- Frank da Cruz, 23 April 2020
