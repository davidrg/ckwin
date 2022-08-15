@echo Full build...
@call mknt.bat
if %errorlevel% neq 0 goto :failed
@call mkg.bat
if %errorlevel% neq 0 goto :failed
goto :end

:failed
echo Build failed
exit /b %errorlevel%

:end