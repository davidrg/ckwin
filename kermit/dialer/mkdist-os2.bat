@echo off
@echo === Make Distribution ===

if not exist k2dial.exe goto :error

@echo Create directories...
if not exist dist-os2\NUL mkdir dist-os2

move k2dial.exe dist-os2
copy dialer.dat dist-os2
copy p_servic.znc dist-os2
copy p_direct.znc dist-os2
copy %root%\zinc\COPYING.LESSOR.txt dist-os2\COPYING.dialer.txt
goto :end

:error
echo Error: k2dial.exe has not been built. Run mkos2 first, then run mkdist-os2

:end