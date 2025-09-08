@echo off
:: https://learn.microsoft.com/sv-se/visualstudio/install/workload-and-component-ids?view=vs-2022
:: https://github.com/microsoft/vswhere/wiki/Find-MSBuild

:: COMMAND "${_vswhere_tool}" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
:: "-products * -latest -requires Microsoft.Component.MSBuild"
:: useful https://ss64.com/nt/call.html
::START /wait /B "" "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" " -latest -products * -requires Microsoft.Component.MSBuild"
setlocal enabledelayedexpansion

:: "%%i" takes the output of the command in "in ()" and runs it, %* runs it with all startupargs (%1 .. %9)
cd /d "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\"
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.Component.MSBuild -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find MSBuild\**\Bin\MSBuild.exe`) do (
    "%%i" %*
    exit /b 0
    )
  ::exit /b !errorlevel!
::)
exit /b 1

:: neither of these set errorLevel
:: latest? need?
::CALL "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
::echo.
::echo errorLevel %errorLevel%
::echo.

:: CALL "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
:: echo.
:: echo errorLevel %errorLevel%
:: echo.
:: FOR /F "" %%i in ('START /wait /B "" "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"') DO echo %%i
:: echo result is %variable%
PAUSE