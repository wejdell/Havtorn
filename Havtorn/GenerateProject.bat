@echo off
cd %~dp0\ProjectSetup\
GenerateProjectFiles.bat
CompileDependencies.bat
