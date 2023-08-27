@echo Full build...

set SUFFIX=
if [%1] NEQ [] set SUFFIX=%1

@call mknt.bat N %SUFFIX%
@call mkg.bat N %SUFFIX%

if not exist cknker.exe exit /b 1
if not exist iksd.exe exit /b 1
if not exist iksdsvc.exe exit /b 1
if not exist k95d.exe exit /b 1
if not exist rlogin.exe exit /b 1
if not exist telnet.exe exit /b 1
if not exist textps.exe exit /b 1
if not exist ctl3dins.exe exit /b 1
if not exist k95g.exe exit /b 1