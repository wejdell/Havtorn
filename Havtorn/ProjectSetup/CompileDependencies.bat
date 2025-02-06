@echo off
echo.

cd ..\External\assimp
REM echo Generating assimp CMake files...
REM echo.
REM cmake CMakeLists.txt
REM echo.
REM echo Building assimp...
REM echo.
REM cmake --build .
REM echo.
copy bin\Debug\assimp-vc143-mtd.dll ..\..\Bin\
copy lib\Debug\assimp-vc143-mtd.lib ..\Lib\Debug\
cd .. 

cd PhysX\physx
REM echo Generating PhysX files...
REM echo.
REM call generate_projects.bat vc17win64 
REM echo.
REM echo Building PhysX...
REM echo.
REM cmake --build compiler/vc17win64 
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
REM echo.
cd ..\..

cd box2d
REM echo.
REM rmdir /s /q build
REM mkdir build
cd build
REM echo Generating box2d files...
REM cmake -DBOX2D_SAMPLES=OFF -DBOX2D_UNIT_TESTS=OFF -S ..
REM echo.
REM echo Building box2d...
REM cmake --build .
REM echo %cd%
copy src\Debug\box2dd.lib ..\..\Lib\Debug\
REM echo.
cd ..
cd ..

cd DirectXTex
REM echo Generating DirectXTex files...
REM echo.
REM cmake CMakeLists.txt -DBUILD_TOOLS=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_SAMPLE=OFF
REM echo.
REM echo Building DirectXTex...
REM echo.
REM cmake --build . 
copy bin\Debug\DirectXTex.dll ..\..\Bin\
copy lib\Debug\DirectXTex.lib ..\Lib\Debug\
cd .. 

echo.