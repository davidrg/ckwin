@echo Making distribution
if not exist dist\NUL mkdir dist
if not exist dist\docs\NUL mkdir dist\docs
if not exist dist\docs\manual\NUL mkdir dist\docs\manual
copy ..\..\doc\manual\ckwin.htm dist\docs\manual\
move *.exe dist
move *.manifest dist
copy k95.ini dist
ren dist\cknker.exe k95.exe
ren dist\cknker.exe.manifest k95.exe.manifest
del dist\ckwart.exe