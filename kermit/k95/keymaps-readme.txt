Kermit 95 Key Maps:

KEYCODES.TXT
  A complete listing of the keycodes for Kermit 95. These are the
  codes that are used in the SET KEY command. MS-DOS kermit should use the
  same key codes.

DEFAULT.KSC
  This is a complete listing of Kermit 95s default key map, showing
  both global and default assignments for each terminal type. If this file is
  missing or you'd like to re-create it, it is generated with the following
  command:
    k95.exe -Y -# 127 -C "save keymap keymaps/default.ksc,exit" > NUL:
  This starts a minimal C-Kermit without running default scripts, then saves
  the keymap to a file.

The following may also be present if this build of Kermit 95 is
capable of running on Windows 9x. If they're not, or you'd like a copy
anyway, they're available from here:
    https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu/pub/kermit/k95/

CTRL2CAP.VXD and .TXT
  A driver for Windows 95 by Mark Russinovich that swaps the caps lock
  and ctrl keys. Also, optionally, swaps ` and Esc. Not supported on anything
  besides Windows 95.

CAPSLOCK.KSC
  A key mapping file to work around a bug with the caps lock key in Windows 95.
  Redundant since Kermit 95 v1.1.5 unless you SET WIN95 KEYBOARD TRANSPARENT.

The following files are no longer needed as the key mappings are now built-in.
However, they may still be interesting as a reference. If they're not present,
you can get them from one of the following locations (you'll want to rename any
.ksc.txt files to just .ksc):
    https://ftp.zx.net.nz/pub/archive/kermit.columbia.edu-2/pub/kermit/archives/k95keymaps.zip

VT220.KSC
  VT220/320 keymap as listed in the Kermit 95 manual

EMACS.KSC
  Key mappings for the EMACS editor. Now built-in and activated with:
    SET TERMINAL KEYBOARD-MODE EMACS
  The alt key becomes meta, Ctrl-Alt becomes Ctrl-Meta. Arrow and editing keys
  are mapped to the corresponding emacs functions.

WP50.KSC, WP51.KSC
  Key maps for WordPerfect 5.0 and 5.1 on UNIX or VMS. Now built-in and
  activated with:
    SET TERMINAL KEYBOARD-MODE WP

SNI.SKC
  Key map for Siemens Nixdorf 97801 emulation. Now built-in.

(End of KEYMAPS\README.TXT)