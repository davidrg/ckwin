@echo off
REM SET CFG=   means write host.cfg into the SCRIPTS directory 
REM Use different versions of this batchfile for different
REM simultaneous Hostmode (modem-) connections 
REM Replace [DIRECTORY] with a name of your choice for a directory
REM for each host-configuration as a place solely for host.cfg
REM .
REM SET CFG=[DIRECTORY]
SET CFG=
if !%CFG% == !  goto SPECIAL
md \%CFG%
cd \%CFG%
..\k95 ../scripts/hostmode.ksc
quit
:SPECIAL
k95 scripts/hostmode.ksc
quit
