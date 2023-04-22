# CKW Compiler Support

Like C-Kermit on Unix, C-Kermit for Windows supports a range of
different compilers capable of targeting different versions of the
operating system on different CPU arhitectures.

This table outlines which compilers you can use to target which
versions of Windows, and what features you loose (or gain) in the process.

For the best security, always use the most recent compiler you 
can. Support for older compilers remains for those wishing to build
C-Kermit for vintage Windows systems, the security situation of which
is well known. C-Kermit for Windows, when built with the right compiler, 
supports Windows NT 3.50+ and Windows 95+. Limited support for Windows NT 3.1
is also present but a few bugs impact its usefulness there. Win32s (Win32 
on Windows 3.x) can not be supported due to the lack of support for threading.

The CI system is set up to do builds with Visual C++ 2022, 2019, 2015, 2003, 
and OpenWatcom 1.9. The Visual C++ 2015/2019/2022 compilers are set up to do x86,
x86-64, arm and arm64 builds.

The Dialer is currently known to build fine with Visual C++ 2.0-7.0 (2002) and
OpenWatcom 1.9. Visual C++ 6.0 SP6 is the recommended compiler as newer versions
lack ctl3d32 - without this the application tends to look a little like
something from Windows 3.1. Compilers newer than 2002 may work but OpenZinc is
known to be incompatible with Visual C++ 14.x at this time.

The highly optional k95cinit.exe utility requires Visual C++ 1.5 to be built.

Support for the RISC NT platforms is extremely limited as the C-Kermit for Windows
Project does not have access to the required compilers. To build for Alpha, MIPS
or PowerPC Windows NT you need to run the appropriate variant of Visual C++ RISC
Edition on the hardware platform you're building for. The build process should
be the same as for the equivalent x86 version of the compiler but the RISC
compilers are known to be a bit quirky so there may be undiscovered issues. If
you run into build errors, log a bug.

Support for ARM and ARM64 builds is limited to ensuring it compiles - due to a
lack of windows-compatible ARM hardware the C-Kermit for Windows project has no
ability to actually test the binaries produced. ARM32 builds do not include
SSH support (as libssh is incompatible), and both ARM32 and ARM64 exclude zlib
(also incompatible)

<table>
<tr>
    <th>Compiler</th>
    <th>Minimum Windows Target</th>
    <th>Free?</th>
    <th>PTY</th>
    <th>SSH</th>
    <th>NTLM Auth</th>
    <th>TAPI</th>
    <th>GUI</th>
    <th>Console</th>
    <th>Dialer</th>
    <th>Notes</th>
</tr>
<tr>
    <td>Visual C++ 2022</td>
    <td>Windows Vista SP2</td>
    <td>✅</td> <!-- Free -->
    <td>✅</td> <!-- PTY -->
    <td>✅</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❌</td> <!-- Dialer -->
    <td>Supported for 32bit and 64bit</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2019</td>
    <td>Windows XP SP3</td>
    <td>✅</td> <!-- Free -->
    <td>✅</td> <!-- PTY -->
    <td>✅</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❌</td> <!-- Dialer -->
    <td>Supported for 32bit and 64bit</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2017</td>
    <td>Windows XP SP3</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>✅</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❌</td> <!-- Dialer -->
    <td>Supported for 32bit and 64bit</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2015</td>
    <td>Windows XP</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>✅</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❌</td> <!-- Dialer -->
    <td>Supported for 32bit and 64bit</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2013</td>
    <td>Windows XP</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Rarely tested but should work</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2012</td>
    <td>Windows XP</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Rarely tested but should work</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2010</td>
    <td>Windows XP</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Rarely tested but should work</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2008</td>
    <td>Windows 2000</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Rarely tested but should work</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2005</td>
    <td>Windows NT 4, 
        Windows 98</td>
    <td>✅*</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Express Edition + Server 2003 Platform SDK is free. Rarely tested but should work</td>
</tr>
<tr>
    <td>Visual C++ 2003</td>
    <td>Windows NT 4, 
        Windows 95</td>
    <td>✅*</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Visual C++ 2003 Toolkit + 
        Server 2003 Platform SDK is free.
        Static CRT only. Can not build dialer (no lib.exe)</td>
</tr>
<tr>
    <td>Visual C++ 2002</td>
    <td>Windows NT 4, 
        Windows 95</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>✅</td> <!-- Dialer -->
    <td></td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 6.0, Alpha AXP</td>
    <td>Windows NT 3.51?</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❔</td> <!-- NTLM/Auth -->
    <td>❔</td> <!-- TAPI -->
    <td>❔</td> <!-- GUI -->
    <td>❔</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Untested</td> <!-- Notes -->
</tr>    
<tr>
    <td>Visual C++ 6.0 SP6</td>
    <td>Windows NT 3.51, 
        Windows 95</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>✅</td> <!-- Dialer -->
    <td></td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 5.0 RISC Edition (Alpha)</td>
    <td>Windows NT 3.51?</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❔</td> <!-- TAPI -->
    <td>❔</td> <!-- GUI -->
    <td>❔</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Untested</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 5.0 (Visual Studio 97)</td>
    <td>Windows NT 3.51, 
        Windows 95</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>✅</td> <!-- Dialer -->
    <td></td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 4.0 RISC Edition - MIPS</td>
    <td>Windows NT 3.51?</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❌</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>TAPI support does not build, Dialer untested</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 4.0 RISC Edition - PowerPC</td>
    <td>Windows NT 3.51?</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❔</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Dialer untested. Unresolved issues require disabling TAPI, XYZMODEM and debug logging.</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 4.0 RISC Edition - Alpha</td>
    <td>Windows NT 3.51?</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❔</td> <!-- TAPI -->
    <td>❔</td> <!-- GUI -->
    <td>❔</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Untested but likely to work. Dialer status unknown.</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 4.0</td>
    <td>Windows NT 3.51, 
        Windows 95, 
        Win32s 1.25</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>✅</td> <!-- Dialer -->
    <td></td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2.0 RISC Edition - MIPS</td>
    <td>Windows NT 3.1</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❌</td> <!-- TAPI -->
    <td>❔</td> <!-- GUI -->
    <td>❔</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Untested</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2.0 RISC Edition - Alpha</td>
    <td>Windows NT 3.51?</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❌</td> <!-- TAPI -->
    <td>❔</td> <!-- GUI -->
    <td>❔</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>Untested</td> <!-- Notes -->
</tr>
<tr>
    <td>Visual C++ 2.x</td>
    <td>Windows NT 3.1, Win32s 1.2</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❌</td> <!-- TAPI -->
    <td>✅*</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>✅</td> <!-- Dialer -->
    <td>No toolbar or some GUI dialogs, window resizing scales font doesn't
        currently work well. Dialer builds refuses to start on NT 3.50 (see bug [#136](https://github.com/davidrg/ckwin/issues/136))
        and would likely have some non-functional TAPI/modem-dialing stuff may be visible if it did. 
        Targets NT 3.50 and 3.10 by default. To target NT 3.50 only (no runtime
        checks for NT 3.50-only APIs), set <tt>set CKT_NT35=no</tt> before building.
        CKW still has known issues when actually running on NT 3.1.
    </td>
</tr>
<tr>
    <td>Visual C++ 1.5x</td>
    <td>Windows 3.x</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❌</td> <!-- TAPI -->
    <td>❌</td> <!-- GUI -->
    <td>❌</td> <!-- Console -->
    <td>❌</td> <!-- Dialer -->
    <td>This compiler is supported for building the k95cinit.exe utility only.
    </td>
</tr>
<tr>
    <td>Visual C++ 1.0 32-bit Edition</td>
    <td>Windows NT 3.1, Win32s 1.1</td>
    <td>❌</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>❌</td> <!-- NTLM/Auth -->
    <td>❌</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>❌</td> <!-- Console -->
    <td>❌</td> <!-- Dialer -->
    <td>
        Compiler works but there are unresolved issues running on NT 3.1
    </td>
</tr>
<tr>
    <td>OpenWatcom 1.9</td>
    <td>Windows NT 3.51, 
        Windows 95</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>✅</td> <!-- Dialer -->
    <td>
        To target NT 3.50, <tt>set CKT_NT35=yes</tt> before building. Can not
        target NT 3.1 (it builds fine with <tt>set CKT_NT31=yes</tt> but the resulting
        binary gives "Unexpected error: 11" on NT 3.1)
    </td>
</tr>
<tr>
    <td>OpenWatcom 2.0</td>
    <td>Windows NT 3.51, 
        Windows 95</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❌</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
    <td>❔</td> <!-- Dialer -->
    <td>2022-08-01 build tested, targeting 32bit win32</td> <!-- Notes -->
</tr>
</table>

Building with the Windows NT 3.1 and 3.50 SDKs is *not* supported. The versions of
nmake and link in the NT 3.1 SDK are too old (the rest is fine), while the Alpha AXP
linker in the NT 3.50 SDK is incompatible and/or broken.
