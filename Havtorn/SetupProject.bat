@echo off
call %~dp0\ProjectSetup\GitAdditionalSetup.bat
if %errorlevel% NEQ 0 goto :ON_REQUIREMENT_ERROR
call %~dp0\ProjectSetup\CMakeSetup.bat
if %errorlevel% NEQ 0 goto :ON_REQUIREMENT_ERROR
call %~dp0\ProjectSetup\PythonSetup.bat
if %errorlevel% NEQ 0 goto :ON_REQUIREMENT_ERROR
goto :CONTINUE_SETUP

:ON_REQUIREMENT_ERROR
echo Error^: Requirement missing. Unable to complete setup.
PAUSE
goto :eof

:CONTINUE_SETUP
cd %~dp0\ProjectSetup\
python --version>nul 2>&1
set pythonCmdError=%errorlevel%
py --version>nul 2>&1
set pyCmdError=%errorlevel%
if %pythonCmdError% NEQ 0 (
    py MasterSetup.py
) else (
    python MasterSetup.py
)
goto :eof
