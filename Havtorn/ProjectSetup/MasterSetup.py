import os
import subprocess
import platform

from SetupPython import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

# from SetupPremake import PremakeConfiguration as PremakeRequirements
from SetupCMake import CMakeConfiguration as CMakeRequirements
# os.chdir('./../') # Change from devtools/scripts directory to root

# premakeInstalled = PremakeRequirements.Validate()
cmakeInstalled = CMakeRequirements.Validate()

# print("\nUpdating submodules...")
# subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

if (cmakeInstalled):
    if platform.system() == "Windows":
        print("\nRunning CMake...")
        subprocess.call([os.path.abspath("./GenerateProjectFiles.bat"), "nopause"])

    print("\nSetup completed!")
else:
    print("\nCMake is required to setup the project. Cannot build project files.")

