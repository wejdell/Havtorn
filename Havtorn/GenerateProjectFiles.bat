@echo off
call External\premake\premake5.exe vs2022
@echo.
@echo Checking license in files...

set fileExtentions=.cpp .h .hlsl .hlsli
set command=call %~dp0CheckFileForLicense.bat @path

for %%G in (%fileExtentions%) do (
	forfiles /p Source /s /m *%%G /C "cmd /c %command%"
)

@echo.
@echo Done.
@echo.
PAUSE