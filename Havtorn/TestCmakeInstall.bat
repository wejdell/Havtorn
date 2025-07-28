@echo off
:: Check for CMake installation
:: Check through PATH
@ cmake --version 3>NUL
if %errorlevel% == 0 goto :CMAKE_INSTALLED
goto :CMAKE_NOT_INSTALLED

:CMAKE_NOT_INSTALLED
:: check if /req-dir/ exists -> yes, continue; no goto CMAKE_INSTALL
:: check if /req-dir/CMake/?/?/cmake.exe --version -> yes, goto CMAKE_SET_VARIABLE; no goto CMAKE_INSTALL
echo CMake is not installed
PAUSE
goto :eof

:CMAKE_INSTALLED
echo CMake is installed, found through PATH
PAUSE
goto :eof

:CMAKE_INSTALL
:: ask permission -> yes, continue; no, goto REQUIREMENT_ERROR_OUT
:: create /req-dir/ if possible
:: download cmake zip to /req-dir/
:: extract cmake zip to /req-dir/CMake/
:: goto CMAKE_SET_VARIABLE

:CMAKE_SET_VARIABLE
:: set cmake=/req-dir/CMake/cmake.exe

:REQUIREMENT_ERROR_OUT
:: echo Failed to install setup requirement, please allow install or install requirement manually