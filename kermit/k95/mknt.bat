SET PLATFORM=NT
SET K95BUILD=K95
move nt\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak msvc |& tee comp.out.nt | list /s
move *.obj nt  > nul

