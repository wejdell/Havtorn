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
echo git command available, proceeding with submodule update
echo.
git submodule update --init --recursive
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
