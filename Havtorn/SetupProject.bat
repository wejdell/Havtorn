@echo off
call %~dp0\ProjectSetup\SetupGitAdditional.bat
if %errorlevel% NEQ 0 goto :ON_REQUIREMENT_ERROR
call %~dp0\ProjectSetup\SetupCMake.bat
if %errorlevel% NEQ 0 goto :ON_REQUIREMENT_ERROR
call %~dp0\ProjectSetup\SetupPython.bat
if %errorlevel% NEQ 0 goto :ON_REQUIREMENT_ERROR
call %~dp0\ProjectSetup\SetupBuildTools.bat
if %errorlevel% NEQ 0 goto :ON_REQUIREMENT_ERROR
goto :CONTINUE_SETUP

:ON_REQUIREMENT_ERROR
echo Error^: Requirement missing. Unable to complete setup.
PAUSE
goto :eof

:CONTINUE_SETUP
echo.
echo Requirements checked. Continuing with setup ...
echo.
cd %~dp0\ProjectSetup\
:: Figure out which cli for python we can use
python --version>nul 2>&1
set pythonCmdError=%errorlevel%
py --version>nul 2>&1
set pyCmdError=%errorlevel%
if %pythonCmdError% NEQ 0 (
    if %pyCmdError% NEQ 0 goto :ON_REQUIREMENT_ERROR
        py AdditionalConfigStart.py
        goto :COMPLETE
)
python AdditionalConfigStart.py

:COMPLETE
echo.
echo Project setup complete
PAUSE
