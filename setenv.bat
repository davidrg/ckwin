@echo off

REM Set this to where your source code lives
set root=C:\src

set include=%include%;%root%\kermit\k95
set include=%include%;%root%\kermit\k95\kui

REM Include paths for obsolete libraries we're not currently doing
REM anything with.
REM set include=%include%;%root%\openssl\0.9.7\inc32
REM set include=%include%;%root%\srp\include
REM set include=%include%;%root%\pwsdk\inc32
REM set include=%include%;%root%\kerberos\kfw-2.2-beta-2\athena\wshelper\include
REM set include=%include%;%root%\superlat\include
REM set include=%include%;%root%\kerberos\kfw-2.2-beta-2\athena\auth\krb5\src\include
REM set include=%include%;%root%\zinc\include

REM Library paths for obsolete libraries we're not currently doing
REM anything with.
REM set lib=%lib%;%root%\kerberos\kfw-2.2-beta-2\target\lib\i386\rel
REM set lib=%lib%;%root%\zinc\lib\mvcpp500

echo Include path set to: %include%
echo Library path set to: %lib%
