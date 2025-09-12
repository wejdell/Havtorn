@echo off
:: This script can be called by itself or from another script, always use absolute path %~dp0
call %~dp0SetVariablesForRequirements.bat

call %~dp0VerifyBuildTools.bat
if %errorlevel% NEQ 0 goto :BT_INSTALL_PERMISSION
goto :eof

:BT_INSTALL_PERMISSION
echo.
echo Error^: Build Tools installation not found, permission to install?
choice /C yn /M "Yes/No?"
if %errorlevel% == 1 goto :BT_DO_INSTALL
goto :REQUIREMENT_ERROR_OUT

:BT_DO_INSTALL
call %~dp0InstallBuildTools.bat
goto :eof

:REQUIREMENT_ERROR_OUT
echo Error^: Failed to find or install Visual Studio Build Tools %btVer% with %btReq1%, either allow setup to install or do a manual installation.
PAUSE
EXIT /B 5

EXIT /B 0