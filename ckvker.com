$	SAVE_VERIFY = 'F$VERIFY(0) + F$VERIFY(0'CKVKER_VERIFY' .GT. 0)
$!	ON CONTROL_Y THEN GOTO ABNORMAL_EXIT
$	ON WARNING THEN GOTO ABNORMAL_EXIT
$!
$	delete = "delete"
$	tmp = f$envirement("procedure")
$	ckv_proc_loc =	f$parse(tmp,,,"DEVICE") + -
			f$parse(tmp,,,"DIRECTORY")
$	if p1 .nes. "BATCH" then goto interactive
$	f = f$search("''ckv_proc_loc'CKVKER.COM")
$	if f .eqs. "" then goto interactive
$	name = 	f$parse(f,,,"name")
$	log_file = F$PARSE(name + ".LOG")
$	com_file = ckv_proc_loc + name + ".COM"
$	submit/keep/noprint/log='log_file' -
'com_file'/para=("''f$environment("default")'","''p2'","''p3'","''p4'","''p5'","''p6'","''p7'","''p8'")
$	write sys$output "Submitting file ''com_file'"
$	exit
$interactive:
$	if f$mode() .nes. "BATCH" then goto no_batch
$! running in batch
$	if p1 .eqs. "" then exit
$	set def 'p1'
$no_batch:
$!
$	define/nolog cms$lib dua2:[ckermit.cms]
$	define/nolog c$include sys$disk:[],dua2:[ckermit.ref]
$	ckv_build_class == "V1"
$	vms_version = f$edit(f$getsyi("version"),"COMPRESS")
$	vms_version = "VMS_V" + f$extract(1,1,vms_version) + f$extract(3,1,vms_version)
$	objlib = "wermit"
$	alternate = ""
$	vckd_disk = "dua1:"
$	vckd_library = "''vckd_disk'[sys0.syslib]"
$	vckd_vaxc = "''vckd_disk'[sys0.sysexe]vaxc.exe"
$	vckd_linker = "''vckd_disk'[sys0.sysexe]linker.exe"
$!
$	if p3 .nes. "ALTERNATE" then goto no_alternate_build
$	alternate = "_v4"
$	vms_version = "VMS_V46"
$	define vaxc 'vckd_vaxc'
$	define linker 'vckd_linker'
$	define sys$library 'vckd_library'
$	objlib = objlib + alternate
$no_alternate_build:
$	if vms_version .eqs. "VMS_V41" then vms_version = "VMS_V40"
$	if vms_version .eqs. "VMS_V43" then vms_version = "VMS_V42"
$	if vms_version .eqs. "VMS_V45" then vms_version = "VMS_V44"
$	if vms_version .eqs. "VMS_V47" then vms_version = "VMS_V46"
$	if vms_version .eqs. "VMS_V51" then vms_version = "VMS_V50"
$!
$! Define command to be used in conditionally compiling C sources.
$!
$	IF "''CCMAKE'" .EQS. "" THEN CCMAKE = "@''ckv_proc_loc'CKVMAK.COM"
$!
$! Determine which version of the C compiler is being used,
$! and define version-specific stuff.
$!
$	CC_VER_OPTS = "/DEFINE=(""DEBUG"",""TLOG"",""VAXC030"",""''VMS_VERSION'"")"	!V3 options
$	C_LIBRARY = "SYS$LIBRARY:VAXCRTL.OLB"		!V3 lib
$	IF F$SEARCH("SYS$LIBRARY:VAXCPAR.OLB") .NES. "" THEN GOTO FOUND_LIB
$!
$	CC_VER_OPTS = "/DEFINE=(""DEBUG"",""TLOG"",""''VMS_VERSION'"")"	!V2 options
$	C_LIBRARY = "SYS$LIBRARY:VAXCRTL.OLB"		!V2 lib
$	IF F$SEARCH(C_LIBRARY) .NES. "" THEN GOTO FOUND_LIB
$!
$	CC_VER_OPTS = ""				!V1 options
$	C_LIBRARY = "SYS$LIBRARY:CRTLIB.OLB"		!V1 lib
$	IF F$SEARCH(C_LIBRARY) .NES. "" THEN GOTO FOUND_LIB
$!
$	WRITE SYS$OUTPUT "Unable to locate C Run-Time Library."
$	GOTO ABNORMAL_EXIT
$FOUND_LIB:
$	WRITE SYS$OUTPUT "Using ",C_LIBRARY," as the C Run-Time Library."
$!
$! Define CC options to be used, if not explicitly supplied.
$!
$	IF f$type(CC_OPTIONS) .EQS. "" -
	  THEN CC_OPTIONS = "/DEBUG=TRACE/OPTIMIZE/NOLIST" + CC_VER_OPTS
$	if f$type(cc_options) .nes. "" -
	  then cc_options = f$edit(cc_options,"upcase")
$	WRITE SYS$OUTPUT "Using CC options of ",CC_OPTIONS,"."
$!
$	CCMAKE CKCFN2.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKCFNS.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKCMAI.C "" 'CC_OPTIONS' 'objlib'
$!
$	ckv_wart_present = (f$search("ckcpro.w") .nes. "")
$	if .not. ckv_wart_present then -
		cms fet/nohist/nolog ckcpro.w/gen='ckv_build_class' ""
$	IF F$SEARCH("CKWART''alternate'.EXE") .EQS. ""	THEN GOTO DO_WART
$!	IF F$SEARCH("CKCPRO.C") .EQS. ""	THEN GOTO DO_WART
$!	IF F$SEARCH("CKCPRO.OBJ") .EQS. ""	THEN GOTO DO_WART
$	if f$search("ckwart''alternate'.olb") .eqs. "" then -
		libr/cre/obj ckwart'alternate'.olb
$!	library/ext=ckcpro/output=ckcpro.obj ckwart'alternate'
$!	SRC_TIME = F$FILE_ATTRIBUTES("CKCPRO.W", "CDT") ! get creation time
$!	OUT_TIME = F$FILE_ATTRIBUTES("CKCPRO.OBJ", "CDT") ! for both files.
$!	IF F$CVTIME(SRC_TIME) .GES. F$CVTIME(OUT_TIME) THEN GOTO DO_WART
$	@'ckv_proc_loc'ckvcdt ckcpro.w 'objlib' ckv$flag
$	if ckv$flag then goto do_wart
$!	delete ckcpro.obj;
$	WRITE SYS$OUTPUT "CKCPRO.OBJ", " is up to date."
$	GOTO DONE_WART
$DO_WART:
$	IF F$SEARCH("CKWART''alternate'.EXE") .NES. "" THEN GOTO GOT_WART
$	CCMAKE CKWART.C "" 'CC_OPTIONS' ckwart'alternate'
$	WRITE SYS$OUTPUT "Linking WART..."
$	LINK/EXECUTABLE=CKWART'alternate'/MAP=ckwart'alternate' -
	  ckwart'alternate'/lib/include=(CKWART),'C_LIBRARY'/LIBRARY
$GOT_WART:
$	WRITE SYS$OUTPUT "Executing WART..."
$	WART = "$SYS$DISK:[]CKWART''alternate'"
$	WART CKCPRO.W CKCPRO.C
$	CCMAKE CKCPRO.C "" 'CC_OPTIONS' 'objlib'
$	if .NOT. ckv_wart_present .and. f$search("ckcpro.w") .nes. "" then -
		delete ckcpro.w;*
$!	delete ckcpro.c;*
$DONE_WART:
$!
$	CCMAKE CKUCMD.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKUDIA.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKUSCR.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKUUS2.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKUUS3.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKUUSR.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKVCON.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKVFIO.C "" 'CC_OPTIONS' 'objlib'
$	CCMAKE CKVTIO.C "" 'CC_OPTIONS' 'objlib'
$	WRITE SYS$OUTPUT "Linking WERMIT..."
$	LINK'p2'/MAP=wermit'alternate'/sym/EXECUTABLE=WERMIT'alternate' -
	  'objlib'/library/include=(CKCFN2,CKCFNS,CKCMAI,CKCPRO,CKUCMD,CKUDIA, -
	  CKUSCR,CKUUS2,CKUUS3,CKUUSR,CKVCON,CKVFIO,CKVTIO), -
	  'C_LIBRARY'/LIBRARY
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
$	deas cms$lib
$	if f$trnlnm("sys$library","lnm$process") .nes. "" then deassign sys$library
$	if f$trnlnm("linker","lnm$process") .nes. "" then deassign linker
$	if f$trnlnm("vaxc","lnm$process") .nes. "" then deassign vaxc
$	EXIT ('STATUS' .OR. %X10000000) + F$VERIFY(SAVE_VERIFY) * 0
$!
$! CKVKER.COM	1.0 (003) 12-Feb-1989
$!
$! Build Kermit modules.
$!
$! Note: this command file implements a very limited "make" capability.
$! If you change ckwart.c, you must delete ckwart.exe
$! If you change any header files, you must delete all .obj files.
$!
$! IF YOU HAVE TROUBLE COMPILING OR RUNNING SOME OF THE GENERATED CODE,
$! TRY COMPILING WITHOUT OPTIMIZATION.
$!
$! input:
$!	CKVKER_VERIFY	If defined and positive, causes verification
$!			of this command file.
$!	CC_OPTIONS	Optional list of qualifiers for use when compiling
$!			(e.g., "/list/debug=all", "/define=xxx", etc.).
$!			Remember to enclose symbol names, etc. in quotes
$!			so that DCL won't convert them to uppercase.
$!
$!	@ckvker	[batch] [link_qual] [alternate]
$!
$!	@ckvker	"BATCH" "/DEBUG" "ALTERNATE"
$!		Will build wermit in batch, with the image linked
$!		with the debugger for V4
$!
$!	@ckvker	"BATCH"
$!		Will build wermit in batch without the debugger
$!		for current version.
$!
$! Modifications:
$!
$!	12-Feb-89	Add C V3 support.
$!				MAB
$!
$!	25-Jun-85	Allow external specification of CC options, and
$!			provide some defaults.
$!							-- Dan Schullman
$!
$!	24-Jun-85	Automatically determine C Run-Time library to use.
$!			Save and restore verification, and exit on errors.
$!			Display additional "progress" information.
$!			Avoid use of LNK$LIBRARY in case user had it defined.
$!			Use CCMAKE to compile CKCPRO and CKWART.
$!							-- Dan Schullman
