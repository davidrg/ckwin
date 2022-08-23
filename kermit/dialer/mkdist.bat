@echo off
@echo === Make Distribution ===

@echo Create directories...
if not exist dist\NUL mkdir dist

move *.exe dist
copy *.manifest dist
copy dialer.dat dist
copy p_servic.znc dist
copy p_direct.znc dist
ren dist\nk95dial.exe dist\k95dial.exe