@echo off
call TestCmakeInstall.bat
echo.
echo CMake Install complete with error level: %errorlevel%
PAUSE
cmake --version
PAUSE