import os
import sys
import subprocess
from pathlib import Path

class MSVCConfiguration:
    msvcVersion = "17"

    @classmethod
    def Validate(cls):
        if (not cls.CheckIfMSVCInstalled()):
            print("MSVC is not installed.")
            return False

        print(f"Compatible MSVC installation located.")
        return True

    @classmethod
    def CheckIfMSVCInstalled(cls):
        try:
            subprocess.check_call([os.path.abspath("./VerifyBuildTools.bat"), "nopause"])
        except subprocess.CalledProcessError:
            return cls.InstallMSVC()

        return True

    @classmethod
    def InstallMSVC(cls):
        permissionGranted = False
        while not permissionGranted:
            reply = str(input("MSVC not found. Would you like to install the VS Build Tools? [Y/N]: ")).lower().strip()[:1]
            if reply == 'n':
                return False
            permissionGranted = (reply == 'y')

        subprocess.check_call([os.path.abspath("./InstallBuildTools.bat"), "nopause"])

        return True

if __name__ == "__main__":
    MSVCConfiguration.Validate()
