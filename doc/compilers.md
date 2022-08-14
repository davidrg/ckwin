# CKW Compiler Support

Like C-Kermit on Unix, C-Kermit for Windows supports a range of
different compilers capable of targeting different versions of the
operating system.

This table outlines Which compilers you can use to target which
versions of Windows, and what features you loose in the process.

For the best security, always use the most recent compiler you 
can. Support for older compilers remains for those wishing to build
C-Kermit for vintage Windows systems, the security situation of which
is well known. C-Kermit for Windows, when built with the right compiler, 
supports Windows NT 3.51+ and Windows 95+. Windows NT 3.1, 3.50 and Win32s 
are unsupported at this time.

The CI system is setup to do builds with Visual C++ 2022, 
Visual C++ 2015, Visual C++ 2003, and OpenWatcom 1.9.



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
</tr>
<tr>
    <td>Visual C++ 2015</td>
    <td>Windows XP</td>
    <td>✅</td> <!-- Free -->
    <td>❌</td> <!-- PTY -->
    <td>❔</td> <!-- SSH -->
    <td>✅</td> <!-- NTLM/Auth -->
    <td>✅</td> <!-- TAPI -->
    <td>✅</td> <!-- GUI -->
    <td>✅</td> <!-- Console -->
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
    <td>Visual C++ 2003 Toolkit + 
        Server 2003 Platform SDK is free.
        Static CRT only.</td>
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
</tr>
</table>