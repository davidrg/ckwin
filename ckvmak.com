$! DEC/CMS REPLACEMENT HISTORY, Element CKVMAK.COM
$! *7    23-APR-1989 17:23:20 BUDA "Minor fixes"
$! *6    16-APR-1989 17:55:12 BUDA "General clean up"
$! *5    11-APR-1989 23:41:39 BUDA "Fix build class"
$! *4    11-APR-1989 23:37:05 BUDA "Fix CMS Fetching"
$! *3    11-APR-1989 23:32:22 BUDA "Automatically find called procedures"
$! *2    11-APR-1989 23:25:20 BUDA "Add CMS support"
$! *1    11-APR-1989 22:56:36 BUDA "Initial creation"
$! DEC/CMS REPLACEMENT HISTORY, Element CKVMAK.COM
$	SAVE_VERIFY = 'F$VERIFY(0) + F$VERIFY(0'CKVMAK_VERIFY' .GT. 0)
$	ON CONTROL_Y THEN GOTO ABNORMAL_EXIT
$	ON WARNING THEN GOTO ABNORMAL_EXIT
$!
$! CKVMAK.COM	1.0 (002)+ 23-Mar-1989
$!
$! The SOURCE file is processed to create the OUTPUT file.  This command
$! checks creation dates and does not compile if both files are present
$! and the OUTPUT file is newer (younger) than the SOURCE file.
$!
$! Usage:
$!
$!	CKVMAK [SOURCE] [OUTPUT] [P3] [P4]
$!
$! input:
$!	P1	Source file specification.
$!	P2	Output file specification.  The name defaults to that
$!		of the source file, and the type defaults to ".OBJ".
$!	P3	Additional qualifiers for the CC command.
$!	P4	Name of object library.
$!	CKVMAK_VERIFY	If defined and positive, causes verification
$!			of this command file.
$!
$! Modifications
$!
$!	24-Mar-1989	MAB
$!			Add object library code
$!
$!	24-Jun-85	Save and restore verification, and exit on any
$!			errors.  Use F$PARSE to default the name and
$!			type of the output file. -- Dan Schullman
$!
$	delete = "delete"
$	SOURCE = P1
$	IF SOURCE .EQS. "" THEN INQUIRE SOURCE "C Source File"
$	SOURCE = F$PARSE(SOURCE,".C")
$	cms_source = f$parse(source,,,"NAME") + f$parse(".C",source,,"TYPE")
$	file_present = (f$search(source) .nes. "")
$	OUTPUT = F$PARSE(P2,".OBJ",SOURCE)
$	name = f$parse(output,,,"NAME")
$	objlib = p4
$	if objlib .eqs. "" then inquire objlib "Object library name"
$	objlib = f$parse(objlib,".OLB")
$!
$! Continue at must_process if either file is missing or the source is younger
$! A missing SOURCE is, of course, an error -- but one that should be
$! caught by the "normal" command.
$!
$	if f$search("''objlib'") .eqs. "" then libr/create/object 'objlib'
$	if file_present then goto check_dates
$	cms fet/nohist/nolog 'cms_source'/gen='ckv_build_class' ""
$check_dates:
$	@'ckv_proc_loc'ckvcdt 'source' 'objlib' ckv$flag
$	if ckv$flag then goto must_process
$	WRITE SYS$OUTPUT OUTPUT," is up to date."
$	GOTO NORMAL_EXIT
$!
$! Come here to build OUTPUT from SOURCE
$!
$MUST_PROCESS:
$	set on
$!
$! Insert commands to create OUTPUT from SOURCE here, for example:
$!
$	WRITE SYS$OUTPUT OUTPUT, " <= ", SOURCE
$	CC/NOLIST/OBJECT='OUTPUT''P3' 'SOURCE'
$	libr/replace 'objlib' 'output'
$	GOTO NORMAL_EXIT
$!
$! Abnormal exit.
$!
$ABNORMAL_EXIT:
$	STATUS = $STATUS			!save failure status
$	IF STATUS THEN STATUS = "%X08000002"	!force error if neccessary
$	GOTO EXIT
$!
$! Normal exit.
$!
$NORMAL_EXIT:
$	STATUS = $STATUS			!save success status
$!
$! Exit.
$!
$EXIT:
$	if .NOT. file_present .and. f$search(source) .nes. "" then -
		delete 'source'
$	if f$search("''output'") .nes. "" then delete 'output'
$	EXIT ('STATUS' .OR. %X10000000) + F$VERIFY(SAVE_VERIFY) * 0

