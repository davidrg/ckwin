@echo Full build...

set SUFFIX=
if [%1] NEQ [] set SUFFIX=%1

if "%CKB_STAT_PREFIX%" NEQ "" echo %CKB_STAT_PREFIX%Console Build...%CKB_STAT_SUFFIX%
@call mknt.bat N %SUFFIX%
if "%CKB_STAT_PREFIX%" NEQ "" echo %CKB_STAT_PREFIX%K95G Build...%CKB_STAT_SUFFIX%
@call mkg.bat N %SUFFIX%
if "%CKB_STAT_PREFIX%" NEQ "" echo %CKB_STAT_PREFIX%IKSD Build...%CKB_STAT_SUFFIX%
@call mkiksd.bat

if not exist cknker.exe exit /b 1
if not exist iksd.exe exit /b 1
if not exist iksdsvc.exe exit /b 1
if not exist iksdnt.exe exit /b 1
if not exist k95d.exe exit /b 1
if not exist rlogin.exe exit /b 1
if not exist telnet.exe exit /b 1
if not exist textps.exe exit /b 1
if not exist ctl3dins.exe exit /b 1
if not exist k95g.exe exit /b 1