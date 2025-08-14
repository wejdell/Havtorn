@echo off
cd %~dp0\ProjectSetup\
GenerateProjectFiles.bat
CompileDependencies.bat
::cmake -G "Visual Studio 17 2022" -A x64 -S Source -B BuildFiles
::cmake -D CMAKE_GENERATOR_INSTANCE:location="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" -G "Visual Studio 17 2022" -A x64 -S Source -B BuildFiles