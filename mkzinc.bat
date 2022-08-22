REM
REM Builds OpenZinc using customised makefiles from kermit\zinc-build
REM 
REM TODO: Detect compiler and set zinc_build accordingly, exiting if no suitable makefiles exist
REM In ui_env.hpp, uncomment #define ZIL_WIND0W_CTL3D
REM	-> read up about ctl3dv2.dll - who, what, when, where, why (msdn)
REM 

set zinc_build=mvcpp600mt

set lib=%lib%;%root%\zinc\lib\%zinc_build%
set include=%include%;%root%\zinc\include
cd %root%\zinc

call copymak.bat %zinc_build%.mak %root%\kermit\zinc-build

cd source
nmake -f %zinc_build%.mak winnt
cd ..

cd design

cd service
nmake -f %zinc_build%.mak winnt
cd ..

cd direct
nmake -f %zinc_build%.mak winnt
cd ..

cd storage
nmake -f %zinc_build%.mak winnt
cd ..

cd i18n
nmake -f %zinc_build%.mak winnt
cd ..

nmake -f %zinc_build%.mak winnt
cd ..
cd ..
