@echo off
:: This script can be called by itself or from another script, always use absolute path %~dp0
call %~dp0SetVariablesForRequirements.bat

echo.
set gitVersion=2.50.1
set gitDownloadFile=PortableGit-%gitVersion%-64-bit.7z.exe
set gitUrl=https://github.com/git-for-windows/git/releases/download/v%gitVersion%.windows.1/%gitDownloadFile%
set gitExeLocation=%requirementsDirName%\%gitDirName%\bin\
set gitExe=git.exe

echo Running additional git setup...
echo Looking for git cli...
:: -v = version, >NUL 2>&1 = hides output of command
git -v >NUL 2>&1
if %errorlevel% NEQ 0 goto :GIT_NOT_FOUND
echo Found git
goto :GIT_RUN_COMMANDS

:GIT_SET_VARIABLE
:: Prepend to PATH, lasts for the lifetime of the process. Path must be full from system root (%~dp0).
:: Do not include the exectuable, PATH only wants the directory of the executable
set PATH=%~dp0%gitExeLocation%;%PATH%
goto :GIT_RUN_COMMANDS

:GIT_RUN_COMMANDS
echo Updating submodules
git submodule update --init --recursive
echo Enabling lfs
git lfs install
echo Enabling git longpaths 
git config core.longpaths true
goto :eof

:GIT_NOT_FOUND
echo.
echo Searching for git inside repository...
if not exist %~dp0%gitExeLocation%\ goto :GIT_INSTALL_PERMISSION
%~dp0%gitExeLocation%\%gitExe% -v >NUL 2>&1
if %errorlevel% NEQ 0 goto :GIT_INSTALL_PERMISSION
goto :GIT_SET_VARIABLE

:GIT_INSTALL_PERMISSION
echo.
echo Permission to download git for the command line?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :GIT_DO_INSTALL
goto :REQUIREMENT_ERROR_OUT

:GIT_DO_INSTALL
if not exist %~dp0%requirementsDirName% mkdir %~dp0%requirementsDirName%
if not exist %~dp0%requirementsDirName%\%gitDirName%\ mkdir %~dp0%requirementsDirName%\%gitDirName%\
echo Ready to start download %~dp0%requirementsDirName%\%gitDownloadFile% ...
bitsadmin /transfer gitDownload /download /priority high "%gitUrl%" "%~dp0%requirementsDirName%\%gitDownloadFile%"
echo Extracting to "%~dp0%requirementsDirName%\%gitDirName%" ...
%~dp0%requirementsDirName%\%gitDownloadFile% -y
del %~dp0%requirementsDirName%\%gitDownloadFile% /q
goto :GIT_SET_VARIABLE

:REQUIREMENT_ERROR_OUT
echo Warning^: Failed to find or install git for the command line, either allow setup to install or do a manual installation.
echo Additional git setup conists of updating submodules, initialising lfs and longpaths for git. 
echo If the additional steps have be done manually, continue? 
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :eof
EXIT /B 5

EXIT /B 0