PREP.EXE /OM /FT /SF _TermScrnUpd cknker
if errorlevel == 1 goto done 
PROFILE.EXE cknker._xe
if errorlevel == 1 goto done 
PREP.EXE /M cknker
if errorlevel == 1 goto done 
PLIST.EXE /ST cknker >cknker.lst
:done

