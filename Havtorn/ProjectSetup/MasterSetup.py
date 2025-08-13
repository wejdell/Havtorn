import os
import subprocess
import platform

from SetupMSVC import MSVCConfiguration as MSVCRequirements
MSVCRequirements.Validate()

from SetupPython import PythonConfiguration as PythonRequirements
PythonRequirements.Validate()

subprocess.call([os.path.abspath("./GenerateProjectFiles.bat"), "nopause"])
subprocess.call([os.path.abspath("./CompileDependencies.bat"), "nopause"])
