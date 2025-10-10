from enum import Enum
from enum import auto

class HavtornKeys(Enum):
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

class HavtornMainFoldersUtility:
    @staticmethod
    def get_folder_path(key:HavtornKeys):
        dictionary = {
                HavtornKeys.Core:"Core/",
                HavtornKeys.Engine:"Engine/",
                HavtornKeys.Editor:"Editor/",
                HavtornKeys.Launcher:"Launcher/",
                HavtornKeys.Game:"Game/",
                HavtornKeys.Platform:"Platform/",
                HavtornKeys.GUI:"GUI/",
                HavtornKeys.Shaders:"Engine/Graphics/Shaders/",
                HavtornKeys.PixelShaders:"Engine/Graphics/Shaders/",
                HavtornKeys.VertexShaders:"Engine/Graphics/Shaders/",
                HavtornKeys.GeometryShaders:"Engine/Graphics/Shaders/",
                HavtornKeys.ShaderIncludes:"Engine/Graphics/Shaders/Includes/",
                HavtornKeys.External:"../External/",
        }
        try:
            return dictionary[key]
        except:
            print("No folder path for " + key.name)
            return ""
    
    @staticmethod
    def get_cmake_variable_name(key:HavtornKeys):
        dictionary = {
                HavtornKeys.Core:"set(CORE_FILES\n",
                HavtornKeys.Engine:"set(ENGINE_FILES\n",
                HavtornKeys.Editor:"set(EDITOR_FILES\n",
                HavtornKeys.Launcher:"set(LAUNCHER_FILES\n",
                HavtornKeys.Game:"set(GAME_FILES\n",
                HavtornKeys.Platform:"set(PLATFORM_FILES\n",
                HavtornKeys.GUI:"set(GUI_FILES\n",
                HavtornKeys.PixelShaders:"set(PIXEL_SHADERS\n",
                HavtornKeys.VertexShaders:"set(VERTEX_SHADERS\n",
                HavtornKeys.GeometryShaders:"set(GEOMETRY_SHADERS\n",
                HavtornKeys.ShaderIncludes:"set(SHADER_INCLUDES\n",
        }
        try:
            return dictionary[key]
        except:
            print("No CMake variable name for " + key.name)
            return ""
    
    @staticmethod
    #TODO: what is the actual name for this?
    def get_file_specifier(key:HavtornKeys):
        dictionary = {
                HavtornKeys.PixelShaders:"_PS",
                HavtornKeys.VertexShaders:"_VS",
                HavtornKeys.GeometryShaders:"_GS",
        }
        try:
            return dictionary[key]
        except:
            print("No file specifier for " + key.name)
            return ""
