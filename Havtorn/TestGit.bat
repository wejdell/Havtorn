@echo off
call ProjectSetup/GitAdditionalSetup.bat
echo.
echo Git Additional Setup complete with error level: %errorlevel%
PAUSE
echo.
echo Checking to see if git command works
git -v
PAUSE