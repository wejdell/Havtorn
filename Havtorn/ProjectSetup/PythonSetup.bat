@echo off
:: This script can be called by itself or from another script, always use absolute path %~dp0
call %~dp0SetVariablesForRequirements.bat

set pythonDownloadFile=python-%pythonVersion%-amd64.exe
set pythonUrl=https://www.python.org/ftp/python/%pythonVersion%/%pythonDownloadFile%
:: C:\Users\<username>\AppData\Roaming
set pythonInstallDir=%AllUsersProfile%\Python

echo %pythonVersion%
echo %requirementsDirName%
echo %pythonDownloadFile%
echo %pythonUrl%
echo %pythonInstallDir%

echo Looking for Python...
:: TODO actually verify version of installed Python both for machine & repo
:: Test py 
python --version>nul 2>&1
set pythonCmdError=%errorlevel%
py --version>nul 2>&1
set pyCmdError=%errorlevel%
if %pythonCmdError% NEQ 0 (
    if %pyCmdError% NEQ 0 goto :PYTHON_INSTALL_PERMISSION
)
echo Python found
PAUSE
goto :eof

:PYTHON_INSTALL_PERMISSION
echo.
echo Error^: Python installation not found, permission to download?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :PYTHON_DO_INSTALL
goto :REQUIREMENT_ERROR_OUT

:PYTHON_DO_INSTALL
if not exist %~dp0%requirementsDirName% mkdir %~dp0%requirementsDirName%
::bitsadmin /transfer pythonDownload /download /priority high "%pythonUrl%" "%~dp0%requirementsDirName%\%pythonDownloadFile%"
::echo Installing to %pythonInstallDir%
rem Could do silent/quiet install? e.g do not show installer window
:: DefaultPath=%pythonInstallDir%
:: PrependPath=1 & InstallAllUsers=1 : Windows (since 10) has an exectuable linked in PATH named Python that opens the windows-store to install Python, conflicting with Python's exe
:: Prepending to PATH and installing to users places Python's exe before Windows'. Allowing intended use.
:: TargetDir= using default that is matched to InstallAllUsers
:: InstallAllUsers=1
"%~dp0%requirementsDirName%\%pythonDownloadFile%" AssociateFiles=1 PrependPath=1 /wait
:: del %~dp0%requirementsDirName%\%pythonDownloadFile% /q
::PAUSE
:: delete installer
goto :eof

:REQUIREMENT_ERROR_OUT
echo Error^: Failed to find or install Python (minimum version-%pythonVersion%), either allow setup to install or do a manual installation.
PAUSE
:: errorcode/errorlevel 5 => indication that the user is not authorized to access the resource
EXIT /B 5

EXIT /B 0