import os
import subprocess

from AdditionalConfigRegistry import RegistryConfiguration as RegistryConfig
RegistryConfig.InstallDeeplink()

from AdditionalConfigPython import PythonConfiguration as PythonRequirements
PythonRequirements.Validate()

subprocess.call([os.path.abspath("./GenerateProjectFiles.bat"), "nopause"])
subprocess.call([os.path.abspath("./CompileDependencies.bat"), "nopause"])
