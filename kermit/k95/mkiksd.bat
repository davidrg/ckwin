SET PLATFORM=NT
SET K95BUILD=IKSD
if not exist iksd\NUL mkdir iksd
move iksd\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak msvc-iksd
move *.obj iksd  > nul

