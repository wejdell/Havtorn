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
goto :eof

:PYTHON_NOT_INSTALLED
echo.
echo Error^: Python not installed, install Python?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :INSTALL_PYTHON
goto :NO_INSTALL_PYTHON

:INSTALL_PYTHON
echo.
echo:Downloading Python
if not exist "SetupRequirements//" mkdir SetupRequirements
rem # get latest download url
rem $URL = "https://api.github.com/repos/microsoft/winget-cli/releases/latest"
rem $URL = (Invoke-WebRequest -Uri $URL).Content | ConvertFrom-Json |
rem        Select-Object -ExpandProperty "assets" |
rem        Where-Object "browser_download_url" -Match '.msixbundle' |
rem        Select-Object -ExpandProperty "browser_download_url"

rem # download
rem Invoke-WebRequest -Uri $URL -OutFile "Setup.msix" -UseBasicParsing

rem # install
rem Add-AppxPackage -Path "Setup.msix"

rem # delete file
rem Remove-Item "Setup.msix"

rem cmd .\python-3.13.5.exe InstallAllUsers=1 AssociateFiles=1 PrependPath=0 AppendPath=1
PAUSE
goto :eof

:NO_INSTALL_PYTHON
echo.
echo:Please install Python 3.13 or later to continue setup
PAUSE
goto :eof