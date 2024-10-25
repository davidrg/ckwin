SET PLATFORM=NT
SET K95BUILD=K95
if not exist ntd\NUL mkdir ntd
move ntd\*.obj . > nul
if exist ckcmai.obj del ckcmai.obj
if exist ckuus5.obj del ckuus5.obj
nmake /nologo /e /f ckoker.mak msvcd
move *.obj ntd > nul 

