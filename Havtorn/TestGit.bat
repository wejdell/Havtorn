@echo off
:: check git command line
:: if not found offer to install
:: download installer for windows to /req-dir/
:: run silent with settings from havtorn_git_install.ini
:: submodule init
:: lfs init
:: longpath init
echo Git Submodule Requirement Step 
echo Looking for git cli availability
:: -v = version, >NUL 2>&1 = hides output of command
git -v >NUL 2>&1
if %errorlevel% NEQ 0 goto :GIT_CLI_NOT_AVAILABLE
goto :GIT_CLI_AVAILABLE

:GIT_CLI_AVAILABLE
echo git command available
echo.
echo updating submodules
git submodule update --init --recursive
echo.
echo enabling lfs
git lfs install
echo.
echo configure longpaths 
git config core.longpaths true
echo.
PAUSE
goto :eof

:GIT_CLI_NOT_AVAILABLE
echo.
echo WARNING^: git is not available through the command line, either update the submodules manually before proceeding or install the git command line to allow automatic updates
echo Would you like to install git for the command line?
echo give an option here for y/n and then download and run the installer https://github.com/git-for-windows/git/releases/download/v2.50.1.windows.1/Git-2.50.1-64-bit.exe
echo.
PAUSE
goto :eof

:GIT_RUN_INSTALL
:: check if /req-dir/ exists -> yes, continue; no goto CMAKE_INSTALL
:: check if /req-dir/CMake/?/?/cmake.exe --version -> yes, goto CMAKE_SET_VARIABLE; no goto CMAKE_INSTALL
if not exist "SetupRequirements/CMake/cmake-%cmakeVersion%-windows-x86_64/bin//" goto :CMAKE_INSTALL_PERMISSION
echo Local CMake directory found
echo %~dp0SetupRequirements\CMake\cmake-%cmakeVersion%-windows-x86_64\bin\cmake.exe
:: cmake-3.31.4-windows-x86_64\bin
:: Can't validate this check currently, without install: run CMake.exe from a custom path
%~dp0SetupRequirements/CMake/cmake-%cmakeVersion%-windows-x86_64/bin/cmake.exe --version 3>NUL
if %errorlevel% == 0 goto :CMAKE_SET_VARIABLE
goto :CMAKE_INSTALL_PERMISSION

:GIT_PERMISSION
:: ask permission -> yes, continue; no, goto REQUIREMENT_ERROR_OUT
echo.
echo Error^: CMake installation not found, install CMake?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :CMAKE_INSTALL
goto :REQUIREMENT_ERROR_OUT

:GIT_DOWNLOAD
if not exist "SetupRequirements//" mkdir SetupRequirements
set downloadDir=SetupRequirements//
set extractedDirectory=SetupRequirements//CMake//
set cmakeZipUrls=https://github.com/Kitware/CMake/releases/download/v%cmakeVersion%/cmake-%cmakeVersion%-windows-x86_64.zip

:: Download Python installer 
echo:Downloading to "%~dp0%downloadDir%\cmake-%cmakeVersion%-windows-x86_64.zip" and then extracting to "%~dp0%extractedDirectory%"
PAUSE
bitsadmin /transfer cmakeDownload /download /priority high "%cmakeZipUrls%" "%~dp0%downloadDir%\cmake-%cmakeVersion%-windows-x86_64.zip"

:: x - extract, v - verbose, f - target archive, C - extraction directory
tar -xvf "%~dp0%downloadDir%\cmake-%cmakeVersion%-windows-x86_64.zip" -C "%~dp0%extractedDirectory%"
PAUSE
