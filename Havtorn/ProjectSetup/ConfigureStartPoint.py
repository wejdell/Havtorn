import os
import subprocess
import platform

from ConfigureMSVC import MSVCConfiguration as MSVCRequirements
MSVCRequirements.Validate()

from ConfigurePython import PythonConfiguration as PythonRequirements
PythonRequirements.Validate()

subprocess.call([os.path.abspath("./GenerateProjectFiles.bat"), "nopause"])
subprocess.call([os.path.abspath("./CompileDependencies.bat"), "nopause"])
