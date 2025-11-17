from enum import Enum
from enum import auto
import time
import os

class FileCreatorResources:
    INPUT_CHARACTERS = ">> "
    CMAKE_LIST_PATH = "CmakeLists.txt"
    GENERATOR_PATH = "./../ProjectSetup/GenerateProjectFiles.bat"
    LICENSE_PATH = "./../ProjectSetup/License.txt"

    @staticmethod
    def print_error(errorMessage:str):
        print(f'<!> {errorMessage}')
        time.sleep(1) # attempt at improving visibility of error
        return

    @staticmethod
    def get_havtorn_license():
        # No exception handling: 
        # it should blow up if we can't access the license file
        havtornLicence = ""
        with open(FileCreatorResources.LICENSE_PATH, "r") as file:
            havtornLicence = file.read()
        
        # TODO add way to change the comment-char used e.g replace "//" with language appropriate char
        return havtornLicence

class HavtornFolders(Enum):
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
        ImGui = auto()
        ImGuizmo = auto()
        ImGuiNode = auto()

class HavtornFolderUtil:
    FOLDER_PATHS = {
        HavtornFolders.Core:"Core/",
        HavtornFolders.Engine:"Engine/",
        HavtornFolders.Editor:"Editor/",
        HavtornFolders.Launcher:"Launcher/",
        HavtornFolders.Game:"Game/",
        HavtornFolders.Platform:"Platform/",
        HavtornFolders.GUI:"GUI/",
        HavtornFolders.Shaders:"Engine/Graphics/Shaders/",
        HavtornFolders.PixelShaders:"Engine/Graphics/Shaders/",
        HavtornFolders.VertexShaders:"Engine/Graphics/Shaders/",
        HavtornFolders.GeometryShaders:"Engine/Graphics/Shaders/",
        HavtornFolders.ShaderIncludes:"Engine/Graphics/Shaders/Includes/",
        HavtornFolders.External:"../External/",
        HavtornFolders.ImGui:"../External/imgui/",
        HavtornFolders.ImGuizmo:"../External/ImGuizmo/",
        HavtornFolders.ImGuiNode:"../External/imgui-node-editor/",
    }

    CMAKE_KEYS = {
        HavtornFolders.Core:"set(CORE_FILES\n",
        HavtornFolders.Engine:"set(ENGINE_FILES\n",
        HavtornFolders.Editor:"set(EDITOR_FILES\n",
        HavtornFolders.Launcher:"set(LAUNCHER_FILES\n",
        HavtornFolders.Game:"set(GAME_FILES\n",
        HavtornFolders.Platform:"set(PLATFORM_FILES\n",
        HavtornFolders.GUI:"set(GUI_FILES\n",
        HavtornFolders.PixelShaders:"set(PIXEL_SHADERS\n",
        HavtornFolders.VertexShaders:"set(VERTEX_SHADERS\n",
        HavtornFolders.GeometryShaders:"set(GEOMETRY_SHADERS\n",
        HavtornFolders.ShaderIncludes:"set(SHADER_INCLUDES\n",
        HavtornFolders.ImGui:"set(IMGUI_FOLDER\n",
        HavtornFolders.ImGuizmo:"set(IMGUIZMO_FOLDER\n",
        HavtornFolders.ImGuiNode:"set(IMGUI_NODE_FOLDER\n",
    }

    NOMENCLATURE_SUFFIX = {
        HavtornFolders.PixelShaders:"_PS",
        HavtornFolders.VertexShaders:"_VS",
        HavtornFolders.GeometryShaders:"_GS",
    }
    
    @staticmethod
    def safe_get_folder_path(key:HavtornFolders):
        try:
            return HavtornFolderUtil.FOLDER_PATHS[key]
        except:
            print("No folder path for " + key.name)
            return ""
    
    @staticmethod
    def safe_get_cmake_variable_name(key:HavtornFolders):
        try:
            return HavtornFolderUtil.CMAKE_KEYS[key]
        except:
            print("No CMake variable name for " + key.name)
            return ""
    
    @staticmethod
    def safe_get_nomenclature_suffix(key:HavtornFolders):
        try:
            return HavtornFolderUtil.NOMENCLATURE_SUFFIX[key]
        except:
            print("No nomenclature suffix for " + key.name)
            return ""
        
    @staticmethod
    def try_append_nomenclature_suffix(key:HavtornFolders, target:str):
        suffix = HavtornFolderUtil.safe_get_nomenclature_suffix(key)
        if suffix == "":
            return target

        if suffix in target:
            target = target.replace(suffix, "")

        (name, extension) = (target, "")
        if "." in target:
            (name, extension) = target.split(".")
    
        if extension != "":
            target = name + suffix + "." + extension
        else:
            target = name + suffix
            
        print(f"Verified suffix {suffix} for {target}")
        return target
    
if __name__ == "__main__":
    print("FileCreatorResources - TESTS")
    print(FileCreatorResources.INPUT_CHARACTERS)
    
    if os.path.exists(FileCreatorResources.CMAKE_LIST_PATH):
        print(f"Can find {FileCreatorResources.CMAKE_LIST_PATH}")
    else:
        FileCreatorResources.print_error(f"Can't find {FileCreatorResources.CMAKE_LIST_PATH}")
    
    if os.path.exists(FileCreatorResources.GENERATOR_PATH):
        print(f"Can find {FileCreatorResources.GENERATOR_PATH}")
    else:
        FileCreatorResources.print_error(f"Can't find {FileCreatorResources.GENERATOR_PATH}")

    if os.path.exists(FileCreatorResources.LICENSE_PATH):
        print(f"Can find {FileCreatorResources.LICENSE_PATH}")
    else:
        FileCreatorResources.print_error(f"Can't find {FileCreatorResources.LICENSE_PATH}")
    
    FileCreatorResources.get_havtorn_license()
    FileCreatorResources.print_error("test error")

    print()
    
    HavtornFolderUtil.try_append_nomenclature_suffix(HavtornFolders.Core, "CoreShouldHaveNone.WithExtension")
    HavtornFolderUtil.try_append_nomenclature_suffix(HavtornFolders.Core, "CoreShouldHaveNone")
    HavtornFolderUtil.try_append_nomenclature_suffix(HavtornFolders.PixelShaders, "PixelShaderShouldHave.WithExtension")
    HavtornFolderUtil.try_append_nomenclature_suffix(HavtornFolders.PixelShaders, "PixelShaderShouldHave")
    HavtornFolderUtil.try_append_nomenclature_suffix(HavtornFolders.VertexShaders, "VertexShaderAlreadyHas_VS")
    HavtornFolderUtil.try_append_nomenclature_suffix(HavtornFolders.VertexShaders, "VertexShaderAlreadyHas_VS.WithExtension")
    HavtornFolderUtil.try_append_nomenclature_suffix(HavtornFolders.GeometryShaders, "GeometryShaderShouldHave.WithExtension")
    HavtornFolderUtil.try_append_nomenclature_suffix(HavtornFolders.ShaderIncludes, "ShaderInclShouldHaveNone.WithExtension")

    for folder in HavtornFolders:
        print(folder.name)
        
        path = HavtornFolderUtil.safe_get_folder_path(folder)
        if path:
            print(" Path: " + repr(path))
        
        cmake = HavtornFolderUtil.safe_get_cmake_variable_name(folder)
        if cmake:
            print(" CMake var: " + repr(cmake))

        fileSpec =  HavtornFolderUtil.safe_get_nomenclature_suffix(folder)
        if fileSpec:
            print(" File spec: " + repr(fileSpec))
    
        print()
    input("Any key to continue ...")
