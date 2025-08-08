@echo off
call ProjectSetup/PythonSetup.bat
echo.
echo Python Install complete with error level: %errorlevel%
PAUSE
echo.
echo Checking to see if cmake command works
python --version
py --version
PAUSE
python TestPythonPrint.py
echo.
cd %~dp0ProjectSetup\
python TestPythonPackages.py
PAUSE