@echo Making distribution
if not exist dist\NUL mkdir dist
move *.exe dist
copy k95.ini dist
ren dist\cknker.exe k95.exe