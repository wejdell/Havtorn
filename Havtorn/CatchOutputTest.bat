@echo off
:: COMMAND "${_vswhere_tool}" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
:: "-products * -latest -requires Microsoft.Component.MSBuild"
:: useful https://ss64.com/nt/call.html
::START /wait /B "" "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" " -latest -products * -requires Microsoft.Component.MSBuild"

:: neither of these set errorLevel
:: latest? need?
CALL "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild
echo.
echo errorLevel %errorLevel%
echo.
CALL "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
echo.
echo errorLevel %errorLevel%
echo.
rem FOR /F "" %%i in ('START /wait /B "" "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"') DO echo %%i
rem echo result is %variable%
PAUSE