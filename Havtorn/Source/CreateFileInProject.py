import os
import subprocess
import re

from ValidationUtils import ValidationUtil

# TODO: Look over if it is possible to restructure how CMakeLists and this script tracks directories -> Generate CMakeLists through script?

# If more filetypes are added the characters used for the license comment need to be filtered
havtornLicense="// Copyright 2025 Team Havtorn. All Rights Reserved."
cmakeListFilePath="CMakeLists.txt"

core = "core"
platform = "platform"
gui = "gui"
imgui = "imgui"
imguizmo = "imguizmo"
imguinode = "imguinode"
engine = "engine"
shaderinclude = "shaderinclude"
vertex = "vertexshader"
geometry = "geometryshader"
pixel = "pixelshader"
game = "game"
editor = "editor"
launcher = "launcher"

folderChoices={
    core,
    platform,
    gui,
    imgui,
    imguizmo,
    imguinode,
    engine,
    shaderinclude,
    vertex,
    geometry,
    pixel,
    game,
    editor,
    launcher
    }
# So we always show them in the same order
folderChoices=sorted(folderChoices)

choiceToCollection={
    core:"CORE_FILES",
    platform:"PLATFORM_FILES",
    gui:"GUI_FILES",
    imgui:"IMGUI_FILES",
    imguizmo:"IMGUI_FILES",
    imguinode:"IMGUI_FILES",
    engine:"ENGINE_FILES",
    shaderinclude:"SHADER_INCLUDES",
    vertex:"VERTEX_SHADERS",
    geometry:"GEOMETRY_SHADERS",
    pixel:"PIXEL_SHADERS",
    game:"GAME_FILES",
    editor:"EDITOR_FILES",
    launcher:"LAUNCHER_FILES",
    }

shaderFolder = "Engine/Graphics/Shaders/"
choiceToPath={
    core:"Core/",
    platform:"Platform/",
    gui:"GUI/",
    imgui:"../External/imgui/",
    imguizmo:"../External/ImGuizmo/",
    imguinode:"../External/imgui-node-editor/",
    engine:"Engine/",
    shaderinclude:shaderFolder + "Includes/",
    vertex:shaderFolder,
    geometry:shaderFolder,
    pixel:shaderFolder,
    game:"Game/",
    editor:"Editor/",
    launcher:"Launcher/",
    }

# TODO: would be nice not to need this, if using a script to generate CMakeLists this could perhaps be avoided
choiceToCMakeFolderVar={
    core:"CORE_FOLDER",
    platform:"PLATFORM_FOLDER",
    gui:"GUI_FOLDER",
    imgui:"IMGUI_FOLDER",
    imguizmo:"IMGUIZMO_FOLDER",
    imguinode:"IMGUI_NODE_FOLDER",
    engine:"ENGINE_FOLDER",
    shaderinclude:"SHADER_INCL_FOLDER",
    vertex:"SHADER_FOLDER",
    geometry:"SHADER_FOLDER",
    pixel:"SHADER_FOLDER",
    game:"GAME_FOLDER",
    editor:"EDITOR_FOLDER",
    launcher:"LAUNCHER_FOLDER",
    }

print("Pick a main directory:")
for choice in folderChoices:
    print("\t" + choice)

print()# for readability
chosenMainFolder=""
while(True):
    chosenMainFolder=input(">> Choose directory:")
    if chosenMainFolder in folderChoices:
        break
    else:
        print(f'! "{chosenMainFolder}" is not a valid choice')

print()# for readability
fileCommand = "-f"
directoryCommand = "-d"
undoFileCommand = "-uf"
undoDirectoryCommand = "-ud"
continueCommand = "-c"
print(f' {fileCommand} to add a file, example: "{fileCommand} ExampleFile.cpp"')
print(f' {directoryCommand} to add a directory for the files, example: "{directoryCommand} ExampleDir"')
print(f' {undoFileCommand} to undo file, example: {undoFileCommand} 1')
print(f' {undoDirectoryCommand} to undo last entered directory, example: {undoDirectoryCommand}')
print(f' {continueCommand} to continue with creating files')

subDirectories = ""
filesToAdd = []
while(True):
    print()# Empty line, for readability
    print(f"Full directory: {choiceToPath[chosenMainFolder]}{subDirectories}")
    print(f"Files:")
    for file in filesToAdd:
        print(f'+ [{filesToAdd.index(file) + 1}] {choiceToPath[chosenMainFolder]}{subDirectories}{file}')
    
    addInput=input(">> ")
    if continueCommand in addInput:
        break

    if fileCommand in addInput: 
        fileToAdd = "".join(addInput.replace(f"{fileCommand}", '').split())
        if fileToAdd == "":
            continue
        
        filenameSplitForValidation = fileToAdd.split('.')
        if ValidationUtil.validate_file_name(filenameSplitForValidation[0]) is False:
            print("Error: filename contains invalid characters")
            continue
        if (len(filenameSplitForValidation) == 1 # Missing extension
            or len(filenameSplitForValidation) > 2 # More than 1 extension
            or ValidationUtil.validate_file_extension(filenameSplitForValidation[1]) is False):
            print("Error: invalid extension")
            continue
        
        filesToAdd.append(fileToAdd)
        continue
    
    if directoryCommand in addInput:
        directoryToAdd = "".join(addInput.replace(f"{directoryCommand}", '').split())
        if directoryToAdd == "":
            continue

        if ValidationUtil.validate_directory_name(directoryToAdd) is False:
            print("Error: directory name contains invalid characters")
            continue

        subDirectories += directoryToAdd + "/"
        continue
    
    # TODO: figure out how handle multiple indices at once
    if undoFileCommand in addInput:
        filesToUndo = re.findall('[0-9]+', addInput)
        if len(filesToUndo) == 0:
            continue
        undoFileIndex = int(filesToUndo[0]) - 1 # To the user we display them as index+1, meaning input 1 == index 0
        if (len(filesToAdd) == 0
            or undoFileIndex >= len(filesToAdd)):
            continue

        del filesToAdd[undoFileIndex]
        continue
    
    if undoDirectoryCommand in addInput:
        try:
            subDirectories = subDirectories.removesuffix('/')
            subDirectories = subDirectories[:subDirectories.rindex('/')] + "/"
        except:
            subDirectories = ""
        continue

# Empty line, for readability
print()

if not os.path.exists(choiceToPath[chosenMainFolder] + subDirectories):
    os.makedirs(choiceToPath[chosenMainFolder] + subDirectories)

# create the files
for fileName in filesToAdd:
    try:
        with open(choiceToPath[chosenMainFolder] + subDirectories + fileName, "x") as file:
            file.write(havtornLicense)
            print(f'> File "{file}" created')
    except FileExistsError:
        print(f'! "{choiceToPath[chosenMainFolder] +subDirectories + fileName}" already exists')

# Read CMakeLists into a list of lines, append new entires and rewrite file
target=f"set({choiceToCollection[chosenMainFolder]}\n"
for fileToAdd in filesToAdd:
    entry=f"\t${{{choiceToCMakeFolderVar[chosenMainFolder]}}}{subDirectories}{fileToAdd}\n"
    fileAsLineList=list[str]
    with open(cmakeListFilePath, "r") as cmakeFile: 
        fileAsLineList = cmakeFile.readlines()
        # for l in fileAsLineList:
        #     print(l)
        # input()
        fileAsLineList.insert(fileAsLineList.index(target) + 1, entry)
        cmakeFile.flush()
    with open(cmakeListFilePath, "w") as cmakeFile:
        cmakeFile.writelines(fileAsLineList)

print("\nRegenerating project ...")
subprocess.call([os.path.abspath("./../ProjectSetup/GenerateProjectFiles.bat"), "nopause"])
