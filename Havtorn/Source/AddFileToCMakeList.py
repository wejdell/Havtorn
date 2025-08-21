import os
import subprocess

havtornLicense="// Copyright 2025 Team Havtorn. All Rights Reserved."

#TODO make these constants
folderChoices={
    "core",
    "platform",
    "gui",
    "imgui",
    "engine",
    "shaderinclude",
    "vertexshader",
    "geometryshader",
    "pixelshader",
    "game",
    "editor",
    "launcher"
    }
# So we always show them in the same order
folderChoices=sorted(folderChoices)

choiceToCMakeCollection={
    'core':"CORE_FILES",
    'platform':"PLATFORM_FILES",
    'gui':"GUI_FILES",
    'imgui':"IMGUI_FILES",
    'engine':"ENGINE_FILES",
    'shaderinclude':"SHADER_INCLUDES",
    'vertexshader':"VERTEX_SHADERS",
    'geometryshader':"GEOMETRY_SHADERS",
    'pixelshader':"PIXEL_SHADERS",
    'game':"GAME_FILES",
    'editor':"EDITOR_FILES",
    'launcher':"LAUNCHER_FILES",
    }

choiceToCMakeFolderVar={
    'core':"CORE_FOLDER",
    'platform':"PLATFORM_FOLDER",
    'gui':"GUI_FOLDER",
    'imgui':"EXTERNAL_FOLDER",
    'engine':"ENGINE_FOLDER",
    'shaderinclude':"SHADER_FOLDER",
    'vertexshader':"SHADER_FOLDER",
    'geometryshader':"SHADER_FOLDER",
    'pixelshader':"SHADER_FOLDER",
    'game':"GAME_FOLDER",
    'editor':"EDITOR_FOLDER",
    'launcher':"LAUNCHER_FOLDER",
    }

shaderFolder = "Engine/Graphics/Shaders/"
choiceToActualFolder={
    'core':"Core/",
    'platform':"Platform/",
    'gui':"GUI/",
    'imgui':"../External/",
    'engine':"Engine/",
    'shaderinclude':shaderFolder + "Includes/",
    'vertexshader':shaderFolder,
    'geometryshader':shaderFolder,
    'pixelshader':shaderFolder,
    'game':"Game/",
    'editor':"Editor/",
    'launcher':"Launcher/",
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
print(f' {fileCommand} to add a file, example: "{fileCommand} File.cpp"')
print(f' {directoryCommand} to add a directory for the files, example: "{directoryCommand} Dir"')
print(f' {undoFileCommand} to undo previous file, example')
print(f' {undoDirectoryCommand} to undo previous directory for the files')
print(f' {continueCommand} to continue with creating files')
additionalDirectories = ""
filesToAdd = []
while(True):
    for file in filesToAdd:
        print(f'+ {choiceToActualFolder[chosenMainFolder]}{additionalDirectories}{file}')
    
    fileInput=input()
    if continueCommand in fileInput:
        break
    
    if directoryCommand in fileInput:
        directoryToAdd = "".join(fileInput.replace(f"{directoryCommand}", '').split())
        if directoryToAdd == "":
            continue
        additionalDirectories += directoryToAdd + "/"
        print(f"> Added directory: {additionalDirectories}")
        continue

    # undo file
    # undo directory

    #TODO: add file extension validation .cpp, .h, etc
    if fileCommand in fileInput:        
        fileToAdd = "".join(fileInput.replace(f"{fileCommand}", '').split())
        if fileToAdd == "":
            continue
        filesToAdd.append(fileToAdd)
        print(f"> Added file: {fileToAdd}")
        continue

print()

# TODO: need actual directory of chosen mainDirectory
if not os.path.exists(choiceToActualFolder[chosenMainFolder] + additionalDirectories):
    os.makedirs(choiceToActualFolder[chosenMainFolder] + additionalDirectories)

for fileName in filesToAdd:
    try:
        with open(choiceToActualFolder[chosenMainFolder] + additionalDirectories + fileName, "x") as file:
            file.write(havtornLicense)
            print(f'> File "{file}" created')
    except FileExistsError:
        print(f'! "{additionalDirectories + fileName}" already exists')

# TODO: add to CMakeList.txt
cmakeListFilePath="CMakeLists.txt"
target=f"set({choiceToCMakeCollection[chosenMainFolder]}\n"

for fileToAdd in filesToAdd:
    entry=f"\t${{{choiceToCMakeFolderVar[chosenMainFolder]}}}{additionalDirectories}{fileToAdd}\n"
    fileAsLineList=list[str]
    with open(cmakeListFilePath, "r") as cmakeFile: 
        fileAsLineList = cmakeFile.readlines()
        # print(fileAsLineList)
        fileAsLineList.insert(fileAsLineList.index(target) + 1, entry)
        cmakeFile.flush()
    with open(cmakeListFilePath, "w") as cmakeFile:
        cmakeFile.writelines(fileAsLineList)

subprocess.call([os.path.abspath("./../ProjectSetup/GenerateProjectFiles.bat"), "nopause"])

input()

# DirectoryMap map with key -> value: DirectoryName -> CMake variable, e.g: GUI -> GUI_FILES
# ToGenerateList<string>
# while true
#   receive input
#   if input == -g 
#       break  
#   validate string contains at least 1 \, e.g: Dir\File
#       report if error, continue
#   validate substring before \ is in DirectoryMap: e.g DirectoryMap.contains(input.substring(index0, indexOfFirst\))
#       report if error, continue
#   add input to ToGenerateList
#
# for files in ToGenerateList
#   create file
#   write license at top of file
#   write file location to CMakeLists.txt 
#
# call subprocess GenerateProjectFiles.bat


# open(dir/file, mode)
# modes:
# r	Opens a file for reading (default)
# w	Open a file for writing. If a file already exists, it deletes all the existing contents and adds new content from the start of the file.
# x	Open a file for exclusive creation. If the file already exists, this operation fails.
# a	Open a file in the append mode and add new content at the end of the file.
# b	Open the file in binary mode.
# t	Opens a file in a text mode (default).
# +	Open a file for updating (reading and writing).