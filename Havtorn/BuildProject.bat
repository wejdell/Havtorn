@echo off
call %~dp0\ProjectSetup\SetupCMake.bat
if %errorlevel% NEQ 0 EXIT /B 1
echo.
echo Generating CMake files...
echo.
cmake -S Source -B BuildFiles
echo.
echo Building Project...
echo.
cmake --build BuildFiles
echo.
PAUSE