$! DEC/CMS REPLACEMENT HISTORY, Element CKVCDT.COM
$! *3    23-APR-1989 17:22:58 BUDA "Minor fixes"
$! *2    16-APR-1989 17:56:59 BUDA "General work"
$! *1    11-APR-1989 22:55:27 BUDA "Initial creation"
$! DEC/CMS REPLACEMENT HISTORY, Element CKVCDT.COM
$	save_verify = f$verify(0'ckvcdt_verify')
$	on control_y then goto error
$	on warning then goto error
$!
$	delete = "delete"
$	'p3' == 1			! By default we must compile
$	pid = f$getjpi("","pid")
$	tmp_file = "ckvcdt_''pid'.tmp"
$!
$	module = f$parse(p1,,,"name")
$	src_time = f$file_attributes(p1, "cdt")
$	set noon
$	ass/user nla0: sys$output
$	ass/user nla0: sys$error
$	libr/list='tmp_file'/only='module'/before="''src_time'"  'p2'
$	if .not. $status then goto module_nf
$	set on
$!
$	open/read/error=error tmp 'tmp_file'
$loop:
$	read/end_of_file=module_ok tmp line
$	line = f$edit(line,"collapse,upcase")
$	if line .eqs. module then goto normal_exit
$	goto loop
$!
$module_ok:
$	status = 1
$	'p3' == 0
$	goto exit
$!
$module_nf:
$	status = 1
$	goto exit
$!
$error:
$	status = $status
$	if status then status = "%x08000002"
$	goto exit
$!
$normal_exit:
$	status = $status
$	goto exit
$!
$exit:
$	if f$trnlnm("tmp") .nes. "" then close tmp/nolog
$	if f$search(tmp_file) .nes. "" then delete 'tmp_file';*
$	save_verify = f$verify(save_verify)
$	exit 'status' .or. %x10000000
$!
$!	001	MAB	 3-MAR-1989
$!		Original
$!
$!
$!	P1 =	File.ext to compare against
$!	P2 =	Object library name
$!	P3 =	Symbol name to return result
$!		0 (false) if module should not be compiled
$!		1 (true) if module should be compiled
$!
$!
