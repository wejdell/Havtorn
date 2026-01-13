@echo off
call %~dp0\SetupCMake.bat
if %errorlevel% NEQ 0 EXIT /B 1
call %~dp0\SetupBuildTools.bat
if %errorlevel% NEQ 0 EXIT /B 1

if not exist %~dp0..\Bin\EditorDebug\ mkdir -p %~dp0..\Bin\EditorDebug\
if not exist %~dp0..\Bin\EditorDevelopment\ mkdir -p %~dp0..\Bin\EditorDevelopment\
if not exist %~dp0..\Bin\GameDebug\ mkdir -p %~dp0..\Bin\GameDebug\
if not exist %~dp0..\Bin\GameRelease\ mkdir -p %~dp0..\Bin\GameRelease\

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
copy /y lib\Debug\assimp-vc143-mtd.lib ..\Lib\Debug\
copy /y bin\Debug\assimp-vc143-mtd.dll ..\..\Bin\assimp-vc143-mtd.dll
echo.
cmake --build . --config Release
echo.
copy /y lib\Release\assimp-vc143-mt.lib ..\Lib\Release\
copy /y bin\Release\assimp-vc143-mt.dll ..\..\Bin\assimp-vc143-mt.dll

:: Add extra DLLs in DeepLink exe path
:: TODO: Figure out how to do deep link without going into exe folder, or figure out how to load dll:s in code
copy /y bin\Release\assimp-vc143-mt.dll ..\..\Bin\EditorDevelopment\assimp-vc143-mt.dll
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
copy /y bin\win.x86_64.vc143.md\debug\PhysX_64.dll ..\..\..\Bin\PhysX_64d.dll
copy /y bin\win.x86_64.vc143.md\debug\PhysX_64.lib ..\..\Lib\Debug\PhysX\PhysX_64d.lib
copy /y bin\win.x86_64.vc143.md\debug\PhysXCommon_64.dll ..\..\..\Bin\PhysXCommon_64d.dll
copy /y bin\win.x86_64.vc143.md\debug\PhysXCommon_64.lib ..\..\Lib\Debug\PhysX\PhysXCommon_64d.lib
copy /y bin\win.x86_64.vc143.md\debug\PhysXExtensions_static_64.lib ..\..\Lib\Debug\PhysX\PhysXExtensions_static_64d.lib
copy /y bin\win.x86_64.vc143.md\debug\PhysXCooking_64.dll ..\..\..\Bin\PhysXCooking_64d.dll
copy /y bin\win.x86_64.vc143.md\debug\PhysXCooking_64.lib ..\..\Lib\Debug\PhysX\PhysXCooking_64d.lib
copy /y bin\win.x86_64.vc143.md\debug\PhysXDevice64.dll ..\..\..\Bin\PhysXDevice64d.dll
copy /y bin\win.x86_64.vc143.md\debug\PhysXFoundation_64.dll ..\..\..\Bin\PhysXFoundation_64d.dll
copy /y bin\win.x86_64.vc143.md\debug\PhysXFoundation_64.lib ..\..\Lib\Debug\PhysX\PhysXFoundation_64d.lib
copy /y bin\win.x86_64.vc143.md\debug\PhysXPvdSDK_static_64.lib ..\..\Lib\Debug\PhysX\PhysXPvdSDK_static_64d.lib
copy /y bin\win.x86_64.vc143.md\debug\PhysXCharacterKinematic_static_64.lib ..\..\Lib\Debug\PhysX\PhysXCharacterKinematic_static_64d.lib
copy /y bin\win.x86_64.vc143.md\debug\PhysXGpu_64.dll ..\..\..\Bin\PhysXGpu_64d.dll
echo.
call generate_projects.bat vc17win64r

echo.
cmake --build compiler/vc17win64r --config Release

copy /y bin\win.x86_64.vc143.md\release\PhysX_64.dll ..\..\..\Bin\PhysX_64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysX_64.lib ..\..\Lib\Release\PhysX\PhysX_64.lib
copy /y bin\win.x86_64.vc143.md\release\PhysXCommon_64.dll ..\..\..\Bin\PhysXCommon_64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXCommon_64.lib ..\..\Lib\Release\PhysX\PhysXCommon_64.lib
copy /y bin\win.x86_64.vc143.md\release\PhysXExtensions_static_64.lib ..\..\Lib\Release\PhysX\PhysXExtensions_static_64.lib
copy /y bin\win.x86_64.vc143.md\release\PhysXCooking_64.dll ..\..\..\Bin\PhysXCooking_64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXCooking_64.lib ..\..\Lib\Release\PhysX\PhysXCooking_64.lib
copy /y bin\win.x86_64.vc143.md\release\PhysXDevice64.dll ..\..\..\Bin\PhysXDevice64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXFoundation_64.dll ..\..\..\Bin\PhysXFoundation_64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXFoundation_64.lib ..\..\Lib\Release\PhysX\PhysXFoundation_64.lib
copy /y bin\win.x86_64.vc143.md\release\PhysXPvdSDK_static_64.lib ..\..\Lib\Release\PhysX\PhysXPvdSDK_static_64.lib
copy /y bin\win.x86_64.vc143.md\release\PhysXCharacterKinematic_static_64.lib ..\..\Lib\Release\PhysX\PhysXCharacterKinematic_static_64.lib
copy /y bin\win.x86_64.vc143.md\release\PhysXGpu_64.dll ..\..\..\Bin\PhysXGpu_64.dll

:: Add extra DLLs in DeepLink exe path
:: TODO: Figure out how to do deep link without going into exe folder, or figure out how to load dll:s in code
copy /y bin\win.x86_64.vc143.md\release\PhysX_64.dll ..\..\..\Bin\EditorDevelopment\PhysX_64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXCommon_64.dll ..\..\..\Bin\EditorDevelopment\PhysXCommon_64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXCooking_64.dll ..\..\..\Bin\EditorDevelopment\PhysXCooking_64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXDevice64.dll ..\..\..\Bin\EditorDevelopment\PhysXDevice64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXFoundation_64.dll ..\..\..\Bin\EditorDevelopment\PhysXFoundation_64.dll
copy /y bin\win.x86_64.vc143.md\release\PhysXGpu_64.dll ..\..\..\Bin\EditorDevelopment\PhysXGpu_64.dll

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
copy /y src\Debug\box2dd.lib ..\..\Lib\Debug\box2dd.lib
echo.
cmake --build . --config=Release
copy /y src\Release\box2d.lib ..\..\Lib\Release\box2d.lib
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
copy /y bin\Debug\DirectXTex.dll ..\..\Bin\DirectXTexd.dll
copy /y lib\Debug\DirectXTex.lib ..\Lib\Debug\DirectXTexd.lib
echo.
cmake --build . --config=Release
copy /y bin\Release\DirectXTex.dll ..\..\Bin\DirectXTex.dll
copy /y lib\Release\DirectXTex.lib ..\Lib\Release\DirectXTex.lib

:: Add extra DLLs in DeepLink exe path
:: TODO: Figure out how to do deep link without going into exe folder, or figure out how to load dll:s in code
copy /y bin\Release\DirectXTex.dll ..\..\Bin\EditorDevelopment\DirectXTex.dll
cd .. 

cd freetype
echo.
echo Generating freetype files...
echo.
cmake -E make_directory build
cmake -G "Visual Studio 17 2022" CMakeLists.txt -B build -DBUILD_SHARED_LIBS=ON
echo.
echo Building freetype...
echo.
cmake --build build  
copy /y build\Debug\freetyped.dll ..\..\Bin\freetyped.dll
copy /y build\Debug\freetyped.lib ..\Lib\Debug\freetyped.lib
echo.
cmake --build build --config Release
copy /y build\Release\freetype.dll ..\..\Bin\freetype.dll
copy /y build\Release\freetype.lib ..\Lib\Release\freetype.lib

:: Add extra DLLs in DeepLink exe path
:: TODO: Figure out how to do deep link without going into exe folder, or figure out how to load dll:s in code
copy /y build\Release\freetype.dll ..\..\Bin\EditorDevelopment\freetype.dll
cd .. 

echo.
echo Installing Nuget
echo.
winget install Microsoft.Nuget

echo.
echo Installing GameInput
echo.
winget install Microsoft.GameInput
nuget install Microsoft.GameInput -Version 3.1.26100.6879

echo.
echo Dependencies compiled
PAUSE
