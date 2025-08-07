@echo off
call ProjectSetup/CMakeSetup.bat
echo.
echo CMake Install complete with error level: %errorlevel%
PAUSE
echo.
echo Checking to see if cmake command works
cmake --version
PAUSE
call GenerateProject.bat