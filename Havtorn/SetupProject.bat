@echo off
:: Check for Python installation
python --version 3<NUL
if %errorlevel% == 0 goto :PYTHON_INSTALLED
goto :PYTHON_NOT_INSTALLED

:PYTHON_INSTALLED
echo.
echo:Python installed, running MasterSetup
cd %~dp0\ProjectSetup\
python MasterSetup.py
PAUSE
goto :eof

:PYTHON_NOT_INSTALLED
echo.
echo Error^: Python not installed, install Python?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :INSTALL_PYTHON
goto :NO_INSTALL_PYTHON

:INSTALL_PYTHON
echo.
if not exist "SetupRequirements//" mkdir SetupRequirements
set "pythonVersion=3.13.5" 
set "pythonDownloadUrl=https://www.python.org/ftp/python/%pythonVersion%/python-%pythonVersion%-amd64.exe" 
  
rem Define the installation directory 
set "downloadDir=SetupRequirements"
set "installDir=SetupRequirements" 

rem rem Download Python installer 
rem rem bitsadmin.exe /transfer "PythonInstaller" "%pythonDownloadUrl%" "SetupRequirements\python-%pythonVersion%-amd64.exe" 
rem echo:Downloading to "%~dp0%downloadDir%\installer-python-%pythonVersion%-amd64.exe"
rem PAUSE
rem bitsadmin /transfer pythonDownload /download /priority high "%pythonDownloadUrl%" "%~dp0%downloadDir%\installer-python-%pythonVersion%-amd64.exe"

if not exist "SetupRequirements//Python//" mkdir SetupRequirements\Python
rem Could do silent/quiet install? e.g do not show installer window
"%~dp0%downloadDir%\installer-python-%pythonVersion%-amd64.exe" InstallAllUsers=0 TargetDir="%~dp0SetupRequirements\Python" AssociateFiles=1 PrependPath=1 AppendPath=0 DefaultPath=%installDir% /wait 
  
rem Clean up 
rem del "%TEMP%\python-installer.exe" /f /q 

rem cmd .\python-3.13.5.exe InstallAllUsers=1 AssociateFiles=1 PrependPath=0 AppendPath=1
PAUSE
goto :eof

:NO_INSTALL_PYTHON
echo.
echo:Please install Python 3.13 or later to continue setup
PAUSE
goto :eof