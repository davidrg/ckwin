@echo off
@echo ==== Build CKNKER.EXE and assocaited utilities ====
SET PLATFORM=NT
SET K95BUILD=K95
if not exist nt\NUL mkdir nt
move nt\*.obj . > nul
del ckcmai.obj ckuus5.obj
nmake /nologo /e /f ckoker.mak msvc
set BLD_ERR_LVL=%errorlevel%
move *.obj nt  > nul

exit /B %BLD_ERR_LVL%
