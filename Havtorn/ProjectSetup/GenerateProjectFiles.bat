@echo off
call Premake\bin\premake5.exe vs2022
REM @echo.
REM @echo Checking license in files...

REM set fileExtentions=.cpp .h .hlsl .hlsli
REM set command=call %~dp0CheckFileForLicense.bat @path

REM for %%G in (%fileExtentions%) do (
REM	forfiles /p Source /s /m *%%G /C "cmd /c %command%"
REM )

@echo.