$ v = f$verify(0)					! PCPRINT.COM
$!
$! Print VMS files on the printer that is attached to your VT102,
$! VT200, or VT300, or PC with MS-DOS Kermit or other VT terminal
$! emulator that supports the "transparent print" function.
$!
$! Author: Mark Buda
$!
$! Usage:
$!	pcprint :== @sys$login:pcprint.com
$!	pcprint file.ext [,...]
$!
$ print_in_progress = 0
$ type		:= type
$ write		:= write
$ say		:= write sys$output
$ sayerr	:= write sys$error
$ ff[0,7]	= 12
$ esc[0,7]	= 27
$ rp_on		= esc + "[5i"	! Remote printer "ON" sequence
$ rp_off	= esc + "[4i"	! Remote printer "OFF" sequence
$!
$ on error then goto exit
$ on control_y then goto ctly_exit
$!
$ file = p1
$check_file:
$ file = f$edit(file,"collapse")
$ if file .eqs. ""
$ then
$	read/prompt="_Filename: "/end_of_file=exit sys$command file
$	goto check_file
$ endif
$ f = f$search(file)
$ if f .eqs. ""
$ then
$	sayerr "%PCPRINT-F-FNF, File not found - ''file'"
$	goto exit
$ endif
$ say rp_on
$ print_in_progress = 1
$ type 'file'
$ print_in_progress = 0
$ say ff
$ goto exit
$!
$ctly_exit:
$ if print_in_progress then say ff
$ goto exit
$!
$exit:
$ say rp_off
$ v = f$verify(v)
$ exit
