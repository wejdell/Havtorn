@echo off
:: This script can be called by itself or from another script, always use absolute path %~dp0
call %~dp0SetVariablesForRequirements.bat

set pythonDownloadFile=python-%pythonVersion%-embed-amd64.zip
set pythonUrl=https://www.python.org/ftp/python/%pythonVersion%/%pythonDownloadFile%
set pythonExeLocation=%requirementsDirName%\%pythonDirName%\
set pipExeLocation=%requirementsDirName%\%pythonDirName%\Scripts\
set pythonExe=python.exe
set pyExe=py.exe
:: might need this too?
:: set pythonLauncherLocation=
:: set pythonLauncher=

:: USER
:: C:\Users\AkiGonzalez\AppData\Local\Programs\Python\Python313\Scripts\
:: C:\Users\AkiGonzalez\AppData\Local\Programs\Python\Python313\
:: C:\Users\AkiGonzalez\AppData\Local\Programs\Python\Launcher\
:: SYSTEM
::
:: https://www.python.org/downloads/release/python-3135/
:: a bit confused what to use
:: install folder: C:\Users\AkiGonzalez\AppData\Local\Programs\Python\Python313

echo %pythonVersion%
echo %requirementsDirName%
echo %pythonDirName%
echo %pythonDownloadFile%
echo %pythonExeLocation%
echo %pythonExe%

echo Looking for Python...
:: TODO actually verify version of installed Python both for machine & repo check
python --version >NUL 2>&1
if %errorlevel% NEQ 0 goto :PYTHON_CHECK_REPO_INSTALL
echo Python found
PAUSE
goto :eof

:PYTHON_CHECK_REPO_INSTALL
if not exist %~dp0%pythonExeLocation%%pythonExe% goto :PYTHON_INSTALL_PERMISSION
%~dp0%pythonExeLocation%%pythonExe% --version >NUL 2>&1
if %errorlevel% NEQ 0 goto :PYTHON_INSTALL_PERMISSION
echo Python found in repository
goto :PYTHON_SET_VARIABLE

:PYTHON_SET_VARIABLE
:: Prepend to PATH, lasts for the lifetime of the process. Path must be full from system root (%~dp0).
:: Do not include the exectuable, PATH only wants the directory of the executable
:: python command will work, py command won't TODO? figure out how to make py command work
set PATH=%~dp0%pythonExeLocation%;%PATH%
set PATH=%~dp0%pipExeLocation%;%PATH%
:: make sure pip is installed and available in PATH, --no-warn-script-location suppresses a warning about python not being in PATH
::python %~dp0get-pip.py
::python exit()
python -m pip install --upgrade pip
::python -m pip313
::python -m ensurepip
::python -m pip install --upgrade pip setuptools wheel
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
if not exist %~dp0%requirementsDirName%\%pythonDirName%\ mkdir %~dp0%requirementsDirName%\%pythonDirName%\
if not exist %~dp0%requirementsDirName%\%pythonDownloadFile% (
    echo Ready to start download %~dp0%requirementsDirName%\%pythonDownloadFile%
    PAUSE
    bitsadmin /transfer pythonDownload /download /priority high "%pythonUrl%" "%~dp0%requirementsDirName%\%pythonDownloadFile%"
)
echo Extracting to "%~dp0%requirementsDirName%\%pythonDirName%\"
:: x - extract, v - verbose, f - target archive, C - extract to directory
tar -xvf "%~dp0%requirementsDirName%\%pythonDownloadFile%" -C "%~dp0%requirementsDirName%\%pythonDirName%\\"
:: Modify python313._pth adding these 2: <..\.., Scripts> on separate lines
PAUSE
:: extract python313.zip as well?
goto :PYTHON_SET_VARIABLE

:REQUIREMENT_ERROR_OUT
echo Error^: Failed to find or install Python (minimum version-%pythonVersion%), either allow setup to install or do a manual installation.
PAUSE
EXIT /B 5

EXIT /B 0