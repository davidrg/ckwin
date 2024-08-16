del k95ssh*.dll

set CKF_SSH_DLL_VARIANT=g
set SSH_LIB=sshg.lib
del k95ssh.res
call mksshdll.bat
move k95ssh.dll k95sshg.dll

set CKF_SSH_DLL_VARIANT=x
set SSH_LIB=sshx.lib
del k95ssh.res
call mksshdll.bat
move k95ssh.dll k95sshx.dll

set CKF_SSH_DLL_VARIANT=gx
set SSH_LIB=sshgx.lib
del k95ssh.res
call mksshdll.bat
move k95ssh.dll k95sshgx.dll

set CKF_SSH_DLL_VARIANT=
set SSH_LIB=ssh.lib
del k95ssh.res
call mksshdll.bat