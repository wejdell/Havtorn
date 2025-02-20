@echo off
REM setlocal enabledelayedexpansion

if exist %ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe 
(
	exit 0
)
else 
(
	exit 1
)

REM Save this in case we need to check for MSBuild
REM cd %ProgramFiles(x86)%\Microsoft Visual Studio\Installer\
REM 
REM for /f "usebackq tokens=*" %%i in (`vswhere -products * -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
REM   "%%i" %*
REM   exit /b !errorlevel!
REM )