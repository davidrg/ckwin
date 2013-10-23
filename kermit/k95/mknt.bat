SET PLATFORM=NT
SET K95BUILD=K95
if not exist nt\NUL mkdir nt
move nt\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak msvc
move *.obj nt  > nul

