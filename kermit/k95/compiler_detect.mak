
# Try to figure out what compiler we're using so that
# we use the appropriate flags, etc.

!IF ([cl 2>&1 | findstr /C:"Version 16.0" > nul] == 0)
# Visual C++ 10.0 (Visual Studio 2010)
MSC_VER = 160

!ELSEIF ([cl 2>&1 | findstr /C:"Version 15.0" > nul] == 0)
# Visual C++ 9.0 (Visual Studio 2008)
MSC_VER = 150

!ELSEIF ([cl 2>&1 | findstr /C:"Version 14.0" > nul] == 0)
# Visual C++ 8.0 (Visual Studio 2005)
MSC_VER = 140

!ELSEIF ([cl 2>&1 | findstr /C:"Version 13.1" > nul] == 0)
# Visual C++ 7.1 (Visual Studio .net 2003)
MSC_VER = 131

!ELSEIF ([cl 2>&1 | findstr /C:"Version 13.0" > nul] == 0)
# Visual C++ 7.0 (Visual Studio .net 2002)
MSC_VER = 130

!ELSEIF ([cl 2>&1 | findstr /C:"Version 12.0" > nul] == 0)
# Visual C++ 6.0 (Visual Studio 6.0)
MSC_VER = 120

!ELSEIF ([cl 2>&1 | findstr /C:"Version 11.0" > nul] == 0)
# Visual C++ 5.0 (Visual Studio 97)
MSC_VER = 110

!ELSEIF ([cl 2>&1 | findstr /C:"Version 10.0" > nul] == 0)
# Visual C++ 4.0 (there was no 3.0)
MSC_VER = 100

!ELSEIF ([cl 2>&1 | findstr /C:"Version 9.0" > nul] == 0)
# Visual C++ 2.0 (very old 32bit compiler)
MSC_VER = 90

!ELSEIF ([cl 2>&1 | findstr /C:"Version 8.0" > nul] == 0)
# Probably the ancient 16bit compiler (1.5) but its
# possible Visual C++ 1.0 32bit edition might also
# report itself as 8.0
MSC_VER = 80

!ELSE
# Just assume its something modern that we've not
# seen before
MSC_VER = 999
!ENDIF