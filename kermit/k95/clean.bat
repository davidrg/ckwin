@echo Cleaning build directories...
@set src_base=%root%\kermit\k95
@if exist %src_base%\*.exe del %src_base%\*.exe
@if exist %src_base%\*.map del %src_base%\*.map
@if exist %src_base%\*.res del %src_base%\*.res
@if exist %src_base%\*.pch del %src_base%\*.pch
@if exist %src_base%\*.ilk del %src_base%\*.ilk
@if exist %src_base%\*.pdb del %src_base%\*.pdb
@if exist %src_base%\*.obj del %src_base%\*.obj
@if exist %src_base%\cko32i*.dll del %src_base%\cko32i*.dll
@if exist %src_base%\k95ssh*.dll del %src_base%\k95ssh*.dll
@if exist %src_base%\nullssh.dll del %src_base%\nullssh.dll
@if exist %src_base%\k95ssh.dll del %src_base%\k95ssh.dll
@if exist %src_base%\nullssh.dll del %src_base%\nullssh.dll
@if exist %src_base%\k95crypt.dll del %src_base%\k95crypt.dll
@if exist %src_base%\nt\*.* del /Q %src_base%\nt\*.*
@if exist %src_base%\ntd\*.* del /Q %src_base%\ntd\*.*
@if exist %src_base%\kui\win95\*.* del /Q %src_base%\kui\win95\*.*
@if exist %src_base%\iksd\*.* del /Q %src_base%\iksd\*.*
@if exist %src_base%\os2\*.* del /Q %src_base%\os2\*.*

