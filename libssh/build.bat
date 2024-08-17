@echo off
setlocal enabledelayedexpansion

if [%1] == [] goto :help

REM available switches: A B D E F H I J L P Q T U V Y
for %%I in (switch valid CKB_LIBSSH CKB_GSSAPI_PATCH CKB_XP_PATCH CKB_OPENSSL CKB_ZLIB CKB_KERB CKB_NAMED CKB_MOVE CKB_CLEAN CKB_REL) do set "%%I="
for %%I in (%*) do (
    rem Value after a switch
    if defined switch (

        rem /O for OpenSSL
        if /i "!switch:~1!"=="O" set "CKB_OPENSSL=%%~I"

        rem /Z for zlib
        if /i "!switch:~1!"=="Z" set "CKB_ZLIB=%%~I"

        rem /K for kerberos
        if /i "!switch:~1!"=="K" set "CKB_KERB=%%~I"

        rem /N for suffix
        if /i "!switch:~1!"=="N" set "CKB_NAMED=%%~I"

        rem /M to move library
        if /i "!switch:~1!"=="M" set "CKB_MOVE=%%~I"

        set "switch="

    ) else (

        REM check if the current parameter is a switch (starts with / or -)
        echo(%%~I | >NUL findstr "^[-/]" && (
            set "switch=%%~I"

            rem Check for a valid switch
            for %%x in (O Z K N S M C R G X W) do (
                if /i "!switch:~1!"=="%%x" set "valid=true"
            )

            if not defined valid (
                echo Unrecognized switch: %%~I
                goto help
            )

            set "valid="

            if /i "!switch:~1!"=="C" (
               set "CKB_CLEAN=yes"
               set "switch="
            )

            if /i "!switch:~1!"=="R" (
               set "CKB_REL=yes"
               set "switch="
            )

            if /i "!switch:~1!"=="S" (
               set "CKB_STATIC=yes"
               set "switch="
            )

            if /i "!switch:~1!"=="W" (
               set "CKB_XP_PATCH=no"
               set "switch="
            )
            if /i "!switch:~1!"=="X" (
               set "CKB_XP_PATCH=yes"
               set "switch="
            )
            if /i "!switch:~1!"=="G" (
               set "CKB_GSSAPI_PATCH=yes"
               set "switch="
            )
        ) || (
            rem Not a switch - this will be the libssh directory name
            set CKB_LIBSSH=%%~I
        )
    )
)

echo zlib: "%CKB_ZLIB%"
echo openssl: "%CKB_OPENSSL%"
echo kerb: "%CKB_KERB%"
echo suffix: "%CKB_NAMED%"

where /Q patch
if errorlevel 1  goto :checkpatch
set CKB_HAVE_PATCH=yes
goto :havepatch
:checkpatch

if "%CKB_GSSAPI_PATCH%" == "yes" (
  echo Could not find `patch` in the PATH. Unable to apply GSSAPI patch.
  echo If `patch` *is* in your PATH, check its path isn't quoted as this can
  echo confuse the `where` command (eg, set path="C:\program files\" is wrong,
  echo do set path=C:\program files\ instead)
  exit /B
)

if "%CKB_XP_PATCH%" == "yes" (
  echo Could not find `patch` in the PATH. Unable to apply Windows XP patch.
  echo If `patch` *is* in your PATH, check its path isn't quoted as this can
  echo confuse the `where` command (eg, set path="C:\program files\" is wrong,
  echo do set path=C:\program files\ instead)
  exit /B
)

if "%CKB_XP_PATCH%" == "no" (
  echo Could not find `patch` in the PATH. Unable to remove Windows XP patch.
  echo If `patch` *is* in your PATH, check its path isn't quoted as this can
  echo confuse the `where` command (eg, set path="C:\program files\" is wrong,
  echo do set path=C:\program files\ instead)
  exit /B
)

:havepatch

if "%CKB_ZLIB%" NEQ "" set CKB_LIBSSH_ZLIB_ROOT=%root%\zlib\%CKB_ZLIB%
if "%CKB_ZLIB%" == "" set CKB_LIBSSH_ZLIB_ROOT=%ZLIB_ROOT%
if exist %CKB_LIBSSH_ZLIB_ROOT%\zlib.h set CKB_LIBSSH_ZLIB_OPT=-DZLIB_ROOT:PATH=%CKB_LIBSSH_ZLIB_ROOT%
if not exist %CKB_LIBSSH_ZLIB_ROOT%\zlib.h echo not found: %CKB_LIBSSH_ZLIB_ROOT%\zlib.h

if "%CKB_OPENSSL%" NEQ "" set CKB_LIBSSH_OPENSSL_ROOT=%root%\openssl\%CKB_OPENSSL%
if "%CKB_OPENSSL%" == "" set CKB_LIBSSH_OPENSSL_ROOT=%openssl_root%\

if "%CKB_KERB%" == "" set CKB_KERB=%k4w_root%
if exist %CKB_KERB%\include\krb5.h set CKB_KERBEROS_OPT=-DGSSAPI_ROOT_DIR=%CKB_KERB%
if "%CKB_KERBEROS_OPT%" NEQ "" echo Building with GSSAPI support
if "%CKB_KERBEROS_OPT%" == "" set CKB_KERBEROS_OPT=-DWITH_GSSAPI=OFF

if "%CKB_NAMED%" NEQ "" set CKB_NAME_OPT=-DCMAKE_SHARED_LIBRARY_SUFFIX_C="%CKB_NAMED%.dll"
if "%CKB_NAME_OPT%" NEQ "" echo Building library named ssh%CKB_NAMED%.dll

if "%CKB_CLEAN%" == "yes" set CKB_FRESH=--fresh


echo.
echo LibSSH Build Configuration
echo ---------------------------
echo LibSSH: %CKB_LIBSSH%
echo OpenSSL: %CKB_LIBSSH_OPENSSL_ROOT%
if "%CKB_LIBSSH_ZLIB_OPT%" NEQ "" echo zlib: %CKB_LIBSSH_ZLIB_ROOT%
if "%CKB_LIBSSH_ZLIB_OPT%" == "" echo zlib: not used
if "%CKB_KERBEROS_OPT%" NEQ "-DWITH_GSSAPI=OFF" echo Kerberos: %CKB_KERB%
if "%CKB_KERBEROS_OPT%" == "-DWITH_GSSAPI=OFF" echo Kerberos: not used
if "%CKB_NAMED%" NEQ "" echo Suffix: %CKB_NAMED%
if "%CKB_MOVE%" NEQ "" echo Move to: %CKB_MOVE%
if "%CKB_REL%" == "yes" echo Release build
if "%CKB_STATIC%" == "yes" echo Static library
if "%CKB_GSSAPI_PATCH%" == "yes" echo Apply GSSAPI patch
if "%CKB_XP_PATCH%" == "yes" echo Apply Windows XP patch
if "%CKB_XP_PATCH%" == "no" echo Reverse Windows XP patch
echo.

if "%CKB_REL%" == "yes" set CKB_REL=-DCMAKE_BUILD_TYPE=Release
if "%CKB_STATIC%" == "yes" set CKB_STATIC=-DBUILD_SHARED_LIBS=OFF

pushd %root%\libssh\%CKB_LIBSSH%

if "%CKB_GSSAPI_PATCH%" == "yes" (
echo Applying GSSAPI patch
patch -N --batch --silent -p1 < %root%\libssh\win32-gssapi.patch
)

if "%CKB_XP_PATCH%" == "yes" (
echo Applying Windows XP patch
patch -N --batch --silent -p1 < %root%\libssh\xp-fix.patch
rem On Windows versions prior to Vista, Thread Local Storage isn't setup
rem correctly when a library is loaded using LoadLibrary (which is what K95
rem does) resulting in hard-to-debug errors. LibSSH only uses Thread Local
rem Storage for logging so when building for XP we just turn that off. This may
rem have some performance impact but the alternative is it not working at all.
set XP_TLS=-DHAVE_MSC_THREAD_LOCAL_STORAGE=0
)

if "%CKB_XP_PATCH%" == "no" (
echo Removing Windows XP patch
patch -R --force --silent -p1 < %root%\libssh\xp-fix.patch
set XP_TLS=
)

REM run the build!
mkdir build
cd build
cmake .. %CKB_FRESH% -G "NMake Makefiles" %CKB_STATIC% %CKB_REL% -DOPENSSL_ROOT_DIR=%CKB_LIBSSH_OPENSSL_ROOT% %CKB_LIBSSH_ZLIB_OPT% %CKB_KERBEROS_OPT% %CKB_NAME_OPT% %XP_TLS% -DWITH_DSA=ON
if %errorlevel% neq 0 exit /b %errorlevel%
if "%CKB_CLEAN%" == "yes" nmake clean
nmake
if %errorlevel% neq 0 exit /b %errorlevel%
if "%CKB_MOVE%" == "" goto :skiprename
cd src
ren ssh.lib ssh%CKB_NAMED%.lib
ren ssh.exp ssh%CKB_NAMED%.exp
ren ssh.pdb ssh%CKB_NAMED%.pdb
mkdir %CKB_MOVE%
move /Y ssh%CKB_NAMED%.* %CKB_MOVE%\
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..
:skiprename
cd ..

popd

echo.
echo LibSSH has built. If it built successfully, you'll want to open a new
echo terminal and run the following to use it:
echo   set root=%root%
if exist %CKB_OPENSSL_ZLIB_ROOT%\zlib.h echo   set zlib_root_override=%CKB_OPENSSL_ZLIB_ROOT%
echo   set openssl_root_override=%CKB_OPENSSL_ROOT_DIR%
echo   set libssh_root_override=%%root%%\libssh\%CKB_LIBSSH%
echo   set libssh_build_override=%%root%%\libssh\%CKB_LIBSSH%\build
echo   cd %%root%%
echo   setenv.bat
echo alternatively, update setenv.bat and update the various root paths
rem echo
rem echo If you built with a suffix (/N), or moved the built library to a different
rem echo location (/M) you'll need to do something along the lines of:
rem echo    set CKF_SSH=yes
rem echo    set include=%include%;%root%\libssh\%CKB_LIBSSH%\include;%root%\libssh\%CKB_LIBSSH%\build\include;
rem echo    set lib=%lib%;path\to\the\.lib-file
rem echo    set SSH_LIB=ssh-lib-name.lib
rem echo    set CK_DIST_DLLS=%CK_DIST_DLLS% path\to\the\.dll-file
rem echo    set CK_DIST_DLLS=%CK_DIST_DLLS% path\to\the\.dll-file   (/S was not used)
rem echo    set CKF_LIBSSH_STATIC=yes                               (/S was used)
echo.

goto :end

:setenv
echo.
echo Error: C-Kermit build environment has not been setup. Run setenv.bat.
echo.
goto :end

:help
echo.
echo Usage: build.bat [/C] [/R] [/G] [/X] [/W] [/O openssl_root] [/Z zlib_root] [/K kerberos_root] [/N name_suffix] [/M move_to] libssh_dir
echo   Builds LibSSH in the specified directory under \libssh
echo.
echo Parameters
echo    libssh      Build LibSSH in \libssh\${libssh}
echo    /O openssl  Use OpenSSL in \openssl\${openssl}. Optional. If not specified,
echo                the OpenSSL root configured by set-env.bat will be used.
echo    /Z zlib     Use zlib in \zlib\${zlib}. Optional. If not specified,
echo                the zlib root configured by set-env.bat will be used. If no zlib
echo                can be found, LibSSH will be built without zlib support.
echo    /K kerberos The MIT Kerberos for Windows root directory. If not
echo                specified, the kerberos root configured by set-env.bat will
echo                be used
echo    /N name_suf Apply the specified suffix to the built dlls filename.
echo                Suffixes x, g, and gx will be automatically  picked up by
echo                setenv.bat
echo    /M move_to  Move the built dll and lib to ${move_to}. Target directory
echo                will be created if necessary. Recommended to use "out" as
echo                this is what setenv.bat will look for.
echo    /C          Do a clean build (cmake --fresh, nmake clean)
echo    /R          Do a release build
rem echo    /S          Produce a static library rather than DLL. Be aware of LGPL
rem echo                requirements!
echo    /G          Apply GSSAPI patch (win32-gssapi.patch). Has no effect if
echo                already applied.
echo    /X          Builds LibSSH without using Thread Local Storage for
echo                compatibility with XP and Server 2003, and also applies the
echo                Windows XP compatibility patch (xp-fix.patch) if necessary.
echo    /W          Unapply the Windows XP patch (xp-fix.patch). Has no effect
echo                if the xp-fix patch was not previously applied.
echo.
echo This script should be run from within the C-Kermit build environment setup by
echo setenv.bat.
echo.

:end