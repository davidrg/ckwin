Info-Zip
==========

Info-Zip is used by the distribution scripts (mkdist.bat and mkdist-os2.bat)
to produce an archive (.zip) and self-extracting archive (.exe) of the
distribution. This is primarily important for the OS/2 version of Kermit 95
which has a text-mode installer that relies on a self-extracting EXE.

To produce .zip archives automatically, place `zip.exe` for your platform in
this directory. At the time of writing, the latest releases are:

| Architecture | Version | ftp.info-zip.org (FTP)                                     | ftp.zx.net.nz mirror (HTTP)                                                            |
|--------------|---------|------------------------------------------------------------|----------------------------------------------------------------------------------------|
| x86-64       | 3.0     | ftp://ftp.info-zip.org/pub/infozip/win32/zip300xn-x64.zip  | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/zip300xn-x64.zip  |
| x86          | 3.0     | ftp://ftp.info-zip.org/pub/infozip/win32/zip300xn.zip      | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/zip300xn.zip      | 
| IA64         | 2.31    | ftp://ftp.info-zip.org/pub/infozip/win32/zip231xn-ia64.zip | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/zip231xn-ia64.zip |
| Alpha        | 2.31    | ftp://ftp.info-zip.org/pub/infozip/win32/zip231xn-axp.zip  | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/zip231xn-axp.zip  |
| MIPS         | 2.1     | ftp://ftp.info-zip.org/pub/infozip/win32/zip21xn-ppc.zip   | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/zip21xn-ppc.zip   |
| PowerPC      | 2.1     | ftp://ftp.info-zip.org/pub/infozip/win32/zip21xn-ppc.zip   | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/zip21xn-ppc.zip   |

unzipsfx.exe
------------

The distribution scripts can also optionally create a self-extracting EXE of
the distribution. This is *required* by the OS/2 installer (setup.exe), but it
may also be useful for producing builds that are easy to extract on vintage
Windows hosts where zip files aren't natively supported by Windows Explorer.

`unzipsfx.exe` is the self-extracting EXE stub and so it is architecture
(and operating system) dependant. It's provided as part of Unzip which is
itself normally distributed as a self-extracting EXE for that operating
system and CPU architecture.

To get it, open the unzip release (eg, unz520xn-ppc.exe) as a zip file in,
eg, 7-zip, or extract it using unzip for your platform with 
`unzip unz520xn-ppc.exe`, and grab unzipsfx.exe. The rest of unzip is not
needed.

At the time of writing, the latest relesaes of unzip can be found in the
table below. The Directory column gives the name of the subdirectory
where `unzipsfx.exe` should be placed:

| Directory | OS and Architecture   | Version | ftp.info-zip.org (FTP)                                     | ftp.zx.net.nz mirror (HTTP)                                                            |
|-----------|-----------------------|---------|------------------------------------------------------------|----------------------------------------------------------------------------------------|
| `OS2`     | IBM OS/2, 32bit x86   | 6.0     | ftp://ftp.info-zip.org/pub/infozip/os2/unz600.exe          | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/os2/unz600.exe          | 
| `AMD64`   | Windows, x86-64       | 5.52    | ftp://ftp.info-zip.org/pub/infozip/win32/unz552xn-x64.exe  | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/unz552xn-x64.exe  |
| `x86`     | Windows, x86          | 6.0     | ftp://ftp.info-zip.org/pub/infozip/win32/unz600xn.exe      | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/unz600xn.exe      |
| `IA64`    | Windows, Itanium      | 5.52    | ftp://ftp.info-zip.org/pub/infozip/win32/unz552xn-ia64.exe | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/unz552xn-ia64.exe |
| `ALPHA`   | Windows NT, DEC Alpha | 5.52    | ftp://ftp.info-zip.org/pub/infozip/win32/unz552xn-axp.exe  | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/unz552xn-axp.exe  |
| `MIPS`    | Windows NT, MIPS      | 5.31    | ftp://ftp.info-zip.org/pub/infozip/win32/unz531xn-mip.exe  | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/unz531xn-mip.exe  |
| `PPC`     | Windows NT, PowerPC   | 5.20    | ftp://ftp.info-zip.org/pub/infozip/win32/unz520xn-ppc.exe  | https://ftp.zx.net.nz/pub/archive/ftp.info-zip.org/pub/infozip/win32/unz520xn-ppc.exe  |

