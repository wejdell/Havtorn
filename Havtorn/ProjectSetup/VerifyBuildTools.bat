@echo off
:: TODO extra verification needed, do we have the required version of VS & the correct packages installed
set file="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist %file% (
	echo "Found VS Build Tools"
	exit 0
) else (
	echo "Failed to find VS Build Tools"
	PAUSE
	exit 1
)
PAUSE

REM Save this in case we need to check for MSBuild
REM cd %ProgramFiles(x86)%\Microsoft Visual Studio\Installer\
REM 
REM for /f "usebackq tokens=*" %%i in (`vswhere -products * -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
REM   "%%i" %*
REM   exit /b !errorlevel!
REM )