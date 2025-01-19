import os
import sys
import subprocess
from pathlib import Path

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

        print(f"Correct CMake located at {os.path.abspath(cls.cmakeDirectory)}")
        return True

    @classmethod
    def CheckIfCMakeInstalled(cls):
        cmakeExe = Path(f"{cls.cmakeDirectory}/cmake-{cls.cmakeVersion}-windows-x86_64/bin/cmake.exe");
        if (not cmakeExe.exists()):
            return cls.InstallCMake()

        return True

    @classmethod
    def InstallCMake(cls):
        permissionGranted = False
        while not permissionGranted:
            reply = str(input("CMake not found. Would you like to download CMake {0:s}? [Y/N]: ".format(cls.cmakeVersion))).lower().strip()[:1]
            if reply == 'n':
                return False
            permissionGranted = (reply == 'y')

        cmakePath = f"{cls.cmakeDirectory}/cmake-{cls.cmakeVersion}-windows.zip"
        print("Downloading {0:s} to {1:s}".format(cls.cmakeZipUrls, cmakePath))
        Utils.DownloadFile(cls.cmakeZipUrls, cmakePath)
        print("Extracting", cmakePath)
        Utils.UnzipFile(cmakePath, deleteZipFile=True)
        print(f"CMake {cls.cmakeVersion} has been downloaded to '{cls.cmakeDirectory}'")

        return True

if __name__ == "__main__":
    CMakeConfiguration.Validate()
