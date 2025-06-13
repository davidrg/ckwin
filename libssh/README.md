Building LibSSH for Kermit 95
==================================================

For full details on how to build LibSSH for use with Kermit 95, see the
[Optional Dependencies](../doc/optional-dependencies.md) document. You should really read
that page first.

The `build.bat` script provided in this directory attempts to automate the LibSSH build
process a little. It depends on the build-environment setup by `setenv.bat`. You must have
already built a suitable version of OpenSSL prior to building this and optionally zlib too.

To use it, extract the LibSSH distribution to a subdirectory alongside build.bat, for
example `/libssh/libssh-0.10.5` then run build.bat with the libssh subdirectory name. 
For example:

```
libssh\build.bat libssh-0.10.5 
```

The parameters are:

| Parameter   | Example                    | Description                                                                                                                        |
|-------------|----------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| /O openssl  | /O openssl-3.0.17          | name of the OpenSSL directory under /openssl                                                                                       |
| /Z zlib     | /Z zlib-1.3.1              | Name of the zlib directory under /zlib                                                                                             |
| /K kerberos | /K C:\ckwin\kerberos\kfw41 | Full path to Kerberos for Windows SDK                                                                                              |
| /N suffix   | /N x                       | Suffix to apply to dll name (eg, /N x produces sshx.dll. The suffixes `x`, `g` and `gx` are automatically recognised by setenv.bat |
| /M dir      | /M out                     | Move built DLL and lib to the specified directory. `/M out` is automatically recognised by setenv.bat                              |
| /C          | /C                         | Do a clean (cmake --fresh, nmake clean) before build                                                                               |
| /R          | /R                         | Do a release build                                                                                                                 |
| /G          | /G                         | Apply the GSSAPI patch. Has no affect if already applied                                                                           |
| /X          | /X                         | Applies the XP patch and produces a build that doesn't use thread local storage                                                    |
| /W          | /W                         | Unapply the Windows XP patch (xp-fix.patch). Has no affect if the patch wasn't previously applied                                  |


The `/G`, `/X` and `/W` parameters require you to have `patch` in your path.
This is usually included as part of Git, so you'll want to do this before
building: `set PATH=%PATH%;C:\Program Files\Git\usr\bin`

Make sure that the path to patch isn't quoted as this confuses the 'where' tool, so
if you were to `set PATH=%PATH%;"C:\Program Files\Git\usr\bin"` patch wouldn't
be found.


GSSAPI (Kerberos) Compatibility Fix
-----------------------------------
Libssh does not currently support GSSAPI on Windows, but only a minor patch is
required to get it working. A single #define needs to be added to the top of
`gssapi.c` to ensure the correct version of the winsock header is included, and
the `FindGSSAPI.cmake` CMake module needs to be adjusted so that it can find the
MIT Kerberos for Windows SDK.

A patch is provided to do all of this, which you can apply by running 
`patch -p1 < ..\win32-gssapi.patch` inside the libssh directory, or
alternatively you can run `build.bat` with the `/G` parameter.

Note that the resulting ssh.dll will depend on gssapi64.dll (or gssapi32.dll for
a 32bit build) meaning it won't work on any systems that don't have MIT Kerberos
for Windows installed. If you're distributing your build to end-users who may or
may not need Kerberos support it may be best to produce two builds of libssh, 
one with GSSAPI support and one without.

Windows XP Compatibility Fix
----------------------------

Libssh 0.10.6 includes a fix for 
[CVE-2023-6004](https://nvd.nist.gov/vuln/detail/CVE-2023-6004) which introduces
a dependency on if_nametoindex in iphlpapi.dll which is not available on 
Windows XP. A patch is provided (`xp-fix.patch`) which corrects this issue by
dynamically loading iphlpapi.dll and if that fails skipping the functionality
that depends on this library. This will of course re-introduce the vulnerability
on Windows XP which is not great.

The vulnerability appears to only affect features that Kermit 95 doesn't use and
which libssh doesn't even support on Windows. So the effects of undoing the fix
are likely not serious at the moment, but this may change in the future.

If you wish to build libssh 0.10.6 with Windows XP compatibility, you can
apply this patch by running `patch -p1 < ..\xp-fix.patch` from inside the
libssh directory, or you can run `build.bat` with the `/X` parameter.

The `/X` parameter also builds libssh without Thread Local Storage which is
required when building Kermit 95 with an external SSH subsystem 
(CKF_SSH_DLL=yes).