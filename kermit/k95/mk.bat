@echo Full build...
@call mknt.bat
@call mkg.bat

if not exist cknker.exe exit 1
if not exist iksd.exe exit 1
if not exist iksdsvc.exe exit 1
if not exist k95d.exe exit 1
if not exist rlogin.exe exit 1
if not exist telnet.exe exit 1
if not exist textps.exe exit 1
if not exist ctl3dins.exe exit 1
if not exist k95g.exe exit 1