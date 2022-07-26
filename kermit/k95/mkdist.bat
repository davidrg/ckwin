@echo Making distribution
if not exist dist\NUL mkdir dist
if not exist dist\docs\NUL mkdir dist\docs
if not exist dist\docs\manual\NUL mkdir dist\docs\manual
copy ..\..\doc\manual\ckwin.htm dist\docs\manual\
move *.exe dist
move *.manifest dist
copy k95.ini dist
copy k95d.cfg dist
ren dist\cknker.exe k95.exe
ren dist\cknker.exe.manifest k95.exe.manifest
del dist\ckwart.exe

@echo Copy runtime libraries
if defined WATCOM copy %WATCOM%\binnt\mt7r*.dll dist
if defined WATCOM copy %WATCOM%\binnt\clbr*.dll dist
if defined WATCOM copy %WATCOM%\binnt\plbr*.dll dist