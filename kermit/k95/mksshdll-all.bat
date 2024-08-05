set CKF_SSH_DLL_VARIANT=
del k95ssh.dll
del k95ssh.res
call mksshdll.bat
move k95ssh.dll dist\k95ssh.dll

set CKF_SSH_DLL_VARIANT=g
del k95ssh.dll
del k95ssh.res
call mksshdll.bat
move k95ssh.dll dist\k95sshg.dll

set CKF_SSH_DLL_VARIANT=x
del k95ssh.dll
del k95ssh.res
call mksshdll.bat
move k95ssh.dll dist\k95sshx.dll

set CKF_SSH_DLL_VARIANT=gx
del k95ssh.dll
del k95ssh.res
call mksshdll.bat
move k95ssh.dll dist\k95sshgx.dll

set CKF_SSH_DLL_VARIANT=