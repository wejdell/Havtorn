@echo off
call %~dp0\SetupCMake.bat
if %errorlevel% NEQ 0 EXIT /B 1
call %~dp0\SetupBuildTools.bat
if %errorlevel% NEQ 0 EXIT /B 1
echo Generating CMake files...
cmake -G "Visual Studio 17 2022" -A x64 -S ../Source -B ../BuildFiles
PAUSE
