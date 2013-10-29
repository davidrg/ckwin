@echo Making distribution
if not exist dist\NUL mkdir dist
move *.exe dist
move *.manifest dist
copy k95.ini dist
ren dist\cknker.exe k95.exe
ren dist\cknker.exe.manifest k95.exe.manifest
del dist\ckwart.exe