import os
import sys
import subprocess
import winreg
from pathlib import Path

class MSVCConfiguration:
    msvcVersion = "17"

    @classmethod
    def Validate(cls):
        cls.InstallDeeplink()
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
    
    @classmethod
    def InstallDeeplink(cls):
        print("Adding Deeplink to Registry")

        PROTOCOL_NAME = "Havtorn"
        ENGINE_PROTOCOL = fr"URL:{PROTOCOL_NAME}"
        LAUNCHER_PATH = str(os.path.abspath("../Bin/Launcher.exe"))

        base = winreg.CreateKey(winreg.HKEY_CURRENT_USER, fr"Software\Classes\{PROTOCOL_NAME}")
        winreg.SetValueEx(base, None, 0, winreg.REG_SZ, ENGINE_PROTOCOL)
        winreg.SetValueEx(base, "URL Protocol", 0, winreg.REG_SZ, "")

        icon = winreg.CreateKey(base, "DefaultIcon")
        winreg.SetValueEx(icon, None, 0, winreg.REG_SZ, f"\"{LAUNCHER_PATH}\",1")

        cmd = winreg.CreateKey(base, r"shell\open\command")
        winreg.SetValueEx(cmd, None, 0, winreg.REG_SZ, f"\"{LAUNCHER_PATH}\" \"%1\"")

if __name__ == "__main__":
    MSVCConfiguration.Validate()
