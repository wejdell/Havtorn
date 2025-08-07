@echo off
:: call ProjectSetup/SetVariablesForRequirements.bat <-- sets %reqdir = SetupRequirements/ cmakeversion, etc for Python and all that
:: TODO actually verify version of installed CMake both for machine & repo check
call ProjectSetup/SetVariablesForRequirements.bat

set cmakeDownloadFile=cmake-%cmakeVersion%-windows-x86_64.zip
set cmakeUrl=https://github.com/Kitware/CMake/releases/download/v%cmakeVersion%/%cmakeDownloadFile%
set cmakeExeLocation=%requirementsDirName%\%cmakeDirName%\cmake-%cmakeVersion%-windows-x86_64\bin\
set cmakeExe=cmake.exe

echo %cmakeVersion%
echo %requirementsDirName%
echo %cmakeDirName%
echo %cmakeDownloadFile%
echo %cmakeExeLocation%
echo %cmakeExe%

cmake --version >NUL 2>&1
if %errorlevel% NEQ 0 goto :CMAKE_CHECK_REPO_INSTALL
echo CMake found
PAUSE
goto :eof

:CMAKE_CHECK_REPO_INSTALL
if not exist %cmakeExeLocation%%cmakeExe% goto :CMAKE_INSTALL_PERMISSION
%cmakeExeLocation%%cmakeExe% --version >NUL 2>&1
if %errorlevel% NEQ 0 goto :CMAKE_INSTALL_PERMISSION
echo CMake found in repository
goto :CMAKE_SET_VARIABLE

:CMAKE_SET_VARIABLE
:: Prepend to PATH, lasts for the lifetime of the process. Path must be full from system root (%~dp0).
:: Do not include the exectuable, PATH only wants the directory of the executable
set PATH=%~dp0%cmakeExeLocation%;%PATH%
goto :eof

:CMAKE_INSTALL_PERMISSION
echo.
echo Error^: CMake installation not found, permission to download CMake?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :CMAKE_DO_INSTALL
goto :REQUIREMENT_ERROR_OUT

:CMAKE_DO_INSTALL
if not exist %requirementsDirName% mkdir %requirementsDirName%
if not exist %requirementsDirName%\%cmakeDirName%\ mkdir %requirementsDirName%\%cmakeDirName%\
echo %~dp0%requirementsDirName%\%cmakeDownloadFile%
if not exist %requirementsDirName%\%cmakeDownloadFile% (
    echo Ready to start download %~dp0%requirementsDirName%\%cmakeDownloadFile%
    PAUSE
    bitsadmin /transfer cmakeDownload /download /priority high "%cmakeUrl%" "%~dp0%requirementsDirName%\%cmakeDownloadFile%"
)
echo Extracting to "%~dp0%requirementsDirName%\%cmakeDirName%\"
:: x - extract, v - verbose, f - target archive, C - extract to directory
tar -xvf "%~dp0%requirementsDirName%\%cmakeDownloadFile%" -C "%~dp0%requirementsDirName%\%cmakeDirName%\\"
PAUSE
goto :CMAKE_SET_VARIABLE

:REQUIREMENT_ERROR_OUT
echo Error^: Failed to find or install CMake (minimum version-%cmakeVersion%), either allow setup to install or do a manual installation.
PAUSE
EXIT /B 5

EXIT /B 0