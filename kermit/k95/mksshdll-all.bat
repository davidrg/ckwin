if exist k95ssh*.dll del k95ssh*.dll

set CKF_SSH_DLL_VARIANT=g
set SSH_LIB=sshg.lib
if exist k95ssh.res del k95ssh.res
call mksshdll.bat
move k95ssh.dll k95sshg.dll

set CKF_SSH_DLL_VARIANT=x
set SSH_LIB=sshx.lib
if exist k95ssh.res del k95ssh.res
call mksshdll.bat
move k95ssh.dll k95sshx.dll

set CKF_SSH_DLL_VARIANT=gx
set SSH_LIB=sshgx.lib
if exist k95ssh.res del k95ssh.res
call mksshdll.bat
move k95ssh.dll k95sshgx.dll

set CKF_SSH_DLL_VARIANT=
set SSH_LIB=ssh.lib
if exist k95ssh.res del k95ssh.res
call mksshdll.bat