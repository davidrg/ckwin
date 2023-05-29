REM
REM Script for building MIT Kerberos for Windows version 2.6.x
REM
REM Run from inside the k4w src directory (where the athena, doc and scripts
REM folders are)
REM
REM Requirements:
REM   * Visual C++ 6.0 or newer, with the August 2001 or newer Platform SDK
REM   * HTML Help compiler (htmlhelp.exe), available from here:
REM     https://learn.microsoft.com/en-us/previous-versions/windows/desktop/htmlhelp/microsoft-html-help-downloads
REM   * The following in your PATH: perl, sed, gawk, cat and rm
REM
REM K4W version 2.6.0 is the final version to support Windows 95.
REM

REM
REM Patch out building of aklog: It appears to depend on things not available
REM outside of MIT
REM
sed -i "s/aklog //g" athena/auth/Makefile.dir

REM
REM Patch htmlhelp build script: It depends on guiwrap.exe which appears not to
REM be available outside of MIT
REM
if exist scripts\hccwrap-patched goto :patched
sed -i "9,16 {s/^/rem /}" scripts/hhcwrap.cmd
sed -i "53,53 {s/rem //}" scripts/hhcwrap.cmd
sed -i "54,54 {s/^/rem /}" scripts/hhcwrap.cmd
copy NUL scripts\hccwrap-patched
:patched

REM Build!
set CPU=i386

pushd athena
perl ..\scripts\build.pl --softdirs NODEBUG=1
popd

:end