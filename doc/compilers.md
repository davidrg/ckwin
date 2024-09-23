# Compiler Support

Kermit 95 should build with all released 32bit or better 
versions of Microsoft Visual C++ as well as 32bit versions of 
OpenWatcom 1.9. For the best security, always use the most recent
compiler you can. Support for older compilers remains for those wishing 
to build Kermit 95 for vintage Windows systems, the security situation of 
which is well known.

The table below outlines which compilers you can use to target which
versions of Windows, and what features you loose (or gain) in the process.
The "Tested Often" column indicates which compilers are tested regularly 
either automatically by github or through occasional manual builds. These
compilers are highly likely to work.

ARM64 and Itanium are built automatically by github actions but
this project lacks the required hardware to actually test these builds.
If you discover any issues please log a bug.

The Dialer is currently known to build fine with Visual C++ 2.0-7.0 (2002) and
OpenWatcom 1.9. Visual C++ 6.0 SP6 is the recommended compiler as newer versions
lack ctl3d32 - without this the application tends to look a little like
something from Windows 3.1. Compilers newer than 2002 may work but OpenZinc is
known to be incompatible with Visual C++ 14.x.

The highly optional k95cinit.exe utility requires Visual C++ 1.5 to be built.

<table>
<tr>
    <th>Compiler</th>
    <th>Arch</th>
    <th>Minimum Windows Target</th>
    <th>Free?</th>
    <th>Tested Often</th>
    <th>PTY</th>
    <th>SSH</th>
    <th>NTLM Auth</th>
    <th>TAPI</th>
    <th>Dialer</th>
    <th>Notes</th>
</tr>
<tr>
    <td rowspan="3">Visual C++ 2022</td>
    <td>x86-64, x86</td>
    <td>Windows Vista SP2</td>
    <td rowspan="3">✅</td> <!-- Free -->
    <td rowspan="2">✅</td> <!-- Tested -->
    <td rowspan="3">✅</td> <!-- PTY -->
    <td rowspan="3">✅</td> <!-- SSH -->
    <td rowspan="3">✅</td> <!-- NTLM/Auth -->
    <td rowspan="3">✅</td> <!-- TAPI -->
    <td rowspan="3">❌</td> <!-- Dialer -->
    <td rowspan="3"></td> <!-- Notes -->
</tr>
<tr>
    <td>ARM64</td>
    <td></td>
    
</tr>
<tr>
    <td>ARM32</td>
    <td></td>
    <td>❌</td>   <!-- Tested often: not really -->
</tr>
<tr>
    <td rowspan="3">Visual C++ 2019</td>
    <td>x86-64, x86</td>
    <td>Windows XP SP3</td>
    <td rowspan="3">✅</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td rowspan="3">✅</td> <!-- PTY -->
    <td rowspan="3">✅</td> <!-- SSH -->
    <td rowspan="3">✅</td> <!-- NTLM/Auth -->
    <td rowspan="3">✅</td> <!-- TAPI -->
    <td rowspan="3">❌</td> <!-- Dialer -->
    <td rowspan="3"></td> <!-- Notes -->
</tr>
<tr>
    <td>ARM64</td>
    <td></td>
    <td>❌</td> <!-- Tested -->
</tr>
<tr>
    <td>ARM32</td>
    <td></td>
    <td>✅</td> <!-- Tested -->
</tr>
<tr>
    <td rowspan="3">Visual C++ 2017</td>
    <td>x86-64, x86</td>
    <td>Windows XP SP3</td>
    <td rowspan="3">✅</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td rowspan="3">❌</td> <!-- PTY -->
    <td rowspan="3">✅</td> <!-- SSH -->
    <td rowspan="3">✅</td> <!-- NTLM/Auth -->
    <td rowspan="3">✅</td> <!-- TAPI -->
    <td rowspan="3">❌</td> <!-- Dialer -->
    <td rowspan="3"></td> <!-- Notes -->
</tr>
<tr>
    <td>ARM64</td>
    <td></td>
    <td>❌</td> <!-- Tested -->
</tr>
<tr>
    <td>ARM32</td>
    <td></td>
    <td>❌</td> <!-- Tested -->
</tr>
<tr>
    <td rowspan="3">Visual C++ 2015</td>
    <td>x86, x86-64</td>
    <td>Windows XP</td>
    <td rowspan="3">✅</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td rowspan="3">❌</td> <!-- PTY -->
    <td rowspan="3">✅</td> <!-- SSH -->
    <td rowspan="3">✅</td> <!-- NTLM/Auth -->
    <td rowspan="3">✅</td> <!-- TAPI -->
    <td rowspan="3">❌</td> <!-- Dialer -->
    <td rowspan="3"></td> <!-- Notes -->
</tr>
<tr>
    <td>ARM64</td>
    <td></td>
    <td>❌</td> <!-- Tested -->
</tr>
<tr>
    <td>ARM32</td>
    <td></td>
    <td>❌</td> <!-- Tested -->
</tr>
<tr>
    <td>Visual C++ 2013</td>
    <td>x86-64, x86</td>
    <td>Windows XP</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>❔</td> <!-- Dialer -->
    <td>Rarely tested but should work</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2012</td>
    <td>x86-64, x86</td>
    <td>Windows XP</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>❔</td> <!-- Dialer -->
    <td>Rarely tested but should work</td> <!-- Notes -->
</tr>
<tr>
    <td rowspan="2">Visual C++ 2010</td>
    <td>x86-64, x86</td>
    <td>Windows XP</td>
    <td rowspan="2">✅</td> <!-- Free -->
    <td>❌</td> <!-- Tested -->
    <td rowspan="2">❌</td> <!-- PTY -->
    <td rowspan="2">❌</td> <!-- SSH -->
    <td rowspan="2">✅</td> <!-- NTLM/Auth -->
    <td rowspan="2">✅</td> <!-- TAPI -->
    <td rowspan="2">❔</td> <!-- Dialer -->
    <td rowspan="2">Free Platform SDK 7.1 includes this compiler and works fine.</td> <!-- Notes -->
</tr>
<tr>
    <td>IA64</td>
    <td></td>
    <td>✅</td> <!-- Tested -->
</tr>
<tr>
    <td rowspan="2">Visual C++ 2008</td>
    <td>x86, x86-64</td>
    <td>Windows 2000</td>
    <td rowspan="2">✅</td> <!-- Free -->
    <td rowspan="2">❌</td> <!-- Tested -->
    <td rowspan="2">❌</td> <!-- PTY -->
    <td rowspan="2">❌</td> <!-- SSH -->
    <td rowspan="2">✅</td> <!-- NTLM/Auth -->
    <td rowspan="2">✅</td> <!-- TAPI -->
    <td rowspan="2">❔</td> <!-- Dialer -->
    <td rowspan="2">Rarely tested but should work</td> <!-- Notes -->
</tr>
<tr>
    <td>IA64</td>
    <td></td>
</tr>
<tr>
    <td rowspan="2">Visual C++ 2005</td>
    <td>x86-64, x86</td>
    <td>Windows NT 4, 
        Windows 98</td>
    <td rowspan="2">✅*</td> <!-- Free -->
    <td>❌</td> <!-- Tested -->
    <td rowspan="2">❌</td> <!-- PTY -->
    <td rowspan="2">❌</td> <!-- SSH -->
    <td rowspan="2">✅</td> <!-- NTLM/Auth -->
    <td rowspan="2">✅</td> <!-- TAPI -->
    <td rowspan="2">❔</td> <!-- Dialer -->
    <td rowspan="2">Express Edition + Server 2003 Platform SDK is free.</td>
</tr>
<tr>
    <td>IA64</td>
    <td></td>
    <td>✅</td> <!-- Tested -->
</tr>
<tr>
    <td>Visual C++ 2003</td>
    <td>x86</td>
    <td>Windows NT 4, 
        Windows 95</td>
    <td>✅*</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>❔</td> <!-- Dialer -->
    <td>Visual C++ 2003 Toolkit + 
        Server 2003 Platform SDK is free.
        Static CRT only. Can not build dialer (no lib.exe)</td>
</tr>
<tr>
    <td>Visual C++ 2002 Professional</td>
    <td>x86</td>
    <td>Windows NT 4, 
        Windows 95</td>
    <td>❌</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- Dialer -->
    <td></td> <!-- Notes -->
</tr>
<tr>
    <td>January 2000 Platform SDK</td>
    <td>Alpha 64bit (AXP64)</td> <!-- Yes, a 64bit DEC Alpha app for 64bit Windows on the DEC Alpha -->
    <td>64bit Windows 2000 for DEC Alpha</td>
    <td>✅</td> <!-- Free: Was available for public download from microsoft -->
    <td>❌</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>❔</td> <!-- Dialer -->
    <td>Code builds fine. Finding a 64bit version of Windows for
        the DEC Alpha to run it on is another matter entirely.</td> <!-- Notes -->
</tr>
<tr>
    <td rowspan="2">Visual C++ 6.0 Professional</td>
    <td>x86</td>
    <td>Windows NT 3.51, 
        Windows 95</td>
    <td rowspan="2">❌</td> <!-- Free -->
    <td rowspan="2">✅</td> <!-- Tested -->
    <td rowspan="2">❌</td> <!-- PTY -->
    <td rowspan="2">❌</td> <!-- SSH -->
    <td rowspan="2">✅</td> <!-- NTLM/Auth -->
    <td rowspan="2">✅</td> <!-- TAPI -->
    <td rowspan="2">✅</td> <!-- Dialer -->
    <td>Service Pack 6 tested</td> <!-- Notes -->
</tr>
<tr>
    <td>Alpha</td>
    <td>Windows NT 3.51?</td>
    <td>Service Pack 1</td> <!-- Notes -->
</tr>
<tr>
    <td rowspan="2">Visual C++ 5.0 (Visual Studio 97) Professional</td>
    <td>x86</td>
    <td>Windows NT 3.51, 
        Windows 95</td>
    <td rowspan="2">❌</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td rowspan="2">❌</td> <!-- PTY -->
    <td rowspan="2">❌</td> <!-- SSH -->
    <td rowspan="2">❌</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- Dialer -->
    <td></td> <!-- Notes -->
</tr>
<tr>
    <td>Alpha</td>
    <td>Windows NT 3.51?</td>
    <td>❌</td> <!-- Tested -->
    <td>❔</td> <!-- TAPI -->
    <td>❔</td> <!-- Dialer -->
    <td>Untested</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 4.x Professional Edition</td>
    <td>x86</td>
    <td>Windows NT 3.51, 
        Windows 95, 
        Win32s 1.25</td>
    <td rowspan="4">❌</td> <!-- Free -->
    <td rowspan="4">✅</td> <!-- Tested -->
    <td rowspan="4">❌</td> <!-- PTY -->
    <td rowspan="4">❌</td> <!-- SSH -->
    <td rowspan="4">❌</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td rowspan="4">✅</td> <!-- Dialer -->
    <td></td> <!-- Notes -->
</tr>
<tr>
    <td rowspan="3">
    Visual C++ 4.x RISC Edition</td>
    <td>MIPS</td>
    <td rowspan="3">Windows NT 3.51?</td>
    <td>❌</td> <!-- TAPI -->
    <td>TAPI support does not build (compiler bug?)</td> <!-- Notes -->
</tr>
<tr>
    <td>PowerPC</td>
    <td>✅</td> <!-- TAPI -->
    <td>No debug logging support due to TOC size limit.</td> <!-- Notes -->
</tr>
<tr>
    <td>Alpha</td>
    <td>✅</td> <!-- TAPI -->
    <td></td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2.x</td>
    <td>x86</td>
    <td>Windows NT 3.1, Win32s 1.2</td>
    <td rowspan="3">❌</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td rowspan="3">❌</td> <!-- PTY -->
    <td rowspan="3">❌</td> <!-- SSH -->
    <td rowspan="3">❌</td> <!-- NTLM/Auth -->
    <td rowspan="3">❌</td> <!-- TAPI -->
    <td>✅</td> <!-- Dialer -->
    <td>No toolbar or some GUI dialogs, window resizing scales font doesn't
        currently work well. Dialer builds refuses to start on NT 3.50 (see bug [#136](https://github.com/davidrg/ckwin/issues/136))
        and would likely have some non-functional TAPI/modem-dialing stuff may be visible if it did. 
        Targets NT 3.50 and 3.10 by default. To target NT 3.50 only (no runtime
        checks for NT 3.50-only APIs), set <tt>set CKT_NT35=no</tt> before building.
        K95 still has known issues when actually running on NT 3.1.
    </td>
</tr>
<tr>
   <td rowspan="2">Visual C++ 2.x RISC Edition for Alpha/MIPS</td>
    <td>MIPS</td>
    <td rowspan="2">Windows NT 3.1?</td>
    <td rowspan="2">❌</td> <!-- Tested -->
    <td rowspan="2">❔</td> <!-- Dialer -->
    <td rowspan="2">RISC Edition is Untested</td> <!-- Notes -->
</tr>
<tr>
    <td>Alpha</td>
</tr>
<tr>
    <td>Visual C++ 1.5x</td>
    <td>x86-16</td>
    <td>Windows 3.x</td>
    <td>❌</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❌</td> <!-- TAPI -->
    <td>❌</td> <!-- Dialer -->
    <td>This compiler is supported for building the k95cinit.exe utility only.
    </td>
</tr>
<tr>
    <td>Visual C++ 1.0 32-bit Edition</td>
    <td>x86</td>
    <td>Windows NT 3.1, Win32s 1.1</td>
    <td>❌</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❌</td> <!-- TAPI -->
    <td>❌</td> <!-- Dialer -->
    <td>
        Compiler works but there are unresolved issues running 
        on NT 3.1
    </td>
</tr>
<tr>
    <td>OpenWatcom 1.9</td>
    <td>x86</td>
    <td>Windows NT 3.51, 
        Windows 95</td>
    <td>✅</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- Dialer -->
    <td>
        To target NT 3.50, <tt>set CKT_NT35=yes</tt> before building. Can not
        target NT 3.1 (it builds fine with <tt>set CKT_NT31=yes</tt> but the resulting
        binary gives "Unexpected error: 11" on NT 3.1)
    </td>
</tr>
<tr>
    <td>OpenWatcom 2.0</td>
    <td>x86</td>
    <td>Windows NT 3.51, 
        Windows 95</td>
    <td>✅</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>❔</td> <!-- Dialer -->
    <td>2022-08-01 build tested, targeting 32bit win32</td> <!-- Notes -->
</tr>
<tr>
    <td>MinGW</td>
    <td>x86</td>
    <td>❔</td>
    <td>✅</td> <!-- Free -->
    <td>✅</td> <!-- Tested -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>❌</td> <!-- Dialer -->
    <td>Limited support for development purposes.
    </td> <!-- Notes -->
</tr>
</table>

## Building with the Windows NT SDK

Some versions of the Platform SDK include a compiler. Sometimes that compiler
is usable for building K95, sometimes it is not.

### Windows NT 3.1
Building with this SDK *is* supported, but you also require some parts from
Visual C++ as the included nmake.exe is too old and the SDK doesn't include
link.exe (instead it has link32.exe which is incompatible). When using a newer
link.exe, you may also need to grab cvtres.exe (link.exe version 3.0 from
Visual C++ 4.0 is incompatible with the version of cvtres.exe included in the
NT 3.1 SDK, possibly earlier versions of link.exe might be ok.)

Both the MIPS and i386 compilers should be correctly detected and configured
to statically link against the C runtime and set the subsystem version to 3.1.

### Windows NT 3.50

As found on the *Microsoft Solutions Development Kit*, the only compiler or linker 
included is for the Alpha platform and is at a similar level to Visual C++ 1.0 32bit
(MSC 8.0). The compiler works but the linker does not, so you'll need a newer
linker from Visual C++ 2.0 or 4.0. You may also need an updated cvtres.exe.

This Alpha cmopiler should be correctly detected and configured to statically
link against the C runtime and set the subsystem version to 3.1.

### January 2000 Platform SDK

The January 2000 Platform SDK includes a very early version of Visual C++ 2002
calling itself `Microsoft (R) & Digital (TM) Alpha C/C++ Optimizing Compiler Version 13.00.8499`.
This compiler is capable of targeting both regular 32bit Alpha NT and
the never released internal only 64bit Windows 2000 port (AXP64).

This is likely both the final publicly released version of Visual C++ for the
Alpha, and the oldest publicly released version of Visual C++ for targeting
64bit Windows.

When built with this SDK, the build system will default to targeting AXP64.
The build should complete without problems but the resulting binaries can not
be run on any publicly released version of Windows.

While the compiler in this SDK is capable of doing 32bit builds, the Platform
SDK headers have some issues (complains about size_t being redefined) and the
required import libraries are not present.