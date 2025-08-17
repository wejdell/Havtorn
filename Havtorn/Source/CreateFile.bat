@echo off
set PLATFORM=PLATFORM_FILES 
set CORE=CORE_FILES
set GUI=GUI_FILES
set IMGUI=IMGUI_FILES
set ENGINE=ENGINE_FILES
set SHADERINCL=SHADER_INCLUDES
set VERTEX=VERTEX_SHADER
set GEOMETRY=GEOMETRY_SHADERS 
set PIXEL=PIXEL_SHADERS
set GAME=GAME_FILES
set EDITOR=EDITOR_FILES
set LAUNCHER=LAUNCHER_FILES
:: Give options for directory
:: Give options for filetype
:: Create file(s) in folders
:: Add to CMakeLists.txt
:: GenerateProjectFiles
py AddFileToCMakeList.py
PAUSE