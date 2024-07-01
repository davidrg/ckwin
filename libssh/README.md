Building LibSSH for C-Kermit for Windows
==================================================

For full details on how to build LibSSH for use with C-Kermit for Windows, see the
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
  * LibSSH directory under /libssh
  * OpenSSL directory under /openssl. If not supplied it uses the OpenSSl directory
    configured by setenv.bat
  * zlib directory under /zlib. If not supplied it uses the zlib directory configured by
    setenv.bat

If you're just building a new version of libssh with no change to the OpenSSL or zlib
version you can just supply the first parameter. If you've just built new versions of
openssl or zlib you'll want to specify those parameters too so that libssh isn't built
against the old versions configured by setenv.bat.

Once you've built libssh you'll want to go and update setenv.bat to ensure the new
version of libssh is used in the future.

Windows XP Compatibility Fix
----------------------------

Libssh 0.10.6 includes a fix for 
[CVE-2023-6004](https://nvd.nist.gov/vuln/detail/CVE-2023-6004) which introduces
a dependency on if_nametoindex in iphlpapi.dll which is not available on 
Windows XP. A patch is provided (`xp-fix.patch`) which corrects this issue by
dynamically loading iphlpapi.dll and if that fails skipping the functionality
that depends on this library. This will of course re-introduce the vulnerability
on Windows XP which is not great.

The vulnerability appears to only affect features that C-Kermit doesn't use and
which libssh doesn't even support on Windows. So the effects of undoing the fix
are likely not serious at the moment, but this may change in the future.

If you wish to build libssh 0.10.6 with Windows XP compatibility, you can
apply this patch by running `patch -p1 < ..\xp-fix.patch` from inside the
libssh directory.