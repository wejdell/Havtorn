@echo off
call %~dp0\SetupCMake.bat
if %errorlevel% NEQ 0 EXIT /B 1

if not exist %~dp0..\External\Lib\Debug\ mkdir -p %~dp0..\External\Lib\Debug\
if not exist %~dp0..\External\Lib\Release\ mkdir -p %~dp0..\External\Lib\Release\

cd ..\External\assimp
echo.
echo Generating assimp CMake files...
echo.
cmake -G "Visual Studio 17 2022" CMakeLists.txt
echo.
echo Building assimp...
echo.
cmake --build .
echo.
copy bin\Debug\assimp-vc143-mtd.dll ..\..\Bin\
copy lib\Debug\assimp-vc143-mtd.lib ..\Lib\Debug\
cd .. 

mkdir -p ..\External\Lib\Debug\PhysX\
mkdir -p ..\External\Lib\Release\PhysX\
cd PhysX\physx
echo.
echo Generating PhysX files...
echo.
:: if PM_PACKAGES_ROOT is not set, packman will be installed under C:/
:: packman init script will only read this path correctly if it is without ""
set PM_PACKAGES_ROOT=%~dp0SetupRequirements\packman\
if not exist %PM_PACKAGES_ROOT%\ mkdir %PM_PACKAGES_ROOT%
call generate_projects.bat vc17win64 
echo.
echo Building PhysX...
echo.
cmake --build compiler/vc17win64 
copy bin\win.x86_64.vc143.md\debug\PhysX_64.dll ..\..\..\Bin\
copy bin\win.x86_64.vc143.md\debug\PhysX_64.lib ..\..\Lib\Debug\PhysX\
copy bin\win.x86_64.vc143.md\debug\PhysXCommon_64.dll ..\..\..\Bin\
copy bin\win.x86_64.vc143.md\debug\PhysXCommon_64.lib ..\..\Lib\Debug\PhysX\
copy bin\win.x86_64.vc143.md\debug\PhysXExtensions_static_64.lib ..\..\Lib\Debug\PhysX\
copy bin\win.x86_64.vc143.md\debug\PhysXCooking_64.dll ..\..\..\Bin\
copy bin\win.x86_64.vc143.md\debug\PhysXCooking_64.lib ..\..\Lib\Debug\PhysX\
copy bin\win.x86_64.vc143.md\debug\PhysXDevice64.dll ..\..\..\Bin\
copy bin\win.x86_64.vc143.md\debug\PhysXFoundation_64.dll ..\..\..\Bin\
copy bin\win.x86_64.vc143.md\debug\PhysXFoundation_64.lib ..\..\Lib\Debug\PhysX\
copy bin\win.x86_64.vc143.md\debug\PhysXPvdSDK_static_64.lib ..\..\Lib\Debug\PhysX\
copy bin\win.x86_64.vc143.md\debug\PhysXCharacterKinematic_static_64.lib ..\..\Lib\Debug\PhysX\
copy bin\win.x86_64.vc143.md\debug\PhysXGpu_64.dll ..\..\..\Bin\
cd ..\..

cd box2d
echo.
rmdir /s /q build
mkdir build
cd build
echo.
echo Generating box2d files...
echo.
cmake -G "Visual Studio 17 2022" -DBOX2D_SAMPLES=OFF -DBOX2D_UNIT_TESTS=OFF -S ..
echo.
echo Building box2d...
echo.
cmake --build .
copy src\Debug\box2dd.lib ..\..\Lib\Debug\
cd ..\..

cd DirectXTex
echo.
echo Generating DirectXTex files...
echo.
cmake -G "Visual Studio 17 2022" CMakeLists.txt -DBUILD_TOOLS=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_SAMPLE=OFF
echo.
echo Building DirectXTex...
echo.
cmake --build . 
copy bin\Debug\DirectXTex.dll ..\..\Bin\
copy lib\Debug\DirectXTex.lib ..\Lib\Debug\
cd .. 

echo.
echo Dependencies compiled
PAUSE
