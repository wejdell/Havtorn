@echo off
echo.
REM echo Generating Assimp CMake files...
REM echo.
REM cmake -S ../External/Source -B BuildFiles
REM echo.
REM echo Building Assimp...
REM echo.
REM cmake --build BuildFiles
REM echo.
echo Generating PhysX files...
echo.
call ../External/PhysX/physx/generate_projects.bat vc17win64 
echo.
echo Building PhysX...
echo.
cmake --build ../External/PhysX/physx/compiler/vc17win64 
echo.