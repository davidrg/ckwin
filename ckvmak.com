$	SAVE_VERIFY = 'F$VERIFY(0) + F$VERIFY(0'CKVMAK_VERIFY' .GT. 0)
$	ON CONTROL_Y THEN GOTO ABNORMAL_EXIT
$	ON WARNING THEN GOTO ABNORMAL_EXIT
$!
$! CKVMAK.COM	1.0 (001)+ 24-June-1985
$!
$! The SOURCE file is processed to create the OUTPUT file.  This command
$! checks creation dates and does not compile if both files are present
$! and the OUTPUT file is newer (younger) than the SOURCE file.
$!
$! Usage:
$!
$!	CKVMAK [SOURCE [OUTPUT [P3]]]
$!
$! input:
$!	P1	Source file specification.
$!	P2	Output file specification.  The name defaults to that
$!		of the source file, and the type defaults to ".OBJ".
$!	P3	Additional qualifiers for the CC command.
$!	CKVMAK_VERIFY	If defined and positive, causes verification
$!			of this command file.
$!
$! Modifications
$!
$!	24-Jun-85	Save and restore verification, and exit on any
$!			errors.  Use F$PARSE to default the name and
$!			type of the output file. -- Dan Schullman
$!
$	SOURCE = P1
$	IF SOURCE .EQS. "" THEN INQUIRE SOURCE "C Source File"
$	SOURCE = F$PARSE(SOURCE,".C")
$	OUTPUT = F$PARSE(P2,".OBJ",SOURCE)
$!
$! Continue at must_process if either file is missing or the source is younger
$! A missing SOURCE is, of course, an error -- but one that should be
$! caught by the "normal" command.
$!
$	IF F$SEARCH(SOURCE) .EQS. "" THEN GOTO MUST_PROCESS
$	IF F$SEARCH(OUTPUT) .EQS. "" THEN GOTO MUST_PROCESS
$	SRC_TIME = F$FILE_ATTRIBUTES(SOURCE, "CDT")	! get creation time
$	OUT_TIME = F$FILE_ATTRIBUTES(OUTPUT, "CDT")	!   for both files.
$	IF F$CVTIME(SRC_TIME) .GES. F$CVTIME(OUT_TIME) THEN GOTO MUST_PROCESS
$	WRITE SYS$OUTPUT OUTPUT," is up to date."
$	GOTO NORMAL_EXIT
$!
$! Come here to build OUTPUT from SOURCE
$!
$MUST_PROCESS:
$!
$! Insert commands to create OUTPUT from SOURCE here, for example:
$!
$	WRITE SYS$OUTPUT OUTPUT, " <= ", SOURCE
$	CC/NOLIST/OBJECT='OUTPUT''P3' 'SOURCE'
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
$	EXIT ('STATUS' .OR. %X10000000) + F$VERIFY(SAVE_VERIFY) * 0
