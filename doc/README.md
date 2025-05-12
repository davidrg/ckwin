Kermit 95 Documentation Index
==============================
This directory contains documentation for Kermit 95:

* [Change Log](changes.md)
* [What's New in Kermit 95 v3.0](whats-new.md)
* [SSH Readme](ssh-readme.md) (slowly becoming obsolete)
* [The User Manual](#the-user-manual)
* [Quick History of Open Source Kermit 95](k95-open.md)
* [Supported Control Sequences](ctlseqs.html) - Also [available online!](https://davidrg.github.io/ckwin/dev/ctlseqs.html)
* Build instructions
  * [Supported Compilers](compilers.md)
  * [Building for Windows with Visual C++ or Open Watcom](building.md)
  * [Building for OS/2 with Open Watcom](os2-building.md)
  * [Building for Windows with MinGW](mingw-building.md)
  * [Optional Dependencies](optional-dependencies.md)
  * [Source code index](code-index.md)
  * [Build Flags](build-flags.md)

The User Manual
---------------
The [manual](manual) directory contains the Kermit 95 User Manual. This was
originally a printed booklet and a collection of text files until Kermit 95
v1.1.17 of June 1998 when it was converted into HTML 1.0 format and updated with
each release.

With the cancellation of the Kermit Project in 2011, Manning Publications
assigned the copyright back to Columbia University and in March 2011 Frank da
Cruz updated it for the web and [put it online](http://www.columbia.edu/kermit/k95manual/).
In July 2024 Frank gave permission to add the manual to the git repo and sent
through the version that was on kermitproject.org at the time. This has now
been revised for the new open-source Kermit 95.

To make keeping it up-to-date, the various version numbers, release dates, and
last modified dates scattered around the place have been updated with tags to
allow them to be updated with a script. That script is 
mkdocs.bat/mkdocs.ksc/mkdocs.rex.

### Building the Manual

The manual can be built in one of two ways:
* For the web: HTML files end in `.html`, and `index.html` is the main page of
  the manual. This is just like [the online version](http://www.columbia.edu/kermit/k95manual/).
* For distribution *with* Kermit 95. HTML files end in `.htm` for better
  compatibility with vintage computers, `index.html` is an index to the offline
  documentation shipped with Kermit 95, and `kermit95.html` is the main page of
  the manual. Can optionally be built with insecure (http:// rather than 
  https://) links to kermitproject.org for better compatibility with older 
  browsers

To build the manual you must:
* Be in the K95 build environment (have run `setenv.bat`)
* Have built the Kermit 95 distribution (run `mkdist.bat`)
* Have built the Kermit 95 distribution for A CPU architecture compatible with
  the computer you're building it on (if you're on an x86 machine, you've got
  to build Kermit 95 for x86 or x86-64 in order to build the manual) as building
  the manual involves running the freshly built version of Kermit 95.
* Kermit 95 must be built with REXX support, and must include the regutil and
  rexxre external function libraries.

The reason for the above requirements is that Kermit 95 is used to build the
manual, and version numbers and build dates from the version of Kermit 95 being
used will be injected into the manual.

To build the manual for the web (note the output path must end with a `\`):
```batch
mkdocs.bat /W
```

To build the manual for distribution with K95:
```batch
mkdocs.bat
```

To build the manual for distribution with K95 on vintage computers (fewer 
https links):
```batch
mkdocs.bat /I
```

Regardless of how you build it, the manual will end up in the dist directory, 
under `\docs\manual`