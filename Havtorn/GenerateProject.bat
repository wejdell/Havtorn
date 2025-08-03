@echo off
echo.
echo Generating CMake files...
echo.
cmake -D CMAKE_GENERATOR_INSTANCE:location="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" -G "Visual Studio 17 2022" -A x64 -S Source -B BuildFiles
echo.
PAUSE