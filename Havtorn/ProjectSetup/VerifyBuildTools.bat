@echo off
setlocal enabledelayedexpansion

:: Move to guaranteed directory on C
cd /d "%ProgramFiles(x86)%"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
	cd /d "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\"
	:: "%%i" takes the output of the command in "in ()" and runs it, %* runs it with all startupargs (%1 .. %9)
	for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.Component.MSBuild -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find MSBuild\**\Bin\MSBuild.exe`) do (
    	::"%%i" %* :: runs it if we want to
    	echo Found Build Tools
    	exit /b 0
    )
) 
echo Failed to find Build Tools
exit /b 1
