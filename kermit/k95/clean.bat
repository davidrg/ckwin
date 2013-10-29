@echo Cleaning build directories...
@set src_base=%root%\kermit\k95
@del %src_base%\*.exe
@del %src_base%\*.map
@del %src_base%\*.res
@del %src_base%\*.pch
@del %src_base%\*.ilk
@del %src_base%\*.pdb
@del %src_base%\*.manifest
@del %src_base%\*.obj
@del /Q %src_base%\nt\*.*
@del /Q %src_base%\ntd\*.*
@del /Q %src_base%\kui\win95\*.*
