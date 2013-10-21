SET PLATFORM=NT
SET K95BUILD=IKSD
move iksd\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak msvc-iksd |& tee comp.out.nt | list /s
move *.obj iksd  > nul

