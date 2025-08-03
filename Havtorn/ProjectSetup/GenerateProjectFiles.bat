@echo off
cmake -G "Visual Studio 17 2022" -A x64 -S ../Source -B ../BuildFiles
PAUSE
REM @echo.
REM @echo Checking license in files...

REM set fileExtentions=.cpp .h .hlsl .hlsli
REM set command=call %~dp0CheckFileForLicense.bat @path

REM for %%G in (%fileExtentions%) do (
REM	forfiles /p Source /s /m *%%G /C "cmd /c %command%"
REM )

@echo.