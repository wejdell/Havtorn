import os
import subprocess
import re
import io
import time

from ValidationUtils import ValidationUtil

# TODO: Look over if it is possible to restructure how CMakeLists and this script tracks directories -> Generate CMakeLists through script?

class FileCreationUtil:
    # class variables, should not be altered
    # TODO: If more filetypes are supported the characters used for the license comment need to be filtered, same goes for namespace structure
    havtornLicense="// Copyright 2025 Team Havtorn. All Rights Reserved.\n\n"
    havtornNameSpace="\nnamespace Havtorn\n{\n\n}\n"
    cmakeListFilePath="CMakeLists.txt"
    
    inputCharacters = ">> "

    generatorScriptPath = "./../ProjectSetup/GenerateProjectFiles.bat"

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
    mainFolderChoices=sorted(mainFolderChoices)

    choiceToCMakeCollection={
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
    choiceToFolder={
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

    addFileCommand = "-f"
    addNewNodeCommand = "-node"  # -node MyCoolFolder/SickNode
    addFileSingle = "-sf"
    undoFileCommand = "-u"
    switchMainCommand = "-m"
    continueCommand = "-g"

    @classmethod
    def __init__(self):
        self.mainFolder = ""
        self.filesToAdd = []
        return

    @classmethod
    def print_command_separator(self):
        print()
        return

    @classmethod
    def on_error(self, errorMessage:str):
        print(f'<!> {errorMessage}')
        time.sleep(1)
        return

    @classmethod
    def select_main_folder(self):
        self.print_command_separator()
        print("Pick a main folder:")
        for choice in self.mainFolderChoices:
            print("\t" + choice)

        self.print_command_separator()
        while(True):
            self.mainFolder=input(self.inputCharacters)
            if self.mainFolder in self.mainFolderChoices:
                break
            else:
                self.on_error(f'invalid option "{self.mainFolder}"')
        return
    
    @classmethod
    def print_options(self):
        self.print_command_separator()
        print(f' {self.addFileCommand} to add folder & file, example: "F1/f2/ex.cpp"')
        print(f' Some file-extensions have associated files auto-generated, example: "ex.h" gets a "ex.cpp"')
        print(f' {self.addNewNodeCommand} Create a new Node and generate base node implementation  Example: -node SubDirectory/NewNodeName')
        print(f' {self.addFileSingle} same as {self.addFileCommand} without auto-generation of associated file')
        print(f' {self.undoFileCommand} to undo, example: {self.undoFileCommand} 1')
        print(f' {self.switchMainCommand} to change main folder')
        print(f' {self.continueCommand} continue to file generation')
        return
    
    @classmethod
    def print_status(self):
        self.print_command_separator()
        print(f"Main folder: {self.choiceToFolder[self.mainFolder]}")
        print(f"Files:")
        for i, (_, file) in enumerate(self.filesToAdd):
            print(f'+ [{i + 1}] {file}')
        return
    
    @classmethod
    def try_add_associated_file(self, fileName:str, folderNames:list[str]):
        # based on file extension determine if an additional file should be added
        fileNameSplit = fileName.split('.')
        extension = fileNameSplit[1]
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
            case _:
                return
                
        folders = "/".join(folderNames)
        self.filesToAdd.append((self.mainFolder, self.choiceToFolder[self.mainFolder] + folders + "/" + fileNameSplit[0] + "." + associatedExtension))
        return
    
    @classmethod
    def valid_folder(self, folderName:str):
        if ValidationUtil.validate_folder_name(folderName) is False:
            self.on_error(f'"{folderName}" contains invalid characters')
            return False     
        return True
    
    @classmethod
    def valid_file(self, fileName:str):
        filenameSplit = fileName.split('.')
        if ValidationUtil.validate_file_name(filenameSplit[0]) is False:
            self.on_error(f'"{fileName}" contains invalid characters')
            return False
        
        if (len(filenameSplit) == 1 # Missing extension
            or len(filenameSplit) > 2 # More than 1 extension
            or ValidationUtil.validate_file_extension(filenameSplit[1]) is False):
            self.on_error(f'unsupported extension in "{fileName}"')
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
    def try_implement_base_node(self, fileName:str, fileStream:io.TextIOWrapper):
        (name, extension) = fileName.split('.')
        checks = [
            extension.lower() in [
                'h', 
                'hpp',
                ],
        ]
        if all(checks):
            headerFile = f"""
            #pragma once
            #include <HexRune/HexRune.h>

            namespace Havtorn
            {{
                namespace HexRune
                {{
                    struct {name} : public SNode
                    {{
                        GAME_API {name} (const U64 id, const U32 typeID, SScript* owningScript);
                        virtual GAME_API I8 OnExecute() override;
                    }}; 
                }}
            }}
            """
            fileStream.write(headerFile)
    

        (name, extension) = fileName.split('.')
        checks = [
            extension.lower() in [
                'c', 
                'cpp',
                ],
        ]
        if all(checks):
            cppFile = f"""
            #include "{name}.h"

            namespace Havtorn
            {{
                namespace HexRune
                {{
                    {name}::{name}(const U64 id, const U32 typeID, SScript* owningScript)
                        : SNode(id, typeID, owningScript, ENodeType::Standard)
                    {{
                        AddInput(UGUIDManager::Generate(), EPinType::Flow, "In");
                        AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Out");                    
                    }}

                    I8 {name}::OnExecute()
                    {{
                        return -1;
                    }}
                }}
            }}
            """
            fileStream.write(cppFile)    

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
    def generate_node_files(self, fileToAdd:str):
        (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
        folders = "/".join(folderNames)
        if not os.path.exists(folders):
            os.makedirs(folders)

        fileTypes = [".h", ".cpp"]
        for fileType in fileTypes:
            try:
                with open(fileToAdd + fileType, "x") as file:
                    file.write(self.havtornLicense)
                    self.try_implement_base_node(fileName + fileType, file)
                    print(f'> File "{fileToAdd  + fileType}" created')
            except FileExistsError:
                self.on_error(f'"{fileToAdd  + fileType}" already exists')

        return

    @classmethod
    def add_file_to_cmake(self, mainFolder:str, fileToAdd:str):
        # Read CMakeLists into a list of lines, append entry and rewrite file
        cmakeTarget=f"set({self.choiceToCMakeCollection[mainFolder]}\n"
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
        subprocess.call([os.path.abspath(self.generatorScriptPath), "nopause"])
        self.filesToAdd = []
        return
    
    @classmethod
    def generate_node_and_flush(self, fileToAdd:str):
        self.generate_node_files(self.choiceToFolder[self.mainFolder] + fileToAdd)
        self.add_file_to_cmake(self.mainFolder, self.choiceToFolder[self.mainFolder] + fileToAdd + ".h")
        self.add_file_to_cmake(self.mainFolder, self.choiceToFolder[self.mainFolder] + fileToAdd + ".cpp")
        print("\nRegenerating project ...")
        subprocess.call([os.path.abspath(self.generatorScriptPath), "nopause"])
        self.filesToAdd = []
        return

    @classmethod
    def try_add_file(self, fileToAdd:str):
        if fileToAdd == "":
            return False

        (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
        foldersValid = True
        for filePart in folderNames:
            if not self.valid_folder(filePart):
                foldersValid = False
        if not foldersValid:
            return False
        if not self.valid_file(fileName):
            return False
        
        pendingAddition = (self.mainFolder, self.choiceToFolder[self.mainFolder] + fileToAdd)                        
        if pendingAddition in self.filesToAdd:
            self.on_error(f"trying to add duplicate {fileToAdd}")
            return False
        self.filesToAdd.append(pendingAddition)
        return True

    @classmethod
    def process_commands(self):
        while(True):
            self.print_options()
            self.print_status()

            userInput=input(self.inputCharacters)

            if self.continueCommand in userInput:
                self.generate_and_flush()
                self.print_command_separator()
                break

            if self.addFileCommand in userInput: 
                fileToAdd = "".join(userInput.replace(f"{self.addFileCommand}", '').split())
                if not self.try_add_file(fileToAdd):
                    continue
                
                (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
                self.try_add_associated_file(fileName, folderNames)
                continue

            if self.addNewNodeCommand in userInput:
                fileToAdd = "".join(userInput.replace(f"{self.addNewNodeCommand}", '').split())
                (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
                foldersValid = True
                for filePart in folderNames:
                    if not self.valid_folder(filePart):
                        foldersValid = False
                if not foldersValid:
                    break
                if "." in fileToAdd:
                    fileToAdd = fileToAdd.split(".")[0] 
            
                self.generate_node_and_flush(fileToAdd)
                self.print_command_separator()
            
            if self.addFileSingle in userInput: 
                fileToAdd = "".join(userInput.replace(f"{self.addFileSingle}", '').split())
                self.try_add_file(fileToAdd)
                continue
            
            # TODO: nice to have - handle multiple indices at once
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
            
            if self.switchMainCommand in userInput:
                self.select_main_folder()
                continue 
        return

if __name__ == "__main__":
    print("** File Creation Utility **")
    print("Instructions:")
    print(" 1 - Select a main folder (can be changed), any file added will be placed under it")
    print(" 2 - Add as many files as you want.")
    print(" 3 - Generate: generation updates CMakeLists and updates the project files")
    print(" Use arrow-key up to retrieve previous input")
    print()
    fileCreator = FileCreationUtil()
    while(True):
        fileCreator.select_main_folder()
        fileCreator.process_commands()