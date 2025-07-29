@echo off
:: Check for CMake installation
:: Check through PATH
@echo off
@ cmake --version 3>NUL
if %errorlevel% == 0 goto :CMAKE_INSTALLED
goto :CMAKE_NOT_INSTALLED

:CMAKE_NOT_INSTALLED
:: check if /req-dir/ exists -> yes, continue; no goto CMAKE_INSTALL
:: check if /req-dir/CMake/?/?/cmake.exe --version -> yes, goto CMAKE_SET_VARIABLE; no goto CMAKE_INSTALL
if not exist "SetupRequirements//" goto :CMAKE_INSTALL_PERMISSION
if not exist "SetupRequirements/CMake//" goto :CMAKE_INSTALL_PERMISSION
:: Can't validate this check currently, without install: run CMake.exe from a custom path
goto :CMAKE_INSTALL_PERMISSION

:CMAKE_INSTALLED
echo CMake is installed, found through PATH
PAUSE
goto :eof

:CMAKE_INSTALL_PERMISSION
:: ask permission -> yes, continue; no, goto REQUIREMENT_ERROR_OUT
echo.
echo Error^: CMake installation not found, install CMake?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :CMAKE_INSTALL
goto :REQUIREMENT_ERROR_OUT

:CMAKE_INSTALL
if not exist "SetupRequirements//" mkdir SetupRequirements
if not exist "SetupRequirements//CMake//" mkdir SetupRequirements\CMake
set cmakeVersion=3.31.4
set downloadDir=SetupRequirements//
set extractedDirectory=SetupRequirements//CMake//
set cmakeZipUrls=https://github.com/Kitware/CMake/releases/download/v%cmakeVersion%/cmake-%cmakeVersion%-windows-x86_64.zip

echo %cmakeVersion%
echo %downloadDir%
echo %extractedDirectory%
echo %cmakeZipUrls%
echo "%~dp0%downloadDir%\cmake-%cmakeVersion%-windows-x86_64.zip"
echo "%~dp0%extractedDirectory%"
PAUSE
:: Download Python installer 
::echo:Downloading to "%~dp0%downloadDir%\cmake-%cmakeVersion%-windows-x86_64.zip"
::PAUSE
::bitsadmin /transfer cmakeDownload /download /priority high "%cmakeZipUrls%" "%~dp0%downloadDir%\cmake-%cmakeVersion%-windows-x86_64.zip"
::PAUSE
rem Could do silent/quiet install? e.g do not show installer window
:: x - extract, v - verbose, f - target archive, C - extraction directory
tar -xvf "%~dp0%downloadDir%\cmake-%cmakeVersion%-windows-x86_64.zip" -C "%~dp0%extractedDirectory%"
PAUSE
:: create /req-dir/ if possible
:: download cmake zip to /req-dir/
:: extract cmake zip to /req-dir/CMake/
:: goto CMAKE_SET_VARIABLE

:CMAKE_SET_VARIABLE
:: set cmake=/req-dir/CMake/cmake.exe
goto :eof

:REQUIREMENT_ERROR_OUT
:: echo Failed to install setup requirement, please allow install or install requirement manually
echo Error^: Failed to find or install requirement, either allow setup to install or do a manual installation.
PAUSE