Contributing
============

Contributions are welcome! Kermit 95 huge project with only a very small development team so any help is appreciated!

### Table of Contents
* [Getting Support](#getting-support)
  * [Documentation and other Resources](#documentation-and-other-resources)
* [Reporting Bugs](#reporting-bugs)
* [Suggesting Enhancements](#suggesting-enhancements)
* [Code Contributions](#code-contributions)
  * [What should I know before I get started?](#what-should-i-know-before-i-get-started)
* [License](#license)
  
## Getting Support

The first place for support is the documentation! It *should* cover just about everything there is to know about
Kermit 95. But if you can't find the answer you need there, you can try asking a question here on Github in the
discussions tab.

You can also email ckw@kermitproject.org with any questions or problems. If your question concerns features available
in C-Kermit on other plaforms (Linux, etc), or involves other Kermit implementations, then this is the best place to
ask.

### Documentation and other Resources

* [The K95 3.0 beta website](https://www.kermitproject.org/ckw10beta.html)
* [K95 How-To](https://www.kermitproject.org/ckwhowto.html)
* [FAQ](https://www.kermitproject.org/k95faq.html) 
* [Kermit 95 Manual](https://www.kermitproject.org/k95manual/)
* [C-Kermit 6.0 manual](https://www.kermitproject.org/onlinebooks/uck2ep2.pdf) (and its [Table of Contents](https://www.kermitproject.org/onlinebooks/uck2ep1.pdf))
  * [C-Kermit 7.0 supplement](https://www.kermitproject.org/ckermit70.html)
  * [C-Kermit 8.0 supplement](https://www.kermitproject.org/ckermit80.html)
  * [C-Kermit 9.0 supplement](https://www.kermitproject.org/ckermit90.html)

## Reporting Bugs

We use Github Issues for tracking bugs. If you think you've found a new one, have a look there to check if its
already been found. It may also be worth looking at [The K95 Bug List](https://kermitproject.org/k95manual/k95bugs.html) 
in case its a known issue with a documented workaround. If your issue isn't described anywhere, please create a new
issue so it can be fixed. 

When creating a new issue, include as much detail as you can to help reproduce the problem. Kermit 95 can produce 
debug logs (the `log debug` command) which may be helpful, though before uploading these anywhere you'll
want to check them for any sensitive information.

If you think you've found a terminal emulation bug, it would be *extremely* helpful if you're able to show what
the real terminal does in the same situation. Terminal documentation can often be inaccurate or misleading, so
unfortunately the only authorative source on "correct" behaivour is the real terminal. 

## Suggesting Enhancements

Ideas are always welcome! The best place for now is probably in the Ideas section of Github Discussions.

## Code Contributions

### What should I know before I get started?

Kermit 95 is a very large and old codebase. It started out on Unix, was ported OS/2 in the late 80s, then to Windows in
the mid 90s. It still shares about half of its code with C-Kermit on other platforms and C-Kermit does not drop support
for old platforms, so care must be taken not to break one of the many obscure platforms or compilers C-Kermit still
supports.

* Any files matching the pattern `ck[cu]*.[cwh]` come from [C-Kermit 10](https://www.kermitproject.org/ckupdates.html)
  and are not specific to Kermit 95 at all. Changes are regularly upstreamed to C-Kermit so they need to
  conform to the [C-Kermit Source Code Portability and Style Guidelines](https://www.kermitproject.org/ckcplm.html#x3).
  These guidelines rule out using many features because C-Kermit is built for many old and obscure operating systems
  that potentially don't even have ANSI C compilers available. Additionally, if your changes to these files are not
  intended to affect other platforms like OpenVMS or Stratus V/OS, make sure they're #ifdef'd for either OS2 (OS/2+Windows)
  or NT (Windows only).
* Any files matching `ck[on]*.*` are specific to OS/2 and Windows and so need to support a
  [much smaller range of compilers](doc/compilers.md) allowing standards to be a little more relaxed. Anything thats
  valid ANSI C will probably be OK. There is a little bit of C++ (in the [kui](kermit/k95/kui) folder, and the
  dialer) - this is all "C-with-classes" style C++ due to the age of some of the compilers K95 supports.
* The dialer data files (dialer.dat, registry.dat) are binary files edited by the zinc designer. It's not at all
  possible to merge these files, so if you want to make changes to these its best to get in touch first so changes
  can be coordinated.
* Kermit 95 aims to support *all* 32bit or better Windows so any code depending on APIs not available in Windows NT 3.10
  should have appropriate #ifdefs.

If in doubt feel free to ask a question on Github Discussions!

## License

By making a contribution to Kermit 95, you agree that your contributions will be licensed
under the COPYING file in the root directory of this source tree.
