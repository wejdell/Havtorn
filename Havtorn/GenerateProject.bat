@echo off
cd %~dp0\ProjectSetup\
call GenerateProjectFiles.bat
:: TODO: find a way to check if CompileDenepndencies needs to run: are there updates to External? Missing files?
call CompileDependencies.bat
