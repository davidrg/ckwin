@echo off
cls
echo ============================
echo Color test - 16 colors
if exist color_tests.exe del color_tests.exe
if exist color_tests.I del color_tests.I
cl /nologo color_tests.c
if not exist color_tests.exe goto :build_failed
color_tests.exe || goto :test_failed

echo ============================
echo Color test - 16 colors debug
if exist color_tests.exe del color_tests.exe
if exist color_tests.I del color_tests.I
cl /nologo color_tests.c -DCK_COLORS_DEBUG
if not exist color_tests.exe goto :build_failed
color_tests.exe || goto :test_failed

echo ============================
echo Color test - 256 colors
if exist color_tests.exe del color_tests.exe
if exist color_tests.I del color_tests.I
cl /nologo color_tests.c -DCK_COLORS_256
if not exist color_tests.exe goto :build_failed
color_tests.exe || goto :test_failed

echo ============================
echo Color test - 24-bit color
if exist color_tests.exe del color_tests.exe
if exist color_tests.I del color_tests.I
cl /nologo color_tests.c -DCK_COLORS_24BIT
if not exist color_tests.exe goto :build_failed
color_tests.exe || goto :test_failed

echo Tests passed!
goto :end

:build_failed
echo Build failed!
goto :end

:test_failed
echo Tests failed!
goto :end

:end