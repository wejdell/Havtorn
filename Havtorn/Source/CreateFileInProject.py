import os
import subprocess

from ValidationUtils import ValidationUtil

# TODO: Look over if it is possible to restructure how CMakeLists and this script tracks directories
havtornLicense="// Copyright 2025 Team Havtorn. All Rights Reserved."
cmakeListFilePath="CMakeLists.txt"

core="core"
platform="platform"
gui="gui"
imgui="imgui"
engine="engine"
shaderinclude="shaderinclude"
vertex="vertexshader"
geometry="geometryshader"
pixel="pixelshader"
game="game"
editor="editor"
launcher="launcher"

folderChoices={
    core,
    platform,
    gui,
    imgui,
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
    imgui:"../External/",
    engine:"Engine/",
    shaderinclude:shaderFolder + "Includes/",
    vertex:shaderFolder,
    geometry:shaderFolder,
    pixel:shaderFolder,
    game:"Game/",
    editor:"Editor/",
    launcher:"Launcher/",
    }

print("Pick a main directory:")
for choice in folderChoices:
    print("\t" + choice)

print("Directory:")
chosenMainFolder=""
while(True):
    chosenMainFolder=input()
    if chosenMainFolder in folderChoices:
        break
    else:
        print(f'! "{chosenMainFolder}" is not a valid choice')

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

additionalDirectories = ""
filesToAdd = []
while(True):
    # Empty line
    print()
    for file in filesToAdd:
        print(f'+ [{filesToAdd.index(file)}] {choiceToPath[chosenMainFolder]}{additionalDirectories}{file}')
    
    addInput=input()
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
        if (len(filenameSplitForValidation) == 1 is True # Missing extension
            or len(filenameSplitForValidation) > 2 is True # More than 1 extension
            or ValidationUtil.validate_file_extension(filenameSplitForValidation[1]) is False):
            print("Error: invalid extension")
            continue
        
        filesToAdd.append(fileToAdd)
        print(f"> Added file: {fileToAdd}")
        continue
    
    if directoryCommand in addInput:
        directoryToAdd = "".join(addInput.replace(f"{directoryCommand}", '').split())
        if directoryToAdd == "":
            continue

        if ValidationUtil.validate_directory_name(directoryToAdd) is False:
            print("Error: directory name contains invalid characters")
            continue

        additionalDirectories += directoryToAdd + "/"
        print(f"> Added directory: {additionalDirectories}")
        continue
    
    if undoFileCommand in addInput:
        fileToUndo = "".join(addInput.replace(f"{undoFileCommand}", '').split())
        continue
    
    if undoDirectoryCommand in addInput:
        continue

# Empty line, for readability
print()

if not os.path.exists(choiceToPath[chosenMainFolder] + additionalDirectories):
    os.makedirs(choiceToPath[chosenMainFolder] + additionalDirectories)

for fileName in filesToAdd:
    try:
        with open(choiceToPath[chosenMainFolder] + additionalDirectories + fileName, "x") as file:
            file.write(havtornLicense)
            print(f'> File "{file}" created')
    except FileExistsError:
        print(f'! "{additionalDirectories + fileName}" already exists')

# TODO: add to CMakeList.txt
target=f"set({choiceToCollection[chosenMainFolder]}\n"

for fileToAdd in filesToAdd:
    entry=f"\t${choiceToPath[chosenMainFolder]}{additionalDirectories}{fileToAdd}\n"
    fileAsLineList=list[str]
    with open(cmakeListFilePath, "r") as cmakeFile: 
        fileAsLineList = cmakeFile.readlines()
        # print(fileAsLineList)
        fileAsLineList.insert(fileAsLineList.index(target) + 1, entry)
        cmakeFile.flush()
    with open(cmakeListFilePath, "w") as cmakeFile:
        cmakeFile.writelines(fileAsLineList)

subprocess.call([os.path.abspath("./../ProjectSetup/GenerateProjectFiles.bat"), "nopause"])
