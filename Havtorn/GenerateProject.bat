@echo off
echo.
echo Generating CMake files...
echo.
cmake -G "Visual Studio 17 2022" -A x64 -S Source -B BuildFiles
echo.
PAUSE