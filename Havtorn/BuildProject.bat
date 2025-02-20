@echo off
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