$	SAVE_VERIFY = 'F$VERIFY(0) + F$VERIFY(0'CKVKER_VERIFY' .GT. 0)
$	ON CONTROL_Y THEN GOTO ABNORMAL_EXIT
$	ON WARNING THEN GOTO ABNORMAL_EXIT
$!
$! CKVKER.COM	1.0 (002) 25-June-1985
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
$! Modifications:
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
$!
$! Define command to be used in conditionally compiling C sources.
$!
$	IF "''CCMAKE'" .EQS. "" THEN CCMAKE = "@CKVMAK.COM"
$!
$! Determine which version of the C compiler is being used,
$! and define version-specific stuff.
$!
$	CC_VER_OPTS = "/DEFINE=(""DEBUG"",""TLOG"")"	!V2 options
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
$	IF "''CC_OPTIONS'" .EQS. "" -
	  THEN CC_OPTIONS = "/NOLIST" + CC_VER_OPTS
$	WRITE SYS$OUTPUT "Using CC options of ",CC_OPTIONS,"."
$!
$	CCMAKE CKCFN2.C "" 'CC_OPTIONS'
$	CCMAKE CKCFNS.C "" 'CC_OPTIONS'
$	CCMAKE CKCMAI.C "" 'CC_OPTIONS'
$!
$	IF F$SEARCH("CKWART.EXE") .EQS. ""	THEN GOTO DO_WART
$	IF F$SEARCH("CKCPRO.C") .EQS. ""	THEN GOTO DO_WART
$	IF F$SEARCH("CKCPRO.OBJ") .EQS. ""	THEN GOTO DO_WART
$	SRC_TIME = F$FILE_ATTRIBUTES("CKCPRO.W", "CDT") ! get creation time
$	OUT_TIME = F$FILE_ATTRIBUTES("CKCPRO.OBJ", "CDT") ! for both files.
$	IF F$CVTIME(SRC_TIME) .GES. F$CVTIME(OUT_TIME) THEN GOTO DO_WART
$	WRITE SYS$OUTPUT "CKCPRO.OBJ", " is up to date."
$	GOTO DONE_WART
$DO_WART:
$	IF F$SEARCH("CKWART.EXE") .NES. "" THEN GOTO GOT_WART
$!	CCMAKE CKWART.C "" 'CC_OPTIONS'/NOOPTIMIZATION
$	CCMAKE CKWART.C "" 'CC_OPTIONS'
$	WRITE SYS$OUTPUT "Linking WART..."
$	LINK/EXECUTABLE=CKWART/NOMAP -
	  CKWART.OBJ,'C_LIBRARY'/LIBRARY
$GOT_WART:
$	WRITE SYS$OUTPUT "Executing WART..."
$	WART = "$SYS$DISK:[]CKWART"
$	WART CKCPRO.W CKCPRO.C
$	CCMAKE CKCPRO.C "" 'CC_OPTIONS'
$DONE_WART:
$!
$	CCMAKE CKUCMD.C "" 'CC_OPTIONS'
$	CCMAKE CKUDIA.C "" 'CC_OPTIONS'
$	CCMAKE CKUSCR.C "" 'CC_OPTIONS'
$	CCMAKE CKUUS2.C "" 'CC_OPTIONS'
$	CCMAKE CKUUS3.C "" 'CC_OPTIONS'
$	CCMAKE CKUUSR.C "" 'CC_OPTIONS'
$	CCMAKE CKVCON.C "" 'CC_OPTIONS'
$	CCMAKE CKVFIO.C "" 'CC_OPTIONS'
$	CCMAKE CKVTIO.C "" 'CC_OPTIONS'
$	WRITE SYS$OUTPUT "Linking WERMIT..."
$	LINK/EXECUTABLE=WERMIT/NOMAP -
	  CKCFN2,CKCFNS,CKCMAI,CKCPRO,CKUCMD,CKUDIA,CKUSCR,-
	  CKUUS2,CKUUS3,CKUUSR,CKVCON,CKVFIO,CKVTIO, -
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
$	EXIT ('STATUS' .OR. %X10000000) + F$VERIFY(SAVE_VERIFY) * 0
