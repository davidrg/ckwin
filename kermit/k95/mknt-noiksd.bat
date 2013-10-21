SET PLATFORM=NT
SET K95BUILD=K95
move noiksd\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak msvc-noiksd |& tee comp.out.nt | list /s
move *.obj noiksd  > nul

