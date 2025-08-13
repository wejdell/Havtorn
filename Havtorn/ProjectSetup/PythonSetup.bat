@echo off
:: This script can be called by itself or from another script, always use absolute path %~dp0
call %~dp0SetVariablesForRequirements.bat

:: TODO: change installer to local python installation through zip+configs
set pythonDownloadFile=python-%pythonVersion%-amd64.exe
set pythonUrl=https://www.python.org/ftp/python/%pythonVersion%/%pythonDownloadFile%
:: C:\Users\<username>\AppData\Roaming
:: LOCALAPPDATA == C:\Users\<user>\AppData\Local
:: This is the same directory the Python installer uses as its default. 
set pythonInstallDir=%LOCALAPPDATA%\Programs\Python\Python313

echo %pythonVersion%
echo %requirementsDirName%
echo %pythonDownloadFile%
echo %pythonUrl%
echo %pythonInstallDir%

echo Looking for Python...
:: TODO actually verify version of installed Python both for machine & repo
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

:PYTHON_SET_VARIABLE
:: Prepend to PATH, lasts for the lifetime of the process. Path must be full from system root (%~dp0).
:: Do not include the exectuable, PATH only wants the directory of the executable
:: Note: python installer is configured to set PATH, however to receive the updated PATH from system the setup-process would need to be restarted.
:: This is a hack of sorts to get around that, as the temporary modification takes immediate effect.
set pythonExeLocation=%pythonInstallDir%
echo %pythonExeLocation%
set pyLauncherLocation=%pythonInstallDir%\..\Launcher\
echo %pyLauncherLocation%
set pipLocation=%pythonInstallDir%\Scripts\
echo %pipLocation%
set PATH=%pythonExeLocation%;%pyLauncherLocation%;%pipLocation%;%PATH%
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
bitsadmin /transfer pythonDownload /download /priority high "%pythonUrl%" "%~dp0%requirementsDirName%\%pythonDownloadFile%"
echo Installing to %pythonInstallDir%. Please wait...
:: PrependPath=1: Windows (since 10 update #somethingsomething) has an exectuable linked in PATH named Python that opens the windows-store to install Python, conflicting with Python's exe
:: Prepending to PATH should execute Python's exe before Windows' allowing intended use use of python as cli. The windows-store association can be removed by users, using windows-store to install also removes it.
:: py cli should always work, as that is the python-launcher.
"%~dp0%requirementsDirName%\%pythonDownloadFile%" TargetDir=%pythonInstallDir% AssociateFiles=1 PrependPath=1 /wait /quiet
del %~dp0%requirementsDirName%\%pythonDownloadFile% /q
goto :PYTHON_SET_VARIABLE

:REQUIREMENT_ERROR_OUT
echo Error^: Failed to find or install Python (minimum version-%pythonVersion%), either allow setup to install or do a manual installation.
PAUSE
:: errorcode/errorlevel 5 => indication that the user is not authorized to access the resource
EXIT /B 5

EXIT /B 0