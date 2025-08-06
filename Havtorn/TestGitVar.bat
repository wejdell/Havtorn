@echo off
::for /f "usebackq tokens=2,*" %%A in (`reg query HKCU\Environment /v PATH`) do set my_user_path=%%B
::echo %my_user_path%
::setx PATH "%~dp0SetupRequirements\PortableGit\bin\;%my_user_path%"
::set PATH="%~dp0SetupRequirements\PortableGit\bin\;%my_user_path%"
:: Modify PATH only for the lifetime of the current process
set PATH=%~dp0SetupRequirements\PortableGit\bin\;%PATH%
git -v
PAUSE