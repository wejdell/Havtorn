@echo off
cd %~dp0\ProjectSetup\
call GenerateProjectFiles.bat
call CompileDependencies.bat
