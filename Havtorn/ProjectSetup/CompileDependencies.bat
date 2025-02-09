@echo off
echo.

cd ..\External\assimp
echo Generating assimp CMake files...
echo.
cmake CMakeLists.txt
echo.
echo Building assimp...
echo.
cmake --build .
echo.
copy bin\Debug\assimp-vc143-mtd.dll ..\..\Bin\
copy lib\Debug\assimp-vc143-mtd.lib ..\Lib\Debug\
cd .. 

cd PhysX\physx
echo Generating PhysX files...
echo.
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
echo.
cd ..\..

cd box2d
echo.
rmdir /s /q build
mkdir build
cd build
echo Generating box2d files...
cmake -DBOX2D_SAMPLES=OFF -DBOX2D_UNIT_TESTS=OFF -S ..
echo.
echo Building box2d...
cmake --build .
echo %cd%
copy src\Debug\box2dd.lib ..\..\Lib\Debug\
echo.
cd ..
cd ..

cd DirectXTex
echo Generating DirectXTex files...
echo.
cmake CMakeLists.txt -DBUILD_TOOLS=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_SAMPLE=OFF
echo.
echo Building DirectXTex...
echo.
cmake --build . 
copy bin\Debug\DirectXTex.dll ..\..\Bin\
copy lib\Debug\DirectXTex.lib ..\Lib\Debug\
cd .. 

echo.