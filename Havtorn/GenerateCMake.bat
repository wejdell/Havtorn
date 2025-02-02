@echo off
pushd %~dp0\ProjectSetup\CMake\cmake-3.31.4-windows-x86_64\bin
cmake -S ../../../../Source -B ../../../../BuildFiles
PAUSE