@echo off
echo Generating CMake files...
cmake -G "Visual Studio 17 2022" -A x64 -S ../Source -B ../BuildFiles
PAUSE