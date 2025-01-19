@echo off

set currentPath=%1
set licenseFile=%~dp0License.txt
set tempLicenseFile=license.txt.new

findstr /C:"Team Havtorn. All Rights Reserved." %currentPath% >nul 2>&1

if %errorlevel% equ 1 (
REM Write the contents of the license file to a temp file
	type %licenseFile% > %tempLicenseFile% 
REM Append the contents of the code file to the temp file
	type %currentPath% >> %tempLicenseFile%
REM Overwrite the code file with the temp file	
	move /y %tempLicenseFile% %currentPath%
)

