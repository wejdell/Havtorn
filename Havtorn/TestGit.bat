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
::git -v >NUL 2>&1
::if %errorlevel% NEQ 0 goto :GIT_CLI_NOT_AVAILABLE
::goto :GIT_CLI_AVAILABLE

goto :GIT_CLI_NOT_AVAILABLE

:GIT_SET_LOCALEXE
set git=SetupRequirements\PortableGit\bin\git.exe
goto :GIT_CLI_AVAILABLE

:GIT_CLI_AVAILABLE
echo git command available
echo.
echo updating submodules
%git% submodule update --init --recursive
echo.
echo enabling lfs
%git% lfs install
echo.
echo configure git longpaths 
%git% config core.longpaths true
echo.
PAUSE
goto :eof

:GIT_CLI_NOT_AVAILABLE
echo.
echo Git for the command line not found 
echo Searching for local portable git...
if not exist "SetupRequirements/PortableGit/bin//" goto :GIT_INSTALL_PERMISSION
echo Searching for executable...
SetupRequirements\PortableGit\bin\git.exe -v >NUL 2>&1
if %errorlevel% NEQ 0 goto :GIT_INSTALL_PERMISSION
goto :GIT_SET_LOCALEXE

:GIT_INSTALL_PERMISSION
echo.
echo Permission to download git for the command line?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :GIT_DOWNLOAD
goto :REQUIREMENT_ERROR_OUT

:GIT_DOWNLOAD
if not exist "SetupRequirements//" mkdir SetupRequirements
set downloadDir=SetupRequirements//
set extractedDirectory=SetupRequirements//
set gitPortableUrl=https://github.com/git-for-windows/git/releases/download/v2.50.1.windows.1/PortableGit-2.50.1-64-bit.7z.exe

::echo Downloading to "%~dp0%downloadDir%\"...
::PAUSE
::bitsadmin /transfer gitDownload /download /priority high "%gitPortableUrl%" "%~dp0%downloadDir%\PortableGit-2.50.1-64-bit.7z.exe"

echo Extracting to "%~dp0%extractedDirectory%\PortableGit" ...
SetupRequirements\PortableGit-2.50.1-64-bit.7z.exe -y
:: x - extract, v - verbose, f - target archive, C - extraction directory
::tar -xvf "%~dp0%downloadDir%\cmake-%cmakeVersion%-windows-x86_64.zip" -C "%~dp0%extractedDirectory%"
echo Git portable download complete, continuing with setup
PAUSE
goto :GIT_SET_LOCALEXE
