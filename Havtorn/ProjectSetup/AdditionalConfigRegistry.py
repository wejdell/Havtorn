import os
import winreg
from pathlib import Path

class RegistryConfiguration:
    
    @classmethod
    def InstallDeeplink(cls):
        print("Adding Deeplink to Registry")

        PROTOCOL_NAME = "Havtorn"
        ENGINE_PROTOCOL = fr"URL:{PROTOCOL_NAME}"
        LAUNCHER_PATH = str(os.path.abspath("../Bin/EditorDevelopment/EditorDevelopment.exe"))

        base = winreg.CreateKey(winreg.HKEY_CURRENT_USER, fr"Software\Classes\{PROTOCOL_NAME}")
        winreg.SetValueEx(base, None, 0, winreg.REG_SZ, ENGINE_PROTOCOL)
        winreg.SetValueEx(base, "URL Protocol", 0, winreg.REG_SZ, "")

        icon = winreg.CreateKey(base, "DefaultIcon")
        winreg.SetValueEx(icon, None, 0, winreg.REG_SZ, f"\"{LAUNCHER_PATH}\",1")

        cmd = winreg.CreateKey(base, r"shell\open\command")
        winreg.SetValueEx(cmd, None, 0, winreg.REG_SZ, f"\"{LAUNCHER_PATH}\" \"%1\"")

if __name__ == "__main__":
    RegistryConfiguration.InstallDeeplink()
