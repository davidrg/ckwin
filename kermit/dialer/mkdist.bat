@echo off
@echo === Make Distribution ===

@echo Create directories...
if not exist dist\NUL mkdir dist

move nk95dial.exe dist\k95dial.exe
copy *.manifest dist
copy dialer.dat dist
copy p_servic.znc dist
copy p_direct.znc dist
copy %root%\zinc\COPYING.LESSOR.txt dist\COPYING.dialer.txt
copy registry\k95regtl.txt dist
copy registry\k95regtl.exe dist
REM copy registry\k95regtl.exe.manifest dist
copy registry\registry.dat dist