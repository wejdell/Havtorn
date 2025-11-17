import os
import subprocess
import re
import io
import time

from ValidationUtils import ValidationUtil
from TemplateCreatorUtil import TemplateCreatorUtil
from FileCreatorResources import FileCreatorResources
from FileCreatorResources import HavtornFolders
from FileCreatorResources import HavtornFolderUtil

# TODO: Look over if it is possible to restructure how CMakeLists and this script tracks directories -> Generate CMakeLists through script?

class FileCreatorUtil:
    FOLDER_CHOICES = [
        HavtornFolders.Core,
        HavtornFolders.Platform,
        HavtornFolders.GUI,
        HavtornFolders.ImGui,
        HavtornFolders.ImGuizmo,
        HavtornFolders.ImGuiNode,
        HavtornFolders.Engine,
        HavtornFolders.ShaderIncludes,
        HavtornFolders.VertexShaders,
        HavtornFolders.GeometryShaders,
        HavtornFolders.PixelShaders,
        HavtornFolders.Game,
        HavtornFolders.Editor,
        HavtornFolders.Launcher,
    ]

    ALT_CMD_FOLDER_CHOICES = {
        HavtornFolders.Core:["c"],
        HavtornFolders.Platform:["p", "pl", "pt"],
        HavtornFolders.ImGui:["im"],
        HavtornFolders.ImGuizmo:["izmo"],
        HavtornFolders.ImGuiNode:["node"],
        HavtornFolders.Engine:["engi", "ngn"],
        HavtornFolders.ShaderIncludes:["si"],
        HavtornFolders.VertexShaders:["vx", "vs"],
        HavtornFolders.GeometryShaders:["gs"],
        HavtornFolders.PixelShaders:["px", "ps"],
        HavtornFolders.Game:["g"],
        HavtornFolders.Editor:["e"],
        HavtornFolders.Launcher:["l"],
    }

    CMD_ADD_EMPTY = "-f"
    CMD_UNDO = "-u"
    CMD_SWITCH_FOLDER = "-m"
    CMD_EXIT = "-e"

    @classmethod
    def __init__(self):
        self.mainFolder = HavtornFolders.Core
        # (HavtornFolders, template, subFoldersAndFile> 
        self.filesToAdd = []
        self.templatesMap = TemplateCreatorUtil.get_templates_map_from(TemplateCreatorUtil.get_default_file_templates_path())
        self.havtornLicense = FileCreatorResources.get_havtorn_license()
        return

    @classmethod
    def print_command_separator(self):
        print()
        return

    @classmethod
    def select_main_folder(self):
        self.print_command_separator()
        print("Pick a main folder:")
        for choice in self.FOLDER_CHOICES:
            text = "  " + choice.name
            if choice in self.ALT_CMD_FOLDER_CHOICES:
                blankCount = 20 - len(choice.name)
                text += " " * blankCount + "alt: " 
                for alt in self.ALT_CMD_FOLDER_CHOICES[choice]:
                    text += alt + ", "
            print(text)

        self.print_command_separator()
        while(True):
            userChoice = input(FileCreatorResources.INPUT_CHARACTERS)
            # TODO some pythonic way of doing this check should exist
            for choice in self.FOLDER_CHOICES:
                userChoice = userChoice.lower()
                if choice.name.lower() != userChoice:
                    if choice not in self.ALT_CMD_FOLDER_CHOICES or userChoice not in self.ALT_CMD_FOLDER_CHOICES[choice]:
                        continue
                
                self.mainFolder = choice
                return
            
            FileCreatorResources.print_error(f'invalid option "{userChoice}"')
        return
    
    @classmethod
    def print_options(self):
        self.print_command_separator()
        print(f' Run command to add folder(s) & file, example: "-<command> F1/f2/File"')
        print(f' Commands:')
        print(f'  {self.CMD_ADD_EMPTY} add an empty file, requires extension e.g: "{self.CMD_ADD_EMPTY} F1/f2/File.h"')
        print(f'  {self.CMD_UNDO} to undo, example: {self.CMD_UNDO} 1')
        print(f'  {self.CMD_SWITCH_FOLDER} to change main folder')
        print(f'  {self.CMD_EXIT} close without generating')
        print(f' Commands with templates:')
        for command in self.templatesMap:
            print(f'  -{command} {self.templatesMap[command][TemplateCreatorUtil.key_description()]}')
        print(f' Run {TemplateCreatorUtil.__name__} to add new template(s)')
        print()
        print(f'  Return/ empty input to continue with generation')
        return
    
    @classmethod
    def print_status(self):
        self.print_command_separator()
        print(f"Main folder: {self.mainFolder.name} - {HavtornFolderUtil.safe_get_folder_path(self.mainFolder)}")
        print(f"Files:")
        for i, (_, _, file) in enumerate(self.filesToAdd):
            print(f'+ [{i + 1}] {file}')
        return
    
    @classmethod
    def valid_folder(self, folderName:str):
        if ValidationUtil.validate_folder_name(folderName) is False:
            FileCreatorResources.print_error(f'"{folderName}" contains invalid characters')
            return False     
        return True
    
    @classmethod
    def valid_file_and_extension(self, fileName:str):
        filenameSplit = fileName.split('.')
        if ValidationUtil.validate_file_name(filenameSplit[0]) is False:
            FileCreatorResources.print_error(f'"{fileName}" contains invalid characters')
            return False
        
        if (len(filenameSplit) == 1 # Missing extension
            or len(filenameSplit) > 2 # More than 1 extension
            or ValidationUtil.validate_file_extension(filenameSplit[1]) is False):
            FileCreatorResources.print_error(f'unsupported extension in "{fileName}"')
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
                    file.write(self.havtornLicense + "\n")
                    
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
                FileCreatorResources.print_error(f'"{fileToAdd}" already exists')
        return

    @classmethod
    def add_file_to_cmake(self, mainFolder:HavtornFolders, fileToAdd:str):
        cmakeTarget = HavtornFolderUtil.CMAKE_KEYS[mainFolder]
        entry = f"\t{fileToAdd}\n"
        fileAsLineList = list[str]
        # Read CMakeLists into a list of lines, append entry and rewrite file
        with open(FileCreatorResources.CMAKE_LIST_PATH, "r") as cmakeFile: 
            fileAsLineList = cmakeFile.readlines()
            fileAsLineList.insert(fileAsLineList.index(cmakeTarget) + 1, entry)
            cmakeFile.flush()
        with open(FileCreatorResources.CMAKE_LIST_PATH, "w") as cmakeFile:
            cmakeFile.writelines(fileAsLineList)
        return
        
    @classmethod
    def generate_and_flush(self):
        self.generate_files()
        for (mainFolder, _, fileToAdd) in self.filesToAdd:
            self.add_file_to_cmake(mainFolder, fileToAdd)
        
        print("\nRegenerating project ...")
        subprocess.call([os.path.abspath(FileCreatorResources.GENERATOR_PATH), "nopause"])
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
        
        fileToAdd = HavtornFolderUtil.try_append_nomenclature_suffix(self.mainFolder, fileToAdd)

        pendingAddition = (self.mainFolder, "custom", HavtornFolderUtil.FOLDER_PATHS[self.mainFolder] + fileToAdd)                        
        if pendingAddition in self.filesToAdd:
            FileCreatorResources.print_error(f"trying to add duplicate {fileToAdd}")
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
            FileCreatorResources.print_error(f'"{fileName}" contains invalid characters')
            return False
        
        fileToAddSplit = HavtornFolderUtil.try_append_nomenclature_suffix(self.mainFolder, fileToAddSplit)

        for fileType in self.templatesMap[template][TemplateCreatorUtil.key_file_types()]:
            pendingAddition = (self.mainFolder, template,  HavtornFolderUtil.FOLDER_PATHS[self.mainFolder] + fileToAddSplit + "." + fileType[TemplateCreatorUtil.key_extension()])                        
            if pendingAddition in self.filesToAdd:
                FileCreatorResources.print_error(f"trying to add duplicate {fileToAdd}")
                continue
            self.filesToAdd.append(pendingAddition)

        return True

    @classmethod
    def process_commands(self):
        while(True):
            self.print_options()
            self.print_status()

            userInput = input(FileCreatorResources.INPUT_CHARACTERS)

            # Process commands without args
            if userInput == self.CMD_SWITCH_FOLDER:
                self.select_main_folder()
                continue 
            
            if userInput == "":
                self.generate_and_flush()
                self.print_command_separator()
                return
            
            if userInput == self.CMD_EXIT:
                return

            # Process commands with args
            try:
                (command, args) = userInput.split(" ")
            except:
                FileCreatorResources.print_error("Error processing input, expected: -<command> <folder/file>")
                continue

            for templateName in self.templatesMap:
                if command == "-" + templateName:
                    if not self.try_add_file_for_template(args, templateName):
                        break
            
            if command == self.CMD_ADD_EMPTY: 
                self.try_add_file_with_extension(args)
                continue
            
            # TODO: nice to have - handle multiple indices at once
            if command == self.CMD_UNDO:
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
    fileCreator = FileCreatorUtil()
    print("-- File Creation Utility --")
    fileCreator.select_main_folder()
    print()
    print("Add any number of files and then generate")
    print("Use arrow-keys up/down to scroll between previous input, arrow-keys left/right to move the cursor")
    fileCreator.process_commands()
    print("Closing ...")
    time.sleep(0.5)
