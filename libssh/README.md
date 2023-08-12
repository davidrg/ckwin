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
