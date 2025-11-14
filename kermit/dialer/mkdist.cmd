@echo off
@echo === Make Distribution ===

@echo Create directories...
if not exist dist-os2\ mkdir dist-os2

move k2dial.exe dist-os2
copy dialer.dat dist-os2
copy p_servic.znc dist-os2
copy p_direct.znc dist-os2
copy %root%\zinc\COPYING.LESSOR.txt dist-os2\COPYING.dialer.txt
