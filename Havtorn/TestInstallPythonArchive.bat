@echo off
if exist "SetupRequirements/Python-3.13.5//" goto :PYTHON_SET_LOCALEXE
if not exist "SetupRequirements//" mkdir SetupRequirements
set downloadDir=SetupRequirements//
set extractedDirectory=SetupRequirements//
set pythonArchiveUrl=https://www.python.org/ftp/python/3.13.5/Python-3.13.5.tgz

:: Download Python installer 
echo Downloading to %downloadDir%
PAUSE
bitsadmin /transfer pythonDownload /download /priority high "%pythonArchiveUrl%" "%~dp0%downloadDir%\Python-3.13.5.tgz"

:: x - extract, v - verbose, f - target archive, C - extraction directory
tar -xvf "%~dp0%downloadDir%\Python-3.13.5.tgz" -C "%~dp0%extractedDirectory%"
PAUSE

:PYTHON_SET_LOCALEXE
set py