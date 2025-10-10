import os
import re
from glob import glob
from enum import Enum
from enum import auto
from collections import defaultdict

# repeating the same mistakes: starting before thinking things through
# what do we want: be able to modify/move files in file-explorer and update CMakeTexts through script
# what do we need for that: search main-folders and replace the entire entries of files under the main-folder sections
# create a generator template of CMakeLists 
class HavtornMainFoldersUtility:
    class Keys(Enum):
        Core = auto()
        Engine = auto()
        Editor = auto()
        Launcher = auto()
        Game = auto()
        Platform = auto()
        GUI = auto()
        Shaders = auto()
        PixelShaders = auto()
        VertexShaders = auto()
        GeometryShaders = auto()
        ShaderIncludes = auto()
        External = auto()

    @staticmethod
    def get_folder_path(key:Keys):
        dictionary = {
                HavtornMainFoldersUtility.Keys.Core:"Core/",
                HavtornMainFoldersUtility.Keys.Engine:"Engine/",
                HavtornMainFoldersUtility.Keys.Editor:"Editor/",
                HavtornMainFoldersUtility.Keys.Launcher:"Launcher/",
                HavtornMainFoldersUtility.Keys.Game:"Game/",
                HavtornMainFoldersUtility.Keys.Platform:"Platform/",
                HavtornMainFoldersUtility.Keys.GUI:"GUI/",
                HavtornMainFoldersUtility.Keys.Shaders:"Engine/Graphics/Shaders/",
                HavtornMainFoldersUtility.Keys.PixelShaders:"Engine/Graphics/Shaders/",
                HavtornMainFoldersUtility.Keys.VertexShaders:"Engine/Graphics/Shaders/",
                HavtornMainFoldersUtility.Keys.GeometryShaders:"Engine/Graphics/Shaders/",
                HavtornMainFoldersUtility.Keys.ShaderIncludes:"Engine/Graphics/Shaders/Includes/",
                HavtornMainFoldersUtility.Keys.External:"../External/",
        }
        try:
            return dictionary[key]
        except:
            print("No folder path for " + key.name)
            return ""
    
    @staticmethod
    def get_cmake_variable_name(key:Keys):
        dictionary = {
                HavtornMainFoldersUtility.Keys.Core:"set(CORE_FILES\n",
                HavtornMainFoldersUtility.Keys.Engine:"set(ENGINE_FILES\n",
                HavtornMainFoldersUtility.Keys.Editor:"set(EDITOR_FILES\n",
                HavtornMainFoldersUtility.Keys.Launcher:"set(LAUNCHER_FILES\n",
                HavtornMainFoldersUtility.Keys.Game:"set(GAME_FILES\n",
                HavtornMainFoldersUtility.Keys.Platform:"set(PLATFORM_FILES\n",
                HavtornMainFoldersUtility.Keys.GUI:"set(GUI_FILES\n",
                HavtornMainFoldersUtility.Keys.PixelShaders:"set(PIXEL_SHADERS\n",
                HavtornMainFoldersUtility.Keys.VertexShaders:"set(VERTEX_SHADERS\n",
                HavtornMainFoldersUtility.Keys.GeometryShaders:"set(GEOMETRY_SHADERS\n",
                HavtornMainFoldersUtility.Keys.ShaderIncludes:"set(SHADER_INCLUDES\n",
        }
        try:
            return dictionary[key]
        except:
            print("No CMake variable name for " + key.name)
            return ""
    
    @staticmethod
    #TODO: what is the actual name for this?
    def get_file_specifier(key:Keys):
        dictionary = {
                HavtornMainFoldersUtility.Keys.PixelShaders:"_PS",
                HavtornMainFoldersUtility.Keys.VertexShaders:"_VS",
                HavtornMainFoldersUtility.Keys.GeometryShaders:"_GS",
        }
        try:
            return dictionary[key]
        except:
            print("No file specifier for " + key.name)
            return ""
    

# Scans Havtorn's Source-folder for files and adds them to CMakeLists
# External-folders are excluded from scan since we want a handpicked selection from them
class CMakeTextsGenerator:
    #TODO: what is the actual name for this?
    class HasFileSpecifiers(Enum):
        Yes = 0,
        No = 1

    targets = {
        HavtornMainFoldersUtility.Keys.Core,
        HavtornMainFoldersUtility.Keys.Engine,
        HavtornMainFoldersUtility.Keys.Editor,
        HavtornMainFoldersUtility.Keys.Launcher,
        HavtornMainFoldersUtility.Keys.Platform,
        HavtornMainFoldersUtility.Keys.Game,
        HavtornMainFoldersUtility.Keys.GUI,
        HavtornMainFoldersUtility.Keys.PixelShaders,
        HavtornMainFoldersUtility.Keys.VertexShaders,
        HavtornMainFoldersUtility.Keys.GeometryShaders,
        HavtornMainFoldersUtility.Keys.ShaderIncludes,
    }

    targetSplitters = {

    }

    fileNameFilters = {
        HavtornMainFoldersUtility.Keys.PixelShaders:{
            HavtornMainFoldersUtility.get_file_specifier( HavtornMainFoldersUtility.Keys.PixelShaders),
            },
        HavtornMainFoldersUtility.Keys.VertexShaders:{
            HavtornMainFoldersUtility.get_file_specifier( HavtornMainFoldersUtility.Keys.VertexShaders),
            },
        HavtornMainFoldersUtility.Keys.GeometryShaders:{
            HavtornMainFoldersUtility.get_file_specifier( HavtornMainFoldersUtility.Keys.GeometryShaders),
            },
    }

    exclusions = {
        HavtornMainFoldersUtility.Keys.Engine:{
            'Shaders',
            },
    }
    # put in __init__
    filesToAdd = defaultdict(list[str])

    #
    # _cmakeLists_ = read CMakeListsTemplate.txt as line list
    # For each _entry_ in _filesToAdd_
    #   find _index_ in _cmakeLists_ of _entry_.value
    #   For each _file_ in _filesToAdd[_entry_]
    #       insert _file_ at _index_+1
    #
    @classmethod
    def Test(self):
        for target in self.targets:
            self.filesToAdd.update({target : list[str]()})
            try:
                exclusions = ''
                if target in self.exclusions:
                    exclusions = '|'.join(self.exclusions[target])

                fileNameFilters = ''
                if target in self.fileNameFilters:
                    fileNameFilters = '|'.join(self.fileNameFilters[target])
                
                for file in glob(os.path.abspath(HavtornMainFoldersUtility.get_folder_path(target)) + '/**', recursive=True):
                    if not os.path.isfile(file):
                        continue
                    if (exclusions and re.search(exclusions, file)):
                        continue
                    if (fileNameFilters and not re.search(fileNameFilters, file)):
                        continue
                    
                    self.filesToAdd[target].append(file.split("Source\\")[1])
            except Exception as e:
                print(e)

        template = "CMakeListsTemplate.txt"
        cmakeLists = "CMakeListsTest.txt"
        
        for target in self.targets:
            print(target.name)
            for file in self.filesToAdd[target]:
                print("\t" + file)
        return

    @classmethod
    def Do(self):
        # for all main folders
            # get all [cpp, h, ..etc] 
        for mainFolder in self.mainFolders:
            print(mainFolder)
            try:
                for file in glob(os.path.abspath(mainFolder) + '/**', recursive=True):
                    if os.path.isfile(file):
                        print("\t" + file.split("Source\\")[1])
            except Exception as e:
                print(e)

        print(self.externalFolder)
        try:
            for file in glob(os.path.abspath(self.externalFolder) + '/**', recursive=True):
                if os.path.isfile(file):
                    print("\t ..\\External\\ " + file.split("\\External\\")[1])
        except Exception as e:
            print(e)
            #for file in os.listdir(mainFolder):
            #    try:
            #        print("\t" + os.path.abspath(file) + "  " + f"{os.path.isfile(os.path.abspath(file))}")
            #    except Exception as e:
            #        print(e)

        while(True):
            c = input("c to close")
            if c == "c":
                break
        return

if __name__ == "__main__":
    generator = CMakeTextsGenerator()
    generator.Test()