Building OpenSSL for Kermit 95
==================================================

For full details on how to build OpenSSL for use with Kermit 95, see the
[Optional Dependencies](../doc/optional-dependencies.md) document. You should really read
that page first.

The `build.bat` script provided in this directory attempts to automate the OpenSSL build
process a little. It depends on the build-environment setup by `setenv.bat`. You'll
also need nasm and perl on your path with the Text::Template package installed as
documented by the *Optional Dependencies* page.

To use it, extract the OpenSSL distribution to a subdirectory alongside build.bat, for
example `/openssl/3.0.9` then run build.bat with the subdirectory name. For example:

```
openssl\build.bat 3.0.9
```

The script is tested with OpenSSL 1.1.1 and 3.x, but it *should* work with older versions.
If you're cross-compiling for something like Itanium with an older compiler you'll likely
run into issues at the link step though due to the generated makefile leaving off the
`/MACHINE` link flag.