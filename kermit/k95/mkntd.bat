SET PLATFORM=NT
SET K95BUILD=K95
move ntd\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak msvcd | tee comp.out.nt | list /s
move *.obj ntd > nul 

