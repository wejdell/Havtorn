import os
import subprocess
import re
import io
import time

from ValidationUtils import ValidationUtil
from TemplateCreatorUtil import TemplateCreatorUtil

# TODO: Look over if it is possible to restructure how CMakeLists and this script tracks directories -> Generate CMakeLists through script?

class FileCreatorUtil:
    # class variables, should not be altered
    # TODO: If more filetypes are supported the characters used for the license comment need to be filtered, same goes for namespace structure
    INPUT_CHARACTERS = ">> "
    CMAKE_LIST_PATH = "CMakeLists.txt"
    GENERATOR_SCRIPT_PATH = "./../ProjectSetup/GenerateProjectFiles.bat"
    LICENSE_PATH = "./../ProjectSetup/License.txt"
    
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
    
    mainFolderChoices = {
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
    mainFolderChoices = sorted(mainFolderChoices)

    choiceToCMakeCollection = {
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
    choiceToFolder = {
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

    addFileSingle = "-f"
    undoFileCommand = "-u"
    switchMainCommand = "-m"
    exitCommand = "-e"

    @classmethod
    def __init__(self):
        self.mainFolder = ""
        # (mainFolder, template, subFoldersAndFile> 
        self.filesToAdd = []
        self.templatesMap = TemplateCreatorUtil.get_templates_map_from(TemplateCreatorUtil.get_default_file_templates_path())
        # No exception handling: it should blow up if we can't access the license file
        with open(self.LICENSE_PATH, "r") as file:
            self.havtornLicense = file.read()
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
            self.mainFolder=input(self.INPUT_CHARACTERS)
            if self.mainFolder in self.mainFolderChoices:
                break
            else:
                self.on_error(f'invalid option "{self.mainFolder}"')
        return
    
    @classmethod
    def print_options(self):
        self.print_command_separator()
        print(f' Run command to add folder(s) & file, example: "-<command> F1/f2/File"')
        print(f' Commands:')
        print(f'  {self.addFileSingle} add an empty file, requires specifying extension e.g: "{self.addFileSingle} F1/f2/File.h"')
        print(f'  {self.undoFileCommand} to undo, example: {self.undoFileCommand} 1')
        print(f'  {self.switchMainCommand} to change main folder')
        print(f'  {self.exitCommand} close without generating')
        print(f' Commands with templates:')
        for command in self.templatesMap:
            print(f'  -{command} {self.templatesMap[command][TemplateCreatorUtil.key_description()]}')
        print(f' Run TemplateCreatorUtil to add new template(s)')
        print()
        print(f'  Return/ empty input to continue with generation')
        return
    
    @classmethod
    def print_status(self):
        self.print_command_separator()
        print(f"Main folder: {self.choiceToFolder[self.mainFolder]}")
        print(f"Files:")
        for i, (_, _, file) in enumerate(self.filesToAdd):
            print(f'+ [{i + 1}] {file}')
        return
    
    @classmethod
    def valid_folder(self, folderName:str):
        if ValidationUtil.validate_folder_name(folderName) is False:
            self.on_error(f'"{folderName}" contains invalid characters')
            return False     
        return True
    
    @classmethod
    def valid_file_and_extension(self, fileName:str):
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
    def generate_files(self):
        for (_, template, fileToAdd) in self.filesToAdd:
            (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
            folders = "/".join(folderNames)
            if not os.path.exists(folders):
                os.makedirs(folders)

            try:
                with open(fileToAdd, "x") as file:
                    file.write(self.havtornLicense)
                    
                    if template in self.templatesMap:
                        for fileTypes in self.templatesMap[template][TemplateCreatorUtil.key_file_types()]:
                            if fileTypes[TemplateCreatorUtil.key_extension()] not in fileToAdd:
                                continue
                            
                            fileName = fileName.split('.')[0]
                            fileTemplate:str = fileTypes[TemplateCreatorUtil.key_template_content()]
                            fileTemplate = fileTemplate.replace(TemplateCreatorUtil.value_replace(), fileName)
                            file.write(fileTemplate)

                    print(f'> File "{fileToAdd}" created')
            except FileExistsError:
                self.on_error(f'"{fileToAdd}" already exists')
        return

    @classmethod
    def add_file_to_cmake(self, mainFolder:str, fileToAdd:str):
        # Read CMakeLists into a list of lines, append entry and rewrite file
        cmakeTarget=f"set({self.choiceToCMakeCollection[mainFolder]}\n"
        entry=f"\t{fileToAdd}\n"
        fileAsLineList=list[str]
        with open(self.CMAKE_LIST_PATH, "r") as cmakeFile: 
            fileAsLineList = cmakeFile.readlines()
            fileAsLineList.insert(fileAsLineList.index(cmakeTarget) + 1, entry)
            cmakeFile.flush()
        with open(self.CMAKE_LIST_PATH, "w") as cmakeFile:
            cmakeFile.writelines(fileAsLineList)
        return
        
    @classmethod
    def generate_and_flush(self):
        self.generate_files()
        for (mainFolder, _, fileToAdd) in self.filesToAdd:
            self.add_file_to_cmake(mainFolder, fileToAdd)
        print("\nRegenerating project ...")
        subprocess.call([os.path.abspath(self.GENERATOR_SCRIPT_PATH), "nopause"])
        self.filesToAdd = []
        return

    @classmethod
    def try_add_file_with_extension(self, fileToAdd:str):
        if fileToAdd == "":
            return False

        (folderNames, fileName) = self.extract_folders_and_file(fileToAdd)
        foldersValid = True
        for filePart in folderNames:
            if not self.valid_folder(filePart):
                foldersValid = False
        if not foldersValid:
            return False
        if not self.valid_file_and_extension(fileName):
            return False
        
        pendingAddition = (self.mainFolder, "custom", self.choiceToFolder[self.mainFolder] + fileToAdd)                        
        if pendingAddition in self.filesToAdd:
            self.on_error(f"trying to add duplicate {fileToAdd}")
            return False
        self.filesToAdd.append(pendingAddition)
        return True

    @classmethod
    def try_add_file_for_template(self, fileToAdd:str, template:str):
        if fileToAdd == "":
            return False
        # To be on the safe side strip the extension if the user happened to add one
        fileToAddSplit = fileToAdd.split('.')[0]
        
        (folderNames, fileName) = self.extract_folders_and_file(fileToAddSplit)
        foldersValid = True
        for folderPart in folderNames:
            if not self.valid_folder(folderPart):
                foldersValid = False
        if not foldersValid:
            return False
        if ValidationUtil.validate_file_name(fileName) is False:
            self.on_error(f'"{fileName}" contains invalid characters')
            return False
        
        for fileType in self.templatesMap[template][TemplateCreatorUtil.key_file_types()]:
            pendingAddition = (self.mainFolder, template, self.choiceToFolder[self.mainFolder] + fileToAddSplit + "." + fileType[TemplateCreatorUtil.key_extension()])                        
            if pendingAddition in self.filesToAdd:
                self.on_error(f"trying to add duplicate {fileToAdd}")
                continue
            self.filesToAdd.append(pendingAddition)

        return True

    @classmethod
    def process_commands(self):
        while(True):
            self.print_options()
            self.print_status()

            userInput = input(self.INPUT_CHARACTERS)

            # Process commands without args
            if userInput == self.switchMainCommand:
                self.select_main_folder()
                continue 
            
            if userInput == "":
                self.generate_and_flush()
                self.print_command_separator()
                return
            
            if userInput == self.exitCommand:
                return

            # Process commands with args
            try:
                (command, args) = userInput.split(" ")
            except:
                self.on_error("Error processing input, expected: -<command> <folder/file>")
                continue

            for templateName in self.templatesMap:
                if command == "-"+templateName:
                    if not self.try_add_file_for_template(args, templateName):
                        break
            
            if command == self.addFileSingle: 
                self.try_add_file_with_extension(args)
                continue
            
            # TODO: nice to have - handle multiple indices at once
            if command == self.undoFileCommand:
                filesToUndo = re.findall('[0-9]+', args)
                if len(filesToUndo) == 0:
                    continue
                undoFileIndex = int(filesToUndo[0]) - 1 # To the user we display them as index+1, meaning input 1 => index 0
                if (len(self.filesToAdd) == 0
                    or undoFileIndex >= len(self.filesToAdd)):
                    continue

                del self.filesToAdd[undoFileIndex]
                continue
            continue

if __name__ == "__main__":
    print("** File Creation Utility **")
    print("Instructions:")
    print(" 1 - Select a main folder")
    print(" 2 - Add files")
    print(" 3 - Generate files and update project files")
    print(" Use arrow-keys up/down to scroll between previous input, arrow-keys left/right to move the cursor")
    print()
    time.sleep(0.75)
    fileCreator = FileCreatorUtil()
    fileCreator.select_main_folder()
    fileCreator.process_commands()
    print("Closing ...")
    time.sleep(0.5)