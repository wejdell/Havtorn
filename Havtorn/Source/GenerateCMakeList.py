import os
from glob import glob

# this is a bit of a hack to gather the not changing things for CMake in 1 place 
class CMakeTextsTemplate:
    @staticmethod
    def GetProjectSettingsString():
        # For the purpose of preview
        string = 'cmake_minimum_required (VERSION 3.31)\n'
        string += '\n'
        string += 'project (Havtorn\n  DESCRIPTION "Havtorn"\n  LANGUAGES CXX\n  VERSION 1.0.0)\n'
        string += '\n'
        string += '# option(BUILD_TEST_TEMPLATE "Ignore warnings related to TODOs" OFF)\n'
        string += '\n'
        string += '# option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)\n'
        string += '\n'
        string += 'set_property(GLOBAL PROPERTY USE_FOLDERS ON)\n'
        string += '\n'
        string += 'set(CMAKE_CXX_STANDARD 20)\n'
        string += 'set(CMAKE_CXX_STANDARD_REQUIRED ON)\n'
        string += 'set(CMAKE_CXX_EXTENSIONS OFF)\n'
        string += '\n'
        string += 'set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")\n'
        string += 'set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")\n'
        string += 'set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")\n'
        string += '\n'     
        string += 'set(COMMON_COMPILE_DEFINITIONS HV_PLATFORM_WINDOWS HV_DEBUG HV_ENABLE_ASSERTS _UNICODE UNICODE)\n'     
        string += 'set(COMMON_COMPILE_OPTIONS /W4 /WX /MP /ZI /JMC /fp:fast /FC)\n'     
        string += 'set(COMMON_LINK_OPTIONS /WX /SUBSYSTEM:WINDOWS)\n'     
        string += '\n'     
        return string
    
    @staticmethod
    def GetSectionDividerString(name:str):
        return f"# ==================== {name} ====================\n"

class CMakeTextsGenerator:
    mainFolders = {
        "Core/",
        "Editor/",
        "Engine/",
        "Game/",
        "Launcher/",
        "GUI/",
        "Platform/",
        #External, Imgui, ...etc
        # everything under External/rapidjson/include
    }

    externalFolder = "../External/"

    # do special file filter for e.g Imgui:
    # imgui.cpp, imgui.h, imgui_tables.cpp, etc copy the ones currently used in CMakeList.txt
    @classmethod
    def Test(self):
        print(CMakeTextsTemplate.GetSectionDividerString("FOLDERS"))
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