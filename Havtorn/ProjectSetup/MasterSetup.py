import os
import subprocess
import platform

from SetupMSVC import MSVCConfiguration as MSVCRequirements
MSVCRequirements.Validate()

from SetupPython import PythonConfiguration as PythonRequirements
PythonRequirements.Validate()

from SetupCMake import CMakeConfiguration as CMakeRequirements
cmakeInstalled = CMakeRequirements.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

if (cmakeInstalled):
    if platform.system() == "Windows":
        print("\nRunning CMake...")
        subprocess.call([os.path.abspath("./GenerateProjectFiles.bat"), "nopause"])
        subprocess.call([os.path.abspath("./CompileDependencies.bat"), "nopause"])

    print("\nSetup completed!")
else:
    print("\nCMake is required to setup the project. Cannot build project files.")

