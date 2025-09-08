@echo off
setlocal enabledelayedexpansion

:: "%%i" takes the output of the command in "in ()" and runs it, %* runs it with all startupargs (%1 .. %9)
cd /d "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\"
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.Component.MSBuild -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find MSBuild\**\Bin\MSBuild.exe`) do (
    "%%i" %*
    exit /b 0
    )
:: Failed to find Build Tools with requirements
exit /b 1