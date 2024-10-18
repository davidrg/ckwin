IBM TCP/IP 2.0 Support
----------------------

To build the IBM TCP/IP 2.0-4.0 support library (`cko32i20.dll`) automatically,
place the required libraries and headers in this directory. For example:

```
\README.md          this file
\lib\tcp32dll.lib
\include\types.h
```

The required headers and libraries can be found in a zip file on the 
OS/2 Warp 4 CD-ROM when accessed under OS/2 (it won't be accessible from
a Windows PC):
```
\CID\IMG\MPTS\UTILITY\TOOLKIT\MPTNTK.ZIP
```
Extract that zip file into this directory such that `lib` and `include` appear 
as subdirectories alongside this README.md file, and setenv.bat should pick
it up automatically.
