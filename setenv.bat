@echo off

REM Uncomment this and set it to where your source code lives
REM set root=C:\src

REM This is required for both Windows and OS/2
set ckinclude=%root%\kermit\k95

REM This and everything else is windows-specific.
set ckwinclude=%ckinclude%;%root%\kermit\k95\kui

REM Include paths for obsolete libraries we're not currently doing
REM anything with.
REM set ckwinclude=%ckwinclude%;%root%\openssl\0.9.7\inc32
REM set ckwinclude=%ckwinclude%;%root%\srp\include
REM set ckwinclude=%ckwinclude%;%root%\pwsdk\inc32
REM set ckwinclude=%ckwinclude%;%root%\kerberos\kfw-2.2-beta-2\athena\wshelper\include
REM set ckwinclude=%ckwinclude%;%root%\superlat\include
REM set ckwinclude=%ckwinclude%;%root%\kerberos\kfw-2.2-beta-2\athena\auth\krb5\src\include
REM set ckwinclude=%ckwinclude%;%root%\zinc\include

REM Set include path for targeting Windows.
set include=%include%;%ckwinclude%

REM Library paths for obsolete libraries we're not currently doing
REM anything with.
REM set lib=%lib%;%root%\kerberos\kfw-2.2-beta-2\target\lib\i386\rel
REM set lib=%lib%;%root%\zinc\lib\mvcpp500

echo Include path set to: %include%
echo Library path set to: %lib%
