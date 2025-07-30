import os
import sys
import subprocess
from pathlib import Path
import pprint

import Utils

from io import BytesIO
from urllib.request import urlopen

class CMakeConfiguration:
    cmakeVersion = "3.31.4"
    cmakeDirectory = "./CMake"
    cmakeZipUrls = f"https://github.com/Kitware/CMake/releases/download/v{cmakeVersion}/cmake-{cmakeVersion}-windows-x86_64.zip"

    @classmethod
    def Validate(cls):
        if (not cls.CheckIfCMakeInstalled()):
            print("CMake is not installed.")
            return False

        #print(f"Correct CMake located at {os.path.abspath(cls.cmakeDirectory)}")
        return True

    @classmethod
    def CheckIfCMakeInstalled(cls):
        #this work if user has env variable with CMake in it
        env_var = os.environ
        path_env_var = env_var["Path"]
        cmake_filter = "CMake"
        path_env_var_split = path_env_var.split(";")
        filtered_cmake_path = list(filter(lambda p: cmake_filter in p, path_env_var_split))
        #os.environ["Path"].add("C:\Users\AkiGonzalez\source\repos\Havtorn\Havtorn\ProjectSetup\SetupCMake.py")
        
        if (not filtered_cmake_path):
            return cls.InstallCMake()

        # Could search custom directory or find using path
        cmakeExe = Path(f"{filtered_cmake_path[0]}/cmake.exe");
        #cmakeExe = Path(f"{cls.cmakeDirectory}/cmake-{cls.cmakeVersion}-windows-x86_64/bin/cmake.exe");
        if (not cmakeExe.exists()):
            return cls.InstallCMake()

        print(f"CMake found: {cmakeExe}")
        # verify version
        return True

    @classmethod
    def InstallCMake(cls):
        permissionGranted = False
        while not permissionGranted:
            reply = str(input("CMake not found. Would you like to download CMake {0:s}? [Y/N]: ".format(cls.cmakeVersion))).lower().strip()[:1]
            if reply == 'n':
                return False
            permissionGranted = (reply == 'y')
        # does this CMake installtion add PATH Env_Var?
        # if installer is used, PATH is set by default
        cmakePath = f"{cls.cmakeDirectory}/cmake-{cls.cmakeVersion}-windows.zip"
        print("Downloading {0:s} to {1:s}".format(cls.cmakeZipUrls, cmakePath))
        Utils.DownloadFile(cls.cmakeZipUrls, cmakePath)
        print("Extracting", cmakePath)
        Utils.UnzipFile(cmakePath, deleteZipFile=True)
        print(f"CMake {cls.cmakeVersion} has been downloaded to '{cls.cmakeDirectory}'")

        return True

if __name__ == "__main__":
    CMakeConfiguration.Validate()
