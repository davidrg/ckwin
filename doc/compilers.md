# CKW Compiler Support

Like C-Kermit on Unix, C-Kermit for Windows supports a range of
different compilers capable of targeting different versions of the
operating system.

This table outlines Which compilers you can use to target which
versions of Windows, and what features you loose (or gain) in the process.

For the best security, always use the most recent compiler you 
can. Support for older compilers remains for those wishing to build
C-Kermit for vintage Windows systems, the security situation of which
is well known. C-Kermit for Windows, when built with the right compiler, 
supports Windows NT 3.50+ and Windows 95+. Windows NT 3.1 is unsupported at 
this time though that may change in the future. Win32s (Win32 on Windows 3.x)
can not be supported due to the lack of support for threading.

The CI system is set-up to do builds with Visual C++ 2022, 
Visual C++ 2015, Visual C++ 2003, and OpenWatcom 1.9.

The Dialer is currently known to build fine with Visual C++ 2.0-7.0 (2002) and
OpenWatcom 1.9. Visual C++ 6.0 SP6 is the recommended compiler as newer versions
lack ctl3d32 - without this the application tends to look a little like
something from Windows 3.1. Compilers newer than 2002 may work but OpenZinc is
known to be incompatible with Visual C++ 14.x at this time.

<table>
<tr>
    <th>Compiler</th>
    <th>Minimum Windows Target</th>
    <th>Free?</th>
    <th>PTY</th>
    <th>SSH</th>
    <th>NTLM / Authentication</th>
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
    <td></td> <!-- Notes -->
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
    <td></td> <!-- Notes -->
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
    <td></td> <!-- Notes -->
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
    <td></td> <!-- Notes -->
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
    <td></td> <!-- Notes -->
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
    <td></td> <!-- Notes -->
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
    <td></td> <!-- Notes -->
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
    <td></td> <!-- Notes -->
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
    <td>Express Edition + Server 2003 Platform SDK is free</td>
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
    <td>Visual C++ 2.0</td>
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
        currently work well. Dialer builds but is largely untested and some
        non-functional TAPI/modem-dialing stuff may be visible.
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
    <td>To target NT 3.50, <tt>set CKT_NT31=yes</tt> before building.
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