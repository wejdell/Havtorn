import os
import subprocess
import re
import io

from ValidationUtils import ValidationUtil

# TODO: Look over if it is possible to restructure how CMakeLists and this script tracks directories -> Generate CMakeLists through script?

class FileCreationUtil:
    # class variables, should not be altered
    # TODO: If more filetypes are supported the characters used for the license comment need to be filtered, same goes for namespace structure
    havtornLicense="// Copyright 2025 Team Havtorn. All Rights Reserved.\n\n"
    havtornNameSpace="\nnamespace Havtorn\n{\n\n}\n" # TODO: true for all file-types?
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
    
    mainFolderChoices={
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
    mainFolderChoices=sorted(mainFolderChoices) # does this work?

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

    addFileCommand = "-f"
    addFileNoExtraCommand = "-sf"
    undoFileCommand = "-u"
    switchMainDirCommand = "-m"
    continueCommand = "-c"

    @classmethod
    def __init__(self):
        self.mainFolder = ""
        self.filesToAdd = []
        return

    @classmethod
    def print_command_separator(self):
        print()

    @classmethod
    def on_error(self, errorMessage):
        print(f'<!> {errorMessage}')
        input()

    @classmethod
    def select_main_dir(self):
        self.print_command_separator()
        print("Pick a main directory:")
        for choice in self.mainFolderChoices:
            print("\t" + choice)

        self.print_command_separator()
        while(True):
            self.mainFolder=input(">>:")
            if self.mainFolder in self.mainFolderChoices:
                break
            else:
                self.on_error(f'invalid option "{self.mainFolder}"')
    
    @classmethod
    def print_options(self):
        self.print_command_separator()
        print(f' {self.addFileCommand} to add folder & file, e.g: "F1/f2/ex.cpp"')
        print(f' Some file-extensions have associated files auto-generated, example: "ex.h" gets a "ex.cpp"')
        print(f' {self.addFileNoExtraCommand} same as {self.addFileCommand} without auto-generated associated file')
        print(f' {self.undoFileCommand} to undo, example: {self.undoFileCommand} 1')
        print(f' {self.switchMainDirCommand} to change main direcotry')
        print(f' {self.continueCommand} to create files')
    
    @classmethod
    def print_status(self):
            self.print_command_separator()
            print(f"Current main directory: {self.choiceToPath[self.mainFolder]}")
            print(f"Files:")
            for i, (_, file) in enumerate(self.filesToAdd):
                print(f'+ [{i + 1}] {file}')
    
    @classmethod
    def try_add_associated_file(self, fileName:str, folderNames:list[str]):
        # based on file extension determine if an additional file should be added
        fileNameSplit = fileName.split('.')
        extension = fileNameSplit[1]
        checks = [
            extension.lower() in [
                'cpp', 
                'c', 
                'h', 
                'hpp', 
                ],
        ]
        if not all(checks):
            return
        
        associatedExtension = ""
        match extension:
            case "cpp":
                associatedExtension = "h"
            case "c":
                associatedExtension = "h"
            case "h":
                associatedExtension = "cpp"
            case "hpp":
                associatedExtension = "cpp"
                
        folders = "/".join(folderNames)
        self.filesToAdd.append((self.mainFolder, self.choiceToPath[self.mainFolder] + folders + "/" + fileNameSplit[0] + "." + associatedExtension))
        return
    
    @classmethod
    def valid_folder(self, folderName:str):
        if ValidationUtil.validate_folder_name(folderName) is False:
            self.on_error(f'folder contains invalid characters "{folderName}"')
            return False     
        return True
    
    @classmethod
    def valid_file(self, fileName:str):
        filenameSplit = fileName.split('.')
        if ValidationUtil.validate_file_name(filenameSplit[0]) is False:
            self.on_error(f'filename contains invalid characters "{fileName}"')
            return False
        
        if (len(filenameSplit) == 1 # Missing extension
            or len(filenameSplit) > 2 # More than 1 extension
            or ValidationUtil.validate_file_extension(filenameSplit[1]) is False):
            self.on_error(f'unsupported extension "{fileName}"')
            return False
        
        return True

    @classmethod
    def extract_folders_and_file(self, fullFile:str):
        folderNames = fullFile.split('/')
        fileName = folderNames[-1]
        folderNames.pop()
        return (folderNames, fileName)

    @classmethod
    def try_add_header_prefixes(self, fileName:str, fileStream:io.TextIOWrapper):
        extension = fileName.split('.')[1]
        checks = [
            extension.lower() in [
                'h', 
                'hpp', 
                ],
        ]
        if not all(checks):
            return
        
        fileStream.write("#pragma once\n")
        return
    
    @classmethod
    def try_include_header(self, fileName:str, fileStream:io.TextIOWrapper):
        fileNameSplit = fileName.split('.')
        checks = [
            fileNameSplit[1].lower() in [
                'c', 
                'cpp', 
                ],
        ]
        if not all(checks):
            return
        
        fileStream.write(f'#include "{fileNameSplit[0]}.h"\n')
        return
    
    @classmethod
    def try_add_namespace(self, fileName:str, fileStream:io.TextIOWrapper):
        # based on file extension determine if namespace can be added
        extension = fileName.split('.')[1]
        checks = [
            extension.lower() in [
                'cpp', 
                'c', 
                'h', 
                'hpp', 
                ],
        ]
        if not all(checks):
            return
        
        fileStream.write(self.havtornNameSpace)
        return

    @classmethod
    def generate_files(self):
        for (_, fileToAdd) in self.filesToAdd:
            (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
            folders = "/".join(folderNames)
            if not os.path.exists(folders):
                os.makedirs(folders)

            try:
                with open(fileToAdd, "x") as file:
                    file.write(self.havtornLicense)
                    self.try_add_header_prefixes(fileName, file)
                    self.try_include_header(fileName, file)
                    self.try_add_namespace(fileName, file)    
                    print(f'> File "{fileToAdd}" created')
            except FileExistsError:
                self.on_error(f'"{fileToAdd}" already exists')
        return

    @classmethod
    def add_file_to_cmake(self, mainFolder:str, fileToAdd:str):
        # Read CMakeLists into a list of lines, append entry and rewrite file
        cmakeTarget=f"set({self.choiceToCollection[mainFolder]}\n"
        entry=f"\t{fileToAdd}\n"
        fileAsLineList=list[str]
        with open(self.cmakeListFilePath, "r") as cmakeFile: 
            fileAsLineList = cmakeFile.readlines()
            fileAsLineList.insert(fileAsLineList.index(cmakeTarget) + 1, entry)
            cmakeFile.flush()
        with open(self.cmakeListFilePath, "w") as cmakeFile:
            cmakeFile.writelines(fileAsLineList)
        return
        
    @classmethod
    def generate_and_flush(self):
        self.generate_files()
        for (mainFolder, fileToAdd) in self.filesToAdd:
            self.add_file_to_cmake(mainFolder, fileToAdd)
        print("\nRegenerating project ...")
        subprocess.call([os.path.abspath("./../ProjectSetup/GenerateProjectFiles.bat"), "nopause"])
        self.filesToAdd = []
        return

    @classmethod
    def process_commands(self):
        while(True):
            self.print_options()
            self.print_status()

            userInput=input(">> ")

            if self.continueCommand in userInput:
                self.generate_and_flush()
                self.print_command_separator()
                break

            if self.addFileCommand in userInput: 
                fileToAdd = "".join(userInput.replace(f"{self.addFileCommand}", '').split())
                if fileToAdd == "":
                    continue

                (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
                foldersValid = True
                for filePart in folderNames:
                    if not self.valid_folder(filePart):
                        foldersValid = False
                if not foldersValid:
                    continue
                if not self.valid_file(fileName):
                    continue
                                        
                self.filesToAdd.append((self.mainFolder, self.choiceToPath[self.mainFolder] + fileToAdd))
                self.try_add_associated_file(fileName, folderNames)
                continue
            
            if self.addFileNoExtraCommand in userInput: 
                fileToAdd = "".join(userInput.replace(f"{self.addFileNoExtraCommand}", '').split())
                if fileToAdd == "":
                    continue

                (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
                foldersValid = True
                for filePart in folderNames:
                    if not self.valid_folder(filePart):
                        foldersValid = False
                if not foldersValid:
                    continue
                if not self.valid_file(fileName):
                    continue
                                        
                self.filesToAdd.append((self.mainFolder, self.choiceToPath[self.mainFolder] + fileToAdd))
                continue
            
            # TODO: figure out how handle multiple indices at once
            if self.undoFileCommand in userInput:
                filesToUndo = re.findall('[0-9]+', userInput)
                if len(filesToUndo) == 0:
                    continue
                undoFileIndex = int(filesToUndo[0]) - 1 # To the user we display them as index+1, meaning input 1 == index 0
                if (len(self.filesToAdd) == 0
                    or undoFileIndex >= len(self.filesToAdd)):
                    continue

                del self.filesToAdd[undoFileIndex]
                continue
            
            if self.switchMainDirCommand in userInput:
                self.select_main_dir()
                continue
            
    
if __name__ == "__main__":
    print("** File Creation Utility **")
    print("Instructions:")
    print(" 1 - Select a main directory (can be changed), any file added will be placed under it")
    print(" 2 - Add as many files as you want.")
    print(" 3 - Generate: generation updates CMakeLists and updates the project files")
    print()
    fileCreator = FileCreationUtil()
    while(True):
        fileCreator.select_main_dir()
        fileCreator.process_commands()