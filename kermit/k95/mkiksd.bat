SET PLATFORM=NT
SET K95BUILD=IKSD
if not exist iksd\NUL mkdir iksd
move iksd\*.obj . > nul
if exist ckcmai.obj del ckcmai.obj
if exist ckuus5.obj del ckuus5.obj
nmake /nologo /e /f ckoker.mak msvc-iksd
move *.obj iksd  > nul

