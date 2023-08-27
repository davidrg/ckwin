SET PLATFORM=NT
SET K95BUILD=K95
SET KERMITDIR=..
if not exist .\win95\NUL mkdir .\win95
if not exist .\kui\win95\NUL mkdir .\kui\win95
SET OUTDIR=.\win95
cd kui
nmake /e
cd ..
SET OUTDIR=.\kui\win95
move kui\win95\ck*.obj . > nul
move kui\win95\p_*.obj . > nul
nmake /nologo /e /f ckoker.mak kuid 
move *.obj kui\win95 > nul


