SET PLATFORM=NT
SET K95BUILD=K95
if not exist ntd\NUL mkdir ntd
move ntd\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak msvcd
move *.obj ntd > nul 

