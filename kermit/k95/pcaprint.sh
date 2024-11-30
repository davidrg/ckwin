#!/bin/sh
#
# pcaprint
#
# Prints named files on locally attached PC or terminal printer, using
# "auto print" commands.  If no files named on command line, prints from
# standard input.
#
# Can be used with  with a VT102, VT2xx, or VT3xx terminal, that has a 
# locally attached printer, or a PC running software such as Kermit 95 or
# MS-DOS Kermit that emulates such a terminal.
#
# Works by sending the escape sequence ESC [ ? 5 i, which activates the
# terminal's autoprint mechanism (meaning that the following characters are
# sent to the local printer after being placed on the screen) and then sends
# the escape sequence ESC [ ? 4 i, which turns off auto printing and puts
# the terminal back to normal.
#
# Note: On certain System-V based UNIX implementations, you might have to
# change the hard escape in the 'echo' commands below into the sequence
# caret (^) left bracket ([).
#
# Usage: pcprint file
#    or: pcprint file file file ...
#    or: pcprint < file
#    or: command | pcprint
#
# Author: C. Gianone, Columbia University, 1995
#
echo -n '[5i'
if [ $# -eq 0 ]; then
  cat
else
  cat $*
fi
echo -n '[4i'
# (end)
