SET KERMITDIR=..
SET OUTDIR=.\win95
SET NODEBUG=1
cd kui
nmake /e
cd ..
UNSET NODEBUG
SET OUTDIR=.\kui\win95
SET PLATFORM=NT
SET K95BUILD=K95
move kui\win95\ck*.obj . > nul
move kui\win95\p_*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak k95g
move *.obj kui\win95 > nul


